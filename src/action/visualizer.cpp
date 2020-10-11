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
#include "visualizer.hpp"
#include "source.hpp"

#include "input.hpp"
#include "color.hpp"
#include "transitionator.hpp"


Visualizer::Type Visualizer::_type = Visualizer::Type::NONE;

void Visualizer::update()
{
	Input* input = Input::get();
	if (input->isKeyHeld(SDL_SCANCODE_ALT))
	{
		if (input->wasKeyPressed(SDL_SCANCODE_1))
		{
			_type = Type::NONE;
		}
		else if (input->wasKeyPressed(SDL_SCANCODE_2))
		{
			_type = Type::TEMPERATURE;
		}
		else if (input->wasKeyPressed(SDL_SCANCODE_3))
		{
			_type = Type::HUMIDITY;
		}
		else if (input->wasKeyPressed(SDL_SCANCODE_4))
		{
			_type = Type::CHAOS;
		}
	}
}

Color Visualizer::mix(const Color& base, float value) const
{
	Color color;
	switch (_type)
	{
		case Type::TEMPERATURE:
		{
			Color chart[21] =
			{
				/* -75 */ Color(  0,   0,   0),
				/* -65 */ Color(100, 100, 100),
				/* -55 */ Color(200, 200, 200),
				/* -45 */ Color(255, 255, 255),
				/* -35 */ Color(200, 100, 255),
				/* -25 */ Color(  0,   0, 255),
				/* -15 */ Color(  0, 150, 255),
				/*  -5 */ Color(  0, 255, 255),
				/*   5 */ Color(  0, 125, 100),
				/*  15 */ Color(  0, 200,  50),
				/*  25 */ Color(  0, 255,   0),
				/*  35 */ Color(200, 255,   0),
				/*  45 */ Color(255, 200,   0),
				/*  55 */ Color(255, 100,   0),
				/*  65 */ Color(255,   0,   0),
				/*  75 */ Color(255,   0, 100),
				/*  85 */ Color(155,   0, 155),
				/*  95 */ Color(  0,   0, 155),
				/* 105 */ Color(  0,   0,   0),
				/* 115 */ Color(200, 200, 200),
				/* 125 */ Color(255, 255, 255),
			};
			float fraction = (value - -75) / (125 - -75);
			// Rounding towards XX5 degrees on purpose: creates a divide at XX0 degrees.
			size_t index = std::min(std::max(0, (int) (fraction * 20 + 0.5f)), 20);
			color = chart[index];
		}
		break;

		case Type::HUMIDITY:
		{
			Color chart[11] =
			{
				/*   0 */ Color(250,  50,   0, /*blend=*/150),
				/*   1 */ Color(250, 200,   0, /*blend=*/150),
				/*   2 */ Color(  0, 240,   0, /*blend=*/100),
				/*   3 */ Color(100, 100, 250, /*blend=*/150),
				/*   4 */ Color(250, 250, 250, /*blend=*/200),
			};
			size_t index = std::min(std::max(0, (int) (value + 0.5f)), 4);
			color = chart[index];
		}
		break;

		case Type::CHAOS:
		{
			Color chart[12] =
			{
				/* pro */ Color(  0, 240,   0, /*blend=*/200),
				/*   0 */ Color(150, 140, 110, /*blend=*/150),
				/*   1 */ Color(250,  50,   0, /*blend=*/200),
			};
			size_t index = std::min(std::max(0, (int) (value + 0.5f)), 1);
			if (value < 0) index = 0;
			else index += 1;
			color = chart[index];
		}
		break;

		case Type::NONE: break;
	}
	return Color::blend(base, color);
}

size_t Visualizer::transitionIndex() const
{
	switch (_type)
	{
		case Type::TEMPERATURE: return TRANSITION_TEMPERATURE;
		case Type::HUMIDITY:    return TRANSITION_HUMIDITY;
		case Type::CHAOS:       return TRANSITION_CHAOS;

		case Type::NONE: return TRANSITION_IMPACT;
	}
	return 0;
}



