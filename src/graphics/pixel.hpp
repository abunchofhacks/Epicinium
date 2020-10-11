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


enum class Layer : uint8_t
{
	BACKGROUND, // MIN
	SURFACE,
	FIGURE,
	UNDERLAY,
	CONTEXT,
	INTERFACE,
	POPUP,
	TOOLTIP,
	// 255 INTENTIONALLY LEFT BLANK
};

namespace SurfaceDrawFlag
{
	enum : int
	{
		UNMASKED = 0x01,
		MASKED = 0x02,
	};
}

struct Proximity
{
	uint32_t index;

private:
	static constexpr uint32_t LAYERSIZE = (1 << 16);

public:
	constexpr Proximity() :
		Proximity(Layer::BACKGROUND)
	{}

	constexpr Proximity(Layer z) :
		Proximity((uint32_t) z * LAYERSIZE)
	{}

private:
	constexpr Proximity(uint32_t x) :
		index(x)
	{}

public:
	constexpr Proximity operator+(uint16_t offset) const
	{
		return Proximity(index + offset);
	}

	Proximity& operator+=(uint16_t offset)
	{
		index += offset;
		return *this;
	}

	constexpr Proximity layer() const
	{
		return Proximity((index / LAYERSIZE) * LAYERSIZE);
	}

	constexpr bool operator==(Proximity other) const
	{
		return (index == other.index);
	}

	constexpr bool operator!=(Proximity other) const
	{
		return (index != other.index);
	}

	constexpr bool operator<(Proximity other) const
	{
		return (index < other.index);
	}

	constexpr bool operator>(Proximity other) const
	{
		return (index > other.index);
	}

	constexpr bool operator<=(Proximity other) const
	{
		return (index <= other.index);
	}

	constexpr bool operator>=(Proximity other) const
	{
		return (index >= other.index);
	}
};

struct Pixel
{
	int xenon;
	int yahoo;
	Proximity proximity;

	constexpr Pixel() :
		Pixel(0, 0)
	{}

	constexpr Pixel(int x, int y) :
		xenon(x),
		yahoo(y)
	{}

	constexpr Pixel(int x, int y, Proximity z) :
		xenon(x),
		yahoo(y),
		proximity(z)
	{}

	constexpr Pixel operator^(Proximity z) const
	{
		return Pixel(xenon, yahoo, z);
	}

	float distanceTo(const Pixel& other) const;
};
