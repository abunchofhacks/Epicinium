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
#include "engineloop.hpp"
#include "source.hpp"

#include "libs/SDL2/SDL_timer.h"

#include "input.hpp"


EngineLoop::EngineLoop(Owner &owner, uint8_t framerate) :
	owner(owner),
	FRAMERATE(std::max((uint8_t) 1, framerate))
{
	_engineSpeed = 1;
	_defaultTempo = 1;
}

EngineLoop::~EngineLoop()
{
#ifdef PERFLOG
	float frames_total = 0;
	float frames_dropped = 0;
	float framerate_average = 0;
	float framerate_percentage = 0;
	float avg_start_delay = 0;
	float avg_runtime_flip = 0;
	float avg_runtime_update = 0;
	float avg_runtime_draw = 0;
	float avg_runtime_finish = 0;
	float avg_duration = 0;
	int bsize = (buffer_filled) ? BUFFERSIZE : buffer_offset;
	if (bsize > 0)
	{
		for (int i = 0; i < (int) bsize; i++)
		{
			frames_total += frames_this_second_buffer[i];
			avg_start_delay += avg_start_delay_buffer[i];
			avg_runtime_flip += avg_runtime_flip_buffer[i];
			avg_runtime_update += avg_runtime_update_buffer[i];
			avg_runtime_draw += avg_runtime_draw_buffer[i];
			avg_runtime_finish += avg_runtime_finish_buffer[i];
			avg_duration += avg_duration_buffer[i];
		}
		framerate_average = 1.0f * frames_total / bsize;
		framerate_percentage = 100.0f * frames_total / (FRAMERATE * bsize);
		frames_dropped = (FRAMERATE * bsize) - frames_total;
		avg_start_delay /= bsize;
		avg_runtime_flip /= bsize;
		avg_runtime_update /= bsize;
		avg_runtime_draw /= bsize;
		avg_runtime_finish /= bsize;
		avg_duration /= bsize;
	}

	PERFLOGI << "timeframe_in_seconds = " << bsize;
	PERFLOGI << "frames_total = " << frames_total;
	PERFLOGI << "frames_dropped = " << frames_dropped;
	PERFLOGI << "framerate_average = " << framerate_average;
	PERFLOGI << "framerate_percentage = " << framerate_percentage;

	// The following are averages of averages. This is intended: if one second
	// has a lot of framerate issues, the number of frames will be low, but the
	// average start delay might be higher. If we took per-frame-averages or
	// totals, these spikes would be underrepresented because they are so few.
	PERFLOGI << "avg_start_delay = " << avg_start_delay;
	PERFLOGI << "avg_runtime_flip = " << avg_runtime_flip;
	PERFLOGI << "avg_runtime_update = " << avg_runtime_update;
	PERFLOGI << "avg_runtime_draw = " << avg_runtime_draw;
	PERFLOGI << "avg_runtime_finish = " << avg_runtime_finish;
	PERFLOGI << "avg_duration = " << avg_duration;
#endif
}

void EngineLoop::run()
{
	running = true;

	current_time = SDL_GetTicks();
	last_frame_time = current_time;
	next_frame_time = current_time;

	last_second = current_time / 1000;
	second_start_time = last_second * 1000;
	frame_offset = int((current_time - second_start_time) * FRAMERATE / 1000.0f);

	while (running)
	{
		frame_start_time = SDL_GetTicks();
		frame_start_delay[frame_offset] = frame_start_time - next_frame_time;

		if (frame_start_time >= second_start_time + 1000)
		{
			wakes_this_second_buffer[buffer_offset] = wakes_this_second;
			events_this_second_buffer[buffer_offset] = events_this_second;
			frames_this_second_buffer[buffer_offset] = frames_this_second;

			wakes_this_second_string = std::to_string(wakes_this_second);
			events_this_second_string = std::to_string(events_this_second);
			frames_this_second_string = std::to_string(frames_this_second)
					+ "/" + std::to_string(FRAMERATE);

			float start_delay_average = 0;
			float runtime_flip_average = 0;
			float runtime_update_average = 0;
			float runtime_draw_average = 0;
			float runtime_finish_average = 0;
			float frame_duration_average = 0;
			if (frames_this_second > 0)
			{
				int start = (int) frame_offset - frames_this_second;
				for (int i = start; i < frame_offset; i++)
				{
					start_delay_average += frame_start_delay[i];
					runtime_flip_average += frame_runtime_flip[i];
					runtime_update_average += frame_runtime_update[i];
					runtime_draw_average += frame_runtime_draw[i];
					runtime_finish_average += frame_runtime_finish[i];
					frame_duration_average += frame_duration[i];
				}
				start_delay_average /= frames_this_second;
				runtime_flip_average /= frames_this_second;
				runtime_update_average /= frames_this_second;
				runtime_draw_average /= frames_this_second;
				runtime_finish_average /= frames_this_second;
				frame_duration_average /= frames_this_second;
			}

			avg_start_delay_buffer[buffer_offset] = start_delay_average;
			avg_runtime_flip_buffer[buffer_offset] = runtime_flip_average;
			avg_runtime_update_buffer[buffer_offset] = runtime_update_average;
			avg_runtime_draw_buffer[buffer_offset] = runtime_draw_average;
			avg_runtime_finish_buffer[buffer_offset] = runtime_finish_average;
			avg_duration_buffer[buffer_offset] = frame_duration_average;

			frame_duration_string = std::to_string((int) frame_duration_average)
					+ "/" + std::to_string(1000 / FRAMERATE);

			PERFLOGV << "frames_this_second = " << frames_this_second;
			PERFLOGV << "start_delay_average = " << start_delay_average;
			PERFLOGV << "runtime_flip_average = " << runtime_flip_average;
			PERFLOGV << "runtime_update_average = " << runtime_update_average;
			PERFLOGV << "runtime_draw_average = " << runtime_draw_average;
			PERFLOGV << "runtime_finish_average = " << runtime_finish_average;
			PERFLOGV << "frame_duration_average = " << frame_duration_average;

			buffer_offset = (buffer_offset + 1) % BUFFERSIZE;
			if (buffer_offset == 0) buffer_filled = true;

			frame_offset = 0;
			wakes_this_second = 0;
			events_this_second = 0;
			frames_this_second = 0;
			last_second = frame_start_time / 1000;
			second_start_time = last_second * 1000;
		}

		_delta = (frame_start_time - last_frame_time) * 0.001f * _engineSpeed;

		_rawTempo = _defaultTempo * _tempo_multiplier;
		_tempo = _rawTempo;
		if (_hitstop > 0)
		{
			float dt = _delta * _rawTempo;
			if (_hitstop >= dt)
			{
				_tempo = 0;
				_hitstop -= dt;
			}
			else
			{
				_tempo *= 1 - _hitstop / dt;
				_hitstop = 0;
			}
		}

		_theta = fmod(_theta + _delta * _tempo, 360.0f);

		if (frames_this_second == 0) owner.doFirst();
		owner.doFrame();

		current_time = SDL_GetTicks();
		frame_runtime_finish[frame_offset] = current_time - finish_start_time;
		frame_duration[frame_offset] = current_time - next_frame_time;
		frame_delta[frame_offset] = _delta;
		frame_tempo[frame_offset] = _tempo;

		frames_this_second++;
		frame_offset++;

		// Milliseconds 0, 1, .., 16 are all less than 16.667 hence belong
		// to frame 0. This means frame 1 starts at millisecond 17.
		last_frame_time = frame_start_time;
		current_time = SDL_GetTicks();
		uint8_t nextframe = int((current_time - second_start_time) * FRAMERATE / 1000.0f) + 1;
		next_frame_time = second_start_time + ceil(1000.0f * nextframe / FRAMERATE);

		wakes_this_second++;

		while (SDL_GetTicks() + 10 < next_frame_time)
		{
			SDL_Delay(10);
			wakes_this_second++;
		}
		while (SDL_GetTicks() < next_frame_time)
		{
			SDL_Delay(1);
			wakes_this_second++;
		}
	}
}

void EngineLoop::stop()
{
	running = false;
}

void EngineLoop::event()
{
	events_this_second++;
}

void EngineLoop::flip()
{
	update_start_time = SDL_GetTicks();
	frame_runtime_flip[frame_offset] = update_start_time - frame_start_time;
}

void EngineLoop::expose()
{
	/* IMGUI */
	static bool show = false;
	bool wasshown = show;

	if (Input::get()->isDebugKeyHeld())
	{
		if (ImGui::Begin("Windows", nullptr,
			ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse))
		{
			ImGui::Checkbox("Performance", &show);
		}
		ImGui::End();
	}

	if (show)
	{
		if (!wasshown) ImGui::SetNextWindowCollapsed(false);

		if (ImGui::Begin("Performance", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::PushItemWidth(200);
			ImGui::DragFloat("Engine Speed", &_engineSpeed, 0.1f, 0.0f, 10.0f);
			ImGui::DragFloat("Default Tempo", &_defaultTempo, 0.1f, 0.0f, 10.0f);
			ImGui::DragFloat("Hit Stop Amount", &_hitstop_amount, 0.025f, 0.0f, 0.5f);
			ImGui::PopItemWidth();
			ImGui::Separator();
			ImVec2 fullsize(200, 80);
			ImVec2 halfsize(200, 40);
			ImVec2 minisize(200, 20);
			ImGui::PlotLines("Wakes This Second", wakes_this_second_buffer, BUFFERSIZE,
					buffer_offset, wakes_this_second_string.c_str(),
					0.0f, FLT_MAX, halfsize);
			ImGui::PlotLines("Events This Second", events_this_second_buffer, BUFFERSIZE,
					buffer_offset, events_this_second_string.c_str(),
					0.0f, FLT_MAX, halfsize);
			ImGui::PlotLines("Frames This Second", frames_this_second_buffer, BUFFERSIZE,
					buffer_offset, frames_this_second_string.c_str(),
					0.0f, 1.0f * FRAMERATE, fullsize);
			ImGui::Separator();
			uint32_t frames_last_second = frames_this_second_buffer[(buffer_offset + BUFFERSIZE - 1) % BUFFERSIZE];
			uint8_t buffersize = std::min(FRAMERATE, (uint8_t) std::min(
					std::max(frames_this_second, frames_last_second),
					uint32_t(256)));
			float frametime = 1000.0f / FRAMERATE;
			ImGui::PlotLines("Frame Start Delay", frame_start_delay, buffersize,
					0, NULL, 0.0f, 0.5f * frametime, minisize);
			ImGui::PlotLines("Frame Flip Runtime", frame_runtime_flip, buffersize,
					0, NULL, 0.0f, 0.5f * frametime, minisize);
			ImGui::PlotLines("Frame Update Runtime", frame_runtime_update, buffersize,
					0, NULL, 0.0f, frametime, halfsize);
			ImGui::PlotLines("Frame Draw Runtime", frame_runtime_draw, buffersize,
					0, NULL, 0.0f, frametime, halfsize);
			ImGui::PlotLines("Frame Finish Runtime", frame_runtime_finish, buffersize,
					0, NULL, 0.0f, 0.5f * frametime, minisize);
			ImGui::PlotLines("Frame Duration", frame_duration, buffersize,
					0, frame_duration_string.c_str(), 0.0f, frametime, fullsize);
			ImGui::PlotLines("Frame Delta", frame_delta, buffersize,
					0, NULL, 0.0f, 0.002f * frametime, halfsize);
			ImGui::PlotLines("Frame Tempo", frame_tempo, buffersize,
					0, NULL, 0.0f, 2.0f, halfsize);
		}
		ImGui::End();
	}

	draw_start_time = SDL_GetTicks();
	frame_runtime_update[frame_offset] = draw_start_time - update_start_time;
}

void EngineLoop::finish()
{
	finish_start_time = SDL_GetTicks();
	frame_runtime_draw[frame_offset] = finish_start_time - draw_start_time;
}
