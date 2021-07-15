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
#include "level.hpp"
#include "source.hpp"

#include "surface.hpp"
#include "change.hpp"
#include "animationgroup.hpp"
#include "bible.hpp"
#include "skinner.hpp"
#include "cycle.hpp"
#include "treetype.hpp"
#include "randomizer.hpp"
#include "treetype.hpp"


Level::Level(const Bible& bible, const Skinner& skinner) :
	_bible(bible),
	_skinner(skinner),
	_year(0),
	_season(Season::SPRING),
	_daytime(Daytime::LATE),
	_phase(Phase::GROWTH)
{
	resize(20, 13);
}

void Level::clear(int cols, int rows)
{
	resize(cols, rows);
}

void Level::resize(int cols, int rows)
{
	DEBUG_ASSERT(cols <= Position::MAX_COLS && rows <= Position::MAX_ROWS);
	if (cols > Position::MAX_COLS)
	{
		cols = Position::MAX_COLS;
	}
	if (rows > Position::MAX_ROWS)
	{
		rows = Position::MAX_ROWS;
	}

	_cells.clear();

	_cols = cols;
	_rows = rows;

	_cells.reserve(_rows * _cols + 1);
	Square* _grid = _cells.data();
	for (int r = 0; r < _rows; r++)
	{
		for (int c = 0; c < _cols; c++)
		{
			_cells.emplace_back(_grid, _rows, _cols, _bible, _skinner, r, c);
		}
	}
	_cells.emplace_back(_grid, _rows, _cols, _bible, _skinner, -1, -1);
}

void Level::generateForests()
{
	TileType foresttype = _bible.tiletype("forest");
	if (foresttype == TileType::NONE)
	{
		LOGE << "No forest type";
		DEBUG_ASSERT(false);
		return;
	}

	for (Square& square : _cells)
	{
		square.tile().unsetTreeType();
	}

	Randomizer<Square*> _seedqueue;
	for (int r = 0; r < _rows; r += 1)
	{
		for (int c = 0; c < _cols; c += 1)
		{
			Square& square = at(r, c);
			if (square.tile().type != foresttype) continue;

			_seedqueue.push(&square);
		}
	}
	while (_seedqueue.count() > 10)
	{
		_seedqueue.pop();
	}

	Randomizer<Square*> _queue;
	Randomizer<Square*> _deferqueue;
	while (!_seedqueue.empty())
	{
		Square* from = _seedqueue.pop();

		from->tile().pickTreeType();

		for (Cell index : area(from->position(), 1, 1))
		{
			Square* to = cell(index.pos());

			if (to->tile().hasTreeType()) continue;

			to->tile().setTreeType(from->tile().getTreeType());
			if (to->tile().type == foresttype) _queue.push(to);
			else _deferqueue.push(to);
		}
	}

	while (!_queue.empty() || !_deferqueue.empty())
	{
		Square* from = (!_queue.empty()) ? _queue.pop() : _deferqueue.pop();

		for (Cell index : area(from->position(), 1, 1))
		{
			Square* to = cell(index.pos());

			if (to->tile().hasTreeType()) continue;

			to->tile().setTreeType(from->tile().getTreeType());
			if (to->tile().type == foresttype) _queue.push(to);
			else _deferqueue.push(to);
		}
	}

	for (Square& square : _cells)
	{
		square.tile().fixTreeType();
	}
}

void Level::generateBorders()
{
	static constexpr int thickness = 5;

	std::vector<TileType> types;
	std::vector<TreeType> treetypes;
	int nrows =  _rows + thickness + thickness + 2;
	int rowlen = _cols + thickness + thickness + 2;
	types.resize(nrows * rowlen, TileType::NONE);
	treetypes.resize(nrows * rowlen, TreeType::UNSET);

	for (int r = thickness + 1; r < _rows + thickness + 1; r++)
	{
		for (int c = thickness + 1; c < _cols + thickness + 1; c++)
		{
			Square& square = at(r - thickness - 1, c - thickness - 1);
			types[r * rowlen + c] = square.tile().type;
			treetypes[r * rowlen + c] = square.tile().getTreeType();
		}
	}
	for (int d = 0; d < thickness; d++)
	{
		{
			int r = thickness - d;
			for (int c = thickness + 1 - d; c < _cols + thickness + 1 + d; c++)
			{
				if (d > 0 && (rand() % (d + 2)) > 1) continue;
				types[r * rowlen + c] = types[(r + 1) * rowlen + c];
				treetypes[r * rowlen + c] = treetypes[(r + 1) * rowlen + c];
			}
		}
		{
			int c = thickness - d;
			for (int r = thickness - d; r < _rows + thickness + 1 + d; r++)
			{
				if (d > 0 && (rand() % (d + 2)) > 1) continue;
				types[r * rowlen + c] = types[r * rowlen + c + 1];
				treetypes[r * rowlen + c] = treetypes[r * rowlen + c + 1];
			}
		}
		{
			int c = _cols + thickness + 1 + d;
			for (int r = thickness - d; r < _rows + thickness + 1 + d; r++)
			{
				if (d > 0 && (rand() % (d + 2)) > 1) continue;
				types[r * rowlen + c] = types[r * rowlen + c - 1];
				treetypes[r * rowlen + c] = treetypes[r * rowlen + c - 1];
			}
		}
		{
			int r = _rows + thickness + 1 + d;
			for (int c = thickness - d; c < _cols + thickness + 2 + d; c++)
			{
				if (d > 0 && (rand() % (d + 2)) > 1) continue;
				types[r * rowlen + c] = types[(r - 1) * rowlen + c];
				treetypes[r * rowlen + c] = treetypes[(r - 1) * rowlen + c];
			}
		}
	}

	size_t count = 2 * (_rows + thickness + _cols + thickness) * thickness;

	if (count != _borders.size())
	{
		_borders.clear();
		_borders.reserve(count);

		for (int r = -thickness; r < 0; r++)
		{
			for (int c = -thickness; c < _cols + thickness; c++)
			{
				_borders.emplace_back(Position(r, c),
					_bible, _skinner, borderDistance(r, c));
			}
		}
		for (int r = 0; r < _rows; r++)
		{
			for (int c = -thickness; c < 0; c++)
			{
				_borders.emplace_back(Position(r, c),
					_bible, _skinner, borderDistance(r, c));
			}
			for (int c = _cols; c < _cols + thickness; c++)
			{
				_borders.emplace_back(Position(r, c),
					_bible, _skinner, borderDistance(r, c));
			}
		}
		for (int r = _rows; r < _rows + thickness; r++)
		{
			for (int c = -thickness; c < _cols + thickness; c++)
			{
				_borders.emplace_back(Position(r, c),
					_bible, _skinner, borderDistance(r, c));
			}
		}
	}

	for (Border& border : _borders)
	{
		populateBorder(border, thickness, rowlen, types, treetypes);
	}

	_borders.front().setAsBackground();
}

int Level::borderDistance(int r, int c)
{
	return std::max(std::max(-c, c - _cols + 1), std::max(-r, r - _rows + 1));
}

void Level::populateBorder(Border& border, int thickness, int rowlen,
	const std::vector<TileType>& types, const std::vector<TreeType>& treetypes)
{
	int r = border.position().row;
	int c = border.position().col;
	int rr = r + thickness + 1;
	int cc = c + thickness + 1;
	border.set(
		types[ rr      * rowlen + cc    ],
		types[ rr      * rowlen + cc + 1],
		types[(rr + 1) * rowlen + cc    ],
		types[ rr      * rowlen + cc - 1],
		types[(rr - 1) * rowlen + cc    ],
		treetypes[rr   * rowlen + cc]);
	border.populate();
}

void Level::enactInSubject(const Change& change, std::shared_ptr<AnimationGroup> group)
{
	Square& square = at(change.subject.position);
	switch (change.subject.type)
	{
		case Descriptor::Type::CELL:
		case Descriptor::Type::TILE:
		{
			square.tile().enact(change, group);
		}
		break;

		case Descriptor::Type::GROUND:
		case Descriptor::Type::AIR:
		case Descriptor::Type::BYPASS:
		{
			square.unit(change.subject.type).enact(change, group);
		}
		break;

		case Descriptor::Type::NONE:
		{
			LOGE << "cannot enact change without subject: "
				<< TypeEncoder(&_bible) << change;
		}
		break;
	}
}

Point Level::centerPoint()
{
	return Point(_cols * Surface::WIDTH / 2, _rows * Surface::HEIGHT / 2);
}

Point Level::topleftPoint()
{
	return Point(0, 0);
}

Point Level::bottomrightPoint()
{
	return Point(_cols * Surface::WIDTH, _rows * Surface::HEIGHT);
}

void Level::update()
{
	for (Square& square : _cells)
	{
		square.update();
	}

	for (Border& border : _borders)
	{
		border.update();
	}
}

void Level::setLight(float light, float duration, float delay)
{
	for (Square& square : _cells)
	{
		square.setLight(light, duration, delay);
	}

	for (Border& border : _borders)
	{
		border.setLight(light, duration, delay);
	}
}

void Level::setBorderLight(float light, float duration, float delay)
{
	for (Border& border : _borders)
	{
		border.setLight(light, duration, delay);
	}
}

void Level::setBorderSpotlight(float light, float duration, float delay)
{
	for (Border& border : _borders)
	{
		border.setSpotlight(light, duration, delay);
	}
}

void Level::enact(const Change& change, std::shared_ptr<AnimationGroup> group)
{
	switch (change.type)
	{
		case Change::Type::NONE:
		{
			enactInSubject(change, group);
		}
		break;

		case Change::Type::STARTS:
		{
			// Pre-move for attack of opportunity. Only do this if necessary,
			// i.e. not while moving as an air unit or bypassing another unit.
			if (change.subject.type == Descriptor::Type::GROUND)
			{
				Square& square = at(change.subject.position);
				square.unit(Descriptor::Type::GROUND).enact(change, group);
			}
		}
		break;

		case Change::Type::MOVES:
		{
			Square& from = at(change.subject.position);
			Square& to   = at(change.target.position);
			// Exchange pointers to move the unit without actually std::move'ing the unit data.
			std::swap(from.unit(change.subject.type), to.unit(change.target.type));
			from.unit(change.subject.type).resetSquare(&from);
			to.unit(change.target.type).resetSquare(&to);
			// Start a move animation.
			to.unit(change.target.type).enact(change, group);
		}
		break;

		case Change::Type::REVEAL:
		{
			Square& square = at(change.subject.position);
			square.vision().add(Player::SELF);
			// Update the markers and counters.
			square.snow() = change.snow;
			square.frostbite() = change.frostbite;
			square.coldfeet() = change.frostbite && group && group->coldfeet;
			square.firestorm() = change.firestorm;
			square.bonedrought() = change.bonedrought;
			square.death() = change.death;
			square.gas() = change.gas;
			square.radiation() = change.radiation;
			square.temperature() = change.temperature;
			square.humidity() = change.humidity;
			square.chaos() = change.chaos;
			// Enact it.
			square.tile().enact(change, group);
			// Also animate it for Chilled ground units.
			square.ground().enact(change, group);
		}
		break;

		case Change::Type::OBSCURE:
		{
			Square& square = at(change.subject.position);
			square.vision().remove(Player::SELF);
			// Enact it.
			square.tile().enact(change, group);
		}
		break;

		case Change::Type::TRANSFORMED:
		case Change::Type::CONSUMED:
		case Change::Type::SHAPES:
		case Change::Type::SHAPED:
		case Change::Type::SETTLES:
		case Change::Type::SETTLED:
		case Change::Type::EXPANDS:
		case Change::Type::EXPANDED:
		case Change::Type::UPGRADES:
		case Change::Type::UPGRADED:
		case Change::Type::CULTIVATES:
		case Change::Type::CULTIVATED:
		case Change::Type::CAPTURES:
		case Change::Type::CAPTURED:
		case Change::Type::PRODUCES:
		case Change::Type::PRODUCED:
		case Change::Type::EXITED:
		case Change::Type::DIED:
		case Change::Type::DESTROYED:
		case Change::Type::SURVIVED:
		case Change::Type::AIMS:
		case Change::Type::ATTACKS:
		case Change::Type::ATTACKED:
		case Change::Type::TRAMPLES:
		case Change::Type::TRAMPLED:
		case Change::Type::SHELLS:
		case Change::Type::SHELLED:
		case Change::Type::BOMBARDS:
		case Change::Type::BOMBARDED:
		case Change::Type::BOMBS:
		case Change::Type::BOMBED:
		case Change::Type::FROSTBITTEN:
		case Change::Type::BURNED:
		case Change::Type::GASSED:
		case Change::Type::IRRADIATED:
		case Change::Type::GROWS:
		{
			enactInSubject(change, group);
		}
		break;

		case Change::Type::ENTERED:
		{
			// Because humans have object permanence, we want to keep any
			// randomized data the same when a Unit leaves and enters vision.
			// Therefore we reuse the same Unit object when it enters again.
			if (group && group->unitslot > 0)
			{
				Square& from = at(Position(group->fromrow, group->fromcol));
				Square& to = at(change.subject.position);
				// This only applies when a unit ENTERED from within
				// the fog of war on a different space.
				if (!to.current() && to.position() != from.position())
				{
					// Exchange pointers to move the unit without actually
					// std::move'ing the unit data.
					Descriptor::Type fromslot = (Descriptor::Type) group->unitslot;
					std::swap(from.unit(fromslot), to.unit(change.subject.type));
					from.unit(fromslot).resetSquare(&from);
					to.unit(change.subject.type).resetSquare(&to);
					// Set the torow and tocol so the Unit knows it has moved.
					group->torow = change.subject.position.row;
					group->tocol = change.subject.position.col;
				}
				// Continue to animate.
			}

			enactInSubject(change, group);
		}
		break;

		case Change::Type::SNOW:
		{
			int r = change.subject.position.row;
			int c = change.subject.position.col;

			// Purely for aesthetics we also update the snow in the fog of war.
			if (r >= 0 && r < _rows && c >= 0 && c < _cols
				&& r * _cols + c > 0)
			{
				// We end just before the current cell.
				size_t index = r * _cols + c;
				auto end = _cells.begin() + index;

				// Update fog of war based on a predictive change with an
				// empty destrictor that prevents the creation of particles.
				Change dummy(Change::Type::SNOW, Descriptor());
				for (auto iter = _cells.begin(); iter != end; iter++)
				{
					Square& square = *iter;
					// TODO This is hardcopied from MarkerTransition.
					bool prediction = (_bible.chaosMinSnow(_season) >= 0
						&& square.humidity() >= _bible.humidityMinSnow(_season)
						&& square.chaos() >= _bible.chaosMinSnow(_season));
					if (prediction == square.snow()) continue;
					dummy.xSnow(prediction);
					square.snow() = dummy.snow;
					square.tile().enact(dummy, group);
					square.ground().enact(dummy, group);
					square.air().enact(dummy, group);
					square.bypass().enact(dummy, group);
					if (group) group->delay += 0.005f;
				}
			}

			{
				Square& square = at(change.subject.position);
				square.snow() = change.snow;
				square.tile().enact(change, group);
				square.ground().enact(change, group);
				square.air().enact(change, group);
				square.bypass().enact(change, group);
				if (group) group->delay += 0.005f;
			}

			// Purely for aesthetics we also update the snow in the fog of war.
			if (r >= 0 && r < _rows && c >= 0 && c < _cols
				&& r * _cols + c + 1 < _rows * _cols)
			{
				// We start with the next cell.
				size_t index = r * _cols + c + 1;
				auto begin = _cells.begin() + index;

				// Update fog of war based on a predictive change with an
				// empty destrictor that prevents the creation of particles.
				Change dummy(Change::Type::SNOW, Descriptor());
				for (auto iter = begin; iter != _cells.end(); iter++)
				{
					Square& square = *iter;
					// TODO This is hardcopied from MarkerTransition.
					bool prediction = (_bible.chaosMinSnow(_season) >= 0
						&& square.humidity() >= _bible.humidityMinSnow(_season)
						&& square.chaos() >= _bible.chaosMinSnow(_season));
					if (prediction == square.snow()) continue;
					dummy.xSnow(prediction);
					square.snow() = dummy.snow;
					square.tile().enact(dummy, group);
					square.ground().enact(dummy, group);
					square.air().enact(dummy, group);
					square.bypass().enact(dummy, group);
					if (group) group->delay += 0.005f;
				}
			}
		}
		break;

		case Change::Type::FROSTBITE:
		{
			Square& square = at(change.subject.position);
			square.frostbite() = change.frostbite;
			square.coldfeet() = change.frostbite && group && group->coldfeet;
			square.tile().enact(change, group);
			square.ground().enact(change, group);
			square.air().enact(change, group);
			square.bypass().enact(change, group);
			if (group) group->delay += 0.020f;
		}
		break;

		case Change::Type::FIRESTORM:
		{
			Square& square = at(change.subject.position);
			square.firestorm() = change.firestorm;
			square.tile().enact(change, group);
			square.ground().enact(change, group);
			square.air().enact(change, group);
			square.bypass().enact(change, group);
			if (group) group->delay += 0.020f;
		}
		break;

		case Change::Type::BONEDROUGHT:
		{
			Square& square = at(change.subject.position);
			square.bonedrought() = change.bonedrought;
			square.tile().enact(change, group);
			square.ground().enact(change, group);
			square.air().enact(change, group);
			square.bypass().enact(change, group);
			if (group) group->delay += 0.020f;
		}
		break;

		case Change::Type::DEATH:
		{
			Square& square = at(change.subject.position);
			square.death() = change.death;
			square.tile().enact(change, group);
			square.ground().enact(change, group);
			square.air().enact(change, group);
			square.bypass().enact(change, group);
			if (group) group->delay += 0.020f;
		}
		break;

		case Change::Type::GAS:
		{
			Square& square = at(change.subject.position);
			square.gas() += change.gas;
			square.tile().enact(change, group);
			square.ground().enact(change, group);
			square.air().enact(change, group);
			square.bypass().enact(change, group);
			if (group) group->delay += 0.020f;
		}
		break;

		case Change::Type::RADIATION:
		{
			Square& square = at(change.subject.position);
			square.radiation() += change.radiation;
			square.tile().enact(change, group);
			square.ground().enact(change, group);
			square.air().enact(change, group);
			square.bypass().enact(change, group);
			if (group) group->delay += 0.020f;
		}
		break;

		case Change::Type::TEMPERATURE:
		{
			Square& square = at(change.subject.position);
			square.temperature() += change.temperature;
			square.tile().enact(change, group);
			square.ground().enact(change, group);
			square.air().enact(change, group);
			square.bypass().enact(change, group);
		}
		break;

		case Change::Type::HUMIDITY:
		{
			Square& square = at(change.subject.position);
			// Change it.
			square.humidity() += change.humidity;
			// Enact it.
			square.tile().enact(change, group);
			square.ground().enact(change, group);
			square.air().enact(change, group);
			square.bypass().enact(change, group);
		}
		break;

		case Change::Type::CHAOS:
		{
			Square& square = at(change.subject.position);
			// Change it.
			square.chaos() += change.chaos;
			// Enact it.
			square.tile().enact(change, group);
			square.ground().enact(change, group);
			square.air().enact(change, group);
			square.bypass().enact(change, group);
		}
		break;

		case Change::Type::VISION:
		{
			Square& square = at(change.subject.position);
			square.vision() = change.vision;
		}
		break;

		case Change::Type::CORNER:
		{
			int cols = (int) change.subject.position.col + 1;
			int rows = (int) change.subject.position.row + 1;

			resize(cols, rows);
		}
		break;

		case Change::Type::BORDER:
		{
			generateForests();
			generateBorders();

			// We just generated the forests based on the tiletypes, but the
			// tiles have already queued in their REVEAL changes. We just
			// repopulate them to get the correct type of trees to show up.
			// We also have to repopulate to make sure the Mountains are
			// placed correctly.
			// This is "fine" because we set the lighting after this change.
			// TODO Fix this ugly hack.
			// This ugly hack even relies on the resize() in the CORNER change
			// _not_ being conditional, to prevent _square.light() == 1.1.
			for (Square& square : _cells)
			{
				square.tile().populate(group);
				square.tile().cleanup();
			}
		}
		break;

		case Change::Type::INCOME:
		{
			Square& square = at(change.subject.position);
			square.tile().enact(change, group);
		}
		break;

		case Change::Type::SCORED:
		{
			Square& square = at(change.subject.position);
			square.tile().enact(change, group);
		}
		break;

		case Change::Type::CHAOSREPORT:
		break;

		case Change::Type::YEAR:
		{
			_year = change.year;
		}
		break;

		case Change::Type::SEASON:
		{
			_season = change.season;
		}
		break;

		case Change::Type::DAYTIME:
		{
			_daytime = change.daytime;
		}
		break;

		case Change::Type::PHASE:
		{
			_phase = change.phase;
		}
		break;

		case Change::Type::INITIATIVE:
		case Change::Type::FUNDS:
		case Change::Type::EXPENDITURE:
		case Change::Type::SLEEPING:
		case Change::Type::ACTING:
		case Change::Type::FINISHED:
		case Change::Type::DISCARDED:
		case Change::Type::POSTPONED:
		case Change::Type::UNFINISHED:
		case Change::Type::ORDERED:
		case Change::Type::DEFEAT:
		case Change::Type::VICTORY:
		case Change::Type::GAMEOVER:
		case Change::Type::AWARD:
		break;
	}
}
