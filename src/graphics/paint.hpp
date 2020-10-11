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

#include "color.hpp"
#include "palette.hpp"

enum class ColorName : uint8_t;


class Paint
{
public:
	static constexpr size_t MAX_STEPS = 8;

private:
	struct Step
	{
		enum class Type : uint8_t
		{
			RAW, // Color(r,g,b,a)
			NAMED, // ColorName::XYZ
			MIX, // Color::mix(A, B, ratio)
			BLEND, // Color::blend(A, B, ratio)
			ALPHA, // Color::alpha(A, value)
		};

		Type type;
		union
		{
			Color data;
			ColorName name;
			float ratio;
			uint8_t value;
		};

		Step() :
			type(Type::RAW),
			data(Color::undefined())
		{}

		static Step mix(float ratio)
		{
			Step step;
			step.type = Type::MIX;
			step.ratio = ratio;
			return step;
		}

		static Step blend(float ratio)
		{
			Step step;
			step.type = Type::BLEND;
			step.ratio = ratio;
			return step;
		}

		static Step alpha(int value)
		{
			Step step;
			step.type = Type::ALPHA;
			step.value = value;
			return step;
		}
	};

	std::array<Step, MAX_STEPS> _steps;
	size_t _length;

	friend class Palette;

public:
	Paint() :
		_length(0)
	{}

	Paint(const Color& color)
	{
		_steps[0].type = Step::Type::RAW;
		_steps[0].data = color;
		_length = 1;
	}

	Paint(const ColorName& name)
	{
		_steps[0].type = Step::Type::NAMED;
		_steps[0].name = name;
		_length = 1;
	}

private:
	static Paint parse(const std::string& str);

public:
	static Paint parse(const Json::Value& json);
	Json::Value toJson() const;

	explicit operator bool() const
	{
		return _length > 0;
	}

	Color solve() const
	{
		// An empty Paint is valid but does not produce a defined color;
		// rather it deterministically produces the color 'undefined'.
		if (_length == 0)
		{
			return Color::undefined();
		}

		std::array<Color, MAX_STEPS> stack;
		size_t stacksize = 0;

		for (size_t i = 0; i < _length; i++)
		{
			DEBUG_ASSERT(stacksize < MAX_STEPS);
			const Step& step = _steps[i];
			switch (step.type)
			{
				case Step::Type::RAW:
				{
					stack[stacksize] = step.data;
					stacksize += 1;
				}
				break;
				case Step::Type::NAMED:
				{
					stack[stacksize] = Palette::get(step.name);
					stacksize += 1;
				}
				break;
				case Step::Type::MIX:
				{
					if (stacksize < 2) return Color::broken();
					Color a = stack[stacksize - 2];
					Color b = stack[stacksize - 1];
					stack[stacksize - 2] = Color::mix(a, b, step.ratio);
					stacksize -= 1;
				}
				break;
				case Step::Type::BLEND:
				{
					if (stacksize < 2) return Color::broken();
					Color a = stack[stacksize - 2];
					Color b = stack[stacksize - 1];
					stack[stacksize - 2] = Color::blend(a, b, step.ratio);
					stacksize -= 1;
				}
				break;
				case Step::Type::ALPHA:
				{
					if (stacksize < 1) return Color::broken();
					Color a = stack[stacksize - 1];
					stack[stacksize - 1] = Color::alpha(a, step.value);
				}
				break;
			}
		}

		if (stacksize > 1)
		{
			return Color::broken();
		}
		DEBUG_ASSERT(stacksize == 1);
		return stack[0];
	}

	operator Color() const
	{
		return solve();
	}

	static Paint mix(Paint, Paint, int ratio) = delete;

	static Paint mix(Paint left, Paint right, float ratio)
	{
		Paint result;
		if (!result.try_add_argument(left, 1))
		{
			return Color::broken();
		}
		if (!result.try_add_argument(right, 1))
		{
			return Color::broken();
		}
		if (!result.try_add_step(Step::mix(ratio)))
		{
			return Color::broken();
		}
		return result;
	}

	static Paint blend(Paint, Paint, int ratio) = delete;

	static Paint blend(Paint left, Paint right, float ratio)
	{
		Paint result;
		if (!result.try_add_argument(left, 1))
		{
			return Color::broken();
		}
		if (!result.try_add_argument(right, 1))
		{
			return Color::broken();
		}
		if (!result.try_add_step(Step::blend(ratio)))
		{
			return Color::broken();
		}
		return result;
	}

	static Paint blend(Paint left, Paint right)
	{
		return blend(left, right, 1.0f);
	}

	static Paint alpha(Paint base, int value)
	{
		if (value < 0 || value > 255)
		{
			return Color::broken();
		}

		Paint result;
		if (!result.try_add_argument(base, 1))
		{
			return Color::broken();
		}
		if (!result.try_add_step(Step::alpha(value)))
		{
			return Color::broken();
		}
		return result;
	}

	static Paint alpha(Paint base, float value) = delete;

private:
	bool try_add_argument(Paint arg, size_t reserved)
	{
		if (_length + arg._length + reserved > MAX_STEPS)
		{
			return false;
		}

		std::copy(arg._steps.begin(), arg._steps.begin() + arg._length,
			_steps.begin() + _length);
		_length += arg._length;
		return true;
	}

	bool try_add_step(Step step)
	{
		if (_length + 1 > MAX_STEPS)
		{
			return false;
		}

		_steps[_length] = step;
		_length += 1;
		return true;
	}
};
