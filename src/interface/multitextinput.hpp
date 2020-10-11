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

#include "scrollablelayout.hpp"
#include "pixel.hpp"
#include "textstyle.hpp"

class Paint;
class Text;
class Primitive;


class MultiTextInput : public ScrollableLayout
{
public:
	MultiTextInput(int fontsize, size_t lines);
	MultiTextInput(int fontsize, size_t lines, const Paint& color);

private:
	TextStyle _style;

	std::string _text = "";
	std::string _prefillText = "";
	std::shared_ptr<Primitive> _caret;
	std::vector<std::shared_ptr<Primitive>> _rectangles;
	std::shared_ptr<InterfaceElement> _menu;
	size_t _openPos = 0;
	size_t _closePos = 0;
	size_t _caretPos = 0;
	Pixel _caretTopleft;
	Pixel _rectangleTopleft;
	std::vector<Pixel> _rectangleToplefts;
	uint16_t _blinkStart;
	bool _textDirty = true;
	bool _posDirty = true;
	bool _wasPowered = false;
	bool _wasHovered = false;
	bool _dragging = false;
	bool _menuOpened = false;
	bool _wasClicked = false;
	bool _movedCaret = false;

	static const uint16_t BLINKSPEED;

	void generate();
	void prerenderSelection();
	void prerenderCaret();
	void prerender();
	void filterUserInput(std::string& input);
	void insertText(std::string input);

	void progress(size_t& pos);
	void regress(size_t& pos);

	size_t determinePixelPosition(const Pixel& pixel);
	std::pair<size_t, size_t> determineLineAndOffset(size_t pos);
	Pixel determineLineAndOffsetPixel(size_t line, size_t linePos);

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

	virtual void placeMenu(const Pixel& topleft);
	virtual void place(const Pixel& topleft) override;

	virtual void setWidth(int w) override;
	virtual void setHeight(int h) override;
	virtual bool resizableWidth() override { return !_fixedWidth; }
	virtual bool resizableHeight() override { return false; }
	virtual void settleWidth() override;
	virtual void settleHeight() override;

	virtual void refresh() override;
	virtual void reset() override;

	virtual std::string text() const override { return _text; }
	virtual void setText(const std::string& str) override;
	virtual void prefillText(const std::string& str) override;

	virtual bool overed() const override;
	virtual bool hovered() const override;
	virtual bool clicked() const override;
	virtual bool held() const override;
	virtual bool released() const override;
};
