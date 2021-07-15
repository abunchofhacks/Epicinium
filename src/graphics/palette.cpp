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
static std::string _last_requested_master_palette_name;

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

std::string Palette::authoredFilename(const std::string& name)
{
	if (name.find_first_of('/') != std::string::npos)
	{
		LOGE << "Cannot determine authored filename for " << name;
		DEBUG_ASSERT(false);
		return "";
	}
	return _palettesfolder + name + ".json";
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
	palette._savedColors = palette._colors;
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

bool Palette::hasUnsavedChanges() const
{
	return _savedColors != _colors;
}

bool Palette::unsaved()
{
	return _installed_master_palette.hasUnsavedChanges();
}

void Palette::save(const std::string& palettename)
{
	std::string fname = authoredFilename(palettename);
	LOGI << "Saving " << fname;

	System::touchFile(fname);
	std::ofstream file = System::ofstream(fname);
	if (!file.is_open())
	{
		LOGE << "Could not open " << fname;
		DEBUG_ASSERT(false);
		return;
	}

	Json::StyledWriter writer;
	file << writer.write(toJson());

	_savedColors = _colors;
	LOGI << "Saved " << fname;
}

Palette Palette::load(const std::string& palettename)
{
	std::string fname;
	if (palettename.find_first_of('/') == std::string::npos)
	{
		fname = authoredFilename(palettename);
	}
	if (fname.empty() || !System::isFile(fname))
	{
		fname = sourceFilename(palettename);
	}

	std::ifstream file = System::ifstream(fname);
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

bool Palette::exists(const std::string& palettename)
{
	std::string fname;
	if (palettename.find_first_of('/') == std::string::npos)
	{
		fname = authoredFilename(palettename);
	}
	if (fname.empty() || !System::isFile(fname))
	{
		return System::isFile(sourceFilename(palettename));
	}
	return true;
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
	_last_requested_master_palette_name = "default";
	Palette palette = load("default");
	if (!palette.isComplete())
	{
		LOGE << "Incomplete default palette";
		DEBUG_ASSERT(false);
		// Continue.
	}
	_installed_master_palette = std::move(palette);
}

void Palette::installNamed(const std::string& palettename)
{
	_last_requested_master_palette_name = palettename;
	if (!Palette::exists(palettename))
	{
		LOGD << "Cannot yet install " << palettename;
		return;
	}
	Palette palette = load(palettename);
	if (!palette.isComplete())
	{
		palette.makeCompleteUsing(_installed_master_palette);
	}
	_installed_master_palette = std::move(palette);
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

	if (_sourcepalettesfolder != _palettesfolder
		&& System::isDirectory(_sourcepalettesfolder))
	{
		loadIndexFromDirectory(_sourcepalettesfolder);
	}

	if (System::isDirectory(_palettesfolder))
	{
		loadIndexFromDirectory(_palettesfolder);
	}
}

void Palette::loadIndexFromDirectory(const std::string& dirname)
{
	for (std::string name : System::listDirectory(dirname, ".json"))
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

	// Sort, but keep "default" at the top.
	std::sort(_indexed_master_palette_names.begin() + 1,
		_indexed_master_palette_names.end());
}

const std::vector<std::string>& Palette::indexedNames()
{
	return _indexed_master_palette_names;
}

static std::vector<Palette::ExternalItem> _cached_external_items;

std::string Palette::sourceFilename(const std::string& name)
{
	for (const auto& item : _cached_external_items)
	{
		if (item.uniqueTag == name)
		{
			return item.sourceFilename;
		}
	}

	return _sourcepalettesfolder + name + ".json";
}

void Palette::listExternalItem(ExternalItem&& newItem)
{
	LOGD << "Listing '" << newItem.uniqueTag << "'"
		" (a.k.a. " << newItem.quotedName << ")"
		": " << newItem.sourceFilename;

	for (auto& item : _cached_external_items)
	{
		if (item.uniqueTag == newItem.uniqueTag)
		{
			item = newItem;
			return;
		}
	}

	_cached_external_items.emplace_back(newItem);

	if (_cached_external_items.back().uniqueTag
			== _last_requested_master_palette_name
		&& !_installed_master_palette.hasUnsavedChanges())
	{
		LOGD << "Installing awaited " << _last_requested_master_palette_name;
		installNamed(_last_requested_master_palette_name);
	}
}

void Palette::unlistExternalItem(const std::string& uniqueTag)
{
	LOGD << "Unlisting '" << uniqueTag << "'";
	_cached_external_items.erase(
		std::remove_if(
			_cached_external_items.begin(),
			_cached_external_items.end(),
			[&](const ExternalItem& item) {
				return item.uniqueTag == uniqueTag;
			}),
		_cached_external_items.end());
}

const std::vector<Palette::ExternalItem>& Palette::externalItems()
{
	return _cached_external_items;
}
