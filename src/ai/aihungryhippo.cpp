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
#include "aihungryhippo.hpp"
#include "source.hpp"

#include "pathingfloodfill.hpp"
#include "difficulty.hpp"


AIHungryHippo::Tile AIHungryHippo::makeTile(Cell index) const
{
	Tile tile;
	tile.descriptor = Descriptor::tile(index.pos());
	tile.stacks = _board.tile(index).stacks;
	tile.power = _board.tile(index).power;
	tile.unfinished = hasUnfinished(tile.descriptor);
	tile.occupied = (_board.ground(index)
		&& _board.ground(index).owner != _board.tile(index).owner);
	return tile;
}

AIHungryHippo::Ground AIHungryHippo::makeGround(Cell index) const
{
	Ground ground;
	ground.descriptor = Descriptor::ground(index.pos());
	ground.stacks = _board.ground(index).stacks;
	ground.unfinished = hasUnfinished(ground.descriptor);
	return ground;
}

bool AIHungryHippo::cityOccupied(Cell index) const
{
	if (!_bible.tileBinding(_board.tile(index).type)) return false;
	if (_board.ground(index).type == UnitType::NONE) return false;
	if (_board.tile(index).owner == _board.ground(index).owner) return false;
	return true;
}

int8_t AIHungryHippo::cityNiceness(Cell index)
{
	int niceness = 0;
	for (Cell at : _board.area(index, 1, 2))
	{
		const TileToken& tile = _board.tile(at);
		if (_bible.tileGrassy(tile.type))
		{
			niceness += 2;
		}
		else if (_bible.tileNatural(tile.type))
		{
			niceness += 1;
		}
	}
	return (niceness / 2);
}

int AIHungryHippo::expectedSoil(Cell index)
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

AIHungryHippo::AIHungryHippo(const Player& player, const Difficulty& difficulty,
		const std::string& rulesetname, char character) :
	AICommander(player, difficulty, rulesetname, character)
{
	if (_difficulty == Difficulty::NONE)
	{
		LOGW << "AI difficulty not set";
		DEBUG_ASSERT(false);
	}

	_citytype = _bible.tiletype("city");
	if (_citytype == TileType::NONE)
	{
		LOGE << "Missing type 'city'";
		DEBUG_ASSERT(false);
	}
	_towntype = _bible.tiletype("town");
	if (_towntype == TileType::NONE)
	{
		LOGE << "Missing type 'town'";
		DEBUG_ASSERT(false);
	}
	_outposttype = _bible.tiletype("outpost");
	if (_outposttype == TileType::NONE)
	{
		LOGE << "Missing type 'outpost'";
		DEBUG_ASSERT(false);
	}
	_industrytype = _bible.tiletype("industry");
	if (_industrytype == TileType::NONE)
	{
		LOGE << "Missing type 'industry'";
		DEBUG_ASSERT(false);
	}
	_barrackstype = _bible.tiletype("barracks");
	if (_barrackstype == TileType::NONE)
	{
		LOGE << "Missing type 'barracks'";
		DEBUG_ASSERT(false);
	}
	_farmtype = _bible.tiletype("farm");
	if (_farmtype == TileType::NONE)
	{
		LOGE << "Missing type 'farm'";
		DEBUG_ASSERT(false);
	}
	_soiltype = _bible.tiletype("soil");
	if (_soiltype == TileType::NONE)
	{
		LOGE << "Missing type 'soil'";
		DEBUG_ASSERT(false);
	}
	_cropstype = _bible.tiletype("crops");
	if (_cropstype == TileType::NONE)
	{
		LOGE << "Missing type 'crops'";
		DEBUG_ASSERT(false);
	}
	_settlertype = _bible.unittype("settler");
	if (_settlertype == UnitType::NONE)
	{
		LOGE << "Missing type 'settler'";
		DEBUG_ASSERT(false);
	}
	_militiatype = _bible.unittype("militia");
	if (_militiatype == UnitType::NONE)
	{
		LOGE << "Missing type 'militia'";
		DEBUG_ASSERT(false);
	}
	_riflemantype = _bible.unittype("rifleman");
	if (_riflemantype == UnitType::NONE)
	{
		LOGE << "Missing type 'rifleman'";
		DEBUG_ASSERT(false);
	}
	_tanktype = _bible.unittype("tank");
	if (_tanktype == UnitType::NONE)
	{
		LOGE << "Missing type 'tank'";
		DEBUG_ASSERT(false);
	}
	_gunnertype = _bible.unittype("gunner");
	if (_gunnertype == UnitType::NONE)
	{
		LOGE << "Missing type 'gunner'";
		DEBUG_ASSERT(false);
	}
	_sappertype = _bible.unittype("sapper");
	if (_sappertype == UnitType::NONE)
	{
		LOGE << "Missing type 'sapper'";
		DEBUG_ASSERT(false);
	}

	// Arbitrary number bigger than maxMoney but where +20 does not overflow.
	uint16_t NOTFOUND = 40404;
	_barracksCost = NOTFOUND;
	for (const Bible::TileBuild& build : _bible.tileExpands(_citytype))
	{
		if (build.type != _barrackstype) continue;
		_barracksCost = build.cost;
		break;
	}
	_industryCost = NOTFOUND;
	for (const Bible::TileBuild& build : _bible.tileExpands(_citytype))
	{
		if (build.type != _industrytype) continue;
		_industryCost = build.cost;
		break;
	}
	_tankCost = NOTFOUND;
	for (const Bible::UnitBuild& build : _bible.tileProduces(_industrytype))
	{
		if (build.type != _tanktype) continue;
		_tankCost = build.cost;
		break;
	}
	_riflemanCost = NOTFOUND;
	for (const Bible::UnitBuild& build : _bible.tileProduces(_citytype))
	{
		if (build.type != _riflemantype) continue;
		_riflemanCost = build.cost;
		break;
	}
	_militiaCost = NOTFOUND;
	for (const Bible::UnitBuild& build : _bible.tileProduces(_farmtype))
	{
		if (build.type != _militiatype) continue;
		_militiaCost = build.cost;
		break;
	}
	_barracksUpgradeCost = NOTFOUND;
	for (const Bible::TileBuild& build : _bible.tileUpgrades(_barrackstype))
	{
		if (build.type != TileType::NONE) continue;
		_barracksUpgradeCost = build.cost;
		break;
	}
	_industryUpgradeCost = NOTFOUND;
	for (const Bible::TileBuild& build : _bible.tileUpgrades(_industrytype))
	{
		if (build.type != TileType::NONE) continue;
		_industryUpgradeCost = build.cost;
		break;
	}
}

void AIHungryHippo::determineState()
{
	// clear our vectors
	_options.clear();
	_myCities.clear();
	_myTowns.clear();
	_myIndustry.clear();
	_myBarracks.clear();
	_myFarms.clear();
	_mySoil.clear();
	_myCrops.clear();
	_mySettlers.clear();
	_myMilitia.clear();
	_myRiflemen.clear();
	_myTanks.clear();
	_enemyGround.clear();
	_queuedIndustry = 0;
	_queuedBarracks = 0;
	_queuedSettlers = 0;
	_queuedRiflemen = 0;
	_queuedTanks = 0;
	_queuedPlows = 0;

	// fill our vectors
	for (Cell index : _board)
	{
		// tiles
		if (_board.tile(index).owner == _player)
		{
			if (_board.tile(index).type == _citytype)
			{
				_myCities.emplace_back(makeTile(index));
			}
			else if (_board.tile(index).type == _towntype)
			{
				_myTowns.emplace_back(makeTile(index));
			}
			else if (_board.tile(index).type == _industrytype)
			{
				_myIndustry.emplace_back(makeTile(index));
			}
			else if (_board.tile(index).type == _barrackstype)
			{
				_myBarracks.emplace_back(makeTile(index));
			}
			else if (_board.tile(index).type == _farmtype)
			{
				_myFarms.emplace_back(makeTile(index));
			}
			else if (_board.tile(index).type == _soiltype)
			{
				_mySoil.emplace_back(makeTile(index));
			}
			else if (_board.tile(index).type == _cropstype)
			{
				_myCrops.emplace_back(makeTile(index));
			}
		}

		// units
		if (_board.ground(index).owner == _player)
		{
			if (_board.ground(index).type == _settlertype)
			{
				_mySettlers.emplace_back(makeGround(index));
			}
			else if (_board.ground(index).type == _militiatype)
			{
				_myMilitia.emplace_back(makeGround(index));
			}
			else if (_board.ground(index).type == _riflemantype)
			{
				_myRiflemen.emplace_back(makeGround(index));
			}
			else if (_board.ground(index).type == _tanktype)
			{
				_myTanks.emplace_back(makeGround(index));
			}
		}
		else if (_board.ground(index).type != UnitType::NONE)
		{
			_enemyGround.emplace_back(makeGround(index));
		}
	}

	// determine queued
	for (Order& order : _unfinishedOrders)
	{
		if (order.type == Order::Type::PRODUCE
			&& order.unittype == _settlertype) _queuedSettlers++;
		else if (order.type == Order::Type::PRODUCE
			&& order.unittype == _militiatype) _queuedMilitia++;
		else if (order.type == Order::Type::PRODUCE
			&& order.unittype == _riflemantype) _queuedRiflemen++;
		else if (order.type == Order::Type::PRODUCE
			&& order.unittype == _tanktype) _queuedTanks++;
		else if (order.type == Order::Type::EXPAND
			&& order.tiletype == _industrytype) _queuedIndustry++;
		else if (order.type == Order::Type::EXPAND
			&& order.tiletype == _barrackstype) _queuedBarracks++;
		else if (order.type == Order::Type::CULTIVATE) _queuedPlows++;
	}
}

void AIHungryHippo::doFarming()
{
	if (_myFarms.size() > 0)
	{
		std::vector<Tile> farms = _myFarms;
		std::random_shuffle(farms.begin(), farms.end());
		std::sort(farms.begin(), farms.end(),
			[&](const Tile& lhs, const Tile& rhs) {

				return expectedSoil(_board.cell(lhs.descriptor.position))
					> expectedSoil(_board.cell(rhs.descriptor.position));
			});
		if (expectedSoil(_board.cell(farms[0].descriptor.position)) > 0)
		{
			Order order(Order::Type::CULTIVATE, farms[0].descriptor,
				_soiltype);
			_options.emplace_back(Option{order, 20});
			_queuedPlows++;
			return;
		}
	}

	// build a farm if we have less than 5 soil and no plows already queued
	if (_mySoil.size() + _myCrops.size() >= 5 || _queuedPlows > 0) return;

	if (_mySettlers.size() > 0)
	{
		bool wait = false;
		for (Ground& settler : _mySettlers)
		{
			if (settler.unfinished.type != Order::Type::NONE)
			{
				wait = true;
			}
			else if (_bible.tileBuildable(
				_board.tile(_board.cell(settler.descriptor.position)).type))
			{
				Order order(Order::Type::SETTLE, settler.descriptor,
					_farmtype);
				_options.emplace_back(Option{order, 20});
				wait = true;
			}
		}
		if (wait) return;

		Cell bestCell = Cell::undefined();
		float bestExpected = 0;
		for (Cell index : _board)
		{
			if (!_bible.tileBuildable(_board.tile(index).type)) continue;
			TileFloodfill citiesTowns(_bible, _board);
			citiesTowns.include({_citytype, _towntype});
			citiesTowns.include({_player});
			citiesTowns.execute();
			if (citiesTowns.steps(index) <= 3
				|| citiesTowns.steps(index) > 6) continue;
			TileFloodfill enemyThreats(_bible, _board);
			enemyThreats.exclude({_soiltype, _cropstype});
			enemyThreats.exclude({_player, Player::NONE});
			enemyThreats.execute();
			if (enemyThreats.steps(index) <= 5) continue;
			float expected = expectedSoil(index)
					+ 0.001f * (rand() % 1000);
			if (bestExpected < expected)
			{
				bestCell = index;
				bestExpected = expected;
			}
		}
		if (bestCell != Cell::undefined())
		{
			UnitFloodfill settlers(_bible, _board);
			settlers.include({_settlertype});
			settlers.include({_player});
			settlers.execute();
			if (settlers.reached(bestCell) && settlers.steps(bestCell) > 0)
			{
				std::vector<Move> moves;
				Move current;
				Cell origin = bestCell;
				while ((current = settlers.step(origin)) != Move::X)
				{
					moves.emplace_back(flip(current));
					origin = origin + current;
				}
				std::reverse(moves.begin(), moves.end());
				Order order(Order::Type::MOVE,
					Descriptor::ground(origin.pos()),
					Descriptor::cell(bestCell.pos()), moves);
				_options.emplace_back(Option{order, 20});
			}
		}
	}
	else if (_queuedSettlers == 0)
	{
		std::vector<Tile> settlerProducers;
		settlerProducers.insert(settlerProducers.end(), _myFarms.begin(),
			_myFarms.end());
		settlerProducers.insert(settlerProducers.end(), _myTowns.begin(),
			_myTowns.end());
		settlerProducers.insert(settlerProducers.end(), _myCities.begin(),
			_myCities.end());
		std::random_shuffle(settlerProducers.begin(), settlerProducers.end());
		for (Tile& tile : settlerProducers)
		{
			if (tile.unfinished.type != Order::Type::NONE) continue;
			if (tile.power < 1 && !_bible.powerAbolished()) continue;
			Order order(Order::Type::PRODUCE, tile.descriptor,
				Descriptor::cell(tile.descriptor.position),
				_settlertype);
			_options.emplace_back(Option{order, 20});
			_queuedSettlers++;
			break;
		}
	}
}

std::string AIHungryHippo::ainame() const
{
	return "HungryHippo";
}

std::string AIHungryHippo::authors() const
{
	return "Daan Mulder";
}

void AIHungryHippo::process()
{
	determineState();

	doFarming();

	// capture anything!
	for (Ground& rifleman : _myRiflemen)
	{
		Cell at = _board.cell(rifleman.descriptor.position);
		if (!_bible.tileOwnable(_board.tile(at).type)) continue;
		if (_board.tile(at).type == _soiltype) continue;
		if (_board.tile(at).type == _cropstype) continue;
		if (_board.tile(at).owner == _player) continue;
		Order order(Order::Type::CAPTURE, rifleman.descriptor);
		_options.emplace_back(Option{order, 25});
	}

	// focus if we get the chance!
	for (Ground& enemyUnit : _enemyGround)
	{
		Cell from = _board.cell(enemyUnit.descriptor.position);
		std::vector<Ground> surrounding;
		for (const Move& move : {Move::E, Move::S, Move::W, Move::N})
		{
			Cell to = from + move;
			if (to.edge()) continue;
			if (_board.ground(to).owner != _player) continue;
			if (   _board.ground(to).type != _militiatype
				&& _board.ground(to).type != _riflemantype
				&& _board.ground(to).type != _gunnertype
				&& _board.ground(to).type != _sappertype) continue;
			surrounding.emplace_back(makeGround(to));
		}
		if (surrounding.size() < 2) continue;
		std::random_shuffle(surrounding.begin(), surrounding.end());
		for (auto& myUnit : surrounding)
		{
			Order order(Order::Type::FOCUS, myUnit.descriptor,
				Descriptor::cell(enemyUnit.descriptor.position));
			int priority = myUnit.stacks * int(surrounding.size());
			if (myUnit.unfinished.type == Order::Type::NONE) priority *= 2;
			_options.emplace_back(Option{order, priority});
		}
	}

	// bombard if next to enemy!
	for (Ground& tank : _myTanks)
	{
		Cell from = _board.cell(tank.descriptor.position);
		std::vector<Ground> surrounding;
		for (const Move& move : {Move::E, Move::S, Move::W, Move::N})
		{
			Cell to = from + move;
			if (to.edge()) continue;

			enum Owner {ME, ENEMY, NONE};
			Owner tile = ENEMY;
			if (_board.tile(to).owner == _player) tile = ME;
			else if (_board.tile(to).owner == Player::NONE) tile = NONE;
			Owner ground = ENEMY;
			if (_board.ground(to).owner == _player) ground = ME;
			else if (_board.ground(to).owner == Player::NONE) ground = NONE;

			if (tile != ENEMY && ground != ENEMY) continue;
			if (ground == ME) continue;
			if (tile == ME
				// so (ground == ENEMY)
				&& _board.tile(to).type != _soiltype
				&& _board.tile(to).type != _cropstype) continue;
			if (ground != ENEMY)
				// so (tile == ENEMY)
			{
				if (_board.tile(to).type == _soiltype) continue;
				if (_board.tile(to).type == _cropstype) continue;
			}

			surrounding.emplace_back(makeGround(to));
		}
		if (surrounding.size() < 1) continue;
		std::random_shuffle(surrounding.begin(), surrounding.end());
		for (auto& enemyUnit : surrounding)
		{
			Order order(Order::Type::SHELL, tank.descriptor,
				Descriptor::cell(enemyUnit.descriptor.position));
			_options.emplace_back(Option{order, 12});
			break;
		}
	}

	// move riflemen to targets!
	TileFloodfill targets(_bible, _board);
	targets.include(
		{_towntype, _outposttype, _farmtype, _barrackstype, _industrytype});
	targets.exclude({_player});
	targets.execute();
	for (Ground& rifleman : _myRiflemen)
	{
		Cell target = _board.cell(rifleman.unfinished.target.position);
		if (rifleman.unfinished.type == Order::Type::MOVE
			&& _board.tile(target).owner != _player
			&& (_board.tile(target).type == _towntype
			|| _board.tile(target).type == _farmtype
			|| _board.tile(target).type == _industrytype)) continue;
		Cell destination = _board.cell(rifleman.descriptor.position);
		if (!targets.reached(destination)) continue;
		if (targets.steps(destination) == 0) continue;
		std::vector<Move> moves;
		Move current;
		while ((current = targets.step(destination)) != Move::X)
		{
			moves.emplace_back(current);
			destination = destination + current;
		}
		Order order(Order::Type::MOVE, rifleman.descriptor,
			Descriptor::cell(destination.pos()), moves);
		_options.emplace_back(Option{order, 10 - int(moves.size())});
	}

	// move tanks next to cities!
	TileFloodfill cities(_bible, _board);
	cities.include({_citytype});
	cities.exclude({_player});
	cities.excludeOccupied();
	cities.execute();
	for (Ground& tank : _myTanks)
	{
		if (tank.unfinished.type != Order::Type::NONE) continue;
		Cell destination = _board.cell(tank.descriptor.position);
		if (!cities.reached(destination)) continue;
		if (cities.steps(destination) <= 1) continue;
		std::vector<Move> moves;
		Move current;
		Cell prev = destination;
		while ((current = cities.step(destination)) != Move::X)
		{
			moves.emplace_back(current);
			prev = destination;
			destination = destination + current;
		}
		if (moves.size() <= 1) continue;
		moves.pop_back();
		Order order(Order::Type::MOVE, tank.descriptor,
			Descriptor::cell(prev.pos()), moves);
		_options.emplace_back(Option{order, 15 - int(moves.size())});
	}

	// move militia to cities!
	for (Ground& militia : _myMilitia)
	{
		Cell target = _board.cell(militia.unfinished.target.position);
		if (militia.unfinished.type != Order::Type::NONE
			&& (militia.unfinished.type != Order::Type::MOVE
			|| !cityOccupied(target))) continue;
		Cell destination = _board.cell(militia.descriptor.position);
		if (cityOccupied(destination)) continue;
		if (!cities.reached(destination)) continue;
		if (cities.steps(destination) == 0) continue;
		std::vector<Move> moves;
		Move current;
		while ((current = cities.step(destination)) != Move::X)
		{
			moves.emplace_back(current);
			destination = destination + current;
		}
		Order order(Order::Type::MOVE, militia.descriptor,
			Descriptor::cell(destination.pos()), moves);
		_options.emplace_back(Option{order, 15 - int(moves.size())});
	}

	// make more barracks!
	for (Tile& city : _myCities)
	{
		Cell at = _board.cell(city.descriptor.position);
		if (_money < _barracksCost) break;
		if (city.unfinished.type != Order::Type::NONE) continue;
		if (city.power == 0 && !_bible.powerAbolished()) continue;
		if (city.occupied) continue;
		if (cityNiceness(at) < 3) continue;
		if (_myBarracks.size() + _queuedBarracks >= 2) continue;
		std::vector<Move> directions = {Move::E, Move::S, Move::W, Move::N};
		std::random_shuffle(directions.begin(), directions.end());
		for (const Move& move : directions)
		{
			Cell to = at + move;
			if (to.edge()) continue;
			if (_board.ground(to)) continue;
			if (!_bible.tileBuildable(_board.tile(to).type)) continue;
			Order order(Order::Type::EXPAND, city.descriptor,
				Descriptor::cell(to.pos()), _barrackstype);
			_options.emplace_back(Option{order, 3 + cityNiceness(at)});
			_queuedBarracks++;
			break;
		}
	}

	// make more industry!
	for (Tile& city : _myCities)
	{
		Cell at = _board.cell(city.descriptor.position);
		if (_money < _industryCost) break;
		if (city.unfinished.type != Order::Type::NONE) continue;
		if (city.power == 0 && !_bible.powerAbolished()) continue;
		if (city.occupied) continue;
		if (cityNiceness(at) < 3) continue;
		if (_myIndustry.size() + _queuedIndustry >= 2) continue;
		std::vector<Move> directions = {Move::E, Move::S, Move::W, Move::N};
		std::random_shuffle(directions.begin(), directions.end());
		for (const Move& move : directions)
		{
			Cell to = at + move;
			if (to.edge()) continue;
			if (_board.ground(to)) continue;
			if (!_bible.tileBuildable(_board.tile(to).type)) continue;
			Order order(Order::Type::EXPAND, city.descriptor,
				Descriptor::cell(to.pos()), _industrytype);
			_options.emplace_back(Option{order, 1 + cityNiceness(at)});
			_queuedIndustry++;
			break;
		}
	}

	// make more riflemen!
	for (Tile& barracks : _myBarracks)
	{
		if (_money < _riflemanCost) break;
		int size = _bible.powerAbolished() ? barracks.stacks : barracks.power;
		if (size == 0) continue;
		if (barracks.occupied) continue;
		if (barracks.unfinished.type != Order::Type::NONE) continue;
		Order order(Order::Type::PRODUCE, barracks.descriptor,
			Descriptor::cell(barracks.descriptor.position),
			_riflemantype);
		_options.emplace_back(Option{order, 4 * size * size});
		_queuedRiflemen++;
	}

	// make more tanks!
	for (Tile& industry : _myIndustry)
	{
		if (_money < _tankCost) break;
		int size = _bible.powerAbolished() ? industry.stacks : industry.power;
		if (size == 0) continue;
		if (industry.occupied) continue;
		if (industry.unfinished.type != Order::Type::NONE) continue;
		if (_myTanks.size() + _queuedTanks >= 2 * _myIndustry.size()) continue;
		Order order(Order::Type::PRODUCE, industry.descriptor,
			Descriptor::cell(industry.descriptor.position),
			_tanktype);
		_options.emplace_back(Option{order, 2 * size * size});
		_queuedTanks++;
	}

	// make more militia!
	for (Tile& farm : _myFarms)
	{
		if (_money < _militiaCost + 10) break;
		int size = _bible.powerAbolished() ? farm.stacks : farm.power;
		if (size < 2) continue;
		if (farm.occupied) continue;
		if (farm.unfinished.type != Order::Type::NONE) continue;
		Order order(Order::Type::PRODUCE, farm.descriptor,
			Descriptor::cell(farm.descriptor.position),
			_militiatype);
		_options.emplace_back(Option{order, 4});
		_queuedMilitia++;
	}

	// upgrade barracks!
	for (Tile& barracks : _myBarracks)
	{
		if (_money < _barracksUpgradeCost + 10) break;
		if (barracks.stacks == _bible.tileStacksMax(_industrytype)) continue;
		if (barracks.power == 0 && !_bible.powerAbolished()) continue;
		if (barracks.occupied) continue;
		if (barracks.unfinished.type != Order::Type::NONE) continue;
		Order order(Order::Type::UPGRADE, barracks.descriptor, TileType::NONE);
		_options.emplace_back(Option{order, 15});
	}

	// upgrade industry!
	for (Tile& industry : _myIndustry)
	{
		if (_money < _industryUpgradeCost + 10) break;
		if (industry.stacks == _bible.tileStacksMax(_industrytype)) continue;
		if (industry.power == 0 && !_bible.powerAbolished()) continue;
		if (industry.occupied) continue;
		if (industry.unfinished.type != Order::Type::NONE) continue;
		Order order(Order::Type::UPGRADE, industry.descriptor, TileType::NONE);
		_options.emplace_back(Option{order, 10});
	}

	// select orders
	std::random_shuffle(_options.begin(), _options.end());
	std::sort(_options.begin(), _options.end(),
		[](const Option& lhs, const Option& rhs) {

		return lhs.priority > rhs.priority;
	});
	for (size_t i = 0; i < _options.size()
		&& _newOrders.size() < _bible.newOrderLimit(); i++)
	{
		if (hasNew(_options[i].order.subject).type != Order::Type::NONE)
			continue;
		_newOrders.emplace_back(_options[i].order);
	}

	if (_difficulty == Difficulty::EASY)
	{
		// Throw away all but one order
		std::random_shuffle(_newOrders.begin(), _newOrders.end());
		if (_newOrders.size() > 1) _newOrders = {_newOrders[0]};
	}
	else if (_difficulty == Difficulty::MEDIUM)
	{
		// Throw away all but three orders
		std::random_shuffle(_newOrders.begin(), _newOrders.end());
		if (_newOrders.size() > 3)
		{
			_newOrders = {_newOrders[0], _newOrders[1], _newOrders[2]};
		}
	}
}
