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
#include "editormenu.hpp"
#include "source.hpp"

#include "settings.hpp"
#include "mapeditor.hpp"
#include "camera.hpp"
#include "client.hpp"


void EditorMenu::build()
{
	_mapeditor.reset(new MapEditor(*this, _gameowner, ""));
}

void EditorMenu::beforeFirstRefreshOfEachSecond()
{
	if (_layout.alive())
	{
		_mapeditor->beforeFirstUpdateOfEachSecond();
	}
}

void EditorMenu::refresh()
{
	if (_layout.alive())
	{
		_mapeditor->update();
	}
}

void EditorMenu::onOpen()
{
	_client.registerHandler(this);

	if (_layout.born())
	{
		Camera::get()->changeScale(_settings.getEditorScale());
	}
}

void EditorMenu::onKill()
{
	_client.deregisterHandler(this);

	Camera::get()->changeScale(_settings.scale.value());
}

void EditorMenu::onShow()
{
	if (_layout.born())
	{
		Camera::get()->changeScale(_settings.getEditorScale());
	}
}

void EditorMenu::onHide()
{
	Camera::get()->changeScale(_settings.scale.value());
}

void EditorMenu::debugHandler() const
{
	LOGD << ((void*) this);
}

void EditorMenu::onConfirmQuit()
{
	quit();
}

bool EditorMenu::hasWorkshop()
{
	return _client.isSteamEnabled();
}

void EditorMenu::openWorkshop(const std::string& mapname,
	const std::string& rulesetname)
{
	if (mapname == "colorsample")
	{
		_client.openWorkshopForPalette(_owner.activePaletteName());
	}
	else if (mapname.empty())
	{
		_client.openWorkshopForRuleset(rulesetname);
	}
	else
	{
		_client.openWorkshopForMap(mapname);
	}
}

void EditorMenu::closeWorkshop()
{
	if (hasWorkshop())
	{
		_client.closeAllWorkshops();
	}
}

void EditorMenu::openPaletteEditor()
{
	_owner.openPaletteEditor("");
}

bool EditorMenu::isTakingScreenshot()
{
	return _owner.isTakingScreenshot();
}

void EditorMenu::takeScreenshotOfMap()
{
	_client.takeScreenshot(_mapeditor->prepareScreenshotOfMap());
}
