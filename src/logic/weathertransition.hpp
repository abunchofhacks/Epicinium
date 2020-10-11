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

#include "cell.hpp"

class Bible;
class Board;
class ChangeSet;
enum class Season : uint8_t;


class WeatherTransition
{
public:
	WeatherTransition(const Bible& bible, Board& board, ChangeSet& changeset,
			const Season& season);

private:
	const Bible& _bible;
	Board& _board;
	ChangeSet& _changeset;

	const Season _season;
	int _totalchaos;

	bool _emissionbased;
	int _emissions;

	std::vector<Cell> _randomizedAridification;

	std::vector<uint8_t> _gain;
	std::vector<uint8_t> _loss;

	void map(Cell index);
	void reduce(Cell index);

public:
	void execute();
};
