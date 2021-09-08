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
#include "onlinediorama.hpp"
#include "source.hpp"

#include "gameowner.hpp"
#include "changeset.hpp"
#include "map.hpp"
#include "cycle.hpp"
#include "client.hpp"
#include "message.hpp"
#include "target.hpp"
#include "library.hpp"
#include "colorname.hpp"


OnlineDiorama::OnlineDiorama(GameOwner& owner, Settings& settings,
		Client& client, const std::string& mapname) :
	_owner(owner),
	_client(client),
	_mapname(mapname)
{
	Json::Value metadata;
	metadata = Map::loadMetadata(_mapname);

	std::string rulesetname = Library::nameCurrentBible();
	if (metadata["ruleset"].isString())
	{
		std::string name = metadata["ruleset"].asString();
		if (Library::existsBible(name))
		{
			rulesetname = name;
		}
	}

	_observer.reset(new Observer(settings, *this, rulesetname));
	_observer->setSkins(metadata);
}

void OnlineDiorama::load()
{
	_observer->load();

	{
		Board board(_observer->bible());
		board.load(_mapname);

		ChangeSet cset;
		cset.push(Change(Change::Type::CORNER, Descriptor::cell(
				Position(board.rows() - 1, board.cols() - 1))),
			Vision::none());
		for (Cell index : board)
		{
			{
				Change change(Change::Type::REVEAL,
					Descriptor::tile(index.pos()),
					board.tile(index),
					board.snow(index),
					board.frostbite(index),
					board.firestorm(index),
					board.bonedrought(index),
					board.death(index),
					board.gas(index),
					board.radiation(index),
					board.temperature(index),
					board.humidity(index),
					board.chaos(index));
				cset.push(change, Vision::none());
			}

			if (board.ground(index))
			{
				Change change(Change::Type::ENTERED,
					Descriptor::ground(index.pos()),
					board.ground(index));
				cset.push(change, Vision::none());
			}

			if (board.air(index))
			{
				Change change(Change::Type::ENTERED,
					Descriptor::air(index.pos()),
					board.air(index));
				cset.push(change, Vision::none());
			}
		}
		cset.push(Change(Change::Type::BORDER), Vision::none());
		_observer->receiveChanges(cset.get(Player::OBSERVER));
	}

	{
		ChangeSet cset;
		cset.push(Change(Change::Type::PHASE, Phase::RESTING), Vision::none());
		_observer->receiveChanges(cset.get(Player::OBSERVER));
	}

	{
		ChangeSet cset;
		cset.push(Change(Change::Type::PHASE, Phase::PLANNING), Vision::none());
		_observer->receiveChanges(cset.get(Player::OBSERVER));
	}

	_observer->addChatmode(stringify(Target::GENERAL),
		_("ALL"),
		ColorName::TEXT800);
	_observer->setChatmode(stringify(Target::GENERAL));

	_client.registerHandler(this);
}

void OnlineDiorama::update()
{
	_observer->update();
}

void OnlineDiorama::sendOrders()
{
	// This is a diorama.
}

void OnlineDiorama::attemptQuit()
{
	_observer->attemptQuit();
}

void OnlineDiorama::confirmQuit()
{
	_client.deregisterHandler(this);
	_owner.stopGame();
}

void OnlineDiorama::chat(const std::string& message, const std::string& target)
{
	_client.send(Message::chat(message, parseTarget(target)));
}

void OnlineDiorama::debugHandler() const
{
	LOGD << ((void*) this);
}

void OnlineDiorama::message(const std::string& message)
{
	_observer->message(message);
}

void OnlineDiorama::chat(const std::string& user, const std::string& message,
		const Target& target)
{
	_observer->chat(user, message, stringify(target));
}
