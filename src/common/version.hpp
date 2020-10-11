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


/* Factories cause pollution in the global namespace. */
#pragma push_macro("major")
#undef major
#pragma push_macro("minor")
#undef minor
/* Factories cause pollution in the global namespace. */

struct Version
{
public:
	uint8_t major;
	uint8_t minor;
	uint8_t patch;
	uint8_t rcand;

	constexpr Version(int major, int minor, int patch, int rcand) :
		major(major),
		minor(minor),
		patch(patch),
		rcand(rcand)
	{}

	constexpr Version() :
		Version(undefined())
	{}

	static Version current();

	static Version latest();

	static constexpr Version prehistoric()
	{
		return Version(0, 0, 0, 0);
	}

	static constexpr Version undefined()
	{
		return Version(255, 255, 255, 255);
	}

	explicit Version(const std::string& str);
	explicit Version(const Json::Value& json);

	std::string toString() const;

	std::string name() const;

	constexpr bool isWellDefined() const
	{
		return ((major >   0 || minor >   0 || patch >   0)
			&&  (major < 255 || minor < 255 || patch < 255));
	}

	constexpr bool isDevelopment() const
	{
		return (major == 255 && minor == 255 && patch == 255 && rcand != 255);
	}

	constexpr bool isRelease() const
	{
		 return isWellDefined() && (rcand == 0);
	}

	constexpr bool isReleaseCandidate() const
	{
		return isWellDefined() && (rcand != 0);
	}

	bool isReleaseCandidateOf(const Version& other) const;

	constexpr Version release() const
	{
		return Version(major, minor, patch, 0);
	}

	bool tryBecomeParsed(const std::string& str);

	constexpr uint32_t asUint32() const
	{
		return (major << 24) | (minor << 16) | (patch << 8) | rcand;
	}
};

std::ostream& operator<<(std::ostream& os, const Version& version);

bool operator==(const Version& a, const Version& b);
bool operator!=(const Version& a, const Version& b);

bool operator<(const Version& a, const Version& b);
bool operator<=(const Version& a, const Version& b);
bool operator>(const Version& a, const Version& b);
bool operator>=(const Version& a, const Version& b);

/* Factories cause pollution in the global namespace. */
#pragma pop_macro("minor")
#pragma pop_macro("major")
/* Factories cause pollution in the global namespace. */
