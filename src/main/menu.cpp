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
#include "menu.hpp"
#include "source.hpp"

#include "layout.hpp"
#include "horizontallayout.hpp"
#include "verticallayout.hpp"
#include "stackedlayout.hpp"
#include "tooltiplayout.hpp"
#include "clickanddrag.hpp"
#include "filler.hpp"
#include "frame.hpp"
#include "textfield.hpp"
#include "multitextfield.hpp"
#include "image.hpp"
#include "paint.hpp"
#include "colorname.hpp"


Menu::Menu(Owner& owner, GameOwner& gameowner) :
	_owner(owner),
	_gameowner(gameowner),
	_settings(_owner.settings()),
	_displaysettings(_owner.displaysettings()),
	_client(_owner.client())
{}

Menu::~Menu() = default;

void Menu::build()
{
	_layout.put(new Frame("ui/frame_menuscreen_9"));
	_layout.content().put(new HorizontalLayout());
	_layout.content().setPadding(
		(InterfaceElement::windowW() < 600 * InterfaceElement::scale()
			|| InterfaceElement::windowH() < 400 * InterfaceElement::scale())
		? 10 * InterfaceElement::scale()
		: 20 * InterfaceElement::scale());
	_layout.content().setBasicColor(0, ColorName::FRAME800);
	_layout.content().setBasicColor(1, Paint::blend(ColorName::FRAME800, ColorName::SHADEBLEND));
	_layout.content().setBasicColor(2, Paint::blend(ColorName::FRAMESTONE, ColorName::SHINEBLEND));
	_layout.content().setBasicColor(3, Paint::blend(ColorName::FRAMESTONE, ColorName::SHADEBLEND));
}

std::unique_ptr<InterfaceElement> Menu::makeButton(const std::string& text, int fontsize)
{
	std::unique_ptr<InterfaceElement> frame;
	frame.reset(new Frame("ui/frame_button_9"));
	frame->put(new TextField(text, fontsize));
	frame->align(HorizontalAlignment::CENTER);
	frame->setPadding(6 * InterfaceElement::scale());
	frame->settleHeight();
	frame->makeClickable();
	return frame;
}

std::unique_ptr<InterfaceElement> Menu::makeTabButton(const std::string& text, int fontsize)
{
	std::unique_ptr<InterfaceElement> frame;
	frame.reset(new Frame("ui/frame_tab_9"));
	frame->put(new TextField(text, fontsize));
	frame->align(HorizontalAlignment::CENTER);
	frame->setPaddingTop(2 * InterfaceElement::scale());
	frame->setPaddingBottom(0 * InterfaceElement::scale());
	frame->setPaddingHorizontal(8 * InterfaceElement::scale());
	frame->settleHeight();
	frame->makeClickable();
	return frame;
}

std::unique_ptr<InterfaceElement> Menu::makeCheckbox(bool selfclickable)
{
	std::unique_ptr<InterfaceElement> it(new Image("ui/checkbox"));
	auto& element = *it;
	element.setColor(0, Paint::blend(ColorName::UIACCENT, ColorName::SHINEBLEND));
	element.setColor(1, ColorName::UIACCENT);
	element.setColor(2, Paint::blend(ColorName::UIACCENT, ColorName::SHADEBLEND));
	element.setColor(3, ColorName::FRAMESTONE);
	element.setColor(4, Paint::blend(ColorName::FRAMESTONE, ColorName::SHADEBLEND));
	element.setColor(5, Color::transparent());
	element.setColor(6, Color::transparent());
	if (selfclickable) element.makeClickable();
	element.setTag("Empty");
	return it;
}

std::unique_ptr<InterfaceElement> Menu::makeWindowbutton(bool selfclickable)
{
	if (selfclickable)
	{
		return makeWindowbutton(ColorName::UIACCENT, selfclickable);
	}
	else
	{
		return makeWindowbutton(ColorName::FRAMESTONE, selfclickable);
	}
}

std::unique_ptr<InterfaceElement> Menu::makeWindowbutton(const Paint& color, bool selfclickable)
{
	std::unique_ptr<InterfaceElement> it(new Image("ui/windowbutton"));
	auto& element = *it;
	element.setColor(0, Paint::blend(color, ColorName::SHINEBLEND));
	element.setColor(1, color);
	element.setColor(2, Paint::blend(color, ColorName::SHADEBLEND));
	element.setColor(3, ColorName::FRAMESTONE);
	element.setColor(4, Paint::blend(ColorName::FRAMESTONE, ColorName::SHADEBLEND));
	element.setColor(5, Color::transparent());
	if (selfclickable) element.makeClickable();
	element.setTag("Box");
	return it;
}

std::unique_ptr<InterfaceElement> Menu::makeSlider(int maxwidth)
{

	std::unique_ptr<InterfaceElement> element(new ClickAndDrag());

	(*element).put(new Frame("ui/frame_window_9"));
	(*element).content().put(new StackedLayout());
	(*element).align(HorizontalAlignment::LEFT);
	(*element).align(VerticalAlignment::MIDDLE);
	(*element).add("slider", new Frame("ui/gauge_liquid_9"));
	(*element)["slider"].setColor(0, ColorName::UIACCENT);
	(*element)["slider"].setColor(1, Paint::blend(ColorName::UIACCENT, ColorName::SHINEBLEND));
	(*element)["slider"].put(new Filler());
	(*element)["slider"].fixWidth(0);
	(*element).add("filler", new Filler());
	(*element)["filler"].fixWidth(maxwidth);
	(*element).content().setPadding(2 * InterfaceElement::scale());
	(*element).fixHeight(12 * InterfaceElement::scale());
	(*element).content().makeClickable();

	return element;
}

std::unique_ptr<InterfaceElement> Menu::makeErrorbubble(
	const std::string& text, int fontsize)
{
	std::unique_ptr<InterfaceElement> it(new TooltipLayout());
	auto& element = *it;
	element.add("icon", new Image("ui/error"));
	auto& icon = element["icon"];
	icon.setTag("Error");
	icon.setColor(0, Paint::blend(ColorName::UIPRIMARY, ColorName::SHINEBLEND));
	icon.setColor(1, ColorName::UIPRIMARY);
	icon.setColor(2, Paint::blend(ColorName::UIPRIMARY, ColorName::SHADEBLEND));
	icon.setColor(3, Color::transparent());
	icon.setMargin(1 * InterfaceElement::scale());
	element.add("tooltip", Frame::makeTooltip());
	element["tooltip"].put(new MultiTextField(text, fontsize));
	element["tooltip"].setMargin(5 * InterfaceElement::scale());
	element["tooltip"].content().setWidth(-1);
	element["tooltip"].fixWidth(std::min(element["tooltip"].width(),
		InterfaceElement::windowW() / 2));
	element.hide();
	return it;
}

std::unique_ptr<InterfaceElement> Menu::makeErrorprompt(
	const std::string& text, int fontsize, bool selfclickable)
{
	std::unique_ptr<InterfaceElement> it((selfclickable)
		? Frame::makeItem(ColorName::UIPRIMARY)
		: Frame::makeMini(ColorName::UIPRIMARY));
	auto& element = *it;
	element.put(new HorizontalLayout());
	element.add("icon", new Image("ui/error"));
	{
		auto& icon = element["icon"];
		icon.setTag("Error");
		icon.setColor(0, Paint::blend(ColorName::FRAME100,
			ColorName::SHINEBLEND));
		icon.setColor(1, ColorName::FRAME100);
		icon.setColor(2, Paint::blend(ColorName::FRAME100,
			ColorName::SHADEBLEND));
		icon.setColor(3, Color::transparent());
		icon.setMargin(1 * InterfaceElement::scale());
	}
	element.add("texts", new MultiTextField(text, fontsize,
		ColorName::TEXT200));
	element["texts"].setMargin(
		2 * InterfaceElement::scale());
	element["texts"].setMarginLeft(
		5 * InterfaceElement::scale());
	element.align(VerticalAlignment::MIDDLE);
	if (selfclickable) element.makeClickable();
	return it;
}

std::unique_ptr<InterfaceElement> Menu::makeInfoprompt(
	const std::string& text, int fontsize, bool selfclickable)
{
	std::unique_ptr<InterfaceElement> it((selfclickable)
		? Frame::makeItem()
		: Frame::makeMini());
	auto& element = *it;
	element.put(new HorizontalLayout());
	element.add("icon", new Image("ui/error"));
	{
		auto& icon = element["icon"];
		icon.setTag("Info");
		icon.setColor(0, Paint::blend(ColorName::UIACCENT,
			ColorName::SHINEBLEND));
		icon.setColor(1, ColorName::UIACCENT);
		icon.setColor(2, Paint::blend(ColorName::UIACCENT,
			ColorName::SHADEBLEND));
		icon.setColor(3, Color::transparent());
		icon.setMargin(1 * InterfaceElement::scale());
	}
	element.add("texts", new MultiTextField(text, fontsize));
	element["texts"].setMargin(
		2 * InterfaceElement::scale());
	element["texts"].setMarginLeft(
		5 * InterfaceElement::scale());
	element.align(VerticalAlignment::MIDDLE);
	if (selfclickable) element.makeClickable();
	return it;
}

std::unique_ptr<InterfaceElement> Menu::makeAvatarFrame()
{
	std::unique_ptr<InterfaceElement> it(new Frame("ui/frame_button_9"));
	InterfaceElement& element = *it;
	element.setColor(0, Paint::alpha(
		ColorName::FRAME200,
		12));
	element.setColor(1, Paint::alpha(
		ColorName::FRAME600,
		200));
	element.setColor(2, Paint::blend(ColorName::FRAME600, ColorName::SHINEBLEND, 0.5f));
	element.setColor(3, Paint::blend(ColorName::FRAME600, ColorName::SHADEBLEND, 0.5f));
	element.setColor(4, Color::transparent());
	element.setColor(5, Color::transparent());
	element.setDisabledColor(0, Paint::alpha(
		Paint::blend(ColorName::FRAME200, ColorName::DISABLEDBLEND),
		230));
	element.setDisabledColor(1, Paint::alpha(
		Paint::blend(ColorName::FRAME600, ColorName::DISABLEDBLEND),
		242));
	element.setDisabledColor(2, Paint::blend(
		Paint::blend(ColorName::FRAME600, ColorName::SHINEBLEND, 0.5f),
		ColorName::DISABLEDBLEND));
	element.setDisabledColor(3, Paint::blend(
		Paint::blend(ColorName::FRAME600, ColorName::SHADEBLEND, 0.5f),
		ColorName::DISABLEDBLEND));
	element.setDisabledColor(4, Color::transparent());
	element.setDisabledColor(5, Color::transparent());
	element.setHoveredColor(0, Paint::alpha(
		Paint::blend(ColorName::FRAME200, ColorName::HOVEREDBLEND),
		6));
	element.setHoveredColor(1, Paint::alpha(
		Paint::blend(ColorName::FRAME600, ColorName::HOVEREDBLEND),
		200));
	element.setHoveredColor(2, Paint::blend(
		Paint::blend(ColorName::FRAME600, ColorName::SHINEBLEND, 0.5f),
		ColorName::HOVEREDBLEND));
	element.setHoveredColor(3, Paint::blend(
		Paint::blend(ColorName::FRAME600, ColorName::SHADEBLEND, 0.5f),
		ColorName::HOVEREDBLEND));
	element.setHoveredColor(4, Color::transparent());
	element.setHoveredColor(5, Color::transparent());
	element.setPressedColor(0, Paint::alpha(
		Paint::blend(ColorName::FRAME200, ColorName::PRESSEDBLEND),
		64));
	element.setPressedColor(1, Paint::alpha(
		Paint::blend(ColorName::FRAME600, ColorName::PRESSEDBLEND),
		216));
	element.setPressedColor(2, Paint::blend(
		Paint::blend(ColorName::FRAME600, ColorName::SHINEBLEND, 0.5f),
		ColorName::PRESSEDBLEND));
	element.setPressedColor(3, Paint::blend(
		Paint::blend(ColorName::FRAME600, ColorName::SHADEBLEND, 0.5f),
		ColorName::PRESSEDBLEND));
	element.setPressedColor(4, Color::transparent());
	element.setPressedColor(5, Color::transparent());
	return it;
}

void Menu::init()
{
	for (auto& submenu : _submenus)
	{
		submenu->init();
	}

	build();

	kill();
}

void Menu::update()
{
	for (auto& submenu : _submenus)
	{
		submenu->update();
	}

	refresh();

	for (auto& submenu : _submenus)
	{
		if (submenu->_quitting)
		{
			submenu->kill();
			open();
		}
	}
}

void Menu::open()
{
	if (_layout.born()) return;

	_layout.bear();
	_background.bear();

	onOpen();
}

void Menu::kill()
{
	for (auto& submenu : _submenus)
	{
		submenu->kill();
	}

	_quitting = false;

	if (!_layout.born()) return;

	_layout.kill();
	_background.kill();

	onKill();
}

void Menu::show()
{
	for (auto& submenu : _submenus)
	{
		submenu->show();
	}

	_layout.bear(1);
	_background.bear(1);

	onShow();
}

void Menu::hide()
{
	for (auto& submenu : _submenus)
	{
		submenu->hide();
	}

	_layout.kill(1);
	_background.kill(1);

	onHide();
}
