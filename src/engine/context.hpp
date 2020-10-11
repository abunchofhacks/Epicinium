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

#include <map>

#include "interfaceelement.hpp"
#include "position.hpp"
#include "point.hpp"

enum class Move : uint8_t;
struct Color;


class Context : public InterfaceElement
{
protected:
	Context(const Position& position);
public:
	Context(const Context&) = delete;
	Context(Context&&) = delete;
	Context& operator=(const Context&) = delete;
	Context& operator=(Context&&) = delete;
	virtual ~Context();

protected:
	Position _position;
	std::map<Move, std::unique_ptr<InterfaceElement>> _panels;
	bool _hoverValid;
	Move _hoverIndex;
	Move _overIndex;
	Point _initialMousePoint;
	float _delay;

private:
	virtual void debugtree(uint8_t depth) override;

	virtual void setWidth(int w) override;
	virtual void setHeight(int h) override;
	virtual bool resizableWidth() override { return false; }
	virtual bool resizableHeight() override { return false; }
	virtual void place(const Pixel& topleft) override;

	virtual void bear(uint8_t depth) override; using InterfaceElement::bear;
	virtual void kill(uint8_t depth) override; using InterfaceElement::kill;
	virtual void show(uint8_t depth) override; using InterfaceElement::show;
	virtual void hide(uint8_t depth) override; using InterfaceElement::hide;
	virtual void enable(uint8_t depth) override; using InterfaceElement::enable;
	virtual void disable(uint8_t depth) override; using InterfaceElement::disable;
	virtual void power(uint8_t depth) override; using InterfaceElement::power;
	virtual void depower(uint8_t depth) override; using InterfaceElement::depower;

protected:
	using InterfaceElement::add;
	void add(const Move& index, const char* panelname,
		bool enabled, const char* iconname, const Paint& iconcolor,
		const std::string& text, const std::string& subtext = "");

	bool hoverValid() const { return _hoverValid; }
	Move hoverIndex() const { return _hoverIndex; }

	virtual std::unique_ptr<InterfaceElement> createPanel(
		const Position& position,
		const Move& index, const char* panelname,
		const char* iconname, const Paint& iconcolor,
		const std::string& text, const std::string& subtext) = 0;

	static const char* spritenamePanel(const Move& index);

public:
	virtual void refresh() override;

	virtual bool overed() const override;
	virtual bool hovered() const override;

	bool moved() const;
};
