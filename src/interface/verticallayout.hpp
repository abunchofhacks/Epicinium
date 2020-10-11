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


class VerticalLayout : public Layout
{
protected:
	HorizontalAlignment _alignment = HorizontalAlignment::LEFT;

	int _internalPaddingLeft = 0;
	int _internalPaddingRight = 0;

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

	virtual void reset() override;

	virtual void place(const Pixel& topleft) override;

	virtual void align(HorizontalAlignment alignment) override { _alignment = alignment; }
};
