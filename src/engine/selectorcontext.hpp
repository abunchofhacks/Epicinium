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

#include "context.hpp"
#include "cell.hpp"

class Board;
class Settings;
enum class Player : uint8_t;
struct Descriptor;
class Paint;


class SelectorContext : public Context
{
public:
	SelectorContext(const Board& board,
		const Settings& settings,
		Cell cell, const Player& player);
	SelectorContext(const SelectorContext&) = delete;
	SelectorContext(SelectorContext&&) = delete;
	SelectorContext& operator=(const SelectorContext&) = delete;
	SelectorContext& operator=(SelectorContext&&) = delete;
	virtual ~SelectorContext() = default;

private:
	const Board& _board;
	const Settings& _settings;
	const Cell _cell;
	bool _tile;
	bool _ground;
	bool _air;

	virtual std::unique_ptr<InterfaceElement> createPanel(
		const Position& position,
		const Move& index, const char* panelname,
		const char* iconname, const Paint& iconcolor,
		const std::string& text, const std::string& subtext) override;

public:
	bool empty() const;
	bool trivial() const;
	Descriptor resolve() const;
};
