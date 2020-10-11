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
 * Can Ur (canur11@gmail.com)
 */
#include "pictureview.hpp"
#include "source.hpp"

#include "collector.hpp"
#include "picture.hpp"
#include "pixel.hpp"
#include "input.hpp"
#include "loop.hpp"
#include "paint.hpp"
#include "setting.hpp"


PictureView::PictureView() :
	PictureView(ArtPanMode::NONE)
{}

PictureView::PictureView(ArtPanMode panmode) :
	_panmode(panmode)
{
	hide();
	_width = 0;
	_height = 0;

	transition(TRANSITION_ENABLED, enabled(), 0.07f);
}

void PictureView::update()
{
	Transitionator::update();
	if (!shown() || !_picture) return;

	Collector::get()->add(_picture, _topleft);
}

void PictureView::debugtree(uint8_t depth)
{
	std::string spaces(depth, '.');
	std::cout << spaces << "PictureView" << std::endl;
}

void PictureView::checkPictures()
{
	if (_picture && _picture->missing())
	{
		setPicture(_picture->nameOfPicture());
	}
}

void PictureView::checkPicture(const std::string& picturename)
{
	if (_picture && _picture->missing()
		&& _picture->nameOfPicture() == picturename)
	{
		setPicture(_picture->nameOfPicture());
	}
}

void PictureView::setPicture(const std::string& picturename)
{
	if (picturename.empty())
	{
		_picture = nullptr;
		hide();
		return;
	}

	_picture = std::make_shared<Picture>(picturename);

	_picture->setWidth(_width);
	_picture->setHeight(_height);
	show();
}

void PictureView::setShineColor(const Paint& color)
{
	if (_picture) _picture->setShineColor(color);
}

void PictureView::bear(uint8_t depth)
{
	bool wasalive = alive();
	InterfaceElement::bear(depth);
	if (wasalive != alive())
	{
		set(TRANSITION_ENABLED, enabled());
		set(TRANSITION_POWER, powered());
	}
}

void PictureView::kill(uint8_t depth)
{
	InterfaceElement::kill(depth);
}

void PictureView::enable(uint8_t depth)
{
	bool wasenabled = enabled();
	InterfaceElement::enable(depth);
	if (wasenabled != enabled())
	{
		transition(TRANSITION_ENABLED, enabled(), 0.07f);
	}
}

void PictureView::disable(uint8_t depth)
{
	bool wasenabled = enabled();
	InterfaceElement::disable(depth);
	if (wasenabled != enabled())
	{
		transition(TRANSITION_ENABLED, enabled(), 0.07f);
	}
}

void PictureView::power(uint8_t depth)
{
	bool waspowered = powered();
	InterfaceElement::power(depth);
	if (waspowered != powered())
	{
		transition(TRANSITION_POWER, powered(), 0.07f);
	}
}

void PictureView::depower(uint8_t depth)
{
	bool waspowered = powered();
	InterfaceElement::depower(depth);
	if (waspowered != powered())
	{
		transition(TRANSITION_POWER, powered(), 0.07f);
	}
}

void PictureView::shine(uint8_t depth)
{
	InterfaceElement::shine(depth);
	if (_picture)
	{
		_picture->setShine(1);
		_picture->setThetaOffset(-0.2f - Loop::theta());
	}
}

void PictureView::deshine(uint8_t depth)
{
	InterfaceElement::deshine(depth);
	if (_picture)
	{
		addAnimation(Animation(nullptr, [this](float progress) {

			_picture->setShine(1.0f - progress);

		}, 0.07f, 0));
	}
}

void PictureView::setWidth(int w)
{
	if (_fixedWidth) return;

	if (_picture)
	{
		_picture->setWidth(w);
		_width = _picture->scaledWidth();
	}
	else
	{
		_width = w;
	}
}

void PictureView::setHeight(int h)
{
	if (_fixedHeight) return;

	if (_picture)
	{
		_picture->setHeight(h);
		_height = _picture->scaledHeight();
	}
	else
	{
		_height = h;
	}
}

void PictureView::settleWidth()
{
	if (_fixedWidth) return;

	if (_picture)
	{
		_picture->setWidth(0);
		_width = _picture->scaledWidth();
	}
	else
	{
		_width = 0;
	}
}

void PictureView::settleHeight()
{
	if (_fixedHeight) return;

	if (_picture)
	{
		_picture->setHeight(0);
		_height = _picture->scaledHeight();
	}
	else
	{
		_height = 0;
	}
}

void PictureView::align(HorizontalAlignment horiz)
{
	_horizontalAlignment = horiz;
}

void PictureView::align(VerticalAlignment vert)
{
	_verticalAlignment = vert;
}

void PictureView::place(const Pixel& topleft)
{
	_topleft = topleft;
	_thickness = 1;
}

void PictureView::refresh()
{
	{
		if (hoverable() && !_hovered && hovered())
		{
			transition(TRANSITION_HOVERED, 1.0f, 0.07f);
			_hovered = true;
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
		}
		else if (_pressed && !Input::get()->isKeyHeld(SDL_SCANCODE_LMB))
		{
			transition(TRANSITION_PRESSED, 0.0f, 0.07f);
			_pressed = false;
		}
	}

	if (_picture)
	{
		int hslack = ((InterfaceElement::scale() * _picture->width())
			- _picture->scaledWidth()) / 2;
		int vslack = ((InterfaceElement::scale() * _picture->height())
			- _picture->scaledHeight()) / 2;
		switch (_panmode)
		{
			case ArtPanMode::AUTO:
			{
				if (!_panning)
				{
					_panning = true;
					int dx = 0;
					int dy = 0;
					float distance;
					if (_verticalAlignment == VerticalAlignment::TOP
						&& _horizontalAlignment == HorizontalAlignment::RIGHT)
					{
						_verticalAlignment = VerticalAlignment::BOTTOM;
						_horizontalAlignment = HorizontalAlignment::RIGHT;
						dx = hslack;
						dy = vslack;
						distance = 2 * vslack;
					}
					else if (_verticalAlignment == VerticalAlignment::BOTTOM
						&& _horizontalAlignment == HorizontalAlignment::RIGHT)
					{
						_verticalAlignment = VerticalAlignment::TOP;
						_horizontalAlignment = HorizontalAlignment::LEFT;
						dx = -hslack;
						dy = -vslack;
						distance = 2 * sqrt(hslack * hslack + vslack * vslack);
					}
					else if (_verticalAlignment == VerticalAlignment::TOP
						&& _horizontalAlignment == HorizontalAlignment::LEFT)
					{
						_verticalAlignment = VerticalAlignment::BOTTOM;
						_horizontalAlignment = HorizontalAlignment::LEFT;
						dx = -hslack;
						dy = vslack;
						distance = 2 * vslack;
					}
					else if (_verticalAlignment == VerticalAlignment::BOTTOM
						&& _horizontalAlignment == HorizontalAlignment::LEFT)
					{
						_verticalAlignment = VerticalAlignment::TOP;
						_horizontalAlignment = HorizontalAlignment::RIGHT;
						dx = hslack;
						dy = -vslack;
						distance = 2 * sqrt(hslack * hslack + vslack * vslack);
					}
					else
					{
						_verticalAlignment = VerticalAlignment::TOP;
						_horizontalAlignment = HorizontalAlignment::RIGHT;
						dx = hslack;
						dy = -vslack;
						distance = sqrt(hslack * hslack + vslack * vslack);
					}
					float time = 0.015f * distance;
					transition(TRANSITION_DISPLACE_X, dx, time);
					transition(TRANSITION_DISPLACE_Y, dy, time);
					addAnimation(Animation(nullptr, [this](float /**/) {
						_panning = false;
					}, 0, time));
				}

				int dx = _transitions[TRANSITION_DISPLACE_X];
				int dy = _transitions[TRANSITION_DISPLACE_Y];
				_picture->setOffset(dx, dy);
			}
			break;

			case ArtPanMode::CURSOR:
			{

				Pixel center;
				center.xenon = InterfaceElement::windowW() / 2;
				center.yahoo = InterfaceElement::windowH() / 2;
				Pixel target = Input::get()->mousePixel();
				int dx = target.xenon - center.xenon;
				int dy = target.yahoo - center.yahoo;
				dx = std::min(std::max(-hslack, dx), hslack);
				dy = std::min(std::max(-vslack, dy), vslack);
				_picture->setOffset(dx, dy);
			}
			break;

			case ArtPanMode::NONE:
			{
				int dx = 0;
				int dy = 0;
				switch (_horizontalAlignment)
				{
					case HorizontalAlignment::LEFT: dx = -hslack; break;
					case HorizontalAlignment::CENTER: dx = 0; break;
					case HorizontalAlignment::RIGHT: dx = hslack; break;
				}
				switch (_verticalAlignment)
				{
					case VerticalAlignment::TOP: dy = -vslack; break;
					case VerticalAlignment::MIDDLE: dy = 0; break;
					case VerticalAlignment::BOTTOM: dy = vslack; break;
				}
				_picture->setOffset(dx, dy);
			}
			break;
		}
	}

	if (alive())
	{
		update();
	}
	else
	{
		// We want transitions to finish without actually drawing any sprites.
		Animator::update();
	}
}

bool PictureView::overed() const
{
	if (!alive()) return false;

	// The view has mass so it itself can be hovered and clicked.
	Pixel mouse = Input::get()->mousePixel();
	if (   mouse.xenon >= _topleft.xenon && mouse.xenon < _topleft.xenon + _width
		&& mouse.yahoo >= _topleft.yahoo && mouse.yahoo < _topleft.yahoo + _height)
	{
		return true;
	}
	else return false;
}

bool PictureView::hovered() const
{
	if (!hasMass()) return false;

	// The view has mass so it itself can be hovered and clicked.
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
