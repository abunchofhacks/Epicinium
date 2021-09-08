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
		case CUSTOM: return "";
		// OLD: "trample_2017_08_01";
		case SHOWCASE: return "showcase_2018_07_04";
		case ELIMINATION: return "elimination_2017_07_20";
		case EVERYTHINGISFREE: return "everythingisfree_2018_07_16";
		case TRAMPLE: return "trample_2018_11_26";
		case HIGHSPEED: return "highspeed_2018_09_21";
		case INVESTMENT: return "investment_2019_04_15";
		case MORALE: return "morale_2020_05_06";
		case ACT1A: return "act_1a_2021_07_14";
		case ACT1B: return "act_1b_2021_07_14";
		case ACT1C: return "act_1c_2021_07_14";
		case ACT1D: return "act_1d_2021_07_14";
		case ACT1E: return "act_1e_2021_07_14";
		case ACT1F: return "act_1f_2021_07_14";
		case ACT2A: return "act_2a_2021_07_14";
		case ACT2B: return "act_2b_2021_07_14";
		case ACT2C: return "act_2c_2021_07_14";
		case ACT2D: return "act_2d_2021_07_14";
		case ACT2E: return "act_2e_2021_07_14";
		case ACT3A: return "act_3a_2021-07-25";
		case ACT3B: return "act_3b_2021-07-25";
		case ACT3C: return "act_3c_2021-07-25";
		case ACT3D: return "act_3d_2021-07-25";
		case ACT3E: return "act_3e_2021-07-25";
		case ACT3F: return "act_3f_2021-07-25";
		case ACT3G: return "act_3g_2021-07-25";
	}
	return "";
}

std::vector<Player> AIChallenge::getPlayers()
{
	switch (_id)
	{
		case TRAMPLE: return ::getPlayers(4);
		default: return ::getPlayers(2);
	}
	return ::getPlayers(2);
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
		case TRAMPLE: return 3;
		default: return 1;
	}
	return 0;
}

const char* AIChallenge::getBotName(const Challenge::Id& id)
{
	switch (id)
	{
		case CUSTOM: return "BrawlingBear";
		case SHOWCASE: return "BrawlingBear";
		case ELIMINATION: return "BrawlingBear";
		case HIGHSPEED: return "BrawlingBear";
		case EVERYTHINGISFREE: return "BrawlingBearFree";
		case TRAMPLE: return "BrawlingBearFree";
		case INVESTMENT: return "Dummy";
		case MORALE: return "BrawlingBearFree";
		case ACT1A: return "StoryStarfish";
		case ACT1B: return "StoryStarfish";
		case ACT1C: return "StoryStarfish";
		case ACT1D: return "StorySalmon";
		case ACT1E: return "StorySeahorse";
		case ACT1F: return "StorySeabass";
		case ACT2A: return "StorySaddlehead";
		case ACT2B: return "StorySandperch";
		case ACT2C: return "StorySturgeon";
		case ACT2D: return "StoryStingray";
		case ACT2E: return "StorySawfish";
		case ACT3A: return "ActingAnchovies";
		case ACT3B: return "ActingAlpaca";
		case ACT3C: return "ActingAardvark";
		case ACT3D: return "ActingArmadillo";
		case ACT3E: return "ActingAlbatross";
		case ACT3F: return "ActingAntilope";
		case ACT3G: return "ActingAlligator";
	}
	return "";
}

Difficulty AIChallenge::getBotDifficulty(const Challenge::Id& id)
{
	switch (id)
	{
		case CUSTOM: return Difficulty::MEDIUM;
		case SHOWCASE: return Difficulty::MEDIUM;
		case ELIMINATION: return Difficulty::MEDIUM;
		case HIGHSPEED: return Difficulty::MEDIUM;
		case EVERYTHINGISFREE: return Difficulty::EASY;
		case TRAMPLE: return Difficulty::MEDIUM;
		case INVESTMENT: return Difficulty::NONE;
		case MORALE: return Difficulty::HARD;
		case ACT1A: return Difficulty::EASY;
		case ACT1B: return Difficulty::MEDIUM;
		case ACT1C: return Difficulty::HARD;
		case ACT1D: return Difficulty::HARD;
		case ACT1E: return Difficulty::HARD;
		case ACT1F: return Difficulty::HARD;
		case ACT2A: return Difficulty::HARD;
		case ACT2B: return Difficulty::HARD;
		case ACT2C: return Difficulty::HARD;
		case ACT2D: return Difficulty::HARD;
		case ACT2E: return Difficulty::HARD;
		case ACT3A: return Difficulty::HARD;
		case ACT3B: return Difficulty::HARD;
		case ACT3C: return Difficulty::HARD;
		case ACT3D: return Difficulty::HARD;
		case ACT3E: return Difficulty::HARD;
		case ACT3F: return Difficulty::HARD;
		case ACT3G: return Difficulty::HARD;
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
		case CUSTOM: return "challenge_custom";
		case SHOWCASE: return "challenge_showcase";
		case ELIMINATION: return "challenge_elimination";
		case EVERYTHINGISFREE: return "spruce1v1";
		case TRAMPLE: return "challenge_trample";
		case HIGHSPEED: return "oasis1v1";
		case INVESTMENT: return "challenge_investment";
		case MORALE: return "challenge_morale";
		case ACT1A: return "Act1a";
		case ACT1B: return "Act1b";
		case ACT1C: return "Act1c";
		case ACT1D: return "Act1d";
		case ACT1E: return "Act1e";
		case ACT1F: return "Act1f";
		case ACT2A: return "Act2a";
		case ACT2B: return "Act2b";
		case ACT2C: return "Act2c";
		case ACT2D: return "Act2d";
		case ACT2E: return "Act2e";
		case ACT3A: return "Act3a";
		case ACT3B: return "Act3b";
		case ACT3C: return "Act3c";
		case ACT3D: return "Act3d";
		case ACT3E: return "Act3e";
		case ACT3F: return "Act3f";
		case ACT3G: return "Act3g";
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
		case CUSTOM: return "challenge_custom";
		case SHOWCASE: return "";
		case ELIMINATION: return "";
		case EVERYTHINGISFREE: return "challenge_everythingisfree";
		case TRAMPLE: return "challenge_trample";
		case HIGHSPEED: return "challenge_highspeed";
		case INVESTMENT: return "challenge_investment";
		case MORALE: return "challenge_morale";
		case ACT1A: return "Act1a";
		case ACT1B: return "Act1b";
		case ACT1C: return "Act1c";
		case ACT1D: return "Act1d";
		case ACT1E: return "Act1e";
		case ACT1F: return "Act1f";
		case ACT2A: return "Act2a";
		case ACT2B: return "Act2b";
		case ACT2C: return "Act2c";
		case ACT2D: return "Act2d";
		case ACT2E: return "Act2e";
		case ACT3A: return "Act3a";
		case ACT3B: return "Act3b";
		case ACT3C: return "Act3c";
		case ACT3D: return "Act3d";
		case ACT3E: return "Act3e";
		case ACT3F: return "Act3f";
		case ACT3G: return "Act3g";
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
		case CUSTOM:
		case SHOWCASE:
		{
			return _("Preservation Challenge");
		}
		break;
		case ELIMINATION:
		{
			return _("Elimination Challenge");
		}
		break;
		case EVERYTHINGISFREE:
		{
			return _("Everything is Free Challenge");
		}
		break;
		case TRAMPLE:
		{
			return _("Tank Derby Challenge");
		}
		break;
		case HIGHSPEED:
		{
			return _("High Speed Challenge");
		}
		break;
		case INVESTMENT:
		{
			return _("Economic Growth Challenge");
		}
		break;
		case MORALE:
		{
			return _("Morale Challenge");
		}
		break;
		case ACT1A: return ::format(
			_("Act %d - Level %d"),
			1, 1);
		case ACT1B: return ::format(
			_("Act %d - Level %d"),
			1, 2);
		case ACT1C: return ::format(
			_("Act %d - Level %d"),
			1, 3);
		case ACT1D: return ::format(
			_("Act %d - Level %d"),
			1, 4);
		case ACT1E: return ::format(
			_("Act %d - Level %d"),
			1, 5);
		case ACT1F: return ::format(
			_("Act %d - Level %d"),
			1, 6);
		case ACT2A: return ::format(
			_("Act %d - Level %d"),
			2, 1);
		case ACT2B: return ::format(
			_("Act %d - Level %d"),
			2, 2);
		case ACT2C: return ::format(
			_("Act %d - Level %d"),
			2, 3);
		case ACT2D: return ::format(
			_("Act %d - Level %d"),
			2, 4);
		case ACT2E: return ::format(
			_("Act %d - Level %d"),
			2, 5);
		case ACT3A: return ::format(
			_("Act %d - Level %d"),
			3, 1);
		case ACT3B: return ::format(
			_("Act %d - Level %d"),
			3, 2);
		case ACT3C: return ::format(
			_("Act %d - Level %d"),
			3, 3);
		case ACT3D: return ::format(
			_("Act %d - Level %d"),
			3, 4);
		case ACT3E: return ::format(
			_("Act %d - Level %d"),
			3, 5);
		case ACT3F: return ::format(
			_("Act %d - Level %d"),
			3, 6);
		case ACT3G: return ::format(
			_("Act %d - Level %d"),
			3, 7);
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
		case CUSTOM: return "panels/challenge_showcase";
		case SHOWCASE: return "panels/challenge_showcase";
		case ELIMINATION: return "panels/challenge_elimination";
		case EVERYTHINGISFREE: return "panels/challenge_everythingisfree";
		case TRAMPLE: return "panels/challenge_trample";
		case HIGHSPEED: return "panels/challenge_highspeed";
		case INVESTMENT: return "panels/challenge_investment";
		case MORALE: return "panels/challenge_morale";
		case ACT1A: return "panels/Act1a";
		case ACT1B: return "panels/Act1b";
		case ACT1C: return "panels/Act1c";
		case ACT1D: return "panels/Act1d";
		case ACT1E: return "panels/Act1e";
		case ACT1F: return "panels/Act1f";
		case ACT2A: return "panels/Act2a";
		case ACT2B: return "panels/Act2b";
		case ACT2C: return "panels/Act2c";
		case ACT2D: return "panels/Act2d";
		case ACT2E: return "panels/Act2e";
		case ACT3A: return "panels/Act3a";
		case ACT3B: return "panels/Act3b";
		case ACT3C: return "panels/Act3c";
		case ACT3D: return "panels/Act3d";
		case ACT3E: return "panels/Act3e";
		case ACT3F: return "panels/Act3f";
		case ACT3G: return "panels/Act3g";
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
		case CUSTOM: return "challenge_showcase";
		case SHOWCASE: return "challenge_showcase";
		case ELIMINATION: return "challenge_elimination";
		case EVERYTHINGISFREE: return "challenge_everythingisfree";
		case TRAMPLE: return "challenge_trample";
		case HIGHSPEED: return "challenge_highspeed";
		case INVESTMENT: return "challenge_investment";
		case MORALE: return "challenge_morale";
		case ACT1A: return "campaign";
		case ACT1B: return "campaign";
		case ACT1C: return "campaign";
		case ACT1D: return "campaign";
		case ACT1E: return "campaign";
		case ACT1F: return "campaign";
		case ACT2A: return "campaign";
		case ACT2B: return "campaign";
		case ACT2C: return "campaign";
		case ACT2D: return "campaign";
		case ACT2E: return "campaign";
		case ACT3A: return "campaign";
		case ACT3B: return "campaign";
		case ACT3C: return "campaign";
		case ACT3D: return "campaign";
		case ACT3E: return "campaign";
		case ACT3F: return "campaign";
		case ACT3G: return "campaign";
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
		std::string txt = AIChallenge::getBrief(_id, brief);
		if (!txt.empty())
		{
			json[stringify(brief)] = txt;
		}
	}
	return json;
}

std::string AIChallenge::getBrief(const Challenge::Id& id,
	const Brief& brief)
{
	switch (id)
	{
		case CUSTOM:
		{
			switch (brief)
			{
				case Brief::GREETING: return _(""
				"Greetings, Commander!"
				"");
				case Brief::DESCRIPTION: return "";
				case Brief::OBJECTIVE: return "";
				// These have to be hardcoded, because they are translated
				// client-side by matching the exact string, and we want
				// challenges to not require client binary patches, only server
				// patches and optionally client translation file updates.
				case Brief::FIRST_STAR: return _(""
				"Defeat the enemy."
				"");
				case Brief::SECOND_STAR: return "";
				case Brief::THIRD_STAR: return "";
				case Brief::SENDOFF: return _(""
				"Good luck!"
				"");
			}
		}
		break;
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
		case ACT1A:
		{
			switch (brief)
			{
				case Brief::GREETING: return _(""
				"Commander!"
				"");
				case Brief::DESCRIPTION: return _(""
				"Our batallion has been stranded on this peninsula with very"
				" little resources."
				" However, there is an enemy city to the east."
				" If we manage to reach that city, we should be able to use it"
				" to our advantage! Destroy any enemies that block your path!"
				"");
				case Brief::OBJECTIVE: return _(""
				"You can give units orders by selecting them, then clicking on"
				" the map to move them. Units that encounter enemies will"
				" automatically shoot at those enemies."
				" Your only current units are militia, which have a movement"
				" speed of 3 and 1 hitpoint."
				" In combat, each militia shoots once."
				"");
				case Brief::FIRST_STAR: return _(""
				"Occupy the enemy city."
				"");
				case Brief::SECOND_STAR: return "";
				case Brief::THIRD_STAR: return "";
				case Brief::SENDOFF: return _(""
				"Good luck!"
				"");
			}
		}
		break;
		case ACT1B:
		{
			switch (brief)
			{
				case Brief::GREETING: return _(""
				"Commander!"
				"");
				case Brief::DESCRIPTION: return _(""
				"Unfortunately, the enemy knew they would lose the previous"
				" battle and emptied their city of any resource."
				" However, we did manage to rescue some captured allies!"
				" These powerful riflemen are more durable than our militia."
				" A rifleman's armor reduces their movement speed by 1"
				" but it takes 2 hits to kill them."
				"");
				case Brief::OBJECTIVE: return _(""
				"Remember that after every combat all units regain their full"
				" hit points! So if you want to take down an enemy rifleman,"
				" you might want to surround it with your own units and use"
				" their focus ability by right-clicking on them. Focus causes"
				" all allied troops around the enemy to attack it"
				" simultaniously, allowing groups of weaker units to take out"
				" even the sturdiest of foes!"
				" We also found an enemy secret note in the city."
				" It reads:"
				" \"hold the left mouse button to speed up animations\"."
				"");
				case Brief::FIRST_STAR: return _(""
				"Capture or occupy all enemy cities."
				"");
				case Brief::SECOND_STAR: return "";
				case Brief::THIRD_STAR: return "";
				case Brief::SENDOFF: return _(""
				"Good luck!"
				"");
			}
		}
		break;
		case ACT1C:
		{
			switch (brief)
			{
				case Brief::GREETING: return _(""
				"Greetings, commander."
				"");
				case Brief::DESCRIPTION: return _(""
				"Taking those enemy cities has won us a valuable strategic"
				" position! Some of the enemy troops have fled into a nearby"
				" mountain range. If they reach their city at the end of this"
				" range, they might be able to regroup! We should prevent this"
				" from happening by routing them all, and capturing their city"
				" in the progress."
				"");
				case Brief::OBJECTIVE: return _(""
				"However, some troops might be waiting to trap us in the"
				" canyon. If we pass by an enemy unit or run away from one,"
				" it will shoot at us. We can prevent this attack of"
				" opportunity by striking first! An attack of opportunity"
				" is also prevented if one of our own units is stationed on"
				" the tile! This allows us to bypass enemy units without any"
				" combat at all. They also have units that hide in the forests"
				" or trenches, which give shelter from gunfire. Our riflemen"
				" can also make trenches, use them wisely!"
				"");
				case Brief::FIRST_STAR: return _(""
				"Capture or occupy all enemy cities."
				"");
				case Brief::SECOND_STAR: return "";
				case Brief::THIRD_STAR: return "";
				case Brief::SENDOFF: return _(""
				"Good luck!"
				"");
			}
		}
		break;
		case ACT1D:
		{
			switch (brief)
			{
				case Brief::GREETING: return _(""
				"Commander!"
				"");
				case Brief::DESCRIPTION: return _(""
				"Our previously captured city is flourishing, which will aid"
				" us in our future battles! Cities gain power every night up"
				" to a maximum of 5. If there are less than 5 buildings in"
				" the city, it even constructs a new building! Cities also"
				" generate money every night, 1 gold for each power!"
				" Creating a militia costs 10 gold. The number of bodies in"
				" your militia unit depends on the power of the city, with a"
				" maximum of 3."
				" For each body created, the city loses one power."
				" Be sure to find a balance!"
				"");
				case Brief::OBJECTIVE: return _(""
				"Cities can also expand into barracks, which gain power only"
				" when the city has at least equal power in it! Barracks cost"
				" a lot to upgrade, but larger stacks of riflemen might be"
				" worth it! Barracks can be upgraded twice to contain a"
				" maximum of 3 buildings."
				" Use this to your advantage, but don't wait to long with"
				" offense as the enemy is trying to take back their city!"
				"");
				case Brief::FIRST_STAR: return _(""
				"Capture or occupy all enemy cities."
				"");
				case Brief::SECOND_STAR: return "";
				case Brief::THIRD_STAR: return "";
				case Brief::SENDOFF: return _(""
				"Good luck!"
				"");
			}
		}
		break;
		case ACT1E:
		{
			switch (brief)
			{
				case Brief::GREETING: return _(""
				"Commander!"
				"");
				case Brief::DESCRIPTION: return _(""
				"The cities that we captured in our last victory contained"
				" some valuable technology! We are now able to create farms,"
				" which cultivate the surrounding terrain into fertile soil."
				" Soil turns into crops, which generate money every night as"
				" long as the ground is not frozen."
				" Farms are created by our new settler unit, which can be"
				" created in the city. Farms are not only very good for"
				" making money, they can also be used to create new militia at"
				" a cheaper price."
				"");
				case Brief::OBJECTIVE: return _(""
				"There is one caveat, however. Farms take up 9 total tiles,"
				" destroying the environment. If you make too many of them,"
				" you might be victorious only to win a barren wasteland."
				" Placing farms next to cities also reduces the maximum power"
				" in a city, as cities need grass and forests to grow."
				" Farms contain a maximum of 2 buildings, limiting the size"
				" of militia."
				" Rout the enemy, but take care not to destroy your future"
				" in the progress."
				"");
				case Brief::FIRST_STAR: return _(""
				"Capture or occupy all enemy cities."
				"");
				case Brief::SECOND_STAR: return _(""
				"Get at least 75 points."
				"");
				case Brief::THIRD_STAR: return "";
				case Brief::SENDOFF: return _(""
				"Good luck!"
				"");
			}
		}
		break;
		case ACT1F:
		{
			switch (brief)
			{
				case Brief::GREETING: return _(""
				"Commander!"
				"");
				case Brief::DESCRIPTION: return _(""
				"We have located some allied troops stuck on an enemy island."
				" A rescue mission is impossible, but we have resources to"
				" aid them in capturing the island! Their outpost seems to"
				" have not been spotted yet, but enemy scouts probably reach"
				" them in 2 years."
				" Our allies only have a basic outpost at the moment,"
				" which is able to create settlers and militia."
				" Fortunately, they have better technology than we do so"
				" their settlers know how to create whole new towns,"
				" which upgrade into cities!"
				"");
				case Brief::OBJECTIVE: return _(""
				"Our allies' militia also know how to create basic outposts,"
				" which when positioned offensively might help in occupying"
				" the enemy cities!"
				" The enemy cities contain valuable research labs,"
				" so keeping this island from being destroyed is of crucial"
				" importance! Build a base to destroy the enemy, but remember"
				" that cities need grassy tiles surrounding them to grow!"
				" There are rumours of the enemy having perfected a super"
				" soldier, able to take out almost anything in a single shot."
				" Beware, commander."
				"");
				case Brief::FIRST_STAR: return _(""
				"Capture or occupy all enemy cities."
				"");
				case Brief::SECOND_STAR: return _(""
				"Get at least 85 points."
				"");
				case Brief::THIRD_STAR: return _(""
				"Get at least 100 points."
				"");
				case Brief::SENDOFF: return _(""
				"Good luck!"
				"");
			}
		}
		break;
		case ACT2A:
		{
			switch (brief)
			{
				case Brief::GREETING: return _(""
				"Commander!"
				"");
				case Brief::DESCRIPTION: return _(""
				"We have secured the enemy's technology for ourselves,"
				" allowing us to create gunners in barracks!"
				" Gunners have the lockdown ability, which locks down a tile"
				" completely. Every enemy moving onto that tile is attacked"
				" by the gunner without chance of retalliation."
				" Their movement is also interrupted."
				" This makes the gunner a very strong defensive unit!"
				"");
				case Brief::OBJECTIVE: return _(""
				"With this new technology, we might be able to take back more"
				" of our homeland! Let us start to the south. These cities"
				" store our islands explosives. If we are to capture it, we"
				" might use it for ourselves!"
				" Beware, as the enemy might use it as well!"
				"");
				case Brief::FIRST_STAR: return _(""
				"Capture or occupy all enemy cities."
				"");
				case Brief::SECOND_STAR: return "";
				case Brief::THIRD_STAR: return "";
				case Brief::SENDOFF: return _(""
				"Good luck!"
				"");
			}
		}
		break;
		case ACT2B:
		{
			switch (brief)
			{
				case Brief::GREETING: return _(""
				"Commander!"
				"");
				case Brief::DESCRIPTION: return _(""
				"We have managed to secure enough explosives to produce our"
				" own sappers at the barracks. Sappers are able to bombard"
				" far away tiles, destroying any unit or building in a"
				" single hit! This will help greatly in these mountains,"
				" where our movement is hindered by the snow. Especially"
				" in winters, this part of the island is very hard to traverse."
				" Snow also renders our farms useless, luckily the small ports"
				" on the nearby islands will provide us with some resources."
				"");
				case Brief::OBJECTIVE: return _(""
				"We have had word from our scouts that the enemy has managed"
				" to create tanks from the industry in the southwest."
				" Our new sappers could help in destroying these heavily"
				" armored vehicles! We have not yet sighted the enemy,"
				" but their tanks could arrive at our cities soon!"
				" Be sure to prepare well."
				" While sappers and tanks have strong abilities,"
				" their bombs have a chance to miss enemy units."
				" Be sure to calculate the risk you are willing to take!"
				"");
				case Brief::FIRST_STAR: return _(""
				"Capture, destroy or occupy all enemy cities."
				"");
				case Brief::SECOND_STAR: return "";
				case Brief::THIRD_STAR: return "";
				case Brief::SENDOFF: return _(""
				"Good luck!"
				"");
			}
		}
		break;
		case ACT2C:
		{
			switch (brief)
			{
				case Brief::GREETING: return _(""
				"Commander!"
				"");
				case Brief::DESCRIPTION: return _(""
				"We have captured back our industry!"
				" We can now create more industry from cities."
				" Industry are able to create tanks, which cannot use"
				" gunfire but have the powerful shell ability."
				" This shoots two bombs per tank at an adjacent tile,"
				" destroying units and buildings alike! Industry also generate"
				" 3 gold each turn, making them a suitable alternative to"
				" farms in these cold mountains. Beware though, as too much"
				" industry dries out the surrounding area,"
				" limiting power in the city."
				"");
				case Brief::OBJECTIVE: return _(""
				"These mountains are even colder than the previous,"
				" especially if the many industrial buildings in this area"
				" impact the climate! Next to slowing us down this cold"
				" weather might freeze our ground units. Chilled units are"
				" unable to use abilities or fire their guns in spring until"
				" they warm up by moving."
				" One last note: tanks are heavy and damage any ground they"
				" drive though, sometimes even buildings!"
				" Our cities are already limited in power in this snowy land,"
				" so be careful."
				"");
				case Brief::FIRST_STAR: return _(""
				"Capture, destroy or occupy all enemy cities."
				"");
				case Brief::SECOND_STAR: return "";
				case Brief::THIRD_STAR: return "";
				case Brief::SENDOFF: return _(""
				"Good luck!"
				"");
			}
		}
		break;
		case ACT2D:
		{
			switch (brief)
			{
				case Brief::GREETING: return _(""
				"Commander!"
				"");
				case Brief::DESCRIPTION: return _(""
				"The enemy has been driven back, we have almost claimed our"
				" whole island! Only a small peninsula is still theirs."
				" In a desperate attempt to gain back some ground, they"
				" are preparing an assault! Protect this bridge at all costs!"
				" Since we have regained our airfields, we are able to field"
				" zeppelins in this battle. Their ability to drop gas might"
				" prove useful in a chokepoint. Gas kills all units in a"
				" tile, and even speads to adjacent tiles."
				"");
				case Brief::OBJECTIVE: return _(""
				"Keep a close eye though, as zepplins are easy targets for"
				" sappers and tanks. Zeppelins also drop gas upon dying,"
				" so be sure to not position them above your own unit."
				" Lastly, zeppelins can be brought down by enemy zeppelins"
				" flying into them. Their movement is very limited, so plan"
				" accordingly."
				"");
				case Brief::FIRST_STAR: return _(""
				"Capture, destroy or occupy all enemy cities."
				"");
				case Brief::SECOND_STAR: return "";
				case Brief::THIRD_STAR: return "";
				case Brief::SENDOFF: return _(""
				"Good luck!"
				"");
			}
		}
		break;
		case ACT2E:
		{
			switch (brief)
			{
				case Brief::GREETING: return _(""
				"Commander!"
				"");
				case Brief::DESCRIPTION: return _(""
				"Look what they have done to our land..."
				" In a last ditch effort to create as much military"
				" as possible, they have completely ruined the environment."
				" Frostbite will devastate our troops in winter."
				" Firestorms will burn infantry, tanks and zeppelins alike"
				" in summers, maybe forests might offer some protection."
				" The land has become incredibly dry, making our troops more"
				" vulnarable. In this bonedrought even a gunner will die to"
				" a single militia shot."
				"");
				case Brief::OBJECTIVE: return _(""
				"We should destroy our enemy and get out of here, before this"
				" whole peninsula turns into a desert and kills us all."
				"");
				case Brief::FIRST_STAR: return _(""
				"Capture, destroy or occupy all enemy cities."
				"");
				case Brief::SECOND_STAR: return "";
				case Brief::THIRD_STAR: return "";
				case Brief::SENDOFF: return _(""
				"Good luck!"
				"");
			}
		}
		break;
		case ACT3A:
		{
			switch (brief)
			{
				case Brief::GREETING: return _(""
				"Greetings, commander."
				"");
				case Brief::DESCRIPTION: return _(""
				"Now that we have freed our on island,"
				" our allies need our help!"
				" We have landed on the northern peninsula of their island,"
				" which is the only part not yet occupied by their enemies."
				" The fog here is very heavy so our vision is limited!"
				"");
				case Brief::OBJECTIVE: return _(""
				"If we wish to survive this expedition, we have to capture"
				" both the enemy city and the neutral city on the west coast."
				" This should allow us to set up a stronghold on this island"
				" for further missions!"
				" Do not forget to also protect our landing base!"
				" If we are unable to safely land backup, we are doomed!"
				"");
				case Brief::FIRST_STAR: return _(""
				"Capture the neutral City before the enemy does."
				"");
				case Brief::SECOND_STAR: return "";
				case Brief::THIRD_STAR: return "";
				case Brief::SENDOFF: return _(""
				"Good luck!"
				"");
			}
		}
		break;
		case ACT3B:
		{
			switch (brief)
			{
				case Brief::GREETING: return _(""
				"Hello, commander."
				"");
				case Brief::DESCRIPTION: return _(""
				"We seem to have caught the enemy off guard,"
				" as they were not prepared for our invasion."
				" Now, we must push further inward!"
				" If we are able to capture all three towns in this area,"
				" we will gain a massive economical advantage for the rest"
				" of the war!"
				"");
				case Brief::OBJECTIVE: return _(""
				"Make haste though, as the enemy is preparing for to"
				" retaliate as we speak!"
				"");
				case Brief::FIRST_STAR: return _(""
				"Capture 3 towns within 4 years."
				"");
				case Brief::SECOND_STAR: return "";
				case Brief::THIRD_STAR: return "";
				case Brief::SENDOFF: return _(""
				"Good luck!"
				"");
			}
		}
		break;
		case ACT3C:
		{
			switch (brief)
			{
				case Brief::GREETING: return _(""
				"Commander!"
				"");
				case Brief::DESCRIPTION: return _(""
				"We succesfully took the enemy towns before they could send"
				" their reinforcements. We cannot rest however, as to the"
				" far south of here lies the enemy industrial area."
				" If we take our the enemy cities on the eastern coast,"
				" we should be able to march on to the south after!"
				"");
				case Brief::OBJECTIVE: return _(""
				"To reach the eastern coast, we have to go either north or"
				" south of a small mountain range."
				" To the north, surely the enemy will send their big guns"
				" into the tight chokepoints. Our northern city also has"
				" to watch our for sappers across the mountains."
				" To the south, the large desert in between our and their city"
				" offers some protection, as reinforcements will take some"
				" time to arrive. However, they might already have patrols"
				" stationed here to protect their far southern industry."
				"");
				case Brief::FIRST_STAR: return _(""
				"Capture, destroy or occupy all enemy cities."
				"");
				case Brief::SECOND_STAR: return "";
				case Brief::THIRD_STAR: return "";
				case Brief::SENDOFF: return _(""
				"Good luck!"
				"");
			}
		}
		break;
		case ACT3D:
		{
			switch (brief)
			{
				case Brief::GREETING: return _(""
				"Hello, commander."
				"");
				case Brief::DESCRIPTION: return _(""
				"Now that we are sure no further reinforcements will be coming"
				" from near enemy areas, we are in prime position to take"
				" their industrial area for ourselves! The most important"
				" supply line for the whole island runs through their airfields."
				" If we take those, we deal a massive blow to the enemy!"
				" Be cautious though, as they will defend it with all they"
				" have."
				"");
				case Brief::OBJECTIVE: return _(""
				"Capture the enemy airfield and send a zeppelin to the second"
				" airfield. As they have no defense on that island, surely"
				" they will surrender when we threaten their airfield!"
				"");
				case Brief::FIRST_STAR: return _(""
				"Capture or occupy both airfields."
				"");
				case Brief::SECOND_STAR: return "";
				case Brief::THIRD_STAR: return "";
				case Brief::SENDOFF: return _(""
				"Good luck!"
				"");
			}
		}
		break;
		case ACT3E:
		{
			switch (brief)
			{
				case Brief::GREETING: return _(""
				"Greetings, commander."
				"");
				case Brief::DESCRIPTION: return _(""
				"Now that we have captured the enemy's essential industrial"
				" area, it is time we strike at their airforce!"
				" Without zeppelins, their reach on the island is limited and"
				" we will have the upper hand."
				" In this area, the wind is very strong and zeppelins can fly"
				" much faster than their usual pace. As gas in ineffective"
				" with such strong winds, we have armed our zeppelins with"
				" bombs, turning them into high-speed flying tanks!"
				" Be careful though, as the enemy will likely have done"
				" the same."
				"");
				case Brief::OBJECTIVE: return _(""
				"Beware of enemy zeppelins hidden in the thick mist"
				" and destroy the enemy!"
				"");
				case Brief::FIRST_STAR: return _(""
				"Destroy all enemy cities."
				"");
				case Brief::SECOND_STAR: return "";
				case Brief::THIRD_STAR: return "";
				case Brief::SENDOFF: return _(""
				"Good luck!"
				"");
			}
		}
		break;
		case ACT3F:
		{
			switch (brief)
			{
				case Brief::GREETING: return _(""
				"Greetings, commander."
				"");
				case Brief::DESCRIPTION: return _(""
				"The enemy is fleeing! They are trying to use a heavily armed"
				" convoy to bring what little resources they have left to"
				" their homebase. If we are able to intercept this convoy"
				" before they reach their city, we will be guaranteed"
				" victory and liberation of our allies!"
				"");
				case Brief::OBJECTIVE: return _(""
				"Kill the enemy gunners before they can reach their city."
				"");
				case Brief::FIRST_STAR: return _(""
				"Kill all enemy gunners."
				"");
				case Brief::SECOND_STAR: return "";
				case Brief::THIRD_STAR: return "";
				case Brief::SENDOFF: return _(""
				"Good luck!"
				"");
			}
		}
		break;
		case ACT3G:
		{
			switch (brief)
			{
				case Brief::GREETING: return _(""
				"Commander!"
				"");
				case Brief::DESCRIPTION: return _(""
				"We have done it, we have liberated the last city on the"
				" island! After out victory, our men have taken most of our"
				" equipment back home."
				" But all is not over... We have received reports that the"
				" enemy has a secret weapon hiding in the woods. This soldier"
				" is rumoured to have armor as tough as 5 militia with double"
				" the firepower of a regular gunner! It is also said to have"
				" the ability to bombard a short range, with its bombs"
				" leaving behind deadly gas!"
				" We must eliminate this threat!"
				"");
				case Brief::OBJECTIVE: return _(""
				"Kill the enemy super gunner."
				" You are unable to produce gunners or upgrade your industry."
				"");
				case Brief::FIRST_STAR: return _(""
				"Kill all enemy gunners."
				"");
				case Brief::SECOND_STAR: return "";
				case Brief::THIRD_STAR: return "";
				case Brief::SENDOFF: return _(""
				"Take care, commander!"
				"");
			}
		}
		break;
	}
	return "";
}
