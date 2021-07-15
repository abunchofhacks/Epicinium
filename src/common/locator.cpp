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
std::string Locator::_authoredroot = "";
std::vector<Locator::ExternalFolder> Locator::_externalfolders = {};

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

void Locator::setAuthoredRoot(const std::string& root)
{
	if (root.empty())
	{
		_authoredroot = "";
	}
	else if (root.back() == '/')
	{
		_authoredroot = root;
	}
	else
	{
		_authoredroot = root + "/";
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
	else if (filename.size() > _authoredroot.size()
		&& filename.compare(0, _authoredroot.size(), _authoredroot) == 0)
	{
		// Cut off the root.
		return filename.substr(_authoredroot.size());
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
	if (picturename.find_first_of('@') != std::string::npos)
	{
		size_t seppos = picturename.find_first_of('/');
		std::string shortname;
		// Use x.rfind(y, 0) == 0 as x.starts_with(y).
		if (picturename.rfind("panels/", 0) == 0)
		{
			shortname = "panel";
		}

		if (!shortname.empty() && seppos != std::string::npos)
		{
			std::string tag = picturename.substr(seppos + 1);
			for (const auto& folder : _externalfolders)
			{
				if (folder.uniqueTag == tag)
				{
					return folder.sourcePath + shortname + ".png";
				}
			}
			LOGW << "Failed to find external tag '" << tag << "'"
				" for picture '" << picturename << "'";
		}
		else
		{
			LOGW << "Failed to determine shortname"
				" for picture '" << picturename << "'";
		}
	}

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
	if (rulesetname.find_first_of('@') != std::string::npos)
	{
		std::string tag = rulesetname;
		for (const auto& folder : _externalfolders)
		{
			if (folder.uniqueTag == tag)
			{
				return folder.sourcePath + "ruleset.json";
			}
		}
		size_t seppos = rulesetname.find_first_of('@');
		std::string subpath = rulesetname.substr(seppos + 1);
		return _cacheroot + "rulesets/external/" + subpath + ".json";
	}
	return _cacheroot + "rulesets/" + rulesetname + ".json";
}

std::string Locator::rulesetResourceFilename(const std::string& rulesetname)
{
	return _resourceroot + "rulesets/" + rulesetname + ".json";
}

std::string Locator::rulesetAuthoredFilename(const std::string& rulesetname)
{
	return _authoredroot + "rulesets/" + rulesetname + ".json";
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

void Locator::useExternalFolder(ExternalFolder&& newFolder)
{
	LOGD << "Using '" << newFolder.uniqueTag << "'"
		": " << newFolder.sourcePath;

	for (auto& folder : _externalfolders)
	{
		if (folder.uniqueTag == newFolder.uniqueTag)
		{
			folder = newFolder;
			return;
		}
	}

	_externalfolders.emplace_back(newFolder);
}

void Locator::forgetExternalFolder(const std::string& uniqueTag)
{
	LOGD << "Forgetting '" << uniqueTag << "'";
	_externalfolders.erase(
		std::remove_if(
			_externalfolders.begin(),
			_externalfolders.end(),
			[&](const ExternalFolder& folder) {
				return folder.uniqueTag == uniqueTag;
			}),
		_externalfolders.end());
}

std::vector<std::string> Locator::externalRulesets()
{
	std::vector<std::string> rulesets;
	for (const auto& folder : _externalfolders)
	{
		if (System::isFile(folder.sourcePath + "ruleset.json"))
		{
			rulesets.emplace_back(folder.uniqueTag);
		}
	}
	return rulesets;
}

const std::vector<Locator::ExternalFolder>& Locator::externalFolders()
{
	return _externalfolders;
}

std::vector<std::string> Locator::listAuthoredRulesets()
{
	auto list = System::listDirectory(_authoredroot + "rulesets/", ".json");
	std::sort(list.begin(), list.end());
	return list;
}
