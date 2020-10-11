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
#include "paint.hpp"
#include "source.hpp"

#include "colorname.hpp"
#include "writer.hpp"
#include "parseerror.hpp"


Paint Paint::parse(const std::string& str)
{
	DEBUG_ASSERT(!str.empty());
	if (str.empty())
	{
		throw ParseError("Empty recipe");
	}

	size_t curpos = str.find_first_of("(,)");
	if (curpos == std::string::npos)
	{
		if (str[0] == '#')
		{
			Color color = Color::parse(str);
			if (color)
			{
				return color;
			}
			else
			{
				throw ParseError("Invalid color '" + str + "'");
			}
		}
		else
		{
			return parseColorName(str);
		}
	}
	else if (str[curpos] != '(')
	{
		throw ParseError("Missing opening parenthesis in '" + str + "'");
	}
	else if (str[str.size() - 1] != ')')
	{
		throw ParseError("Missing closing parenthesis in '" + str + "'");
	}

	Paint result;
	std::string typestring = str.substr(0, curpos);

	size_t depth = 0;
	size_t argcount = 0;
	for (size_t i = str.find_first_of("(,)", curpos + 1);
		i < str.size() - 1;
		i = str.find_first_of("(,)", i + 1))
	{
		if (str[i] == ',')
		{
			if (depth == 0)
			{
				size_t len = i - (curpos + 1);
				if (len == 0)
				{
					throw ParseError("Missing argument in '" + str + "'");
				}
				Paint arg = Paint::parse(str.substr(curpos + 1, len));
				if (!result.try_add_argument(arg, 1))
				{
					throw ParseError("Recipe '" + str + "' is too complex");
				}
				argcount += 1;
				curpos = i;
			}
		}
		else if (str[i] == '(')
		{
			depth += 1;
		}
		else if (str[i] == ')')
		{
			if (depth > 0)
			{
				depth -= 1;
			}
			else
			{
				throw ParseError("Unbalanced parentheses within '" + str + "'");
			}
		}
	}
	if (depth > 0)
	{
		throw ParseError("Unbalanced parentheses within '" + str + "'");
	}

	size_t lastlen = (str.size() - 1) - (curpos + 1);
	if (lastlen == 0)
	{
		throw ParseError("Missing argument in '" + str + "'");
	}
	std::string lastarg = str.substr(curpos + 1, lastlen);

	if (typestring == "mix")
	{
		if (lastarg[lastarg.size() - 1] == '%')
		{
			if (argcount > 2)
			{
				throw ParseError("Too many arguments in '" + str + "'");
			}
			else if (argcount < 2)
			{
				throw ParseError("Too few arguments in '" + str + "'");
			}

			try
			{
				int percent = std::stoi(lastarg, &curpos, 10);
				if (percent >= 0 && curpos == lastarg.size() - 1)
				{
					result.try_add_step(Step::mix(0.01f * percent));
					return result;
				}
				else throw ParseError("Invalid percentage " + lastarg + "'");
			}
			catch (...)
			{
				throw ParseError("Invalid percentage " + lastarg + "'");
			}
		}
		else
		{
			if (argcount > 1)
			{
				throw ParseError("Too many arguments in '" + str + "'");
			}
			else if (argcount < 1)
			{
				throw ParseError("Too few arguments in '" + str + "'");
			}

			Paint arg = Paint::parse(lastarg);
			if (!result.try_add_argument(arg, 1))
			{
				throw ParseError("Recipe '" + str + "' is too complex");
			}
			result.try_add_step(Step::blend(1.00f));
			return result;
		}
	}
	else if (typestring == "blend")
	{
		if (lastarg[lastarg.size() - 1] == '%')
		{
			if (argcount > 2)
			{
				throw ParseError("Too many arguments in '" + str + "'");
			}
			else if (argcount < 2)
			{
				throw ParseError("Too few arguments in '" + str + "'");
			}

			try
			{
				int percent = std::stoi(lastarg, &curpos, 10);
				if (percent >= 0 && curpos == lastarg.size() - 1)
				{
					result.try_add_step(Step::blend(0.01f * percent));
					return result;
				}
				else throw ParseError("Invalid percentage " + lastarg + "'");
			}
			catch (...)
			{
				throw ParseError("Invalid percentage " + lastarg + "'");
			}
		}
		else
		{
			if (argcount > 1)
			{
				throw ParseError("Too many arguments in '" + str + "'");
			}
			else if (argcount < 1)
			{
				throw ParseError("Too few arguments in '" + str + "'");
			}

			Paint arg = Paint::parse(lastarg);
			if (!result.try_add_argument(arg, 1))
			{
				throw ParseError("Recipe '" + str + "' is too complex");
			}
			result.try_add_step(Step::blend(1.00f));
			return result;
		}
	}
	else if (typestring == "alpha")
	{
		if (argcount > 1)
		{
			throw ParseError("Too many arguments in '" + str + "'");
		}
		else if (argcount < 1)
		{
			throw ParseError("Too few arguments in '" + str + "'");
		}

		if (lastarg[lastarg.size() - 1] == '%')
		{
			try
			{
				int percent = std::stoi(lastarg, &curpos, 10);
				if (percent >= 0 && percent <= 100
					&& curpos == lastarg.size() - 1)
				{
					result.try_add_step(Step::alpha(percent * 255 / 100));
					return result;
				}
				else throw ParseError("Invalid percentage " + lastarg + "'");
			}
			catch (...)
			{
				throw ParseError("Invalid alpha value '" + lastarg + "'");
			}
		}
		else
		{
			try
			{
				uint32_t hex = std::stoul(lastarg, &curpos, 16);
				if (hex <= 0xFF && curpos == lastarg.size())
				{
					result.try_add_step(Step::alpha((uint8_t) hex));
					return result;
				}
				else throw ParseError("Invalid alpha value '" + lastarg + "'");
			}
			catch (...)
			{
				throw ParseError("Invalid alpha value '" + lastarg + "'");
			}
		}
	}
	else
	{
		throw ParseError("Unknown paint method '" + typestring + "'");
	}
}

Paint Paint::parse(const Json::Value& json)
{
	if (json.isNull())
	{
		return {};
	}
	else if (json.isString())
	{
		if (json.asString().empty())
		{
			return {};
		}
		else
		{
			return Paint::parse(json.asString());
		}
	}

	LOGE << "Cannot parse paint recipe '" << Writer::write(json) << "'";
	DEBUG_ASSERT(false);
	return Color::broken();
}

Json::Value Paint::toJson() const
{
	if (_length == 0)
	{
		return Json::nullValue;
	}

	std::array<std::string, MAX_STEPS> stack;
	size_t stacksize = 0;

	for (size_t i = 0; i < _length; i++)
	{
		DEBUG_ASSERT(stacksize < MAX_STEPS);
		const Step& step = _steps[i];
		switch (step.type)
		{
			case Step::Type::RAW:
			{
				if (step.data)
				{
					stack[stacksize] = step.data.toString();
				}
				else
				{
					stack[stacksize] = "";
				}
				stacksize += 1;
			}
			break;
			case Step::Type::NAMED:
			{
				stack[stacksize] = ::stringify(step.name);
				stacksize += 1;
			}
			break;
			case Step::Type::MIX:
			{
				size_t args = std::min(stacksize, (size_t) 2);
				std::stringstream strm;
				strm << "mix(";
				for (size_t j = stacksize - args;
					j < stacksize; j++)
				{
					strm << stack[j];
					if (j + 1 < stacksize)
					{
						strm << ",";
					}
				}
				int percent = int(100 * step.ratio + 0.5f);
				if (percent == 100)
				{
					strm << ")";
				}
				else
				{
					strm << "," << percent << '%' << ")";
				}
				stacksize -= args;
				stack[stacksize] = strm.str();
				stacksize += 1;
			}
			break;
			case Step::Type::BLEND:
			{
				size_t args = std::min(stacksize, (size_t) 2);
				std::stringstream strm;
				strm << "blend(";
				for (size_t j = stacksize - args;
					j < stacksize; j++)
				{
					strm << stack[j];
					if (j + 1 < stacksize)
					{
						strm << ",";
					}
				}
				int percent = int(100 * step.ratio + 0.5f);
				if (percent == 100)
				{
					strm << ")";
				}
				else
				{
					strm << "," << percent << '%' << ")";
				}
				stacksize -= args;
				stack[stacksize] = strm.str();
				stacksize += 1;
			}
			break;
			case Step::Type::ALPHA:
			{
				size_t args = std::min(stacksize, (size_t) 1);
				std::stringstream strm;
				strm << "alpha(";
				for (size_t j = stacksize - args;
					j < stacksize; j++)
				{
					strm << stack[j] << ",";
				}
				strm << std::hex << std::setfill('0')
					<< std::setw(2) << int(step.value)
					<< std::dec << ")";
				stacksize -= args;
				stack[stacksize] = strm.str();
				stacksize += 1;
			}
			break;
		}
	}

	if (stacksize > 1)
	{
		for (size_t i = 1; i < stacksize; i++)
		{
			stack[0] += ' ';
			stack[0] += stack[i];
		}
		stacksize = 1;
	}
	DEBUG_ASSERT(stacksize == 1);
	return stack[0];
}
