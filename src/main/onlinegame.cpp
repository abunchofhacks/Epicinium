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
#include "onlinegame.hpp"
#include "source.hpp"

#include "libs/SDL2/SDL_timer.h"

#include "gameowner.hpp"
#include "input.hpp"
#include "client.hpp"
#include "message.hpp"
#include "player.hpp"
#include "target.hpp"
#include "cycle.hpp"
#include "paint.hpp"
#include "colorname.hpp"
#include "settings.hpp"


OnlineGame::OnlineGame(GameOwner& owner, Settings& settings, Client& client,
		const Player& player,
		const std::string& rulesetname,
		uint32_t planningTime) :
	_owner(owner),
	_client(client),
	_player(player),
	_planningTime(planningTime),
	_time(0),
	_planningStart(0),
	_phase(Phase::GROWTH),
	_commander(settings, *this, player, rulesetname)
{
	_time = SDL_GetTicks() * 0.001f;

	if (settings.enableGeneralChat.value())
	{
		_commander.addChatmode(stringify(Target::GENERAL),
			_("ALL"),
			ColorName::TEXT800);
	}
	if (settings.enableLobbyChat.value())
	{
		_commander.addChatmode(stringify(Target::LOBBY),
			_("GAME"),
			ColorName::TEXTLOBBYCHAT);
	}
}

void OnlineGame::load()
{
	_commander.load();

	_commander.setChatmode(stringify(Target::LOBBY));

	_client.registerHandler(this);
}

void OnlineGame::update()
{
	_time = SDL_GetTicks() * 0.001f;

	switch (_phase)
	{
		case Phase::GROWTH:
		case Phase::ACTION:
		{
			// Wait for animations to finish.
			if (_commander.phase() == Phase::RESTING
				&& !_commander.animating())
			{
				_client.send(Message::sync());

				LOGV << "Leaving " << _phase << " phase";
				_phase = Phase::RESTING;
				LOGV << "Entered " << _phase << " phase";
			}
		}
		break;

		case Phase::RESTING:
		{
			// Wait for sync.
		}
		break;

		case Phase::PLANNING:
		{
			// Wait for planning timer, for the staging phase announcement or
			// for the user to manually send orders.
			if (_planningTime > 0
				&& _time - _planningStart >= _planningTime)
			{
				sendOrders();
			}
			else if (_commander.phase() == Phase::STAGING)
			{
				sendOrders();
			}
		}
		break;

		case Phase::STAGING:
		{
			// Wait for the action phase to start.
			if (_commander.phase() == Phase::ACTION)
			{
				LOGV << "Leaving " << _phase << " phase";
				_phase = Phase::ACTION;
				LOGV << "Entered " << _phase << " phase";
			}
		}
		break;

		case Phase::DECAY:
		break;
	}

	_commander.update();

	/* IMGUI */
	static bool show = false;
	bool wasshown = show;

	if (Input::get()->isDebugKeyHeld())
	{
		if (ImGui::Begin("Windows", nullptr,
			ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse))
		{
			ImGui::Checkbox("Countdown", &show);
		}
		ImGui::End();
	}

	if (show)
	{
		if (!wasshown) ImGui::SetNextWindowCollapsed(false);

		if (ImGui::Begin("Countdown", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			if (_commander.phase() == Phase::PLANNING)
			{
				float timeLeft = _planningTime - (_time - _planningStart);
				ImGui::Text("%05.2f", timeLeft > 0 ? timeLeft : 0);
			}
			else ImGui::Text("-");
		}
		ImGui::End();
	}
}

bool OnlineGame::online() const
{
	return true;
}

float OnlineGame::planningTimeTotal() const
{
	return _planningTime;
}

float OnlineGame::planningTimeSpent() const
{
	if (_commander.phase() == Phase::PLANNING)
	{
		return _time - _planningStart;
	}
	else return 0;
}

bool OnlineGame::haveSentOrders() const
{
	return (_phase == Phase::STAGING);
}

void OnlineGame::sendOrders()
{
	if (_phase == Phase::PLANNING)
	{
		LOGV << "Leaving " << _phase << " phase";
		_phase = Phase::STAGING;
		LOGV << "Entered " << _phase << " phase";
	}
	else if (_phase == Phase::STAGING)
	{
		// Valid.
	}
	else
	{
		LOGW << "Sync received outside of planning or staging phase";
		DEBUG_ASSERT(false);
	}

	_client.send(Message::order_new(_commander.bible(), _commander.orders()));
}

void OnlineGame::chat(const std::string& message, const std::string& target)
{
	_client.send(Message::chat(message, parseTarget(target)));
}

void OnlineGame::attemptQuit()
{
	_commander.attemptQuit();
}

void OnlineGame::confirmQuit()
{
	if (!_commander.gameover())
	{
		_client.send(Message::resign());
	}
	_client.send(Message::leave_lobby());
	_client.deregisterHandler(this);
	_owner.stopGame();
}

void OnlineGame::disconnected()
{
	_client.deregisterHandler(this);
	_owner.stopGame();
}

void OnlineGame::debugHandler() const
{
	LOGD << ((void*) this);
}

void OnlineGame::message(const std::string& message)
{
	_commander.message(message);
}

void OnlineGame::chat(const std::string& user, const std::string& message,
		const Target& target)
{
	_commander.chat(user, message, stringify(target));
}

void OnlineGame::assignColor(const std::string& name,
		const Player& player)
{
	_commander.assignColor(name, player);
}

void OnlineGame::setSkins(const Json::Value& metadata)
{
	_commander.setSkins(metadata);
}

void OnlineGame::receiveBriefing(const Json::Value& metadata)
{
	_commander.receiveBriefing(metadata);
}

void OnlineGame::outLobby()
{
	disconnected();
}

void OnlineGame::receiveChanges(const Json::Value& json)
{
	_commander.receiveChanges(Change::parseChanges(_commander.bible(), json));
}

void OnlineGame::sync(uint32_t time)
{
	if (_phase == Phase::GROWTH || _phase == Phase::RESTING)
	{
		LOGV << "Leaving " << _phase << " phase";
		_phase = Phase::PLANNING;
		LOGV << "Entered " << _phase << " phase";
	}
	else
	{
		LOGW << "Sync received outside of resting or growth phase";
		DEBUG_ASSERT(false);
	}

	_planningStart = _time + time - _planningTime;
}

void OnlineGame::setAnimation(bool animate)
{
	_commander.setAnimation(animate);
}
