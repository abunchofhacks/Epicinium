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
#include "selectorcontext.hpp"
#include "source.hpp"

#include "board.hpp"
#include "settings.hpp"
#include "move.hpp"
#include "stackedlayout.hpp"
#include "image.hpp"
#include "verticallayout.hpp"
#include "textfield.hpp"
#include "padding.hpp"
#include "paint.hpp"
#include "colorname.hpp"
#include "graphics.hpp"


SelectorContext::SelectorContext(const Board& board,
		const Settings& settings,
		Cell cell, const Player& player) :
	Context(cell.pos()),
	_board(board),
	_settings(settings),
	_cell(cell),
	_tile(  _board.tile(_cell)   && _board.tile(_cell).owner   == player),
	_ground(_board.ground(_cell) && _board.ground(_cell).owner == player),
	_air(   _board.air(_cell)    && _board.air(_cell).owner    == player)
{
	if (! (_tile || _ground || _air)) return;

	add(Move::X, nullptr, true, nullptr, ColorName::FRAME400, "");
	add(Move::E, nullptr, _ground, nullptr, ColorName::FRAME400,
		_("GROUND"),
		_("UNIT"));
	add(Move::S, nullptr, _tile, nullptr, ColorName::FRAME400,
		_("TILE"));
	add(Move::W, nullptr, false, nullptr, ColorName::FRAME400, "");
	add(Move::N, nullptr, _air, nullptr, ColorName::FRAME400,
		_("AIR"),
		_("UNIT"));
}

std::unique_ptr<InterfaceElement> SelectorContext::createPanel(const Position&,
	const Move& index, const char*,
	const char*, const Paint&,
	const std::string& text, const std::string& subtext)
{
	std::unique_ptr<InterfaceElement> panel(new StackedLayout());

	(*panel).add("panel", new Image(spritenamePanel(index)));

	(*panel)["panel"].makeClickable();

	(*panel)["panel"].setBasicColor(0, ColorName::FRAME100);
	(*panel)["panel"].setBasicColor(1, Paint::mix(ColorName::FRAME100, ColorName::FRAME400, 0.5f));
	(*panel)["panel"].setBasicColor(2, Paint::blend(ColorName::FRAME400, ColorName::SHINEBLEND));
	(*panel)["panel"].setBasicColor(3, ColorName::FRAME400);
	(*panel)["panel"].setBasicColor(4, Paint::alpha(ColorName::FRAME400, 127));

	(*panel)["panel"].setColor(5, Color::transparent());

	if (!text.empty())
	{
		(*panel).add("texts", new Padding());
		(*panel)["texts"].put(new VerticalLayout());
		(*panel)["texts"].add("text",
			new TextField(text,
				_settings.getFontSizeMenuButton(),
				ColorName::TEXT800));
		if (!subtext.empty())
		{
			(*panel)["texts"].add("subtext",
				new TextField(subtext,
					_settings.getFontSize(),
					ColorName::TEXT700));
		}
		(*panel)["texts"].align(HorizontalAlignment::CENTER);

		switch (index)
		{
			case Move::X:
			break;

			case Move::E:
			{
				(*panel)["texts"].setPaddingRight(8 * scale());
			}
			break;

			case Move::S:
			{
				(*panel)["texts"].setPaddingBottom(12 * scale());
			}
			break;

			case Move::W:
			{
				(*panel)["texts"].setPaddingLeft(8 * scale());
			}
			break;

			case Move::N:
			{
				(*panel)["texts"].setPaddingTop(16 * scale());
			}
			break;
		}

		(*panel)["texts"].settle();
	}

	(*panel).align(HorizontalAlignment::CENTER);
	(*panel).align(VerticalAlignment::MIDDLE);
	(*panel).settle();

	return panel;
}

bool SelectorContext::empty() const
{
	return (_tile + _ground + _air == 0);
}

bool SelectorContext::trivial() const
{
	return (_tile + _ground + _air <= 1);
}

Descriptor SelectorContext::resolve() const
{
	if (!_hoverValid)
	{
		return Descriptor();
	}

	Move index = _hoverIndex;

	if (index == Move::X)
	{
		if      (_ground) index = Move::E;
		else if (_air)    index = Move::N;
		else if (_tile)   index = Move::S;
		else return Descriptor();
	}

	switch (index)
	{
		case Move::E: return Descriptor::ground(_cell.pos());
		case Move::S: return Descriptor::tile(  _cell.pos());
		case Move::N: return Descriptor::air(   _cell.pos());
		default:      return Descriptor();
	}
}
