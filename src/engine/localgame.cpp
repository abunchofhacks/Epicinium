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
#include "localgame.hpp"
#include "source.hpp"

#include "gameowner.hpp"
#include "bot.hpp"
#include "input.hpp"
#include "commander.hpp"
#include "aicommander.hpp"
#include "ailibrary.hpp"
#include "observer.hpp"
#include "cycle.hpp"
#include "aichallenge.hpp"
#include "implodingptr.hpp"
#include "map.hpp"
#include "library.hpp"
#include "settings.hpp"
#include "clock.hpp"


LocalGame::LocalGame(GameOwner& owner, Settings& settings,
		std::shared_ptr<AIChallenge> challenge,
		const std::vector<Player>& players,
		const std::vector<Bot>& bots,
		const std::string& mapname,
		const std::string& rulesetname,
		unsigned int humanplayers,
		bool silentConfirmQuit,
		bool enableRecording) :
	_owner(owner),
	_settings(settings),
	_mapname(mapname),
	_rulesetname(rulesetname.empty()
		? Library::nameCurrentBible()
		: rulesetname),
	_automaton(players, _rulesetname),
	_phase(Phase::GROWTH),
	_challenge(challenge),
	_players(players),
	_bots(bots),
	_activeindex(0),
	_silentConfirmQuit(silentConfirmQuit),
	_enableRecording(enableRecording)
{
	if (_settings.seed.defined())
	{
		_seed = _settings.seed.value();
	}
	else
	{
		auto timestampMs = SteadyClock::milliseconds();
		_seed = timestampMs;
	}
	srand(_seed);

	if (_challenge)
	{
		_automaton.setChallenge(_challenge);
	}

	_commanders.reserve(_players.size());
	for (const Player& player : _players)
	{
		if (_commanders.size() < humanplayers)
		{
			_commanders.emplace_back(
				new Commander(_settings, *this, player, _rulesetname));
		}
		else if (_aicommanders.size() < _bots.size())
		{
			_aicommanders.emplace_back(
				_bots[_aicommanders.size()].createAI(player, _rulesetname));
		}
		else
		{
			_commanders.emplace_back(
				new Commander(_settings, *this, player, _rulesetname));
		}
	}

	_blind.reset(new Observer(_settings, *this, _rulesetname));
	_observer.reset(new Observer(_settings, *this, _rulesetname));
	if (humanplayers <= 0) _activeindex = -2;
}

LocalGame::LocalGame(GameOwner& owner, Settings& settings,
		const std::vector<Player>& players,
		const std::vector<Bot>& bots,
		const std::string& mapname,
		const std::string& rulesetname,
		unsigned int humanplayers,
		bool silentConfirmQuit,
		bool enableRecording) :
	LocalGame(owner, settings,
		nullptr,
		players,
		bots,
		mapname,
		rulesetname,
		humanplayers,
		silentConfirmQuit,
		enableRecording)
{}

LocalGame::LocalGame(GameOwner& owner, Settings& settings,
		std::shared_ptr<AIChallenge> challenge,
		const std::vector<Player>& players,
		const std::vector<Bot>& bots,
		const std::string& mapname,
		unsigned int humanplayers,
		bool silentConfirmQuit,
		bool enableRecording) :
	LocalGame(owner, settings,
		challenge,
		players,
		bots,
		mapname,
		challenge->getRulesetName(),
		humanplayers,
		silentConfirmQuit,
		enableRecording)
{}

LocalGame::LocalGame(GameOwner& owner, Settings& settings,
		const std::vector<Player>& players,
		const std::vector<Bot>& bots,
		const std::string& mapname,
		unsigned int humanplayers,
		bool silentConfirmQuit,
		bool enableRecording) :
	LocalGame(owner, settings,
		players,
		bots,
		mapname,
		"",
		humanplayers,
		silentConfirmQuit,
		enableRecording)
{}

LocalGame::LocalGame(GameOwner& owner, Settings& settings,
		std::shared_ptr<AIChallenge> challenge,
		bool silentConfirmQuit,
		bool enableRecording) :
	LocalGame(owner, settings,
		challenge,
		challenge->getPlayers(),
		challenge->getBots(),
		challenge->getMapName(),
		1,
		silentConfirmQuit,
		enableRecording)
{}

void LocalGame::load()
{
	Json::Value metadata;

	metadata = Map::loadMetadata(_mapname);
	metadata["map"] = _mapname;

	metadata["online"] = false;

	metadata["players"] = Json::arrayValue;
	metadata["bots"] = Json::arrayValue;
	for (const auto& commander : _commanders)
	{
		Json::Value json = Json::objectValue;
		json["player"] = ::stringify(commander->player());
		metadata["players"].append(json);
	}
	for (const auto& ai : _aicommanders)
	{
		Json::Value json = Json::objectValue;
		json["player"] = ::stringify(ai->player());
		json["character"] = ai->characterstring();
		json["displayname"] = ai->displayname();
		json["ainame"] = ai->ainame();
		json["authors"] = ai->authors();
		metadata["bots"].append(json);
	}
	for (const auto& ai : _ailibraries)
	{
		Json::Value json = Json::objectValue;
		json["player"] = ::stringify(ai->player());
		json["character"] = ai->characterstring();
		json["displayname"] = ai->displayname();
		json["ainame"] = ai->ainame();
		json["authors"] = ai->authors();
		metadata["bots"].append(json);
	}

	_automaton.load(_mapname, /*shufflePlayers=*/false);
	if (_enableRecording)
	{
		_automaton.startRecording(metadata);
	}
	for (const auto& commander : _commanders)
	{
		commander->setSkins(metadata);
		commander->load();
	}
	_blind->setSkins(metadata);
	_blind->load();
	_observer->setSkins(metadata);
	_observer->load();

	for (const auto& player : _commanders)
	{
		std::string name = ::colorPlayerName(player->player());
		for (const auto& commander : _commanders)
		{
			commander->assignColor(name, player->player());
		}
		{
			_blind->assignColor(name, player->player());
			_observer->assignColor(name, player->player());
		}
	}
	for (const auto& ai : _aicommanders)
	{
		std::string name = ai->descriptivename();
		for (const auto& commander : _commanders)
		{
			commander->assignColor(name, ai->player());
		}
		{
			_blind->assignColor(name, ai->player());
			_observer->assignColor(name, ai->player());
		}
	}
	for (const auto& ai : _ailibraries)
	{
		std::string name = ai->descriptivename();
		for (const auto& commander : _commanders)
		{
			commander->assignColor(name, ai->player());
		}
		{
			_blind->assignColor(name, ai->player());
			_observer->assignColor(name, ai->player());
		}
	}

	if (_challenge)
	{
		Json::Value briefing = _challenge->getMissionBriefing();
		if (metadata["challenge"].isObject()
			&& metadata["challenge"]["briefing"].isObject())
		{
			briefing = metadata["challenge"]["briefing"];
		}
		for (const auto& commander : _commanders)
		{
			commander->receiveBriefing(briefing);
		}
		if (_blind) _blind->receiveBriefing(briefing);
		if (_observer) _observer->receiveBriefing(briefing);
	}
}

void LocalGame::update()
{
	switch (_phase)
	{
		case Phase::GROWTH:
		case Phase::ACTION:
		{
			if (_automaton.active())
			{
				stepActionPhase();
			}
			else
			{
				leaveActionPhase();
			}
		}
		break;

		case Phase::RESTING:
		{
			// Game over.
		}
		break;

		case Phase::PLANNING:
		{
			// Allow the AIs to calculate their next move.
			for (const auto& ai: _aicommanders)
			{
				ai->prepareOrders();
			}
			for (const auto& ai: _ailibraries)
			{
				ai->prepareOrders();
			}

			// Force AIs to play.
			// TODO Change to a button in the observer UI that is only active in planning phase.
			if (_commanders.empty()
				&& (Input::get()->wasKeyPressed(SDL_SCANCODE_SPACE)
					|| Input::get()->isKeyLongHeld(SDL_SCANCODE_SPACE)))
			{
				sendOrders();
			}
		}
		break;

		case Phase::DECAY:
		case Phase::STAGING:
		{
			LOGE << "Impossible LocalGame::_phase";
			DEBUG_ASSERT(false);
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
			for (size_t i = 0; i < _commanders.size(); i++)
			{
				static constexpr const char* names[] = {
					"Alice",
					"Bob",
					"Carol",
					"Dave",
					"Emma",
					"Frank",
					"Gwen",
					"Harold",
				};
				ImGui::RadioButton(names[i], &_activeindex,  i);
			}
			ImGui::RadioButton("Blind", &_activeindex, -1);
			ImGui::RadioButton("Observer", &_activeindex, -2);
			ImGui::Separator();
			ImGui::Text("Global score: %u", _automaton.globalScore());
			ImGui::Separator();
			int seed = _seed;
			ImGui::InputInt("Seed", &seed);
			if (_settings.seed.defined())
			{
				if (seed != _settings.seed.value())
				{
					_settings.seed.override(seed);
					_seed = seed;
				}
			}
			else
			{
				if (seed != _seed)
				{
					_settings.seed.override(seed);
					_seed = seed;
				}
			}
			if (ImGui::Button("Reload"))
			{
				_owner.startGame(imploding_ptr<Game>(new LocalGame(
					_owner, _settings,
					_players, _bots, _mapname, _commanders.size(),
					_silentConfirmQuit, _enableRecording)));
			}
		}
		ImGui::End();
	}

	/* UPDATES */
	if (_activeindex >= 0 && _activeindex < (int) _commanders.size())
	{
		_commanders[_activeindex]->update();
	}
	else if (_activeindex == -2)
	{
		_observer->update();
	}
	else
	{
		_blind->update();
	}
}

bool LocalGame::online() const
{
	return false;
}

void LocalGame::sendOrders()
{
	if (!_automaton.active())
	{
		enterActionPhase();
	}
}

void LocalGame::attemptQuit()
{
	if (_silentConfirmQuit) return confirmQuit();

	for (const auto& commander : _commanders)
	{
		commander->attemptQuit();
	}

	if (_blind) _blind->attemptQuit();

	if (_observer) _observer->attemptQuit();
}

void LocalGame::confirmQuit()
{
	_owner.stopGame();
}

void LocalGame::enterActionPhase()
{
	pipe(_automaton.awake());

	_phase = Phase::STAGING;

	for (const auto& ai: _aicommanders)
	{
		_automaton.receive(ai->player(), ai->orders());
	}
	for (const auto& ai: _ailibraries)
	{
		_automaton.receive(ai->player(), ai->orders(_automaton.bible()));
	}
	for (const auto& commander : _commanders)
	{
		_automaton.receive(commander->player(), commander->orders());
	}

	pipe(_automaton.prepare());

	_phase = Phase::ACTION;
}

void LocalGame::stepActionPhase()
{
	pipe(_automaton.act());
}

void LocalGame::pipe(const ChangeSet& changeset)
{
	for (const auto& ai: _aicommanders)
	{
		ai->receiveChanges(changeset.get(ai->player()));
	}
	for (const auto& ai: _ailibraries)
	{
		ai->receiveChanges(changeset.get(ai->player()), _automaton.bible());
	}
	for (const auto& commander : _commanders)
	{
		commander->receiveChanges(changeset.get(commander->player()));
	}
	_blind->receiveChanges(changeset.get(Player::BLIND));
	_observer->receiveChanges(changeset.get(Player::OBSERVER));
}

void LocalGame::leaveActionPhase()
{
	_phase = Phase::RESTING;

	if (_automaton.gameover()) return;

	pipe(_automaton.hibernate());

	_phase = Phase::PLANNING;
}

void LocalGame::chat(const std::string& message, const std::string&)
{
	for (const auto& commander : _commanders)
	{
		commander->message(message);
	}
	_observer->message(message);
}

