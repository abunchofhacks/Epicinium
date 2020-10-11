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
#include "cursor.hpp"
#include "source.hpp"

#include "typenamer.hpp"
#include "sprite.hpp"
#include "collector.hpp"
#include "input.hpp"
#include "surface.hpp"
#include "board.hpp"
#include "paint.hpp"
#include "colorname.hpp"


Cursor::Cursor(const Board& board, const TypeNamer& typenamer) :
	_board(board),
	_typenamer(typenamer),
	_cell(Cell::undefined()),
	_state(State::BUSY),
	_hoversprite(new Sprite("ui/cursor_hover")),
	_busysprite(new Sprite("ui/cursor_busy")),
	_waitingsprite(new Sprite("ui/cursor_waiting")),
	_readingsprite(new Sprite("ui/cursor_reading")),
	_outofboundssprite(new Sprite("ui/cursor_outofbounds"))
{
	_hoversprite->setColor(0, Paint(ColorName::CURSORACTIVE));
	_hoversprite->setOriginAtBase();
	_busysprite->setColor(0, Paint(ColorName::CURSORBUSY));
	_busysprite->setOriginAtCenter();
	_waitingsprite->setColor(0, Paint(ColorName::CURSORWAITING));
	_waitingsprite->setOriginAtCenter();
	_waitingsprite->setOffset(1, 0);
	_readingsprite->setColor(0, Paint(ColorName::CURSORACTIVE));
	_readingsprite->setOriginAtCenter();
	_outofboundssprite->setColor(0, Paint(ColorName::CURSORINVALID));
	_outofboundssprite->setOriginAtCenter();
}

Cursor::~Cursor() = default;

void Cursor::update()
{
	switch (_state)
	{
		case State::ACTIVE:
		{
			if (_cell.valid())
			{
				Point point = Surface::convertOrigin(_cell.pos());
				Collector::get()->addHovercursor(_hoversprite, point);
			}
			else
			{
				Collector::get()->addBusycursor(_outofboundssprite,
					Input::get()->mousePixel());
			}
		}
		break;
		case State::BUSY:
		{
			_busysprite->update();
			Collector::get()->addBusycursor(_busysprite,
				Input::get()->mousePixel());
		}
		break;
		case State::WAITING:
		{
			_waitingsprite->update();
			Collector::get()->addBusycursor(_waitingsprite,
				Input::get()->mousePixel());
		}
		break;
		case State::READING:
		{
			_readingsprite->update();
			Collector::get()->addBusycursor(_readingsprite,
				Input::get()->mousePixel());
		}
		break;
	}



	/* IMGUI */
	static bool show = false;
	bool wasshown = show;

	if (Input::get()->isDebugKeyHeld())
	{
		if (ImGui::Begin("Windows", nullptr,
			ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse))
		{
			ImGui::Checkbox("Square", &show);
		}
		ImGui::End();
	}

	if (show)
	{
		if (!wasshown) ImGui::SetNextWindowCollapsed(false);

		if (ImGui::Begin("Square", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			if (_cell.valid())
			{
				std::stringstream pos;
				pos << _cell.pos();
				ImGui::Text("%s", pos.str().c_str());
				ImGui::Text("Temperature: %i degrees celcius",
					_board.temperature(_cell));
				ImGui::Text("Humidity: %i percent", _board.humidity(_cell));
				ImGui::Text("Chaos: %i", _board.chaos(_cell));
				if (_board.gas(_cell))
				{
					ImGui::Text("[!] Gas: %u", _board.gas(_cell));
				}
				if (_board.radiation(_cell))
				{
					ImGui::Text("[!] Radiation: %u", _board.radiation(_cell));
				}
				if (_board.snow(_cell))        ImGui::Text("[!] Snow");
				if (_board.frostbite(_cell))   ImGui::Text("[!] Frostbite");
				if (_board.firestorm(_cell))   ImGui::Text("[!] Firestorm");
				if (_board.bonedrought(_cell)) ImGui::Text("[!] Bonedrought");
				if (_board.death(_cell))       ImGui::Text("[!] Death");
				ImGui::Separator();
				if (_board.tile(_cell))
				{
					std::stringstream tile;
					tile << TypeEncoder(&_typenamer);
					tile << _board.tile(_cell);
					ImGui::Text("%s", tile.str().c_str());
				}
				if (_board.ground(_cell))
				{
					ImGui::Separator();
					std::stringstream unit;
					unit << TypeEncoder(&_typenamer);
					unit << _board.ground(_cell);
					ImGui::Text("%s", unit.str().c_str());
				}
				if (_board.air(_cell))
				{
					ImGui::Separator();
					std::stringstream unit;
					unit << TypeEncoder(&_typenamer);
					unit << _board.air(_cell);
					ImGui::Text("%s", unit.str().c_str());
				}
				if (_board.bypass(_cell))
				{
					ImGui::Separator();
					std::stringstream unit;
					unit << TypeEncoder(&_typenamer);
					unit << _board.bypass(_cell);
					ImGui::Text("%s", unit.str().c_str());
				}
			}
		}
		ImGui::End();
	}
}
