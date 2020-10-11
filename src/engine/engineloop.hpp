/**
 * Part of Epicinium
 * developed by A Bunch of Hacks.
 *
 * Copyright (c) 2017-2020 A Bunch of Hacks
 *
 * Epicinium is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Epicinium is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * [authors:]
 * Sander in 't Veld (sander@abunchofhacks.coop)
 * Daan Mulder (daan@abunchofhacks.coop)
 */
#pragma once
#include "header.hpp"

#include "loop.hpp"


class EngineLoop : public Loop
{
public:
	class Owner
	{
	protected:
		Owner() = default;

	public:
		Owner(const Owner& /**/) = delete;
		Owner(Owner&& /**/) = delete;
		Owner& operator=(const Owner& /**/) = delete;
		Owner& operator=(Owner&& /**/) = delete;
		virtual ~Owner() = default;

		virtual void doFirst() = 0;
		virtual void doFrame() = 0;
	};

	EngineLoop(Owner& owner, uint8_t framerate);

	EngineLoop(const EngineLoop&) = delete;
	EngineLoop(EngineLoop&&) = delete;
	EngineLoop& operator=(const EngineLoop&) = delete;
	EngineLoop& operator=(EngineLoop&&) = delete;
	~EngineLoop();

	void run();
	void stop();

	void event();

	void flip();
	void expose();
	void finish();

private:
	float _engineSpeed;
	float _defaultTempo;

	Owner& owner;
	bool running;

	uint32_t current_time;
	uint32_t last_frame_time;
	uint32_t next_frame_time;
	uint32_t frame_start_time;
	uint32_t update_start_time;
	uint32_t draw_start_time;
	uint32_t finish_start_time;
	uint32_t second_start_time;

	uint32_t last_second;
	uint8_t frame_offset;

	uint32_t wakes_this_second = 0;
	uint32_t events_this_second = 0;
	uint32_t frames_this_second = 0;

	const uint8_t BUFFERSIZE = 100;
	float wakes_this_second_buffer[256] = {0};
	float events_this_second_buffer[256] = {0};
	float frames_this_second_buffer[256] = {0};
	float avg_start_delay_buffer[256] = {0};
	float avg_runtime_flip_buffer[256] = {0};
	float avg_runtime_update_buffer[256] = {0};
	float avg_runtime_draw_buffer[256] = {0};
	float avg_runtime_finish_buffer[256] = {0};
	float avg_duration_buffer[256] = {0};
	uint8_t buffer_offset = 0;
	bool buffer_filled = false;

	std::string wakes_this_second_string;
	std::string events_this_second_string;
	std::string frames_this_second_string;

	const uint8_t FRAMERATE;

	float frame_start_delay[256] = {0};
	float frame_runtime_flip[256] = {0};
	float frame_runtime_update[256] = {0};
	float frame_runtime_draw[256] = {0};
	float frame_runtime_finish[256] = {0};
	float frame_duration[256] = {0};
	float frame_delta[256] = {0};
	float frame_tempo[256] = {0};

	std::string frame_duration_string;
};
