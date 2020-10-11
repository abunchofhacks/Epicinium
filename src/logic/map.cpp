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
#include "map.hpp"
#include "source.hpp"

#include "system.hpp"


const char* stringify(const PoolType& type)
{
	switch (type)
	{
		case PoolType::NONE: return "none";
		case PoolType::MULTIPLAYER: return "multiplayer";
		case PoolType::CUSTOM: return "custom";
		case PoolType::DIORAMA: return "diorama";
	}
	return "";
}

PoolType parsePoolType(const std::string& str)
{
	for (size_t i = 0; i < POOLTYPE_SIZE; i++)
	{
		PoolType type = (PoolType) i;
		if (str.compare(stringify(type)) == 0) return type;
	}

	LOGE << "Cannot parse unknown PoolType '" << str << "'";
	DEBUG_ASSERT(false);
	return PoolType::NONE;
}

std::ostream& operator<<(std::ostream& os, const PoolType& type)
{
	return os << ::stringify(type);
}


std::string Map::_resourcemapsfolder = "maps/";
std::string Map::_authoredmapsfolder = "maps/";

void Map::setResourceRoot(const std::string& root)
{
	if (root.empty())
	{
		_resourcemapsfolder = "maps/";
	}
	else if (root.back() == '/')
	{
		_resourcemapsfolder = root + "maps/";
	}
	else
	{
		_resourcemapsfolder = root + "/maps/";
	}
}

void Map::setAuthoredRoot(const std::string& root)
{
	if (root.empty())
	{
		_authoredmapsfolder = "maps/";
	}
	else if (root.back() == '/')
	{
		_authoredmapsfolder = root + "maps/";
	}
	else
	{
		_authoredmapsfolder = root + "/maps/";
	}
}

std::string Map::readOnlyFilename(const std::string& name)
{
	std::string fname = authoredFilename(name);
	if (System::isFile(fname))
	{
		return fname;
	}
	fname = _resourcemapsfolder + name + ".map";
	if (System::isFile(fname))
	{
		return fname;
	}
	fname = _resourcemapsfolder + name + ".json";
	if (System::isFile(fname))
	{
		return fname;
	}
	return authoredFilename(name);
}

std::string Map::authoredFilename(const std::string& name)
{
	return _authoredmapsfolder + name + ".map";
}

Json::Value Map::loadMetadata(const std::string& name)
{
	std::ifstream file;
	try
	{
		file.open(readOnlyFilename(name));
		if (file.is_open())
		{
			Json::Reader reader;
			Json::Value json;
			std::string line;

			if (!std::getline(file, line) || !reader.parse(line, json)
				|| !json.isObject())
			{
				return Json::nullValue;
			}

			return json;
		}
	}
	catch (const std::ifstream::failure& ignored)
	{
		return Json::nullValue;
	}
	return Json::nullValue;
}

bool Map::exists(const std::string& name)
{
	// Any file that exists exists.
	return System::isFile(readOnlyFilename(name));
}

const std::vector<std::string>& Map::pool()
{
	static std::vector<std::string> pool = {
		{"toad1v1"},
		{"spruce1v1"},
		{"small1v1"},
		{"oceanside1v1"},
		{"oasis1v1"},
		{"small3ffa"},
		{"oasis3ffa"},
		{"small4ffa"},
		{"beetle4ffa"},
		{"small8ffa"},
		{"cornered8ffa"},
	};
	return pool;
}

const std::vector<std::string>& Map::customPool()
{
	static std::vector<std::string> pool = {
		{"snowcut1v1"},
		{"challenge_morale"},
	};
	return pool;
}

const std::vector<std::string>& Map::userPool()
{
	static std::vector<std::string> pool = {
		{"Overlord_Vadim/Abandoned Places"},
		{"Overlord_Vadim/Civil War"},
		{"Overlord_Vadim/Great Duel"},
		{"Overlord_Vadim/Island of Augarn"},
		{"Overlord_Vadim/Mountain Crossings"},
		{"Overlord_Vadim/Neutrality Impossible"},
		{"Overlord_Vadim/Northmar's Heart"},
		{"Overlord_Vadim/Sins of the Past"},
		{"Overlord_Vadim/Town's Greatness"},
		{"Overlord_Vadim/Triumvirate Islands"},
		{"Overlord_Vadim/War Never Changes"},
		{"Overlord_Vadim/Winter Confrontation"},
	};
	return pool;
}

const std::vector<std::string>& Map::hiddenTutorialPool()
{
	static std::vector<std::string> pool = {
		{"tutorial"},
	};
	return pool;
}

const std::vector<std::string>& Map::hiddenChallengePool()
{
	static std::vector<std::string> pool = {
		{"challenge_showcase"},
		{"challenge_elimination"},
		{"challenge_trample"},
		{"challenge_investment"},
		{"challenge_morale"},
	};
	return pool;
}

std::string Map::DIORAMA_MAPNAME = "overview";

const std::vector<std::string>& Map::hiddenDioramaPool()
{
	static std::vector<std::string> pool = {
		DIORAMA_MAPNAME,
	};
	return pool;
}
