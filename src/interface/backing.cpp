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
#include "backing.hpp"
#include "source.hpp"

#include "pixel.hpp"


std::unique_ptr<InterfaceElement> Backing::putBacking(
	std::unique_ptr<InterfaceElement> backing)
{
	impart(backing);
	std::swap(_backing, backing);

	if (_content)
	{
		_backing->fixWidth(_content->width());
		_backing->fixHeight(_content->height());
	}

	return backing;
}

std::unique_ptr<InterfaceElement> Backing::put(
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

InterfaceElement& Backing::content()
{
	if (!_content)
	{
		LOGW << "No content set";
		DEBUG_ASSERT(false);
		return *garbage();
	}

	return *_content;
}

void Backing::debugtree(uint8_t depth)
{
	std::string spaces(depth, '.');
	std::cout << spaces << "Content:" << std::endl;
	if (_content) _content->debugtree(depth + 1);
	std::cout << spaces << "Backing:" << std::endl;
	if (_backing) _backing->debugtree(depth + 1);
}

void Backing::bear(uint8_t depth)
{
	InterfaceElement::bear(depth);

	if (_content) _content->bear(depth + 1);
	if (_backing) _backing->bear(depth + 1);
}

void Backing::kill(uint8_t depth)
{
	InterfaceElement::kill(depth);

	if (_content) _content->kill(depth + 1);
	if (_backing) _backing->kill(depth + 1);
}

void Backing::show(uint8_t depth)
{
	InterfaceElement::show(depth);

	if (_content) _content->show(depth + 1);
	if (_backing) _backing->show(depth + 1);
}

void Backing::hide(uint8_t depth)
{
	InterfaceElement::hide(depth);

	if (_content) _content->hide(depth + 1);
	if (_backing) _backing->hide(depth + 1);
}

void Backing::enable(uint8_t depth)
{
	InterfaceElement::enable(depth);

	if (_content) _content->enable(depth + 1);
	if (_backing) _backing->enable(depth + 1);
}

void Backing::disable(uint8_t depth)
{
	InterfaceElement::disable(depth);

	if (_content) _content->disable(depth + 1);
	if (_backing) _backing->disable(depth + 1);
}

void Backing::power(uint8_t depth)
{
	InterfaceElement::power(depth);

	if (_content) _content->power(depth + 1);
	if (_backing) _backing->power(depth + 1);
}

void Backing::depower(uint8_t depth)
{
	InterfaceElement::depower(depth);

	if (_content) _content->depower(depth + 1);
	if (_backing) _backing->depower(depth + 1);
}

void Backing::shine(uint8_t depth)
{
	InterfaceElement::shine(depth);

	if (_content) _content->shine(depth + 1);
	if (_backing) _backing->shine(depth + 1);
}

void Backing::deshine(uint8_t depth)
{
	InterfaceElement::deshine(depth);

	if (_content) _content->deshine(depth + 1);
	if (_backing) _backing->deshine(depth + 1);
}

void Backing::setWidth(int w)
{
	if (_fixedWidth) return;

	if (!_content)
	{
		LOGW << "No content set.";
		DEBUG_ASSERT(false);
		return;
	}

	_content->setWidth(w);
	_backing->fixWidth(_content->width());
	_width = _content->width();
}

void Backing::setHeight(int h)
{
	if (_fixedHeight) return;

	if (!_content)
	{
		LOGW << "No content set.";
		DEBUG_ASSERT(false);
		return;
	}

	_content->setHeight(h);
	_backing->fixHeight(_content->height());
	_height = _content->height();
}

void Backing::settleWidth()
{
	if (_fixedWidth) return;

	if (!_content)
	{
		LOGW << "No content set.";
		DEBUG_ASSERT(false);
		return;
	}

	_content->settleWidth();
	_backing->fixWidth(_content->width());
	_width = _content->width();
}

void Backing::settleHeight()
{
	if (_fixedHeight) return;

	if (!_content)
	{
		LOGW << "No content set.";
		DEBUG_ASSERT(false);
		return;
	}

	_content->settleHeight();
	_backing->fixHeight(_content->height());
	_height = _content->height();
}

void Backing::setMarginTop(int margin)
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

void Backing::setMarginLeft(int margin)
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

void Backing::setMarginRight(int margin)
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

void Backing::setMarginBottom(int margin)
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

// The following few functions pass the call to the content to make the Backing
// class transparent.
size_t Backing::size() const
{
	if (!_content)
	{
		LOGW << "No content set.";
		DEBUG_ASSERT(false);
		return 0;
	}

	return _content->size();
}

std::string Backing::name(size_t offset)
{
	if (!_content)
	{
		LOGW << "No content set.";
		DEBUG_ASSERT(false);
		return "";
	}

	return _content->name(offset);
}

bool Backing::contains(stringref name)
{
	if (!_content)
	{
		LOGW << "No content set.";
		DEBUG_ASSERT(false);
		return "";
	}

	return _content->contains(name);
}

std::string Backing::text() const
{
	if (!_content)
	{
		LOGW << "No content set.";
		DEBUG_ASSERT(false);
		return "";
	}

	return _content->text();
}

// The following few functions pass the call to the content to make the Backing
// class transparent, except that they listen for the word "backing".
void Backing::add(stringref name, std::unique_ptr<InterfaceElement> element)
{
	if (name == "backing")
	{
		putBacking(std::move(element));
		return;
	}

	if (!_content)
	{
		LOGW << "No content set.";
		DEBUG_ASSERT(false);
		return;
	}

	_content->add(name, std::move(element));
}

std::unique_ptr<InterfaceElement> Backing::replace(stringref name,
	std::unique_ptr<InterfaceElement> element,
	stringref newname)
{
	if (name == "backing")
	{
		return putBacking(std::move(element));
	}

	if (!_content)
	{
		LOGW << "No content set.";
		DEBUG_ASSERT(false);
		return makeGarbage();
	}

	return _content->replace(name, std::move(element), newname);
}

InterfaceElement* Backing::get(stringref name)
{
	if (name == "backing")
	{
		return _backing.get();
	}

	if (!_content)
	{
		LOGW << "No content set.";
		DEBUG_ASSERT(false);
		return garbage();
	}

	return _content->get(name);
}

std::unique_ptr<InterfaceElement> Backing::remove(stringref name)
{
	if (name == "backing")
	{
		return putBacking(nullptr);
	}

	if (!_content)
	{
		LOGW << "No content set.";
		DEBUG_ASSERT(false);
		return makeGarbage();
	}

	return _content->remove(name);
}

// The following few functions pass the call to the content to make the Backing
// class transparent.
void Backing::reset()
{
	if (!_content)
	{
		LOGW << "No content set.";
		DEBUG_ASSERT(false);
		return;
	}

	_content->reset();
}

bool Backing::resizableWidth()
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

bool Backing::resizableHeight()
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

void Backing::align(HorizontalAlignment horiz)
{
	if (!_content)
	{
		LOGW << "No content set.";
		DEBUG_ASSERT(false);
		return;
	}

	return _content->align(horiz);
}
void Backing::align(VerticalAlignment vert)
{
	if (!_content)
	{
		LOGW << "No content set.";
		DEBUG_ASSERT(false);
		return;
	}

	return _content->align(vert);
}

void Backing::setTag(const std::string& tag, bool randomize)
{
	if (!_content)
	{
		LOGW << "No content set.";
		DEBUG_ASSERT(false);
		return;
	}

	return  _content->setTag(tag, randomize);
}

void Backing::setTagActive(const std::string& tag, bool restart)
{
	if (!_content)
	{
		LOGW << "No content set.";
		DEBUG_ASSERT(false);
		return;
	}

	return  _content->setTagActive(tag, restart);
}

std::string Backing::getTag()
{
	if (!_content)
	{
		LOGW << "No content set.";
		DEBUG_ASSERT(false);
		return "";
	}

	return  _content->getTag();
}

bool Backing::overed() const
{
	if (_content && _content->overed()) return true;
	if (_backing && _backing->overed()) return true;

	return false;
}

bool Backing::hovered() const
{
	// Because some items might occlude others, we must know which layer the mouse operates in.
	// We therefore need to check all items to see if they are hovered.
	bool result = false;

	if (_content && _content->hovered()) result = true;
	if (_backing && _backing->hovered()) result = true;

	return result;
}

bool Backing::clicked() const
{
	// Because some items might occlude others, we must know which layer the mouse operates in.
	// We therefore need to check all items to see if they are hovered.
	bool result = false;

	if (_content && _content->clicked()) result = true;
	if (_backing && _backing->clicked()) result = true;

	return result;
}

bool Backing::held() const
{
	// Because some items might occlude others, we must know which layer the mouse operates in.
	// We therefore need to check all items to see if they are hovered.
	bool result = false;

	if (_content && _content->held()) result = true;
	if (_backing && _backing->held()) result = true;

	return result;
}

bool Backing::released() const
{
	// Because some items might occlude others, we must know which layer the mouse operates in.
	// We therefore need to check all items to see if they are hovered.
	bool result = false;

	if (_content && _content->released()) result = true;
	if (_backing && _backing->released()) result = true;

	return result;
}

void Backing::refresh()
{
	if (_content) _content->refresh();
	if (_backing) _backing->refresh();
}

void Backing::place(const Pixel& topleft)
{
	_topleft = topleft;
	_thickness = 0;

	Pixel elementTopleft(_topleft);
	if (_backing)
	{
		_backing->place(elementTopleft);
		_thickness += _backing->thickness();
		elementTopleft.proximity += _backing->thickness();
	}

	if (_content)
	{
		_content->place(elementTopleft);
		_thickness += _content->thickness();
		elementTopleft.proximity += _content->thickness();
	}
}

void Backing::checkPictures()
{
	if (_content) _content->checkPictures();
	if (_backing) _backing->checkPictures();
}

void Backing::checkPicture(const std::string& picturename)
{
	if (_content) _content->checkPicture(picturename);
	if (_backing) _backing->checkPicture(picturename);
}
