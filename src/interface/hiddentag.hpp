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


class HiddenTag : public InterfaceElement
{
public:
	HiddenTag(const std::string text) :
		_text(text)
	{}

private:
	std::string _text;

	virtual void debugtree(uint8_t /**/) override;

	virtual std::string text() const override { return _text; }
	virtual void setText(const std::string& text) override { _text = text; }

	virtual void setWidth(int) override {}
	virtual void setHeight(int) override {}

	virtual bool resizableWidth() override { return false; }
	virtual bool resizableHeight() override { return false; }

	virtual void place(const Pixel&) override {}

	virtual void refresh() override {}
};
