/*
 * Copyright 2017-2020 Modern Ancient Instruments Networked AB, dba Elk
 *
 * SUSHI is free software: you can redistribute it and/or modify it under the terms of
 * the GNU Affero General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * SUSHI is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License along with
 * SUSHI.  If not, see http://www.gnu.org/licenses/
 */

/**
 * @brief Implementation of external control interface for sushi.
 * @copyright 2017-2020 Modern Ancient Instruments Networked AB, dba Elk, Stockholm
 */

#include "midi_controller.h"

#include "logging.h"

SUSHI_GET_LOGGER_WITH_MODULE_NAME("controller");

namespace sushi {

namespace ext {
ext::MidiChannel midi_channel_from_int(int channel_int)
{
    switch (channel_int)
    {
        case 0:  return sushi::ext::MidiChannel::MIDI_CH_1;
        case 1:  return sushi::ext::MidiChannel::MIDI_CH_2;
        case 2:  return sushi::ext::MidiChannel::MIDI_CH_3;
        case 3:  return sushi::ext::MidiChannel::MIDI_CH_4;
        case 4:  return sushi::ext::MidiChannel::MIDI_CH_5;
        case 5:  return sushi::ext::MidiChannel::MIDI_CH_6;
        case 6:  return sushi::ext::MidiChannel::MIDI_CH_7;
        case 7:  return sushi::ext::MidiChannel::MIDI_CH_8;
        case 8:  return sushi::ext::MidiChannel::MIDI_CH_9;
        case 9:  return sushi::ext::MidiChannel::MIDI_CH_10;
        case 10: return sushi::ext::MidiChannel::MIDI_CH_11;
        case 11: return sushi::ext::MidiChannel::MIDI_CH_12;
        case 12: return sushi::ext::MidiChannel::MIDI_CH_13;
        case 13: return sushi::ext::MidiChannel::MIDI_CH_14;
        case 14: return sushi::ext::MidiChannel::MIDI_CH_15;
        case 15: return sushi::ext::MidiChannel::MIDI_CH_16;
        case 16: return sushi::ext::MidiChannel::MIDI_CH_OMNI;
        default: return sushi::ext::MidiChannel::MIDI_CH_OMNI;
    }
}
int int_from_midi_channel(ext::MidiChannel channel)
{
    switch (channel)
    {
        case sushi::ext::MidiChannel::MIDI_CH_1: return 0;
        case sushi::ext::MidiChannel::MIDI_CH_2: return 1;
        case sushi::ext::MidiChannel::MIDI_CH_3: return 2;
        case sushi::ext::MidiChannel::MIDI_CH_4: return 3;
        case sushi::ext::MidiChannel::MIDI_CH_5: return 4;
        case sushi::ext::MidiChannel::MIDI_CH_6: return 5;
        case sushi::ext::MidiChannel::MIDI_CH_7: return 6;
        case sushi::ext::MidiChannel::MIDI_CH_8: return 7;
        case sushi::ext::MidiChannel::MIDI_CH_9: return 8;
        case sushi::ext::MidiChannel::MIDI_CH_10: return 9;
        case sushi::ext::MidiChannel::MIDI_CH_11: return 10;
        case sushi::ext::MidiChannel::MIDI_CH_12: return 11;
        case sushi::ext::MidiChannel::MIDI_CH_13: return 12;
        case sushi::ext::MidiChannel::MIDI_CH_14: return 13;
        case sushi::ext::MidiChannel::MIDI_CH_15: return 14;
        case sushi::ext::MidiChannel::MIDI_CH_16: return 15;
        case sushi::ext::MidiChannel::MIDI_CH_OMNI: return 16;
        default: return 16;
    }
}
}

namespace engine {
namespace controller_impl {

ext::MidiCCConnection populate_cc_connection(const midi_dispatcher::CCInputConnection& connection)
{
    ext::MidiCCConnection ext_connection;

    ext_connection.processor_id = connection.input_connection.target;
    ext_connection.parameter_id = connection.input_connection.parameter;
    ext_connection.min_range = connection.input_connection.min_range;
    ext_connection.max_range = connection.input_connection.max_range;
    ext_connection.relative_mode = connection.input_connection.relative;
    ext_connection.channel = ext::midi_channel_from_int(connection.channel);
    ext_connection.port = connection.port;
    ext_connection.cc_number = connection.cc;

    return ext_connection;
}

ext::MidiPCConnection populate_pc_connection(const midi_dispatcher::PCInputConnection& connection)
{
    ext::MidiPCConnection ext_connection;

    ext_connection.processor_id = connection.processor_id;
    ext_connection.channel = ext::midi_channel_from_int(connection.channel);
    ext_connection.port = connection.port;

    return ext_connection;
}


MidiController::MidiController(BaseEngine* engine,
                               midi_dispatcher::MidiDispatcher* midi_dispatcher,
                               ext::ParameterController* parameter_controller) : _engine(engine),
                                                                                 _event_dispatcher(engine->event_dispatcher()),
                                                                                 _midi_dispatcher(midi_dispatcher),
                                                                                 _parameter_controller(parameter_controller)
{}

int MidiController::get_input_ports() const
{
    return _midi_dispatcher->get_midi_inputs();
}

int MidiController::get_output_ports() const
{
    return _midi_dispatcher->get_midi_outputs();
}

std::vector<ext::MidiKbdConnection> MidiController::get_all_kbd_input_connections() const
{
    std::vector<ext::MidiKbdConnection> returns;

    const auto connections = _midi_dispatcher->get_all_kb_input_connections();
    for (auto connection : connections)
    {
        ext::MidiKbdConnection ext_connection;
        ext_connection.track_id = connection.input_connection.target;
        ext_connection.port = connection.port;
        ext_connection.channel = ext::midi_channel_from_int(connection.channel);
        ext_connection.raw_midi = connection.raw_midi;
        returns.push_back(ext_connection);
    }

    return returns;
}

std::vector<ext::MidiKbdConnection> MidiController::get_all_kbd_output_connections() const
{
    std::vector<ext::MidiKbdConnection> returns;

    const auto connections = _midi_dispatcher->get_all_kb_output_connections();
    for (auto connection : connections)
    {
        ext::MidiKbdConnection ext_connection;
        ext_connection.track_id = connection.track_id;
        ext_connection.port = connection.port;
        ext_connection.channel = ext::midi_channel_from_int(connection.channel);
        ext_connection.raw_midi = false;
        returns.push_back(ext_connection);
    }

    return returns;
}

std::vector<ext::MidiCCConnection> MidiController::get_all_cc_input_connections() const
{
    std::vector<ext::MidiCCConnection> returns;

    const auto connections = _midi_dispatcher->get_all_cc_input_connections();
    for (auto connection : connections)
    {
        auto ext_connection = populate_cc_connection(connection);
        returns.push_back(ext_connection);
    }

    return returns;
}

std::vector<ext::MidiPCConnection> MidiController::get_all_pc_input_connections() const
{
    std::vector<ext::MidiPCConnection> returns;

    const auto connections = _midi_dispatcher->get_all_pc_input_connections();
    for (auto connection : connections)
    {
        auto ext_connection = populate_pc_connection(connection);
        returns.push_back(ext_connection);
    }

    return returns;
}

std::pair<ext::ControlStatus, std::vector<ext::MidiCCConnection>>
MidiController::get_cc_input_connections_for_processor(int processor_id) const
{
    std::pair<ext::ControlStatus, std::vector<ext::MidiCCConnection>> returns;
    returns.first = ext::ControlStatus::OK;

    const auto connections = _midi_dispatcher->get_cc_input_connections_for_processor(processor_id);
    for (auto connection : connections)
    {
        auto ext_connection = populate_cc_connection(connection);
        returns.second.push_back(ext_connection);
    }

    return returns;
}

std::pair<ext::ControlStatus, std::vector<ext::MidiPCConnection>>
MidiController::get_pc_input_connections_for_processor(int processor_id) const
{
    std::pair<ext::ControlStatus, std::vector<ext::MidiPCConnection>> returns;
    returns.first = ext::ControlStatus::OK;

    const auto connections = _midi_dispatcher->get_pc_input_connections_for_processor(processor_id);
    for (auto connection : connections)
    {
        auto ext_connection = populate_pc_connection(connection);
        returns.second.push_back(ext_connection);
    }

    return returns;
}

ext::ControlStatus MidiController::connect_kbd_input_to_track(int track_id,
                                                              ext::MidiChannel channel,
                                                              int port,
                                                              bool raw_midi)
{
    const int int_channel = ext::int_from_midi_channel(channel);

    auto lambda = [=] () -> int
    {
        midi_dispatcher::MidiDispatcherStatus status;
        if(!raw_midi)
        {
            status = _midi_dispatcher->connect_kb_to_track(port, track_id, int_channel);
        }
        else
        {
            status = _midi_dispatcher->connect_raw_midi_to_track(port, track_id, int_channel);
        }

        if(status == midi_dispatcher::MidiDispatcherStatus::OK)
        {
            return EventStatus::HANDLED_OK;
        }
        else
        {
            return EventStatus::ERROR;
        }
    };

    // If you get a compilation error here, it is due to a bug in gcc 8 - upgrade to 9.
    auto event = new LambdaEvent(lambda, IMMEDIATE_PROCESS);
    _event_dispatcher->post_event(event);
    return ext::ControlStatus::OK;
}

ext::ControlStatus MidiController::connect_kbd_output_from_track(int track_id,
                                                                 ext::MidiChannel channel,
                                                                 int port)
{
    const int int_channel = ext::int_from_midi_channel(channel);

    auto lambda = [=] () -> int
    {
        midi_dispatcher::MidiDispatcherStatus status;
        status = _midi_dispatcher->connect_track_to_output(port, track_id, int_channel);

        if(status == midi_dispatcher::MidiDispatcherStatus::OK)
        {
            return EventStatus::HANDLED_OK;
        }
        else
        {
            return EventStatus::ERROR;
        }
    };

    auto event = new LambdaEvent(lambda, IMMEDIATE_PROCESS);
    _event_dispatcher->post_event(event);
    return ext::ControlStatus::OK;
}

ext::ControlStatus MidiController::connect_cc_to_parameter(int processor_id,
                                                           int parameter_id,
                                                           ext::MidiChannel channel,
                                                           int port,
                                                           int cc_number,
                                                           float min_range,
                                                           float max_range,
                                                           bool relative_mode)
{
    const int int_channel = ext::int_from_midi_channel(channel);

    auto lambda = [=] () -> int
    {
        auto status = _midi_dispatcher->connect_cc_to_parameter(port, // midi_input maps to port
                                                                processor_id,
                                                                parameter_id,
                                                                cc_number,
                                                                min_range,
                                                                max_range,
                                                                relative_mode,
                                                                int_channel);

        if (status == midi_dispatcher::MidiDispatcherStatus::OK)
        {
            return EventStatus::HANDLED_OK;
        }
        else
        {
            return EventStatus::ERROR;
        }
    };

    auto event = new LambdaEvent(lambda, IMMEDIATE_PROCESS);
    _event_dispatcher->post_event(event);
    return ext::ControlStatus::OK;
}

ext::ControlStatus MidiController::connect_pc_to_processor(int processor_id, ext::MidiChannel channel, int port)
{
    const int int_channel = ext::int_from_midi_channel(channel);

    auto lambda = [=] () -> int
    {
        midi_dispatcher::MidiDispatcherStatus status;

        status = _midi_dispatcher->connect_pc_to_processor(port, // midi_input maps to port
                                                           processor_id,
                                                           int_channel);

        if (status == midi_dispatcher::MidiDispatcherStatus::OK)
        {
            return EventStatus::HANDLED_OK;
        }
        else
        {
            return EventStatus::ERROR;
        }
    };

    auto event = new LambdaEvent(lambda, IMMEDIATE_PROCESS);
    _event_dispatcher->post_event(event);
    return ext::ControlStatus::OK;
}

ext::ControlStatus MidiController::disconnect_kbd_input(int track_id, ext::MidiChannel channel, int port, bool raw_midi)
{
    const int int_channel = ext::int_from_midi_channel(channel);

    auto lambda = [=]() -> int
    {
        midi_dispatcher::MidiDispatcherStatus status;
        if(!raw_midi)
        {
            status = _midi_dispatcher->disconnect_kb_from_track(port, // port maps to midi_input
                                                                track_id,
                                                                int_channel);
        }
        else
        {
            status = _midi_dispatcher->disconnect_raw_midi_from_track(port, // port maps to midi_input
                                                                      track_id,
                                                                      int_channel);
        }

        if(status == midi_dispatcher::MidiDispatcherStatus::OK)
        {
            return EventStatus::HANDLED_OK;
        }
        else
        {
            return EventStatus::ERROR;
        }
    };

    auto event = new LambdaEvent(lambda, IMMEDIATE_PROCESS);

    _event_dispatcher->post_event(event);
    return ext::ControlStatus::OK;
}

ext::ControlStatus MidiController::disconnect_kbd_output(int track_id, ext::MidiChannel channel, int port)
{
    const int int_channel = ext::int_from_midi_channel(channel);

    auto lambda = [=] () -> int
    {
        midi_dispatcher::MidiDispatcherStatus status;
        status = _midi_dispatcher->disconnect_track_from_output(port, track_id, int_channel);

        if(status == midi_dispatcher::MidiDispatcherStatus::OK)
        {
            return EventStatus::HANDLED_OK;
        }
        else
        {
            return EventStatus::ERROR;
        }
    };

    auto event = new LambdaEvent(lambda, IMMEDIATE_PROCESS);
    _event_dispatcher->post_event(event);
    return ext::ControlStatus::OK;
}

ext::ControlStatus MidiController::disconnect_cc(int processor_id, ext::MidiChannel channel, int port, int cc_number)
{
    const int int_channel = ext::int_from_midi_channel(channel);

    auto lambda = [=] () -> int
    {
        const auto status = _midi_dispatcher->disconnect_cc_from_parameter(port, // port maps to midi_input
                                                                           processor_id,
                                                                           cc_number,
                                                                           int_channel);

        if (status == midi_dispatcher::MidiDispatcherStatus::OK)
        {
            return EventStatus::HANDLED_OK;
        }
        else
        {
            return EventStatus::ERROR;
        }
    };

    auto event = new LambdaEvent(lambda, IMMEDIATE_PROCESS);
    _event_dispatcher->post_event(event);
    return ext::ControlStatus::OK;
}

ext::ControlStatus MidiController::disconnect_pc(int processor_id, ext::MidiChannel channel, int port)
{
    const int int_channel = ext::int_from_midi_channel(channel);

    auto lambda = [=] () -> int
    {
        midi_dispatcher::MidiDispatcherStatus status;

        status = _midi_dispatcher->disconnect_pc_from_processor(port,
                                                                processor_id,
                                                                int_channel);

        if (status == midi_dispatcher::MidiDispatcherStatus::OK)
        {
            return EventStatus::HANDLED_OK;
        }
        else
        {
            return EventStatus::ERROR;
        }
    };

    auto event = new LambdaEvent(lambda, IMMEDIATE_PROCESS);
    _event_dispatcher->post_event(event);
    return ext::ControlStatus::OK;
}

ext::ControlStatus MidiController::disconnect_all_cc_from_processor(int processor_id)
{
    auto lambda = [=] () -> int
    {
        const auto status = _midi_dispatcher->disconnect_all_cc_from_processor(processor_id);

        if (status == midi_dispatcher::MidiDispatcherStatus::OK)
        {
            return EventStatus::HANDLED_OK;
        }
        else
        {
            return EventStatus::ERROR;
        }
    };

    auto event = new LambdaEvent(lambda, IMMEDIATE_PROCESS);
    _event_dispatcher->post_event(event);
    return ext::ControlStatus::OK;
}

ext::ControlStatus MidiController::disconnect_all_pc_from_processor(int processor_id)
{
    auto lambda = [=] () -> int
    {
        const auto status = _midi_dispatcher->disconnect_all_pc_from_processor(processor_id);

        if (status == midi_dispatcher::MidiDispatcherStatus::OK)
        {
            return EventStatus::HANDLED_OK;
        }
        else
        {
            return EventStatus::ERROR;
        }
    };

    auto event = new LambdaEvent(lambda, IMMEDIATE_PROCESS);
    _event_dispatcher->post_event(event);
    return ext::ControlStatus::OK;
}

} // namespace controller_impl
} // namespace engine
} // namespace sushi
