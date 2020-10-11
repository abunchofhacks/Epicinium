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
#include "damage.hpp"
#include "source.hpp"

#include "unittoken.hpp"
#include "tiletoken.hpp"
#include "bible.hpp"
#include "board.hpp"
#include "cell.hpp"


Damage::Shot::Shot(const Descriptor& desc, const UnitToken* token, int8_t fig,
		int8_t damage) :
	desc(desc),
	unit(token),
	figure(fig),
	damage(damage)
{}

Damage::Body::Body(const Descriptor& desc, const UnitToken* token, int8_t fig,
		int8_t hp) :
	desc(desc),
	unit(token),
	tile(nullptr),
	figure(fig),
	killable(true),
	depowerable(false),
	hitpoints(hp),
	taken(0)
{}

Damage::Body::Body(const Descriptor& desc, const TileToken* token, int8_t fig,
		int8_t hp, bool killable, bool depowerable) :
	desc(desc),
	unit(nullptr),
	tile(token),
	figure(fig),
	killable(killable),
	depowerable(depowerable),
	hitpoints(hp),
	taken(0)
{}

Damage::Hit::Hit(const Shot& shot, const Body& body,
		bool killing, bool depowering) :
	shot(shot),
	body(body),
	killing(killing),
	depowering(depowering)
{}

void Damage::addAttacker(const Descriptor& desc, const UnitToken& unit)
{
	for (int stack = 0; stack < unit.stacks; stack++)
	{
		for (int i = 0; i < _bible.unitAttackShots(unit.type); i++)
		{
			_shots.emplace_back(desc, &unit, stack,
				_bible.unitAttackDamage(unit.type));
		}
	}
}

void Damage::addTrampler(const Descriptor& desc, const UnitToken& unit)
{
	for (int stack = 0; stack < unit.stacks; stack++)
	{
		for (int i = 0; i < _bible.unitTrampleShots(unit.type); i++)
		{
			_shots.emplace_back(desc, &unit, stack,
				_bible.unitTrampleDamage(unit.type));
		}
	}
}

void Damage::addCaster(const Descriptor& desc, const UnitToken& unit)
{
	for (int stack = 0; stack < unit.stacks; stack++)
	{
		for (int i = 0; i < _bible.unitAbilityShots(unit.type); i++)
		{
			_shots.emplace_back(desc, &unit, stack,
				_bible.unitAbilityDamage(unit.type));
		}
	}
}

void Damage::addEffect(const Descriptor& desc, int8_t shots, int8_t damage)
{
	for (int i = 0; i < shots; i++)
	{
		_shots.emplace_back(desc, nullptr, -1, damage);
	}
}

void Damage::addTaker(const Descriptor& desc, const UnitToken& unit)
{
	// Determine the hitpoints each body has.
	int hp = hitpoints(_board.cell(desc.position), unit);

	// Add the actual figures as bodies.
	for (int stack = 0; stack < unit.stacks; stack++)
	{
		// Each body is killable.
		_bodies.emplace_back(desc, &unit, stack, hp);
	}
}

void Damage::addTaker(const Descriptor& desc, const TileToken& tile)
{
	// If the tile has no stacks, we add a ground miss chance.
	if (tile.stacks == 0)
	{
		for (int i = 0; i < _bible.missCountGround(); i++)
		{
			// This miss chance is unkillable.
			_bodies.emplace_back(desc, &tile, -1, _bible.missHitpointsGround(),
				false, false);
		}
		return;
	}

	// Determine the hitpoints each body has.
	int hp = hitpoints(_board.cell(desc.position), tile);

	// Add the actual figures as bodies.
	for (int stack = 0; stack < tile.stacks; stack++)
	{
		// Each body is killable.
		// It is important that some of the bodies may belong to powered stacks.
		bool powered = stack < tile.power;
		_bodies.emplace_back(desc, &tile, stack, hp, true, powered);
	}
}

void Damage::addBackground(const Descriptor& desc, const UnitToken& unit)
{
	// Impossible.
	if (unit.stacks == 0) return;

	// Determine the hitpoints each body has.
	int hp = hitpoints(_board.cell(desc.position), unit);

	// If the unit is in the background, only one body is added.
	// This body is killable.
	int8_t fig = (rand() % unit.stacks);
	_bodies.emplace_back(desc, &unit, fig, hp);
}

void Damage::addBackground(const Descriptor& desc, const TileToken& tile)
{
	// If the tile has no stacks, there is no background body.
	if (tile.stacks == 0) return;

	// Determine the hitpoints each body has.
	int hp = hitpoints(_board.cell(desc.position), tile);

	// If the tile has stacks, only one body is added.
	// This body is killable.
	bool killable = _bible.collateralDamageKillsTiles();
	// It is important that this body may belong to a powered stack.
	bool powered = (tile.power > 0);
	int8_t fig = powered ? (rand() % tile.power) : (rand() % tile.stacks);
	_bodies.emplace_back(desc, &tile, fig, hp, killable, powered);
}

void Damage::addTrenches(const Descriptor& desc, const TileToken& tile)
{
	// The trenches provide some ground miss chances.
	for (int i = 0; i < _bible.missCountTrenches(); i++)
	{
		// This body is unkillable.
		_bodies.emplace_back(desc, &tile, -1, _bible.missHitpointsTrenches(),
			false, false);
	}
}

void Damage::addClouds(const Descriptor& desc)
{
	// The clouds provide some air miss chances.
	for (int i = 0; i < _bible.missCountAir(); i++)
	{
		// This body is unkillable.
		_bodies.emplace_back(desc, nullptr, -1, _bible.missHitpointsAir(),
			false, false);
	}
}

size_t Damage::shots() const
{
	// If there are no shots fired, the damage step can be skipped.
	return _shots.size();
}

size_t Damage::bodies() const
{
	// If there are no bodies, the damage step must be skipped.
	return _bodies.size();
}

void Damage::resolve()
{
	if (_bodies.size() == 0)
	{
		LOGE << "Cannot resolve Damage without bodies";
		DEBUG_ASSERT(false && "Cannot resolve Damage without bodies");
		return;
	}

	_targets.reserve(_bodies.size());

	// Shots are fired in sequential order.
	_bullets.reserve(_shots.size());
	for (size_t i = 0; i < _shots.size(); i++)
	{
		_bullets.push_back(i);
	}

	// Every bullet that is fired must impact something.
	for (uint8_t bullet : _bullets)
	{
		Shot& shot = _shots[bullet];

		// Bullets randomly hit one of the targets.
		if (_targets.empty())
		{
			for (size_t i = 0; i < _bodies.size(); i++)
			{
				_targets.push_back(i);
			}
		}
		size_t offset = rand() % _targets.size();

		// Apply damage.
		Body& body = _bodies[_targets[offset]];
		body.taken += shot.damage;

		// Store the hit, noting in particular if this hit killed the body.
		bool lethal = (body.taken >= body.hitpoints);
		_hits.emplace_back(shot, body,
			(lethal && body.killable),
			(lethal && body.depowerable));

		// If this bullet was lethal, the body is no longer a valid target.
		if (lethal)
		{
			body.killable = false;
			body.depowerable = false;
			_targets.erase(_targets.begin() + offset);
		}

		// If all the targets are killed, the remaining bullets are spread out
		// among all targets as overkill damage. These impacts have no effect
		// but must be shown to the player.
	}
}

int Damage::hitpoints(Cell index, const TileToken& tile)
{
	// Get default hitpoint value.
	int hp = _bible.tileHitpoints(tile.type);

	// Do we lose hitpoints due to vulnerability?
	int loss = 0;

	// Frostbite only affects ground units.
	if (_board.frostbite(index) >= _bible.frostbiteThresholdVulnerability()
			&& !_bible.frostbiteOnlyTargetsGroundUnits())
	{
		loss++;
	}

	// Bonedrought affects everything.
	if (_board.bonedrought(index))
	{
		loss++;
	}

	// Gas only affects ground units.
	if (_board.gas(index) >= _bible.gasThresholdVulnerability()
			&& !_bible.gasOnlyTargetsGroundUnits())
	{
		loss++;
	}

	// Radiation affects everything.
	if (_board.radiation(index) >= _bible.radiationThresholdVulnerability())
	{
		loss++;
	}

	// Do vulnerabilities stack?
	if (loss > 1 && !_bible.vulnerabilitiesStack())
	{
		loss = 1;
	}

	// Modify the hitpoint value.
	hp -= loss;

	// Minimum hitpoint value is 1.
	return std::min(std::max(1, hp), 100);
}

int Damage::hitpoints(Cell index, const UnitToken& unit)
{
	// Get default hitpoint value.
	int hp = _bible.unitHitpoints(unit.type);

	// Do we lose hitpoints due to vulnerability?
	int loss = 0;

	// Frostbite affects everything.
	if (_board.frostbite(index) >= _bible.frostbiteThresholdVulnerability()
			&& (!_bible.frostbiteOnlyTargetsGroundUnits()
				|| !_bible.unitAir(unit.type)))
	{
		loss++;
	}

	// Bonedrought affects everything.
	if (_board.bonedrought(index))
	{
		loss++;
	}

	// Gas only affects ground units.
	if (_board.gas(index) >= _bible.gasThresholdVulnerability()
			&& !_bible.unitAir(unit.type))
	{
		loss++;
	}

	// Radiation affects everything.
	if (_board.radiation(index) >= _bible.radiationThresholdVulnerability())
	{
		loss++;
	}

	// Do vulnerabilities stack?
	if (loss > 1 && !_bible.vulnerabilitiesStack())
	{
		loss = 1;
	}

	// Modify the hitpoint value.
	hp -= loss;

	// Minimum hitpoint value is 1.
	return std::min(std::max(1, hp), 100);
}
