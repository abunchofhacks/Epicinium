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

#include "layout.hpp"


class TooltipLayout : public Layout
{
public:
	enum class Type
	{
		TOOLTIP,
		DROPDOWN,
		MANUAL,
	};

	TooltipLayout() :
		TooltipLayout(Type::TOOLTIP)
	{}

	explicit TooltipLayout(bool requireClick) :
		TooltipLayout(requireClick ? Type::DROPDOWN : Type::TOOLTIP)
	{}

	explicit TooltipLayout(Type type) :
		_type(type)
	{}

private:
	Type _type;

	HorizontalAlignment _alignment = HorizontalAlignment::LEFT;

	bool _opened = false;
	bool _wasClicked = false;
	float _buildup = 0.0f;
	Pixel _prevItemPixel;
	Pixel _prevMousePixel;

	virtual void added() override;
	virtual void removed() override;

	void calculateWidth();
	void calculateHeight();

public:
	virtual void setWidth(int w) override;
	virtual void setHeight(int h) override;
	virtual bool resizableWidth() override;
	virtual bool resizableHeight() override;
	virtual void settleWidth() override;
	virtual void settleHeight() override;

	virtual void refresh() override;
	virtual void reset() override;

	virtual void place(const Pixel& topleft) override;

	virtual void align(HorizontalAlignment alignment) override { _alignment = alignment; }
};
