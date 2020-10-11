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
#include "textstyle.hpp"
#include "pixel.hpp"

class Paint;
class Text;
class Primitive;


class TextInput : public InterfaceElement
{
public:
	TextInput(int fontsize);
	TextInput(int fontsize, const Paint& color);
	TextInput(int fontsize, size_t maxLength);
	TextInput(int fontsize, size_t maxLength, const Paint& color);

protected:
	TextStyle _style;
	size_t _maxLengthInCodepoints;

	std::string _text = "";
	std::string _prefillText = "";
	std::shared_ptr<Text> _displayText;
	std::shared_ptr<Primitive> _caret;
	std::shared_ptr<Primitive> _rectangle;
	std::shared_ptr<InterfaceElement> _menu;
	size_t _startPos = 0;
	size_t _openPos = 0;
	size_t _closePos = 0;
	size_t _caretPos = 0;
	Pixel _caretTopleft;
	Pixel _rectangleTopleft;
	uint16_t _blinkStart;
	bool _dirty = true;
	bool _wasPowered = false;
	bool _wasHovered = false;
	bool _dragging = false;
	bool _menuOpened = false;
	bool _wasClicked = false;
	bool _copyable = true;

	static const uint16_t BLINKSPEED;

	virtual void prerenderText();

	void prerenderText(const std::string& text);
	void prerenderSelection();
	void prerenderCaret();
	void prerender();

	void truncateTextForDisplay(std::string& text);

	virtual void filterUserInput(std::string& input);

	void progress(size_t& pos);
	void regress(size_t& pos);

	void insertText(std::string input);

	size_t determinePixelPosition(const Pixel& pixel);

	void setAlpha();

public:
	virtual void debugtree(uint8_t depth) override;

	virtual void bear(uint8_t depth) override; using InterfaceElement::bear;
	virtual void kill(uint8_t depth) override; using InterfaceElement::kill;
	virtual void show(uint8_t depth) override; using InterfaceElement::show;
	virtual void hide(uint8_t depth) override; using InterfaceElement::hide;
	virtual void disable(uint8_t depth) override; using InterfaceElement::disable;
	virtual void power(uint8_t depth) override; using InterfaceElement::power;
	virtual void depower(uint8_t depth) override; using InterfaceElement::depower;

	virtual void setWidth(int w) override;
	virtual void setHeight(int h) override;
	virtual bool resizableWidth() override { return !_fixedWidth; }
	virtual bool resizableHeight() override { return false; }

	virtual void placeMenu(const Pixel& topleft);
	virtual void place(const Pixel& topleft) override;

	virtual void refresh() override;
	virtual void reset() override;

	virtual std::string text() const override { return _text; }
	virtual void setText(const std::string& str) override;
	virtual void prefillText(const std::string& str) override;

	virtual bool overed() const override;
	virtual bool hovered() const override;
};
