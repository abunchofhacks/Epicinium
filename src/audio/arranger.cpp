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
 * Can Ur (canur11@gmail.com)
 */
#include "arranger.hpp"
#include "source.hpp"

#include "mixer.hpp"
#include "loop.hpp"
#include "input.hpp"
#include "cycle.hpp"


Arranger* Arranger::_installed = nullptr;

Arranger::Arranger(uint8_t bpm, uint8_t signature) :
	_bpm(bpm),
	_signature(signature),
	_beat(signature - 2),
	_offset(-2 * 60.0 / bpm)
{
	_cues.emplace_back(Clip::Type::TICK, 1, 0);
	for (uint8_t i = 1; i < _signature; i++)
	{
		_cues.emplace_back(Clip::Type::TOCK, 1, i);
	}
	_cues.emplace_back(Clip::Type::SPRING, 41, 0);
	_cues.emplace_back(Clip::Type::SUMMER, 41, 0);
	_cues.emplace_back(Clip::Type::AUTUMN, 41, 0);
	_cues.emplace_back(Clip::Type::WINTER, 41, 0);
	_cues.emplace_back(Clip::Type::NIGHTTIME, 25, 0);
	_cues.emplace_back(Clip::Type::ACTION, 41, 0);
}

void Arranger::queueNextBeat(double delay)
{
	if (_muted) return;
	if (_cues.size() == 0) return;
	Mixer* mixer = Mixer::get();
	uint8_t nextBeat = (_beat + 1) % _signature;
	uint32_t nextMeasure = (nextBeat == 0 ? _measure + 1 : _measure);
	for (Cue& cue : _cues)
	{
		if (cue.beat != nextBeat) continue;
		if (nextMeasure % cue.measures != 0) continue;

		cue.audibleId = mixer->queue(cue.clip, delay, true, cue.volume);
		cue.queued = true;
		if (!cue.fader) continue;
		mixer->fade(cue.audibleId, cue.direction ? 1.0f : 0.0f, cue.remaining);
	}
}

void Arranger::fadeIn(Cue& cue, float duration)
{
	if (cue.volume == 1.0f) return;
	cue.direction = true;
	float origin = cue.volume;
	duration *= 1.0f - cue.volume;

	cue.fader.reset(new Animation(nullptr, [duration, origin, &cue](float progress) {

		cue.volume = origin + progress * (1.0f - origin);
		cue.remaining = (1.0f - progress) * duration;

	}, duration, 0));
	if (cue.queued) Mixer::get()->fade(cue.audibleId, 1.0f, duration);
}

void Arranger::fadeOut(Cue& cue, float duration)
{
	if (cue.volume == 0.0f) return;
	cue.direction = false;
	float origin = cue.volume;
	duration *= cue.volume;

	cue.fader.reset(new Animation(nullptr, [duration, origin, &cue](float progress) {

		cue.volume = origin - progress * origin;
		cue.remaining = (1.0f - progress) * duration;

	}, duration, 0));
	if (cue.queued) Mixer::get()->fade(cue.audibleId, 0.0f, duration);
}

void Arranger::install()
{
	_installed = this;
}

void Arranger::update()
{
	_offset += Loop::delta();

	double spb = 60.0 / _bpm;
	double spm = spb * _signature;

	if (_offset > spm)
	{
		_measure++;
		_beat = 0;
		_offset -= spm;
		queueNextBeat(spb - _offset);
	}

	for (int i = _beat + 1; i < _signature; i++)
	{
		if (_offset > i * spb)
		{
			_beat = i;
			queueNextBeat((_beat + 1) * spb - _offset);
		}
		else break;
	}

	if (_offset >= 0)
	{
		for (Cue& cue : _cues)
		{
			if (cue.fader)
			{
				cue.fader->update();
				if (cue.fader->finished()) cue.fader.reset(nullptr);
			}
		}
	}

	/* IMGUI */
	static bool show = false;
	bool wasshown = show;

	if (Input::get()->isDebugKeyHeld())
	{
		if (ImGui::Begin("Windows", nullptr,
			ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse))
		{
			ImGui::Checkbox("Arranger", &show);
		}
		ImGui::End();
	}

	if (show)
	{
		if (!wasshown) ImGui::SetNextWindowCollapsed(false);

		if (ImGui::Begin("Arranger", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::Checkbox("Mute", &_muted);
			int beat = _beat;
			ImGui::SliderInt("Beat", &beat, 0, _signature - 1);
			float offset = _offset;
			ImGui::SliderFloat("Offset", &offset, 0.0f, spm);
			static bool metronome = false;
			static bool wasmetronome = false;
			ImGui::Checkbox("Metronome", &metronome);
			if (!wasmetronome && metronome)
			{
				metronomeStart();
				wasmetronome = true;
			}
			else if (wasmetronome && !metronome)
			{
				metronomeStop();
				wasmetronome = false;
			}
		}
		ImGui::End();
	}
}

void Arranger::metronomeStart()
{
	for (Cue& cue : _cues)
	{
		if (cue.clip == Clip::Type::TICK || cue.clip == Clip::Type::TOCK)
		{
			fadeIn(cue, 1.0f);
		}
	}
}

void Arranger::metronomeStop()
{
	for (Cue& cue : _cues)
	{
		if (cue.clip == Clip::Type::TICK || cue.clip == Clip::Type::TOCK)
		{
			fadeOut(cue, 1.0f);
		}
	}
}

void Arranger::fadeInNightAmbience()
{
	for (Cue& cue : _cues)
	{
		switch (cue.clip)
		{
			case Clip::Type::NIGHTTIME: 	fadeIn(cue, 1.5f);		break;

			//All seasonal loops need to fade out
			case Clip::Type::SPRING:		fadeOut(cue, 1.5f);		break;
			case Clip::Type::AUTUMN:		fadeOut(cue, 1.5f);		break;
			case Clip::Type::SUMMER:		fadeOut(cue, 1.5f);		break;
			case Clip::Type::WINTER:		fadeOut(cue, 1.5f);		break;
			case Clip::Type::ACTION:		fadeOut(cue, 3.5f);		break;
			default:	break;
		}
	}
}

void Arranger::fadeOutNightAmbience()
{
	for (Cue& cue : _cues)
	{
		if (cue.clip == Clip::Type::NIGHTTIME) {
			fadeOut(cue, 1.0f);
			return;
		}
	}
}

void Arranger::fadeOutMusic() {
	for(Cue& cue : _cues)
	{
		switch(cue.clip)
		{
			case Clip::Type::SPRING: fadeOut(cue, 4.0f); break;
			case Clip::Type::SUMMER: fadeOut(cue, 4.0f); break;
			case Clip::Type::AUTUMN: fadeOut(cue, 4.0f); break;
			case Clip::Type::WINTER: fadeOut(cue, 4.0f); break;
			case Clip::Type::ACTION: fadeOut(cue, 4.0f); break;
			case Clip::Type::NIGHTTIME: fadeOut(cue, 4.0f); break;

			default: break;
		}
	}
}

void Arranger::newSeasonPhase(Season season, Phase phase)
{
	for (Cue& cue : _cues)
	{
		switch (cue.clip)
		{
			case Clip::Type::ACTION:
			{
				if(phase == Phase::ACTION) fadeIn(cue, 1.25f);
				else fadeOut(cue, 2.00f);
			}
			break;

			case Clip::Type::SPRING:
			{
				if (season == Season::SPRING && phase != Phase::ACTION) fadeIn(cue, 2.0f);
				else fadeOut(cue, 1.5f);
			}
			break;

			case Clip::Type::SUMMER:
			{
				if (season == Season::SUMMER && phase != Phase::ACTION)  fadeIn(cue, 2.0f);
				else fadeOut(cue, 1.5f);
			}
			break;

			case Clip::Type::AUTUMN:
			{
				if (season == Season::AUTUMN && phase != Phase::ACTION) fadeIn(cue, 2.0f);
				else fadeOut(cue, 1.5f);
			}
			break;

			case Clip::Type::WINTER:
			{
				if (season == Season::WINTER && phase != Phase::ACTION) fadeIn(cue, 2.0f);
				else fadeOut(cue, 1.5f);
			}
			break;

			default:
			break;
		}
	}
}

