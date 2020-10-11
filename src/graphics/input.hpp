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
#pragma once
#include "header.hpp"

#include <array>
#include <bitset>

#include "libs/SDL2/SDL_scancode.h"

union SDL_Event;

#include "pixel.hpp"
#include "point.hpp"

class Dictator;


// ENTER is a synonym for RETURN
const SDL_Scancode SDL_SCANCODE_ENTER = SDL_SCANCODE_RETURN;

// CTRL is a macro for "LCTRL || RCTRL".
const SDL_Scancode SDL_SCANCODE_CTRL  = (SDL_Scancode) 320;
const SDL_Scancode SDL_SCANCODE_SHIFT = (SDL_Scancode) 321;
const SDL_Scancode SDL_SCANCODE_ALT   = (SDL_Scancode) 322;

// We treat mouse buttons the same as keys.
const SDL_Scancode SDL_SCANCODE_LMB  = (SDL_Scancode) 384;
const SDL_Scancode SDL_SCANCODE_RMB  = (SDL_Scancode) 385;
const SDL_Scancode SDL_SCANCODE_MMB  = (SDL_Scancode) 386;
const SDL_Scancode SDL_SCANCODE_LMB2 = (SDL_Scancode) 387;
const SDL_Scancode SDL_SCANCODE_RMB2 = (SDL_Scancode) 388;
const SDL_Scancode SDL_SCANCODE_MMB2 = (SDL_Scancode) 389;


class Input
{
private:
	static Input* _installed;

	void down(const SDL_Scancode key);
	void up(const SDL_Scancode key);
	void repeat(const SDL_Scancode key);
	void keyDown(const SDL_Event& event);
	void keyUp(const SDL_Event& event);
	void keyRepeat(const SDL_Event& event);
	void mouseDown(const SDL_Event& event);
	void mouseUp(const SDL_Event& event);
	void mouseDoubleDown(const SDL_Event& event);
	void mouseDoubleUp(const SDL_Event& event);
	void mouseScroll(const SDL_Event& event);
	void textInput(const SDL_Event& event);
	void textEdit(const SDL_Event& event);

	std::bitset<512> _held;
	std::bitset<512> _pressed;
	std::bitset<512> _released;
	std::bitset<512> _repeated;
	std::bitset<512> _longheld;
	std::bitset<512> _longpressed;
	std::bitset<512> _longreleased;
	std::bitset<512> _shortreleased;
	std::bitset<512> _doublepressed;
	std::array<float, 512> _time;

	bool _input = false;
	bool _edited = false;
	std::string _inputText;
	std::string _editText;

	int _scroll;
	Pixel _mousepixel;
	Point _mousepoint;

public:
	static Input* get() { return _installed; }

	Input();

	Input(const Input&) = delete;
	Input(Input&&) = delete;
	Input& operator=(const Input&) = delete;
	Input& operator=(Input&&) = delete;
	~Input() = default;

	void install();

	void reset();
	void handle(const SDL_Event& event);

	bool isKeyHeld(const SDL_Scancode key) const { return _held[key]; }
	bool wasKeyPressed(const SDL_Scancode key) const { return _pressed[key]; }
	bool wasKeyReleased(const SDL_Scancode key) const { return _released[key]; }
	bool wasKeyRepeated(const SDL_Scancode key) const { return _repeated[key]; }

	bool isKeyLongHeld(const SDL_Scancode key) const { return _longheld[key]; }
	bool wasKeyLongPressed(const SDL_Scancode key) const { return _longpressed[key]; }
	bool wasKeyLongReleased(const SDL_Scancode key) const { return _longreleased[key]; }
	bool wasKeyShortReleased(const SDL_Scancode key) const { return _shortreleased[key]; }
	bool wasKeyDoublePressed(const SDL_Scancode key) const { return _doublepressed[key]; }

	float longPressThreshold() const;

	bool wasTextInput() const { return _input; }
	bool wasTextEdited() const { return _edited; }
	const std::string& inputText();
	const std::string& editText();

	int mouseScrollDelta() const;

	void moveMouse();
	void layerMouse(const Proximity& proximity);
	const Pixel& mousePixel() const;
	const Point& mousePoint() const;

	float significantMoveDistance() const;
	float tooltipThreshold() const;

	bool isDebugKeyHeld() const
	{
		return isKeyLongHeld(SDL_SCANCODE_DELETE);
	}

	void obey(const Dictator& dictator);
};
