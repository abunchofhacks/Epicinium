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
#include "interfaceelement.hpp"
#include "source.hpp"

#include "camera.hpp"
#include "graphics.hpp"
#include "input.hpp"
#include "text.hpp"


int InterfaceElement::scale()
{
	return Camera::get()->SCALE;
}

int InterfaceElement::windowW()
{
	return Camera::get()->WINDOW_W;
}

int InterfaceElement::windowH()
{
	return Camera::get()->WINDOW_H;
}

int InterfaceElement::fontH(int fontsize)
{
	int h = Text::fontH(fontsize);
	// Round up to the nearest multiple of s.
	int s = scale();
	return h + ((s - (h % s)) % s);
}

int InterfaceElement::fontH(const TextStyle& style)
{
	int h = Text::fontH(style);
	// Round up to the nearest multiple of s.
	int s = scale();
	return h + ((s - (h % s)) % s);
}

int InterfaceElement::textW(const TextStyle& style, const std::string& text)
{
	int w = Text::calculateWidth(style, text);
	// TODO round up to the nearest multiple of s?
	return w;
}

bool InterfaceElement::inspect()
{
	return Graphics::get()->debugUI();
}

void InterfaceElement::impart(const std::unique_ptr<InterfaceElement>& child)
{
	for (uint8_t depth = 0; depth + 1 < MAXDEPTH; depth++)
	{
		if (_killed[depth])
		{
			if (!child->_killed[depth + 1]) child->kill(depth + 1);
		}
		else if (child->_killed[depth + 1]) child->bear(depth + 1);

		if (_hidden[depth])
		{
			if (!child->_hidden[depth + 1]) child->hide(depth + 1);
		}
		else if (child->_hidden[depth + 1]) child->show(depth + 1);

		if (_disabled[depth])
		{
			if (!child->_disabled[depth + 1]) child->disable(depth + 1);
		}
		else if (child->_disabled[depth + 1]) child->enable(depth + 1);
	}
}

void InterfaceElement::bearIf(bool condition)
{
	if (condition)
	{
		if (_killed[0]) bear();
	}
	else
	{
		if (!_killed[0]) kill();
	}
}

void InterfaceElement::showIf(bool condition)
{
	if (condition)
	{
		if (_hidden[0]) show();
	}
	else
	{
		if (!_hidden[0]) hide();
	}
}

void InterfaceElement::enableIf(bool condition)
{
	if (condition)
	{
		if (_disabled[0]) enable();
	}
	else
	{
		if (!_disabled[0]) disable();
	}
}

void InterfaceElement::powerIf(bool condition)
{
	if (condition)
	{
		if (!_powered) power();
	}
	else
	{
		if (_powered) depower();
	}
}

void InterfaceElement::fixWidth()
{
	fixWidth(width());
}

void InterfaceElement::fixWidth(int w)
{
	_fixedWidth = 0;
	setWidth(w);
	_fixedWidth = width();
	// We can't set _fixedWidth to 0 because "if (_fixedWidth)" should return true.
	if (w == 0) _fixedWidth = -1;
}

void InterfaceElement::fixHeight()
{
	fixHeight(height());
}

void InterfaceElement::fixHeight(int h)
{
	_fixedHeight = 0;
	setHeight(h);
	_fixedHeight = height();
	// We can't set _fixedHeight to 0 because "if (_fixedHeight)" should return true.
	if (h == 0) _fixedHeight = -1;
}

void InterfaceElement::unfixWidth()
{
	_fixedWidth = 0;
}

void InterfaceElement::unfixHeight()
{
	_fixedHeight = 0;
}

void InterfaceElement::settleWidth()
{
	if (resizableWidth() && !_fixedWidth) setWidth(0);
}

void InterfaceElement::settleHeight()
{
	if (resizableHeight() && !_fixedHeight) setHeight(0);
}

void InterfaceElement::settle()
{
	settleWidth();
	settleHeight();
	place(topleft());
}

void InterfaceElement::setMarginTop(int margin)
{
	_marginTop = margin;
}
void InterfaceElement::setMarginLeft(int margin)
{
	_marginLeft = margin;
}
void InterfaceElement::setMarginRight(int margin)
{
	_marginRight = margin;
}
void InterfaceElement::setMarginBottom(int margin)
{
	_marginBottom = margin;
}

void InterfaceElement::setPaddingTop(int)
{
	LOGW << "Non-overridden InterfaceElement::setPaddingTop() call";
	DEBUG_ASSERT(false);
}
void InterfaceElement::setPaddingLeft(int)
{
	LOGW << "Non-overridden InterfaceElement::setPaddingLeft() call";
	DEBUG_ASSERT(false);
}
void InterfaceElement::setPaddingRight(int)
{
	LOGW << "Non-overridden InterfaceElement::setPaddingRight() call";
	DEBUG_ASSERT(false);
}
void InterfaceElement::setPaddingBottom(int)
{
	LOGW << "Non-overridden InterfaceElement::setPaddingBottom() call";
	DEBUG_ASSERT(false);
}

void InterfaceElement::setMarginHorizontal(int margin)
{
	setMarginLeft(margin);
	setMarginRight(margin);
}
void InterfaceElement::setMarginVertical(int margin)
{
	setMarginTop(margin);
	setMarginBottom(margin);
}
void InterfaceElement::setMargin(int margin)
{
	setMarginTop(margin);
	setMarginLeft(margin);
	setMarginRight(margin);
	setMarginBottom(margin);
}

void InterfaceElement::setPaddingHorizontal(int padding)
{
	setPaddingLeft(padding);
	setPaddingRight(padding);
}
void InterfaceElement::setPaddingVertical(int padding)
{
	setPaddingTop(padding);
	setPaddingBottom(padding);
}
void InterfaceElement::setPadding(int padding)
{
	setPaddingTop(padding);
	setPaddingLeft(padding);
	setPaddingRight(padding);
	setPaddingBottom(padding);
}

void InterfaceElement::makeMassless()
{
	_hasMass = false;
	_reactsToHover = false;
}

void InterfaceElement::makeHoverable()
{
	_hasMass = true;
	_reactsToHover = true;
}

void InterfaceElement::makeClickable()
{
	_hasMass = true;
	_reactsToHover = true;
	_reactsToClick = true;
}

void InterfaceElement::reset()
{
	LOGW << "Non-overridden InterfaceElement::reset() call";
	DEBUG_ASSERT(false);
}

std::string InterfaceElement::text() const
{
	LOGW << "Non-overridden InterfaceElement::text() call";
	DEBUG_ASSERT(false);
	return "";
}

void InterfaceElement::setText(const std::string&)
{
	LOGW << "Non-overridden InterfaceElement::setText() call";
	DEBUG_ASSERT(false);
}

void InterfaceElement::prefillText(const std::string&)
{
	LOGW << "Non-overridden InterfaceElement::prefillText() call";
	DEBUG_ASSERT(false);
}

void InterfaceElement::addText(const std::string&)
{
	LOGW << "Non-overridden InterfaceElement::addText() call";
	DEBUG_ASSERT(false);
}

void InterfaceElement::addText(const std::string&, float, float)
{
	LOGW << "Non-overridden InterfaceElement::addText() call";
	DEBUG_ASSERT(false);
}

void InterfaceElement::addText(const std::string&, int, const Paint&)
{
	LOGW << "Non-overridden InterfaceElement::addText() call";
	DEBUG_ASSERT(false);
}

void InterfaceElement::addText(const std::string&, int, const Paint&, float,
	float)
{
	LOGW << "Non-overridden InterfaceElement::addText() call";
	DEBUG_ASSERT(false);
}

void InterfaceElement::addIcon(const char*)
{
	LOGW << "Non-overridden InterfaceElement::addIcon() call";
	DEBUG_ASSERT(false);
}

void InterfaceElement::addIcon(const char*, const Paint&)
{
	LOGW << "Non-overridden InterfaceElement::addIcon() call";
	DEBUG_ASSERT(false);
}

void InterfaceElement::setTextColor(const Paint&)
{
	LOGW << "Non-overridden InterfaceElement::setTextColor() call";
	DEBUG_ASSERT(false);
}

std::unique_ptr<InterfaceElement> InterfaceElement::put(
	std::unique_ptr<InterfaceElement>)
{
	LOGW << "Non-overridden InterfaceElement::put() call";
	DEBUG_ASSERT(false);
	return makeGarbage();
}

void InterfaceElement::add(stringref, std::unique_ptr<InterfaceElement>)
{
	LOGW << "Non-overridden InterfaceElement::add() call";
	DEBUG_ASSERT(false);
}

std::unique_ptr<InterfaceElement> InterfaceElement::replace(stringref,
	std::unique_ptr<InterfaceElement>,
	stringref)
{
	LOGW << "Non-overridden InterfaceElement::replace() call";
	DEBUG_ASSERT(false);
	return makeGarbage();
}

std::unique_ptr<InterfaceElement> InterfaceElement::remove(stringref)
{
	LOGW << "Non-overridden InterfaceElement::remove() call";
	DEBUG_ASSERT(false);
	return makeGarbage();
}

std::string InterfaceElement::name(size_t)
{
	LOGW << "Non-overridden InterfaceElement::name() call";
	DEBUG_ASSERT(false);
	return "";
}

InterfaceElement* InterfaceElement::get(stringref)
{
	LOGW << "Non-overridden InterfaceElement::get() call";
	DEBUG_ASSERT(false);
	return garbage();
}

bool InterfaceElement::contains(stringref)
{
	LOGW << "Non-overridden InterfaceElement::name() call";
	DEBUG_ASSERT(false);
	return false;
}

InterfaceElement& InterfaceElement::content()
{
	LOGW << "Non-overridden InterfaceElement::content() call";
	DEBUG_ASSERT(false);
	return *garbage();
}

bool InterfaceElement::overed() const
{
	// Interface elements do not have mass by default, so cannot be overed.
	return false;
}

bool InterfaceElement::hovered() const
{
	// Interface elements do not have mass by default, so cannot be hovered.
	return false;
}

bool InterfaceElement::clicked() const
{
	return enabled()
		&& ((_immediateHotkeyScancode > 0
				&& Input::get()->wasKeyPressed(
					(SDL_Scancode) _immediateHotkeyScancode)
				&& shown())
			|| (_immediateHotkeyScancode2 > 0
				&& Input::get()->wasKeyPressed(
					(SDL_Scancode) _immediateHotkeyScancode2)
				&& shown())
			|| (_altHotkeyScancode > 0
				&& Input::get()->isKeyHeld(SDL_SCANCODE_ALT)
				&& Input::get()->wasKeyPressed(
					(SDL_Scancode) _altHotkeyScancode)
				&& shown())
			|| (hovered()
				&& Input::get()->wasKeyPressed(SDL_SCANCODE_LMB)));
}

bool InterfaceElement::held() const
{
	return enabled() && hovered() && Input::get()->isKeyHeld(SDL_SCANCODE_LMB);
}

bool InterfaceElement::released() const
{
	return enabled() && hovered() && Input::get()->wasKeyReleased(SDL_SCANCODE_LMB);
}

bool InterfaceElement::doubleclicked() const
{
	return held() && Input::get()->wasKeyDoublePressed(SDL_SCANCODE_LMB);
}

bool InterfaceElement::longheld() const
{
	return held() && Input::get()->isKeyLongHeld(SDL_SCANCODE_LMB);
}

bool InterfaceElement::longreleased() const
{
	return released() && Input::get()->wasKeyLongReleased(SDL_SCANCODE_LMB);
}

bool InterfaceElement::shortreleased() const
{
	return released() && Input::get()->wasKeyShortReleased(SDL_SCANCODE_LMB);
}

bool InterfaceElement::rightClicked() const
{
	return enabled() && hovered() && Input::get()->wasKeyPressed(SDL_SCANCODE_RMB);
}

void InterfaceElement::align(HorizontalAlignment)
{
	LOGW << "Non-overridden InterfaceElement::align() call";
	DEBUG_ASSERT(false);
}

void InterfaceElement::align(VerticalAlignment)
{
	LOGW << "Non-overridden InterfaceElement::align() call";
	DEBUG_ASSERT(false);
}

void InterfaceElement::setColor(size_t, const Paint&)
{
	LOGW << "Non-overridden InterfaceElement::setColor() call";
	DEBUG_ASSERT(false);
}

void InterfaceElement::setBasicColor(size_t, const Paint&)
{
	LOGW << "Non-overridden InterfaceElement::setColor() call";
	DEBUG_ASSERT(false);
}

void InterfaceElement::setDisabledColor(size_t, const Paint&)
{
	LOGW << "Non-overridden InterfaceElement::setColor() call";
	DEBUG_ASSERT(false);
}

void InterfaceElement::setHoveredColor(size_t, const Paint&)
{
	LOGW << "Non-overridden InterfaceElement::setColor() call";
	DEBUG_ASSERT(false);
}

void InterfaceElement::setPressedColor(size_t, const Paint&)
{
	LOGW << "Non-overridden InterfaceElement::setColor() call";
	DEBUG_ASSERT(false);
}

void InterfaceElement::setPowerColor(size_t, const Paint&)
{
	LOGW << "Non-overridden InterfaceElement::setColor() call";
	DEBUG_ASSERT(false);
}

void InterfaceElement::setShineColor(const Paint&)
{
	LOGW << "Non-overridden InterfaceElement::setColor() call";
	DEBUG_ASSERT(false);
}

void InterfaceElement::setTag(const std::string&, bool /**/)
{
	LOGW << "Non-overridden InterfaceElement::setTag() call";
	DEBUG_ASSERT(false);
}

void InterfaceElement::setTagActive(const std::string&, bool /**/)
{
	LOGW << "Non-overridden InterfaceElement::setTagActive() call";
	DEBUG_ASSERT(false);
}

std::string InterfaceElement::getTag()
{
	LOGW << "Non-overridden InterfaceElement::getTag() call";
	DEBUG_ASSERT(false);
	return "";
}

void InterfaceElement::setPicture(const std::string&)
{
	LOGW << "Non-overridden InterfaceElement::setPicture() call";
	DEBUG_ASSERT(false);
}






class GarbageInterfaceElement : public InterfaceElement
{
	virtual void debugtree(uint8_t depth) override
	{
		std::string spaces(depth, '.');
		std::cout << spaces << "GARBAGE" << std::endl;
	}

	virtual void setWidth(int) override
	{
		LOGW << "garbage";
		DEBUG_ASSERT(false);
	}

	virtual void setHeight(int) override
	{
		LOGW << "garbage";
		DEBUG_ASSERT(false);
	}

	virtual bool resizableWidth() override
	{
		LOGW << "garbage";
		DEBUG_ASSERT(false);
		return false;
	}

	virtual bool resizableHeight() override
	{
		LOGW << "garbage";
		DEBUG_ASSERT(false);
		return false;
	}

	virtual void place(const Pixel&) override
	{
		LOGW << "garbage";
		DEBUG_ASSERT(false);
	}

	virtual void refresh() override
	{
		LOGW << "garbage";
		DEBUG_ASSERT(false);
	}
};


static GarbageInterfaceElement _garbage = {};

InterfaceElement* InterfaceElement::garbage()
{
	LOGW << "garbage";
	DEBUG_ASSERT(false);
	return &_garbage;
}

std::unique_ptr<InterfaceElement> InterfaceElement::makeGarbage()
{
	return std::unique_ptr<InterfaceElement>(new GarbageInterfaceElement());
}
