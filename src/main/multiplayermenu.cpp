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
#include "multiplayermenu.hpp"
#include "source.hpp"

#include "libs/SDL2/SDL_mouse.h"

#include "horizontallayout.hpp"
#include "frame.hpp"
#include "tooltiplayout.hpp"
#include "slideshowlayout.hpp"
#include "scrollablelayout.hpp"
#include "textinput.hpp"
#include "textfield.hpp"
#include "multitextinput.hpp"
#include "multitextfield.hpp"
#include "hiddentag.hpp"
#include "image.hpp"
#include "clickanddrag.hpp"
#include "input.hpp"
#include "client.hpp"
#include "mixer.hpp"
#include "message.hpp"
#include "filler.hpp"
#include "player.hpp"
#include "role.hpp"
#include "visiontype.hpp"
#include "target.hpp"
#include "paint.hpp"
#include "colorname.hpp"
#include "map.hpp"
#include "difficulty.hpp"
#include "ranking.hpp"
#include "system.hpp"
#include "locator.hpp"
#include "library.hpp"
#include "openurl.hpp"
#include "parseerror.hpp"
#include "gameowner.hpp"
#include "settings.hpp"


enum class InputMode
{
	CHAT_GENERAL,
	CHAT_LOBBY,
	LOBBYNAME,
};

MultiplayerMenu::~MultiplayerMenu()
{
	if (_layout.born()) kill();
}

void MultiplayerMenu::debugHandler() const
{
	LOGD << ((void*) this);
}

void MultiplayerMenu::onOpen()
{
	_client.registerHandler(this);
	_client.join();
	_client.requestLeaderboard();
}

void MultiplayerMenu::onKill()
{
	_client.deregisterHandler(this);
	_client.leave();

	outServer();
}

void MultiplayerMenu::onShow()
{
	if (!_client.disconnected())
	{
		message("");
		message(
			_("Welcome back!"));
		_client.requestLeaderboard();
		Mixer::get()->fade(Mixer::get()->getOSTid(), 0.2f, 1.5f);
		Mixer::get()->fade(Mixer::get()->getMidiOSTid(), 0.8f, 1.5f);
	}
}

void MultiplayerMenu::onHide()
{
	message("");
}

void MultiplayerMenu::build()
{
	Menu::build();

	const int FONTSIZE = _settings.getFontSize();
	const int FONTSIZE_MENUBUTTON = _settings.getFontSizeMenuButton();

	_layout.add("left", new SlideshowLayout());
	_layout["left"].add("browser", new VerticalLayout());
	_layout["left"].add("inlobby", new VerticalLayout());
	_layout.add("right", new VerticalLayout());

	{
		InterfaceElement& container = _layout["left"]["browser"];

		container.add("t_lobbies", new TextField(
			_("LOBBIES"),
			FONTSIZE, ColorName::TEXT200));
		container.add("lobbies", new Frame("ui/frame_screen_9"));
		int fontHeight = InterfaceElement::fontH(FONTSIZE);
		container["lobbies"].put(new ScrollableLayout(false));
		container["lobbies"].add("filler", new Filler());
		container["lobbies"]["filler"].fixHeight(fontHeight);
		container["lobbies"].setMargin(4 * InterfaceElement::scale());
		container["lobbies"].setMarginTop(2 * InterfaceElement::scale());
		container["lobbies"].setPadding(6 * InterfaceElement::scale());
		container["lobbies"].settleHeight();

		container.add("panels2", new HorizontalLayout());
		container["panels2"].add("onevsone", makePanel(
			_("One vs. One"),
			"panels/onevsone",
			ColorName::FRAME200));
		container["panels2"]["onevsone"].setMargin(
			4 * InterfaceElement::scale());
		container["panels2"].add("versusai", makePanel(
			_("Versus A.I."),
			"panels/versusai",
			ColorName::FRAME200));
		container["panels2"]["versusai"].setMargin(
			4 * InterfaceElement::scale());
		container["panels2"].add("create", makePanel(
			_("Custom Lobby"),
			"panels/freeforall",
			ColorName::FRAME200));
		container["panels2"]["create"].setMargin(
			4 * InterfaceElement::scale());
		container["panels2"].settleHeight();
		container["panels2"].fixHeight();

		container.add("buttons", new HorizontalLayout());
		container["buttons"].add("overview",
			makeButton(
				_("overview map"),
				FONTSIZE_MENUBUTTON));
		container["buttons"]["overview"].setMargin(
			4 * InterfaceElement::scale());
		container["buttons"].add("guide",
			makeButton(
				_("open guide"),
				FONTSIZE_MENUBUTTON));
		container["buttons"]["guide"].setMargin(
			4 * InterfaceElement::scale());
		container["buttons"].add("replay",
			makeButton(
				_("watch replay"),
				FONTSIZE_MENUBUTTON));
		container["buttons"]["replay"].setMargin(
			4 * InterfaceElement::scale());
		container["buttons"].settleHeight();
		container["buttons"].fixHeight();

		container.add("panels1", new HorizontalLayout());
		container["panels1"].add("tutorial", makePanel(
			_("Tutorial"),
			"panels/tutorial",
			ColorName::UIACCENT));
		container["panels1"]["tutorial"].setMargin(
			4 * InterfaceElement::scale());
		container["panels1"].add("challenge", makePanel(
			_("loading..."),
			"unknown",
			ColorName::STAR));
		{
			InterfaceElement& element = container["panels1"]["challenge"];
			element.add("stars", new HorizontalLayout());
			element["stars"].add("filler1", new HorizontalFiller());
			for (int i = 1; i <= 3; i++)
			{
				std::string index = std::to_string(i);
				element["stars"].add(index, new Image("effects/star1"));

				auto& icon = element["stars"][index];
				icon.setColor(0, Paint::blend(ColorName::FRAME600,
					ColorName::SHINEBLEND, 0.2f));
				icon.setColor(1, ColorName::FRAME600);
				icon.setColor(2, Paint::blend(ColorName::FRAME600,
					ColorName::SHADEBLEND));
				icon.setPowerColor(0, Paint::blend(ColorName::STAR,
					ColorName::SHINEBLEND));
				icon.setPowerColor(1, ColorName::STAR);
				icon.setPowerColor(2, Paint::blend(ColorName::STAR,
					ColorName::SHADEBLEND, 0.2f));
				icon.setMargin(8 * InterfaceElement::scale());
			}
			element["stars"].add("filler2", new HorizontalFiller());
		}
		container["panels1"]["challenge"].setMargin(
			4 * InterfaceElement::scale());
		container["panels1"].settleHeight();
		container["panels1"].fixHeight();
	}

	{
		InterfaceElement& container = _layout["right"];
		container.add("tabs", new HorizontalLayout());
		container.add("users", new SlideshowLayout());

		container["tabs"].add("users",
			Menu::makeTabButton(
				_("USERS"),
				FONTSIZE_MENUBUTTON));
		container["tabs"]["users"].settleWidth();
		container["tabs"]["users"].fixWidth();
		container["tabs"].add("rankings", new TooltipLayout());
		container["tabs"]["rankings"].add("button",
			Menu::makeTabButton(
				_("LEADERBOARD"),
				FONTSIZE_MENUBUTTON));
		container["tabs"]["rankings"].add("tooltip", Frame::makeTooltip());
		container["tabs"]["rankings"]["tooltip"].put(
			new TextField(
				_("The leaderboard is updated once per day, at 01:00 UTC."),
				FONTSIZE));
		container["tabs"]["rankings"]["tooltip"].setMargin(
			5 * InterfaceElement::scale());
		container["tabs"]["rankings"].settleWidth();
		container["tabs"]["rankings"].fixWidth();
		container["tabs"].add("filler", new HorizontalFiller());
		container["tabs"].add("feedback",
			Menu::makeTabButton(
				_("FEEDBACK?"),
				FONTSIZE_MENUBUTTON));
		container["tabs"]["feedback"].settleWidth();
		container["tabs"]["feedback"].fixWidth();
	}

	{
		InterfaceElement& container = _layout["right"]["users"];

		container.add("users", new Frame("ui/frame_screen_9"));
		int fontHeight = InterfaceElement::fontH(FONTSIZE);
		container["users"].put(new VerticalLayout());
		container["users"].add("list", new ScrollableLayout(false));
		container["users"]["list"].add("filler", new Filler());
		container["users"]["list"]["filler"].fixHeight(fontHeight);
		container["users"]["list"].setMargin(4 * InterfaceElement::scale());
		container["users"]["list"].setMarginTop(0 * InterfaceElement::scale());
		container["users"]["list"].settleHeight();
		container["users"].setPadding(6 * InterfaceElement::scale());
		container["users"].setPaddingLeft(8 * InterfaceElement::scale());
		container["users"].setPaddingRight(8 * InterfaceElement::scale());
	}

	{
		InterfaceElement& container = _layout["right"]["users"];

		container.add("rankings", new Frame("ui/frame_screen_9"));
		int fontHeight = InterfaceElement::fontH(FONTSIZE) * 13 / 10;
		container["rankings"].put(new ScrollableLayout(false));
		container["rankings"].add("filler", new Filler());
		container["rankings"]["filler"].fixHeight(fontHeight);
		container["rankings"].setMargin(4 * InterfaceElement::scale());
		container["rankings"].setMarginTop(0 * InterfaceElement::scale());
		container["rankings"].setPadding(6 * InterfaceElement::scale());
		container["rankings"].settleHeight();
	}

	{
		InterfaceElement& container = _layout["right"]["users"];
		container.add("feedback", Frame::makeForm());
		InterfaceElement& form = container["feedback"];
		form.put(new VerticalLayout());
		form.add("filler1", new Filler());
		form.add("error", makeErrorprompt(
			_("An error has occurred."),
			FONTSIZE, /*clickable=*/false));
		form["error"].kill();
		form.add("info", makeInfoprompt(
			"", FONTSIZE_MENUBUTTON, /*clickable=*/true));
		form["info"].add("url", new HiddenTag(""));
		form["info"].setMarginBottom(
			5 * InterfaceElement::scale());
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
				std::make_pair(stringref("bug"),
					_("Bug report")),
				std::make_pair(stringref("wish"),
					// Not translated because Stomts are in English.
					std::string("I wish Epicinium...")),
				std::make_pair(stringref("like"),
					// Not translated because Stomts are in English.
					std::string("I like Epicinium...")),
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
			FONTSIZE, /*clickable=*/false));
		form["row"]["info"]["texts"].setWidth(50 * InterfaceElement::scale());
		form["row"]["info"].setMarginLeft(
			(InterfaceElement::windowW() < 600 * InterfaceElement::scale())
			? 5 * InterfaceElement::scale()
			: 10 * InterfaceElement::scale());
		form["row"]["info"].setPadding(3 * InterfaceElement::scale());
		form.add("input", new Frame("ui/frame_window_9"));
		form["input"].put(new MultiTextInput(
			_settings.getFontSizeTextInput(),
			/*lines=*/4));
		form["input"].setWidth(50 * InterfaceElement::scale());
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
		form["sendlogs"]["title"].setWidth(50 * InterfaceElement::scale());
		form["sendlogs"].add("options", Menu::makeCheckbox(
			/*selfclickable=*/false));
		form["sendlogs"]["options"].setTag("Checked");
		form["sendlogs"].makeClickable();
		form["sendlogs"].setMarginTop(
			(InterfaceElement::windowH() < 400 * InterfaceElement::scale())
			? 5 * InterfaceElement::scale()
			: 15 * InterfaceElement::scale());
		form["sendlogs"].align(VerticalAlignment::MIDDLE);
		form.add("buttons", new HorizontalLayout());
		form["buttons"].add("filler1", new HorizontalFiller());
		form["buttons"].add("send", makeButton(
			_("send"),
			FONTSIZE_MENUBUTTON));
		form["buttons"]["send"].setMargin(4 * InterfaceElement::scale());
		form["buttons"]["send"].fixWidth(
			InterfaceElement::fontH(FONTSIZE_MENUBUTTON) * 8);
		form["buttons"].add("filler2", new HorizontalFiller());
		form["buttons"].setMarginTop(
			(InterfaceElement::windowH() < 400 * InterfaceElement::scale())
			? 5 * InterfaceElement::scale()
			: 15 * InterfaceElement::scale());
		form.add("filler2", new Filler());
		form.setPaddingVertical(
			(InterfaceElement::windowH() < 400 * InterfaceElement::scale())
			? 5 * InterfaceElement::scale()
			: 6 * InterfaceElement::scale());
		form.setPaddingHorizontal(
			(InterfaceElement::windowH() < 400 * InterfaceElement::scale())
			? 5 * InterfaceElement::scale()
			: 8 * InterfaceElement::scale());
	}

	{
		InterfaceElement& container = _layout["right"]["users"];
		container.add("discord", Frame::makeForm());
		InterfaceElement& form = container["discord"];
		form.put(new VerticalLayout());
		form.add("filler1", new Filler());
		form.add("content", new HorizontalLayout());
		form["content"].add("filler1", new HorizontalFiller());
		form["content"].add("avatar", makeAvatarFrame());
		form["content"]["avatar"].put(new Filler());
		form["content"]["avatar"].fixWidth(
			64 * InterfaceElement::scale());
		form["content"]["avatar"].fixHeight(
			64 * InterfaceElement::scale());
		form["content"]["avatar"].setMarginHorizontal(
			10 * InterfaceElement::scale());
		form["content"].add("text", new MultiTextField(
			_("Someone wants to join your lobby."),
			FONTSIZE));
		form["content"]["text"].setWidth(
			100 * InterfaceElement::scale());
		form["content"]["text"].setMarginHorizontal(
			10 * InterfaceElement::scale());
		form["content"].add("filler2", new HorizontalFiller());
		form["content"].align(VerticalAlignment::MIDDLE);
		form.add("buttons", new HorizontalLayout());
		form["buttons"].add("filler1", new HorizontalFiller());
		form["buttons"].add("accept", makeButton(
			_("accept"),
			FONTSIZE_MENUBUTTON));
		form["buttons"]["accept"].setMargin(4 * InterfaceElement::scale());
		form["buttons"]["accept"].fixWidth(
			InterfaceElement::fontH(FONTSIZE_MENUBUTTON) * 8);
		form["buttons"].add("deny", makeButton(
			_("deny"),
			FONTSIZE_MENUBUTTON));
		form["buttons"]["deny"].setMargin(4 * InterfaceElement::scale());
		form["buttons"]["deny"].fixWidth(
			InterfaceElement::fontH(FONTSIZE_MENUBUTTON) * 8);
		form["buttons"].add("filler2", new HorizontalFiller());
		form["buttons"].setMarginTop(15 * InterfaceElement::scale());
		form.add("filler2", new Filler());
		form.setPadding(6 * InterfaceElement::scale());
		form.setPaddingLeft(8 * InterfaceElement::scale());
		form.setPaddingRight(8 * InterfaceElement::scale());
	}

	{
		InterfaceElement& container = _layout["right"];

		container.add("t_chat", new TextField(
			_("CHAT"),
			FONTSIZE, ColorName::TEXT200));
		container.add("chat", new Frame("ui/frame_screen_9"));
		int fontHeight = InterfaceElement::fontH(FONTSIZE);
		container["chat"].put(new ScrollableLayout(true));
		container["chat"].add("filler", new Filler());
		container["chat"]["filler"].fixHeight(fontHeight);
		container["chat"].setMargin(4 * InterfaceElement::scale());
		container["chat"].setMarginTop(2 * InterfaceElement::scale());
		container["chat"].setPadding(6 * InterfaceElement::scale());
		container["chat"].settleHeight();

		container.add("inputline", new HorizontalLayout());

		container["inputline"].add("indicator", new TooltipLayout());
		container["inputline"]["indicator"].add("button", makeButton(
			_("LOBBY"),
			FONTSIZE_MENUBUTTON));
		container["inputline"]["indicator"]["button"].setHotkeyScancode(
			SDL_SCANCODE_TAB);
		container["inputline"]["indicator"]["button"].setMarginVertical(4 * InterfaceElement::scale());
		container["inputline"]["indicator"]["button"].setPaddingVertical(4 * InterfaceElement::scale());
		int lobbyw = InterfaceElement::textW(
			TextStyle(FONTSIZE_MENUBUTTON, Color::broken()),
			_("LOBBY"));
		int allw = InterfaceElement::textW(
			TextStyle(FONTSIZE_MENUBUTTON, Color::broken()),
			_("ALL"));
		int namew = InterfaceElement::textW(
			TextStyle(FONTSIZE_MENUBUTTON, Color::broken()),
			_("NAME"));
		container["inputline"]["indicator"]["button"].content().fixWidth(
			std::max(lobbyw, std::max(allw, namew)));
		container["inputline"]["indicator"]["button"].settleWidth();
		container["inputline"]["indicator"]["button"].fixWidth();
		container["inputline"]["indicator"].add("tooltip", Frame::makeTooltip());
		container["inputline"]["indicator"]["tooltip"].put(new TextField(
			_(""
			"The chat target."
			" When in a lobby, click to switch (also TAB key)."
			""),
			FONTSIZE));
		container["inputline"]["indicator"]["tooltip"].setMargin(
			5 * InterfaceElement::scale());
		container["inputline"].add("input", new Frame("ui/frame_window_9"));
		container["inputline"]["input"].put(new TextInput(
			_settings.getFontSizeTextInput(),
			/*maxlength=*/100));
		container["inputline"]["input"].setMarginVertical(4 * InterfaceElement::scale());
		container["inputline"]["input"].setPadding(4 * InterfaceElement::scale());
		container["inputline"]["input"].power();
		container["inputline"]["input"].disable();
		container["inputline"].settleHeight();
	}

	{
		InterfaceElement& container = _layout["right"];

		container.add("return", makeButton(
			_("return to main menu"),
			FONTSIZE_MENUBUTTON));
		container["return"].setHotkeyScancode(SDL_SCANCODE_ESCAPE);
		container["return"].setMargin(4 * InterfaceElement::scale());
	}

	{
		InterfaceElement& container = _layout["left"]["inlobby"];

		container.add("t_players", new TextField(
			_("PLAYERS"),
			FONTSIZE_MENUBUTTON, ColorName::TEXT200));
		container.add("players", new SlideshowLayout());
	}

	{
		InterfaceElement& container = _layout["left"]["inlobby"]["players"];

		container.add("players", new Frame("ui/frame_screen_9"));
		int fontHeight = InterfaceElement::fontH(FONTSIZE);
		container["players"].put(new ScrollableLayout(false));
		container["players"].add("filler", new Filler());
		container["players"]["filler"].fixHeight(fontHeight);
		container["players"].setMargin(4 * InterfaceElement::scale());
		container["players"].setMarginTop(2 * InterfaceElement::scale());
		container["players"].setPadding(6 * InterfaceElement::scale());
		container["players"].settleHeight();
	}

	{
		InterfaceElement& container = _layout["left"]["inlobby"]["players"];

		container.add("replays", new Frame("ui/frame_screen_9"));
		int fontHeight = InterfaceElement::fontH(FONTSIZE);
		container["replays"].put(new ScrollableLayout(false));
		container["replays"].add("filler", new Filler());
		container["replays"]["filler"].fixHeight(fontHeight);
		container["replays"].setMargin(4 * InterfaceElement::scale());
		container["replays"].setMarginTop(2 * InterfaceElement::scale());
		container["replays"].setPadding(6 * InterfaceElement::scale());
		container["replays"].settleHeight();
	}

	{
		InterfaceElement& container = _layout["left"]["inlobby"];

		container.add("t_observers", new TextField(
			_("OBSERVERS"),
			FONTSIZE, ColorName::TEXT200));
		container.add("observers", new Frame("ui/frame_screen_9"));
		int fontHeight = InterfaceElement::fontH(FONTSIZE);
		container["observers"].put(new ScrollableLayout(false));
		container["observers"].add("filler", new Filler());
		container["observers"]["filler"].fixHeight(fontHeight);
		container["observers"].setMargin(4 * InterfaceElement::scale());
		container["observers"].setMarginTop(2 * InterfaceElement::scale());
		container["observers"].setPadding(6 * InterfaceElement::scale());
		container["observers"].settleHeight();

		container.add("t_settings", new TextField(
			_("GAME"),
			FONTSIZE, ColorName::TEXT200));
		container.add("settings", new Frame("ui/frame_screen_9"));
		container["settings"].put(new VerticalLayout());
		container["settings"].setMargin(4 * InterfaceElement::scale());
		container["settings"].setMarginTop(2 * InterfaceElement::scale());
		container["settings"].setPadding(6 * InterfaceElement::scale());

		container["settings"].add("name", new HorizontalLayout());
		auto& name = container["settings"]["name"];
		name.add("text", new TextField("", FONTSIZE, ColorName::TEXT900));
		name["text"].setMarginLeft(8 * InterfaceElement::scale());
		name.add("rename", Frame::makeItem());
		name["rename"].put(new TextField(
			_("rename"),
			FONTSIZE, ColorName::TEXT900));
		name["rename"].makeClickable();
		name["rename"].fixWidth();
		name.align(VerticalAlignment::MIDDLE);

		container["settings"].add("lock", new HorizontalLayout());
		auto& lock = container["settings"]["lock"];
		lock.add("title", new TooltipLayout());
		lock["title"].add("text", new TextField(
			_("Private lobby:"),
			FONTSIZE));
		lock["title"].add("tooltip", Frame::makeTooltip());
		lock["title"]["tooltip"].put(new TextField(
			_("Prevent other users from joining your lobby."),
			FONTSIZE));
		lock["title"]["tooltip"].setMargin(
			5 * InterfaceElement::scale());
		lock.add("options", makeCheckbox());
		lock["options"].setTag("Empty");
		lock["options"].setMarginLeft(10 * InterfaceElement::scale());
		lock.add("filler", new HorizontalFiller());
		lock.align(VerticalAlignment::MIDDLE);

		container["settings"].add("custom", new HorizontalLayout());
		auto& custom = container["settings"]["custom"];
		custom.add("title", new TooltipLayout());
		custom["title"].add("text", new TextField(
			_("Custom maps:"),
			FONTSIZE));
		custom["title"].add("tooltip", Frame::makeTooltip());
		custom["title"]["tooltip"].put(new TextField(
			_("Allow picking custom maps and gamemodes."),
			FONTSIZE));
		custom["title"]["tooltip"].setMargin(
			5 * InterfaceElement::scale());
		custom.add("options", makeCheckbox());
		custom["options"].setTag("Empty");
		custom["options"].setMarginLeft(10 * InterfaceElement::scale());
		custom.add("filler", new HorizontalFiller());
		custom.align(VerticalAlignment::MIDDLE);

		container["settings"].add("map", makeMapDropdown());
		container["settings"].add("planningtime", makeTimerDropdown());
		container["settings"].settleWidth();
		straightenLobbySettings();
		container["settings"].settleHeight();

		container.add("buttons", new HorizontalLayout());
		container["buttons"].add("start", new TooltipLayout());
		container["buttons"]["start"].add("it", makeButton(
			_("start game"),
			FONTSIZE_MENUBUTTON));
		container["buttons"]["start"]["it"].setMargin(
			4 * InterfaceElement::scale());
		container["buttons"]["start"].add("tooltip",
			Frame::makeTooltip());
		container["buttons"]["start"]["tooltip"].put(new TextField(
			_(""
			"You can add AI players by clicking the blue \"+\" icon"
			" on the right side of the \"PLAYERS\" list."
			""),
			FONTSIZE));
		container["buttons"]["start"]["tooltip"].setMargin(
			5 * InterfaceElement::scale());
		container["buttons"]["start"]["tooltip"].kill(1);
		container["buttons"].add("leave", makeButton(
			_("leave lobby"),
			FONTSIZE_MENUBUTTON));
		container["buttons"]["leave"].setMargin(4 * InterfaceElement::scale());
	}

	_layout["right"].setMarginLeft(8 * InterfaceElement::scale());

	_layout.setWidth(InterfaceElement::windowW());
	_layout.setHeight(InterfaceElement::windowH());
	_layout.align(VerticalAlignment::BOTTOM);
	_layout["left"].align(VerticalAlignment::BOTTOM);
	_layout.place(Pixel(0, 0, Layer::INTERFACE));

	_layout["right"]["chat"].remove("filler");
	_layout["left"]["browser"]["lobbies"].remove("filler");
	_layout["left"]["inlobby"]["players"]["players"].remove("filler");
	_layout["left"]["inlobby"]["players"]["replays"].remove("filler");
	_layout["left"]["inlobby"]["observers"].remove("filler");
	_layout["right"]["users"]["users"]["list"].remove("filler");
	_layout["right"]["users"]["rankings"].remove("filler");
	_layout["right"]["users"]["users"].fixHeight();
	_layout["right"]["users"]["feedback"].fixWidth();
	_layout["right"]["users"]["feedback"].fixHeight();
	_layout["right"]["users"]["feedback"]["input"].content().fixWidth();
	_layout["right"]["users"]["discord"].fixWidth();
	_layout["right"]["users"]["discord"].fixHeight();
	_layout["right"]["users"]["discord"]["content"]["text"].fixWidth();

	_layout["left"].setTag("browser");
}

std::unique_ptr<InterfaceElement> MultiplayerMenu::makeMapDropdown()
{
	const int FONTSIZE = _settings.getFontSize();
	const int FONTSIZE_MENUBUTTON = _settings.getFontSizeMenuButton();

	std::unique_ptr<InterfaceElement> it(new HorizontalLayout());
	auto& element = *it;

	element.add("title", new TooltipLayout());
	element["title"].add("text", new TextField(
		_("Map:"),
		FONTSIZE));
	element["title"].add("tooltip", Frame::makeTooltip());
	element["title"]["tooltip"].put(new TextField(
		_("The map the game is played on."),
		FONTSIZE));
	element["title"]["tooltip"].setMargin(
		5 * InterfaceElement::scale());
	element.add("options", new TooltipLayout(/*dropdown=*/true));
	{
		auto& options = element["options"];

		options.add("content", new SlideshowLayout());
		auto& content = options["content"];

		options.add("dropdown", Frame::makeTooltip(/*dropdown=*/true));
		auto& dropdown = options["dropdown"];
		dropdown.put(new VerticalLayout());
		{
			std::string map = "unknown";
			std::string description = _("unknown");
			content.add(map, Frame::makeItem());
			content[map].put(new TextField(description, FONTSIZE_MENUBUTTON,
				ColorName::TEXT700));
			dropdown.add(map, Frame::makeItem());
			dropdown[map].put(new TextField(description, FONTSIZE_MENUBUTTON,
				ColorName::TEXT700));
			content.setTag(map);
		}
		dropdown.settleWidth();
		dropdown.settleHeight();
	}
	element["options"].setMarginLeft(10 * InterfaceElement::scale());
	element.align(VerticalAlignment::MIDDLE);
	element.setMarginTop(1 * InterfaceElement::scale());

	return it;
}

std::unique_ptr<InterfaceElement> MultiplayerMenu::makePanel(
	const std::string& text, const std::string& background,
	const Paint& framecolor)
{
	const int FONTSIZE_MENUBUTTON = _settings.getFontSizeMenuButton();

	std::unique_ptr<InterfaceElement> it(new Frame("ui/frame_button_9"));
	InterfaceElement& element = *it;
	element.setPicture(background);
	element.setColor(0, Paint::alpha(
		ColorName::FRAME200,
		25));
	element.setColor(1, Paint::alpha(
		framecolor,
		200));
	element.setColor(2, Paint::blend(framecolor, ColorName::SHINEBLEND, 0.5f));
	element.setColor(3, Paint::blend(framecolor, ColorName::SHADEBLEND, 0.5f));
	element.setColor(4, Color::transparent());
	element.setColor(5, Color::transparent());
	element.setDisabledColor(0, Paint::alpha(
		Paint::blend(ColorName::FRAME200, ColorName::DISABLEDBLEND),
		230));
	element.setDisabledColor(1, Paint::alpha(
		Paint::blend(framecolor, ColorName::DISABLEDBLEND),
		242));
	element.setDisabledColor(2, Paint::blend(
		Paint::blend(framecolor, ColorName::SHINEBLEND, 0.5f),
		ColorName::DISABLEDBLEND));
	element.setDisabledColor(3, Paint::blend(
		Paint::blend(framecolor, ColorName::SHADEBLEND, 0.5f),
		ColorName::DISABLEDBLEND));
	element.setDisabledColor(4, Color::transparent());
	element.setDisabledColor(5, Color::transparent());
	element.setHoveredColor(0, Paint::alpha(
		Paint::blend(ColorName::FRAME200, ColorName::HOVEREDBLEND),
		12));
	element.setHoveredColor(1, Paint::alpha(
		Paint::blend(framecolor, ColorName::HOVEREDBLEND),
		200));
	element.setHoveredColor(2, Paint::blend(
		Paint::blend(framecolor, ColorName::SHINEBLEND, 0.5f),
		ColorName::HOVEREDBLEND));
	element.setHoveredColor(3, Paint::blend(
		Paint::blend(framecolor, ColorName::SHADEBLEND, 0.5f),
		ColorName::HOVEREDBLEND));
	element.setHoveredColor(4, Color::transparent());
	element.setHoveredColor(5, Color::transparent());
	element.setPressedColor(0, Paint::alpha(
		Paint::blend(ColorName::FRAME200, ColorName::PRESSEDBLEND),
		64));
	element.setPressedColor(1, Paint::alpha(
		Paint::blend(framecolor, ColorName::PRESSEDBLEND),
		216));
	element.setPressedColor(2, Paint::blend(
		Paint::blend(framecolor, ColorName::SHINEBLEND, 0.5f),
		ColorName::PRESSEDBLEND));
	element.setPressedColor(3, Paint::blend(
		Paint::blend(framecolor, ColorName::SHADEBLEND, 0.5f),
		ColorName::PRESSEDBLEND));
	element.setPressedColor(4, Color::transparent());
	element.setPressedColor(5, Color::transparent());
	element.put(new VerticalLayout());
	element.add("title", new TextField(text,
		FONTSIZE_MENUBUTTON, ColorName::TEXT200,
		InterfaceElement::scale(),
		ColorName::TEXT800));
	element["title"].align(HorizontalAlignment::CENTER);
	element.add("view", new Filler());
	element["view"].fixHeight(50 * InterfaceElement::scale());
	element.add("filler", new Filler());
	element.setPadding(6 * InterfaceElement::scale());
	element.settleHeight();
	element.makeClickable();
	return it;
}

void MultiplayerMenu::resetMapDropdown()
{
	const int FONTSIZE_MENUBUTTON = _settings.getFontSizeMenuButton();

	auto& settings = _layout["left"]["inlobby"]["settings"];

	if (settings.contains("custom"))
	{
		settings["custom"]["options"].setTag("Empty");
		settings["custom"]["options"].enable();
	}

	if (!settings.contains("map")) return;
	auto& options = settings["map"]["options"];
	auto& content = options["content"];
	auto& dropdown = options["dropdown"];
	content.reset();
	dropdown.reset();
	{
		std::string map = "unknown";
		std::string description = _("unknown");
		content.add(map, Frame::makeItem());
		content[map].put(new TextField(description, FONTSIZE_MENUBUTTON,
			ColorName::TEXT700));
		dropdown.add(map, Frame::makeItem());
		dropdown[map].put(new TextField(description, FONTSIZE_MENUBUTTON,
			ColorName::TEXT700));
		content.setTag(map);
	}
	dropdown.settleWidth();
	dropdown.settleHeight();
}

std::unique_ptr<InterfaceElement> MultiplayerMenu::makeTimerDropdown()
{
	const int FONTSIZE = _settings.getFontSize();
	const int FONTSIZE_MENUBUTTON = _settings.getFontSizeMenuButton();

	std::unique_ptr<InterfaceElement> element(new HorizontalLayout());
	auto& planningtime = *element;

	planningtime.add("title", new TooltipLayout());
	planningtime["title"].add("text", new TextField(
		_("Planning timer:"),
		FONTSIZE));
	planningtime["title"].add("tooltip", Frame::makeTooltip());
	planningtime["title"]["tooltip"].put(new TextField(
		_("Duration of the planning phase in seconds."),
		FONTSIZE));
	planningtime["title"]["tooltip"].setMargin(
		5 * InterfaceElement::scale());
	planningtime.add("options", new TooltipLayout(/*dropdown=*/true));
	{
		auto& options = planningtime["options"];

		options.add("content", new SlideshowLayout());
		auto& content = options["content"];

		options.add("dropdown", Frame::makeTooltip(/*dropdown=*/true));
		auto& dropdown = options["dropdown"];
		dropdown.put(new VerticalLayout());

		for (int x : {10, 20, 30, 45, 60, 90, 120, 180, 0})
		{
			std::string tagname = std::to_string(x);
			std::string name = tagname;
			std::string desc = "";
			switch (x)
			{
				case 10: desc = _("insane"); break;
				case 30: desc = _("fast"); break;
				case 60: desc = _("standard"); break;
				case 120: desc = _("relaxed"); break;
				case 0: name = _("OFF"); desc = _("unlimited"); break;
			}
			if (!desc.empty()) desc = "(" + desc + ")";
			content.add(tagname, Frame::makeItem());
			content[tagname].put(new TextField(name, FONTSIZE_MENUBUTTON,
				ColorName::TEXT900));
			content[tagname].makeClickable();
			dropdown.add(tagname, Frame::makeItem());
			dropdown[tagname].put(new HorizontalLayout());
			dropdown[tagname].add("name", new TextField(
				name,
				FONTSIZE_MENUBUTTON, ColorName::TEXT900));
			dropdown[tagname]["name"].setMarginRight(4 * InterfaceElement::scale());
			dropdown[tagname]["name"].settleWidth();
			dropdown[tagname]["name"].fixWidth(
				std::max(InterfaceElement::textW(
						TextStyle(FONTSIZE_MENUBUTTON, Color::broken()),
						"9999"),
					InterfaceElement::textW(
						TextStyle(FONTSIZE_MENUBUTTON, Color::broken()),
						_("OFF"))));
			dropdown[tagname]["name"].align(HorizontalAlignment::RIGHT);
			dropdown[tagname].add("desc", new TextField(
				desc,
				FONTSIZE_MENUBUTTON, ColorName::TEXT700));
			dropdown[tagname].makeClickable();
		}
		content.setTag("60");

		dropdown.settleWidth();
		dropdown.settleHeight();
		dropdown.fixWidth();
	}
	planningtime["options"].setMarginLeft(10 * InterfaceElement::scale());
	planningtime.align(VerticalAlignment::MIDDLE);
	planningtime.setMarginTop(1 * InterfaceElement::scale());

	return element;
}

std::string MultiplayerMenu::formatRating(float rating)
{
	std::stringstream strm;
	strm << std::fixed << std::setprecision(1);
	strm << rating;
	return strm.str();
}

void MultiplayerMenu::straightenLobbySettings()
{
	auto& settings = _layout["left"]["inlobby"]["settings"];

	int width = 0;
	for (size_t i = 0; i < settings.size(); i++)
	{
		std::string tagname = settings.name(i);
		if (!settings[tagname].contains("title")) continue;
		width = std::max(width, settings[tagname]["title"].width());
	}
	for (size_t i = 0; i < settings.size(); i++)
	{
		std::string tagname = settings.name(i);
		if (!settings[tagname].contains("title")) continue;
		settings[tagname]["title"].fixWidth(width);
	}
}

void MultiplayerMenu::refresh()
{
	if (_layout.alive() && _client.disconnected())
	{
		quit();
		return;
	}

	bool linkHovered = false;

	// Because some items might occlude others, we must know which layer the mouse operates in.
	// We therefore need to check all items to see if they are hovered.
	_layout.hovered();

	_layout.refresh();

	auto& settings = _layout["left"]["inlobby"]["settings"];
	if (settings["name"]["rename"].clicked())
	{
		_inputMode = InputMode::LOBBYNAME;
		_layout["right"]["inputline"]["indicator"]["button"].setText(
			_("NAME"));
		_layout["right"]["inputline"]["indicator"]["button"].setTextColor(ColorName::TEXTANNOUNCEMENT);
		message(
			_("Please enter a new lobby name."));
	}

	if (settings.contains("map")
		&& settings["map"]["options"]["dropdown"].hovered())
	{
		auto& options = settings["map"]["options"];
		auto& content = options["content"];
		auto& dropdown = options["dropdown"];
		for (size_t i = 0; i < content.size(); i++)
		{
			std::string name = content.name(i);
			if (dropdown.contains(name) && dropdown[name].clicked()
				&& content.getTag() != name)
			{
				_client.send(Message::pick_map(name));
				content.setTag(name);
			}
		}
	}

	if (settings["lock"]["options"].clicked())
	{
		auto& options = settings["lock"]["options"];
		if (options.getTag() == "Empty")
		{
			_client.send(Message::lock_lobby());
			options.setTag("Checked");
		}
		else
		{
			_client.send(Message::unlock_lobby());
			options.setTag("Empty");
		}
	}

	if (settings["custom"]["options"].clicked())
	{
		auto& options = settings["custom"]["options"];
		if (options.getTag() == "Empty")
		{
			_client.send(Message::enable_custom_maps());
			options.setTag("Checked");
			options.disable();
		}
	}

	if (settings.contains("planningtime")
		&& settings["planningtime"]["options"]["dropdown"].hovered())
	{
		auto& options = settings["planningtime"]["options"];
		auto& content = options["content"];
		auto& dropdown = options["dropdown"];
		for (size_t i = 0; i < content.size(); i++)
		{
			std::string name = content.name(i);
			if (dropdown.contains(name) && dropdown[name].clicked()
				&& content.getTag() != name)
			{
				uint32_t timer = std::stoul(name);
				_client.send(Message::pick_timer(timer));
				content.setTag(name);
			}
		}
	}

	if (_layout["left"]["inlobby"]["players"].getTag() == "players")
	{
		auto& players = _layout["left"]["inlobby"]["players"]["players"];
		auto& observers = _layout["left"]["inlobby"]["observers"];
		for (size_t p = 0; p < players.size(); p++)
		{
			std::string username = players.name(p);
			auto& player = players[username];
			if (player.contains("ainame")
				&& player["ainame"]["dropdown"].hovered())
			{
				auto& options = player["ainame"];
				auto& content = options["content"];
				auto& dropdown = options["dropdown"];
				for (size_t i = 0; i < content.size(); i++)
				{
					std::string ainame = content.name(i);
					if (dropdown.contains(ainame) && dropdown[ainame].clicked()
						&& content.getTag() != ainame)
					{
						_client.send(Message::claim_ai(ainame, username));
						content.setTag(ainame);
					}
				}
			}
			else if (player.contains("difficulty")
				&& player["difficulty"]["dropdown"].hovered())
			{
				auto& options = player["difficulty"];
				auto& content = options["content"];
				auto& dropdown = options["dropdown"];
				for (size_t i = 0; i < content.size(); i++)
				{
					std::string difficulty = content.name(i);
					if (dropdown.contains(difficulty)
						&& dropdown[difficulty].clicked()
						&& content.getTag() != difficulty)
					{
						_client.send(Message::claim_difficulty(
							parseDifficulty(difficulty), username));
						content.setTag(difficulty);
					}
				}
			}
			else if (player.contains("color")
				&& player["color"]["dropdown"].hovered())
			{
				auto& options = player["color"];
				auto& content = options["content"];
				auto& dropdown = options["dropdown"];
				for (size_t i = 0; i < content.size(); i++)
				{
					std::string colorname = content.name(i);
					if (dropdown.contains(colorname) && dropdown[colorname].clicked()
						&& content.getTag() != colorname)
					{
						Player color = parsePlayer(colorname);
						_client.send(Message::claim_color(color, username));
						content.setTag(colorname);
					}
				}
			}
			else if (player.contains("visiontype")
				&& player["visiontype"]["dropdown"].hovered())
			{
				auto& options = player["visiontype"];
				auto& content = options["content"];
				auto& dropdown = options["dropdown"];
				for (size_t i = 0; i < content.size(); i++)
				{
					std::string tagname = content.name(i);
					if (dropdown.contains(tagname) && dropdown[tagname].clicked()
						&& content.getTag() != tagname)
					{
						VisionType visiontype = parseVisionType(tagname);
						_client.send(
							Message::claim_visiontype(visiontype, username));
						content.setTag(tagname);
					}
				}
			}
			else if (player.contains("more")
				&& player["more"]["dropdown"].hovered())
			{
				auto& dropdown = player["more"]["dropdown"];
				if (dropdown.contains("observer") && dropdown["observer"].clicked())
				{
					_client.send(
						Message::claim_role(Role::OBSERVER, username));
				}
				else if (dropdown.contains("removebot") && dropdown["removebot"].clicked())
				{
					_client.send(
						Message::remove_bot(username));
				}
				else if (dropdown.contains("copybot") && dropdown["copybot"].clicked())
				{
					_client.send(
						Message::add_bot());
					if (player.contains("ainame"))
					{
						std::string ainame =
							player["ainame"]["content"].getTag();
						_client.send(
							Message::claim_ai(ainame, std::string("")));
					}
					if (player.contains("difficulty"))
					{
						Difficulty difficulty = parseDifficulty(
							player["difficulty"]["content"].getTag());
						_client.send(
							Message::claim_difficulty(
								difficulty, std::string("")));
					}
					if (player.contains("visiontype"))
					{
						VisionType visiontype = parseVisionType(
							player["visiontype"]["content"].getTag());
						_client.send(
							Message::claim_visiontype(
								visiontype, std::string("")));
					}
				}
				else if (dropdown.contains("addbot") && dropdown["addbot"].clicked())
				{
					_client.send(
						Message::add_bot());
				}
			}
			else if (player.held() && observers.overed())
			{
				std::unique_ptr<InterfaceElement> element = players.remove(username);
				addAddBot();
				observers.add(username, std::move(element));
			}
			else if (player.released() && player.contains("name")
				&& !player.contains("color"))
			{
				_client.send(
						Message::claim_role(Role::PLAYER, username));
			}
		}
	}
	else if (_layout["left"]["inlobby"]["players"].getTag() == "replays")
	{
		auto& replays = _layout["left"]["inlobby"]["players"]["replays"];
		for (size_t i = 0; i < replays.size(); i++)
		{
			std::string replayname = replays.name(i);
			auto& replay = replays[replayname];
			if (replay.clicked())
			{
				_client.send(Message::pick_replay(replayname));
			}
		}
	}

	{
		auto& players = _layout["left"]["inlobby"]["players"]["players"];
		auto& observers = _layout["left"]["inlobby"]["observers"];

		std::string addslot;
		std::string botslot;
		for (size_t j = 0; j < players.size(); j++)
		{
			std::string itemname = players.name(j);
			if (itemname.compare(0, 7, "%""addbot") == 0)
			{
				addslot = itemname;
				break;
			}
			else if (players[itemname].contains("ainame"))
			{
				botslot = itemname;
				// Take the last one.
			}
		}

		for (size_t i = 0; i < observers.size(); i++)
		{
			std::string username = observers.name(i);
			auto& observer = observers[username];

			if (observer.contains("more")
				&& observer["more"]["dropdown"].contains("player"))
			{
				observer["more"]["dropdown"]["player"].enableIf(
					!addslot.empty() || !botslot.empty());
			}

			if (observer.contains("more")
				&& observer["more"]["dropdown"].hovered())
			{
				auto& dropdown = observer["more"]["dropdown"];
				if (dropdown.contains("player")
					&& dropdown["player"].clicked())
				{
					if (!botslot.empty())
					{
						_client.send(Message::remove_bot(botslot));
					}

					_client.send(
						Message::claim_role(Role::PLAYER, username));
				}
			}
			else if (observer.held() && players.overed())
			{
				if (!addslot.empty())
				{
					std::unique_ptr<InterfaceElement> element = observers.remove(username);
					players.replace(addslot, std::move(element), username);
				}
			}
			else if (observer.released() && observer.contains("name")
				&& observer.contains("color"))
			{
				_client.send(
					Message::claim_role(Role::OBSERVER, username));
			}
			else if (observer.released() && players.overed())
			{
				if (!botslot.empty())
				{
					_client.send(Message::remove_bot(botslot));
					_client.send(
						Message::claim_role(Role::PLAYER, username));
				}
			}
		}
	}

	if (_layout["left"]["browser"]["buttons"]["guide"].hovered())
	{
		linkHovered = true;
	}

	if (_layout["left"]["browser"]["buttons"]["replay"].clicked())
	{
		Json::Value metadata = Json::objectValue;
		metadata["lobby_type"] = "replay";
		metadata["is_public"] = true;
		_client.send(Message::make_lobby(metadata));
		_client.send(Message::save_lobby());
	}
	else if (_layout["left"]["browser"]["buttons"]["overview"].clicked())
	{
		message(
			_("Starting overview mode..."));
		_gameowner.startDiorama();
	}
	else if (_layout["left"]["browser"]["buttons"]["guide"].clicked())
	{
		System::openURL("https://epicinium.nl/resources/guide.pdf");
	}
	else if (_layout["left"]["browser"]["panels1"]["tutorial"].clicked())
	{
		Json::Value metadata = Json::objectValue;
		metadata["lobby_type"] = "tutorial";
		metadata["is_public"] = false;
		_client.send(Message::make_lobby(metadata));
		_client.send(Message::start());
		_client.send(Message::save_lobby());
	}
	else if (_layout["left"]["browser"]["panels1"]["challenge"].clicked())
	{
		Json::Value metadata = Json::objectValue;
		metadata["lobby_type"] = "challenge";
		metadata["is_public"] = true;
		_client.send(Message::make_lobby(metadata));
		_client.send(Message::start());
		_client.send(Message::save_lobby());
	}
	else if (_layout["left"]["browser"]["panels2"]["onevsone"].clicked())
	{
		if (joinOneVsOne()) {}
		else
		{
			Json::Value metadata = Json::objectValue;
			metadata["lobby_type"] = "one_vs_one";
			metadata["max_players"] = 2;
			metadata["is_public"] = true;
			_client.send(Message::make_lobby(metadata));
			_client.send(Message::save_lobby());
		}
	}
	else if (_layout["left"]["browser"]["panels2"]["create"].clicked())
	{
		Json::Value metadata = Json::objectValue;
		metadata["max_players"] = 4;
		metadata["is_public"] = true;
		_client.send(Message::make_lobby(metadata));
		_client.send(Message::save_lobby());
	}
	else if (_layout["left"]["browser"]["panels2"]["versusai"].clicked())
	{
		Json::Value metadata = Json::objectValue;
		metadata["max_players"] = 2;
		metadata["num_bot_players"] = 1;
		metadata["is_public"] = true;
		_client.send(Message::make_lobby(metadata));
		_client.send(Message::pick_timer(0));
		_client.send(Message::save_lobby());
	}
	else if (_layout["left"]["inlobby"]["buttons"]["leave"].clicked())
	{
		_client.send(Message::leave_lobby());
	}
	else if (_layout["left"]["inlobby"]["buttons"]["start"]["it"].clicked())
	{
		_client.send(Message::start());
	}
	else if (_layout["right"]["return"].clicked())
	{
		quit();
		Mixer::get()->fade(Mixer::get()->getOSTid(), 1.0f, 4.0f);
		Mixer::get()->fade(Mixer::get()->getMidiOSTid(), 0.0f, 1.0f);
		return;
	}

	_layout["right"]["tabs"]["users"].powerIf(
		_layout["right"]["users"].getTag() == "users");
	_layout["right"]["tabs"]["rankings"]["button"].powerIf(
		_layout["right"]["users"].getTag() == "rankings");
	_layout["right"]["tabs"]["feedback"].powerIf(
		_layout["right"]["users"].getTag() == "feedback");

	if (_layout["right"]["tabs"]["users"].clicked()
		&& _layout["right"]["users"].getTag() != "users")
	{
		_layout["right"]["users"].setTag("users");
		_layout["right"]["inputline"]["input"].content().power();
		_layout["right"]["users"]["feedback"]["input"].content().depower();
	}
	else if (_layout["right"]["tabs"]["rankings"]["button"].clicked()
		&& _layout["right"]["users"].getTag() != "rankings")
	{
		_layout["right"]["users"].setTag("rankings");
		_layout["right"]["inputline"]["input"].content().power();
		_layout["right"]["users"]["feedback"]["input"].content().depower();
	}
	else if (_layout["right"]["tabs"]["feedback"].clicked()
		&& _layout["right"]["users"].getTag() != "feedback")
	{
		_layout["right"]["users"].setTag("feedback");
		_layout["right"]["inputline"]["input"].content().depower();
		_layout["right"]["users"]["feedback"]["input"].content().power();
	}

	if (_layout["left"]["browser"]["lobbies"].clicked())
	{
		for (size_t i = 0; i < _layout["left"]["browser"]["lobbies"].size(); i++)
		{
			std::string name = _layout["left"]["browser"]["lobbies"].name(i);
			if (_layout["left"]["browser"]["lobbies"][name].clicked())
			{
				if (_layout["left"]["browser"]["lobbies"][name].powered())
				{
					_client.send(Message::join_lobby(name));
					_layout["left"]["browser"]["lobbies"][name].depower();
				}
				else
				{
					_layout["left"]["browser"]["lobbies"][name].power();
					_layout["left"]["browser"]["lobbies"][name]["name"].setTextColor(ColorName::TEXT900);
				}
			}
			else
			{
				_layout["left"]["browser"]["lobbies"][name].depower();
				_layout["left"]["browser"]["lobbies"][name]["name"].setTextColor(ColorName::TEXT800);
			}
		}
	}

	if (_layout["left"]["inlobby"].born()
		&& _layout["right"]["inputline"]["indicator"]["button"].clicked())
	{
		if (_inputMode == InputMode::CHAT_LOBBY)
		{
			_inputMode = InputMode::CHAT_GENERAL;
			_layout["right"]["inputline"]["indicator"]["button"].setText(
				_("ALL"));
			_layout["right"]["inputline"]["indicator"]["button"].setTextColor(ColorName::TEXT800);
		}
		else
		{
			_inputMode = InputMode::CHAT_LOBBY;
			_layout["right"]["inputline"]["indicator"]["button"].setText(
				_("LOBBY"));
			_layout["right"]["inputline"]["indicator"]["button"].setTextColor(ColorName::TEXTLOBBYCHAT);
		}
	}

	if (_layout["right"]["inputline"]["input"].content().powered())
	{
		_layout["right"]["inputline"]["indicator"]["button"].setHotkeyScancode(
			SDL_SCANCODE_TAB);
	}
	else
	{
		_layout["right"]["inputline"]["indicator"]["button"].setHotkeyScancode(
			0);
	}

	std::string input = _layout["right"]["inputline"]["input"].text();
	if (_layout["right"]["inputline"]["input"].content().powered()
		&& Input::get()->wasKeyPressed(SDL_SCANCODE_ENTER)
		&& !input.empty())
	{
		LOGD << "User input: " << input;
		switch (_inputMode)
		{
			case InputMode::LOBBYNAME:
			{
				_client.send(Message::name_lobby(input));
				_inputMode = InputMode::CHAT_LOBBY;
				_layout["right"]["inputline"]["indicator"]["button"].setText(
					_("LOBBY"));
				_layout["right"]["inputline"]["indicator"]["button"].setTextColor(ColorName::TEXTLOBBYCHAT);
			}
			break;
			case InputMode::CHAT_GENERAL:
			{
				_client.send(Message::chat(input, Target::GENERAL));
			}
			break;
			case InputMode::CHAT_LOBBY:
			{
				_client.send(Message::chat(input, Target::LOBBY));
			}
		}
		_layout["right"]["inputline"]["input"].reset();
	}

	{
		auto& form = _layout["right"]["users"]["feedback"];
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
						form["input"].setText(
							// Not translated because Stomts are in English.
							"would ");
						int w = form["row"]["info"]["texts"].width();
						form["row"]["info"]["texts"].setText(
							_("Please finish the sentence."));
						form["row"]["info"]["texts"].setWidth(w);
						form["row"]["info"].place(
							form["row"]["info"].topleft());
						form["sendlogs"]["options"].setTag("Empty");
						form["sendlogs"].enable();
					}
					else if (name == "like")
					{
						form["input"].setText(
							// Not translated because Stomts are in English.
							"because ");
						int w = form["row"]["info"]["texts"].width();
						form["row"]["info"]["texts"].setText(
							_("Please finish the sentence."));
						form["row"]["info"]["texts"].setWidth(w);
						form["row"]["info"].place(
							form["row"]["info"].topleft());
						form["sendlogs"]["options"].setTag("Empty");
						form["sendlogs"].disable();
					}
					else // if (name == "bug")
					{
						form["input"].setText("");
						int w = form["row"]["info"]["texts"].width();
						form["row"]["info"]["texts"].setText(
							_("Please describe what happened."));
						form["row"]["info"]["texts"].setWidth(w);
						form["row"]["info"].place(
							form["row"]["info"].topleft());
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
				form["input"].setText(
					// Not translated because Stomts are in English.
					"would ");
			}
			else if (content.getTag() == "like")
			{
				_client.feedback(true, form["input"].text(), sendlogs);
				form["input"].setText(
					// Not translated because Stomts are in English.
					"because ");
			}
			else // if (content.getTag() == "bug")
			{
				_client.bugReport(form["input"].text(), sendlogs);
				form["input"].setText("");
			}
		}

		if (form["info"].hovered() && !form["info"]["url"].text().empty())
		{
			linkHovered = true;
			if (form["info"].clicked())
			{
				System::openURL(form["info"]["url"].text());
			}
		}

		if (form.enabled())
		{
			if (form["input"].clicked())
			{
				form["input"].content().power();
				_layout["right"]["inputline"]["input"].content().depower();
			}
			if (_layout["right"]["inputline"]["input"].clicked())
			{
				form["input"].content().depower();
				_layout["right"]["inputline"]["input"].content().power();
			}
		}
	}

	{
		auto& form = _layout["right"]["users"]["discord"];

		if (form["buttons"]["accept"].clicked())
		{
			_client.acceptJoinRequest();
			_layout["right"]["users"].setTag("users");
			_layout["right"]["inputline"]["input"].content().power();
		}
		else if (form["buttons"]["deny"].clicked())
		{
			_client.denyJoinRequest();
			_layout["right"]["users"].setTag("users");
			_layout["right"]["inputline"]["input"].content().power();
		}
	}

	{
		auto& users = _layout["right"]["users"]["users"];
		if (users.contains("discord") && users["discord"].hovered())
		{
			linkHovered = true;
			if (users["discord"].clicked())
			{
				System::openURL("https://discord.gg/vQhTURC");
			}
		}
	}

	{
		auto& chat = _layout["right"]["chat"];
		for (size_t i = 0; i < chat.size(); i++)
		{
			std::string name = chat.name(i);
			if (chat[name].clickable() && chat[name].hovered())
			{
				linkHovered = true;
				if (chat[name].clicked())
				{
					std::string url = chat[name].text();
					if (url.compare(0, 8, "https://") == 0)
					{
						LOGI << "Opening server chat url '" << url << "'";
						System::openURL(url);
					}
					else
					{
						LOGW << "User clicked on non-url '" << url << "'";
					}
				}
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

bool MultiplayerMenu::joinOneVsOne()
{
	InterfaceElement& lobbies = _layout["left"]["browser"]["lobbies"];
	for (size_t i = 0; i < lobbies.size(); i++)
	{
		std::string lobbyid = lobbies.name(i);
		InterfaceElement& lobby = lobbies[lobbyid];
		if (!lobby.contains("name")) continue;
		if (!lobby.contains("type")) continue;
		if (!lobby.contains("status")) continue;
		if (!lobby.contains("num_players")) continue;
		if (!lobby.contains("max_players")) continue;
		if (lobby["type"].text() != "1v1") continue;
		if (lobby["status"].getTag() != "unlocked") continue;
		if (lobby["status"]["unlocked"].getTag() != "open") continue;

		try
		{
			int num = std::stoi(lobby["num_players"].text());
			int max = std::stoi(lobby["max_players"].text());
			if (num <= 0) continue;
			if (num >= max) continue;
			if (max > 2) continue;
		}
		catch (...)
		{
			RETHROW_IF_DEV();
			continue;
		}

		_client.send(Message::join_lobby(lobbyid));
		return true;
	}

	return false;
}

void MultiplayerMenu::message(const std::string& message)
{
	const int FONTSIZE = _settings.getFontSize();

	TextField* textField = new TextField(message,
		FONTSIZE, ColorName::TEXTANNOUNCEMENT);
	if (message.compare(0, 8, "https://") == 0
		&& textField->textLeftover().empty())
	{
		textField->setTextColor(ColorName::UIACCENT);
		textField->makeClickable();
	}
	_layout["right"]["chat"].add(
		std::to_string(_layout["right"]["chat"].size()),
		textField);
	_layout["right"]["chat"].setWidth(_layout["right"]["chat"].width());

	while (!textField->textLeftover().empty())
	{
		textField = new TextField(textField->textLeftover(),
			FONTSIZE, ColorName::TEXTANNOUNCEMENT);
		_layout["right"]["chat"].add(
			std::to_string(_layout["right"]["chat"].size()),
			textField);
		_layout["right"]["chat"].setWidth(_layout["right"]["chat"].width());
	}
}

void MultiplayerMenu::chat(const std::string& user, const std::string& message, const Target& target)
{
	const int FONTSIZE = _settings.getFontSize();

	Paint color = (target == Target::LOBBY ? ColorName::TEXTLOBBYCHAT : ColorName::TEXT800);
	TextField* textField = new TextField(user + ": " + message, FONTSIZE, color);
	_layout["right"]["chat"].add(std::to_string(_layout["right"]["chat"].size()), textField);
	_layout["right"]["chat"].setWidth(_layout["right"]["chat"].width());

	while (!textField->textLeftover().empty())
	{
		textField = new TextField(textField->textLeftover(), FONTSIZE, color);
		_layout["right"]["chat"].add(std::to_string(_layout["right"]["chat"].size()), textField);
		_layout["right"]["chat"].setWidth(_layout["right"]["chat"].width());
	}
}

void MultiplayerMenu::addUser(const std::string& user,
	const Json::Value& metadata, bool isSelf)
{
	const int FONTSIZE = _settings.getFontSize();
	const int FONTSIZE_MENUBUTTON = _settings.getFontSizeMenuButton();

	if (_layout["right"]["users"]["users"]["list"].contains(user))
	{
		LOGE << "There is already a user called '" << user << "' online";
		DEBUG_ASSERT(false);
		return;
	}

	bool dev = (metadata["dev"].isBool() && metadata["dev"].asBool());
	bool guest = (metadata["guest"].isBool() && metadata["guest"].asBool());
	bool supporter = (metadata["supporter"].isBool()
		&& metadata["supporter"].asBool());

	HorizontalLayout* layout = new HorizontalLayout();
	(*layout).align(VerticalAlignment::MIDDLE);
	(*layout).add("name",
		new TextField(user, FONTSIZE,
		isSelf ? ColorName::TEXTOWNNAME : ColorName::TEXT900));
	(*layout)["name"].settleWidth();
	(*layout)["name"].fixWidth();
	if (dev)
	{
		(*layout).add("dev", new TextField(
			// TRANSLATORS: Abbreviation of "developer".
			_("dev"),
			FONTSIZE, ColorName::TEXT600));
		(*layout)["dev"].setMarginLeft(8 * InterfaceElement::scale());
		(*layout)["dev"].setMarginRight(2 * InterfaceElement::scale());
		(*layout)["dev"].settleWidth();
		(*layout)["dev"].fixWidth();
	}
	else if (guest)
	{
		(*layout).add("guest", new TextField(
			_("guest"),
			FONTSIZE, ColorName::TEXT600));
		(*layout)["guest"].setMarginLeft(8 * InterfaceElement::scale());
		(*layout)["guest"].setMarginRight(2 * InterfaceElement::scale());
		(*layout)["guest"].settleWidth();
		(*layout)["guest"].fixWidth();
	}
	if (supporter && !dev && false)
	{
		(*layout).add("icon_heart", new Image("effects/heart1_still"));
		{
			auto& icon = (*layout)["icon_heart"];
			icon.setColor(0,
				Paint::blend(ColorName::HEART, ColorName::SHINEBLEND));
			icon.setColor(1, ColorName::HEART);
			icon.setColor(2,
				Paint::blend(ColorName::HEART, ColorName::SHADEBLEND, 0.2f));
			icon.setMarginLeft(8 * InterfaceElement::scale());
			icon.setMarginRight(2 * InterfaceElement::scale());
			icon.setTag("Still");
		}
	}
	if (!dev && !guest)
	{
		(*layout).add("icon_star", new Image("effects/star1"));
		(*layout).add("stars", new TextField(
			"x9999",
			FONTSIZE, ColorName::TEXT600));
		(*layout)["stars"].settleWidth();
		(*layout)["stars"].fixWidth();
		(*layout)["stars"].setText("x0");
		{
			auto& icon = (*layout)["icon_star"];
			icon.setColor(0,
				Paint::blend(ColorName::STAR, ColorName::SHINEBLEND));
			icon.setColor(1, ColorName::STAR);
			icon.setColor(2,
				Paint::blend(ColorName::STAR, ColorName::SHADEBLEND, 0.2f));
			icon.setMarginLeft(8 * InterfaceElement::scale());
			icon.setMarginRight(2 * InterfaceElement::scale());
		}
	}
	(*layout).add("filler", new HorizontalFiller());
	if (!guest)
	{
		(*layout).add("rating", new TextField(
			"999.9",
			FONTSIZE, ColorName::TEXT600));
		(*layout)["rating"].settleWidth();
		(*layout)["rating"].fixWidth();
		(*layout)["rating"].setText("");
		(*layout)["rating"].align(HorizontalAlignment::RIGHT);
		(*layout)["rating"].setMarginLeft(2 * InterfaceElement::scale());
	}
	(*layout).add("status", new SlideshowLayout());
	(*layout)["status"].add("available", makeWindowbutton(/*clickable=*/false));
	(*layout)["status"]["available"].setTag("Empty");
	(*layout)["status"].add("inlobby", new TooltipLayout());
	(*layout)["status"]["inlobby"].add("image",
		makeWindowbutton(/*clickable=*/false));
	(*layout)["status"]["inlobby"].add("tooltip", Frame::makeTooltip());
	(*layout)["status"]["inlobby"]["tooltip"].put(new TextField(
		_("This player is in a lobby."),
		FONTSIZE));
	(*layout)["status"]["inlobby"]["tooltip"].setMargin(
		5 * InterfaceElement::scale());
	(*layout)["status"]["inlobby"]["image"].setTag("Ring");
	(*layout)["status"].add("ingame", new TooltipLayout());
	(*layout)["status"]["ingame"].add("image",
		makeWindowbutton(/*clickable=*/false));
	(*layout)["status"]["ingame"].add("tooltip", Frame::makeTooltip());
	(*layout)["status"]["ingame"]["tooltip"].put(new TextField(
		_("This player is in a game."),
		FONTSIZE));
	(*layout)["status"]["ingame"]["tooltip"].setMargin(
		5 * InterfaceElement::scale());
	(*layout)["status"]["ingame"]["image"].setTag("Target");
	(*layout)["status"].add("spectating", new TooltipLayout());
	(*layout)["status"]["spectating"].add("image",
		makeWindowbutton(/*clickable=*/false));
	(*layout)["status"]["spectating"].add("tooltip", Frame::makeTooltip());
	(*layout)["status"]["spectating"]["tooltip"].put(new TextField(
		_("This player is spectating a game in progress."),
		FONTSIZE));
	(*layout)["status"]["spectating"]["tooltip"].setMargin(
		5 * InterfaceElement::scale());
	(*layout)["status"]["spectating"]["image"].setTag("Eye");
	(*layout)["status"].add("inreplay", new TooltipLayout());
	(*layout)["status"]["inreplay"].add("image",
		makeWindowbutton(/*clickable=*/false));
	(*layout)["status"]["inreplay"].add("tooltip", Frame::makeTooltip());
	(*layout)["status"]["inreplay"]["tooltip"].put(new TextField(
		_("This player is watching a replay."),
		FONTSIZE));
	(*layout)["status"]["inreplay"]["tooltip"].setMargin(
		5 * InterfaceElement::scale());
	(*layout)["status"]["inreplay"]["image"].setTag("Camera");
	(*layout)["status"].setTag("available");
	// TODO should the following line be necessary?
	(*layout).settleHeight();

	auto& users = _layout["right"]["users"]["users"];
	users["list"].add(user, layout);
	users.setWidth(users.width());
	users["list"][user].fixWidth();
	users["list"][user].place(users["list"][user].topleft());

	if (users["list"].size() < 4)
	{
		if (!users.contains("discord"))
		{
			users.add("discord", makeInfoprompt(
				_("Looking for opponents? Try our Discord."),
				FONTSIZE_MENUBUTTON, /*clickable=*/true));
			users["discord"]["texts"].setWidth(50 * InterfaceElement::scale());
			users["discord"].setWidth(users["list"].width());
			users["discord"].settleHeight();
			users.fixHeight(users.height());
			users.settle();
			users.setWidth(users.width());
			users.place(users.topleft());
		}
	}
	else
	{
		if (users.contains("discord"))
		{
			users.remove("discord");
			users.fixHeight(users.height());
			users.settle();
		}
	}
}

void MultiplayerMenu::removeUser(const std::string& user)
{
	const int FONTSIZE_MENUBUTTON = _settings.getFontSizeMenuButton();

	auto& users = _layout["right"]["users"]["users"];
	if (users["list"].contains(user))
	{
		users["list"].remove(user);

		if (users["list"].size() < 4 && !users.contains("discord"))
		{
			users.add("discord", makeInfoprompt(
				_("Looking for opponents? Try our Discord."),
				FONTSIZE_MENUBUTTON, /*clickable=*/true));
			users["discord"]["texts"].setWidth(50 * InterfaceElement::scale());
			users["discord"].setWidth(users["list"].width());
			users["discord"].settleHeight();
			users.fixHeight(users.height());
			users.settle();
			users.setWidth(users.width());
			users.place(users.topleft());
		}
	}
}

void MultiplayerMenu::addLobby(const std::string& lobby)
{
	const int FONTSIZE = _settings.getFontSize();
	const int FONTSIZE_MENUBUTTON = _settings.getFontSizeMenuButton();

	if (_layout["left"]["browser"]["lobbies"].contains(lobby)) return;

	_layout["left"]["browser"]["lobbies"].add(lobby, Frame::makeItem());
	_layout["left"]["browser"]["lobbies"][lobby].put(new HorizontalLayout());
	_layout["left"]["browser"]["lobbies"][lobby].align(VerticalAlignment::MIDDLE);
	InterfaceElement& newlobby = _layout["left"]["browser"]["lobbies"][lobby];
	_layout["left"]["browser"]["lobbies"][lobby].add("name", new TextField("",
		FONTSIZE_MENUBUTTON));
	_layout["left"]["browser"]["lobbies"][lobby]["name"].setMarginLeft(4 * InterfaceElement::scale());
	newlobby.add("type", new TextField("custom", FONTSIZE, ColorName::TEXT600));
	newlobby["type"].setMarginHorizontal(4 * InterfaceElement::scale());
	newlobby["type"].fixWidth();
	newlobby["type"].setText("");
	newlobby["type"].align(HorizontalAlignment::RIGHT);
	_layout["left"]["browser"]["lobbies"][lobby].add("num_players",
			new TextField("?", FONTSIZE));
	_layout["left"]["browser"]["lobbies"][lobby]["num_players"].fixWidth();
	_layout["left"]["browser"]["lobbies"][lobby]["num_players"].align(
			HorizontalAlignment::RIGHT);
	_layout["left"]["browser"]["lobbies"][lobby].add("slash",
			new TextField("/", FONTSIZE));
	_layout["left"]["browser"]["lobbies"][lobby]["slash"].fixWidth();
	_layout["left"]["browser"]["lobbies"][lobby]["slash"].setMarginHorizontal(
			1 * InterfaceElement::scale());
	_layout["left"]["browser"]["lobbies"][lobby].add("max_players",
			new TextField("?", FONTSIZE));
	_layout["left"]["browser"]["lobbies"][lobby]["max_players"].align(
			HorizontalAlignment::LEFT);
	_layout["left"]["browser"]["lobbies"][lobby]["max_players"].fixWidth();
	newlobby.add("status", new SlideshowLayout());
	newlobby["status"].add("locked", new TooltipLayout());
	newlobby["status"]["locked"].add("image",
		makeWindowbutton(/*clickable=*/false));
	newlobby["status"]["locked"].add("tooltip", Frame::makeTooltip());
	newlobby["status"]["locked"]["tooltip"].put(new TextField(
		_("This lobby is locked."),
		FONTSIZE));
	newlobby["status"]["locked"]["tooltip"].setMargin(
		5 * InterfaceElement::scale());
	newlobby["status"]["locked"]["image"].setTag("Lock");
	newlobby["status"].add("unlocked", new SlideshowLayout());
	newlobby["status"]["unlocked"].add("open",
		makeWindowbutton(/*clickable=*/false));
	newlobby["status"]["unlocked"]["open"].setTag("Empty");
	newlobby["status"]["unlocked"].add("ingame", new TooltipLayout());
	newlobby["status"]["unlocked"]["ingame"].add("image",
		makeWindowbutton(/*clickable=*/false));
	newlobby["status"]["unlocked"]["ingame"].add("tooltip",
		Frame::makeTooltip());
	newlobby["status"]["unlocked"]["ingame"]["tooltip"].put(new TextField(
		_("This game is already in progress."),
		FONTSIZE));
	newlobby["status"]["unlocked"]["ingame"]["tooltip"].setMargin(
		5 * InterfaceElement::scale());
	newlobby["status"]["unlocked"]["ingame"]["image"].setTag("Target");
	newlobby["status"]["unlocked"].add("replay", new TooltipLayout());
	newlobby["status"]["unlocked"]["replay"].add("image",
		makeWindowbutton(/*clickable=*/false));
	newlobby["status"]["unlocked"]["replay"].add("tooltip",
		Frame::makeTooltip());
	newlobby["status"]["unlocked"]["replay"]["tooltip"].put(new TextField(
		_("Players in this lobby are watching a replay."),
		FONTSIZE));
	newlobby["status"]["unlocked"]["replay"]["tooltip"].setMargin(
		5 * InterfaceElement::scale());
	newlobby["status"]["unlocked"]["replay"]["image"].setTag("Camera");
	newlobby["status"]["unlocked"].setTag("open");
	newlobby["status"].setTag("unlocked");
	_layout["left"]["browser"]["lobbies"][lobby].settleHeight();
	_layout["left"]["browser"]["lobbies"][lobby].align(VerticalAlignment::MIDDLE);
	_layout["left"]["browser"]["lobbies"][lobby].makeClickable();
	_layout["left"]["browser"]["lobbies"].setWidth(
		_layout["left"]["browser"]["lobbies"].width());
	_layout["left"]["browser"]["lobbies"][lobby]["name"].fixWidth();
	_layout["left"]["browser"]["lobbies"][lobby].place(
		_layout["left"]["browser"]["lobbies"][lobby].topleft());
}

void MultiplayerMenu::removeLobby(const std::string& lobby)
{
	if (_layout["left"]["browser"]["lobbies"].contains(lobby))
	{
		std::string lobbyname = _layout["left"]["browser"]["lobbies"][lobby]["name"].text();
		message(::format(
			// TRANSLATORS: The argument is an English or custom lobby name.
			_("Lobby \"%s\" was disbanded."),
			lobbyname.c_str()));
		_layout["left"]["browser"]["lobbies"].remove(lobby);
	}
}

void MultiplayerMenu::joinsLobby(const std::string& sender)
{
	auto& users = _layout["right"]["users"]["users"]["list"];
	if (users.contains(sender))
	{
		users[sender]["status"].setTag("inlobby");
	}
}

void MultiplayerMenu::inGame(const std::string& lobbyid,
	const std::string& sender, const Role& role)
{
	Mixer::get()->stop();

	bool isreplay = false;
	if (_layout["left"]["browser"]["lobbies"].contains(lobbyid))
	{
		auto& lobby = _layout["left"]["browser"]["lobbies"][lobbyid];
		if (lobby["status"]["unlocked"].getTag() == "open")
		{
			lobby["status"]["unlocked"].setTag("ingame");
		}
		else if (lobby["status"]["unlocked"].getTag() == "replay")
		{
			isreplay = true;
		}
	}

	auto& users = _layout["right"]["users"]["users"]["list"];
	if (users.contains(sender))
	{
		if (isreplay)
		{
			users[sender]["status"].setTag("inreplay");
		}
		else if (role == Role::OBSERVER)
		{
			users[sender]["status"].setTag("spectating");
		}
		else
		{
			users[sender]["status"].setTag("ingame");
		}
	}
}

void MultiplayerMenu::leavesLobby(const std::string& sender)
{
	auto& users = _layout["right"]["users"]["users"]["list"];
	if (users.contains(sender))
	{
		users[sender]["status"].setTag("available");
	}
}

void MultiplayerMenu::listLobby(const std::string& lobbyid,
	const std::string& name, const Json::Value& metadata)
{
	bool own = (lobbyid == _ownLobby);

	addLobby(lobbyid);

	nameLobby(lobbyid, name);
	if (own) nameOwnLobby(name);

	int max = 0;
	if (metadata["max_players"].isInt())
	{
		max = metadata["max_players"].asInt();
	}
	maxPlayers(lobbyid, max);

	int num = 0;
	if (metadata["num_players"].isInt())
	{
		num = metadata["num_players"].asInt();
	}
	numPlayers(lobbyid, num);

	bool isPublic = false;
	if (metadata["is_public"].isBool())
	{
		isPublic = metadata["is_public"].asBool();
	}
	if (isPublic)
	{
		unlockLobby(lobbyid);
		if (own) unlockOwnLobby();
	}
	else
	{
		lockLobby(lobbyid);
		if (own) lockOwnLobby();
	}

	if (metadata["lobby_type"].isString())
	{
		std::string type = metadata["lobby_type"].asString();
		if (type == "one_vs_one")
		{
			tagLobby(lobbyid,
				// TRANSLATORS: Short for "one versus one".
				_("1v1"));
			if (own)
			{
				restrictLobbySettingsForOneVsOne();
			}
		}
		else if (type == "custom")
		{
			tagLobby(lobbyid,
				_("custom"));
		}
		else
		{
			tagLobby(lobbyid, "");
		}
	}
}

void MultiplayerMenu::nameLobby(const std::string& lobby,
	const std::string& name)
{
	_layout["left"]["browser"]["lobbies"][lobby]["name"].setText(name);
	_layout["left"]["browser"]["lobbies"][lobby]["name"].setWidth(
		_layout["left"]["browser"]["lobbies"][lobby]["name"].width());
	_layout["left"]["browser"]["lobbies"][lobby].place(
		_layout["left"]["browser"]["lobbies"][lobby].topleft());
}

void MultiplayerMenu::tagLobby(const std::string& lobby,
	const std::string& type)
{
	_layout["left"]["browser"]["lobbies"][lobby]["type"].setText(type);
}

void MultiplayerMenu::lockLobby(const std::string& lobbyid)
{
	if (_layout["left"]["browser"]["lobbies"].contains(lobbyid))
	{
		auto& lobby = _layout["left"]["browser"]["lobbies"][lobbyid];
		lobby["status"].setTag("locked");
	}
}

void MultiplayerMenu::unlockLobby(const std::string& lobbyid)
{
	if (_layout["left"]["browser"]["lobbies"].contains(lobbyid))
	{
		auto& lobby = _layout["left"]["browser"]["lobbies"][lobbyid];
		lobby["status"].setTag("unlocked");
	}
}

void MultiplayerMenu::joinsOwnLobby(const std::string& sender, bool isSelf)
{
	addObserver(sender, isSelf);
}

void MultiplayerMenu::leavesOwnLobby(const std::string& sender)
{
	if (_layout["left"]["inlobby"]["players"]["players"].contains(sender))
	{
		_layout["left"]["inlobby"]["players"]["players"].remove(sender);
		addAddBot();
	}
	if (_layout["left"]["inlobby"]["observers"].contains(sender))
	{
		_layout["left"]["inlobby"]["observers"].remove(sender);
	}
}

void MultiplayerMenu::nameOwnLobby(const std::string& name)
{
	if (_layout["left"]["inlobby"].born())
	{
		_layout["left"]["inlobby"]["settings"]["name"]["text"].setText(name);
	}
}

void MultiplayerMenu::lockOwnLobby()
{
	auto& options = _layout["left"]["inlobby"]["settings"]["lock"]["options"];
	options.setTag("Checked");
}

void MultiplayerMenu::unlockOwnLobby()
{
	auto& options = _layout["left"]["inlobby"]["settings"]["lock"]["options"];
	options.setTag("Empty");
}

void MultiplayerMenu::maxPlayers(const std::string& lobbyid, uint32_t count)
{
	if (lobbyid == _ownLobby || lobbyid.empty())
	{
		auto& players = _layout["left"]["inlobby"]["players"]["players"];

		for (int i = 1; i <= (int) PLAYER_MAX; i++)
		{
			std::string addbotslot = "%""addbot" + std::to_string(i);
			if (players.contains(addbotslot))
			{
				players.remove(addbotslot);
			}
		}

		// Enable the start game button for now;
		// it will disabled if we do not have enough players.
		_layout["left"]["inlobby"]["buttons"]["start"]["it"].enable();
		_layout["left"]["inlobby"]["buttons"]["start"]["tooltip"].kill(1);

		for (int i = (int) players.size();
			i < (int) count; i++)
		{
			addAddBot();
		}
	}

	if (_layout["left"]["browser"]["lobbies"].contains(lobbyid))
	{
		auto& lobby = _layout["left"]["browser"]["lobbies"][lobbyid];
		lobby["max_players"].setText(std::to_string(count));

		if (count == 0 && lobby["status"]["unlocked"].getTag() == "open")
		{
			lobby["status"]["unlocked"].setTag("replay");
		}
		else if (count != 0 && lobby["status"]["unlocked"].getTag() == "replay")
		{
			lobby["status"]["unlocked"].setTag("open");
		}
	}
}

void MultiplayerMenu::numPlayers(const std::string& lobby, uint32_t count)
{
	if (_layout["left"]["browser"]["lobbies"].contains(lobby))
	{
		_layout["left"]["browser"]["lobbies"][lobby]["num_players"].setText(
			std::to_string(count));
	}
}

void MultiplayerMenu::addPlayer(const std::string& username, bool isSelf)
{
	const int FONTSIZE = _settings.getFontSize();
	const int FONTSIZE_MENUBUTTON = _settings.getFontSizeMenuButton();

	auto& players = _layout["left"]["inlobby"]["players"]["players"];
	if (players.contains(username)) return;

	{
		auto& observers = _layout["left"]["inlobby"]["observers"];
		if (observers.contains(username))
		{
			observers.remove(username);
		}
	}

	int addbots = 0;
	for (int i = 1; i <= (int) PLAYER_MAX; i++)
	{
		std::string addbotslot = "%""addbot" + std::to_string(i);
		if (players.contains(addbotslot))
		{
			players.remove(addbotslot);
			addbots++;
		}
	}

	{
		std::unique_ptr<InterfaceElement> element(new ClickAndDrag(true));
		auto& player = *element;
		player.put(Frame::makeItem(Paint::alpha(ColorName::FRAMESAND, 100)));
		player.content().put(new HorizontalLayout());
		player.add("name",
			new TextField(username, FONTSIZE,
			isSelf ? ColorName::TEXTOWNNAME : ColorName::TEXT900));
		player["name"].setMarginHorizontal(
			(InterfaceElement::windowW() < 600 * InterfaceElement::scale())
			? 2 * InterfaceElement::scale()
			: 8 * InterfaceElement::scale());

		player.add("visiontype", new TooltipLayout(/*dropdown=*/true));
		{
			auto& options = player["visiontype"];

			options.add("content", new SlideshowLayout());
			auto& content = options["content"];

			options.add("dropdown", Frame::makeTooltip(/*dropdown=*/true));
			auto& dropdown = options["dropdown"];
			dropdown.put(new VerticalLayout());

			for (auto& type : {VisionType::NORMAL, VisionType::GLOBAL})
			{
				std::string tagname = std::string(stringify(type));
				std::string sname;
				std::string name;
				switch (type)
				{
					case VisionType::NORMAL:
					{
						sname = _("normal");
						name = _("normal vision");
					}
					break;
					case VisionType::GLOBAL:
					{
						sname = _("global");
						name = _("global vision");
					}
					break;
					case VisionType::NONE:
					break;
				}
				Paint textcolor = ColorName::TEXT800;
				content.add(tagname, Frame::makeItem());
				content[tagname].put(new TextField(sname, FONTSIZE_MENUBUTTON,
					textcolor));
				content[tagname].align(HorizontalAlignment::CENTER);
				content[tagname].makeClickable();
				dropdown.add(tagname, Frame::makeItem());
				dropdown[tagname].put(new TextField(name, FONTSIZE_MENUBUTTON,
					textcolor));
				dropdown[tagname].align(HorizontalAlignment::CENTER);
				dropdown[tagname].makeClickable();
			}
			content.settleWidth();
			content.settleHeight();
			content.fixWidth();
			content.setTag(stringify(VisionType::NORMAL));

			dropdown.settleWidth();
			dropdown.settleHeight();
			dropdown.fixWidth();

			// A little hack to detect OneVsOne lobbies.
			auto& settings = _layout["left"]["inlobby"]["settings"];
			if (!settings["planningtime"].enabled())
			{
				options.kill(1);
			}
		}
		player["visiontype"].setMarginHorizontal(
			(InterfaceElement::windowW() < 600 * InterfaceElement::scale())
			? 2 * InterfaceElement::scale()
			: 8 * InterfaceElement::scale());

		player.add("color", new TooltipLayout(/*dropdown=*/true));
		{
			auto& options = player["color"];

			options.add("content", new SlideshowLayout());
			auto& content = options["content"];

			options.add("dropdown", Frame::makeTooltip(/*dropdown=*/true));
			auto& dropdown = options["dropdown"];
			dropdown.put(new VerticalLayout());

			std::vector<Player> colors = getPlayers(PLAYER_MAX);
			colors.emplace_back(Player::NONE);
			for (auto& color : colors)
			{
				std::string tagname = std::string(stringify(color));
				std::string name;
				switch (color)
				{
					case Player::RED:    name = _("red");    break;
					case Player::BLUE:   name = _("blue");   break;
					case Player::TEAL:   name = _("teal");   break;
					case Player::YELLOW: name = _("yellow"); break;
					case Player::PINK:   name = _("pink");   break;
					case Player::BLACK:  name = _("black");  break;
					case Player::INDIGO: name = _("indigo"); break;
					case Player::PURPLE: name = _("purple"); break;
					default:             name = _("auto");   break;
				}
				Paint textcolor = ColorName::TEXT100;
				Paint uicolor = ColorName::FRAMETRANSPARENTITEM;
				switch (color)
				{
					case Player::RED:    uicolor = ColorName::RED;    break;
					case Player::BLUE:   uicolor = ColorName::BLUE;   break;
					case Player::TEAL:   uicolor = ColorName::TEAL;   break;
					case Player::YELLOW: uicolor = ColorName::YELLOW; break;
					case Player::PINK:   uicolor = ColorName::PINK;   break;
					case Player::BLACK:  uicolor = ColorName::BLACK;  break;
					case Player::INDIGO: uicolor = ColorName::INDIGO; break;
					case Player::PURPLE: uicolor = ColorName::PURPLE; break;
					default: textcolor = ColorName::TEXT800; break;
				}
				content.add(tagname, Frame::makeItem(uicolor));
				content[tagname].put(new TextField(name, FONTSIZE_MENUBUTTON,
					textcolor));
				content[tagname].align(HorizontalAlignment::CENTER);
				content[tagname].makeClickable();
				dropdown.add(tagname, Frame::makeItem(uicolor));
				dropdown[tagname].put(new TextField(name, FONTSIZE_MENUBUTTON,
					textcolor));
				dropdown[tagname].align(HorizontalAlignment::CENTER);
				dropdown[tagname].makeClickable();
			}
			content.settleWidth();
			content.settleHeight();
			content.fixWidth();
			content.setTag(stringify(Player::NONE));

			dropdown.settleWidth();
			dropdown.settleHeight();
			dropdown.fixWidth();
		}
		player["color"].setMarginHorizontal(
			(InterfaceElement::windowW() < 600 * InterfaceElement::scale())
			? 2 * InterfaceElement::scale()
			: 8 * InterfaceElement::scale());

		player.add("more", new TooltipLayout(/*dropdown=*/true));
		{
			auto& options = player["more"];

			options.add("content", makeWindowbutton());
			auto& content = options["content"];
			content.setTag("Dots");

			options.add("dropdown", Frame::makeTooltip(/*dropdown=*/true));
			auto& dropdown = options["dropdown"];
			dropdown.put(new VerticalLayout());
			{
				std::string tagname = "observer";
				dropdown.add(tagname, Frame::makeItem());
				dropdown[tagname].put(new TextField(
					_("Move to observers"),
					FONTSIZE_MENUBUTTON));
				dropdown[tagname].align(HorizontalAlignment::LEFT);
				dropdown[tagname].makeClickable();
			}
			dropdown.settleWidth();
			dropdown.settleHeight();
			dropdown.fixWidth();
		}
		player["more"].setMarginLeft(
			(InterfaceElement::windowW() < 600 * InterfaceElement::scale())
			? 2 * InterfaceElement::scale()
			: 8 * InterfaceElement::scale());
		player["more"].setMarginRight(
			(InterfaceElement::windowW() < 600 * InterfaceElement::scale())
			? 5 * InterfaceElement::scale()
			: 8 * InterfaceElement::scale());
		player.align(VerticalAlignment::MIDDLE);
		player.settleWidth();
		player.settleHeight();
		players.add(username, std::move(element));
	}

	// Enable the start game button for now;
	// it will disabled if we do not have enough players.
	_layout["left"]["inlobby"]["buttons"]["start"]["it"].enable();
	_layout["left"]["inlobby"]["buttons"]["start"]["tooltip"].kill(1);

	// reset width of lobby to set width of textfield (for clicking)
	players.setWidth(
		players.width());
	players.place(
		players.topleft());

	for (int i = 0; i < addbots - 1; i++)
	{
		addAddBot();
	}
}

void MultiplayerMenu::addObserver(const std::string& username, bool isSelf)
{
	const int FONTSIZE = _settings.getFontSize();
	const int FONTSIZE_MENUBUTTON = _settings.getFontSizeMenuButton();

	auto& observers = _layout["left"]["inlobby"]["observers"];
	if (observers.contains(username)) return;

	{
		auto& players = _layout["left"]["inlobby"]["players"]["players"];
		if (players.contains(username)) return;
	}

	{
		std::unique_ptr<InterfaceElement> element(new ClickAndDrag(true));
		auto& observer = *element;
		observer.put(Frame::makeItem(Paint::alpha(ColorName::FRAMESAND, 100)));
		observer.content().put(new HorizontalLayout());
		observer.add("name",
			new TextField(username, FONTSIZE,
			isSelf ? ColorName::TEXTOWNNAME : ColorName::TEXT900));
		observer["name"].setMarginHorizontal(
			(InterfaceElement::windowW() < 600 * InterfaceElement::scale())
			? 2 * InterfaceElement::scale()
			: 8 * InterfaceElement::scale());
		observer.add("more", new TooltipLayout(/*dropdown=*/true));
		{
			auto& options = observer["more"];

			options.add("content", makeWindowbutton());
			auto& content = options["content"];
			content.setTag("Dots");

			options.add("dropdown", Frame::makeTooltip(/*dropdown=*/true));
			auto& dropdown = options["dropdown"];
			dropdown.put(new VerticalLayout());
			{
				std::string tagname = "player";
				dropdown.add(tagname, Frame::makeItem());
				dropdown[tagname].put(new TextField(
					_("Move to players"),
					FONTSIZE_MENUBUTTON));
				dropdown[tagname].align(HorizontalAlignment::LEFT);
				dropdown[tagname].makeClickable();
			}
			dropdown.settleWidth();
			dropdown.settleHeight();
			dropdown.fixWidth();
		}
		observer["more"].setMarginLeft(
			(InterfaceElement::windowW() < 600 * InterfaceElement::scale())
			? 2 * InterfaceElement::scale()
			: 8 * InterfaceElement::scale());
		observer["more"].setMarginRight(
			(InterfaceElement::windowW() < 600 * InterfaceElement::scale())
			? 5 * InterfaceElement::scale()
			: 8 * InterfaceElement::scale());
		observer.align(VerticalAlignment::MIDDLE);
		observer.settleWidth();
		observer.settleHeight();
		observers.add(username, std::move(element));
	}

	// reset width of lobby to set width of textfield (for clicking)
	observers.setWidth(observers.width());
	observers.place(observers.topleft());
}

void MultiplayerMenu::addBot(const std::string& botslot)
{
	const int FONTSIZE_MENUBUTTON = _settings.getFontSizeMenuButton();

	auto& players = _layout["left"]["inlobby"]["players"]["players"];

	int addbots = 0;
	for (int i = 1; i <= (int) PLAYER_MAX; i++)
	{
		std::string addbotslot = "%""addbot" + std::to_string(i);
		if (players.contains(addbotslot))
		{
			players.remove(addbotslot);
			addbots++;
		}
	}

	players.add(botslot, new Padding());
	{
		auto& player = players[botslot];
		player.put(new HorizontalLayout());
		player.setPaddingHorizontal(
			(InterfaceElement::windowW() < 600 * InterfaceElement::scale())
			? 0 * InterfaceElement::scale()
			: 3 * InterfaceElement::scale());
		player.add("ainame", new TooltipLayout(/*dropdown=*/true));
		{
			auto& options = player["ainame"];

			options.add("content", new SlideshowLayout());
			auto& content = options["content"];

			options.add("dropdown", Frame::makeTooltip(/*dropdown=*/true));
			auto& dropdown = options["dropdown"];
			dropdown.put(new VerticalLayout());

			for (size_t i = 0; i < _ainames.size(); i++)
			{
				std::string ainame = _ainames[i];
				std::string desc = _aidescriptions[i];
				content.add(ainame, Frame::makeItem());
				content[ainame].put(new TextField(ainame, FONTSIZE_MENUBUTTON,
					ColorName::TEXT800));
				content[ainame].makeClickable();
				dropdown.add(ainame, Frame::makeItem());
				dropdown[ainame].put(new TextField(desc, FONTSIZE_MENUBUTTON,
					ColorName::TEXT800));
				dropdown[ainame].makeClickable();
			}

			content.settleWidth();
			content.settleHeight();
			content.fixWidth();

			dropdown.settleWidth();
			dropdown.settleHeight();
			dropdown.fixWidth();
		}
		player["ainame"].setMarginRight(
			(InterfaceElement::windowW() < 600 * InterfaceElement::scale())
			? 2 * InterfaceElement::scale()
			: 4 * InterfaceElement::scale());

		player.add("filler", new Filler());

		player.add("difficulty", new TooltipLayout(/*dropdown=*/true));
		{
			auto& options = player["difficulty"];

			options.add("content", new SlideshowLayout());
			auto& content = options["content"];

			options.add("dropdown", Frame::makeTooltip(/*dropdown=*/true));
			auto& dropdown = options["dropdown"];
			dropdown.put(new VerticalLayout());

			for (Difficulty difficulty : {Difficulty::EASY,
				Difficulty::MEDIUM, Difficulty::HARD})
			{
				std::string tagname = stringify(difficulty);
				std::string name;
				switch (difficulty)
				{
					// TRANSLATORS: This is a difficulty.
					case Difficulty::EASY:   name = _("easy");    break;
					// TRANSLATORS: This is a difficulty.
					case Difficulty::MEDIUM: name = _("medium");  break;
					// TRANSLATORS: This is a difficulty.
					case Difficulty::HARD:   name = _("hard");    break;

					case Difficulty::NONE:
					break;
				}
				content.add(tagname, Frame::makeItem());
				content[tagname].put(new TextField(name,
					FONTSIZE_MENUBUTTON, ColorName::TEXT800));
				content[tagname].align(HorizontalAlignment::CENTER);
				content[tagname].makeClickable();
				dropdown.add(tagname, Frame::makeItem());
				dropdown[tagname].put(new TextField(name,
					FONTSIZE_MENUBUTTON, ColorName::TEXT800));
				dropdown[tagname].align(HorizontalAlignment::CENTER);
				dropdown[tagname].makeClickable();
			}

			content.settleWidth();
			content.settleHeight();
			content.fixWidth();

			dropdown.settleWidth();
			dropdown.settleHeight();
			dropdown.fixWidth();
		}
		player["difficulty"].setMarginHorizontal(
			(InterfaceElement::windowW() < 600 * InterfaceElement::scale())
			? 2 * InterfaceElement::scale()
			: 4 * InterfaceElement::scale());

		player.add("visiontype", new TooltipLayout(/*dropdown=*/true));
		{
			auto& options = player["visiontype"];

			options.add("content", new SlideshowLayout());
			auto& content = options["content"];

			options.add("dropdown", Frame::makeTooltip(/*dropdown=*/true));
			auto& dropdown = options["dropdown"];
			dropdown.put(new VerticalLayout());

			for (auto& type : {VisionType::NORMAL, VisionType::GLOBAL})
			{
				std::string tagname = std::string(stringify(type));
				std::string sname;
				std::string name;
				switch (type)
				{
					case VisionType::NORMAL:
					{
						sname = _("normal");
						name = _("normal vision");
					}
					break;
					case VisionType::GLOBAL:
					{
						sname = _("global");
						name = _("global vision");
					}
					break;
					case VisionType::NONE:
					break;
				}
				Paint textcolor = ColorName::TEXT800;
				content.add(tagname, Frame::makeItem());
				content[tagname].put(new TextField(sname,
					FONTSIZE_MENUBUTTON, textcolor));
				content[tagname].align(HorizontalAlignment::CENTER);
				content[tagname].makeClickable();
				dropdown.add(tagname, Frame::makeItem());
				dropdown[tagname].put(new TextField(name,
					FONTSIZE_MENUBUTTON, textcolor));
				dropdown[tagname].align(HorizontalAlignment::CENTER);
				dropdown[tagname].makeClickable();
			}
			content.settleWidth();
			content.settleHeight();
			content.fixWidth();
			content.setTag(stringify(VisionType::NORMAL));

			dropdown.settleWidth();
			dropdown.settleHeight();
			dropdown.fixWidth();
		}
		player["visiontype"].setMarginHorizontal(
			(InterfaceElement::windowW() < 600 * InterfaceElement::scale())
			? 2 * InterfaceElement::scale()
			: 8 * InterfaceElement::scale());

		player.add("color", new TooltipLayout(/*dropdown=*/true));
		{
			auto& options = player["color"];

			options.add("content", new SlideshowLayout());
			auto& content = options["content"];

			options.add("dropdown", Frame::makeTooltip(/*dropdown=*/true));
			auto& dropdown = options["dropdown"];
			dropdown.put(new VerticalLayout());

			std::vector<Player> colors = getPlayers(PLAYER_MAX);
			colors.emplace_back(Player::NONE);
			for (auto& color : colors)
			{
				std::string tagname = std::string(stringify(color));
				std::string name;
				switch (color)
				{
					case Player::RED:    name = _("red");    break;
					case Player::BLUE:   name = _("blue");   break;
					case Player::TEAL:   name = _("teal");   break;
					case Player::YELLOW: name = _("yellow"); break;
					case Player::PINK:   name = _("pink");   break;
					case Player::BLACK:  name = _("black");  break;
					case Player::INDIGO: name = _("indigo"); break;
					case Player::PURPLE: name = _("purple"); break;
					default:             name = _("auto");   break;
				}
				Paint textcolor = ColorName::TEXT100;
				Paint uicolor = ColorName::FRAMETRANSPARENTITEM;
				switch (color)
				{
					case Player::RED:    uicolor = ColorName::RED;    break;
					case Player::BLUE:   uicolor = ColorName::BLUE;   break;
					case Player::TEAL:   uicolor = ColorName::TEAL;   break;
					case Player::YELLOW: uicolor = ColorName::YELLOW; break;
					case Player::PINK:   uicolor = ColorName::PINK;   break;
					case Player::BLACK:  uicolor = ColorName::BLACK;  break;
					case Player::INDIGO: uicolor = ColorName::INDIGO; break;
					case Player::PURPLE: uicolor = ColorName::PURPLE; break;
					default: textcolor = ColorName::TEXT800; break;
				}
				content.add(tagname, Frame::makeItem(uicolor));
				content[tagname].put(new TextField(name,
					FONTSIZE_MENUBUTTON, textcolor));
				content[tagname].align(HorizontalAlignment::CENTER);
				content[tagname].makeClickable();
				dropdown.add(tagname, Frame::makeItem(uicolor));
				dropdown[tagname].put(new TextField(name,
					FONTSIZE_MENUBUTTON, textcolor));
				dropdown[tagname].align(HorizontalAlignment::CENTER);
				dropdown[tagname].makeClickable();
			}
			content.settleWidth();
			content.settleHeight();
			content.fixWidth();
			content.setTag(stringify(Player::NONE));

			dropdown.settleWidth();
			dropdown.settleHeight();
			dropdown.fixWidth();
		}
		player["color"].setMarginHorizontal(
			(InterfaceElement::windowW() < 600 * InterfaceElement::scale())
			? 2 * InterfaceElement::scale()
			: 8 * InterfaceElement::scale());
		player.add("more", new TooltipLayout(/*dropdown=*/true));
		{
			auto& options = player["more"];

			options.add("content", makeWindowbutton());
			auto& content = options["content"];
			content.setTag("Dots");

			options.add("dropdown", Frame::makeTooltip(/*dropdown=*/true));
			auto& dropdown = options["dropdown"];
			dropdown.put(new VerticalLayout());
			{
				std::string tagname = "removebot";
				dropdown.add(tagname, Frame::makeItem());
				dropdown[tagname].put(new TextField(
					_("Remove AI player"),
					FONTSIZE_MENUBUTTON));
				dropdown[tagname].align(HorizontalAlignment::LEFT);
				dropdown[tagname].makeClickable();
			}
			{
				std::string tagname = "copybot";
				dropdown.add(tagname, Frame::makeItem());
				dropdown[tagname].put(new TextField(
					_("Copy AI player"),
					FONTSIZE_MENUBUTTON));
				dropdown[tagname].align(HorizontalAlignment::LEFT);
				dropdown[tagname].makeClickable();
			}
			dropdown.settleWidth();
			dropdown.settleHeight();
			dropdown.fixWidth();
		}
		player["more"].setMarginLeft(
			(InterfaceElement::windowW() < 600 * InterfaceElement::scale())
			? 2 * InterfaceElement::scale()
			: 8 * InterfaceElement::scale());
		player["more"].setMarginRight(
			(InterfaceElement::windowW() < 600 * InterfaceElement::scale())
			? 5 * InterfaceElement::scale()
			: 8 * InterfaceElement::scale());
		player.align(VerticalAlignment::MIDDLE);
		player.settleWidth();
		player.settleHeight();
	}

	// Enable the start game button for now;
	// it will disabled if we do not have enough players.
	_layout["left"]["inlobby"]["buttons"]["start"]["it"].enable();
	_layout["left"]["inlobby"]["buttons"]["start"]["tooltip"].kill(1);

	// reset width of lobby to set width of textfield (for clicking)
	players.setWidth(
		players.width());
	players.place(
		players.topleft());

	for (int i = 0; i < addbots - 1; i++)
	{
		addAddBot();
	}
}

void MultiplayerMenu::removeBot(const std::string& botslot)
{
	auto& players = _layout["left"]["inlobby"]["players"]["players"];
	if (players.contains(botslot))
	{
		players.remove(botslot);
		addAddBot();
	}
}

void MultiplayerMenu::addAddBot()
{
	const int FONTSIZE_MENUBUTTON = _settings.getFontSizeMenuButton();

	auto& players = _layout["left"]["inlobby"]["players"]["players"];

	int i = 1;
	std::string addbotslot = "%""addbot" + std::to_string(i);
	while (players.contains(addbotslot))
	{
		i++;
		if (i > (int) PLAYER_MAX) return;
		addbotslot = "%""addbot" + std::to_string(i);
	}
	players.add(addbotslot,
		new Padding());
	{
		auto& player = players[addbotslot];
		player.put(new HorizontalLayout());
		player.setPaddingHorizontal(2 * InterfaceElement::scale());
		player.add("filler", new Filler());
		player.add("more", new TooltipLayout(/*dropdown=*/true));
		{
			auto& options = player["more"];

			options.add("content", makeWindowbutton());
			auto& content = options["content"];
			content.setTag("Plus");

			options.add("dropdown", Frame::makeTooltip(/*dropdown=*/true));
			auto& dropdown = options["dropdown"];
			dropdown.put(new VerticalLayout());
			{
				std::string tagname = "addbot";
				dropdown.add(tagname, Frame::makeItem());
				dropdown[tagname].put(new TextField(
					_("Add AI player"),
					FONTSIZE_MENUBUTTON));
				dropdown[tagname].align(HorizontalAlignment::LEFT);
				dropdown[tagname].makeClickable();
			}
			dropdown.settleWidth();
			dropdown.settleHeight();
			dropdown.fixWidth();
		}
		player["more"].setMarginLeft(
			(InterfaceElement::windowW() < 600 * InterfaceElement::scale())
			? 2 * InterfaceElement::scale()
			: 8 * InterfaceElement::scale());
		player["more"].setMarginRight(
			(InterfaceElement::windowW() < 600 * InterfaceElement::scale())
			? 5 * InterfaceElement::scale()
			: 8 * InterfaceElement::scale());
		player.align(VerticalAlignment::MIDDLE);
		player.settleWidth();
		player.settleHeight();

		// A little hack to detect OneVsOne lobbies.
		auto& settings = _layout["left"]["inlobby"]["settings"];
		if (!settings["planningtime"].enabled())
		{
			player.kill(1);
		}
	}

	// Disable the start game button, because we do not have enough players.
	_layout["left"]["inlobby"]["buttons"]["start"]["it"].disable();
	_layout["left"]["inlobby"]["buttons"]["start"]["tooltip"].bear(1);

	// reset width of lobby to set width of textfield (for clicking)
	players.setWidth(
		players.width());
	players.place(
		players.topleft());
}

void MultiplayerMenu::assignRole(const std::string& username, const Role& role,
		bool isSelf)
{
	auto& players = _layout["left"]["inlobby"]["players"]["players"];

	if (players.contains(username))
	{
		players.remove(username);
		addAddBot();
	}
	if (_layout["left"]["inlobby"]["observers"].contains(username))
	{
		_layout["left"]["inlobby"]["observers"].remove(username);
	}

	switch (role)
	{
		case Role::NONE:                                    break;
		case Role::PLAYER:   addPlayer(username, isSelf);   break;
		case Role::OBSERVER: addObserver(username, isSelf); break;
	}
}

void MultiplayerMenu::assignColor(const std::string& username, const Player& color)
{
	auto& players = _layout["left"]["inlobby"]["players"]["players"];
	if (players.contains(username))
	{
		auto& content = players[username]["color"]["content"];
		std::string colorname = stringify(color);
		if (content.contains(colorname) && content.getTag() != colorname)
		{
			content.setTag(colorname);
		}
	}
}
void MultiplayerMenu::assignVisionType(const std::string& username, const VisionType& type)
{
	auto& players = _layout["left"]["inlobby"]["players"]["players"];
	if (players.contains(username))
	{
		auto& content = players[username]["visiontype"]["content"];
		std::string tagname = stringify(type);
		if (content.contains(tagname) && content.getTag() != tagname)
		{
			content.setTag(tagname);
		}
	}
}

void MultiplayerMenu::assignAI(const std::string& bot,
	const std::string& ainame)
{
	auto& players = _layout["left"]["inlobby"]["players"]["players"];
	if (players.contains(bot) && players[bot].contains("ainame"))
	{
		auto& content = players[bot]["ainame"]["content"];
		if (content.contains(ainame) && content.getTag() != ainame)
		{
			content.setTag(ainame);
		}
	}
}

void MultiplayerMenu::assignDifficulty(const std::string& bot,
	Difficulty difficulty)
{
	auto& players = _layout["left"]["inlobby"]["players"]["players"];
	if (players.contains(bot) && players[bot].contains("difficulty"))
	{
		auto& content = players[bot]["difficulty"]["content"];
		std::string difficultyName = stringify(difficulty);
		if (content.contains(difficultyName)
			&& content.getTag() != difficultyName)
		{
			content.setTag(difficultyName);
		}
	}
}

void MultiplayerMenu::pickMap(const std::string& mapname)
{
	auto& options = _layout["left"]["inlobby"]["settings"]["map"]["options"];
	auto& preview = _layout["left"]["inlobby"]["observers"];
	auto& content = options["content"];
	if (content.contains(mapname))
	{
		content.setTag(mapname);

		std::string picturename = "panels/" + mapname;
		preview.setPicture(picturename);
		preview.setColor(0, Paint::alpha(ColorName::FRAME200, 192));

		_owner.getPicture(picturename);
	}
	else
	{
		LOGE << "Missing from options";
		DEBUG_ASSERT(false);
	}
}

void MultiplayerMenu::pickTimer(uint32_t timer)
{
	becomeGameLobby();
	auto& options = _layout["left"]["inlobby"]["settings"]["planningtime"]["options"];
	auto& content = options["content"];
	std::string name = std::to_string(timer);
	if (content.contains(name))
	{
		content.setTag(name);
	}
	else
	{
		LOGE << "Missing from options";
		DEBUG_ASSERT(false);
	}
}

void MultiplayerMenu::pickReplay(const std::string& replayname)
{
	auto& replays = _layout["left"]["inlobby"]["players"]["replays"];
	if (replays.contains(replayname))
	{
		replays.depower();
		replays[replayname].power();
	}
	else
	{
		LOGE << "Missing from replays";
		DEBUG_ASSERT(false);
	}
}

void MultiplayerMenu::listMap(const std::string& mapname,
	const Json::Value& metadata)
{
	const int FONTSIZE_MENUBUTTON = _settings.getFontSizeMenuButton();

	becomeGameLobby();
	auto& options = _layout["left"]["inlobby"]["settings"]["map"]["options"];
	auto& content = options["content"];
	auto& dropdown = options["dropdown"];

	if (content.contains(mapname)) return;

	bool usermade = (mapname.find_first_of('/') != std::string::npos);
	bool custom = usermade;
	PoolType pooltype = PoolType::NONE;
	if (metadata["pool"].isString())
	{
		try
		{
			pooltype = parsePoolType(metadata["pool"].asString());
		}
		catch (ParseError& error)
		{
			LOGE << "Failed to parse pooltype: " << error.what();
			RETHROW_IF_DEV();
		}
	}

	switch (pooltype)
	{
		case PoolType::NONE:
		{
			LOGE << "Map '" << mapname << "' has no pooltype";
			DEBUG_ASSERT(false && pooltype != PoolType::NONE);
			custom = true;
		}
		break;
		case PoolType::MULTIPLAYER:
		break;
		case PoolType::CUSTOM:
		{
			custom = true;
		}
		break;
		case PoolType::DIORAMA:
		break;
	}

	std::string description = mapname;
	int playercount = (metadata["playercount"].isInt()) ?
		metadata["playercount"].asInt() : 2;
	if (usermade)
	{
		size_t seppos = mapname.find_first_of("/");
		description = "(" + std::to_string(playercount) + ") "
			+ ::format(
				// TRANSLATORS: The first argument is the name of a custom map
				// or AI, the second argument is the name of its creator.
				_("\"%s\" by %s"),
				mapname.substr(seppos + 1).c_str(),
				mapname.substr(0, seppos).c_str());
	}
	else
	{
		description = "(" + std::to_string(playercount) + ") " + mapname;
		if (custom)
		{
			description += "*";
		}
	}

	content.add(mapname, Frame::makeItem());
	content[mapname].put(
		new TextField(description, FONTSIZE_MENUBUTTON, ColorName::TEXT900));
	content[mapname].makeClickable();
	dropdown.add(mapname, Frame::makeItem());
	dropdown[mapname].put(
		new TextField(description, FONTSIZE_MENUBUTTON, ColorName::TEXT900));
	dropdown[mapname].makeClickable();

	if (content.contains("unknown"))
	{
		content.remove("unknown");
		dropdown.remove("unknown");
		content.setTag(mapname);
	}

	options.settleWidth();
	options.settleHeight();
	options.place(options.topleft());
	_layout["left"]["inlobby"]["settings"]["map"].fixWidth();
	dropdown.settleWidth();
	dropdown.setWidth(dropdown.width());

	if (custom)
	{
		if (_layout["left"]["inlobby"]["settings"].contains("custom"))
		{
			auto& checkbox = _layout["left"]["inlobby"]["settings"]["custom"];
			checkbox["options"].setTag("Checked");
			checkbox["options"].disable();
		}
		else
		{
			LOGW << "Missing 'custom' checkbox";
		}
	}
}

void MultiplayerMenu::listReplay(const std::string& replayname, const Json::Value& metadata)
{
	const int FONTSIZE = _settings.getFontSize();
	const int FONTSIZE_MENUBUTTON = _settings.getFontSizeMenuButton();

	becomeReplayLobby();
	auto& replays = _layout["left"]["inlobby"]["players"]["replays"];

	if (replays.contains(replayname)) return;

	replays.add(replayname, Frame::makeItem());
	{
		auto& replay = replays[replayname];
		// The description and timestamp are not translated because they are
		// generated based on metadata and just a little too unpredictable.
		std::string description;
		std::string timestamp;
		if (!metadata.isNull())
		{
			std::stringstream strm;
			std::string separator = "";
			if (metadata["players"].isArray())
			{
				for (const Json::Value& player : metadata["players"])
				{
					strm << separator;
					if (player["username"].isString()) strm << player["username"].asString();
					else strm << "Player";
					separator = " vs. ";
				}
			}
			if (metadata["bots"].isArray())
			{
				for (const Json::Value& player : metadata["bots"])
				{
					strm << separator;
					if (player["ainame"].isString()) strm << player["ainame"].asString();
					else strm << "Bot";
					separator = " vs. ";
				}
			}
			if (metadata["map"].isString()) strm << " on " << metadata["map"].asString();
			description = strm.str();

			if (metadata["starttime"].isInt64())
			{
				uint64_t starttime = metadata["starttime"].asInt64();
				time_t localtime = std::chrono::system_clock::to_time_t(
					std::chrono::system_clock::time_point(std::chrono::seconds(starttime)));
				timestamp = std::asctime(std::localtime(&localtime));
				timestamp.resize(timestamp.size() - 1);
			}
		}
		else description = "Unknown replay " + replayname + "";
		replay.put(new VerticalLayout());
		replay.add("desc", new TextField(description, FONTSIZE_MENUBUTTON));
		replay.add("timestamp", new TextField(timestamp, FONTSIZE, ColorName::TEXT700));
		replay["timestamp"].align(HorizontalAlignment::RIGHT);
		replay.settleHeight();
		replay.makeClickable();
	}

	replays.setWidth(replays.width());
	replays.place(replays.topleft());
}

void MultiplayerMenu::listRuleset(const std::string& rulesetname,
		const Json::Value& /**/)
{
	// Do we have this ruleset in our library, either cached or saved?
	if (!Library::existsBible(rulesetname))
	{
		_client.send(Message::ruleset_request(rulesetname));
	}
	else
	{
		// Confirm that we have the ruleset.
		_client.send(Message::list_ruleset(rulesetname));
	}
}

void MultiplayerMenu::listAI(const std::string& ainame,
	const Json::Value& metadata)
{
	if (std::find(_ainames.begin(), _ainames.end(), ainame) == _ainames.end())
	{
		std::string desc;
		if (metadata["authors"].isString())
		{
			desc = ::format(
				// TRANSLATORS: The first argument is the name of a custom map
				// or AI, the second argument is the name of its creator.
				_("\"%s\" by %s"),
				ainame.c_str(),
				metadata["authors"].asString().c_str());
		}
		else
		{
			desc = ainame;
		}

		_ainames.emplace_back(ainame);
		_aidescriptions.emplace_back(desc);

		const int FONTSIZE_MENUBUTTON = _settings.getFontSizeMenuButton();
		auto& players = _layout["left"]["inlobby"]["players"]["players"];
		for (size_t i = 0; i < players.size(); i++)
		{
			std::string name = players.name(i);
			if (players[name].contains("ainame"))
			{
				auto& options = players[name]["ainame"];
				auto& content = options["content"];
				auto& dropdown = options["dropdown"];

				content.add(ainame, Frame::makeItem());
				content[ainame].put(new TextField(ainame, FONTSIZE_MENUBUTTON,
					ColorName::TEXT800));
				content[ainame].makeClickable();
				dropdown.add(ainame, Frame::makeItem());
				dropdown[ainame].put(new TextField(desc, FONTSIZE_MENUBUTTON,
					ColorName::TEXT800));
				dropdown[ainame].makeClickable();

				content.unfixWidth();
				content.settleWidth();
				content.settleHeight();
				content.fixWidth();

				dropdown.unfixWidth();
				dropdown.settleWidth();
				dropdown.settleHeight();
				dropdown.fixWidth();

				players[name].setWidth(players[name].width());
				players[name].place(players[name].topleft());
			}
		}
	}
}

void MultiplayerMenu::listChallenge(const std::string& /**/,
		const Json::Value& metadata)
{
	auto& element = _layout["left"]["browser"]["panels1"]["challenge"];
	if (metadata["display-name"].isString())
	{
		std::string displayname = metadata["display-name"].asString();
		element["title"].setText(::format(
			// TRANSLATORS: The argument is the name of a challenge.
			_("%s Challenge"),
			GETTEXT_FROM_SERVER(displayname.c_str()).c_str()));
	}
	if (metadata["panel-picture-name"].isString())
	{
		std::string picturename = metadata["panel-picture-name"].asString();
		element.setPicture(picturename);
		_owner.getPicture(picturename);
	}
}

void MultiplayerMenu::requestFulfilled(const std::string& filename)
{
	std::string picturename = Locator::pictureName(filename);
	if (!picturename.empty())
	{
		_layout.checkPicture(picturename);
		return;
	}

	std::string rulesetname = Locator::rulesetName(filename);
	if (!rulesetname.empty())
	{
		// Confirm that we have the ruleset.
		_client.send(Message::list_ruleset(rulesetname));
		return;
	}
}

void MultiplayerMenu::updateOwnRating(float rating)
{
	message(::format(
		// TRANSLATORS: The argument is a player's MMR rating.
		_("Your rating is now %s."),
		formatRating(rating).c_str()));
}

void MultiplayerMenu::updateRating(const std::string& name, float rating)
{
	auto& users = _layout["right"]["users"]["users"]["list"];
	if (users.contains(name) && users[name].contains("rating"))
	{
		users[name]["rating"].setText(formatRating(rating));
	}
}

void MultiplayerMenu::updateStars(const std::string& name, int stars)
{
	auto& users = _layout["right"]["users"]["users"]["list"];
	if (users.contains(name) && users[name].contains("stars"))
	{
		users[name]["stars"].setText("x" + std::to_string(stars));
	}
}

void MultiplayerMenu::updateRecentStars(int stars)
{
	auto& element = _layout["left"]["browser"]["panels1"]["challenge"];
	for (int i = 1; i <= std::min(stars, 3); i++)
	{
		std::string index = std::to_string(i);
		auto& icon = element["stars"][index];
		icon.power();
	}
}

void MultiplayerMenu::displayRankings(const std::vector<Ranking>& rankings)
{
	const int FONTSIZE = _settings.getFontSize();

	if (rankings.empty()) return;

	auto& list = _layout["right"]["users"]["rankings"];
	list.reset();

	for (const Ranking& ranking : rankings)
	{
		std::unique_ptr<InterfaceElement> it(new HorizontalLayout());
		auto& element = *it;
		element.add("rank", new TextField("10", FONTSIZE, ColorName::TEXT700));
		element.add("username", new TextField(ranking.name, FONTSIZE));
		element.add("rating", new TextField("100.0", FONTSIZE));
		element.settle();
		element["rank"].fixWidth();
		element["rating"].fixWidth();
		element["rank"].setText(std::to_string(ranking.rank));
		element["rating"].setText(formatRating(ranking.rating));
		element["rank"].setMargin(4 * InterfaceElement::scale());
		element["username"].setMargin(4 * InterfaceElement::scale());
		element["rating"].setMargin(4 * InterfaceElement::scale());
		element["rank"].align(HorizontalAlignment::CENTER);
		element["rating"].align(HorizontalAlignment::CENTER);
		list.add(ranking.name, std::move(it));
	}

	list.setWidth(list.width());
	list.setHeight(list.height());
	list.fixWidth();
	list.fixHeight();
	list.place(list.topleft());
}

void MultiplayerMenu::becomeGameLobby()
{
	if (_layout["left"]["inlobby"]["players"].getTag() == "players") return;
	_layout["left"]["inlobby"]["t_players"].setText("PLAYERS");
	_layout["left"]["inlobby"]["players"].setTag("players");

	_layout["left"]["inlobby"]["buttons"]["start"]["it"].setText(
		_("start game"));
	auto& settings = _layout["left"]["inlobby"]["settings"];
	if (!settings.contains("map")) settings.add("map", makeMapDropdown());
	if (!settings.contains("planningtime")) settings.add("planningtime", makeTimerDropdown());
	settings.settleWidth();
	straightenLobbySettings();
	settings.settleHeight();
	_layout["left"].setWidth(_layout["left"].width());
	_layout["left"].setHeight(_layout["left"].height());
	_layout["left"].place(_layout["left"].topleft());
}

void MultiplayerMenu::becomeReplayLobby()
{
	if (_layout["left"]["inlobby"]["players"].getTag() == "replays") return;
	_layout["left"]["inlobby"]["t_players"].setText("REPLAYS");
	_layout["left"]["inlobby"]["players"].setTag("replays");

	auto& preview = _layout["left"]["inlobby"]["observers"];
	preview.setPicture("");
	preview.setColor(0, ColorName::FRAME200);

	_layout["left"]["inlobby"]["buttons"]["start"]["it"].setText(
		_("start replay"));
	auto& settings = _layout["left"]["inlobby"]["settings"];
	if (settings.contains("map")) settings.remove("map");
	if (settings.contains("planningtime")) settings.remove("planningtime");
	settings.settleWidth();
	straightenLobbySettings();
	settings.settleHeight();
	_layout["left"].setWidth(_layout["left"].width());
	_layout["left"].setHeight(_layout["left"].height());
	_layout["left"].place(_layout["left"].topleft());
}

void MultiplayerMenu::restrictLobbySettingsForOneVsOne()
{
	auto& players = _layout["left"]["inlobby"]["players"]["players"];
	for (size_t i = 0; i < players.size(); i++)
	{
		std::string name = players.name(i);
		if (name.compare(0, 7, "%""addbot") == 0)
		{
			players[name].kill(1);
		}
		else if (players[name].contains("visiontype"))
		{
			players[name]["visiontype"].kill(1);
		}
	}

	auto& settings = _layout["left"]["inlobby"]["settings"];
	if (settings.contains("custom"))
	{
		settings["custom"].disable();
	}
	if (settings.contains("map"))
	{
		auto& options = settings["map"]["options"];
		auto& content = options["content"];
		auto& dropdown = options["dropdown"];
		std::vector<std::string> removals;
		for (size_t i = 0; i < content.size(); i++)
		{
			std::string name = content.name(i);
			if (name.find("1v1") == std::string::npos)
			{
				removals.push_back(name);
			}
		}
		for (const std::string& name : removals)
		{
			content.remove(name);
			dropdown.remove(name);
		}
		options.settleWidth();
		options.settleHeight();
		options.place(options.topleft());
		settings["map"].fixWidth();
	}
	if (settings.contains("planningtime"))
	{
		settings["planningtime"].disable();
	}
}

void MultiplayerMenu::identified()
{
	_client.join();
}

void MultiplayerMenu::sessionFailed(const ResponseStatus&)
{
	quit();
}

void MultiplayerMenu::sendingFeedback()
{
	// TODO do something?
}

void MultiplayerMenu::sentFeedback(const std::string& link)
{
	InterfaceElement& form = _layout["right"]["users"]["feedback"];

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

void MultiplayerMenu::feedbackFailed(const ResponseStatus& responsestatus)
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
	InterfaceElement& form =_layout["right"]["users"]["feedback"];
	form["error"]["texts"].fixWidth();
	form["error"]["texts"].setText(errortext);
	form["error"].bear();
	form["error"].settle();
	form["info"].kill();
	form.settle();
}

void MultiplayerMenu::inServer()
{
	_layout["left"].setTag("browser");
	_layout["left"]["browser"]["panels2"]["onevsone"].enable();
	_layout["left"]["browser"]["panels2"]["create"].enable();
	_layout["left"]["browser"]["panels2"]["versusai"].enable();
	_layout["left"]["browser"]["buttons"]["replay"].enable();
	_layout["left"]["browser"]["panels1"]["tutorial"].enable();
	_layout["left"]["browser"]["panels1"]["challenge"].enable();
	_layout["left"]["browser"]["buttons"]["overview"].enable();
	_layout["left"]["browser"]["buttons"]["guide"].enable();
	_inputMode = InputMode::CHAT_GENERAL;
	_layout["right"]["inputline"]["indicator"]["button"].setText(
		_("ALL"));
	_layout["right"]["inputline"]["indicator"]["button"].setTextColor(ColorName::TEXT800);
	_layout["right"]["inputline"]["input"].enable();
	_layout["right"]["inputline"]["input"].content().power();
	_layout["right"]["users"]["feedback"]["input"].content().depower();

	if (_client.hotJoining())
	{
		_client.hotJoin();
	}
	else if (_client.hotSpectating())
	{
		_client.hotSpectate();
	}
}

void MultiplayerMenu::outServer()
{
	_layout["left"]["browser"]["lobbies"].reset();
	_layout["left"]["inlobby"]["players"]["players"].reset();
	_layout["left"]["inlobby"]["players"]["replays"].reset();
	_layout["left"]["inlobby"]["observers"].reset();
	_layout["right"]["users"]["users"]["list"].reset();
	_layout["right"]["users"]["rankings"].reset();
	_layout["right"]["chat"].reset();
	_layout["right"]["inputline"]["input"].reset();
	_layout["right"]["inputline"]["input"].disable();
	_layout["left"]["browser"]["panels2"]["onevsone"].disable();
	_layout["left"]["browser"]["panels2"]["create"].disable();
	_layout["left"]["browser"]["panels2"]["versusai"].disable();
	_layout["left"]["browser"]["buttons"]["replay"].disable();
	_layout["left"]["browser"]["panels1"]["tutorial"].disable();
	_layout["left"]["browser"]["panels1"]["challenge"].disable();
	_layout["left"]["browser"]["buttons"]["overview"].disable();
	_layout["left"]["browser"]["buttons"]["guide"].disable();
	_layout["left"]["browser"].bear();
	_layout["left"]["inlobby"]["settings"]["custom"].enable();
	_layout["left"]["inlobby"]["settings"]["planningtime"].enable();
	_layout["left"]["inlobby"].kill();

	resetMapDropdown();
	_ainames.clear();
	_aidescriptions.clear();
}

void MultiplayerMenu::inLobby(const std::string& lobby)
{
	_ownLobby = lobby;
	_layout["left"].setTag("inlobby");
	std::string name = "";
	if (_layout["left"]["browser"]["lobbies"].contains(lobby))
	{
		name = _layout["left"]["browser"]["lobbies"][lobby]["name"].text();
		message(::format(
			// TRANSLATORS: The argument is an English or custom lobby name.
			_("You joined lobby \"%s\"."),
			name.c_str()));
	}
	_layout["left"]["inlobby"]["settings"]["name"]["text"].setText(name);
	_inputMode = InputMode::CHAT_LOBBY;
	_layout["right"]["inputline"]["indicator"]["button"].setText(
		_("LOBBY"));
	_layout["right"]["inputline"]["indicator"]["button"].setTextColor(ColorName::TEXTLOBBYCHAT);
}

void MultiplayerMenu::outLobby()
{
	_ownLobby = "";
	if (_layout["left"].getTag() == "inlobby")
	{
		std::string lobbyname = _layout["left"]["inlobby"]["settings"]["name"]["text"].text();
		message(::format(
			// TRANSLATORS: The argument is an English or custom lobby name.
			_("You left lobby \"%s\"."),
			lobbyname.c_str()));
	}
	_layout["left"].setTag("browser");
	_layout["left"]["inlobby"]["players"]["players"].reset();
	_layout["left"]["inlobby"]["players"]["replays"].reset();
	_layout["left"]["inlobby"]["observers"].reset();
	_layout["left"]["inlobby"]["settings"]["custom"].enable();
	_layout["left"]["inlobby"]["settings"]["planningtime"].enable();
	_inputMode = InputMode::CHAT_GENERAL;
	_layout["right"]["inputline"]["indicator"]["button"].setText(
		_("ALL"));
	_layout["right"]["inputline"]["indicator"]["button"].setTextColor(ColorName::TEXT800);

	resetMapDropdown();
	_ainames.clear();
	_aidescriptions.clear();
}

void MultiplayerMenu::serverClosing()
{
	_layout["left"]["browser"]["panels2"]["onevsone"].disable();
	_layout["left"]["browser"]["panels2"]["create"].disable();
	_layout["left"]["browser"]["panels2"]["versusai"].disable();
	_layout["left"]["browser"]["buttons"]["replay"].disable();
	_layout["left"]["browser"]["panels1"]["tutorial"].disable();
	_layout["left"]["browser"]["panels1"]["challenge"].disable();
	_layout["left"]["browser"]["buttons"]["overview"].disable();
	_layout["left"]["browser"]["buttons"]["guide"].disable();
}

void MultiplayerMenu::hotJoin(const std::string& secret)
{
	if (_layout["left"]["inlobby"].shown())
	{
		_client.hotClear();
		return;
	}

	std::string lobbyid = secret.substr(0, secret.find_first_of('-'));
	Json::Value json(Json::objectValue);
	json["join-secret"] = secret;
	_client.send(Message::join_lobby(lobbyid, json));

	_layout["left"]["browser"]["lobbies"].depower();
}

void MultiplayerMenu::hotSpectate(const std::string& secret)
{
	if (_layout["left"]["inlobby"].shown())
	{
		_client.hotClear();
		return;
	}

	std::string lobbyid = secret.substr(0, secret.find_first_of('-'));
	Json::Value json(Json::objectValue);
	json["spectate-secret"] = secret;
	_client.send(Message::join_lobby(lobbyid, json));

	_layout["left"]["browser"]["lobbies"].depower();
}

void MultiplayerMenu::discordJoinRequest(const char* username,
	const char* discriminator, const std::string& picturename)
{
	InterfaceElement& form = _layout["right"]["users"]["discord"];

	form["content"]["avatar"].setPicture(picturename);
	int w = form["content"]["text"].width();
	form["content"]["text"].setText(::format(
		// TRANSLATORS: The first argument is a Discord username, the second
		// a Discord discriminator string, e.g. together "Alice#1234".
		_("%s#%s on Discord wants to join your lobby."),
		username, discriminator));
	form["content"]["text"].setWidth(w);
	form.settle();

	_layout["right"]["users"].setTag("discord");
	_layout["right"]["inputline"]["input"].content().power();
	_layout["right"]["users"]["feedback"]["input"].content().depower();
}
