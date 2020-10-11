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
#include "bot.hpp"
#include "source.hpp"

#include "difficulty.hpp"
#include "ai.hpp"
#include "aicommander.hpp"


Bot::Bot(const std::string& s, const std::string& a, const Difficulty& d) :
	slotname(s),
	character(s[1]),
	ainame(a),
	difficulty(d)
{}

std::unique_ptr<AICommander> Bot::createAI(const Player& player,
	const std::string& rulesetname) const
{
	return AI::create(ainame, player, difficulty, rulesetname, character);
}
