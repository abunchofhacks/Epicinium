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

#include "padding.hpp"
#include "stackedlayout.hpp"
#include "exitcode.hpp"

struct Color;
class Settings;
class DisplaySettings;
class Client;
class GameOwner;


class Menu
{
public:
	class Owner
	{
	protected:
		Owner() = default;

	public:
		Owner(const Owner& /**/) = delete;
		Owner(Owner&& /**/) = delete;
		Owner& operator=(const Owner& /**/) = delete;
		Owner& operator=(Owner&& /**/) = delete;
		virtual ~Owner() = default;

		virtual Settings& settings() = 0;
		virtual DisplaySettings& displaysettings() = 0;
		virtual Client& client() = 0;

		virtual void quit(ExitCode exitcode = ExitCode::DONE) = 0;

		virtual std::string getPicture(const std::string& name) = 0;
	};

	Menu(Owner& owner, GameOwner& gameowner);
	virtual ~Menu();

protected:
	Owner& _owner;
	GameOwner& _gameowner;
	Settings& _settings;
	DisplaySettings& _displaysettings;
	Client& _client;
	StackedLayout _background;
	Padding _layout;
	std::vector<std::unique_ptr<Menu>> _submenus;

private:
	bool _quitting = false;

protected:
	void quit() { _quitting = true; }

	virtual void build();
	virtual void beforeFirstRefreshOfEachSecond() {}
	virtual void refresh() = 0;

	virtual void onOpen() {};
	virtual void onKill() {};
	virtual void onShow() {};
	virtual void onHide() {};

public:
	void init();
	void beforeFirstUpdateOfEachSecond();
	void update();

	void open();
	void kill();
	void show();
	void hide();

	static std::unique_ptr<InterfaceElement> makeButton(const std::string& text, int fontsize);
	static std::unique_ptr<InterfaceElement> makeTabButton(const std::string& text, int fontsize);
	static std::unique_ptr<InterfaceElement> makeCheckbox(bool selfclickable = true);
	static std::unique_ptr<InterfaceElement> makeWindowbutton(bool selfclickable = true);
	static std::unique_ptr<InterfaceElement> makeWindowbutton(const Paint& color, bool selfclickable);
	static std::unique_ptr<InterfaceElement> makeSlider(int maxwidth);
	static std::unique_ptr<InterfaceElement> makeErrorbubble(const std::string& text, int fontsize);
	static std::unique_ptr<InterfaceElement> makeErrorprompt(const std::string& text, int fontsize, bool selfclickable);
	static std::unique_ptr<InterfaceElement> makeInfoprompt(const std::string& text, int fontsize, bool selfclickable);
	static std::unique_ptr<InterfaceElement> makeAvatarFrame();
};
