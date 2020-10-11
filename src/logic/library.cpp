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
#include "library.hpp"
#include "source.hpp"

#include <mutex>

#include "bible.hpp"
#include "system.hpp"
#include "locator.hpp"
#include "parseerror.hpp"


static std::string _indexfilename = "rulesets/index.list";
static std::string _localindexfilename = "rulesets/local.list";

static std::mutex _mutex;

Library* Library::_installed = nullptr;

Library::Library() = default;
Library::~Library() = default;

std::string Library::nameCurrentBible()
{
	std::lock_guard<std::mutex> lock(_mutex);
	return _installed->currentRuleset();
}

std::string Library::nameCompatibleBible(const Version& version)
{
	std::lock_guard<std::mutex> lock(_mutex);
	return _installed->compatibleRuleset(version);
}

std::string Library::nameCompatibleBible(const std::string& rulesetname)
{
	std::lock_guard<std::mutex> lock(_mutex);
	return _installed->compatibleRuleset(rulesetname);
}

bool Library::existsBible(const std::string& rulesetname)
{
	std::lock_guard<std::mutex> lock(_mutex);
	return _installed->exists(rulesetname);
}

Bible Library::getBible(const std::string& rulesetname)
{
	std::lock_guard<std::mutex> lock(_mutex);
	return _installed->get(rulesetname);
}

bool Library::storeBible(const std::string& rulesetname,
	const Json::Value& biblejson)
{
	std::lock_guard<std::mutex> lock(_mutex);
	return _installed->store(rulesetname, biblejson);
}

void Library::install()
{
	std::lock_guard<std::mutex> lock(_mutex);
	_installed = this;
}

bool Library::loadIndex(const std::string& filename)
{
	// Our bible is the most recent bible, unless shown otherwise.
	Version myversion = Version::current();

	std::ifstream index(filename);
	if (!index.is_open())
	{
		return false;
	}

	std::string line;
	while (std::getline(index, line))
	{
		Version version;
		try
		{
			version = Version(line);
		}
		catch (const ParseError& error)
		{
			LOGE << "Ignoring " << error.what();
			RETHROW_IF_DEV();
			continue;
		}

		if (version.major < myversion.major
			|| (version.major == myversion.major
				&& version.minor <= myversion.minor)
			|| myversion.isDevelopment())
		{
			_available.push_back(version);
		}
		else
		{
			LOGI << "Cannot load incompatible ruleset v" << version;
		}
	}

	return true;
}

void Library::load()
{
	// Cache the current bible.
	_available.push_back(Version::current());
	_cache.emplace_back(new Bible(Bible::createDefault()));
}

void Library::loadAndUpdateIndex()
{
	// Load the versions for which bibles exist from the index.
	if (!loadIndex(_indexfilename))
	{
		LOGF << "Could not open " << _indexfilename;
		throw std::runtime_error("Could not open " + _indexfilename);
	}

	// The local index file might not exist but that is ok.
	loadIndex(_localindexfilename);

	// Sort from oldest to newest and filter out duplicates.
	if (!_available.empty())
	{
		std::sort(_available.begin(), _available.end());
		_available.erase(std::unique(_available.begin(), _available.end()),
			_available.end());
	}

	// If we're in dev, cache the current bible.
	Version myversion = Version::current();
	if (myversion.isDevelopment())
	{
		_available.push_back(myversion);
		_cache.emplace_back(new Bible(Bible::createDefault()));
	}
	// We are not in dev. If our bible is more recent, cache it.
	// If it has not yet been saved, save it.
	else
	{
		// Create the current bible.
		Bible mybible = Bible::createDefault();

		// If it is saved, we call it by its oldest name.
		bool unsaved = true;
		bool unsavedrelease = true;
		if (!_available.empty())
		{
			Version latest = _available.back();
			const Bible& latestbible = loadBible(latest);
			unsaved = ((latest.isReleaseCandidateOf(myversion)
					|| myversion.isReleaseCandidateOf(latest)
					|| latest < myversion)
				&& latestbible != mybible);
			if (unsaved)
			{
				unsavedrelease = (latest.release() != myversion.release());
			}
		}

		// If it is unsaved, cache it and save it.
		if (unsaved)
		{
			_available.push_back(myversion);
			_cache.emplace_back(new Bible(mybible));

			if (myversion.isReleaseCandidate())
			{
				saveBible(myversion.release().name(), mybible);
				if (unsavedrelease)
				{
					addVersionToIndex(myversion.release());
				}
			}

			saveBible(myversion.name(), mybible);
			addVersionToIndex(myversion);
		}
	}

	// Save a custom ruleset.
	{
		Bible custombible = Bible::createCustom();
		bool unsaved = true;
		if (exists(custombible.name()))
		{
			Bible oldbible = get(custombible.name());
			if (oldbible.version().major == myversion.major
				&& oldbible == custombible)
			{
				unsaved = false;
			}
		}
		if (unsaved)
		{
			saveBible(custombible.name(), custombible);
		}
	}
}

bool Library::saveBible(const std::string& rulesetname, const Bible& bible)
{
	std::string fname = Locator::rulesetFilename(rulesetname);
	std::cout << "Saving '" << fname << "'..." << std::endl;

	std::ofstream file(fname);
	if (!file.is_open())
	{
		LOGE << "Could not open " << fname;
		DEBUG_ASSERT(false);
		return false;
	}

	Json::StyledWriter writer;
	file << writer.write(bible.toJson());

	std::cout << "Saved." << std::endl;

	return true;
}

bool Library::addVersionToIndex(const Version& version)
{
	std::cout << "Adding " << version << " to the index..." << std::endl;

	if (version.isRelease())
	{
		std::ofstream index(_indexfilename,
			std::ofstream::out | std::ofstream::app);
		if (!index.is_open())
		{
			LOGE << "Could not open " << _indexfilename;
			DEBUG_ASSERT(false);
			return false;
		}

		index << version.release() << std::endl;
	}
	else
	{
		std::ofstream index(_localindexfilename,
			std::ofstream::out | std::ofstream::app);
		if (!index.is_open())
		{
			LOGE << "Could not open " << _localindexfilename;
			DEBUG_ASSERT(false);
			return false;
		}

		index << version << std::endl;
	}

	std::cout << "Added." << std::endl;

	return true;
}

const Bible& Library::loadBible(const Version& version)
{
	return loadBible(version.name());
}

const Bible& Library::loadBible(const std::string& rulesetname)
{
	std::string fname = Locator::rulesetFilename(rulesetname);
	std::ifstream file(fname);
	if (!file.is_open())
	{
		LOGF << "Could not open " << fname;
		throw std::runtime_error("Could not open " + fname);
	}

	Json::Reader reader;
	Json::Value json;
	if (!reader.parse(file, json))
	{
		LOGF << "Broken ruleset " << fname << ": "
			<< reader.getFormattedErrorMessages();
		throw std::runtime_error("Broken ruleset " + fname + ": "
			+ reader.getFormattedErrorMessages());
	}

	try
	{
		_cache.emplace_back(new Bible(rulesetname, json));
		return *(_cache.back());
	}
	catch (const ParseError& error)
	{
		LOGF << "Broken ruleset " << fname << ": " << error.what();
		throw std::runtime_error("Broken ruleset " + fname + ": " + error.what());
	}
	catch (const Json::Exception& error)
	{
		LOGF << "Broken ruleset " << fname << ": " << error.what();
		throw std::runtime_error("Broken ruleset " + fname + ": " + error.what());
	}
}

bool Library::exists(const std::string& rulesetname)
{
	// The current ruleset is always cached.
	// In particular, while in dev the dev ruleset is cached but not saved.
	if (rulesetname == currentRuleset()) return true;

	// As the server, we can serve any of the rulesets in our folder.
	// As the client, we want to know if we have downloaded it already.
	return System::isFile(Locator::rulesetFilename(rulesetname));
}

Bible Library::get(const std::string& rulesetname)
{
	// Look for the bible in the cache.
	for (auto& bible : _cache)
	{
		if (bible->name() == rulesetname)
		{
			return *bible;
		}
	}

	// Does a ruleset with this name exist?
	if (exists(rulesetname))
	{
		// Not yet cached, load it now.
		return loadBible(rulesetname);
	}

	// Not found, just use the default bible.
	LOGW << "Missing bible '" << rulesetname << "'";
	return Bible::createDefault();
}

bool Library::store(const std::string& rulesetname, const Json::Value& json)
{
	if (exists(rulesetname))
	{
		LOGW << "Not storing ruleset '" << rulesetname << "'"
			<< " because a ruleset with that name already exists";
		return false;
	}

	try
	{
		Bible bible(rulesetname, json);
		saveBible(rulesetname, bible);
		return true;
	}
	catch (ParseError& error)
	{
		LOGE << "Not storing broken ruleset '" << rulesetname << "'"
			<< " because of a parse error: " << error.what();
		return false;
	}
	catch (Json::Exception& error)
	{
		LOGE << "Not storing broken ruleset '" << rulesetname << "'"
			<< " because of a parse error: " << error.what();
		return false;
	}
}

std::string Library::currentRuleset()
{
	Version latestversion = _available.back();
	return latestversion.name();
}

std::string Library::compatibleRuleset(const Version& targetversion)
{
	// Look for the most recent bible version that is compatible with the
	// requested version.
	for (auto iter = _available.rbegin(); iter != _available.rend(); ++iter)
	{
		Version bibleversion = *iter;
		if (bibleversion <= targetversion
			|| bibleversion.isReleaseCandidateOf(targetversion))
		{
			return bibleversion.name();
		}
	}

	LOGW << "Nothing compatible with v" << targetversion << " found";
	Version oldestversion = _available.front();
	return oldestversion.name();
}

std::string Library::compatibleRuleset(const std::string& rulesetname)
{
	try
	{
		Version namedversion = Version(rulesetname);
		return compatibleRuleset(namedversion);
	}
	catch (const ParseError& ignored)
	{
		return rulesetname;
	}
}

std::string Library::previousRuleset()
{
	Version latestversion = _available.back();
	for (auto iter = _available.rbegin(); iter != _available.rend(); ++iter)
	{
		Version bibleversion = *iter;
		if (bibleversion < latestversion
			&& !latestversion.isReleaseCandidateOf(bibleversion))
		{
			return bibleversion.name();
		}
	}

	LOGW << "Nothing older than v" << latestversion << " found";
	Version oldestversion = _available.front();
	return oldestversion.name();
}
