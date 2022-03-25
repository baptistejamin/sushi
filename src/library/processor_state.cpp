/*
 * Copyright 2017-2021 Modern Ancient Instruments Networked AB, dba Elk
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
 * @brief Container class implementation for the full state of a processor.
 * @copyright 2017-2021 Modern Ancient Instruments Networked AB, dba Elk, Stockholm
 */

#include "processor_state.h"

namespace sushi {

ProcessorState::~ProcessorState() = default;

std::vector<std::byte> ProcessorState::serialize() const
{
    return std::vector<std::byte>();
}

bool ProcessorState::deserialize(std::vector<std::byte>)
{
    return false;
}

void ProcessorState::set_program(int program_id)
{
    _program = program_id;
}

void ProcessorState::set_bypass(bool enabled)
{
    _bypassed = enabled;
}

void ProcessorState::add_parameter_change(ObjectId parameter_id, float value)
{
    _parameter_changes.emplace_back(parameter_id, value);
}

void ProcessorState::add_property_change(ObjectId property_id, const std::string& value)
{
    _property_changes.emplace_back(property_id, value);
}

std::optional<int> ProcessorState::program() const
{
    return _program;
}

std::optional<int> ProcessorState::bypassed() const
{
    return _bypassed;
}

const std::vector<std::pair<ObjectId, float>>& ProcessorState::parameters() const
{
    return _parameter_changes;
}

const std::vector<std::pair<ObjectId, std::string>>& ProcessorState::properties() const
{
    return _property_changes;
}

RtState::RtState() = default;

RtState::RtState(const ProcessorState& state)
{
    _bypassed = state.bypassed();
    _parameter_changes = state.parameters();
}

RtState::~RtState() = default;

void RtState::set_bypass(bool enabled)
{
    _bypassed = enabled;
}

void RtState::add_parameter_change(ObjectId parameter_id, float value)
{
    _parameter_changes.emplace_back(parameter_id, value);
}

std::optional<int> RtState::bypassed() const
{
    return _bypassed;
}

const std::vector<std::pair<ObjectId, float>>& RtState::parameters() const
{
    return _parameter_changes;
}
}; // end namespace sushi
