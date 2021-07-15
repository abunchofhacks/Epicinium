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

#include "localgame.hpp"
#include "clienthandler.hpp"

class GameOwner;
class Settings;
class Client;
enum class Target : uint8_t;


class OnlineLocalGame : public LocalGame, private ClientHandler
{
public:
	OnlineLocalGame(GameOwner& owner, Settings& settings, Client& client,
		std::shared_ptr<AIChallenge> challenge,
		const std::string& mapname, const std::string& rulesetname);
	OnlineLocalGame(const OnlineLocalGame&) = delete;
	OnlineLocalGame(OnlineLocalGame&&) = delete;
	OnlineLocalGame& operator=(const OnlineLocalGame&) = delete;
	OnlineLocalGame& operator=(OnlineLocalGame&&) = delete;
	virtual ~OnlineLocalGame() = default;

private:
	Client& _client;

	virtual void load() override;

	virtual bool online() const override { return true; }
	virtual bool test() const override { return false; }

	virtual void chat(const std::string& message,
		const std::string& target) override;

	virtual void confirmQuit() override;

	virtual void debugHandler() const override;

	virtual void message(const std::string& message) override;
	virtual void chat(const std::string& user, const std::string& message,
		const Target& target) override;
};
