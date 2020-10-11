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
#include "image.hpp"
#include "source.hpp"

#include "collector.hpp"
#include "sprite.hpp"
#include "input.hpp"
#include "paint.hpp"
#include "colorname.hpp"
#include "loop.hpp"
#include "skin.hpp"
#include "mixer.hpp"


Image::Image(const char* sname) :
	_sprite(new Sprite(sname))
{
	_sprite->trimMargins();

	_width = _sprite->scaledWidth();
	_height = _sprite->scaledHeight();

	transition(TRANSITION_ENABLED, enabled(), 0.07f);
}

Image::Image(const Skin& skin) :
	_sprite(new Sprite(skin.spritename.c_str()))
{
	// TODO skin.trimmed
	if (false)
	{
		_sprite->trimMargins();
	}

	_width = _sprite->scaledWidth();
	_height = _sprite->scaledHeight();

	transition(TRANSITION_ENABLED, enabled(), 0.07f);

	const Paint* basicpalette = skin.palette("basic");
	if (basicpalette != nullptr)
	{
		for (int i = 0; i < skin.palettesize; i++)
		{
			setColor(i, basicpalette[i]);
		}
	}

	const Paint* disabledpalette = skin.palette("disabled");
	if (disabledpalette != nullptr)
	{
		for (int i = 0; i < skin.palettesize; i++)
		{
			setDisabledColor(i, disabledpalette[i]);
		}
	}

	const Paint* hoveredpalette = skin.palette("hovered");
	if (hoveredpalette != nullptr)
	{
		for (int i = 0; i < skin.palettesize; i++)
		{
			setHoveredColor(i, hoveredpalette[i]);
		}
	}

	const Paint* pressedpalette = skin.palette("pressed");
	if (pressedpalette != nullptr)
	{
		for (int i = 0; i < skin.palettesize; i++)
		{
			setPressedColor(i, pressedpalette[i]);
		}
	}

	const Paint* powerpalette = skin.palette("power");
	if (powerpalette != nullptr)
	{
		for (int i = 0; i < skin.palettesize; i++)
		{
			setPowerColor(i, powerpalette[i]);
		}
	}
}

void Image::update()
{
	Transitionator::update();
	mix();
	_sprite->setObscured(_transitions[TRANSITION_OBSCURED]);
	_sprite->update();
	drawSprite();
}

void Image::mix()
{
	for (size_t i = 0; i < _palettesize; i++)
	{
		Color mixed = _basicPalette[i];

		float power = _transitions[TRANSITION_POWER];
		if (power > 0.001f)
		{
			mixed = Color::mix(mixed, _powerPalette[i], power);
		}

		float hover = _transitions[TRANSITION_HOVERED];
		float press = _transitions[TRANSITION_PRESSED];
		float disab = 1.0f - _transitions[TRANSITION_ENABLED];
		hover = hover - press - disab;
		press = press - disab;
		if (hover > 0.001f)
		{
			mixed = Color::blend(mixed, _hoveredPalette[i], hover);
		}
		if (press > 0.001f)
		{
			mixed = Color::blend(mixed, _pressedPalette[i], press);
		}
		if (disab > 0.001f)
		{
			mixed = Color::blend(mixed, _disabledPalette[i], disab);
		}

		_sprite->setColor(i, mixed);
	}
}

void Image::drawSprite()
{
	if (!shown()) return;

	Collector::get()->addImage(_sprite, _topleft);
}

void Image::debugtree(uint8_t depth)
{
	std::string spaces(depth, '.');
	std::cout << spaces << "Image" << std::endl;
}

void Image::checkColorsSize(size_t index)
{
	if (index >= _palettesize && index >= _basicPalette.size())
	{
		_palettesize = index + 1;
		_basicPalette.resize(_palettesize);
		_disabledPalette.resize(_palettesize, ColorName::DISABLEDBLEND);
		_hoveredPalette.resize(_palettesize, ColorName::HOVEREDBLEND);
		_pressedPalette.resize(_palettesize, ColorName::PRESSEDBLEND);
		_powerPalette.resize(_palettesize);
		_palettesize = std::min((size_t) _sprite->paletteSize(), _palettesize);
	}
}

void Image::setColor(size_t index, const Paint& color)
{
	checkColorsSize(index);
	_basicPalette[index] = color;
	_disabledPalette[index] = ColorName::DISABLEDBLEND;
	_hoveredPalette[index] = ColorName::HOVEREDBLEND;
	_pressedPalette[index] = ColorName::PRESSEDBLEND;
	_powerPalette[index] = color;
}

void Image::setBasicColor(size_t index, const Paint& color)
{
	checkColorsSize(index);
	_basicPalette[index] = color;
}

void Image::setDisabledColor(size_t index, const Paint& color)
{
	checkColorsSize(index);
	_disabledPalette[index] = color;
}

void Image::setHoveredColor(size_t index, const Paint& color)
{
	checkColorsSize(index);
	_hoveredPalette[index] = color;
}

void Image::setPressedColor(size_t index, const Paint& color)
{
	checkColorsSize(index);
	_pressedPalette[index] = color;
}

void Image::setPowerColor(size_t index, const Paint& color)
{
	checkColorsSize(index);
	_powerPalette[index] = color;
}

void Image::setShineColor(const Paint& color)
{
	_sprite->setShineColor(color);
}

void Image::bear(uint8_t depth)
{
	bool wasalive = alive();
	InterfaceElement::bear(depth);
	if (wasalive != alive())
	{
		transition(TRANSITION_ENABLED, enabled(), 0.07f);
		transition(TRANSITION_POWER, powered(), 0.07f);
	}
}

void Image::kill(uint8_t depth)
{
	InterfaceElement::kill(depth);
}

void Image::enable(uint8_t depth)
{
	bool wasenabled = enabled();
	InterfaceElement::enable(depth);
	if (wasenabled != enabled())
	{
		transition(TRANSITION_ENABLED, enabled(), 0.07f);
	}
}

void Image::disable(uint8_t depth)
{
	bool wasenabled = enabled();
	InterfaceElement::disable(depth);
	if (wasenabled != enabled())
	{
		transition(TRANSITION_ENABLED, enabled(), 0.07f);
	}
}

void Image::power(uint8_t depth)
{
	bool waspowered = powered();
	InterfaceElement::power(depth);
	if (waspowered != powered())
	{
		transition(TRANSITION_POWER, powered(), 0.07f);
	}
}

void Image::depower(uint8_t depth)
{
	bool waspowered = powered();
	InterfaceElement::depower(depth);
	if (waspowered != powered())
	{
		transition(TRANSITION_POWER, powered(), 0.07f);
	}
}

void Image::shine(uint8_t depth)
{
	InterfaceElement::shine(depth);
	_sprite->setShine(1);
	_sprite->setThetaOffset(-0.2f - Loop::theta());
}

void Image::deshine(uint8_t depth)
{
	InterfaceElement::deshine(depth);
	addAnimation(Animation(nullptr, [this](float progress) {

		_sprite->setShine(1.0f - progress);

	}, 0.07f, 0));
}

void Image::setWidth(int)
{
	LOGW << "Trying to set width of an image.";
	DEBUG_ASSERT(false);
}

void Image::setHeight(int)
{
	LOGW << "Trying to set height of an image.";
	DEBUG_ASSERT(false);
}

void Image::place(const Pixel& topleft)
{
	_topleft = topleft;
	_thickness = 1;
}

void Image::refresh()
{
	{
		if (hoverable() && !_hovered && hovered())
		{
			transition(TRANSITION_HOVERED, 1.0f, 0.07f);
			_hovered = true;
			Mixer::get()->queue(Clip::Type::UI_HOVER, 0.0f);
		}
		else if (_hovered && !hovered())
		{
			transition(TRANSITION_HOVERED, 0.0f, 0.07f);
			_hovered = false;
		}

		if (clickable() && !_pressed && clicked())
		{
			transition(TRANSITION_PRESSED, 1.0f, 0.07f);
			_pressed = true;
			Mixer::get()->queue(Clip::Type::UI_CLICK, 0.0f);
		}
		else if (_pressed && !Input::get()->isKeyHeld(SDL_SCANCODE_LMB))
		{
			transition(TRANSITION_PRESSED, 0.0f, 0.07f);
			_pressed = false;
		}
	}

	if (alive())
	{
		update();
	}
	else
	{
		// We want transitions to finish without actually drawing any sprites.
		Transitionator::update();
	}
}

bool Image::overed() const
{
	if (!alive()) return false;

	// The image has mass so it itself can be hovered and clicked.
	Pixel mouse = Input::get()->mousePixel();
	if (_sprite->onMask(_topleft, mouse))
	{
		return true;
	}
	else return false;
}

bool Image::hovered() const
{
	if (!hasMass()) return false;

	// The image has mass so it itself can be hovered and clicked.
	Pixel mouse = Input::get()->mousePixel();
	if (mouse.proximity.layer() > _topleft.proximity.layer()) return false;
	if (_sprite->onMask(_topleft, mouse))
	{
		Input::get()->layerMouse(_topleft.proximity.layer());
		return true;
	}
	else return false;
}

void Image::setTag(const std::string& tag, bool randomizeStart)
{
	_sprite->setTag(tag);

	if (randomizeStart)
	{
		_sprite->randomizeAnimationStart();
	}
}

void Image::setTagActive(const std::string& tag, bool restart)
{
	_sprite->setTagActive(tag, restart);
}

std::string Image::getTag()
{
	return _sprite->getTag();
}

Image* Image::upscale(int scale, Image* raw)
{
	raw->_sprite->setUpscale(scale);
	raw->_width = raw->_sprite->scaledWidth();
	raw->_height = raw->_sprite->scaledHeight();
	return raw;
}
