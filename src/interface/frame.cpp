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
#include "frame.hpp"
#include "source.hpp"

#include "paint.hpp"
#include "colorname.hpp"
#include "sprite.hpp"
#include "picture.hpp"
#include "collector.hpp"
#include "loop.hpp"


Frame::Frame(const char* sname) :
	Image(sname)
{
	_sprite->setNinePatch();

	_basicPalette = {
		ColorName::FRAME200,
		Paint::blend(ColorName::FRAME200, ColorName::SHADEBLEND, 0.5f),
		Paint::blend(ColorName::FRAME600, ColorName::SHINEBLEND),
		Paint::blend(ColorName::FRAME600, ColorName::SHADEBLEND),
		Color::transparent(),
		Color::transparent(),
	};

	_disabledPalette = {
		ColorName::DISABLEDBLEND,
		ColorName::DISABLEDBLEND,
		ColorName::DISABLEDBLEND,
		ColorName::DISABLEDBLEND,
		ColorName::DISABLEDBLEND,
		ColorName::DISABLEDBLEND,
	};

	_hoveredPalette = {
		ColorName::HOVEREDBLEND,
		ColorName::HOVEREDBLEND,
		ColorName::HOVEREDBLEND,
		ColorName::HOVEREDBLEND,
		ColorName::HOVEREDBLEND,
		ColorName::HOVEREDBLEND,
	};

	_pressedPalette = {
		ColorName::PRESSEDBLEND,
		ColorName::PRESSEDBLEND,
		ColorName::PRESSEDBLEND,
		ColorName::PRESSEDBLEND,
		ColorName::PRESSEDBLEND,
		ColorName::PRESSEDBLEND,
	};

	_powerPalette = {
		ColorName::FRAME100,
		Paint::blend(ColorName::FRAME100, ColorName::SHADEBLEND, 0.5f),
		Paint::blend(ColorName::FRAME800, ColorName::SHINEBLEND),
		Paint::blend(ColorName::FRAME800, ColorName::SHADEBLEND),
		Color::transparent(),
		Color::transparent(),
	};

	_palettesize = std::min(_sprite->paletteSize(), 6);
}

Frame::Frame(const Skin& skin) :
	Image(skin)
{}

std::unique_ptr<InterfaceElement> Frame::makeItem()
{
	return makeItem(ColorName::FRAMETRANSPARENTITEM);
}

std::unique_ptr<InterfaceElement> Frame::makeItem(const Paint& color)
{
	return makeItem(color, true, true);
}

std::unique_ptr<InterfaceElement> Frame::makeItem(const Paint& color,
	bool isTop, bool isBottom)
{
	const char* framesprite = "ui/frame_item_9";
	if      ( isTop &&  isBottom) framesprite = "ui/frame_item_9";
	else if ( isTop && !isBottom) framesprite = "ui/frame_segment_top_9";
	else if (!isTop &&  isBottom) framesprite = "ui/frame_segment_bot_9";
	else if (!isTop && !isBottom) framesprite = "ui/frame_segment_mid_9";

	std::unique_ptr<InterfaceElement> elementPtr(new Frame(framesprite));
	auto& element = *elementPtr;

	element.setBasicColor(0, color);
	element.setBasicColor(1, Paint::blend(color, ColorName::SHADEBLEND, 0.5f));
	element.setBasicColor(2, Paint::blend(color, ColorName::SHINEBLEND));
	element.setBasicColor(3, Paint::blend(color, ColorName::SHADEBLEND));
	element.setPowerColor(0, ColorName::FRAME100);
	element.setPowerColor(1, Paint::blend(ColorName::FRAME100, ColorName::SHADEBLEND, 0.5f));
	element.setPowerColor(2, Paint::blend(color, ColorName::SHINEBLEND));
	element.setPowerColor(3, Paint::blend(color, ColorName::SHADEBLEND));
	element.setPaddingHorizontal(5 * InterfaceElement::scale());
	element.setPaddingVertical(1 * InterfaceElement::scale());

	return elementPtr;
}

std::unique_ptr<InterfaceElement> Frame::makeMini()
{
	return makeMini(ColorName::FRAMETRANSPARENTITEM);
}

std::unique_ptr<InterfaceElement> Frame::makeMini(const Paint& color)
{
	const char* framesprite = "ui/frame_mini_9";
	std::unique_ptr<InterfaceElement> elementPtr(new Frame(framesprite));
	auto& element = *elementPtr;

	element.setBasicColor(0, color);
	element.setBasicColor(1, Paint::blend(color, ColorName::SHADEBLEND, 0.5f));
	element.setBasicColor(2, Paint::blend(color, ColorName::SHINEBLEND));
	element.setBasicColor(3, Paint::blend(color, ColorName::SHADEBLEND));
	element.setPowerColor(0, ColorName::FRAME100);
	element.setPowerColor(1, Paint::blend(ColorName::FRAME100, ColorName::SHADEBLEND, 0.5f));
	element.setPowerColor(2, Paint::blend(color, ColorName::SHINEBLEND));
	element.setPowerColor(3, Paint::blend(color, ColorName::SHADEBLEND));
	element.setPadding(2 * InterfaceElement::scale());

	return elementPtr;
}

std::unique_ptr<InterfaceElement> Frame::makeTooltip(bool hard)
{
	std::unique_ptr<InterfaceElement> elementPtr(new Frame("ui/frame_window_9"));
	auto& element = *elementPtr;

	if (hard)
	{
		element.setBasicColor(0, ColorName::FRAMESAND);
		element.setBasicColor(1, Paint::blend(ColorName::FRAMESAND, ColorName::SHADEBLEND, 0.5f));
	}
	else
	{
		Color color = Paint::alpha(ColorName::FRAMESAND, 220);
		element.setColor(0, color);
		element.setColor(1, Paint::blend(color, ColorName::SHADEBLEND, 0.33f));
		element.setColor(2, Paint::blend(color, ColorName::SHADEBLEND, 0.67f));
		element.setColor(3, Paint::blend(color, ColorName::SHADEBLEND));
		element.setDisabledColor(0, Color::transparent());
		element.setDisabledColor(1, Color::transparent());
		element.setDisabledColor(2, Color::transparent());
		element.setDisabledColor(3, Color::transparent());
	}

	element.setPadding(4 * InterfaceElement::scale());

	return elementPtr;
}

std::unique_ptr<InterfaceElement> Frame::makeInform()
{
	std::unique_ptr<InterfaceElement> elementPtr(new Frame("ui/frame_window_9"));
	auto& element = *elementPtr;

	{
		Color color = Paint::alpha(ColorName::FRAME600, 240);
		element.setColor(0, color);
		element.setColor(1, Paint::blend(color, ColorName::SHADEBLEND, 0.33f));
		element.setColor(2, Paint::blend(color, ColorName::SHADEBLEND, 0.67f));
		element.setColor(3, Paint::blend(color, ColorName::SHADEBLEND));
		element.setDisabledColor(0, Color::transparent());
		element.setDisabledColor(1, Color::transparent());
		element.setDisabledColor(2, Color::transparent());
		element.setDisabledColor(3, Color::transparent());
	}

	element.setPadding(4 * InterfaceElement::scale());

	return elementPtr;
}

std::unique_ptr<InterfaceElement> Frame::makeForm()
{
	std::unique_ptr<InterfaceElement> elementPtr(new Frame("ui/frame_window_9"));
	auto& element = *elementPtr;

	{
		Color color = Paint::alpha(ColorName::FRAMESAND, 240);
		element.setColor(0, color);
		element.setColor(1, Paint::blend(color, ColorName::SHADEBLEND, 0.33f));
		element.setColor(2, Paint::blend(color, ColorName::SHADEBLEND, 0.67f));
		element.setColor(3, Paint::blend(color, ColorName::SHADEBLEND));
		element.setDisabledColor(0, Color::transparent());
		element.setDisabledColor(1, Color::transparent());
		element.setDisabledColor(2, Color::transparent());
		element.setDisabledColor(3, Color::transparent());
	}

	element.setPadding(10 * InterfaceElement::scale());

	return elementPtr;
}

void Frame::mix()
{
	if (_background)
	{
		{
			float hover = hoverable() ? _transitions[TRANSITION_HOVERED] : 0.0f;
			float saturation = _transitions[TRANSITION_ENABLED]
				* (0.30f + 0.70f * hover);
			_background->setGrayed(1.00f - saturation);
		}

		// We mix the hovered, pressed and disabled palettes instead of
		// blending, so we can control the alpha channel.
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
				mixed = Color::mix(mixed, _hoveredPalette[i], hover);
			}
			if (press > 0.001f)
			{
				mixed = Color::mix(mixed, _pressedPalette[i], press);
			}
			if (disab > 0.001f)
			{
				mixed = Color::mix(mixed, _disabledPalette[i], disab);
			}

			_sprite->setColor(i, mixed);
		}
	}
	else
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
}

void Frame::drawSprite()
{
	if (!shown()) return;

	if (_background)
	{
		Collector::get()->add(_background, _topleft);

		Pixel topleftplus = _topleft;
		topleftplus.proximity += 1;
		Collector::get()->addFrame(_sprite, topleftplus);
	}
	else
	{
		Collector::get()->addFrame(_sprite, _topleft);
	}
}

void Frame::debugtree(uint8_t depth)
{
	std::string spaces(depth, '.');
	std::cout << spaces << "Content:" << std::endl;
	if (_content) _content->debugtree(depth + 1);
}

void Frame::checkPictures()
{
	if (_content) _content->checkPictures();

	if (_background
		&& _background->missing())
	{
		setPicture(_background->nameOfPicture());
	}
}

void Frame::checkPicture(const std::string& picturename)
{
	if (_content) _content->checkPicture(picturename);

	if (_background
		&& _background->missing()
		&& _background->nameOfPicture() == picturename)
	{
		setPicture(picturename);
	}
}

void Frame::setPicture(const std::string& picturename)
{
	if (picturename.empty())
	{
		_background = nullptr;
		return;
	}

	_background = std::make_shared<Picture>(picturename);

	_background->setWidth(_width);
	_background->setHeight(_height);
	_background->setAsBackgroundOf(_sprite->backgroundPattern());
}

std::unique_ptr<InterfaceElement> Frame::put(
	std::unique_ptr<InterfaceElement> content)
{
	impart(content);
	std::swap(_content, content);

	setPaddingTop(std::max(_paddingTop, _content->marginTop()));
	setPaddingLeft(std::max(_paddingLeft, _content->marginLeft()));
	setPaddingRight(std::max(_paddingRight, _content->marginRight()));
	setPaddingBottom(std::max(_paddingBottom, _content->marginBottom()));
	_sprite->setWidth(_content->width() + _content->marginLeft() + _content->marginRight());
	_sprite->setHeight(_content->height() + _content->marginTop() + _content->marginBottom());
	_width = _sprite->scaledWidth();
	_height = _sprite->scaledHeight();
	place(_topleft);

	return content;
}

InterfaceElement& Frame::content()
{
	if (!_content)
	{
		LOGW << "No content set";
		DEBUG_ASSERT(false);
		return *garbage();
	}

	return *_content;
}

void Frame::bear(uint8_t depth)
{
	Image::bear(depth);

	if (_content) _content->bear(depth + 1);
}

void Frame::kill(uint8_t depth)
{
	Image::kill(depth);

	if (_content) _content->kill(depth + 1);
}

void Frame::show(uint8_t depth)
{
	Image::show(depth);

	if (_content) _content->show(depth + 1);
}

void Frame::hide(uint8_t depth)
{
	Image::hide(depth);

	if (_content) _content->hide(depth + 1);
}

void Frame::enable(uint8_t depth)
{
	Image::enable(depth);

	if (_content) _content->enable(depth + 1);
}

void Frame::disable(uint8_t depth)
{
	Image::disable(depth);

	if (_content) _content->disable(depth + 1);
}

void Frame::power(uint8_t depth)
{
	Image::power(depth);

	if (_content) _content->power(depth + 1);
}

void Frame::depower(uint8_t depth)
{
	Image::depower(depth);

	if (_content) _content->depower(depth + 1);
}

void Frame::shine(uint8_t depth)
{
	Image::shine(depth);

	if (_background)
	{
		_background->setShine(1);
		_background->setThetaOffset(-0.2f - Loop::theta());
	}

	if (_content) _content->shine(depth + 1);
}

void Frame::deshine(uint8_t depth)
{
	Image::deshine(depth);

	if (_background)
	{
		addAnimation(Animation(nullptr, [this](float progress) {

			_background->setShine(1.0f - progress);

		}, 0.07f, 0));
	}

	if (_content) _content->deshine(depth + 1);
}

void Frame::setWidth(int w)
{
	if (_fixedWidth) return;

	_sprite->setWidth(w);
	if (_content)
	{
		_content->settleWidth();
		int marginWidth = _content->marginLeft() + _content->marginRight();
		if (_content->resizableWidth())
		{
			_content->setWidth(std::max(0, _sprite->scaledWidth() - marginWidth));
		}
		_sprite->setWidth(_content->width() + marginWidth);
	}
	_width = _sprite->scaledWidth();

	if (_background)
	{
		_background->setWidth(_width);
	}
}

void Frame::setHeight(int h)
{
	if (_fixedHeight) return;

	_sprite->setHeight(h);
	if (_content)
	{
		_content->settleHeight();
		int marginHeight = _content->marginTop() + _content->marginBottom();
		if (_content->resizableHeight())
		{
			_content->setHeight(std::max(0, _sprite->scaledHeight() - marginHeight));
		}
		_sprite->setHeight(_content->height() + marginHeight);
	}
	_height = _sprite->scaledHeight();

	if (_background)
	{
		_background->setHeight(_height);
	}
}

void Frame::settleWidth()
{
	if (_fixedWidth) return;

	_sprite->setWidth(0);
	if (_content)
	{
		_content->settleWidth();
		int marginWidth = _content->marginLeft() + _content->marginRight();
		_sprite->setWidth(_content->width() + marginWidth);
	}
	_width = _sprite->scaledWidth();

	if (_background)
	{
		_background->setWidth(_width);
	}
}

void Frame::settleHeight()
{
	if (_fixedHeight) return;

	_sprite->setHeight(0);
	if (_content)
	{
		_content->settleHeight();
		int marginHeight = _content->marginTop() + _content->marginBottom();
		_sprite->setHeight(_content->height() + marginHeight);
	}
	_height = _sprite->scaledHeight();

	if (_background)
	{
		_background->setHeight(_height);
	}
}

void Frame::setPaddingTop(int padding)
{
	_paddingTop = padding;

	if (!_content) return;

	_content->setMarginTop(padding);
}

void Frame::setPaddingLeft(int padding)
{
	_paddingLeft = padding;

	if (!_content) return;

	_content->setMarginLeft(padding);
}

void Frame::setPaddingRight(int padding)
{
	_paddingRight = padding;

	if (!_content) return;

	_content->setMarginRight(padding);
}

void Frame::setPaddingBottom(int padding)
{
	_paddingBottom = padding;

	if (!_content) return;

	_content->setMarginBottom(padding);
}

void Frame::setShineColor(const Paint& color)
{
	Image::setShineColor(color);

	if (_background)
	{
		_background->setShineColor(color);
	}
}

void Frame::place(const Pixel& topleft)
{
	_topleft = topleft;
	_thickness = 1;

	if (_background)
	{
		_thickness += 1;
	}

	if (_content)
	{
		Pixel contentTopleft(_topleft.xenon + _content->marginLeft(),
			_topleft.yahoo + _content->marginTop(),
			_topleft.proximity + _thickness);
		_content->place(contentTopleft);
		_thickness += _content->thickness();
	}
}

void Frame::refresh()
{
	Image::refresh();

	if (_content) _content->refresh();
}

// The following few functions pass the call to the content to make the frame
// class transparent.
size_t Frame::size() const
{
	if (!_content)
	{
		LOGW << "No content set.";
		DEBUG_ASSERT(false);
		return 0;
	}

	return _content->size();
}

std::string Frame::name(size_t offset)
{
	if (!_content)
	{
		LOGW << "No content set.";
		DEBUG_ASSERT(false);
		return "";
	}

	return _content->name(offset);
}

bool Frame::contains(stringref name)
{
	if (!_content)
	{
		LOGW << "No content set.";
		DEBUG_ASSERT(false);
		return "";
	}

	return _content->contains(name);
}

std::string Frame::text() const
{
	if (!_content)
	{
		LOGW << "No content set.";
		DEBUG_ASSERT(false);
		return "";
	}

	return _content->text();
}

void Frame::setText(const std::string& str)
{
	if (!_content)
	{
		LOGW << "No content set.";
		DEBUG_ASSERT(false);
		return;
	}

	return _content->setText(str);
}

void Frame::prefillText(const std::string& str)
{
	if (!_content)
	{
		LOGW << "No content set.";
		DEBUG_ASSERT(false);
		return;
	}

	return _content->prefillText(str);
}

void Frame::setTextColor(const Paint& color)
{
	if (!_content)
	{
		LOGW << "No content set.";
		DEBUG_ASSERT(false);
		return;
	}

	return _content->setTextColor(color);
}

void Frame::add(stringref name, std::unique_ptr<InterfaceElement> element)
{
	if (!_content)
	{
		LOGW << "No content set.";
		DEBUG_ASSERT(false);
		return;
	}

	_content->add(name, std::move(element));
}

std::unique_ptr<InterfaceElement> Frame::replace(stringref name,
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

InterfaceElement* Frame::get(stringref name)
{
	if (!_content)
	{
		LOGW << "No content set.";
		DEBUG_ASSERT(false);
		return garbage();
	}

	return _content->get(name);
}

std::unique_ptr<InterfaceElement> Frame::remove(stringref name)
{
	if (!_content)
	{
		LOGW << "No content set.";
		DEBUG_ASSERT(false);
		return makeGarbage();
	}

	return _content->remove(name);
}

void Frame::reset()
{
	if (!_content)
	{
		LOGW << "No content set.";
		DEBUG_ASSERT(false);
		return;
	}

	_content->reset();
}

bool Frame::resizableWidth()
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

bool Frame::resizableHeight()
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

void Frame::align(HorizontalAlignment horiz)
{
	if (!_content)
	{
		LOGW << "No content set.";
		DEBUG_ASSERT(false);
		return;
	}

	return _content->align(horiz);
}
void Frame::align(VerticalAlignment vert)
{
	if (!_content)
	{
		LOGW << "No content set.";
		DEBUG_ASSERT(false);
		return;
	}

	return _content->align(vert);
}

void Frame::setTag(const std::string& tag, bool randomize)
{
	if (!_content)
	{
		LOGW << "No content set.";
		DEBUG_ASSERT(false);
		return;
	}

	return  _content->setTag(tag, randomize);
}

void Frame::setTagActive(const std::string& tag, bool restart)
{
	if (!_content)
	{
		LOGW << "No content set.";
		DEBUG_ASSERT(false);
		return;
	}

	return  _content->setTagActive(tag, restart);
}

std::string Frame::getTag()
{
	if (!_content)
	{
		LOGW << "No content set.";
		DEBUG_ASSERT(false);
		return "";
	}

	return  _content->getTag();
}

bool Frame::overed() const
{
	if (!alive()) return false;

	bool result = false;

	// The content might have a tooltip that exceeds our borders.
	if (_content && _content->overed()) result = true;

	// The frame has mass so it itself can be hovered and clicked.
	if (Image::overed()) result = true;
	return result;
}

bool Frame::hovered() const
{
	if (!alive()) return false;

	bool result = false;

	// Because some items might occlude others, we must know which layer the
	// mouse operates in. We therefore need to check all items to see if they
	// are hovered.
	// Also, the content might have a tooltip that exceeds our borders.
	if (_content && _content->hovered()) result = true;

	// The frame has mass so it itself can be hovered and clicked.
	if (Image::hovered()) result = true;
	return result;
}
