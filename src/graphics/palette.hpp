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

#include "color.hpp"


enum class ColorName : uint8_t;

class Palette
{
public:
	struct ExternalItem
	{
		std::string uniqueTag;
		std::string quotedName;
		std::string sourceFilename;
	};

private:
	std::vector<Color> _colors;
	std::vector<Color> _savedColors;

public:
	static Palette parse(const Json::Value& json);
	Json::Value toJson() const;

	bool isComplete() const;

	void makeCompleteUsing(const Palette& other);

	bool hasUnsavedChanges() const;

	void save(const std::string& palettename);
	static Palette load(const std::string& palettename);
	static bool exists(const std::string& palettename);
	static bool unsaved();

	static void saveInstalledAs(const std::string& palettename);
	static void installDefault();
	static void installNamed(const std::string& palettename);

	static Color get(const ColorName& name);
	static void set(const ColorName& name, Color color);

	static void loadIndex();
	static void listExternalItem(ExternalItem&& item);
	static void unlistExternalItem(const std::string& uniqueTag);

	static const std::vector<std::string>& indexedNames();
	static const std::vector<ExternalItem>& externalItems();

private:
	static void loadIndexFromDirectory(const std::string& dirname);

	static std::string _sourcepalettesfolder;
	static std::string _palettesfolder;

public:
	static void setResourceRoot(const std::string& root);
	static void setAuthoredRoot(const std::string& root);

	static std::string sourceFilename(const std::string& name);
	static std::string authoredFilename(const std::string& name);

	static std::string paletteNameFromWorkshopId(const std::string& id);
};
