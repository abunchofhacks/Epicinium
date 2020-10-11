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
#include "order.hpp"
#include "source.hpp"

#include "parseerror.hpp"
#include "move.hpp"
#include "tiletype.hpp"
#include "unittype.hpp"


Order::Type Order::parseType(const std::string& str)
{
	if      (str == "move")    return Order::Type::MOVE;
	else if (str == "guard")   return Order::Type::GUARD;
	else if (str == "focus")   return Order::Type::FOCUS;
	else if (str == "lockdown") return Order::Type::LOCKDOWN;
	else if (str == "shell")   return Order::Type::SHELL;
	else if (str == "bombard") return Order::Type::BOMBARD;
	else if (str == "bomb")    return Order::Type::BOMB;
	else if (str == "capture") return Order::Type::CAPTURE;
	else if (str == "shape")   return Order::Type::SHAPE;
	else if (str == "settle")  return Order::Type::SETTLE;
	else if (str == "expand")  return Order::Type::EXPAND;
	else if (str == "build")   return Order::Type::EXPAND; // backwards compatiblity
	else if (str == "upgrade") return Order::Type::UPGRADE;
	else if (str == "cultivate") return Order::Type::CULTIVATE;
	else if (str == "produce") return Order::Type::PRODUCE;
	else if (str == "halt")    return Order::Type::HALT;
	else if (str == "none")    return Order::Type::NONE;
	else throw ParseError("Unknown order type '" + str + "'");
}

const char* Order::stringify(const Order::Type& type)
{
	switch (type)
	{
		case Order::Type::MOVE:      return "move";
		case Order::Type::GUARD:     return "guard";
		case Order::Type::FOCUS:     return "focus";
		case Order::Type::LOCKDOWN:  return "lockdown";
		case Order::Type::SHELL:     return "shell";
		case Order::Type::BOMBARD:   return "bombard";
		case Order::Type::BOMB:      return "bomb";
		case Order::Type::CAPTURE:   return "capture";
		case Order::Type::SHAPE:     return "shape";
		case Order::Type::SETTLE:    return "settle";
		case Order::Type::EXPAND:    return "expand";
		case Order::Type::UPGRADE:   return "upgrade";
		case Order::Type::CULTIVATE: return "cultivate";
		case Order::Type::PRODUCE:   return "produce";
		case Order::Type::HALT:      return "halt";
		case Order::Type::NONE:      return "none";
	}
	return "error";
}

Order::Order(const TypeNamer& namer, const Json::Value& json) :
	type(parseType(json["type"].asString()))
{
	switch (type)
	{
		case Order::Type::MOVE:
		{
			subject = Descriptor(json["subject"]);
			target = Descriptor(json["target"]);
			moves.reserve(json["moves"].size());
			for (const Json::Value& movejson : json["moves"])
			{
				moves.emplace_back(parseMove(movejson.asString()));
			}
		}
		break;

		case Order::Type::GUARD:
		{
			subject = Descriptor(json["subject"]);
			target = Descriptor(json["target"]);
		}
		break;

		case Order::Type::FOCUS:
		{
			subject = Descriptor(json["subject"]);
			target = Descriptor(json["target"]);
		}
		break;

		case Order::Type::LOCKDOWN:
		{
			subject = Descriptor(json["subject"]);
			target = Descriptor(json["target"]);
		}
		break;

		case Order::Type::SHELL:
		{
			subject = Descriptor(json["subject"]);
			target = Descriptor(json["target"]);
		}
		break;

		case Order::Type::BOMBARD:
		{
			subject = Descriptor(json["subject"]);
			target = Descriptor(json["target"]);
		}
		break;

		case Order::Type::BOMB:
		{
			subject = Descriptor(json["subject"]);
		}
		break;

		case Order::Type::CAPTURE:
		{
			subject = Descriptor(json["subject"]);
		}
		break;

		case Order::Type::SHAPE:
		{
			subject = Descriptor(json["subject"]);
			tiletype = parseTileType(namer, json["tiletype"].asString());
		}
		break;

		case Order::Type::SETTLE:
		{
			subject = Descriptor(json["subject"]);
			tiletype = parseTileType(namer, json["tiletype"].asString());
		}
		break;

		case Order::Type::EXPAND:
		{
			subject = Descriptor(json["subject"]);
			target = Descriptor(json["target"]);
			tiletype = parseTileType(namer, json["tiletype"].asString());
		}
		break;

		case Order::Type::UPGRADE:
		{
			subject = Descriptor(json["subject"]);
			tiletype = parseTileType(namer, json["tiletype"].asString());
		}
		break;

		case Order::Type::CULTIVATE:
		{
			subject = Descriptor(json["subject"]);
			tiletype = parseTileType(namer, json["tiletype"].asString());
		}
		break;

		case Order::Type::PRODUCE:
		{
			subject = Descriptor(json["subject"]);
			if (json["target"].isNull())
			{
				target = Descriptor::cell(subject.position);
			}
			else target = Descriptor(json["target"]);
			unittype = parseUnitType(namer, json["unittype"].asString());
		}
		break;

		case Order::Type::HALT:
		{
			subject = Descriptor(json["subject"]);
		}
		break;

		case Order::Type::NONE:
		break;
	}
}

std::ostream& operator<<(std::ostream& os, const Order::Type& type)
{
	return os << Order::stringify(type);
}

std::ostream& operator<<(std::ostream& os, const Order& order)
{
	os << "{\"type\":\"" << Order::stringify(order.type) << "\"";

	switch (order.type)
	{
		case Order::Type::MOVE:
		{
			os << ",\"subject\":" << order.subject;
			os << ",\"target\":" << order.target;

			os << ",\"moves\":[";
			bool empty = true;
			for (const Move& move : order.moves)
			{
				if (empty) empty = false;
				else os << ",";
				os << "\"" << move << "\"";
			}
			os << "]";
		}
		break;

		case Order::Type::GUARD:
		{
			os << ",\"subject\":" << order.subject;
			os << ",\"target\":" << order.target;
		}
		break;

		case Order::Type::FOCUS:
		{
			os << ",\"subject\":" << order.subject;
			os << ",\"target\":" << order.target;
		}
		break;

		case Order::Type::LOCKDOWN:
		{
			os << ",\"subject\":" << order.subject;
			os << ",\"target\":" << order.target;
		}
		break;

		case Order::Type::SHELL:
		{
			os << ",\"subject\":" << order.subject;
			os << ",\"target\":" << order.target;
		}
		break;

		case Order::Type::BOMBARD:
		{
			os << ",\"subject\":" << order.subject;
			os << ",\"target\":" << order.target;
		}
		break;

		case Order::Type::BOMB:
		{
			os << ",\"subject\":" << order.subject;
		}
		break;

		case Order::Type::CAPTURE:
		{
			os << ",\"subject\":" << order.subject;
		}
		break;

		case Order::Type::SHAPE:
		{
			os << ",\"subject\":" << order.subject;
			os << ",\"tiletype\":\"" << order.tiletype << "\"";
		}
		break;

		case Order::Type::SETTLE:
		{
			os << ",\"subject\":" << order.subject;
			os << ",\"tiletype\":\"" << order.tiletype << "\"";
		}
		break;

		case Order::Type::EXPAND:
		{
			os << ",\"subject\":" << order.subject;
			os << ",\"target\":" << order.target;
			os << ",\"tiletype\":\"" << order.tiletype << "\"";
		}
		break;

		case Order::Type::UPGRADE:
		{
			os << ",\"subject\":" << order.subject;
			os << ",\"tiletype\":\"" << order.tiletype << "\"";
		}
		break;

		case Order::Type::CULTIVATE:
		{
			os << ",\"subject\":" << order.subject;
			os << ",\"tiletype\":\"" << order.tiletype << "\"";
		}
		break;

		case Order::Type::PRODUCE:
		{
			os << ",\"subject\":" << order.subject;
			os << ",\"target\":" << order.target;
			os << ",\"unittype\":\"" << order.unittype << "\"";
		}
		break;

		case Order::Type::HALT:
		{
			os << ",\"subject\":" << order.subject;
		}
		break;

		case Order::Type::NONE:
		break;
	}

	return os << "}";
}

bool Order::operator==(const Order& other) const
{
	if (type != other.type) return false;
	if (subject != other.subject) return false;
	if (target != other.target) return false;

	switch (type)
	{
		case Order::Type::MOVE:
		{
			if (moves != other.moves) return false;
		}
		break;

		case Order::Type::GUARD:
		case Order::Type::FOCUS:
		case Order::Type::LOCKDOWN:
		case Order::Type::SHELL:
		case Order::Type::BOMBARD:
		case Order::Type::BOMB:
		case Order::Type::CAPTURE:
		break;

		case Order::Type::SHAPE:
		case Order::Type::SETTLE:
		case Order::Type::EXPAND:
		case Order::Type::UPGRADE:
		case Order::Type::CULTIVATE:
		{
			if (tiletype != other.tiletype) return false;
		}
		break;

		case Order::Type::PRODUCE:
		{
			if (unittype != other.unittype) return false;
		}
		break;

		case Order::Type::HALT:
		break;

		case Order::Type::NONE:
		break;
	}

	return true;
}

bool Order::operator!=(const Order& other) const
{
	return !(*this == other);
}

std::vector<Order> Order::parseOrders(const TypeNamer& typenamer,
		const Json::Value& json)
{
	if (!json.isArray())
	{
		LOGE << "Error while parsing orders: "
			<< "\'" << json << "\'"
			<< ", not an array";
		DEBUG_ASSERT(false);
		return {};
	}

	std::vector<Order> orders;
	try
	{
		for (auto& order : json)
		{
			orders.emplace_back(typenamer, order);
		}
	}
	catch (const ParseError& error)
	{
		LOGE << "Error while parsing orders: "
			<< "\'" << json << "\'"
			<< ", error "
			<< error.what();
		RETHROW_IF_DEV();
	}
	catch (const Json::Exception& error)
	{
		LOGE << "Error while parsing orders: "
			<< "\'" << json << "\'"
			<< ", error "
			<< error.what();
		RETHROW_IF_DEV();
	}
	return orders;
}

std::vector<Order> Order::parseOrders(const TypeNamer& typenamer,
		const std::string& str)
{
	Json::Value json;
	try
	{
		Json::Reader reader;
		if (!reader.parse(str, json))
		{
			LOGE << "Error while parsing orders: "
				<< "\'" << json << "\'"
				<< ", error "
				<< reader.getFormattedErrorMessages();
			DEBUG_ASSERT(false);
			return {};
		}
	}
	catch (const Json::Exception& error)
	{
		LOGE << "Error while parsing orders: "
			<< "\'" << json << "\'"
			<< ", error "
			<< error.what();
		RETHROW_IF_DEV();
		return {};
	}

	return parseOrders(typenamer, json);
}
