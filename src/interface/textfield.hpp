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

class Paint;
class Text;


class TextField : public InterfaceElement
{
public:
	TextField(const std::string& str, int fontsize);
	TextField(const std::string& str, int fontsize, const Paint& color);
	TextField(const std::string& str, int fontsize, const Paint& color,
		int outline, const Paint& outlinecolor);
	TextField(const std::string& str, const TextStyle& style);

private:
	HorizontalAlignment _alignment;
	TextStyle _style;
	std::string _text;
	std::string _textLeftover;
	std::shared_ptr<Text> _displayText;
	size_t _numTrailingSpace;
	bool _newlined;
	bool _hovered;
	bool _dirty;

	virtual void prerender();

	void setAlpha();

public:
	virtual void debugtree(uint8_t depth) override;

	virtual void bear(uint8_t depth) override; using InterfaceElement::bear;
	virtual void kill(uint8_t depth) override; using InterfaceElement::kill;
	virtual void enable(uint8_t depth) override; using InterfaceElement::enable;
	virtual void disable(uint8_t depth) override; using InterfaceElement::disable;
	virtual void power(uint8_t depth) override; using InterfaceElement::power;
	virtual void depower(uint8_t depth) override; using InterfaceElement::depower;

	virtual void setWidth(int w) override;
	virtual void setHeight(int h) override;
	virtual bool resizableWidth() override { return !_fixedWidth; }
	virtual bool resizableHeight() override { return false; }
	virtual void place(const Pixel& topleft) override;
	virtual void settleWidth() override;

	virtual void refresh() override;

	virtual std::string text() const override;
	virtual void setText(const std::string& str) override;

	virtual void setTextColor(const Paint& color) override;

	virtual bool overed() const override;
	virtual bool hovered() const override;

	virtual void align(HorizontalAlignment alignment) override { _alignment = alignment; }

	const std::string& textLeftover() const { return _textLeftover; }
};
