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


class Locator
{
public:
	static std::string picture(const std::string& picturename);

	static std::string pictureFilename(const std::string& picturename);
	static std::string pictureName(const std::string& filename);
	static std::string rulesetFilename(const std::string& rulesetname);
	static std::string rulesetResourceFilename(const std::string& rulesetname);
	static std::string rulesetName(const std::string& filename);
	static std::string fzmodelFilename(const std::string& fzmodelname);
	static std::string fzmodelName(const std::string& filename);

	static std::string getRelativeFilename(const std::string& filename);

private:
	static std::string _resourceroot;
	static std::string _cacheroot;

public:
	static void setResourceRoot(const std::string& root);
	static void setCacheRoot(const std::string& root);
};
