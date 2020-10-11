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

#include "game.hpp"

class GameOwner;
class Settings;
class Observer;
class ChangeSet;


class LocalDiorama : public Game
{
public:
	LocalDiorama(GameOwner& owner, Settings& settings,
		const std::string& mapname,
		bool silentConfirmQuit = false);
	LocalDiorama(const LocalDiorama&) = delete;
	LocalDiorama(LocalDiorama&&) = delete;
	LocalDiorama& operator=(const LocalDiorama&) = delete;
	LocalDiorama& operator=(LocalDiorama&&) = delete;
	virtual ~LocalDiorama() = default;

private:
	GameOwner& _owner;

	const std::string _mapname;

	std::unique_ptr<Observer> _observer;

	bool _silentConfirmQuit;

	virtual void load() override;
	virtual void update() override;

	virtual bool online() const override { return false; }
	virtual bool test() const override { return _silentConfirmQuit; }

	virtual float planningTimeTotal() const override { return 0; }
	virtual float planningTimeSpent() const override { return 0; }
	virtual bool haveSentOrders() const override { return false; }

	virtual void sendOrders() override;

	virtual void chat(const std::string& message,
		const std::string& target) override;

	virtual void attemptQuit() override;
	virtual void confirmQuit() override;
};
