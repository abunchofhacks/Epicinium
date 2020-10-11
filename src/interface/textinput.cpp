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
#include "textinput.hpp"
#include "source.hpp"

#include "libs/SDL2/SDL.h"

#include "color.hpp"
#include "collector.hpp"
#include "text.hpp"
#include "primitive.hpp"
#include "input.hpp"
#include "paint.hpp"
#include "colorname.hpp"
#include "textfield.hpp"
#include "frame.hpp"
#include "verticallayout.hpp"
#include "sizeiter.hpp"


const uint16_t TextInput::BLINKSPEED = 500;

TextInput::TextInput(int fontsize) :
	TextInput(fontsize, 1073741824)
{}

TextInput::TextInput(int fontsize, const Paint& color) :
	TextInput(fontsize, 1073741824, color)
{}

TextInput::TextInput(int fontsize, size_t maxLength) :
	TextInput(fontsize, maxLength, ColorName::TEXT800)
{}

TextInput::TextInput(int fontsize, size_t maxLength, const Paint& color) :
	_style(fontsize, color),
	_maxLengthInCodepoints(maxLength),
	_displayText(new Text("", _style)),
	_caret(new Primitive(Primitive::Type::RECTANGLE_FILLED,
		color,
		1,
		fontH(_style) / InterfaceElement::scale()
	)),
	_menu(Frame::makeTooltip(/*dropdown=*/true)),
	_blinkStart(SDL_GetTicks() % (2 * BLINKSPEED))
{
	_height = fontH(_style);
	setAlpha();

	Input::get()->reset();

	{
		InterfaceElement& it = *_menu;
		it.put(new VerticalLayout());
		it.add("cut", Frame::makeItem(ColorName::FRAMEITEM));
		it["cut"].put(new TextField(
			_("Cut"),
			fontsize));
		it["cut"].align(HorizontalAlignment::LEFT);
		it["cut"].makeClickable();
		it.add("copy", Frame::makeItem(ColorName::FRAMEITEM));
		it["copy"].put(new TextField(
			_("Copy"),
			fontsize));
		it["copy"].align(HorizontalAlignment::LEFT);
		it["copy"].makeClickable();
		it.add("paste", Frame::makeItem(ColorName::FRAMEITEM));
		it["paste"].put(new TextField(
			_("Paste"),
			fontsize));
		it["paste"].align(HorizontalAlignment::LEFT);
		it["paste"].makeClickable();
		it.setPadding(2 * InterfaceElement::scale());
		it.settle();
		it.fixWidth();
		it.kill();
	}
}

void TextInput::debugtree(uint8_t depth)
{
	std::string spaces(depth, '.');
	std::cout << spaces << "TextInput" << std::endl;
	std::cout << spaces << "Menu:" << std::endl;
	if (_menu) _menu->debugtree(depth + 1);
}

void TextInput::bear(uint8_t depth)
{
	InterfaceElement::bear(depth);

	if (_menu) _menu->bear(depth + 1);

	setAlpha();
}

void TextInput::kill(uint8_t depth)
{
	InterfaceElement::kill(depth);

	if (_menu) _menu->kill(depth + 1);

	// If we are killing the input field, instead of disabling/hiding it,
	// we are effectively destroying it. The contents are discarded since
	// the user will expect the 'new' field to be empty when it appears.
	reset();
}

void TextInput::show(uint8_t depth)
{
	InterfaceElement::show(depth);

	if (_menu) _menu->show(depth + 1);

	setAlpha();
}

void TextInput::hide(uint8_t depth)
{
	InterfaceElement::hide(depth);

	if (_menu) _menu->hide(depth + 1);

	setAlpha();
}

void TextInput::disable(uint8_t depth)
{
	InterfaceElement::disable(depth);
	depower();
}

void TextInput::power(uint8_t depth)
{
	InterfaceElement::power(depth);
	setAlpha();
}

void TextInput::depower(uint8_t depth)
{
	InterfaceElement::depower(depth);
	setAlpha();

	_dragging = false;
	_openPos = _caretPos;
	_closePos = _caretPos;
	_rectangle.reset();
	_menuOpened = false;
}

void TextInput::setWidth(int w)
{
	_width = w;
}

void TextInput::setHeight(int)
{
	LOGW << "Trying to set height of a TextInput.";
	DEBUG_ASSERT(false);
}

void TextInput::placeMenu(const Pixel& topleft)
{
	Pixel elementTopleft = topleft;
	elementTopleft.proximity = Layer::TOOLTIP;

	// Are we cut off by the right of the screen
	if (elementTopleft.xenon + _menu->width() + _menu->marginRight()
		>= InterfaceElement::windowW())
	{
		// Move the appropriate distance to the left
		elementTopleft.xenon = InterfaceElement::windowW() - _menu->width()
			- _menu->marginRight();
	}

	// Are we at the bottom of the screen?
	if (elementTopleft.yahoo + _menu->height() + _menu->marginBottom()
		>= InterfaceElement::windowH())
	{
		// Alignment bottom aligns the bottom of the tooltip to the top of the content.
		elementTopleft.yahoo = topleft.yahoo - _menu->height()
			- _menu->marginBottom();
	}

	_menu->place(elementTopleft);
}

void TextInput::place(const Pixel& topleft)
{
	_topleft = topleft;
	_thickness = 3;
	if (_menu) placeMenu(_topleft);
}

void TextInput::prerenderText()
{
	prerenderText(_text);
}

void TextInput::prerenderText(const std::string& fullText)
{
	std::string text = fullText.substr(_startPos);
	truncateTextForDisplay(text);

	// Ensure the caret fits in the displayed text.
	while (_startPos + text.length() < _caretPos)
	{
		progress(_startPos);

		// Because we cannot (accurately) guess how much space cutting this
		// codepoint gave us, we start all over again.
		text = fullText.substr(_startPos);
		truncateTextForDisplay(text);
	}
	_displayText.reset(new Text(text, _style));
}

void TextInput::truncateTextForDisplay(std::string& text)
{
	// Because we cannot (accurately) guess how long any piece of text is,
	// we cut one character at a time until the remaining text fits.
	while (textW(_style, text) > _width)
	{
		// Look for the last non-continuation byte.
		auto codepointstart = std::find_if(text.rbegin(), text.rend(),
			isNonContinuationByte);

		// Trim one codepoint at a time.
		size_t trimlength = 1 + std::distance(text.rbegin(), codepointstart);

		// Do not trim the entire text, because then we have nothing to draw.
		if (trimlength >= text.length())
		{
			return;
		}
		text = text.substr(0, text.length() - trimlength);
	}
}

void TextInput::progress(size_t& pos)
{
	pos++;
	while (pos < _text.size() && isContinuationByte(_text[pos]))
	{
		pos++;
	}
}

void TextInput::regress(size_t& pos)
{
	pos--;
	while (pos > 0 && isContinuationByte(_text[pos]))
	{
		pos--;
	}
}

void TextInput::prerenderSelection()
{
	if (_openPos < _closePos)
	{
		size_t pos = std::max(_startPos, _openPos);
		size_t len = std::min(_displayText->text().length(), _closePos - pos);
		std::string leftTextOpen =
			_displayText->text().substr(0, pos - _startPos);
		std::string leftTextClose =
			_displayText->text().substr(0, pos - _startPos + len);
		// To make sure we can line up with the caret, align to scale grid.
		int x0 = textW(_style, leftTextOpen) / InterfaceElement::scale()
			* InterfaceElement::scale();
		int x1 = textW(_style, leftTextClose) / InterfaceElement::scale()
			* InterfaceElement::scale();
		int w = x1 - x0;
		_rectangle.reset(new Primitive(Primitive::Type::RECTANGLE_FILLED,
			Paint(ColorName::TEXTSELECTION),
			w / InterfaceElement::scale(),
			_height / InterfaceElement::scale()
		));
		_rectangleTopleft = _topleft;
		_rectangleTopleft.xenon += x0;
	}
	else _rectangle.reset();
}

void TextInput::prerenderCaret()
{
	std::string leftText = _displayText->text().substr(0, _caretPos - _startPos);
	// To make sure we can line up with the rectangle, align to scale grid.
	int x = textW(_style, leftText) / InterfaceElement::scale()
		* InterfaceElement::scale();
	_caretTopleft = _topleft;
	_caretTopleft.xenon += x - InterfaceElement::scale();
	_caretTopleft.proximity += 2;
}

void TextInput::prerender()
{
	if (_startPos > _caretPos) _startPos = _caretPos;

	setAlpha();
	prerenderText();
	prerenderSelection();
	prerenderCaret();

	_blinkStart = SDL_GetTicks() % (2 * BLINKSPEED);
	_dirty = false;
}

void TextInput::filterUserInput(std::string& input)
{
	std::replace(input.begin(), input.end(), '\t', ' ');
	std::replace(input.begin(), input.end(), '\r', ' ');
	std::replace(input.begin(), input.end(), '\n', ' ');
}

void TextInput::insertText(std::string input)
{
	filterUserInput(input);

	// The number of codepoints in a UTF8-encoded string is equal to the
	// number of non-continuation bytes.
	size_t num = std::count_if(_text.begin(), _text.end(),
		isNonContinuationByte);

	size_t rem = _maxLengthInCodepoints - std::min(_maxLengthInCodepoints, num);
	if (_openPos < _closePos)
	{
		rem += std::count_if(_text.begin() + _openPos,
			_text.begin() + _closePos,
			isNonContinuationByte);
		// Truncate input after rem codepoints, so truncate at the start of the
		// next codepoint after that if there are more than rem codepoints.
		auto cutoff = std::find_if(input.begin(), input.end(), [&rem](char c){
			return (isNonContinuationByte(c) && (rem--) == 0);
		});
		input.erase(cutoff, input.end());
		_text = _text.substr(0, _openPos)
			+ input
			+ _text.substr(_closePos);
		_openPos += input.length();
		_caretPos = _openPos;
		_closePos = _openPos;
	}
	else
	{
		// Truncate input after rem codepoints, so truncate at the start of the
		// next codepoint after that if there are more than rem codepoints.
		auto cutoff = std::find_if(input.begin(), input.end(), [&rem](char c){
			return (isNonContinuationByte(c) && (rem--) == 0);
		});
		input.erase(cutoff, input.end());
		_text = _text.substr(0, _caretPos)
			+ input
			+ _text.substr(_caretPos);
		_caretPos += input.length();
		_openPos = _caretPos;
		_closePos = _caretPos;
	}
	_dirty = true;
}

void TextInput::refresh()
{
	if (!alive())
	{
		if (_wasHovered)
		{
			_wasHovered = false;
			static SDL_Cursor* cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
			SDL_SetCursor(cursor);
		}
		return;
	}

	if (powered())
	{
		Input* input = Input::get();
		if (input->wasTextInput())
		{
			insertText(input->inputText());
		}
		// TODO probably using editText wrongly
		else if (input->wasTextEdited())
		{
			insertText(input->editText());
		}

		if ((input->wasKeyPressed(SDL_SCANCODE_LEFT)
			|| input->wasKeyRepeated(SDL_SCANCODE_LEFT))
			&& _caretPos > 0)
		{
			regress(_caretPos);
			if (input->isKeyHeld(SDL_SCANCODE_SHIFT))
			{
				if (_caretPos < _openPos) regress(_openPos);
				else regress(_closePos);
			}
			else
			{
				_openPos = _caretPos;
				_closePos = _caretPos;
			}
			_dirty = true;
		}

		if ((input->wasKeyPressed(SDL_SCANCODE_RIGHT)
			|| input->wasKeyRepeated(SDL_SCANCODE_RIGHT))
			&& _caretPos < _text.length())
		{
			progress(_caretPos);
			if (input->isKeyHeld(SDL_SCANCODE_SHIFT))
			{
				if (_caretPos > _closePos) progress(_closePos);
				else progress(_openPos);
			}
			else
			{
				_openPos = _caretPos;
				_closePos = _caretPos;
			}
			_dirty = true;
		}

		if (input->wasKeyPressed(SDL_SCANCODE_BACKSPACE)
			|| input->wasKeyRepeated(SDL_SCANCODE_BACKSPACE))
		{
			if (_openPos < _closePos)
			{
				_text = _text.substr(0, _openPos)
					+ _text.substr(_closePos);
				_startPos -= std::min(_startPos, _closePos - _openPos);
				_caretPos = _openPos;
				_closePos = _openPos;
				_dirty = true;
			}
			else if (_caretPos > 0)
			{
				size_t oldpos = _caretPos;
				regress(_caretPos);
				_text = _text.substr(0, _caretPos) + _text.substr(oldpos);
				if (_startPos > 0) regress(_startPos);
				_dirty = true;
			}
		}

		if (input->wasKeyPressed(SDL_SCANCODE_DELETE)
			|| input->wasKeyRepeated(SDL_SCANCODE_DELETE))
		{
			if (_openPos < _closePos)
			{
				_text = _text.substr(0, _openPos)
					+ _text.substr(_closePos);
				_startPos -= std::min(_startPos, _closePos - _openPos);
				_caretPos = _openPos;
				_closePos = _openPos;
				_dirty = true;
			}
			else if (_caretPos < _text.length())
			{
				size_t cutpos = _caretPos;
				progress(cutpos);
				_text = _text.substr(0, _caretPos) + _text.substr(cutpos);
				_dirty = true;
			}
		}

		{
			auto& menu = *_menu;

			menu["copy"].enableIf(_copyable && _openPos < _closePos);
			menu["cut"].enableIf(_copyable && _openPos < _closePos);
			menu["paste"].enableIf(SDL_HasClipboardText());

			if (((Input::get()->isKeyHeld(SDL_SCANCODE_CTRL)
				&& Input::get()->wasKeyPressed(SDL_SCANCODE_C))
				|| menu["copy"].clicked())
				&& _copyable
				&& _openPos < _closePos)
			{
				SDL_SetClipboardText(
					_text.substr(_openPos, _closePos - _openPos).c_str());
			}

			if (((Input::get()->isKeyHeld(SDL_SCANCODE_CTRL)
				&& Input::get()->wasKeyPressed(SDL_SCANCODE_X))
				|| menu["cut"].clicked())
				&& _copyable
				&& _openPos < _closePos)
			{
				SDL_SetClipboardText(
					_text.substr(_openPos, _closePos - _openPos).c_str());
				_text = _text.substr(0, _openPos)
					+ _text.substr(_closePos);
				_startPos -= std::min(_startPos, _closePos - _openPos);
				_caretPos = _openPos;
				_closePos = _openPos;
				_dirty = true;
			}

			if (((Input::get()->isKeyHeld(SDL_SCANCODE_CTRL)
				&& Input::get()->wasKeyPressed(SDL_SCANCODE_V))
				|| menu["paste"].clicked())
				&& SDL_HasClipboardText())
			{
				std::string cbText(SDL_GetClipboardText());
				insertText(cbText);
			}

			if (Input::get()->isKeyHeld(SDL_SCANCODE_CTRL)
				&& Input::get()->wasKeyPressed(SDL_SCANCODE_A))
			{
				_openPos = 0;
				_closePos = _text.length();
				_caretPos = _closePos;
				 _dirty = true;
			}
		}
	}

	if (powered() && rightClicked())
	{
		_menuOpened = true;
		Pixel topleft = Input::get()->mousePixel();
		topleft.proximity = _menu->topleft().proximity;
		placeMenu(topleft);
	}
	else if (_wasClicked)
	{
		_menuOpened = false;
		_wasClicked = false;
	}
	else if (clicked())
	{
		_wasClicked = true;
	}
	else if (!hovered() && Input::get()->wasKeyPressed(SDL_SCANCODE_LMB))
	{
		_menuOpened = false;
	}

	if (_menuOpened)
	{
		_menu->bear();
	}
	else _menu->kill();

	_menu->refresh();

	if (!shown()) return;

	if (!_wasPowered && powered())
	{
		_blinkStart = SDL_GetTicks() % (2 * BLINKSPEED);
		if (clicked())
		{
			_caretPos = determinePixelPosition(Input::get()->mousePixel());
		}
		_openPos = _caretPos;
		_closePos = _caretPos;
		_dirty = true;
	}
	_wasPowered = powered();

	if (hovered())
	{
		if (held() && powered() && !_menu->clicked())
		{
			size_t oldpos = _caretPos;
			_caretPos = determinePixelPosition(Input::get()->mousePixel());
			if (clicked())
			{
				_openPos = _caretPos;
				_closePos = _caretPos;
				_dirty = true;
			}
			else if (!_menuOpened)
			{
				if (_caretPos > oldpos)
				{
					if (oldpos >= _closePos)
					{
						_closePos = _caretPos;
					}
					else if (_caretPos > _closePos)
					{
						_openPos = _closePos;
						_closePos = _caretPos;
					}
					else
					{
						_openPos = _caretPos;
					}
					_dragging = true;
					_dirty = true;
				}
				else if (_caretPos < oldpos)
				{
					if (oldpos <= _openPos)
					{
						_openPos = _caretPos;
					}
					else if (_caretPos < _openPos)
					{
						_closePos = _openPos;
						_openPos = _caretPos;
					}
					else
					{
						_closePos = _caretPos;
					}
					_dragging = true;
					_dirty = true;
				}
			}
		}
		else _dragging = false;

		if (enabled() && !_wasHovered)
		{
			_wasHovered = true;
			static SDL_Cursor* cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_IBEAM);
			SDL_SetCursor(cursor);
		}
	}
	else
	{
		_dragging = false;

		if (_wasHovered)
		{
			_wasHovered = false;
			static SDL_Cursor* cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
			SDL_SetCursor(cursor);
		}
	}

	if (_dirty) prerender();
	Pixel topleft = _topleft;
	topleft.proximity += 1;
	if (_rectangle)
	{
		Collector::get()->add(_rectangle, _rectangleTopleft);
	}
	Collector::get()->add(_displayText, topleft);
	if (powered() && (SDL_GetTicks() + (2 * BLINKSPEED) - _blinkStart) % (2 * BLINKSPEED) < BLINKSPEED)
	{
		Collector::get()->add(_caret, _caretTopleft);
	}
}

void TextInput::reset()
{
	_text = _prefillText;
	_startPos = 0;
	_caretPos = 0;
	_openPos = 0;
	_closePos = 0;
	_dirty = true;
	_dragging = false;
}

void TextInput::setText(const std::string& str)
{
	if (str == text()) return;
	_text = str;
	_startPos = 0;
	_caretPos = _text.length();
	_openPos = _caretPos;
	_closePos = _caretPos;
	_dirty = true;
}

void TextInput::prefillText(const std::string& str)
{
	_prefillText = str;
	reset();
}

bool TextInput::overed() const
{
	if (!alive()) return false;
	bool result = false;
	if (_menu && _menu->hovered()) result = true;

	Pixel mouse = Input::get()->mousePixel();
	if (   mouse.xenon >= _topleft.xenon && mouse.xenon < _topleft.xenon + _width
		&& mouse.yahoo >= _topleft.yahoo && mouse.yahoo < _topleft.yahoo + _height)
	{
		return true;
	}
	else return result;
}

bool TextInput::hovered() const
{
	if (!hasMass()) return false;
	bool result = false;
	if (_menu && _menu->hovered()) result = true;

	Pixel mouse = Input::get()->mousePixel();
	if (_dragging && mouse.proximity.layer() <= Layer::TOOLTIP)
	{
		// Users can select text by clicking and dragging a selection box.
		// While doing this, we fake the existence of a hovered overlay.
		Input::get()->layerMouse(Layer::TOOLTIP);
		return true;
	}
	if (mouse.proximity.layer() > _topleft.proximity.layer()) return result;
	if (   mouse.xenon >= _topleft.xenon && mouse.xenon < _topleft.xenon + _width
		&& mouse.yahoo >= _topleft.yahoo && mouse.yahoo < _topleft.yahoo + _height)
	{
		Input::get()->layerMouse(_topleft.proximity.layer());
		return true;
	}
	else return result;
}

void TextInput::setAlpha()
{
	_displayText->setAlpha(0.5 + 0.3 * enabled() + 0.2 * powered());
}

size_t TextInput::determinePixelPosition(const Pixel& pixel)
{
	SizeIter iter = std::lower_bound(
		SizeIter(0),
		SizeIter(_text.length()),
		pixel,
		[this](size_t lhs, const Pixel& rhs) {

			size_t pos = lhs;
			while (pos > 0 && isContinuationByte(_text[pos]))
			{
				pos--;
			}
			int wl = textW(_style, _text.substr(0, _startPos));
			int w0 = textW(_style, _text.substr(0, pos));
			pos++;
			while (pos < _text.size() && isContinuationByte(_text[pos]))
			{
				pos++;
			}
			int w1 = textW(_style, _text.substr(0, pos));
			int textxenon = _topleft.xenon - wl + (w0 + w1) / 2;
			return (textxenon < rhs.xenon);
		});
	return *iter;
}
