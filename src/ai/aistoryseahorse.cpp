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


#include "aistoryseahorse.hpp"
#include "source.hpp"

#include "pathingfloodfill.hpp"
#include "difficulty.hpp"

AIStorySeahorse::Tile AIStorySeahorse::makeTile(Cell index) const
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

AIStorySeahorse::Ground AIStorySeahorse::makeGround(Cell index) const
{
	Ground ground;
	ground.descriptor = Descriptor::ground(index.pos());
	ground.stacks = _board.ground(index).stacks;
	ground.unfinished = hasUnfinished(ground.descriptor);
	return ground;
}

bool AIStorySeahorse::cityOccupied(Cell index) const
{
	if (!_bible.tileBinding(_board.tile(index).type)) return false;
	if (_board.ground(index).type == UnitType::NONE) return false;
	if (_board.tile(index).owner == _board.ground(index).owner) return false;
	return true;
}

int8_t AIStorySeahorse::cityNiceness(Cell index)
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

int AIStorySeahorse::expectedSoil(Cell index)
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

AIStorySeahorse::AIStorySeahorse(const Player& player, const Difficulty& difficulty,
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
	_trenchestype = _bible.tiletype("trenches");
	if (_trenchestype == TileType::NONE)
	{
		LOGE << "Missing type 'trenches'";
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
	_gunnerCost = NOTFOUND;
	for (const Bible::UnitBuild& build : _bible.tileProduces(_barrackstype))
	{
		if (build.type != _gunnertype) continue;
		_gunnerCost = build.cost;
		break;
	}
	_sapperCost = NOTFOUND;
	for (const Bible::UnitBuild& build : _bible.tileProduces(_barrackstype))
	{
		if (build.type != _sappertype) continue;
		_sapperCost = build.cost;
		break;
	}
	_militiaCost = NOTFOUND;
	for (const Bible::UnitBuild& build : _bible.tileProduces(_farmtype))
	{
		if (build.type != _militiatype) continue;
		_militiaCost = build.cost;
		break;
	}
	_settlerCost = NOTFOUND;
	for (const Bible::UnitBuild& build : _bible.tileProduces(_citytype))
	{
		if (build.type != _settlertype) continue;
		_settlerCost = build.cost;
		break;
	}
	for (const Bible::TileBuild& build : _bible.tileUpgrades(_industrytype))
	{
		if (build.type != TileType::NONE) continue;
		_industryUpgradeCost = build.cost;
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
	_farmCost = NOTFOUND;
	for (const Bible::TileBuild& build : _bible.unitSettles(_settlertype))
	{
		if (build.type != _farmtype) continue;
		_farmCost = build.cost;
		break;
	}
}

void AIStorySeahorse::determineState()
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




// This makes sure that if a city that a rifleman was moving to is captured or destroyed, it changes the order type to none so it can receive new ones.
void AIStorySeahorse::checkUnfinished()
{
	for (Ground& rifleman :_myRiflemen){
		if (rifleman.unfinished.type == Order::Type::NONE) continue;
		Cell target = _board.cell(rifleman.unfinished.target.position);
		if (_board.ground(target).owner == _player || (_board.tile(target).type != _citytype &&  _board.tile(target).type != _barrackstype)){
			rifleman.unfinished.type = Order::Type::NONE;
		}
	}
	
	for (Ground& militia :_myMilitia){
		if (militia.unfinished.type == Order::Type::NONE) continue;
		Cell target = _board.cell(militia.unfinished.target.position);
		if (_board.ground(target).owner == _player || (_board.tile(target).type != _citytype &&  _board.tile(target).type != _barrackstype)){
			militia.unfinished.type = Order::Type::NONE;
		}
	}
}


std::string AIStorySeahorse::ainame() const
{
	return "StorySeahorse";
}

std::string AIStorySeahorse::authors() const
{
	return "Tom van den Bosch";
}

std::string AIStorySeahorse::displayname() const
{
	return "Big evil guy";
}

// this is the part where I start shuffling things around and changing things
void AIStorySeahorse::process()
{
	determineState();









	checkUnfinished();

	// pathingfloodfills that we might need
	TileFloodfill occupiedcities(_bible, _board);
	occupiedcities.include({_citytype});
	occupiedcities.include({_player});
	occupiedcities.includeOccupied();
	occupiedcities.execute();

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

	TileFloodfill alliedcities(_bible, _board);
	alliedcities.include({_citytype});
	alliedcities.include({_player});
	alliedcities.execute();

	TileFloodfill alliedbarracks(_bible, _board);
	alliedbarracks.include({_barrackstype});
	alliedbarracks.include({_player});
	alliedbarracks.execute();
	
	TileFloodfill occupiedbarracks(_bible, _board);
	occupiedbarracks.include({_barrackstype});
	occupiedbarracks.include({_player});
	occupiedbarracks.includeOccupied();
	occupiedbarracks.execute();
	
	
	// Step 1: build a farm with our settler. We never build more so this is fine here, as its also already in position.	
	for (Ground& settler : _mySettlers)
	{
		Order order(Order::Type::SETTLE, settler.descriptor,
			_farmtype);
		_options.emplace_back(Option{order, 20});
	}
	
	// Step 2: Spam militia in farms if power==2
	for (Tile& farm : _myFarms)
	{
		if (farm.power!=2) continue;
		if (farm.occupied) continue;
		if (farm.unfinished.type != Order::Type::NONE) continue;
		Order order(Order::Type::PRODUCE, farm.descriptor,
			Descriptor::cell(farm.descriptor.position),
			_militiatype);
		_options.emplace_back(Option{order, 20});
	}
	
	// Step 3: Create riflemen in Barracks if power==2
	for (Tile& barracks : _myBarracks)
	{
		if (barracks.power == 2)
		{
				Order order(Order::Type::PRODUCE, barracks.descriptor,
					Descriptor::cell(barracks.descriptor.position),
					_riflemantype);
				_options.emplace_back(Option{order, 15});
		}
	}

	// Step 3: Focus if possible
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
		if (surrounding.size() < 1) continue;
		std::random_shuffle(surrounding.begin(), surrounding.end());
		for (auto& myUnit : surrounding)
		{
			Order order(Order::Type::FOCUS, myUnit.descriptor,
				Descriptor::cell(enemyUnit.descriptor.position));
			_options.emplace_back(Option{order, 15});
		}
	}
	
	// Step 4: Move riflemen to enemy 
	for (Ground& rifleman : _myRiflemen)
	{
		Cell at = _board.cell(rifleman.descriptor.position);
		if (_bible.tileOwnable(_board.tile(at).type) && !(_board.tile(at).owner == _player) && !(_board.tile(at).type == _soiltype) && !(_board.tile(at).type == _cropstype))
		{
			Order order(Order::Type::CAPTURE, rifleman.descriptor);
			_options.emplace_back(Option{order, 25});
		}
		else
		{
		Cell target = _board.cell(rifleman.unfinished.target.position);
		if (rifleman.unfinished.type != Order::Type::NONE || (rifleman.unfinished.type == Order::Type::MOVE && _board.tile(target).owner == _player && cityOccupied(target)==true)) continue;
		Cell destination = _board.cell(rifleman.descriptor.position);
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
		Order order(Order::Type::MOVE, rifleman.descriptor,
			Descriptor::cell(destination.pos()), moves);
		_options.emplace_back(Option{order, 15});
		}
	}
	// Step 5: Move militia to enemy
	for (Ground& militia : _myMilitia)
	{
		Cell at = _board.cell(militia.descriptor.position);
		if (_bible.tileOwnable(_board.tile(at).type) && !(_board.tile(at).owner == _player) && !(_board.tile(at).type == _soiltype) && !(_board.tile(at).type == _cropstype))
		{
			Order order(Order::Type::CAPTURE, militia.descriptor);
			_options.emplace_back(Option{order, 25});
		}
		else
		{
		Cell target = _board.cell(militia.unfinished.target.position);
		if (militia.unfinished.type != Order::Type::NONE || (militia.unfinished.type == Order::Type::MOVE && _board.tile(target).owner == _player && cityOccupied(target)==true)) continue;
		Cell destination = _board.cell(militia.descriptor.position);
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
		Order order(Order::Type::MOVE, militia.descriptor,
			Descriptor::cell(destination.pos()), moves);
		_options.emplace_back(Option{order, 15});
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
}
