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
#include "layout.hpp"
#include "source.hpp"


void Layout::debugtree(uint8_t depth)
{
	std::string spaces(depth, '.');
	std::cout << spaces << "{" << std::endl;
	for (size_t i = 0; i < std::max(_names.size(), _elements.size()); i++)
	{
		if (i < _names.size())
		{
			std::cout << spaces << _names[i] << ":" << std::endl;
		}
		else std::cout << spaces << "NULL:" << std::endl;

		if (i < _elements.size())
		{
			_elements[i]->debugtree(depth + 1);
		}
		else std::cout << spaces << "VOID" << std::endl;
	}
	std::cout << spaces << "}" << std::endl;
}

void Layout::bear(uint8_t depth)
{
	InterfaceElement::bear(depth);
	for (auto& element : _elements)
	{
		element->bear(depth + 1);
	}
}

void Layout::kill(uint8_t depth)
{
	InterfaceElement::kill(depth);
	for (auto& element : _elements)
	{
		element->kill(depth + 1);
	}
}

void Layout::show(uint8_t depth)
{
	InterfaceElement::show(depth);
	for (auto& element : _elements)
	{
		element->show(depth + 1);
	}
}

void Layout::hide(uint8_t depth)
{
	InterfaceElement::hide(depth);
	for (auto& element : _elements)
	{
		element->hide(depth + 1);
	}
}

void Layout::enable(uint8_t depth)
{
	InterfaceElement::enable(depth);
	for (auto& element : _elements)
	{
		element->enable(depth + 1);
	}
}

void Layout::disable(uint8_t depth)
{
	InterfaceElement::disable(depth);
	for (auto& element : _elements)
	{
		element->disable(depth + 1);
	}
}

void Layout::power(uint8_t depth)
{
	InterfaceElement::power(depth);
	for (auto& element : _elements)
	{
		element->power(depth + 1);
	}
}

void Layout::depower(uint8_t depth)
{
	InterfaceElement::depower(depth);
	for (auto& element : _elements)
	{
		element->depower(depth + 1);
	}
}

void Layout::shine(uint8_t depth)
{
	InterfaceElement::shine(depth);
	for (auto& element : _elements)
	{
		element->shine(depth + 1);
	}
}

void Layout::deshine(uint8_t depth)
{
	InterfaceElement::deshine(depth);
	for (auto& element : _elements)
	{
		element->deshine(depth + 1);
	}
}


void Layout::add(stringref name, std::unique_ptr<InterfaceElement> element)
{
	impart(element);
	_elements.emplace_back(std::move(element));
	_names.emplace_back(name.str());
	DEBUG_ASSERT(_elements.size() == _names.size());
	added();
}

std::unique_ptr<InterfaceElement> Layout::replace(stringref name,
		std::unique_ptr<InterfaceElement> element,
		stringref newname)
{
	std::unique_ptr<InterfaceElement> found;

	for (size_t i = 0; i < _elements.size(); i++)
	{
		if (_names[i] == name)
		{
			impart(element);
			found = std::move(_elements[i]);
			_elements[i] = std::move(element);
			_names[i] = newname.str();
			break;
		}
	}

	if (found)
	{
		added();
		return found;
	}
	else
	{
		LOGW << "Replacing non-existent item '" << name << "'";
		DEBUG_ASSERT(false);
		return makeGarbage();
	}
}

std::unique_ptr<InterfaceElement> Layout::remove(stringref name)
{
	std::unique_ptr<InterfaceElement> found;

	for (size_t i = 0; i < _elements.size(); i++)
	{
		if (_names[i] == name)
		{
			found = std::move(_elements[i]);
			_elements.erase(_elements.begin() + i);
			_names.erase(_names.begin() + i);
			break;
		}
	}

	if (found)
	{
		removed();
		return found;
	}
	else
	{
		LOGW << "Removing non-existent item '" << name << "'";
		DEBUG_ASSERT("Removing non-existent item.");
		return makeGarbage();
	}
}

void Layout::refresh()
{
	for (auto& element : _elements)
	{
		element->refresh();
	}
}

bool Layout::overed() const
{
	for (auto& element : _elements)
	{
		if (element->overed()) return true;
	}

	return false;
}

bool Layout::hovered() const
{
	// Because some items might occlude others, we must know which layer the mouse operates in.
	// We therefore need to check all items to see if they are hovered.
	bool result = false;

	for (auto& element : _elements)
	{
		if (element->hovered()) result = true;
	}

	return result;
}

bool Layout::clicked() const
{
	// Because some items might occlude others, we must know which layer the mouse operates in.
	// We therefore need to check all items to see if they are hovered.
	bool result = false;

	for (auto& element : _elements)
	{
		if (element->clicked()) result = true;
	}

	return result;
}

bool Layout::held() const
{
	// Because some items might occlude others, we must know which layer the mouse operates in.
	// We therefore need to check all items to see if they are hovered.
	bool result = false;

	for (auto& element : _elements)
	{
		if (element->held()) result = true;
	}

	return result;
}

bool Layout::released() const
{
	// Because some items might occlude others, we must know which layer the mouse operates in.
	// We therefore need to check all items to see if they are hovered.
	bool result = false;

	for (auto& element : _elements)
	{
		if (element->released()) result = true;
	}

	return result;
}

std::string Layout::name(size_t offset)
{
	if (offset >= _names.size())
	{
		LOGE << "index of out bounds";
		DEBUG_ASSERT(false);
		return "";
	}

	return _names[offset];
}

bool Layout::contains(stringref name)
{
	for (size_t i = 0; i < _names.size(); i++)
	{
		if (_names[i] == name) return true;
	}
	return false;
}

InterfaceElement* Layout::get(stringref name)
{
	for (size_t i = 0; i < _names.size(); i++)
	{
		if (_names[i] == name) return _elements[i].get();
	}
	LOGE << "unknown index '" << name << "'";
	DEBUG_ASSERT(false);
	return garbage();
}

void Layout::checkPictures()
{
	for (auto& element : _elements)
	{
		element->checkPictures();
	}
}

void Layout::checkPicture(const std::string& picturename)
{
	for (auto& element : _elements)
	{
		element->checkPicture(picturename);
	}
}
