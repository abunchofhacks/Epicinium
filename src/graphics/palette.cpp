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
#include "palette.hpp"
#include "source.hpp"

#include "color.hpp"
#include "colorname.hpp"
#include "system.hpp"
#include "parseerror.hpp"


static Palette _installed_master_palette;

void Palette::install(Palette master)
{
	_installed_master_palette = std::move(master);
}

std::string Palette::_sourcepalettesfolder = "resources/palettes/";
std::string Palette::_palettesfolder = "resources/palettes/";

void Palette::setResourceRoot(const std::string& root)
{
	if (root.empty())
	{
		_sourcepalettesfolder = "resources/palettes/";
	}
	else if (root.back() == '/')
	{
		_sourcepalettesfolder = root + "resources/palettes/";
	}
	else
	{
		_sourcepalettesfolder = root + "/resources/palettes/";
	}
}

void Palette::setAuthoredRoot(const std::string& root)
{
	if (root.empty())
	{
		_palettesfolder = "resources/palettes/";
	}
	else if (root.back() == '/')
	{
		_palettesfolder = root + "resources/palettes/";
	}
	else
	{
		_palettesfolder = root + "/resources/palettes/";
	}
}

Palette Palette::parse(const Json::Value& json)
{
	Palette palette;
	palette._colors.reserve(COLORNAME_SIZE);
	for (size_t i = 0; i < COLORNAME_SIZE; i++)
	{
		const Json::Value& item = json[::stringify((ColorName) i)];
		if (item.isNull())
		{
			palette._colors.push_back(Color::undefined());
		}
		else if (item.isString())
		{
			try
			{
				palette._colors.push_back(Color::parse(item.asString()));
			}
			catch (ParseError& error)
			{
				LOGE << "Invalid color in palette";
				DEBUG_ASSERT(false);
				palette._colors.push_back(Color::broken());
			}
		}
		else
		{
			LOGE << "Invalid color in palette";
			DEBUG_ASSERT(false);
			palette._colors.push_back(Color::broken());
		}
	}
	return palette;
}

Json::Value Palette::toJson() const
{
	Json::Value json = Json::objectValue;
	for (size_t i = 0; i < COLORNAME_SIZE; i++)
	{
		if (i < _colors.size() && _colors[i])
		{
			json[::stringify((ColorName) i)] = _colors[i].toString();
		}
	}
	return json;
}

bool Palette::isComplete() const
{
	if (_colors.size() < COLORNAME_SIZE)
	{
		return false;
	}

	for (size_t	i = 0; i < COLORNAME_SIZE; i++)
	{
		if (!_colors[i])
		{
			return false;
		}
	}

	return true;
}

void Palette::makeCompleteUsing(const Palette& other)
{
	if (_colors.size() < other._colors.size())
	{
		_colors.resize(other._colors.size());
	}

	for (size_t	i = 0; i < other._colors.size(); i++)
	{
		if (!_colors[i])
		{
			_colors[i] = other._colors[i];
		}
	}
}

void Palette::save(const std::string& palettename)
{
	std::string fname = _palettesfolder + palettename + ".json";
	LOGI << "Saving " << fname;

	System::touchFile(fname);
	std::ofstream file(fname);
	if (!file.is_open())
	{
		LOGE << "Could not open " << fname;
		DEBUG_ASSERT(false);
		return;
	}

	Json::StyledWriter writer;
	file << writer.write(toJson());

	LOGI << "Saved " << fname;
}

Palette Palette::load(const std::string& palettename)
{
	std::string fname = _palettesfolder + palettename + ".json";
	if (!System::isFile(fname))
	{
		fname = _sourcepalettesfolder + palettename + ".json";
	}

	std::ifstream file(fname);
	Json::Reader reader;
	Json::Value json;
	if (!reader.parse(file, json))
	{
		LOGE << "Failed to open " << fname;
		DEBUG_ASSERT(false);
		return Palette();
	}
	return parse(json);
}

void Palette::saveInstalledAs(const std::string& palettename)
{
	if (_installed_master_palette._colors.size() < COLORNAME_SIZE)
	{
		LOGE << "No master palette installed";
		DEBUG_ASSERT(false);
		return;
	}

	_installed_master_palette.save(palettename);
}

void Palette::installDefault()
{
	Palette palette = load("default");
	if (!palette.isComplete())
	{
		LOGE << "Incomplete default palette";
		DEBUG_ASSERT(false);
		// Continue.
	}
	install(palette);
}

void Palette::installNamed(const std::string& palettename)
{
	Palette palette = load(palettename);
	if (!palette.isComplete())
	{
		palette.makeCompleteUsing(_installed_master_palette);
	}
	install(palette);
}

Color Palette::get(const ColorName& name)
{
	if (_installed_master_palette._colors.size() < COLORNAME_SIZE)
	{
		LOGE << "No master palette installed";
		DEBUG_ASSERT(false);
		return Color::broken();
	}

	size_t index = (size_t) name;
	if (index >= COLORNAME_SIZE)
	{
		LOGE << "Invalid colorname (" << index << ")";
		DEBUG_ASSERT(false);
		return Color::broken();
	}

	return _installed_master_palette._colors[index];
}

void Palette::set(const ColorName& name, Color newcolor)
{
	if (_installed_master_palette._colors.size() < COLORNAME_SIZE)
	{
		LOGE << "No master palette installed";
		DEBUG_ASSERT(false);
		return;
	}

	size_t index = (size_t) name;
	if (index >= COLORNAME_SIZE)
	{
		LOGE << "Invalid colorname (" << index << ")";
		DEBUG_ASSERT(false);
		return;
	}

	_installed_master_palette._colors[index] = newcolor;
}

static std::vector<std::string> _indexed_master_palette_names;

void Palette::loadIndex()
{
	_indexed_master_palette_names = {"default"};

	std::string fname = _palettesfolder + "index.list";

	std::string sourcefname = _sourcepalettesfolder + "index.list";
	if (sourcefname != fname && System::isFile(sourcefname))
	{
		loadIndexFromFile(sourcefname);
	}

	if (System::isFile(fname))
	{
		loadIndexFromFile(fname);
	}
}

void Palette::loadIndexFromFile(const std::string& fname)
{
	std::ifstream indexfile(fname);
	if (!indexfile.is_open())
	{
		LOGE << "Could not open " << fname;
		return;
	}

	std::string name;
	while (std::getline(indexfile, name))
	{
		if (name.empty()) continue;
		if (name == "default") continue;
		if (std::find(_indexed_master_palette_names.begin(),
				_indexed_master_palette_names.end(),
				name)
			!= _indexed_master_palette_names.end())
		{
			continue;
		}

		_indexed_master_palette_names.push_back(name);
	}
}

void Palette::saveIndex()
{
	std::string fname = _palettesfolder + "index.list";
	std::ofstream indexfile(fname, std::ios::trunc);
	if (!indexfile.is_open())
	{
		LOGE << "Could not save " << fname;
		DEBUG_ASSERT(false);
		return;
	}

	for (const std::string& name : _indexed_master_palette_names)
	{
		if (name == "default") continue;

		indexfile << name << std::endl;
	}
}

void Palette::addToIndex(std::string palettename)
{
	DEBUG_ASSERT(palettename != "default");
	DEBUG_ASSERT(palettename.find("test") == std::string::npos);
	_indexed_master_palette_names.emplace_back(std::move(palettename));
}

const std::vector<std::string>& Palette::indexedNames()
{
	return _indexed_master_palette_names;
}
