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
#include "footprint.hpp"
#include "source.hpp"

#include "sprite.hpp"
#include "collector.hpp"
#include "player.hpp"
#include "loop.hpp"
#include "paint.hpp"
#include "colorname.hpp"


Paint Footprint::teamColor(const Player& player)
{
	switch (player)
	{
		case Player::NONE: return ColorName::SELECTIONNEUTRAL;
		case Player::RED: return ColorName::SELECTIONRED;
		case Player::BLUE: return ColorName::SELECTIONBLUE;
		case Player::TEAL: return ColorName::SELECTIONTEAL;
		case Player::YELLOW: return ColorName::SELECTIONYELLOW;
		case Player::PINK: return ColorName::SELECTIONPINK;
		case Player::BLACK: return ColorName::SELECTIONBLACK;
		case Player::INDIGO: return ColorName::SELECTIONINDIGO;
		case Player::PURPLE: return ColorName::SELECTIONPURPLE;

		case Player::BLIND:
		case Player::OBSERVER:
		case Player::SELF:
		break;
	}

	LOGE << "Player '" << player << "' without team color";
	DEBUG_ASSERT(false);
	return ColorName::SELECTIONNEUTRAL;
}

Footprint::Footprint(const Point& figurepoint, const std::string& spritename) :
	_sprite((!spritename.empty()) ? new Sprite(spritename) : nullptr),
	_point(figurepoint),
	_selected(false)
{
	if (_sprite)
	{
		_sprite->setOriginAtCenter();
		_sprite->setColor(0, teamColor(Player::NONE));
		_sprite->setShineColor(Paint(ColorName::GUIDESHINE));
		_sprite->setShine(1);
	}
}

Footprint::~Footprint() = default;

void Footprint::update()
{
	if (!_sprite) return;

	if (_selected) Collector::get()->addFootprint(_sprite, _point);
}

void Footprint::select()
{
	if (!_sprite) return;

	_selected = true;
	_sprite->setThetaOffset(-0.2f - Loop::theta());
}

void Footprint::deselect()
{
	if (!_sprite) return;

	_selected = false;
}

void Footprint::setPlayer(const Player& player)
{
	if (!_sprite) return;

	_sprite->setColor(0, teamColor(player));
}
