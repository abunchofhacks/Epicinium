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
#include "airampantrhino.hpp"
#include "source.hpp"

#include "difficulty.hpp"
#include "pathingfloodfill.hpp"
#include "aim.hpp"
#include "bible.hpp"

 // windows.h is being annoying
#undef near


std::string AIRampantRhino::ainame() const
{
	return "RampantRhino";
}

std::string AIRampantRhino::authors() const
{
	return "Sander in 't Veld";
}

AIRampantRhino::AIRampantRhino(const Player& player,
		const Difficulty& difficulty,
		const std::string& rulesetname, char character) :
	AICommander(player, difficulty, rulesetname, character)
{
	if (_difficulty == Difficulty::NONE)
	{
		LOGW << "AI difficulty not set";
	}

	_citytype = _bible.tiletype("city");
	if (_citytype == TileType::NONE)
	{
		LOGE << "Missing type 'city'";
	}
	_towntype = _bible.tiletype("town");
	if (_towntype == TileType::NONE)
	{
		LOGE << "Missing type 'town'";
	}
	_outposttype = _bible.tiletype("outpost");
	if (_outposttype == TileType::NONE)
	{
		LOGE << "Missing type 'outpost'";
	}
	_industrytype = _bible.tiletype("industry");
	if (_industrytype == TileType::NONE)
	{
		LOGE << "Missing type 'industry'";
	}
	_barrackstype = _bible.tiletype("barracks");
	if (_barrackstype == TileType::NONE)
	{
		LOGE << "Missing type 'barracks'";
	}
	_airfieldtype = _bible.tiletype("airfield");
	if (_airfieldtype == TileType::NONE)
	{
		LOGE << "Missing type 'airfield'";
	}
	_farmtype = _bible.tiletype("farm");
	if (_farmtype == TileType::NONE)
	{
		LOGE << "Missing type 'farm'";
	}
	_soiltype = _bible.tiletype("soil");
	if (_soiltype == TileType::NONE)
	{
		LOGE << "Missing type 'soil'";
	}
	_cropstype = _bible.tiletype("crops");
	if (_cropstype == TileType::NONE)
	{
		LOGE << "Missing type 'crops'";
	}
	_trenchestype = _bible.tiletype("trenches");
	if (_trenchestype == TileType::NONE)
	{
		LOGE << "Missing type 'trenches'";
	}
	_settlertype = _bible.unittype("settler");
	if (_settlertype == UnitType::NONE)
	{
		LOGE << "Missing type 'settler'";
	}
	_militiatype = _bible.unittype("militia");
	if (_militiatype == UnitType::NONE)
	{
		LOGE << "Missing type 'militia'";
	}
	_riflemantype = _bible.unittype("rifleman");
	if (_riflemantype == UnitType::NONE)
	{
		LOGE << "Missing type 'rifleman'";
	}
	_tanktype = _bible.unittype("tank");
	if (_tanktype == UnitType::NONE)
	{
		LOGE << "Missing type 'tank'";
	}
	_gunnertype = _bible.unittype("gunner");
	if (_gunnertype == UnitType::NONE)
	{
		LOGE << "Missing type 'gunner'";
	}
	_sappertype = _bible.unittype("sapper");
	if (_sappertype == UnitType::NONE)
	{
		LOGE << "Missing type 'sapper'";
	}

	_settlermoney = 0;
	for (const auto& build : _bible.tileProduces(_citytype))
	{
		if (build.type == _settlertype)
		{
			_settlermoney = build.cost;
			break;
		}
	}

	_settlingmoney = 0;
	for (const auto& build : _bible.unitSettles(_settlertype))
	{
		if (build.type == _farmtype)
		{
			_settlingmoney = build.cost;
			break;
		}
	}
}

void AIRampantRhino::process()
{
	count();

	if (_captors.size() > 0)
	{
		controlCaptors();
	}

	if (_blockers.size() > 0)
	{
		controlBlockers();
	}

	if (_bombarders.size() > 0)
	{
		controlBombarders();
	}

	if (_offenses.size() > 0)
	{
		controlOffenses();
	}
	else if ((int) _defenses.size() >= maxDefenseUnits())
	{
		declareOffenses();
	}

	if ((int) (_offenses.size() + _defenses.size()) < maxMilitaryUnits())
	{
		createDefenses();
	}

	if (_defenses.size() > 0)
	{
		controlDefenses();
	}

	if (_cultivators.size() > 0)
	{
		controlCultivators();
	}

	if (_settlers.size() > 0)
	{
		controlSettlers();
	}
	else
	{
		createSettlers();
	}

	if (_defenses.size() > 0)
	{
		controlIdleDefenses();
	}

	if (_stoppers.size() > 0)
	{
		controlStoppers();
	}
}

void AIRampantRhino::count()
{
	_settlers.clear();
	_settlercreators.clear();
	_industrycreators.clear();
	_barrackscreators.clear();
	_defenses.clear();
	_offenses.clear();
	_defensecreators.clear();
	_defenseupgraders.clear();
	_economyupgraders.clear();
	_captors.clear();
	_blockers.clear();
	_bombarders.clear();
	_cultivators.clear();
	_stoppers.clear();
	_targets.clear();

	_moneystarting = _money;
	_moneyleftover = _money;

	// Do not spend all of our money on toys when we have bills to pay.
	_moneyreserved = _settlermoney + _settlingmoney;

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

			int power = _bible.powerAbolished()
				? _board.tile(index).stacks
				: _board.tile(index).power;
			int powermax = _bible.powerAbolished()
				? _bible.tileStacksMax(_board.tile(index).type)
				: _bible.tilePowerMax(_board.tile(index).type);

			if (std::find_if(prods.begin(), prods.end(),
				[this](const Bible::UnitBuild& build){

					return (build.type == _settlertype);
				}) != prods.end())
			{
				if (power > 0)
				{
					_settlercreators.emplace_back(
						Descriptor::tile(index.pos()));
				}
			}

			if (std::find_if(prods.begin(), prods.end(),
				[this](const Bible::UnitBuild& build){

					return (build.type == _militiatype);
				}) != prods.end())
			{
				if (power >= std::max(2, std::min(3, powermax)))
				{
					_defensecreators.emplace_back(
						Descriptor::tile(index.pos()));
				}
			}
			else if (std::find_if(prods.begin(), prods.end(),
				[this](const Bible::UnitBuild& build){

					return (build.type != _settlertype
						&&  build.type != _militiatype);
				}) != prods.end())
			{
				if (power >= 1 + 1 * (_board.tile(index).type == _citytype))
				{
					_defensecreators.emplace_back(
						Descriptor::tile(index.pos()));
				}
			}

			if (!upgrs.empty() && power >= 1 && !_board.ground(index))
			{
				Descriptor desc = Descriptor::tile(index.pos());
				if (std::find(_defensecreators.begin(),
						_defensecreators.end(), desc)
					!= _defensecreators.end())
				{
					_defenseupgraders.emplace_back(desc);
				}
				else
				{
					_economyupgraders.emplace_back(desc);
				}
			}

			if (std::find_if(expos.begin(), expos.end(),
				[this](const Bible::TileBuild& build){

					return (build.type == _industrytype);
				}) != expos.end())
			{
				for (const Move& move : {Move::E, Move::S, Move::W, Move::N})
				{
					Cell target = index + move;
					if (target.edge()) continue;

					if (power > 0
						&& !_board.ground(target)
						&& _bible.tileBuildable(_board.tile(target).type))
					{
						_industrycreators.emplace_back(
							Descriptor::tile(index.pos()));
						break;
					}
				}
			}

			if (canBuildBarracks()
				&& std::find_if(expos.begin(), expos.end(),
					[this](const Bible::TileBuild& build){

						return (build.type == _barrackstype);
					}) != expos.end())
			{
				for (const Move& move : {Move::E, Move::S, Move::W, Move::N})
				{
					Cell target = index + move;
					if (target.edge()) continue;

					if (power > 1
						&& !_board.ground(target)
						&& _bible.tileBuildable(_board.tile(target).type))
					{
						_barrackscreators.emplace_back(
							Descriptor::tile(index.pos()));
						break;
					}
				}
			}

			if (!cults.empty())
			{
				if (power > 0
					&& expectedSoil(index) >= 4)
				{
					_cultivators.emplace_back(
						Descriptor::tile(index.pos()));
				}
			}
		}

		if (_board.ground(index).owner == _player)
		{
			if (!_bible.unitSettles(_board.ground(index).type).empty()
				&& (!_bible.unitCanAttack(_board.ground(index).type)
					|| _board.ground(index).stacks == 1))
			{
				_settlers.emplace_back(Descriptor::ground(index.pos()));
			}

			if (_bible.unitCanAttack(_board.ground(index).type)
				|| (_bible.unitAbilityVolleys(_board.ground(index).type) > 0))
			{
				Descriptor desc = Descriptor::ground(index.pos());
				bool offense = false;
				for (const Order& order : _unfinishedOrders)
				{
					if (order.subject == desc
						&& order.type == Order::Type::MOVE)
					{
						int speed = _bible.unitSpeed(_board.ground(index).type);
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
				if (offense) _offenses.emplace_back(desc);
				else if (std::find(_settlers.begin(), _settlers.end(), desc)
					!= _settlers.end())
				{
					// This unit acts as a settler, not a defender.
				}
				else _defenses.emplace_back(desc);
			}
			else
			{
				Descriptor desc = Descriptor::ground(index.pos());
				for (const Order& order : _unfinishedOrders)
				{
					if (order.subject == desc
						&& order.type == Order::Type::MOVE)
					{
						int speed = _bible.unitSpeed(_board.ground(index).type);
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

			if (_bible.unitCanCapture(_board.ground(index).type)
				&& _board.tile(index)
				&& _board.tile(index).owner != _player
				&& _bible.tileOwnable(_board.tile(index).type))
			{
				_captors.emplace_back(Descriptor::ground(index.pos()));
			}

			if (!_bible.unitCanCapture(_board.ground(index).type)
				&& !_bible.unitCanOccupy(_board.ground(index).type)
				&& _board.tile(index)
				&& _board.tile(index).owner != _player
				&& _bible.tileBinding(_board.tile(index).type))
			{
				_blockers.emplace_back(Descriptor::ground(index.pos()));
			}

			if (_bible.unitCanBombard(_board.ground(index).type))
			{
				_bombarders.emplace_back(Descriptor::ground(index.pos()));
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

void AIRampantRhino::lookAtUnfinishedOrder(const Order& order)
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

void AIRampantRhino::createSettlers()
{
	if (_settlercreators.empty()
		|| _moneystarting < minSettlersMoney())
	{
		createIndustry();
		return;
	}

	if ((rand() % 10) == 0)
	{
		createIndustry();
	}
	else if (!_economyupgraders.empty() && (rand() % 3) == 0)
	{
		upgradeEconomy();
	}

	_settlercreators.erase(std::remove_if(_settlercreators.begin(),
		_settlercreators.end(), [this](const Descriptor& desc){

			return hasNewOrder(desc);
		}), _settlercreators.end());
	std::random_shuffle(_settlercreators.begin(), _settlercreators.end());
	std::sort(_settlercreators.begin(), _settlercreators.end(), [this](
			const Descriptor& a, const Descriptor& b){

		// Non-busy should go first.
		return hasOldOrder(a) < hasOldOrder(b);
	});
	int max = std::max(0, (int) _settlercreators.size());
	for (int i = 0; i < max && (int) _newOrders.size() < maxOrders(); i++)
	{
		const Descriptor& citydesc = _settlercreators[i];
		Cell from = _board.cell(citydesc.position);

		int cost = 0;
		TileType fromtype = _board.tile(from).type;
		for (const Bible::UnitBuild& build : _bible.tileProduces(fromtype))
		{
			if (build.type == _settlertype)
			{
				cost = build.cost;
			}
		}
		if (_moneyleftover < cost + _settlingmoney) continue;

		Descriptor targetdesc;
		if (!_board.ground(from) && !isTarget(from.pos()))
		{
			targetdesc = Descriptor::cell(from.pos());
		}
		else
		{
			std::vector<Move> dirs = {Move::E, Move::S, Move::W, Move::N};
			std::random_shuffle(dirs.begin(), dirs.end());
			for (const Move& move : dirs)
			{
				Cell target = from + move;
				if (target.edge()) continue;

				if (!_board.ground(target)
					&& _bible.tileWalkable(_board.tile(target).type)
					&& !isTarget(target.pos()))
				{
					targetdesc = Descriptor::cell(target.pos());
					break;
				}
			}
		}
		if (targetdesc.type == Descriptor::Type::NONE) continue;

		_newOrders.emplace_back(Order::Type::PRODUCE, citydesc,
			targetdesc,
			_settlertype);
		_targets.emplace_back(targetdesc.position);
		_moneyleftover -= cost + _settlingmoney;
	}
}

void AIRampantRhino::controlSettlers()
{
	_settlers.erase(std::remove_if(_settlers.begin(),
		_settlers.end(), [this](const Descriptor& desc){

			return hasNewOrder(desc);
		}), _settlers.end());
	std::random_shuffle(_settlers.begin(), _settlers.end());
	std::sort(_settlers.begin(), _settlers.end(), [this](
			const Descriptor& a, const Descriptor& b){

		// Non-busy should go first.
		return hasOldOrder(a) < hasOldOrder(b);
	});
	int max = std::max(0, (int) _settlers.size());
	for (int i = 0; i < max && (int) _newOrders.size() < maxOrders(); i++)
	{
		controlSettler(_settlers[i]);
	}
}

void AIRampantRhino::controlSettler(const Descriptor& unitdesc)
{
	Cell from = _board.cell(unitdesc.position);
	const UnitToken& unit = _board.ground(from);

	TileFloodfill enemyThreats(_bible, _board);
	enemyThreats.exclude({_soiltype, _cropstype});
	enemyThreats.exclude({_player, Player::NONE});
	enemyThreats.execute();

	const auto& builds = _bible.unitSettles(unit.type);
	bool cancity = false;
	int citycost = 0;
	bool cantown = false;
	int towncost = 0;
	bool canfarm = false;
	int farmcost = 0;
	bool canoutpost = false;
	int outpostcost = 0;
	for (const Bible::TileBuild& build : builds)
	{
		if      (build.type == _citytype)
		{
			cancity = true;
			citycost = build.cost;
		}
		else if (build.type == _towntype)
		{
			cantown = true;
			towncost = build.cost;
		}
		else if (build.type == _farmtype)
		{
			canfarm = true;
			farmcost = build.cost;
		}
		else if (build.type == _outposttype)
		{
			canoutpost = true;
			outpostcost = build.cost;
		}
	}

	if (_bible.tileBuildable(_board.tile(from).type))
	{
		int buildables = expectedSoil(from);
		int niceness = expectedNiceness(from);
		int blocking = 0;
		for (const Move& dir : {Move::E, Move::S, Move::W, Move::N})
		{
			Cell near = from + dir;
			if (near.edge()) continue;

			const TileToken& city = _board.tile(near);
			if (city.type == _citytype && city.owner == _player)
			{
				blocking++;
			}
		}

		if (blocking)
		{
			// Do not build here.
		}
		else if (cancity
			&& _moneyleftover >= citycost
			&& niceness >= 6
			&& canBuildCities()
			&& (rand() % 2) == 0)
		{
			_newOrders.emplace_back(Order::Type::SETTLE, unitdesc,
				_citytype);
			_moneyleftover -= citycost;
			return;
		}
		else if (canfarm
			&& _moneyleftover >= farmcost
			&& buildables >= 6
			&& (rand() % 2) == 0)
		{
			_newOrders.emplace_back(Order::Type::SETTLE, unitdesc,
				_farmtype);
			_moneyleftover -= farmcost;
			return;
		}
		else if (cancity
			&& _moneyleftover >= 2 * citycost
			&& niceness >= 6
			&& canBuildCities())
		{
			_newOrders.emplace_back(Order::Type::SETTLE, unitdesc,
				_citytype);
			_moneyleftover -= citycost;
			return;
		}
		else if (cantown
			&& _moneyleftover >= towncost
			&& niceness >= 6)
		{
			_newOrders.emplace_back(Order::Type::SETTLE, unitdesc,
				_towntype);
			_moneyleftover -= towncost;
			return;
		}
		else if (canfarm
			&& _moneyleftover >= farmcost
			&& buildables >= 4)
		{
			_newOrders.emplace_back(Order::Type::SETTLE, unitdesc,
				_farmtype);
			_moneyleftover -= farmcost;
			return;
		}
		else if (canoutpost
			&& _moneyleftover >= outpostcost
			&& enemyThreats.steps(from) <= 6)
		{
			_newOrders.emplace_back(Order::Type::SETTLE, unitdesc,
				_outposttype);
			_moneyleftover -= outpostcost;
			return;
		}
	}

	PathingFloodfill pathing(_bible, _board);
	pathing.put(from);
	pathing.execute();

	int speed = _bible.unitSpeed(unit.type);
	if (speed < 1) return;
	if (_board.snow(from)) speed = std::max(1, speed - 1);
	Cell bestCell = Cell::undefined();
	float bestScore = 0;
	for (Cell at : _board)
	{
		if (at == from) continue;
		if (!_bible.tileBuildable(_board.tile(at).type)) continue;
		int blocking = 0;
		for (const Move& dir : {Move::E, Move::S, Move::W, Move::N})
		{
			Cell near = at + dir;
			if (near.edge()) continue;

			const TileToken& city = _board.tile(near);
			if (city.type == _citytype && city.owner == _player)
			{
				blocking++;
			}
		}
		if (blocking) continue;
		bool economical = (cancity || cantown || canfarm);
		int threatdis = enemyThreats.steps(at);
		int expected = economical
			? expectedSoil(at)
			: (3 * (threatdis >= 3 && threatdis <= 6));
		int turns = std::max(1, (pathing.steps(at) + speed - 1) / speed);
		float score = 1.0f * expected / (turns + 1)
				- 1.5f * std::max(0, 4 - threatdis) * economical
				- 1.5f * (threatdis <= 1)
				+ 0.001f * (rand() % 1000);
		if (score > bestScore)
		{
			bestCell = at;
			bestScore = score;
		}
	}
	if (bestCell == Cell::undefined()) return;
	Cell target = bestCell;
	if (target == from) return;

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
	if (length == 0) return;
	else if (length < moves.size())
	{
		moves.resize(length);
		target = at;
	}
	else if (target != at)
	{
		LOGE << "Assertion failure";
		DEBUG_ASSERT(target == at);
	}

	_newOrders.emplace_back(Order::Type::MOVE, unitdesc,
		Descriptor::cell(target.pos()),
		moves);
	_targets.emplace_back(target.pos());
}

void AIRampantRhino::createIndustry()
{
	if (_industrycreators.empty())
	{
		return;
	}

	_industrycreators.erase(std::remove_if(_industrycreators.begin(),
		_industrycreators.end(), [this](const Descriptor& desc){

			return hasNewOrder(desc);
		}), _industrycreators.end());
	std::random_shuffle(_industrycreators.begin(), _industrycreators.end());
	std::sort(_industrycreators.begin(), _industrycreators.end(), [this](
			const Descriptor& a, const Descriptor& b){

		// Non-busy should go first.
		return hasOldOrder(a) < hasOldOrder(b);
	});
	int max = std::max(0, (int) _industrycreators.size());
	for (int i = 0; i < max && (int) _newOrders.size() < maxOrders(); i++)
	{
		createIndustry(_industrycreators[i]);
	}
}

void AIRampantRhino::createIndustry(const Descriptor& citydesc)
{
	int cost = 0;
	Cell from = _board.cell(citydesc.position);
	TileType fromtype = _board.tile(from).type;
	for (const Bible::TileBuild& build : _bible.tileExpands(fromtype))
	{
		if (build.type == _industrytype)
		{
			cost = build.cost;
		}
	}
	if (_moneyleftover < cost) return;

	std::vector<Move> dirs = {Move::E, Move::S, Move::W, Move::N};
	std::random_shuffle(dirs.begin(), dirs.end());

	for (const Move& move : dirs)
	{
		Cell target = _board.cell(citydesc.position) + move;
		if (target.edge()) continue;

		if (_bible.tileBuildable(_board.tile(target).type))
		{
			_newOrders.emplace_back(Order::Type::EXPAND, citydesc,
				Descriptor::tile(target.pos()),
				_industrytype);
			_moneyleftover -= cost;
			break;
		}
	}
}

void AIRampantRhino::createBarracks()
{
	if (_barrackscreators.empty()
		|| _moneystarting < minBarracksMoney())
	{
		return;
	}

	_barrackscreators.erase(std::remove_if(_barrackscreators.begin(),
		_barrackscreators.end(), [this](const Descriptor& desc){

			return hasNewOrder(desc);
		}), _barrackscreators.end());
	std::random_shuffle(_barrackscreators.begin(), _barrackscreators.end());
	std::sort(_barrackscreators.begin(), _barrackscreators.end(), [this](
			const Descriptor& a, const Descriptor& b){

		// Non-busy should go first.
		return hasOldOrder(a) < hasOldOrder(b);
	});
	int max = std::max(0, (int) _barrackscreators.size());
	for (int i = 0; i < max && (int) _newOrders.size() < maxMilitaryOrders(); i++)
	{
		createBarracks(_barrackscreators[i]);

		// Only build a second barracks each turn if we have plenty of money.
		if (_moneyleftover - _moneyreserved < minBarracksMoney()) break;
	}
}

void AIRampantRhino::createBarracks(const Descriptor& citydesc)
{
	int cost = 0;
	Cell from = _board.cell(citydesc.position);
	TileType fromtype = _board.tile(from).type;
	for (const Bible::TileBuild& build : _bible.tileExpands(fromtype))
	{
		if (build.type == _barrackstype)
		{
			cost = build.cost;
		}
	}
	if (_moneyleftover - _moneyreserved < cost) return;

	std::vector<Move> dirs = {Move::E, Move::S, Move::W, Move::N};
	std::random_shuffle(dirs.begin(), dirs.end());

	for (const Move& move : dirs)
	{
		Cell target = _board.cell(citydesc.position) + move;
		if (target.edge()) continue;

		if (_bible.tileBuildable(_board.tile(target).type))
		{
			_newOrders.emplace_back(Order::Type::EXPAND, citydesc,
				Descriptor::tile(target.pos()),
				_barrackstype);
			_moneyleftover -= cost;
			break;
		}
	}
}

void AIRampantRhino::upgradeDefenseCreator()
{
	if (_defenseupgraders.empty()
		|| _moneystarting < minDefenseUpgradeMoney())
	{
		return;
	}

	_defenseupgraders.erase(std::remove_if(_defenseupgraders.begin(),
		_defenseupgraders.end(), [this](const Descriptor& desc){

			return hasNewOrder(desc);
		}), _defenseupgraders.end());
	std::random_shuffle(_defenseupgraders.begin(), _defenseupgraders.end());
	std::sort(_defenseupgraders.begin(), _defenseupgraders.end(), [this](
			const Descriptor& a, const Descriptor& b){

		// Non-busy should go first.
		return hasOldOrder(a) < hasOldOrder(b);
	});
	int max = std::max(0, (int) _defenseupgraders.size());
	for (int i = 0; i < max && (int) _newOrders.size() < maxMilitaryOrders(); i++)
	{
		upgradeDefenseCreator(_defenseupgraders[i]);

		// Only upgrade a second barracks each turn if we have plenty of money.
		if (_moneyleftover - _moneyreserved < minDefenseUpgradeMoney()) break;
	}
}

void AIRampantRhino::upgradeDefenseCreator(const Descriptor& citydesc)
{
	Cell from = _board.cell(citydesc.position);
	if (_board.ground(from) || isTarget(from.pos())) return;
	const auto& builds = _bible.tileUpgrades(_board.tile(from).type);
	for (const Bible::TileBuild& build : builds)
	{
		if (build.type == TileType::NONE
			&& _board.tile(from).stacks
				>= _bible.tileStacksMax(_board.tile(from).type))
		{
			continue;
		}

		if (2 * build.cost <= _moneyleftover - _moneyreserved)
		{
			_newOrders.emplace_back(Order::Type::UPGRADE, citydesc,
				build.type);
			_moneyleftover -= build.cost;
			return;
		}
	}
}

void AIRampantRhino::upgradeEconomy()
{
	if (_economyupgraders.empty()
		|| _moneystarting < minEconomyUpgradeMoney())
	{
		return;
	}

	_economyupgraders.erase(std::remove_if(_economyupgraders.begin(),
		_economyupgraders.end(), [this](const Descriptor& desc){

			return hasNewOrder(desc);
		}), _economyupgraders.end());
	std::random_shuffle(_economyupgraders.begin(), _economyupgraders.end());
	std::sort(_economyupgraders.begin(), _economyupgraders.end(), [this](
			const Descriptor& a, const Descriptor& b){

		// Non-busy should go first.
		return hasOldOrder(a) < hasOldOrder(b);
	});
	int max = std::max(0, (int) _economyupgraders.size());
	for (int i = 0; i < max && (int) _newOrders.size() < maxOrders(); i++)
	{
		upgradeEconomy(_economyupgraders[i]);

		// Only build a second town each turn if we have plenty of money.
		if (_moneyleftover < minEconomyUpgradeMoney()) break;
	}
}

void AIRampantRhino::upgradeEconomy(const Descriptor& citydesc)
{
	Cell from = _board.cell(citydesc.position);
	if (_board.ground(from) || isTarget(from.pos())) return;
	const auto& builds = _bible.tileUpgrades(_board.tile(from).type);
	for (const Bible::TileBuild& build : builds)
	{
		if (build.type == TileType::NONE
			&& _board.tile(from).stacks
				>= _bible.tileStacksMax(_board.tile(from).type))
		{
			continue;
		}

		if (2 * build.cost <= _moneyleftover)
		{
			_newOrders.emplace_back(Order::Type::UPGRADE, citydesc,
				build.type);
			_moneyleftover -= build.cost;
			return;
		}
	}
}

void AIRampantRhino::controlCultivators()
{
	_cultivators.erase(std::remove_if(_cultivators.begin(),
		_cultivators.end(), [this](const Descriptor& desc){

			return hasNewOrder(desc);
		}), _cultivators.end());
	std::random_shuffle(_cultivators.begin(), _cultivators.end());
	std::sort(_cultivators.begin(), _cultivators.end(), [this](
			const Descriptor& a, const Descriptor& b){

		// Non-busy should go first.
		return hasOldOrder(a) < hasOldOrder(b);
	});
	int max = std::max(0, (int) _cultivators.size());
	for (int i = 0; i < max && (int) _newOrders.size() < maxOrders(); i++)
	{
		controlCultivator(_cultivators[i]);
	}
}

void AIRampantRhino::controlCultivator(const Descriptor& tiledesc)
{
	Cell at = _board.cell(tiledesc.position);
	TileType tiletype = _board.tile(at).type;
	const auto& newtypes = _bible.tileCultivates(tiletype);
	if (newtypes.empty()) return;
	TileType newtype = newtypes[0].type;
	if (_moneyleftover < newtypes[0].cost) return;

	_newOrders.emplace_back(Order::Type::CULTIVATE, tiledesc,
		newtype);
	_moneyleftover -= newtypes[0].cost;
}

void AIRampantRhino::createDefenses()
{
	if (_moneystarting < minDefenseMoney())
	{
		return;
	}

	if (canBuildBarracks())
	{
		if ((rand() % (_defensecreators.size() + 1)) > 2)
		{
			upgradeDefenseCreator();
		}
		else if ((rand() % (_defensecreators.size() + 1)) < 3)
		{
			createBarracks();
		}
	}

	if (_defensecreators.empty())
	{
		return;
	}

	_defensecreators.erase(std::remove_if(_defensecreators.begin(),
		_defensecreators.end(), [this](const Descriptor& desc){

			return hasNewOrder(desc);
		}), _defensecreators.end());
	std::random_shuffle(_defensecreators.begin(), _defensecreators.end());
	std::sort(_defensecreators.begin(), _defensecreators.end(), [this](
			const Descriptor& a, const Descriptor& b){

		// Non-busy should go first.
		return hasOldOrder(a) < hasOldOrder(b);
	});
	int max = std::max(0, (int) _defensecreators.size());
	for (int i = 0; i < max && (int) _newOrders.size() < maxMilitaryOrders(); i++)
	{
		const Descriptor& citydesc = _defensecreators[i];
		Cell from = _board.cell(citydesc.position);
		std::vector<Bible::UnitBuild> prods = _bible.tileProduces(
			_board.tile(from).type);
		if (prods.empty()) continue;
		prods.erase(std::remove_if(prods.begin(), prods.end(),
			[this](const Bible::UnitBuild& build){

				// remove if
				return !_bible.unitCanAttack(build.type);
		}), prods.end());
		if (prods.empty()) continue;
		std::random_shuffle(prods.begin(), prods.end());
		while (_moneyleftover - _moneyreserved < prods.back().cost)
		{
			prods.pop_back();
			if (prods.empty()) break;
		}
		if (prods.empty()) continue;
		UnitType newtype = prods.back().type;

		Descriptor targetdesc;
		int targetscore = 0;
		if (!_board.ground(from) && !isTarget(from.pos()))
		{
			targetdesc = Descriptor::cell(from.pos());
			targetscore = 1;
			if (_bible.tileBinding(_board.tile(from).type))
			{
				targetscore += 1;
			}
		}
		bool attackofopportunity = false;
		if (!_board.ground(from))
		{
			std::vector<Move> dirs = {Move::E, Move::S, Move::W, Move::N};
			for (const Move& move : dirs)
			{
				Cell at = from + move;
				if (at.edge()) continue;

				if (_board.ground(at)
					&& _bible.unitCanAttack(_board.ground(at).type)
					&& _board.ground(at).owner != _player)
				{
					attackofopportunity = true;
				}
			}
		}
		if (!attackofopportunity)
		{
			std::vector<Move> dirs = {Move::E, Move::S, Move::W, Move::N};
			std::random_shuffle(dirs.begin(), dirs.end());
			for (const Move& move : dirs)
			{
				Cell target = from + move;
				if (target.edge()) continue;

				if (!_board.ground(target)
					&& _bible.tileWalkable(_board.tile(target).type)
					&& !isTarget(target.pos()))
				{
					int score = 1;
					if (_bible.tileBinding(_board.tile(target).type))
					{
						score += 1;
					}
					if (score > targetscore)
					{
						targetdesc = Descriptor::cell(target.pos());
						targetscore = score;
					}
				}
			}
		}
		if (targetdesc.type == Descriptor::Type::NONE) continue;

		_newOrders.emplace_back(Order::Type::PRODUCE, citydesc,
			targetdesc,
			newtype);
		_targets.emplace_back(targetdesc.position);
		_moneyleftover -= prods.back().cost;
	}
}

void AIRampantRhino::declareOffenses()
{
	_defenses.erase(std::remove_if(_defenses.begin(),
		_defenses.end(), [this](const Descriptor& desc){

			return hasNewOrder(desc);
		}), _defenses.end());
	std::random_shuffle(_defenses.begin(), _defenses.end());
	std::sort(_defenses.begin(), _defenses.end(), [this](
			const Descriptor& a, const Descriptor& b){

		// Non-busy should go first.
		return hasOldOrder(a) < hasOldOrder(b);
	});
	int max = std::max(0, (int) _defenses.size());
	for (int i = 0; i < max && (int) _newOrders.size() < maxMilitaryOrders(); i++)
	{
		declareOffense(_defenses[i]);
	}
}

void AIRampantRhino::controlDefenses()
{
	_defenses.erase(std::remove_if(_defenses.begin(),
		_defenses.end(), [this](const Descriptor& desc){

			return hasNewOrder(desc);
		}), _defenses.end());
	std::random_shuffle(_defenses.begin(), _defenses.end());
	std::sort(_defenses.begin(), _defenses.end(), [this](
			const Descriptor& a, const Descriptor& b){

		// Non-busy should go first.
		return hasOldOrder(a) < hasOldOrder(b);
	});
	int max = std::max(0, (int) _defenses.size());
	for (int i = 0; i < max && (int) _newOrders.size() < maxMilitaryOrders(); i++)
	{
		controlDefense(_defenses[i]);
	}
}

void AIRampantRhino::controlIdleDefenses()
{
	_defenses.erase(std::remove_if(_defenses.begin(),
		_defenses.end(), [this](const Descriptor& desc){

			return hasNewOrder(desc);
		}), _defenses.end());
	std::random_shuffle(_defenses.begin(), _defenses.end());
	std::sort(_defenses.begin(), _defenses.end(), [this](
			const Descriptor& a, const Descriptor& b){

		// Non-busy should go first.
		return hasOldOrder(a) < hasOldOrder(b);
	});
	int max = std::max(0, (int) _defenses.size());
	for (int i = 0; i < max && (int) _newOrders.size() < maxMilitaryOrders(); i++)
	{
		controlIdleDefense(_defenses[i]);
	}
}

void AIRampantRhino::controlOffenses()
{
	_offenses.erase(std::remove_if(_offenses.begin(),
		_offenses.end(), [this](const Descriptor& desc){

			return hasNewOrder(desc);
		}), _offenses.end());
	std::random_shuffle(_offenses.begin(), _offenses.end());
	std::sort(_offenses.begin(), _offenses.end(), [this](
			const Descriptor& a, const Descriptor& b){

		// Non-busy should go first.
		return hasOldOrder(a) < hasOldOrder(b);
	});
	int max = std::max(0, (int) _offenses.size());
	for (int i = 0; i < max && (int) _newOrders.size() < maxMilitaryOrders(); i++)
	{
		controlOffense(_offenses[i]);
	}
}

void AIRampantRhino::controlCaptors()
{
	_captors.erase(std::remove_if(_captors.begin(),
		_captors.end(), [this](const Descriptor& desc){

			return hasNewOrder(desc);
		}), _captors.end());
	std::random_shuffle(_captors.begin(), _captors.end());
	std::sort(_captors.begin(), _captors.end(), [this](
			const Descriptor& a, const Descriptor& b){

		// Non-busy should go first.
		return hasOldOrder(a) < hasOldOrder(b);
	});
	int max = std::max(0, (int) _captors.size());
	for (int i = 0; i < max && (int) _newOrders.size() < maxCaptures(); i++)
	{
		controlCaptor(_captors[i]);
	}
}

void AIRampantRhino::controlBlockers()
{
	_blockers.erase(std::remove_if(_blockers.begin(),
		_blockers.end(), [this](const Descriptor& desc){

			return hasNewOrder(desc);
		}), _blockers.end());
	std::random_shuffle(_blockers.begin(), _blockers.end());
	std::sort(_blockers.begin(), _blockers.end(), [this](
			const Descriptor& a, const Descriptor& b){

		// Non-busy should go first.
		return hasOldOrder(a) < hasOldOrder(b);
	});
	int max = std::max(0, (int) _blockers.size());
	for (int i = 0; i < max && (int) _newOrders.size() < maxDeblockers(); i++)
	{
		controlBlocker(_blockers[i]);
	}
}

void AIRampantRhino::controlBombarders()
{
	_bombarders.erase(std::remove_if(_bombarders.begin(),
		_bombarders.end(), [this](const Descriptor& desc){

			return hasNewOrder(desc);
		}), _bombarders.end());
	std::random_shuffle(_bombarders.begin(), _bombarders.end());
	std::sort(_bombarders.begin(), _bombarders.end(), [this](
			const Descriptor& a, const Descriptor& b){

		// Non-busy should go first.
		return hasOldOrder(a) < hasOldOrder(b);
	});
	int max = std::max(0, (int) _bombarders.size());
	for (int i = 0; i < max && (int) _newOrders.size() < maxBombardments(); i++)
	{
		controlBombarder(_bombarders[i]);
	}
}

void AIRampantRhino::controlStoppers()
{
	_stoppers.erase(std::remove_if(_stoppers.begin(),
		_stoppers.end(), [this](const Descriptor& desc){

			return hasNewOrder(desc);
		}), _stoppers.end());
	std::random_shuffle(_stoppers.begin(), _stoppers.end());
	// All stoppers have old orders.
	int max = std::max(0, (int) _stoppers.size());
	for (int i = 0; i < max && (int) _newOrders.size() < maxOrders(); i++)
	{
		controlStopper(_stoppers[i]);
	}
}

bool AIRampantRhino::isTarget(const Position& position)
{
	return (std::find(_targets.begin(), _targets.end(), position)
			!= _targets.end());
}

bool AIRampantRhino::hasOldOrder(const Descriptor& unitdesc)
{
	for (const Order& order : _unfinishedOrders)
	{
		if (order.subject == unitdesc) return true;
	}
	return false;
}

bool AIRampantRhino::hasNewOrder(const Descriptor& unitdesc)
{
	for (const Order& order : _newOrders)
	{
		if (order.subject == unitdesc) return true;
	}
	return false;
}

bool AIRampantRhino::checkLockdown(const Descriptor& unitdesc)
{
	Cell at = _board.cell(unitdesc.position);

	bool canUseAbilities = true;
	if (_bible.frostbiteGivesColdFeet()
		// In Spring, frostbite is used to indicate "Chilled" units.
		&& _board.frostbite(at) && _bible.chaosMinFrostbite(_season) < 0)
	{
		canUseAbilities = false;
	}

	std::vector<Move> dirs = {Move::E, Move::S, Move::W, Move::N};
	std::random_shuffle(dirs.begin(), dirs.end());
	for (const Move& move : dirs)
	{
		Cell target = at + move;
		if (target.edge()) continue;

		if (_board.ground(target)
			&& _board.ground(target).owner != _player)
		{
			if (canUseAbilities && canUseCombatAbilities())
			{
				UnitType unittype = _board.ground(at).type;
				std::vector<Bible::TileBuild> shapes = _bible.unitShapes(
					unittype);
				if (std::find_if(shapes.begin(), shapes.end(),
						[this](const Bible::TileBuild& build){

							return (build.type == _trenchestype);
						}) != shapes.end()
					&& _board.tile(at).type != _trenchestype
					&& _bible.tileBuildable(_board.tile(at).type))
				{
					_newOrders.emplace_back(Order::Type::SHAPE, unitdesc,
						_trenchestype);
				}
				else if (_bible.unitCanShell(unittype))
				{
					_newOrders.emplace_back(Order::Type::SHELL, unitdesc,
						Descriptor::cell(target.pos()));
				}
				else if (_bible.unitCanFocus(unittype))
				{
					_newOrders.emplace_back(Order::Type::FOCUS, unitdesc,
						Descriptor::cell(target.pos()));
				}
			}

			return true;
		}
	}

	return false;
}

bool AIRampantRhino::isOccupied(const Position& position)
{
	Cell index = _board.cell(position);
	if (_board.ground(index).type == UnitType::NONE) return false;
	if (_board.tile(index).owner == _board.ground(index).owner) return false;
	return true;
}

void AIRampantRhino::controlIdleDefense(const Descriptor& unitdesc)
{
	if (!canUseCombatAbilities()) return;

	Cell at = _board.cell(unitdesc.position);

	if (_bible.frostbiteGivesColdFeet()
		// In Spring, frostbite is used to indicate "Chilled" units.
		&& _board.frostbite(at) && _bible.chaosMinFrostbite(_season) < 0)
	{
		return;
	}

	std::vector<Move> dirs = {Move::E, Move::S, Move::W, Move::N};
	std::random_shuffle(dirs.begin(), dirs.end());
	for (const Move& move : dirs)
	{
		Cell target = at + move;
		if (target.edge()) continue;

		if (!_board.ground(target)
			&& _bible.tileWalkable(_board.tile(target).type))
		{
			UnitType unittype = _board.ground(at).type;
			if (_bible.unitCanLockdown(unittype))
			{
				_newOrders.emplace_back(Order::Type::LOCKDOWN, unitdesc,
					Descriptor::cell(target.pos()));
				return;
			}
		}
	}
}

void AIRampantRhino::controlCaptor(const Descriptor& unitdesc)
{
	Cell at = _board.cell(unitdesc.position);

	if (_bible.frostbiteGivesColdFeet()
		// In Spring, frostbite is used to indicate "Chilled" units.
		&& _board.frostbite(at) && _bible.chaosMinFrostbite(_season) < 0)
	{
		return;
	}

	if (_board.tile(at)
		&& _board.tile(at).owner != _player
		&& _bible.tileOwnable(_board.tile(at).type))
	{
		_newOrders.emplace_back(Order::Type::CAPTURE, unitdesc);
	}
}

void AIRampantRhino::controlBlocker(const Descriptor& unitdesc)
{
	// Do not check busy to avoid deadlock traffic jams.
	if (checkLockdown(unitdesc)) return;

	Cell from = _board.cell(unitdesc.position);
	const UnitToken& unit = _board.ground(from);

	PathingFloodfill pathing(_bible, _board);
	pathing.put(from);
	pathing.execute();

	int speed = _bible.unitSpeed(unit.type);
	if (speed < 1) return;
	if (_board.snow(from)) speed = std::max(1, speed - 1);
	Cell bestCell = Cell::undefined();
	float bestScore = 0;
	for (Cell at : _board)
	{
		if (at == from) continue;
		if (_board.ground(at)) continue;
		if (_board.gas(at)) continue;
		if (_board.frostbite(at)
			// In Spring, frostbite is used to indicate "Chilled" units.
			&& !(_bible.frostbiteGivesColdFeet()
				&& _bible.chaosMinFrostbite(_season) < 0)) continue;
		if (_board.firestorm(at)) continue;
		if (_board.death(at)) continue;
		if (isTarget(at.pos())) continue;
		int bonus = 0;
		if (_board.tile(at).stacks > 0
			|| _board.tile(at).type == _trenchestype)
		{
			bonus += 1;
		}
		int expected = 100.0f * (1 + bonus);
		int turns = std::max(1, (pathing.steps(at) + speed - 1) / speed);
		float score = 1.0f * expected / (turns + 1)
				+ 0.001f * (rand() % 1000);
		if (score > bestScore)
		{
			bestCell = at;
			bestScore = score;
		}
	}
	if (bestCell == Cell::undefined()) return;
	Cell target = bestCell;

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
	if (length == 0) return;
	else if (length < moves.size())
	{
		moves.resize(length);
		target = at;
	}
	else if (target != at)
	{
		LOGE << "Assertion failure";
		DEBUG_ASSERT(target == at);
	}

	_newOrders.emplace_back(Order::Type::MOVE, unitdesc,
		Descriptor::cell(target.pos()),
		moves);
	_targets.emplace_back(target.pos());
}

void AIRampantRhino::controlDefense(const Descriptor& unitdesc)
{
	if (checkLockdown(unitdesc)) return;

	Cell from = _board.cell(unitdesc.position);
	const UnitToken& unit = _board.ground(from);

	PathingFloodfill pathing(_bible, _board);
	pathing.put(from);
	pathing.execute();

	TileFloodfill enemyThreats(_bible, _board);
	enemyThreats.exclude({_soiltype, _cropstype});
	enemyThreats.exclude({_player, Player::NONE});
	enemyThreats.execute();

	int currentthreatdis = enemyThreats.steps(from);

	TileFloodfill civilians(_bible, _board);
	civilians.include({_player});
	civilians.execute();

	int speed = _bible.unitSpeed(unit.type);
	if (speed < 1) return;
	if (_board.snow(from)) speed = std::max(1, speed - 1);
	Cell bestCell = Cell::undefined();
	float bestScore = 0;
	float currentScore = 0;
	for (Cell at : _board)
	{
		if (at != from && _board.ground(at)
				&& _board.ground(at).owner == _player) continue;
		if (_board.gas(at)) continue;
		if (_board.frostbite(at)
			// In Spring, frostbite is used to indicate "Chilled" units.
			&& !(_bible.frostbiteGivesColdFeet()
				&& _bible.chaosMinFrostbite(_season) < 0)) continue;
		if (_board.firestorm(at)) continue;
		if (_board.death(at)) continue;
		if (isTarget(at.pos())) continue;
		int threatdis = enemyThreats.steps(at);
		int civiesdis = civilians.steps(at);
		if (threatdis <= civiesdis) continue;
		if (threatdis > currentthreatdis + 2) continue;
		if (civiesdis > 3) continue;
		int bonus = 0;
		if (civiesdis == 0)
		{
			if (_bible.tileBinding(_board.tile(at).type)) bonus += 10;
			else if (_board.tile(at).type == _outposttype) bonus += 1;
			else bonus += 3;
		}
		else if (civiesdis >= 2)
		{
			bonus += 1;
		}
		const UnitToken& enemy = _board.ground(at);
		if (_bible.unitCanAttack(unit.type)
				&& enemy && enemy.owner != _player
				&& (unit.stacks * _bible.unitAttackShots(unit.type)
								* _bible.unitAttackDamage(unit.type)
						>= _bible.unitHitpoints(enemy.type)))
		{
			bonus += 2;
		}
		if (_board.tile(at).stacks > 0
			|| _board.tile(at).type == _trenchestype)
		{
			if (at == from) bonus += 3;
			else bonus += 1;
		}
		int expected = 100.0f * (4 + bonus) / 5;
		int turns = std::max(1, (pathing.steps(at) + speed - 1) / speed);
		float score = 1.0f * expected / (3 * turns + 1)
				+ 0.001f * (rand() % 1000);
		if (at == from)
		{
			currentScore = score;
		}
		else if (score > bestScore)
		{
			bestCell = at;
			bestScore = score;
		}
	}
	if (bestCell == Cell::undefined()) return;
	if (bestScore < currentScore) return;
	Cell target = bestCell;
	if (target == from) return;

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
	if (length == 0) return;
	else if (length < moves.size())
	{
		moves.resize(length);
		target = at;
	}
	else if (target != at)
	{
		LOGE << "Assertion failure";
		DEBUG_ASSERT(target == at);
	}

	_newOrders.emplace_back(Order::Type::MOVE, unitdesc,
		Descriptor::cell(target.pos()),
		moves);
	_targets.emplace_back(target.pos());
}

void AIRampantRhino::declareOffense(const Descriptor& unitdesc)
{
	if (checkLockdown(unitdesc)) return;

	Cell from = _board.cell(unitdesc.position);
	const UnitToken& unit = _board.ground(from);

	PathingFloodfill pathing(_bible, _board);
	pathing.put(from);
	pathing.execute();

	TileFloodfill enemyThreats(_bible, _board);
	enemyThreats.exclude({_soiltype, _cropstype});
	enemyThreats.exclude({_player, Player::NONE});
	enemyThreats.execute();

	int currentthreatdis = enemyThreats.steps(from);

	TileFloodfill civilians(_bible, _board);
	civilians.include({_player});
	civilians.execute();

	UnitType unittype = unit.type;
	bool canCapture = _bible.unitCanCapture(unittype);
	bool canOccupy = _bible.unitCanOccupy(unittype);
	bool hasAbility = (_bible.unitAbilityVolleys(unittype) > 0);
	int rangeMin = _bible.unitRangeMin(unittype);
	int rangeMax = _bible.unitRangeMax(unittype);

	int speed = _bible.unitSpeed(unittype);
	if (speed < 1) return;
	if (_board.snow(from)) speed = std::max(1, speed - 1);
	Cell bestCell = Cell::undefined();
	float bestScore = 0;
	float currentScore = 0;
	for (Cell at : _board)
	{
		if (at != from && _board.ground(at)
				&& _board.ground(at).owner == _player) continue;
		if (_board.gas(at)) continue;
		if (_board.frostbite(at)
			// In Spring, frostbite is used to indicate "Chilled" units.
			&& !(_bible.frostbiteGivesColdFeet()
				&& _bible.chaosMinFrostbite(_season) < 0)) continue;
		if (_board.firestorm(at)) continue;
		if (_board.death(at)) continue;
		if (isTarget(at.pos())) continue;
		int threatdis = enemyThreats.steps(at);
		int civiesdis = civilians.steps(at);
		if (threatdis > currentthreatdis) continue;
		if (civiesdis == 0) continue;
		int bonus = 0;
		if (_bible.tileBinding(_board.tile(at).type))
		{
			if (canCapture) bonus += 12;
			else if (canOccupy) bonus += 4;
			else bonus += -5;
		}
		else if (threatdis == 0)
		{
			if (canCapture) bonus += 6;
			else if (canOccupy) bonus += 1;
			else bonus += -2;
		}
		const UnitToken& enemy = _board.ground(at);
		if (_bible.unitCanAttack(unit.type)
				&& enemy && enemy.owner != _player
				&& (unit.stacks * _bible.unitAttackShots(unit.type)
								* _bible.unitAttackDamage(unit.type)
						>= _bible.unitHitpoints(enemy.type)))
		{
			bonus += 1;
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
					int val = 0;
					if (tiletoken.type == _outposttype) val = 1;
					else if (tiletoken.type == _farmtype) val = 2;
					else if (tiletoken.type == _towntype) val = 4;
					else val = 8;
					if (maxval < val) maxval = val;
				}
			}
			bonus += maxval;
		}
		if (_board.tile(at).stacks > 0
			|| _board.tile(at).type == _trenchestype)
		{
			bonus += 1;
		}
		int expected = 100.0f * (6 + bonus) / (4 + threatdis);
		int turns = std::max(1, (pathing.steps(at) + speed - 1) / speed);
		float score = 1.0f * expected / (2 + turns)
				+ 0.001f * (rand() % 1000);
		if (at == from)
		{
			currentScore = score;
		}
		else if (score > bestScore)
		{
			bestCell = at;
			bestScore = score;
		}
	}
	if (bestCell == Cell::undefined()) return;
	if (bestScore < currentScore) return;
	Cell target = bestCell;
	if (target == from) return;

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

	Cell at = from;
	size_t length = 0;
	for (; length < moves.size(); length++)
	{
		bool stop = false;
		for (const Move& dir : {Move::E, Move::S, Move::W, Move::N})
		{
			if (dir == moves[length] || ::flip(dir) == moves[length]) continue;

			Cell to = at + dir;
			if (to.edge()) continue;

			const UnitToken& enemy = _board.ground(to);
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
	if (length == 0) return;
	else if (length < moves.size())
	{
		moves.resize(length);
		target = at;
	}
	else if (target != at)
	{
		LOGE << "Assertion failure";
		DEBUG_ASSERT(target == at);
	}

	_newOrders.emplace_back(Order::Type::MOVE, unitdesc,
		Descriptor::cell(target.pos()),
		moves);
	_targets.emplace_back(target.pos());
}

void AIRampantRhino::controlOffense(const Descriptor& unitdesc)
{
	checkLockdown(unitdesc);
}

void AIRampantRhino::controlBombarder(const Descriptor& unitdesc)
{
	// Do not check lockdown because bombarding is better.

	Cell from = _board.cell(unitdesc.position);
	const UnitToken& unit = _board.ground(from);

	if (_bible.frostbiteGivesColdFeet()
		// In Spring, frostbite is used to indicate "Chilled" units.
		&& _board.frostbite(from) && _bible.chaosMinFrostbite(_season) < 0)
	{
		return;
	}

	UnitType unittype = unit.type;
	int rangeMin = _bible.unitRangeMin(unittype);
	int rangeMax = _bible.unitRangeMax(unittype);
	int threshold = _bible.unitAbilityDamage(unittype);

	Cell bestCell = Cell::undefined();
	float bestScore = 0;
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
		float score = (50.0f + 50.0f * _board.current(target)) * expected
				+ 0.001f * (rand() % 1000);
		if (score > bestScore)
		{
			bestCell = target;
			bestScore = score;
		}
	}
	if (bestCell == Cell::undefined()) return;
	Cell target = bestCell;

	_newOrders.emplace_back(Order::Type::BOMBARD, unitdesc,
		Descriptor::cell(target.pos()));
}

void AIRampantRhino::controlStopper(const Descriptor& tiledesc)
{
	_newOrders.emplace_back(Order::Type::HALT, tiledesc);
}

int AIRampantRhino::maxOrders()
{
	switch (_difficulty)
	{
		case Difficulty::NONE: return 0;
		case Difficulty::EASY: return 2;
		case Difficulty::MEDIUM: return 5;
		case Difficulty::HARD: return 5;
	}
	return 0;
}

int AIRampantRhino::maxMilitaryOrders()
{
	switch (_difficulty)
	{
		case Difficulty::NONE: return 0;
		case Difficulty::EASY: return 1;
		case Difficulty::MEDIUM: return 3;
		case Difficulty::HARD: return 4;
	}
	return 0;
}

int AIRampantRhino::maxMilitaryUnits()
{
	switch (_difficulty)
	{
		case Difficulty::NONE: return 0;
		case Difficulty::EASY: return 4;
		case Difficulty::MEDIUM: return 6;
		case Difficulty::HARD: return 10;
	}
	return 0;
}

int AIRampantRhino::maxDefenseUnits()
{
	switch (_difficulty)
	{
		case Difficulty::NONE: return 0;
		case Difficulty::EASY: return 4;
		case Difficulty::MEDIUM: return 4;
		case Difficulty::HARD: return 4;
	}
	return 0;
}

int AIRampantRhino::maxCaptures()
{
	switch (_difficulty)
	{
		case Difficulty::NONE: return 0;
		case Difficulty::EASY: return 1;
		case Difficulty::MEDIUM: return 2;
		case Difficulty::HARD: return 2;
	}
	return 0;
}

int AIRampantRhino::maxDeblockers()
{
	switch (_difficulty)
	{
		case Difficulty::NONE: return 0;
		case Difficulty::EASY: return 1;
		case Difficulty::MEDIUM: return 1;
		case Difficulty::HARD: return 1;
	}
	return 0;
}

int AIRampantRhino::maxBombardments()
{
	switch (_difficulty)
	{
		case Difficulty::NONE: return 0;
		case Difficulty::EASY: return 0;
		case Difficulty::MEDIUM: return 1;
		case Difficulty::HARD: return 1;
	}
	return 0;
}

int AIRampantRhino::minDefenseMoney()
{
	switch (_difficulty)
	{
		case Difficulty::NONE:   return 10 * _settlingmoney;
		case Difficulty::EASY:   return  4 * _settlingmoney;
		case Difficulty::MEDIUM: return  2 * _settlingmoney;
		case Difficulty::HARD:   return  1 * _settlingmoney;
	}
	return 0;
}

int AIRampantRhino::minSettlersMoney()
{
	switch (_difficulty)
	{
		case Difficulty::NONE:   return 2 * _settlingmoney;
		case Difficulty::EASY:   return 2 * _settlingmoney;
		case Difficulty::MEDIUM: return 2 * _settlingmoney;
		case Difficulty::HARD:   return 2 * _settlingmoney;
	}
	return 0;
}

int AIRampantRhino::minBarracksMoney()
{
	switch (_difficulty)
	{
		case Difficulty::NONE:   return 3 * _settlingmoney;
		case Difficulty::EASY:   return 3 * _settlingmoney;
		case Difficulty::MEDIUM: return 3 * _settlingmoney;
		case Difficulty::HARD:   return 3 * _settlingmoney;
	}
	return 0;
}

int AIRampantRhino::minDefenseUpgradeMoney()
{
	switch (_difficulty)
	{
		case Difficulty::NONE:   return 6 * _settlingmoney;
		case Difficulty::EASY:   return 6 * _settlingmoney;
		case Difficulty::MEDIUM: return 6 * _settlingmoney;
		case Difficulty::HARD:   return 6 * _settlingmoney;
	}
	return 0;
}

int AIRampantRhino::minEconomyUpgradeMoney()
{
	switch (_difficulty)
	{
		case Difficulty::NONE:   return 2 * _settlingmoney;
		case Difficulty::EASY:   return 2 * _settlingmoney;
		case Difficulty::MEDIUM: return 2 * _settlingmoney;
		case Difficulty::HARD:   return 2 * _settlingmoney;
	}
	return 0;
}

bool AIRampantRhino::canUseCombatAbilities()
{
	switch (_difficulty)
	{
		case Difficulty::NONE:   return false;
		case Difficulty::EASY:   return false;
		case Difficulty::MEDIUM: return true;
		case Difficulty::HARD:   return true;
	}
	return false;
}

bool AIRampantRhino::canBuildBarracks()
{
	switch (_difficulty)
	{
		case Difficulty::NONE:   return false;
		case Difficulty::EASY:   return true;
		case Difficulty::MEDIUM: return true;
		case Difficulty::HARD:   return true;
	}
	return false;
}

bool AIRampantRhino::canBuildCities()
{
	switch (_difficulty)
	{
		case Difficulty::NONE:   return false;
		case Difficulty::EASY:   return false;
		case Difficulty::MEDIUM: return true;
		case Difficulty::HARD:   return true;
	}
	return false;
}

int AIRampantRhino::expectedSoil(Cell index)
{
	int expected = 0;
	for (Cell at : _board.area(index, 1, 2))
	{
		// Only some tiles (grass, dirt) can be built on.
		if (!_bible.tileBuildable(_board.tile(at).type)) continue;
		// If there is a ground unit occupying the tile, we cannot build on it.
		if (_board.ground(at)) continue;
		// If the tile is owned by the player, we do not build on it.
		if (_board.tile(at).owner == _player) continue;

		expected++;
	}
	return expected;
}

int AIRampantRhino::expectedNiceness(Cell index)
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
