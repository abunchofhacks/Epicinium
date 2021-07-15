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
#include "aineuralnewt.hpp"
#include "source.hpp"

#include "difficulty.hpp"
#include "aim.hpp"
#include "bible.hpp"

 // windows.h is being annoying
#undef near


std::string AINeuralNewt::ainame() const
{
	return "NeuralNewt";
}

std::string AINeuralNewt::authors() const
{
	return "Sander in 't Veld, Daan Mulder";
}

AINeuralNewt::AINeuralNewt(const Player& player, const Difficulty& difficulty,
		const std::string& rulesetname, char character,
		std::shared_ptr<NewtBrain> brain) :
	AICommander(player, difficulty, rulesetname, character),
	_ownBaseFloodfill(_bible, _board),
	_enemyBaseFloodfill(_bible, _board),
	_brain(std::move(brain))
{
	if (_difficulty == Difficulty::NONE)
	{
		LOGW << "AI difficulty not set";
	}

	_pathingFloodfillCache.reserve(30);
}

AINeuralNewt::AINeuralNewt(const Player& player, const Difficulty& difficulty,
		const std::string& rulesetname, char character) :
	AINeuralNewt(player, difficulty, rulesetname, character,
		std::make_shared<DummyNewtBrain>())
{}

void AINeuralNewt::preprocess()
{
	_ownBaseFloodfill.reset();
	_ownBaseFloodfill.include({_player});
	_ownBaseFloodfill.execute();

	_enemyBaseFloodfill.reset();
	_enemyBaseFloodfill.exclude({_player, Player::NONE});
	_enemyBaseFloodfill.execute();

	_pathingFloodfillUses.clear();

	count();

	sortSubjects(_settlers);
	sortSubjects(_defenses);
	sortSubjects(_offenses);
	sortSubjects(_captors);
	sortSubjects(_blockers);
	sortSubjects(_bombarders);
	sortSubjects(_creators);
	sortSubjects(_expanders);
	sortSubjects(_cultivators);
	sortSubjects(_upgraders);
	sortSubjects(_stoppers);
}

void AINeuralNewt::process()
{
	_brain->prepare(*this);
}

bool AINeuralNewt::postprocess()
{
	_evaluation = _brain->evaluate();
	LOGV << _player << " evaluation #" << (_newOrders.size() + 1)
		<< ":\n" << _evaluation.toPrettyString();

	Order order;
	int cost = 0;
	Descriptor target;
	float score = 0;

	for (Descriptor desc : _settlers)
	{
		controlSettler(desc, order, cost, target, score);
	}

	for (Descriptor desc : _defenses)
	{
		controlCombatant(desc, order, cost, target, score);
		moveDefense(desc, order, cost, target, score);
	}

	for (Descriptor desc : _offenses)
	{
		controlCombatant(desc, order, cost, target, score);
	}

	for (Descriptor desc : _captors)
	{
		controlCaptor(desc, order, cost, target, score);
	}

	for (Descriptor desc : _blockers)
	{
		controlCombatant(desc, order, cost, target, score);
		moveDefense(desc, order, cost, target, score);
	}

	for (Descriptor desc : _bombarders)
	{
		controlBombarder(desc, order, cost, target, score);
	}

	for (Descriptor desc : _creators)
	{
		controlCreator(desc, order, cost, target, score);
	}

	for (Descriptor desc : _expanders)
	{
		controlExpander(desc, order, cost, target, score);
	}

	for (Descriptor desc : _cultivators)
	{
		controlCultivator(desc, order, cost, target, score);
	}

	for (Descriptor desc : _upgraders)
	{
		controlUpgrader(desc, order, cost, target, score);
	}

	for (Descriptor desc : _stoppers)
	{
		controlStopper(desc, order, cost, target, score);
	}

	if (score > 0)
	{
		LOGV << _player << " order #" << (_newOrders.size() + 1)
			<< " scores " << std::fixed << std::setprecision(2) << score
			<< " and is " << TypeEncoder(&_bible) << order;
		_newOrders.push_back(order);
		_moneyleftover -= cost;
		if (target.type != Descriptor::Type::NONE)
		{
			_targets.emplace_back(target.position);
		}
	}
	else return true;

	filterSubjects(_settlers);
	filterSubjects(_defenses);
	filterSubjects(_offenses);
	filterSubjects(_captors);
	filterSubjects(_blockers);
	filterSubjects(_bombarders);
	filterSubjects(_creators);
	filterSubjects(_expanders);
	filterSubjects(_cultivators);
	filterSubjects(_upgraders);
	filterSubjects(_stoppers);

	return (_newOrders.size() >= _bible.newOrderLimit());
}

void AINeuralNewt::count()
{
	_settlers.clear();
	_defenses.clear();
	_offenses.clear();
	_captors.clear();
	_blockers.clear();
	_bombarders.clear();
	_creators.clear();
	_expanders.clear();
	_cultivators.clear();
	_upgraders.clear();
	_stoppers.clear();
	_targets.clear();

	_moneyleftover = _money;

	for (const Order& order : _unfinishedOrders)
	{
		lookAtUnfinishedOrder(order);
	}

	for (Cell index : _board)
	{
		if (_board.tile(index).owner == _player)
		{
			const std::vector<Bible::UnitBuild>& prods = _bible.tileProduces(
				_board.tile(index).type);
			const std::vector<Bible::TileBuild>& expos = _bible.tileExpands(
				_board.tile(index).type);
			const std::vector<Bible::TileBuild>& upgrs = _bible.tileUpgrades(
				_board.tile(index).type);
			const std::vector<Bible::TileBuild>& cults = _bible.tileCultivates(
				_board.tile(index).type);
			const Descriptor tiledesc = Descriptor::tile(index.pos());

			const int power = _bible.powerAbolished()
				? _board.tile(index).stacks
				: _board.tile(index).power;

			if (!prods.empty() && power > 0)
			{
				_creators.emplace_back(tiledesc);
			}

			if (!upgrs.empty() && power > 0)
			{
				_upgraders.emplace_back(tiledesc);
			}

			if (!expos.empty() && power > 0)
			{
				for (const Move& move : {Move::E, Move::S, Move::W, Move::N})
				{
					Cell target = index + move;
					if (target.edge()) continue;

					if (_bible.tileBuildable(_board.tile(target).type))
					{
						_expanders.emplace_back(tiledesc);
						break;
					}
				}
			}

			if (!cults.empty() && power > 0 && expectedSoil(index) > 0)
			{
				_cultivators.emplace_back(tiledesc);
			}
		}

		if (_board.ground(index).owner == _player)
		{
			const Descriptor unitdesc = Descriptor::ground(index.pos());
			const UnitType unittype = _board.ground(index).type;

			if (!_bible.unitSettles(unittype).empty()
				&& (!_bible.unitCanAttack(unittype)
					|| _board.ground(index).stacks == 1))
			{
				_settlers.emplace_back(unitdesc);
			}

			if (_bible.unitCanAttack(unittype)
				|| _bible.unitAbilityVolleys(unittype) > 0)
			{
				bool offense = false;
				for (const Order& order : _unfinishedOrders)
				{
					if (order.subject == unitdesc
						&& order.type == Order::Type::MOVE)
					{
						int speed = _bible.unitSpeed(unittype);
						if (_board.snow(index)) speed = std::max(1, speed - 1);

						if (std::max(0, (int) order.moves.size()) <= speed)
						{
							_targets.emplace_back(order.target.position);
						}

						Cell target = _board.cell(order.target.position);
						Cell next = index + order.moves[0];
						if (_board.ground(next)
							&& _board.ground(next).owner == _player)
						{
							// The unit might be blocked, so give it new orders.
						}
						else if (!_bible.tileOwnable(_board.tile(target).type)
							|| _board.tile(target).owner == _player)
						{
							// The unit is moving towards a tile that has
							// already been captured or destroyed,
							// so give it new orders.
						}
						else
						{
							offense = true;
						}
						break;
					}
				}
				if (offense) _offenses.emplace_back(unitdesc);
				else _defenses.emplace_back(unitdesc);
			}
			else
			{
				for (const Order& order : _unfinishedOrders)
				{
					if (order.subject == unitdesc
						&& order.type == Order::Type::MOVE)
					{
						int speed = _bible.unitSpeed(unittype);
						if (_board.snow(index)) speed = std::max(1, speed - 1);

						if (std::max(0, (int) order.moves.size()) <= speed)
						{
							_targets.emplace_back(order.target.position);
						}
						else if (order.moves.size() > 0)
						{
							Cell at = _board.cell(order.subject.position);
							Cell next = at + order.moves[0];
							_targets.emplace_back(next.pos());
						}
					}
				}
			}

			if (_board.tile(index)
				&& _board.tile(index).owner != _player
				&& _bible.tileOwnable(_board.tile(index).type))
			{
				if (_bible.unitCanCapture(unittype))
				{
					_captors.emplace_back(unitdesc);
				}
				else if (!_bible.unitCanCapture(unittype)
					&& !_bible.unitCanOccupy(unittype))
				{
					_blockers.emplace_back(unitdesc);
				}
			}

			if (_bible.unitCanBombard(unittype))
			{
				_bombarders.emplace_back(unitdesc);
			}
		}
	}

	for (const Position& targetposition : _targets)
	{
		Descriptor unitdesc = Descriptor::ground(targetposition);
		if (_board.ground(_board.cell(targetposition)).owner == _player
			&& !hasOldOrder(unitdesc) && !hasNewOrder(unitdesc))
		{
			_blockers.emplace_back(unitdesc);
		}
	}
}

void AINeuralNewt::lookAtUnfinishedOrder(const Order& order)
{
	switch (order.type)
	{
		case Order::Type::EXPAND:
		{
			if (isOccupied(order.subject.position))
			{
				// If possible, give a stop order to prevent being softlocked.
				_stoppers.emplace_back(order.subject);

				// Do not count it towards money leftover.
				break;
			}

			Cell from = _board.cell(order.subject.position);
			TileType fromtype = _board.tile(from).type;
			for (const Bible::TileBuild& build : _bible.tileExpands(fromtype))
			{
				if (build.type == order.tiletype)
				{
					_moneyleftover -= build.cost;
					break;
				}
			}
		}
		break;

		case Order::Type::UPGRADE:
		{
			if (isOccupied(order.subject.position))
			{
				// If possible, give a stop order to prevent being softlocked.
				_stoppers.emplace_back(order.subject);

				// Do not count it towards money leftover.
				break;
			}

			Cell from = _board.cell(order.subject.position);
			TileType fromtype = _board.tile(from).type;
			for (const Bible::TileBuild& build : _bible.tileUpgrades(fromtype))
			{
				if (build.type == order.tiletype)
				{
					_moneyleftover -= build.cost;
					break;
				}
			}
		}
		break;

		case Order::Type::CULTIVATE:
		{
			if (isOccupied(order.subject.position))
			{
				// If possible, give a stop order to prevent being softlocked.
				_stoppers.emplace_back(order.subject);
			}
		}
		break;

		case Order::Type::PRODUCE:
		{
			if (isOccupied(order.subject.position))
			{
				// If possible, give a stop order to prevent being softlocked.
				_stoppers.emplace_back(order.subject);

				// Do not count it towards money leftover.
				break;
			}

			Cell from = _board.cell(order.subject.position);
			TileType fromtype = _board.tile(from).type;
			for (const Bible::UnitBuild& build : _bible.tileProduces(fromtype))
			{
				if (build.type == order.unittype)
				{
					_moneyleftover -= build.cost;
					break;
				}
			}
		}
		break;

		case Order::Type::SHAPE:
		{
			Cell from = _board.cell(order.subject.position);
			UnitType fromtype = _board.unit(from, order.subject.type).type;
			for (const Bible::TileBuild& build : _bible.unitShapes(fromtype))
			{
				if (build.type == order.tiletype)
				{
					_moneyleftover -= build.cost;
					break;
				}
			}
		}
		break;

		case Order::Type::SETTLE:
		{
			Cell from = _board.cell(order.subject.position);
			UnitType fromtype = _board.unit(from, order.subject.type).type;
			for (const Bible::TileBuild& build : _bible.unitSettles(fromtype))
			{
				if (build.type == order.tiletype)
				{
					_moneyleftover -= build.cost;
					break;
				}
			}
		}
		break;

		case Order::Type::NONE:
		case Order::Type::MOVE:
		case Order::Type::GUARD:
		case Order::Type::FOCUS:
		case Order::Type::LOCKDOWN:
		case Order::Type::SHELL:
		case Order::Type::BOMBARD:
		case Order::Type::BOMB:
		case Order::Type::CAPTURE:
		case Order::Type::HALT:
		break;
	}
}

PathingFloodfill& AINeuralNewt::getExecutedPathingFloodfill(Cell from,
	const Descriptor& descriptor)
{
	DEBUG_ASSERT(from.pos() == descriptor.position);

	for (size_t i = 0; i < _pathingFloodfillUses.size(); i++)
	{
		if (_pathingFloodfillUses[i] == descriptor)
		{
			return _pathingFloodfillCache[i];
		}
	}

	size_t i;
	if (_pathingFloodfillUses.size() < _pathingFloodfillCache.size())
	{
		i = _pathingFloodfillUses.size();
		_pathingFloodfillUses.push_back(descriptor);
		_pathingFloodfillCache[i].reset();
	}
	else if (_pathingFloodfillCache.size() < _pathingFloodfillCache.capacity())
	{
		i = _pathingFloodfillCache.size();
		_pathingFloodfillUses.push_back(descriptor);
		_pathingFloodfillCache.emplace_back(_bible, _board);
	}
	else
	{
		LOGV << "_pathingFloodfillCache full";
		i = _pathingFloodfillCache.size() - 1;
		_pathingFloodfillUses.back() = descriptor;
		_pathingFloodfillCache.back().reset();
	}
	PathingFloodfill& pathing = _pathingFloodfillCache[i];

	switch (descriptor.type)
	{
		case Descriptor::Type::GROUND: pathing.walk(); break;
		case Descriptor::Type::AIR: pathing.fly(); break;

		case Descriptor::Type::BYPASS:
		case Descriptor::Type::TILE:
		case Descriptor::Type::CELL:
		case Descriptor::Type::NONE:
		{
			LOGW << "Unhandled type of " << descriptor;
			DEBUG_ASSERT(false);
		}
		break;
	}
	pathing.put(from);
	pathing.execute();

	// Return a reference to an element of the cache.
	// We never remove elements from the cache and we never reallocate the
	// cache because we only emplace elements up to its initial capacity.
	return pathing;
}

void AINeuralNewt::filterSubjects(std::vector<Descriptor>& subjects)
{
	// Subjects may have gotten orders earlier this planning phase.
	subjects.erase(
		std::remove_if(subjects.begin(), subjects.end(),
			[this](const Descriptor& desc) {
				return hasNewOrder(desc);
			}),
		subjects.end());
}

void AINeuralNewt::sortSubjects(std::vector<Descriptor>& subjects)
{
	// Subjects may have gotten orders earlier this planning phase.
	filterSubjects(subjects);
	// Shuffle them to avoid top-left-bias.
	std::random_shuffle(subjects.begin(), subjects.end());
	// Non-busy should go first.
	std::sort(subjects.begin(), subjects.end(),
		[this](const Descriptor& a, const Descriptor& b) {
			return hasOldOrder(a) < hasOldOrder(b);
		});
}

void AINeuralNewt::controlSettler(const Descriptor& unitdesc,
	Order& bestOrder, int& bestOrderCost, Descriptor& bestOrderTarget,
	float& bestOrderScore)
{
	if (isAdjacentToEnemyCombatant(unitdesc)) return;

	Cell from = _board.cell(unitdesc.position);
	const UnitToken& unit = _board.ground(from);

	if (_bible.tileBuildable(_board.tile(from).type))
	{
		for (const Bible::TileBuild& build : _bible.unitSettles(unit.type))
		{
			float score = 2.0f * _evaluation.tiletypes[(uint8_t) build.type];
			if (!_bible.tileCultivates(build.type).empty())
			{
				int soil = expectedSoil(from);
				score += 0.25f * soil;
				score += 0.50f * std::max(0, soil - 4);
			}
			else if (_bible.tileNeedsNiceness(build.type))
			{
				float niceness = 0.5f * expectedNiceness(from);
				if (_bible.tileIncome(build.type) > 0)
				{
					score += 0.50f * niceness;
				}
				else
				{
					score += 0.10f * std::min(niceness,
						(float) _bible.tilePowerMax(build.type));
				}
				if (niceness > _bible.tilePowerMax(build.type))
				{
					score += 1.00f;
				}
			}
			score += 0.001f * (rand() % 1000);
			int subrow = unitdesc.position.row;
			int subcol = unitdesc.position.col;
			score *= 2 * _evaluation.groundSubjectPreference[subrow][subcol];
			score -= 100.0f * expectedOverlap(from)
				* _evaluation.params[NewtBrain::Output::WEIGHT_OVERLAP];
			if (build.cost > _moneyleftover)
			{
				score -= 100.0f
					* _evaluation.params[NewtBrain::Output::WEIGHT_MONEY];
			}
			score *= 2 * _evaluation.params[NewtBrain::Output::MUL_SETTLE];

			if (score > bestOrderScore)
			{
				bestOrderScore = score;
				bestOrder = Order(Order::Type::SETTLE, unitdesc, build.type);
				bestOrderCost = build.cost;
				bestOrderTarget = Descriptor();
			}
		}
	}

	bool economical = false;
	for (const Bible::TileBuild& build : _bible.unitSettles(unit.type))
	{
		if (_bible.tileIncome(build.type) > 0
			|| !_bible.tileCultivates(build.type).empty())
		{
			economical = true;
		}
	}

	PathingFloodfill& pathing = getExecutedPathingFloodfill(from, unitdesc);

	int speed = _bible.unitSpeed(unit.type);
	if (speed < 1) return;
	if (_board.snow(from)) speed = std::max(1, speed - 1);
	Cell bestCell = Cell::undefined();
	float bestCellScore = bestOrderScore;
	for (Cell at : _board)
	{
		if (at == from) continue;
		if (!_bible.tileBuildable(_board.tile(at).type)) continue;
		int threatdis = _enemyBaseFloodfill.steps(at);
		int expected = economical
			? expectedSoil(at)
			: ((7 - 2 * unit.stacks) * (threatdis >= 3 && threatdis <= 6));
		int turns = std::max(1, (pathing.steps(at) + speed - 1) / speed);
		float score = 1.0f * expected / (turns + 1)
			- 1.5f * std::max(0, 4 - threatdis) * economical
			- 3.0f * (threatdis > 6) * (!economical)
			- 1.5f * (threatdis <= 1);
		score += 0.001f * (rand() % 1000);
		int subrow = unitdesc.position.row;
		int subcol = unitdesc.position.col;
		score *= 2 * _evaluation.groundSubjectPreference[subrow][subcol];
		score -= 100.0f * expectedOverlap(at)
			* _evaluation.params[NewtBrain::Output::WEIGHT_OVERLAP];
		score *= 2 * _evaluation.params[NewtBrain::Output::MUL_MOVE_SETTLER];

		if (score > bestCellScore)
		{
			bestCell = at;
			bestCellScore = score;
		}
	}
	if (bestCell == Cell::undefined()) return;

	moveUnit(unitdesc, pathing, from, bestCell, bestCellScore,
		bestOrder, bestOrderCost, bestOrderTarget, bestOrderScore);
}

void AINeuralNewt::controlCaptor(const Descriptor& unitdesc,
	Order& bestOrder, int& bestOrderCost, Descriptor& bestOrderTarget,
	float& bestOrderScore)
{
	Cell at = _board.cell(unitdesc.position);
	UnitType unittype = _board.ground(at).type;

	if (_bible.unitCanCapture(unittype)
		&& _board.tile(at)
		&& _board.tile(at).owner != _player
		&& _bible.tileOwnable(_board.tile(at).type))
	{
		TileType tiletype = _board.tile(at).type;
		float score = 6.0f * _evaluation.tiletypes[(uint8_t) tiletype];
		score += 4.0f * _bible.tileBinding(tiletype);
		score += 0.001f * (rand() % 1000);
		int subrow = unitdesc.position.row;
		int subcol = unitdesc.position.col;
		score *= 2 * _evaluation.groundSubjectPreference[subrow][subcol];
		score *= 2 * _evaluation.params[NewtBrain::Output::MUL_CAPTURE];

		if (score > bestOrderScore)
		{
			bestOrder = Order(Order::Type::CAPTURE, unitdesc);
			bestOrderScore = score;
			bestOrderCost = 0;
			bestOrderTarget = unitdesc;
		}
	}
}

void AINeuralNewt::controlCombatant(const Descriptor& unitdesc,
	Order& bestOrder, int& bestOrderCost, Descriptor& bestOrderTarget,
	float& bestOrderScore)
{
	bool incombat = isAdjacentToEnemyCombatant(unitdesc);

	Cell at = _board.cell(unitdesc.position);
	UnitType unittype = _board.ground(at).type;

	for (const auto& build : _bible.unitShapes(unittype))
	{
		if (_bible.tileBuildable(_board.tile(at).type))
		{
			float score = 2.0f * _evaluation.tiletypes[(uint8_t) build.type];
			score += 3.0f * incombat;
			score += 0.001f * (rand() % 1000);
			int subrow = unitdesc.position.row;
			int subcol = unitdesc.position.col;
			score *= 2 * _evaluation.groundSubjectPreference[subrow][subcol];
			score *= 2 * _evaluation.params[NewtBrain::Output::MUL_SHAPE];

			if (score > bestOrderScore)
			{
				bestOrderScore = score;
				bestOrder = Order(Order::Type::SHAPE, unitdesc, build.type);
				bestOrderCost = build.cost;
				bestOrderTarget = Descriptor();
			}
		}
	}

	std::vector<Move> dirs = {Move::E, Move::S, Move::W, Move::N};
	std::random_shuffle(dirs.begin(), dirs.end());
	for (const Move& move : dirs)
	{
		Cell target = at + move;
		if (target.edge()) continue;

		bool victim = (_board.ground(target)
			&& _board.ground(target).owner != _player);

		if (_bible.unitCanFocus(unittype))
		{
			float score = 0.0f;
			score += 3.5f * victim;
			score += 0.001f * (rand() % 1000);
			int subrow = unitdesc.position.row;
			int subcol = unitdesc.position.col;
			score *= 2 * _evaluation.groundSubjectPreference[subrow][subcol];
			score *= 2 * _evaluation.params[NewtBrain::Output::MUL_FOCUS];

			if (score > bestOrderScore)
			{
				bestOrderScore = score;
				bestOrder = Order(Order::Type::FOCUS, unitdesc,
					Descriptor::cell(target.pos()));
				bestOrderCost = 0;
				bestOrderTarget = Descriptor();
			}
		}

		if (_bible.unitCanLockdown(unittype))
		{
			float score = 0.5f;
			score += 2.5f * victim;
			score -= 2.0f * incombat;
			score += 0.001f * (rand() % 1000);
			int subrow = unitdesc.position.row;
			int subcol = unitdesc.position.col;
			score *= 2 * _evaluation.groundSubjectPreference[subrow][subcol];
			score *= 2 * _evaluation.params[NewtBrain::Output::MUL_LOCKDOWN];

			if (score > bestOrderScore)
			{
				bestOrderScore = score;
				bestOrder = Order(Order::Type::LOCKDOWN, unitdesc,
					Descriptor::cell(target.pos()));
				bestOrderCost = 0;
				bestOrderTarget = Descriptor::ground(target.pos());
			}
		}

		if (_bible.unitCanShell(unittype))
		{
			float score = 0.0f;
			score += 3.0f * victim;
			score += 3.0f * (_board.tile(target).owner != Player::NONE
				&& _board.tile(target).owner != _player);
			score += 0.001f * (rand() % 1000);
			int subrow = unitdesc.position.row;
			int subcol = unitdesc.position.col;
			score *= 2 * _evaluation.groundSubjectPreference[subrow][subcol];
			score *= 2 * _evaluation.params[NewtBrain::Output::MUL_SHELL];

			if (score > bestOrderScore)
			{
				bestOrderScore = score;
				bestOrder = Order(Order::Type::SHELL, unitdesc,
					Descriptor::cell(target.pos()));
				bestOrderCost = 0;
				bestOrderTarget = Descriptor::ground(target.pos());
			}
		}
	}
}

void AINeuralNewt::controlBombarder(const Descriptor& unitdesc,
	Order& bestOrder, int& bestOrderCost, Descriptor& bestOrderTarget,
	float& bestOrderScore)
{
	Cell from = _board.cell(unitdesc.position);
	const UnitToken& unit = _board.ground(from);

	if (!_bible.unitCanBombard(unit.type))
	{
		return;
	}

	int rangeMin = _bible.unitRangeMin(unit.type);
	int rangeMax = _bible.unitRangeMax(unit.type);
	int threshold = _bible.unitAbilityDamage(unit.type);

	for (Cell target : _board.area(from, rangeMin, rangeMax))
	{
		if (_board.tile(target).owner == _player) continue;
		if (_board.ground(target).owner == _player) continue;
		if (_board.air(target).owner == _player) continue;
		int expected = 0;
		if (_bible.tileControllable(_board.tile(target).type)
			&& _bible.tileDestructible(_board.tile(target).type))
		{
			expected = (_board.tile(target).owner != Player::NONE
				? (5 + _board.tile(target).stacks)
				: 0)
			+ 2 * (_board.ground(target) ? _board.ground(target).stacks : 0)
			+ 4 * (_board.air(target) ? _board.air(target).stacks : 0);
		}
		else if (_board.ground(target)
			&& _bible.unitHitpoints(_board.ground(target).type) >= threshold)
		{
			bool adjacent = false;
			for (Cell to : _board.area(target, 1, 1))
			{
				if (_board.tile(to).owner == _player
					&& _bible.tileControllable(_board.tile(to).type))
				{
					adjacent = true;
				}
			}
			expected = -3 * std::max((int) _board.tile(target).stacks, 1)
				+ (2 + 4 * adjacent)
					* (_board.ground(target) ? _board.ground(target).stacks : 0)
				+ 4 * (_board.air(target) ? _board.air(target).stacks : 0);
		}
		else if (_board.air(target))
		{
			expected = -3 * std::max((int) _board.tile(target).stacks, 1)
				+ 2 * (_board.ground(target) ? _board.ground(target).stacks : 0)
				+ 4 * (_board.air(target) ? _board.air(target).stacks : 0);
		}
		if (expected <= 0) continue;

		float score = 0.40f * (0.5f + 0.5f * _board.current(target)) * expected;
		score += 0.001f * (rand() % 1000);
		int subrow = unitdesc.position.row;
		int subcol = unitdesc.position.col;
		score *= 2 * _evaluation.groundSubjectPreference[subrow][subcol];
		score *= 2 * _evaluation.params[NewtBrain::Output::MUL_BOMBARD];

		if (score > bestOrderScore)
		{
			bestOrderScore = score;
			bestOrder = Order(Order::Type::BOMBARD, unitdesc,
				Descriptor::cell(target.pos()));
			bestOrderCost = 0;
			bestOrderTarget = Descriptor::cell(target.pos());
		}
	}
}

void AINeuralNewt::moveDefense(const Descriptor& unitdesc,
	Order& bestOrder, int& bestOrderCost, Descriptor& bestOrderTarget,
	float& bestOrderScore)
{
	if (isAdjacentToEnemyCombatant(unitdesc)) return;

	// TODO remove once the constant floats it modifies are NN-generated.
	bool offense = 0.001f * (rand() % 1000)
		< _evaluation.params[NewtBrain::Output::ATTACK_CHANCE];

	Cell from = _board.cell(unitdesc.position);
	const UnitToken& unit = _board.ground(from);

	PathingFloodfill& pathing = getExecutedPathingFloodfill(from, unitdesc);

	int currentthreatdis = _enemyBaseFloodfill.steps(from);

	bool canCapture = _bible.unitCanCapture(unit.type);
	bool canOccupy = _bible.unitCanOccupy(unit.type);
	bool hasAbility = (_bible.unitAbilityVolleys(unit.type) > 0);
	int rangeMin = _bible.unitRangeMin(unit.type);
	int rangeMax = _bible.unitRangeMax(unit.type);

	int speed = _bible.unitSpeed(unit.type);
	if (speed < 1) return;
	if (_board.snow(from)) speed = std::max(1, speed - 1);
	Cell bestCell = Cell::undefined();
	float bestCellScore = bestOrderScore;
	float currentCellScore = 0;
	for (Cell at : _board)
	{
		if (_board.gas(at)) continue;
		if (_board.frostbite(at)
			// In Spring, frostbite is used to indicate "Chilled" units.
			&& !(_bible.frostbiteGivesColdFeet()
				&& _bible.chaosMinFrostbite(_season) < 0)) continue;
		if (_board.firestorm(at)) continue;
		if (_board.death(at)) continue;
		int threatdis = _enemyBaseFloodfill.steps(at);
		int civiesdis = _ownBaseFloodfill.steps(at);
		if (threatdis > currentthreatdis + 2) continue;
		float bonus = 0;
		if (civiesdis == 0)
		{
			// Friendly tile.
			if (_bible.tileBinding(_board.tile(at).type)) bonus += 10.0f;
			else bonus += 3.0f;
		}
		else if (_bible.tileBinding(_board.tile(at).type))
		{
			// Enemy or neutral City.
			if (canCapture) bonus += 12.0f;
			else if (canOccupy) bonus += 4.0f;
			else bonus += -5.0f;
		}
		else if (threatdis == 0)
		{
			// Enemy tile.
			if (canCapture) bonus += 6.0f;
			else if (canOccupy) bonus += 1.0f;
			else bonus += -2.0f;
		}
		else if (civiesdis >= 2)
		{
			// Not blocking expansion.
			bonus += 1.0f;
		}
		if (hasAbility)
		{
			int maxval = 0;
			for (Cell target : _board.area(at, rangeMin, rangeMax))
			{
				const TileToken& tiletoken = _board.tile(target);
				if (_bible.tileControllable(tiletoken.type)
					&& tiletoken.owner != _player)
				{
					int val = 4.0f;
					if (maxval < val) maxval = val;
				}
			}
			bonus += maxval;
		}
		const UnitToken& enemy = _board.ground(at);
		if (_bible.unitCanAttack(unit.type)
				&& enemy && enemy.owner != _player
				&& (unit.stacks * _bible.unitAttackShots(unit.type)
								* _bible.unitAttackDamage(unit.type)
						>= _bible.unitHitpoints(enemy.type)))
		{
			bonus += 2.0f;
		}
		if (_board.tile(at).stacks > 0)
		{
			if (at == from) bonus += 3.0f;
			else bonus += 1.0f;
		}
		float expected = 2.0f * (4 + bonus) / 5;
		int turns = std::max(1, (pathing.steps(at) + speed - 1) / speed);
		float score = 0.0f
				+ 4.0f * expected / (3.0f * turns + 1)
				- 5.0f * (!offense && threatdis <= civiesdis)
				- 5.0f * (!offense && civiesdis > 3)
				- 5.0f * (offense && threatdis > currentthreatdis)
				- 5.0f * (offense && civiesdis == 0);
		score += 0.001f * (rand() % 1000);
		int subrow = unitdesc.position.row;
		int subcol = unitdesc.position.col;
		score *= 2 * _evaluation.groundSubjectPreference[subrow][subcol];
		int targetrow = at.pos().row;
		int targetcol = at.pos().col;
		score *= 2 * _evaluation.placementPreference[targetrow][targetcol];
		score -= 100.0f * isTarget(at.pos())
			* _evaluation.params[NewtBrain::Output::WEIGHT_ISTARGET];
		score *= 2 * _evaluation.params[NewtBrain::Output::MUL_MOVE_DEFENSE];
		if (at == from)
		{
			currentCellScore = score;
		}
		else if (score > bestCellScore)
		{
			bestCell = at;
			bestCellScore = score;
		}
	}
	if (bestCell == Cell::undefined()) return;
	if (bestCellScore < currentCellScore) return;

	moveUnit(unitdesc, pathing, from, bestCell, bestCellScore,
		bestOrder, bestOrderCost, bestOrderTarget, bestOrderScore);
}

void AINeuralNewt::moveUnit(const Descriptor& unitdesc,
	const PathingFloodfill& pathing,
	const Cell& from, const Cell& target, float targetScore,
	Order& bestOrder, int& bestOrderCost, Descriptor& bestOrderTarget,
	float& bestOrderScore)
{
	std::vector<Move> moves;
	{
		Move current;
		Cell at = target;
		while ((current = pathing.step(at)) != Move::X)
		{
			moves.emplace_back(flip(current));
			at = at + current;
		}
	}
	std::reverse(moves.begin(), moves.end());

	const UnitToken& unit = _board.ground(from);
	Cell at = from;
	size_t length = 0;
	for (; length < moves.size(); length++)
	{
		bool stop = false;
		for (const Move& dir : {Move::E, Move::S, Move::W, Move::N})
		{
			if (dir == moves[length] || ::flip(dir) == moves[length]) continue;

			Cell near = at + dir;
			if (near.edge()) continue;

			const UnitToken& enemy = _board.ground(near);
			if (enemy && enemy.owner != _player
				&& _bible.unitCanAttack(enemy.type)
				&& (enemy.stacks * _bible.unitAttackShots(enemy.type)
						* _bible.unitAttackDamage(enemy.type)
					>= unit.stacks * _bible.unitHitpoints(unit.type))
				&& !(_board.ground(at)
					&& _board.ground(at).owner == _player
					&& at != from))
			{
				stop = true;
				break;
			}
		}
		if (stop) break;
		at = at + moves[length];
	}
	if (length == 0)
	{
		return;
	}
	else if (length < moves.size())
	{
		moves.resize(length);
	}
	else if (target != at)
	{
		LOGE << "Assertion failure";
		DEBUG_ASSERT(target == at);
	}

	bestOrderScore = targetScore;
	bestOrder = Order(Order::Type::MOVE, unitdesc,
		Descriptor::cell(at.pos()),
		moves);
	bestOrderCost = 0;
	bestOrderTarget = Descriptor::ground(at.pos());
}

void AINeuralNewt::controlCreator(const Descriptor& citydesc,
		Order& bestOrder, int& bestOrderCost, Descriptor& bestOrderTarget,
		float& bestOrderScore)
{
	Cell from = _board.cell(citydesc.position);
	TileType fromtype = _board.tile(from).type;

	for (const Bible::UnitBuild& build : _bible.tileProduces(fromtype))
	{
		std::vector<Descriptor> targetdescs;
		if (_bible.unitAir(build.type))
		{
			// TODO allow air units
			continue;
		}
		else
		{
			targetdescs.push_back(Descriptor::ground(from.pos()));
			for (const Move& move : {Move::E, Move::S, Move::W, Move::N})
			{
				Cell target = from + move;
				if (target.edge()) continue;

				if (_bible.tileWalkable(_board.tile(target).type))
				{
					targetdescs.push_back(Descriptor::ground(target.pos()));
				}
			}
		}

		int power = _bible.powerAbolished()
			? _board.tile(from).stacks
			: _board.tile(from).power;
		int stacks = std::min(power, (int) _bible.unitStacksMax(build.type));

		for (const Descriptor& targetdesc : targetdescs)
		{
			Cell target = _board.cell(targetdesc.position);

			float score = (1.0f + 1.0f * stacks)
				* 2 * _evaluation.unittypes[(uint8_t) build.type];
			score += 0.001f * (rand() % 1000);
			int subrow = citydesc.position.row;
			int subcol = citydesc.position.col;
			score *= 2 * _evaluation.tileSubjectPreference[subrow][subcol];
			int targetrow = targetdesc.position.row;
			int targetcol = targetdesc.position.col;
			score *= 2 * _evaluation.placementPreference[targetrow][targetcol];
			score -= 100.0f * ((bool) _board.ground(target))
				* _evaluation.params[NewtBrain::Output::WEIGHT_OCCUPIED];
			score -= 100.0f * isTarget(target.pos())
				* _evaluation.params[NewtBrain::Output::WEIGHT_ISTARGET];
			if (build.cost > _moneyleftover)
			{
				score -= 100.0f
					* _evaluation.params[NewtBrain::Output::WEIGHT_MONEY];
			}
			score *= 2 * _evaluation.params[NewtBrain::Output::MUL_PRODUCE];

			if (score > bestOrderScore)
			{
				bestOrderScore = score;
				bestOrder = Order(Order::Type::PRODUCE, citydesc,
					Descriptor::cell(targetdesc.position), build.type);
				bestOrderCost = build.cost;
				bestOrderTarget = targetdesc;
			}
		}
	}
}

void AINeuralNewt::controlExpander(const Descriptor& citydesc,
		Order& bestOrder, int& bestOrderCost, Descriptor& bestOrderTarget,
		float& bestOrderScore)
{
	Cell from = _board.cell(citydesc.position);
	TileType fromtype = _board.tile(from).type;

	for (const Bible::TileBuild& build : _bible.tileExpands(fromtype))
	{
		std::vector<Descriptor> targetdescs;
		for (const Move& move : {Move::E, Move::S, Move::W, Move::N})
		{
			Cell target = from + move;
			if (target.edge()) continue;

			if (_bible.tileBuildable(_board.tile(target).type))
			{
				targetdescs.push_back(Descriptor::tile(target.pos()));
			}
		}

		for (const Descriptor& targetdesc : targetdescs)
		{
			Cell target = _board.cell(targetdesc.position);

			float score = 4.0f * _evaluation.tiletypes[(uint8_t) build.type];
			score += 0.001f * (rand() % 1000);
			int subrow = citydesc.position.row;
			int subcol = citydesc.position.col;
			score *= 2 * _evaluation.tileSubjectPreference[subrow][subcol];
			int targetrow = targetdesc.position.row;
			int targetcol = targetdesc.position.col;
			score *= 2 * _evaluation.placementPreference[targetrow][targetcol];
			score -= 100.0f * ((bool) _board.ground(target))
				* _evaluation.params[NewtBrain::Output::WEIGHT_OCCUPIED];
			score -= 100.0f * isTarget(target.pos())
				* _evaluation.params[NewtBrain::Output::WEIGHT_ISTARGET];
			if (build.cost > _moneyleftover)
			{
				score -= 100.0f
					* _evaluation.params[NewtBrain::Output::WEIGHT_MONEY];
			}
			score *= 2 * _evaluation.params[NewtBrain::Output::MUL_EXPAND];

			if (score > bestOrderScore)
			{
				bestOrderScore = score;
				bestOrder = Order(Order::Type::EXPAND, citydesc,
					Descriptor::cell(targetdesc.position), build.type);
				bestOrderCost = build.cost;
				bestOrderTarget = targetdesc;
			}
		}
	}
}

void AINeuralNewt::controlCultivator(const Descriptor& citydesc,
		Order& bestOrder, int& bestOrderCost, Descriptor& bestOrderTarget,
		float& bestOrderScore)
{
	Cell from = _board.cell(citydesc.position);
	TileType fromtype = _board.tile(from).type;

	for (const Bible::TileBuild& build : _bible.tileCultivates(fromtype))
	{
		float score = 2.0f * _evaluation.tiletypes[(uint8_t) build.type];
		score += 0.001f * (rand() % 1000);
		int subrow = citydesc.position.row;
		int subcol = citydesc.position.col;
		score *= 2 * _evaluation.tileSubjectPreference[subrow][subcol];
		if (build.cost > _moneyleftover)
		{
			score -= 100.0f
				* _evaluation.params[NewtBrain::Output::WEIGHT_MONEY];
		}
		score *= 2 * _evaluation.params[NewtBrain::Output::MUL_CULTIVATE];

		if (score > bestOrderScore)
		{
			bestOrderScore = score;
			bestOrder = Order(Order::Type::CULTIVATE, citydesc, build.type);
			bestOrderCost = build.cost;
			bestOrderTarget = Descriptor();
		}
	}
}

void AINeuralNewt::controlUpgrader(const Descriptor& citydesc,
		Order& bestOrder, int& bestOrderCost, Descriptor& bestOrderTarget,
		float& bestOrderScore)
{
	Cell from = _board.cell(citydesc.position);
	TileType fromtype = _board.tile(from).type;

	bool occupied = (_board.ground(from)
		&& (_board.ground(from).owner != _player
			|| _bible.upgradeNotBlockedByFriendly()));

	for (const Bible::TileBuild& build : _bible.tileUpgrades(fromtype))
	{
		if (build.type == TileType::NONE
			&& _board.tile(from).stacks
				>= _bible.tileStacksMax(_board.tile(from).type))
		{
			continue;
		}

		TileType newtype = (build.type == TileType::NONE)
			? fromtype : build.type;

		float score = 4.0f * _evaluation.tiletypes[(uint8_t) newtype];
		score += 0.001f * (rand() % 1000);
		int subrow = citydesc.position.row;
		int subcol = citydesc.position.col;
		score *= 2 * _evaluation.tileSubjectPreference[subrow][subcol];
		score -= 100.0f * occupied
			* _evaluation.params[NewtBrain::Output::WEIGHT_OCCUPIED];
		if (build.cost > _moneyleftover)
		{
			score -= 100.0f
				* _evaluation.params[NewtBrain::Output::WEIGHT_MONEY];
		}
		score *= 2 * _evaluation.params[NewtBrain::Output::MUL_UPGRADE];

		if (score > bestOrderScore)
		{
			bestOrderScore = score;
			bestOrder = Order(Order::Type::UPGRADE, citydesc, build.type);
			bestOrderCost = build.cost;
			bestOrderTarget = citydesc;
		}
	}
}

void AINeuralNewt::controlStopper(const Descriptor& desc,
	Order& bestOrder, int& bestOrderCost, Descriptor& bestOrderTarget,
	float& bestOrderScore)
{
	float score = 0.5f;
	int subrow = desc.position.row;
	int subcol = desc.position.col;
	if (desc.type == Descriptor::Type::GROUND)
	{
		score *= 2 * _evaluation.groundSubjectPreference[subrow][subcol];
	}
	else if (desc.type == Descriptor::Type::AIR)
	{
		// TODO airSubjectPreference
	}
	else if (desc.type == Descriptor::Type::TILE)
	{
		score *= 2 * _evaluation.tileSubjectPreference[subrow][subcol];
	}

	if (score > bestOrderScore)
	{
		bestOrderScore = score;
		bestOrder = Order(Order::Type::HALT, desc);
		bestOrderCost = 0;
		bestOrderTarget = desc;
	}
}

bool AINeuralNewt::isTarget(const Position& position)
{
	return (std::find(_targets.begin(), _targets.end(), position)
			!= _targets.end());
}

bool AINeuralNewt::hasOldOrder(const Descriptor& unitdesc)
{
	for (const Order& order : _unfinishedOrders)
	{
		if (order.subject == unitdesc) return true;
	}
	return false;
}

bool AINeuralNewt::hasNewOrder(const Descriptor& unitdesc)
{
	for (const Order& order : _newOrders)
	{
		if (order.subject == unitdesc) return true;
	}
	return false;
}

bool AINeuralNewt::isAdjacentToEnemyCombatant(const Descriptor& unitdesc)
{
	Cell at = _board.cell(unitdesc.position);

	std::vector<Move> dirs = {Move::E, Move::S, Move::W, Move::N};
	std::random_shuffle(dirs.begin(), dirs.end());
	for (const Move& move : dirs)
	{
		Cell target = at + move;
		if (target.edge()) continue;

		if (_board.ground(target)
			&& _board.ground(target).owner != _player
			&& _bible.unitCanAttack(_board.ground(target).type))
		{
			return true;
		}
	}

	return false;
}

bool AINeuralNewt::isOccupied(const Position& position)
{
	Cell index = _board.cell(position);
	if (_board.ground(index).type == UnitType::NONE) return false;
	if (_board.tile(index).owner == _board.ground(index).owner) return false;
	return true;
}

int AINeuralNewt::expectedSoil(Cell index)
{
	int expected = 0;
	for (Cell at : _board.area(index, 1, 2))
	{
		// Only some tiles (grass, dirt) can be built on.
		if (!_bible.tileBuildable(_board.tile(at).type)) continue;
		// If the tile is owned by the player, we do not build on it.
		if (_board.tile(at).owner == _player) continue;

		expected++;
	}
	return expected;
}

int AINeuralNewt::expectedNiceness(Cell index)
{
	int expected = 0;
	for (Cell at : _board.area(index, 1, 2))
	{
		if (_bible.tileGrassy(_board.tile(at).type))
		{
			expected += 2;
		}
		else if (_bible.tileNatural(_board.tile(at).type))
		{
			expected += 1;
		}
	}
	return expected;
}

int AINeuralNewt::expectedOverlap(Cell index)
{
	int blocking = 0;
	for (const Move& dir : {Move::E, Move::S, Move::W, Move::N})
	{
		Cell near = index + dir;
		if (near.edge()) continue;

		if (_board.tile(near).owner == _player
			&& !_bible.tileExpands(_board.tile(near).type).empty())
		{
			blocking++;
		}
	}
	return blocking;
}

NewtBrain::Output DummyNewtBrain::evaluate()
{
	Output output;
	output.assign(std::vector<float>(Output::SIZE, 0.5f));
	output.params[Output::ATTACK_CHANCE] = 1.00f;
	return output;
}
