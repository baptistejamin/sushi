#include <thread>
#include <deque>

#include "logging.h"
#include "jack_frontend.h"
#include "library/random_note_player.h"
namespace sushi {

namespace audio_frontend {

MIND_GET_LOGGER;


AudioFrontendStatus JackFrontend::init(BaseAudioFrontendConfiguration* config)
{
    auto ret_code = BaseAudioFrontend::init(config);
    if (ret_code != AudioFrontendStatus::OK)
    {
        return ret_code;
    }

    auto jack_config = static_cast<JackFrontendConfiguration*>(_config);
    return setup_client(jack_config->client_name, jack_config->server_name);
}


void JackFrontend::cleanup()
{
    if (_client)
    {
        jack_client_close(_client);
        _client = nullptr;
    }
}


void JackFrontend::run()
{
    int status = jack_activate(_client);
    if (status != 0)
    {
        MIND_LOG_ERROR("Failed to activate Jack client, error {}.", status);
    }
    connect_ports();
    // TODO - get the sample rate in here somehow.

    bool run = true;
   /* This runs the randomizer loop to generate random midi notes */
    std::thread rand_thread(dev_util::random_note_player, &_event_queue, &run);

    sleep(1000);
    run = false;
    sleep(1);
    rand_thread.join();
}


AudioFrontendStatus JackFrontend::setup_client(const std::string client_name,
                                               const std::string server_name)
{
    jack_status_t jack_status;
    jack_options_t options = JackNullOption;
    if (!server_name.empty())
    {
        MIND_LOG_ERROR("Using option JackServerName");
        options = JackServerName;
    }
    /* Start Jack client */
    _client = jack_client_open(client_name.c_str(), options, &jack_status, server_name.c_str());
    if (!_client)
    {
        MIND_LOG_ERROR("Failed to open Jack server, error: {}.", jack_status);
        return AudioFrontendStatus::AUDIO_HW_ERROR;
    }

    /* Set the process callback and send the 'this' pointer as data */
    int ret = jack_set_process_callback(_client, rt_process_callback, this);
    if (ret != 0)
    {
        MIND_LOG_ERROR("Failed to set Jack callback function, error: {}.", ret);
        return AudioFrontendStatus::AUDIO_HW_ERROR;
    }
    return setup_ports();
}


AudioFrontendStatus JackFrontend::setup_ports()
{
    /* Setup and register ports */
    int port_no = 0;
    for (auto& port : _output_ports)
    {
        port = jack_port_register (_client,
                                   std::string("audio_output_" + std::to_string(port_no++)).c_str(),
                                   JACK_DEFAULT_AUDIO_TYPE,
                                   JackPortIsOutput,
                                   0);
        if (!port)
        {
            MIND_LOG_ERROR("Failed to open Jack output port {}.", port_no - 1);
            return AudioFrontendStatus::AUDIO_HW_ERROR;
        }
    }
    port_no = 0;
    for (auto& port : _input_ports)
    {
        port = jack_port_register (_client,
                                   std::string("audio_input_" + std::to_string(port_no++)).c_str(),
                                   JACK_DEFAULT_AUDIO_TYPE,
                                   JackPortIsInput,
                                   0);
        if (!port)
        {
            MIND_LOG_ERROR("Failed to open Jack input port {}.", port_no - 1);
            return AudioFrontendStatus::AUDIO_HW_ERROR;
        }
    }
    return AudioFrontendStatus::OK;
}


AudioFrontendStatus JackFrontend::connect_ports()
{
    const char** out_ports = jack_get_ports(_client, nullptr, nullptr, JackPortIsPhysical|JackPortIsInput);
    if (!out_ports)
    {
        MIND_LOG_ERROR("Failed to get ports from Jack.");
        return AudioFrontendStatus::AUDIO_HW_ERROR;
    }
    for (int id = 0; id < static_cast<int>(_input_ports.size()); ++id)
    {
        if (out_ports[id])
        {
            int ret = jack_connect(_client, jack_port_name(_output_ports[id]), out_ports[id]);
            if (ret != 0)
            {
                MIND_LOG_WARNING("Failed to connect out port {}, error {}.", jack_port_name(_output_ports[id]), id);
            }
        }
    }
    jack_free(out_ports);

    /* Same for input ports */
    const char** in_ports = jack_get_ports(_client, nullptr, nullptr, JackPortIsPhysical|JackPortIsOutput);
    if (!in_ports)
    {
        MIND_LOG_ERROR("Failed to get ports from Jack.");
        return AudioFrontendStatus::AUDIO_HW_ERROR;
    }
    for (int id = 0; id < static_cast<int>(_input_ports.size()); ++id)
    {
        if (in_ports[id])
        {
            int ret = jack_connect(_client, jack_port_name(_input_ports[id]), in_ports[id]);
            if (ret != 0)
            {
                MIND_LOG_WARNING("Failed to connect port {}, error {}.", jack_port_name(_input_ports[id]), id);
            }
        }
    }

    jack_free(in_ports);
    return AudioFrontendStatus::OK;
}


int JackFrontend::internal_process_callback(jack_nframes_t nframes)
{
    if (nframes < 64 || nframes % 64)
    {
        MIND_LOG_WARNING("Chunk size not a multiple of AUDIO_CHUNK_SIZE. Skipping.");
        return 0;
    }
    process_events();
    process_midi();
    process_audio(nframes);
    return 0;
}


void inline JackFrontend::process_events()
{
    while (!_event_queue.isEmpty())
    {
        auto event = _event_queue.pop();
        if (event.valid)
        {
            _engine->send_rt_event(event.item);
        }
    }
}


void inline JackFrontend::process_midi()
{


}


void inline JackFrontend::process_audio(jack_nframes_t nframes)
{
    /* Get pointers to audio buffers from ports */
    std::array<const float*, MAX_FRONTEND_CHANNELS> in_data;
    std::array<float*, MAX_FRONTEND_CHANNELS> out_data;

    for (size_t i = 0; i < in_data.size(); ++i)
    {
        in_data[i] = static_cast<float*>(jack_port_get_buffer(_input_ports[i], nframes));
    }
    for (size_t i = 0; i < in_data.size(); ++i)
    {
        out_data[i] = static_cast<float*>(jack_port_get_buffer(_output_ports[i], nframes));
    }

    /* And process audio in chunks of size AUDIO_CHUNK_SIZE */
    for (jack_nframes_t frames = 0; frames < nframes; frames += AUDIO_CHUNK_SIZE)
    {
        for (size_t i = 0; i < _input_ports.size(); ++i)
        {
            std::copy(in_data[i] + frames, in_data[i] + frames + AUDIO_CHUNK_SIZE, _in_buffer.channel(i));
        }
        _out_buffer.clear();
        _engine->process_chunk(&_in_buffer, &_out_buffer);
        for (size_t i = 0; i < _input_ports.size(); ++i)
        {
            std::copy(_out_buffer.channel(i), _out_buffer.channel(i) + AUDIO_CHUNK_SIZE, out_data[i] + frames);
        }
    }
}

}; // end namespace audio_frontend

}; // end namespace sushi
