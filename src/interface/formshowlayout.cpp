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
#include "formshowlayout.hpp"
#include "source.hpp"


void FormshowLayout::added()
{
	StackedLayout::added();
	setSlide(_slide, false);
}

void FormshowLayout::removed()
{
	StackedLayout::removed();
	setSlide(_slide, false);
}

void FormshowLayout::setSlide(size_t offset, bool caching)
{
	if (_elements.empty())
	{
		_slide = 0;
		return;
	}

	if (offset >= _elements.size())
	{
		offset = _elements.size() - 1;
	}

	if (caching && _slide == offset) return;

	_slide = offset;

	// Element 0 is always alive.
	for (size_t i = 1; i < _elements.size(); i++)
	{
		auto& element = _elements[i];
		if (i == _slide)
		{
			if (!element->born()) element->bear();
		}
		else
		{
			if (element->born()) element->kill();
		}
	}
}

void FormshowLayout::reset()
{
	StackedLayout::reset();
	setSlide(0);
}

void FormshowLayout::place(const Pixel& pixel)
{
	_topleft = pixel;
	_thickness = 0;

	Pixel elementTopleft(_topleft);
	bool first = true;

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

		if (first)
		{
			elementTopleft.proximity = Layer::POPUP;
			_thickness = std::max(_thickness, element->thickness());
			first = false;
		}
	}
}

void FormshowLayout::setTag(const std::string& tag, bool /**/)
{
	for (size_t i = 0; i < _names.size(); i++)
	{
		if (_names[i] == tag)
		{
			setSlide(i, false);
			return;
		}
	}

	LOGE << "Formshow does not contain tag '" << tag << "'";
	DEBUG_ASSERT(false);
}

std::string FormshowLayout::getTag()
{
	if (_slide < _names.size()) return _names[_slide];

	LOGE << "No tag set";
	DEBUG_ASSERT(false);
	return "";
}
