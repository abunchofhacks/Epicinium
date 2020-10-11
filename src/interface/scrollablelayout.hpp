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

#include "verticallayout.hpp"


class ScrollableLayout : public VerticalLayout
{
public:
	ScrollableLayout(bool stickyBottom = false, bool downAdjusted = true);
	ScrollableLayout(int capacity, int unitHeight, bool stickyBottom = false,
		bool downAdjusted = true);

private:
	VerticalAlignment _excessalignment = VerticalAlignment::MIDDLE;

	int _capacity;
	int _unitHeight;
	int _excessHeight;
	int _scrollbarHeight;
	int _scrollbarDragYahooOffset;
	int _offset;
	int _offsetDelta;

	bool _stickyBottom;
	bool _downAdjusted;

	std::unique_ptr<InterfaceElement> _scrollbar;

	void addScrollbar();

	virtual void added() override;
	virtual void removed() override;

	void calculateUnitHeight();
	void calculateCapacity(int h);
	void calculateScrollbarHeight();

public:
	virtual void place(const Pixel& topleft) override;

	virtual void setHeight(int h) override;
	virtual bool resizableHeight() override;
	virtual void settleHeight() override;

	virtual void refresh() override;
	virtual void reset() override;

	virtual bool overed() const override;
	virtual bool hovered() const override;

	using VerticalLayout::align;
	virtual void align(VerticalAlignment alignment) override { _excessalignment = alignment; }

	void scroll(int delta) { _offsetDelta += delta; }

	int capacity() { return _capacity; }
	int offset() { return _offset; }
};
