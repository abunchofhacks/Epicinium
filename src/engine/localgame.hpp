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
#include "automaton.hpp"

class GameOwner;
enum class Player : uint8_t;
struct Bot;
enum class Phase : uint8_t;
class Settings;
class Commander;
class AICommander;
class AILibrary;
class Observer;
class ChangeSet;
class AIChallenge;


class LocalGame : public Game
{
private:
	LocalGame(GameOwner& owner, Settings& settings,
		std::shared_ptr<AIChallenge> challenge,
		const std::vector<Player>& players,
		const std::vector<Bot>& bots, const std::string& mapname,
		const std::string& rulesetname,
		unsigned int humanplayers,
		bool silentConfirmQuit, bool enableRecording);

public:
	LocalGame(GameOwner& owner, Settings& settings,
		const std::vector<Player>& players,
		const std::vector<Bot>& bots, const std::string& mapname,
		const std::string& rulesetname,
		unsigned int humanplayers,
		bool silentConfirmQuit = false, bool enableRecording = true);
	LocalGame(GameOwner& owner, Settings& settings,
		std::shared_ptr<AIChallenge> challenge,
		const std::vector<Player>& players,
		const std::vector<Bot>& bots, const std::string& mapname,
		unsigned int humanplayers,
		bool silentConfirmQuit = false, bool enableRecording = true);
	LocalGame(GameOwner& owner, Settings& settings,
		const std::vector<Player>& players,
		const std::vector<Bot>& bots, const std::string& mapname,
		unsigned int humanplayers,
		bool silentConfirmQuit = false, bool enableRecording = true);
	LocalGame(GameOwner& owner, Settings& settings,
		std::shared_ptr<AIChallenge> challenge,
		const std::string& mapname, const std::string& rulesetname,
		bool silentConfirmQuit = false, bool enableRecording = true);
	LocalGame(GameOwner& owner, Settings& settings,
		std::shared_ptr<AIChallenge> challenge,
		bool silentConfirmQuit = false, bool enableRecording = true);

	LocalGame(const LocalGame&) = delete;
	LocalGame(LocalGame&&) = delete;
	LocalGame& operator=(const LocalGame&) = delete;
	LocalGame& operator=(LocalGame&&) = delete;
	virtual ~LocalGame() = default;

protected:
	GameOwner& _owner;
	Settings& _settings;

	const std::string _mapname;
	const std::string _rulesetname;
	Automaton _automaton;
	Phase _phase;

	std::shared_ptr<AIChallenge> _challenge;
	std::vector<Player> _players;
	std::vector<Bot> _bots;
	std::vector<std::unique_ptr<Commander>> _commanders;
	std::vector<std::unique_ptr<AICommander>> _aicommanders;
	std::vector<std::shared_ptr<AILibrary>> _ailibraries; // (unique ownership)
	std::unique_ptr<Observer> _blind;
	std::unique_ptr<Observer> _observer;
	int _activeindex;

	bool _silentConfirmQuit;
	bool _enableRecording;

	int _seed;

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

	void enterActionPhase();
	void stepActionPhase();
	void pipe(const ChangeSet& changeset);
	void leaveActionPhase();
};
