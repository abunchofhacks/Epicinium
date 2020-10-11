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
#include "color.hpp"
#include "source.hpp"

#include "parseerror.hpp"


std::string Color::toString() const
{
	std::stringstream strm;
	strm << '#';
	strm << std::hex << std::setfill('0');
	strm << std::setw(2) << (int) (uint8_t) r;
	strm << std::setw(2) << (int) (uint8_t) g;
	strm << std::setw(2) << (int) (uint8_t) b;
	if (a != 0xFF)
	{
		strm << std::setw(2) << (int) (uint8_t) a;
	}
	return strm.str();
}

Color Color::parse(std::string fullstr)
{
	std::string str = fullstr;
	if (!fullstr.empty() && fullstr[0] == '#')
	{
		str = fullstr.substr(1);
	}

	uint32_t hex;
	try
	{
		size_t offset;
		hex = std::stoul(str, &offset, 16);
		if (offset != str.size())
		{
			throw ParseError("Invalid characters in color '" + fullstr + "'");
		}
	}
	catch (...)
	{
		throw ParseError("Failed to parse '" + fullstr + "' as a color");
	}

	if (str.size() == 6)
	{
		DEBUG_ASSERT(hex <= 0xFFFFFF);
		uint32_t rgb = hex;
		uint8_t r = 0xFF & (rgb >> 16);
		uint8_t g = 0xFF & (rgb >> 8);
		uint8_t b = 0xFF & rgb;
		return {r, g, b};
	}
	else if (str.size() == 8)
	{
		uint32_t rgba = hex;
		uint8_t r = 0xFF & (rgba >> 24);
		uint8_t g = 0xFF & (rgba >> 16);
		uint8_t b = 0xFF & (rgba >> 8);
		uint8_t a = 0xFF & rgba;
		return {r, g, b, a};
	}
	else if (str.size() == 3)
	{
		DEBUG_ASSERT(hex <= 0xFFF);
		uint32_t rgb = hex;
		uint8_t r = 0xF & (rgb >> 8);
		uint8_t g = 0xF & (rgb >> 4);
		uint8_t b = 0xF & rgb;
		r |= (r << 4);
		g |= (g << 4);
		b |= (b << 4);
		return {r, g, b};
	}
	else if (str.size() == 4)
	{
		DEBUG_ASSERT(hex <= 0xFFFF);
		uint32_t rgba = hex;
		uint8_t r = 0xF & (rgba >> 12);
		uint8_t g = 0xF & (rgba >> 8);
		uint8_t b = 0xF & (rgba >> 4);
		uint8_t a = 0xF & rgba;
		r |= (r << 4);
		g |= (g << 4);
		b |= (b << 4);
		a |= (a << 4);
		return {r, g, b, a};
	}
	else if (str.size() < 3)
	{
		throw ParseError("Too few digits in color '" + fullstr + "'");
	}
	else
	{
		throw ParseError("Wrong number of digits in color '" + fullstr + "'");
	}
}
