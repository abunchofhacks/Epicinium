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

#include "interfaceelement.hpp"


class Layout : public InterfaceElement
{
protected:
	std::vector<std::unique_ptr<InterfaceElement>> _elements;
	std::vector<std::string> _names;

	virtual void added() = 0;
	virtual void removed() = 0;

public:
	virtual void debugtree(uint8_t depth) override;

	virtual void bear(uint8_t depth) override; using InterfaceElement::bear;
	virtual void kill(uint8_t depth) override; using InterfaceElement::kill;
	virtual void show(uint8_t depth) override; using InterfaceElement::show;
	virtual void hide(uint8_t depth) override; using InterfaceElement::hide;
	virtual void enable(uint8_t depth) override; using InterfaceElement::enable;
	virtual void disable(uint8_t depth) override; using InterfaceElement::disable;
	virtual void power(uint8_t depth) override; using InterfaceElement::power;
	virtual void depower(uint8_t depth) override; using InterfaceElement::depower;
	virtual void shine(uint8_t depth) override; using InterfaceElement::shine;
	virtual void deshine(uint8_t depth) override; using InterfaceElement::deshine;

	using InterfaceElement::add;
	virtual void add(stringref name,
		std::unique_ptr<InterfaceElement> element) override;
	using InterfaceElement::replace;
	virtual std::unique_ptr<InterfaceElement> replace(stringref name,
		std::unique_ptr<InterfaceElement> element,
		stringref newname) override;
	virtual std::unique_ptr<InterfaceElement> remove(stringref name) override;
	virtual void refresh() override;
	virtual void reset() override = 0;

	virtual bool overed() const override;
	virtual bool hovered() const override;
	virtual bool clicked() const override;
	virtual bool held() const override;
	virtual bool released() const override;

	virtual std::string name(size_t offset) override;
	virtual bool contains(stringref name) override;
	virtual InterfaceElement* get(stringref name) override;

	virtual size_t size() const override { return _elements.size(); }

	virtual void checkPictures() override;
	virtual void checkPicture(const std::string& picturename) override;
};
