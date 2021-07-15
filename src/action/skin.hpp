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

#include <map>

#include "paint.hpp"
#include "stringref.hpp"


enum class FormType : uint8_t
{
	NONE = 0,
	PERSON,
	VEHICLE,
	BALLOON,
	BUILDING,
	FENCE,
	TREE,
	MOUNTAIN,
};

const char* stringify(FormType type);
FormType parseFormType(stringref str);
std::ostream& operator<<(std::ostream& os, const FormType& type);

enum class TextureType : uint8_t
{
	NONE = 0,
	GRASS,
	DIRT,
	DESERT,
	STONE,
	MOUNTAIN,
	WATER,
};

const char* stringify(TextureType type);
TextureType parseTextureType(stringref str);
std::ostream& operator<<(std::ostream& os, const TextureType& type);

struct Skin
{
private:
	static std::string _skinsfolder;

public:
	static void setResourceRoot(const std::string& root);

	Skin() = default;
	Skin(const Skin&) = delete;
	Skin(Skin&&) = delete;
	Skin& operator=(const Skin&) = delete;
	Skin& operator=(Skin&&) = delete;
	~Skin() = default;

	std::string name;
	std::string spritename;
	std::string selectionspritename;
	std::string shadowspritename;
	std::string iconspritename;

private:
	friend class Figure;
	friend class Surface;
	friend class SkinEditor;

	std::vector<std::string> _randomspritenames;
	std::vector<Paint> _palettedata;

	// Each palette is a pointer to an array of recipes stored somewhere in
	// _palettedata, valid from index 0 to palettesize - 1.
	std::vector<Paint*> palettes; // (married)
	std::vector<std::string> palettenames; // (married)

public:
	FormType formtype = FormType::NONE;
	TextureType texturetype = TextureType::NONE;
	int palettesize = 0;
	int baseOffset = 0;
	int yahooOffset = 0;
	bool deathsound = true;
	bool trimmed = false;
	bool ninepatched = false;

	static std::map<std::string, std::shared_ptr<Skin>> _skins;

private:
	void save(const std::string& skinname) const;

	void storeContentsIntoJson(Json::Value& json) const;
	void fillContentsFromJson(const Json::Value& json);

	std::shared_ptr<Skin> clone(const std::string& newname) const;

	static std::shared_ptr<Skin> load(const std::string& name);
	static std::shared_ptr<Skin> loadNow(const std::string& name);
	static std::shared_ptr<Skin> loadMissing(const std::string& name);

	static std::string filename(const std::string& name);

public:
	static const Skin& get(const std::string& name);

	const std::string& randomspritename() const;
	const Paint* palette(stringref name) const;

private:
	static std::shared_ptr<Skin> missingSkin();
};
