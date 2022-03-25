/*
 * Copyright 2017-2019 Modern Ancient Instruments Networked AB, dba Elk
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
 * @brief Sushi Control Service, gRPC service for external control of Sushi
 * @copyright 2017-2019 Modern Ancient Instruments Networked AB, dba Elk, Stockholm
 */

#include "control_service.h"
#include "control_notifications.h"

#include "async_service_call_data.h"

namespace sushi_rpc {

/* Convenience conversion functions between sushi enums and their respective grpc implementations */
inline sushi_rpc::ParameterType::Type to_grpc(const sushi::ext::ParameterType type)
{
    switch (type)
    {
        case sushi::ext::ParameterType::FLOAT:            return sushi_rpc::ParameterType::FLOAT;
        case sushi::ext::ParameterType::INT:              return sushi_rpc::ParameterType::INT;
        case sushi::ext::ParameterType::BOOL:             return sushi_rpc::ParameterType::BOOL;
        default:                                          return sushi_rpc::ParameterType::FLOAT;
    }
}

inline sushi_rpc::PlayingMode::Mode to_grpc(const sushi::ext::PlayingMode mode)
{
    switch (mode)
    {
        case sushi::ext::PlayingMode::STOPPED:      return sushi_rpc::PlayingMode::STOPPED;
        case sushi::ext::PlayingMode::PLAYING:      return sushi_rpc::PlayingMode::PLAYING;
        case sushi::ext::PlayingMode::RECORDING:    return sushi_rpc::PlayingMode::RECORDING;
        default:                                    return sushi_rpc::PlayingMode::PLAYING;
    }
}

inline MidiChannel_Channel to_grpc(const sushi::ext::MidiChannel channel)
{
    switch (channel)
    {
        case sushi::ext::MidiChannel::MIDI_CH_1:    return sushi_rpc::MidiChannel::MIDI_CH_1;
        case sushi::ext::MidiChannel::MIDI_CH_2:    return sushi_rpc::MidiChannel::MIDI_CH_2;
        case sushi::ext::MidiChannel::MIDI_CH_3:    return sushi_rpc::MidiChannel::MIDI_CH_3;
        case sushi::ext::MidiChannel::MIDI_CH_4:    return sushi_rpc::MidiChannel::MIDI_CH_4;
        case sushi::ext::MidiChannel::MIDI_CH_5:    return sushi_rpc::MidiChannel::MIDI_CH_5;
        case sushi::ext::MidiChannel::MIDI_CH_6:    return sushi_rpc::MidiChannel::MIDI_CH_6;
        case sushi::ext::MidiChannel::MIDI_CH_7:    return sushi_rpc::MidiChannel::MIDI_CH_7;
        case sushi::ext::MidiChannel::MIDI_CH_8:    return sushi_rpc::MidiChannel::MIDI_CH_8;
        case sushi::ext::MidiChannel::MIDI_CH_9:    return sushi_rpc::MidiChannel::MIDI_CH_9;
        case sushi::ext::MidiChannel::MIDI_CH_10:   return sushi_rpc::MidiChannel::MIDI_CH_10;
        case sushi::ext::MidiChannel::MIDI_CH_11:   return sushi_rpc::MidiChannel::MIDI_CH_11;
        case sushi::ext::MidiChannel::MIDI_CH_12:   return sushi_rpc::MidiChannel::MIDI_CH_12;
        case sushi::ext::MidiChannel::MIDI_CH_13:   return sushi_rpc::MidiChannel::MIDI_CH_13;
        case sushi::ext::MidiChannel::MIDI_CH_14:   return sushi_rpc::MidiChannel::MIDI_CH_14;
        case sushi::ext::MidiChannel::MIDI_CH_15:   return sushi_rpc::MidiChannel::MIDI_CH_15;
        case sushi::ext::MidiChannel::MIDI_CH_16:   return sushi_rpc::MidiChannel::MIDI_CH_16;
        case sushi::ext::MidiChannel::MIDI_CH_OMNI: return sushi_rpc::MidiChannel::MIDI_CH_OMNI;
        default:                                    return sushi_rpc::MidiChannel::MIDI_CH_OMNI;
    }
}

inline sushi::ext::MidiChannel to_sushi_ext(const MidiChannel_Channel channel)
{
    switch (channel)
    {
        case sushi_rpc::MidiChannel::MIDI_CH_1:    return sushi::ext::MidiChannel::MIDI_CH_1;
        case sushi_rpc::MidiChannel::MIDI_CH_2:    return sushi::ext::MidiChannel::MIDI_CH_2;
        case sushi_rpc::MidiChannel::MIDI_CH_3:    return sushi::ext::MidiChannel::MIDI_CH_3;
        case sushi_rpc::MidiChannel::MIDI_CH_4:    return sushi::ext::MidiChannel::MIDI_CH_4;
        case sushi_rpc::MidiChannel::MIDI_CH_5:    return sushi::ext::MidiChannel::MIDI_CH_5;
        case sushi_rpc::MidiChannel::MIDI_CH_6:    return sushi::ext::MidiChannel::MIDI_CH_6;
        case sushi_rpc::MidiChannel::MIDI_CH_7:    return sushi::ext::MidiChannel::MIDI_CH_7;
        case sushi_rpc::MidiChannel::MIDI_CH_8:    return sushi::ext::MidiChannel::MIDI_CH_8;
        case sushi_rpc::MidiChannel::MIDI_CH_9:    return sushi::ext::MidiChannel::MIDI_CH_9;
        case sushi_rpc::MidiChannel::MIDI_CH_10:   return sushi::ext::MidiChannel::MIDI_CH_10;
        case sushi_rpc::MidiChannel::MIDI_CH_11:   return sushi::ext::MidiChannel::MIDI_CH_11;
        case sushi_rpc::MidiChannel::MIDI_CH_12:   return sushi::ext::MidiChannel::MIDI_CH_12;
        case sushi_rpc::MidiChannel::MIDI_CH_13:   return sushi::ext::MidiChannel::MIDI_CH_13;
        case sushi_rpc::MidiChannel::MIDI_CH_14:   return sushi::ext::MidiChannel::MIDI_CH_14;
        case sushi_rpc::MidiChannel::MIDI_CH_15:   return sushi::ext::MidiChannel::MIDI_CH_15;
        case sushi_rpc::MidiChannel::MIDI_CH_16:   return sushi::ext::MidiChannel::MIDI_CH_16;
        case sushi_rpc::MidiChannel::MIDI_CH_OMNI: return sushi::ext::MidiChannel::MIDI_CH_OMNI;
        default:                                   return sushi::ext::MidiChannel::MIDI_CH_OMNI;
    }
}

inline sushi::ext::PlayingMode to_sushi_ext(const sushi_rpc::PlayingMode::Mode mode)
{
    switch (mode)
    {
        case sushi_rpc::PlayingMode::STOPPED:   return sushi::ext::PlayingMode::STOPPED;
        case sushi_rpc::PlayingMode::PLAYING:   return sushi::ext::PlayingMode::PLAYING;
        case sushi_rpc::PlayingMode::RECORDING: return sushi::ext::PlayingMode::RECORDING;
        default:                                return sushi::ext::PlayingMode::PLAYING;
    }
}

inline sushi_rpc::SyncMode::Mode to_grpc(const sushi::ext::SyncMode mode)
{
    switch (mode)
    {
        case sushi::ext::SyncMode::INTERNAL: return sushi_rpc::SyncMode::INTERNAL;
        case sushi::ext::SyncMode::MIDI:     return sushi_rpc::SyncMode::MIDI;
        case sushi::ext::SyncMode::LINK:     return sushi_rpc::SyncMode::LINK;
        default:                             return sushi_rpc::SyncMode::INTERNAL;
    }
}

inline sushi::ext::SyncMode to_sushi_ext(const sushi_rpc::SyncMode::Mode mode)
{
    switch (mode)
    {
        case sushi_rpc::SyncMode::INTERNAL: return sushi::ext::SyncMode::INTERNAL;
        case sushi_rpc::SyncMode::MIDI:     return sushi::ext::SyncMode::MIDI;
        case sushi_rpc::SyncMode::LINK:     return sushi::ext::SyncMode::LINK;
        default:                            return sushi::ext::SyncMode::INTERNAL;
    }
}

inline const char* to_string(const sushi::ext::ControlStatus status)
{
   switch (status)
    {
        case sushi::ext::ControlStatus::OK:                    return "OK";
        case sushi::ext::ControlStatus::ERROR:                 return "ERROR";
        case sushi::ext::ControlStatus::UNSUPPORTED_OPERATION: return "UNSUPPORTED OPERATION";
        case sushi::ext::ControlStatus::NOT_FOUND:             return "NOT FOUND";
        case sushi::ext::ControlStatus::OUT_OF_RANGE:          return "OUT OF RANGE";
        case sushi::ext::ControlStatus::INVALID_ARGUMENTS:     return "INVALID ARGUMENTS";
        default:                                               return "INTERNAL";
    }
}

inline grpc::Status to_grpc_status(sushi::ext::ControlStatus status, const char* error = nullptr)
{
    if (!error)
    {
        error = to_string(status);
    }
    switch (status)
    {
        case sushi::ext::ControlStatus::OK:
            return ::grpc::Status::OK;

        case sushi::ext::ControlStatus::ERROR:
            return ::grpc::Status(::grpc::StatusCode::UNKNOWN, error);

        case sushi::ext::ControlStatus::UNSUPPORTED_OPERATION:
            return ::grpc::Status(::grpc::StatusCode::FAILED_PRECONDITION, error);

        case sushi::ext::ControlStatus::NOT_FOUND:
            return ::grpc::Status(::grpc::StatusCode::NOT_FOUND, error);

        case sushi::ext::ControlStatus::OUT_OF_RANGE:
            return ::grpc::Status(::grpc::StatusCode::OUT_OF_RANGE, error);

        case sushi::ext::ControlStatus::INVALID_ARGUMENTS:
            return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, error);

        default:
            return ::grpc::Status(::grpc::StatusCode::INTERNAL, error);
    }
}

inline void to_grpc(ParameterInfo& dest, const sushi::ext::ParameterInfo& src)
{
    dest.set_id(src.id);
    dest.mutable_type()->set_type(to_grpc(src.type));
    dest.set_label(src.label);
    dest.set_name(src.name);
    dest.set_unit(src.unit);
    dest.set_automatable(src.automatable);
    dest.set_min_domain_value(src.min_domain_value);
    dest.set_max_domain_value(src.max_domain_value);
}

inline void to_grpc(PropertyInfo& dest, const sushi::ext::PropertyInfo& src)
{
    dest.set_id(src.id);
    dest.set_name(src.name);
    dest.set_label(src.label);
}


inline void to_grpc(sushi_rpc::ProcessorInfo& dest, const sushi::ext::ProcessorInfo& src)
{
    dest.set_id(src.id);
    dest.set_label(src.label);
    dest.set_name(src.name);
    dest.set_parameter_count(src.parameter_count);
    dest.set_program_count(src.program_count);
}

inline void to_grpc(sushi_rpc::MidiKbdConnection& dest, const sushi::ext::MidiKbdConnection& src)
{
    dest.mutable_track()->set_id(src.track_id);
    dest.mutable_channel()->set_channel(to_grpc(src.channel));
    dest.set_port(src.port);
    dest.set_raw_midi(src.raw_midi);
}

inline void to_grpc(sushi_rpc::MidiCCConnection& dest, const sushi::ext::MidiCCConnection& src)
{
    dest.mutable_parameter()->set_processor_id(src.processor_id);
    dest.mutable_parameter()->set_parameter_id(src.parameter_id);
    dest.mutable_parameter()->set_processor_id(src.processor_id);
    dest.mutable_channel()->set_channel(to_grpc(src.channel));
    dest.set_port(src.port);
    dest.set_cc_number(src.cc_number);
    dest.set_min_range(src.min_range);
    dest.set_max_range(src.max_range);
    dest.set_relative_mode(src.relative_mode);
}

inline void to_grpc(sushi_rpc::MidiPCConnection& dest, const sushi::ext::MidiPCConnection& src)
{
    dest.mutable_processor()->set_id(src.processor_id);
    dest.mutable_channel()->set_channel(to_grpc(src.channel));
    dest.set_port(src.port);
}

inline void to_grpc(sushi_rpc::TrackInfo& dest, const sushi::ext::TrackInfo& src)
{
    dest.set_id(src.id);
    dest.set_label(src.label);
    dest.set_name(src.name);
    dest.set_input_channels(src.input_channels);
    dest.set_input_busses(src.input_busses);
    dest.set_output_channels(src.output_channels);
    dest.set_output_busses(src.output_busses);
    for (auto i : src.processors)
    {
        dest.mutable_processors()->Add()->set_id(i);
    }
}

inline void to_grpc(sushi_rpc::CpuTimings& dest, const sushi::ext::CpuTimings& src)
{
    dest.set_average(src.avg);
    dest.set_min(src.min);
    dest.set_max(src.max);
}

inline void to_grpc(sushi_rpc::AudioConnection& dest, const sushi::ext::AudioConnection& src)
{
    dest.mutable_track()->set_id(src.track_id);
    dest.set_track_channel(src.track_channel);
    dest.set_engine_channel(src.engine_channel);
}

inline sushi::ext::PluginType to_sushi_ext(const sushi_rpc::PluginType::Type type)
{
    switch (type)
    {
        case sushi_rpc::PluginType::INTERNAL:       return sushi::ext::PluginType::INTERNAL;
        case sushi_rpc::PluginType::VST2X:          return sushi::ext::PluginType::VST2X;
        case sushi_rpc::PluginType::VST3X:          return sushi::ext::PluginType::VST3X;
        case sushi_rpc::PluginType::LV2:            return sushi::ext::PluginType::LV2;
        default:                                    return sushi::ext::PluginType::INTERNAL;
    }
}

inline void to_grpc(sushi_rpc::ProcessorState& dest, const sushi::ext::ProcessorState& src)
{
    if (src.program.has_value())
    {
        dest.mutable_program_id()->set_value(src.program.value());
        dest.mutable_program_id()->set_has_value(true);
    }
    if (src.bypassed.has_value())
    {
        dest.mutable_bypassed()->set_value(src.bypassed.value());
        dest.mutable_bypassed()->set_has_value(true);
    }
    dest.mutable_properties()->Reserve(src.properties.size());
    for (const auto& p : src.properties)
    {
        auto target = dest.mutable_properties()->Add();
        target->mutable_property()->set_property_id(p.first);
        target->set_value(p.second);
    }
    dest.mutable_parameters()->Reserve(src.parameters.size());
    for (const auto& p : src.parameters)
    {
        auto target = dest.mutable_parameters()->Add();
        target->mutable_parameter()->set_parameter_id(p.first);
        target->set_value(p.second);
    }
}

inline void to_sushi_ext(sushi::ext::ProcessorState& dest, const sushi_rpc::ProcessorState& src)
{
    if (src.program_id().has_value())
    {
        dest.program = src.program_id().value();
    }
    if (src.bypassed().has_value())
    {
        dest.bypassed = src.bypassed().value();
    }
    dest.properties.reserve(src.properties_size());
    for (const auto& p : src.properties())
    {
        dest.properties.push_back({p.property().property_id(), p.value()});
    }
    dest.parameters.reserve(src.parameters_size());
    for (const auto& p : src.parameters())
    {
        dest.parameters.push_back({p.parameter().parameter_id(), p.value()});
    }
}

grpc::Status SystemControlService::GetSushiVersion(grpc::ServerContext* /*context*/,
                                                   const sushi_rpc::GenericVoidValue* /*request*/,
                                                   sushi_rpc::GenericStringValue* response)
{
    response->set_value(_controller->get_sushi_version());
    return grpc::Status::OK;
}

grpc::Status SystemControlService::GetBuildInfo(grpc::ServerContext* /*context*/,
                                                const sushi_rpc::GenericVoidValue* /*request*/,
                                                sushi_rpc::SushiBuildInfo* response)
{
    auto build_info = _controller->get_sushi_build_info();

    response->set_version(build_info.version);

    for (auto& option : build_info.build_options)
    {
        response->add_build_options(option);
    }

    response->set_audio_buffer_size(build_info.audio_buffer_size);
    response->set_commit_hash(build_info.commit_hash);
    response->set_build_date(build_info.build_date);

    return grpc::Status::OK;
}

grpc::Status SystemControlService::GetInputAudioChannelCount(grpc::ServerContext* /*context*/,
                                                             const sushi_rpc::GenericVoidValue* /*request*/,
                                                             sushi_rpc::GenericIntValue* response)
{
    response->set_value(_controller->get_input_audio_channel_count());
    return grpc::Status::OK;
}

grpc::Status SystemControlService::GetOutputAudioChannelCount(grpc::ServerContext* /*context*/,
                                                              const sushi_rpc::GenericVoidValue* /*request*/,
                                                              sushi_rpc::GenericIntValue* response)
{
    response->set_value(_controller->get_output_audio_channel_count());
    return grpc::Status::OK;
}


grpc::Status TransportControlService::GetSamplerate(grpc::ServerContext* /*context*/,
                                                    const sushi_rpc::GenericVoidValue* /*request*/,
                                                    sushi_rpc::GenericFloatValue* response)
{
    response->set_value(_controller->get_samplerate());
    return grpc::Status::OK;
}

grpc::Status TransportControlService::GetPlayingMode(grpc::ServerContext* /*context*/,
                                                     const sushi_rpc::GenericVoidValue* /*request*/,
                                                     sushi_rpc::PlayingMode* response)
{
    response->set_mode(to_grpc(_controller->get_playing_mode()));
    return grpc::Status::OK;
}

grpc::Status TransportControlService::GetSyncMode(grpc::ServerContext* /*context*/,
                                                  const sushi_rpc::GenericVoidValue* /*request*/,
                                                  sushi_rpc::SyncMode* response)
{
    response->set_mode(to_grpc(_controller->get_sync_mode()));
    return grpc::Status::OK;
}

grpc::Status TransportControlService::GetTimeSignature(grpc::ServerContext* /*context*/,
                                                       const sushi_rpc::GenericVoidValue* /*request*/,
                                                       sushi_rpc::TimeSignature* response)
{
    auto ts = _controller->get_time_signature();
    response->set_denominator(ts.denominator);
    response->set_numerator(ts.numerator);
    return grpc::Status::OK;
}

grpc::Status TransportControlService::GetTempo(grpc::ServerContext* /*context*/,
                                               const sushi_rpc::GenericVoidValue* /*request*/,
                                               sushi_rpc::GenericFloatValue* response)
{
    response->set_value(_controller->get_tempo());
    return grpc::Status::OK;
}

grpc::Status TransportControlService::SetTempo(grpc::ServerContext* /*context*/,
                                               const sushi_rpc::GenericFloatValue* request,
                                               sushi_rpc::GenericVoidValue* /*response*/)
{
    auto status = _controller->set_tempo(request->value());
    return to_grpc_status(status);
}

grpc::Status TransportControlService::SetPlayingMode(grpc::ServerContext* /*context*/,
                                                     const sushi_rpc::PlayingMode* request,
                                                     sushi_rpc::GenericVoidValue* /*response*/)
{
    _controller->set_playing_mode(to_sushi_ext(request->mode()));
    return grpc::Status::OK;
}

grpc::Status TransportControlService::SetSyncMode(grpc::ServerContext* /*context*/,
                                                  const sushi_rpc::SyncMode*request,
                                                  sushi_rpc::GenericVoidValue* /*response*/)
{
    _controller->set_sync_mode(to_sushi_ext(request->mode()));
    // TODO - set_sync_mode should return a status, not void
    return grpc::Status::OK;
}

grpc::Status TransportControlService::SetTimeSignature(grpc::ServerContext* /*context*/,
                                                       const sushi_rpc::TimeSignature* request,
                                                       sushi_rpc::GenericVoidValue* /*response*/)
{
    sushi::ext::TimeSignature ts;
    ts.numerator = request->numerator();
    ts.denominator = request->denominator();
    auto status = _controller->set_time_signature(ts);
    return to_grpc_status(status);
}

grpc::Status TimingControlService::GetTimingsEnabled(grpc::ServerContext* /*context*/,
                                                     const sushi_rpc::GenericVoidValue* /*request*/,
                                                     sushi_rpc::GenericBoolValue* response)
{
    response->set_value(_controller->get_timing_statistics_enabled());
    return grpc::Status::OK;
}

grpc::Status TimingControlService::SetTimingsEnabled(grpc::ServerContext* /*context*/,
                                                     const sushi_rpc::GenericBoolValue* request,
                                                     sushi_rpc::GenericVoidValue* /*response*/)
{
    // TODO - should not really be void here
    _controller->set_timing_statistics_enabled(request->value());
    return grpc::Status::OK;
}

grpc::Status TimingControlService::GetEngineTimings(grpc::ServerContext* /*context*/,
                                                    const sushi_rpc::GenericVoidValue* /*request*/,
                                                    sushi_rpc::CpuTimings* response)
{
    auto [status, timings] = _controller->get_engine_timings();
    if (status != sushi::ext::ControlStatus::OK)
    {
        return to_grpc_status(status);
    }
    to_grpc(*response, timings);
    return grpc::Status::OK;
}

grpc::Status TimingControlService::GetTrackTimings(grpc::ServerContext* /*context*/,
                                                   const sushi_rpc::TrackIdentifier* request,
                                                   sushi_rpc::CpuTimings* response)
{
    auto [status, timings] = _controller->get_track_timings(request->id());
    if (status != sushi::ext::ControlStatus::OK)
    {
        return to_grpc_status(status);
    }
    to_grpc(*response, timings);
    return grpc::Status::OK;
}

grpc::Status TimingControlService::GetProcessorTimings(grpc::ServerContext* /*context*/,
                                                       const sushi_rpc::ProcessorIdentifier* request,
                                                       sushi_rpc::CpuTimings* response)
{
    auto [status, timings] = _controller->get_processor_timings(request->id());
    if (status != sushi::ext::ControlStatus::OK)
    {
        return to_grpc_status(status);
    }
    to_grpc(*response, timings);
    return grpc::Status::OK;
}

grpc::Status TimingControlService::ResetAllTimings(grpc::ServerContext* /*context*/,
                                                   const sushi_rpc::GenericVoidValue* /*request*/,
                                                   sushi_rpc::GenericVoidValue* /*response*/)
{
    _controller->reset_all_timings();
    return grpc::Status::OK;
}

grpc::Status TimingControlService::ResetTrackTimings(grpc::ServerContext* /*context*/,
                                                     const sushi_rpc::TrackIdentifier* request,
                                                     sushi_rpc::GenericVoidValue* /*response*/)
{
    auto status = _controller->reset_track_timings(request->id());
    return to_grpc_status(status);
}

grpc::Status TimingControlService::ResetProcessorTimings(grpc::ServerContext* /*context*/,
                                                         const sushi_rpc::ProcessorIdentifier* request,
                                                         sushi_rpc::GenericVoidValue* /*response*/)
{
    auto status = _controller->reset_processor_timings(request->id());
    return to_grpc_status(status);
}

grpc::Status KeyboardControlService::SendNoteOn(grpc::ServerContext* /*context*/,
                                                const sushi_rpc::NoteOnRequest*request,
                                                sushi_rpc::GenericVoidValue* /*response*/)
{
    auto status = _controller->send_note_on(request->track().id(), request->channel(), request->note(), request->velocity());
    return to_grpc_status(status);
}

grpc::Status KeyboardControlService::SendNoteOff(grpc::ServerContext* /*context*/,
                                                 const sushi_rpc::NoteOffRequest* request,
                                                 sushi_rpc::GenericVoidValue* /*response*/)
{
    auto status = _controller->send_note_off(request->track().id(), request->channel(), request->note(), request->velocity());
    return to_grpc_status(status);
}

grpc::Status KeyboardControlService::SendNoteAftertouch(grpc::ServerContext* /*context*/,
                                                        const sushi_rpc::NoteAftertouchRequest* request,
                                                        sushi_rpc::GenericVoidValue* /*response*/)
{
    auto status = _controller->send_note_aftertouch(request->track().id(), request->channel(), request->note(), request->value());
    return to_grpc_status(status);
}

grpc::Status KeyboardControlService::SendAftertouch(grpc::ServerContext* /*context*/,
                                                    const sushi_rpc::NoteModulationRequest* request,
                                                    sushi_rpc::GenericVoidValue* /*response*/)
{
    auto status = _controller->send_aftertouch(request->track().id(), request->channel(), request->value());
    return to_grpc_status(status);
}

grpc::Status KeyboardControlService::SendPitchBend(grpc::ServerContext* /*context*/,
                                                   const sushi_rpc::NoteModulationRequest* request,
                                                   sushi_rpc::GenericVoidValue* /*response*/)
{
    auto status = _controller->send_pitch_bend(request->track().id(), request->channel(), request->value());
    return to_grpc_status(status);
}

grpc::Status KeyboardControlService::SendModulation(grpc::ServerContext* /*context*/,
                                                    const sushi_rpc::NoteModulationRequest* request,
                                                    sushi_rpc::GenericVoidValue* /*response*/)
{
    auto status = _controller->send_modulation(request->track().id(), request->channel(), request->value());
    return to_grpc_status(status);
}

grpc::Status AudioGraphControlService::GetAllProcessors(grpc::ServerContext* /*context*/,
                                                        const sushi_rpc::GenericVoidValue* /*request*/,
                                                        sushi_rpc::ProcessorInfoList* response)
{
    auto processors = _controller->get_all_processors();
    for (const auto& processor : processors)
    {
        auto info = response->add_processors();
        to_grpc(*info, processor);
    }
    return grpc::Status::OK;
}

grpc::Status AudioGraphControlService::GetAllTracks(grpc::ServerContext* /*context*/,
                                                    const sushi_rpc::GenericVoidValue* /*request*/,
                                                    sushi_rpc::TrackInfoList* response)
{
    auto tracks = _controller->get_all_tracks();
    for (const auto& track : tracks)
    {
        auto info = response->add_tracks();
        to_grpc(*info, track);
    }
    return grpc::Status::OK;
}

grpc::Status AudioGraphControlService::GetTrackId(grpc::ServerContext* /*context*/,
                                                  const sushi_rpc::GenericStringValue* request,
                                                  sushi_rpc::TrackIdentifier* response)
{
    auto [status, id] = _controller->get_track_id(request->value());
    if (status != sushi::ext::ControlStatus::OK)
    {
        return to_grpc_status(status, "No track with that name");
    }
    response->set_id(id);
    return grpc::Status::OK;
}

grpc::Status AudioGraphControlService::GetTrackInfo(grpc::ServerContext* /*context*/,
                                                    const sushi_rpc::TrackIdentifier* request,
                                                    sushi_rpc::TrackInfo* response)
{
    auto [status, track] = _controller->get_track_info(request->id());
    if (status != sushi::ext::ControlStatus::OK)
    {
        return to_grpc_status(status, nullptr);
    }
    to_grpc(*response, track);
    return grpc::Status::OK;
}

grpc::Status AudioGraphControlService::GetTrackProcessors(grpc::ServerContext* /*context*/,
                                                          const sushi_rpc::TrackIdentifier* request,
                                                          sushi_rpc::ProcessorInfoList* response)
{
    auto [status, processors] = _controller->get_track_processors(request->id());
    for (const auto& processor : processors)
    {
        auto info = response->add_processors();
        to_grpc(*info, processor);
    }
    return to_grpc_status(status);
}

grpc::Status AudioGraphControlService::GetProcessorId(grpc::ServerContext* /*context*/,
                                                      const sushi_rpc::GenericStringValue* request,
                                                      sushi_rpc::ProcessorIdentifier* response)
{
    auto [status, id] = _controller->get_processor_id(request->value());
    if (status != sushi::ext::ControlStatus::OK)
    {
        return to_grpc_status(status, "No processor with that name");
    }
    response->set_id(id);
    return grpc::Status::OK;
}

grpc::Status AudioGraphControlService::GetProcessorInfo(grpc::ServerContext* /*context*/,
                                                        const sushi_rpc::ProcessorIdentifier* request,
                                                        sushi_rpc::ProcessorInfo* response)
{
    auto [status, processor] = _controller->get_processor_info(request->id());
    if (status != sushi::ext::ControlStatus::OK)
    {
        return to_grpc_status(status);
    }
    to_grpc(*response, processor);
    return grpc::Status::OK;
}

grpc::Status AudioGraphControlService::GetProcessorBypassState(grpc::ServerContext* /*context*/,
                                                               const sushi_rpc::ProcessorIdentifier* request,
                                                               sushi_rpc::GenericBoolValue* response)
{
    auto [status, state] = _controller->get_processor_bypass_state(request->id());
    if (status != sushi::ext::ControlStatus::OK)
    {
        return to_grpc_status(status);
    }
    response->set_value(state);
    return grpc::Status::OK;
}

grpc::Status AudioGraphControlService::GetProcessorState(grpc::ServerContext* /*context*/,
                                                         const sushi_rpc::ProcessorIdentifier* request,
                                                         sushi_rpc::ProcessorState* response)
{
    auto [status, state] = _controller->get_processor_state(request->id());
    if (status != sushi::ext::ControlStatus::OK)
    {
        return to_grpc_status(status);
    }
    to_grpc(*response, state);
    return grpc::Status::OK;
}

grpc::Status AudioGraphControlService::SetProcessorBypassState(grpc::ServerContext* /*context*/,
                                                               const sushi_rpc::ProcessorBypassStateSetRequest* request,
                                                               sushi_rpc::GenericVoidValue* /*response*/)
{
    auto status = _controller->set_processor_bypass_state(request->processor().id(), request->value());
    return to_grpc_status(status);
}

grpc::Status AudioGraphControlService::SetProcessorState(grpc::ServerContext* /*context*/,
                                                         const sushi_rpc::ProcessorStateSetRequest* request,
                                                         sushi_rpc::GenericVoidValue* /*response*/)
{
    sushi::ext::ProcessorState sushi_state;
    to_sushi_ext(sushi_state, request->state());
    auto status = _controller->set_processor_state(request->processor().id(), sushi_state);
    return to_grpc_status(status);
}

grpc::Status AudioGraphControlService::CreateTrack(grpc::ServerContext* /*context*/,
                                                   const sushi_rpc::CreateTrackRequest* request,
                                                   sushi_rpc::GenericVoidValue* /*response*/)
{
    auto status = _controller->create_track(request->name(), request->channels());
    return to_grpc_status(status);
}

grpc::Status AudioGraphControlService::CreateMultibusTrack(grpc::ServerContext* /*context*/,
                                                           const sushi_rpc::CreateMultibusTrackRequest* request,
                                                           sushi_rpc::GenericVoidValue* /*response*/)
{
    auto status = _controller->create_multibus_track(request->name(), request->input_busses(), request->output_busses());
    return to_grpc_status(status);
}

grpc::Status AudioGraphControlService::DeleteTrack(grpc::ServerContext* /*context*/,
                                                   const sushi_rpc::TrackIdentifier* request,
                                                   sushi_rpc::GenericVoidValue* /*response*/)
{
    auto status = _controller->delete_track(request->id());
    return to_grpc_status(status);
}

grpc::Status AudioGraphControlService::CreateProcessorOnTrack(grpc::ServerContext* /*context*/,
                                                              const sushi_rpc::CreateProcessorRequest* request,
                                                              sushi_rpc::GenericVoidValue* /*response*/)
{
    std::optional<int> before_processor = std::nullopt;
    if (request->position().add_to_back() == false)
    {
        before_processor = request->position().before_processor().id();
    }
    auto status = _controller->create_processor_on_track(request->name(),
                                                         request->uid(),
                                                         request->path(),
                                                         to_sushi_ext(request->type().type()),
                                                         request->track().id(),
                                                         before_processor);
    return to_grpc_status(status);
}

grpc::Status AudioGraphControlService::MoveProcessorOnTrack(grpc::ServerContext* /*context*/, const sushi_rpc::MoveProcessorRequest*request,
                                                            sushi_rpc::GenericVoidValue* /*response*/)
{
    std::optional<int> before_processor = std::nullopt;
    if (request->position().add_to_back() == false)
    {
        before_processor = request->position().before_processor().id();
    }
    auto status = _controller->move_processor_on_track(request->processor().id(),
                                                       request->source_track().id(),
                                                       request->dest_track().id(),
                                                       before_processor);
    return to_grpc_status(status);

}

grpc::Status AudioGraphControlService::DeleteProcessorFromTrack(grpc::ServerContext* /*context*/,
                                                                const sushi_rpc::DeleteProcessorRequest* request,
                                                                sushi_rpc::GenericVoidValue* /*response*/)
{
    auto status = _controller->delete_processor_from_track(request->processor().id(),
                                                           request->track().id());
    return to_grpc_status(status);
}

grpc::Status ParameterControlService::GetTrackParameters(grpc::ServerContext* /*context*/,
                                                         const sushi_rpc::TrackIdentifier* request,
                                                         sushi_rpc::ParameterInfoList* response)
{
    auto [status, parameters] = _controller->get_track_parameters(request->id());
    for (const auto& parameter : parameters)
    {
        auto info = response->add_parameters();
        to_grpc(*info, parameter);
    }
    return to_grpc_status(status);
}

grpc::Status ParameterControlService::GetParameterId(grpc::ServerContext* /*context*/,
                                                     const sushi_rpc::ParameterIdRequest* request,
                                                     sushi_rpc::ParameterIdentifier* response)
{
    auto [status, id] = _controller->get_parameter_id(request->processor().id(), request->parametername());
    if (status != sushi::ext::ControlStatus::OK)
    {
        return to_grpc_status(status,  "No parameter with that name");
    }
    response->set_parameter_id(id);
    return grpc::Status::OK;
}

grpc::Status ParameterControlService::GetParameterInfo(grpc::ServerContext* /*context*/,
                                                       const sushi_rpc::ParameterIdentifier* request,
                                                       sushi_rpc::ParameterInfo* response)
{
    auto [status, parameter] = _controller->get_parameter_info(request->processor_id(), request->parameter_id());
    if (status != sushi::ext::ControlStatus::OK)
    {
        return to_grpc_status(status);
    }
    to_grpc(*response, parameter);
    return grpc::Status::OK;
}

grpc::Status ParameterControlService::GetParameterValue(grpc::ServerContext* /*context*/,
                                                        const sushi_rpc::ParameterIdentifier* request,
                                                        sushi_rpc::GenericFloatValue* response)
{
    auto [status, value] = _controller->get_parameter_value(request->processor_id(), request->parameter_id());
    if (status != sushi::ext::ControlStatus::OK)
    {
        return to_grpc_status(status);
    }
    response->set_value(value);
    return grpc::Status::OK;
}

grpc::Status ParameterControlService::GetParameterValueInDomain(grpc::ServerContext* /*context*/,
                                                                const sushi_rpc::ParameterIdentifier* request,
                                                                sushi_rpc::GenericFloatValue* response)
{
    auto [status, value] = _controller->get_parameter_value_in_domain(request->processor_id(), request->parameter_id());
    if (status != sushi::ext::ControlStatus::OK)
    {
        return to_grpc_status(status);
    }
    response->set_value(value);
    return grpc::Status::OK;
}

grpc::Status ParameterControlService::GetParameterValueAsString(grpc::ServerContext* /*context*/,
                                                            const sushi_rpc::ParameterIdentifier* request,
                                                            sushi_rpc::GenericStringValue* response)
{
    auto [status, value] = _controller->get_parameter_value_as_string(request->processor_id(), request->parameter_id());
    if (status != sushi::ext::ControlStatus::OK)
    {
        return to_grpc_status(status);
    }
    response->set_value(value);
    return grpc::Status::OK;
}

grpc::Status ParameterControlService::SetParameterValue(grpc::ServerContext* /*context*/,
                                                        const sushi_rpc::ParameterValue* request,
                                                        sushi_rpc::GenericVoidValue* /*response*/)
{
    auto status = _controller->set_parameter_value(request->parameter().processor_id(),
                                                   request->parameter().parameter_id(),
                                                   request->value());
    return to_grpc_status(status);
}

grpc::Status ProgramControlService::GetProcessorCurrentProgram(grpc::ServerContext* /*context*/,
                                                               const sushi_rpc::ProcessorIdentifier* request,
                                                               sushi_rpc::ProgramIdentifier* response)
{
    auto [status, program] = _controller->get_processor_current_program(request->id());
    if (status != sushi::ext::ControlStatus::OK)
    {
        return to_grpc_status(status);
    }
    response->set_program(program);
    return grpc::Status::OK;
}

grpc::Status ProgramControlService::GetProcessorCurrentProgramName(grpc::ServerContext* /*context*/,
                                                                   const sushi_rpc::ProcessorIdentifier* request,
                                                                   sushi_rpc::GenericStringValue* response)
{
    auto [status, program] = _controller->get_processor_current_program_name(request->id());
    if (status != sushi::ext::ControlStatus::OK)
    {
        return to_grpc_status(status);
    }
    response->set_value(program);
    return grpc::Status::OK;
}

grpc::Status ProgramControlService::GetProcessorProgramName(grpc::ServerContext* /*context*/,
                                                            const sushi_rpc::ProcessorProgramIdentifier* request,
                                                            sushi_rpc::GenericStringValue* response)
{
    auto [status, program] = _controller->get_processor_program_name(request->processor().id(), request->program());
    if (status != sushi::ext::ControlStatus::OK)
    {
        return to_grpc_status(status);
    }
    response->set_value(program);
    return grpc::Status::OK;
}

grpc::Status ProgramControlService::GetProcessorPrograms(grpc::ServerContext* /*context*/,
                                                         const sushi_rpc::ProcessorIdentifier* request,
                                                         sushi_rpc::ProgramInfoList* response)
{
    auto [status, programs] = _controller->get_processor_programs(request->id());
    int id = 0;
    for (auto& program : programs)
    {
        auto info = response->add_programs();
        info->set_name(program);
        info->mutable_id()->set_program(id++);
    }
    return to_grpc_status(status);
}

grpc::Status ProgramControlService::SetProcessorProgram(grpc::ServerContext* /*context*/,
                                                        const sushi_rpc::ProcessorProgramSetRequest* request,
                                                        sushi_rpc::GenericVoidValue* /*response*/)
{
    auto status = _controller->set_processor_program(request->processor().id(), request->program().program());
    return to_grpc_status(status);
}

grpc::Status ParameterControlService::GetProcessorParameters(grpc::ServerContext* /*context*/,
                                                             const sushi_rpc::ProcessorIdentifier* request,
                                                             sushi_rpc::ParameterInfoList* response)
{
    auto [status, parameters] = _controller->get_processor_parameters(request->id());
    for (const auto& parameter : parameters)
    {
        auto info = response->add_parameters();
        to_grpc(*info, parameter);
    }
    return to_grpc_status(status);
}

grpc::Status ParameterControlService::GetTrackProperties(grpc::ServerContext* /*context*/,
                                                         const sushi_rpc::TrackIdentifier* request,
                                                         sushi_rpc::PropertyInfoList* response)
{
    auto [status, properties] = _controller->get_track_properties(request->id());
    for (const auto& property : properties)
    {
        auto info = response->add_properties();
        to_grpc(*info, property);
    }
    return to_grpc_status(status);
}

grpc::Status ParameterControlService::GetProcessorProperties(grpc::ServerContext* /*context*/,
                                                             const sushi_rpc::ProcessorIdentifier* request,
                                                             sushi_rpc::PropertyInfoList* response)
{
    auto [status, properties] = _controller->get_processor_properties(request->id());
    for (const auto& property : properties)
    {
        auto info = response->add_properties();
        to_grpc(*info, property);
    }
    return to_grpc_status(status);}

grpc::Status ParameterControlService::GetPropertyId(grpc::ServerContext* /*context*/,
                                                    const sushi_rpc::PropertyIdRequest* request,
                                                    sushi_rpc::PropertyIdentifier* response)
{
    auto [status, id] = _controller->get_property_id(request->processor().id(), request->property_name());
    if (status != sushi::ext::ControlStatus::OK)
    {
        return to_grpc_status(status,  "No property with that name");
    }
    response->set_property_id(id);
    return grpc::Status::OK;
}

grpc::Status ParameterControlService::GetPropertyInfo(grpc::ServerContext* /*context*/,
                                                      const sushi_rpc::PropertyIdentifier* request,
                                                      sushi_rpc::PropertyInfo* response)
{
    auto [status, property] = _controller->get_property_info(request->processor_id(), request->property_id());
    if (status != sushi::ext::ControlStatus::OK)
    {
        return to_grpc_status(status);
    }
    to_grpc(*response, property);
    return grpc::Status::OK;
}

grpc::Status ParameterControlService::GetPropertyValue(grpc::ServerContext* /*context*/,
                                                       const sushi_rpc::PropertyIdentifier* request,
                                                       sushi_rpc::GenericStringValue* response)
{
    auto [status, value] = _controller->get_property_value(request->processor_id(), request->property_id());
    if (status != sushi::ext::ControlStatus::OK)
    {
        return to_grpc_status(status);
    }
    response->set_value(std::move(value));
    return grpc::Status::OK;
}

grpc::Status ParameterControlService::SetPropertyValue(grpc::ServerContext* /*context*/,
                                                       const sushi_rpc::PropertyValue* request,
                                                       sushi_rpc::GenericVoidValue* /*response*/)
{
    auto status = _controller->set_property_value(request->property().processor_id(),
                                                  request->property().property_id(),
                                                  request->value());
    return to_grpc_status(status);
}

grpc::Status MidiControlService::GetInputPorts(grpc::ServerContext* /*context*/,
                                               const sushi_rpc::GenericVoidValue* /*request*/,
                                               sushi_rpc::GenericIntValue* response)
{
    response->set_value(_midi_controller->get_input_ports());
    return grpc::Status::OK;
}

grpc::Status MidiControlService::GetOutputPorts(grpc::ServerContext* /*context*/,
                                                const sushi_rpc::GenericVoidValue* /*request*/,
                                                sushi_rpc::GenericIntValue* response)
{
    response->set_value(_midi_controller->get_output_ports());
    return grpc::Status::OK;
}

grpc::Status MidiControlService::GetAllKbdInputConnections(grpc::ServerContext* /*context*/,
                                                           const sushi_rpc::GenericVoidValue* /*request*/,
                                                           sushi_rpc::MidiKbdConnectionList* response)
{
    auto input_connections = _midi_controller->get_all_kbd_input_connections();
    for (const auto& connection : input_connections)
    {
        auto info = response->add_connections();
        to_grpc(*info, connection);
    }
    return grpc::Status::OK;
}

grpc::Status MidiControlService::GetAllKbdOutputConnections(grpc::ServerContext* /*context*/,
                                                            const sushi_rpc::GenericVoidValue* /*request*/,
                                                            sushi_rpc::MidiKbdConnectionList* response)
{
    auto output_connections = _midi_controller->get_all_kbd_output_connections();
    for (const auto& connection : output_connections)
    {
        auto info = response->add_connections();
        to_grpc(*info, connection);
    }
    return grpc::Status::OK;
}

grpc::Status MidiControlService::GetAllCCInputConnections(grpc::ServerContext* /*context*/,
                                                          const sushi_rpc::GenericVoidValue* /*request*/,
                                                          sushi_rpc::MidiCCConnectionList* response)
{
    auto output_connections = _midi_controller->get_all_cc_input_connections();
    for (const auto& connection : output_connections)
    {
        auto info = response->add_connections();
        to_grpc(*info, connection);
    }
    return grpc::Status::OK;
}

grpc::Status MidiControlService::GetAllPCInputConnections(grpc::ServerContext* /*context*/,
                                                          const sushi_rpc::GenericVoidValue* /*request*/,
                                                          sushi_rpc::MidiPCConnectionList* response)
{
    auto input_connections = _midi_controller->get_all_pc_input_connections();
    for (const auto& connection : input_connections)
    {
        auto info = response->add_connections();
        to_grpc(*info, connection);
    }
    return grpc::Status::OK;
}

grpc::Status MidiControlService::GetCCInputConnectionsForProcessor(grpc::ServerContext* /*context*/,
                                                                   const sushi_rpc::ProcessorIdentifier* request,
                                                                   sushi_rpc::MidiCCConnectionList* response)
{
    const auto processor_id = request->id();
    auto output_connections = _midi_controller->get_cc_input_connections_for_processor(processor_id);
    if(output_connections.first == sushi::ext::ControlStatus::OK)
    {
        for (const auto& connection : output_connections.second)
        {
            auto info = response->add_connections();
            to_grpc(*info, connection);
        }
        return grpc::Status::OK;
    }
    else
    {
        return to_grpc_status(output_connections.first);
    }
}

grpc::Status MidiControlService::GetPCInputConnectionsForProcessor(grpc::ServerContext* /*context*/,
                                                                   const sushi_rpc::ProcessorIdentifier* request,
                                                                   sushi_rpc::MidiPCConnectionList* response)
{
    const auto processor_id = request->id();
    auto output_connections = _midi_controller->get_pc_input_connections_for_processor(processor_id);
    if(output_connections.first == sushi::ext::ControlStatus::OK)
    {
        for (const auto& connection : output_connections.second)
        {
            auto info = response->add_connections();
            to_grpc(*info, connection);
        }
        return grpc::Status::OK;
    }
    else
    {
        return to_grpc_status(output_connections.first);
    }
}

grpc::Status MidiControlService::ConnectKbdInputToTrack(grpc::ServerContext* /*context*/,
                                                        const sushi_rpc::MidiKbdConnection* request,
                                                        sushi_rpc::GenericVoidValue* /*response*/)
{
    const auto track_id = request->track();
    const auto channel = request->channel().channel();
    const auto port = request->port();
    const auto raw_midi = request->raw_midi();
    const auto midi_channel = to_sushi_ext(channel);

    const auto status = _midi_controller->connect_kbd_input_to_track(track_id.id(), midi_channel, port, raw_midi);

    return to_grpc_status(status);
}

grpc::Status MidiControlService::ConnectKbdOutputFromTrack(grpc::ServerContext* /*context*/,
                                                           const sushi_rpc::MidiKbdConnection* request,
                                                           sushi_rpc::GenericVoidValue* /*response*/)
{
    const auto track_id = request->track();
    const auto channel = request->channel().channel();
    const auto port = request->port();
    const auto midi_channel = to_sushi_ext(channel);

    const auto status = _midi_controller->connect_kbd_output_from_track(track_id.id(), midi_channel, port);

    return to_grpc_status(status);
}

grpc::Status MidiControlService::ConnectCCToParameter(grpc::ServerContext* /*context*/,
                                                      const sushi_rpc::MidiCCConnection* request,
                                                      sushi_rpc::GenericVoidValue* /*response*/)
{
    const auto midi_channel = to_sushi_ext(request->channel().channel());
    const auto status = _midi_controller->connect_cc_to_parameter(request->parameter().processor_id(),
                                                                  request->parameter().parameter_id(),
                                                                  midi_channel,
                                                                  request->port(),
                                                                  request->cc_number(),
                                                                  request->min_range(),
                                                                  request->max_range(),
                                                                  request->relative_mode());

    return to_grpc_status(status);
}

grpc::Status MidiControlService::ConnectPCToProcessor(grpc::ServerContext* /*context*/,
                                                      const sushi_rpc::MidiPCConnection* request,
                                                      sushi_rpc::GenericVoidValue* /*response*/)
{
    const auto processor_id = request->processor().id();
    const MidiChannel_Channel channel = request->channel().channel();
    const auto port = request->port();

    sushi::ext::MidiChannel midi_channel = to_sushi_ext(channel);

    const auto status = _midi_controller->connect_pc_to_processor(processor_id, midi_channel, port);

    return to_grpc_status(status);
}

grpc::Status MidiControlService::DisconnectKbdInput(grpc::ServerContext* /*context*/,
                                                    const sushi_rpc::MidiKbdConnection* request,
                                                    sushi_rpc::GenericVoidValue* /*response*/)
{
    const auto track_id = request->track();
    const auto channel = request->channel().channel();
    const auto port = request->port();
    const auto midi_channel = to_sushi_ext(channel);
    const auto raw_midi = request->raw_midi();
    const auto status = _midi_controller->disconnect_kbd_input(track_id.id(), midi_channel, port, raw_midi);

    return to_grpc_status(status);
}

grpc::Status MidiControlService::DisconnectKbdOutput(grpc::ServerContext* /*context*/,
                                                     const sushi_rpc::MidiKbdConnection* request,
                                                     sushi_rpc::GenericVoidValue* /*response*/)
{
    const auto track_id = request->track();
    const auto channel = request->channel().channel();
    const auto port = request->port();
    const auto midi_channel = to_sushi_ext(channel);

    const auto status = _midi_controller->disconnect_kbd_output(track_id.id(), midi_channel, port);

    return to_grpc_status(status);
}

grpc::Status MidiControlService::DisconnectCC(grpc::ServerContext* /*context*/,
                                              const sushi_rpc::MidiCCConnection* request,
                                              sushi_rpc::GenericVoidValue* /*response*/)
{
    const auto midi_channel = to_sushi_ext(request->channel().channel());
    const auto status = _midi_controller->disconnect_cc(request->parameter().processor_id(),
                                                        midi_channel,
                                                        request->port(),
                                                        request->cc_number());

    return to_grpc_status(status);
}

grpc::Status MidiControlService::DisconnectPC(grpc::ServerContext* /*context*/,
                                              const sushi_rpc::MidiPCConnection* request,
                                              sushi_rpc::GenericVoidValue* /*response*/)
{
    const auto processor_id = request->processor().id();
    const MidiChannel_Channel channel = request->channel().channel();
    const auto port = request->port();
    sushi::ext::MidiChannel midi_channel = to_sushi_ext(channel);

    const auto status = _midi_controller->disconnect_pc(processor_id, midi_channel, port);
    return to_grpc_status(status);
}

grpc::Status MidiControlService::DisconnectAllCCFromProcessor(grpc::ServerContext* /*context*/,
                                                              const sushi_rpc::ProcessorIdentifier* request,
                                                              sushi_rpc::GenericVoidValue* /*response*/)
{
    const auto processor_id = request->id();
    const auto status = _midi_controller->disconnect_all_cc_from_processor(processor_id);
    return to_grpc_status(status);
}

grpc::Status MidiControlService::DisconnectAllPCFromProcessor(grpc::ServerContext* /*context*/,
                                                              const sushi_rpc::ProcessorIdentifier* request,
                                                              sushi_rpc::GenericVoidValue* /*response*/)
{
    const auto processor_id = request->id();
    const auto status = _midi_controller->disconnect_all_pc_from_processor(processor_id);
    return to_grpc_status(status);
}

grpc::Status AudioRoutingControlService::GetAllInputConnections(grpc::ServerContext* /*context*/,
                                                                const sushi_rpc::GenericVoidValue* /*request*/,
                                                                sushi_rpc::AudioConnectionList* response)
{
    auto connections = _controller->get_all_input_connections();
    for (const auto& connection : connections)
    {
        auto c = response->add_connections();
        to_grpc(*c, connection);
    }
    return grpc::Status::OK;
}

grpc::Status AudioRoutingControlService::GetAllOutputConnections(grpc::ServerContext* /*context*/,
                                                                 const sushi_rpc::GenericVoidValue* /*request*/,
                                                                 sushi_rpc::AudioConnectionList* response)
{
    auto connections = _controller->get_all_output_connections();
    for (const auto& connection : connections)
    {
        auto c = response->add_connections();
        to_grpc(*c, connection);
    }
    return grpc::Status::OK;
}

grpc::Status AudioRoutingControlService::GetInputConnectionsForTrack(grpc::ServerContext* /*context*/,
                                                                     const sushi_rpc::TrackIdentifier* request,
                                                                     sushi_rpc::AudioConnectionList* response)
{
    auto connections = _controller->get_input_connections_for_track(request->id());
    for (const auto& connection : connections)
    {
        auto c = response->add_connections();
        to_grpc(*c, connection);
    }
    return grpc::Status::OK;
}

grpc::Status AudioRoutingControlService::GetOutputConnectionsForTrack(grpc::ServerContext* /*context*/,
                                                                      const sushi_rpc::TrackIdentifier* request,
                                                                      sushi_rpc::AudioConnectionList* response)
{
    auto connections = _controller->get_output_connections_for_track(request->id());
    for (const auto& connection : connections)
    {
        auto c = response->add_connections();
        to_grpc(*c, connection);
    }
    return grpc::Status::OK;
}

grpc::Status AudioRoutingControlService::ConnectInputChannelToTrack(grpc::ServerContext* /*context*/,
                                                                    const sushi_rpc::AudioConnection* request,
                                                                    sushi_rpc::GenericVoidValue* /*response*/)
{
    auto status = _controller->connect_input_channel_to_track(request->track().id(),
                                                              request->track_channel(),
                                                              request->engine_channel());
    return to_grpc_status(status);
}

grpc::Status AudioRoutingControlService::ConnectOutputChannelFromTrack(grpc::ServerContext* /*context*/,
                                                                       const sushi_rpc::AudioConnection* request,
                                                                       sushi_rpc::GenericVoidValue* /*response*/)
{
    auto status = _controller->connect_output_channel_to_track(request->track().id(),
                                                               request->track_channel(),
                                                               request->engine_channel());
    return to_grpc_status(status);
}

grpc::Status AudioRoutingControlService::DisconnectInput(grpc::ServerContext* /*context*/,
                                                         const sushi_rpc::AudioConnection* request,
                                                         sushi_rpc::GenericVoidValue* /*response*/)
{
    auto status = _controller->disconnect_input(request->track().id(),
                                                request->track_channel(),
                                                request->engine_channel());
    return to_grpc_status(status);
}

grpc::Status AudioRoutingControlService::DisconnectOutput(grpc::ServerContext* /*context*/,
                                                          const sushi_rpc::AudioConnection* request,
                                                          sushi_rpc::GenericVoidValue* /*response*/)
{
    auto status = _controller->disconnect_output(request->track().id(),
                                                 request->track_channel(),
                                                 request->engine_channel());
    return to_grpc_status(status);
}

grpc::Status AudioRoutingControlService::DisconnectAllInputsFromTrack(grpc::ServerContext* /*context*/,
                                                                      const sushi_rpc::TrackIdentifier* request,
                                                                      sushi_rpc::GenericVoidValue* /*response*/)
{
    auto status = _controller->disconnect_all_inputs_from_track(request->id());
    return to_grpc_status(status);
}

// This function is deprecated and should be removed eventually.
grpc::Status AudioRoutingControlService::DisconnectAllOutputFromTrack(grpc::ServerContext* /*context*/,
                                                                      const sushi_rpc::TrackIdentifier* request,
                                                                      sushi_rpc::GenericVoidValue* /*response*/)
{
    auto status = _controller->disconnect_all_outputs_from_track(request->id());
    return to_grpc_status(status);
}

grpc::Status AudioRoutingControlService::DisconnectAllOutputsFromTrack(grpc::ServerContext* /*context*/,
                                                                       const sushi_rpc::TrackIdentifier* request,
                                                                       sushi_rpc::GenericVoidValue* /*response*/)
{
    auto status = _controller->disconnect_all_outputs_from_track(request->id());
    return to_grpc_status(status);
}

grpc::Status CvGateControlService::GetCvInputChannelCount(grpc::ServerContext* context,
                                                          const sushi_rpc::GenericVoidValue* request,
                                                          sushi_rpc::GenericIntValue* response)
{
    return Service::GetCvInputChannelCount(context, request, response);
}

grpc::Status CvGateControlService::GetCvOutputChannelCount(grpc::ServerContext* context,
                                                           const sushi_rpc::GenericVoidValue* request,
                                                           sushi_rpc::GenericIntValue* response)
{
    return Service::GetCvOutputChannelCount(context, request, response);
}

grpc::Status CvGateControlService::GetAllCvInputConnections(grpc::ServerContext* context,
                                                            const sushi_rpc::GenericVoidValue* request,
                                                            sushi_rpc::CvConnectionList* response)
{
    return Service::GetAllCvInputConnections(context, request, response);
}

grpc::Status CvGateControlService::GetAllCvOutputConnections(grpc::ServerContext* context,
                                                             const sushi_rpc::GenericVoidValue* request,
                                                             sushi_rpc::CvConnectionList* response)
{
    return Service::GetAllCvOutputConnections(context, request, response);
}

grpc::Status CvGateControlService::GetAllGateInputConnections(grpc::ServerContext* context,
                                                              const sushi_rpc::GenericVoidValue* request,
                                                              sushi_rpc::GateConnectionList* response)
{
    return Service::GetAllGateInputConnections(context, request, response);
}

grpc::Status CvGateControlService::GetAllGateOutputConnections(grpc::ServerContext* context,
                                                               const sushi_rpc::GenericVoidValue* request,
                                                               sushi_rpc::GateConnectionList* response)
{
    return Service::GetAllGateOutputConnections(context, request, response);
}

grpc::Status CvGateControlService::GetCvInputConnectionsForProcessor(grpc::ServerContext* context,
                                                                     const sushi_rpc::ProcessorIdentifier* request,
                                                                     sushi_rpc::CvConnectionList* response)
{
    return Service::GetCvInputConnectionsForProcessor(context, request, response);
}

grpc::Status CvGateControlService::GetCvOutputConnectionsForProcessor(grpc::ServerContext* context,
                                                                      const sushi_rpc::ProcessorIdentifier* request,
                                                                      sushi_rpc::CvConnectionList* response)
{
    return Service::GetCvOutputConnectionsForProcessor(context, request, response);
}

grpc::Status CvGateControlService::GetGateInputConnectionsForProcessor(grpc::ServerContext* context,
                                                                       const sushi_rpc::ProcessorIdentifier* request,
                                                                       sushi_rpc::GateConnectionList* response)
{
    return Service::GetGateInputConnectionsForProcessor(context, request, response);
}

grpc::Status CvGateControlService::GetGateOutputConnectionsForProcessor(grpc::ServerContext* context,
                                                                        const sushi_rpc::ProcessorIdentifier* request,
                                                                        sushi_rpc::GateConnectionList* response)
{
    return Service::GetGateOutputConnectionsForProcessor(context, request, response);
}

grpc::Status CvGateControlService::ConnectCvInputToParameter(grpc::ServerContext* context,
                                                             const sushi_rpc::CvConnection* request,
                                                             sushi_rpc::GenericVoidValue* response)
{
    return Service::ConnectCvInputToParameter(context, request, response);
}

grpc::Status CvGateControlService::ConnectCvOutputFromParameter(grpc::ServerContext* context,
                                                                const sushi_rpc::CvConnection* request,
                                                                sushi_rpc::GenericVoidValue* response)
{
    return Service::ConnectCvOutputFromParameter(context, request, response);
}

grpc::Status CvGateControlService::ConnectGateInputToProcessor(grpc::ServerContext* context,
                                                               const sushi_rpc::GateConnection* request,
                                                               sushi_rpc::GenericVoidValue* response)
{
    return Service::ConnectGateInputToProcessor(context, request, response);
}

grpc::Status CvGateControlService::ConnectGateOutputFromProcessor(grpc::ServerContext* context,
                                                                  const sushi_rpc::GateConnection* request,
                                                                  sushi_rpc::GenericVoidValue* response)
{
    return Service::ConnectGateOutputFromProcessor(context, request, response);
}

grpc::Status CvGateControlService::DisconnectCvInput(grpc::ServerContext* context,
                                                     const sushi_rpc::CvConnection* request,
                                                     sushi_rpc::GenericVoidValue* response)
{
    return Service::DisconnectCvInput(context, request, response);
}

grpc::Status CvGateControlService::DisconnectCvOutput(grpc::ServerContext* context,
                                                      const sushi_rpc::CvConnection* request,
                                                      sushi_rpc::GenericVoidValue* response)
{
    return Service::DisconnectCvOutput(context, request, response);
}

grpc::Status CvGateControlService::DisconnectGateInput(grpc::ServerContext* context,
                                                       const sushi_rpc::GateConnection* request,
                                                       sushi_rpc::GenericVoidValue* response)
{
    return Service::DisconnectGateInput(context, request, response);
}

grpc::Status CvGateControlService::DisconnectGateOutput(grpc::ServerContext* context,
                                                        const sushi_rpc::GateConnection* request,
                                                        sushi_rpc::GenericVoidValue* response)
{
    return Service::DisconnectGateOutput(context, request, response);
}

grpc::Status CvGateControlService::DisconnectAllCvInputsFromProcessor(grpc::ServerContext* context,
                                                                      const sushi_rpc::ProcessorIdentifier* request,
                                                                      sushi_rpc::GenericVoidValue* response)
{
    return Service::DisconnectAllCvInputsFromProcessor(context, request, response);
}

grpc::Status CvGateControlService::DisconnectAllCvOutputsFromProcessor(grpc::ServerContext* context,
                                                                       const sushi_rpc::ProcessorIdentifier* request,
                                                                       sushi_rpc::GenericVoidValue* response)
{
    return Service::DisconnectAllCvOutputsFromProcessor(context, request, response);
}

grpc::Status CvGateControlService::DisconnectAllGateInputsFromProcessor(grpc::ServerContext* context,
                                                                        const sushi_rpc::ProcessorIdentifier* request,
                                                                        sushi_rpc::GenericVoidValue* response)
{
    return Service::DisconnectAllGateInputsFromProcessor(context, request, response);
}

grpc::Status CvGateControlService::DisconnectAllGateOutputsFromProcessor(grpc::ServerContext* context,
                                                                         const sushi_rpc::ProcessorIdentifier* request,
                                                                         sushi_rpc::GenericVoidValue* response)
{
    return Service::DisconnectAllGateOutputsFromProcessor(context, request, response);
}


grpc::Status OscControlService::GetSendPort(grpc::ServerContext* /*context*/,
                                            const sushi_rpc::GenericVoidValue* /*request*/,
                                            sushi_rpc::GenericIntValue* response)
{
    response->set_value(_controller->get_send_port());
    return grpc::Status::OK;
}

grpc::Status OscControlService::GetReceivePort(grpc::ServerContext* /*context*/,
                                               const sushi_rpc::GenericVoidValue* /*request*/,
                                               sushi_rpc::GenericIntValue* response)
{
    response->set_value(_controller->get_receive_port());
    return grpc::Status::OK;
}

grpc::Status OscControlService::GetEnabledParameterOutputs(grpc::ServerContext* /*context*/,
                                                           const sushi_rpc::GenericVoidValue* /*request*/,
                                                           sushi_rpc::OscParameterOutputList* response)
{
    auto enabled_outputs = _controller->get_enabled_parameter_outputs();

    for (const auto& path : enabled_outputs)
    {
        response->add_path(path);
    }
    return grpc::Status::OK;
}

grpc::Status OscControlService::EnableOutputForParameter(grpc::ServerContext* /*context*/,
                                                         const sushi_rpc::ParameterIdentifier* request,
                                                         sushi_rpc::GenericVoidValue* /*response*/)
{
    const auto processor_id = request->processor_id();
    const auto parameter_id = request->parameter_id();

    auto status = _controller->enable_output_for_parameter(processor_id, parameter_id);

    return to_grpc_status(status);
}

grpc::Status OscControlService::DisableOutputForParameter(grpc::ServerContext* /*context*/,
                                                          const sushi_rpc::ParameterIdentifier* request,
                                                          sushi_rpc::GenericVoidValue* /*response*/)
{
    const auto processor_id = request->processor_id();
    const auto parameter_id = request->parameter_id();

    auto status = _controller->disable_output_for_parameter(processor_id, parameter_id);

    return to_grpc_status(status);
}

grpc::Status OscControlService::EnableAllOutput(grpc::ServerContext* /*context*/,
                                                const sushi_rpc::GenericVoidValue* /*request*/,
                                                sushi_rpc::GenericVoidValue* /*response*/)
{
    auto status = _controller->enable_all_output();
    return to_grpc_status(status);
}

grpc::Status OscControlService::DisableAllOutput(grpc::ServerContext* /*context*/,
                                                 const sushi_rpc::GenericVoidValue* /*request*/,
                                                 sushi_rpc::GenericVoidValue* /*response*/)
{
    auto status = _controller->disable_all_output();
    return to_grpc_status(status);
}

NotificationControlService::NotificationControlService(sushi::ext::SushiControl* controller) : _controller{controller},
                                                                                               _audio_graph_controller{controller->audio_graph_controller()}
{
    _controller->subscribe_to_notifications(sushi::ext::NotificationType::TRANSPORT_UPDATE, this);
    _controller->subscribe_to_notifications(sushi::ext::NotificationType::CPU_TIMING_UPDATE, this);
    _controller->subscribe_to_notifications(sushi::ext::NotificationType::TRACK_UPDATE, this);
    _controller->subscribe_to_notifications(sushi::ext::NotificationType::PROCESSOR_UPDATE, this);
    _controller->subscribe_to_notifications(sushi::ext::NotificationType::PARAMETER_CHANGE, this);
}

void NotificationControlService::notification(const sushi::ext::ControlNotification* notification)
{
    switch(notification->type())
    {
        case sushi::ext::NotificationType::TRANSPORT_UPDATE:
        {
            _forward_transport_notification_to_subscribers(notification);
            break;
        }
        case sushi::ext::NotificationType::CPU_TIMING_UPDATE:
        {
            _forward_cpu_timing_notification_to_subscribers(notification);
            break;
        }
        case sushi::ext::NotificationType::TRACK_UPDATE:
        {
            _forward_track_notification_to_subscribers(notification);
            break;
        }
        case sushi::ext::NotificationType::PROCESSOR_UPDATE:
        {
            _forward_processor_notification_to_subscribers(notification);
            break;
        }
        case sushi::ext::NotificationType::PARAMETER_CHANGE:
        {
            _forward_parameter_notification_to_subscribers(notification);
            break;
        }
        default:
            break;
    }
}

void NotificationControlService::_forward_transport_notification_to_subscribers(const sushi::ext::ControlNotification* notification)
{
    auto typed_notification = static_cast<const sushi::ext::TransportNotification*>(notification);
    auto notification_content = std::make_shared<TransportUpdate>();
    auto action = typed_notification->action();

    switch(action)
    {
        case sushi::ext::TransportAction::TEMPO_CHANGED:
        {
            float value = std::get<float>(typed_notification->value());
            notification_content->set_tempo(value);
            break;
        }
        case sushi::ext::TransportAction::PLAYING_MODE_CHANGED:
        {
            auto grpc_playing_mode = to_grpc(std::get<sushi::ext::PlayingMode>(typed_notification->value()));
            notification_content->mutable_playing_mode()->set_mode(grpc_playing_mode);
            break;
        }
        case sushi::ext::TransportAction::SYNC_MODE_CHANGED:
        {
            auto grpc_sync_mode = to_grpc(std::get<sushi::ext::SyncMode>(typed_notification->value()));
            notification_content->mutable_sync_mode()->set_mode(grpc_sync_mode);
            break;
        }
        case sushi::ext::TransportAction::TIME_SIGNATURE_CHANGED:
        {
            auto mutable_time_signature = notification_content->mutable_time_signature();
            const auto source_time_signature = std::get<sushi::ext::TimeSignature>(typed_notification->value());
            mutable_time_signature->set_denominator(source_time_signature.denominator);
            mutable_time_signature->set_numerator(source_time_signature.numerator);
            break;
        }
        default:
        {
            assert(false);
            break;
        }
    }

    std::scoped_lock lock(_transport_subscriber_lock);
    for (auto& subscriber : _transport_subscribers)
    {
        subscriber->push(notification_content);
    }
}

void NotificationControlService::_forward_cpu_timing_notification_to_subscribers(const sushi::ext::ControlNotification* notification)
{
    auto typed_notification = static_cast<const sushi::ext::CpuTimingNotification*>(notification);
    auto notification_content = std::make_shared<CpuTimings>();
    auto timings = typed_notification->cpu_timings();
    notification_content->set_average(timings.avg);
    notification_content->set_min(timings.min);
    notification_content->set_max(timings.max);

    std::scoped_lock lock(_timing_subscriber_lock);
    for (auto& subscriber : _timing_subscribers)
    {
        subscriber->push(notification_content);
    }
}

void NotificationControlService::_forward_track_notification_to_subscribers(const sushi::ext::ControlNotification* notification)
{
    auto typed_notification = static_cast<const sushi::ext::TrackNotification*>(notification);
    auto notification_content = std::make_shared<TrackUpdate>();
    auto action = typed_notification->action();

    switch(action)
    {
        case sushi::ext::TrackAction::ADDED:
        {
            notification_content->set_action(TrackUpdate_Action_TRACK_ADDED);
            break;
        }
        case sushi::ext::TrackAction::DELETED:
        {
            notification_content->set_action(TrackUpdate_Action_TRACK_DELETED);
            break;
        }
        default:
        {
            assert(false);
            break;
        }
    }

    notification_content->mutable_track()->set_id(typed_notification->track_id());

    std::scoped_lock lock(_track_subscriber_lock);
    for (auto& subscriber : _track_subscribers)
    {
        subscriber->push(notification_content);
    }
}

void NotificationControlService::_forward_processor_notification_to_subscribers(const sushi::ext::ControlNotification* notification)
{
    auto typed_notification = static_cast<const sushi::ext::ProcessorNotification*>(notification);
    auto notification_content = std::make_shared<ProcessorUpdate>();
    auto action = typed_notification->action();

    switch(action)
    {
        case sushi::ext::ProcessorAction::ADDED:
        {
            notification_content->set_action(ProcessorUpdate_Action_PROCESSOR_ADDED);
            break;
        }
        case sushi::ext::ProcessorAction::DELETED:
        {
            notification_content->set_action(ProcessorUpdate_Action_PROCESSOR_DELETED);
            break;
        }
        default:
        {
            assert(false);
            break;
        }
    }

    notification_content->mutable_processor()->set_id(typed_notification->processor_id());
    notification_content->mutable_parent_track()->set_id(typed_notification->parent_track_id());

    std::scoped_lock lock(_processor_subscriber_lock);
    for (auto& subscriber : _processor_subscribers)
    {
        subscriber->push(notification_content);
    }
}

void NotificationControlService::_forward_parameter_notification_to_subscribers(const sushi::ext::ControlNotification* notification)
{
    auto typed_notification = static_cast<const sushi::ext::ParameterChangeNotification*>(notification);
    auto notification_content = std::make_shared<ParameterValue>();
    notification_content->set_value(typed_notification->value());
    notification_content->mutable_parameter()->set_parameter_id(typed_notification->parameter_id());
    notification_content->mutable_parameter()->set_processor_id(typed_notification->processor_id());

    std::scoped_lock lock(_parameter_subscriber_lock);
    for (auto& subscriber : _parameter_subscribers)
    {
        subscriber->push(notification_content);
    }
}

void NotificationControlService::subscribe(SubscribeToTransportChangesCallData* subscriber)
{
    std::scoped_lock lock(_transport_subscriber_lock);
    _transport_subscribers.push_back(subscriber);
}

void NotificationControlService::unsubscribe(SubscribeToTransportChangesCallData* subscriber)
{
    std::scoped_lock lock(_transport_subscriber_lock);
    _transport_subscribers.erase(std::remove(_transport_subscribers.begin(),
                                             _transport_subscribers.end(),
                                             subscriber));
}

void NotificationControlService::subscribe(SubscribeToCpuTimingUpdatesCallData* subscriber)
{
    std::scoped_lock lock(_timing_subscriber_lock);
    _timing_subscribers.push_back(subscriber);
}

void NotificationControlService::unsubscribe(SubscribeToCpuTimingUpdatesCallData* subscriber)
{
    std::scoped_lock lock(_timing_subscriber_lock);
    _timing_subscribers.erase(std::remove(_timing_subscribers.begin(),
                                          _timing_subscribers.end(),
                                          subscriber));
}

void NotificationControlService::subscribe(SubscribeToTrackChangesCallData* subscriber)
{
    std::scoped_lock lock(_track_subscriber_lock);
    _track_subscribers.push_back(subscriber);
}

void NotificationControlService::unsubscribe(SubscribeToTrackChangesCallData* subscriber)
{
    std::scoped_lock lock(_track_subscriber_lock);
    _track_subscribers.erase(std::remove(_track_subscribers.begin(),
                                         _track_subscribers.end(),
                                         subscriber));
}

void NotificationControlService::subscribe(SubscribeToProcessorChangesCallData* subscriber)
{
    std::scoped_lock lock(_processor_subscriber_lock);
    _processor_subscribers.push_back(subscriber);
}

void NotificationControlService::unsubscribe(SubscribeToProcessorChangesCallData* subscriber)
{
    std::scoped_lock lock(_processor_subscriber_lock);
    _processor_subscribers.erase(std::remove(_processor_subscribers.begin(),
                                             _processor_subscribers.end(),
                                             subscriber));
}

void NotificationControlService::subscribe(SubscribeToParameterUpdatesCallData* subscriber)
{
    std::scoped_lock lock(_parameter_subscriber_lock);
    _parameter_subscribers.push_back(subscriber);
}

void NotificationControlService::unsubscribe(SubscribeToParameterUpdatesCallData* subscriber)
{
    std::scoped_lock lock(_parameter_subscriber_lock);
    _parameter_subscribers.erase(std::remove(_parameter_subscribers.begin(),
                                             _parameter_subscribers.end(),
                                             subscriber));
}

void NotificationControlService::delete_all_subscribers()
{
    /* Unsubscribe and delete CallData subscribers directly, without
     * waiting for them to be asynchronously deleted when a worker
     * thread pulls them of the completion queue. */
    {
        std::scoped_lock lock(_transport_subscriber_lock);
        for (auto& subscriber : _transport_subscribers)
        {
            delete subscriber;
        }
        _transport_subscribers.clear();
    }

    {
        std::scoped_lock lock(_timing_subscriber_lock);
        for (auto& subscriber : _timing_subscribers)
        {
            delete subscriber;
        }
        _timing_subscribers.clear();
    }

    {
        std::scoped_lock lock(_track_subscriber_lock);
        for (auto& subscriber : _track_subscribers)
        {
            delete subscriber;
        }
        _track_subscribers.clear();
    }

    {
        std::scoped_lock lock(_parameter_subscriber_lock);
        for (auto& subscriber : _parameter_subscribers)
        {
            delete subscriber;
        }
        _parameter_subscribers.clear();
    }

    {
        std::scoped_lock lock(_processor_subscriber_lock);
        for (auto& subscriber : _processor_subscribers)
        {
            delete subscriber;
        }
        _processor_subscribers.clear();
    }
}

} // sushi_rpc
