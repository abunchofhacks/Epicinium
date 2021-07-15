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


namespace System
{
#ifdef PLATFORMWINDOWS
	std::wstring utf16FromUtf8(const std::string& str);
#endif

	std::vector<std::string> listDirectory(const std::string& filename,
		const std::string& extension);

	FILE* fopen(const std::string& filename, const char* mode);
	std::ifstream ifstream(const std::string& filename);
	std::ifstream ifstream(const std::string& filename,
		std::ios_base::openmode mode);
	std::ofstream ofstream(const std::string& filename);
	std::ofstream ofstream(const std::string& filename,
		std::ios_base::openmode mode);

	bool isDirectory(const std::string& filename);
	bool isFile(const std::string& filename);
	bool isFileNonempty(const std::string& filename);
	bool isFileExecutable(const std::string& filename);
	void makeFileExecutable(const std::string& filename);

	std::string getHttpModificationTimeString(const std::string& filename);

	void touchFile(const std::string& filename);
	void touchDirectory(const std::string& dirname);
	void touchParentDirectory(const std::string& filename);

	void purgeFile(const std::string& filename);
	void unlinkFile(const std::string& filename);

	void moveFile(const std::string& filename, const std::string& dest);
	void copyFile(const std::string& filename, const std::string& dest);

	bool isLink(const std::string& filename);
	std::string readLink(const std::string& filename);
	void makeLink(const std::string& filename, const char* target);

	std::string absolutePath(const std::string& filename);

	bool hasStorageIssuesForSelfPatch();

	std::string getPersistentConfigRoot();
	std::string getPersistentDataRoot();
	std::string getPersistentCacheRoot();
}
