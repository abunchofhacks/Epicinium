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
#include "stackedlayout.hpp"
#include "source.hpp"


void StackedLayout::added()
{
	calculateWidth();
	calculateHeight();
}

void StackedLayout::removed()
{
	calculateWidth();
	calculateHeight();
}

void StackedLayout::calculateWidth()
{
	if (_fixedWidth) return;

	_width = 0;

	for (auto& element : _elements)
	{
		if (_width < element->width())
		{
			_width = element->width();
		}
	}

	for (auto& element : _elements)
	{
		switch (_halignment)
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

void StackedLayout::calculateHeight()
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
		switch (_valignment)
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

void StackedLayout::setWidth(int w)
{
	for (auto& element : _elements)
	{
		if (element->resizableWidth())
		{
			element->setWidth(w);
		}
	}

	calculateWidth();
}

void StackedLayout::setHeight(int h)
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

bool StackedLayout::resizableWidth()
{
	if (_fixedWidth) return false;

	for (auto& element : _elements)
	{
		if (element->resizableWidth()) return true;
	}

	return false;
}

bool StackedLayout::resizableHeight()
{
	if (_fixedHeight) return false;

	for (auto& element : _elements)
	{
		if (element->resizableHeight()) return true;
	}

	return false;
}

void StackedLayout::settleWidth()
{
	if (_fixedWidth) return;

	for (auto& element : _elements)
	{
		element->settleWidth();
	}

	calculateWidth();
}

void StackedLayout::settleHeight()
{
	if (_fixedHeight) return;

	for (auto& element : _elements)
	{
		element->settleHeight();
	}

	calculateHeight();
}

void StackedLayout::reset()
{
	_elements.clear();
	_names.clear();
	calculateWidth();
	calculateHeight();
}

void StackedLayout::place(const Pixel& pixel)
{
	_topleft = pixel;
	_thickness = 0;

	Pixel elementTopleft(_topleft);

	for (auto& element : _elements)
	{
		switch (_halignment)
		{
			case HorizontalAlignment::LEFT:
			{
				elementTopleft.xenon = _topleft.xenon;
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
				elementTopleft.xenon = _topleft.xenon + _width - element->width();
			}
			break;
		}

		switch (_valignment)
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

		element->place(elementTopleft);

		elementTopleft.proximity += element->thickness();
		_thickness += element->thickness();
	}
}
