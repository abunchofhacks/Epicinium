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
 *
 */
 /**
 *	AI Edit by Tom van den Bosch (@Zanath)
 *  All credits to Daan Mulder for HungryHippo code on which this is based, and Sander & Daan for all other source code and the great game.
 */


#include "aichargingcheetah.hpp"
#include "source.hpp"

#include "pathingfloodfill.hpp"
#include "difficulty.hpp"


AIChargingCheetah::Tile AIChargingCheetah::makeTile(Cell index) const
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

AIChargingCheetah::Ground AIChargingCheetah::makeGround(Cell index) const
{
	Ground ground;
	ground.descriptor = Descriptor::ground(index.pos());
	ground.stacks = _board.ground(index).stacks;
	ground.unfinished = hasUnfinished(ground.descriptor);
	return ground;
}

bool AIChargingCheetah::cityOccupied(Cell index) const
{
	if (!_bible.tileBinding(_board.tile(index).type)) return false;
	if (_board.ground(index).type == UnitType::NONE) return false;
	if (_board.tile(index).owner == _board.ground(index).owner) return false;
	return true;
}

int8_t AIChargingCheetah::cityNiceness(Cell index)
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

int AIChargingCheetah::expectedSoil(Cell index)
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

AIChargingCheetah::AIChargingCheetah(const Player& player, const Difficulty& difficulty,
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
	_trenchestype = _bible.tiletype("trenches");
	if (_trenchestype == TileType::NONE)
	{
		LOGE << "Missing type 'trenches'";
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
	if (_barracksCost == NOTFOUND)
	{
		LOGW << "Missing _barracksCost";
	}
	_industryCost = NOTFOUND;
	for (const Bible::TileBuild& build : _bible.tileExpands(_citytype))
	{
		if (build.type != _industrytype) continue;
		_industryCost = build.cost;
		break;
	}
	if (_industryCost == NOTFOUND)
	{
		LOGW << "Missing _industryCost";
	}
	_tankCost = NOTFOUND;
	for (const Bible::UnitBuild& build : _bible.tileProduces(_industrytype))
	{
		if (build.type != _tanktype) continue;
		_tankCost = build.cost;
		break;
	}
	if (_tankCost == NOTFOUND)
	{
		LOGW << "Missing _tankCost";
	}
	_riflemanCost = NOTFOUND;
	for (const Bible::UnitBuild& build : _bible.tileProduces(_barrackstype))
	{
		if (build.type != _riflemantype) continue;
		_riflemanCost = build.cost;
		break;
	}
	if (_riflemanCost == NOTFOUND)
	{
		LOGW << "Missing _riflemanCost";
	}
	_gunnerCost = NOTFOUND;
	for (const Bible::UnitBuild& build : _bible.tileProduces(_barrackstype))
	{
		if (build.type != _gunnertype) continue;
		_gunnerCost = build.cost;
		break;
	}
	if (_gunnerCost == NOTFOUND)
	{
		LOGW << "Missing _gunnerCost";
	}
	_sapperCost = NOTFOUND;
	for (const Bible::UnitBuild& build : _bible.tileProduces(_barrackstype))
	{
		if (build.type != _sappertype) continue;
		_sapperCost = build.cost;
		break;
	}
	if (_sapperCost == NOTFOUND)
	{
		LOGW << "Missing _sapperCost";
	}
	_militiaCost = NOTFOUND;
	for (const Bible::UnitBuild& build : _bible.tileProduces(_farmtype))
	{
		if (build.type != _militiatype) continue;
		_militiaCost = build.cost;
		break;
	}
	if (_militiaCost == NOTFOUND)
	{
		LOGW << "Missing _militiaCost";
	}
	_settlerCost = NOTFOUND;
	for (const Bible::UnitBuild& build : _bible.tileProduces(_citytype))
	{
		if (build.type != _settlertype) continue;
		_settlerCost = build.cost;
		break;
	}
	if (_settlerCost == NOTFOUND)
	{
		LOGW << "Missing _settlerCost";
	}
	_barracksUpgradeCost = NOTFOUND;
	for (const Bible::TileBuild& build : _bible.tileUpgrades(_barrackstype))
	{
		if (build.type != TileType::NONE) continue;
		_barracksUpgradeCost = build.cost;
		break;
	}
	if (_barracksUpgradeCost == NOTFOUND)
	{
		LOGW << "Missing _barracksUpgradeCost";
	}
	_industryUpgradeCost = NOTFOUND;
	for (const Bible::TileBuild& build : _bible.tileUpgrades(_industrytype))
	{
		if (build.type != TileType::NONE) continue;
		_industryUpgradeCost = build.cost;
		break;
	}
	if (_industryUpgradeCost == NOTFOUND)
	{
		LOGW << "Missing _industryUpgradeCost";
	}
	_farmCost = NOTFOUND;
	for (const Bible::TileBuild& build : _bible.unitSettles(_settlertype))
	{
		if (build.type != _farmtype) continue;
		_farmCost = build.cost;
		break;
	}
	if (_farmCost == NOTFOUND)
	{
		LOGW << "Missing _farmCost";
	}
}

void AIChargingCheetah::determineState()
{
	// clear our vectors
	_options.clear();
	_myCities.clear();
	_myTowns.clear();
	_myIndustry.clear();
	_myBarracks.clear();
	_myFarms.clear();
	_myOutposts.clear();
	_mySoil.clear();
	_myCrops.clear();
	_mySettlers.clear();
	_myMilitia.clear();
	_myRiflemen.clear();
	_myGunners.clear();
	_myTanks.clear();
	_mySappers.clear();
	_enemyCities.clear();
	_enemyGround.clear();
	_queuedIndustry = 0;
	_queuedBarracks = 0;
	_queuedSettlers = 0;
	_queuedRiflemen = 0;
	_queuedGunners = 0;
	_queuedTanks = 0;
	_queuedSappers = 0;
	_queuedPlows = 0;
	_queuedMoney = 0;
	_turnNumber++;

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
			else if (_board.tile(index).type == _outposttype)
			{
				_myOutposts.emplace_back(makeTile(index));
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

		if (!(_board.tile(index).owner == _player) && _board.tile(index).type == _citytype)
		{
			_enemyCities.emplace_back(makeTile(index));
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
			else if (_board.ground(index).type == _gunnertype)
			{
				_myGunners.emplace_back(makeGround(index));
			}
			else if (_board.ground(index).type == _tanktype)
			{
				_myTanks.emplace_back(makeGround(index));
			}
			else if (_board.ground(index).type == _sappertype)
			{
				_mySappers.emplace_back(makeGround(index));
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
			&& order.unittype == _gunnertype) _queuedGunners++;
		else if (order.type == Order::Type::PRODUCE
			&& order.unittype == _tanktype) _queuedTanks++;
		else if (order.type == Order::Type::PRODUCE
			&& order.unittype == _sappertype) _queuedSappers++;
		else if (order.type == Order::Type::EXPAND
			&& order.tiletype == _industrytype) _queuedIndustry++;
		else if (order.type == Order::Type::EXPAND
			&& order.tiletype == _barrackstype) _queuedBarracks++;
		else if (order.type == Order::Type::CULTIVATE) _queuedPlows++;
	}
}


void AIChargingCheetah::doFarming()
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

	// build a farm if we have less than 9 soil and no plows already queued
	if (_mySoil.size() + _myCrops.size() >= 9 || _queuedPlows > 0) return;

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
				_queuedMoney = _queuedMoney + _farmCost;
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
//	else if (_queuedSettlers == 0)
//	{
//		std::vector<Tile> settlerProducers;
//		settlerProducers.insert(settlerProducers.end(), _myFarms.begin(),
//			_myFarms.end());
//		settlerProducers.insert(settlerProducers.end(), _myTowns.begin(),
//			_myTowns.end());
//		settlerProducers.insert(settlerProducers.end(), _myCities.begin(),
//			_myCities.end());
//		std::random_shuffle(settlerProducers.begin(), settlerProducers.end());
//		for (Tile& tile : settlerProducers)
//		{
//			if (tile.unfinished.type != Order::Type::NONE) continue;
//			if (tile.power < 1 && !_bible.powerAbolished()) continue;
//			if (_money == _gunnerCost) continue;
//			Order order(Order::Type::PRODUCE, tile.descriptor,
//				Descriptor::cell(tile.descriptor.position),
//				_settlertype);
//			_options.emplace_back(Option{order, 20});
//			_queuedSettlers++;
//			break;
//		}
//	}
}


void AIChargingCheetah::doFirstTurn()
{

	TileFloodfill enemycities(_bible, _board);
	enemycities.include({_citytype});
	enemycities.exclude({_player});
	enemycities.execute();

	float minenemydist = 1000000000;
	Move bestMove = Move::X;
	for (Tile& city : _myCities)
	{
		Cell cityloc = _board.cell(city.descriptor.position);
		for (const Move& move : {Move::E, Move::S, Move::W, Move::N})
		{
			Cell to = cityloc + move;
			if (to.edge()) continue;
			if(!(_bible.tileBuildable(_board.tile(to).type))) continue;
			if (enemycities.steps(to) < minenemydist)
			{
				minenemydist = enemycities.steps(to);
				bestMove = move;
			}
		}
	}

	bool barracksorder = false;
	for (Tile& city : _myCities)
	{
		Cell cityloc = _board.cell(city.descriptor.position);
		if (bestMove == Move::X) continue;
		Cell to = cityloc + bestMove;
		if(!(_bible.tileBuildable(_board.tile(to).type))) continue;
		if (enemycities.steps(to) == minenemydist && barracksorder == false)
		{
		Order order(Order::Type::EXPAND, city.descriptor,
			Descriptor::cell(to.pos()), _barrackstype);
		_options.emplace_back(Option{order,999});
		_queuedBarracks++;
		//barracksorder == true;
		}
	}

	TileFloodfill alliedcities(_bible, _board);
	alliedcities.include({_citytype});
	alliedcities.include({_player});
	alliedcities.execute();

	for (Ground& militia : _myMilitia)
	{
		minenemydist = 10000;
		Cell unitloc = _board.cell(militia.descriptor.position);
		std::vector<Move> bestMoves;
		if (_difficulty == Difficulty::EASY || _difficulty == Difficulty::MEDIUM)
		{
			continue;
		}
		Cell bestDestination = _board.cell(militia.descriptor.position);
		for (const Move& move1 : {Move::E, Move::S, Move::W, Move::N}){
		for (const Move& move2 : {Move::E, Move::S, Move::W, Move::N}){
		for (const Move& move3 : {Move::E, Move::S, Move::W, Move::N}){
			std::vector<Move> moves;
			//Cell destination = _board.cell(militia.descriptor.position);
			Cell to1 = unitloc + move1;
			Cell to2 = to1 + move2;
			Cell to3 = to2 + move3;
			moves.emplace_back(move1);
			moves.emplace_back(move2);
			moves.emplace_back(move3);
			if (to1.edge() || to2.edge() || to3.edge()) continue;
			if (!(_bible.tileWalkable(_board.tile(to1).type) && _bible.tileWalkable(_board.tile(to2).type) && _bible.tileWalkable(_board.tile(to3).type))) continue;
			if (enemycities.steps(to3) < minenemydist && alliedcities.steps(to3) > 1)
			{
				minenemydist = enemycities.steps(to3);
				bestMoves = moves;
				bestDestination = to3;
			}
		}}}
		if (bestMoves.empty()) continue;
		Order order(Order::Type::MOVE, militia.descriptor,
			Descriptor::cell(bestDestination.pos()), bestMoves);
		_options.emplace_back(Option{order, 999});
	}
}

// This makes sure that if a city that a gunner was moving to is captured or destroyed, it changes the order type to none so it can receive new ones.
void AIChargingCheetah::checkUnfinished()
{
	for (Ground& gunner :_myGunners){
		if (gunner.unfinished.type == Order::Type::NONE) continue;
		Cell target = _board.cell(gunner.unfinished.target.position);
		if (_board.ground(target).owner == _player || _board.tile(target).type != _citytype){
			gunner.unfinished.type 	= Order::Type::NONE;
		}
	}
}


std::string AIChargingCheetah::ainame() const
{
	return "ChargingCheetah";
}

std::string AIChargingCheetah::authors() const
{
	return "Tom van den Bosch";
}


// this is the part where I start shuffling things around and changing things
void AIChargingCheetah::process()
{
	determineState();


	// First turn need special orders. It is important that both the first barracks is put in the right spot, and that the first two militia move in the right directions.
	if (_turnNumber == 1)
	{
		doFirstTurn();
	}

	checkUnfinished();

	bool defense=false;
	for (Tile& city : _myCities)
	{
		Cell at = _board.cell(city.descriptor.position);
		if (cityOccupied(at) && _board.ground(at).owner != _player)
		{
			defense=true;
		}
	}





	TileFloodfill occupiedcities(_bible, _board);
	occupiedcities.include({_citytype});
	occupiedcities.include({_player});
	occupiedcities.includeOccupied();
	occupiedcities.execute();

	// Go to defend a city if you are in range
	for (Ground& gunner : _myGunners)
	{
		Cell destination = _board.cell(gunner.descriptor.position);
		if (occupiedcities.steps(destination) > 5) continue;
		std::vector<Move> moves;
		Move current;
		while ((current = occupiedcities.step(destination)) != Move::X)
		{
			moves.emplace_back(current);
			destination = destination + current;
		}
		Order order(Order::Type::MOVE, gunner.descriptor,
			Descriptor::cell(destination.pos()), moves);
		_options.emplace_back(Option{order, 101});
	}



	TileFloodfill targets(_bible, _board);
	targets.include(
		{_citytype, _farmtype, _outposttype, _towntype, _industrytype, _barrackstype});
	targets.exclude({_player});
	targets.execute();

	TileFloodfill cities(_bible, _board);
	cities.include({_citytype});
	cities.exclude({_player});
	cities.excludeOccupied();
	cities.execute();

	// Move sappers within range of cities. Then bombard them. Also bombard units or other buildings along the way.
	for (Ground& sapper : _mySappers)
	{
		Cell destination = _board.cell(sapper.descriptor.position);
						Cell from = _board.cell(sapper.descriptor.position);
						int rangeMin = _bible.unitRangeMin(_sappertype);
						int rangeMax = _bible.unitRangeMax(_sappertype);
						//int threshold = _bible.unitAbilityDamage(_sappertype);
						Cell bestCell = Cell::undefined();
						float bestScore = 0;
						for (Cell target : _board.area(from, rangeMin, rangeMax))
						{
							if (_board.tile(target).owner == _player) continue;
							if (_board.ground(target).owner == _player) continue;
							if (_board.air(target).owner == _player) continue;
							if (_board.tile(target).type == _citytype && bestScore < 2){
								bestCell = target;
								bestScore = 1;
							}
							if (_board.tile(target).type == _barrackstype && bestScore < 6)
							{
								bestCell = target;
								bestScore = 6;
							}
							if (_board.tile(target).type == _industrytype && bestScore < 1)
							{
								bestCell = target;
								bestScore = 2;
							}
							if (_board.ground(target).type == _sappertype && bestScore < 4)
							{
								bestCell = target;
								bestScore = 4;
							}
							if (_board.ground(target).type == _gunnertype && bestScore < 3)
							{
								bestCell = target;
								bestScore = 3;
							}
							if (_board.ground(target).type == _tanktype && bestScore < 5)
							{
								bestCell = target;
								bestScore = 5;
							}
							if (_board.tile(target).type == _trenchestype && _board.ground(target).type != UnitType::NONE){
								bestCell = target;
								bestScore = 1;
							}
						}
		if (bestScore > 0)
		{
		Cell target = bestCell;
		Order order(Order::Type::BOMBARD, sapper.descriptor,
			Descriptor::cell(target.pos()));
		_options.emplace_back(Option{order, 98});
		}
		else
		{
		if (sapper.unfinished.type != Order::Type::NONE) continue;
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
		Order order(Order::Type::MOVE, sapper.descriptor,
			Descriptor::cell(prev.pos()), moves);
		_options.emplace_back(Option{order, 15 - int(moves.size())});
		}
	}





	// move gunner to cities! Capture if on something that's not ours
	for (Ground& gunner : _myGunners)
	{
		Cell at = _board.cell(gunner.descriptor.position);
		if (_bible.tileOwnable(_board.tile(at).type) && !(_board.tile(at).owner == _player) && !(_board.tile(at).type == _soiltype) && !(_board.tile(at).type == _cropstype))
		{
		Order order(Order::Type::CAPTURE, gunner.descriptor);
		_options.emplace_back(Option{order, 25});
		}
		else
		{
		Cell target = _board.cell(gunner.unfinished.target.position);
		if (gunner.unfinished.type != Order::Type::NONE || (gunner.unfinished.type == Order::Type::MOVE && _board.tile(target).owner == _player && cityOccupied(target)==true)) continue;
		Cell destination = _board.cell(gunner.descriptor.position);
		if (cityOccupied(destination)) continue; // If this line does what I think it does, it might have to be removed?
		if (!cities.reached(destination)) continue;
		if (cities.steps(destination) == 0) continue;
		std::vector<Move> moves;
		Move current;
		while ((current = cities.step(destination)) != Move::X)
		{
			moves.emplace_back(current);
			destination = destination + current;
		}
		Order order(Order::Type::MOVE, gunner.descriptor,
			Descriptor::cell(destination.pos()), moves);
		_options.emplace_back(Option{order, 15 - int(moves.size())});
		}
	}


	// focus if we get the chance!
	for (Ground& enemyUnit : _enemyGround)
	{
		Cell from = _board.cell(enemyUnit.descriptor.position);
		std::vector<Ground> surrounding;
		if (_difficulty == Difficulty::EASY)
		{
			continue;
		}
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
		if (surrounding.size() < 1) continue;
		std::random_shuffle(surrounding.begin(), surrounding.end());
		for (auto& myUnit : surrounding)
		{
			if (surrounding.size() ==1)
			{
			Cell attacker = _board.cell(myUnit.descriptor.position);
			if (_board.ground(attacker).type == _gunnertype && (_board.ground(from).type == _gunnertype || _board.ground(from).type == _sappertype || cityOccupied(from)))
			{
				Order order(Order::Type::LOCKDOWN, myUnit.descriptor,
					Descriptor::cell(enemyUnit.descriptor.position));
				_options.emplace_back(Option{order, 30});
			}
			//else if ( !((_board.ground(attacker).type == _militiatype || _board.ground(attacker).type == _sappertype) && _board.ground(attacker).stacks == 1 && (_board.ground(from).type == _riflemantype || _board.ground(from).type == _gunnertype) ))
			//{
			//Order order(Order::Type::FOCUS, myUnit.descriptor,
			//	Descriptor::cell(enemyUnit.descriptor.position));
			//_options.emplace_back(Option{order, 100});
			//}
			// Not sure why this makes the AI worse but it does.
			}
			else if(surrounding.size() >1)
			{
			//Cell attacker = _board.cell(myUnit.descriptor.position);
			Order order(Order::Type::FOCUS, myUnit.descriptor,
				Descriptor::cell(enemyUnit.descriptor.position));
			_options.emplace_back(Option{order, 100});
			}
		}
	}

	TileFloodfill alliedcities(_bible, _board);
	alliedcities.include({_citytype});
	alliedcities.include({_player});
	alliedcities.execute();

	// move militia to targets
	for (Ground& militia : _myMilitia)
	{
		Cell from = _board.cell(militia.descriptor.position);
		if (militia.stacks == 1 && _bible.tileBuildable(_board.tile(_board.cell(militia.descriptor.position)).type) && _myOutposts.size() < 3 && alliedcities.steps(from) > 2)
		{
			Order order(Order::Type::SETTLE, militia.descriptor,
				_outposttype);
			_options.emplace_back(Option{order, 20});
		}
		else
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
		} //part of above militia make outpost loop
	}

	// move tanks next to cities!
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




	// Make 1 industry and barracks until we have 1 industry and 1 barracks. Then make more industry, but only if we have at least 1 gunner. Stop to make a second barracks when we have 4 industry
	for (Tile& barracks : _myBarracks)
	{
			if (barracks.power == 0 && !_bible.powerAbolished()) continue;
			if (barracks.occupied) continue;
			if (barracks.unfinished.type != Order::Type::NONE) continue;

		if (defense==true)
		{
			if (_money - _queuedMoney >= _gunnerCost)
			{
			if (barracks.occupied) continue;
				Order order(Order::Type::PRODUCE, barracks.descriptor,
					Descriptor::cell(barracks.descriptor.position),
					_gunnertype);
				_options.emplace_back(Option{order, 15});
				_queuedGunners++;
				_queuedMoney = _queuedMoney + _gunnerCost;
			}
		}
		else
		{
			if (barracks.power != barracks.stacks) continue;
		if ((_queuedGunners == 0 && _money - _queuedMoney >= _gunnerCost && _money- _queuedMoney < _barracksUpgradeCost) || (_money- _queuedMoney >= _gunnerCost && _myGunners.size() + _queuedGunners < 3 && barracks.stacks == _bible.tileStacksMax(_barrackstype)))
		{
			Order order(Order::Type::PRODUCE, barracks.descriptor,
				Descriptor::cell(barracks.descriptor.position),
				_gunnertype);
			_options.emplace_back(Option{order, 15});
			_queuedGunners++;
			_queuedMoney = _queuedMoney + _gunnerCost;
		}
		else if (_money - _queuedMoney >= _barracksUpgradeCost && _queuedGunners + _myGunners.size()>0 && !(barracks.stacks == _bible.tileStacksMax(_barrackstype)))
		{
			Order order(Order::Type::UPGRADE, barracks.descriptor, TileType::NONE);
			_options.emplace_back(Option{order, 15});
			_queuedMoney = _queuedMoney + _barracksUpgradeCost;
		}
		else if (_money - _queuedMoney >= _sapperCost &&  barracks.stacks >= 2 && _myGunners.size() > 2 && _myMilitia.size() > 2)
		{
			Order order(Order::Type::PRODUCE, barracks.descriptor,
				Descriptor::cell(barracks.descriptor.position),
				_sappertype);
			_options.emplace_back(Option{order, 15});
			_queuedSappers++;
			_queuedMoney = _queuedMoney + _sapperCost;
		}
		else if (_money - _queuedMoney >= _gunnerCost + 25 && barracks.power == barracks.stacks)
		{
			Order order(Order::Type::PRODUCE, barracks.descriptor,
				Descriptor::cell(barracks.descriptor.position),
				_gunnertype);
			_options.emplace_back(Option{order, 15});
			_queuedGunners++;
			_queuedMoney = _queuedMoney + _gunnerCost;
		}
		}
	}

	// make more militia!
	for (Tile& farm : _myFarms)
	{
		if (_money - _queuedMoney < _militiaCost || _money - _queuedMoney - (_industryCost * (_myMilitia.size() - _myIndustry.size())) < _militiaCost) break;
		int size = _bible.powerAbolished() ? farm.stacks : farm.power;
		if (size < 2) continue;
		if (farm.occupied) continue;
		if (farm.unfinished.type != Order::Type::NONE) continue;
		Order order(Order::Type::PRODUCE, farm.descriptor,
			Descriptor::cell(farm.descriptor.position),
			_militiatype);
		_options.emplace_back(Option{order, 20});
		_queuedMilitia++;
		_queuedMoney = _queuedMoney + _militiaCost;
	}


	doFarming();

	for (Tile& outpost : _myOutposts)
	{
		if (_money - _queuedMoney < _militiaCost || _money - _queuedMoney - (_industryCost * (_myMilitia.size() - _myIndustry.size())) < _militiaCost) break;
		int size = _bible.powerAbolished() ? outpost.stacks : outpost.power;
		if (size < 2) continue;
		if (outpost.occupied) continue;
		if (outpost.unfinished.type != Order::Type::NONE) continue;
		Order order(Order::Type::PRODUCE, outpost.descriptor,
			Descriptor::cell(outpost.descriptor.position),
			_militiatype);
		_options.emplace_back(Option{order, 20});
		_queuedMilitia++;
		_queuedMoney = _queuedMoney + _militiaCost;
	}

	TileFloodfill alliedbarracks(_bible, _board);
	alliedbarracks.include({_barrackstype});
	alliedbarracks.include({_player});
	alliedbarracks.execute();


	for (Tile& city : _myCities)
	{

		if (defense==true)
		{
			if (_money - _queuedMoney < _militiaCost) break;
			if (city.occupied) continue;
			// When defense is true, do nothing with cities. Winrate shows that this is useless, maybe spending money in barracks or outposts is better.
			//Order order(Order::Type::PRODUCE, city.descriptor,
			//	Descriptor::cell(city.descriptor.position),
			//	_militiatype);
			//_options.emplace_back(Option{order, 4});
			//_queuedMilitia++;
		}
		else
		{
		Cell at = _board.cell(city.descriptor.position);
		if (_money - _queuedMoney < _industryCost) break;
		if (city.unfinished.type != Order::Type::NONE) continue;
		if (city.power == 0 && !_bible.powerAbolished()) continue;
		if (city.occupied) continue;
		if (cityNiceness(at) < 3) continue;
		// create when we have 0, or when we have 1 barracks and <3 industry and 1 gunner, or when we have >1 barracks and >1 gunner
		// ADD &&(_mySettlers.size() + _queuedSettlers + _myFarms.size()>=2) to make 2 settlers turn 3. This does not seem to work as intended yet.
		if ((_myIndustry.size() + _queuedIndustry == 0) || ((_myBarracks.size()+_queuedBarracks == 1) && (_myIndustry.size() + _queuedIndustry < 3) && (_myGunners.size() + _queuedGunners>0)) || (_myGunners.size()+_queuedGunners>0 && _myBarracks.size()>1))
		{
			std::vector<Move> directions = {Move::E, Move::S, Move::W, Move::N};
			std::random_shuffle(directions.begin(), directions.end());
			if (cityNiceness(at)<4 && city.power < 4 && alliedbarracks.steps(at) < 2) continue;
			for (const Move& move : directions)
			{
				if (_difficulty == Difficulty::EASY) continue;
				if (_difficulty == Difficulty::MEDIUM && _myIndustry.size() + _queuedIndustry > 1) continue;
				Cell to = at + move;
				if (_board.ground(to)) continue;
				if (!_bible.tileBuildable(_board.tile(to).type)) continue;
				Order order(Order::Type::EXPAND, city.descriptor,
					Descriptor::cell(to.pos()), _industrytype);
				_options.emplace_back(Option{order, 1 + cityNiceness(at)});
				_queuedIndustry++;
				_queuedMoney = _queuedMoney + _industryCost;
				break;
			}
		}
		else if ((_myBarracks.size() + _queuedBarracks == 0) || (_myIndustry.size() + _queuedIndustry > 2 && _myBarracks.size() + _queuedBarracks == 1 && _myGunners.size() + _queuedGunners > 0))
		{
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
				_queuedMoney = _queuedMoney + _barracksCost;
			break;
			}
		}
		//We want max 2 farms. And only after we have an industry, barracks, and gunner
		if ((_queuedSettlers + _mySettlers.size() + _myFarms.size() < 2) && _myBarracks.size() > 0 && _myIndustry.size() > 0)
		{
			if (city.unfinished.type != Order::Type::NONE) continue;
			if (city.power < 1 && !_bible.powerAbolished()) continue;
			if (_money == _gunnerCost) continue;
			if (_difficulty == Difficulty::EASY || _difficulty == Difficulty::MEDIUM)
			{
				continue;
			}
			Order order(Order::Type::PRODUCE, city.descriptor,
				Descriptor::cell(city.descriptor.position),
				_settlertype);
			_options.emplace_back(Option{order, 20});
			_queuedSettlers++;
			_queuedMoney = _queuedMoney + _settlerCost;
			break;
		}
		}
	}


	// select orders
	// shuffle them first to prevent north-south bias
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
		if (_newOrders.size() > 1)
		{
			_newOrders = {_newOrders[0]};
		}
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
