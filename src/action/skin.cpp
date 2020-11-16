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
#include "skin.hpp"
#include "source.hpp"

#include "stringref.hpp"
#include "surface.hpp"
#include "figure.hpp"
#include "system.hpp"
#include "writer.hpp"
#include "colorname.hpp"
#include "parseerror.hpp"


std::string Skin::_skinsfolder = "resources/";

void Skin::setResourceRoot(const std::string& root)
{
	if (root.empty())
	{
		_skinsfolder = "resources/";
	}
	else if (root.back() == '/')
	{
		_skinsfolder = root + "resources/";
	}
	else
	{
		_skinsfolder = root + "/resources/";
	}
}

std::map<std::string, std::shared_ptr<Skin>> Skin::_skins = {
	{"missing", Skin::missingSkin()},
	{"surfaces/missing", Surface::missingSkin()},
	{"figures/missing", Figure::missingSkin()},
};

std::shared_ptr<Skin> Skin::missingSkin()
{
	auto skinptr = std::make_shared<Skin>();
	Skin& skin = *skinptr;
	skin.name = "missing";
	skin.spritename = "ui/missing";
	skin._palettedata = {
		Paint::blend(Color::broken(), ColorName::SHINEBLEND),
		Color::broken(),
		Paint::blend(Color::broken(), ColorName::SHADEBLEND),
		Color::transparent(),
	};
	skin.palettes = {skin._palettedata.data()};
	skin.palettenames = {"basic"};
	skin.palettesize = skin._palettedata.size();
	return skinptr;
}

std::string Skin::filename(const std::string& skinname)
{
	return _skinsfolder + skinname + ".json";
}

void Skin::save(const std::string& savename) const
{
	std::string fname = filename(savename);
	LOGI << "Saving " << fname;

	System::touchFile(fname);
	std::ofstream file = System::ofstream(fname);
	if (!file.is_open())
	{
		LOGE << "Could not open " << fname;
		DEBUG_ASSERT(false);
		return;
	}

	Json::Value json(Json::objectValue);
	json["name"] = name;
	if (name == savename)
	{
		storeContentsIntoJson(json);
	}

	Json::StyledWriter writer;
	file << writer.write(json);

	LOGI << "Saved " << fname;
}

void Skin::storeContentsIntoJson(Json::Value& json) const
{
	json["spritename"] = spritename;
	if (!_randomspritenames.empty())
	{
		json["randomspritenames"] = Json::arrayValue;
		for (const std::string& elem : _randomspritenames)
		{
			json["randomspritenames"].append(elem);
		}
	}
	if (palettesize) json["palettesize"] = palettesize;
	for (size_t i = 0; i < palettenames.size(); i++)
	{
		Json::Value list(Json::arrayValue);
		int nulloffset = -1;
		for (int offset = 0; offset < palettesize; offset++)
		{
			const Paint& recipe = *(palettes[i] + offset);
			if (recipe)
			{
				if (nulloffset >= 0)
				{
					for (int l = nulloffset; l < offset; l++)
					{
						list.append(Json::nullValue);
					}
					nulloffset = -1;
				}
				list.append(recipe.toJson());
			}
			else if (nulloffset < 0) nulloffset = offset;
		}
		json["palette"][palettenames[i]] = list;
	}
	if (!selectionspritename.empty())
	{
		json["selectionspritename"] = selectionspritename;
	}
	if (!shadowspritename.empty())
	{
		json["shadowspritename"] = shadowspritename;
	}
	if (!iconspritename.empty())
	{
		json["iconspritename"] = iconspritename;
	}
	if (formtype != FormType::NONE)
	{
		json["formtype"] = ::stringify(formtype);
	}
	if (texturetype != TextureType::NONE)
	{
		json["texturetype"] = ::stringify(texturetype);
	}
	if (baseOffset) json["base-offset"] = baseOffset;
	if (yahooOffset) json["yahoo-offset"] = yahooOffset;
	if (deathsound) json["deathsound"] = deathsound;
}

std::shared_ptr<Skin> Skin::clone(const std::string& newname) const
{
	Json::Value json(Json::objectValue);
	storeContentsIntoJson(json);

	auto newskinptr = std::make_shared<Skin>();
	newskinptr->name = newname;
	newskinptr->fillContentsFromJson(json);

	_skins.emplace(newname, newskinptr);
	return newskinptr;

}

std::shared_ptr<Skin> Skin::loadNow(const std::string& skinname)
{
	if (!System::isFile(filename(skinname)))
	{
		LOGW << "Missing " << filename(skinname);
		return loadMissing(skinname);
	}

	try
	{
		std::ifstream file = System::ifstream(filename(skinname));
		Json::Reader reader;
		Json::Value json;
		if (!reader.parse(file, json))
		{
			LOGE << "Failed to open " << filename(skinname);
			DEBUG_ASSERT(false);
			return loadMissing(skinname);
		}

		std::string named = json["name"].asString();
		if (named != skinname)
		{
			// Load 'named' instead. The recursion depth is unbounded.
			// TODO check against circular skins
			auto loadedskin = load(named);
			_skins.emplace(skinname, loadedskin);
			return loadedskin;
		}

		auto skinptr = std::make_shared<Skin>();
		skinptr->name = skinname;
		skinptr->fillContentsFromJson(json);

		_skins.emplace(skinname, skinptr);
		return skinptr;
	}
	catch (const Json::Exception& error)
	{
		LOGE << "Failed to parse " << filename(skinname)
			<< ": " << error.what();
		RETHROW_IF_DEV();
		return loadMissing(skinname);
	}
}

void Skin::fillContentsFromJson(const Json::Value& json)
{
	spritename = json["spritename"].asString();

	if (!json["randomspritenames"].isNull())
	{
		_randomspritenames.reserve(json["randomspritenames"].size());
		for (const Json::Value& elem : json["randomspritenames"])
		{
			_randomspritenames.emplace_back(elem.asString());
		}
	}

	if (!json["palette"].isNull())
	{
		palettesize = json["palettesize"].asInt();
		int n = json["palette"].size();
		palettes.reserve(n);
		palettenames.reserve(n);
		_palettedata.resize(n * palettesize);
		Paint* cur = _palettedata.data();
		for (const std::string& pname : json["palette"].getMemberNames())
		{
			const Json::Value& pal = json["palette"][pname];
			for (unsigned int offset = 0; offset < pal.size(); offset++)
			{
				Paint paint;
				try
				{
					paint = Paint::parse(pal[offset]);
				}
				catch (ParseError& error)
				{
					LOGE << "Failed to parse skin palette paint: "
						<< error.what();
					RETHROW_IF_DEV();
					paint = Color::broken();
				}
				*(cur + offset) = paint;
			}
			palettes.emplace_back(cur);
				palettenames.emplace_back(pname);
			cur += palettesize;
		}
	}

	if (!json["selectionspritename"].isNull())
	{
		selectionspritename = json["selectionspritename"].asString();
	}

	if (!json["shadowspritename"].isNull())
	{
		shadowspritename = json["shadowspritename"].asString();
	}

	if (!json["iconspritename"].isNull())
	{
		iconspritename = json["iconspritename"].asString();
	}

	if (!json["formtype"].isNull())
	{
		formtype = parseFormType(json["formtype"].asString());
	}

	if (!json["texturetype"].isNull())
	{
		texturetype = parseTextureType(json["texturetype"].asString());
	}

	if (!json["base-offset"].isNull())
	{
		baseOffset = json["base-offset"].asInt();
	}

	if (!json["yahoo-offset"].isNull())
	{
		yahooOffset = json["yahoo-offset"].asInt();
	}

	if (!json["deathsound"].isNull())
	{
		deathsound = json["deathsound"].asBool();
	}
}

std::shared_ptr<Skin> Skin::loadMissing(const std::string& skinname)
{
	std::string loaded;

	// Search order: abc/def/xyz, abc/def/missing, abc/missing, missing.
	size_t pos = skinname.find_last_of('/');
	if (pos != std::string::npos)
	{
		std::string dir = skinname.substr(0, pos);
		if (skinname.compare(pos + 1, std::string::npos, "missing") != 0)
		{
			loaded = dir + "/missing";
		}
		else
		{
			pos = dir.find_last_of('/');
			if (pos != std::string::npos)
			{
				loaded = dir.substr(0, pos) + "/missing";
			}
			else
			{
				loaded = "missing";
			}
		}
	}
	else
	{
		loaded = "missing";
	}

	// Load 'loaded' instead; because "missing" always exists, the recursion
	// depth is bounded by the number of slashes in skinname plus one.
	std::shared_ptr<Skin> loadedskin = load(loaded);
	_skins.emplace(skinname, loadedskin);
	return loadedskin;
}

std::shared_ptr<Skin> Skin::load(const std::string& skinname)
{
	{
		auto iter = _skins.find(skinname);
		if (iter != _skins.end()) return iter->second;
	}

	return loadNow(skinname);
}

const Skin& Skin::get(const std::string& skinname)
{
	return *load(skinname);
}

const std::string& Skin::randomspritename() const
{
	if (!_randomspritenames.empty())
	{
		return _randomspritenames[rand() % _randomspritenames.size()];
	}
	else return spritename;
}

const Paint* Skin::palette(stringref name) const
{
	for (size_t i = 0; i < palettenames.size(); i++)
	{
		if (palettenames[i] == name)
		{
			return palettes[i];
		}
	}
	return nullptr;
}

const char* stringify(FormType type)
{
	switch (type)
	{
		case FormType::NONE: return "none";
		case FormType::PERSON: return "person";
		case FormType::VEHICLE: return "vehicle";
		case FormType::BALLOON: return "balloon";
		case FormType::BUILDING: return "building";
		case FormType::FENCE: return "fence";
		case FormType::TREE: return "tree";
		case FormType::MOUNTAIN: return "mountain";
	}
	return "";
}

FormType parseFormType(stringref str)
{
	if (str == "none") return FormType::NONE;
	if (str == "person") return FormType::PERSON;
	if (str == "vehicle") return FormType::VEHICLE;
	if (str == "balloon") return FormType::BALLOON;
	if (str == "building") return FormType::BUILDING;
	if (str == "fence") return FormType::FENCE;
	if (str == "tree") return FormType::TREE;
	if (str == "mountain") return FormType::MOUNTAIN;

	LOGE << "Unknown FormType '" << str << "'";
	DEBUG_ASSERT(false);
	return FormType::NONE;
}

const char* stringify(TextureType type)
{
	switch (type)
	{
		case TextureType::NONE: return "none";
		case TextureType::GRASS: return "grass";
		case TextureType::DIRT: return "dirt";
		case TextureType::DESERT: return "desert";
		case TextureType::STONE: return "stone";
		case TextureType::MOUNTAIN: return "mountain";
		case TextureType::WATER: return "water";
	}
	return "";
}

TextureType parseTextureType(stringref str)
{
	if (str == "none") return TextureType::NONE;
	if (str == "grass") return TextureType::GRASS;
	if (str == "dirt") return TextureType::DIRT;
	if (str == "desert") return TextureType::DESERT;
	if (str == "stone") return TextureType::STONE;
	if (str == "mountain") return TextureType::MOUNTAIN;
	if (str == "water") return TextureType::WATER;

	LOGE << "Unknown TextureType '" << str << "'";
	DEBUG_ASSERT(false);
	return TextureType::NONE;
}

std::ostream& operator<<(std::ostream& os, const FormType& type)
{
	return os << ::stringify(type);
}

std::ostream& operator<<(std::ostream& os, const TextureType& type)
{
	return os << ::stringify(type);
}
