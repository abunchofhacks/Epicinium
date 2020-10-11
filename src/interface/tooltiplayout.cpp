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
#include "tooltiplayout.hpp"
#include "source.hpp"

#include "input.hpp"
#include "loop.hpp"


void TooltipLayout::added()
{
	calculateWidth();
	calculateHeight();
}

void TooltipLayout::removed()
{
	calculateWidth();
	calculateHeight();
}

void TooltipLayout::calculateWidth()
{
	if (_fixedWidth) return;

	_width = 0;

	if (_elements.empty()) return;

	auto& content = _elements.front();
	_width = content->width();
	_marginLeft = std::max(_marginLeft, content->marginLeft());
	_marginRight = std::max(_marginRight, content->marginRight());
}

void TooltipLayout::calculateHeight()
{
	if (_fixedHeight) return;

	_height = 0;

	if (_elements.empty()) return;

	auto& content = _elements.front();
	_height = content->height();
	_marginTop = std::max(_marginTop, content->marginTop());
	_marginBottom = std::max(_marginBottom, content->marginBottom());
}

void TooltipLayout::setWidth(int w)
{
	if (_elements.empty()) return;

	auto& content = _elements.front();
	content->setWidth(w);

	calculateWidth();
}

void TooltipLayout::setHeight(int h)
{
	if (_elements.empty()) return;

	auto& content = _elements.front();
	content->setHeight(h);

	calculateHeight();
}

bool TooltipLayout::resizableWidth()
{
	if (_fixedWidth) return false;

	if (_elements.empty()) return false;

	auto& content = _elements.front();
	return content->resizableWidth();
}

bool TooltipLayout::resizableHeight()
{
	if (_fixedHeight) return false;

	if (_elements.empty()) return false;

	auto& content = _elements.front();
	return content->resizableHeight();
}

void TooltipLayout::settleWidth()
{
	if (_fixedWidth) return;

	for (auto& element : _elements)
	{
		element->settleWidth();
	}

	calculateWidth();
}

void TooltipLayout::settleHeight()
{
	if (_fixedHeight) return;

	for (auto& element : _elements)
	{
		element->settleHeight();
	}

	calculateHeight();
}

void TooltipLayout::refresh()
{
	if (_type == Type::DROPDOWN)
	{
		// Menu-like behavior: the tooltip appears when the content is clicked and disappears
		// when either the content or the tooltip is clicked
		// or when the mouse button is held down (e.g. for dragging)
		// or when something else is clicked.
		if (!_opened)
		{
			if (!_elements.empty() && _elements.front()->clicked()) _opened = true;
		}
		else if (_wasClicked)
		{
			_opened = false;
			_wasClicked = false;
		}
		else if (clicked())
		{
			_wasClicked = true;
		}
		else if (!hovered() && Input::get()->wasKeyPressed(SDL_SCANCODE_LMB))
		{
			_opened = false;
		}
		else if (Input::get()->isKeyLongHeld(SDL_SCANCODE_LMB))
		{
			_opened = false;
		}
	}
	else if (_type == Type::TOOLTIP)
	{
		// Default behavior: the tooltip appears when the content is hovered
		// for a short duration without the item or the mouse moving,
		// and disappears when the content is held or is no longer hovered.
		bool hovering = (!_elements.empty()
			&& _elements.front()->hovered()
			&& !_elements.front()->held());
		if (hovering)
		{
			float delta = Loop::delta();
			Pixel pixel = _elements.front()->topleft();
			bool itemstable = pixel.distanceTo(_prevItemPixel) < 1 * delta;
			if (!itemstable) _prevItemPixel = pixel;
			Pixel mouse = Input::get()->mousePixel();
			bool mousestable = mouse.distanceTo(_prevMousePixel)
					< Input::get()->significantMoveDistance() * delta;
			if (!mousestable) _prevMousePixel = mouse;

			if (!_opened)
			{
				if (itemstable && mousestable)
				{
					_buildup += delta;
					if (_buildup >= Input::get()->tooltipThreshold())
					{
						_opened = true;
					}
				}
				else _buildup = 0.0f;
			}
			else
			{
				if (!itemstable)
				{
					_opened = false;
				}
			}
		}
		else if (_opened)
		{
			_opened = false;
		}
	}

	if (_type == Type::MANUAL)
	{
		// This is done manually by the creator.
	}
	else if (_opened)
	{
		for (size_t i = 1; i < _elements.size(); i++)
		{
			if (!_elements[i]->born()) _elements[i]->bear();
		}
	}
	else
	{
		for (size_t i = 1; i < _elements.size(); i++)
		{
			if (_elements[i]->born()) _elements[i]->kill();
		}
	}

	Layout::refresh();
}

void TooltipLayout::reset()
{
	_elements.clear();
	_names.clear();
	calculateWidth();
	calculateHeight();
}

void TooltipLayout::place(const Pixel& pixel)
{
	_topleft = pixel;
	_thickness = 0;

	if (_elements.empty()) return;

	auto& content = _elements.front();
	content->place(_topleft);
	_thickness += content->thickness();

	Pixel elementTopleft(_topleft);
	elementTopleft.proximity = Layer::TOOLTIP;

	for (size_t i = 1; i < _elements.size(); i++)
	{
		auto& element = _elements[i];

		switch (_alignment)
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

		// Are we cut off by the right of the screen?
		if (elementTopleft.xenon + element->width() + element->marginRight()
			>= InterfaceElement::windowW())
		{
			// Move the appropriate distance to the left
			elementTopleft.xenon = InterfaceElement::windowW() - element->width()
				- element->marginRight();
		}

		// We align the top of the tooltip to the bottom of the content.
		elementTopleft.yahoo = content->topleft().yahoo + content->height()
			+ element->marginTop();

		// Are we at the bottom of the screen?
		if (elementTopleft.yahoo + element->height() + element->marginBottom()
			>= InterfaceElement::windowH())
		{
			// We align the bottom of the tooltip to the top of the content.
			elementTopleft.yahoo = content->topleft().yahoo - element->height()
				- element->marginBottom();
		}

		element->place(elementTopleft);

		elementTopleft.proximity += element->thickness();
	}
}
