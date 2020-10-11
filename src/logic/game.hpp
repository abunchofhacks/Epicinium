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


class Game
{
protected:
	Game() = default;

public:
	Game(const Game&) = delete;
	Game(Game&&) = delete;
	Game& operator=(const Game&) = delete;
	Game& operator=(Game&&) = delete;
	virtual ~Game() = default;

	virtual void load() = 0;
	virtual void update() = 0;

	virtual bool online() const = 0;
	virtual bool test() const = 0;

	virtual float planningTimeTotal() const = 0;
	virtual float planningTimeSpent() const = 0;
	virtual bool haveSentOrders() const = 0;

	virtual void sendOrders() = 0;

	virtual void receiveChanges(const Json::Value& /**/) {}
	virtual void sync(uint32_t /*time*/) {}
	virtual void setAnimation(bool /*animate*/) {}

	virtual void chat(const std::string& message,
		const std::string& target) = 0;

	virtual void attemptQuit() = 0;
	virtual void confirmQuit() = 0;

};
