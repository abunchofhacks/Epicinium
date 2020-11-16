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
#include "aichallenge.hpp"
#include "source.hpp"

#include "difficulty.hpp"


std::string AIChallenge::getKey()
{
	return AIChallenge::getKey(_id);
}

const char* AIChallenge::getKey(const Challenge::Id& id)
{
	switch (id)
	{
		case SHOWCASE: return "showcase_2018_07_04";
		case ELIMINATION: return "elimination_2017_07_20";
		case EVERYTHINGISFREE: return "everythingisfree_2018_07_16";
		case TRAMPLE: return "trample_2017_08_01";
		case TRAMPLE2: return "trample_2018_11_26";
		case HIGHSPEED: return "highspeed_2018_09_21";
		case INVESTMENT: return "investment_2019_04_15";
		case MORALE: return "morale_2020_05_06";
	}
	return "";
}

std::vector<Player> AIChallenge::getPlayers()
{
	switch (_id)
	{
		case SHOWCASE: return ::getPlayers(2);
		case ELIMINATION: return ::getPlayers(2);
		case EVERYTHINGISFREE: return ::getPlayers(2);
		case TRAMPLE: return ::getPlayers(4);
		case TRAMPLE2: return ::getPlayers(4);
		case HIGHSPEED: return ::getPlayers(2);
		case INVESTMENT: return ::getPlayers(2);
		case MORALE: return ::getPlayers(2);
	}
	return {};
}

std::vector<Bot> AIChallenge::getBots()
{
	std::vector<Bot> bots;
	size_t n = AIChallenge::getNumBots(_id);
	bots.reserve(n);
	char slotname[3] = "%""A";
	for (size_t i = 0; i < n; i++)
	{
		bots.emplace_back(slotname, AIChallenge::getBotName(_id),
			AIChallenge::getBotDifficulty(_id));
		slotname[1] += 1;
	}
	return bots;
}

size_t AIChallenge::getNumBots(const Challenge::Id& id)
{
	switch (id)
	{
		case SHOWCASE: return 1;
		case ELIMINATION: return 1;
		case HIGHSPEED: return 1;
		case EVERYTHINGISFREE: return 1;
		case TRAMPLE: return 3;
		case TRAMPLE2: return 3;
		case INVESTMENT: return 1;
		case MORALE: return 1;
	}
	return 0;
}

const char* AIChallenge::getBotName(const Challenge::Id& id)
{
	switch (id)
	{
		case SHOWCASE: return "BrawlingBear";
		case ELIMINATION: return "BrawlingBear";
		case HIGHSPEED: return "BrawlingBear";
		case EVERYTHINGISFREE: return "BrawlingBearFree";
		case TRAMPLE: return "BrawlingBearFree";
		case TRAMPLE2: return "BrawlingBearFree";
		case INVESTMENT: return "Dummy";
		case MORALE: return "BrawlingBearFree";
	}
	return "";
}

Difficulty AIChallenge::getBotDifficulty(const Challenge::Id& id)
{
	switch (id)
	{
		case SHOWCASE: return Difficulty::MEDIUM;
		case ELIMINATION: return Difficulty::MEDIUM;
		case HIGHSPEED: return Difficulty::MEDIUM;
		case EVERYTHINGISFREE: return Difficulty::EASY;
		case TRAMPLE: return Difficulty::MEDIUM;
		case TRAMPLE2: return Difficulty::MEDIUM;
		case INVESTMENT: return Difficulty::NONE;
		case MORALE: return Difficulty::HARD;
	}
	return Difficulty::NONE;
}

std::string AIChallenge::getMapName()
{
	return AIChallenge::getMapName(_id);
}

const char* AIChallenge::getMapName(const Challenge::Id& id)
{
	switch (id)
	{
		case SHOWCASE: return "challenge_showcase";
		case ELIMINATION: return "challenge_elimination";
		case EVERYTHINGISFREE: return "spruce1v1";
		case TRAMPLE: return "challenge_trample";
		case TRAMPLE2: return "challenge_trample";
		case HIGHSPEED: return "oasis1v1";
		case INVESTMENT: return "challenge_investment";
		case MORALE: return "challenge_morale";
	}
	return "";
}

std::string AIChallenge::getRulesetName()
{
	return AIChallenge::getRulesetName(_id);
}

const char* AIChallenge::getRulesetName(const Challenge::Id& id)
{
	switch (id)
	{
		case SHOWCASE: return "";
		case ELIMINATION: return "";
		case EVERYTHINGISFREE: return "challenge_everythingisfree";
		case TRAMPLE: return "challenge_trample";
		case TRAMPLE2: return "challenge_trample";
		case HIGHSPEED: return "challenge_highspeed";
		case INVESTMENT: return "challenge_investment";
		case MORALE: return "challenge_morale";
	}
	return "";
}

std::string AIChallenge::getDisplayName()
{
	return AIChallenge::getDisplayName(_id);
}

std::string AIChallenge::getDisplayName(const Challenge::Id& id)
{
	switch (id)
	{
		case SHOWCASE:
		{
			return _("Showcase");
		}
		break;
		case ELIMINATION:
		{
			return _("Elimination");
		}
		break;
		case EVERYTHINGISFREE:
		{
			return _("Everything is Free");
		}
		break;
		case TRAMPLE:
		case TRAMPLE2:
		{
			return _("Tank Derby");
		}
		break;
		case HIGHSPEED:
		{
			return _("High Speed");
		}
		break;
		case INVESTMENT:
		{
			return _("Economic Growth");
		}
		break;
		case MORALE:
		{
			return _("Morale");
		}
		break;
	}
	return "";
}

std::string AIChallenge::getPanelPictureName()
{
	return AIChallenge::getPanelPictureName(_id);
}

const char* AIChallenge::getPanelPictureName(const Challenge::Id& id)
{
	switch (id)
	{
		case SHOWCASE: return "panels/challenge_showcase";
		case ELIMINATION: return "panels/challenge_elimination";
		case EVERYTHINGISFREE: return "panels/challenge_everythingisfree";
		case TRAMPLE: return "panels/challenge_trample";
		case TRAMPLE2: return "panels/challenge_trample";
		case HIGHSPEED: return "panels/challenge_highspeed";
		case INVESTMENT: return "panels/challenge_investment";
		case MORALE: return "panels/challenge_morale";
	}
	return "";
}

std::string AIChallenge::getDiscordImageKey()
{
	return AIChallenge::getDiscordImageKey(_id);
}

const char* AIChallenge::getDiscordImageKey(const Challenge::Id& id)
{
	switch (id)
	{
		case SHOWCASE: return "challenge_showcase";
		case ELIMINATION: return "challenge_elimination";
		case EVERYTHINGISFREE: return "challenge_everythingisfree";
		case TRAMPLE: return "challenge_trample";
		case TRAMPLE2: return "challenge_trample";
		case HIGHSPEED: return "challenge_highspeed";
		case INVESTMENT: return "challenge_investment";
		case MORALE: return "challenge_morale";
	}
	return "";
}

std::string AIChallenge::getSteamShortKey()
{
	return AIChallenge::getSteamShortKey(_id);
}

const char* AIChallenge::getSteamShortKey(const Challenge::Id& id)
{
	switch (id)
	{
		case SHOWCASE: return "showcase";
		case ELIMINATION: return "elimination";
		case EVERYTHINGISFREE: return "everythingisfree";
		case TRAMPLE: return "trample";
		case TRAMPLE2: return "trample";
		case HIGHSPEED: return "highspeed";
		case INVESTMENT: return "investment";
		case MORALE: return "morale";
	}
	return "";
}

const char* AIChallenge::stringify(const Brief& brief)
{
	switch (brief)
	{
		case Brief::GREETING: return "greeting";
		case Brief::DESCRIPTION: return "description";
		case Brief::OBJECTIVE: return "objective";
		case Brief::FIRST_STAR: return "1";
		case Brief::SECOND_STAR: return "2";
		case Brief::THIRD_STAR: return "3";
		case Brief::SENDOFF: return "sendoff";
	}
	return "";
}

Json::Value AIChallenge::getMissionBriefing()
{
	Json::Value json(Json::objectValue);
	for (size_t i = 0; i < BRIEF_SIZE; i++)
	{
		Brief brief = (Brief) i;
		json[stringify(brief)] = AIChallenge::getBrief(_id, brief);
	}
	return json;
}

std::string AIChallenge::getBrief(const Challenge::Id& id,
	const Brief& brief)
{
	switch (id)
	{
		case SHOWCASE:
		{
			switch (brief)
			{
				case Brief::GREETING: return _(""
				"Listen up, Commander!"
				"");
				case Brief::DESCRIPTION: return _(""
				"The enemy has dug in among the mountains in the east."
				" The longer they are allowed to stay,"
				" the more damage their industriousness will do"
				" to the surrounding area."
				" Use whatever means necessary to obliterate them."
				" There are some neutral City and Industry tiles"
				" in the northwest of the map."
				" Feel free to capture them with your Rifleman units."
				"");
				case Brief::OBJECTIVE: return _(""
				"Defeat your opponent by occupying, capturing"
				" or destroying all of their City tiles."
				" At the end of the game,"
				" you get 1 point for every Grass and Forest tile that remains."
				"");
				// These have to be hardcoded, because they are translated
				// client-side by matching the exact string, and we want
				// challenges to not require client binary patches, only server
				// patches and optionally client translation file updates.
				case Brief::FIRST_STAR: return _(""
				"Get 1 point."
				"");
				case Brief::SECOND_STAR: return _(""
				"Get 25 points."
				"");
				case Brief::THIRD_STAR: return _(""
				"Get 40 points."
				"");
				case Brief::SENDOFF: return _(""
				"Now go out there and lay claim to this region"
				" while there's still something worth claiming!"
				"");
			}
		}
		break;
		case ELIMINATION:
		{
			switch (brief)
			{
				case Brief::GREETING: return _(""
				"Listen up, Commander!"
				"");
				case Brief::DESCRIPTION: return _(""
				"Some of the population of this peninsula has rebelled!"
				" Eliminate all of the rebels in the area"
				" using your tanks, gunners and sappers."
				" Don't worry about capturing cities; they will"
				" fall back in line once you have quelled the rebellion."
				"");
				case Brief::OBJECTIVE: return _(""
				"Defeat your opponent by eliminating all of their units."
				" At the end of the game,"
				" you get 1 point for every Grass and Forest tile that remains."
				"");
				case Brief::FIRST_STAR: return _(""
				"Eliminate all enemy units."
				"");
				case Brief::SECOND_STAR: return _(""
				"Get at least 25 points."
				"");
				case Brief::THIRD_STAR: return _(""
				"Do not destroy any City, Farm or Industry tiles."
				"");
				case Brief::SENDOFF: return _(""
				"Go get rid of those rebels!"
				"");
			}
		}
		break;
		case EVERYTHINGISFREE:
		{
			switch (brief)
			{
				case Brief::GREETING: return _(""
				"Greetings, Commander!"
				"");
				case Brief::DESCRIPTION: return _(""
				"Who needs money anyway?"
				"\n"
				"In this challenge,"
				" all unit and tile costs have been reduced to zero,"
				" and buildings do not generate any income at night."
				"");
				case Brief::OBJECTIVE: return _(""
				"Defeat your opponent by occupying, capturing"
				" or destroying all of their City tiles."
				" At the end of the game,"
				" you get 1 point for every Grass and Forest tile that remains."
				"");
				case Brief::FIRST_STAR: return _(""
				"Get 1 point."
				"");
				case Brief::SECOND_STAR: return _(""
				"Get 30 points."
				"");
				case Brief::THIRD_STAR: return _(""
				"Get 80 points."
				"");
				case Brief::SENDOFF: return _(""
				"Good luck!"
				"");
			}
		}
		break;
		case TRAMPLE:
		case TRAMPLE2:
		{
			switch (brief)
			{
				case Brief::GREETING: return _(""
				"Well hello there, partner!"
				"");
				case Brief::DESCRIPTION: return _(""
				"Are you looking for some no-fuss action?"
				" Do you want to wake up"
				" at the sound of tanks thundering down the open fields?"
				" Then you should partake in the"
				" spectactular Tank Derby!"
				" Hop in one of them beauty's and show 'em what you got!"
				"");
				case Brief::OBJECTIVE: return _(""
				"Defeat all three opponents by destroying their industry"
				" and eliminating all of their tanks."
				" Tanks are cheaper than normal and can do regular attacks,"
				" but only have one hitpoint."
				" you get 1 point for every Grass and Forest tile that remains."
				"");
				case Brief::FIRST_STAR: return _(""
				"Eliminate all enemy tanks."
				"");
				case Brief::SECOND_STAR: return _(""
				"Get at least 4 points."
				"");
				case Brief::THIRD_STAR: return _(""
				"Get at least 10 points."
				"");
				case Brief::SENDOFF: return _(""
				"Let's rumble!"
				"");
			}
		}
		break;
		case HIGHSPEED:
		{
			switch (brief)
			{
				case Brief::GREETING: return _(""
				"Greetings, Commander!"
				"");
				case Brief::DESCRIPTION: return _(""
				"What happens when we, ahum, \"spill\""
				" a year's worth of energy drinks"
				" into the water supply of the entire region?"
				" Let's find out!"
				"\n\n"
				"In this challenge,"
				" all units have their movement speed"
				" increased by 2."
				"");
				case Brief::OBJECTIVE: return _(""
				"Defeat your opponent by occupying, capturing"
				" or destroying all of their City tiles."
				" At the end of the game,"
				" you get 1 point for every Grass and Forest tile that remains."
				"");
				case Brief::FIRST_STAR: return _(""
				"Get 1 point."
				"");
				case Brief::SECOND_STAR: return _(""
				"Get 30 points."
				"");
				case Brief::THIRD_STAR: return _(""
				"Get 80 points."
				"");
				case Brief::SENDOFF: return _(""
				"Good luck!"
				"");
			}
		}
		break;
		case INVESTMENT:
		{
			switch (brief)
			{
				case Brief::GREETING: return _(""
				"Greetings, Commander!"
				"");
				case Brief::DESCRIPTION: return _(""
				"In this challenge,"
				" there are no enemies to fight or opponents to defeat."
				" Just focus on growing your economy."
				"");
				case Brief::OBJECTIVE: return _(""
				"The game ends after the second winter night."
				" Your score is equal to how much money you have at the end."
				"");
				case Brief::FIRST_STAR: return _(""
				"Get 21 points."
				"");
				case Brief::SECOND_STAR: return _(""
				"Get 86 points."
				"");
				case Brief::THIRD_STAR: return _(""
				"Get 132 points."
				"");
				case Brief::SENDOFF: return _(""
				"Good luck!"
				"");
			}
		}
		break;
		case MORALE:
		{
			switch (brief)
			{
				case Brief::GREETING: return _(""
				"Greetings, Commander!"
				"");
				case Brief::DESCRIPTION: return _(""
				"In this challenge you will use \"Morale\" instead of money"
				" and power to produce units,"
				" which is gained on the field of battle."
				" Each enemy unit killed earns you 1 Morale"
				" and each enemy tile you destroy is worth 5 Morale."
				" Capturing a tile gains you 3 Morale"
				" and also lowers enemy Morale by 5."
				"");
				case Brief::OBJECTIVE: return _(""
				"Occupy or destroy your opponent's City to win."
				" Each remaining City tile is worth 2 points"
				" at the end of the game,"
				" and each remaining Barracks, Industry or Airfield tile"
				" is worth 1 point."
				"");
				case Brief::FIRST_STAR: return _(""
				"Get 2 points."
				"");
				case Brief::SECOND_STAR: return _(""
				"Get 5 points."
				"");
				case Brief::THIRD_STAR: return _(""
				"Get 8 points."
				"");
				case Brief::SENDOFF: return _(""
				"Good luck!"
				"");
			}
		}
		break;
	}
	return "";
}
