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
#include "context.hpp"
#include "source.hpp"

#include "input.hpp"
#include "loop.hpp"
#include "move.hpp"
#include "surface.hpp"
#include "camera.hpp"
#include "aim.hpp"


Context::Context(const Position& position) :
	_position(position),
	_hoverValid(false),
	_hoverIndex(Move::X),
	_overIndex(Move::X),
	_initialMousePoint(Input::get()->mousePoint()),
	_delay(Input::get()->longPressThreshold())
{}

Context::~Context() = default;

void Context::debugtree(uint8_t depth)
{
	std::string spaces(depth, '.');
	std::cout << spaces << "{" << std::endl;
	for (const auto& kv : _panels)
	{
		std::cout << spaces << kv.first << ":" << std::endl;
		InterfaceElement* panel = kv.second.get();
		panel->debugtree(depth + 1);
	}
	std::cout << spaces << "}" << std::endl;
}

void Context::setWidth(int)
{
	LOGW << "Trying to set width of a Context.";
	DEBUG_ASSERT(false);
}

void Context::setHeight(int)
{
	LOGW << "Trying to set height of a Context.";
	DEBUG_ASSERT(false);
}

void Context::place(const Pixel&)
{
	LOGW << "Trying to place a Context.";
	DEBUG_ASSERT(false);
}

void Context::refresh()
{
	_hoverValid = false;
	for (const auto& kv : _panels)
	{
		InterfaceElement* panel = kv.second.get();
		if (panel->enabled() && panel->hovered())
		{
			_hoverIndex = kv.first;
			_hoverValid = true;
			_overIndex = kv.first;
		}
		else if (panel->overed())
		{
			_overIndex = kv.first;
		}
	}

	if (_delay > 0)
	{
		_delay -= Loop::delta();
		if (_delay <= 0) show();
		else if (shown()) hide();
	}

	for (const auto& kv : _panels)
	{
		Move index = kv.first;
		Position panelpos = _position + Aim(index);
		Point pointTopleft = Surface::convertTopleft(panelpos);
		pointTopleft.xenon -= Surface::WIDTH / 2;
		pointTopleft.yahoo -= Surface::HEIGHT / 2;
		switch (index)
		{
			case Move::X:                          break;
			case Move::E: pointTopleft.xenon -= 2; break;
			case Move::S: pointTopleft.yahoo -= 2; break;
			case Move::W: pointTopleft.xenon += 2; break;
			case Move::N: pointTopleft.yahoo += 2; break;
		}
		Pixel pixelTopleft = Camera::get()->convert(pointTopleft, Layer::CONTEXT);

		InterfaceElement* panel = kv.second.get();
		panel->place(pixelTopleft);
		panel->refresh();
	}
}

void Context::add(
	const Move& index, const char* panelname,
	bool enabled, const char* iconname, const Paint& iconcolor,
	const std::string& text, const std::string& subtext)
{
	_panels[index] = createPanel(_position, index, panelname,
		iconname, iconcolor, text, subtext);

	if (enabled) _panels[index]->enable();
	else _panels[index]->disable();

	if (enabled && index == Move::X)
	{
		_hoverValid = true;
		_hoverIndex = index;
		_overIndex = index;
	}
}

const char* Context::spritenamePanel(const Move& index)
{
	switch (index)
	{
		case Move::X: return "ui/radial_mid";
		case Move::E: return "ui/radial_east";
		case Move::S: return "ui/radial_south";
		case Move::W: return "ui/radial_west";
		case Move::N: return "ui/radial_north";
	}
	return nullptr;
}

void Context::bear(uint8_t depth)
{
	InterfaceElement::bear(depth);

	for (const auto& kv : _panels)
	{
		InterfaceElement* panel = kv.second.get();
		panel->bear(depth + 1);
	}
}

void Context::kill(uint8_t depth)
{
	InterfaceElement::kill(depth);

	for (const auto& kv : _panels)
	{
		InterfaceElement* panel = kv.second.get();
		panel->kill(depth + 1);
	}
}

void Context::show(uint8_t depth)
{
	InterfaceElement::show(depth);

	for (const auto& kv : _panels)
	{
		InterfaceElement* panel = kv.second.get();
		panel->show(depth + 1);
	}
}

void Context::hide(uint8_t depth)
{
	InterfaceElement::hide(depth);

	for (const auto& kv : _panels)
	{
		InterfaceElement* panel = kv.second.get();
		panel->hide(depth + 1);
	}
}

void Context::enable(uint8_t depth)
{
	InterfaceElement::enable(depth);

	for (const auto& kv : _panels)
	{
		InterfaceElement* panel = kv.second.get();
		panel->enable(depth + 1);
	}
}

void Context::disable(uint8_t depth)
{
	InterfaceElement::disable(depth);

	for (const auto& kv : _panels)
	{
		InterfaceElement* panel = kv.second.get();
		panel->disable(depth + 1);
	}
}

void Context::power(uint8_t depth)
{
	InterfaceElement::power(depth);

	for (const auto& kv : _panels)
	{
		InterfaceElement* panel = kv.second.get();
		panel->power(depth + 1);
	}
}

void Context::depower(uint8_t depth)
{
	InterfaceElement::depower(depth);

	for (const auto& kv : _panels)
	{
		InterfaceElement* panel = kv.second.get();
		panel->depower(depth + 1);
	}
}

bool Context::overed() const
{
	for (auto& kv : _panels)
	{
		InterfaceElement* panel = kv.second.get();
		if (panel->overed()) return true;
	}

	return false;
}

bool Context::hovered() const
{
	// Because some items might occlude others, we must know which layer the mouse operates in.
	// We therefore need to check all items to see if they are hovered.
	bool result = false;

	for (auto& kv : _panels)
	{
		InterfaceElement* panel = kv.second.get();
		if (panel->hovered()) result = true;
	}

	return result;
}

bool Context::moved() const
{
	if (!_hoverValid || _hoverIndex == Move::X) return false;

	float distance = _initialMousePoint.distanceTo(Input::get()->mousePoint());

	return (distance >= Input::get()->significantMoveDistance());
}
