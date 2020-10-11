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
#include "verticallayout.hpp"
#include "source.hpp"


void VerticalLayout::added()
{
	calculateWidth();
	calculateHeight();
}

void VerticalLayout::removed()
{
	calculateWidth();
	calculateHeight();
}

void VerticalLayout::calculateWidth()
{
	if (_fixedWidth) return;

	_width = 0;

	int padding = _internalPaddingLeft + _internalPaddingRight;
	for (auto& element : _elements)
	{
		if (_width < element->width() + padding)
		{
			_width = element->width() + padding;
		}
	}

	// TODO subtract _internalPaddingLeft and _internalPaddingRight somehow?
	for (auto& element : _elements)
	{
		switch (_alignment)
		{
			case HorizontalAlignment::LEFT:
			{
				if (_marginLeft < element->marginLeft())
				{
					_marginLeft = element->marginLeft();
				}
				if (_marginRight + _width < element->marginRight() + element->width())
				{
					_marginRight = element->marginRight() + element->width() - _width;
				}
			}
			break;
			case HorizontalAlignment::CENTER:
			{
				if (_marginLeft + _width / 2 < element->marginLeft() + element->width() / 2)
				{
					_marginLeft = element->marginLeft() + element->width() / 2 - _width / 2;
				}
				if (_marginRight + _width / 2 < element->marginRight() + element->width() / 2)
				{
					_marginRight = element->marginRight() + element->width() / 2 - _width / 2;
				}
			}
			break;
			case HorizontalAlignment::RIGHT:
			{
				if (_marginLeft + _width < element->marginLeft() + element->width())
				{
					_marginLeft = element->marginLeft() + element->width() - _width;
				}
				if (_marginRight < element->marginRight())
				{
					_marginRight = element->marginRight();
				}
			}
			break;
		}
	}
}

void VerticalLayout::calculateHeight()
{
	if (_fixedHeight) return;

	_height = 0;
	int margin = -1;

	for (auto& element : _elements)
	{
		if (margin >= 0) _height += std::max(margin, element->marginTop());
		_height += element->height();
		margin = element->marginBottom();
	}

	if (!_elements.empty())
	{
		_marginTop = std::max(_marginTop, _elements.front()->marginTop());
		_marginBottom = std::max(_marginBottom, _elements.back()->marginBottom());
	}
}

void VerticalLayout::setWidth(int w)
{
	for (auto& element : _elements)
	{
		if (element->resizableWidth())
		{
			int padding = _internalPaddingLeft + _internalPaddingRight;
			element->setWidth(std::max(0, w - padding));
		}
	}

	calculateWidth();
}

void VerticalLayout::setHeight(int h)
{
	// We calculate the minimal height. We separate this in 'flat' height,
	// i.e. the part that is not resizable, and 'stretch' height, i.e. the part
	// that belongs to elements that can be resized if needed.
	int flat = 0;
	int stretch = 0;
	int margin = -1;
	std::vector<int> resizables;
	for (auto& element : _elements)
	{
		if (element->resizableHeight())
		{
			element->settleHeight();
			stretch += element->height();
			resizables.push_back(element->height());
		}
		else
		{
			flat += element->height();
		}

		if (margin >= 0) flat += std::max(margin, element->marginTop());
		margin = element->marginBottom();
	}

	// The minimal height is the flat height plus the stretch height.
	int minimal = flat + stretch;

	// If there are no resizable elements or if the minimal height is larger
	// than the desired height, there is nothing left to do.
	if (resizables.empty())
	{
		_height = minimal;
		return;
	}
	else if (h <= minimal)
	{
		_height = minimal;
		return;
	}

	// Sort the resizable width values from small to large.
	std::sort(resizables.begin(), resizables.end());

	// We want all resizable elements to be the same height, h - flat in total.
	// Note that h - flat > stretch because otherwise h <= minimal (see above).
	// However, maybe some resizable elements are already larger than their
	// share of h - flat; in that case we add that element to the flat height
	// and have the remaining elements make up for the difference.
	// Note that there will be at least 1 sharer because if we arrive at
	//                       resizables[0] > (h - flat) / 1
	// then minimal = flat + resizables[0] >  h, which is a contradiction.
	size_t sharers;
	for (sharers = resizables.size(); sharers >= 1; sharers--)
	{
		if (resizables[sharers - 1] > (h - flat) / (int) sharers)
		{
			flat += resizables[sharers - 1];
		}
		else break;
	}

	// The remaining resizable elements are each resized to the same size,
	// except that some elements may be 1 taller to account for the remainder.
	int share = (h - flat) / sharers;
	int leftover = (h - flat) % sharers;
	for (auto& element : _elements)
	{
		if (!element->resizableHeight()) continue;

		if (element->height() <= share)
		{
			// This is one of the sharers, so it takes its share.
			element->setHeight(share + (leftover ? 1 : 0));
			if (leftover > 0) leftover--;
		}
		else
		{
			// Unsettle this element to size all of its children the same way.
			element->setHeight(element->height());
		}
	}

	calculateHeight();
}

bool VerticalLayout::resizableWidth()
{
	if (_fixedWidth) return false;

	for (auto& element : _elements)
	{
		if (element->resizableWidth()) return true;
	}

	return false;
}

bool VerticalLayout::resizableHeight()
{
	if (_fixedHeight) return false;

	for (auto& element : _elements)
	{
		if (element->resizableHeight()) return true;
	}

	return false;
}

void VerticalLayout::settleWidth()
{
	if (_fixedWidth) return;

	for (auto& element : _elements)
	{
		element->settleWidth();
	}

	calculateWidth();
}

void VerticalLayout::settleHeight()
{
	if (_fixedHeight) return;

	for (auto& element : _elements)
	{
		element->settleHeight();
	}

	calculateHeight();
}

void VerticalLayout::reset()
{
	_elements.clear();
	_names.clear();
	calculateWidth();
	calculateHeight();
}

void VerticalLayout::place(const Pixel& pixel)
{
	_topleft = pixel;
	_thickness = 0;

	Pixel elementTopleft(_topleft);
	int margin = -1;

	for (auto& element : _elements)
	{
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
					- element->width() / 2;
			}
			break;
			case HorizontalAlignment::RIGHT:
			{
				elementTopleft.xenon = _topleft.xenon + _width
					- element->width() - _internalPaddingRight;
			}
			break;
		}

		if (margin >= 0) elementTopleft.yahoo += std::max(margin, element->marginTop());
		element->place(elementTopleft);
		elementTopleft.yahoo += element->height();
		margin = element->marginBottom();
		_thickness = std::max(_thickness, element->thickness());
	}
}
