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
#include "commander.hpp"
#include "clienthandler.hpp"

class GameOwner;
class Settings;
class Client;
enum class Player : uint8_t;
enum class Target : uint8_t;
enum class Phase : uint8_t;


class OnlineGame : public Game, private ClientHandler
{
public:
	OnlineGame(GameOwner& owner, Settings& settings, Client& client,
		const Player& player,
		const std::string& rulesetname,
		uint32_t planningTime);
	OnlineGame(const OnlineGame&) = delete;
	OnlineGame(OnlineGame&&) = delete;
	OnlineGame& operator=(const OnlineGame&) = delete;
	OnlineGame& operator=(OnlineGame&&) = delete;
	virtual ~OnlineGame() = default;

protected:
	GameOwner& _owner;
	Client& _client;

	Player _player;

	const uint32_t _planningTime;

	float _time;
	float _planningStart;
	Phase _phase;

	Commander _commander;

	virtual void load() override;
	virtual void update() override;

	virtual bool online() const override;
	virtual bool test() const override { return false; }

	virtual float planningTimeTotal() const override;
	virtual float planningTimeSpent() const override;
	virtual bool haveSentOrders() const override;

	virtual void sendOrders() override;

	virtual void chat(const std::string& message,
		const std::string& target) override;

	virtual void attemptQuit() override;
	virtual void confirmQuit() override;

	virtual void debugHandler() const override;

	virtual void message(const std::string& message) override;
	virtual void chat(const std::string& user, const std::string& message,
		const Target& target) override;

	virtual void assignColor(const std::string& name,
		const Player& player) override;
	virtual void setSkins(const Json::Value& metadata) override;
	virtual void receiveBriefing(const Json::Value& metadata) override;

	virtual void disconnected() override;
	virtual void outLobby() override;

public:
	virtual void receiveChanges(const Json::Value& json) override;
	virtual void sync(uint32_t time) override;
	virtual void setAnimation(bool animate) override;
};
