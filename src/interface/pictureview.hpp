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

class Picture;
struct Pixel;
enum class ArtPanMode : uint8_t;


class PictureView : public InterfaceElement, protected Transitionator
{
public:
	explicit PictureView();
	explicit PictureView(ArtPanMode panmode);
	PictureView(const PictureView&) = delete;
	PictureView(PictureView&&) = delete;
	PictureView& operator=(const PictureView&) = delete;
	PictureView& operator=(PictureView&&) = delete;
	virtual ~PictureView() = default;

protected:
	std::shared_ptr<Picture> _picture; // (unique ownership)

	HorizontalAlignment _horizontalAlignment = HorizontalAlignment::CENTER;
	VerticalAlignment _verticalAlignment = VerticalAlignment::MIDDLE;
	ArtPanMode _panmode;
	bool _panning = false;

	bool _hovered = false;
	bool _pressed = false;

private:
	void update();

public:
	virtual void debugtree(uint8_t depth) override;

	virtual void checkPictures() override;
	virtual void checkPicture(const std::string& picturename) override;
	virtual void setPicture(const std::string& picturename) override;

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
	virtual bool resizableWidth() override { return !_fixedWidth; }
	virtual bool resizableHeight() override { return !_fixedHeight; }
	virtual void settleWidth() override;
	virtual void settleHeight() override;

	virtual void align(HorizontalAlignment horiz) override;
	virtual void align(VerticalAlignment vert) override;

	virtual void place(const Pixel& topleft) override;
	virtual void refresh() override;

	virtual bool overed() const override;
	virtual bool hovered() const override;
};
