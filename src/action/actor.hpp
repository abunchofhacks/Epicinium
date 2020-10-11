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

#include "transitionator.hpp"
#include "point.hpp"
#include "visualizer.hpp"
#include "particlebuffer.hpp"

class Sprite;


class Actor : public Transitionator
{
protected:
	Actor(const std::string& spritename, const Point& point);
	Actor(const std::string& spritename);
public:
	virtual ~Actor();

protected:
	std::shared_ptr<Sprite> _sprite; // (unique ownership)
	Point _point;
	Visualizer _visualizer;
	ParticleBuffer _particlebuffer;

	virtual void mix() = 0;
	virtual void drawSprite() = 0;

public:
	void update();

	void setTag(std::shared_ptr<AnimationGroup> group,
		const std::string& tag, float delay);
	void setTrigger(std::shared_ptr<AnimationGroup> group,
		const std::string& tag, const std::string& passivetag, float delay);
	void setTrigger(std::shared_ptr<AnimationGroup> group,
		const std::string& tag, float delay);
	void setVisible(std::shared_ptr<AnimationGroup> group, bool visible, float delay);
	void setVisible(std::shared_ptr<AnimationGroup> group, bool visible);
	void setVisible(bool visible);

	void addHitstop(std::shared_ptr<AnimationGroup> group, float delay,
		float multiplier = 1);
};
