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
#include "onlinelocalgame.hpp"
#include "source.hpp"

#include "gameowner.hpp"
#include "client.hpp"
#include "message.hpp"
#include "target.hpp"
#include "colorname.hpp"
#include "commander.hpp"
#include "bot.hpp"
#include "aicommander.hpp"


OnlineLocalGame::OnlineLocalGame(GameOwner& owner, Settings& settings,
		Client& client,
		std::shared_ptr<AIChallenge> challenge,
		const std::string& mapname, const std::string& rulesetname) :
	LocalGame(owner, settings, challenge, mapname, rulesetname),
	_client(client)
{}

void OnlineLocalGame::load()
{
	LocalGame::load();

	for (const auto& commander : _commanders)
	{
		commander->addChatmode(stringify(Target::GENERAL),
			_("ALL"),
			ColorName::TEXT800);
		commander->setChatmode(stringify(Target::GENERAL));
	}

	_client.registerHandler(this);
}

void OnlineLocalGame::confirmQuit()
{
	_client.deregisterHandler(this);
	_owner.stopGame();
}

void OnlineLocalGame::chat(const std::string& message,
	const std::string& target)
{
	_client.send(Message::chat(message, parseTarget(target)));
}

void OnlineLocalGame::debugHandler() const
{
	LOGD << ((void*) this);
}

void OnlineLocalGame::message(const std::string& message)
{
	for (const auto& commander : _commanders)
	{
		commander->message(message);
	}
}

void OnlineLocalGame::chat(const std::string& user, const std::string& message,
		const Target& target)
{
	for (const auto& commander : _commanders)
	{
		commander->chat(user, message, stringify(target));
	}
}
