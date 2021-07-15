/**
 * Part of Epicinium
 * developed by A Bunch of Hacks.
 *
 * Copyright (c) 2017-2021 A Bunch of Hacks
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

#include "bot.hpp"
#include "automaton.hpp"

class Settings;
class Client;
enum class Player : uint8_t;
enum class VisionType : uint8_t;
enum class Phase : uint8_t;


class HostedGame
{
public:
	HostedGame(Client& client,
		const std::vector<Player>& colors,
		const std::vector<VisionType>& visiontypes,
		const std::vector<std::string>& usernames,
		const std::vector<Bot>& bots,
		bool hasObservers,
		const std::string& mapname, const std::string& rulesetname);
	HostedGame(const HostedGame&) = delete;
	HostedGame(HostedGame&&) = delete;
	HostedGame& operator=(const HostedGame&) = delete;
	HostedGame& operator=(HostedGame&&) = delete;
	virtual ~HostedGame() = default;

protected:
	Client& _client;

	Automaton _automaton;
	std::vector<std::string> _usernames; // (married)
	std::vector<Player> _usernamecolors; // (married)
	std::vector<std::unique_ptr<AICommander>> _bots;
	bool _hasObservers;
	Phase _phase;

	void broadcast(const ChangeSet& changeset);

public:
	void sync();

	void receiveOrders(const Json::Value& orders, const Json::Value& metadata);
	void handleResign(const std::string& username);
	void handleRejoin(const std::string& username, const Player& vision);
};
