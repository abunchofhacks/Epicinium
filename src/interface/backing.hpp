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

#include "interfaceelement.hpp"


class Backing : public InterfaceElement
{
private:
	std::unique_ptr<InterfaceElement> _content;
	std::unique_ptr<InterfaceElement> _backing;

	std::unique_ptr<InterfaceElement> putBacking(
		std::unique_ptr<InterfaceElement> backing);

public:
	using InterfaceElement::put;
	virtual std::unique_ptr<InterfaceElement> put(
		std::unique_ptr<InterfaceElement> content) override;
	virtual InterfaceElement& content() override;

	virtual void debugtree(uint8_t depth) override;

	virtual void bear(uint8_t depth) override; using InterfaceElement::bear;
	virtual void kill(uint8_t depth) override; using InterfaceElement::kill;
	virtual void show(uint8_t depth) override; using InterfaceElement::show;
	virtual void hide(uint8_t depth) override; using InterfaceElement::hide;
	virtual void enable(uint8_t depth) override; using InterfaceElement::enable;
	virtual void disable(uint8_t depth) override; using InterfaceElement::disable;
	virtual void power(uint8_t depth) override; using InterfaceElement::power;
	virtual void depower(uint8_t depth) override; using InterfaceElement::depower;
	virtual void shine(uint8_t depth) override; using InterfaceElement::shine;
	virtual void deshine(uint8_t depth) override; using InterfaceElement::deshine;

	virtual void setWidth(int w) override;
	virtual void setHeight(int h) override;
	virtual bool resizableWidth() override;
	virtual bool resizableHeight() override;
	virtual void settleWidth() override;
	virtual void settleHeight() override;

	virtual void align(HorizontalAlignment horiz) override;
	virtual void align(VerticalAlignment vert) override;

	virtual void setTag(const std::string& tag, bool randomize) override;
	virtual void setTagActive(const std::string& tag, bool restart) override;
	virtual std::string getTag() override;

	virtual void setMarginTop(int margin) override;
	virtual void setMarginLeft(int margin) override;
	virtual void setMarginRight(int margin) override;
	virtual void setMarginBottom(int margin) override;

	virtual void place(const Pixel& topleft) override;
	virtual void refresh() override;

	virtual size_t size() const override;
	virtual std::string name(size_t offset) override;
	virtual bool contains(stringref name) override;

	virtual std::string text() const override;

	using InterfaceElement::add;
	virtual void add(stringref name,
		std::unique_ptr<InterfaceElement> element) override;
	using InterfaceElement::replace;
	virtual std::unique_ptr<InterfaceElement> replace(stringref name,
		std::unique_ptr<InterfaceElement> element, stringref newname) override;
	virtual InterfaceElement* get(stringref name) override;
	virtual std::unique_ptr<InterfaceElement> remove(stringref name) override;
	virtual void reset() override;

	virtual bool overed() const override;
	virtual bool hovered() const override;
	virtual bool clicked() const override;
	virtual bool held() const override;
	virtual bool released() const override;

	virtual void checkPictures() override;
	virtual void checkPicture(const std::string& picturename) override;
};
