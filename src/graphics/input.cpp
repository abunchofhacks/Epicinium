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
#include "input.hpp"
#include "source.hpp"

#include "libs/SDL2/SDL.h"

#include "loop.hpp"
#include "camera.hpp"
#include "dictator.hpp"


constexpr float _threshold = 0.25f;
constexpr float _doubleclickHeldFactor = 0.5f;
constexpr float _significantMoveDistance = 10/*px*/;
constexpr float _tooltipThreshold = 0.10f;

Input* Input::_installed = nullptr;

Input::Input()
{
	for (size_t i = 0; i < 512; i++)
	{
		_time[i] = 999;/*infty*/
	}
}

void Input::install()
{
	_installed = this;
}

void Input::reset()
{
	_pressed.reset();
	_released.reset();
	_repeated.reset();
	_longpressed.reset();
	_longreleased.reset();
	_shortreleased.reset();
	_doublepressed.reset();
	_input = false;
	_edited = false;
	_inputText = "";
	_editText = "";
	_scroll = 0;
	for (size_t i = 0; i < 512; i++)
	{
		_time[i] += Loop::delta();
		if (!_held[i] || _longheld[i] || _time[i] < _threshold) continue;
		_longheld[i] = true;
		_longpressed[i] = true;
	}
}

void Input::handle(const SDL_Event& event)
{
	switch (event.type)
	{
	case SDL_TEXTINPUT:       textInput(event);                          break;
	case SDL_TEXTEDITING:     textEdit(event);                           break;
	case SDL_KEYDOWN:
	{
		if (event.key.repeat == 0) keyDown(event);
		else keyRepeat(event);
	}
	break;
	case SDL_KEYUP:           keyUp(event);                              break;
	case SDL_MOUSEBUTTONDOWN: mouseDown(event);                          break;
	case SDL_MOUSEBUTTONUP:   mouseUp(event);                            break;
	case SDL_MOUSEWHEEL:      mouseScroll(event);                        break;
	default:                                                             break;
	}
}

void Input::down(const SDL_Scancode key)
{
	_held[key] = true;
	_pressed[key] = true;
	if (_time[key] < _threshold)
	{
		_doublepressed[key] = true;
	}
	_time[key] = 0;
}

void Input::up(const SDL_Scancode key)
{
	if (!_held[key]) return;
	_held[key] = false;
	_released[key] = true;
	if (_longheld[key])
	{
		_longheld[key] = false;
		_longreleased[key] = true;
	}
	else
	{
		_shortreleased[key] = true;
	}
	// On the next down event, we detect double-click by measuring the time
	// since the previous down event. To be more forgiving for old people
	// (such as me), we only count the time spent holding down the button
	// for a certain percentage. If _doubleclickHeldFactor == 0, we instead
	// measure the time between this up event and the next down event.
	_time[key] *= _doubleclickHeldFactor;
}

void Input::repeat(const SDL_Scancode key)
{
	_repeated[key] = true;
}

void Input::keyDown(const SDL_Event& event)
{
	down(event.key.keysym.scancode);
	switch (event.key.keysym.scancode)
	{
	case SDL_SCANCODE_LCTRL:
	case SDL_SCANCODE_RCTRL:    down(SDL_SCANCODE_CTRL);    break;
	case SDL_SCANCODE_LSHIFT:
	case SDL_SCANCODE_RSHIFT:   down(SDL_SCANCODE_SHIFT);   break;
	case SDL_SCANCODE_LALT:
	case SDL_SCANCODE_RALT:     down(SDL_SCANCODE_ALT);     break;
	default:                                                break;
	}
}

void Input::keyUp(const SDL_Event& event)
{
	up(event.key.keysym.scancode);
	switch (event.key.keysym.scancode)
	{
	case SDL_SCANCODE_LCTRL:
	case SDL_SCANCODE_RCTRL:    up(SDL_SCANCODE_CTRL);      break;
	case SDL_SCANCODE_LSHIFT:
	case SDL_SCANCODE_RSHIFT:   up(SDL_SCANCODE_SHIFT);     break;
	case SDL_SCANCODE_LALT:
	case SDL_SCANCODE_RALT:     up(SDL_SCANCODE_ALT);       break;
	default:                                                break;
	}
}

void Input::keyRepeat(const SDL_Event& event)
{
	repeat(event.key.keysym.scancode);
}

void Input::mouseDown(const SDL_Event& event)
{
	if (event.button.clicks > 1)
		mouseDoubleDown(event);

	switch (event.button.button)
	{
	case SDL_BUTTON_LEFT:       down(SDL_SCANCODE_LMB);     break;
	case SDL_BUTTON_RIGHT:      down(SDL_SCANCODE_RMB);     break;
	case SDL_BUTTON_MIDDLE:     down(SDL_SCANCODE_MMB);     break;
	default:                                                break;
	}
}

void Input::mouseUp(const SDL_Event& event)
{
	if (event.button.clicks > 1)
		mouseDoubleUp(event);

	switch (event.button.button)
	{
	case SDL_BUTTON_LEFT:       up(SDL_SCANCODE_LMB);       break;
	case SDL_BUTTON_RIGHT:      up(SDL_SCANCODE_RMB);       break;
	case SDL_BUTTON_MIDDLE:     up(SDL_SCANCODE_MMB);       break;
	default:                                                break;
	}
}

void Input::mouseDoubleDown(const SDL_Event& event)
{
	switch (event.button.button)
	{
	case SDL_BUTTON_LEFT:       down(SDL_SCANCODE_LMB2);    break;
	case SDL_BUTTON_RIGHT:      down(SDL_SCANCODE_RMB2);    break;
	case SDL_BUTTON_MIDDLE:     down(SDL_SCANCODE_MMB2);    break;
	default:                                                break;
	}
}

void Input::mouseDoubleUp(const SDL_Event& event)
{
	switch (event.button.button)
	{
	case SDL_BUTTON_LEFT:       up(SDL_SCANCODE_LMB2);      break;
	case SDL_BUTTON_RIGHT:      up(SDL_SCANCODE_RMB2);      break;
	case SDL_BUTTON_MIDDLE:     up(SDL_SCANCODE_MMB2);      break;
	default:                                                break;
	}
}

void Input::mouseScroll(const SDL_Event& event)
{
	_scroll += event.wheel.y;
}

void Input::textInput(const SDL_Event& event)
{
	_input = true;
	_inputText = std::string(event.text.text);
}

void Input::textEdit(const SDL_Event& event)
{
	_edited = true;
	_editText = std::string(event.edit.text);
}

float Input::longPressThreshold() const
{
	return _threshold;
}

const std::string& Input::inputText()
{
	_input = false;
	_edited = false;
	return _inputText;
}
const std::string& Input::editText()
{
	_edited = false;
	return _editText;
}

int Input::mouseScrollDelta() const
{
	return _scroll;
}

void Input::moveMouse()
{
	_mousepixel.proximity = Proximity();
	int x, y;
	SDL_GetMouseState(&x, &y);
	_mousepixel.xenon = x;
	_mousepixel.yahoo = y;
	Camera::get()->convertTo(_mousepixel, _mousepoint);
}

void Input::layerMouse(const Proximity& proximity)
{
	_mousepixel.proximity = std::max(_mousepixel.proximity, proximity.layer());
}

const Pixel& Input::mousePixel() const
{
	return _mousepixel;
}

const Point& Input::mousePoint() const
{
	return _mousepoint;
}

float Input::significantMoveDistance() const
{
	return _significantMoveDistance;
}

float Input::tooltipThreshold() const
{
	return _tooltipThreshold;
}

#if DICTATOR_ENABLED
/* ############################ DICTATOR_ENABLED ############################ */
void Input::obey(const Dictator& dictator)
{
	if (!dictator.textInput.empty())
	{
		_input = true;
		_inputText = dictator.textInput;
	}
	if (!dictator.textEdit.empty())
	{
		_edited = true;
		_editText = dictator.textEdit;
	}
	for (int scancode : dictator.scancodesUp)
	{
		up((SDL_Scancode) scancode);
	}
	for (int scancode : dictator.scancodesDown)
	{
		down((SDL_Scancode) scancode);
	}
	for (int scancode : dictator.scancodesRepeat)
	{
		repeat((SDL_Scancode) scancode);
	}
	if (dictator.mousex)
	{
		_mousepixel.xenon = dictator.mousex;
	}
	if (dictator.mousey)
	{
		_mousepixel.yahoo = dictator.mousey;
	}
	if (dictator.mousex || dictator.mousey)
	{
		Camera::get()->convertTo(_mousepixel, _mousepoint);
	}
	if (dictator.scroll)
	{
		_scroll += dictator.scroll;
	}
}
/* ############################ DICTATOR_ENABLED ############################ */
#else
/* ########################## not DICTATOR_ENABLED ########################## */
void Input::obey(const Dictator& /**/)
{
	// Nothing to do.
}
/* ########################## not DICTATOR_ENABLED ########################## */
#endif
