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

#include "challenge.hpp"
#include "player.hpp"
#include "bot.hpp"


class AIChallenge : public Challenge
{
public:
	enum class Brief : uint8_t
	{
		GREETING,
		DESCRIPTION,
		OBJECTIVE,
		FIRST_STAR,
		SECOND_STAR,
		THIRD_STAR,
		SENDOFF,
	};
	static constexpr size_t BRIEF_SIZE = ((size_t) Brief::SENDOFF) + 1;
	static const char* stringify(const Brief& brief);

	using Challenge::Challenge;

	AIChallenge(const Challenge& that) : Challenge(that) {}
	AIChallenge(Challenge&& that) : Challenge(that) {}
	AIChallenge& operator=(const Challenge&) = delete;
	AIChallenge& operator=(Challenge&&) = delete;

public:
	std::string getKey();
	std::vector<Player> getPlayers();
	std::vector<Bot> getBots();
	std::string getMapName();
	std::string getRulesetName();
	std::string getDisplayName();
	std::string getPanelPictureName();
	std::string getDiscordImageKey();
	std::string getSteamShortKey();
	Json::Value getMissionBriefing();

	static const char* getKey(const Challenge::Id& id);
	static size_t getNumBots(const Challenge::Id& id);
	static const char* getBotName(const Challenge::Id& id);
	static Difficulty getBotDifficulty(const Challenge::Id& id);
	static const char* getMapName(const Challenge::Id& id);
	static const char* getRulesetName(const Challenge::Id& id);
	static const char* getDisplayName(const Challenge::Id& id);
	static const char* getPanelPictureName(const Challenge::Id& id);
	static const char* getDiscordImageKey(const Challenge::Id& id);
	static const char* getSteamShortKey(const Challenge::Id& id);
	static const char* getBrief(const Challenge::Id& id, const Brief& brief);
};
