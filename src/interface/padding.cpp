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
#include "padding.hpp"
#include "source.hpp"

#include "color.hpp"


std::unique_ptr<InterfaceElement> Padding::put(
	std::unique_ptr<InterfaceElement> content)
{
	impart(content);
	std::swap(_content, content);

	_width = _content->width() + _content->marginLeft() + _content->marginRight();
	_height = _content->height() + _content->marginTop() + _content->marginBottom();

	return content;
}

InterfaceElement& Padding::content()
{
	if (!_content)
	{
		LOGW << "No content set";
	DEBUG_ASSERT(false);
		return *garbage();
	}

	return *_content;
}

void Padding::debugtree(uint8_t depth)
{
	std::string spaces(depth, '.');
	std::cout << spaces << "Content:" << std::endl;
	if (_content) _content->debugtree(depth + 1);
}

void Padding::bear(uint8_t depth)
{
	InterfaceElement::bear(depth);

	if (_content) _content->bear(depth + 1);
}

void Padding::kill(uint8_t depth)
{
	InterfaceElement::kill(depth);

	if (_content) _content->kill(depth + 1);
}

void Padding::show(uint8_t depth)
{
	InterfaceElement::show(depth);

	if (_content) _content->show(depth + 1);
}

void Padding::hide(uint8_t depth)
{
	InterfaceElement::hide(depth);

	if (_content) _content->hide(depth + 1);
}

void Padding::enable(uint8_t depth)
{
	InterfaceElement::enable(depth);

	if (_content) _content->enable(depth + 1);
}

void Padding::disable(uint8_t depth)
{
	InterfaceElement::disable(depth);

	if (_content) _content->disable(depth + 1);
}

void Padding::power(uint8_t depth)
{
	InterfaceElement::power(depth);

	if (_content) _content->power(depth + 1);
}

void Padding::depower(uint8_t depth)
{
	InterfaceElement::depower(depth);

	if (_content) _content->depower(depth + 1);
}

void Padding::shine(uint8_t depth)
{
	InterfaceElement::shine(depth);

	if (_content) _content->shine(depth + 1);
}

void Padding::deshine(uint8_t depth)
{
	InterfaceElement::deshine(depth);

	if (_content) _content->deshine(depth + 1);
}

void Padding::setWidth(int w)
{
	if (_fixedWidth) return;

	if (!_content)
	{
		LOGW << "No content set.";
	DEBUG_ASSERT(false);
		return;
	}

	_content->settleWidth();
	int marginWidth = _content->marginLeft() + _content->marginRight();
	if (_content->resizableWidth())
	{
		_content->setWidth(std::max(0, w - marginWidth));
	}
	_width = _content->width() + marginWidth;
}

void Padding::setHeight(int h)
{
	if (_fixedHeight) return;

	if (!_content)
	{
		LOGW << "No content set.";
	DEBUG_ASSERT(false);
		return;
	}

	_content->settleHeight();
	int marginHeight = _content->marginTop() + _content->marginBottom();
	if (_content->resizableHeight())
	{
		_content->setHeight(std::max(0, h - marginHeight));
	}
	_height = _content->height() + marginHeight;
}

void Padding::settleWidth()
{
	if (_fixedWidth) return;

	if (!_content)
	{
		LOGW << "No content set.";
	DEBUG_ASSERT(false);
		return;
	}

	_content->settleWidth();
	int marginWidth = _content->marginLeft() + _content->marginRight();
	_width = _content->width() + marginWidth;
}

void Padding::settleHeight()
{
	if (_fixedHeight) return;

	if (!_content)
	{
		LOGW << "No content set.";
	DEBUG_ASSERT(false);
		return;
	}

	_content->settleHeight();
	int marginHeight = _content->marginTop() + _content->marginBottom();
	_height = _content->height() + marginHeight;
}

void Padding::setPaddingTop(int padding)
{
	if (!_content)
	{
		LOGW << "No content set";
	DEBUG_ASSERT(false);
		return;
	}

	_content->setMarginTop(std::max(padding, _content->marginTop()));
}

void Padding::setPaddingLeft(int padding)
{
	if (!_content)
	{
		LOGW << "No content set";
	DEBUG_ASSERT(false);
		return;
	}

	_content->setMarginLeft(std::max(padding, _content->marginLeft()));
}

void Padding::setPaddingRight(int padding)
{
	if (!_content)
	{
		LOGW << "No content set";
	DEBUG_ASSERT(false);
		return;
	}

	_content->setMarginRight(std::max(padding, _content->marginRight()));
}

void Padding::setPaddingBottom(int padding)
{
	if (!_content)
	{
		LOGW << "No content set";
	DEBUG_ASSERT(false);
		return;
	}

	_content->setMarginBottom(std::max(padding, _content->marginBottom()));
}

void Padding::place(const Pixel& topleft)
{
	_topleft = topleft;
	_thickness = 0;

	if (_content)
	{
		Pixel contentTopleft(_topleft.xenon + _content->marginLeft(),
			_topleft.yahoo + _content->marginTop(),
			_topleft.proximity + _thickness);
		_content->place(contentTopleft);
		_thickness += _content->thickness();
	}
}

void Padding::refresh()
{
	if (_content) _content->refresh();
}

// The following few functions pass the call to the content to make the Padding
// class transparent.
size_t Padding::size() const
{
	if (!_content)
	{
		LOGW << "No content set.";
		DEBUG_ASSERT(false);
		return 0;
	}

	return _content->size();
}

std::string Padding::name(size_t offset)
{
	if (!_content)
	{
		LOGW << "No content set.";
		DEBUG_ASSERT(false);
		return "";
	}

	return _content->name(offset);
}

bool Padding::contains(stringref name)
{
	if (!_content)
	{
		LOGW << "No content set.";
		DEBUG_ASSERT(false);
		return "";
	}

	return _content->contains(name);
}

std::string Padding::text() const
{
	if (!_content)
	{
		LOGW << "No content set.";
		DEBUG_ASSERT(false);
		return "";
	}

	return _content->text();
}

void Padding::setText(const std::string& str)
{
	if (!_content)
	{
		LOGW << "No content set.";
	DEBUG_ASSERT(false);
		return;
	}

	return _content->setText(str);
}

void Padding::prefillText(const std::string& str)
{
	if (!_content)
	{
		LOGW << "No content set.";
	DEBUG_ASSERT(false);
		return;
	}

	return _content->prefillText(str);
}

void Padding::setTextColor(const Paint& color)
{
	if (!_content)
	{
		LOGW << "No content set.";
	DEBUG_ASSERT(false);
		return;
	}

	return _content->setTextColor(color);
}

void Padding::add(stringref name, std::unique_ptr<InterfaceElement> element)
{
	if (!_content)
	{
		LOGW << "No content set.";
		DEBUG_ASSERT(false);
		return;
	}

	_content->add(name, std::move(element));
}

std::unique_ptr<InterfaceElement> Padding::replace(stringref name,
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

InterfaceElement* Padding::get(stringref name)
{
	if (!_content)
	{
		LOGW << "No content set.";
		DEBUG_ASSERT(false);
		return garbage();
	}

	return _content->get(name);
}

std::unique_ptr<InterfaceElement> Padding::remove(stringref name)
{
	if (!_content)
	{
		LOGW << "No content set.";
		DEBUG_ASSERT(false);
		return makeGarbage();
	}

	return _content->remove(name);
}

void Padding::reset()
{
	if (!_content)
	{
		LOGW << "No content set.";
		DEBUG_ASSERT(false);
		return;
	}

	_content->reset();
}

bool Padding::resizableWidth()
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

bool Padding::resizableHeight()
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

void Padding::align(HorizontalAlignment horiz)
{
	if (!_content)
	{
		LOGW << "No content set.";
		DEBUG_ASSERT(false);
		return;
	}

	return _content->align(horiz);
}

void Padding::align(VerticalAlignment vert)
{
	if (!_content)
	{
		LOGW << "No content set.";
		DEBUG_ASSERT(false);
		return;
	}

	return _content->align(vert);
}

void Padding::setColor(size_t index, const Paint& color)
{
	if (!_content)
	{
		LOGW << "No content set.";
		DEBUG_ASSERT(false);
		return;
	}

	return _content->setColor(index, color);
}

void Padding::setBasicColor(size_t index, const Paint& color)
{
	if (!_content)
	{
		LOGW << "No content set.";
		DEBUG_ASSERT(false);
		return;
	}

	return _content->setBasicColor(index, color);
}

void Padding::setDisabledColor(size_t index, const Paint& color)
{
	if (!_content)
	{
		LOGW << "No content set.";
		DEBUG_ASSERT(false);
		return;
	}

	return _content->setDisabledColor(index, color);
}

void Padding::setHoveredColor(size_t index, const Paint& color)
{
	if (!_content)
	{
		LOGW << "No content set.";
		DEBUG_ASSERT(false);
		return;
	}

	return _content->setHoveredColor(index, color);
}

void Padding::setPressedColor(size_t index, const Paint& color)
{
	if (!_content)
	{
		LOGW << "No content set.";
		DEBUG_ASSERT(false);
		return;
	}

	return _content->setPressedColor(index, color);
}

void Padding::setPowerColor(size_t index, const Paint& color)
{
	if (!_content)
	{
		LOGW << "No content set.";
		DEBUG_ASSERT(false);
		return;
	}

	return _content->setPowerColor(index, color);
}

void Padding::setShineColor(const Paint& color)
{
	if (!_content)
	{
		LOGW << "No content set.";
		DEBUG_ASSERT(false);
		return;
	}

	return _content->setShineColor(color);
}

void Padding::setTag(const std::string& tag, bool randomize)
{
	if (!_content)
	{
		LOGW << "No content set.";
	DEBUG_ASSERT(false);
		return;
	}

	return  _content->setTag(tag, randomize);
}

void Padding::setTagActive(const std::string& tag, bool restart)
{
	if (!_content)
	{
		LOGW << "No content set.";
	DEBUG_ASSERT(false);
		return;
	}

	return  _content->setTagActive(tag, restart);
}

std::string Padding::getTag()
{
	if (!_content)
	{
		LOGW << "No content set.";
	DEBUG_ASSERT(false);
		return "";
	}

	return  _content->getTag();
}

bool Padding::overed() const
{
	if (_content) return _content->overed();
	else return false;
}

bool Padding::hovered() const
{
	if (_content) return _content->hovered();
	else return false;
}

bool Padding::clicked() const
{
	if (_content) return _content->clicked();
	else return false;
}

bool Padding::held() const
{
	if (_content) return _content->held();
	else return false;
}

bool Padding::released() const
{
	if (_content) return _content->released();
	else return false;
}

void Padding::checkPictures()
{
	if (_content) _content->checkPictures();
}

void Padding::checkPicture(const std::string& picturename)
{
	if (_content) _content->checkPicture(picturename);
}
