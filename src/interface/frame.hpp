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

#include "image.hpp"
#include "pixel.hpp"

class Paint;
class Picture;


class Frame : public Image
{
public:
	explicit Frame(const char* sname);
	explicit Frame(const Skin& skin);
	Frame(const Frame&) = delete;
	Frame(Frame&&) = delete;
	Frame& operator=(const Frame&) = delete;
	Frame& operator=(Frame&&) = delete;
	virtual ~Frame() = default;

	static std::unique_ptr<InterfaceElement> makeItem();
	static std::unique_ptr<InterfaceElement> makeItem(const Paint& color);
	static std::unique_ptr<InterfaceElement> makeItem(const Paint& color,
		bool isTop, bool isBottom);
	static std::unique_ptr<InterfaceElement> makeMini();
	static std::unique_ptr<InterfaceElement> makeMini(const Paint& color);
	static std::unique_ptr<InterfaceElement> makeTooltip(bool hard = false);
	static std::unique_ptr<InterfaceElement> makeInform();
	static std::unique_ptr<InterfaceElement> makeForm();

private:
	std::unique_ptr<InterfaceElement> _content;
	std::shared_ptr<Picture> _background; // (unique ownership)

	int _paddingTop = 0;
	int _paddingLeft = 0;
	int _paddingRight = 0;
	int _paddingBottom = 0;

	virtual void mix() override;
	virtual void drawSprite() override;

public:
	virtual void debugtree(uint8_t depth) override;

	virtual void checkPictures() override;
	virtual void checkPicture(const std::string& picturename) override;
	virtual void setPicture(const std::string& picturename) override;

	using InterfaceElement::put;
	virtual std::unique_ptr<InterfaceElement> put(
		std::unique_ptr<InterfaceElement> content) override;
	virtual InterfaceElement& content() override;

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

	virtual void setPaddingTop(int padding) override;
	virtual void setPaddingLeft(int padding) override;
	virtual void setPaddingRight(int padding) override;
	virtual void setPaddingBottom(int padding) override;

	virtual void setShineColor(const Paint& color) override;

	virtual void place(const Pixel& topleft) override;
	virtual void refresh() override;

	virtual size_t size() const override;
	virtual std::string name(size_t offset) override;
	virtual bool contains(stringref name) override;

	virtual std::string text() const override;
	virtual void setText(const std::string& str) override;
	virtual void prefillText(const std::string& str) override;
	virtual void setTextColor(const Paint& color) override;

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
};
