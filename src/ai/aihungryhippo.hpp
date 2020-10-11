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

#include "aicommander.hpp"
#include "cell.hpp"

enum class TileType : uint8_t;
enum class UnitType : uint8_t;


class AIHungryHippo : public AICommander
{
public:
	AIHungryHippo(const Player& player, const Difficulty& difficulty,
		const std::string& rulesetname, char character);
	AIHungryHippo(const AIHungryHippo&) = delete;
	AIHungryHippo(AIHungryHippo&&) = delete;
	AIHungryHippo& operator=(const AIHungryHippo&) = delete;
	AIHungryHippo& operator=(AIHungryHippo&&) = delete;
	virtual ~AIHungryHippo() = default;

private:
	struct Option
	{
		Order order;
		int priority;
	};
	struct Tile
	{
		Descriptor descriptor;
		int8_t stacks;
		int8_t power;
		Order unfinished;
		bool occupied;
	};
	struct Ground
	{
		Descriptor descriptor;
		int8_t stacks;
		Order unfinished;
	};

	TileType _citytype;
	TileType _towntype;
	TileType _outposttype;
	TileType _industrytype;
	TileType _barrackstype;
	TileType _farmtype;
	TileType _soiltype;
	TileType _cropstype;
	UnitType _settlertype;
	UnitType _militiatype;
	UnitType _riflemantype;
	UnitType _tanktype;
	UnitType _gunnertype;
	UnitType _sappertype;
	uint16_t _barracksCost;
	uint16_t _industryCost;
	uint16_t _tankCost;
	uint16_t _riflemanCost;
	uint16_t _militiaCost;
	uint16_t _barracksUpgradeCost;
	uint16_t _industryUpgradeCost;

	std::vector<Option> _options;
	std::vector<Tile> _myCities;
	std::vector<Tile> _myTowns;
	std::vector<Tile> _myIndustry;
	std::vector<Tile> _myBarracks;
	std::vector<Tile> _myFarms;
	std::vector<Tile> _mySoil;
	std::vector<Tile> _myCrops;
	std::vector<Ground> _mySettlers;
	std::vector<Ground> _myMilitia;
	std::vector<Ground> _myRiflemen;
	std::vector<Ground> _myTanks;
	std::vector<Ground> _enemyGround;
	int _queuedIndustry = 0;
	int _queuedBarracks = 0;
	int _queuedSettlers = 0;
	int _queuedMilitia = 0;
	int _queuedRiflemen = 0;
	int _queuedTanks = 0;
	int _queuedPlows = 0;

	Tile makeTile(Cell index) const;
	Ground makeGround(Cell index) const;
	bool cityOccupied(Cell index) const;
	int8_t cityNiceness(Cell index);
	int expectedSoil(Cell index);
	void determineState();
	void doFarming();

protected:
	virtual std::string ainame() const override;
	virtual std::string authors() const override;

	virtual void process() override;
};
