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
 * Can Ur (canur11@gmail.com)
 */
#include "mainmenu.hpp"
#include "source.hpp"

#include "libs/SDL2/SDL_mouse.h"

#include "layout.hpp"
#include "horizontallayout.hpp"
#include "verticallayout.hpp"
#include "scrollablelayout.hpp"
#include "tooltiplayout.hpp"
#include "slideshowlayout.hpp"
#include "formshowlayout.hpp"
#include "frame.hpp"
#include "pictureview.hpp"
#include "client.hpp"
#include "settings.hpp"
#include "mixer.hpp"
#include "input.hpp"
#include "gameowner.hpp"
#include "message.hpp"
#include "multiplayermenu.hpp"
#include "settingsmenu.hpp"
#include "creditsmenu.hpp"
#include "filler.hpp"
#include "textfield.hpp"
#include "multitextfield.hpp"
#include "textinput.hpp"
#include "multitextinput.hpp"
#include "passwordinput.hpp"
#include "hiddentag.hpp"
#include "image.hpp"
#include "paint.hpp"
#include "colorname.hpp"
#include "system.hpp"
#include "responsestatus.hpp"
#include "challenge.hpp"
#include "validation.hpp"
#include "locator.hpp"
#include "openurl.hpp"


enum
{
	MULTIPLAYER,
	SETTINGS,
	CREDITS,
};

MainMenu::MainMenu(Owner& owner, GameOwner& gameowner) :
	Menu(owner, gameowner),
	_connectionStatus(ConnectionStatus::NONE),
	_loginStatus(LoginStatus::DISABLED),
	_steamStatus(SteamStatus::NONE),
	_discordStatus(DiscordStatus::NONE),
	_linkWasHovered(false)
{
	_submenus.emplace_back((Menu*) new MultiplayerMenu(_owner, _gameowner));
	_submenus.emplace_back((Menu*) new SettingsMenu(_owner, _gameowner));
	_submenus.emplace_back((Menu*) new CreditsMenu(_owner, _gameowner));
}

MainMenu::~MainMenu()
{
	if (_layout.born()) kill();
}

std::unique_ptr<InterfaceElement> MainMenu::makeButtonLarge(
	const std::string& text, int fontsize)
{
	std::unique_ptr<InterfaceElement> element = makeButton(text, fontsize);

	element->setMargin(4 * InterfaceElement::scale());
	int w = 7;
	element->fixWidth(InterfaceElement::fontH(fontsize) * w);

	return element;
}

std::unique_ptr<InterfaceElement> MainMenu::makeButtonWide(
	const std::string& text, int fontsize)
{
	std::unique_ptr<InterfaceElement> element = makeButton(text, fontsize);

	element->setMargin(4 * InterfaceElement::scale());
	int w = 24;
	element->fixWidth(InterfaceElement::fontH(fontsize) * w);

	return element;
}

std::unique_ptr<InterfaceElement> MainMenu::makeButtonSmall(
	const std::string& text, int fontsize)
{
	std::unique_ptr<InterfaceElement> element = makeButton(text, fontsize);

	element->setMargin(4 * InterfaceElement::scale());
	int w = 8;
	element->fixWidth(InterfaceElement::fontH(fontsize) * w);

	return element;
}

std::unique_ptr<InterfaceElement> MainMenu::makeButtonTiny(
	const std::string& text, int fontsize)
{
	std::unique_ptr<InterfaceElement> element = makeButton(text, fontsize);

	element->setMargin(4 * InterfaceElement::scale());
	int w = 6;
	element->fixWidth(InterfaceElement::fontH(fontsize) * w);

	return element;
}

std::unique_ptr<InterfaceElement> MainMenu::makeUsernameInput()
{
	std::unique_ptr<InterfaceElement> it(new Frame("ui/frame_window_9"));
	auto& element = *it;
	element.put(new HorizontalLayout());
	element.add("input", new TextInput(
		_settings.getFontSizeTextInput()));
	element.setPadding(4 * InterfaceElement::scale());
	element.add("error", Menu::makeErrorbubble(
		_(""
		"Usernames must be between 3 and 36 characters long"
		" and can only contain letters, numbers,"
		" underscores, dashes, periods and tildes."
		""),
		_settings.getFontSize()));
	element["error"].settle();
	element.align(VerticalAlignment::MIDDLE);
	return it;
}

std::unique_ptr<InterfaceElement> MainMenu::makeEmailInput()
{
	std::unique_ptr<InterfaceElement> it(new Frame("ui/frame_window_9"));
	auto& element = *it;
	element.put(new HorizontalLayout());
	element.add("input", new TextInput(
		_settings.getFontSizeTextInput()));
	element.setPadding(4 * InterfaceElement::scale());
	element.add("error", Menu::makeErrorbubble(
		_(""
		"This is not a valid email address."
		""),
		_settings.getFontSize()));
	element["error"].settle();
	element.align(VerticalAlignment::MIDDLE);
	return it;
}

std::unique_ptr<InterfaceElement> MainMenu::makePasswordInput()
{
	std::unique_ptr<InterfaceElement> it(new Frame("ui/frame_window_9"));
	auto& element = *it;
	element.put(new HorizontalLayout());
	element.add("input", new PasswordInput(
		_settings.getFontSizeTextInput()));
	element.setPadding(4 * InterfaceElement::scale());
	element.add("error", Menu::makeErrorbubble(
		_(""
		"Passwords must consist of at least 8 characters"
		" and must contain a letter and a non-letter."
		""),
		_settings.getFontSize()));
	element["error"].settle();
	element.align(VerticalAlignment::MIDDLE);
	return it;
}

std::unique_ptr<InterfaceElement> MainMenu::makePassword2Input()
{
	std::unique_ptr<InterfaceElement> it(new Frame("ui/frame_window_9"));
	auto& element = *it;
	element.put(new HorizontalLayout());
	element.add("input", new PasswordInput(
		_settings.getFontSizeTextInput()));
	element.setPadding(4 * InterfaceElement::scale());
	element.add("error", Menu::makeErrorbubble(
		_(""
		"The passwords do not match."
		""),
		_settings.getFontSize()));
	element["error"].settle();
	element.align(VerticalAlignment::MIDDLE);
	return it;
}

std::unique_ptr<InterfaceElement> MainMenu::makePasswordResetTokenInput()
{
	std::unique_ptr<InterfaceElement> it(new Frame("ui/frame_window_9"));
	auto& element = *it;
	element.put(new HorizontalLayout());
	element.add("input", new TextInput(
		_settings.getFontSizeTextInput()));
	element.setPadding(4 * InterfaceElement::scale());
	element.add("error", Menu::makeErrorbubble(
		_(""
		"This is not a valid password reset token."
		""),
		_settings.getFontSize()));
	element["error"].settle();
	element.align(VerticalAlignment::MIDDLE);
	return it;
}

std::unique_ptr<InterfaceElement> MainMenu::makeKeyTokenInput()
{
	std::unique_ptr<InterfaceElement> it(new Frame("ui/frame_window_9"));
	auto& element = *it;
	element.put(new HorizontalLayout());
	element.add("input", new TextInput(
		_settings.getFontSizeTextInput()));
	element.setPadding(4 * InterfaceElement::scale());
	element.add("error", Menu::makeErrorbubble(
		// TRANSLATORS: "Key" here refers to a sequence of letters and numbers
		// that the user enters in a textfield to unlock access to the game.
		_(""
		"This is not a valid key."
		""),
		_settings.getFontSize()));
	element["error"].settle();
	element.align(VerticalAlignment::MIDDLE);
	return it;
}

void MainMenu::build()
{
	Menu::build();

	const int FONTSIZESMALL = _settings.getFontSize();
	const int FONTSIZE_MENUBUTTON = _settings.getFontSizeMenuButton();

	_background.add("fallback", new Frame("ui/canvas_9"));
	_background["fallback"].setColor(0, ColorName::FRAME800);
	_background["fallback"].put(new Filler());
	_background.add("art", new PictureView(_settings.artpanmode.value()));
	if (InterfaceElement::windowW() / InterfaceElement::scale() < 800
		|| InterfaceElement::windowH() / InterfaceElement::scale() < 400)
	{
		_background["art"].setPicture("art/menu960");
	}
	else
	{
		_background["art"].setPicture("art/menu1440");
	}
	_background.add("letterbox", new VerticalLayout());
	_background["letterbox"].add("top", new Frame("ui/canvas_9"));
	_background["letterbox"]["top"].setColor(0, ColorName::FRAME800);
	_background["letterbox"]["top"].put(new Filler());
	_background["letterbox"].add("viewport", new Frame("ui/canvas_9"));
	_background["letterbox"]["viewport"].setColor(0, Color::transparent());
	_background["letterbox"]["viewport"].put(new Filler());
	_background["letterbox"].add("bottom", new Frame("ui/canvas_9"));
	_background["letterbox"]["bottom"].setColor(0, ColorName::FRAME800);
	_background["letterbox"]["bottom"].put(new Filler());
	_background.add("error", new SlideshowLayout());

	{
		InterfaceElement& error = _background["error"];
		error.add("resetting", new Frame("ui/canvas_9"));
		error["resetting"].setColor(0, ColorName::FRAME800);
		error["resetting"].put(new VerticalLayout());
		error["resetting"].add("filler1", new VerticalFiller());
		error["resetting"].add("1", new TextField(
			_("Oops! Connection to server was lost."),
			FONTSIZESMALL, ColorName::TEXT200));
		error["resetting"].add("2", new TextField(
			_("Attempting to reconnect..."),
			FONTSIZESMALL, ColorName::TEXT200));
		error["resetting"].add("filler2", new VerticalFiller());
		error["resetting"]["1"].align(HorizontalAlignment::CENTER);
		error["resetting"]["2"].align(HorizontalAlignment::CENTER);
		error.add("reconnected", new Frame("ui/canvas_9"));
		error["reconnected"].setColor(0, ColorName::FRAME800);
		error["reconnected"].put(new VerticalLayout());
		error["reconnected"].add("filler1", new VerticalFiller());
		error["reconnected"].add("1", new TextField(
			_("Oops! Connection to server was lost."),
			FONTSIZESMALL, ColorName::TEXT200));
		error["reconnected"].add("2", new TextField(
			_("Press PLAY to rejoin."),
			FONTSIZESMALL, ColorName::TEXT200));
		error["reconnected"].add("filler2", new VerticalFiller());
		error["reconnected"]["1"].align(HorizontalAlignment::CENTER);
		error["reconnected"]["2"].align(HorizontalAlignment::CENTER);
		error.hide();
	}

	_layout.content().setColor(0, Color::transparent());

	_layout.add("left", new VerticalLayout());
	_layout.add("mid", new FormshowLayout());
	_layout.add("right", new VerticalLayout());

	{
		_layout["left"].add("filler", new Filler());
		_layout["left"].add("logout", makeButtonSmall(
			_("LOG OUT"),
			FONTSIZE_MENUBUTTON));
		_layout["left"].add("login", makeButtonSmall(
			_("LOG IN"),
			FONTSIZE_MENUBUTTON));
		_layout["left"].add("register", makeButtonSmall(
			_("REGISTER"),
			FONTSIZE_MENUBUTTON));
	}

	_layout["mid"].add("buttons", new VerticalLayout());

	{
		InterfaceElement& buttons = _layout["mid"]["buttons"];
		buttons.add("filler1", new Filler());
		buttons["filler1"].fixHeight(
			24 * InterfaceElement::scale());
		buttons.add("logo", new StackedLayout());
		InterfaceElement& logo = _layout["mid"]["buttons"]["logo"];
		logo.add("stamp", new TooltipLayout());
		logo["stamp"].add("picture", new PictureView());
		logo["stamp"]["picture"].align(VerticalAlignment::BOTTOM);
		logo["stamp"].add("tooltip", Frame::makeTooltip());
		logo["stamp"]["tooltip"].put(
			new MultiTextField("", FONTSIZESMALL));
		logo["stamp"]["tooltip"].setMargin(
			5 * InterfaceElement::scale());
		logo["stamp"]["tooltip"].hide();
		logo["stamp"].add("url", new HiddenTag(""));
		logo["stamp"].align(HorizontalAlignment::RIGHT);
		logo.add("title", new Image("logos/title"));
		logo["title"].settle();
		logo.align(HorizontalAlignment::RIGHT);
		logo.align(VerticalAlignment::TOP);
		logo["stamp"].fixWidth(
			logo["title"].width()
			+ 5 * InterfaceElement::scale());
		logo["stamp"].fixHeight(
			logo["title"].height()
			+ 80 * InterfaceElement::scale());
		buttons.add("filler2", new Filler());
		buttons.add("play", new SlideshowLayout());
		buttons["play"].add("mp", new TooltipLayout());
		buttons["play"]["mp"].add("button", makeButtonLarge(
			_("PLAY"),
			_settings.getFontSizePlayButton()));
		buttons["play"]["mp"]["button"].setHotkeyScancode(SDL_SCANCODE_ENTER);
		buttons["play"]["mp"].add("tooltip", Frame::makeTooltip());
		buttons["play"]["mp"]["tooltip"].put(new TextField("", FONTSIZESMALL));
		buttons["play"]["mp"]["tooltip"].setMargin(5 * InterfaceElement::scale());
		buttons["play"]["mp"]["button"].setMarginTop(
			/*No margin because the logo drips onto the button.*/0);
		buttons["play"]["mp"].setMarginTop(
			/*No margin because the logo drips onto the button.*/0);
		buttons["play"].add("downloading", Frame::makeInform());
		buttons["play"]["downloading"].put(new HorizontalLayout());
		buttons["play"]["downloading"].add("texts", new VerticalLayout());
		buttons["play"]["downloading"]["texts"].add("1", new TextField(
			_("Download in progress"),
			FONTSIZESMALL, ColorName::TEXT200));
		buttons["play"]["downloading"]["texts"].add("2", new TextField(
			_("Downloading..."),
			FONTSIZESMALL, ColorName::TEXT400));
		buttons["play"]["downloading"].add("icon", new Image("ui/downloading"));
		{
			InterfaceElement& icon = buttons["play"]["downloading"]["icon"];
			int i = 0;
			icon.setColor(i++, Paint::blend(ColorName::LOGOTEXT, ColorName::SHINEBLEND));
			icon.setColor(i++, ColorName::LOGOTEXT);
			icon.setColor(i++, Paint::blend(ColorName::LOGOTEXT, ColorName::SHADEBLEND));
			icon.setColor(i++, Paint::blend(ColorName::FRAME200, ColorName::SHINEBLEND));
			icon.setColor(i++, ColorName::FRAME200);
			icon.setColor(i++, Paint::blend(ColorName::FRAME200, ColorName::SHADEBLEND));
			icon.setColor(i++, Paint::blend(ColorName::FRAME600, ColorName::SHINEBLEND, 0.5f));
			icon.setColor(i++, ColorName::FRAME600);
			icon.setColor(i++, Paint::blend(ColorName::FRAME600, ColorName::SHADEBLEND, 0.5f));
			icon.setColor(i++, Color::transparent());
			icon.setTag("Spinning");
		}
		buttons["play"]["downloading"].align(VerticalAlignment::MIDDLE);
		buttons["play"].setTag("mp");
		buttons["play"].align(HorizontalAlignment::CENTER);
		buttons["play"].align(VerticalAlignment::MIDDLE);
		buttons.add("enterkey", makeButtonSmall(
			_("Enter Key"),
			FONTSIZE_MENUBUTTON));
		buttons.add("feedback", makeButtonSmall(
			_("Feedback?"),
			FONTSIZE_MENUBUTTON));
		buttons.align(HorizontalAlignment::CENTER);
		buttons.setMarginHorizontal(
			(InterfaceElement::windowW() < 600 * InterfaceElement::scale())
			? 5 * InterfaceElement::scale()
			: 20 * InterfaceElement::scale());
	}

	{
		_layout["mid"].add("loginform", Frame::makeForm());
		InterfaceElement& form = _layout["mid"]["loginform"];
		form.put(new VerticalLayout());
		form.add("filler1", new Filler());
		form.add("error", makeErrorprompt(
			_("An error has occurred."),
			FONTSIZESMALL, /*clickable=*/false));
		form["error"].setMarginBottom(
			15 * InterfaceElement::scale());
		form["error"].kill();
		form.add("info", makeInfoprompt(
			"", FONTSIZESMALL, /*clickable=*/false));
		form["info"].setMarginBottom(
			15 * InterfaceElement::scale());
		form["info"].kill();
		form.add("title_email", new TextField(
			_("Email address:"),
			FONTSIZESMALL));
		form["title_email"].setMarginTop(
			5 * InterfaceElement::scale());
		form["title_email"].setMarginBottom(
			1 * InterfaceElement::scale());
		form.add("email", makeEmailInput());
		form.add("title_password", new TextField(
			_("Password:"),
			FONTSIZESMALL));
		form["title_password"].setMarginTop(
			5 * InterfaceElement::scale());
		form["title_password"].setMarginBottom(
			1 * InterfaceElement::scale());
		form.add("password", makePasswordInput());
		form.add("keepmeloggedin", Frame::makeItem(
			ColorName::FRAME200));
		form["keepmeloggedin"].put(new HorizontalLayout());
		form["keepmeloggedin"].add("title", new TextField(
			_("Keep me logged in:"),
			FONTSIZE_MENUBUTTON));
		form["keepmeloggedin"].add("options", Menu::makeCheckbox(
			/*selfclickable=*/false));
		form["keepmeloggedin"].makeClickable();
		form["keepmeloggedin"].setMarginTop(15 * InterfaceElement::scale());
		form["keepmeloggedin"].align(VerticalAlignment::MIDDLE);
		form.add("buttons", new HorizontalLayout());
		form["buttons"].add("filler1", new HorizontalFiller());
		form["buttons"].add("confirm", makeButtonTiny(
			_("confirm"),
			FONTSIZE_MENUBUTTON));
		form["buttons"]["confirm"].setHotkeyScancode(SDL_SCANCODE_ENTER);
		form["buttons"].add("cancel", makeButtonTiny(
			_("cancel"),
			FONTSIZE_MENUBUTTON));
		form["buttons"]["cancel"].setHotkeyScancode(SDL_SCANCODE_ESCAPE);
		form["buttons"].add("filler2", new HorizontalFiller());
		form["buttons"].setMarginTop(
			15 * InterfaceElement::scale());
		form.add("filler2", new Filler());
		form.add("bottom", new HorizontalLayout());
		form["bottom"].add("filler1", new HorizontalFiller());
		form["bottom"].add("forgotpw", makeButton(
			_("forgot password?"),
			FONTSIZE_MENUBUTTON));
		form["bottom"]["forgotpw"].settleWidth();
		form["bottom"]["forgotpw"].fixWidth();
		form["bottom"]["forgotpw"].setMargin(4 * InterfaceElement::scale());
		form["bottom"].add("filler2", new HorizontalFiller());
		form.add("token", new HiddenTag(""));
		form.setMargin(5 * InterfaceElement::scale());
	}

	{
		_layout["mid"].add("guestform", Frame::makeForm());
		InterfaceElement& form = _layout["mid"]["guestform"];
		form.put(new VerticalLayout());
		form.add("filler1", new Filler());
		form.add("error", makeErrorprompt(
			_("An error has occurred."),
			FONTSIZESMALL, /*clickable=*/false));
		form["error"].setMarginBottom(
			15 * InterfaceElement::scale());
		form["error"].kill();
		form.add("question", new MultiTextField(
			_(""
			"You are not logged in."
			" Would you like to play as a guest using your Discord username?"
			""),
			FONTSIZESMALL));
		form["question"].setMarginBottom(
			15 * InterfaceElement::scale());
		form.add("buttons", new HorizontalLayout());
		form["buttons"].add("filler1", new HorizontalFiller());
		form["buttons"].add("confirm", makeButtonTiny(
			_("confirm"),
			FONTSIZE_MENUBUTTON));
		form["buttons"]["confirm"].setHotkeyScancode(SDL_SCANCODE_ENTER);
		form["buttons"].add("cancel", makeButtonTiny(
			_("cancel"),
			FONTSIZE_MENUBUTTON));
		form["buttons"]["cancel"].setHotkeyScancode(SDL_SCANCODE_ESCAPE);
		form["buttons"].add("filler2", new HorizontalFiller());
		form["buttons"].setMarginTop(
			15 * InterfaceElement::scale());
		form.add("filler2", new Filler());
		form.add("info", makeInfoprompt(
			_(""
			"If you are on Discord, you can join our community by"
			" visiting our Discord server (click here)."
			""),
			FONTSIZE_MENUBUTTON, /*clickable=*/true));
		form.setMargin(5 * InterfaceElement::scale());
	}

	{
		_layout["mid"].add("discordloginform", Frame::makeForm());
		InterfaceElement& form = _layout["mid"]["discordloginform"];
		form.put(new VerticalLayout());
		form.add("filler1", new Filler());
		form.add("error", makeErrorprompt(
			_("An error has occurred."),
			FONTSIZESMALL, /*clickable=*/false));
		form["error"].setMarginBottom(
			15 * InterfaceElement::scale());
		form["error"].kill();
		form.add("content", new HorizontalLayout());
		form["content"].add("filler1", new HorizontalFiller());
		form["content"].add("avatar", makeAvatarFrame());
		form["content"]["avatar"].put(new StackedLayout());
		form["content"]["avatar"].add("filler", new Filler());
		form["content"]["avatar"].add("spinning", new Image("ui/discord"));
		form["content"]["avatar"]["spinning"].setTag("Spinning");
		{
			InterfaceElement& icon = form["content"]["avatar"]["spinning"];
			int i = 0;
			icon.setColor(i++, Paint::mix(Paint::blend(ColorName::LOGOTEXT, ColorName::SHINEBLEND), ColorName::FRAME400, 0.5f));
			icon.setColor(i++, Paint::mix(ColorName::LOGOTEXT, ColorName::FRAME400, 0.5f));
			icon.setColor(i++, Paint::mix(Paint::blend(ColorName::LOGOTEXT, ColorName::SHADEBLEND), ColorName::FRAME400, 0.5f));
			icon.setColor(i++, Paint::mix(Paint::blend(ColorName::LOGOICE, ColorName::SHINEBLEND), ColorName::FRAME400, 0.5f));
			icon.setColor(i++, Paint::mix(ColorName::LOGOICE, ColorName::FRAME400, 0.5f));
			icon.setColor(i++, Paint::mix(Paint::blend(ColorName::LOGOICE, ColorName::SHADEBLEND), ColorName::FRAME400, 0.5f));
			icon.setColor(i++, Paint::mix(Paint::blend(ColorName::FRAME800, ColorName::SHINEBLEND, 0.5f), ColorName::FRAME400, 0.5f));
			icon.setColor(i++, Paint::mix(ColorName::FRAME800, ColorName::FRAME400, 0.5f));
			icon.setColor(i++, Paint::mix(Paint::blend(ColorName::FRAME800, ColorName::SHADEBLEND, 0.5f), ColorName::FRAME400, 0.5f));
			icon.setColor(i++, Color::transparent());
		}
		form["content"]["avatar"].align(HorizontalAlignment::CENTER);
		form["content"]["avatar"].align(VerticalAlignment::MIDDLE);
		form["content"]["avatar"].fixWidth(
			64 * InterfaceElement::scale());
		form["content"]["avatar"].fixHeight(
			64 * InterfaceElement::scale());
		form["content"]["avatar"].setMarginHorizontal(
			10 * InterfaceElement::scale());
		form["content"].add("text", new MultiTextField(
			_("Looking for the Discord application..."),
			FONTSIZESMALL));
		form["content"]["text"].setMarginHorizontal(
			10 * InterfaceElement::scale());
		form["content"].add("filler2", new HorizontalFiller());
		form["content"].align(VerticalAlignment::MIDDLE);
		form.add("buttons", new HorizontalLayout());
		form["buttons"].add("filler1", new HorizontalFiller());
		form["buttons"].add("confirm", makeButtonTiny(
			_("confirm"),
			FONTSIZE_MENUBUTTON));
		form["buttons"]["confirm"].setHotkeyScancode(SDL_SCANCODE_ENTER);
		form["buttons"].add("cancel", makeButtonTiny(
			_("cancel"),
			FONTSIZE_MENUBUTTON));
		form["buttons"]["cancel"].setHotkeyScancode(SDL_SCANCODE_ESCAPE);
		form["buttons"].add("filler2", new HorizontalFiller());
		form["buttons"].setMarginTop(
			15 * InterfaceElement::scale());
		form.add("filler2", new Filler());
		form.add("info", makeInfoprompt(
			_(""
			"If you are on Discord, you can join our community by"
			" visiting our Discord server (click here)."
			""),
			FONTSIZE_MENUBUTTON, /*clickable=*/true));
		form.setMargin(5 * InterfaceElement::scale());
	}

	{
		_layout["mid"].add("steammergeform", Frame::makeForm());
		InterfaceElement& form = _layout["mid"]["steammergeform"];
		form.put(new VerticalLayout());
		form.add("filler1", new Filler());
		form.add("error", makeErrorprompt(
			_("An error has occurred."),
			FONTSIZESMALL, /*clickable=*/false));
		form["error"].setMarginBottom(
			15 * InterfaceElement::scale());
		form["error"].kill();
		form.add("steam", new HorizontalLayout());
		form["steam"].add("filler1", new HorizontalFiller());
		form["steam"].add("avatar", makeAvatarFrame());
		form["steam"]["avatar"].put(new StackedLayout());
		form["steam"]["avatar"].add("filler", new Filler());
		form["steam"]["avatar"].add("spinning", new Image("ui/steam"));
		form["steam"]["avatar"]["spinning"].setTag("Spinning");
		{
			InterfaceElement& icon = form["steam"]["avatar"]["spinning"];
			int i = 0;
			icon.setColor(i++, Paint::blend(ColorName::LOGOTEXT,
				ColorName::SHINEBLEND));
			icon.setColor(i++, ColorName::LOGOTEXT);
			icon.setColor(i++, Paint::blend(ColorName::LOGOTEXT,
				ColorName::SHADEBLEND));
			icon.setColor(i++, Paint::blend(ColorName::LOGOWATER,
				ColorName::SHINEBLEND));
			icon.setColor(i++, ColorName::LOGOWATER);
			icon.setColor(i++, Paint::blend(ColorName::LOGOWATER,
				ColorName::SHADEBLEND));
			icon.setColor(i++, Paint::blend(ColorName::FRAME100,
				ColorName::SHINEBLEND, 0.5f));
			icon.setColor(i++, ColorName::FRAME100);
			icon.setColor(i++, Paint::blend(ColorName::FRAME100,
				ColorName::SHADEBLEND, 0.5f));
			icon.setColor(i++, Color::transparent());
		}
		form["steam"]["avatar"].align(HorizontalAlignment::CENTER);
		form["steam"]["avatar"].align(VerticalAlignment::MIDDLE);
		form["steam"]["avatar"].fixWidth(
			32 * InterfaceElement::scale());
		form["steam"]["avatar"].fixHeight(
			32 * InterfaceElement::scale());
		form["steam"]["avatar"].setMarginHorizontal(
			10 * InterfaceElement::scale());
		form["steam"].add("text", new MultiTextField(
			_("Connecting with Steam..."),
			FONTSIZESMALL));
		form["steam"]["text"].setMarginHorizontal(
			10 * InterfaceElement::scale());
		form["steam"].add("filler2", new HorizontalFiller());
		form["steam"].align(VerticalAlignment::MIDDLE);
		form.add("title_email", new TextField(
			_("Email address:"),
			FONTSIZESMALL));
		form["title_email"].setMarginTop(
			5 * InterfaceElement::scale());
		form["title_email"].setMarginBottom(
			1 * InterfaceElement::scale());
		form.add("email", makeEmailInput());
		form.add("title_password", new TextField(
			_("Password:"),
			FONTSIZESMALL));
		form["title_password"].setMarginTop(
			5 * InterfaceElement::scale());
		form["title_password"].setMarginBottom(
			1 * InterfaceElement::scale());
		form.add("password", makePasswordInput());
		form.add("sanitycheck", Frame::makeItem(
			ColorName::FRAME200));
		form["sanitycheck"].put(new HorizontalLayout());
		form["sanitycheck"].add("options", Menu::makeCheckbox(
			/*selfclickable=*/false));
		form["sanitycheck"]["options"].setMarginRight(
			5 * InterfaceElement::scale());
		form["sanitycheck"].add("text", new MultiTextField(
			_(""
			"I confirm that I want to merge my Steam-based Epicinium account"
			" with my email-based Epicinium account."
			""),
			FONTSIZESMALL));
		form["sanitycheck"]["text"].setMarginVertical(
			2 * InterfaceElement::scale());
		form["sanitycheck"].makeClickable();
		form["sanitycheck"].setMarginTop(15 * InterfaceElement::scale());
		form["sanitycheck"].align(VerticalAlignment::MIDDLE);
		form.add("buttons", new HorizontalLayout());
		form["buttons"].add("filler1", new HorizontalFiller());
		form["buttons"].add("confirm", makeButtonTiny(
			_("confirm"),
			FONTSIZESMALL));
		form["buttons"]["confirm"].setHotkeyScancode(SDL_SCANCODE_ENTER);
		form["buttons"].add("cancel", makeButtonTiny(
			_("cancel"),
			FONTSIZESMALL));
		form["buttons"]["cancel"].setHotkeyScancode(SDL_SCANCODE_ESCAPE);
		form["buttons"].add("filler2", new HorizontalFiller());
		form["buttons"].setMarginTop(
			15 * InterfaceElement::scale());
		form.add("filler2", new Filler());
		form.setMargin(5 * InterfaceElement::scale());
	}

	{
		_layout["mid"].add("keyform", Frame::makeForm());
		InterfaceElement& form = _layout["mid"]["keyform"];
		form.put(new VerticalLayout());
		form.add("filler1", new Filler());
		form.add("error", makeErrorprompt(
			_("An error has occurred."),
			FONTSIZESMALL, /*clickable=*/false));
		form["error"].setMarginBottom(
			15 * InterfaceElement::scale());
		form["error"].kill();
		form.add("info", makeInfoprompt(
			// TRANSLATORS: "Key" here refers to a sequence of letters and
			// numbers that the user enters in a textfield to unlock access
			// to the game.
			_(""
			"If you have a key, activate it here."
			""),
			FONTSIZESMALL, /*clickable=*/false));
		form["info"].setMarginBottom(
			15 * InterfaceElement::scale());
		form.add("title_token", new TextField(
			// TRANSLATORS: "Key" here refers to a sequence of letters and
			// numbers that the user enters in a textfield to unlock access
			// to the game.
			_("Key:"),
			FONTSIZESMALL));
		form["title_token"].setMarginTop(
			5 * InterfaceElement::scale());
		form["title_token"].setMarginBottom(
			1 * InterfaceElement::scale());
		form.add("token", makeKeyTokenInput());
		form.add("buttons", new HorizontalLayout());
		form["buttons"].add("filler1", new HorizontalFiller());
		form["buttons"].add("confirm", makeButtonTiny(
			_("confirm"),
			FONTSIZE_MENUBUTTON));
		form["buttons"]["confirm"].setHotkeyScancode(SDL_SCANCODE_ENTER);
		form["buttons"].add("cancel", makeButtonTiny(
			_("cancel"),
			FONTSIZE_MENUBUTTON));
		form["buttons"]["cancel"].setHotkeyScancode(SDL_SCANCODE_ESCAPE);
		form["buttons"].add("filler2", new HorizontalFiller());
		form["buttons"].setMarginTop(
			15 * InterfaceElement::scale());
		form.add("filler2", new Filler());
		form.setMargin(5 * InterfaceElement::scale());
	}

	{
		_layout["mid"].add("unlockform", Frame::makeForm());
		InterfaceElement& form = _layout["mid"]["unlockform"];
		form.put(new VerticalLayout());
		form.add("filler1", new Filler());
		form.add("info", makeInfoprompt(
			_("Key activation successful!"),
			FONTSIZESMALL, /*clickable=*/false));
		form["info"].setMarginBottom(
			15 * InterfaceElement::scale());
		form.add("buttons", new HorizontalLayout());
		form["buttons"].add("filler1", new HorizontalFiller());
		form["buttons"].add("confirm", makeButtonTiny(
			_("confirm"),
			FONTSIZE_MENUBUTTON));
		form["buttons"]["confirm"].setHotkeyScancode(SDL_SCANCODE_ENTER);
		form["buttons"]["confirm"].setSecondHotkeyScancode(SDL_SCANCODE_ESCAPE);
		form["buttons"].add("filler2", new HorizontalFiller());
		form["buttons"].setMarginTop(
			15 * InterfaceElement::scale());
		form.add("filler2", new Filler());
		form.setMargin(5 * InterfaceElement::scale());
	}

	{
		_layout["mid"].add("steamusernameform", Frame::makeForm());
		InterfaceElement& form = _layout["mid"]["steamusernameform"];
		form.put(new VerticalLayout());
		form.add("filler1", new Filler());
		form.add("error", makeErrorprompt(
			_(""
			"There already exists an Epicinium user with the same name."
			" Please enter a new username to use within Epicinium."
			""),
			FONTSIZESMALL, /*clickable=*/false));
		form["error"].setMarginBottom(
			15 * InterfaceElement::scale());
		form.add("title_username", new TextField(
			_("Username:"),
			FONTSIZESMALL));
		form["title_username"].setMarginTop(
			5 * InterfaceElement::scale());
		form["title_username"].setMarginBottom(
			1 * InterfaceElement::scale());
		form.add("username", makeUsernameInput());
		form.add("buttons", new HorizontalLayout());
		form["buttons"].add("filler1", new HorizontalFiller());
		form["buttons"].add("confirm", makeButtonTiny(
			_("confirm"),
			FONTSIZESMALL));
		form["buttons"]["confirm"].setHotkeyScancode(SDL_SCANCODE_ENTER);
		form["buttons"].add("cancel", makeButtonTiny(
			_("cancel"),
			FONTSIZESMALL));
		form["buttons"]["cancel"].setHotkeyScancode(SDL_SCANCODE_ESCAPE);
		form["buttons"].add("filler2", new HorizontalFiller());
		form["buttons"].setMarginTop(
			15 * InterfaceElement::scale());
		form.add("filler2", new Filler());
		form.setMargin(5 * InterfaceElement::scale());
	}

	{
		_layout["mid"].add("forgotpwform", Frame::makeForm());
		InterfaceElement& form = _layout["mid"]["forgotpwform"];
		form.put(new VerticalLayout());
		form.add("filler1", new Filler());
		form.add("info", makeInfoprompt(
			_(""
			"If you forgot your password and want to reset it,"
			" please enter the email address that you used to register."
			" We will email you a password reset token."
			""),
			FONTSIZESMALL, /*clickable=*/false));
		form["info"].setMarginBottom(
			15 * InterfaceElement::scale());
		form.add("title_email", new TextField(
			_("Email address:"),
			FONTSIZESMALL));
		form["title_email"].setMarginTop(
			5 * InterfaceElement::scale());
		form["title_email"].setMarginBottom(
			1 * InterfaceElement::scale());
		form.add("email", makeEmailInput());
		form.add("buttons", new HorizontalLayout());
		form["buttons"].add("filler1", new HorizontalFiller());
		form["buttons"].add("confirm", makeButtonTiny(
			_("confirm"),
			FONTSIZE_MENUBUTTON));
		form["buttons"]["confirm"].setHotkeyScancode(SDL_SCANCODE_ENTER);
		form["buttons"].add("cancel", makeButtonTiny(
			_("cancel"),
			FONTSIZE_MENUBUTTON));
		form["buttons"]["cancel"].setHotkeyScancode(SDL_SCANCODE_ESCAPE);
		form["buttons"].add("filler2", new HorizontalFiller());
		form["buttons"].setMarginTop(
			15 * InterfaceElement::scale());
		form.add("filler2", new Filler());
		form.setMargin(5 * InterfaceElement::scale());
	}

	{
		_layout["mid"].add("resetpwform", Frame::makeForm());
		InterfaceElement& form = _layout["mid"]["resetpwform"];
		form.put(new VerticalLayout());
		form.add("filler1", new Filler());
		form.add("error", makeErrorprompt(
			_("An error has occurred."),
			FONTSIZESMALL, /*clickable=*/false));
		form["error"].setMarginBottom(
			15 * InterfaceElement::scale());
		form["error"].kill();
		form.add("info", makeInfoprompt(
			_(""
			"Within a few minutes, you should receive an email"
			" containing your password reset token."
			" Please check your spam folder as well!"
			" If you have trouble restting your password,"
			" please contact us at help@epicinium.nl"
			""),
			FONTSIZESMALL, /*clickable=*/false));
		form["info"].setMarginBottom(
			15 * InterfaceElement::scale());
		form.add("title_email", new TextField(
			_("Email address:"),
			FONTSIZESMALL));
		form["title_email"].setMarginTop(
			5 * InterfaceElement::scale());
		form["title_email"].setMarginBottom(
			1 * InterfaceElement::scale());
		form.add("email", new Padding());
		form["email"].put(new TextField("", FONTSIZESMALL));
		form["email"].setPaddingLeft(4 * InterfaceElement::scale());
		form["email"].setMargin(4 * InterfaceElement::scale());
		form.add("title_token", new TextField(
			_("Token:"),
			FONTSIZESMALL));
		form["title_token"].setMarginTop(
			5 * InterfaceElement::scale());
		form["title_token"].setMarginBottom(
			1 * InterfaceElement::scale());
		form.add("token", makePasswordResetTokenInput());
		form.add("title_password", new TextField(
			_("New password:"),
			FONTSIZESMALL));
		form["title_password"].setMarginTop(
			5 * InterfaceElement::scale());
		form["title_password"].setMarginBottom(
			1 * InterfaceElement::scale());
		form.add("password", makePasswordInput());
		form.add("title_password2", new TextField(
			_("Confirm password:"),
			FONTSIZESMALL));
		form["title_password2"].setMarginTop(
			5 * InterfaceElement::scale());
		form["title_password2"].setMarginBottom(
			1 * InterfaceElement::scale());
		form.add("password2", makePassword2Input());
		form.add("buttons", new HorizontalLayout());
		form["buttons"].add("filler1", new HorizontalFiller());
		form["buttons"].add("confirm", makeButtonTiny(
			_("confirm"),
			FONTSIZE_MENUBUTTON));
		form["buttons"]["confirm"].setHotkeyScancode(SDL_SCANCODE_ENTER);
		form["buttons"].add("cancel", makeButtonTiny(
			_("cancel"),
			FONTSIZE_MENUBUTTON));
		form["buttons"]["cancel"].setHotkeyScancode(SDL_SCANCODE_ESCAPE);
		form["buttons"].add("filler2", new HorizontalFiller());
		form["buttons"].setMarginTop(
			15 * InterfaceElement::scale());
		form.add("filler2", new Filler());
		form.setMargin(5 * InterfaceElement::scale());
	}

	{
		_layout["mid"].add("registrationform", Frame::makeForm());
		InterfaceElement& form = _layout["mid"]["registrationform"];
		form.put(new VerticalLayout());
		form.add("filler1", new Filler());
		form.add("error", makeErrorprompt(
			_("An error has occurred."),
			FONTSIZESMALL, /*clickable=*/false));
		form["error"].setMarginBottom(
			(InterfaceElement::windowH() < 500 * InterfaceElement::scale())
			? 5 * InterfaceElement::scale()
			: 15 * InterfaceElement::scale());
		form["error"].kill();
		form.add("title_username", new TextField(
			_("Username:"),
			FONTSIZESMALL));
		form["title_username"].setMarginTop(
			5 * InterfaceElement::scale());
		form["title_username"].setMarginBottom(
			1 * InterfaceElement::scale());
		form.add("username", makeUsernameInput());
		form.add("title_email", new TextField(
			_("Email address:"),
			FONTSIZESMALL));
		form["title_email"].setMarginTop(
			5 * InterfaceElement::scale());
		form["title_email"].setMarginBottom(
			1 * InterfaceElement::scale());
		form.add("email", makeEmailInput());
		form.add("title_password", new TextField(
			_("Password:"),
			FONTSIZESMALL));
		form["title_password"].setMarginTop(
			5 * InterfaceElement::scale());
		form["title_password"].setMarginBottom(
			1 * InterfaceElement::scale());
		form.add("password", makePasswordInput());
		form.add("title_password2", new TextField(
			_("Confirm password:"),
			FONTSIZESMALL));
		form["title_password2"].setMarginTop(
			5 * InterfaceElement::scale());
		form["title_password2"].setMarginBottom(
			1 * InterfaceElement::scale());
		form.add("password2", makePassword2Input());
		form.add("mailinglist", Frame::makeItem(
			ColorName::FRAME200));
		form["mailinglist"].put(new HorizontalLayout());
		form["mailinglist"].add("texts", new MultiTextField(
			_(""
			"Keep me up to date"
			" via email"
			" with news about Epicinium"
			" and other games made by A Bunch of Hacks."
			""),
			FONTSIZE_MENUBUTTON));
		form["mailinglist"].add("options", Menu::makeCheckbox(
			/*selfclickable=*/false));
		form["mailinglist"].makeClickable();
		form["mailinglist"]["texts"].setMargin(
			2 * InterfaceElement::scale());
		form["mailinglist"].setMarginTop(
			(InterfaceElement::windowH() < 500 * InterfaceElement::scale())
			? 5 * InterfaceElement::scale()
			: 15 * InterfaceElement::scale());
		form["mailinglist"].align(VerticalAlignment::MIDDLE);
		form.add("buttons", new HorizontalLayout());
		form["buttons"].add("filler1", new HorizontalFiller());
		form["buttons"].add("confirm", makeButtonTiny(
			_("confirm"),
			FONTSIZE_MENUBUTTON));
		form["buttons"]["confirm"].setHotkeyScancode(SDL_SCANCODE_ENTER);
		form["buttons"].add("cancel", makeButtonTiny(
			_("cancel"),
			FONTSIZE_MENUBUTTON));
		form["buttons"]["cancel"].setHotkeyScancode(SDL_SCANCODE_ESCAPE);
		form["buttons"].add("filler2", new HorizontalFiller());
		form["buttons"].setMarginTop(
			(InterfaceElement::windowH() < 500 * InterfaceElement::scale())
			? 5 * InterfaceElement::scale()
			: 15 * InterfaceElement::scale());
		form.add("filler2", new Filler());
		form.add("disclosure", makeInfoprompt(
			_(""
			"A valid email address is necessary to recover your account."
			" We will only send emails related to account management"
			" and (if you opt in) updates about our games."
			" Click here to read our full privacy statement."
			""),
			FONTSIZE_MENUBUTTON, /*clickable=*/true));
		form.add("token", new HiddenTag(""));
		form.setMargin(5 * InterfaceElement::scale());
	}

	{
		_layout["mid"].add("storageissueform", Frame::makeForm());
		InterfaceElement& form = _layout["mid"]["storageissueform"];
		form.put(new VerticalLayout());
		form.add("filler1", new Filler());
		form.add("texts", new MultiTextField(
			_(""
			"Saving files seems not to be working!"
			" Please make sure the game is unpacked or installed"
			" in a folder where you have permission to write."
			" Settings might not be stored correctly"
			" when the game closes"
			"."
			""),
			FONTSIZESMALL));
		form.add("buttons", new HorizontalLayout());
		form["buttons"].add("filler1", new HorizontalFiller());
		form["buttons"].add("ok", makeButtonSmall(
			_("ok"),
			FONTSIZE_MENUBUTTON));
		form["buttons"]["ok"].setHotkeyScancode(SDL_SCANCODE_ENTER);
		form["buttons"]["ok"].setSecondHotkeyScancode(SDL_SCANCODE_ESCAPE);
		form["buttons"].add("filler2", new HorizontalFiller());
		form["buttons"].setMarginTop(
			15 * InterfaceElement::scale());
		form.add("filler2", new Filler());
		form.setMargin(5 * InterfaceElement::scale());
	}

	{
		_layout["mid"].add("feedbackform", Frame::makeForm());
		InterfaceElement& form = _layout["mid"]["feedbackform"];
		form.put(new VerticalLayout());
		form.add("filler1", new Filler());
		form.add("error", makeErrorprompt(
			_("An error has occurred."),
			FONTSIZESMALL, /*clickable=*/false));
		form["error"].kill();
		form.add("info", makeInfoprompt(
			"", FONTSIZE_MENUBUTTON, /*clickable=*/true));
		form["info"].add("url", new HiddenTag(""));
		form["info"].setMarginBottom(
			15 * InterfaceElement::scale());
		form["info"].kill();
		form.add("row", new HorizontalLayout());
		form["row"].align(VerticalAlignment::MIDDLE);
		form["row"].add("type", new TooltipLayout(/*dropdown=*/true));
		form["row"]["type"].add("content", new SlideshowLayout());
		auto& content = form["row"]["type"]["content"];
		form["row"]["type"].add("dropdown", Frame::makeTooltip(/*dropdown=*/true));
		auto& dropdown = form["row"]["type"]["dropdown"];
		dropdown.put(new VerticalLayout());
		for (auto& text : {
				std::make_pair(stringref("bug"), (const char*)
					_("Bug report")),
				std::make_pair(stringref("wish"),
					// Not translated because Stomts are in English.
					"I wish Epicinium..."),
				std::make_pair(stringref("like"),
					// Not translated because Stomts are in English.
					"I like Epicinium..."),
			})
		{
			auto tagname = text.first;
			auto name = text.second;
			content.add(tagname, Frame::makeItem(ColorName::FRAMEITEM));
			content[tagname].put(new TextField(name, FONTSIZE_MENUBUTTON));
			content[tagname].align(HorizontalAlignment::CENTER);
			content[tagname].makeClickable();
			content[tagname].setPadding(4 * InterfaceElement::scale());
			dropdown.add(tagname, Frame::makeItem());
			dropdown[tagname].put(new TextField(name, FONTSIZE_MENUBUTTON));
			dropdown[tagname].align(HorizontalAlignment::CENTER);
			dropdown[tagname].makeClickable();
		}
		content.settleWidth();
		content.settleHeight();
		content.fixWidth();
		dropdown.settleWidth();
		dropdown.settleHeight();
		dropdown.fixWidth();
		form["row"].add("info", makeInfoprompt(
			_("Please describe what happened."),
			FONTSIZESMALL, /*clickable=*/false));
		form["row"]["info"].setMarginLeft(10 * InterfaceElement::scale());
		form["row"]["info"].setPadding(3 * InterfaceElement::scale());
		form.add("input", new Frame("ui/frame_window_9"));
		form["input"].put(new MultiTextInput(
			_settings.getFontSizeTextInput(),
			/*lines=*/8));
		form["input"].setMarginTop(5 * InterfaceElement::scale());
		form["input"].setPadding(4 * InterfaceElement::scale());
		form.add("sendlogs", Frame::makeItem(ColorName::FRAME200));
		form["sendlogs"].put(new HorizontalLayout());
		form["sendlogs"].add("title", new MultiTextField(
			_(""
			"Send my Epicinium log files to the developers"
			" to help them with debugging:"
			""),
			FONTSIZE_MENUBUTTON));
		form["sendlogs"].add("options", Menu::makeCheckbox(
			/*selfclickable=*/false));
		form["sendlogs"]["options"].setTag("Checked");
		form["sendlogs"].makeClickable();
		form["sendlogs"].setMarginTop(15 * InterfaceElement::scale());
		form["sendlogs"].align(VerticalAlignment::MIDDLE);
		form.add("buttons", new HorizontalLayout());
		form["buttons"].add("filler1", new HorizontalFiller());
		form["buttons"].add("send", makeButtonTiny(
			_("send"),
			FONTSIZE_MENUBUTTON));
		form["buttons"].add("cancel", makeButtonTiny(
			_("cancel"),
			FONTSIZE_MENUBUTTON));
		form["buttons"]["cancel"].setHotkeyScancode(SDL_SCANCODE_ESCAPE);
		form["buttons"].add("filler2", new HorizontalFiller());
		form["buttons"].setMarginTop(15 * InterfaceElement::scale());
		form.add("filler2", new Filler());
		form.add("version", new TextField(
			_client.getVersionDebugString(),
			_settings.getFontSize(),
			ColorName::TEXT600));
		form.setMargin(5 * InterfaceElement::scale());
	}

	{
		_layout["right"].add("topright", new VerticalLayout());
		InterfaceElement& topright = _layout["right"]["topright"];
		topright.add("connection", new TooltipLayout());
		topright["connection"].add("icon", new Image("ui/connection"));
		topright["connection"].add("tooltip", Frame::makeTooltip());
		topright["connection"]["tooltip"].put(new MultiTextField(
			_("Initializing..."),
			FONTSIZESMALL));
		topright["connection"]["tooltip"].setMargin(
			5 * InterfaceElement::scale());
		topright["connection"].setMarginBottom(
			5 * InterfaceElement::scale());
		if (_client.isSteamEnabled())
		{
			topright.add("steam", new TooltipLayout());
			topright["steam"].add("icon", new Image("ui/steam"));
			topright["steam"].add("tooltip", Frame::makeTooltip());
			topright["steam"]["tooltip"].put(new MultiTextField(
				_("Initializing..."),
				FONTSIZESMALL));
			topright["steam"]["tooltip"].setMargin(
				5 * InterfaceElement::scale());
			topright["steam"].setMarginBottom(
				5 * InterfaceElement::scale());
		}
		else
		{
			topright.add("login", new TooltipLayout());
			topright["login"].add("icon", new Image("ui/login"));
			topright["login"].add("tooltip", Frame::makeTooltip());
			topright["login"]["tooltip"].put(new MultiTextField(
				_("Not logged in."),
				FONTSIZESMALL));
			topright["login"]["tooltip"].setMargin(
				5 * InterfaceElement::scale());
			topright["login"].setMarginBottom(
				5 * InterfaceElement::scale());
		}
		topright.add("discord", new TooltipLayout());
		topright["discord"].add("icon", new Image("ui/discord"));
		topright["discord"].add("tooltip", Frame::makeTooltip());
		topright["discord"]["tooltip"].put(new MultiTextField(
			_("Find us on Discord!"),
			FONTSIZESMALL));
		topright["discord"]["tooltip"].setMargin(
			5 * InterfaceElement::scale());
		topright["discord"].setMarginBottom(
			5 * InterfaceElement::scale());
	}

	{
		_layout["right"].add("filler", new Filler());

		_layout["right"].add("settings", makeButtonSmall(
			_("SETTINGS"),
			FONTSIZE_MENUBUTTON));
		_layout["right"]["settings"].setAltHotkeyScancode(SDL_SCANCODE_S);
		_layout["right"].add("credits", makeButtonSmall(
			_("CREDITS"),
			FONTSIZE_MENUBUTTON));
		_layout["right"]["credits"].setAltHotkeyScancode(SDL_SCANCODE_C);
		_layout["right"].add("quit", makeButtonSmall(
			// TRANSLATORS: Closes the entire game.
			_("QUIT"),
			FONTSIZE_MENUBUTTON));
		_layout["right"]["quit"].setHotkeyScancode(SDL_SCANCODE_ESCAPE);

		_layout["right"].align(HorizontalAlignment::RIGHT);
	}

	{
		InterfaceElement& icon = _layout["right"]["topright"]["connection"]["icon"];
		int i = 0;
		icon.setColor(i++, Paint::blend(ColorName::LOGOTEXT, ColorName::SHINEBLEND));
		icon.setColor(i++, ColorName::LOGOTEXT);
		icon.setColor(i++, Paint::blend(ColorName::LOGOTEXT, ColorName::SHADEBLEND));
		icon.setColor(i++, Paint::blend(ColorName::LOGODIRT, ColorName::SHINEBLEND));
		icon.setColor(i++, ColorName::LOGODIRT);
		icon.setColor(i++, Paint::blend(ColorName::LOGODIRT, ColorName::SHADEBLEND));
		icon.setColor(i++, Paint::blend(ColorName::LOGOGRASS, ColorName::SHINEBLEND));
		icon.setColor(i++, ColorName::LOGOGRASS);
		icon.setColor(i++, Paint::blend(ColorName::LOGOGRASS, ColorName::SHADEBLEND));
		icon.setColor(i++, Paint::blend(ColorName::LOGOWATER, ColorName::SHINEBLEND));
		icon.setColor(i++, ColorName::LOGOWATER);
		icon.setColor(i++, Paint::blend(ColorName::LOGOWATER, ColorName::SHADEBLEND));
		icon.setColor(i++, Color::transparent());
		icon.setTag("Empty");
		icon.makeHoverable();
	}

	if (_layout["right"]["topright"].contains("login"))
	{
		InterfaceElement& icon = _layout["right"]["topright"]["login"]["icon"];
		int i = 0;
		icon.setColor(i++, Paint::blend(ColorName::LOGOTEXT, ColorName::SHINEBLEND));
		icon.setColor(i++, ColorName::LOGOTEXT);
		icon.setColor(i++, Paint::blend(ColorName::LOGOTEXT, ColorName::SHADEBLEND));
		icon.setColor(i++, Paint::blend(ColorName::FRAME400, ColorName::SHINEBLEND));
		icon.setColor(i++, ColorName::FRAME400);
		icon.setColor(i++, Paint::blend(ColorName::FRAME400, ColorName::SHADEBLEND));
		icon.setColor(i++, Paint::blend(ColorName::FRAME800, ColorName::SHINEBLEND, 0.5f));
		icon.setColor(i++, ColorName::FRAME800);
		icon.setColor(i++, Paint::blend(ColorName::FRAME800, ColorName::SHADEBLEND, 0.5f));
		icon.setColor(i++, Color::transparent());
		icon.setTag("Empty");
		icon.makeClickable();
	}

	if (_layout["right"]["topright"].contains("steam"))
	{
		InterfaceElement& icon = _layout["right"]["topright"]["steam"]["icon"];
		int i = 0;
		icon.setColor(i++, Paint::blend(ColorName::LOGOTEXT, ColorName::SHINEBLEND));
		icon.setColor(i++, ColorName::LOGOTEXT);
		icon.setColor(i++, Paint::blend(ColorName::LOGOTEXT, ColorName::SHADEBLEND));
		icon.setColor(i++, Paint::blend(ColorName::LOGOWATER, ColorName::SHINEBLEND));
		icon.setColor(i++, ColorName::LOGOWATER);
		icon.setColor(i++, Paint::blend(ColorName::LOGOWATER, ColorName::SHADEBLEND));
		icon.setColor(i++, Paint::blend(ColorName::FRAME100, ColorName::SHINEBLEND, 0.5f));
		icon.setColor(i++, ColorName::FRAME100);
		icon.setColor(i++, Paint::blend(ColorName::FRAME100, ColorName::SHADEBLEND, 0.5f));
		icon.setColor(i++, Color::transparent());
		icon.setTag("Empty");
		icon.makeHoverable();
	}

	if (_layout["right"]["topright"].contains("discord"))
	{
		InterfaceElement& icon = _layout["right"]["topright"]["discord"]["icon"];
		int i = 0;
		icon.setColor(i++, Paint::blend(ColorName::LOGOTEXT, ColorName::SHINEBLEND));
		icon.setColor(i++, ColorName::LOGOTEXT);
		icon.setColor(i++, Paint::blend(ColorName::LOGOTEXT, ColorName::SHADEBLEND));
		icon.setColor(i++, Paint::blend(ColorName::LOGOICE, ColorName::SHINEBLEND));
		icon.setColor(i++, ColorName::LOGOICE);
		icon.setColor(i++, Paint::blend(ColorName::LOGOICE, ColorName::SHADEBLEND));
		icon.setColor(i++, Paint::blend(ColorName::FRAME800, ColorName::SHINEBLEND, 0.5f));
		icon.setColor(i++, ColorName::FRAME800);
		icon.setColor(i++, Paint::blend(ColorName::FRAME800, ColorName::SHADEBLEND, 0.5f));
		icon.setColor(i++, Color::transparent());
		icon.setTag("Empty");
		icon.makeClickable();
	}

	{
		InterfaceElement& logo = _layout["mid"]["buttons"]["logo"]["title"];
		int i = 0;

		logo.setColor(i++, Paint::blend(ColorName::LOGOTEXT, ColorName::SHINEBLEND));
		logo.setColor(i++, ColorName::LOGOTEXT);
		logo.setColor(i++, Paint::blend(ColorName::LOGOTEXT, ColorName::SHADEBLEND));

		logo.setColor(i++, ColorName::LOGOTEXTBORDER);
		logo.setColor(i++, ColorName::LOGOTEXTBORDER);
		logo.setColor(i++, Paint::blend(ColorName::LOGOTEXTBORDER, ColorName::SHADEBLEND, 2.00f));
	}

	_layout.fixWidth(InterfaceElement::windowW());
	_layout.fixHeight(InterfaceElement::windowH());
	_layout.place(Pixel(0, 0, Layer::INTERFACE));

	_background["letterbox"]["top"].fixHeight(
		_layout["right"]["topright"].height()
		+ ((InterfaceElement::windowW() < 600 * InterfaceElement::scale()
			|| InterfaceElement::windowH() < 400 * InterfaceElement::scale())
		? 20 * InterfaceElement::scale()
		: 30 * InterfaceElement::scale()));
	_background["letterbox"]["bottom"].fixHeight(
		_background["letterbox"]["top"].height());

	_background.fixWidth(InterfaceElement::windowW());
	_background.fixHeight(InterfaceElement::windowH());
	_background.place(Pixel(0, 0, Layer::BACKGROUND));

	_layout["left"].fixWidth();
	_layout["left"].fixHeight();
	_layout["mid"].fixWidth();
	_layout["mid"].fixHeight();
	_layout["right"].fixWidth();
	_layout["right"].fixHeight();
	for (size_t i = 0; i < _layout["mid"].size(); i++)
	{
		const std::string& name = _layout["mid"].name(i);
		_layout["mid"][name].fixWidth();
		_layout["mid"][name].fixHeight();
	}
	_layout["mid"]["feedbackform"]["input"].content().fixWidth();

	if (_settings.patchmode.value()
		== PatchMode::SERVER_BUT_DISABLED_DUE_TO_STORAGE_ISSUES)
	{
		_toForm = "storageissueform";
	}

	evaluatePlayable();

	Mixer::get()->loopOST(0.0f);
	Mixer::get()->fade(Mixer::get()->getOSTid(), 1.0f, 3.0f);
}

InterfaceElement& MainMenu::getForms()
{
	return _layout["mid"];
}

InterfaceElement& MainMenu::getLogo()
{
	return _layout["mid"]["buttons"]["logo"];
}

InterfaceElement& MainMenu::getStamp()
{
	return _layout["mid"]["buttons"]["logo"]["stamp"];
}

InterfaceElement& MainMenu::getEnterKeyButton()
{
	return _layout["mid"]["buttons"]["enterkey"];
}

InterfaceElement& MainMenu::getFeedbackButton()
{
	return _layout["mid"]["buttons"]["feedback"];
}

InterfaceElement& MainMenu::getPlayOrDownloading()
{
	return _layout["mid"]["buttons"]["play"];
}

InterfaceElement& MainMenu::getPlay()
{
	InterfaceElement& element = getPlayOrDownloading();
	return element["mp"];
}

InterfaceElement& MainMenu::getPlayButton()
{
	InterfaceElement& play = getPlay();
	return play["button"];
}

InterfaceElement& MainMenu::getConnectionNub()
{
	return _layout["right"]["topright"]["connection"];
}

bool MainMenu::hasLoginNub()
{
	return _layout["right"]["topright"].contains("login");
}

InterfaceElement& MainMenu::getLoginNub()
{
	return _layout["right"]["topright"]["login"];
}

bool MainMenu::hasSteamNub()
{
	return _layout["right"]["topright"].contains("steam");
}

InterfaceElement& MainMenu::getSteamNub()
{
	return _layout["right"]["topright"]["steam"];
}

bool MainMenu::hasDiscordNub()
{
	return _layout["right"]["topright"].contains("discord");
}

InterfaceElement& MainMenu::getDiscordNub()
{
	return _layout["right"]["topright"]["discord"];
}

InterfaceElement& MainMenu::getSettingsButton()
{
	return _layout["right"]["settings"];
}

InterfaceElement& MainMenu::getCreditsButton()
{
	return _layout["right"]["credits"];
}

InterfaceElement& MainMenu::getQuitButton()
{
	return _layout["right"]["quit"];
}

InterfaceElement& MainMenu::getLoginButton()
{
	return _layout["left"]["login"];
}

InterfaceElement& MainMenu::getLogoutButton()
{
	return _layout["left"]["logout"];
}

InterfaceElement& MainMenu::getRegisterButton()
{
	return _layout["left"]["register"];
}

InterfaceElement& MainMenu::getCenteredError()
{
	return _background["error"];
}

void MainMenu::refresh()
{
	if (_layout.alive() && _client.resetting() && _client.autoresetActive())
	{
		resetting();
		_client.connect();
	}
	else if (_layout.alive() && _client.disconnected()
		&& getPlayButton().enabled())
	{
		disconnected();
	}
	else if (getCenteredError().getTag() == "resetting"
		&& _connectionStatus != ConnectionStatus::CONNECTING
		&& _connectionStatus != ConnectionStatus::RESETTING
		&& !_client.disconnected())
	{
		InterfaceElement& error = getCenteredError();
		error.setTag("reconnected");
	}

	bool linkHovered = false;

	// Because some items might occlude others, we must know which layer the
	// mouse operates in.
	// We therefore need to check all UI items to see if they are hovered.
	_background.hovered();
	_layout.hovered();

	_background.refresh();
	_layout.refresh();

	{
		InterfaceElement& stamp = getStamp();
		if (stamp.hovered() && !stamp["url"].text().empty())
		{
			linkHovered = true;
			if (stamp.clicked())
			{
				System::openURL(stamp["url"].text());
			}
		}
	}

	InterfaceElement& playbutton = getPlayButton();
	if (playbutton.enabled()
		&& getForms().getTag() == "buttons"
		&& playbutton.shown()
		&& (playbutton.clicked()
			|| _client.hotJoining()
			|| _client.hotSpectating()))
	{
		if (_connectionStatus == ConnectionStatus::PATCH_AVAILABLE)
		{
			_client.startPatch();
		}
		else if (_connectionStatus == ConnectionStatus::PATCH_PRIMED)
		{
			_owner.quit(ExitCode::APPLY_PATCH_AND_RESTART);
		}
		else if (_client.isSteamEnabled())
		{
			switch (_steamStatus)
			{
				case SteamStatus::CONNECTED:
				{
					Mixer::get()->stop();
					kill();
					_submenus[MULTIPLAYER]->open();
					Mixer::get()->fade(Mixer::get()->getOSTid(), 0.2f, 1.5f);
					Mixer::get()->fade(Mixer::get()->getMidiOSTid(), 0.8f, 1.5f);
				}
				break;

				case SteamStatus::NONE:
				case SteamStatus::CONNECTING:
				{
					LOGE << "The button should be disabled";
					evaluatePlayable();
				}
				break;
			}
		}
		else
		{
			switch (_loginStatus)
			{
				case LoginStatus::NONE:
				{
#if DISCORD_GUEST_ENABLED
					switch (_discordStatus)
					{
						case DiscordStatus::CONNECTED:
						{
							Mixer::get()->stop();
							kill();
							_submenus[MULTIPLAYER]->open();
						}
						break;

						case DiscordStatus::NONE:
						{
							if (_settings.discord.value()
								&& _settings.allowDiscordLogin.value())
							{
								_client.activateDiscord();
								_toForm = "discordloginform";
							}
							else
							{
								_toForm = "guestform";
							}
						}
						break;
					}
#else
					LOGE << "The button should be disabled";
					evaluatePlayable();
#endif
				}
				break;

				case LoginStatus::LOGGINGIN:
				case LoginStatus::REGISTERING:
				case LoginStatus::REGISTERED:
				case LoginStatus::RESETTING_PW:
				case LoginStatus::ACTIVATING_KEY:
				case LoginStatus::RESET_PW:
				{
					LOGE << "The button should be disabled";
					evaluatePlayable();
				}
				break;

				case LoginStatus::DISABLED:
				case LoginStatus::LOGGEDIN:
				{
					Mixer::get()->fade(Mixer::get()->getOSTid(), 0.2f, 1.5f);
					Mixer::get()->fade(Mixer::get()->getMidiOSTid(), 0.8f, 1.5f);
					kill();
					_submenus[MULTIPLAYER]->open();
				}
				break;
			}
		}
	}

	if (getForms().getTag() == "buttons")
	{
		playbutton.setHotkeyScancode(SDL_SCANCODE_ENTER);
	}
	else
	{
		playbutton.setHotkeyScancode(0);
	}

	{
		InterfaceElement& quitbutton = getQuitButton();
		if (quitbutton.clicked())
		{
			_owner.quit();
		}

		if (getForms().getTag() == "buttons")
		{
			quitbutton.setHotkeyScancode(SDL_SCANCODE_ESCAPE);
		}
		else
		{
			quitbutton.setHotkeyScancode(0);
		}
	}

	{
		InterfaceElement& forms = getForms();
		InterfaceElement& form = forms["loginform"];

		{
			auto& input = form["email"];
			if (input.clicked())
			{
				input["input"].power();
			}
			else if (input["input"].powered()
				&& Input::get()->wasKeyPressed(SDL_SCANCODE_LMB))
			{
				input["input"].depower();
			}
		}

		{
			auto& input = form["password"];
			if (input.clicked())
			{
				input["input"].power();
			}
			else if (input["input"].powered()
				&& Input::get()->wasKeyPressed(SDL_SCANCODE_LMB))
			{
				input["input"].depower();
			}
		}

		if (Input::get()->wasKeyPressed(SDL_SCANCODE_TAB))
		{
			static constexpr stringref tab[] = {
				"email",
				"password",
			};
			for (size_t i = 0; i < array_size(tab); i++)
			{
				if (form[tab[i]]["input"].powered())
				{
					size_t j;
					if (Input::get()->isKeyHeld(SDL_SCANCODE_SHIFT))
					{
						j = (i + array_size(tab) - 1) % array_size(tab);
					}
					else
					{
						j = (i + 1) % array_size(tab);
					}
					form[tab[i]]["input"].depower();
					form[tab[j]]["input"].power();
					break;
				}
			}
		}

		if (form["keepmeloggedin"].clicked())
		{
			if (form["keepmeloggedin"]["options"].getTag() != "Empty")
			{
				form["keepmeloggedin"]["options"].setTag("Empty");
			}
			else
			{
				form["keepmeloggedin"]["options"].setTag("Checked");
			}
		}

		form["buttons"]["confirm"].enableIf(
			!form["email"]["input"].text().empty()
			&& !form["password"]["input"].text().empty());

		if (form["buttons"]["confirm"].clicked())
		{
			form["error"].kill();
			form.settle();
			_client.login(form["email"]["input"].text(),
				form["password"]["input"].text(),
				form["keepmeloggedin"]["options"].getTag() == "Checked");
		}

		if (form["buttons"]["cancel"].clicked())
		{
			_toForm = "buttons";
		}

		if (form["bottom"]["forgotpw"].clicked())
		{
			_toForm = "forgotpwform";
			forms["forgotpwform"]["email"]["input"].setText(
				form["email"]["input"].text());
			forms["forgotpwform"]["email"]["input"].power();
		}
	}

	{
		InterfaceElement& forms = getForms();
		InterfaceElement& form = forms["guestform"];

		if (form["info"].hovered())
		{
			linkHovered = true;
			if (form["info"].clicked())
			{
				System::openURL("https://discord.gg/vQhTURC");
			}
		}

		if (form["buttons"]["confirm"].clicked())
		{
			_client.activateDiscord();

			_settings.allowDiscordLogin = true;
			_settings.save();

			_toForm = "discordloginform";
		}

		if (form["buttons"]["cancel"].clicked())
		{
			_toForm = "buttons";
		}
	}

	{
		InterfaceElement& forms = getForms();
		InterfaceElement& form = forms["discordloginform"];

		if (form["info"].hovered())
		{
			linkHovered = true;
			if (form["info"].clicked())
			{
				System::openURL("https://discord.gg/vQhTURC");
			}
		}

		form["buttons"]["confirm"].enableIf(
			_discordStatus == DiscordStatus::CONNECTED);

		if (form["buttons"]["confirm"].clicked())
		{
			_toForm = "buttons";
		}

		form["buttons"]["cancel"].enableIf(
			_discordStatus != DiscordStatus::CONNECTED);

		if (form["buttons"]["cancel"].clicked())
		{
			_client.deactivateDiscord();

			_toForm = "buttons";
		}
	}

	{
		InterfaceElement& forms = getForms();
		InterfaceElement& form = forms["steammergeform"];

		{
			auto& input = form["email"];
			if (input.clicked())
			{
				input["input"].power();
			}
			else if (input["input"].powered()
				&& Input::get()->wasKeyPressed(SDL_SCANCODE_LMB))
			{
				input["input"].depower();
			}
		}

		{
			auto& input = form["password"];
			if (input.clicked())
			{
				input["input"].power();
			}
			else if (input["input"].powered()
				&& Input::get()->wasKeyPressed(SDL_SCANCODE_LMB))
			{
				input["input"].depower();
			}
		}

		if (Input::get()->wasKeyPressed(SDL_SCANCODE_TAB))
		{
			static constexpr stringref tab[] = {
				"email",
				"password",
			};
			for (size_t i = 0; i < array_size(tab); i++)
			{
				if (form[tab[i]]["input"].powered())
				{
					size_t j;
					if (Input::get()->isKeyHeld(SDL_SCANCODE_SHIFT))
					{
						j = (i + array_size(tab) - 1) % array_size(tab);
					}
					else
					{
						j = (i + 1) % array_size(tab);
					}
					form[tab[i]]["input"].depower();
					form[tab[j]]["input"].power();
					break;
				}
			}
		}

		form["email"]["error"].showIf(
			!form["email"]["input"].text().empty()
			&& !isValidEmail(form["email"]["input"].text()));

		if (form["sanitycheck"].clicked())
		{
			if (form["sanitycheck"]["options"].getTag() != "Empty")
			{
				form["sanitycheck"]["options"].setTag("Empty");
			}
			else
			{
				form["sanitycheck"]["options"].setTag("Checked");
			}
		}

		form["buttons"]["confirm"].enableIf(
			_steamStatus == SteamStatus::CONNECTED
			&& !form["email"]["input"].text().empty()
			&& !form["email"]["error"].shown()
			&& !form["password"]["input"].text().empty()
			&& form["sanitycheck"]["options"].getTag() == "Checked");

		if (form["buttons"]["confirm"].clicked())
		{
			form["error"].kill();
			form.settle();
			_client.requestSteamAccountMergeToken(
				form["email"]["input"].text(),
				form["password"]["input"].text());
		}

		form["buttons"]["cancel"].enableIf(true);

		if (form["buttons"]["cancel"].clicked())
		{
			_client.cancelSteamAccountMergeToken();
			_toForm = "buttons";
		}
	}

	{
		InterfaceElement& forms = getForms();
		InterfaceElement& form = forms["keyform"];

		{
			auto& input = form["token"];
			if (input.clicked())
			{
				input["input"].power();
			}
			else if (input["input"].powered()
				&& Input::get()->wasKeyPressed(SDL_SCANCODE_LMB))
			{
				input["input"].depower();
			}
		}

		if (Input::get()->wasKeyPressed(SDL_SCANCODE_TAB))
		{
			static constexpr stringref tab[] = {
				"token",
			};
			for (size_t i = 0; i < array_size(tab); i++)
			{
				if (form[tab[i]]["input"].powered())
				{
					size_t j;
					if (Input::get()->isKeyHeld(SDL_SCANCODE_SHIFT))
					{
						j = (i + array_size(tab) - 1) % array_size(tab);
					}
					else
					{
						j = (i + 1) % array_size(tab);
					}
					form[tab[i]]["input"].depower();
					form[tab[j]]["input"].power();
					break;
				}
			}
		}

		if (!form["token"]["input"].text().empty())
		{
			form["token"]["input"].setText(
				canonizeKeyToken(form["token"]["input"].text()));
		}

		form["token"]["error"].showIf(
			!form["token"]["input"].text().empty()
			&& !isValidKeyToken(form["token"]["input"].text()));

		form["buttons"]["confirm"].enableIf(
			_loginStatus != LoginStatus::ACTIVATING_KEY
			&& !form["token"]["input"].text().empty()
			&& !form["token"]["error"].shown());

		if (form["buttons"]["confirm"].clicked())
		{
			_client.activateKey(form["token"]["input"].text());
		}

		form["buttons"]["cancel"].enableIf(
			_loginStatus != LoginStatus::ACTIVATING_KEY);

		if (form["buttons"]["cancel"].clicked())
		{
			_toForm = "buttons";
		}
	}

	{
		InterfaceElement& forms = getForms();
		InterfaceElement& form = forms["unlockform"];
		if (form["buttons"]["confirm"].clicked())
		{
			_toForm = "buttons";
		}
	}

	{
		InterfaceElement& forms = getForms();
		InterfaceElement& form = forms["steamusernameform"];

		{
			auto& input = form["username"];
			if (input.clicked())
			{
				input["input"].power();
			}
			else if (input["input"].powered()
				&& Input::get()->wasKeyPressed(SDL_SCANCODE_LMB))
			{
				input["input"].depower();
			}
		}

		if (Input::get()->wasKeyPressed(SDL_SCANCODE_TAB))
		{
			static constexpr stringref tab[] = {
				"username",
			};
			for (size_t i = 0; i < array_size(tab); i++)
			{
				if (form[tab[i]]["input"].powered())
				{
					size_t j;
					if (Input::get()->isKeyHeld(SDL_SCANCODE_SHIFT))
					{
						j = (i + array_size(tab) - 1) % array_size(tab);
					}
					else
					{
						j = (i + 1) % array_size(tab);
					}
					form[tab[i]]["input"].depower();
					form[tab[j]]["input"].power();
					break;
				}
			}
		}

		form["username"]["error"].showIf(
			!form["username"]["input"].text().empty()
			&& !isValidUsername(form["username"]["input"].text()));

		form["buttons"]["confirm"].enableIf(
			_steamStatus == SteamStatus::CONNECTED
			&& !form["username"]["input"].text().empty()
			&& !form["username"]["error"].shown());

		if (form["buttons"]["confirm"].clicked())
		{
			_client.setUsernameForSteam(form["username"]["input"].text());
			_toForm = "buttons";
		}

		form["buttons"]["cancel"].enableIf(true);

		if (form["buttons"]["cancel"].clicked())
		{
			_toForm = "buttons";
		}
	}

	{
		InterfaceElement& forms = getForms();
		InterfaceElement& form = forms["forgotpwform"];

		{
			auto& input = form["email"];
			if (input.clicked())
			{
				input["input"].power();
			}
			else if (input["input"].powered()
				&& Input::get()->wasKeyPressed(SDL_SCANCODE_LMB))
			{
				input["input"].depower();
			}
		}

		form["email"]["error"].showIf(
			!form["email"]["input"].text().empty()
			&& !isValidEmail(form["email"]["input"].text()));

		form["buttons"]["confirm"].enableIf(
			   !form["email"]["input"].text().empty()
			&& !form["email"]["error"].shown());

		if (form["buttons"]["confirm"].clicked())
		{
			_client.forgotPassword(form["email"]["input"].text());
			_toForm = "resetpwform";
			forms["resetpwform"]["email"].setText(
				form["email"]["input"].text());
			forms["resetpwform"]["token"]["input"].power();
			forms["resetpwform"]["error"].kill();
			forms["resetpwform"].fixWidth();
			forms["resetpwform"].settle();
		}

		if (form["buttons"]["cancel"].clicked())
		{
			_toForm = "loginform";
			forms["loginform"]["email"]["input"].power();
			forms["loginform"]["password"]["input"].depower();
			forms["loginform"]["error"].kill();
			forms["loginform"]["info"].kill();
			forms["loginform"].fixWidth();
			forms["loginform"].settle();
		}
	}

	{
		InterfaceElement& forms = getForms();
		InterfaceElement& form = forms["resetpwform"];

		{
			auto& input = form["token"];
			if (input.clicked())
			{
				input["input"].power();
			}
			else if (input["input"].powered()
				&& Input::get()->wasKeyPressed(SDL_SCANCODE_LMB))
			{
				input["input"].depower();
			}
		}

		{
			auto& input = form["password"];
			if (input.clicked())
			{
				input["input"].power();
			}
			else if (input["input"].powered()
				&& Input::get()->wasKeyPressed(SDL_SCANCODE_LMB))
			{
				input["input"].depower();
			}
		}

		{
			auto& input = form["password2"];
			if (input.clicked())
			{
				input["input"].power();
			}
			else if (input["input"].powered()
				&& Input::get()->wasKeyPressed(SDL_SCANCODE_LMB))
			{
				input["input"].depower();
			}
		}

		if (Input::get()->wasKeyPressed(SDL_SCANCODE_TAB))
		{
			static constexpr stringref tab[] = {
				"token",
				"password",
				"password2",
			};
			for (size_t i = 0; i < array_size(tab); i++)
			{
				if (form[tab[i]]["input"].powered())
				{
					size_t j;
					if (Input::get()->isKeyHeld(SDL_SCANCODE_SHIFT))
					{
						j = (i + array_size(tab) - 1) % array_size(tab);
					}
					else
					{
						j = (i + 1) % array_size(tab);
					}
					form[tab[i]]["input"].depower();
					form[tab[j]]["input"].power();
					break;
				}
			}
		}

		form["token"]["error"].showIf(
			!form["token"]["input"].text().empty()
			&& !isValidPasswordResetToken(form["token"]["input"].text()));

		form["password"]["error"].showIf(
			!form["password"]["input"].text().empty()
			&& !isValidPassword(form["password"]["input"].text()));

		form["password2"]["error"].showIf(
			!form["password2"]["input"].text().empty()
			&& form["password2"]["input"].text()
				!= form["password"]["input"].text());

		form["buttons"]["confirm"].enableIf(
			_loginStatus != LoginStatus::RESETTING_PW
			&& !form["token"]["input"].text().empty()
			&& !form["token"]["error"].shown()
			&& !form["password"]["input"].text().empty()
			&& !form["password"]["error"].shown()
			&& !form["password2"]["input"].text().empty()
			&& !form["password2"]["error"].shown());

		if (form["buttons"]["confirm"].clicked())
		{
			_client.resetPassword(form["email"].text(),
				form["token"]["input"].text(),
				form["password"]["input"].text());
		}

		form["buttons"]["cancel"].enableIf(
			_loginStatus != LoginStatus::RESETTING_PW);

		if (form["buttons"]["cancel"].clicked())
		{
			_toForm = "loginform";
			forms["loginform"]["email"]["input"].power();
			forms["loginform"]["password"]["input"].depower();
			forms["loginform"]["error"].kill();
			forms["loginform"]["info"].kill();
			forms["loginform"].fixWidth();
			forms["loginform"].settle();
		}
	}

	{
		InterfaceElement& forms = getForms();
		InterfaceElement& form = forms["registrationform"];

		{
			auto& input = form["username"];
			if (input.clicked())
			{
				input["input"].power();
			}
			else if (input["input"].powered()
				&& Input::get()->wasKeyPressed(SDL_SCANCODE_LMB))
			{
				input["input"].depower();
			}
		}

		{
			auto& input = form["email"];
			if (input.clicked())
			{
				input["input"].power();
			}
			else if (input["input"].powered()
				&& Input::get()->wasKeyPressed(SDL_SCANCODE_LMB))
			{
				input["input"].depower();
			}
		}

		{
			auto& input = form["password"];
			if (input.clicked())
			{
				input["input"].power();
			}
			else if (input["input"].powered()
				&& Input::get()->wasKeyPressed(SDL_SCANCODE_LMB))
			{
				input["input"].depower();
			}
		}

		{
			auto& input = form["password2"];
			if (input.clicked())
			{
				input["input"].power();
			}
			else if (input["input"].powered()
				&& Input::get()->wasKeyPressed(SDL_SCANCODE_LMB))
			{
				input["input"].depower();
			}
		}

		if (form["disclosure"].hovered())
		{
			linkHovered = true;
			if (form["disclosure"].clicked())
				System::openURL("https://epicinium.nl/privacy.php");
		}

		if (Input::get()->wasKeyPressed(SDL_SCANCODE_TAB))
		{
			static constexpr stringref tab[] = {
				"username",
				"email",
				"password",
				"password2",
			};
			for (size_t i = 0; i < array_size(tab); i++)
			{
				if (form[tab[i]]["input"].powered())
				{
					size_t j;
					if (Input::get()->isKeyHeld(SDL_SCANCODE_SHIFT))
					{
						j = (i + array_size(tab) - 1) % array_size(tab);
					}
					else
					{
						j = (i + 1) % array_size(tab);
					}
					form[tab[i]]["input"].depower();
					form[tab[j]]["input"].power();
					break;
				}
			}
		}

		form["username"]["error"].showIf(
			!form["username"]["input"].text().empty()
			&& !isValidUsername(form["username"]["input"].text()));

		form["email"]["error"].showIf(
			!form["email"]["input"].text().empty()
			&& !isValidEmail(form["email"]["input"].text()));

		form["password"]["error"].showIf(
			!form["password"]["input"].text().empty()
			&& !isValidPassword(form["password"]["input"].text()));

		form["password2"]["error"].showIf(
			!form["password2"]["input"].text().empty()
			&& form["password2"]["input"].text()
				!= form["password"]["input"].text());

		if (form["mailinglist"].clicked())
		{
			if (form["mailinglist"]["options"].getTag() != "Empty")
			{
				form["mailinglist"]["options"].setTag("Empty");
			}
			else
			{
				form["mailinglist"]["options"].setTag("Checked");
			}
		}

		form["buttons"]["confirm"].enableIf(
			_loginStatus != LoginStatus::REGISTERING
			&& !form["username"]["input"].text().empty()
			&& !form["username"]["error"].shown()
			&& !form["email"]["input"].text().empty()
			&& !form["email"]["error"].shown()
			&& !form["password"]["input"].text().empty()
			&& !form["password"]["error"].shown()
			&& !form["password2"]["input"].text().empty()
			&& !form["password2"]["error"].shown());

		if (form["buttons"]["confirm"].clicked())
		{
			_client.registration(form["username"]["input"].text(),
				form["email"]["input"].text(),
				form["password"]["input"].text());
			form["error"].kill();
			form.settle();
		}

		form["buttons"]["cancel"].enableIf(
			_loginStatus != LoginStatus::REGISTERING);

		if (form["buttons"]["cancel"].clicked())
		{
			_toForm = "buttons";
		}
	}

	{
		InterfaceElement& forms = getForms();
		InterfaceElement& form = forms["storageissueform"];
		if (form["buttons"]["ok"].clicked())
		{
			_toForm = "buttons";
		}
	}

	if (Input::get()->wasKeyPressed(SDL_SCANCODE_M)
		&& Input::get()->isKeyHeld(SDL_SCANCODE_ALT))
	{
		_toForm = "steammergeform";
		InterfaceElement& forms = getForms();
		InterfaceElement& form = forms["steammergeform"];
		form["email"]["input"].power();
		form.fixWidth();
		form.settle();
	}

	if (!_toForm.empty())
	{
		InterfaceElement& forms = getForms();
		forms.setTag(_toForm);
		_toForm = "";
	}

	{
		InterfaceElement& logoutbutton = getLogoutButton();
		logoutbutton.bearIf(
			_client.accessedPortal()
			&& !_client.isSteamEnabled()
			&& _connectionStatus == ConnectionStatus::CONNECTED
			&& _loginStatus == LoginStatus::LOGGEDIN);
		if (logoutbutton.clicked())
		{
			_client.logout(true);
		}
	}

	{
		InterfaceElement& loginbutton = getLoginButton();
		InterfaceElement& forms = getForms();
		loginbutton.bearIf(
			_client.accessedPortal()
			&& !_client.isSteamEnabled());
		loginbutton.enableIf(
			forms.getTag() != "loginform"
			&& _connectionStatus == ConnectionStatus::CONNECTED
			&& _loginStatus != LoginStatus::DISABLED
			&& _loginStatus != LoginStatus::LOGGINGIN
			&& _loginStatus != LoginStatus::REGISTERING
			&& _loginStatus != LoginStatus::RESETTING_PW
			&& _loginStatus != LoginStatus::ACTIVATING_KEY
			&& _loginStatus != LoginStatus::LOGGEDIN);
		if (loginbutton.clicked())
		{
			_toForm = "loginform";
			forms["loginform"]["email"]["input"].power();
			forms["loginform"]["password"]["input"].depower();
			forms["loginform"]["token"].setText("");
			forms["loginform"]["error"].kill();
			forms["loginform"]["info"].kill();
			forms["loginform"].fixWidth();
			forms["loginform"].settle();
		}
	}

	{
		InterfaceElement& registerbutton = getRegisterButton();
		InterfaceElement& forms = getForms();
		registerbutton.bearIf(
			_client.accessedPortal()
			&& !_client.isSteamEnabled());
		registerbutton.enableIf(
			forms.getTag() != "registrationform"
			&& _connectionStatus == ConnectionStatus::CONNECTED
			&& _loginStatus != LoginStatus::DISABLED
			&& _loginStatus != LoginStatus::LOGGINGIN
			&& _loginStatus != LoginStatus::REGISTERING
			&& _loginStatus != LoginStatus::RESETTING_PW
			&& _loginStatus != LoginStatus::ACTIVATING_KEY
			&& _loginStatus != LoginStatus::LOGGEDIN);
		if (registerbutton.clicked())
		{
			_toForm = "registrationform";
			forms["registrationform"]["username"]["input"].power();
			forms["registrationform"]["email"]["input"].depower();
			forms["registrationform"]["password"]["input"].depower();
			forms["registrationform"]["password2"]["input"].depower();
			forms["registrationform"]["error"].kill();
			forms["registrationform"].fixWidth();
			forms["registrationform"].settle();
		}
	}

	{
		InterfaceElement& settingsbutton = getSettingsButton();
		if (settingsbutton.clicked())
		{
			kill();
			_submenus[SETTINGS]->open();
			return;
		}
	}

	{
		InterfaceElement& creditsbutton = getCreditsButton();
		if (creditsbutton.clicked())
		{
			kill();
			_submenus[CREDITS]->open();
		}
	}

	InterfaceElement& connection = getConnectionNub();
	connection["icon"].enableIf(
		_connectionStatus != ConnectionStatus::ACCESSING_PORTAL);

	if (connection["icon"].clicked())
	{
		switch (_connectionStatus)
		{
			case ConnectionStatus::NONE:
			{
				_client.connect();
			}
			break;
			case ConnectionStatus::ACCESSING_PORTAL:
			{
				// What has been done cannot be undone.
			}
			break;
			case ConnectionStatus::CONNECTING:
			case ConnectionStatus::RESETTING:
			{
				_client.disconnect();
			}
			break;
			case ConnectionStatus::CONNECTED:
			{
				_client.disconnect();
			}
			break;
			case ConnectionStatus::PATCH_AVAILABLE:
			{
				_client.startPatch();
			}
			break;
			case ConnectionStatus::PATCH_DOWNLOADING:
			{
				_client.disconnect();
			}
			break;
			case ConnectionStatus::PATCH_PRIMED:
			{
				_owner.quit(ExitCode::APPLY_PATCH_AND_RESTART);
			}
			break;
			case ConnectionStatus::PATCH_FAILED:
			{
				_client.disconnect();
			}
			break;
		}
	}

	if (hasLoginNub())
	{
		InterfaceElement& login = getLoginNub();
		login["icon"].enableIf(
			_client.accessedPortal()
			&& _connectionStatus == ConnectionStatus::CONNECTED
			&& _loginStatus != LoginStatus::DISABLED
			&& _loginStatus != LoginStatus::ACTIVATING_KEY
			&& _loginStatus != LoginStatus::REGISTERING
			&& _loginStatus != LoginStatus::RESETTING_PW);

		if (login["icon"].clicked())
		{
			switch (_loginStatus)
			{
				case LoginStatus::DISABLED:
				{
					LOGE << "The button should be disabled";
				}
				break;
				case LoginStatus::NONE:
				case LoginStatus::REGISTERED:
				case LoginStatus::RESET_PW:
				{
					_toForm = "loginform";
					InterfaceElement& forms = getForms();
					forms["loginform"]["email"]["input"].power();
					forms["loginform"]["password"]["input"].depower();
					forms["loginform"]["error"].kill();
					forms["loginform"]["info"].kill();
					forms["loginform"].fixWidth();
					forms["loginform"].settle();
				}
				break;
				case LoginStatus::LOGGINGIN:
				{
					_client.logout(true);
				}
				break;
				case LoginStatus::LOGGEDIN:
				{
					_client.logout(true);
				}
				break;
				case LoginStatus::REGISTERING:
				case LoginStatus::RESETTING_PW:
				case LoginStatus::ACTIVATING_KEY:
				{
					// What has been done cannot be undone.
				}
				break;
			}
		}
	}

	if (hasDiscordNub())
	{
		InterfaceElement& discord = getDiscordNub();
		if (discord["icon"].hovered() && discord["icon"].getTag() == "Empty")
		{
			linkHovered = true;
			if (discord["icon"].clicked())
			{
				System::openURL("https://discord.gg/vQhTURC");
			}
		}
#if DISCORD_GUEST_ENABLED
		else if (discord["icon"].getTag() == "Checked"
			&& !_settings.discord.value())
		{
			discordDeactivated();
		}
#endif
	}

	{
		InterfaceElement& enterkeybutton = getEnterKeyButton();
		enterkeybutton.bearIf(
			_client.accessedPortal()
			&& !_client.isSteamEnabled());
		enterkeybutton.enableIf(
			_connectionStatus == ConnectionStatus::CONNECTED
			&& _loginStatus == LoginStatus::LOGGEDIN);

		if (enterkeybutton.clicked())
		{
			_toForm = "keyform";
			InterfaceElement& forms = getForms();
			forms["keyform"]["error"].kill();
			forms["keyform"]["token"]["input"].power();
			forms["keyform"].fixWidth();
			forms["keyform"].settle();
		}
	}

	{
		InterfaceElement& feedbackbutton = getFeedbackButton();
		if (feedbackbutton.clicked())
		{
			_toForm = "feedbackform";
			InterfaceElement& forms = getForms();
			InterfaceElement& form = forms["feedbackform"];
			auto& content = form["row"]["type"]["content"];
			form.fixWidth();
			if (content.getTag() == "wish")
			{
				form["input"].prefillText(
					// Not translated because Stomts are in English.
					"would ");
			}
			else if (content.getTag() == "like")
			{
				form["input"].prefillText(
					// Not translated because Stomts are in English.
					"because ");
			}
			else // if (content.getTag() == "bug")
			{
				form["input"].prefillText("");
			}
			form.settle();
			form["input"].power();
		}
	}

	{
		InterfaceElement& forms = getForms();
		InterfaceElement& form = forms["feedbackform"];
		auto& type = form["row"]["type"];
		auto& content = type["content"];
		auto& dropdown = type["dropdown"];

		if (dropdown.clicked())
		{
			for (size_t i = 0; i < content.size(); i++)
			{
				std::string name = content.name(i);
				if (dropdown.contains(name) && dropdown[name].clicked()
					&& content.getTag() != name)
				{
					form["error"].kill();
					form["info"].kill();
					form.settle();
					if (name == "wish")
					{
						form["input"].prefillText(
							// Not translated because Stomts are in English.
							"would ");
						form["row"]["info"]["texts"].setText(
							_("Please finish the sentence."));
						form["sendlogs"]["options"].setTag("Empty");
						form["sendlogs"].enable();
					}
					else if (name == "like")
					{
						form["input"].prefillText(
							// Not translated because Stomts are in English.
							"because ");
						form["row"]["info"]["texts"].setText(
							_("Please finish the sentence."));
						form["sendlogs"]["options"].setTag("Empty");
						form["sendlogs"].disable();
					}
					else // if (name == "bug")
					{
						form["input"].prefillText("");
						form["row"]["info"]["texts"].setText(
							_("Please describe what happened."));
						form["sendlogs"]["options"].setTag("Checked");
						form["sendlogs"].enable();
					}
					content.setTag(name);
				}
			}
		}

		if (form["sendlogs"].clicked())
		{
			if (form["sendlogs"]["options"].getTag() != "Empty")
			{
				form["sendlogs"]["options"].setTag("Empty");
			}
			else
			{
				form["sendlogs"]["options"].setTag("Checked");
			}
		}

		form["buttons"]["send"].enableIf(form["input"].text().length() > 10);

		if (form["buttons"]["send"].clicked())
		{
			form["error"].kill();
			form["info"].kill();
			form.settle();
			bool sendlogs = (form["sendlogs"]["options"].getTag() != "Empty");
			if (content.getTag() == "wish")
			{
				_client.feedback(false, form["input"].text(), sendlogs);
				form["input"].prefillText(
					// Not translated because Stomts are in English.
					"would ");
			}
			else if (content.getTag() == "like")
			{
				_client.feedback(true, form["input"].text(), sendlogs);
				form["input"].prefillText(
					// Not translated because Stomts are in English.
					"because ");
			}
			else // if (content.getTag() == "bug")
			{
				_client.bugReport(form["input"].text(), sendlogs);
				form["input"].prefillText("");
			}
		}

		if (form["buttons"]["cancel"].clicked())
		{
			form["error"].kill();
			form["info"].kill();
			_toForm = "buttons";
		}

		if (form["info"].hovered() && !form["info"]["url"].text().empty())
		{
			linkHovered = true;
			if (form["info"].clicked())
			{
				System::openURL(form["info"]["url"].text());
			}
		}
	}

	if (_layout.alive())
	{
		if (linkHovered && !_linkWasHovered)
		{
			_linkWasHovered = true;
			static SDL_Cursor* cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);
			SDL_SetCursor(cursor);
		}
		else if (!linkHovered && _linkWasHovered)
		{
			_linkWasHovered = false;
			static SDL_Cursor* cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
			SDL_SetCursor(cursor);
		}
	}
}

void MainMenu::onOpen()
{
	_client.registerHandler(this);

	{
		InterfaceElement& error = getCenteredError();
		if (error.shown())
		{
			error.hide();

			InterfaceElement& logo = getLogo();
			logo.show();

			InterfaceElement& pod = getPlayOrDownloading();
			if (pod.getTag() != "mp")
			{
				pod.setTag("mp");
			}
		}
	}

	if (_client.sessionStatus() != ResponseStatus::SUCCESS)
	{
		sessionFailed(_client.sessionStatus());
	}

	_client.hotClear();
	if (hasDiscordNub())
	{
		InterfaceElement& discord = getDiscordNub();
		if (discord["icon"].getTag() == "Checked")
		{
			if (!_settings.discord.value())
			{
				discordDeactivated();
			}
		}
		else _client.checkDiscordReady();
	}

	evaluatePlayable();
}

void MainMenu::onKill()
{
	_client.deregisterHandler(this);
}

void MainMenu::onHide()
{
	Mixer::get()->stop();
}

void MainMenu::debugHandler() const
{
	LOGD << ((void*) this);
}

void MainMenu::message(const std::string& message)
{
	InterfaceElement& connection = getConnectionNub();
	connection["tooltip"].setText(message);
	connection.settle();

	InterfaceElement& play = getPlay();
	play["tooltip"].setText(message);
	play["tooltip"].settleWidth();
	play["tooltip"].show();

	InterfaceElement& pod = getPlayOrDownloading();
	pod["downloading"]["texts"]["2"].setText(message);
}

void MainMenu::displayStamp(const std::string& image,
		const std::string& tooltip, const std::string& url)
{
	auto& stamp = getStamp();
	stamp["picture"].setPicture(image);
	_owner.getPicture(image);

	stamp["tooltip"].setText(tooltip);
	if (tooltip.empty()) stamp["tooltip"].hide();
	else
	{
		stamp["tooltip"].settle();
		stamp["tooltip"].show();
	}
	stamp["url"].setText(url);
	stamp.settle();
	stamp.show();
}

void MainMenu::evaluatePlayable()
{
	getSettingsButton().enableIf(
		_connectionStatus != ConnectionStatus::PATCH_DOWNLOADING);
	getCreditsButton().enableIf(
		_connectionStatus != ConnectionStatus::PATCH_DOWNLOADING);

	if (_connectionStatus == ConnectionStatus::PATCH_DOWNLOADING)
	{
		InterfaceElement& pod = getPlayOrDownloading();
		if (pod.getTag() != "downloading")
		{
			pod.setTag("downloading");
		}
	}
	else
	{
		InterfaceElement& pod = getPlayOrDownloading();
		if (pod.getTag() != "mp")
		{
			pod.setTag("mp");
		}
	}

	InterfaceElement& play = getPlay();

	switch (_connectionStatus)
	{
		case ConnectionStatus::NONE:
		case ConnectionStatus::ACCESSING_PORTAL:
		case ConnectionStatus::CONNECTING:
		case ConnectionStatus::RESETTING:
		{
			play["button"].setText(
				_("PLAY"));
			play["button"].disable();
			play["tooltip"].setText(
				_("You need to be connected to the server to play!"));
			play["tooltip"].settleWidth();
			play["tooltip"].show();
		}
		break;
		case ConnectionStatus::PATCH_AVAILABLE:
		{
			switch (_settings.patchmode.value(PatchMode::NONE))
			{
				case PatchMode::SERVER:
				{
					// The tooltip text is already set
					play["button"].setText(
						_("UPDATE"));
					play["button"].enable();
				}
				break;
				case PatchMode::GAMEJOLT:
				{
					play["button"].setText(
						_("PLAY"));
					play["button"].disable();
					play["tooltip"].setText(
						_(""
						"A new version is available!"
						" Please update Epicinium via your GameJolt client."
						""));
					play["tooltip"].settleWidth();
					play["tooltip"].show();
				}
				break;
				case PatchMode::ITCHIO:
				{
					play["button"].setText(
						_("PLAY"));
					play["button"].disable();
					play["tooltip"].setText(
						_(""
						"A new version is available!"
						" Please update Epicinium via your itch desktop app."
						""));
					play["tooltip"].settleWidth();
					play["tooltip"].show();
				}
				break;
				case PatchMode::STEAM:
				{
					play["button"].setText(
						_("PLAY"));
					play["button"].disable();
					play["tooltip"].setText(
						_(""
						"A new version is available!"
						" Please update Epicinium via Steam."
						""));
					play["tooltip"].settleWidth();
					play["tooltip"].show();
				}
				break;
				case PatchMode::SERVER_BUT_DISABLED_DUE_TO_STORAGE_ISSUES:
				case PatchMode::NONE:
				{
					play["button"].setText(
						_("PLAY"));
					play["button"].disable();
					play["tooltip"].setText(
						_(""
						"A new version is available!"
						" Please download the latest version of Epicinium."
						""));
					play["tooltip"].settleWidth();
					play["tooltip"].show();
				}
				break;
			}
		}
		break;
		case ConnectionStatus::PATCH_DOWNLOADING:
		{
			play["button"].setText(
				_("UPDATE"));
			play["button"].disable();
		}
		break;
		case ConnectionStatus::PATCH_FAILED:
		{
			play["button"].disable();
		}
		break;
		case ConnectionStatus::PATCH_PRIMED:
		{
			play["button"].setText(
				_("RESTART"));
			play["button"].enable();
		}
		break;

		case ConnectionStatus::CONNECTED:
		{
			if (_client.isSteamEnabled())
			{
				switch (_steamStatus)
				{
					case SteamStatus::NONE:
					{
						play["button"].setText(
							_("PLAY"));
						play["button"].disable();
						play["tooltip"].setText(
							_("Failed to connect to Steam."));
						play["tooltip"].settleWidth();
						play["tooltip"].show();
					}
					break;

					case SteamStatus::CONNECTING:
					{
						play["button"].setText(
							_("PLAY"));
						play["button"].disable();
						play["tooltip"].setText(
							_("Connecting to Steam..."));
						play["tooltip"].settleWidth();
						play["tooltip"].show();
					}
					break;

					case SteamStatus::CONNECTED:
					{
						play["button"].setText(
							_("PLAY"));
						play["button"].enable();
						play["tooltip"].hide();
					}
					break;
				}

				return;
			}

			switch (_loginStatus)
			{
				case LoginStatus::NONE:
				{
					play["button"].setText(
						_("PLAY"));
					play["button"].disable();
					play["tooltip"].setText(
						_("Please log in to play!"));
#if DISCORD_GUEST_ENABLED
					if (_settings.discord.value())
					{
						play["button"].enable();
						play["tooltip"].setText(
							_(""
							"Log in to play"
							" or play as a guest using your Discord username."
							""));
					}
#endif
					play["tooltip"].settleWidth();
					play["tooltip"].show();
				}
				break;

				case LoginStatus::LOGGINGIN:
				case LoginStatus::REGISTERING:
				case LoginStatus::REGISTERED:
				case LoginStatus::RESETTING_PW:
				case LoginStatus::ACTIVATING_KEY:
				case LoginStatus::RESET_PW:
				{
					play["button"].setText(
						_("PLAY"));
					play["button"].disable();
					play["tooltip"].setText(
						_("Please log in to play!"));
					play["tooltip"].settleWidth();
					play["tooltip"].show();
				}
				break;

				case LoginStatus::DISABLED:
				case LoginStatus::LOGGEDIN:
				{
					play["button"].setText(
						_("PLAY"));
					play["button"].enable();
					play["tooltip"].hide();
				}
				break;
			}
		}
		break;
	}
}

void MainMenu::connecting()
{
	InterfaceElement& connection = getConnectionNub();
	connection["icon"].setTag("Spinning");

	_connectionStatus = ConnectionStatus::CONNECTING;
}

void MainMenu::connected()
{
	InterfaceElement& connection = getConnectionNub();
	connection["icon"].setTag("Checked");

	_connectionStatus = ConnectionStatus::CONNECTED;

	evaluatePlayable();
}

void MainMenu::disconnected()
{
	InterfaceElement& connection = getConnectionNub();
	connection["icon"].setTag("Empty");

	InterfaceElement& error = getCenteredError();
	if (error.shown())
	{
		error.hide();

		InterfaceElement& logo = getLogo();
		logo.show();

		InterfaceElement& pod = getPlayOrDownloading();
		if (pod.getTag() != "mp")
		{
			pod.setTag("mp");
		}
	}

	_connectionStatus = ConnectionStatus::NONE;

	evaluatePlayable();
}

void MainMenu::systemFailure()
{
	InterfaceElement& connection = getConnectionNub();
	connection["icon"].setTag("Crossed");

	if (hasLoginNub())
	{
		InterfaceElement& login = getLoginNub();
		login["icon"].setTag("Crossed");
	}

	_connectionStatus = ConnectionStatus::NONE;
	if (_loginStatus != LoginStatus::DISABLED)
	{
		_loginStatus = LoginStatus::NONE;
	}

	evaluatePlayable();
}

void MainMenu::serverOffline()
{
	InterfaceElement& connection = getConnectionNub();
	connection["icon"].setTag("Crossed");

	_connectionStatus = ConnectionStatus::NONE;

	evaluatePlayable();
}

void MainMenu::serverShutdown()
{
	InterfaceElement& connection = getConnectionNub();
	connection["icon"].setTag("Empty");

	_connectionStatus = ConnectionStatus::NONE;

	evaluatePlayable();
}

void MainMenu::serverClosing()
{
	InterfaceElement& connection = getConnectionNub();
	connection["icon"].setTag("Empty");

	_connectionStatus = ConnectionStatus::NONE;

	evaluatePlayable();
}

void MainMenu::versionMismatch(const Version& version)
{
	InterfaceElement& connection = getConnectionNub();
	connection["icon"].setTag("Screaming");
	std::string versionmask = std::to_string(version.major) + "."
		+ std::to_string(version.minor) + ".x";
	message(::format(
		// TRANSLATORS: The argument is a SemVer number, e.g. "0.35.0".
		_("Version mismatch. Please update to version %s."),
		versionmask.c_str()));

	_connectionStatus = ConnectionStatus::NONE;

	evaluatePlayable();
}

void MainMenu::patchAvailable(const Version& version)
{
	InterfaceElement& connection = getConnectionNub();
	if (connection["icon"].getTag() != "Screaming")
	{
		connection["icon"].setTag("Exclamation");
	}
	message(::format(
		// TRANSLATORS: The argument is a SemVer number, e.g. "0.35.0".
		_("A patch is available! Click to download version %s."),
		version.toString().c_str()));

	_connectionStatus = ConnectionStatus::PATCH_AVAILABLE;

	evaluatePlayable();
}

void MainMenu::patchDownloading()
{
	InterfaceElement& connection = getConnectionNub();
	connection["icon"].setTag("Downloading");
	message(
		_("Downloading..."));

	_connectionStatus = ConnectionStatus::PATCH_DOWNLOADING;

	evaluatePlayable();
}

void MainMenu::patchPrimed(const Version& version)
{
	InterfaceElement& connection = getConnectionNub();
	connection["icon"].setTag("Screaming");
	message(::format(
		// TRANSLATORS: The argument is a SemVer number, e.g. "0.35.0".
		_("Patch downloaded. Click to install version %s."),
		version.toString().c_str()));

	_connectionStatus = ConnectionStatus::PATCH_PRIMED;

	evaluatePlayable();
}

void MainMenu::patchFailed()
{
	InterfaceElement& connection = getConnectionNub();
	connection["icon"].setTag("Crossed");
	message(
		_("Downloading patch failed."));

	_connectionStatus = ConnectionStatus::PATCH_FAILED;

	evaluatePlayable();
}

void MainMenu::patchContinuing(const Version& version)
{
	if (_connectionStatus != ConnectionStatus::PATCH_AVAILABLE)
	{
		LOGW << "Auto patch disabled because the connection status changed";
		return;
	}

	InterfaceElement& connection = getConnectionNub();
	connection["icon"].setTag("Downloading");
	message(::format(
		// TRANSLATORS: The argument is a SemVer number, e.g. "0.35.0".
		_("Continuing download of version %s."),
		version.toString().c_str()));

	_connectionStatus = ConnectionStatus::PATCH_DOWNLOADING;

	evaluatePlayable();

	// We don't want music if we are restarting over and over again.
	Mixer::get()->stop();
}

void MainMenu::patchContinued(const Version&)
{
	if (getForms().getTag() == "buttons")
	{
		_owner.quit(ExitCode::APPLY_PATCH_AND_RESTART);
	}
}

void MainMenu::downloading(const std::string& filename, float percentage)
{
	// Concatenating the percentage at the end is not ideal but we're not
	// going to bother formatting the percentage properly in other langauges.
	std::stringstream strm;
	if (filename.empty())
	{
		strm << _("Downloading additional files...");
	}
	else
	{
		strm << ::format(
			// TRANSLATORS: The argument is a filename.
			_("Downloading %s..."),
			filename.c_str());
	}
	if (percentage >= 0)
	{
		strm << " (" << std::setfill('0') << std::setw(5)
			<< std::fixed << std::setprecision(2) << percentage << "%"")";
	}
	InterfaceElement& pod = getPlayOrDownloading();
	pod["downloading"]["texts"]["2"].setText(
		strm.str());
}

void MainMenu::resetting()
{
	InterfaceElement& connection = getConnectionNub();
	connection["icon"].setTag("Crossed");

	InterfaceElement& error = getCenteredError();
	error.setTag("resetting");
	error.show();

	InterfaceElement& logo = getLogo();
	logo.hide();

	InterfaceElement& pod = getPlayOrDownloading();
	if (pod.getTag() != "mp")
	{
		pod.setTag("mp");
	}

	_connectionStatus = ConnectionStatus::RESETTING;

	evaluatePlayable();
}

void MainMenu::accessingPortal()
{
	InterfaceElement& connection = getConnectionNub();
	connection["icon"].setTag("Spinning");
	connection["tooltip"].setText(
		_("Connecting to official servers..."));
	connection.settle();

	_connectionStatus = ConnectionStatus::ACCESSING_PORTAL;

	if (_loginStatus == LoginStatus::DISABLED)
	{
		_loginStatus = LoginStatus::NONE;
	}
}

void MainMenu::accessedPortal()
{
	InterfaceElement& connection = getConnectionNub();
	connection["icon"].setTag("Empty");
	connection["tooltip"].setText(
		_("Connected to official servers."));
	connection.settle();

	_connectionStatus = ConnectionStatus::NONE;
}

void MainMenu::accessingPortalFailed(const PortalStatus& status)
{
	std::string message;
	switch (status)
	{
		case PortalStatus::MAINTENANCE:
		{
			message = _(""
				"The server has been shut down for maintenance."
				" We'll be back soon!"
				"");
		}
		break;

		case PortalStatus::BAD_REQUEST:
		{
			// TODO check for patches
			message = _("An update is required to connect to the server.");
		}
		break;

		case PortalStatus::SUCCESS:
		case PortalStatus::RESPONSE_MALFORMED:
		case PortalStatus::CONNECTION_FAILED:
		{
			DEBUG_ASSERT(status != PortalStatus::SUCCESS);
			message = _("Failed to connect to official servers.");
		}
		break;
	}

	InterfaceElement& connection = getConnectionNub();
	connection["icon"].setTag("Crossed");
	connection["tooltip"].setText(message);
	connection.settle();

	_connectionStatus = ConnectionStatus::NONE;
}

void MainMenu::loggingIn()
{
	InterfaceElement& login = getLoginNub();
	login["icon"].setTag("Spinning");
	login["tooltip"].setText(
		_("Attempting to log in..."));
	login.settle();

	_loginStatus = LoginStatus::LOGGINGIN;
}

void MainMenu::loggedIn()
{
	InterfaceElement& login = getLoginNub();
	login["icon"].setTag("Checked");
	login["tooltip"].setText(
		_("Login successful."));
	login.settle();

	InterfaceElement& forms = getForms();
	const std::string& token = forms["loginform"]["token"].text();
	if (!token.empty())
	{
		_toForm = "keyform";
		forms["keyform"]["error"].kill();
		forms["keyform"]["token"]["input"].power();
		forms["keyform"]["token"].fixWidth();
		if (token != "?")
		{
			forms["keyform"]["token"]["input"].setText(token);
		}
		forms["keyform"]["token"].settle();
		forms["keyform"].fixWidth();
		forms["keyform"].settle();

		forms["loginform"]["token"].setText("");
	}
	else _toForm = "buttons";

	_loginStatus = LoginStatus::LOGGEDIN;

	evaluatePlayable();
}

void MainMenu::loggedOut()
{
	InterfaceElement& login = getLoginNub();
	login["icon"].setTag("Empty");
	login["tooltip"].setText(
		_("Not logged in."));
	login.settle();

	_loginStatus = LoginStatus::NONE;

	evaluatePlayable();
}

void MainMenu::loginFailed(const ResponseStatus& responsestatus)
{
	std::string errortext;
	switch (responsestatus)
	{
		case ResponseStatus::CONNECTION_FAILED:
		{
			errortext = _(""
				"Could not connect to the login server."
				"");
		}
		break;

		case ResponseStatus::CREDS_INVALID:
		{
			errortext = _(""
				"Unknown email address or incorrect password."
				"");
		}
		break;

		case ResponseStatus::ACCOUNT_LOCKED:
		{
			errortext = _(""
				"Your account has been locked"
				" due to too many failed login attempts."
				" Please try again in 2 hours."
				"");
		}
		break;

		case ResponseStatus::ACCOUNT_DISABLED:
		{
			errortext = _(""
				"Your account has been disabled."
				" If you have any questions about this,"
				" you can reach us at support@epicinium.nl"
				"");
		}
		break;

		case ResponseStatus::SUCCESS:
		case ResponseStatus::USERNAME_TAKEN:
		case ResponseStatus::EMAIL_TAKEN:
		case ResponseStatus::KEY_TAKEN:
		case ResponseStatus::IP_BLOCKED:
		case ResponseStatus::KEY_REQUIRED:
		case ResponseStatus::EMAIL_UNVERIFIED:
		case ResponseStatus::USERNAME_REQUIRED_NOUSER:
		case ResponseStatus::USERNAME_REQUIRED_INVALID:
		case ResponseStatus::USERNAME_REQUIRED_TAKEN:
		case ResponseStatus::METHOD_INVALID:
		case ResponseStatus::REQUEST_MALFORMED:
		case ResponseStatus::RESPONSE_MALFORMED:
		case ResponseStatus::DATABASE_ERROR:
		case ResponseStatus::UNKNOWN_ERROR:
		{
			errortext = _(""
				"An unexpected error has occurred while logging in."
				"");
		}
		break;
	}

	InterfaceElement& login = getLoginNub();
	login["icon"].setTag("Crossed");
	login["tooltip"].setText(
		errortext);
	login.settle();

	InterfaceElement& forms = getForms();
	forms["loginform"]["error"]["texts"].setText(errortext);
	forms["loginform"]["error"].bear();
	forms["loginform"]["error"].settle();
	forms["loginform"]["info"].kill();
	forms["loginform"].fixWidth();
	forms["loginform"].settle();

	_loginStatus = LoginStatus::NONE;

	evaluatePlayable();
}

void MainMenu::registering()
{
	InterfaceElement& login = getLoginNub();
	login["icon"].setTag("Spinning");
	login["tooltip"].setText(
		_("Attempting to register..."));
	login.settle();

	_loginStatus = LoginStatus::REGISTERING;
}

void MainMenu::registered()
{
	InterfaceElement& login = getLoginNub();
	login["icon"].setTag("Empty");
	login["tooltip"].setText(
		_("Not logged in."));
	login.settle();

	_loginStatus = LoginStatus::REGISTERED;

	InterfaceElement& forms = getForms();
	if (forms["registrationform"].alive())
	{
		InterfaceElement& form = forms["registrationform"];
		const std::string& email = form["email"]["input"].text();
		const std::string& token = form["token"].text();

		if (form["mailinglist"]["options"].getTag() == "Checked")
		{
			EmailPreference pref;
			pref.mailinglist = EmailPreference::YES;
			_client.updateEmailPref(email, pref);
		}

		_toForm = "loginform";
		forms["loginform"]["email"]["input"].setText(email);
		forms["loginform"]["email"]["input"].depower();
		forms["loginform"]["token"].setText(token);
		forms["loginform"]["password"]["input"].power();
		forms["loginform"]["error"].kill();
		forms["loginform"]["info"]["texts"].setText(
			_(""
			"Successfully registered."
			" Please log in."
			""));
		forms["loginform"]["info"].bear();
		forms["loginform"]["info"].settle();
		forms["loginform"].fixWidth();
		forms["loginform"].settle();
	}
	else
	{
		LOGW << "Registration form closed while registering,"
			<< " email preferences discarded.";
		DEBUG_ASSERT(false);
	}

	evaluatePlayable();
}

void MainMenu::registrationFailed(const ResponseStatus& responsestatus)
{
	std::string errortext;
	switch (responsestatus)
	{
		case ResponseStatus::CONNECTION_FAILED:
		{
			errortext = _(""
				"Could not connect to the login server."
				"");
		}
		break;

		case ResponseStatus::CREDS_INVALID:
		{
			errortext = _(""
				"This is not a valid email address."
				"");
		}
		break;

		case ResponseStatus::USERNAME_TAKEN:
		{
			errortext = _(""
				"This username is already taken."
				"");
		}
		break;

		case ResponseStatus::EMAIL_TAKEN:
		{
			errortext = _(""
				"There is already an account registered for this email address."
				"");
		}
		break;

		case ResponseStatus::SUCCESS:
		case ResponseStatus::ACCOUNT_LOCKED:
		case ResponseStatus::ACCOUNT_DISABLED:
		case ResponseStatus::KEY_TAKEN:
		case ResponseStatus::IP_BLOCKED:
		case ResponseStatus::KEY_REQUIRED:
		case ResponseStatus::EMAIL_UNVERIFIED:
		case ResponseStatus::USERNAME_REQUIRED_NOUSER:
		case ResponseStatus::USERNAME_REQUIRED_INVALID:
		case ResponseStatus::USERNAME_REQUIRED_TAKEN:
		case ResponseStatus::METHOD_INVALID:
		case ResponseStatus::REQUEST_MALFORMED:
		case ResponseStatus::RESPONSE_MALFORMED:
		case ResponseStatus::DATABASE_ERROR:
		case ResponseStatus::UNKNOWN_ERROR:
		{
			errortext = _(""
				"An unexpected error has occurred while registering."
				"");
		}
		break;
	}

	InterfaceElement& login = getLoginNub();
	login["icon"].setTag("Crossed");
	login["tooltip"].setText(
		errortext);
	login.settle();

	InterfaceElement& forms = getForms();
	forms["registrationform"]["error"]["texts"].setText(errortext);
	forms["registrationform"]["error"].bear();
	forms["registrationform"]["error"].settle();
	forms["registrationform"].fixWidth();
	forms["registrationform"].settle();

	_loginStatus = LoginStatus::NONE;

	evaluatePlayable();
}

void MainMenu::sessionFailed(const ResponseStatus& responsestatus)
{
	std::string errortext;
	switch (responsestatus)
	{
		case ResponseStatus::CREDS_INVALID:
		{
			if (_client.isSteamEnabled())
			{
				errortext = _("Failed to authenticate using Steam.");
				_steamStatus = SteamStatus::NONE;
			}
			else
			{
				errortext = _(""
					"Your login session has expired."
					" Please log in again."
					"");
				_client.logout(true);
			}
		}
		break;
		case ResponseStatus::USERNAME_TAKEN:
		{
			errortext = _(""
				"You are already online and logged in"
				" with this account elsewhere."
				"");
			if (_client.isSteamEnabled())
			{
				_steamStatus = SteamStatus::NONE;
			}
			else
			{
				_client.logout(false);
			}
		}
		break;
		case ResponseStatus::ACCOUNT_LOCKED:
		{
			errortext = _(""
				"Your account has been locked"
				" due to too many failed login attempts."
				" Please try again in 2 hours."
				"");
			if (_client.isSteamEnabled())
			{
				_steamStatus = SteamStatus::NONE;
			}
			else
			{
				_client.logout(true);
			}
		}
		break;
		case ResponseStatus::ACCOUNT_DISABLED:
		{
			errortext = _(""
				"Your account has been disabled."
				" If you have any questions about this,"
				" You can reach us at support@epicinium.nl"
				"");
			if (_client.isSteamEnabled())
			{
				_steamStatus = SteamStatus::NONE;
			}
			else
			{
				_client.logout(true);
			}
		}
		break;
		case ResponseStatus::KEY_REQUIRED:
		{
			if (_client.isSteamEnabled())
			{
				errortext = _(""
					"You do not own this game on Steam."
					"");
				_steamStatus = SteamStatus::NONE;
			}
			else
			{
				errortext = _(""
					"A key is required to unlock access to the server."
					"");
				_toForm = "keyform";
				InterfaceElement& forms = getForms();
				forms["keyform"]["token"]["input"].power();
				forms["keyform"]["error"]["texts"].setText(errortext);
				forms["keyform"]["error"].bear();
				forms["keyform"]["error"].settle();
				forms["keyform"].fixWidth();
				forms["keyform"].settle();
			}
		}
		break;
		case ResponseStatus::USERNAME_REQUIRED_NOUSER:
		{
			if (_client.isSteamEnabled())
			{
				_toForm = "steamusernameform";
				InterfaceElement& forms = getForms();
				InterfaceElement& form = forms["steamusernameform"];
				form["error"]["texts"].setText(
					_(""
					"Please enter a username to use within Epicinium."
					""));
				form["username"]["input"].power();
				form.fixWidth();
				form.settle();
			}
			else
			{
				errortext = _(""
					"An unexpected error occurred while validating your"
					" login session. Please log in again."
					"");
				_client.logout(false);
			}
		}
		break;
		case ResponseStatus::USERNAME_REQUIRED_TAKEN:
		{
			if (_client.isSteamEnabled())
			{
				_toForm = "steamusernameform";
				InterfaceElement& forms = getForms();
				InterfaceElement& form = forms["steamusernameform"];
				form["error"]["texts"].setText(
					_(""
					"There already exists an Epicinium user with the same name."
					" Please enter a new username to use within Epicinium."
					""));
				form["username"]["input"].power();
				form.fixWidth();
				form.settle();
			}
			else
			{
				errortext = _(""
					"An unexpected error occurred while validating your"
					" login session. Please log in again."
					"");
				_client.logout(false);
			}
		}
		break;
		case ResponseStatus::SUCCESS:
		case ResponseStatus::EMAIL_TAKEN:
		case ResponseStatus::KEY_TAKEN:
		case ResponseStatus::IP_BLOCKED:
		case ResponseStatus::EMAIL_UNVERIFIED:
		case ResponseStatus::USERNAME_REQUIRED_INVALID:
		case ResponseStatus::DATABASE_ERROR:
		case ResponseStatus::METHOD_INVALID:
		case ResponseStatus::REQUEST_MALFORMED:
		case ResponseStatus::RESPONSE_MALFORMED:
		case ResponseStatus::CONNECTION_FAILED:
		case ResponseStatus::UNKNOWN_ERROR:
		{
			if (_client.isSteamEnabled())
			{
				errortext = _(""
					"An unexpected error occurred while authenticating"
					" using your Steam account."
					"");
				_steamStatus = SteamStatus::NONE;
			}
			else
			{
				errortext = _(""
					"An unexpected error occurred while validating your"
					" login session. Please log in again."
					"");
				_client.logout(false);
			}
		}
	}

	if (hasLoginNub() && !errortext.empty())
	{
		InterfaceElement& login = getLoginNub();
		login["icon"].setTag("Crossed");
		login["tooltip"].setText(
			errortext);
		login.settle();
	}
	else if (hasSteamNub() && !errortext.empty())
	{
		InterfaceElement& steam = getSteamNub();
		steam["icon"].setTag("Crossed");
		steam["tooltip"].setText(
			errortext);
		steam.settle();
	}

	if (_client.isSteamEnabled())
	{
		// Nothing to do.
	}
	else if (_loginStatus == LoginStatus::NONE)
	{
		_toForm = "loginform";
		InterfaceElement& forms = getForms();
		forms["loginform"]["email"]["input"].power();
		forms["loginform"]["password"]["input"].depower();
		forms["loginform"]["error"]["texts"].setText(errortext);
		forms["loginform"]["error"].bear();
		forms["loginform"]["error"].settle();
		forms["loginform"]["info"].kill();
		forms["loginform"].fixWidth();
		forms["loginform"].settle();
	}

	evaluatePlayable();
}

void MainMenu::activatingKey()
{
	InterfaceElement& login = getLoginNub();
	login["icon"].setTag("Spinning");
	login["tooltip"].setText(
		_("Attempting to activate key..."));
	login.settle();

	_loginStatus = LoginStatus::ACTIVATING_KEY;
}

void MainMenu::activatedKey()
{
	InterfaceElement& login = getLoginNub();
	login["icon"].setTag("Empty");
	login["tooltip"].setText(
		_("Logged in."));
	login.settle();

	_loginStatus = LoginStatus::LOGGEDIN;

	InterfaceElement& forms = getForms();
	if (forms["keyform"].alive())
	{
		_toForm = "unlockform";
	}
	else
	{
		LOGW << "keyform closed while activating key";
		DEBUG_ASSERT(false);
	}

	evaluatePlayable();
}

void MainMenu::activatingKeyFailed(const ResponseStatus& responsestatus)
{
	std::string errortext;
	switch (responsestatus)
	{
		case ResponseStatus::CREDS_INVALID:
		{
			errortext = _(""
				"The key is incorrect."
				"");
		}
		break;

		case ResponseStatus::KEY_TAKEN:
		{
			errortext = _(""
				"The key has already been activated."
				"");
		}
		break;

		case ResponseStatus::IP_BLOCKED:
		{
			errortext = _(""
				"Too many attempts. Please try again in 2 hours."
				"");
		}
		break;

		case ResponseStatus::SUCCESS:
		case ResponseStatus::ACCOUNT_LOCKED:
		case ResponseStatus::USERNAME_TAKEN:
		case ResponseStatus::EMAIL_TAKEN:
		case ResponseStatus::ACCOUNT_DISABLED:
		case ResponseStatus::KEY_REQUIRED:
		case ResponseStatus::EMAIL_UNVERIFIED:
		case ResponseStatus::USERNAME_REQUIRED_NOUSER:
		case ResponseStatus::USERNAME_REQUIRED_INVALID:
		case ResponseStatus::USERNAME_REQUIRED_TAKEN:
		case ResponseStatus::DATABASE_ERROR:
		case ResponseStatus::METHOD_INVALID:
		case ResponseStatus::REQUEST_MALFORMED:
		case ResponseStatus::RESPONSE_MALFORMED:
		case ResponseStatus::CONNECTION_FAILED:
		case ResponseStatus::UNKNOWN_ERROR:
		{
			errortext = _(""
				"An unexpected error has occurred"
				" while activating your key."
				" Please try again."
				"");
		}
		break;
	}

	InterfaceElement& login = getLoginNub();
	login["icon"].setTag("Crossed");
	login["tooltip"].setText(
		errortext);
	login.settle();

	InterfaceElement& forms = getForms();
	forms["keyform"]["error"]["texts"].setText(errortext);
	forms["keyform"]["error"].bear();
	forms["keyform"]["error"].settle();
	forms["keyform"].fixWidth();
	forms["keyform"].settle();

	_loginStatus = LoginStatus::LOGGEDIN;

	evaluatePlayable();
}

void MainMenu::resettingPassword()
{
	InterfaceElement& login = getLoginNub();
	login["icon"].setTag("Spinning");
	login["tooltip"].setText(
		_("Attempting to reset password..."));
	login.settle();

	_loginStatus = LoginStatus::RESETTING_PW;
}

void MainMenu::resetPassword()
{
	InterfaceElement& login = getLoginNub();
	login["icon"].setTag("Empty");
	login["tooltip"].setText(
		_("Not logged in."));
	login.settle();

	_loginStatus = LoginStatus::RESET_PW;

	InterfaceElement& forms = getForms();
	if (forms["resetpwform"].alive())
	{
		InterfaceElement& form = forms["resetpwform"];
		const std::string& email = form["email"].text();

		_toForm = "loginform";
		forms["loginform"]["email"]["input"].setText(email);
		forms["loginform"]["email"]["input"].depower();
		forms["loginform"]["password"]["input"].power();
		forms["loginform"]["error"].kill();
		forms["loginform"]["info"]["texts"].setText(
			_("Successfully reset password."));
		forms["loginform"]["info"].bear();
		forms["loginform"]["info"].settle();
		forms["loginform"].fixWidth();
		forms["loginform"].settle();
	}
	else
	{
		LOGW << "PW reset form closed while registering";
		DEBUG_ASSERT(false);
	}

	_loginStatus = LoginStatus::RESET_PW;

	evaluatePlayable();
}

void MainMenu::resetPasswordFailed(const ResponseStatus& responsestatus)
{
	std::string errortext;
	switch (responsestatus)
	{
		case ResponseStatus::CREDS_INVALID:
		{
			errortext = _(""
				"The token is incorrect."
				"");
		}
		break;

		case ResponseStatus::SUCCESS:
		case ResponseStatus::ACCOUNT_LOCKED:
		case ResponseStatus::USERNAME_TAKEN:
		case ResponseStatus::EMAIL_TAKEN:
		case ResponseStatus::ACCOUNT_DISABLED:
		case ResponseStatus::KEY_TAKEN:
		case ResponseStatus::IP_BLOCKED:
		case ResponseStatus::KEY_REQUIRED:
		case ResponseStatus::EMAIL_UNVERIFIED:
		case ResponseStatus::USERNAME_REQUIRED_NOUSER:
		case ResponseStatus::USERNAME_REQUIRED_INVALID:
		case ResponseStatus::USERNAME_REQUIRED_TAKEN:
		case ResponseStatus::DATABASE_ERROR:
		case ResponseStatus::METHOD_INVALID:
		case ResponseStatus::REQUEST_MALFORMED:
		case ResponseStatus::RESPONSE_MALFORMED:
		case ResponseStatus::CONNECTION_FAILED:
		case ResponseStatus::UNKNOWN_ERROR:
		{
			errortext = _(""
				"An unexpected error has occurred"
				" while resetting your password."
				" Please try again."
				"");
		}
		break;
	}

	InterfaceElement& login = getLoginNub();
	login["icon"].setTag("Crossed");
	login["tooltip"].setText(
		errortext);
	login.settle();

	InterfaceElement& forms = getForms();
	if (responsestatus == ResponseStatus::CREDS_INVALID)
	{
		forms["resetpwform"]["error"]["texts"].setText(errortext);
		forms["resetpwform"]["error"].bear();
		forms["resetpwform"]["error"].settle();
		forms["resetpwform"].fixWidth();
		forms["resetpwform"].settle();
	}
	else
	{
		_toForm = "loginform";
		forms["loginform"]["email"]["input"].power();
		forms["loginform"]["password"]["input"].depower();
		forms["loginform"]["error"]["texts"].setText(errortext);
		forms["loginform"]["error"].bear();
		forms["loginform"]["error"].settle();
		forms["loginform"]["info"].kill();
		forms["loginform"].fixWidth();
		forms["loginform"].settle();
	}

	_loginStatus = LoginStatus::NONE;

	evaluatePlayable();
}

void MainMenu::sendingFeedback()
{
	// TODO do something?
}

void MainMenu::sentFeedback(const std::string& link)
{
	InterfaceElement& forms = getForms();
	InterfaceElement& form = forms["feedbackform"];

	if (form.alive())
	{
		form["error"].kill();
		form["info"]["texts"].fixWidth();
		if (link.empty())
		{
			form["info"]["texts"].setText(
				_(""
				"Thanks for your feedback! "
				"We will get back to you soon."
				""));
			form["info"]["url"].setText(link);
		}
		else
		{
			form["info"]["texts"].setText(
				_(""
				"Thanks for your feedback! "
				"Click here to view your submission."
				""));
			form["info"]["url"].setText(link);
		}
		form["info"].bear();
		form["info"].settle();
		form.settle();
	}
}

void MainMenu::feedbackFailed(const ResponseStatus& responsestatus)
{
	std::string errortext;
	switch (responsestatus)
	{
		case ResponseStatus::CONNECTION_FAILED:
		case ResponseStatus::SUCCESS:
		case ResponseStatus::CREDS_INVALID:
		case ResponseStatus::ACCOUNT_LOCKED:
		case ResponseStatus::USERNAME_TAKEN:
		case ResponseStatus::EMAIL_TAKEN:
		case ResponseStatus::ACCOUNT_DISABLED:
		case ResponseStatus::KEY_TAKEN:
		case ResponseStatus::IP_BLOCKED:
		case ResponseStatus::KEY_REQUIRED:
		case ResponseStatus::EMAIL_UNVERIFIED:
		case ResponseStatus::USERNAME_REQUIRED_NOUSER:
		case ResponseStatus::USERNAME_REQUIRED_INVALID:
		case ResponseStatus::USERNAME_REQUIRED_TAKEN:
		case ResponseStatus::METHOD_INVALID:
		case ResponseStatus::REQUEST_MALFORMED:
		case ResponseStatus::RESPONSE_MALFORMED:
		case ResponseStatus::DATABASE_ERROR:
		case ResponseStatus::UNKNOWN_ERROR:
		{
			errortext = _(""
				"An unexpected error has occurred while sending feedback."
				"");
		}
		break;
	}
	InterfaceElement& forms = getForms();
	InterfaceElement& form = forms["feedbackform"];
	form["error"]["texts"].fixWidth();
	form["error"]["texts"].setText(errortext);
	form["error"].bear();
	form["error"].settle();
	form["info"].kill();
	form.settle();
}

void MainMenu::requestFulfilled(const std::string& filename)
{
	evaluatePlayable();

	std::string picturename = Locator::pictureName(filename);
	if (!picturename.empty())
	{
		_layout.checkPicture(picturename);
		return;
	}
}

void MainMenu::discordReady(const char* username, const char* discriminator,
	const std::string& picturename)
{
	_discordStatus = DiscordStatus::CONNECTED;

	InterfaceElement& discord = getDiscordNub();
	discord["icon"].setTag("Checked");
	discord["tooltip"].setText(::format(
		// TRANSLATORS: The first argument is a Discord username, the second
		// a Discord discriminator string, e.g. together "Alice#1234".
		_("Connected as %s#%s on Discord."),
		username, discriminator));
	discord.settle();

	InterfaceElement& forms = getForms();
	InterfaceElement& form = forms["discordloginform"];
	form["content"]["avatar"]["spinning"].hide();
	form["content"]["avatar"].setPicture(picturename);
	form["content"]["text"].setText(::format(
		// TRANSLATORS: The first argument is a Discord username, the second
		// a Discord discriminator string, e.g. together "Alice#1234".
		_("Connected as %s#%s on Discord."),
		username, discriminator));
	form.settle();

	evaluatePlayable();
}

void MainMenu::discordDeactivated()
{
	_discordStatus = DiscordStatus::NONE;

	InterfaceElement& discord = getDiscordNub();
	discord["icon"].setTag("Empty");
	discord["tooltip"].setText(
		_("Find us on Discord!"));
	discord.settle();

	evaluatePlayable();
}

void MainMenu::discordDisconnected(int /**/, const char* /**/)
{
	_discordStatus = DiscordStatus::NONE;

	InterfaceElement& discord = getDiscordNub();
	discord["icon"].setTag("Crossed");
	discord["tooltip"].setText(
		_("Disconnected from Discord."));
	discord.settle();

	evaluatePlayable();
}

void MainMenu::discordError(int /**/, const char* /**/)
{
	_discordStatus = DiscordStatus::NONE;

	InterfaceElement& discord = getDiscordNub();
	discord["icon"].setTag("Crossed");
	discord["tooltip"].setText(
		_("An error has occurred."));
	discord.settle();

	evaluatePlayable();
}

void MainMenu::hotJoin(const std::string& /*secret*/)
{
	// We will handle it in ::refresh().
}

void MainMenu::hotSpectate(const std::string& /*secret*/)
{
	// We will handle it in ::refresh().
}

void MainMenu::steamConnecting()
{
	_steamStatus = SteamStatus::CONNECTING;

	InterfaceElement& steam = getSteamNub();
	steam["icon"].setTag("Spinning");
	steam["tooltip"].setText(
		_("Connecting to Steam..."));
	steam.settle();

	evaluatePlayable();
}

void MainMenu::steamConnected(const char* personaname,
		const std::string& avatarpicturename)
{
	_steamStatus = SteamStatus::CONNECTED;

	InterfaceElement& steam = getSteamNub();
	steam["icon"].setTag("Checked");
	steam["tooltip"].setText(::format(
		// TRANSLATORS: The argument is a Steam persona name.
		_("Connected as %s on Steam."),
		personaname));
	steam.settle();

	InterfaceElement& forms = getForms();
	InterfaceElement& form = forms["steammergeform"];
	form["steam"]["avatar"]["spinning"].hide();
	form["steam"]["avatar"].setPicture(avatarpicturename);
	form["steam"]["text"].setText(::format(
		// TRANSLATORS: The argument is a Steam persona name.
		_("Connected as %s on Steam."),
		personaname));
	form.settle();

	evaluatePlayable();
}

void MainMenu::steamDisconnected()
{
	_steamStatus = SteamStatus::NONE;

	InterfaceElement& steam = getSteamNub();
	steam["icon"].setTag("Crossed");
	steam["tooltip"].setText(
		_("Failed to connect with Steam."));
	steam.settle();

	evaluatePlayable();
}

void MainMenu::steamMissing()
{
	_steamStatus = SteamStatus::NONE;

	InterfaceElement& steam = getSteamNub();
	steam["icon"].setTag("Crossed");
	steam["tooltip"].setText(
		_("Failed to detect Steam!"));
	steam.settle();

	evaluatePlayable();
}

void MainMenu::steamAccountMergeTokenResult(
		const ResponseStatus& responsestatus)
{
	InterfaceElement& forms = getForms();
	InterfaceElement& form = forms["steammergeform"];

	switch (responsestatus)
	{
		case ResponseStatus::SUCCESS:
		{
			form["error"].kill();
			form.settle();
			_toForm = "buttons";
		}
		break;
		case ResponseStatus::CREDS_INVALID:
		{
			form["error"]["texts"].setText(
				_(""
				"Unknown email address or incorrect password."
				""));
			form["error"].bear();
			form["error"].settle();
			form.fixWidth();
			form.settle();
		}
		break;
		case ResponseStatus::ACCOUNT_LOCKED:
		{
			form["error"]["texts"].setText(
				_(""
				"Your account has been locked"
				" due to too many failed login attempts."
				" Please try again in 2 hours."
				""));
			form["error"].bear();
			form["error"].settle();
			form.fixWidth();
			form.settle();
		}
		break;
		case ResponseStatus::USERNAME_TAKEN:
		case ResponseStatus::EMAIL_TAKEN:
		case ResponseStatus::ACCOUNT_DISABLED:
		case ResponseStatus::KEY_TAKEN:
		case ResponseStatus::IP_BLOCKED:
		case ResponseStatus::KEY_REQUIRED:
		case ResponseStatus::EMAIL_UNVERIFIED:
		case ResponseStatus::USERNAME_REQUIRED_NOUSER:
		case ResponseStatus::USERNAME_REQUIRED_INVALID:
		case ResponseStatus::USERNAME_REQUIRED_TAKEN:
		case ResponseStatus::DATABASE_ERROR:
		case ResponseStatus::METHOD_INVALID:
		case ResponseStatus::REQUEST_MALFORMED:
		case ResponseStatus::RESPONSE_MALFORMED:
		case ResponseStatus::CONNECTION_FAILED:
		case ResponseStatus::UNKNOWN_ERROR:
		{
			form["error"]["texts"].setText(
				_(""
				"An unexpected error has occurred."
				""));
			form["error"].bear();
			form["error"].settle();
			form.fixWidth();
			form.settle();
		}
		break;
	}
}
