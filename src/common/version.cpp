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
#include "version.hpp"
#include "source.hpp"

#include "parseerror.hpp"


Version Version::current()
{
#ifdef DEVELOPMENT
#ifdef CANDIDATE
	return latest();
#else
	return Version(255, 255, 255, 1);
#endif
#else
	return latest().release();
#endif
}

Version Version::latest()
{
	return Version(1, 0, 0, 1);
}

std::istream& operator>>(std::istream &is, Version &version);

Version::Version(const std::string& str)
{
	std::stringstream stream(str);
	stream >> *this;
}

Version::Version(const Json::Value& json) :
	Version(json.asString())
{}

std::string Version::toString() const
{
	std::stringstream stream;
	stream << *this;
	return stream.str();
}

std::string Version::name() const
{
	std::stringstream stream;
	stream << "v" << *this;
	return stream.str();
}

std::ostream& operator<<(std::ostream& os, const Version& vers)
{
	os << (int) vers.major << "." << (int) vers.minor << "." << (int) vers.patch;
	if (vers.rcand) os << "-rc" << (int) vers.rcand;
	return os;
}

std::istream& operator>>(std::istream &is, Version &version)
{
	if (is.peek() == 'v') is.ignore();
	int major, minor, patch;
	if (is.peek() < '0' || is.peek() > '9') throw ParseError("Unrecognized version");
	is >> major;
	if (major > 255) throw ParseError("Unrecognized version");
	is.ignore();
	if (is.peek() < '0' || is.peek() > '9') throw ParseError("Unrecognized version");
	is >> minor;
	if (minor > 255) throw ParseError("Unrecognized version");
	is.ignore();
	if (is.peek() < '0' || is.peek() > '9') throw ParseError("Unrecognized version");
	is >> patch;
	if (patch > 255) throw ParseError("Unrecognized version");
	int rcand = 0;
	if (is.peek() == '-')
	{
		is.ignore();
		is.ignore();
		is.ignore();
		if (is.peek() < '0' || is.peek() > '9') throw ParseError("Unrecognized version");
		is >> rcand;
		if (rcand > 255) throw ParseError("Unrecognized version");
	}
	if (!is.eof()) throw ParseError("Unrecognized version");
	version.major = major;
	version.minor = minor;
	version.patch = patch;
	version.rcand = rcand;
	return is;
}

bool operator==(const Version& a, const Version& b)
{
	return (a.major == b.major
		&&  a.minor == b.minor
		&&  a.patch == b.patch
		&&  a.rcand == b.rcand);
}

bool operator!=(const Version& a, const Version& b)
{
	return !(a == b);
}

/* 2.9.3 < 2.9.4 < 2.9.4-rc1 < 2.9.4-rc2 < 2.10.0 < 3.0.0 */
bool operator<(const Version& a, const Version& b)
{
	return (                       a.major < b.major
		|| (a.major == b.major && (a.minor < b.minor
		|| (a.minor == b.minor && (a.patch < b.patch
		|| (a.patch == b.patch && (a.rcand < b.rcand)))))));
}

bool operator<=(const Version& a, const Version& b)
{
	return (a < b || a == b);
}

bool operator>(const Version& a, const Version& b)
{
	return (b < a);
}

bool operator>=(const Version& a, const Version& b)
{
	return (b <= a);
}

bool Version::isReleaseCandidateOf(const Version& other) const
{
	return (major == other.major
		&&  minor == other.minor
		&&  patch == other.patch
		&& rcand && !other.rcand);
}

bool Version::tryBecomeParsed(const std::string& str)
{
	try
	{
		*this = Version(str);
		return true;
	}
	catch (ParseError& error)
	{
		LOGE << "Error while trying to parse '" << str << "'";
		return false;
	}
}
