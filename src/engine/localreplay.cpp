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
#include "localreplay.hpp"
#include "source.hpp"

#include "gameowner.hpp"
#include "input.hpp"
#include "observer.hpp"
#include "cycle.hpp"


LocalReplay::LocalReplay(GameOwner& owner, Settings& settings,
		const std::string& replayname,
		bool silentConfirmQuit) :
	_owner(owner),
	_recording(replayname),
	_replayname(replayname),
	_rulesetname(_recording.getRuleset()),
	_automaton(_recording.getPlayers(), _rulesetname),
	_phase(Phase::GROWTH),
	_silentConfirmQuit(silentConfirmQuit)
{
	_observer.reset(new Observer(settings, *this, _rulesetname));
}

void LocalReplay::load()
{
	// TODO lelijk dat dit niet samen met Recording::getPlayers() gebeurt
	// TODO waarom is ::load() uberhaupt iets anders dan de constructor?
	Json::Value metadata = _recording.metadata();
	std::vector<Player> playercolors;
	std::vector<std::string> playernames;
	{
		const Json::Value& json = metadata;
		if (json.isObject())
		{
			if (json["players"].isArray())
			{
				for (auto& playerjson : json["players"])
				{
					if (playerjson["player"].isString())
					{
						playercolors.emplace_back(
							::parsePlayer(playerjson["player"].asString()));
						if (playerjson["username"].isString())
						{
							playernames.emplace_back(
								playerjson["username"].asString());
						}
						else
						{
							playernames.emplace_back(
								::toupper(playerjson["player"].asString()));
						}
					}
				}
			}

			if (json["bots"].isArray())
			{
				for (auto& botjson : json["bots"])
				{
					if (botjson["player"].isString())
					{
						playercolors.emplace_back(
							::parsePlayer(botjson["player"].asString()));
						if (botjson["libraryname"].isString()
							&& botjson["difficulty"].isString()
							&& botjson["displayname"].isString())
						{
							playernames.emplace_back(
								botjson["displayname"].asString()
								+ " ("
								+ ::toupper1(botjson["difficulty"].asString())
								+ " "
								+ botjson["libraryname"].asString()
								+ ")");
						}
						else if (botjson["ainame"].isString()
							&& botjson["difficulty"].isString()
							&& botjson["displayname"].isString())
						{
							playernames.emplace_back(
								botjson["displayname"].asString()
								+ " ("
								+ ::toupper1(botjson["difficulty"].asString())
								+ " "
								+ botjson["ainame"].asString()
								+ ")");
						}
						else
						{
							playernames.emplace_back(
								::toupper(botjson["player"].asString())
								+ " (AI)");
						}
					}
				}
			}
		}
	}

	_automaton.replay(_recording);
	_observer->setSkins(metadata);
	_observer->load();

	for (size_t i = 0; i < playernames.size() && i < playercolors.size(); i++)
	{
		_observer->assignColor(playernames[i], playercolors[i]);
	}
}

void LocalReplay::update()
{
	switch (_phase)
	{
		case Phase::GROWTH:
		case Phase::ACTION:
		{
			if (_automaton.active())
			{
				pipe(_automaton.act());
			}
			else
			{
				_phase = Phase::RESTING;
			}
		}
		break;

		case Phase::RESTING:
		{
			if (_automaton.gameover())
			{
				_phase = Phase::DECAY;
			}
			else if (_automaton.replaying())
			{
				pipe(_automaton.act());
				_phase = Phase::ACTION;
			}
		}
		break;

		case Phase::PLANNING:
		case Phase::STAGING:
		{
			LOGE << "Impossible LocalReplay::_phase";
			DEBUG_ASSERT(false);
		}
		break;

		case Phase::DECAY:
		{
			// Nothing left to do.
		}
		break;
	}

	/* IMGUI */
	static bool show = false;
	bool wasshown = show;

	if (Input::get()->isDebugKeyHeld())
	{
		if (ImGui::Begin("Windows", nullptr,
			ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse))
		{
			ImGui::Checkbox("Commander", &show);
		}
		ImGui::End();
	}

	if (show)
	{
		if (!wasshown) ImGui::SetNextWindowCollapsed(false);

		if (ImGui::Begin("Commander", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::Text("Global score: %u", _automaton.globalScore());
		}
		ImGui::End();
	}

	/* UPDATES */
	{
		_observer->update();
	}
}

bool LocalReplay::online() const
{
	return false;
}

void LocalReplay::sendOrders()
{
	// There are no commanders.
}

void LocalReplay::attemptQuit()
{
	if (_silentConfirmQuit) return confirmQuit();

	if (_observer) _observer->attemptQuit();
}

void LocalReplay::confirmQuit()
{
	_owner.stopGame();
}

void LocalReplay::pipe(const ChangeSet& changeset)
{
	_observer->receiveChanges(changeset.get(Player::OBSERVER));
}

void LocalReplay::chat(const std::string& message, const std::string&)
{
	_observer->message(message);
}

