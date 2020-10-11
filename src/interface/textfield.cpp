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
#include "textfield.hpp"
#include "source.hpp"

#include "text.hpp"
#include "collector.hpp"
#include "input.hpp"
#include "paint.hpp"
#include "colorname.hpp"


TextField::TextField(const std::string& str, int fontsize) :
	TextField(str, fontsize, ColorName::TEXT800)
{}

TextField::TextField(const std::string& str, int fontsize, const Paint& color) :
	TextField(str, fontsize, color, 0, color)
{}

TextField::TextField(const std::string& str, int fontsize, const Paint& color,
		int outline, const Paint& outlinecolor) :
	TextField(str, TextStyle(FontStyle(fontsize, outline, false, false),
			color, outlinecolor))
{}

TextField::TextField(const std::string& str, const TextStyle& style) :
	_alignment(HorizontalAlignment::LEFT),
	_style(style),
	_text(str),
	_textLeftover(""),
	_displayText(new Text("", _style)),
	_numTrailingSpace(0),
	_newlined(false),
	_hovered(false),
	_dirty(true)
{
	// See below.
	if (_text == "\u200b") _text = "";

	size_t newline = _text.find('\n');
	if (newline != std::string::npos)
	{
		_newlined = true;
		_textLeftover = _text.substr(newline + 1);
		_text = _text.substr(0, newline);

		// If there was a newline at the end of this TextField, we want to show
		// that the line has ended without any text leftover.
		// We use a zero width space to indicate this.
		if (_textLeftover.empty()) _textLeftover = "\u200b";
	}

	_width = textW(_style, _text);
	_height = fontH(_style);
	power();
	setAlpha();
}

void TextField::debugtree(uint8_t depth)
{
	std::string spaces(depth, '.');
	std::cout << spaces << "TextField" << std::endl;
}

void TextField::prerender()
{
	// Do not display the last _numTrailingSpace characters. See below.
	_displayText.reset(new Text(
		_text.substr(0, _text.length() - _numTrailingSpace),
		_style));

	setAlpha();

	_dirty = false;
}

void TextField::setWidth(int w)
{
	if (w < 0) return;
	_width = w;

	// First we try breaking at the first newline. If the line is still to long,
	// we have to add extra characters to _textLeftover, which means we need to
	// remember to add back the newline. Hence _newlined.
	size_t newline = _text.find('\n');
	if (newline != std::string::npos)
	{
		_newlined = true;
		_textLeftover = _text.substr(newline + 1);
		_text = _text.substr(0, newline);
		_dirty = true;
	}

	// The last _numTrailingSpace characters are not shown. See below.
	if (textW(_style, _text.substr(0, _text.length() - _numTrailingSpace)) <= w)
	{
		return;
	}

	// Go backwards through the string, looking for a place to break.
	// Note that if _text is empty or contains only one byte, we never enter
	// the for loop. Also note that we stop before we reach the first byte,
	// because it is pointless (and dangerously recursive) to break everything.
	bool lastfittingcodepointfound = false;
	size_t cutoff = _text.length();
	_numTrailingSpace = 0;
	for (int i = ((int) _text.length()) - 1; i > 0; i--)
	{
		uint8_t byte = (uint8_t) _text[i];
		// If the two most-significant bits are 10, i.e. the byte is 10xxxxxx,
		// this is a UTF8 continuation byte.
		if ((byte & 0xC0) == 0x80)
		{
			continue;
		}

		if (lastfittingcodepointfound)
		{
			// If this character is a space, a tab or a zero width space,
			// this is an excellent position to start breaking the line.
			if (byte == ' ' || byte == '\t')
			{
				// Cut *after* this space.
				cutoff = i + 1;
				break;
			}
			else if (byte == 0xE2
				&& i + 2 < ((int) _text.length())
				&& ((uint8_t) _text[i + 1]) == 0x80
				&& ((uint8_t) _text[i + 2]) == 0x8B)
			{
				// Cut *after* this space.
				cutoff = i + 3;
				break;
			}
		}
		else if (textW(_style, _text.substr(0, i)) <= w)
		{
			// If this character is a space, a tab or a zero width space,
			// this is an excellent position to start breaking the line.
			// We will keep the space on this line even though it did not fit,
			// because visually it doesn't matter that much and it looks weird
			// to start the next line with a space character.
			// However we have to remember not to count this character when
			// determining the width in subsequent calls, and we also do not
			// draw this character for consistency's sake.
			if (byte == ' ' || byte == '\t')
			{
				// Cut *after* this space.
				cutoff = i + 1;
				_numTrailingSpace = 1;
				break;
			}
			else if (byte == 0xE2
				&& i + 2 < ((int) _text.length())
				&& ((uint8_t) _text[i + 1]) == 0x80
				&& ((uint8_t) _text[i + 2]) == 0x8B)
			{
				// Cut *after* this space.
				cutoff = i + 3;
				_numTrailingSpace = 3;
				break;
			}
			else
			{
				// We have found the start of the first codepoint that does not
				// fit, and therefore we have found the end of the last
				// codepoint that does. Remember it in case the entire remaining
				// line does not contain any spaces.
				lastfittingcodepointfound = true;
				cutoff = i;
			}
		}
		else
		{
			// Because we are at a codepoint, we could cutoff here if we have
			// to; we'd just rather not.
			cutoff = i;
		}
	}

	// Hopefully we found an appropriate point to break, but otherwise
	// we break at the first codepoint that won't fully fit.
	// TODO also consider grapheme clusters when breaking text
	if (cutoff < _text.length())
	{
		if (_newlined)
		{
			_textLeftover = _text.substr(cutoff) + "\n" + _textLeftover;
		}
		else
		{
			_textLeftover = _text.substr(cutoff);
		}
		_text = _text.substr(0, cutoff);
		_dirty = true;
	}
}

void TextField::setHeight(int)
{
	LOGW << "Trying to set height of a TextField.";
	DEBUG_ASSERT(false);
}

void TextField::place(const Pixel& topleft)
{
	_topleft = topleft;
	_thickness = 2;
}

void TextField::settleWidth()
{
	if (_fixedWidth) return;

	if (_dirty) prerender();

	_width = _displayText->width();
}

void TextField::refresh()
{
	if (!alive()) return;
	if (!shown()) return;

	if (hoverable() && enabled() && hovered())
	{
		if (!_hovered)
		{
			_hovered = true;
			setAlpha();
		}
	}
	else if (_hovered)
	{
		_hovered = false;
		setAlpha();
	}

	if (_dirty) prerender();

	Pixel pixel(_topleft);
	switch (_alignment)
	{
		case HorizontalAlignment::LEFT:
		break;
		case HorizontalAlignment::CENTER:
		{
			pixel.xenon = _topleft.xenon + _width / 2
				- _displayText->width() / 2;
		}
		break;
		case HorizontalAlignment::RIGHT:
		{
			pixel.xenon = _topleft.xenon + _width - _displayText->width();
		}
		break;
	}

	Collector::get()->add(_displayText, pixel);
}

std::string TextField::text() const
{
	return _text;
}

void TextField::setText(const std::string& str)
{
	if (str != _text)
	{
		_text = str;
		_dirty = true;
	}
}

void TextField::setTextColor(const Paint& color)
{
	_style.textcolor = color;
	_dirty = true;
}

bool TextField::overed() const
{
	if (!alive()) return false;
	Pixel mouse = Input::get()->mousePixel();
	if (   mouse.xenon >= _topleft.xenon && mouse.xenon < _topleft.xenon + _width
		&& mouse.yahoo >= _topleft.yahoo && mouse.yahoo < _topleft.yahoo + _height)
	{
		return true;
	}
	else return false;
}

bool TextField::hovered() const
{
	if (!hasMass()) return false;
	Pixel mouse = Input::get()->mousePixel();
	if (mouse.proximity.layer() > _topleft.proximity.layer()) return false;
	if (   mouse.xenon >= _topleft.xenon && mouse.xenon < _topleft.xenon + _width
		&& mouse.yahoo >= _topleft.yahoo && mouse.yahoo < _topleft.yahoo + _height)
	{
		Input::get()->layerMouse(_topleft.proximity.layer());
		return true;
	}
	else return false;
}

void TextField::bear(uint8_t depth)
{
	InterfaceElement::bear(depth);

	setAlpha();
}

void TextField::kill(uint8_t depth)
{
	InterfaceElement::kill(depth);

	setAlpha();
}

void TextField::enable(uint8_t depth)
{
	InterfaceElement::enable(depth);

	setAlpha();
}

void TextField::disable(uint8_t depth)
{
	InterfaceElement::disable(depth);

	setAlpha();
}

void TextField::power(uint8_t depth)
{
	InterfaceElement::power(depth);

	setAlpha();
}

void TextField::depower(uint8_t depth)
{
	InterfaceElement::depower(depth);

	setAlpha();
}

void TextField::setAlpha()
{
	_displayText->setAlpha(
		0.5 + 0.3 * enabled() + 0.2 * powered() - 0.3 * _hovered);
}
