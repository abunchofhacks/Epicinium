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

class TypeNamer;
struct Change;
struct Order;
enum class Player : uint8_t;
enum class Difficulty : uint8_t;


class AILibrary
{
public:
	AILibrary() = default;
	AILibrary(const AILibrary&) = delete;
	AILibrary(AILibrary&&) = delete;
	AILibrary& operator=(const AILibrary&) = delete;
	AILibrary& operator=(AILibrary&&) = delete;
	virtual ~AILibrary() = default;

	void receiveChanges(const std::vector<Change>& changes,
		const TypeNamer& namer);
	virtual void receiveChangesAsString(const std::string& changes) = 0;

	virtual void prepareOrders() = 0;

	std::vector<Order> orders(const TypeNamer& namer);
	virtual std::string ordersAsString() = 0;

	Player player() const;
	virtual std::string playerAsString() const = 0;

	Difficulty difficulty() const;
	virtual std::string difficultyAsString() const = 0;

	virtual std::string characterstring() const = 0;

	virtual std::string displayname() const = 0;
	virtual std::string ainame() const = 0;
	virtual std::string authors() const = 0;
	virtual std::string descriptivename() const = 0;

	static bool exists(const std::string& name);
	static bool isLibraryReminder(const std::string& name);
	static bool hasFastDirective(const std::string& name);

	static void preload(const std::string& name);

	static std::shared_ptr<AILibrary> create(const std::string& name,
		const Player& player, const Difficulty& difficulty,
		const std::string& ruleset, char character);
	static AILibrary* allocate(const std::string& name,
		const Player& player, const Difficulty& difficulty,
		const std::string& ruleset, char character);

	static void setup(const char* name, int argc, const char* const argv[]);
};
