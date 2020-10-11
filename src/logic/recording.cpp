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
#include "recording.hpp"
#include "source.hpp"

#include <mutex>

#include "clock.hpp"
#include "keycode.hpp"
#include "player.hpp"
#include "library.hpp"
#include "system.hpp"


static std::mutex _recordingsmutex;

std::string Recording::_recordingsfolder = "recordings/";
std::string Recording::_historyfilename = "recordings/history.list";

void Recording::setRoot(const std::string& root)
{
	if (root.empty())
	{
		_recordingsfolder = "recordings/";
	}
	else if (root.back() == '/')
	{
		_recordingsfolder = root + "recordings/";
	}
	else
	{
		_recordingsfolder = root + "/recordings/";
	}

	_historyfilename = _recordingsfolder + "history.list";
}

Recording::Recording() = default;

Recording::Recording(const std::string& name) :
	_name(name),
	_filename(Recording::filename(name))
{}

Recording::~Recording() = default;
Recording::Recording(Recording&&) = default;
Recording& Recording::operator=(Recording&&) = default;

std::string Recording::filename(const std::string& name)
{
	return _recordingsfolder + name + ".rec";
}

void Recording::start()
{
	static uint64_t counter = EpochClock::milliseconds();

	std::lock_guard<std::mutex> lock(_recordingsmutex);

	uint16_t key = rand();
	_name = keycode(key, counter++);
	_filename = Recording::filename(_name);

	System::touchFile(_filename);
}

void Recording::start(const std::string& name)
{
	_name = name;
	_filename = Recording::filename(name);
	_listed = false;

	System::touchFile(_filename);
}

void Recording::end()
{
	if (!_listed) return;

	std::lock_guard<std::mutex> lock(_recordingsmutex);

	std::ofstream index;
	index.open(_historyfilename, std::ofstream::out | std::ofstream::app);
	index << _name << std::endl;
}

void Recording::addMetadata(Json::Value& metadata)
{
	uint64_t starttime = EpochClock::seconds();
	metadata["starttime"] = starttime;

	time_t currenttime;
	std::time(&currenttime);
	std::string timestamp = std::asctime(std::localtime(&currenttime));
	timestamp.resize(timestamp.size() - 1);
	metadata["localtime"] = timestamp;
}

Json::Value Recording::metadata()
{
	if (_metadata) return *_metadata;

	std::ifstream file;
	try
	{
		file.open(filename());
		if (!file.is_open())
		{
			LOGW << "Failed to open '" << filename() << "'";
			return Json::nullValue;
		}

		Json::Reader reader;
		Json::Value json;
		std::string line;

		if (!std::getline(file, line) || !reader.parse(line, json)
			|| !json.isObject())
		{
			LOGW << "Error while parsing '" << filename() << "'";
			return Json::nullValue;
		}

		_metadata.reset(new Json::Value(json));
		return json;
	}
	catch (const std::ifstream::failure& error)
	{
		LOGW << "Error while reading '" << filename() << "': " << error.what();
		return Json::nullValue;
	}
}

bool Recording::exists(const std::string& name)
{
	try
	{
		std::ifstream file;
		file.open(Recording(name).filename());
		if (file.is_open())
		{
			return true;
		}
	}
	catch (const std::ifstream::failure& ignored)
	{
		return false;
	}
	return false;
}

std::vector<Recording> Recording::list(int count)
{
	static constexpr int LENGTH = 80;
	static char buffer[LENGTH + 1] = {0};

	if (count <= 0) return {};
	size_t maxcount = count;
	std::vector<Recording> results;

	System::touchDirectory(_recordingsfolder);


	std::lock_guard<std::mutex> lock(_recordingsmutex);

	std::ifstream index;
	index.open(_historyfilename, std::ifstream::in | std::ifstream::binary);
	index.seekg(0, std::ifstream::end);

	while (index.tellg() > 0 && results.size() < maxcount)
	{
		int i = LENGTH;
		while (index.tellg() > 0 && i >= 0)
		{
			index.seekg(-1, std::ifstream::cur);
			if (index.peek() != '\r' && index.peek() != '\n')
			{
				--i;
				buffer[i] = index.peek();
			}
			else
			{
				break;
			}
		}
		if (i < LENGTH)
		{
			results.emplace_back(std::string(buffer + i));
		}
	}

	return results;
}

std::vector<Player> Recording::getPlayers()
{
	std::vector<Player> playercolors = {Player::NONE};

	// TODO lelijk dat dit niet samen met LocalReplay::load() gebeurt
	Json::Value json = metadata();
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
				}
			}
		}
	}
	else
	{
		LOGW << "Missing players or bots while reading " << _name;
	}

	return playercolors;
}

std::string Recording::getRuleset()
{
	std::string rulesetname = Library::nameCurrentBible();

	// TODO lelijk dat dit niet samen met ::getPlayers() gebeurt
	Json::Value json = metadata();
	if (json.isObject())
	{
		if (json["ruleset"].isString())
		{
			rulesetname = json["ruleset"].asString();

			// The ruleset 'challenge_trample' was named 'custom' in recordings.
			if (rulesetname == "custom" && json["map"].isString())
			{
				std::string mapname = json["map"].asString();
				if (mapname.compare(0, 10, "challenge_") == 0)
				{
					LOGI << "Overriding rulesetname with '" << mapname << "'";
					rulesetname = mapname;
				}
			}
		}
		else if (json["bible-version"].isString())
		{
			// Old recording where every ruleset used was the master ruleset.
			Version bibleversion = Version(json["bible-version"]);
			rulesetname = Library::nameCompatibleBible(bibleversion);
		}
	}
	else
	{
		rulesetname = Library::nameCompatibleBible(Version::prehistoric());
	}

	// Look for a bible with that version.
	if (!Library::existsBible(rulesetname))
	{
		// Backwards compatibility: retry with a compatible ruleset.
		rulesetname = Library::nameCompatibleBible(rulesetname);

		if (!Library::existsBible(rulesetname))
		{
			LOGE << "Missing bible '" << rulesetname << "' while reading "
				<< _name;
			LOGI << "Continuing with current bible.";
			return Library::nameCurrentBible();
		}
	}

	return rulesetname;
}
