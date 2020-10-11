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
#include "transitionator.hpp"
#include "color.hpp"
#include "pixel.hpp"

class Sprite;
struct Skin;


class Image : public InterfaceElement, protected Transitionator
{
public:
	explicit Image(const char* sname);
	explicit Image(const Skin& skin);
	Image(const Image&) = delete;
	Image(Image&&) = delete;
	Image& operator=(const Image&) = delete;
	Image& operator=(Image&&) = delete;
	virtual ~Image() = default;

protected:
	std::shared_ptr<Sprite> _sprite; // (unique ownership)
	std::vector<Paint> _basicPalette;
	std::vector<Paint> _disabledPalette;
	std::vector<Paint> _hoveredPalette;
	std::vector<Paint> _pressedPalette;
	std::vector<Paint> _powerPalette;
	size_t _palettesize = 0;

	bool _hovered = false;
	bool _pressed = false;

	void update();

	virtual void mix();
	virtual void drawSprite();

	void checkColorsSize(size_t index);

public:
	virtual void debugtree(uint8_t depth) override;

	virtual void setColor(size_t index, const Paint& color) override;
	virtual void setBasicColor(size_t index, const Paint& color) override;
	virtual void setDisabledColor(size_t index, const Paint& color) override;
	virtual void setHoveredColor(size_t index, const Paint& color) override;
	virtual void setPressedColor(size_t index, const Paint& color) override;
	virtual void setPowerColor(size_t index, const Paint& color) override;
	virtual void setShineColor(const Paint& color) override;

	virtual void bear(uint8_t depth) override; using InterfaceElement::bear;
	virtual void kill(uint8_t depth) override; using InterfaceElement::kill;
	virtual void enable(uint8_t depth) override; using InterfaceElement::enable;
	virtual void disable(uint8_t depth) override; using InterfaceElement::disable;
	virtual void power(uint8_t depth) override; using InterfaceElement::power;
	virtual void depower(uint8_t depth) override; using InterfaceElement::depower;
	virtual void shine(uint8_t depth) override; using InterfaceElement::shine;
	virtual void deshine(uint8_t depth) override; using InterfaceElement::deshine;

	virtual void setWidth(int w) override;
	virtual void setHeight(int h) override;
	virtual bool resizableWidth() override { return false; }
	virtual bool resizableHeight() override { return false; }

	virtual void place(const Pixel& topleft) override;
	virtual void refresh() override;

	virtual bool overed() const override;
	virtual bool hovered() const override;

	virtual void setTag(const std::string& tag, bool randomize) override;
	virtual void setTagActive(const std::string& tag, bool restart) override;
	virtual std::string getTag() override;

	static Image* upscale(int scale, Image* raw);
};
