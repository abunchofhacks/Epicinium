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

class Settings;


class Language
{
private:
	static std::string _locdir;

public:
	static void setRoot(const std::string& root);

public:
	class ScopedOverride
	{
	public:
		ScopedOverride(const std::string& tag);
		ScopedOverride(const ScopedOverride&) = delete;
		ScopedOverride(ScopedOverride&&) = delete;
		ScopedOverride& operator=(const ScopedOverride&) = delete;
		ScopedOverride& operator=(ScopedOverride&&) = delete;
		~ScopedOverride();

#if INTL_ENABLED
	private:
		std::string _oldvalue;
#endif
	};

	static void use(const Settings& settings);

	static bool isCurrentlyEnglish();

	static std::vector<std::string> supportedTags();
	static std::vector<std::string> experimentalTags();
	static std::vector<std::string> incompleteTags();
	static std::vector<std::string> allDetectedTags();

	static std::string getNameInOwnLanguage(const std::string& tag);
	static std::string getNameInActiveLanguage(const std::string& tag);

public:
	// This replaces libintl's gettext(), except it returns std::string.
	static std::string gettext(const char* message);
};
