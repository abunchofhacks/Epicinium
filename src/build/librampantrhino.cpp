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
#include "source.hpp"

#include "ailibrary.hpp"
#include "airampantrhino.hpp"
#include "player.hpp"
#include "difficulty.hpp"


#ifdef PLATFORMUNIX
#define EXPORT __attribute__ ((visibility ("default")))
#else
#define EXPORT __declspec(dllexport)
#endif

extern "C"
{
	EXPORT void setup(int argc, const char* const argv[]);
	EXPORT AIRampantRhino* allocate(
		const char* player, const char* difficulty,
		const char* rulesetname, char character);

	void setup(int argc, const char* const argv[])
	{
		AILibrary::setup("librampantrhino", argc, argv);
	}

	AIRampantRhino* allocate(
		const char* player, const char* difficulty,
		const char* rulesetname, char character)
	{
		LOGD << "Allocating " << difficulty << " " << player << ""
			" AIRampantRhino named '" << character << "'"
			" with ruleset " << rulesetname;

		return new AIRampantRhino(parsePlayer(player),
			parseDifficulty(difficulty), rulesetname, character);
	}
}
