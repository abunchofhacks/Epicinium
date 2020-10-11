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
#include "horizontallayout.hpp"
#include "source.hpp"


void HorizontalLayout::added()
{
	calculateWidth();
	calculateHeight();
}

void HorizontalLayout::removed()
{
	calculateWidth();
	calculateHeight();
}

void HorizontalLayout::calculateWidth()
{
	if (_fixedWidth) return;

	_width = 0;
	int margin = -1;

	for (auto& element : _elements)
	{
		if (margin >= 0) _width += std::max(margin, element->marginLeft());
		_width += element->width();
		margin = element->marginRight();
	}

	if (!_elements.empty())
	{
		_marginLeft = std::max(_marginLeft, _elements.front()->marginLeft());
		_marginRight = std::max(_marginRight, _elements.back()->marginRight());
	}
}

void HorizontalLayout::calculateHeight()
{
	if (_fixedHeight) return;

	_height = 0;

	for (auto& element : _elements)
	{
		if (_height < element->height())
		{
			_height = element->height();
		}
	}

	for (auto& element : _elements)
	{
		switch (_alignment)
		{
			case VerticalAlignment::TOP:
			{
				if (_marginTop < element->marginTop())
				{
					_marginTop = element->marginTop();
				}
				if (_marginBottom + _height < element->marginBottom() + element->height())
				{
					_marginBottom = element->marginBottom() + element->height() - _height;
				}
			}
			break;
			case VerticalAlignment::MIDDLE:
			{
				if (_marginTop + _height / 2 < element->marginTop() + element->height() / 2)
				{
					_marginTop = element->marginTop() + element->height() / 2 - _height / 2;
				}
				if (_marginBottom + _height / 2 < element->marginBottom() + element->height() / 2)
				{
					_marginBottom = element->marginBottom() + element->height() / 2 - _height / 2;
				}
			}
			break;
			case VerticalAlignment::BOTTOM:
			{
				if (_marginTop + _height < element->marginTop() + element->height())
				{
					_marginTop = element->marginTop() + element->height() - _height;
				}
				if (_marginBottom < element->marginBottom())
				{
					_marginBottom = element->marginBottom();
				}
			}
			break;
		}
	}
}

void HorizontalLayout::setWidth(int w)
{
	// We calculate the minimal width. We separate this in 'flat' width,
	// i.e. the part that is not resizable, and 'stretch' width, i.e. the part
	// that belongs to elements that can be resized if needed.
	int flat = 0;
	int stretch = 0;
	int margin = -1;
	std::vector<int> resizables;
	for (auto& element : _elements)
	{
		if (element->resizableWidth())
		{
			element->settleWidth();
			stretch += element->width();
			resizables.push_back(element->width());
		}
		else
		{
			flat += element->width();
		}

		if (margin >= 0) flat += std::max(margin, element->marginLeft());
		margin = element->marginRight();
	}

	// The minimal width is the flat width plus the stretch width.
	int minimal = flat + stretch;

	// If there are no resizable elements or if the minimal width is larger
	// than the desired width, there is nothing left to do.
	if (resizables.empty())
	{
		_width = minimal;
		return;
	}
	else if (w <= minimal)
	{
		_width = minimal;
		return;
	}

	// Sort the resizable width values from small to large.
	std::sort(resizables.begin(), resizables.end());

	// We want all resizable elements to be the same width, w - flat in total.
	// Note that w - flat > stretch because otherwise w <= minimal (see above).
	// However, maybe some resizable elements are already larger than their
	// share of w - flat; in that case we add that element to the flat width
	// and have the remaining elements make up for the difference.
	// Note that there will be at least 1 sharer because if we arrive at
	//                       resizables[0] > (w - flat) / 1
	// then minimal = flat + resizables[0] >  w, which is a contradiction.
	size_t sharers;
	for (sharers = resizables.size(); sharers >= 1; sharers--)
	{
		if (resizables[sharers - 1] > (w - flat) / (int) sharers)
		{
			flat += resizables[sharers - 1];
		}
		else break;
	}

	// The remaining resizable elements are each resized to the same size,
	// except that some elements may be 1 wider to account for the remainder.
	int share = (w - flat) / sharers;
	int leftover = (w - flat) % sharers;
	for (auto& element : _elements)
	{
		if (!element->resizableWidth()) continue;

		if (element->width() <= share)
		{
			// This is one of the sharers, so it takes its share.
			element->setWidth(share + (leftover ? 1 : 0));
			if (leftover > 0) leftover--;
		}
		else
		{
			// Unsettle this element to size all of its children the same way.
			element->setWidth(element->width());
		}
	}

	calculateWidth();
}

void HorizontalLayout::setHeight(int h)
{
	for (auto& element : _elements)
	{
		if (element->resizableHeight())
		{
			element->setHeight(h);
		}
	}

	calculateHeight();
}

bool HorizontalLayout::resizableWidth()
{
	if (_fixedWidth) return false;

	for (auto& element : _elements)
	{
		if (element->resizableWidth()) return true;
	}

	return false;
}

bool HorizontalLayout::resizableHeight()
{
	if (_fixedHeight) return false;

	for (auto& element : _elements)
	{
		if (element->resizableHeight()) return true;
	}

	return false;
}

void HorizontalLayout::settleWidth()
{
	if (_fixedWidth) return;

	for (auto& element : _elements)
	{
		element->settleWidth();
	}

	calculateWidth();
}

void HorizontalLayout::settleHeight()
{
	if (_fixedHeight) return;

	for (auto& element : _elements)
	{
		element->settleHeight();
	}

	calculateHeight();
}

void HorizontalLayout::reset()
{
	_elements.clear();
	_names.clear();
	calculateWidth();
	calculateHeight();
}

void HorizontalLayout::place(const Pixel& pixel)
{
	_topleft = pixel;
	_thickness = 0;

	Pixel elementTopleft(_topleft);
	int margin = -1;

	for (auto& element : _elements)
	{
		switch (_alignment)
		{
			case VerticalAlignment::TOP:
			{
				elementTopleft.yahoo = _topleft.yahoo;
			}
			break;
			case VerticalAlignment::MIDDLE:
			{
				elementTopleft.yahoo = _topleft.yahoo + _height / 2
					- element->height() / 2;
			}
			break;
			case VerticalAlignment::BOTTOM:
			{
				elementTopleft.yahoo = _topleft.yahoo + _height - element->height();
			}
			break;
		}

		if (margin >= 0) elementTopleft.xenon += std::max(margin, element->marginLeft());
		element->place(elementTopleft);
		elementTopleft.xenon += element->width();
		margin = element->marginRight();
		_thickness = std::max(_thickness, element->thickness());
	}
}
