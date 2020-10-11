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
#include "multitextinput.hpp"
#include "source.hpp"

#include "libs/SDL2/SDL.h"

#include "color.hpp"
#include "collector.hpp"
#include "textfield.hpp"
#include "primitive.hpp"
#include "input.hpp"
#include "paint.hpp"
#include "colorname.hpp"
#include "textfield.hpp"
#include "frame.hpp"
#include "sizeiter.hpp"


const uint16_t MultiTextInput::BLINKSPEED = 500;

MultiTextInput::MultiTextInput(int fontsize, size_t lines) :
	MultiTextInput(fontsize, lines, ColorName::TEXT800)
{}

MultiTextInput::MultiTextInput(int fontsize, size_t lines, const Paint& color) :
	ScrollableLayout(lines, fontH(fontsize), false, false),
	_style(fontsize, color),
	_caret(new Primitive(Primitive::Type::RECTANGLE_FILLED,
		color,
		1,
		fontH(_style) / InterfaceElement::scale()
	)),
	_menu(Frame::makeTooltip(/*dropdown=*/true)),
	_blinkStart(SDL_GetTicks() % (2 * BLINKSPEED))
{
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

void MultiTextInput::debugtree(uint8_t depth)
{
	std::string spaces(depth, '.');
	std::cout << spaces << "MultiTextInput" << std::endl;
	std::cout << spaces << "Menu:" << std::endl;
	if (_menu) _menu->debugtree(depth + 1);
}

void MultiTextInput::bear(uint8_t depth)
{
	InterfaceElement::bear(depth);

	if (_menu) _menu->bear(depth + 1);

	setAlpha();
}

void MultiTextInput::kill(uint8_t depth)
{
	InterfaceElement::kill(depth);

	if (_menu) _menu->kill(depth + 1);

	// If we are killing the input field, instead of disabling/hiding it,
	// we are effectively destroying it. The contents are discarded since
	// the user will expect the 'new' field to be empty when it appears.
	reset();
}

void MultiTextInput::show(uint8_t depth)
{
	InterfaceElement::show(depth);

	if (_menu) _menu->show(depth + 1);

	setAlpha();
}

void MultiTextInput::hide(uint8_t depth)
{
	InterfaceElement::hide(depth);

	if (_menu) _menu->hide(depth + 1);

	setAlpha();
}

void MultiTextInput::power(uint8_t depth)
{
	InterfaceElement::power(depth);

	for (auto& element : _elements)
	{
		element->power(depth + 1);
	}

	setAlpha();
}

void MultiTextInput::depower(uint8_t depth)
{
	InterfaceElement::depower(depth);

	for (auto& element : _elements)
	{
		element->depower(depth + 1);
	}

	setAlpha();

	_dragging = false;
	_openPos = _caretPos;
	_closePos = _caretPos;
	for (auto& rectangle : _rectangles)
	{
		rectangle.reset();
	}
	_menuOpened = false;
}

void MultiTextInput::disable(uint8_t depth)
{
	InterfaceElement::disable(depth);
	depower();
}

void MultiTextInput::placeMenu(const Pixel& topleft)
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

void MultiTextInput::place(const Pixel& topleft)
{
	ScrollableLayout::place(topleft);
	_thickness = 3;
	if (_menu) placeMenu(_topleft);
	_posDirty = true;
}

void MultiTextInput::setWidth(int w)
{
	if (w < 0 && !_fixedWidth)
	{
		// This is a little trick that we can use specifically on MultiTextFields:
		// when you settle their height, they return to only one line,
		// which allows you to get their natural width.
		// In contrast, settleWidth() tries to get the width as small as possible
		// because otherwise it cannot stretch while settling everything else.
		generate();
		return;
	}

	int old = _fixedWidth;
	_width = w;
	_fixedWidth = _width;

	generate();

	_fixedWidth = old;
}

void MultiTextInput::setHeight(int)
{
	LOGW << "Trying to set height of a MultiTextInput.";
	DEBUG_ASSERT(false);
}

void MultiTextInput::settleWidth()
{
	if (_fixedWidth) return;

	int old = _fixedWidth;
	_width = std::min(_width, 200 * InterfaceElement::scale());
	_fixedWidth = _width;

	generate();

	_fixedWidth = old;
}

void MultiTextInput::settleHeight()
{
	return InterfaceElement::settleHeight();
}

void MultiTextInput::generate()
{
	VerticalLayout::reset();

	if (_fixedWidth)
	{
		std::string leftover = _text;
		int line = 0;
		do
		{
			std::unique_ptr<TextField> textfield(
				new TextField(leftover, _style));
			textfield->setWidth(_width);
			leftover = textfield->textLeftover();
			add(std::to_string(line), std::move(textfield));
			line++;

		} while (!leftover.empty());
	}
	else
	{
		add("0", new TextField(_text, _style));
		_width = _elements[0]->width();
	}

	calculateHeight();
	ScrollableLayout::place(_topleft);

	_textDirty = false;
}

void MultiTextInput::prerenderSelection()
{
	if (_openPos < _closePos)
	{
		_rectangles.resize(_elements.size());
		_rectangleToplefts.resize(_elements.size());
		size_t pos = 0;
		for (size_t i = 0; i < _elements.size(); i++)
		{
			size_t lineLength = _elements[i]->text().length();
			int lineOpenPos = -1;
			if (_openPos < pos) lineOpenPos = 0;
			else if (_openPos <= pos + lineLength) lineOpenPos = _openPos - pos;
			int lineClosePos = -1;
			if (_closePos > pos + lineLength) lineClosePos = lineLength;
			else if (_closePos >= pos) lineClosePos = _closePos - pos;
			if (!_elements[i]->shown() || lineOpenPos < 0 || lineClosePos < 0)
			{
				_rectangles[i] = nullptr;
				pos += lineLength;
				continue;
			}
			Pixel pixelOpen = determineLineAndOffsetPixel(i, lineOpenPos);
			Pixel pixelClose = determineLineAndOffsetPixel(i, lineClosePos);
			int w = pixelClose.xenon - pixelOpen.xenon;
			int h = _elements[i]->height();

			_rectangles[i] = std::make_shared<Primitive>(
				Primitive::Type::RECTANGLE_FILLED,
				Paint(ColorName::TEXTSELECTION),
				w / InterfaceElement::scale(),
				h / InterfaceElement::scale()
			);
			_rectangleToplefts[i] = pixelOpen;

			pos += lineLength;
		}
	}
	else
	{
		for (auto& rectangle : _rectangles)
		{
			rectangle.reset();
		}
	}
}

void MultiTextInput::prerenderCaret()
{
	size_t caretLine, caretLinePos;
	std::tie(caretLine, caretLinePos) = determineLineAndOffset(_caretPos);
	if (_movedCaret && caretLine < _elements.size() && !_elements[caretLine]->shown())
	{
		if (int(caretLine) < offset()) scroll(int(caretLine) - offset());
		else scroll(int(caretLine) - capacity() - offset() + 1);
	}
	if (caretLine < _elements.size() && _elements[caretLine]->shown())
	{
		_caretTopleft = determineLineAndOffsetPixel(caretLine, caretLinePos);
		_caretTopleft.xenon -= InterfaceElement::scale();
		_caretTopleft.proximity += 2;
	}
	else
	{
		_caretTopleft = Pixel(0, 0);
	}
	_movedCaret = false;
}

void MultiTextInput::prerender()
{
	setAlpha();
	prerenderSelection();
	prerenderCaret();

	_blinkStart = SDL_GetTicks() % (2 * BLINKSPEED);
	_posDirty = false;
}

void MultiTextInput::filterUserInput(std::string& input)
{
	std::replace(input.begin(), input.end(), '\t', ' ');
	std::replace(input.begin(), input.end(), '\r', ' ');
	std::replace(input.begin(), input.end(), '\n', ' ');
}

void MultiTextInput::insertText(std::string input)
{
	filterUserInput(input);

	if (_openPos < _closePos)
	{
		_text = _text.substr(0, _openPos)
			+ input
			+ _text.substr(_closePos);
		_openPos += input.length();
		_caretPos = _openPos;
		_closePos = _openPos;
	}
	else
	{
		_text = _text.substr(0, _caretPos)
			+ input
			+ _text.substr(_caretPos);
		_caretPos += input.length();
		_openPos = _caretPos;
		_closePos = _caretPos;
	}

	_movedCaret = true;
	_textDirty = true;
	_posDirty = true;
}

void MultiTextInput::progress(size_t& pos)
{
	pos++;
	while (pos < _text.size() && isContinuationByte(_text[pos]))
	{
		pos++;
	}
}

void MultiTextInput::regress(size_t& pos)
{
	pos--;
	while (pos > 0 && isContinuationByte(_text[pos]))
	{
		pos--;
	}
}

void MultiTextInput::refresh()
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
			_movedCaret = true;
			_posDirty = true;
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
			_movedCaret = true;
			_posDirty = true;
		}

		if (input->wasKeyPressed(SDL_SCANCODE_UP)
			|| input->wasKeyRepeated(SDL_SCANCODE_UP))
		{
			size_t line, linePos;
			std::tie(line, linePos) = determineLineAndOffset(_caretPos);
			if (line > 0)
			{
				Pixel source = determineLineAndOffsetPixel(line, linePos);
				Pixel target = _elements[line - 1]->topleft();
				target.xenon = source.xenon;
				_caretPos = determinePixelPosition(target);
				if (input->isKeyHeld(SDL_SCANCODE_SHIFT))
				{
					if (_caretPos < _openPos) _openPos = _caretPos;
					else _closePos = _caretPos;
				}
				else
				{
					_openPos = _caretPos;
					_closePos = _caretPos;
				}
				_movedCaret = true;
				_posDirty = true;
			}
		}

		if (input->wasKeyPressed(SDL_SCANCODE_DOWN)
			|| input->wasKeyRepeated(SDL_SCANCODE_DOWN))
		{
			size_t line, linePos;
			std::tie(line, linePos) = determineLineAndOffset(_caretPos);
			if (line + 1 < _elements.size())
			{
				Pixel source = determineLineAndOffsetPixel(line, linePos);
				Pixel target = _elements[line + 1]->topleft();
				target.xenon = source.xenon;
				_caretPos = determinePixelPosition(target);
				if (input->isKeyHeld(SDL_SCANCODE_SHIFT))
				{
					if (_caretPos > _closePos) _closePos = _caretPos;
					else _openPos = _caretPos;
				}
				else
				{
					_openPos = _caretPos;
					_closePos = _caretPos;
				}
				_movedCaret = true;
				_posDirty = true;
			}
		}

		if (input->wasKeyPressed(SDL_SCANCODE_BACKSPACE)
			|| input->wasKeyRepeated(SDL_SCANCODE_BACKSPACE))
		{
			if (_openPos < _closePos)
			{
				_text = _text.substr(0, _openPos)
					+ _text.substr(_closePos);
				_caretPos = _openPos;
				_closePos = _openPos;
				_movedCaret = true;
				_textDirty = true;
				_posDirty = true;
			}
			else if (_caretPos > 0)
			{
				size_t oldpos = _caretPos;
				regress(_caretPos);
				_text = _text.substr(0, _caretPos) + _text.substr(oldpos);
				_movedCaret = true;
				_textDirty = true;
				_posDirty = true;
			}
		}

		if (input->wasKeyPressed(SDL_SCANCODE_DELETE)
			|| input->wasKeyRepeated(SDL_SCANCODE_DELETE))
		{
			if (_openPos < _closePos)
			{
				_text = _text.substr(0, _openPos)
					+ _text.substr(_closePos);
				_caretPos = _openPos;
				_closePos = _openPos;
				_movedCaret = true;
				_posDirty = true;
				_textDirty = true;
			}
			else if (_caretPos < _text.length())
			{
				size_t cutpos = _caretPos;
				progress(cutpos);
				_text = _text.substr(0, _caretPos) + _text.substr(cutpos);
				_textDirty = true;
			}
		}

		{
			auto& menu = *_menu;

			menu["copy"].enableIf(_openPos < _closePos);
			menu["cut"].enableIf(_openPos < _closePos);
			menu["paste"].enableIf(SDL_HasClipboardText());

			if (((Input::get()->isKeyHeld(SDL_SCANCODE_CTRL)
				&& Input::get()->wasKeyPressed(SDL_SCANCODE_C))
				|| menu["copy"].clicked())
				&& _openPos < _closePos)
			{
				SDL_SetClipboardText(
					_text.substr(_openPos, _closePos - _openPos).c_str());
			}

			if (((Input::get()->isKeyHeld(SDL_SCANCODE_CTRL)
				&& Input::get()->wasKeyPressed(SDL_SCANCODE_X))
				|| menu["cut"].clicked())
				&& _openPos < _closePos)
			{
				SDL_SetClipboardText(
					_text.substr(_openPos, _closePos - _openPos).c_str());
				_text = _text.substr(0, _openPos)
					+ _text.substr(_closePos);
				_caretPos = _openPos;
				_closePos = _openPos;
				_movedCaret = true;
				_textDirty = true;
				_posDirty = true;
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
				_movedCaret = true;
				 _posDirty = true;
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
		if (clicked()) _caretPos = determinePixelPosition(Input::get()->mousePixel());
		_openPos = _caretPos;
		_closePos = _caretPos;
		_posDirty = true;
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
				_posDirty = true;
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
					_posDirty = true;
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
					_posDirty = true;
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

	if (_textDirty) generate();
	ScrollableLayout::refresh();
	if (_posDirty) prerender();
	Pixel topleft = _topleft;
	topleft.yahoo += InterfaceElement::scale();
	topleft.proximity += 1;
	for (size_t i = 0; i < _rectangles.size(); i++)
	{
		if (!_rectangles[i]) continue;
		Collector::get()->add(_rectangles[i], _rectangleToplefts[i]);
	}
	if (powered() && (SDL_GetTicks() + (2 * BLINKSPEED) - _blinkStart) % (2 * BLINKSPEED) < BLINKSPEED)
	{
		Collector::get()->add(_caret, _caretTopleft);
	}
}

void MultiTextInput::reset()
{
	_text = _prefillText;
	_caretPos = _text.length();
	_openPos = _caretPos;
	_closePos = _caretPos;
	_textDirty = true;
	_posDirty = true;
	_dragging = false;
}

void MultiTextInput::setText(const std::string& str)
{
	if (str == text()) return;
	_text = str;
	_caretPos = _text.length();
	_openPos = _caretPos;
	_closePos = _caretPos;
	_textDirty = true;
	_posDirty = true;
}

void MultiTextInput::prefillText(const std::string& str)
{
	_prefillText = str;
	reset();
}

bool MultiTextInput::overed() const
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

bool MultiTextInput::hovered() const
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

bool MultiTextInput::clicked() const
{
	// This function is overriden in Layout to check if one of the elements is
	// clicked, but here we do not want that behavior so we use the original
	// implementation.
	return InterfaceElement::clicked();
}

bool MultiTextInput::held() const
{
	// This function is overriden in Layout to check if one of the elements is
	// held, but here we do not want that behavior so we use the original
	// implementation.
	return InterfaceElement::held();
}

bool MultiTextInput::released() const
{
	// This function is overriden in Layout to check if one of the elements is
	// released, but here we do not want that behavior so we use the original
	// implementation.
	return InterfaceElement::released();
}

void MultiTextInput::setAlpha()
{
	// Nothing to do.
}

size_t MultiTextInput::determinePixelPosition(const Pixel& pixel)
{
	if (_textDirty) generate();

	size_t pos = 0;
	for (auto& line : _elements)
	{
		if (line->topleft().yahoo + line->height() <= pixel.yahoo)
		{
			pos += line->text().length();
			continue;
		}
		SizeIter iter = std::lower_bound(
			SizeIter(0),
			SizeIter(line->text().length()),
			pixel,
			[this, &line](size_t lhs, const Pixel& rhs) {

				size_t i = lhs;
				while (i > 0 && isContinuationByte(line->text()[i]))
				{
					i--;
				}
				int w0 = textW(_style, line->text().substr(0, i));
				i++;
				while (i < line->text().size()
					&& isContinuationByte(line->text()[i]))
				{
					i++;
				}
				int w1 = textW(_style, line->text().substr(0, i));
				int textxenon = line->topleft().xenon + (w0 + w1) / 2;
				return (textxenon < rhs.xenon);
			});
		pos += *iter;
		break;
	}
	return pos;
}

std::pair<size_t, size_t> MultiTextInput::determineLineAndOffset(size_t pos)
{
	if (_textDirty) generate();

	size_t lineNr = 0;
	size_t linePos = pos;
	for (auto& line : _elements)
	{
		if (linePos <= line->text().size()) break;
		linePos -= line->text().size();
		lineNr++;
	}
	return std::make_pair(lineNr, linePos);
}

Pixel MultiTextInput::determineLineAndOffsetPixel(size_t line, size_t linePos)
{
	if (_textDirty) generate();

	// Backtrack to the nearest codepoint start.
	while (linePos > 0 && linePos < _elements[line]->text().size()
		&& isContinuationByte(_elements[line]->text()[linePos]))
	{
		linePos--;
	}

	std::string leftText =
		_elements[line]->text().substr(0, linePos);
	// To make sure we can line up with the rectangles, align to scale grid.
	int x = textW(_style, leftText) / InterfaceElement::scale()
		* InterfaceElement::scale();
	Pixel pixel = _elements[line]->topleft();
	pixel.xenon += x;
	return pixel;
}
