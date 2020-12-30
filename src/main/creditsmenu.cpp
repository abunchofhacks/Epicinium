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
#include "creditsmenu.hpp"
#include "source.hpp"

#include "frame.hpp"
#include "layout.hpp"
#include "horizontallayout.hpp"
#include "verticallayout.hpp"
#include "scrollablelayout.hpp"
#include "textfield.hpp"
#include "filler.hpp"
#include "paint.hpp"
#include "colorname.hpp"
#include "input.hpp"
#include "settings.hpp"


constexpr ColorName NAMECOLOR = ColorName::TEXT200;
constexpr ColorName ROLECOLOR = ColorName::TEXT400;
constexpr ColorName ATTRCOLOR = ColorName::TEXT400;

void CreditsMenu::build()
{
	Menu::build();

	const int FONTSIZEMEDIUM = _settings.getFontSize();
	const int FONTSIZE_MENUBUTTON = _settings.getFontSizeMenuButton();
	const int FONTSIZE_MENUHEADER = _settings.getFontSizeMenuHeader();

	_layout.add("filler1", new Filler());
	_layout.add("it", new VerticalLayout());
	_layout.add("filler2", new Filler());

	_layout["it"].add("filler1", new Filler());
	_layout["it"].add("title", new TextField(
		// TRANSLATORS: This is the title of the credits menu.
		_("Credits"),
		FONTSIZE_MENUHEADER, ColorName::TEXT600));
	_layout["it"]["title"].align(HorizontalAlignment::CENTER);
	_layout["it"]["title"].setMarginBottom(30 * InterfaceElement::scale());

	_layout["it"].add("credits", new HorizontalLayout());
	_layout["it"]["credits"].add("names", new VerticalLayout());
	_layout["it"]["credits"].add("filler", new Filler());
	_layout["it"]["credits"]["filler"].fixWidth(30 * InterfaceElement::scale());
	_layout["it"]["credits"]["filler"].fixHeight(0);
	_layout["it"]["credits"].add("roles", new VerticalLayout());

	{
		_layout["it"]["credits"]["names"].add("aboh",
			new TextField("", FONTSIZEMEDIUM, NAMECOLOR));
		_layout["it"]["credits"]["roles"].add("aboh",
			new TextField(
				// TRANSLATORS: Please translate "a bunch of hacks"
				// as "A Bunch of Hacks" in non-English languages that use the
				// Latin script, and as a transliteration (NOT a translation)
				// in other languages.  The lowercase usage in English
				// reinforces a pun that only works in English.
				_("a game by a bunch of hacks"),
				FONTSIZEMEDIUM, NAMECOLOR));
	}

	{
		_layout["it"]["credits"]["names"].add("sander",
			new TextField("Sander in 't Veld",
				FONTSIZEMEDIUM, NAMECOLOR));
		_layout["it"]["credits"]["roles"].add("sander",
			new TextField(
				_("game design, programming, graphic design"),
				FONTSIZEMEDIUM, ROLECOLOR));
	}
	{
		_layout["it"]["credits"]["names"].add("daan",
			new TextField("Daan Mulder",
				FONTSIZEMEDIUM, NAMECOLOR));
		_layout["it"]["credits"]["roles"].add("daan",
			new TextField(
				_("game design, programming, engineering, sound design"),
				FONTSIZEMEDIUM, ROLECOLOR));
	}
	{
		_layout["it"]["credits"]["names"].add("masha",
			new TextField("Masha Loutanina",
				FONTSIZEMEDIUM, NAMECOLOR));
		_layout["it"]["credits"]["roles"].add("masha",
			new TextField(
				_("graphic design"),
				FONTSIZEMEDIUM, ROLECOLOR));
	}
	{
		_layout["it"]["credits"]["names"].add("masha",
			new TextField("Can Ur",
				FONTSIZEMEDIUM, NAMECOLOR));
		_layout["it"]["credits"]["roles"].add("masha",
			new TextField(
				_("programming, sound design"),
				FONTSIZEMEDIUM, ROLECOLOR));
	}

	{
		_layout["it"]["credits"]["names"].add("separator", new Filler());
		_layout["it"]["credits"]["names"]["separator"].fixHeight(15 * InterfaceElement::scale());
		_layout["it"]["credits"]["roles"].add("separator", new Filler());
		_layout["it"]["credits"]["roles"]["separator"].fixHeight(15 * InterfaceElement::scale());
	}

	{
		_layout["it"]["credits"]["names"].add("thanks",
			new TextField("", FONTSIZEMEDIUM, NAMECOLOR));
		_layout["it"]["credits"]["roles"].add("thanks",
			new TextField(
				_("special thanks"),
				FONTSIZEMEDIUM, NAMECOLOR));
	}

	{
		_layout["it"]["credits"]["names"].add("tom",
			new TextField("Tom van den Bosch",
				FONTSIZEMEDIUM, NAMECOLOR));
		_layout["it"]["credits"]["roles"].add("tom",
			new TextField(
				_("game balance, additional content"),
				FONTSIZEMEDIUM, ROLECOLOR));
	}
	{
		_layout["it"]["credits"]["names"].add("noppy",
			new TextField("Thomas Noppers",
				FONTSIZEMEDIUM, NAMECOLOR));
		_layout["it"]["credits"]["roles"].add("noppy",
			new TextField(
				_("box art"),
				FONTSIZEMEDIUM, ROLECOLOR));
	}
	{
		_layout["it"]["credits"]["names"].add("loulou",
			new TextField("Loulou Swarte",
				FONTSIZEMEDIUM, NAMECOLOR));
		_layout["it"]["credits"]["roles"].add("loulou",
			new TextField(
				_("story concepts, copywriting"),
				FONTSIZEMEDIUM, ROLECOLOR));
	}
	{
		_layout["it"]["credits"]["names"].add("thomas",
			new TextField("Thomas Jager",
				FONTSIZEMEDIUM, NAMECOLOR));
		_layout["it"]["credits"]["roles"].add("thomas",
			new TextField(
				_("copywriting"),
				FONTSIZEMEDIUM, ROLECOLOR));
	}
	{
		_layout["it"]["credits"]["names"].add("jan",
			new TextField("Jan Westerdiep",
				FONTSIZEMEDIUM, NAMECOLOR));
		_layout["it"]["credits"]["roles"].add("jan",
			new TextField(
				_("initial os x support"),
				FONTSIZEMEDIUM, ROLECOLOR));
	}

	_layout["it"]["credits"].fixWidth();
	_layout["it"]["credits"].fixHeight();
	_layout["it"]["credits"].setMarginBottom(15 * InterfaceElement::scale());

	_layout["it"].add("attributions", new VerticalLayout());

	_layout["it"]["attributions"].add("translations",
			new TextField(
				_("community translation project"),
				FONTSIZEMEDIUM, ROLECOLOR));
	_layout["it"]["attributions"]["translations"].align(
		HorizontalAlignment::LEFT);
	_layout["it"]["attributions"].add("translators",
			new HorizontalLayout());
	auto& translators = _layout["it"]["attributions"]["translators"];
	for (const char* name : {
			"JoeKerr555",
			"AlexisBarroso",
			"1diyabl",
		})
	{
		translators.add(stringref(name),
				new TextField(name, FONTSIZEMEDIUM, NAMECOLOR));
		translators[stringref(name)].setMarginHorizontal(
			10 * InterfaceElement::scale());
	}
	translators[translators.name(translators.size() - 1)].fixWidth();
	_layout["it"]["attributions"].add("others",
			new TextField(
				_("and many others"),
				FONTSIZEMEDIUM, ROLECOLOR));
	_layout["it"]["attributions"]["others"].align(HorizontalAlignment::RIGHT);

	_layout["it"]["attributions"].fixWidth();
	_layout["it"]["attributions"].fixHeight();
	_layout["it"]["attributions"].setMarginBottom(30 * InterfaceElement::scale());

	_layout["it"].add("return", makeButton(
		_("return to main menu"),
		FONTSIZE_MENUBUTTON));
	_layout["it"]["return"].setHotkeyScancode(SDL_SCANCODE_ESCAPE);
	_layout["it"]["return"].settleWidth();
	_layout["it"]["return"].fixWidth();
	_layout["it"]["return"].setMarginTop(30 * InterfaceElement::scale());
	_layout["it"].add("filler2", new Filler());

	_layout.setWidth(InterfaceElement::windowW());
	_layout.setHeight(InterfaceElement::windowH());
	_layout["it"].align(HorizontalAlignment::CENTER);
	_layout.align(VerticalAlignment::MIDDLE);
	_layout.place(Pixel(0, 0, Layer::INTERFACE));
}

void CreditsMenu::refresh()
{
	// Because some items might occlude others, we must know which layer the mouse operates in.
	// We therefore need to check all items to see if they are hovered.
	_layout.hovered();

	_layout.refresh();

	if (_layout["it"]["return"].clicked())
	{
		quit();
		return;
	}
}
