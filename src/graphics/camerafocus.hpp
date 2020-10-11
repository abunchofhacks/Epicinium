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

#include "point.hpp"
#include "animator.hpp"

class Settings;
struct AnimationGroup;


class CameraFocus : private Animator
{
public:
	CameraFocus(const Settings& settings, int measure);

private:
	const Settings& _settings;
	const int _measure;

	float _maxSpeed = 1000.0f;
	float _accSpeed = 5000.0f;
	float _decSpeed = 5000.0f;

	float _hspeed = 0;
	float _vspeed = 0;

	bool _acting = true;
	bool _keysEnabled = true;

	bool _scrollingWasd = true;
	bool _scrollingArrows = true;
	bool _scrollingEdge = true;

	Point _point;
	float _xenon;
	float _yahoo;
	float _shake = 0;

	bool _panning = false;
	Point _target;

	Point _topleftLimit;
	Point _bottomrightLimit;

	uint16_t _panningAudibleId;

public:
	void load(const Point& topleft, const Point& bottomright);
	void update();

	void shake(std::shared_ptr<AnimationGroup> group,
		float amount, float durationSolid, float durationFalloff, float delay);

	void set(const Point& point);
	void pan(const Point& point);
	void stop();
	bool panning() { return _panning; }

	const Point& point() { return _point; }

	void startActing() { _acting = true; }
	void stopActing() { _acting = false; }

	void enableKeys() { _keysEnabled = true; }
	void disableKeys() { _keysEnabled = false; }
};
