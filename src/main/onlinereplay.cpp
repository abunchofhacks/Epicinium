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
#include "onlinereplay.hpp"
#include "source.hpp"

#include "libs/SDL2/SDL_timer.h"

#include "gameowner.hpp"
#include "input.hpp"
#include "client.hpp"
#include "message.hpp"
#include "role.hpp"
#include "target.hpp"
#include "cycle.hpp"
#include "paint.hpp"
#include "colorname.hpp"
#include "settings.hpp"


OnlineReplay::OnlineReplay(GameOwner& owner, Settings& settings, Client& client,
		const Role& role,
		const std::string& rulesetname,
		uint32_t planningTime) :
	_owner(owner),
	_client(client),
	_role(role),
	_planningTime(planningTime),
	_time(0),
	_planningStart(0),
	_phase(Phase::GROWTH),
	_observer(settings, *this, getVisionLevel(_role), rulesetname)
{
	if (settings.enableGeneralChat.value())
	{
		_observer.addChatmode(stringify(Target::GENERAL),
			_("ALL"),
			ColorName::TEXT800);
	}
	if (settings.enableLobbyChat.value())
	{
		_observer.addChatmode(stringify(Target::LOBBY),
			_("GAME"),
			ColorName::TEXTLOBBYCHAT);
	}
}

void OnlineReplay::load()
{
	_observer.load();

	_observer.setChatmode(stringify(Target::LOBBY));

	_client.registerHandler(this);
}

void OnlineReplay::update()
{
	_time = SDL_GetTicks() * 0.001f;

	switch (_phase)
	{
		case Phase::GROWTH:
		case Phase::ACTION:
		{
			// Wait for animations to finish.
			if (_observer.phase() == Phase::RESTING
				&& !_observer.animating())
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
			// Wait for the staging phase to start.
			if (_observer.phase() == Phase::STAGING)
			{
				LOGV << "Leaving " << _phase << " phase";
				_phase = Phase::STAGING;
				LOGV << "Entered " << _phase << " phase";
			}
		}
		break;

		case Phase::STAGING:
		{
			// Wait for the action phase to start.
			if (_observer.phase() == Phase::ACTION)
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

	_observer.update();

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
			if (_observer.phase() == Phase::PLANNING)
			{
				float timeLeft = _planningTime - (_time - _planningStart);
				ImGui::Text("%05.2f", timeLeft > 0 ? timeLeft : 0);
			}
			else ImGui::Text("-");
		}
		ImGui::End();
	}
}

bool OnlineReplay::online() const
{
	return true;
}

float OnlineReplay::planningTimeTotal() const
{
	return _planningTime;
}

float OnlineReplay::planningTimeSpent() const
{
	if (_observer.phase() == Phase::PLANNING)
	{
		return _time - _planningStart;
	}
	else return 0;
}

void OnlineReplay::chat(const std::string& message, const std::string& target)
{
	_client.send(Message::chat(message, parseTarget(target)));
}

void OnlineReplay::attemptQuit()
{
	_observer.attemptQuit();
}

void OnlineReplay::confirmQuit()
{
	_client.send(Message::leave_lobby());
	_client.deregisterHandler(this);
	_owner.stopGame();
}

void OnlineReplay::disconnected()
{
	_client.deregisterHandler(this);
	_owner.stopGame();
}

void OnlineReplay::debugHandler() const
{
	LOGD << ((void*) this);
}

void OnlineReplay::message(const std::string& message)
{
	_observer.message(message);
}

void OnlineReplay::chat(const std::string& user, const std::string& message,
		const Target& target)
{
	_observer.chat(user, message, stringify(target));
}

void OnlineReplay::assignColor(const std::string& name,
		const Player& player)
{
	_observer.assignColor(name, player);
}

void OnlineReplay::setSkins(const Json::Value& metadata)
{
	_observer.setSkins(metadata);
}

void OnlineReplay::receiveBriefing(const Json::Value& metadata)
{
	_observer.receiveBriefing(metadata);
}

void OnlineReplay::outLobby()
{
	disconnected();
}

void OnlineReplay::receiveChanges(const Json::Value& json)
{
	_observer.receiveChanges(Change::parseChanges(_observer.bible(), json));
}

void OnlineReplay::sync(uint32_t time)
{
	if (_phase != Phase::PLANNING)
	{
		LOGV << "Leaving " << _phase << " phase";
		_phase = Phase::PLANNING;
		LOGV << "Entered " << _phase << " phase";
	}

	_planningStart = _time + time - _planningTime;
}

void OnlineReplay::setAnimation(bool animate)
{
	_observer.setAnimation(animate);
}
