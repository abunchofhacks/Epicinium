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

#include "version.hpp"
#include "loginstaller.hpp"
#include "writer.hpp"
#include "player.hpp"
#include "cycle.hpp"
#include "unittype.hpp"
#include "tiletype.hpp"
#include "unittoken.hpp"
#include "tiletoken.hpp"
#include "bible.hpp"
#include "library.hpp"
#include "randomizer.hpp"
#include "area.hpp"
#include "aim.hpp"


static Player randomPlayer()
{
	// From 0 = NONE to PLAYER_MAX = PURPLE.
	return (Player) (rand() % (PLAYER_MAX + 1));
}

static UnitType randomUnitType(const Bible& bible)
{
	Randomizer<UnitType> randomizer;
	randomizer.push(UnitType::NONE);
	randomizer.push(UnitType::NONE);
	randomizer.push(UnitType::NONE);
	for (UnitType type : bible.unittypes()) randomizer.push(type);
	return randomizer.pop();
}

static TileType randomTileType(const Bible& bible)
{
	Randomizer<TileType> randomizer;
	randomizer.push(TileType::NONE);
	randomizer.push(TileType::NONE);
	randomizer.push(TileType::NONE);
	for (TileType type : bible.tiletypes()) randomizer.push(type);
	return randomizer.pop();
}

static int8_t randomStacks()
{
	return rand() % 6;
}

static int8_t randomPower()
{
	return rand() % 6;
}

static UnitToken randomUnitToken(const Bible& b)
{
	UnitToken newtoken;
	newtoken.type = randomUnitType(b);
	newtoken.owner = randomPlayer();
	newtoken.stacks = randomStacks();
	return newtoken;
}

static TileToken randomTileToken(const Bible& b)
{
	TileToken newtoken;
	newtoken.type = randomTileType(b);
	newtoken.owner = randomPlayer();
	newtoken.stacks = randomStacks();
	newtoken.power = randomPower();
	return newtoken;
}

class AreaSanityTest
{
public:
	static void run(int rows, int cols, int minrange, int maxrange, int seed)
	{
		Cell board = Cell::create(rows, cols, rows * cols);
		Cell from = Cell::create(rows, cols, seed % (rows * cols));
		assert(from.valid());
		LOGI << "Checking Area(" << from << " " << from.pos()
			<< ", " << minrange << ", " << maxrange << ")";

		int inside = 0;
		int outside = 0;
		for (Cell to : board)
		{
			int dis = Aim(from.pos(), to.pos()).sumofsquares();
			if (dis >= minrange && dis <= maxrange)
			{
				inside += 1;
			}
			else
			{
				outside += 1;
			}
		}
		LOGV << inside << " inside";
		assert(inside + outside == rows * cols);
		int verified = 0;
		for (Cell to : Area(from, minrange, maxrange))
		{
			assert(to.valid());
			int dis = Aim(from.pos(), to.pos()).sumofsquares();
			assert(dis >= minrange && dis <= maxrange);
			verified += 1;
		}
		LOGV << verified << " verified";
		assert(inside == verified);
	}
};

int main(int /**/, char* /**/[])
{
	std::cout << "[ Epicinium Sanity Checker ]";
	std::cout << " (v" << Version::current() << ")";
	std::cout << std::endl << std::endl;

	Writer writer;
	writer.install();

	Json::Reader reader;

	LogInstaller("sanitychecker", 5).install();

	LOGI << "Start v" << Version::current();

#ifndef DEVELOPMENT
	DEBUG_ASSERT(false && "Debug assertions are not turned off properly.");
#endif

	Library library;
	library.load();
	Bible bible = library.get(library.currentRuleset());

	// Seed the randomizer.
	srand(time(nullptr));

	for (size_t i = 0; i <= PLAYER_MAX; i++)
	{
		Player player = (Player) i;
		std::stringstream strm;
		strm << player;
		LOGI << "Checking " << player;
		assert(player == ::parsePlayer(strm.str().c_str()));
	}

	for (size_t i = 0; i < SEASON_SIZE; i++)
	{
		Season season = (Season) i;
		std::stringstream strm;
		strm << season;
		LOGI << "Checking " << season;
		assert(season == ::parseSeason(strm.str().c_str()));
	}

	for (size_t i = 0; i < DAYTIME_SIZE; i++)
	{
		Daytime daytime = (Daytime) i;
		std::stringstream strm;
		strm << daytime;
		LOGI << "Checking " << daytime;
		assert(daytime == ::parseDaytime(strm.str().c_str()));
	}

	for (size_t i = 0; i < PHASE_SIZE; i++)
	{
		Phase phase = (Phase) i;
		std::stringstream strm;
		strm << phase;
		LOGI << "Checking " << phase;
		assert(phase == ::parsePhase(strm.str().c_str()));
	}

	for (size_t i = 0; i <= bible.unittype_max(); i++)
	{
		UnitType type = (UnitType) i;
		std::stringstream strm;
		strm << TypeEncoder(&bible);
		strm << type;
		LOGI << "Checking " << strm.str();
		LOGI << " that is " << TypeEncoder(&bible) << type;
		assert(type == ::parseUnitType(bible, strm.str().c_str()));
	}

	for (size_t i = 0; i <= bible.tiletype_max(); i++)
	{
		TileType type = (TileType) i;
		std::stringstream strm;
		strm << TypeEncoder(&bible);
		strm << type;
		LOGI << "Checking " << strm.str();
		LOGI << " that is " << TypeEncoder(&bible) << type;
		assert(type == ::parseTileType(bible, strm.str().c_str()));
	}

	for (int i = 0; i < 10; i++)
	{
		UnitToken token = randomUnitToken(bible);
		std::stringstream strm;
		strm << TypeEncoder(&bible);
		strm << token;
		LOGI << "Checking " << strm.str();
		LOGI << " that is " << TypeEncoder(&bible) << token;
		Json::Value value;
		reader.parse(strm.str(), value);
		assert(!(token != UnitToken(bible, value)));
	}

	for (int i = 0; i < 10; i++)
	{
		TileToken token = randomTileToken(bible);
		std::stringstream strm;
		strm << TypeEncoder(&bible);
		strm << token;
		LOGI << "Checking " << strm.str();
		LOGI << " that is " << TypeEncoder(&bible) << token;
		Json::Value value;
		reader.parse(strm.str(), value);
		assert(!(token != TileToken(bible, value)));
	}

	AreaSanityTest::run(20, 13, 2, 10, 259);
	AreaSanityTest::run(20, 13, 2, 2, 12);
	AreaSanityTest::run(20, 13, 4, 4, 158);
	AreaSanityTest::run(20, 13, 20, 25, 24);
	AreaSanityTest::run(Position::MAX_ROWS, Position::MAX_COLS, 0, 5, 1);
	AreaSanityTest::run(1, 1, 0, 0, 0);

	for (int minrange = 0; minrange <= 20; minrange++)
	{
		for (int j = 0; j < 10; j++)
		{
			int maxrange = minrange + ((j <= 1) ? j : (rand() % (1 << j)));
			AreaSanityTest::run(20, 13, minrange, maxrange, rand());
		}
	}

	// Check the implementation of ::format().
	std::string xs(1022, 'x');
	assert(1022 == ::format("%s", xs.c_str()).size());
	assert(1023 == ::format("y%s", xs.c_str()).size());
	assert(1024 == ::format("yy%s", xs.c_str()).size());
	assert(1025 == ::format("yyy%s", xs.c_str()).size());
	assert(1026 == ::format("yyyy%s", xs.c_str()).size());

	LOGI << "OK";

	std::cout << std::endl << "[ Done ]" << std::endl;
	return 0;
}
