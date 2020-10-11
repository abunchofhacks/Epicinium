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
#include "recording.hpp"
#include "automaton.hpp"

class GameOwner;
enum class Player : uint8_t;
enum class Phase : uint8_t;
class Settings;
class Observer;
class ChangeSet;


class LocalReplay : public Game
{
public:
	LocalReplay(GameOwner& owner, Settings& settings,
		const std::string& replayname,
		bool silentConfirmQuit = false);
	LocalReplay(const LocalReplay&) = delete;
	LocalReplay(LocalReplay&&) = delete;
	LocalReplay& operator=(const LocalReplay&) = delete;
	LocalReplay& operator=(LocalReplay&&) = delete;
	virtual ~LocalReplay() = default;

private:
	GameOwner& _owner;

	Recording _recording;
	const std::string _replayname;
	const std::string _rulesetname;
	Automaton _automaton;
	Phase _phase;

	std::unique_ptr<Observer> _observer;

	bool _silentConfirmQuit;

	virtual void load() override;
	virtual void update() override;

	virtual bool online() const override;
	virtual bool test() const override { return _silentConfirmQuit; }

	virtual float planningTimeTotal() const override { return 0; }
	virtual float planningTimeSpent() const override { return 0; }
	virtual bool haveSentOrders() const override { return false; }

	virtual void sendOrders() override;

	virtual void chat(const std::string& message,
		const std::string& target) override;

	virtual void attemptQuit() override;
	virtual void confirmQuit() override;

	void pipe(const ChangeSet& changeset);
};
