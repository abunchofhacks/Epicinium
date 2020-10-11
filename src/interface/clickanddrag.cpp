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
#include "clickanddrag.hpp"
#include "source.hpp"

#include "input.hpp"
#include "frame.hpp"
#include "filler.hpp"
#include "paint.hpp"
#include "colorname.hpp"



ClickAndDrag::ClickAndDrag(bool movable) :
	ClickAndDrag(movable,
		std::unique_ptr<InterfaceElement>(new Frame("ui/frame_mini_9")))
{
	impart(_shadow);

	_shadow->put(new Filler());
	_shadow->setColor(0, Paint::alpha(ColorName::SHADEBLEND, 30));
	_shadow->setColor(1, Paint::alpha(ColorName::SHADEBLEND, 10));
	_shadow->setColor(2, Paint::alpha(ColorName::SHADEBLEND, 5));
	_shadow->setColor(3, Paint::alpha(ColorName::SHADEBLEND, 5));
}

ClickAndDrag::ClickAndDrag(bool movable,
		std::unique_ptr<InterfaceElement> shadow) :
	_shadow(std::move(shadow)),
	_movable(movable),
	_opened(false),
	_dragging(false),
	_moving(false)
{}

std::unique_ptr<InterfaceElement> ClickAndDrag::put(
	std::unique_ptr<InterfaceElement> content)
{
	impart(content);
	std::swap(_content, content);

	_width = _content->width();
	_height = _content->height();
	_marginLeft = std::max(_marginLeft, _content->marginLeft());
	_marginRight = std::max(_marginRight, _content->marginRight());
	_marginTop = std::max(_marginTop, _content->marginTop());
	_marginBottom = std::max(_marginBottom, _content->marginBottom());

	return content;
}

InterfaceElement& ClickAndDrag::content()
{
	if (!_content)
	{
		LOGW << "No content set";
		DEBUG_ASSERT(false);
		return *garbage();
	}

	return *_content;
}

void ClickAndDrag::debugtree(uint8_t depth)
{
	std::string spaces(depth, '.');
	std::cout << spaces << "Shadow:" << std::endl;
	if (_shadow) _shadow->debugtree(depth + 1);
	std::cout << spaces << "Content:" << std::endl;
	if (_content) _content->debugtree(depth + 1);
}

void ClickAndDrag::bear(uint8_t depth)
{
	InterfaceElement::bear(depth);

	_shadow->bear(depth + 1);
	if (_content) _content->bear(depth + 1);
}

void ClickAndDrag::kill(uint8_t depth)
{
	InterfaceElement::kill(depth);

	_shadow->kill(depth + 1);
	if (_content) _content->kill(depth + 1);
}

void ClickAndDrag::show(uint8_t depth)
{
	InterfaceElement::show(depth);

	_shadow->show(depth + 1);
	if (_content) _content->show(depth + 1);
}

void ClickAndDrag::hide(uint8_t depth)
{
	InterfaceElement::hide(depth);

	_shadow->hide(depth + 1);
	if (_content) _content->hide(depth + 1);
}

void ClickAndDrag::enable(uint8_t depth)
{
	InterfaceElement::enable(depth);

	_shadow->enable(depth + 1);
	if (_content) _content->enable(depth + 1);
}

void ClickAndDrag::disable(uint8_t depth)
{
	InterfaceElement::disable(depth);

	_shadow->disable(depth + 1);
	if (_content) _content->disable(depth + 1);
}

void ClickAndDrag::power(uint8_t depth)
{
	InterfaceElement::power(depth);

	_shadow->power(depth + 1);
	if (_content) _content->power(depth + 1);
}

void ClickAndDrag::depower(uint8_t depth)
{
	InterfaceElement::depower(depth);

	_shadow->depower(depth + 1);
	if (_content) _content->depower(depth + 1);
}

void ClickAndDrag::shine(uint8_t depth)
{
	InterfaceElement::shine(depth);

	_shadow->shine(depth + 1);
	if (_content) _content->shine(depth + 1);
}

void ClickAndDrag::deshine(uint8_t depth)
{
	InterfaceElement::deshine(depth);

	_shadow->deshine(depth + 1);
	if (_content) _content->deshine(depth + 1);
}

void ClickAndDrag::setWidth(int w)
{
	if (_fixedWidth) return;

	if (!_content)
	{
		LOGW << "No content set.";
		DEBUG_ASSERT(false);
		return;
	}

	_content->setWidth(w);
	_shadow->fixWidth(_content->width());
	_width = _content->width();
}

void ClickAndDrag::setHeight(int h)
{
	if (_fixedHeight) return;

	if (!_content)
	{
		LOGW << "No content set.";
		DEBUG_ASSERT(false);
		return;
	}

	_content->setHeight(h);
	_shadow->fixHeight(_content->height());
	_height = _content->height();
}

void ClickAndDrag::settleWidth()
{
	if (_fixedWidth) return;

	if (!_content)
	{
		LOGW << "No content set.";
		DEBUG_ASSERT(false);
		return;
	}

	_content->settleWidth();
	_shadow->fixWidth(_content->width());
	_width = _content->width();
}

void ClickAndDrag::settleHeight()
{
	if (_fixedHeight) return;

	if (!_content)
	{
		LOGW << "No content set.";
		DEBUG_ASSERT(false);
		return;
	}

	_content->settleHeight();
	_shadow->fixHeight(_content->height());
	_height = _content->height();
}

void ClickAndDrag::setMarginTop(int margin)
{
	if (!_content)
	{
		LOGW << "No content set";
		DEBUG_ASSERT(false);
		return;
	}

	_content->setMarginTop(margin);
	_marginTop = _content->marginTop();
}

void ClickAndDrag::setMarginLeft(int margin)
{
	if (!_content)
	{
		LOGW << "No content set";
		DEBUG_ASSERT(false);
		return;
	}

	_content->setMarginLeft(margin);
	_marginLeft = _content->marginLeft();
}

void ClickAndDrag::setMarginRight(int margin)
{
	if (!_content)
	{
		LOGW << "No content set";
		DEBUG_ASSERT(false);
		return;
	}

	_content->setMarginRight(margin);
	_marginRight = _content->marginRight();
}

void ClickAndDrag::setMarginBottom(int margin)
{
	if (!_content)
	{
		LOGW << "No content set";
		DEBUG_ASSERT(false);
		return;
	}

	_content->setMarginBottom(margin);
	_marginBottom = _content->marginBottom();
}

// The following few functions pass the call to the content to make the Padding
// class transparent.
size_t ClickAndDrag::size() const
{
	if (!_content)
	{
		LOGW << "No content set.";
		DEBUG_ASSERT(false);
		return 0;
	}

	return _content->size();
}

std::string ClickAndDrag::name(size_t offset)
{
	if (!_content)
	{
		LOGW << "No content set.";
		DEBUG_ASSERT(false);
		return "";
	}

	return _content->name(offset);
}

bool ClickAndDrag::contains(stringref name)
{
	if (!_content)
	{
		LOGW << "No content set.";
		DEBUG_ASSERT(false);
		return "";
	}

	return _content->contains(name);
}

std::string ClickAndDrag::text() const
{
	if (!_content)
	{
		LOGW << "No content set.";
		DEBUG_ASSERT(false);
		return "";
	}

	return _content->text();
}

void ClickAndDrag::add(stringref name, std::unique_ptr<InterfaceElement> element)
{
	if (!_content)
	{
		LOGW << "No content set.";
		DEBUG_ASSERT(false);
		return;
	}

	_content->add(name, std::move(element));
}

std::unique_ptr<InterfaceElement> ClickAndDrag::replace(stringref name,
	std::unique_ptr<InterfaceElement> element,
	stringref newname)
{
	if (!_content)
	{
		LOGW << "No content set.";
		DEBUG_ASSERT(false);
		return makeGarbage();
	}

	return _content->replace(name, std::move(element), newname);
}

InterfaceElement* ClickAndDrag::get(stringref name)
{
	if (!_content)
	{
		LOGW << "No content set.";
		DEBUG_ASSERT(false);
		return garbage();
	}

	return _content->get(name);
}

std::unique_ptr<InterfaceElement> ClickAndDrag::remove(stringref name)
{
	if (!_content)
	{
		LOGW << "No content set.";
		DEBUG_ASSERT(false);
		return makeGarbage();
	}

	return _content->remove(name);
}

void ClickAndDrag::reset()
{
	if (!_content)
	{
		LOGW << "No content set.";
		DEBUG_ASSERT(false);
		return;
	}

	_content->reset();
}

bool ClickAndDrag::resizableWidth()
{
	if (_fixedWidth) return false;

	if (!_content)
	{
		LOGW << "No content set.";
		DEBUG_ASSERT(false);
		return false;
	}

	return _content->resizableWidth();
}

bool ClickAndDrag::resizableHeight()
{
	if (_fixedHeight) return false;

	if (!_content)
	{
		LOGW << "No content set.";
		DEBUG_ASSERT(false);
		return false;
	}

	return _content->resizableHeight();
}

void ClickAndDrag::align(HorizontalAlignment horiz)
{
	if (!_content)
	{
		LOGW << "No content set.";
		DEBUG_ASSERT(false);
		return;
	}

	return _content->align(horiz);
}
void ClickAndDrag::align(VerticalAlignment vert)
{
	if (!_content)
	{
		LOGW << "No content set.";
		DEBUG_ASSERT(false);
		return;
	}

	return _content->align(vert);
}

void ClickAndDrag::setTag(const std::string& tag, bool randomize)
{
	if (!_content)
	{
		LOGW << "No content set.";
		DEBUG_ASSERT(false);
		return;
	}

	return  _content->setTag(tag, randomize);
}

void ClickAndDrag::setTagActive(const std::string& tag, bool restart)
{
	if (!_content)
	{
		LOGW << "No content set.";
		DEBUG_ASSERT(false);
		return;
	}

	return  _content->setTagActive(tag, restart);
}

std::string ClickAndDrag::getTag()
{
	if (!_content)
	{
		LOGW << "No content set.";
		DEBUG_ASSERT(false);
		return "";
	}

	return  _content->getTag();
}

bool ClickAndDrag::overed() const
{
	if (_movable && (_opened || _moving))
	{
		return _shadow->overed();
	}

	if (_content && _content->overed()) return true;

	return false;
}

bool ClickAndDrag::hovered() const
{
	// Because some items might occlude others, we must know which layer the mouse operates in.
	// We therefore need to check all items to see if they are hovered.
	bool result = false;

	if (_shadow->hovered()) result = true;

	if (_content && _content->hovered()) result = true;

	// While dragging, we fake the existence of a hovered overlay.
	Pixel mouse = Input::get()->mousePixel();
	if (_dragging && mouse.proximity.layer() <= Layer::TOOLTIP)
	{
		Input::get()->layerMouse(Layer::TOOLTIP);
		result = true;
	}

	return result;
}

bool ClickAndDrag::clicked() const
{
	// Because some items might occlude others, we must know which layer the mouse operates in.
	// We therefore need to check all items to see if they are hovered.
	bool result = false;

	if (_shadow->clicked()) result = true;

	if (_content && _content->clicked()) result = true;

	// While dragging, we fake the existence of a hovered overlay.
	if (_dragging && InterfaceElement::clicked()) result = true;

	return result;
}

bool ClickAndDrag::held() const
{
	// Because some items might occlude others, we must know which layer the mouse operates in.
	// We therefore need to check all items to see if they are hovered.
	bool result = false;

	// We only consider it held if we are in the middle of dragging.
	// Therefore we ignore the shadow and the content.
	_shadow->held();
	if (_content) _content->held();

	// While dragging, we fake the existence of a hovered overlay.
	if (_dragging && InterfaceElement::held()) result = true;

	return result;
}

bool ClickAndDrag::released() const
{
	// Because some items might occlude others, we must know which layer the mouse operates in.
	// We therefore need to check all items to see if they are hovered.
	bool result = false;

	// We only consider it released if we were in the middle of dragging.
	// Therefore we ignore the shadow and the content.
	_shadow->released();
	if (_content) _content->released();

	// While dragging, we fake the existence of a hovered overlay.
	if (_dragging && InterfaceElement::released()) result = true;

	return result;
}

void ClickAndDrag::refresh()
{
	// The overlay becomes active when the content is clicked and disappears
	// when the left mouse button is released for more than one frame.
	if (_movable && _moving)
	{
		if (_content && _content->topleft().distanceTo(_contentTopleft) <= 0.5f)
		{
			_shadow->kill();
			_content->place(_contentTopleft);
			_moving = false;
		}
	}
	else if (!_dragging)
	{
		if (_content && _content->clicked())
		{
			if (_movable)
			{
				_shadow->bear();
				_shadow->fixWidth(_content->width());
				_shadow->fixHeight(_content->height());
			}
			_from = Input::get()->mousePixel();
			_from.xenon -= _contentTopleft.xenon;
			_from.yahoo -= _contentTopleft.yahoo;
			_opened = true;
			_dragging = true;
		}
	}
	else if (!_opened)
	{
		_dragging = false;
		if (_content)
		{
			if (_movable)
			{
				startMovingFrom(_content->topleft() ^ Layer::TOOLTIP);
			}
			else
			{
				_shadow->kill();
			}
		}
	}

	if (_dragging && Input::get()->wasKeyReleased(SDL_SCANCODE_LMB))
	{
		_opened = false;
	}

	if (_movable && _opened && _content)
	{
		Pixel pixel = Input::get()->mousePixel();
		pixel.xenon -= _from.xenon;
		pixel.yahoo -= _from.yahoo;
		_content->place(pixel ^ Layer::TOOLTIP);
	}

	Animator::update();

	_shadow->refresh();

	if (_content) _content->refresh();
}

void ClickAndDrag::startMovingFrom(const Pixel& start)
{
	if (start.distanceTo(_contentTopleft) <= 0.5f)
	{
		_shadow->kill();
		_content->place(_contentTopleft);
		return;
	}

	_shadow->bear();
	_shadow->fixWidth(_content->width());
	_shadow->fixHeight(_content->height());

	float duration = 0.05f + 0.75f * std::min(1.0f,
		start.distanceTo(_contentTopleft) / std::min(windowW(), windowH()));
	addAnimation(Animation(nullptr, [this, start](float progress) {

		if (progress > 0.999f)
		{
			_content->place(_contentTopleft);
			return;
		}

		/*
		*   Speed (v):         Travel (x):
		*   .   XX             .     xXX
		*   .  X  X            .    XXXX
		*   . X    X           .   XXXXX
		*   .X      X          .xxXXXXXX
		*   --------- (time)   --------- (time)
		*
		*   x(0) = 0, x(0.5) = 0.5, x(1) = 1
		*   x(0.25) = integral from 0 to 0.25 of v(t) dt
		*   v(0) = 0, v(0.5) = vmax, v(1) = 0
		*   v(0.25) = [vmax * t / 0.5](t => 0.25)
		*           = [2.0 * vmax * t](t => 0.25)
		*   x(0.25) = [vmax * t * t](t => 0.25) - [vmax * 0 * 0]
		*           = [vmax * t * t](t => 0.25)
		*   x(0.75) = 0.5 + integral from 0.5 to 0.75 of v(t) dt
		*   v(0.75) = [vmax - 2.0 * vmax * (t - 0.5)](t => 0.75)
		*           = [2.0 * vmax - 2.0 * vmax * t](t => 0.75)
		*   x(0.75) = 0.5 + 2.0 * vmax
		*             * integral from 0.5 to 0.75 of (1 - t) dt
		*           = 0.5 + 2.0 * vmax
		*             * ([t - 0.5 * t * t](t => 0.75) - [0.5 - 0.125])
		*           = 0.5 - 0.75 * vmax + [2.0 * vmax * t](t => 0.75)
		*             - [vmax * t * t](t => 0.75)
		*   0.5 = x(0.5) = vmax * 0.5 * 0.5  ==>  vmax = 2.0
		*/
		static constexpr float vmax = 2.0f;
		float travel;
		if (progress <= 0.5f)
		{
			travel = vmax * progress * progress;
		}
		else
		{
			travel = 0.5f - 0.75f * vmax + 2.0f * vmax * progress
				- vmax * progress * progress;
		}
		Pixel pixel = start;
		pixel.xenon += travel * (_contentTopleft.xenon - start.xenon);
		pixel.yahoo += travel * (_contentTopleft.yahoo - start.yahoo);
		_content->place(pixel);

	}, duration, 0));
	_moving = true;
}

void ClickAndDrag::place(const Pixel& topleft)
{
	_topleft = topleft;
	_thickness = 0;

	_shadow->place(_topleft);
	_thickness += _shadow->thickness();

	_contentTopleft = _topleft;
	_contentTopleft.proximity += _shadow->thickness();

	if (!_content) return;

	if (_movable && !_opened && !_moving
		&& _content->topleft().proximity.layer() != Layer::BACKGROUND)
	{
		startMovingFrom(_content->topleft() ^ _contentTopleft.proximity);
	}
	else
	{
		_content->place(_contentTopleft);
	}
	_thickness += _content->thickness();
}

void ClickAndDrag::checkPictures()
{
	if (_content) _content->checkPictures();
}

void ClickAndDrag::checkPicture(const std::string& picturename)
{
	if (_content) _content->checkPicture(picturename);
}
