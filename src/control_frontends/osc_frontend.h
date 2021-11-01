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
 * @brief OSC runtime control frontend
 * @copyright 2017-2019 Modern Ancient Instruments Networked AB, dba Elk, Stockholm
 *
 * Starts a thread listening for OSC commands at the given port
 * (configurable with proper command sent with apply_command.
 *
 */

#ifndef SUSHI_OSC_FRONTEND_H_H
#define SUSHI_OSC_FRONTEND_H_H

#include <vector>
#include <map>

#include "lo/lo.h"

#include "control_interface.h"
#include "base_control_frontend.h"

namespace sushi {
namespace control_frontend {

class OSCFrontend;

struct OscConnection
{
    ObjectId           processor;
    ObjectId           parameter;
    OSCFrontend*       instance;
    ext::SushiControl* controller;
    lo_method          liblo_cb;
};

class OSCFrontend : public BaseControlFrontend
{
public:
    OSCFrontend(engine::BaseEngine* engine, ext::SushiControl* controller, int receive_port, int send_port);

    ~OSCFrontend();

    /**
     * @brief Connect osc to the bypass state of a given processor.
     *        The resulting osc path will be:
     *        "/bypass/processor_name,i(enabled == 1, disabled == 0)"
     *
     * @param processor_name
     * @return
     */
    bool connect_to_bypass_state(const std::string& processor_name);

    /**
     * @brief Connect program change messages to a specific processor.
     *        The resulting osc path will be;
     *        "/program/processor i (program_id)"
     * @param processor_name Name of the processor
     * @return
     */
    bool connect_to_program_change(const std::string& processor_name);

    /**
     * @brief Output changes from the given parameter of the given
     *        processor to osc messages. The output will be on the form:
     *        "/parameter/processor_name/parameter_name,f(value)"
     * @param processor_name
     * @param parameter_name
     * @return
     */
    bool connect_from_parameter(const std::string& processor_name,
                                const std::string& parameter_name);

    /**
     * @brief Stops the broadcasting of OSC messages reflecting changes of a parameter.
     * @param processor_name
     * @param parameter_name
     * @return
     */
    bool disconnect_from_parameter(const std::string& processor_name,
                                   const std::string& parameter_name);

    /**
     * @brief Connect keyboard messages to a given track.
     *        The target osc path will be:
     *        "/keyboard_event/track_name,sif(note_on/note_off, note_value, velocity)"
     * @param track_name The track to send to
     * @return true
     */
    bool connect_kb_to_track(const std::string& track_name);

    /**
     * @brief Connect to control all parameters from a given processor.
     * @param processor_name The name of the processor to connect.
     * @param processor_id The id of the processor to connect.
     * @return
     */
    bool connect_to_parameters_and_properties(const std::string& processor_name, int processor_id);

    /**
     * @brief Enable OSC broadcasting of all parameters from a given processor.
     * @param processor_name The name of the processor to connect.
     * @param processor_id The id of the processor to connect.
     * @return
     */
    bool connect_from_processor_parameters(const std::string& processor_name, int processor_id);

    /**
     * @brief Disable OSC broadcasting of all parameters from a given processor.
     * @param processor_name The name of the processor to connect.
     * @param processor_id The id of the processor to connect.
     * @return
     */
    bool disconnect_from_processor_parameters(const std::string& processor_name, int processor_id);

    /**
     * @brief Register OSC callbacks for all midi kb and pc data to tracks and processors.
     */
    void connect_to_all();

    /**
     * @brief Register OSC callbacks for all parameters of all plugins.
     */
    void connect_from_all_parameters();

    /**
     * @brief Deregister OSC callbacks for all parameters of all plugins.
     */
    void disconnect_from_all_parameters();

    /**
     * Returns all OSC Address Patterns that are currently enabled to output state changes.
     * @return
     */
    std::vector<std::string> get_enabled_parameter_outputs();

    void run() override {_start_server();}

    void stop() override {_stop_server();}

    ControlFrontendStatus init() override;

    /* Inherited from EventPoster */
    int process(Event* event) override;

    int poster_id() override {return EventPosterId::OSC_FRONTEND;}

    int receive_port() const;

    int send_port() const;

    bool get_connect_from_all_parameters() {return _connect_from_all_parameters;}

    void set_connect_from_all_parameters(bool connect) {_connect_from_all_parameters = connect;}

private:
    bool _connect_to_parameter(const std::string& processor_name,
                               const std::string& parameter_name,
                               ObjectId processor_id,
                               ObjectId parameter_id);

    bool _connect_to_property(const std::string& processor_name,
                              const std::string& property_name,
                              ObjectId processor_id,
                              ObjectId property_id);

    void _completion_callback(Event* event, int return_status) override;

    void _start_server();

    void _stop_server();

    void _setup_engine_control();

    bool _remove_processor_connections(ObjectId processor_id);

    std::pair<OscConnection*, std::string> _create_processor_connection(const std::string& processor_name,
                                                                        const std::string& osc_path_prefix);

    void _handle_param_change_notification(const ParameterChangeNotificationEvent* event);

    void _handle_engine_notification(const EngineNotificationEvent* event);

    void _handle_audio_graph_notification(const AudioGraphNotificationEvent* event);

    void _handle_clipping_notification(const ClippingNotificationEvent* event);

    lo_server_thread _osc_server {nullptr};
    int _receive_port;
    int _send_port;
    lo_address _osc_out_address {nullptr};

    bool _connect_from_all_parameters {false};

    std::atomic_bool _osc_initialized {false};

    std::atomic_bool _running {false};

    sushi::ext::SushiControl* _controller {nullptr};
    sushi::ext::AudioGraphController* _graph_controller {nullptr};
    sushi::ext::ParameterController*  _param_controller {nullptr};

    /* Currently only stored here so they can be deleted */
    std::vector<std::unique_ptr<OscConnection>> _connections;

    std::map<ObjectId, std::map<ObjectId, std::string>> _outgoing_connections;
};

}; // namespace control_frontend
}; // namespace sushi

#endif //SUSHI_OSC_FRONTEND_H_H
