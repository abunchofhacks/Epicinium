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
#include "locator.hpp"
#include "source.hpp"

#include "system.hpp"


std::string Locator::_resourceroot = "";
std::string Locator::_cacheroot = "";

void Locator::setResourceRoot(const std::string& root)
{
	if (root.empty())
	{
		_resourceroot = "";
	}
	else if (root.back() == '/')
	{
		_resourceroot = root;
	}
	else
	{
		_resourceroot = root + "/";
	}
}

void Locator::setCacheRoot(const std::string& root)
{
	if (root.empty())
	{
		_cacheroot = "";
	}
	else if (root.back() == '/')
	{
		_cacheroot = root;
	}
	else
	{
		_cacheroot = root + "/";
	}
}

std::string Locator::getRelativeFilename(const std::string& filename)
{
	if (filename.size() > _cacheroot.size()
		&& filename.compare(0, _cacheroot.size(), _cacheroot) == 0)
	{
		// Cut off the cacheroot.
		return filename.substr(_cacheroot.size());
	}
	else if (filename.size() > _resourceroot.size()
		&& filename.compare(0, _resourceroot.size(), _resourceroot) == 0)
	{
		// Cut off the root.
		return filename.substr(_resourceroot.size());
	}
	else
	{
		return filename;
	}
}

std::string Locator::picture(const std::string& picturename)
{
	std::string filename = pictureFilename(picturename);
	// Use the pre-installed version as a fallback, if it exists, but
	// we'll try to download a new version later (see Picture and EpiCDN).
	std::string fallbackfilename = _resourceroot
		+ getRelativeFilename(filename);
	if (System::isFile(filename)) return filename;
	else if (System::isFile(fallbackfilename)) return fallbackfilename;
	else return _resourceroot + "pictures/unknown.png";
}

std::string Locator::pictureFilename(const std::string& picturename)
{
	return _cacheroot + "pictures/" + picturename + ".png";
}

std::string Locator::pictureName(const std::string& fullfilename)
{
	std::string filename = getRelativeFilename(fullfilename);
	size_t slashpos = filename.find_first_of('/');
	size_t dotpos = filename.find_last_of('.');
	if (filename.substr(0, slashpos) == "pictures"
		&& filename.substr(dotpos, std::string::npos) == ".png")
	{
		return filename.substr(slashpos + 1, dotpos - (slashpos + 1));
	}
	return "";
}

std::string Locator::rulesetFilename(const std::string& rulesetname)
{
	return _cacheroot + "rulesets/" + rulesetname + ".json";
}

std::string Locator::rulesetResourceFilename(const std::string& rulesetname)
{
	return _resourceroot + "rulesets/" + rulesetname + ".json";
}

std::string Locator::rulesetName(const std::string& fullfilename)
{
	std::string filename = getRelativeFilename(fullfilename);
	size_t slashpos = filename.find_first_of('/');
	size_t dotpos = filename.find_last_of('.');
	if (filename.substr(0, slashpos) == "rulesets"
		&& filename.substr(dotpos, std::string::npos) == ".json")
	{
		return filename.substr(slashpos + 1, dotpos - (slashpos + 1));
	}
	return "";
}

std::string Locator::fzmodelFilename(const std::string& fzmodelname)
{
	return _cacheroot + "sessions/" + fzmodelname + ".fzm";
}

std::string Locator::fzmodelName(const std::string& fullfilename)
{
	std::string filename = getRelativeFilename(fullfilename);
	size_t slashpos = filename.find_first_of('/');
	size_t dotpos = filename.find_last_of('.');
	if (filename.substr(0, slashpos) == "sessions"
		&& filename.substr(dotpos, std::string::npos) == ".fzm")
	{
		return filename.substr(slashpos + 1, dotpos - (slashpos + 1));
	}
	return "";
}
