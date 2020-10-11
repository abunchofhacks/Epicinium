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
#include "ranking.hpp"
#include "source.hpp"


Ranking::Ranking(const Json::Value& json)
{
	if (json["name"].isString())   name   = json["name"].asString();
	else if (json["username"].isString()) name = json["username"].asString();

	if (json["rank"].isInt())      rank   = json["rank"].asInt();
	if (json["rating"].isDouble()) rating = json["rating"].asFloat();
}

Json::Value Ranking::toJson() const
{
	Json::Value json(Json::objectValue);
	json["name"] = name;
	json["rank"] = rank;
	json["rating"] = rating;
	return json;
}

std::vector<Ranking> parseRankings(Json::Value json)
{
	std::vector<Ranking> rankings;
	if (json.isArray())
	{
		for (auto& ranking : json)
		{
			rankings.emplace_back(ranking);
		}
	}
	return rankings;
}
