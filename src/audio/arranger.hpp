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
#pragma once
#include "header.hpp"

#include <functional>

#include "clip.hpp"
#include "animation.hpp"

enum class Season : uint8_t;
enum class Phase : uint8_t;


class Arranger
{
private:
	static Arranger* _installed;

public:
	static Arranger* get() { return _installed; }

	Arranger(uint8_t bpm, uint8_t signature);
	Arranger(const Arranger&) = delete;
	Arranger(Arranger&&) = delete;
	Arranger& operator=(const Arranger&) = delete;
	Arranger& operator=(Arranger&&) = delete;
	~Arranger() = default;

private:
	struct Cue
	{
		Clip::Type clip;
		uint8_t measures;
		uint8_t beat;
		float volume = 0.0f;
		std::unique_ptr<Animation> fader = nullptr;
		float remaining = 0.0f;
		bool direction = false;
		bool queued = false;
		uint16_t audibleId = 0;

		Cue(Clip::Type c, uint8_t m, uint8_t b) : clip(c), measures(m), beat(b)
		{}
	};

	uint8_t _bpm;
	uint8_t _signature;
	bool _muted = false;

	std::vector<Cue> _cues;

	uint32_t _measure = -1;
	uint8_t _beat;
	double _offset;

	void queueNextBeat(double delay);
	void fadeIn(Cue& cue, float duration);
	void fadeOut(Cue& cue, float duration);

public:
	void install();
	void update();

	void metronomeStart();
	void metronomeStop();
	void newSeasonPhase(Season season, Phase phase);
	void fadeInNightAmbience();
	void fadeOutNightAmbience();
	void fadeOutMusic();
};
