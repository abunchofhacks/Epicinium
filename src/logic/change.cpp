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
#include "change.hpp"
#include "source.hpp"

#include "parseerror.hpp"
#include "player.hpp"
#include "cycle.hpp"
#include "notice.hpp"


Change::Type Change::parseType(const std::string& str)
{
	if      (str == "none")        return Change::Type::NONE;
	else if (str == "starts")      return Change::Type::STARTS;
	else if (str == "moves")       return Change::Type::MOVES;
	else if (str == "reveal")      return Change::Type::REVEAL;
	else if (str == "obscure")     return Change::Type::OBSCURE;
	else if (str == "transformed") return Change::Type::TRANSFORMED;
	else if (str == "consumed")    return Change::Type::CONSUMED;
	else if (str == "shapes")      return Change::Type::SHAPES;
	else if (str == "shaped")      return Change::Type::SHAPED;
	else if (str == "settles")     return Change::Type::SETTLES;
	else if (str == "settled")     return Change::Type::SETTLED;
	else if (str == "expands")     return Change::Type::EXPANDS;
	else if (str == "expanded")    return Change::Type::EXPANDED;
	else if (str == "upgrades")    return Change::Type::UPGRADES;
	else if (str == "upgraded")    return Change::Type::UPGRADED;
	else if (str == "cultivates")  return Change::Type::CULTIVATES;
	else if (str == "cultivated")  return Change::Type::CULTIVATED;
	else if (str == "captures")    return Change::Type::CAPTURES;
	else if (str == "captured")    return Change::Type::CAPTURED;
	else if (str == "produces")    return Change::Type::PRODUCES;
	else if (str == "produced")    return Change::Type::PRODUCED;
	else if (str == "entered")     return Change::Type::ENTERED;
	else if (str == "exited")      return Change::Type::EXITED;
	else if (str == "died")        return Change::Type::DIED;
	else if (str == "destroyed")   return Change::Type::DESTROYED;
	else if (str == "survived")    return Change::Type::SURVIVED;
	else if (str == "aims")        return Change::Type::AIMS;
	else if (str == "attacks")     return Change::Type::ATTACKS;
	else if (str == "attacked")    return Change::Type::ATTACKED;
	else if (str == "tramples")    return Change::Type::TRAMPLES;
	else if (str == "trampled")    return Change::Type::TRAMPLED;
	else if (str == "shells")      return Change::Type::SHELLS;
	else if (str == "shelled")     return Change::Type::SHELLED;
	else if (str == "bombards")    return Change::Type::BOMBARDS;
	else if (str == "bombarded")   return Change::Type::BOMBARDED;
	else if (str == "bombs")       return Change::Type::BOMBS;
	else if (str == "bombed")      return Change::Type::BOMBED;
	else if (str == "frostbitten") return Change::Type::FROSTBITTEN;
	else if (str == "burned")      return Change::Type::BURNED;
	else if (str == "gassed")      return Change::Type::GASSED;
	else if (str == "irradiated")  return Change::Type::IRRADIATED;
	else if (str == "grows")       return Change::Type::GROWS;
	else if (str == "snow")        return Change::Type::SNOW;
	else if (str == "frostbite")   return Change::Type::FROSTBITE;
	else if (str == "firestorm")   return Change::Type::FIRESTORM;
	else if (str == "bonedrought") return Change::Type::BONEDROUGHT;
	else if (str == "death")       return Change::Type::DEATH;
	else if (str == "gas")         return Change::Type::GAS;
	else if (str == "radiation")   return Change::Type::RADIATION;
	else if (str == "temperature") return Change::Type::TEMPERATURE;
	else if (str == "humidity")    return Change::Type::HUMIDITY;
	else if (str == "chaos")       return Change::Type::CHAOS;
	else if (str == "chaosreport") return Change::Type::CHAOSREPORT;
	else if (str == "year")        return Change::Type::YEAR;
	else if (str == "season")      return Change::Type::SEASON;
	else if (str == "daytime")     return Change::Type::DAYTIME;
	else if (str == "phase")       return Change::Type::PHASE;
	else if (str == "initiative")  return Change::Type::INITIATIVE;
	else if (str == "funds")       return Change::Type::FUNDS;
	else if (str == "income")      return Change::Type::INCOME;
	else if (str == "expenditure") return Change::Type::EXPENDITURE;
	else if (str == "sleeping")    return Change::Type::SLEEPING;
	else if (str == "acting")      return Change::Type::ACTING;
	else if (str == "finished")    return Change::Type::FINISHED;
	else if (str == "discarded")   return Change::Type::DISCARDED;
	else if (str == "postponed")   return Change::Type::POSTPONED;
	else if (str == "unfinished")  return Change::Type::UNFINISHED;
	else if (str == "ordered")     return Change::Type::ORDERED;
	else if (str == "vision")      return Change::Type::VISION;
	else if (str == "corner")      return Change::Type::CORNER;
	else if (str == "border")      return Change::Type::BORDER;
	else if (str == "scored")      return Change::Type::SCORED;
	else if (str == "defeat")      return Change::Type::DEFEAT;
	else if (str == "victory")     return Change::Type::VICTORY;
	else if (str == "gameover")    return Change::Type::GAMEOVER;
	else if (str == "award")       return Change::Type::AWARD;
	else throw ParseError("Unknown change type '" + str + "'");
}

const char* Change::stringify(const Change::Type& type)
{
	switch (type)
	{
		case Change::Type::NONE:        return "none";
		case Change::Type::STARTS:      return "starts";
		case Change::Type::MOVES:       return "moves";
		case Change::Type::REVEAL:      return "reveal";
		case Change::Type::OBSCURE:     return "obscure";
		case Change::Type::TRANSFORMED: return "transformed";
		case Change::Type::CONSUMED:    return "consumed";
		case Change::Type::SHAPES:      return "shapes";
		case Change::Type::SHAPED:      return "shaped";
		case Change::Type::SETTLES:     return "settles";
		case Change::Type::SETTLED:     return "settled";
		case Change::Type::EXPANDS:     return "expands";
		case Change::Type::EXPANDED:    return "expanded";
		case Change::Type::UPGRADES:    return "upgrades";
		case Change::Type::UPGRADED:    return "upgraded";
		case Change::Type::CULTIVATES:  return "cultivates";
		case Change::Type::CULTIVATED:  return "cultivated";
		case Change::Type::CAPTURES:    return "captures";
		case Change::Type::CAPTURED:    return "captured";
		case Change::Type::PRODUCES:    return "produces";
		case Change::Type::PRODUCED:    return "produced";
		case Change::Type::ENTERED:     return "entered";
		case Change::Type::EXITED:      return "exited";
		case Change::Type::DIED:        return "died";
		case Change::Type::DESTROYED:   return "destroyed";
		case Change::Type::SURVIVED:    return "survived";
		case Change::Type::AIMS:        return "aims";
		case Change::Type::ATTACKS:     return "attacks";
		case Change::Type::ATTACKED:    return "attacked";
		case Change::Type::TRAMPLES:    return "tramples";
		case Change::Type::TRAMPLED:    return "trampled";
		case Change::Type::SHELLS:      return "shells";
		case Change::Type::SHELLED:     return "shelled";
		case Change::Type::BOMBARDS:    return "bombards";
		case Change::Type::BOMBARDED:   return "bombarded";
		case Change::Type::BOMBS:       return "bombs";
		case Change::Type::BOMBED:      return "bombed";
		case Change::Type::FROSTBITTEN: return "frostbitten";
		case Change::Type::BURNED:      return "burned";
		case Change::Type::GASSED:      return "gassed";
		case Change::Type::IRRADIATED:  return "irradiated";
		case Change::Type::GROWS:       return "grows";
		case Change::Type::SNOW:        return "snow";
		case Change::Type::FROSTBITE:   return "frostbite";
		case Change::Type::FIRESTORM:   return "firestorm";
		case Change::Type::BONEDROUGHT: return "bonedrought";
		case Change::Type::DEATH:       return "death";
		case Change::Type::GAS:         return "gas";
		case Change::Type::RADIATION:   return "radiation";
		case Change::Type::TEMPERATURE: return "temperature";
		case Change::Type::HUMIDITY:    return "humidity";
		case Change::Type::CHAOS:       return "chaos";
		case Change::Type::CHAOSREPORT: return "chaosreport";
		case Change::Type::YEAR:        return "year";
		case Change::Type::SEASON:      return "season";
		case Change::Type::DAYTIME:     return "daytime";
		case Change::Type::PHASE:       return "phase";
		case Change::Type::INITIATIVE:  return "initiative";
		case Change::Type::FUNDS:       return "funds";
		case Change::Type::INCOME:      return "income";
		case Change::Type::EXPENDITURE: return "expenditure";
		case Change::Type::SLEEPING:    return "sleeping";
		case Change::Type::ACTING:      return "acting";
		case Change::Type::FINISHED:    return "finished";
		case Change::Type::DISCARDED:   return "discarded";
		case Change::Type::POSTPONED:   return "postponed";
		case Change::Type::UNFINISHED:  return "unfinished";
		case Change::Type::ORDERED:     return "ordered";
		case Change::Type::VISION:      return "vision";
		case Change::Type::CORNER:      return "corner";
		case Change::Type::BORDER:      return "border";
		case Change::Type::SCORED:      return "scored";
		case Change::Type::DEFEAT:      return "defeat";
		case Change::Type::VICTORY:     return "victory";
		case Change::Type::GAMEOVER:    return "gameover";
		case Change::Type::AWARD:       return "award";
	}
	return "error";
}

Change::Change(const TypeNamer& namer, const Json::Value& json) :
	type(parseType(json["type"].asString()))
{
	switch (type)
	{
		case Change::Type::NONE:
		{
			subject = Descriptor(json["subject"]);
			if (!json["target"].isNull()) target = Descriptor(json["target"]);
			else target = Descriptor();
			if (!json["notice"].isNull()) notice = parseNotice(json["notice"].asString());
			else notice = Notice::NONE;
		}
		break;

		case Change::Type::STARTS:
		{
			subject = Descriptor(json["subject"]);
			target = Descriptor(json["target"]);
		}
		break;

		case Change::Type::MOVES:
		{
			subject = Descriptor(json["subject"]);
			target = Descriptor(json["target"]);
		}
		break;

		case Change::Type::REVEAL:
		{
			subject = Descriptor(json["subject"]);
			tile = TileToken(namer, json["tile"]);
			snow = !json["snow"].isNull() ? json["snow"].asBool() : false;
			frostbite = !json["frostbite"].isNull() ? json["frostbite"].asBool() : false;
			firestorm = !json["firestorm"].isNull() ? json["firestorm"].asBool() : false;
			bonedrought = !json["bonedrought"].isNull() ? json["bonedrought"].asBool() : false;
			death = !json["death"].isNull() ? json["death"].asBool() : false;
			gas = !json["gas"].isNull() ? json["gas"].asInt() : 0;
			radiation = !json["radiation"].isNull() ? json["radiation"].asInt() : 0;
			temperature = !json["temperature"].isNull() ? json["temperature"].asInt() : 0;
			humidity = !json["humidity"].isNull() ? json["humidity"].asInt() : 0;
			chaos = !json["chaos"].isNull() ? json["chaos"].asInt() : 0;
		}
		break;

		case Change::Type::OBSCURE:
		{
			subject = Descriptor(json["subject"]);
		}
		break;

		case Change::Type::TRANSFORMED:
		{
			subject = Descriptor(json["subject"]);
			tile = TileToken(namer, json["tile"]);
		}
		break;

		case Change::Type::CONSUMED:
		{
			subject = Descriptor(json["subject"]);
			tile = TileToken(namer, json["tile"]);
		}
		break;

		case Change::Type::SHAPES:
		{
			subject = Descriptor(json["subject"]);
		}
		break;

		case Change::Type::SHAPED:
		{
			subject = Descriptor(json["subject"]);
			tile = TileToken(namer, json["tile"]);
		}
		break;

		case Change::Type::SETTLES:
		{
			subject = Descriptor(json["subject"]);
		}
		break;

		case Change::Type::SETTLED:
		{
			subject = Descriptor(json["subject"]);
			tile = TileToken(namer, json["tile"]);
		}
		break;

		case Change::Type::EXPANDS:
		{
			subject = Descriptor(json["subject"]);
			target = Descriptor(json["target"]);
			power = json["power"].asInt();
		}
		break;

		case Change::Type::EXPANDED:
		{
			subject = Descriptor(json["subject"]);
			tile = TileToken(namer, json["tile"]);
		}
		break;

		case Change::Type::UPGRADES:
		{
			subject = Descriptor(json["subject"]);
			power = json["power"].asInt();
		}
		break;

		case Change::Type::UPGRADED:
		{
			subject = Descriptor(json["subject"]);
			tile = TileToken(namer, json["tile"]);
		}
		break;

		case Change::Type::CULTIVATES:
		{
			subject = Descriptor(json["subject"]);
			power = json["power"].asInt();
		}
		break;

		case Change::Type::CULTIVATED:
		{
			subject = Descriptor(json["subject"]);
			tile = TileToken(namer, json["tile"]);
		}
		break;

		case Change::Type::CAPTURES:
		{
			subject = Descriptor(json["subject"]);
		}
		break;

		case Change::Type::CAPTURED:
		{
			subject = Descriptor(json["subject"]);
			player = ::parsePlayer(json["player"].asString());
		}
		break;

		case Change::Type::PRODUCES:
		{
			subject = Descriptor(json["subject"]);
			power = json["power"].asInt();
		}
		break;

		case Change::Type::PRODUCED:
		{
			subject = Descriptor(json["subject"]);
			unit = UnitToken(namer, json["unit"]);
		}
		break;

		case Change::Type::ENTERED:
		{
			subject = Descriptor(json["subject"]);
			unit = UnitToken(namer, json["unit"]);
		}
		break;

		case Change::Type::EXITED:
		{
			subject = Descriptor(json["subject"]);
		}
		break;

		case Change::Type::DIED:
		{
			subject = Descriptor(json["subject"]);
		}
		break;

		case Change::Type::DESTROYED:
		{
			subject = Descriptor(json["subject"]);
			tile = TileToken(namer, json["tile"]);
		}
		break;

		case Change::Type::SURVIVED:
		{
			subject = Descriptor(json["subject"]);
		}
		break;

		case Change::Type::AIMS:
		{
			subject = Descriptor(json["subject"]);
			target = Descriptor(json["target"]);
			if (!json["notice"].isNull()) notice = parseNotice(json["notice"].asString());
			else notice = Notice::NONE;
		}
		break;

		case Change::Type::ATTACKS:
		{
			subject = Descriptor(json["subject"]);
			target = Descriptor(json["target"]);
			figure = json["figure"].asInt();
		}
		break;

		case Change::Type::ATTACKED:
		{
			subject = Descriptor(json["subject"]);
			attacker = Attacker(namer, json["attacker"]);
			figure = json["figure"].asInt();
			parseDeadPoweredKilledDepowered(json);
		}
		break;

		case Change::Type::TRAMPLES:
		{
			subject = Descriptor(json["subject"]);
			figure = json["figure"].asInt();
		}
		break;

		case Change::Type::TRAMPLED:
		{
			subject = Descriptor(json["subject"]);
			bombarder = Bombarder(namer, json["bombarder"]);
			figure = json["figure"].asInt();
			parseDeadPoweredKilledDepowered(json);
		}
		break;

		case Change::Type::SHELLS:
		{
			subject = Descriptor(json["subject"]);
			target = Descriptor(json["target"]);
			figure = json["figure"].asInt();
		}
		break;

		case Change::Type::SHELLED:
		{
			subject = Descriptor(json["subject"]);
			attacker = Attacker(namer, json["attacker"]);
			figure = json["figure"].asInt();
			parseDeadPoweredKilledDepowered(json);
		}
		break;

		case Change::Type::BOMBARDS:
		{
			subject = Descriptor(json["subject"]);
			figure = json["figure"].asInt();
		}
		break;

		case Change::Type::BOMBARDED:
		{
			subject = Descriptor(json["subject"]);
			bombarder = Bombarder(namer, json["bombarder"]);
			figure = json["figure"].asInt();
			parseDeadPoweredKilledDepowered(json);
		}
		break;

		case Change::Type::BOMBS:
		{
			subject = Descriptor(json["subject"]);
			figure = json["figure"].asInt();
		}
		break;

		case Change::Type::BOMBED:
		{
			subject = Descriptor(json["subject"]);
			bombarder = Bombarder(namer, json["bombarder"]);
			figure = json["figure"].asInt();
			parseDeadPoweredKilledDepowered(json);
		}
		break;

		case Change::Type::FROSTBITTEN:
		{
			subject = Descriptor(json["subject"]);
			figure = json["figure"].asInt();
			parseDeadPoweredKilledDepowered(json);
		}
		break;

		case Change::Type::BURNED:
		{
			subject = Descriptor(json["subject"]);
			figure = json["figure"].asInt();
			parseDeadPoweredKilledDepowered(json);
		}
		break;

		case Change::Type::GASSED:
		{
			subject = Descriptor(json["subject"]);
			figure = json["figure"].asInt();
			parseDeadPoweredKilledDepowered(json);
		}
		break;

		case Change::Type::IRRADIATED:
		{
			subject = Descriptor(json["subject"]);
			figure = json["figure"].asInt();
			parseDeadPoweredKilledDepowered(json);
		}
		break;

		case Change::Type::GROWS:
		{
			subject = Descriptor(json["subject"]);
			stacks = json["stacks"].asInt();
			power = json["power"].asInt();
		}
		break;

		case Change::Type::SNOW:
		{
			subject = Descriptor(json["subject"]);
			snow = json["snow"].asBool();
		}
		break;

		case Change::Type::FROSTBITE:
		{
			subject = Descriptor(json["subject"]);
			frostbite = json["frostbite"].asBool();
		}
		break;

		case Change::Type::FIRESTORM:
		{
			subject = Descriptor(json["subject"]);
			firestorm = json["firestorm"].asBool();
		}
		break;

		case Change::Type::BONEDROUGHT:
		{
			subject = Descriptor(json["subject"]);
			bonedrought = json["bonedrought"].asBool();
		}
		break;

		case Change::Type::DEATH:
		{
			subject = Descriptor(json["subject"]);
			death = json["death"].asBool();
		}
		break;

		case Change::Type::GAS:
		{
			subject = Descriptor(json["subject"]);
			gas = json["gas"].asInt();
		}
		break;

		case Change::Type::RADIATION:
		{
			subject = Descriptor(json["subject"]);
			radiation = json["radiation"].asInt();
		}
		break;

		case Change::Type::TEMPERATURE:
		{
			subject = Descriptor(json["subject"]);
			temperature = json["temperature"].asInt();
		}
		break;

		case Change::Type::HUMIDITY:
		{
			subject = Descriptor(json["subject"]);
			humidity = json["humidity"].asInt();
		}
		break;

		case Change::Type::CHAOS:
		{
			subject = Descriptor(json["subject"]);
			chaos = json["chaos"].asInt();
		}
		break;

		case Change::Type::CHAOSREPORT:
		{
			level = json["level"].asInt();
		}
		break;

		case Change::Type::YEAR:
		{
			year = json["year"].asInt();
		}
		break;

		case Change::Type::SEASON:
		{
			season = ::parseSeason(json["season"].asString());
		}
		break;

		case Change::Type::DAYTIME:
		{
			daytime = ::parseDaytime(json["daytime"].asString());
		}
		break;

		case Change::Type::PHASE:
		{
			phase = ::parsePhase(json["phase"].asString());
		}
		break;

		case Change::Type::INITIATIVE:
		{
			player = !json["player"].isNull()
				? ::parsePlayer(json["player"].asString())
				: Player::NONE;
			initiative = json["initiative"].asInt();
		}
		break;

		case Change::Type::FUNDS:
		{
			player = !json["player"].isNull()
				? ::parsePlayer(json["player"].asString())
				: Player::NONE;
			money = !json["money"].isNull()
				? json["money"].asInt()
				: 0;
		}
		break;

		case Change::Type::INCOME:
		{
			subject = Descriptor(json["subject"]);
			player = !json["player"].isNull()
				? ::parsePlayer(json["player"].asString())
				: Player::NONE;
			money = !json["money"].isNull()
				? json["money"].asInt()
				: 0;
		}
		break;

		case Change::Type::EXPENDITURE:
		{
			subject = Descriptor(json["subject"]);
			player = !json["player"].isNull()
				? ::parsePlayer(json["player"].asString())
				: Player::NONE;
			money = !json["money"].isNull()
				? json["money"].asInt()
				: 0;
		}
		break;

		case Change::Type::SLEEPING:
		{
			player = !json["player"].isNull()
				? ::parsePlayer(json["player"].asString())
				: Player::NONE;
		}
		break;

		case Change::Type::ACTING:
		{
			subject = Descriptor(json["subject"]);
		}
		break;

		case Change::Type::FINISHED:
		{
			subject = Descriptor(json["subject"]);
		}
		break;

		case Change::Type::DISCARDED:
		{
			subject = Descriptor(json["subject"]);
		}
		break;

		case Change::Type::POSTPONED:
		{
			subject = Descriptor(json["subject"]);
			player = !json["player"].isNull()
				? ::parsePlayer(json["player"].asString())
				: Player::NONE;
			order = Order(namer, json["order"]);
		}
		break;

		case Change::Type::UNFINISHED:
		{
			subject = Descriptor(json["subject"]);
			player = !json["player"].isNull()
				? ::parsePlayer(json["player"].asString())
				: Player::NONE;
			order = Order(namer, json["order"]);
		}
		break;

		case Change::Type::ORDERED:
		{
			subject = Descriptor(json["subject"]);
			player = !json["player"].isNull()
				? ::parsePlayer(json["player"].asString())
				: Player::NONE;
			order = Order(namer, json["order"]);
		}
		break;

		case Change::Type::VISION:
		{
			subject = Descriptor(json["subject"]);
			vision = Vision(json["vision"]);
		}
		break;

		case Change::Type::CORNER:
		{
			subject = Descriptor(json["subject"]);
		}
		break;

		case Change::Type::BORDER:
		break;

		case Change::Type::SCORED:
		{
			subject = Descriptor(json["subject"]);
			player = ::parsePlayer(json["player"].asString());
			score = json["score"].asInt();
		}
		break;

		case Change::Type::DEFEAT:
		{
			player = ::parsePlayer(json["player"].asString());
			score = json["score"].asInt();
			if (!json["notice"].isNull()) notice = parseNotice(json["notice"].asString());
			else notice = Notice::NONE;
		}
		break;

		case Change::Type::VICTORY:
		{
			player = ::parsePlayer(json["player"].asString());
			score = json["score"].asInt();
		}
		break;

		case Change::Type::GAMEOVER:
		{
			score = json["score"].asInt();
		}
		break;

		case Change::Type::AWARD:
		{
			player = ::parsePlayer(json["player"].asString());
			level = json["level"].asInt();
		}
		break;
	}
}

void Change::parseDeadPoweredKilledDepowered(const Json::Value& json)
{
	// Backwards compatibility: v0.1.0
	if (!json["dead"].isNull())
	{
		bool powered = (!json["powered"].isNull() && json["powered"].asBool());
		bool dead = json["dead"].asBool();

		if (type == Change::Type::ATTACKED
			&& subject.type != Descriptor::Type::TILE)
		{
			killed = false;
			depowered = dead && powered;
		}
		else
		{
			killed = dead;
			depowered = dead && powered;
		}

		return;
	}

	killed = (!json["killed"].isNull() && json["killed"].asBool());
	depowered = (!json["depowered"].isNull() && json["depowered"].asBool());
}

std::ostream& operator<<(std::ostream& os, const Change::Type& type)
{
	return os << Change::stringify(type);
}

std::ostream& operator<<(std::ostream& os, const Change& change)
{
	os << "{\"type\":\"" << Change::stringify(change.type) << "\"";

	switch (change.type)
	{
		case Change::Type::NONE:
		{
			os << ",\"subject\":" << change.subject;
			if (change.target.type != Descriptor::Type::NONE)
			{
				os << ",\"target\":" << change.target;
			}
			if (change.notice != Notice::NONE)
			{
				os << ",\"notice\":\"" << change.notice << "\"";
			}
		}
		break;

		case Change::Type::STARTS:
		{
			os << ",\"subject\":" << change.subject;
			os << ",\"target\":" << change.target;
		}
		break;

		case Change::Type::MOVES:
		{
			os << ",\"subject\":" << change.subject;
			os << ",\"target\":" << change.target;
		}
		break;

		case Change::Type::REVEAL:
		{
			os << ",\"subject\":" << change.subject;
			os << ",\"tile\":" << change.tile;
			if (change.snow != false)
			{
				os << ",\"snow\":" << ::jsonify(change.snow);
			}
			if (change.frostbite != false)
			{
				os << ",\"frostbite\":" << ::jsonify(change.frostbite);
			}
			if (change.firestorm != false)
			{
				os << ",\"firestorm\":" << ::jsonify(change.firestorm);
			}
			if (change.bonedrought != false)
			{
				os << ",\"bonedrought\":" << ::jsonify(change.bonedrought);
			}
			if (change.death != false)
			{
				os << ",\"death\":" << ::jsonify(change.death);
			}
			if (change.gas != 0)
			{
				os << ",\"gas\":" << ((int) change.gas);
			}
			if (change.radiation != 0)
			{
				os << ",\"radiation\":" << ((int) change.radiation);
			}
			if (change.temperature != 0)
			{
				os << ",\"temperature\":" << ((int) change.temperature);
			}
			if (change.humidity != 0)
			{
				os << ",\"humidity\":" << ((int) change.humidity);
			}
			if (change.chaos != 0)
			{
				os << ",\"chaos\":" << ((int) change.chaos);
			}
		}
		break;

		case Change::Type::OBSCURE:
		{
			os << ",\"subject\":" << change.subject;
		}
		break;

		case Change::Type::TRANSFORMED:
		{
			os << ",\"subject\":" << change.subject;
			os << ",\"tile\":" << change.tile;
		}
		break;

		case Change::Type::CONSUMED:
		{
			os << ",\"subject\":" << change.subject;
			os << ",\"tile\":" << change.tile;
		}
		break;

		case Change::Type::SHAPES:
		{
			os << ",\"subject\":" << change.subject;
		}
		break;

		case Change::Type::SHAPED:
		{
			os << ",\"subject\":" << change.subject;
			os << ",\"tile\":" << change.tile;
		}
		break;

		case Change::Type::SETTLES:
		{
			os << ",\"subject\":" << change.subject;
		}
		break;

		case Change::Type::SETTLED:
		{
			os << ",\"subject\":" << change.subject;
			os << ",\"tile\":" << change.tile;
		}
		break;

		case Change::Type::EXPANDS:
		{
			os << ",\"subject\":" << change.subject;
			os << ",\"target\":" << change.target;
			os << ",\"power\":" << ((int) change.power);
		}
		break;

		case Change::Type::EXPANDED:
		{
			os << ",\"subject\":" << change.subject;
			os << ",\"tile\":" << change.tile;
		}
		break;

		case Change::Type::UPGRADES:
		{
			os << ",\"subject\":" << change.subject;
			os << ",\"power\":" << ((int) change.power);
		}
		break;

		case Change::Type::UPGRADED:
		{
			os << ",\"subject\":" << change.subject;
			os << ",\"tile\":" << change.tile;
		}
		break;

		case Change::Type::CULTIVATES:
		{
			os << ",\"subject\":" << change.subject;
			os << ",\"power\":" << ((int) change.power);
		}
		break;

		case Change::Type::CULTIVATED:
		{
			os << ",\"subject\":" << change.subject;
			os << ",\"tile\":" << change.tile;
		}
		break;

		case Change::Type::CAPTURES:
		{
			os << ",\"subject\":" << change.subject;
		}
		break;

		case Change::Type::CAPTURED:
		{
			os << ",\"subject\":" << change.subject;
			os << ",\"player\":\"" << change.player << "\"";
		}
		break;

		case Change::Type::PRODUCES:
		{
			os << ",\"subject\":" << change.subject;
			os << ",\"power\":" << ((int) change.power);
		}
		break;

		case Change::Type::PRODUCED:
		{
			os << ",\"subject\":" << change.subject;
			os << ",\"unit\":" << change.unit;
		}
		break;

		case Change::Type::ENTERED:
		{
			os << ",\"subject\":" << change.subject;
			os << ",\"unit\":" << change.unit;
		}
		break;

		case Change::Type::EXITED:
		{
			os << ",\"subject\":" << change.subject;
		}
		break;

		case Change::Type::DIED:
		{
			os << ",\"subject\":" << change.subject;
		}
		break;

		case Change::Type::DESTROYED:
		{
			os << ",\"subject\":" << change.subject;
			os << ",\"tile\":" << change.tile;
		}
		break;

		case Change::Type::SURVIVED:
		{
			os << ",\"subject\":" << change.subject;
		}
		break;

		case Change::Type::AIMS:
		{
			os << ",\"subject\":" << change.subject;
			os << ",\"target\":" << change.target;
			if (change.notice != Notice::NONE)
			{
				os << ",\"notice\":\"" << change.notice << "\"";
			}
		}
		break;

		case Change::Type::ATTACKS:
		{
			os << ",\"subject\":" << change.subject;
			os << ",\"target\":" << change.target;
			os << ",\"figure\":" << ((int) change.figure);
		}
		break;

		case Change::Type::ATTACKED:
		{
			os << ",\"subject\":" << change.subject;
			os << ",\"attacker\":" << change.attacker;
			os << ",\"figure\":" << ((int) change.figure);
			os << ",\"killed\":" << ::jsonify(change.killed);
			os << ",\"depowered\":" << ::jsonify(change.depowered);
		}
		break;

		case Change::Type::TRAMPLES:
		{
			os << ",\"subject\":" << change.subject;
			os << ",\"figure\":" << ((int) change.figure);
		}
		break;

		case Change::Type::TRAMPLED:
		{
			os << ",\"subject\":" << change.subject;
			os << ",\"bombarder\":" << change.bombarder;
			os << ",\"figure\":" << ((int) change.figure);
			os << ",\"killed\":" << ::jsonify(change.killed);
			os << ",\"depowered\":" << ::jsonify(change.depowered);
		}
		break;

		case Change::Type::SHELLS:
		{
			os << ",\"subject\":" << change.subject;
			os << ",\"target\":" << change.target;
			os << ",\"figure\":" << ((int) change.figure);
		}
		break;

		case Change::Type::SHELLED:
		{
			os << ",\"subject\":" << change.subject;
			os << ",\"attacker\":" << change.attacker;
			os << ",\"figure\":" << ((int) change.figure);
			os << ",\"killed\":" << ::jsonify(change.killed);
			os << ",\"depowered\":" << ::jsonify(change.depowered);
		}
		break;

		case Change::Type::BOMBARDS:
		{
			os << ",\"subject\":" << change.subject;
			os << ",\"figure\":" << ((int) change.figure);
		}
		break;

		case Change::Type::BOMBARDED:
		{
			os << ",\"subject\":" << change.subject;
			os << ",\"bombarder\":" << change.bombarder;
			os << ",\"figure\":" << ((int) change.figure);
			os << ",\"killed\":" << ::jsonify(change.killed);
			os << ",\"depowered\":" << ::jsonify(change.depowered);
		}
		break;

		case Change::Type::BOMBS:
		{
			os << ",\"subject\":" << change.subject;
			os << ",\"figure\":" << ((int) change.figure);
		}
		break;

		case Change::Type::BOMBED:
		{
			os << ",\"subject\":" << change.subject;
			os << ",\"bombarder\":" << change.bombarder;
			os << ",\"figure\":" << ((int) change.figure);
			os << ",\"killed\":" << ::jsonify(change.killed);
			os << ",\"depowered\":" << ::jsonify(change.depowered);
		}
		break;

		case Change::Type::FROSTBITTEN:
		{
			os << ",\"subject\":" << change.subject;
			os << ",\"figure\":" << ((int) change.figure);
			os << ",\"killed\":" << ::jsonify(change.killed);
			os << ",\"depowered\":" << ::jsonify(change.depowered);
		}
		break;

		case Change::Type::BURNED:
		{
			os << ",\"subject\":" << change.subject;
			os << ",\"figure\":" << ((int) change.figure);
			os << ",\"killed\":" << ::jsonify(change.killed);
			os << ",\"depowered\":" << ::jsonify(change.depowered);
		}
		break;

		case Change::Type::GASSED:
		{
			os << ",\"subject\":" << change.subject;
			os << ",\"figure\":" << ((int) change.figure);
			os << ",\"killed\":" << ::jsonify(change.killed);
			os << ",\"depowered\":" << ::jsonify(change.depowered);
		}
		break;

		case Change::Type::IRRADIATED:
		{
			os << ",\"subject\":" << change.subject;
			os << ",\"figure\":" << ((int) change.figure);
			os << ",\"killed\":" << ::jsonify(change.killed);
			os << ",\"depowered\":" << ::jsonify(change.depowered);
		}
		break;

		case Change::Type::GROWS:
		{
			os << ",\"subject\":" << change.subject;
			os << ",\"stacks\":" << ((int) change.stacks);
			os << ",\"power\":" << ((int) change.power);
		}
		break;

		case Change::Type::SNOW:
		{
			os << ",\"subject\":" << change.subject;
			os << ",\"snow\":" << ::jsonify(change.snow);
		}
		break;

		case Change::Type::FROSTBITE:
		{
			os << ",\"subject\":" << change.subject;
			os << ",\"frostbite\":" << ::jsonify(change.frostbite);
		}
		break;

		case Change::Type::FIRESTORM:
		{
			os << ",\"subject\":" << change.subject;
			os << ",\"firestorm\":" << ::jsonify(change.firestorm);
		}
		break;

		case Change::Type::BONEDROUGHT:
		{
			os << ",\"subject\":" << change.subject;
			os << ",\"bonedrought\":" << ::jsonify(change.bonedrought);
		}
		break;

		case Change::Type::DEATH:
		{
			os << ",\"subject\":" << change.subject;
			os << ",\"death\":" << ::jsonify(change.death);
		}
		break;

		case Change::Type::GAS:
		{
			os << ",\"subject\":" << change.subject;
			os << ",\"gas\":" << ((int) change.gas);
		}
		break;

		case Change::Type::RADIATION:
		{
			os << ",\"subject\":" << change.subject;
			os << ",\"radiation\":" << ((int) change.radiation);
		}
		break;

		case Change::Type::TEMPERATURE:
		{
			os << ",\"subject\":" << change.subject;
			os << ",\"temperature\":" << ((int) change.temperature);
		}
		break;

		case Change::Type::HUMIDITY:
		{
			os << ",\"subject\":" << change.subject;
			os << ",\"humidity\":" << ((int) change.humidity);
		}
		break;

		case Change::Type::CHAOS:
		{
			os << ",\"subject\":" << change.subject;
			os << ",\"chaos\":" << ((int) change.chaos);
		}
		break;

		case Change::Type::CHAOSREPORT:
		{
			os << ",\"level\":" << ((int) change.level);
		}
		break;

		case Change::Type::YEAR:
		{
			os << ",\"year\":" << ((int) change.year);
		}
		break;

		case Change::Type::SEASON:
		{
			os << ",\"season\":\"" << change.season << "\"";
		}
		break;

		case Change::Type::DAYTIME:
		{
			os << ",\"daytime\":\"" << change.daytime << "\"";
		}
		break;

		case Change::Type::PHASE:
		{
			os << ",\"phase\":\"" << change.phase << "\"";
		}
		break;

		case Change::Type::INITIATIVE:
		{
			os << ",\"player\":\"" << change.player << "\"";
			os << ",\"initiative\":" << ((int) change.initiative);
		}
		break;

		case Change::Type::FUNDS:
		{
			os << ",\"player\":\"" << change.player << "\"";
			os << ",\"money\":" << ((int) change.money);
		}
		break;

		case Change::Type::INCOME:
		{
			os << ",\"subject\":" << change.subject;
			os << ",\"player\":\"" << change.player << "\"";
			os << ",\"money\":" << ((int) change.money);
		}
		break;

		case Change::Type::EXPENDITURE:
		{
			os << ",\"subject\":" << change.subject;
			os << ",\"player\":\"" << change.player << "\"";
			os << ",\"money\":" << ((int) change.money);
		}
		break;

		case Change::Type::SLEEPING:
		{
			os << ",\"player\":\"" << change.player << "\"";
		}
		break;

		case Change::Type::ACTING:
		{
			os << ",\"subject\":" << change.subject;
		}
		break;

		case Change::Type::FINISHED:
		{
			os << ",\"subject\":" << change.subject;
		}
		break;

		case Change::Type::DISCARDED:
		{
			os << ",\"subject\":" << change.subject;
		}
		break;

		case Change::Type::POSTPONED:
		{
			os << ",\"subject\":" << change.subject;
			os << ",\"player\":\"" << change.player << "\"";
			os << ",\"order\":" << change.order;
		}
		break;

		case Change::Type::UNFINISHED:
		{
			os << ",\"subject\":" << change.subject;
			os << ",\"player\":\"" << change.player << "\"";
			os << ",\"order\":" << change.order;
		}
		break;

		case Change::Type::ORDERED:
		{
			os << ",\"subject\":" << change.subject;
			os << ",\"player\":\"" << change.player << "\"";
			os << ",\"order\":" << change.order;
		}
		break;

		case Change::Type::VISION:
		{
			os << ",\"subject\":" << change.subject;
			os << ",\"vision\":" << change.vision;
		}
		break;

		case Change::Type::CORNER:
		{
			os << ",\"subject\":" << change.subject;
		}
		break;

		case Change::Type::BORDER:
		break;

		case Change::Type::SCORED:
		{
			os << ",\"subject\":" << change.subject;
			os << ",\"player\":\"" << change.player << "\"";
			os << ",\"score\":" << ((int) change.score);
		}
		break;

		case Change::Type::DEFEAT:
		{
			os << ",\"player\":\"" << change.player << "\"";
			os << ",\"score\":" << ((int) change.score);
			if (change.notice != Notice::NONE)
			{
				os << ",\"notice\":\"" << change.notice << "\"";
			}
		}
		break;

		case Change::Type::VICTORY:
		{
			os << ",\"player\":\"" << change.player << "\"";
			os << ",\"score\":" << ((int) change.score);
		}
		break;

		case Change::Type::GAMEOVER:
		{
			os << ",\"score\":" << ((int) change.score);
		}
		break;

		case Change::Type::AWARD:
		{
			os << ",\"player\":\"" << change.player << "\"";
			os << ",\"level\":" << ((int) change.level);
		}
		break;
	}

	return os << "}";
}

std::vector<Change> Change::parseChanges(const TypeNamer& typenamer,
		const Json::Value& json)
{
	if (!json.isArray())
	{
		LOGE << "Error while parsing changes: "
			<< "\'" << json << "\'"
			<< ", not an array";
		DEBUG_ASSERT("Error while parsing changes: not an array.");
		return {};
	}

	std::vector<Change> changes;
	try
	{
		for (auto& change : json)
		{
			changes.emplace_back(typenamer, change);
		}
	}
	catch (const ParseError& error)
	{
		LOGE << "Error while parsing changes: "
			<< "\'" << json << "\'"
			<< ", error "
			<< error.what();
		RETHROW_IF_DEV();
	}
	catch (const Json::Exception& error)
	{
		LOGE << "Error while parsing changes: "
			<< "\'" << json << "\'"
			<< ", error "
			<< error.what();
		RETHROW_IF_DEV();
	}
	return changes;
}

std::vector<Change> Change::parseChanges(const TypeNamer& typenamer,
		const std::string& str)
{
	Json::Value json;
	try
	{
		Json::Reader reader;
		if (!reader.parse(str, json))
		{
			LOGE << "Error while parsing changes: "
				<< "\'" << json << "\'"
				<< ", error "
				<< reader.getFormattedErrorMessages();
			DEBUG_ASSERT(false);
			return {};
		}
	}
	catch (const Json::Exception& error)
	{
		LOGE << "Error while parsing changes: "
			<< "\'" << json << "\'"
			<< ", error "
			<< error.what();
		RETHROW_IF_DEV();
		return {};
	}

	return parseChanges(typenamer, json);
}
