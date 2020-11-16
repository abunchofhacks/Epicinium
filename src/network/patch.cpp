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
#include "patch.hpp"
#include "source.hpp"

#include "parseerror.hpp"
#include "platform.hpp"
#include "system.hpp"
#include "download.hpp"
#include "locator.hpp"
#include "compress.hpp"


std::string Patch::indexFilename(const Platform& platform)
{
	return "patches/" + std::string(::stringify(platform)) + "/index.list";
}

std::vector<Version> Patch::list(const Platform& platform,
	const Version& fromversion)
{
	static constexpr int LENGTH = 80;
	static char buffer[LENGTH + 1] = {0};

	if (platform == Platform::UNKNOWN) return {};
	if (fromversion == Version::undefined()) return {};
	size_t maxcount = 65535;
	Version myversion = Version::current();
	std::vector<Version> patchversions;

	// Walk backwards through the patch history looking for patches.
	std::string indexname = indexFilename(platform);
	std::ifstream index = System::ifstream(indexname,
		std::ifstream::in | std::ifstream::binary);
	if (!index.is_open())
	{
		LOGW << "Could not open " << indexname;
		return std::vector<Version>();
	}

	index.seekg(0, std::ifstream::end);

	while (index.tellg() > 0 && patchversions.size() < maxcount)
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

		if (i < LENGTH && strchr("# \t", *(buffer + i)) != nullptr)
		{
			continue;
		}

		if (i < LENGTH)
		{
			Version version;
			try
			{
				version = Version(std::string(buffer + i));
			}
			catch (const ParseError& error)
			{
				LOGE << "Cannot read version: " << error.what();
				RETHROW_IF_DEV();
				return {};
			}

			if (myversion.isDevelopment())
			{
				// This is a dev patch, so we need to apply it.
				patchversions.push_back(version);
			}
			else if (version.major == myversion.major
				&& version.minor <= myversion.minor)
			{
				if (version <= fromversion)
				{
					// This patch and older patches are already part of fromversion.
					// We can stop looking through the index.
					break;
				}

				// If we mention a version twice, it is a break point and we
				// cannot yet patch anything that comes after that version.
				if (!patchversions.empty()
					&& version == patchversions.back())
				{
					patchversions.clear();
				}

				// This patch came out after _fromversion, so we need to apply it.
				patchversions.push_back(version);
			}
			else
			{
				LOGI << "Cannot patch incompatible v" << version;

				// We can stop looking through the index.
				break;
			}
		}
	}

	return patchversions;
}

Version Patch::next(const Platform& platform, const Version& fromversion)
{
	// Get the list of patches that we can apply right away.
	std::vector<Version> patchlist = list(platform, fromversion);

	// If it is empty, there is no patch to apply.
	if (patchlist.empty())
	{
		return Version::undefined();
	}

	// Because Patch::list() reads the index from the back of the index file
	// and clears its working list whenever a break point appears, the first
	// element of patchlist is the first break point that the client encounters.
	return patchlist.front();
}

Patch::Patch(const Platform& platform, const Version& fromversion) :
	_platform(platform),
	_fromversion(fromversion)
{
	// Get the versions that we need to patch.
	std::vector<Version> patchversions = list(_platform, _fromversion);

	// Is there at least one patch to apply?
	if (patchversions.empty())
	{
		return;
	}

	// Is the patch version we're starting from compatible with the current
	// version?
	{
		Version myversion = Version::current();
		Version oldestversion = patchversions.back();
		if ((oldestversion.major != myversion.major
				|| oldestversion.minor > myversion.minor)
			&& !myversion.isDevelopment())
		{
			return;
		}
	}

	// We have found the target version.
	_toversion = patchversions.front();

	// Incorporate all patches.
	for (const Version& patchversion : patchversions)
	{
		std::ifstream list = System::ifstream(
			listfilename(_platform, patchversion), std::ifstream::in);
		if (!list.is_open())
		{
			LOGF << "Could not open " << listfilename(_platform, patchversion);
			throw std::runtime_error("Could not open " + listfilename(_platform, patchversion));
		}

		std::string line;
		while (std::getline(list, line))
		{
			if (std::find(_filenames.begin(), _filenames.end(), line) == _filenames.end())
			{
				_filenames.emplace_back(line);
			}
		}
	}

	// If possible, we get the source files from packages/VERSION/PLATFORM;
	// if this does not exist then we switch to packages/PLATFORM.
	_current = false;
	for (const std::string& filename : _filenames)
	{
		if (!System::isFile(packageFilename(filename)))
		{
			_current = true;
			if (System::isFile(packageFilename(filename)))
			{
				// Either the folder packages/VERSION/PLATFORM does not exist,
				// or a file we need only exists in packages/PLATFORM,
				// we must therefore switch to packages/PLATFORM.
				break;
			}
			else
			{
				// The file does not exist in either directory so we ignore it.
				LOGW << "File '" << filename << "' does not exist";
				_current = false;
			}
		}
	}

	// Remove non-existing files.
	_filenames.erase(std::remove_if(_filenames.begin(), _filenames.end(),
			[this](const std::string& filename) {

		return (!System::isFile(packageFilename(filename)));

	}), _filenames.end());
}

std::string Patch::listfilename(const Platform& platform, const Version& patchversion)
{
	return "patches/" + std::string(::stringify(platform))
		+ "/v" + patchversion.toString() + ".list";
}

std::string Patch::lockedmanifestfilename()
{
	return "patches/tmp/manifest.list";
}

std::string Patch::manifestfilename(const Version& patchversion)
{
	return "manifests/v" + patchversion.toString() + ".list";
}

std::string Patch::mementofilename()
{
	return "downloads/memento.list";
}

std::string Patch::primedlistfilename()
{
	return "downloads/primed.list";
}

std::string Patch::primedrestartfilename()
{
	return "downloads/primed.restart";
}

std::string Patch::packageFilename(const std::string& filename)
{
	if (_current)
	{
		return "packages/"
			+ std::string(::stringify(_platform)) + "/" + filename;
	}
	else
	{
		return "packages/v" + _toversion.toString() + "/"
			+ std::string(::stringify(_platform)) + "/" + filename;
	}
}

std::string Patch::requestFilename(const std::string& filename)
{
	// Is the requested file a picture?
	if (!Locator::pictureName(filename).empty()) {}
	// Or a ruleset?
	else if (!Locator::rulesetName(filename).empty()) {}
	// Or a femtozip model?
	else if (!Locator::fzmodelName(filename).empty()) {}
	// Otherwise it is not requestable.
	else return "";

	// Does the requested file exist?
	if (!System::isFile(filename))
	{
		// Otherwise it is not requestable.
		return "";
	}

	return filename;
}

bool Patch::filenameForbidden(const std::string& filename)
{
	return (filename.empty() || filename.find("prime") != std::string::npos);
}

void Patch::restart()
{
#if SELF_PATCH_ENABLED
	// Tell the launcher to launch the zero program next.
	// The whole purpose of bin/zero (or bin\\zero.exe) was to do nothing.
	// In the new launcher we simple treat this as a request for a restart.
	// For compatibility reasons we do not want to make a new command.
	std::ofstream file = System::ofstream(primedrestartfilename(),
		std::ios::binary | std::ios::trunc);

#ifdef PLATFORMUNIX
	file << "./bin/zero";
#else
	file << ".\\bin\\zero.exe";
#endif
#else
	// We will use exit status 2 instead.
#endif
}

static std::vector<Download> _primedDownloads = {};

void Patch::purge()
{
	_primedDownloads.clear();

#if SELF_PATCH_ENABLED
	System::purgeFile(primedlistfilename());
	System::purgeFile(primedrestartfilename());
	System::purgeFile(mementofilename());
#endif
}

void Patch::prime(std::vector<Download> downloads)
{
	_primedDownloads = std::move(downloads);
}

bool Patch::install(const Download& download)
{
	// Is the file a picture?
	if (!Locator::pictureName(download.targetfilename).empty()) {}
	// Or a ruleset?
	else if (!Locator::rulesetName(download.targetfilename).empty()) {}
	// Or a compression model?
	else if (!Locator::fzmodelName(download.targetfilename).empty()) {}
	else
	{
		// Otherwise it is not installable outside of a full patch.
		LOGE << "File '" << download.targetfilename << "'"
			" is not live-installable";
		return false;
	}

	if (download.executable || download.symbolic
		|| isLive(download.targetfilename))
	{
		LOGE << "Executable file '" << download.targetfilename << "'"
			" is not live-installable";
		return false;
	}

	const std::string& targetfilename = download.targetfilename;
	const std::string& sourcefilename = download.sourcefilename;
	LOGI << "Installing '" << targetfilename << "'"
		" from '" << sourcefilename << "'...";

	// Verify that the download has been downloaded.
	if (!System::isFile(sourcefilename))
	{
		LOGE << "Failed to open " << sourcefilename;
		DEBUG_ASSERT(false);
		return false;
	}

	// Touch the new file and create the necessary folders.
	System::touchFile(targetfilename);
	if (!System::isFile(targetfilename))
	{
		LOGE << "Failed to create " << targetfilename;
		DEBUG_ASSERT(false);
		return false;
	}

	// Copy the downloads to the main folder.
	if (download.compressed)
	{
		Compress::gunzip(sourcefilename, targetfilename);
	}
	else
	{
		System::moveFile(sourcefilename, targetfilename);
	}

	return true;
}

bool Patch::install()
{
#if SELF_PATCH_ENABLED
	if (_primedDownloads.empty())
	{
		LOGE << "No downloads primed for installing.";
		DEBUG_ASSERT(false);
		return false;
	}
	const std::vector<Download>& downloads = _primedDownloads;

	LOGI << "Installing " << std::to_string(downloads.size())
		<< " files...";

	// Verify that the downloads have been downloaded.
	for (const Download& download : downloads)
	{
		if (!System::isFile(download.sourcefilename))
		{
			LOGE << "Failed to open " << download.sourcefilename;
			DEBUG_ASSERT(false);
			return false;
		}
	}

	// Unlink live binaries and libraries to prevent crashes.
	for (const Download& download : downloads)
	{
		if (isLive(download.targetfilename))
		{
			System::unlinkFile(download.targetfilename);

			if (System::isFile(download.targetfilename))
			{
				LOGE << "Failed to unlink " << download.targetfilename;
				DEBUG_ASSERT(false);
				return false;
			}
		}
	}

	// Touch all the new files and create the necessary folders.
	for (const Download& download : downloads)
	{
		System::touchFile(download.targetfilename);

		if (!System::isFile(download.targetfilename))
		{
			LOGE << "Failed to create " << download.targetfilename;
			DEBUG_ASSERT(false);
			return false;
		}
	}

	// Copy the downloads to the main folder.
	for (const Download& download : downloads)
	{
		const std::string& targetfilename = download.targetfilename;
		const std::string& sourcefilename = download.sourcefilename;
		LOGI << "Installing '" << targetfilename << "'"
			" from '" << sourcefilename << "'...";

		if (download.symbolic)
		{
			System::unlinkFile(download.targetfilename);

			LOGV << "Reconstructing symlink, reading data...";
			std::string symtarget;
			{
				std::ifstream file = System::ifstream(download.sourcefilename);
				if (!file || !std::getline(file, symtarget))
				{
					LOGE << "Failed to read " << download.sourcefilename;
					DEBUG_ASSERT(false);
					return false;
				}
			}

			LOGV << "Reconstructing symlink to '" << symtarget << "'...";
			System::makeLink(download.targetfilename, symtarget.c_str());
		}
		else if (download.compressed)
		{
			Compress::gunzip(sourcefilename, targetfilename);
		}
		else
		{
			System::moveFile(sourcefilename, targetfilename);
		}
	}

	// Make the appropriate files binary.
	for (const Download& download : downloads)
	{
		if (download.executable)
		{
			System::makeFileExecutable(download.targetfilename);
		}
	}

	// Patch installed.
	return true;
#else
	LOGE << "Cannot install primed downloads: self patch not enabled.";
	DEBUG_ASSERT(false);
	return false;
#endif
}

bool Patch::isLive(const std::string& filename)
{
	return strncmp(filename.c_str(), "epicinium", 9) == 0
		|| strncmp(filename.c_str(), "bin/", 4) == 0;
}
