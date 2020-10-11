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
#include "source.hpp"

#include "settings.hpp"
#include "language.hpp"
#include "loginstaller.hpp"
#include "version.hpp"


void record(const char* term, const char* translation, const char* description);

int main(int argc, char* argv[])
{
	Settings settings("settings-termlocalizer.json", argc, argv);

	if (!settings.logname.defined())
	{
		settings.logname.override("termlocalizer");
	}

	if (!settings.logrollback.defined()) settings.logrollback.override(5);
	LogInstaller(settings).install();

	LOGI << "Start v" << Version::current();

	Language::use(settings);

	record("Unit", _("Unit"), "A unit is a soldier or group of soldiers"
		" occupying a single space.");
	record("Tile", _("Tile"), "A tile is a square space in the game"
		" along with the buildings or trees that are on it.");

	record("Rifleman", _("Rifleman"), "Unit type."
		" All-round infantry unit.");
	record("Gunner", _("Gunner"), "Unit type."
		" Soldier with a heavy machinegun."
		" Possible alternative name: Commando.");
	record("Sapper", _("Sapper"), "Unit type."
		" Soldier with a long-distance mortar."
		" Possible alternative name: Artillerist.");
	record("Tank", _("Tank"), "Unit type."
		" Armored tank with a heavy cannon.");
	record("Settler", _("Settler"), "Unit type."
		" Can build settlements.");
	record("Militia", _("Militia"), "Unit type."
		" Fast but fragile infantry unit.");

	record("Grass", _("Grass"), "Tile type.");
	record("Dirt", _("Dirt"), "Tile type."
		" Left over after grass was destroyed.");
	record("Desert", _("Desert"), "Tile type.");
	record("Rubble", _("Rubble"), "Tile type.");
	record("Ridge", _("Ridge"), "Tile type.");
	record("Mountain", _("Mountain"), "Tile type.");
	record("Water", _("Water"), "Tile type.");
	record("Forest", _("Forest"), "Tile type.");
	record("City", _("City"), "Tile type."
		" The most important tile type in the game."
		" Has one or more tall buildings on it.");
	record("Town", _("Town"), "Tile type."
		" Has smaller buildings than the City.");
	record("Outpost", _("Outpost"), "Tile type."
		" Usually built near enemy borders.");
	record("Industry", _("Industry"), "Tile type."
		" Produces tanks and generates income.");
	record("Barracks", _("Barracks"), "Tile type."
		" Produces military units.");
	record("Airfield", _("Airfield"), "Tile type."
		" Produces air units.");
	record("Farm", _("Farm"), "Tile type."
		" Surrounded by Soil or Crops.");
	record("Soil", _("Soil"), "Tile type."
		" Fertile land where Crops can grow.");
	record("Crops", _("Crops"), "Tile type."
		" Similar to a Soil tile but with crops on it.");
	record("Trenches", _("Trenches"), "Tile type.");

	record("Snow", _("Snow"), "Weather effect.");
	record("Frostbite", _("Frostbite"), "Weather effect."
		" The affected area is extremely cold.");
	record("Firestorm", _("Firestorm"), "Weather effect."
		" The affected area is on fire.");
	record("Bonedrought", _("Bonedrought"), "Weather effect."
		" The affected area is extremely dry.");
	record("Gas", _("Gas"), "A lethal gas.");

	record("Focus", _("Focus"), "Ability name."
		" Causes multiple units to focus on one enemy.");
	record("Lockdown", _("Lockdown"), "Ability name."
		" Causes a unit to guard a space.");
	record("Shell", _("Shell"), "Ability name."
		" Causes a unit to fire explosive rounds at an adjacent space.");
	record("Bombard", _("Bombard"), "Ability name."
		" Causes a unit to fire mortar rounds at long range.");
	record("Capture", _("Capture"), "Ability name."
		" Causes a tile to change owners.");
	record("Upgrade", _("Upgrade"), "Ability name."
		" Causes a tile to change type or to increase in size.");

	record("Planning phase", _("Planning phase"), "Phase of the game"
		" where players give orders to their units and tiles.");
	record("Action phase", _("Action phase"), "Phase of the game"
		" where units and tiles execute their orders.");
	record("Decay phase", _("Decay phase"), "Phase of the game"
		" where Gas and certain weather effects deal damage.");
	record("Night phase", _("Night phase"), "Phase of the game"
		" where certain tiles produce income.");
	record("Weather phase", _("Weather phase"), "Phase of the game"
		" where weather effects change.");

	return 0;
}

void record(const char* term, const char* translation, const char* description)
{
	if (strcmp(term, translation) == 0)
	{
		std::cout << term << "," << description << std::endl;
	}
	else
	{
		std::cout << term << "," << description
			<< " Current translation: " << translation
			<< std::endl;
	}
}
