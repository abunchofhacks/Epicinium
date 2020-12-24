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
#include "ai.hpp"
#include "source.hpp"

#include "aihungryhippo.hpp"
#include "aichargingcheetah.hpp"
#include "aiquickquack.hpp"
#include "aitutorialturtle.hpp"
#include "airampantrhino.hpp"
#include "aibrawlingbear.hpp"
#include "aibrawlingbearfree.hpp"
#include "aineuralnewt.hpp"
#include "ailibrary.hpp"


std::unique_ptr<AICommander> AI::create(const std::string& ainame,
	const Player& player, const Difficulty& difficulty,
	const std::string& ruleset, char character)
{
	return std::unique_ptr<AICommander>(
		allocate(ainame, player, difficulty, ruleset, character));
}

AICommander* AI::allocate(const std::string& ainame,
	const Player& player, const Difficulty& difficulty,
	const std::string& ruleset, char character)
{
	std::string lowered = tolower(ainame);
	if (lowered == "neuralnewt")
	{
		return new AINeuralNewt(player, difficulty, ruleset, character);
	}
	if (lowered == "quickquack")
	{
		return new AIQuickQuack(player, difficulty, ruleset, character);
	}
	if (lowered == "hungryhippo")
	{
		return new AIHungryHippo(player, difficulty, ruleset, character);
	}
	if (lowered == "chargingcheetah")
	{
		return new AIChargingCheetah(player, difficulty, ruleset, character);
	}
	if (lowered == "tutorialturtle")
	{
		return new AITutorialTurtle(player, difficulty, ruleset, character);
	}
	if (lowered == "rampantrhino")
	{
		return new AIRampantRhino(player, difficulty, ruleset, character);
	}
	if (lowered == "brawlingbear")
	{
		return new AIBrawlingBear(player, difficulty, ruleset, character);
	}
	if (lowered == "brawlingbearfree")
	{
		return new AIBrawlingBearFree(player, difficulty, ruleset, character);
	}
	if (lowered == "dummy")
	{
		return new AIDummy(player, difficulty, ruleset, character);
	}

	LOGE << "Unknown AI '" << ainame << "'";
	DEBUG_ASSERT(false);
	return nullptr;
}

bool AI::exists(const std::string& name)
{
	std::string lowered = tolower(name);
	for (const std::string& pooled : pool())
	{
		if (lowered == tolower(pooled)) return true;
	}
	for (const std::string& pooled : hiddenPool())
	{
		if (lowered == tolower(pooled)) return true;
	}
	return false;
}

std::string AI::libraryDefaultFilename(const std::string& name)
{
	return "bin/ai" + tolower(name) + "default.so";
}

const std::vector<std::string>& AI::pool()
{
	static std::vector<std::string> pool = {
		"RampantRhino",
		"HungryHippo",
		"NeuralNewt",
	};
	return pool;
}

const std::vector<std::string>& AI::hiddenPool()
{
	static std::vector<std::string> pool = {
		"ChargingCheetah",
		"QuickQuack",
		"TutorialTurtle",
		"BrawlingBear",
		"BrawlingBearFree",
		"Dummy",
	};
	return pool;
}
