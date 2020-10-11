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
#include "scrollablelayout.hpp"
#include "source.hpp"

#include "input.hpp"
#include "primitive.hpp"


ScrollableLayout::ScrollableLayout(bool stickyBottom, bool downAdjusted) :
	_capacity(1),
	_unitHeight(0),
	_excessHeight(0),
	_scrollbarHeight(0),
	_scrollbarDragYahooOffset(0),
	_offset(0),
	_offsetDelta(0),
	_stickyBottom(stickyBottom),
	_downAdjusted(downAdjusted)
{
	addScrollbar();
}

ScrollableLayout::ScrollableLayout(int capacity, int unitHeight,
	bool stickyBottom, bool downAdjusted) :
	_capacity(capacity),
	_unitHeight(unitHeight),
	_excessHeight(0),
	_scrollbarHeight(0),
	_scrollbarDragYahooOffset(0),
	_offset(0),
	_offsetDelta(0),
	_stickyBottom(stickyBottom),
	_downAdjusted(downAdjusted)
{
	_fixedHeight = _height = _capacity * _unitHeight;

	addScrollbar();
}

void ScrollableLayout::addScrollbar()
{
	_scrollbar.reset(new ClickAndDrag());
	_scrollbar->put(new Frame("ui/canvas_9"));
	_scrollbar->content().setColor(0, ColorName::SCROLLBAR);
	_scrollbar->content().setHoveredColor(0,
		Paint::mix(ColorName::HOVEREDBLEND, Color::transparent(), 0.25f));
	_scrollbar->content().setPressedColor(0,
		Paint::mix(ColorName::PRESSEDBLEND, Color::transparent(), 0.25f));
	_scrollbar->content().put(new Filler());
	_scrollbar->fixWidth(4 * InterfaceElement::scale());
	_scrollbar->content().makeClickable();

	_internalPaddingLeft = (4 + 1) * InterfaceElement::scale();
	_internalPaddingRight = _internalPaddingLeft;
}

void ScrollableLayout::added()
{
	calculateWidth();
	calculateUnitHeight();
	calculateCapacity(_height);
	calculateScrollbarHeight();
	if (_stickyBottom) _offset = int(_elements.size()) - _capacity;
	place(_topleft);
}

void ScrollableLayout::removed()
{
	calculateWidth();
	calculateUnitHeight();
	calculateCapacity(_height);
	calculateScrollbarHeight();
	if (_stickyBottom) _offset = int(_elements.size()) - _capacity;
	place(_topleft);
}

void ScrollableLayout::reset()
{
	_elements.clear();
	_names.clear();
	calculateWidth();
	calculateUnitHeight();
	calculateCapacity(_height);
	calculateScrollbarHeight();
}

void ScrollableLayout::place(const Pixel& pixel)
{
	_topleft = pixel;
	_thickness = 0;

	Pixel elementTopleft;
	elementTopleft.proximity = _topleft.proximity + 1.0f;

	int excessHeightOffset = 0;
	if (_excessHeight > 0)
	{
		switch (_excessalignment)
		{
			case VerticalAlignment::TOP:    excessHeightOffset = 0;                 break;
			case VerticalAlignment::MIDDLE: excessHeightOffset = _excessHeight / 2; break;
			case VerticalAlignment::BOTTOM: excessHeightOffset = _excessHeight;     break;
		}
	}

	for (size_t i = 0; i < _elements.size(); i++)
	{
		elementTopleft.yahoo = _topleft.yahoo + (i - _offset) * _unitHeight
			+ excessHeightOffset
			+ _unitHeight / 2 - _elements[i]->height() / 2;

		switch (_alignment)
		{
			case HorizontalAlignment::LEFT:
			{
				elementTopleft.xenon = _topleft.xenon + _internalPaddingLeft;
			}
			break;
			case HorizontalAlignment::CENTER:
			{
				elementTopleft.xenon = _topleft.xenon + _width / 2
					- _elements[i]->width() / 2;
			}
			break;
			case HorizontalAlignment::RIGHT:
			{
				elementTopleft.xenon = _topleft.xenon + _width
					- _elements[i]->width() - _internalPaddingRight;
			}
			break;
		}

		_elements[i]->place(elementTopleft);
		_thickness = std::max(_thickness, _elements[i]->thickness());
	}

	if (_scrollbar)
	{
		Pixel scrollbarTopleft = _topleft;
		scrollbarTopleft.xenon += _width - _scrollbar->width();
		scrollbarTopleft.proximity += _thickness - 1;
		_scrollbar->place(scrollbarTopleft);
		_thickness += _scrollbar->thickness();
	}
}

void ScrollableLayout::calculateUnitHeight()
{
	int maximum = _unitHeight;

	for (auto& element : _elements)
	{
		if (maximum < element->height())
		{
			maximum = element->height() + (element->marginTop() + element->marginBottom()) / 2;
		}
	}

	_unitHeight = maximum;
}

void ScrollableLayout::calculateCapacity(int h)
{
	if (_unitHeight > 0)
	{
		_capacity = std::max(1, h / _unitHeight);
		_height = _capacity * _unitHeight;
		_excessHeight = std::max(0, std::min(h - _height, _unitHeight - 1));
		_height += _excessHeight;
	}
}

void ScrollableLayout::calculateScrollbarHeight()
{
	int newheight = -1;

	DEBUG_ASSERT(_capacity >= 1);
	if (_capacity >= 1 && _capacity < (int) _elements.size())
	{
		float fraction = 1.0f * _capacity / _elements.size();
		newheight = (int) (fraction * _height + 0.5f);
		if (_height > 50 * InterfaceElement::scale())
		{
			newheight = std::max(newheight, 25 * InterfaceElement::scale());
		}
		else
		{
			newheight = std::max(newheight, 5 * InterfaceElement::scale());
		}
	}

	if (newheight > 0)
	{
		_scrollbar->fixHeight(newheight);
	}

	_scrollbarHeight = newheight;
}

void ScrollableLayout::setHeight(int h)
{
	calculateCapacity(h);
	calculateScrollbarHeight();
}

bool ScrollableLayout::resizableHeight()
{
	if (_fixedHeight) return false;

	return true;
}

void ScrollableLayout::settleHeight()
{
	if (_fixedHeight) return;

	for (auto& element : _elements)
	{
		element->settleHeight();
	}

	calculateUnitHeight();
	calculateCapacity(0);
	calculateScrollbarHeight();
}

void ScrollableLayout::refresh()
{
	int prevoffset = _offset;
	int maximum = int(_elements.size()) - _capacity;

	if (_scrollbar && _scrollbar->held())
	{
		int dy = _scrollbar->topleft().yahoo - _topleft.yahoo;
		int maxdy = std::max(1, _height - _scrollbar->height());
		_offset = std::max(0, std::min(maximum, maximum * dy / maxdy));
	}
	else
	{
		if (enabled() && hovered())
		{
			scroll(-Input::get()->mouseScrollDelta());
		}
		_offset += _offsetDelta;
		_offsetDelta = 0;
	}

	if (_offset > maximum) _offset = maximum;
	int minimum = (_stickyBottom && _downAdjusted) ? std::min(0, maximum) : 0;
	if (_offset < minimum) _offset = minimum;

	if (_offset != prevoffset) place(_topleft);

	int from = std::max(0, _offset);
	int to = std::min(int(_elements.size()), _offset + _capacity);
	for (int i = 0; i < from; i++)
	{
		if ( _elements[i]->born()) _elements[i]->kill();
	}
	for (int i = from; i < to; i++)
	{
		if (!_elements[i]->born()) _elements[i]->bear();
	}
	for (int i = to; i < (int) _elements.size(); i++)
	{
		if ( _elements[i]->born()) _elements[i]->kill();
	}

	Layout::refresh();

	if (_scrollbar && _scrollbarHeight > 0 && shown())
	{
		int my = Input::get()->mousePixel().yahoo;
		if (_scrollbar->clicked())
		{
			_scrollbarDragYahooOffset = _scrollbar->topleft().yahoo - my;
		}

		int y = _topleft.yahoo;
		if (_scrollbar->held())
		{
			y = my + _scrollbarDragYahooOffset;
			y = std::max(_topleft.yahoo, std::min(y,
				_topleft.yahoo + _height - _scrollbarHeight));
		}
		else
		{
			float fraction = 1.0f * _offset / std::max(1, maximum);
			int maxdy = std::max(0, _height - _scrollbar->height());
			y = _topleft.yahoo + (int) (fraction * maxdy + 0.5f);
		}

		if (_scrollbar->topleft().yahoo != y)
		{
			Pixel scrollbarTopleft = _scrollbar->topleft();
			scrollbarTopleft.yahoo = y;
			_scrollbar->place(scrollbarTopleft);
		}

		_scrollbar->refresh();
	}
}

bool ScrollableLayout::overed() const
{
	if (!alive()) return false;

	bool result = false;

	// One of the elements might have a tooltip that exceeds our borders.
	if (VerticalLayout::overed()) result = true;
	if (_scrollbar && _scrollbar->overed()) result = true;

	// The scrollable layout has mass so you can scroll when the cursor is between items.
	Pixel mouse = Input::get()->mousePixel();
	if (   mouse.xenon >= _topleft.xenon && mouse.xenon < _topleft.xenon + _width
		&& mouse.yahoo >= _topleft.yahoo && mouse.yahoo < _topleft.yahoo + _height)
	{
		result = true;
	}

	return result;
}

bool ScrollableLayout::hovered() const
{
	if (!alive()) return false;

	bool result = false;

	// Because some items might occlude others, we must know which layer the
	// mouse operates in. We therefore need to check all items to see if they
	// are hovered.
	// Also, one of the elements might have a tooltip that exceeds our borders.
	if (VerticalLayout::hovered()) result = true;
	if (_scrollbar && _scrollbar->hovered()) result = true;

	// Do not check hasMass.
	// The scrollable layout has mass so you can scroll when the cursor is between items.
	Pixel mouse = Input::get()->mousePixel();
	if (mouse.proximity.layer() <= _topleft.proximity.layer()
		&& mouse.xenon >= _topleft.xenon && mouse.xenon < _topleft.xenon + _width
		&& mouse.yahoo >= _topleft.yahoo && mouse.yahoo < _topleft.yahoo + _height)
	{
		Input::get()->layerMouse(_topleft.proximity.layer());
		result = true;
	}

	return result;
}
