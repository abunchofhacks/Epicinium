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
#include "targetcursor.hpp"
#include "source.hpp"

#include "bible.hpp"
#include "board.hpp"
#include "sprite.hpp"
#include "collector.hpp"
#include "surface.hpp"
#include "aim.hpp"
#include "guide.hpp"
#include "paint.hpp"
#include "colorname.hpp"
#include "input.hpp"
#include "mixer.hpp"


TargetCursor::TargetCursor(const Bible& bible, const Board& board, Cell from,
		const Order& order, const char* stampspritename) :
	_bible(bible),
	_board(board),
	_origin(from),
	_target(Cell::undefined()),
	_order(order),
	_minrange(0),
	_maxrange(0),
	_valid(false),
	_hoversprite(new Sprite("ui/cursor_hover")),
	_stampsprite(new Sprite(stampspritename)),
	_invalidsprite(new Sprite("ui/cursor_invalid")),
	_outofboundssprite(new Sprite("ui/cursor_outofbounds"))
{
	determineRanges();

	_hoversprite->setColor(0, Paint(ColorName::GUIDEVALIDTARGET));
	_hoversprite->setOriginAtBase();
	_stampsprite->setColor(0, Paint(ColorName::GUIDEVALIDTARGET));
	_stampsprite->setOriginAtBase();
	_stampsprite->setOffset(0, 3);
	_invalidsprite->setColor(0, Paint(ColorName::CURSORINVALID));
	_invalidsprite->setOriginAtBase();

	_outofboundssprite->setColor(0, Paint(ColorName::CURSORINVALID));
	_outofboundssprite->setOriginAtCenter();

	Mixer::get()->queue(Clip::Type::COMMANDSCREEN, 0.0f);

	for (Cell index : _board.area(_origin, _minrange, _maxrange))
	{
		if (checkValidity(index))
		{
			_foci.push_back(index);
			_targetguides.push_back(Guide::validtarget(index));
		}
		else
		{
			_targetguides.push_back(Guide::invalidtarget(index));
		}
	}
}

TargetCursor::~TargetCursor() = default;

void TargetCursor::update()
{
	if (!_target.valid())
	{
		Collector::get()->addBusycursor(_outofboundssprite,
			Input::get()->mousePixel());
	}
	else if (_valid)
	{
		Point point = Surface::convertOrigin(_target.pos());
		Collector::get()->addHovercursor(_hoversprite, point);

		point.xenon = point.xenon - Surface::WIDTH / 3 + 1;
		Collector::get()->addGuidestamp(_stampsprite, point);
	}
	else
	{
		Collector::get()->addHovercursor(_invalidsprite,
			Surface::convertOrigin(_target.pos()));
	}

	for (Guide& guide : _targetguides)
	{
		guide.update();
	}
}

void TargetCursor::set(Cell to)
{
	_target = to;
	if (!_target.valid())
	{
		_valid = false;
		return;
	}

	_order.target = Descriptor::cell(_target.pos());

	_valid = checkValidity();
}

bool TargetCursor::valid() const
{
	return _valid;
}

Order TargetCursor::resolve() const
{
	return _order;
}

void TargetCursor::determineRanges()
{
	switch (_order.type)
	{
		case Order::Type::GUARD:
		{
			_minrange = 1;
			_maxrange = 1;
		}
		break;

		case Order::Type::FOCUS:
		{
			_minrange = 1;
			_maxrange = 1;
		}
		break;

		case Order::Type::LOCKDOWN:
		{
			_minrange = 1;
			_maxrange = 1;
		}
		break;

		case Order::Type::SHELL:
		{
			const UnitToken& unit = _board.unit(_origin, _order.subject.type);
			_minrange = _bible.unitRangeMin(unit.type);
			_maxrange = _bible.unitRangeMax(unit.type);
		}
		break;

		case Order::Type::BOMBARD:
		{
			const UnitToken& unit = _board.unit(_origin, _order.subject.type);
			_minrange = _bible.unitRangeMin(unit.type);
			_maxrange = _bible.unitRangeMax(unit.type);
		}
		break;

		case Order::Type::EXPAND:
		{
			_minrange = _bible.tileExpandRangeMin();
			_maxrange = _bible.tileExpandRangeMax();
		}
		break;

		case Order::Type::PRODUCE:
		{
			_minrange = _bible.tileProduceRangeMin();
			_maxrange = _bible.tileProduceRangeMax();
		}
		break;

		case Order::Type::MOVE:
		case Order::Type::BOMB:
		case Order::Type::CAPTURE:
		case Order::Type::SHAPE:
		case Order::Type::SETTLE:
		case Order::Type::UPGRADE:
		case Order::Type::CULTIVATE:
		case Order::Type::HALT:
		case Order::Type::NONE:
		break;
	}
}

bool TargetCursor::checkValidity() const
{
	return checkValidity(_target);
}

bool TargetCursor::checkValidity(Cell target) const
{
	// The target cell must be in range.
	Aim aim(_origin.pos(), target.pos());
	int distance = aim.sumofsquares();
	if (distance < _minrange || distance > _maxrange) return false;

	switch (_order.type)
	{
		case Order::Type::GUARD:
		{
			// The tile to guard must be reachable by ground units.
			const TileToken& tile = _board.tile(target);
			if (!tile) return false;
			return _bible.tileWalkable(tile.type);
		}
		break;

		case Order::Type::FOCUS:
		{
			// The tile to focus must be reachable by ground units.
			const TileToken& tile = _board.tile(target);
			if (!tile) return false;
			return _bible.tileWalkable(tile.type);
		}
		break;

		case Order::Type::LOCKDOWN:
		{
			// The tile to lockdown must be reachable by ground units.
			const TileToken& tile = _board.tile(target);
			if (!tile) return false;
			return _bible.tileWalkable(tile.type);
		}
		break;

		case Order::Type::SHELL:
		{
			// The tile to shell must be reachable by ground or air units.
			const TileToken& tile = _board.tile(target);
			if (!tile) return false;
			return _bible.tileAccessible(tile.type);
		}
		break;

		case Order::Type::BOMBARD:
		{
			// The tile to bombard must be reachable by ground or air units.
			const TileToken& tile = _board.tile(target);
			if (!tile) return false;
			return _bible.tileAccessible(tile.type);
		}
		break;

		case Order::Type::EXPAND:
		{
			// The tile to be built on must be buildable.
			const TileToken& tile = _board.tile(target);
			if (!tile) return false;
			return _bible.tileBuildable(tile.type);
		}
		break;

		case Order::Type::UPGRADE:
		{
			// Note that tiles can upgrade themselves,
			// even though they are not tileBuildable().
			return true;
		}
		break;

		case Order::Type::PRODUCE:
		{
			// The tile to produce onto must be reachable by ground units,
			// unless the produced unit is an air unit.
			const TileToken& tile = _board.tile(target);
			if (!tile) return false;
			if (!_bible.tileWalkable(tile.type)
				&& !_bible.unitAir(_order.unittype))
			{
				return false;
			}
			return true;
		}
		break;

		case Order::Type::MOVE:
		case Order::Type::BOMB:
		case Order::Type::CAPTURE:
		case Order::Type::SHAPE:
		case Order::Type::SETTLE:
		case Order::Type::CULTIVATE:
		case Order::Type::HALT:
		case Order::Type::NONE:
		break;
	}
	return false;
}
