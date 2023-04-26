/*
 * Copyright 2017-2023 Elk Audio AB
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
 * @brief Flanger from Brickworks library
 * @copyright 2017-2023 Elk Audio AB, Stockholm
 */

#ifndef FLANGER_PLUGIN_H
#define FLANGER_PLUGIN_H

#include <bw_chorus.h>

#include "library/internal_plugin.h"

namespace sushi {
namespace flanger_plugin {

constexpr int MAX_CHANNELS_SUPPORTED = 8;

class FlangerPlugin : public InternalPlugin, public UidHelper<FlangerPlugin>
{
public:
    FlangerPlugin(HostControl hostControl);

    ~FlangerPlugin();

    virtual ProcessorReturnCode init(float sample_rate) override;

    void configure(float sample_rate) override;

    void set_enabled(bool enabled) override;

    void process_audio(const ChunkSampleBuffer &in_buffer, ChunkSampleBuffer &out_buffer) override;

    static std::string_view static_uid();

private:
    FloatParameterValue* _rate;
    FloatParameterValue* _amount;

    bw_chorus_coeffs _chorus_coeffs;
	bw_chorus_state	_chorus_states[MAX_CHANNELS_SUPPORTED];
    void* _delay_mem_areas[MAX_CHANNELS_SUPPORTED]{nullptr};
};

}// namespace flanger_plugin
}// namespace sushi
#endif // FLANGER_PLUGIN_H
