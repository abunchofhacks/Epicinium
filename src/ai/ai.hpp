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

class AICommander;
enum class Player : uint8_t;
enum class Difficulty : uint8_t;


class AI
{
public:
	static AICommander* allocate(const std::string& ainame,
		const Player& player, const Difficulty& difficulty,
		const std::string& ruleset, char character);

public:
	static std::unique_ptr<AICommander> create(const std::string& ainame,
		const Player& player, const Difficulty& difficulty,
		const std::string& ruleset, char character);

	static bool exists(const std::string& name);
	static std::string libraryDefaultFilename(const std::string& name);

	static const std::vector<std::string>& pool();
    static const std::vector<std::string>& selfHostedPool();

private:
	static const std::vector<std::string>& hiddenPool();
};
