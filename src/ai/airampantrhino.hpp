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
#include "descriptor.hpp"
#include "position.hpp"


class AIRampantRhino : public AICommander
{
public:
	AIRampantRhino(const Player& player, const Difficulty& difficulty,
		const std::string& rulesetname, char character);
	AIRampantRhino(const AIRampantRhino&) = delete;
	AIRampantRhino(AIRampantRhino&&) = delete;
	AIRampantRhino& operator=(const AIRampantRhino&) = delete;
	AIRampantRhino& operator=(AIRampantRhino&&) = delete;
	virtual ~AIRampantRhino() = default;

private:
	TileType _citytype;
	TileType _towntype;
	TileType _outposttype;
	TileType _industrytype;
	TileType _barrackstype;
	TileType _airfieldtype;
	TileType _farmtype;
	TileType _soiltype;
	TileType _cropstype;
	TileType _trenchestype;
	UnitType _settlertype;
	UnitType _militiatype;
	UnitType _riflemantype;
	UnitType _tanktype;
	UnitType _gunnertype;
	UnitType _sappertype;
	uint16_t _settlermoney;
	uint16_t _settlingmoney;

	std::vector<Descriptor> _settlers;
	std::vector<Descriptor> _settlercreators;
	std::vector<Descriptor> _industrycreators;
	std::vector<Descriptor> _barrackscreators;
	std::vector<Descriptor> _defenses;
	std::vector<Descriptor> _offenses;
	std::vector<Descriptor> _defensecreators;
	std::vector<Descriptor> _defenseupgraders;
	std::vector<Descriptor> _economyupgraders;
	std::vector<Descriptor> _captors;
	std::vector<Descriptor> _blockers;
	std::vector<Descriptor> _bombarders;
	std::vector<Descriptor> _cultivators;
	std::vector<Descriptor> _stoppers;
	std::vector<Position> _targets;

	int _moneystarting;
	int _moneyleftover;
	int _moneyreserved;

	void count();

	void lookAtUnfinishedOrder(const Order& order);

	void createSettlers();
	void controlSettlers();
	void controlSettler(const Descriptor& desc);
	void createIndustry();
	void createIndustry(const Descriptor& desc);
	void createBarracks();
	void createBarracks(const Descriptor& desc);
	void upgradeDefenseCreator();
	void upgradeDefenseCreator(const Descriptor& desc);
	void upgradeEconomy();
	void upgradeEconomy(const Descriptor& desc);
	void controlCultivators();
	void controlCultivator(const Descriptor& desc);
	void createDefenses();
	void controlDefenses();
	void controlDefense(const Descriptor& desc);
	void controlIdleDefenses();
	void controlIdleDefense(const Descriptor& desc);
	void declareOffenses();
	void declareOffense(const Descriptor& desc);
	void controlOffenses();
	void controlOffense(const Descriptor& desc);
	void controlCaptors();
	void controlCaptor(const Descriptor& desc);
	void controlBlockers();
	void controlBlocker(const Descriptor& desc);
	void controlBombarders();
	void controlBombarder(const Descriptor& desc);
	void controlStoppers();
	void controlStopper(const Descriptor& desc);

	bool isTarget(const Position& position);
	bool hasOldOrder(const Descriptor& desc);
	bool hasNewOrder(const Descriptor& desc);
	bool checkLockdown(const Descriptor& desc);
	bool isOccupied(const Position& position);

	virtual int maxOrders();
	virtual int maxMilitaryOrders();
	virtual int maxMilitaryUnits();
	virtual int maxDefenseUnits();
	virtual int maxCaptures();
	virtual int maxDeblockers();
	virtual int maxBombardments();
	virtual int minDefenseMoney();
	virtual int minSettlersMoney();
	virtual int minBarracksMoney();
	virtual int minDefenseUpgradeMoney();
	virtual int minEconomyUpgradeMoney();

	virtual bool canUseCombatAbilities();
	virtual bool canBuildBarracks();
	virtual bool canBuildCities();

	int expectedSoil(Cell index);
	int expectedNiceness(Cell index);

protected:
	virtual std::string ainame() const override;
	virtual std::string authors() const override;

	virtual void process() override;
};
