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
#include "writer.hpp"
#include "loginstaller.hpp"
#include "map.hpp"
#include "challenge.hpp"
#include "aichallenge.hpp"
#include "bible.hpp"
#include "library.hpp"
#include "board.hpp"


int main(int /**/, char* /**/[])
{
#ifdef DEVELOPMENT
#ifndef CANDIDATE
	return 0;
#endif
#endif

	std::cout << "[ Epicinium Map Checker ]";
	std::cout << " (v" << Version::current() << ")";
	std::cout << std::endl << std::endl;

	Writer writer;
	writer.install();

	Json::Reader reader;

	LogInstaller("mapchecker", 5).install();

	LOGI << "Start v" << Version::current();

	Library library;
	library.load();

	Bible latestbible = library.get(library.currentRuleset());
	Board board(latestbible);

	for (const std::string& mapname : Map::pool())
	{
		Json::Value json = Map::loadMetadata(mapname);
		LOGI << "Checking version of " << mapname;
		Version editorversion = Version(json["editor-version"]);
		assert(editorversion.isWellDefined());
		LOGI << "Parsing pooltype of " << mapname;
		assert(json["pool"].isString());
		PoolType pooltype = parsePoolType(json["pool"].asString());
		LOGI << "Checking pooltype of " << mapname;
		assert(pooltype == PoolType::MULTIPLAYER);

		LOGI << "Checking compatibility of " << mapname
			<< " with " << latestbible.name();
		board.load(mapname);

		LOGI << "Checking grassiness or " << mapname;
		{
			int score = 0;
			for (Cell index : board)
			{
				const TileToken& tile = board.tile(index);
				score += latestbible.tileScoreBase(tile.type);
				score += tile.stacks * latestbible.tileScoreStack(tile.type);
			}
			assert(score % 50 == 0);
		}
	}

	for (const std::string& mapname : Map::customPool())
	{
		Json::Value json = Map::loadMetadata(mapname);
		LOGI << "Checking version of " << mapname;
		Version editorversion = Version(json["editor-version"]);
		assert(editorversion.isWellDefined());
		LOGI << "Parsing pooltype of " << mapname;
		assert(json["pool"].isString());
		PoolType pooltype = parsePoolType(json["pool"].asString());
		LOGI << "Checking pooltype of " << mapname;
		assert(pooltype != PoolType::NONE);
		LOGI << "Parsing ruleset of " << mapname;
		assert(json["ruleset"].isString());
		std::string rulesetname = json["ruleset"].asString();
		LOGI << "Checking existence of ruleset " << rulesetname;
		assert(library.exists(rulesetname)
			|| library.exists(library.compatibleRuleset(rulesetname)));
		Bible custombible = library.get(rulesetname);
		LOGI << "Checking version of ruleset " << rulesetname;
		assert(custombible.version().isWellDefined());
	}

	for (const std::string& mapname : Map::userPool())
	{
		LOGI << "Checking for slash in " << mapname;
		assert(mapname.find_first_of('/') != std::string::npos);
		Json::Value json = Map::loadMetadata(mapname);
		LOGI << "Checking version of " << mapname;
		Version editorversion = Version(json["editor-version"]);
		assert(editorversion.isWellDefined());
		LOGI << "Parsing pooltype of " << mapname;
		assert(json["pool"].isString());
		PoolType pooltype = parsePoolType(json["pool"].asString());
		LOGI << "Checking pooltype of " << mapname;
		assert(pooltype != PoolType::NONE);
		LOGI << "Parsing ruleset of " << mapname;
		assert(json["ruleset"].isString());
		std::string rulesetname = json["ruleset"].asString();
		LOGI << "Checking existence of ruleset " << rulesetname;
		assert(library.exists(rulesetname)
			|| library.exists(library.compatibleRuleset(rulesetname)));
		Bible custombible = library.get(rulesetname);
		LOGI << "Checking version of ruleset " << rulesetname;
		assert(custombible.version().isWellDefined());
	}

	for (const std::string& mapname : Map::hiddenTutorialPool())
	{
		Json::Value json = Map::loadMetadata(mapname);
		LOGI << "Checking version of " << mapname;
		Version editorversion = Version(json["editor-version"]);
		assert(editorversion.isWellDefined());
		LOGI << "Parsing pooltype of " << mapname;
		assert(json["pool"].isString());
		PoolType pooltype = parsePoolType(json["pool"].asString());
		LOGI << "Checking pooltype of " << mapname;
		assert(pooltype == PoolType::CUSTOM);
		LOGI << "Parsing ruleset of " << mapname;
		assert(json["ruleset"].isString());
		std::string rulesetname = json["ruleset"].asString();
		LOGI << "Checking existence of ruleset " << rulesetname;
		assert(library.exists(rulesetname)
			|| library.exists(library.compatibleRuleset(rulesetname)));
		Bible custombible = library.get(rulesetname);
		LOGI << "Checking version of ruleset " << rulesetname;
		assert(custombible.version().isWellDefined());
	}

	for (const std::string& mapname : Map::hiddenChallengePool())
	{
		Json::Value json = Map::loadMetadata(mapname);
		LOGI << "Checking version of " << mapname;
		Version editorversion = Version(json["editor-version"]);
		assert(editorversion.isWellDefined());
		LOGI << "Parsing pooltype of " << mapname;
		assert(json["pool"].isString());
		PoolType pooltype = parsePoolType(json["pool"].asString());
		LOGI << "Checking pooltype of " << mapname;
		assert(pooltype == PoolType::CUSTOM);
		LOGI << "Parsing ruleset of " << mapname;
		assert(json["ruleset"].isString());
		std::string rulesetname = json["ruleset"].asString();
		LOGI << "Checking existence of ruleset " << rulesetname;
		assert(library.exists(rulesetname)
			|| library.exists(library.compatibleRuleset(rulesetname)));
		Bible custombible = library.get(rulesetname);
		LOGI << "Checking version of ruleset " << rulesetname;
		assert(custombible.version().isWellDefined());
	}

	for (const std::string& mapname : Map::hiddenDioramaPool())
	{
		Json::Value json = Map::loadMetadata(mapname);
		LOGI << "Checking version of " << mapname;
		Version editorversion = Version(json["editor-version"]);
		assert(editorversion.isWellDefined());
		LOGI << "Parsing pooltype of " << mapname;
		assert(json["pool"].isString());
		PoolType pooltype = parsePoolType(json["pool"].asString());
		LOGI << "Checking pooltype of " << mapname;
		assert(pooltype == PoolType::DIORAMA);
		LOGI << "Parsing ruleset of " << mapname;
		assert(json["ruleset"].isString());
		std::string rulesetname = json["ruleset"].asString();
		LOGI << "Checking existence of ruleset " << rulesetname;
		assert(library.exists(rulesetname)
			|| library.exists(library.compatibleRuleset(rulesetname)));
		Bible custombible = library.get(rulesetname);
		LOGI << "Checking version of ruleset " << rulesetname;
		assert(custombible.version().isWellDefined());
	}

	for (const Challenge::Id& challengeId : Challenge::pool())
	{
		AIChallenge challenge(challengeId);

		{
			std::string challengemapname = challenge.getMapName();
			LOGI << "Checking existence of " << challengemapname;
			bool found = false;
			for (const std::string& mapname : Map::pool())
			{
				if (challengemapname == mapname) found = true;
			}
			for (const std::string& mapname : Map::hiddenChallengePool())
			{
				if (challengemapname == mapname) found = true;
			}
			assert(found);
		}

		{
			std::string rulesetname = challenge.getRulesetName();
			if (!rulesetname.empty())
			{
				LOGI << "Checking existence of ruleset " << rulesetname;
				assert(library.exists(rulesetname));
				Bible custombible = library.get(rulesetname);
				LOGI << "Checking version of ruleset " << rulesetname;
				assert(custombible.version().isWellDefined());
			}
		}
	}

	LOGI << "OK";

	std::cout << std::endl << "[ Done ]" << std::endl;

	return 0;
}
