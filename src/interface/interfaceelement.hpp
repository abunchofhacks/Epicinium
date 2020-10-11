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

#include <bitset>

#include "stringref.hpp"
#include "pixel.hpp"

struct Color;
class Paint;
struct TextStyle;


enum class HorizontalAlignment
{
	LEFT,
	CENTER,
	RIGHT
};

enum class VerticalAlignment
{
	TOP,
	MIDDLE,
	BOTTOM
};

class InterfaceElement
{
public:
	static InterfaceElement* garbage();
	static std::unique_ptr<InterfaceElement> makeGarbage();

	static int scale();
	static int windowW();
	static int windowH();
	static int fontH(int fontsize);
	static int fontH(const TextStyle& style);
	static int textW(const TextStyle& style, const std::string& text);
	static bool inspect();

	virtual ~InterfaceElement() = default;

protected:
	Pixel _topleft = Pixel();
	int _fixedWidth = 0;
	int _fixedHeight = 0;
	int _width = 0;
	int _height = 0;
	int _thickness = 0;
	int _marginTop = 0;
	int _marginLeft = 0;
	int _marginRight = 0;
	int _marginBottom = 0;

private:
	static constexpr uint8_t MAXDEPTH = 32;
	std::bitset<MAXDEPTH> _killed;
	std::bitset<MAXDEPTH> _hidden;
	std::bitset<MAXDEPTH> _disabled;
	bool _powered = false;
	bool _hasMass = true;
	bool _reactsToHover = false;
	bool _reactsToClick = false;
	int _immediateHotkeyScancode = 0;
	int _immediateHotkeyScancode2 = 0;
	int _altHotkeyScancode = 0;

protected:

	void impart(const std::unique_ptr<InterfaceElement>& child);

public:
	virtual void debugtree(uint8_t depth) = 0;

	void bearIf(bool condition);
	void showIf(bool condition);
	void enableIf(bool condition);
	void powerIf(bool condition);

	void bear() { return bear(0); }
	void kill() { return kill(0); }
	void show() { return show(0); }
	void hide() { return hide(0); }
	void enable() { return enable(0); }
	void disable() { return disable(0); }
	void power() { return power(0); }
	void depower() { return depower(0); }
	void shine() { return shine(0); }
	void deshine() { return deshine(0); }

	virtual void bear(uint8_t depth) { _killed[depth] = false; }
	virtual void kill(uint8_t depth) { _killed[depth] = true; }
	virtual void show(uint8_t depth) { _hidden[depth] = false; }
	virtual void hide(uint8_t depth) { _hidden[depth] = true; }
	virtual void enable(uint8_t depth) { _disabled[depth] = false; }
	virtual void disable(uint8_t depth) { _disabled[depth] = true; }
	virtual void power(uint8_t /*depth*/) { _powered = true; }
	virtual void depower(uint8_t /*depth*/) { _powered = false; }
	virtual void shine(uint8_t /*depth*/) {}
	virtual void deshine(uint8_t /*depth*/) {}

	void fixWidth();
	void fixWidth(int w);
	void fixHeight();
	void fixHeight(int h);
	void unfixWidth();
	void unfixHeight();

	virtual void setWidth(int w) = 0;
	virtual void setHeight(int h) = 0;
	virtual bool resizableWidth() = 0;
	virtual bool resizableHeight() = 0;
	virtual void place(const Pixel& topleft) = 0;
	virtual void settleWidth();
	virtual void settleHeight();

	void settle();

	virtual void setMarginTop(int margin);
	virtual void setMarginLeft(int margin);
	virtual void setMarginRight(int margin);
	virtual void setMarginBottom(int margin);
	virtual void setPaddingTop(int margin);
	virtual void setPaddingLeft(int margin);
	virtual void setPaddingRight(int margin);
	virtual void setPaddingBottom(int margin);

	void setMarginHorizontal(int margin);
	void setMarginVertical(int margin);
	void setMargin(int margin);
	void setPaddingHorizontal(int margin);
	void setPaddingVertical(int margin);
	void setPadding(int margin);

	void makeMassless();
	// TODO rename these two functions
	void makeHoverable();
	void makeClickable();

	void setHotkeyScancode(int scancode)
	{
		_immediateHotkeyScancode = scancode;
	}
	void setSecondHotkeyScancode(int scancode)
	{
		_immediateHotkeyScancode2 = scancode;
	}
	void setAltHotkeyScancode(int scancode)
	{
		_altHotkeyScancode = scancode;
	}

	virtual void refresh() = 0;
	virtual void reset();

	Pixel topleft() const { return _topleft; }
	int width() const { return _width; }
	int height() const { return _height; }
	int thickness() const { return _thickness; }
	int marginTop() const { return _marginTop; }
	int marginLeft() const { return _marginLeft; }
	int marginRight() const { return _marginRight; }
	int marginBottom() const { return _marginBottom; }

	bool born() const { return !_killed[0]; }
	bool alive() const { return _killed.none(); }
	bool shown() const { return alive() && _hidden.none(); }
	bool enabled() const { return alive() && _disabled.none(); }
	bool powered() const { return alive() && _powered; }
	bool hasMass() const { return alive() && _hasMass; }
	// TODO rename these two functions
	bool hoverable() const { return alive() && _reactsToHover; }
	bool clickable() const { return alive() && _reactsToClick; }

	virtual size_t size() const { return 0; }
	virtual std::string text() const;
	virtual void setText(const std::string& str);
	virtual void prefillText(const std::string& str);
	virtual void addText(const std::string& str);
	virtual void addText(const std::string& str, int fontsize,
		const Paint& color);
	virtual void addText(const std::string& str, float revealDelay,
		float initDelay = 0);
	virtual void addText(const std::string& str, int fontsize,
		const Paint& color, float revealDelay, float initDelay = 0);
	virtual void addIcon(const char* spritename);
	virtual void addIcon(const char* spritename,
		const Paint& color);
	virtual void setTextColor(const Paint& color);
	virtual std::unique_ptr<InterfaceElement> put(
		std::unique_ptr<InterfaceElement> element);
	virtual void add(stringref name, std::unique_ptr<InterfaceElement> element);
	virtual std::unique_ptr<InterfaceElement> replace(stringref name,
		std::unique_ptr<InterfaceElement> element,
		stringref newname);
	virtual std::string name(size_t offset);
	virtual bool contains(stringref name);
	virtual InterfaceElement* get(stringref name);
	virtual std::unique_ptr<InterfaceElement> remove(stringref name);
	virtual InterfaceElement& content();

	std::unique_ptr<InterfaceElement> put(InterfaceElement* element)
	{
		return put(std::unique_ptr<InterfaceElement>(element));
	}
	void add(stringref name, InterfaceElement* element)
	{
		return add(name, std::unique_ptr<InterfaceElement>(element));
	}
	std::unique_ptr<InterfaceElement> replace(stringref name,
		InterfaceElement* element)
	{
		return replace(name, std::unique_ptr<InterfaceElement>(element), name);
	}
	std::unique_ptr<InterfaceElement> replace(stringref name,
		std::unique_ptr<InterfaceElement> element)
	{
		return replace(name, std::move(element), name);
	}
	std::unique_ptr<InterfaceElement> replace(stringref name,
		InterfaceElement* element, stringref newname)
	{
		return replace(name, std::unique_ptr<InterfaceElement>(element), newname);
	}

	InterfaceElement& operator[](stringref name) { return *(get(name)); }

	// Hovered does not penetrate underlying elements (so if you hover an
	// element, the elements beneath are not hovered), while overed does.
	virtual bool overed() const;
	virtual bool hovered() const;
	virtual bool clicked() const;
	virtual bool held() const;
	virtual bool released() const;

	bool doubleclicked() const;
	bool longheld() const;
	bool longreleased() const;
	bool shortreleased() const;

	virtual bool rightClicked() const;

	virtual void align(HorizontalAlignment horiz);
	virtual void align(VerticalAlignment vert);

	virtual void setColor(size_t index, const Paint& color);
	virtual void setBasicColor(size_t index, const Paint& color);
	virtual void setDisabledColor(size_t index, const Paint& color);
	virtual void setHoveredColor(size_t index, const Paint& color);
	virtual void setPressedColor(size_t index, const Paint& color);
	virtual void setPowerColor(size_t index, const Paint& color);
	virtual void setShineColor(const Paint& color);

	virtual void setTag(const std::string& tag, bool randomize = false);
	virtual void setTagActive(const std::string& tag, bool restart = true);
	virtual std::string getTag();

	virtual void checkPictures() {}
	virtual void checkPicture(const std::string& /*picturename*/) {}
	virtual void setPicture(const std::string& picturename);
};
