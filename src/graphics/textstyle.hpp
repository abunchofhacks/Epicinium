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

#include "font.hpp"
#include "paint.hpp"


struct TextStyle : public FontStyle
{
	Font font;
	Paint textcolor;
	Paint outlinecolor;

	TextStyle(int sz, const Color& color) :
		TextStyle(FontStyle(sz), color)
	{}

	TextStyle(const FontStyle& fontstyle, const Color& color) :
		TextStyle(fontstyle, color, color)
	{}

	TextStyle(const FontStyle& fontstyle, const Color& tc, const Color& oc) :
		FontStyle(fontstyle),
		font(Font::DEFAULT),
		textcolor(tc),
		outlinecolor(oc)
	{}
};
