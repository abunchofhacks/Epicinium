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

#include "unittoken.hpp"
#include "animator.hpp"
#include "descriptor.hpp"

class Square;
class Figure;
struct Change;
struct AnimationGroup;
class Bible;
class Skinner;
class PlacementBox;


class Unit : public UnitToken, private Animator
{
public:
	Unit(Square* square, const Bible& bible, const Skinner& skinner);
	Unit(const Unit&) = delete;
	Unit(Unit&& that);
	~Unit();

	Unit& operator=(const Unit&) = delete;
	Unit& operator=(Unit&& that);

private:
	Square* _square;
	const Bible& _bible;
	const Skinner& _skinner;
	std::vector<std::shared_ptr<Figure>> _figures; // (unique ownership)
	bool _selected = false;
	bool _dying = false;
	bool _air = false;
	bool _premoving = false;
	float _premoveDuration = 0;
	float _idleTimer = -1;

	PlacementBox newPlacement();
	std::shared_ptr<Figure> newFigure();

	void populate(std::shared_ptr<AnimationGroup> group);
	void reanimate(std::shared_ptr<AnimationGroup> group);

	bool isEntrenched(const Square* square, const Descriptor::Type& slot);

	float animateMove(const Change& change, std::shared_ptr<AnimationGroup> group);

public:
	void update();

	bool hovered() const;
	bool selected() const;

	void select();
	void deselect();

	void resetSquare(Square* dest);

	void enact(const Change& change, std::shared_ptr<AnimationGroup> group);
	void light(float light, float duration, float delay);
	void spotlight(float light, float duration, float delay);
	void flashlight(float light, float duration, float delay);

	void precleanup();
	void cleanup();

	void idle();
	void unidle();
};
