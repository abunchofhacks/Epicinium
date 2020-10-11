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
#pragma once
#include "header.hpp"

#include "version.hpp"

enum class Platform : uint8_t;
struct Download;


class Patch
{
public:
	Patch(const Platform& platform, const Version& fromversion);

private:
	Platform _platform;
	Version _fromversion;
	Version _toversion;
	bool _current;
	std::vector<std::string> _filenames;

public:
	const Platform& platform() const { return _platform; }
	const Version& version() const { return _toversion; }
	const std::vector<std::string>& contents() const { return _filenames; }

	explicit operator bool() const
	{
		return _toversion != Version::undefined();
	}

private:
	static std::string indexFilename(const Platform& platform);

public:
	std::string packageFilename(const std::string& filename);

	static std::string requestFilename(const std::string& filename);

	static std::vector<Version> list(const Platform& platform,
		const Version& fromversion);
	static Version next(const Platform& platform,
		const Version& fromversion);

	static std::string primedlistfilename();
	static std::string primedrestartfilename();
	static std::string listfilename(const Platform& platform,
		const Version& patchversion);
	static std::string lockedmanifestfilename();
	static std::string manifestfilename(const Version& patchversion);
	static std::string mementofilename();

	static bool filenameForbidden(const std::string& filename);

	static void restart();
	static void purge();

	static void prime(std::vector<Download> downloads);

	static bool install();
	static bool install(const Download& download);

private:
	static bool isLive(const std::string& filename);
};
