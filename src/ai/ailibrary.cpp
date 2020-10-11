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
#include "ailibrary.hpp"
#include "source.hpp"

#if LIBLOADER_ENABLED
#include <mutex>
#include "libloader.hpp"
#include "system.hpp"
#endif

#include "typenamer.hpp"
#include "change.hpp"
#include "order.hpp"
#include "player.hpp"
#include "difficulty.hpp"
#include "loginstaller.hpp"
#include "library.hpp"
#include "version.hpp"
#include "settings.hpp"


void AILibrary::receiveChanges(const std::vector<Change>& changes,
	const TypeNamer& namer)
{
	std::stringstream strm;
	strm << TypeEncoder(&namer) << "[";
	const char* sep = "";
	for (const Change& change : changes)
	{
		strm << sep << change;
		sep = ",";
	}
	strm << "]";
	return receiveChangesAsString(strm.str());
}

std::vector<Order> AILibrary::orders(const TypeNamer& namer)
{
	// TODO try catch?
	return Order::parseOrders(namer, ordersAsString());
}

Player AILibrary::player() const
{
	// TODO try catch?
	return ::parsePlayer(playerAsString());
}

Difficulty AILibrary::difficulty() const
{
	// TODO try catch?
	return ::parseDifficulty(difficultyAsString());
}




#if LIBLOADER_ENABLED

bool AILibrary::exists(const std::string& name)
{
	if (name.find("bin/ai") != std::string::npos)
	{
		size_t seppos = name.find('#');
		if (seppos != std::string::npos)
		{
			return System::isFile(name.substr(0, seppos));
		}
		else
		{
			return System::isFile(name);
		}
	}
	else return false;
}

bool AILibrary::isLibraryReminder(const std::string& name)
{
	// We assume (AI::exists(name) || AILibrary::exists(name)) holds.
	return (name.find('/') != std::string::npos);
}

bool AILibrary::hasFastDirective(const std::string& name)
{
	// We assume (AILibrary::exists(name)) holds.
	return (name.find("#fast") != std::string::npos);
}

static std::mutex _libloadermutex;
static std::vector<LibLoader<AILibrary>> _libloaders;

static LibLoader<AILibrary>& getLib(std::string name)
{
	{
		size_t seppos = name.find('#');
		if (seppos != std::string::npos)
		{
			name = name.substr(0, seppos);
		}
	}

	for (auto& loader : _libloaders)
	{
		if (loader.pathToLib() == name)
		{
			return loader;
		}
	}

	_libloaders.emplace_back(name);
	return _libloaders.back();
}

void AILibrary::preload(const std::string& name)
{
	std::lock_guard<std::mutex> lock(_libloadermutex);

	if (!getLib(name))
	{
		LOGE << "Unknown AILibrary '" << name << "'";
		DEBUG_ASSERT(false);
	}
}

std::shared_ptr<AILibrary> AILibrary::create(const std::string& name,
	const Player& player, const Difficulty& difficulty,
	const std::string& ruleset, char character)
{
	AILibrary* ptr = allocate(name, player, difficulty, ruleset, character);
	if (ptr != nullptr)
	{
		return std::shared_ptr<AILibrary>(ptr);
	}
	else
	{
		return nullptr;
	}
}

AILibrary* AILibrary::allocate(const std::string& name,
	const Player& player, const Difficulty& difficulty,
	const std::string& ruleset, char character)
{
	std::lock_guard<std::mutex> lock(_libloadermutex);

	if (auto& lib = getLib(name))
	{
		return lib.instantiate(::stringify(player), ::stringify(difficulty),
			ruleset.c_str(), character);
	}
	else
	{
		LOGE << "Unknown AILibrary '" << name << "'";
		DEBUG_ASSERT(false);
		return nullptr;
	}
}

#else /* LIBLOADER_ENABLED */

bool AILibrary::exists(const std::string&)
{
	return false;
}

bool AILibrary::isLibraryReminder(const std::string&)
{
	return false;
}

bool AILibrary::hasFastDirective(const std::string&)
{
	return false;
}

void AILibrary::preload(const std::string&)
{
	// Do nothing.
}

std::shared_ptr<AILibrary> AILibrary::create(const std::string&,
	const Player&, const Difficulty&,
	const std::string&, char /**/)
{
	LOGF << "Cannot create AILibrary if LibLoader is disabled";
	DEBUG_ASSERT("LIBLOADER_ENABLED=" && LIBLOADER_ENABLED && "; enable it?");
	return nullptr;
}

#endif /* LIBLOADER_ENABLED */




static std::unique_ptr<LogInstaller> _loginstaller;
static std::unique_ptr<Library> _library;

void AILibrary::setup(const char* name, int argc, const char* const argv[])
{
	Settings settings(argc, argv);

	if (settings.dataRoot.defined())
	{
		LogInstaller::setRoot(settings.dataRoot.value());
	}

	std::string logname = name;
	if (settings.logname.defined())
	{
		logname = settings.logname.value();
	}
	else settings.logname.override(logname);

	_loginstaller.reset(new LogInstaller(settings));
	_loginstaller->install();

	_library.reset(new Library());
	_library->load();
	_library->install();

	LOGI << "Set up " << logname << " v" << Version::current();
}
