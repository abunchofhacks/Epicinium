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
#include "hostedgame.hpp"
#include "source.hpp"

#include "aicommander.hpp"
#include "client.hpp"
#include "message.hpp"
#include "player.hpp"
#include "visiontype.hpp"
#include "cycle.hpp"
#include "map.hpp"
#include "aichallenge.hpp"


HostedGame::HostedGame(Client& client,
		std::shared_ptr<Challenge> challenge,
		const std::vector<Player>& colors,
		const std::vector<VisionType>& visiontypes,
		const std::vector<std::string>& usernames,
		const std::vector<Bot>& bots,
		bool hasObservers,
		const std::string& mapname, const std::string& rulesetname) :
	_client(client),
	_automaton(colors, rulesetname),
	_usernames(usernames),
	_hasObservers(hasObservers),
	_phase(Phase::GROWTH)
{
	Json::Value metadata = Map::loadMetadata(mapname);
	metadata["map"] = mapname;
	metadata["players"] = Json::arrayValue;
	metadata["bots"] = Json::arrayValue;

	assert(visiontypes.size() == colors.size());
	assert(_usernames.size() + bots.size() == colors.size());
	for (size_t i = 0; i < colors.size(); i++)
	{
		Player color = colors[i];
		switch (visiontypes[i])
		{
			case VisionType::NONE:
			case VisionType::NORMAL:
			break;
			case VisionType::GLOBAL:
			{
				_automaton.grantGlobalVision(color);
			}
			break;
		}

		if (i < _usernames.size())
		{
			{
				Json::Value json = Json::objectValue;
				json["player"] = ::stringify(color);
				json["username"] = _usernames[i];
				metadata["players"].append(json);
			}
			_usernamecolors.emplace_back(color);
		}
		else
		{
			const Bot& bot = bots[i - _usernames.size()];
			auto ai = bot.createAI(color, rulesetname);
			{
				Json::Value json = Json::objectValue;
				json["player"] = ::stringify(color);
				json["difficulty"] = ::stringify(ai->difficulty());
				json["character"] = ai->characterstring();
				json["displayname"] = ai->displayname();
				json["ainame"] = ai->ainame();
				json["authors"] = ai->authors();
				metadata["bots"].append(json);
			}
			_bots.emplace_back(std::move(ai));
		}
	}

	if (challenge)
	{
		_automaton.setChallenge(challenge);

		AIChallenge aichallenge(*challenge);
		Json::Value briefing = aichallenge.getMissionBriefing();
		if ((!briefing["description"].isString()
				|| briefing["description"].asString().empty())
			&& metadata["challenge"].isObject()
			&& metadata["challenge"]["briefing"].isObject())
		{
			briefing = metadata["challenge"]["briefing"];
		}
		_client.send(Message::briefing(briefing));
	}

	bool shufflePlayers = !challenge;
	_automaton.load(mapname, shufflePlayers);
	_automaton.startRecording(metadata);
}

void HostedGame::broadcast(const ChangeSet& changeset)
{
	for (auto& ai : _bots)
	{
		ai->receiveChanges(changeset.get(ai->player()));
	}

	for (const Player& player : _usernamecolors)
	{
		Json::Value forwarding = Json::objectValue;
		forwarding["player"] = ::stringify(player);
		_client.send(Message::change(_automaton.bible(),
			changeset.get(player), forwarding));
	}

	if (_hasObservers)
	{
		Player player = Player::OBSERVER;
		Json::Value forwarding = Json::objectValue;
		forwarding["player"] = ::stringify(player);
		_client.send(Message::change(_automaton.bible(),
			changeset.get(player), forwarding));
	}
}

void HostedGame::sync()
{
	switch (_phase)
	{
		case Phase::GROWTH:
		case Phase::ACTION:
		{
			while (_automaton.active())
			{
				broadcast(_automaton.act());
			}

			std::vector<Player> defeatedplayers;
			for (const Player& player : _usernamecolors)
			{
				if (_automaton.defeated(player))
				{
					defeatedplayers.push_back(player);
				}
			}

			Json::Value metadata = Json::objectValue;
			metadata["game_over"] = _automaton.gameover();
			if (!defeatedplayers.empty())
			{
				metadata["defeated_players"] = Json::arrayValue;
				for (const Player& player : defeatedplayers)
				{
					metadata["defeated_players"].append(::stringify(player));
				}
			}
			if (_automaton.gameover() && _usernamecolors.size() >= 1
				&& _automaton.award(_usernamecolors[0]) > 0)
			{
				metadata["stars"] = _automaton.award(_usernamecolors[0]);
			}
			_client.send(Message::host_sync(metadata));

			LOGV << "Leaving " << _phase << " phase";
			_phase = Phase::RESTING;
			LOGV << "Entered " << _phase << " phase";
		}
		break;
		case Phase::RESTING:
		{
			broadcast(_automaton.hibernate());

			_client.send(Message::host_sync());

			LOGV << "Leaving " << _phase << " phase";
			_phase = Phase::PLANNING;
			LOGV << "Entered " << _phase << " phase";

			// Allow the bots to calculate their next move.
			// Do this right after we have synced with the server, because
			// we want to minimize the delay between when the server decides
			// that the planning phase has ended and us confirming it.
			for (auto& ai : _bots)
			{
				if (!_automaton.defeated(ai->player()))
				{
					ai->prepareOrders();
				}
			}
		}
		break;
		case Phase::PLANNING:
		{
			broadcast(_automaton.awake());

			_client.send(Message::host_sync());

			LOGV << "Leaving " << _phase << " phase";
			_phase = Phase::STAGING;
			LOGV << "Entered " << _phase << " phase";

			// Lock in the bots' orders.
			// Do this right after we have synced with the server, because
			// we want to minimize the delay between when the server decides
			// that the staging phase has ended and us confirming it.
			for (auto& ai : _bots)
			{
				if (!_automaton.defeated(ai->player()))
				{
					_automaton.receive(ai->player(), ai->orders());
				}
			}
		}
		break;
		case Phase::STAGING:
		{
			broadcast(_automaton.prepare());

			_client.send(Message::host_sync());

			LOGV << "Leaving " << _phase << " phase";
			_phase = Phase::ACTION;
			LOGV << "Entered " << _phase << " phase";
		}
		break;
		case Phase::DECAY:
		{
			LOGE << "Impossible phase";
			DEBUG_ASSERT(false);
		}
		break;
	}
}

void HostedGame::receiveOrders(const Json::Value& json,
	const Json::Value& forwarding)
{
	Player player = ::parsePlayer(forwarding["player"].asString());
	_automaton.receive(player, Order::parseOrders(_automaton.bible(), json));
}

void HostedGame::handleResign(const std::string& username)
{
	for (size_t i = 0; i < _usernamecolors.size(); i++)
	{
		if (username == _usernames[i])
		{
			_automaton.resign(_usernamecolors[i]);
		}
	}
}

void HostedGame::handleRejoin(const std::string& username, const Player& vision)
{
	// We have to wait until not syncing before responding to this.
	// But because ::sync() is currently synchronous/blocking, we're not.
	_client.send(Message::host_rejoin_changes(
		_automaton.bible(),
		_automaton.rejoin(vision).get(vision),
		vision, username));
}
