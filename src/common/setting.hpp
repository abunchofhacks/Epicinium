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


enum class ScreenMode : uint8_t { WINDOWED, BORDERLESS, FULLSCREEN, DESKTOP };
enum class SelectorMode : uint8_t { CONTEXT, FIGURE };
enum class ArtPanMode : uint8_t { NONE, AUTO, CURSOR };
enum class PatchMode : uint8_t
{
	NONE,
	SERVER,
	SERVER_BUT_DISABLED_DUE_TO_STORAGE_ISSUES,
	ITCHIO,
	GAMEJOLT,
	STEAM
};

class Settings;

template <typename T>
class Setting
{
private:
	template <typename S>
	friend class Setting;

	friend Settings;

	Settings* const _owner;
	const size_t _index;
	const char* const _name; // nullable
	const char* const _altname; // nullable

	bool _defined;
	T _value;

public:
	Setting(Settings* owner, /*nullable*/ const char* name);
	Setting(Settings* owner, const char* name, const char* altname);

	Setting& operator=(const T& value);
	void override(const T& value);
	void clear();
	bool defined() const;
	const Setting& definition() const;
	const T& value() const;
	const T& value(const T& fallback) const;

private:
	bool parse(const Json::Value& root);
	bool parse(const Json::Value& root, const char* name);
	void store(Json::Value& root) const;
};
