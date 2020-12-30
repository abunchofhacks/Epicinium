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
#include "bible.hpp"
#include "source.hpp"

#include "tiletype.hpp"
#include "unittype.hpp"
#include "cycle.hpp"



Bible::TileBuild::TileBuild(TileType tt) :
	type(tt),
	cost(-1)
{
	// This cost will be set at the end of the Bible constructor.
}

Bible::TileBuild::TileBuild(TileType tt, int16_t x) :
	type(tt),
	cost(x)
{}

bool Bible::TileBuild::operator==(const TileBuild& other) const
{
	return (type == other.type && cost == other.cost);
}

Bible::UnitBuild::UnitBuild(UnitType ut) :
	type(ut),
	cost(-1)
{
	// This cost will be set at the end of the Bible constructor.
}

Bible::UnitBuild::UnitBuild(UnitType ut, int16_t x) :
	type(ut),
	cost(x)
{}

bool Bible::UnitBuild::operator==(const UnitBuild& other) const
{
	return (type == other.type && cost == other.cost);
}

Bible::Bible(const std::string& name, const Version& version) :
	_name(name.empty() ? version.name() : name),
	_version(version),
	_tileAccessible(/*TILETYPE_SIZE, false*/),
	_tileWalkable(/*TILETYPE_SIZE, false*/),
	_tileBuildable(/*TILETYPE_SIZE, false*/),
	_tileDestructible(/*TILETYPE_SIZE, false*/),
	_tileGrassy(/*TILETYPE_SIZE, false*/),
	_tileNatural(/*TILETYPE_SIZE, false*/),
	_tileLaboring(/*TILETYPE_SIZE, false*/),
	_tileEnergizing(/*TILETYPE_SIZE, false*/),
	_tilePowered(/*TILETYPE_SIZE, false*/),
	_tileNeedsNiceness(/*TILETYPE_SIZE, false*/),
	_tileNeedsLabor(/*TILETYPE_SIZE, false*/),
	_tileNeedsEnergy(/*TILETYPE_SIZE, false*/),
	_tileNeedsTime(/*TILETYPE_SIZE, false*/),
	_tileOwnable(/*TILETYPE_SIZE, false*/),
	_tileControllable(/*TILETYPE_SIZE, false*/),
	_tileBinding(/*TILETYPE_SIZE, false*/),
	_tileAutoCultivates(/*TILETYPE_SIZE, false*/),
	_tilePlane(/*TILETYPE_SIZE, false*/),
	_tileFlammable(/*TILETYPE_SIZE, false*/),
	_tileWater(/*TILETYPE_SIZE, false*/),
	_tileMountain(/*TILETYPE_SIZE, false*/),
	_tileDesert(/*TILETYPE_SIZE, false*/),
	_tileStone(/*TILETYPE_SIZE, false*/),
	_tileTrenches(/*TILETYPE_SIZE, false*/),
	_tileForceOccupy(/*TILETYPE_SIZE, false*/),
	_tileChaosProtection(/*TILETYPE_SIZE, false*/),
	_tileRegrowOnlyInSpring(/*TILETYPE_SIZE, false*/),
	_tileGathersMorale(/*TILETYPE_SIZE, false*/),
	_tileStacksBuilt(TILETYPE_SIZE, 0),
	_tileStacksMax(TILETYPE_SIZE, 0),
	_tilePowerBuilt(TILETYPE_SIZE, 0),
	_tilePowerMax(TILETYPE_SIZE, 0),
	_tileGrowthMax(TILETYPE_SIZE, 0),
	_tileVision(TILETYPE_SIZE, 0),
	_tileHitpoints(TILETYPE_SIZE, 0),
	_tileIncome(TILETYPE_SIZE, 0),
	_tileLeakGas(TILETYPE_SIZE, 0),
	_tileLeakRads(TILETYPE_SIZE, 0),
	_tileEmission(TILETYPE_SIZE, 0),
	_tilePollutionAmount(TILETYPE_SIZE, 0),
	_tilePollutionRadius(TILETYPE_SIZE, 0),
	_tileSlowAmount(TILETYPE_SIZE, 0),
	_tileSlowMaximum(TILETYPE_SIZE, 0),
	_tileRegrowthProbabilityDivisor(TILETYPE_SIZE, 0),
	_tileRegrowthAmount(TILETYPE_SIZE, 0),
	_tileFirestormResistance(TILETYPE_SIZE, 0),
	_tileMoraleGainWhenBuilt(TILETYPE_SIZE, 0),
	_tileMoraleGainWhenLost(TILETYPE_SIZE, 0),
	_tileMoraleGainWhenDestroyed(TILETYPE_SIZE, 0),
	_tileMoraleGainWhenCaptured(TILETYPE_SIZE, 0),
	_tileMoraleGainWhenRazed(TILETYPE_SIZE, 0),
	_tileMoraleGainWhenGathered(TILETYPE_SIZE, 0),
	_tileProduces(TILETYPE_SIZE),
	_tileExpands(TILETYPE_SIZE),
	_tileUpgrades(TILETYPE_SIZE),
	_tileCultivates(TILETYPE_SIZE),
	_tileCost(TILETYPE_SIZE, 0),
	_tileScoreBase(TILETYPE_SIZE, 0),
	_tileScoreStack(TILETYPE_SIZE, 0),
	_tileDestroyed(TILETYPE_SIZE, TileType::NONE),
	_tileDegraded(TILETYPE_SIZE, TileType::NONE),
	_tileDesertified(TILETYPE_SIZE, TileType::NONE),
	_tileConsumed(TILETYPE_SIZE, TileType::NONE),
	_tileRegrown(TILETYPE_SIZE, TileType::NONE),
	_tileExpandRangeMin(0),
	_tileExpandRangeMax(0),
	_tileProduceRangeMin(0),
	_tileProduceRangeMax(0),
	_unitAir(/*UNITTYPE_SIZE, false*/),
	_unitInfantry(/*UNITTYPE_SIZE, false*/),
	_unitMechanical(/*UNITTYPE_SIZE, false*/),
	_unitCanMove(/*UNITTYPE_SIZE, false*/),
	_unitCanAttack(/*UNITTYPE_SIZE, false*/),
	_unitCanGuard(/*UNITTYPE_SIZE, false*/),
	_unitCanFocus(/*UNITTYPE_SIZE, false*/),
	_unitCanLockdown(/*UNITTYPE_SIZE, false*/),
	_unitCanShell(/*UNITTYPE_SIZE, false*/),
	_unitCanBombard(/*UNITTYPE_SIZE, false*/),
	_unitCanBomb(/*UNITTYPE_SIZE, false*/),
	_unitCanCapture(/*UNITTYPE_SIZE, false*/),
	_unitCanOccupy(/*UNITTYPE_SIZE, false*/),
	_unitStacksMax(UNITTYPE_SIZE, 0),
	_unitSpeed(UNITTYPE_SIZE, 0),
	_unitVision(UNITTYPE_SIZE, 0),
	_unitHitpoints(UNITTYPE_SIZE, 0),
	_unitAttackShots(UNITTYPE_SIZE, 0),
	_unitAttackDamage(UNITTYPE_SIZE, 0),
	_unitTrampleShots(UNITTYPE_SIZE, 0),
	_unitTrampleDamage(UNITTYPE_SIZE, 0),
	_unitAbilityVolleys(UNITTYPE_SIZE, 0),
	_unitAbilityShots(UNITTYPE_SIZE, 0),
	_unitAbilityDamage(UNITTYPE_SIZE, 0),
	_unitAbilityGas(UNITTYPE_SIZE, 0),
	_unitAbilityRads(UNITTYPE_SIZE, 0),
	_unitAbilityRadius(UNITTYPE_SIZE, 0),
	_unitRangeMin(UNITTYPE_SIZE, 0),
	_unitRangeMax(UNITTYPE_SIZE, 0),
	_unitLeakGas(UNITTYPE_SIZE, 0),
	_unitLeakRads(UNITTYPE_SIZE, 0),
	_unitMoraleGainWhenLost(UNITTYPE_SIZE, 0),
	_unitMoraleGainWhenKilled(UNITTYPE_SIZE, 0),
	_unitShapes(UNITTYPE_SIZE),
	_unitSettles(UNITTYPE_SIZE),
	_unitCost(UNITTYPE_SIZE, 0),
	_unitSizeMax(0),
	_unitVisionMax(0),
	_missCountGround(0),
	_missCountAir(0),
	_missCountTrenches(0),
	_missHitpointsGround(0),
	_missHitpointsAir(0),
	_missHitpointsTrenches(0),
	_seasonTemperatureSwing(SEASON_SIZE, 0),
	_seasonGlobalWarmingFactor(SEASON_SIZE, 0),
	_emissionDivisor(0),
	_gasPollutionAmount(0),
	_aridificationAmountHumid(0),
	_aridificationAmountDegraded(0),
	_aridificationRange(0),
	_aridificationCount(0),
	_firestormCount(0),
	_deathCount(0),
	_temperatureMax(0),
	_temperatureMin(0),
	_humidityMax(0),
	_humidityMin(0),
	_chaosMax(0),
	_chaosMin(0),
	_gasMax(0),
	_gasMin(0),
	_radiationMax(0),
	_radiationMin(0),
	_chaosThreshold(0),
	_temperatureMinHotDeath(SEASON_SIZE, 0),
	_temperatureMinFirestorm(SEASON_SIZE, 0),
	_temperatureMinAridification(SEASON_SIZE, 0),
	_temperatureMaxComfortable(SEASON_SIZE, 0),
	_temperatureMinComfortable(SEASON_SIZE, 0),
	_temperatureMaxSnow(SEASON_SIZE, 0),
	_temperatureMaxFrostbite(SEASON_SIZE, 0),
	_temperatureMaxColdDeath(SEASON_SIZE, 0),
	_humidityMinWet(SEASON_SIZE, 0),
	_humidityMaxDegradation(SEASON_SIZE, 0),
	_humidityMaxDesertification(SEASON_SIZE, 0),
	_humidityMinSnow(SEASON_SIZE, 0),
	_humidityMinFrostbite(SEASON_SIZE, 0),
	_humidityMaxFirestorm(SEASON_SIZE, 0),
	_humidityMaxBonedrought(SEASON_SIZE, 0),
	_humidityMaxStonedrought(SEASON_SIZE, 0),
	_humidityMaxDeath(SEASON_SIZE, 0),
	_chaosMinDegradation(SEASON_SIZE, 0),
	_chaosMinDesertification(SEASON_SIZE, 0),
	_chaosMinAridification(SEASON_SIZE, 0),
	_chaosMinSnow(SEASON_SIZE, 0),
	_chaosMinFrostbite(SEASON_SIZE, 0),
	_chaosMinFirestorm(SEASON_SIZE, 0),
	_chaosMinBonedrought(SEASON_SIZE, 0),
	_chaosMinStonedrought(SEASON_SIZE, 0),
	_chaosMinDeath(SEASON_SIZE, 0),
	_frostbiteShots(0),
	_frostbiteDamage(0),
	_frostbiteThresholdDamage(0),
	_frostbiteThresholdVulnerability(0),
	_firestormShots(0),
	_firestormDamage(0),
	_firestormBasePercentage(0),
	_firestormDroughtPercentage(0),
	_gasShots(0),
	_gasDamage(0),
	_gasThresholdDamage(0),
	_gasThresholdVulnerability(0),
	_radiationShots(0),
	_radiationDamage(0),
	_radiationThresholdDamage(0),
	_radiationThresholdVulnerability(0),
	_radiationThresholdDeath(0),
	_tempGenDefault(0),
	_humGenDefault(0),
	_tempGenGainRange(0),
	_humGenGainRange(0),
	_tempGenMountainGain(_tempGenGainRange + 1, 0),
	_tempGenOceanGain(_tempGenGainRange + 1, 0),
	_humGenLakeGain(_humGenGainRange + 1, 0),
	_humGenOceanGain(_humGenGainRange + 1, 0),
	_humGenDesertGain(_humGenGainRange + 1, 0),
	_humGenMountainGain(_humGenGainRange + 1, 0),
	_powerAbolished(false),
	_separatePowerStages(false),
	_industryNicenessQuantitative(false),
	_reactorNicenessQuantitative(false),
	_forestChaosProtectionPermanent(false),
	_collateralDamageKillsTiles(false),
	_gasOnlyTargetsGroundUnits(false),
	_frostbiteOnlyTargetsGroundUnits(false),
	_trenchesProtectMovingUnits(false),
	_trenchesProtectBypassingUnits(false),
	_trenchesHideBypassedUnit(false),
	_captureStrengthCheck(false),
	_focusOnce(false),
	_powerDrainScaled(false),
	_powerCanDrop(false),
	_upgradeKeepPower(false),
	_upgradeKeepStacks(false),
	_upgradeNotBlockedByFriendly(false),
	_groundPollutionOnlyInAutumn(false),
	_counterBasedWeather(false),
	_quantitativeChaos(false),
	_stackBasedFrostbite(false),
	_emptyBasedFrostbite(false),
	_planeBasedFrostbite(false),
	_planeBasedAridification(false),
	_flammableBasedFirestorm(false),
	_percentageBasedFirestorm(false),
	_randomizedFirestorm(false),
	_randomizedAridification(false),
	_cumulativeDeath(false),
	_vulnerabilitiesStack(false),
	_markersChangeAtNight(false),
	_markersChangeAfterBuild(false),
	_publicInitiative(false),
	_weatherAfterNight(false),
	_moraleReplacesMoney(false),
	_moraleGatheredInSummer(false),
	_moraleGatheredWhenBuilt(false),
	_snowCoversNiceness(false),
	_snowSlowAmount(0),
	_snowSlowMaximum(0),
	_startingMoney(0),
	_maxMoney(0),
	_minMoney(0),
	_newOrderLimit(0)
{
	/* TYPES */
	_tiletype_max = 0;
	_unittype_max = 0;
	_tiletypes[0] = forceTypeWord("none");
	_unittypes[0] = forceTypeWord("none");
}

#define VARIABLE(X) _##X
#define NAME(X) #X
#define AUTO(F, X) F(VARIABLE(X), NAME(X))

#define FILLBUILDLIST(BUILDLIST, COSTVARIABLE) \
	{ \
		for (auto& build : BUILDLIST) \
		{ \
			if (build.cost < 0) \
			{ \
				build.cost = COSTVARIABLE[(size_t) build.type]; \
			} \
		} \
	}

Bible::Bible() :
	Bible("", Version::undefined())
{}

Bible Bible::createDefault()
{
	Bible bible("", Version::current());

	bible.initialize();
	bible.finalize();

	return bible;
}

Bible Bible::createCustom()
{
	Bible bible("custom", Version::current());

	bible.initialize();
	bible.customize();
	bible.finalize();

	return bible;
}

void Bible::initialize()
{
	size_t i;

	/* TYPES */
	i = 0;
	// Implicit NONE at 0.
	_tiletypes[++i] = forceTypeWord("grass"); const size_t GRASS = i;
	_tiletypes[++i] = forceTypeWord("dirt"); const size_t DIRT = i;
	_tiletypes[++i] = forceTypeWord("desert"); const size_t DESERT = i;
	_tiletypes[++i] = forceTypeWord("rubble"); const size_t RUBBLE = i;
	_tiletypes[++i] = forceTypeWord("ridge"); const size_t RIDGE = i;
	_tiletypes[++i] = forceTypeWord("mountain"); const size_t MOUNTAIN = i;
	_tiletypes[++i] = forceTypeWord("water"); const size_t WATER = i;
	_tiletypes[++i] = forceTypeWord("forest"); const size_t FOREST = i;
	_tiletypes[++i] = forceTypeWord("city"); const size_t CITY = i;
	_tiletypes[++i] = forceTypeWord("town"); const size_t TOWN = i;
	_tiletypes[++i] = forceTypeWord("outpost"); const size_t OUTPOST = i;
	_tiletypes[++i] = forceTypeWord("industry"); const size_t INDUSTRY = i;
	_tiletypes[++i] = forceTypeWord("barracks"); const size_t BARRACKS = i;
	_tiletypes[++i] = forceTypeWord("airfield"); const size_t AIRFIELD = i;
	_tiletypes[++i] = forceTypeWord("farm"); const size_t FARM = i;
	_tiletypes[++i] = forceTypeWord("soil"); const size_t SOIL = i;
	_tiletypes[++i] = forceTypeWord("crops"); const size_t CROPS = i;
	_tiletypes[++i] = forceTypeWord("trenches"); const size_t TRENCHES = i;
	_tiletype_max = i;
	i = 0;
	// Implicit NONE at 0.
	_unittypes[++i] = forceTypeWord("rifleman"); const size_t RIFLEMAN = i;
	_unittypes[++i] = forceTypeWord("gunner"); const size_t GUNNER = i;
	_unittypes[++i] = forceTypeWord("sapper"); const size_t SAPPER = i;
	_unittypes[++i] = forceTypeWord("tank"); const size_t TANK = i;
	_unittypes[++i] = forceTypeWord("settler"); const size_t SETTLER = i;
	_unittypes[++i] = forceTypeWord("militia"); const size_t MILITIA = i;
	_unittypes[++i] = forceTypeWord("zeppelin"); const size_t ZEPPELIN = i;
	_unittype_max = i;

	/* TILES */
	i = GRASS;
	_tileAccessible[i] = true;
	_tileWalkable[i] = true;
	_tileBuildable[i] = true;
	_tileDestructible[i] = true;
	_tileGrassy[i] = true;
	_tileNatural[i] = true;
	_tilePlane[i] = true;
	_tileFlammable[i] = true;
	//_tileFirestormResistance[i] = 10;
	_tileRegrowOnlyInSpring[i] = true;
	_tileRegrowthProbabilityDivisor[i] = 4;
	_tileRegrowthAmount[i] = 1;
	_tileDestroyed[i] = (TileType) DIRT;
	_tileDegraded[i] = (TileType) DIRT;
	_tileRegrown[i] = (TileType) FOREST;
	_tileScoreBase[i] = 1;

	i = DIRT;
	_tileAccessible[i] = true;
	_tileWalkable[i] = true;
	_tileBuildable[i] = true;
	_tileGrassy[i] = false;
	_tileNatural[i] = false;
	_tilePlane[i] = true;
	_tileDesertified[i] = (TileType) DESERT;

	i = DESERT;
	_tileAccessible[i] = true;
	_tileWalkable[i] = true;
	_tileBuildable[i] = false;
	_tileGrassy[i] = false;
	_tileNatural[i] = false;
	_tilePlane[i] = true;
	_tileDesert[i] = true;

	i = RUBBLE;
	_tileAccessible[i] = true;
	_tileWalkable[i] = true;
	_tileBuildable[i] = false;
	_tileGrassy[i] = false;
	_tileNatural[i] = false;
	_tilePlane[i] = true;
	_tileStone[i] = true;
	_tileEmission[i] = 1;

	i = RIDGE;
	_tileAccessible[i] = true;
	_tileWalkable[i] = false;
	_tileGrassy[i] = false;
	_tileNatural[i] = false;
	_tileMountain[i] = false;
	_tileStacksMax[i] = 4;
	_tileStacksBuilt[i] = 4;
	_tileHitpoints[i] = 100;

	i = MOUNTAIN;
	_tileAccessible[i] = true;
	_tileWalkable[i] = false;
	_tileGrassy[i] = false;
	_tileNatural[i] = false;
	_tileMountain[i] = true;
	_tileStacksMax[i] = 5;
	_tileStacksBuilt[i] = 5;
	_tileHitpoints[i] = 100;

	i = WATER;
	_tileAccessible[i] = true;
	_tileWalkable[i] = false;
	_tileGrassy[i] = false;
	_tileNatural[i] = true;
	_tilePlane[i] = false;
	_tileWater[i] = true;

	i = FOREST;
	_tileAccessible[i] = true;
	_tileWalkable[i] = true;
	_tileBuildable[i] = true;
	_tileDestructible[i] = true;
	_tileGrassy[i] = true;
	_tileNatural[i] = true;
	_tileFlammable[i] = true;
	//_tileFirestormResistance[i] = 10;
	_tileChaosProtection[i] = true;
	_tileRegrowOnlyInSpring[i] = true;
	_tileStacksBuilt[i] = 1;
	_tileStacksMax[i] = 5;
	_tileHitpoints[i] = 1;
	_tileRegrowthProbabilityDivisor[i] = 4;
	_tileRegrowthAmount[i] = 1;
	_tileDestroyed[i] = (TileType) GRASS;
	_tileRegrown[i] = (TileType) FOREST;
	_tileScoreBase[i] = 1;

	i = CITY;
	_tileAccessible[i] = true;
	_tileWalkable[i] = true;
	_tileDestructible[i] = true;
	_tileLaboring[i] = true;
	_tilePowered[i] = true;
	_tileNeedsNiceness[i] = true;
	_tileOwnable[i] = true;
	_tileControllable[i] = true;
	_tileStone[i] = true;
	_tileBinding[i] = true;
	_tileStacksBuilt[i] = 1;
	_tileStacksMax[i] = 5;
	_tilePowerBuilt[i] = 1;
	_tilePowerMax[i] = 5;
	_tileGrowthMax[i] = 5;
	_tileVision[i] = 2;
	_tileHitpoints[i] = 2;
	_tileIncome[i] = 1;
	_tileEmission[i] = 2;
	//_tileFlammable[i] = true;
	//_tileFirestormResistance[i] = 70;
	_tileProduces[i] = {{(UnitType) MILITIA, 10}, (UnitType) SETTLER};
	_tileExpands[i] = {(TileType) INDUSTRY, (TileType) BARRACKS};
	_tileCost[i] = 50;
	_tileDestroyed[i] = (TileType) RUBBLE;

	i = TOWN;
	_tileAccessible[i] = true;
	_tileWalkable[i] = true;
	_tileDestructible[i] = true;
	_tileLaboring[i] = false;
	_tilePowered[i] = true;
	_tileNeedsNiceness[i] = true;
	_tileOwnable[i] = true;
	_tileControllable[i] = true;
	_tileStone[i] = true;
	_tileStacksBuilt[i] = 1;
	_tileStacksMax[i] = 5;
	_tilePowerBuilt[i] = 1;
	_tilePowerMax[i] = 5;
	_tileGrowthMax[i] = 3;
	_tileVision[i] = 2;
	_tileHitpoints[i] = 2;
	_tileIncome[i] = 1;
	_tileEmission[i] = 1;
	//_tileFlammable[i] = true;
	//_tileFirestormResistance[i] = 70;
	_tileProduces[i] = {(UnitType) SETTLER};
	_tileUpgrades[i] = {{TileType::NONE, 4}, {(TileType) CITY, 40}};
	_tileCost[i] = 10;
	_tileDestroyed[i] = (TileType) RUBBLE;

	i = OUTPOST;
	_tileAccessible[i] = true;
	_tileWalkable[i] = true;
	_tileDestructible[i] = true;
	_tileLaboring[i] = false;
	_tilePowered[i] = true;
	_tileNeedsNiceness[i] = true;
	_tileOwnable[i] = true;
	_tileControllable[i] = true;
	_tileStone[i] = true;
	_tileStacksBuilt[i] = 1;
	_tileStacksMax[i] = 2;
	_tilePowerBuilt[i] = 1;
	_tilePowerMax[i] = 2;
	_tileGrowthMax[i] = 2;
	_tileVision[i] = 2;
	_tileHitpoints[i] = 2;
	_tileIncome[i] = 0;
	_tileEmission[i] = 1;
	//_tileFlammable[i] = true;
	//_tileFirestormResistance[i] = 70;
	_tileProduces[i] = {(UnitType) SETTLER, (UnitType) MILITIA};
	_tileUpgrades[i] = {};
	_tileCost[i] = 0;
	_tileDestroyed[i] = (TileType) RUBBLE;

	i = INDUSTRY;
	_tileAccessible[i] = true;
	_tileWalkable[i] = true;
	_tileDestructible[i] = true;
	_tileEnergizing[i] = true;
	_tilePowered[i] = true;
	_tileNeedsLabor[i] = true;
	_tileOwnable[i] = true;
	_tileControllable[i] = true;
	_tileStone[i] = true;
	_tileStacksBuilt[i] = 1;
	_tileStacksMax[i] = 3;
	_tilePowerBuilt[i] = 1;
	_tilePowerMax[i] = 3;
	_tileGrowthMax[i] = 1;
	_tileVision[i] = 2;
	_tileHitpoints[i] = 3;
	_tileIncome[i] = 3;
	_tileLeakGas[i] = 1;
	_tileEmission[i] = 5;
	_tilePollutionAmount[i] = 1;
	_tilePollutionRadius[i] = 2;
	//_tileFlammable[i] = true;
	//_tileFirestormResistance[i] = 70;
	_tileProduces[i] = {(UnitType) TANK};
	_tileExpands[i] = {(TileType) AIRFIELD};
	_tileUpgrades[i] = {{TileType::NONE, 50}};
	_tileCost[i] = 5;
	_tileDestroyed[i] = (TileType) RUBBLE;

	i = BARRACKS;
	_tileAccessible[i] = true;
	_tileWalkable[i] = true;
	_tileDestructible[i] = true;
	_tilePowered[i] = true;
	_tileNeedsLabor[i] = true;
	_tileOwnable[i] = true;
	_tileControllable[i] = true;
	_tileStone[i] = true;
	_tileStacksBuilt[i] = 1;
	_tileStacksMax[i] = 3;
	_tilePowerBuilt[i] = 1;
	_tilePowerMax[i] = 3;
	_tileGrowthMax[i] = 1;
	_tileVision[i] = 2;
	_tileHitpoints[i] = 3;
	_tileEmission[i] = 1;
	//_tileFlammable[i] = true;
	//_tileFirestormResistance[i] = 70;
	_tileProduces[i] = {(UnitType) RIFLEMAN, (UnitType) GUNNER, (UnitType) SAPPER};
	_tileUpgrades[i] = {{TileType::NONE, 25}};
	_tileCost[i] = 5;
	_tileDestroyed[i] = (TileType) RUBBLE;

	i = AIRFIELD;
	_tileAccessible[i] = true;
	_tileWalkable[i] = true;
	_tileDestructible[i] = true;
	_tilePowered[i] = true;
	_tileNeedsEnergy[i] = true;
	_tileOwnable[i] = true;
	_tileControllable[i] = true;
	_tileStone[i] = true;
	_tileStacksBuilt[i] = 1;
	_tileStacksMax[i] = 1;
	_tilePowerBuilt[i] = 1;
	_tilePowerMax[i] = 1;
	_tileGrowthMax[i] = 1;
	_tileVision[i] = 2;
	_tileHitpoints[i] = 3;
	_tileLeakGas[i] = 1;
	_tileEmission[i] = 1;
	//_tileFlammable[i] = true;
	//_tileFirestormResistance[i] = 70;
	_tileProduces[i] = {(UnitType) ZEPPELIN};
	_tileCost[i] = 5;
	_tileDestroyed[i] = (TileType) RUBBLE;

	i = FARM;
	_tileAccessible[i] = true;
	_tileWalkable[i] = true;
	_tileDestructible[i] = true;
	_tilePowered[i] = true;
	_tileNeedsNiceness[i] = true;
	_tileOwnable[i] = true;
	_tileControllable[i] = true;
	_tileAutoCultivates[i] = true;
	_tileStone[i] = true;
	_tileStacksBuilt[i] = 1;
	_tileStacksMax[i] = 2;
	_tilePowerBuilt[i] = 1;
	_tilePowerMax[i] = 2;
	_tileGrowthMax[i] = 2;
	_tileVision[i] = 2;
	_tileHitpoints[i] = 2;
	_tileIncome[i] = 0;
	_tileEmission[i] = 1;
	//_tileFlammable[i] = true;
	//_tileFirestormResistance[i] = 70;
	_tileProduces[i] = {(UnitType) SETTLER, (UnitType) MILITIA};
	_tileCultivates[i] = {(TileType) SOIL};
	_tileCost[i] = 5;
	_tileDestroyed[i] = (TileType) RUBBLE;

	i = SOIL;
	_tileAccessible[i] = true;
	_tileWalkable[i] = true;
	_tileBuildable[i] = true;
	_tileDestructible[i] = true;
	_tilePowered[i] = false;
	_tileOwnable[i] = true;
	_tileControllable[i] = false;
	_tileGrassy[i] = false;
	_tileNatural[i] = false;
	_tilePlane[i] = true;
	_tileFlammable[i] = true;
	//_tileFirestormResistance[i] = 10;
	_tileRegrowOnlyInSpring[i] = false;
	_tileVision[i] = 0;
	_tileIncome[i] = 0;
	_tileRegrowthProbabilityDivisor[i] = 1;
	_tileCost[i] = 0;
	_tileDestroyed[i] = (TileType) DIRT;
	_tileDesertified[i] = (TileType) DESERT;
	_tileRegrown[i] = (TileType) CROPS;

	i = CROPS;
	_tileAccessible[i] = true;
	_tileWalkable[i] = true;
	_tileBuildable[i] = true;
	_tileDestructible[i] = true;
	_tilePowered[i] = false;
	_tileOwnable[i] = true;
	_tileControllable[i] = false;
	_tileGrassy[i] = false;
	_tileNatural[i] = true;
	_tilePlane[i] = true;
	_tileFlammable[i] = true;
	//_tileFirestormResistance[i] = 10;
	_tileVision[i] = 0;
	_tileIncome[i] = 1;
	_tileDestroyed[i] = (TileType) DIRT;
	_tileDegraded[i] = (TileType) SOIL;
	_tileConsumed[i] = (TileType) SOIL;

	i = TRENCHES;
	_tileAccessible[i] = true;
	_tileWalkable[i] = true;
	_tileBuildable[i] = false;
	_tileGrassy[i] = false;
	_tileNatural[i] = false;
	_tilePowered[i] = false;
	_tileOwnable[i] = false;
	_tileControllable[i] = false;
	_tilePlane[i] = false;
	_tileFlammable[i] = true;
	//_tileFirestormResistance[i] = 10;
	_tileTrenches[i] = true;
	_tileForceOccupy[i] = true;
	_tileCost[i] = 0;

	_tileExpandRangeMin = 1;
	_tileExpandRangeMax = 1;
	_tileProduceRangeMin = 0;
	_tileProduceRangeMax = 1;

	/* UNITS */
	i = RIFLEMAN;
	_unitInfantry[i] = true;
	_unitCanMove[i] = true;
	_unitCanAttack[i] = true;
	_unitCanFocus[i] = true;
	_unitCanCapture[i] = true;
	_unitCanOccupy[i] = true;
	_unitStacksMax[i] = 3;
	_unitSpeed[i] = 2;
	_unitVision[i] = 4;
	_unitHitpoints[i] = 2;
	_unitAttackShots[i] = 1;
	_unitAttackDamage[i] = 1;
	_unitShapes[i] = {(TileType) TRENCHES};
	_unitCost[i] = 5;

	i = GUNNER;
	_unitInfantry[i] = true;
	_unitCanMove[i] = true;
	_unitCanAttack[i] = true;
	_unitCanFocus[i] = true;
	_unitCanLockdown[i] = true;
	_unitCanOccupy[i] = true;
	_unitCanCapture[i] = true;
	_unitStacksMax[i] = 3;
	_unitSpeed[i] = 2;
	_unitVision[i] = 4;
	_unitHitpoints[i] = 2;
	_unitAttackShots[i] = 3;
	_unitAttackDamage[i] = 1;
	_unitCost[i] = 15;

	i = SAPPER;
	_unitInfantry[i] = true;
	_unitCanMove[i] = true;
	_unitCanAttack[i] = true;
	_unitCanFocus[i] = true;
	_unitCanBombard[i] = true;
	_unitCanOccupy[i] = true;
	_unitCanCapture[i] = true;
	_unitStacksMax[i] = 3;
	_unitSpeed[i] = 3;
	_unitVision[i] = 4;
	_unitHitpoints[i] = 1;
	_unitAttackShots[i] = 1;
	_unitAttackDamage[i] = 1;
	_unitAbilityVolleys[i] = 1;
	_unitAbilityShots[i] = 1;
	_unitAbilityDamage[i] = 3;
	_unitRangeMin[i] = 2;
	_unitRangeMax[i] = 10;
	_unitCost[i] = 15;

	i = TANK;
	_unitMechanical[i] = true;
	_unitCanMove[i] = true;
	_unitCanAttack[i] = false;
	_unitCanFocus[i] = false;
	_unitCanShell[i] = true;
	_unitCanOccupy[i] = true;
	_unitStacksMax[i] = 3;
	_unitSpeed[i] = 3;
	_unitVision[i] = 2;
	_unitHitpoints[i] = 3;
	_unitAttackShots[i] = 0;
	_unitAttackDamage[i] = 0;
	_unitTrampleShots[i] = 1;
	_unitTrampleDamage[i] = 1;
	_unitAbilityVolleys[i] = 2;
	_unitAbilityShots[i] = 1;
	_unitAbilityDamage[i] = 3;
	_unitRangeMin[i] = 1;
	_unitRangeMax[i] = 1;
	_unitCost[i] = 25;

	i = SETTLER;
	_unitCanMove[i] = true;
	_unitCanOccupy[i] = true;
	_unitStacksMax[i] = 1;
	_unitSpeed[i] = 3;
	_unitVision[i] = 2;
	_unitHitpoints[i] = 1;
	_unitSettles[i] = {(TileType) OUTPOST, (TileType) TOWN, (TileType) FARM};
	_unitCost[i] = 1;

	i = MILITIA;
	_unitInfantry[i] = true;
	_unitCanMove[i] = true;
	_unitCanAttack[i] = true;
	_unitCanFocus[i] = true;
	_unitCanCapture[i] = false;
	_unitCanOccupy[i] = true;
	_unitStacksMax[i] = 3;
	_unitSpeed[i] = 3;
	_unitVision[i] = 4;
	_unitHitpoints[i] = 1;
	_unitAttackShots[i] = 1;
	_unitAttackDamage[i] = 1;
	_unitSettles[i] = {(TileType) OUTPOST};
	_unitCost[i] = 5;

	i = ZEPPELIN;
	_unitAir[i] = true;
	_unitMechanical[i] = true;
	_unitCanMove[i] = true;
	_unitCanBomb[i] = true;
	_unitCanOccupy[i] = false;
	_unitStacksMax[i] = 1;
	_unitSpeed[i] = 1;
	_unitVision[i] = 10;
	_unitHitpoints[i] = 3;
	_unitAbilityVolleys[i] = 1;
	_unitAbilityShots[i] = 1;
	_unitAbilityDamage[i] = 0;
	_unitRangeMin[i] = 0;
	_unitRangeMax[i] = 0;
	_unitAbilityGas[i] = 2;
	_unitLeakGas[i] = 1;
	_unitCost[i] = 25;

	for (i = 0; i < UNITTYPE_SIZE; i++)
	{
		if (_unitSizeMax < _unitStacksMax[i])
		{
			_unitSizeMax = _unitStacksMax[i];
		}
		if (_unitVisionMax < _unitVision[i])
		{
			_unitVisionMax = _unitVision[i];
		}
	}

	/* COMBAT */
	_missCountGround = 1;
	_missCountTrenches = 3;
	_missHitpointsGround = 1;
	_missHitpointsTrenches = 1;

	/* WEATHER */
	_emissionDivisor = 2;

	_gasPollutionAmount = 1;
	_aridificationAmountHumid = 1;
	_aridificationAmountDegraded = 1;

	_aridificationRange = 2;

	_aridificationCount = 4;
	_firestormCount = 5;
	_deathCount = 1;

	_humidityMax = 4;
	_humidityMin = 0;
	_chaosMax = 1;
	_chaosMin = 0;
	_gasMax = 2;
	_gasMin = 0;

	_chaosThreshold = 25;

	i = (size_t) Season::SPRING;
	_humidityMinWet[i] = 1;
	_humidityMaxDegradation[i] = 0;
	_humidityMaxDesertification[i] = 0;
	_humidityMinSnow[i] = 4;
	_humidityMaxBonedrought[i] = 0;
	_humidityMaxStonedrought[i] = 0;
	_chaosMinDegradation[i] = 0;
	_chaosMinDesertification[i] = 2 * _chaosThreshold;
	_chaosMinAridification[i] = -1;
	_chaosMinSnow[i] = 0;
	_chaosMinFrostbite[i] = -1;
	_chaosMinFirestorm[i] = -1;
	_chaosMinBonedrought[i] = 3 * _chaosThreshold;
	_chaosMinStonedrought[i] = 4 * _chaosThreshold;
	_chaosMinDeath[i] = -1;

	i = (size_t) Season::SUMMER;
	_humidityMinWet[i] = 1;
	_humidityMaxDegradation[i] = 0;
	_humidityMaxDesertification[i] = 0;
	_humidityMinSnow[i] = 4;
	_humidityMaxFirestorm[i] = 4;
	_humidityMaxBonedrought[i] = 0;
	_humidityMaxStonedrought[i] = 0;
	_chaosMinDegradation[i] = 0;
	_chaosMinDesertification[i] = 2 * _chaosThreshold;
	_chaosMinAridification[i] = -1;
	_chaosMinSnow[i] = 0;
	_chaosMinFrostbite[i] = -1;
	_chaosMinFirestorm[i] = 2 * _chaosThreshold;
	_chaosMinBonedrought[i] = 3 * _chaosThreshold;
	_chaosMinStonedrought[i] = 4 * _chaosThreshold;
	_chaosMinDeath[i] = -1;

	i = (size_t) Season::AUTUMN;
	_humidityMinWet[i] = 1;
	_humidityMaxDegradation[i] = 0;
	_humidityMaxDesertification[i] = 0;
	_humidityMinSnow[i] = 4;
	_humidityMaxBonedrought[i] = 0;
	_humidityMaxStonedrought[i] = 0;
	_chaosMinDegradation[i] = 0;
	_chaosMinDesertification[i] = 2 * _chaosThreshold;
	_chaosMinAridification[i] = 1 * _chaosThreshold;
	_chaosMinSnow[i] = 0;
	_chaosMinFrostbite[i] = -1;
	_chaosMinFirestorm[i] = -1;
	_chaosMinBonedrought[i] = 3 * _chaosThreshold;
	_chaosMinStonedrought[i] = 4 * _chaosThreshold;
	_chaosMinDeath[i] = -1;

	i = (size_t) Season::WINTER;
	_humidityMinWet[i] = 1;
	_humidityMaxDegradation[i] = 0;
	_humidityMaxDesertification[i] = 0;
	_humidityMinSnow[i] = 1;
	_humidityMinFrostbite[i] = 0;
	_humidityMaxBonedrought[i] = 0;
	_humidityMaxStonedrought[i] = 0;
	_chaosMinDegradation[i] = 0;
	_chaosMinDesertification[i] = 2 * _chaosThreshold;
	_chaosMinAridification[i] = -1;
	_chaosMinSnow[i] = 0;
	_chaosMinFrostbite[i] = 1 * _chaosThreshold;
	_chaosMinFirestorm[i] = -1;
	_chaosMinBonedrought[i] = 3 * _chaosThreshold;
	_chaosMinStonedrought[i] = 4 * _chaosThreshold;
	_chaosMinDeath[i] = -1;

	_frostbiteShots = 2;
	_frostbiteDamage = 1;
	_frostbiteThresholdDamage = 1;
	_frostbiteThresholdVulnerability = 100;

	_firestormShots = 3;
	_firestormDamage = 2;
	//_firestormBasePercentage = 30;
	//_firestormDroughtPercentage = 20;

	_gasShots = 3;
	_gasDamage = 1;
	_gasThresholdDamage = 1;
	_gasThresholdVulnerability = 1;

	_radiationShots = 3;
	_radiationDamage = 1;
	_radiationThresholdDamage = 2;
	_radiationThresholdVulnerability = 1;
	_radiationThresholdDeath = 3;

	_humGenDefault = 2;
	_humGenGainRange = 5;

	// Distance from src =    0    1    2   -    4    5.
	_humGenLakeGain      = { +1,  +1,  +1,  0,  +1,  +1};
	_humGenOceanGain     = { +1,  +1,  +1,  0,   0,   0};
	_humGenDesertGain    = { -2,  -1,  -1,  0,   0,   0};
	_humGenMountainGain  = { +3,  +2,  +2,  0,  +1,  +1};

	/* MECHANICS */
	_powerAbolished = false;
	_separatePowerStages = true;
	_industryNicenessQuantitative = false;
	_reactorNicenessQuantitative = false;
	_forestChaosProtectionPermanent = true;
	_collateralDamageKillsTiles = true;
	_gasOnlyTargetsGroundUnits = true;
	_frostbiteOnlyTargetsGroundUnits = true;
	_trenchesProtectMovingUnits = false;
	_trenchesProtectBypassingUnits = false;
	_trenchesHideBypassedUnit = true;
	_captureStrengthCheck = false;
	_focusOnce = true;
	_powerDrainScaled = true;
	_upgradeKeepPower = true;
	_upgradeKeepStacks = true;
	_upgradeNotBlockedByFriendly = true;
	_groundPollutionOnlyInAutumn = true;
	_counterBasedWeather = true;
	_quantitativeChaos = true;
	_stackBasedFrostbite = false;
	_emptyBasedFrostbite = false;
	_planeBasedFrostbite = true;
	_planeBasedAridification = true;
	_flammableBasedFirestorm = true;
	_percentageBasedFirestorm = false;
	_randomizedFirestorm = true;
	_randomizedAridification = true;
	_cumulativeDeath = true;
	_vulnerabilitiesStack = true;
	_markersChangeAtNight = false;
	_markersChangeAfterBuild = true;
	_publicInitiative = true;
	_weatherAfterNight = true;
	_moraleReplacesMoney = false;
	_moraleGatheredInSummer = false;
	_moraleGatheredWhenBuilt = false;
	_snowCoversNiceness = true;

	_snowSlowAmount = 1;
	_snowSlowMaximum = 1;

	/* COMMANDERS */
	_startingMoney = 20;
	_newOrderLimit = 5;
}

void Bible::customize()
{
	const size_t CITY = (size_t) tiletype("city");
	const size_t BARRACKS = (size_t) tiletype("barracks");
	const size_t RIFLEMAN = (size_t) unittype("rifleman");

	_tileExpands[CITY] = {(TileType) BARRACKS};
	_tileProduces[BARRACKS] = {(UnitType) RIFLEMAN};
}

void Bible::finalize()
{
	/* COUNT NAMED TYPES */
	if (_tiletype_max >= TILETYPE_SIZE)
	{
		LOGE << "TileType size limit (" << TILETYPE_SIZE << ") exceeded";
	}
	if (_unittype_max >= UNITTYPE_SIZE)
	{
		LOGE << "UnitType size limit (" << UNITTYPE_SIZE << ") exceeded";
	}
	DEBUG_ASSERT(_tiletype_max < TILETYPE_SIZE);
	DEBUG_ASSERT(_unittype_max < UNITTYPE_SIZE);

	/* FILLING IN THE BLANKS */
	for (size_t i = 0; i < TILETYPE_SIZE; i++)
	{
		FILLBUILDLIST(_tileProduces[i], _unitCost)
		FILLBUILDLIST(_tileExpands[i], _tileCost)
		FILLBUILDLIST(_tileUpgrades[i], _tileCost)
		FILLBUILDLIST(_tileCultivates[i], _tileCost)
	}
	for (size_t i = 0; i < UNITTYPE_SIZE; i++)
	{
		FILLBUILDLIST(_unitShapes[i], _tileCost)
		FILLBUILDLIST(_unitSettles[i], _tileCost)
	}
}

static void Bible_log_sizelimit_reached(const char* NAME, size_t SIZE)
{
	LOGW << "Size limit (" << SIZE << ") reached at " << NAME;
}

#define PARSETYPEWORDS(VARIABLE, NAME, MAX, SIZE) \
	if (!json[NAME].isNull()) \
	{ \
		int i = 1; \
		for (const Json::Value& item : json[NAME]) \
		{ \
			if ((size_t) i >= SIZE) \
			{ \
				Bible_log_sizelimit_reached(NAME, SIZE); \
				break; \
			} \
			if (item.asString() == "none" || item.asString() == "") continue; \
			VARIABLE[i] = parseTypeWord(item.asString().c_str()); \
			MAX = i; \
			i++; \
		} \
	}

#define PARSEBOOL(VARIABLE, NAME) \
	if (!json[NAME].isNull()) \
	{ \
		const Json::Value& item = json[NAME]; \
		VARIABLE = item.asBool(); \
	}

#define PARSEINT(VARIABLE, NAME) \
	if (!json[NAME].isNull()) \
	{ \
		const Json::Value& item = json[NAME]; \
		VARIABLE = item.asInt(); \
	}

#define PARSETILEBOOLS(VARIABLE, NAME) \
	if (!json[NAME].isNull()) \
	{ \
		const Json::Value& map = json[NAME]; \
		for (Json::Value::const_iterator iter = map.begin(); iter != map.end(); ++iter) \
		{ \
			TileType type = tiletype(iter.key().asString().c_str()); \
			bool val = iter->asBool(); \
			VARIABLE[(size_t) type] = val; \
		} \
	}

#define PARSETILEINTS(VARIABLE, NAME) \
	if (!json[NAME].isNull()) \
	{ \
		const Json::Value& map = json[NAME]; \
		for (Json::Value::const_iterator iter = map.begin(); iter != map.end(); ++iter) \
		{ \
			TileType type = tiletype(iter.key().asString().c_str()); \
			int val = iter->asInt(); \
			VARIABLE[(size_t) type] = val; \
		} \
	}

#define PARSETILETILES(VARIABLE, NAME) \
	if (!json[NAME].isNull()) \
	{ \
		const Json::Value& map = json[NAME]; \
		for (Json::Value::const_iterator iter = map.begin(); iter != map.end(); ++iter) \
		{ \
			TileType type = tiletype(iter.key().asString().c_str()); \
			TileType val = tiletype(iter->asString().c_str()); \
			VARIABLE[(size_t) type] = val; \
		} \
	}

#define PARSETILEUNITLISTS(VARIABLE, NAME) \
	if (!json[NAME].isNull()) \
	{ \
		const Json::Value& map = json[NAME]; \
		for (Json::Value::const_iterator iter = map.begin(); \
			iter != map.end(); ++iter) \
		{ \
			TileType type = tiletype(iter.key().asString().c_str()); \
			auto& builds = VARIABLE[(size_t) type]; \
			const Json::Value& list = *iter; \
			for (const Json::Value& item : list) \
			{ \
				if (item.isString()) \
				{ \
					UnitType itemtype = unittype(item.asString().c_str()); \
					builds.push_back(itemtype); \
				} \
				else \
				{ \
					UnitBuild itembuild( \
						unittype(item["type"].asString().c_str()), \
						0); \
					PARSECOSTFORBUILD(itembuild, item) \
					builds.push_back(itembuild); \
				} \
			} \
		} \
	}

#define PARSECOSTFORBUILD(BUILD, JSON) \
	/*{*/ \
		if (!JSON["moneycost"].isNull()) \
		{ \
			BUILD.cost = JSON["moneycost"].asInt(); \
		} \
		else \
		{ \
			BUILD.cost = JSON["cost"].asInt(); \
		} \
	/*}*/

#define PARSETILETILELISTS(VARIABLE, NAME) \
	if (!json[NAME].isNull()) \
	{ \
		const Json::Value& map = json[NAME]; \
		for (Json::Value::const_iterator iter = map.begin(); \
			iter != map.end(); ++iter) \
		{ \
			TileType type = tiletype(iter.key().asString().c_str()); \
			auto& builds = VARIABLE[(size_t) type]; \
			const Json::Value& list = *iter; \
			for (const Json::Value& item : list) \
			{ \
				if (item.isString()) \
				{ \
					TileType itemtype = tiletype(item.asString().c_str()); \
					builds.push_back(itemtype); \
				} \
				else \
				{ \
					TileBuild itembuild( \
						tiletype(item["type"].asString().c_str()), \
						0); \
					PARSECOSTFORBUILD(itembuild, item) \
					builds.push_back(itembuild); \
				} \
			} \
		} \
	}

#define PARSEUNITBOOLS(VARIABLE, NAME) \
	if (!json[NAME].isNull()) \
	{ \
		const Json::Value& map = json[NAME]; \
		for (Json::Value::const_iterator iter = map.begin(); \
			iter != map.end(); ++iter) \
		{ \
			UnitType type = unittype(iter.key().asString().c_str()); \
			bool val = iter->asBool(); \
			VARIABLE[(size_t) type] = val; \
		} \
	}

#define PARSEUNITINTS(VARIABLE, NAME) \
	if (!json[NAME].isNull()) \
	{ \
		const Json::Value& map = json[NAME]; \
		for (Json::Value::const_iterator iter = map.begin(); \
			iter != map.end(); ++iter) \
		{ \
			UnitType type = unittype(iter.key().asString().c_str()); \
			int val = iter->asInt(); \
			VARIABLE[(size_t) type] = val; \
		} \
	}

#define PARSEUNITUNITLISTS(VARIABLE, NAME) \
	if (!json[NAME].isNull()) \
	{ \
		const Json::Value& map = json[NAME]; \
		for (Json::Value::const_iterator iter = map.begin(); \
			iter != map.end(); ++iter) \
		{ \
			UnitType type = unittype(iter.key().asString().c_str()); \
			auto& builds = VARIABLE[(size_t) type]; \
			const Json::Value& list = *iter; \
			for (const Json::Value& item : list) \
			{ \
				if (item.isString()) \
				{ \
					UnitType itemtype = unittype(item.asString().c_str()); \
					builds.push_back(itemtype); \
				} \
				else \
				{ \
					UnitBuild itembuild( \
						unittype(item["type"].asString().c_str()), \
						0); \
					PARSECOSTFORBUILD(itembuild, item) \
					builds.push_back(itembuild); \
				} \
			} \
		} \
	}

#define PARSEUNITTILELISTS(VARIABLE, NAME) \
	if (!json[NAME].isNull()) \
	{ \
		const Json::Value& map = json[NAME]; \
		for (Json::Value::const_iterator iter = map.begin(); \
			iter != map.end(); ++iter) \
		{ \
			UnitType type = unittype(iter.key().asString().c_str()); \
			auto& builds = VARIABLE[(size_t) type]; \
			const Json::Value& list = *iter; \
			for (const Json::Value& item : list) \
			{ \
				if (item.isString()) \
				{ \
					TileType itemtype = tiletype(item.asString().c_str()); \
					builds.push_back(itemtype); \
				} \
				else \
				{ \
					TileBuild itembuild( \
						tiletype(item["type"].asString().c_str()), \
						0); \
					PARSECOSTFORBUILD(itembuild, item) \
					builds.push_back(itembuild); \
				} \
			} \
		} \
	}

#define PARSESEASONINTS(VARIABLE, NAME) \
	if (json[NAME].isInt()) \
	{ \
		const Json::Value& item = json[NAME]; \
		int val = item.asInt(); \
		for (size_t i = 0; i < SEASON_SIZE; i++)\
		{ \
			VARIABLE[i] = val; \
		} \
	} \
	else if (!json[NAME].isNull()) \
	{ \
		const Json::Value& map = json[NAME]; \
		for (Json::Value::const_iterator iter = map.begin(); \
			iter != map.end(); ++iter) \
		{ \
			Season season = parseSeason(iter.key().asString().c_str()); \
			int val = iter->asInt(); \
			VARIABLE[(size_t) season] = val; \
		} \
	}

#define AUTOPARSERANGEINTS(X, MAX) PARSERANGEINTS(VARIABLE(X), NAME(X), MAX)
#define PARSERANGEINTS(VARIABLE, NAME, MAX) \
	VARIABLE.resize(MAX + 1, 0); \
	if (!json[NAME].isNull()) \
	{ \
		const Json::Value& list = json[NAME]; \
		for (int i = 0; i <= MAX; i++) \
		{ \
			int8_t val = list[i].asInt(); \
			VARIABLE[i] = val; \
		} \
	}

Bible::Bible(const std::string& biblename, const Json::Value& json) :
	Bible(biblename, Version(json["version"]))
{
	/* TYPES */
	PARSETYPEWORDS(_tiletypes, "tiletypes", _tiletype_max, TILETYPE_SIZE)
	else
	{
		// Backwards compatibility: these tiletypes existed in code at v0.31.0.
		size_t i = 0;
		// Implicit NONE at 0.
		_tiletypes[++i] = forceTypeWord("grass");
		_tiletypes[++i] = forceTypeWord("dirt");
		_tiletypes[++i] = forceTypeWord("desert");
		_tiletypes[++i] = forceTypeWord("stone");
		_tiletypes[++i] = forceTypeWord("rubble");
		_tiletypes[++i] = forceTypeWord("ridge");
		_tiletypes[++i] = forceTypeWord("mountain");
		_tiletypes[++i] = forceTypeWord("water");
		_tiletypes[++i] = forceTypeWord("forest");
		_tiletypes[++i] = forceTypeWord("city");
		_tiletypes[++i] = forceTypeWord("town");
		_tiletypes[++i] = forceTypeWord("settlement");
		_tiletypes[++i] = forceTypeWord("industry");
		_tiletypes[++i] = forceTypeWord("embassy");
		_tiletypes[++i] = forceTypeWord("barracks");
		_tiletypes[++i] = forceTypeWord("airfield");
		_tiletypes[++i] = forceTypeWord("reactor");
		_tiletypes[++i] = forceTypeWord("farm");
		_tiletypes[++i] = forceTypeWord("soil");
		_tiletypes[++i] = forceTypeWord("crops");
		_tiletypes[++i] = forceTypeWord("trenches");
		_tiletype_max = i;
	}
	PARSETYPEWORDS(_unittypes, "unittypes", _unittype_max, UNITTYPE_SIZE)
	else
	{
		// Backwards compatibility: these unittypes existed in code at v0.31.0.
		size_t i = 0;
		// Implicit NONE at 0
		_unittypes[++i] = forceTypeWord("rifleman");
		_unittypes[++i] = forceTypeWord("gunner");
		_unittypes[++i] = forceTypeWord("sapper");
		_unittypes[++i] = forceTypeWord("tank");
		_unittypes[++i] = forceTypeWord("settler");
		_unittypes[++i] = forceTypeWord("militia");
		_unittypes[++i] = forceTypeWord("diplomat");
		_unittypes[++i] = forceTypeWord("zeppelin");
		_unittypes[++i] = forceTypeWord("glider");
		_unittypes[++i] = forceTypeWord("nuke");
		_unittype_max = i;
	}

	/* TILES */
	AUTO(PARSETILEBOOLS, tileAccessible)
	AUTO(PARSETILEBOOLS, tileWalkable)
	AUTO(PARSETILEBOOLS, tileBuildable)
	AUTO(PARSETILEBOOLS, tileDestructible)
	AUTO(PARSETILEBOOLS, tileGrassy)
	AUTO(PARSETILEBOOLS, tileNatural)
	AUTO(PARSETILEBOOLS, tileLaboring)
	else
	{
		// Backwards compatibility: City and Settlement tiles provide labor.
		TileType citytype = tiletype("city");
		if (citytype != TileType::NONE)
		{
			_tileLaboring[(size_t) citytype] = true;
		}
		TileType settlementtype = tiletype("settlement");
		if (settlementtype != TileType::NONE)
		{
			_tileLaboring[(size_t) settlementtype] = true;
		}
	}
	AUTO(PARSETILEBOOLS, tileEnergizing)
	else
	{
		// Backwards compatibility: Industry tiles provide energy to airfields.
		TileType industrytype = tiletype("industry");
		if (industrytype != TileType::NONE)
		{
			_tileEnergizing[(size_t) industrytype] = true;
		}
	}
	AUTO(PARSETILEBOOLS, tilePowered)
	AUTO(PARSETILEBOOLS, tileNeedsNiceness)
	else
	{
		// Backwards compatibility: cities need niceness.
		for (const char* name : {"city", "town", "settlement", "farm"})
		{
			TileType namedtype = tiletype(name);
			if (namedtype != TileType::NONE)
			{
				_tileNeedsNiceness[(size_t) namedtype] = true;
			}
		}
	}
	AUTO(PARSETILEBOOLS, tileNeedsLabor)
	else
	{
		// Backwards compatibility: industry needs labor.
		for (const char* name : {"industry", "embassy", "barracks"})
		{
			TileType namedtype = tiletype(name);
			if (namedtype != TileType::NONE)
			{
				_tileNeedsLabor[(size_t) namedtype] = true;
			}
		}
	}
	AUTO(PARSETILEBOOLS, tileNeedsEnergy)
	else
	{
		// Backwards compatibility: reactor needs energy.
		for (const char* name : {"reactor", "airfield"})
		{
			TileType namedtype = tiletype(name);
			if (namedtype != TileType::NONE)
			{
				_tileNeedsEnergy[(size_t) namedtype] = true;
			}
		}
	}
	AUTO(PARSETILEBOOLS, tileNeedsTime)
	AUTO(PARSETILEBOOLS, tileOwnable)
	else
	{
		// Backwards compatibility: powered tiles are ownable.
		for (size_t i = 0; i < TILETYPE_SIZE; i++)
		{
			_tileOwnable[i] = _tilePowered[i];
		}
	}
	AUTO(PARSETILEBOOLS, tileControllable)
	else
	{
		// Backwards compatibility: powered tiles are controllable.
		for (size_t i = 0; i < TILETYPE_SIZE; i++)
		{
			_tileControllable[i] = _tilePowered[i];
		}
	}
	AUTO(PARSETILEBOOLS, tileBinding)
	else
	{
		// Backwards compatibility: players are bound to city tiles.
		TileType citytype = tiletype("city");
		if (citytype != TileType::NONE)
		{
			_tileBinding[(size_t) citytype] = true;
		}
	}
	AUTO(PARSETILEBOOLS, tileAutoCultivates)
	AUTO(PARSETILEBOOLS, tilePlane)
	AUTO(PARSETILEBOOLS, tileFlammable)
	AUTO(PARSETILEBOOLS, tileWater)
	else
	{
		// Backwards compatibility: water is water.
		TileType watertype = tiletype("water");
		if (watertype != TileType::NONE)
		{
			_tileWater[(size_t) watertype] = true;
		}
	}
	AUTO(PARSETILEBOOLS, tileMountain)
	else
	{
		// Backwards compatibility: mountain is mountain.
		TileType mountaintype = tiletype("mountain");
		if (mountaintype != TileType::NONE)
		{
			_tileMountain[(size_t) mountaintype] = true;
		}
	}
	AUTO(PARSETILEBOOLS, tileDesert)
	else
	{
		// Backwards compatibility: desert is desert.
		TileType deserttype = tiletype("desert");
		if (deserttype != TileType::NONE)
		{
			_tileDesert[(size_t) deserttype] = true;
		}
	}
	AUTO(PARSETILEBOOLS, tileStone)
	else
	{
		// Backwards compatibility: these tiles are stone-based.
		for (const char* name : {"stone", "rubble",
			"city", "town", "settlement", "farm",
			"industry", "barracks", "embassy",
			"airfield", "reactor"})
		{
			TileType namedtype = tiletype(name);
			if (namedtype != TileType::NONE)
			{
				_tileStone[(size_t) namedtype] = true;
			}
		}
	}
	AUTO(PARSETILEBOOLS, tileTrenches)
	else
	{
		// Backwards compatibility: trenches are trenches.
		TileType trenchestype = tiletype("trenches");
		if (trenchestype != TileType::NONE)
		{
			_tileTrenches[(size_t) trenchestype] = true;
		}
	}
	AUTO(PARSETILEBOOLS, tileForceOccupy)
	AUTO(PARSETILEBOOLS, tileChaosProtection)
	else
	{
		// Backwards compatibility: forests have protection.
		TileType foresttype = tiletype("forest");
		if (foresttype != TileType::NONE)
		{
			_tileChaosProtection[(size_t) foresttype] = true;
		}
	}
	AUTO(PARSETILEBOOLS, tileRegrowOnlyInSpring)
	AUTO(PARSETILEBOOLS, tileGathersMorale)

	AUTO(PARSETILEINTS, tileStacksBuilt)
	AUTO(PARSETILEINTS, tileStacksMax)
	AUTO(PARSETILEINTS, tilePowerBuilt)
	AUTO(PARSETILEINTS, tilePowerMax)
	AUTO(PARSETILEINTS, tileGrowthMax)
	else
	{
		// Backwards compatibility with v0.32.0-rc1: "tileManualStackGrowth"
		// indicates which tiles do not grow automatically as a result
		// of gaining power.
		std::bitset<TILETYPE_SIZE> tileManualStackGrowth;
		bool parsed = true;
		PARSETILEBOOLS(tileManualStackGrowth, "tileManualStackGrowth")
		else parsed = false;
		if (parsed)
		{
			for (size_t i = 0; i < TILETYPE_SIZE; i++)
			{
				if (tileManualStackGrowth[i]) _tileGrowthMax[i] = 1;
				// else _tileGrowthMax[i] = 0; /*unlimited*/
			}
		}
	}
	AUTO(PARSETILEINTS, tileVision)
	AUTO(PARSETILEINTS, tileHitpoints)

	AUTO(PARSETILEINTS, tileIncome)
	else
	{
		// Backwards compatibility: if tileIncome is not specified, all powered tiles have 1.
		for (size_t i = 0; i < TILETYPE_SIZE; i++)
		{
			_tileIncome[i] = (_tilePowered[i]) ? 1 : 0;
		}
	}

	AUTO(PARSETILEINTS, tileLeakGas)
	AUTO(PARSETILEINTS, tileLeakRads)
	AUTO(PARSETILEINTS, tileEmission)
	else PARSETILEINTS(_tileEmission, "tileEmitChaos")
	AUTO(PARSETILEINTS, tilePollutionAmount)
	AUTO(PARSETILEINTS, tilePollutionRadius)
	AUTO(PARSETILEINTS, tileSlowAmount)
	AUTO(PARSETILEINTS, tileSlowMaximum)
	AUTO(PARSETILEINTS, tileRegrowthProbabilityDivisor)
	AUTO(PARSETILEINTS, tileRegrowthAmount)
	AUTO(PARSETILEINTS, tileFirestormResistance)
	AUTO(PARSETILEINTS, tileMoraleGainWhenBuilt)
	AUTO(PARSETILEINTS, tileMoraleGainWhenLost)
	AUTO(PARSETILEINTS, tileMoraleGainWhenDestroyed)
	AUTO(PARSETILEINTS, tileMoraleGainWhenCaptured)
	AUTO(PARSETILEINTS, tileMoraleGainWhenRazed)
	AUTO(PARSETILEINTS, tileMoraleGainWhenGathered)

	AUTO(PARSETILEUNITLISTS, tileProduces)
	AUTO(PARSETILETILELISTS, tileExpands)
	else PARSETILETILELISTS(_tileExpands, "tileBuilds")
	AUTO(PARSETILETILELISTS, tileUpgrades)
	AUTO(PARSETILETILELISTS, tileCultivates)

	AUTO(PARSETILEINTS, tileCost)
	else PARSETILEINTS(_tileCost, "tileCostMoney")
	AUTO(PARSETILEINTS, tileScoreBase)
	else
	{
		// Backwards compatibility: old score system.
		TileType grasstype = tiletype("grass");
		if (grasstype != TileType::NONE)
		{
			_tileScoreBase[(size_t) grasstype] = 8;
		}
		TileType foresttype = tiletype("forest");
		if (foresttype != TileType::NONE)
		{
			_tileScoreBase[(size_t) foresttype] = 8;
		}
		TileType mountaintype = tiletype("mountain");
		if (mountaintype != TileType::NONE)
		{
			_tileScoreBase[(size_t) mountaintype] = 3;
		}
		TileType watertype = tiletype("water");
		if (watertype != TileType::NONE)
		{
			_tileScoreBase[(size_t) watertype] = 5;
		}
	}
	AUTO(PARSETILEINTS, tileScoreStack)
	else
	{
		// Backwards compatibility: old score system.
		TileType foresttype = tiletype("forest");
		if (foresttype != TileType::NONE)
		{
			_tileScoreStack[(size_t) foresttype] = 3;
		}
	}

	AUTO(PARSETILETILES, tileDestroyed)
	AUTO(PARSETILETILES, tileDegraded)
	else
	{
		// Backwards compatibility: grass degrades to dirt.
		TileType grasstype = tiletype("grass");
		TileType dirttype = tiletype("dirt");
		if (grasstype != TileType::NONE && dirttype != TileType::NONE)
		{
			_tileDegraded[(size_t) grasstype] = dirttype;
		}
		// (Crops and soil are defined by "cropsConsumedAtNight".)
	}
	AUTO(PARSETILETILES, tileDesertified)
	else
	{
		// Backwards compatibility: dirt desertifies.
		TileType dirttype = tiletype("dirt");
		TileType deserttype = tiletype("desert");
		if (dirttype != TileType::NONE && deserttype != TileType::NONE)
		{
			_tileDesertified[(size_t) dirttype] = deserttype;
		}
		// (Crops and soil are defined by "cropsConsumedAtNight".)
	}
	AUTO(PARSETILETILES, tileConsumed)
	AUTO(PARSETILETILES, tileRegrown)
	else
	{
		// Backwards compatibility: dirt regrows to grass and grass to forest.
		TileType dirttype = tiletype("dirt");
		TileType grasstype = tiletype("grass");
		TileType foresttype = tiletype("forest");
		if (dirttype != TileType::NONE && grasstype != TileType::NONE)
		{
			_tileRegrown[(size_t) dirttype] = grasstype;
		}
		if (grasstype != TileType::NONE && foresttype != TileType::NONE)
		{
			_tileRegrown[(size_t) grasstype] = foresttype;
		}
		if (foresttype != TileType::NONE)
		{
			_tileRegrown[(size_t) foresttype] = foresttype;
		}
		// (Crops and soil are defined by "cropsConsumedAtNight".)
	}

	AUTO(PARSEINT, tileExpandRangeMin)
	else PARSEINT(_tileExpandRangeMin, "tileBuildRangeMin")
	AUTO(PARSEINT, tileExpandRangeMax)
	else PARSEINT(_tileExpandRangeMax, "tileBuildRangeMax")
	AUTO(PARSEINT, tileProduceRangeMin)
	AUTO(PARSEINT, tileProduceRangeMax)

	/* UNITS */
	AUTO(PARSEUNITBOOLS, unitAir)
	AUTO(PARSEUNITBOOLS, unitMechanical)
	AUTO(PARSEUNITBOOLS, unitInfantry)
	AUTO(PARSEUNITBOOLS, unitCanMove)
	AUTO(PARSEUNITBOOLS, unitCanAttack)
	AUTO(PARSEUNITBOOLS, unitCanGuard)
	else
	{
		// Backwards compatibility: all units that can attack can guard.
		for (size_t i = 0; i < UNITTYPE_SIZE; i++)
		{
			_unitCanGuard[i] = _unitCanAttack[i];
		}
	}
	AUTO(PARSEUNITBOOLS, unitCanFocus)
	AUTO(PARSEUNITBOOLS, unitCanLockdown)
	AUTO(PARSEUNITBOOLS, unitCanShell)
	AUTO(PARSEUNITBOOLS, unitCanBombard)
	AUTO(PARSEUNITBOOLS, unitCanBomb)
	AUTO(PARSEUNITBOOLS, unitCanCapture)
	AUTO(PARSEUNITBOOLS, unitCanOccupy)

	AUTO(PARSEUNITINTS, unitStacksMax)
	else
	{
		// Backwards compatibility: number of stacks is always bounded by power, so max 5.
		for (size_t i = 0; i < UNITTYPE_SIZE; i++)
		{
			_unitStacksMax[i] = 5;
		}
	}
	AUTO(PARSEUNITINTS, unitSpeed)
	AUTO(PARSEUNITINTS, unitVision)
	AUTO(PARSEUNITINTS, unitHitpoints)
	AUTO(PARSEUNITINTS, unitAttackShots)
	AUTO(PARSEUNITINTS, unitAttackDamage)
	AUTO(PARSEUNITINTS, unitTrampleShots)
	AUTO(PARSEUNITINTS, unitTrampleDamage)
	AUTO(PARSEUNITINTS, unitAbilityShots)
	AUTO(PARSEUNITINTS, unitAbilityVolleys)
	else
	{
		// Backwards compatibility: a single volley.
		for (size_t i = 0; i < UNITTYPE_SIZE; i++)
		{
			if (_unitAbilityShots[i] > 0) _unitAbilityVolleys[i] = 1;
		}
	}
	AUTO(PARSEUNITINTS, unitAbilityDamage)
	AUTO(PARSEUNITINTS, unitAbilityGas)
	AUTO(PARSEUNITINTS, unitAbilityRads)
	AUTO(PARSEUNITINTS, unitAbilityRadius)
	AUTO(PARSEUNITINTS, unitRangeMin)
	AUTO(PARSEUNITINTS, unitRangeMax)
	AUTO(PARSEUNITINTS, unitLeakGas)
	AUTO(PARSEUNITINTS, unitLeakRads)
	AUTO(PARSEUNITINTS, unitMoraleGainWhenLost)
	AUTO(PARSEUNITINTS, unitMoraleGainWhenKilled)

	AUTO(PARSEUNITTILELISTS, unitShapes)
	AUTO(PARSEUNITTILELISTS, unitSettles)

	AUTO(PARSEUNITINTS, unitCost)
	else PARSEUNITINTS(_unitCost, "unitCostMoney")

	AUTO(PARSEINT, unitSizeMax)
	else _unitSizeMax = 5;
	AUTO(PARSEINT, unitVisionMax)

	/* COMBAT */
	AUTO(PARSEINT, missCountGround)
	AUTO(PARSEINT, missCountAir)
	AUTO(PARSEINT, missCountTrenches)
	AUTO(PARSEINT, missHitpointsGround)
	AUTO(PARSEINT, missHitpointsAir)
	AUTO(PARSEINT, missHitpointsTrenches)

	/* WEATHER */
	AUTO(PARSESEASONINTS, seasonTemperatureSwing)
	AUTO(PARSESEASONINTS, seasonGlobalWarmingFactor)

	AUTO(PARSEINT, emissionDivisor)
	// Backwards compatibility: hardcoded "forestGrowthProbabilityDivisor"; note
	// that this is now the _tileRegrowthProbabilityDivisor of forest.
	{
		int divisor = 0;
		PARSEINT(divisor, "forestGrowthProbabilityDivisor")
		TileType foresttype = tiletype("forest");
		if (foresttype != TileType::NONE)
		{
			if (divisor > 0)
			{
				_tileRegrowthProbabilityDivisor[(size_t) foresttype] = divisor;
			}
			else if (divisor < 0)
			{
				_tileRegrown[(size_t) foresttype] = TileType::NONE;
			}
		}
	}
	// Backwards compatibility: hardcoded "forestRegrowthProbabilityDivisor",
	// which is the probability that grass regrows into forest; note that this
	// is now the _tileRegrowthProbabilityDivisor of grass.
	{
		int divisor = 0;
		PARSEINT(divisor, "forestRegrowthProbabilityDivisor")
		TileType grasstype = tiletype("grass");
		if (grasstype != TileType::NONE)
		{
			if (divisor > 0)
			{
				_tileRegrowthProbabilityDivisor[(size_t) grasstype] = divisor;
			}
			else if (divisor < 0)
			{
				_tileRegrown[(size_t) grasstype] = TileType::NONE;
			}
		}
	}
	// Backwards compatibility: hardcoded "grassRegrowthProbabilityDivisor",
	// which is the probability that dirt regrows into grass; note that this
	// is now the _tileRegrowthProbabilityDivisor of dirt.
	{
		int divisor = 0;
		PARSEINT(divisor, "grassRegrowthProbabilityDivisor")
		TileType dirttype = tiletype("dirt");
		if (dirttype != TileType::NONE)
		{
			if (divisor > 0)
			{
				_tileRegrowthProbabilityDivisor[(size_t) dirttype] = divisor;
			}
			else if (divisor < 0)
			{
				_tileRegrown[(size_t) dirttype] = TileType::NONE;
			}
		}
	}
	// Backwards compatibility: hardcoded "cropsRegrowthProbabilityDivisor",
	// which is the probability that soil regrows into crops; note that this
	// is now the _tileRegrowthProbabilityDivisor of soil.
	{
		int divisor = 0;
		PARSEINT(divisor, "cropsRegrowthProbabilityDivisor")
		TileType soiltype = tiletype("soil");
		if (soiltype != TileType::NONE)
		{
			if (divisor > 0)
			{
				_tileRegrowthProbabilityDivisor[(size_t) soiltype] = divisor;
			}
			else if (divisor < 0)
			{
				_tileRegrown[(size_t) soiltype] = TileType::NONE;
			}
		}
	}
	// Backwards compatiblity: "groundPollutionAmount" indicates the pollution
	// amount for industry tiles in an area with radius 2.
	{
		int groundPollutionAmount = 0;
		PARSEINT(groundPollutionAmount, "groundPollutionAmount")
		if (groundPollutionAmount)
		{
			TileType industrytype = tiletype("industry");
			if (industrytype != TileType::NONE)
			{
				_tilePollutionAmount[(size_t) industrytype] = 1;
				_tilePollutionRadius[(size_t) industrytype] = 2;
			}
		}
	}
	AUTO(PARSEINT, gasPollutionAmount)
	AUTO(PARSEINT, aridificationAmountHumid)
	AUTO(PARSEINT, aridificationAmountDegraded)
	else
	{
		// Backwards compatibility: aridification of 1 percent, only occurs when degraded.
		_aridificationAmountDegraded = 1;
	}
	// Backwards compatibility: hardcoded "forestGrowthAmount"; note that this
	// is now the _tileRegrowthAmount of forest.
	{
		int amount = 0;
		PARSEINT(amount, "forestGrowthAmount")
		if (amount)
		{
			TileType foresttype = tiletype("forest");
			if (foresttype != TileType::NONE)
			{
				_tileRegrowthAmount[(size_t) foresttype] = amount;
			}
		}
	}
	// Backwards compatibility: hardcoded "forestRegrowthAmount", which is the
	// amount that grass regrows into forest; note that this is now the
	// _tileRegrowthAmount of grass.
	{
		int amount = 0;
		PARSEINT(amount, "forestRegrowthAmount")
		if (amount)
		{
			TileType grasstype = tiletype("grass");
			if (grasstype != TileType::NONE)
			{
				_tileRegrowthAmount[(size_t) grasstype] = amount;
			}
		}
	}

	AUTO(PARSEINT, aridificationRange)

	AUTO(PARSEINT, aridificationCount)
	else _aridificationCount = 10;
	AUTO(PARSEINT, firestormCount)
	else _firestormCount = 10;
	AUTO(PARSEINT, deathCount)
	else _deathCount = 1;

	AUTO(PARSEINT, temperatureMax)
	else _temperatureMax = 125;
	AUTO(PARSEINT, temperatureMin)
	else _temperatureMin = -75;
	AUTO(PARSEINT, humidityMax)
	else _humidityMax = 100;
	AUTO(PARSEINT, humidityMin)
	else _humidityMin = 0;
	AUTO(PARSEINT, chaosMax)
	else _chaosMax = 10;
	AUTO(PARSEINT, chaosMin)
	else _chaosMin = 0;
	AUTO(PARSEINT, gasMax)
	else _gasMax = 2;
	AUTO(PARSEINT, gasMin)
	else _gasMin = 0;
	AUTO(PARSEINT, radiationMax)
	else _radiationMax = 3;
	AUTO(PARSEINT, radiationMin)
	else _radiationMin = 0;

	AUTO(PARSEINT, chaosThreshold)
	else _chaosThreshold = 25;

	AUTO(PARSESEASONINTS, temperatureMinHotDeath)
	AUTO(PARSESEASONINTS, temperatureMinFirestorm)
	AUTO(PARSESEASONINTS, temperatureMinAridification)
	AUTO(PARSESEASONINTS, temperatureMaxComfortable)
	AUTO(PARSESEASONINTS, temperatureMinComfortable)
	AUTO(PARSESEASONINTS, temperatureMaxSnow)
	AUTO(PARSESEASONINTS, temperatureMaxFrostbite)
	AUTO(PARSESEASONINTS, temperatureMaxColdDeath)

	AUTO(PARSESEASONINTS, humidityMinWet)
	AUTO(PARSESEASONINTS, humidityMaxDegradation)
	AUTO(PARSESEASONINTS, humidityMaxDesertification)
	AUTO(PARSESEASONINTS, humidityMinSnow)
	AUTO(PARSESEASONINTS, humidityMinFrostbite)
	AUTO(PARSESEASONINTS, humidityMaxFirestorm)
	AUTO(PARSESEASONINTS, humidityMaxBonedrought)
	AUTO(PARSESEASONINTS, humidityMaxStonedrought)
	AUTO(PARSESEASONINTS, humidityMaxDeath)

	AUTO(PARSESEASONINTS, chaosMinDegradation)
	AUTO(PARSESEASONINTS, chaosMinDesertification)
	AUTO(PARSESEASONINTS, chaosMinAridification)
	AUTO(PARSESEASONINTS, chaosMinSnow)
	AUTO(PARSESEASONINTS, chaosMinFrostbite)
	AUTO(PARSESEASONINTS, chaosMinFirestorm)
	AUTO(PARSESEASONINTS, chaosMinBonedrought)
	AUTO(PARSESEASONINTS, chaosMinStonedrought)
	AUTO(PARSESEASONINTS, chaosMinDeath)

	AUTO(PARSEINT, frostbiteShots)
	AUTO(PARSEINT, frostbiteDamage)
	AUTO(PARSEINT, frostbiteThresholdDamage)
	AUTO(PARSEINT, frostbiteThresholdVulnerability)
	AUTO(PARSEINT, firestormShots)
	AUTO(PARSEINT, firestormDamage)
	AUTO(PARSEINT, firestormBasePercentage)
	AUTO(PARSEINT, firestormDroughtPercentage)
	AUTO(PARSEINT, gasShots)
	AUTO(PARSEINT, gasDamage)
	AUTO(PARSEINT, gasThresholdDamage)
	AUTO(PARSEINT, gasThresholdVulnerability)
	AUTO(PARSEINT, radiationShots)
	AUTO(PARSEINT, radiationDamage)
	AUTO(PARSEINT, radiationThresholdDamage)
	AUTO(PARSEINT, radiationThresholdVulnerability)
	AUTO(PARSEINT, radiationThresholdDeath)

	AUTO(PARSEINT, tempGenDefault)
	else
	{
		// Backwards compatibility.
		_tempGenDefault = 30;
	}
	AUTO(PARSEINT, humGenDefault)
	else
	{
		// Backwards compatibility.
		_tempGenDefault = 55;
	}
	AUTO(PARSEINT, tempGenGainRange)
	else
	{
		// Backwards compatibility.
		_tempGenGainRange = 5;
	}
	AUTO(PARSEINT, humGenGainRange)
	else
	{
		// Backwards compatibility.
		_humGenGainRange = 5;
	}

	AUTOPARSERANGEINTS(tempGenMountainGain, _tempGenGainRange)
	else
	{
		// Backwards compatibility.
		_tempGenMountainGain = {-40, -32, -24, -16, -16, -8};
	}

	AUTOPARSERANGEINTS(tempGenOceanGain, _tempGenGainRange)
	else
	{
		// Backwards compatibility.
		_tempGenOceanGain = {-10, -8, -6, -4, -4, -2};
	}
	AUTOPARSERANGEINTS(humGenLakeGain, _humGenGainRange)
	else
	{
		// Backwards compatibility.
		_humGenLakeGain = {+40, +32, +24, +16, +16, +8};
	}
	AUTOPARSERANGEINTS(humGenOceanGain, _humGenGainRange)
	else
	{
		// Backwards compatibility.
		_humGenOceanGain = {+8, 0, 0, 0, 0, 0, -1};
	}
	AUTOPARSERANGEINTS(humGenDesertGain, _humGenGainRange)
	else
	{
		// Backwards compatibility.
		_humGenDesertGain = {-40, -32, -24, -16, -16, -8};
	}
	AUTOPARSERANGEINTS(humGenMountainGain, _humGenGainRange)

	/* MECHANICS */
	AUTO(PARSEBOOL, powerAbolished)
	AUTO(PARSEBOOL, separatePowerStages)
	AUTO(PARSEBOOL, industryNicenessQuantitative)
	AUTO(PARSEBOOL, reactorNicenessQuantitative)
	// Backwards compatibility: hardcoded "grassOnlyGrowInSpring", which
	// indicates that dirt only regrows into grass in spring. Note that this is
	// now the _tileRegrowOnlyInSpring of dirt. It also indicates that grass and
	// forest tiles gain trees only in spring, unless "treesOnlyGrowInSpring" is
	// separately mentioned.
	{
		bool grassOnlyGrowInSpring = false;
		PARSEBOOL(grassOnlyGrowInSpring, "grassOnlyGrowInSpring")
		if (grassOnlyGrowInSpring)
		{
			TileType dirttype = tiletype("dirt");
			if (dirttype != TileType::NONE)
			{
				_tileRegrowOnlyInSpring[(size_t) dirttype] = true;
			}
		}
		bool treesOnlyGrowInSpring = grassOnlyGrowInSpring;
		PARSEBOOL(treesOnlyGrowInSpring, "treesOnlyGrowInSpring")
		if (treesOnlyGrowInSpring)
		{
			TileType grasstype = tiletype("grass");
			if (grasstype != TileType::NONE)
			{
				_tileRegrowOnlyInSpring[(size_t) grasstype] = true;
			}
			TileType foresttype = tiletype("forest");
			if (foresttype != TileType::NONE)
			{
				_tileRegrowOnlyInSpring[(size_t) foresttype] = true;
			}
		}
	}
	// Backwards compatibility: hardcoded "cropsOnlyGrowInSpring", which
	// indicates that soil only regrows into crops in spring. Note that this is
	// now the _tileRegrowOnlyInSpring of soil.
	{
		bool cropsOnlyGrowInSpring = false;
		PARSEBOOL(cropsOnlyGrowInSpring, "cropsOnlyGrowInSpring")
		if (cropsOnlyGrowInSpring)
		{
			TileType soiltype = tiletype("soil");
			if (soiltype != TileType::NONE)
			{
				_tileRegrowOnlyInSpring[(size_t) soiltype] = true;
			}
		}
	}
	// Backwards compatiblity: "cropsConsumedAtNight" indicates that crops turn
	// into soil when consumed.
	{
		bool cropsConsumedAtNight = false;
		PARSEBOOL(cropsConsumedAtNight, "cropsConsumedAtNight")
		if (cropsConsumedAtNight)
		{
			TileType cropstype = tiletype("crops");
			TileType soiltype = tiletype("soil");
			if (cropstype != TileType::NONE && soiltype != TileType::NONE)
			{
				_tileDegraded[(size_t) cropstype] = soiltype;
				_tileConsumed[(size_t) cropstype] = soiltype;
				_tileRegrown[(size_t) soiltype] = cropstype;
			}
			TileType deserttype = tiletype("desert");
			if (soiltype != TileType::NONE && deserttype != TileType::NONE)
			{
				_tileDesertified[(size_t) soiltype] = deserttype;
			}
		}
	}
	AUTO(PARSEBOOL, forestChaosProtectionPermanent)
	AUTO(PARSEBOOL, collateralDamageKillsTiles)
	AUTO(PARSEBOOL, gasOnlyTargetsGroundUnits)
	AUTO(PARSEBOOL, frostbiteOnlyTargetsGroundUnits)
	AUTO(PARSEBOOL, trenchesProtectMovingUnits)
	AUTO(PARSEBOOL, trenchesProtectBypassingUnits)
	// Backwards compatiblity: "trenchesForceOccupy" indicates that trenches
	// force non-mechanical units to occupy them, but also that they no longer
	// protect moving or bypassing units.
	{
		bool trenchesForceOccupy = false;
		PARSEBOOL(trenchesForceOccupy, "trenchesForceOccupy")
		else
		{
			// Backwards compatibility.
			_trenchesProtectMovingUnits = true;
			_trenchesProtectBypassingUnits = true;
		}
		if (trenchesForceOccupy)
		{
			TileType trenchestype = tiletype("trenches");
			if (trenchestype != TileType::NONE)
			{
				_tileForceOccupy[(size_t) trenchestype] = true;
			}
		}
	}
	AUTO(PARSEBOOL, trenchesHideBypassedUnit)
	AUTO(PARSEBOOL, captureStrengthCheck)
	else
	{
		// Backwards compatibility.
		_captureStrengthCheck = true;
	}
	AUTO(PARSEBOOL, focusOnce)
	AUTO(PARSEBOOL, powerDrainScaled)
	AUTO(PARSEBOOL, powerCanDrop)
	AUTO(PARSEBOOL, upgradeKeepPower)
	AUTO(PARSEBOOL, upgradeKeepStacks)
	AUTO(PARSEBOOL, upgradeNotBlockedByFriendly)
	AUTO(PARSEBOOL, groundPollutionOnlyInAutumn)
	else
	{
		// Backwards compatibility.
		_groundPollutionOnlyInAutumn = true;
	}
	AUTO(PARSEBOOL, counterBasedWeather);
	// Backwards compatibility: emission based global warming.
	if (!_counterBasedWeather && _emissionDivisor > 0)
	{
		TileType industrytype = tiletype("industry");
		if (industrytype != TileType::NONE)
		{
			_tileEmission[(size_t) industrytype] = 1;
		}
	}
	AUTO(PARSEBOOL, quantitativeChaos);
	AUTO(PARSEBOOL, stackBasedFrostbite);
	AUTO(PARSEBOOL, emptyBasedFrostbite);
	AUTO(PARSEBOOL, planeBasedFrostbite);
	AUTO(PARSEBOOL, planeBasedAridification);
	AUTO(PARSEBOOL, flammableBasedFirestorm);
	AUTO(PARSEBOOL, percentageBasedFirestorm);
	AUTO(PARSEBOOL, randomizedFirestorm);
	AUTO(PARSEBOOL, randomizedAridification);
	AUTO(PARSEBOOL, cumulativeDeath);
	AUTO(PARSEBOOL, vulnerabilitiesStack);
	AUTO(PARSEBOOL, markersChangeAtNight);
	AUTO(PARSEBOOL, markersChangeAfterBuild);
	// Backwards compatiblity: "cityManualStackGrowth" indicates that cities
	// and other tiles that need niceness do not grow automatically as a result
	// of gaining power.
	{
		bool cityManualStackGrowth = false;
		PARSEBOOL(cityManualStackGrowth, "cityManualStackGrowth")
		if (cityManualStackGrowth)
		{
			for (size_t i = 0; i < TILETYPE_SIZE; i++)
			{
				if (_tileNeedsNiceness[i]) _tileGrowthMax[i] = 1;
			}
		}
	}
	// Backwards compatiblity: "industryManualStackGrowth" indicates that
	// industry and other tiles that need labor do not grow automatically as a
	// result of gaining power.
	{
		bool industryManualStackGrowth = false;
		PARSEBOOL(industryManualStackGrowth, "industryManualStackGrowth")
		if (industryManualStackGrowth)
		{
			for (size_t i = 0; i < TILETYPE_SIZE; i++)
			{
				if (_tileNeedsLabor[i]) _tileGrowthMax[i] = 1;
			}
		}
	}
	// Backwards compatiblity: "reactorManualStackGrowth" indicates that
	// reactors and other tiles that need energy do not grow automatically as a
	// result of gaining power.
	{
		bool reactorManualStackGrowth = false;
		PARSEBOOL(reactorManualStackGrowth, "reactorManualStackGrowth")
		if (reactorManualStackGrowth)
		{
			for (size_t i = 0; i < TILETYPE_SIZE; i++)
			{
				if (_tileNeedsEnergy[i]) _tileGrowthMax[i] = 1;
			}
		}
	}
	AUTO(PARSEBOOL, publicInitiative)
	AUTO(PARSEBOOL, weatherAfterNight)
	AUTO(PARSEBOOL, moraleReplacesMoney)
	AUTO(PARSEBOOL, moraleGatheredInSummer)
	AUTO(PARSEBOOL, moraleGatheredWhenBuilt)
	else if (!_moraleGatheredInSummer)
	{
		for (int8_t value : _tileMoraleGainWhenGathered)
		{
			if (value != 0)
			{
				_moraleGatheredWhenBuilt = true;
				break;
			}
		}
	}
	AUTO(PARSEBOOL, snowCoversNiceness)

	AUTO(PARSEINT, snowSlowAmount)
	else
	{
		// Backwards compatibility.
		_snowSlowAmount = 10;
	}
	AUTO(PARSEINT, snowSlowMaximum)
	else
	{
		// Backwards compatibility.
		_snowSlowMaximum = 10;
	}
	// Backwards compatiblity: trenches had their slow amount hardcoded.
	{
		bool trenchesSlowAmount;
		PARSEINT(trenchesSlowAmount, "trenchesSlowAmount")
		else
		{
			// Backwards compatibility.
			trenchesSlowAmount = 10;
		}
		if (trenchesSlowAmount)
		{
			TileType trenchestype = tiletype("trenches");
			if (trenchestype != TileType::NONE)
			{
				_tileSlowAmount[(size_t) trenchestype] = trenchesSlowAmount;
			}
		}
	}
	// Backwards compatiblity: trenches had their slow maximum hardcoded.
	{
		bool trenchesSlowMaximum;
		PARSEINT(trenchesSlowMaximum, "trenchesSlowMaximum")
		else
		{
			// Backwards compatibility.
			trenchesSlowMaximum = 10;
		}
		if (trenchesSlowMaximum)
		{
			TileType trenchestype = tiletype("trenches");
			if (trenchestype != TileType::NONE)
			{
				_tileSlowMaximum[(size_t) trenchestype] = trenchesSlowMaximum;
			}
		}
	}

	/* COMMANDERS */
	AUTO(PARSEINT, startingMoney)
	else PARSEINT(_startingMoney, "startingIncome")
	AUTO(PARSEINT, maxMoney)
	AUTO(PARSEINT, minMoney)
	AUTO(PARSEINT, newOrderLimit)

	/* FINALIZE */
	finalize();
}

#define PUTTYPEWORDS(VARIABLE, NAME) \
	{ \
		Json::Value list(Json::arrayValue); \
		for (const TypeWord& typeword : VARIABLE) \
		{ \
			if (typeword == "none" || typeword == "") continue; \
			list.append(::stringify(typeword)); \
		} \
		json[NAME] = list; \
	}

#define PUTBOOL(VARIABLE, NAME) \
	if (VARIABLE) json[NAME] = VARIABLE;

#define FORCEPUTBOOL(VARIABLE, NAME) \
	json[NAME] = VARIABLE;

#define PUTINT(VARIABLE, NAME) \
	if (VARIABLE) json[NAME] = (int) VARIABLE;

#define FORCEPUTINT(VARIABLE, NAME) \
	json[NAME] = (int) VARIABLE;

#define PUTTILEBOOLS(VARIABLE, NAME) \
	{ \
		MAPTILEBOOLS(VARIABLE, NAME) \
		if (!map.empty()) json[NAME] = map; \
	}

#define FORCEPUTTILEBOOLS(VARIABLE, NAME) \
	{ \
		MAPTILEBOOLS(VARIABLE, NAME) \
		json[NAME] = map; \
	}

#define MAPTILEBOOLS(VARIABLE, NAME) \
	/*{*/ \
		Json::Value map(Json::objectValue); \
		for (size_t i = 0; i < TILETYPE_SIZE; i++) \
		{ \
			bool val = VARIABLE[i]; \
			if (val) map[::stringify(typeword((TileType) i))] = true; \
		} \
	/*}*/

#define PUTTILEINTS(VARIABLE, NAME) \
	{ \
		MAPTILEINTS(VARIABLE, NAME) \
		if (!map.empty()) json[NAME] = map; \
	}

#define FORCEPUTTILEINTS(VARIABLE, NAME) \
	{ \
		MAPTILEINTS(VARIABLE, NAME) \
		json[NAME] = map; \
	}

#define MAPTILEINTS(VARIABLE, NAME) \
	/*{*/ \
		Json::Value map(Json::objectValue); \
		for (size_t i = 0; i < TILETYPE_SIZE; i++) \
		{ \
			int val = VARIABLE[i]; \
			if (val) map[::stringify(typeword((TileType) i))] = val; \
		} \
	/*}*/

#define PUTTILETILES(VARIABLE, NAME) \
	{ \
		MAPTILETILES(VARIABLE, NAME) \
		if (!map.empty()) json[NAME] = map; \
	}

#define FORCEPUTTILETILES(VARIABLE, NAME) \
	{ \
		MAPTILETILES(VARIABLE, NAME) \
		json[NAME] = map; \
	}

#define MAPTILETILES(VARIABLE, NAME) \
	/*{*/ \
		Json::Value map(Json::objectValue); \
		for (size_t i = 0; i < TILETYPE_SIZE; i++) \
		{ \
			TileType type = VARIABLE[i]; \
			if (type != TileType::NONE) \
			{ \
				map[::stringify(typeword((TileType) i))] = ::stringify(typeword(type)); \
			} \
		} \
	/*}*/

#define PUTTILEUNITLISTS(VARIABLE, NAME) \
	{ \
		MAPTILEUNITLISTS(VARIABLE, NAME) \
		if (!map.empty()) json[NAME] = map; \
	}

#define FORCEPUTTILEUNITLISTS(VARIABLE, NAME) \
	{ \
		MAPTILEUNITLISTS(VARIABLE, NAME) \
		json[NAME] = map; \
	}

#define MAPTILEUNITLISTS(VARIABLE, NAME) \
	/*{*/ \
		Json::Value map(Json::objectValue); \
		for (size_t i = 0; i < TILETYPE_SIZE; i++) \
		{ \
			const auto& builds = VARIABLE[i]; \
			if (!builds.empty()) \
			{ \
				Json::Value list(Json::arrayValue); \
				ADDBUILDSTOJSONLIST(builds, list) \
				map[::stringify(typeword((TileType) i))] = list; \
			} \
		} \
	/*}*/

#define ADDBUILDSTOJSONLIST(BUILDS, LIST) \
	/*{*/ \
		for (auto& build : BUILDS) \
		{ \
			Json::Value item(Json::objectValue); \
			item["type"] = ::stringify(typeword(build.type)); \
			if (build.cost) \
			{ \
				item["cost"] = build.cost; \
			} \
			LIST.append(item); \
		} \
	/*}*/

#define PUTTILETILELISTS(VARIABLE, NAME) \
	{ \
		MAPTILETILELISTS(VARIABLE, NAME) \
		if (!map.empty()) json[NAME] = map; \
	}

#define FORCEPUTTILETILELISTS(VARIABLE, NAME) \
	{ \
		MAPTILETILELISTS(VARIABLE, NAME) \
		json[NAME] = map; \
	}

#define MAPTILETILELISTS(VARIABLE, NAME) \
	/*{*/ \
		Json::Value map(Json::objectValue); \
		for (size_t i = 0; i < TILETYPE_SIZE; i++) \
		{ \
			const auto& builds = VARIABLE[i]; \
			if (!builds.empty()) \
			{ \
				Json::Value list(Json::arrayValue); \
				ADDBUILDSTOJSONLIST(builds, list) \
				map[::stringify(typeword((TileType) i))] = list; \
			} \
		} \
	/*}*/

#define PUTUNITBOOLS(VARIABLE, NAME) \
	{ \
		MAPUNITBOOLS(VARIABLE, NAME) \
		if (!map.empty()) json[NAME] = map; \
	}

#define FORCEPUTUNITBOOLS(VARIABLE, NAME) \
	{ \
		MAPUNITBOOLS(VARIABLE, NAME) \
		json[NAME] = map; \
	}

#define MAPUNITBOOLS(VARIABLE, NAME) \
	/*{*/ \
		Json::Value map(Json::objectValue); \
		for (size_t i = 0; i < UNITTYPE_SIZE; i++) \
		{ \
			bool val = VARIABLE[i]; \
			if (val) map[::stringify(typeword((UnitType) i))] = true; \
		} \
	/*}*/

#define PUTUNITINTS(VARIABLE, NAME) \
	{ \
		MAPUNITINTS(VARIABLE, NAME) \
		if (!map.empty()) json[NAME] = map; \
	}

#define FORCEPUTUNITINTS(VARIABLE, NAME) \
	{ \
		MAPUNITINTS(VARIABLE, NAME) \
		json[NAME] = map; \
	}

#define MAPUNITINTS(VARIABLE, NAME) \
	/*{*/ \
		Json::Value map(Json::objectValue); \
		for (size_t i = 0; i < UNITTYPE_SIZE; i++) \
		{ \
			int val = VARIABLE[i]; \
			if (val) map[::stringify(typeword((UnitType) i))] = val; \
		} \
	/*}*/

#define PUTUNITUNITLISTS(VARIABLE, NAME) \
	{ \
		MAPUNITUNITLISTS(VARIABLE, NAME) \
		if (!map.empty()) json[NAME] = map; \
	}

#define FORCEPUTUNITUNITLISTS(VARIABLE, NAME) \
	{ \
		MAPUNITUNITLISTS(VARIABLE, NAME) \
		json[NAME] = map; \
	}

#define MAPUNITUNITLISTS(VARIABLE, NAME) \
	/*{*/ \
		Json::Value map(Json::objectValue); \
		for (size_t i = 0; i < UNITTYPE_SIZE; i++) \
		{ \
			const auto& builds = VARIABLE[i]; \
			if (!builds.empty()) \
			{ \
				Json::Value list(Json::arrayValue); \
				ADDBUILDSTOJSONLIST(builds, list) \
				map[::stringify(typeword((UnitType) i))] = list; \
			} \
		} \
	/*}*/

#define PUTUNITTILELISTS(VARIABLE, NAME) \
	{ \
		MAPUNITTILELISTS(VARIABLE, NAME) \
		if (!map.empty()) json[NAME] = map; \
	}

#define FORCEPUTUNITTILELISTS(VARIABLE, NAME) \
	{ \
		MAPUNITTILELISTS(VARIABLE, NAME) \
		json[NAME] = map; \
	}

#define MAPUNITTILELISTS(VARIABLE, NAME) \
	/*{*/ \
		Json::Value map(Json::objectValue); \
		for (size_t i = 0; i < UNITTYPE_SIZE; i++) \
		{ \
			const auto& builds = VARIABLE[i]; \
			if (!builds.empty()) \
			{ \
				Json::Value list(Json::arrayValue); \
				ADDBUILDSTOJSONLIST(builds, list) \
				map[::stringify(typeword((UnitType) i))] = list; \
			} \
		} \
	/*}*/

#define PUTSEASONINTS(VARIABLE, NAME) \
	{ \
		MAPSEASONINTS(VARIABLE, NAME) \
		if (!map.empty()) json[NAME] = map; \
	}

#define FORCEPUTSEASONINTS(VARIABLE, NAME) \
	{ \
		MAPSEASONINTS(VARIABLE, NAME) \
		json[NAME] = map; \
	}

#define MAPSEASONINTS(VARIABLE, NAME) \
	/*{*/ \
		Json::Value map(Json::objectValue); \
		for (size_t i = 0; i < SEASON_SIZE; i++) \
		{ \
			int val = VARIABLE[i]; \
			if (val) map[::stringify((Season) i)] = val; \
		} \
	/*}*/

#define AUTOPUTRANGEINTS(X, MAX) PUTRANGEINTS(VARIABLE(X), NAME(X), MAX)
#define AUTOFORCEPUTRANGEINTS(X, MAX) FORCEPUTRANGEINTS(VARIABLE(X), NAME(X), MAX)

#define PUTRANGEINTS(VARIABLE, NAME, MAX) \
	if (MAX > 1 || (MAX == 0 && VARIABLE[MAX])) \
	{ \
		Json::Value list(Json::arrayValue); \
		for (int i = 0; i <= MAX; i++) \
		{ \
			Json::Value val = (int) VARIABLE[i]; \
			list.append(val); \
		} \
		json[NAME] = list; \
	}

#define FORCEPUTRANGEINTS(VARIABLE, NAME, MAX) \
	{ \
		Json::Value list(Json::arrayValue); \
		for (int i = 0; i <= MAX; i++) \
		{ \
			Json::Value val = (int) VARIABLE[i]; \
			list.append(val); \
		} \
		json[NAME] = list; \
	}

Json::Value Bible::toJson() const
{
	Json::Value json;

	/* VERSION */
	json["version"] = _version.toString();

	/* TYPES */
	AUTO(PUTTYPEWORDS, tiletypes)
	AUTO(PUTTYPEWORDS, unittypes)

	/* TILES */
	AUTO(PUTTILEBOOLS, tileAccessible)
	AUTO(PUTTILEBOOLS, tileWalkable)
	AUTO(PUTTILEBOOLS, tileBuildable)
	AUTO(PUTTILEBOOLS, tileDestructible)
	AUTO(PUTTILEBOOLS, tileGrassy)
	AUTO(PUTTILEBOOLS, tileNatural)
	AUTO(FORCEPUTTILEBOOLS, tileLaboring) // Backwards compatibility.
	AUTO(FORCEPUTTILEBOOLS, tileEnergizing) // Backwards compatibility.
	AUTO(PUTTILEBOOLS, tilePowered)
	AUTO(FORCEPUTTILEBOOLS, tileNeedsNiceness) // Backwards compatibility.
	AUTO(FORCEPUTTILEBOOLS, tileNeedsLabor) // Backwards compatibility.
	AUTO(FORCEPUTTILEBOOLS, tileNeedsEnergy) // Backwards compatibility.
	AUTO(PUTTILEBOOLS, tileNeedsTime)
	AUTO(FORCEPUTTILEBOOLS, tileOwnable) // Backwards compatibility.
	AUTO(FORCEPUTTILEBOOLS, tileControllable) // Backwards compatibility.
	AUTO(FORCEPUTTILEBOOLS, tileBinding) // Backwards compatibility.
	AUTO(PUTTILEBOOLS, tileAutoCultivates)
	AUTO(PUTTILEBOOLS, tilePlane)
	AUTO(PUTTILEBOOLS, tileFlammable)
	AUTO(FORCEPUTTILEBOOLS, tileWater) // Backwards compatibility.
	AUTO(FORCEPUTTILEBOOLS, tileMountain) // Backwards compatibility.
	AUTO(FORCEPUTTILEBOOLS, tileDesert) // Backwards compatibility.
	AUTO(FORCEPUTTILEBOOLS, tileStone) // Backwards compatibility.
	AUTO(FORCEPUTTILEBOOLS, tileTrenches) // Backwards compatibility.
	AUTO(PUTTILEBOOLS, tileForceOccupy)
	AUTO(FORCEPUTTILEBOOLS, tileChaosProtection) // Backwards compatibility.
	AUTO(FORCEPUTTILEBOOLS, tileRegrowOnlyInSpring) // Backwards compatibility.
	AUTO(PUTTILEBOOLS, tileGathersMorale)

	AUTO(PUTTILEINTS, tileStacksBuilt)
	AUTO(PUTTILEINTS, tileStacksMax)
	AUTO(PUTTILEINTS, tilePowerBuilt)
	AUTO(PUTTILEINTS, tilePowerMax)
	AUTO(PUTTILEINTS, tileGrowthMax)
	AUTO(PUTTILEINTS, tileVision)
	AUTO(PUTTILEINTS, tileHitpoints)
	AUTO(FORCEPUTTILEINTS, tileIncome) // Backwards compatibility.
	AUTO(PUTTILEINTS, tileLeakGas)
	AUTO(PUTTILEINTS, tileLeakRads)
	AUTO(PUTTILEINTS, tileEmission)
	AUTO(PUTTILEINTS, tilePollutionAmount)
	AUTO(PUTTILEINTS, tilePollutionRadius)
	AUTO(PUTTILEINTS, tileSlowAmount)
	AUTO(PUTTILEINTS, tileSlowMaximum)
	AUTO(PUTTILEINTS, tileRegrowthProbabilityDivisor)
	AUTO(PUTTILEINTS, tileRegrowthAmount)
	AUTO(PUTTILEINTS, tileFirestormResistance)
	AUTO(PUTTILEINTS, tileMoraleGainWhenBuilt)
	AUTO(PUTTILEINTS, tileMoraleGainWhenLost)
	AUTO(PUTTILEINTS, tileMoraleGainWhenDestroyed)
	AUTO(PUTTILEINTS, tileMoraleGainWhenCaptured)
	AUTO(PUTTILEINTS, tileMoraleGainWhenRazed)
	AUTO(PUTTILEINTS, tileMoraleGainWhenGathered)

	AUTO(PUTTILEUNITLISTS, tileProduces)
	AUTO(PUTTILETILELISTS, tileExpands)
	AUTO(PUTTILETILELISTS, tileUpgrades)
	AUTO(PUTTILETILELISTS, tileCultivates)

	AUTO(PUTTILEINTS, tileCost)
	AUTO(FORCEPUTTILEINTS, tileScoreBase) // Backwards compatibility.
	AUTO(FORCEPUTTILEINTS, tileScoreStack) // Backwards compatibility.

	AUTO(PUTTILETILES, tileDestroyed)
	AUTO(FORCEPUTTILETILES, tileDegraded) // Backwards compatibility.
	AUTO(FORCEPUTTILETILES, tileDesertified) // Backwards compatibility.
	AUTO(PUTTILETILES, tileConsumed)
	AUTO(FORCEPUTTILETILES, tileRegrown) // Backwards compatibility.

	AUTO(PUTINT, tileExpandRangeMin)
	AUTO(PUTINT, tileExpandRangeMax)
	AUTO(PUTINT, tileProduceRangeMin)
	AUTO(PUTINT, tileProduceRangeMax)

	/* UNITS */
	AUTO(PUTUNITBOOLS, unitAir)
	AUTO(PUTUNITBOOLS, unitMechanical)
	AUTO(PUTUNITBOOLS, unitInfantry)
	AUTO(PUTUNITBOOLS, unitCanMove)
	AUTO(PUTUNITBOOLS, unitCanAttack)
	AUTO(FORCEPUTUNITBOOLS, unitCanGuard) // Backwards compatibility.
	AUTO(PUTUNITBOOLS, unitCanFocus)
	AUTO(PUTUNITBOOLS, unitCanLockdown)
	AUTO(PUTUNITBOOLS, unitCanShell)
	AUTO(PUTUNITBOOLS, unitCanBombard)
	AUTO(PUTUNITBOOLS, unitCanBomb)
	AUTO(PUTUNITBOOLS, unitCanCapture)
	AUTO(PUTUNITBOOLS, unitCanOccupy)

	AUTO(FORCEPUTUNITINTS, unitStacksMax) // Backwards compatibility.
	AUTO(PUTUNITINTS, unitSpeed)
	AUTO(PUTUNITINTS, unitVision)
	AUTO(PUTUNITINTS, unitHitpoints)
	AUTO(PUTUNITINTS, unitAttackShots)
	AUTO(PUTUNITINTS, unitAttackDamage)
	AUTO(PUTUNITINTS, unitTrampleShots)
	AUTO(PUTUNITINTS, unitTrampleDamage)
	AUTO(FORCEPUTUNITINTS, unitAbilityVolleys) // Backwards compatibility.
	AUTO(PUTUNITINTS, unitAbilityShots)
	AUTO(PUTUNITINTS, unitAbilityDamage)
	AUTO(PUTUNITINTS, unitAbilityGas)
	AUTO(PUTUNITINTS, unitAbilityRads)
	AUTO(PUTUNITINTS, unitAbilityRadius)
	AUTO(PUTUNITINTS, unitRangeMin)
	AUTO(PUTUNITINTS, unitRangeMax)
	AUTO(PUTUNITINTS, unitLeakGas)
	AUTO(PUTUNITINTS, unitLeakRads)
	AUTO(PUTUNITINTS, unitMoraleGainWhenLost)
	AUTO(PUTUNITINTS, unitMoraleGainWhenKilled)

	AUTO(PUTUNITTILELISTS, unitShapes)
	AUTO(PUTUNITTILELISTS, unitSettles)

	AUTO(PUTUNITINTS, unitCost)

	AUTO(FORCEPUTINT, unitSizeMax) // Backwards compatibility.
	AUTO(PUTINT, unitVisionMax)

	/* COMBAT */
	AUTO(PUTINT, missCountGround)
	AUTO(PUTINT, missCountAir)
	AUTO(PUTINT, missCountTrenches)
	AUTO(PUTINT, missHitpointsGround)
	AUTO(PUTINT, missHitpointsAir)
	AUTO(PUTINT, missHitpointsTrenches)

	/* WEATHER */
	AUTO(PUTSEASONINTS, seasonTemperatureSwing)
	AUTO(PUTSEASONINTS, seasonGlobalWarmingFactor)

	AUTO(PUTINT, emissionDivisor)

	AUTO(PUTINT, gasPollutionAmount)
	AUTO(PUTINT, aridificationAmountHumid)
	AUTO(FORCEPUTINT, aridificationAmountDegraded) // Backwards compatibility.

	AUTO(PUTINT, aridificationRange)

	AUTO(FORCEPUTINT, aridificationCount) // Backwards compatibility.
	AUTO(FORCEPUTINT, firestormCount) // Backwards compatibility.
	AUTO(FORCEPUTINT, deathCount) // Backwards compatibility.

	AUTO(FORCEPUTINT, temperatureMax) // Backwards compatibility.
	AUTO(FORCEPUTINT, temperatureMin) // Backwards compatibility.
	AUTO(FORCEPUTINT, humidityMax) // Backwards compatibility.
	AUTO(FORCEPUTINT, humidityMin) // Backwards compatibility.
	AUTO(FORCEPUTINT, chaosMax) // Backwards compatibility.
	AUTO(FORCEPUTINT, chaosMin) // Backwards compatibility.
	AUTO(FORCEPUTINT, gasMax) // Backwards compatibility.
	AUTO(FORCEPUTINT, gasMin) // Backwards compatibility.
	AUTO(FORCEPUTINT, radiationMax) // Backwards compatibility.
	AUTO(FORCEPUTINT, radiationMin) // Backwards compatibility.

	AUTO(FORCEPUTINT, chaosThreshold) // Backwards compatibility.

	AUTO(PUTSEASONINTS, temperatureMinHotDeath)
	AUTO(PUTSEASONINTS, temperatureMinFirestorm)
	AUTO(PUTSEASONINTS, temperatureMinAridification)
	AUTO(PUTSEASONINTS, temperatureMaxComfortable)
	AUTO(PUTSEASONINTS, temperatureMinComfortable)
	AUTO(PUTSEASONINTS, temperatureMaxSnow)
	AUTO(PUTSEASONINTS, temperatureMaxFrostbite)
	AUTO(PUTSEASONINTS, temperatureMaxColdDeath)

	AUTO(PUTSEASONINTS, humidityMinWet)
	AUTO(PUTSEASONINTS, humidityMaxDegradation)
	AUTO(PUTSEASONINTS, humidityMaxDesertification)
	AUTO(PUTSEASONINTS, humidityMinSnow)
	AUTO(PUTSEASONINTS, humidityMinFrostbite)
	AUTO(PUTSEASONINTS, humidityMaxFirestorm)
	AUTO(PUTSEASONINTS, humidityMaxBonedrought)
	AUTO(PUTSEASONINTS, humidityMaxStonedrought)
	AUTO(PUTSEASONINTS, humidityMaxDeath)

	AUTO(PUTSEASONINTS, chaosMinDegradation)
	AUTO(PUTSEASONINTS, chaosMinDesertification)
	AUTO(PUTSEASONINTS, chaosMinAridification)
	AUTO(PUTSEASONINTS, chaosMinSnow)
	AUTO(PUTSEASONINTS, chaosMinFrostbite)
	AUTO(PUTSEASONINTS, chaosMinFirestorm)
	AUTO(PUTSEASONINTS, chaosMinBonedrought)
	AUTO(PUTSEASONINTS, chaosMinStonedrought)
	AUTO(PUTSEASONINTS, chaosMinDeath)

	AUTO(PUTINT, frostbiteShots)
	AUTO(PUTINT, frostbiteDamage)
	AUTO(PUTINT, frostbiteThresholdDamage)
	AUTO(PUTINT, frostbiteThresholdVulnerability)
	AUTO(PUTINT, firestormShots)
	AUTO(PUTINT, firestormDamage)
	AUTO(PUTINT, firestormBasePercentage)
	AUTO(PUTINT, firestormDroughtPercentage)
	AUTO(PUTINT, gasShots)
	AUTO(PUTINT, gasDamage)
	AUTO(PUTINT, gasThresholdDamage)
	AUTO(PUTINT, gasThresholdVulnerability)
	AUTO(PUTINT, radiationShots)
	AUTO(PUTINT, radiationDamage)
	AUTO(PUTINT, radiationThresholdDamage)
	AUTO(PUTINT, radiationThresholdVulnerability)
	AUTO(PUTINT, radiationThresholdDeath)

	AUTO(FORCEPUTINT, tempGenDefault) // Backwards compatibility.
	AUTO(FORCEPUTINT, humGenDefault) // Backwards compatibility.
	AUTO(FORCEPUTINT, tempGenGainRange) // Backwards compatibility.
	AUTO(FORCEPUTINT, humGenGainRange) // Backwards compatibility.

	AUTOFORCEPUTRANGEINTS(tempGenMountainGain, _tempGenGainRange) // Backwards compatibility.
	AUTOFORCEPUTRANGEINTS(tempGenOceanGain, _tempGenGainRange) // Backwards compatibility.
	AUTOFORCEPUTRANGEINTS(humGenLakeGain, _humGenGainRange) // Backwards compatibility.
	AUTOFORCEPUTRANGEINTS(humGenOceanGain, _humGenGainRange) // Backwards compatibility.
	AUTOFORCEPUTRANGEINTS(humGenDesertGain, _humGenGainRange) // Backwards compatibility.
	AUTOPUTRANGEINTS(humGenMountainGain, _humGenGainRange)

	/* MECHANICS */
	AUTO(PUTBOOL, powerAbolished)
	AUTO(PUTBOOL, separatePowerStages)
	AUTO(PUTBOOL, industryNicenessQuantitative)
	AUTO(PUTBOOL, reactorNicenessQuantitative)
	AUTO(PUTBOOL, forestChaosProtectionPermanent)
	AUTO(PUTBOOL, collateralDamageKillsTiles)
	AUTO(PUTBOOL, gasOnlyTargetsGroundUnits)
	AUTO(PUTBOOL, frostbiteOnlyTargetsGroundUnits)
	FORCEPUTBOOL(true, "trenchesForceOccupy") // Backwards compatibility.
	AUTO(PUTBOOL, trenchesProtectMovingUnits)
	AUTO(PUTBOOL, trenchesProtectBypassingUnits)
	AUTO(PUTBOOL, trenchesHideBypassedUnit)
	AUTO(FORCEPUTBOOL, captureStrengthCheck) // Backwards compatibility.
	AUTO(PUTBOOL, focusOnce)
	AUTO(PUTBOOL, powerDrainScaled)
	AUTO(PUTBOOL, powerCanDrop)
	AUTO(PUTBOOL, upgradeKeepPower)
	AUTO(PUTBOOL, upgradeKeepStacks)
	AUTO(PUTBOOL, upgradeNotBlockedByFriendly)
	AUTO(FORCEPUTBOOL, groundPollutionOnlyInAutumn) // Backwards compatibility.
	AUTO(PUTBOOL, counterBasedWeather)
	AUTO(PUTBOOL, quantitativeChaos)
	AUTO(PUTBOOL, stackBasedFrostbite)
	AUTO(PUTBOOL, emptyBasedFrostbite)
	AUTO(PUTBOOL, planeBasedFrostbite)
	AUTO(PUTBOOL, planeBasedAridification)
	AUTO(PUTBOOL, flammableBasedFirestorm)
	AUTO(PUTBOOL, percentageBasedFirestorm)
	AUTO(PUTBOOL, randomizedFirestorm)
	AUTO(PUTBOOL, randomizedAridification)
	AUTO(PUTBOOL, cumulativeDeath)
	AUTO(PUTBOOL, vulnerabilitiesStack)
	AUTO(PUTBOOL, markersChangeAtNight)
	AUTO(PUTBOOL, markersChangeAfterBuild)
	AUTO(PUTBOOL, publicInitiative)
	AUTO(PUTBOOL, weatherAfterNight)
	AUTO(PUTBOOL, moraleReplacesMoney)
	AUTO(PUTBOOL, moraleGatheredInSummer)
	AUTO(PUTBOOL, moraleGatheredWhenBuilt)
	AUTO(PUTBOOL, snowCoversNiceness)

	AUTO(FORCEPUTINT, snowSlowAmount) // Backwards compatibility.
	AUTO(FORCEPUTINT, snowSlowMaximum) // Backwards compatibility.
	FORCEPUTINT(0, "trenchesSlowAmount") // Backwards compatibility.
	FORCEPUTINT(0, "trenchesSlowMaximum") // Backwards compatibility.

	/* COMMANDERS */
	AUTO(PUTINT, startingMoney)
	AUTO(PUTINT, maxMoney)
	AUTO(PUTINT, minMoney)
	AUTO(PUTINT, newOrderLimit)

	return json;
}

template <typename X>
bool is_permutation(const std::vector<X>& a, const std::vector<X>& b)
{
	return (a.size() == b.size() && std::is_permutation(a.begin(), a.end(), b.begin()));
}

static void Bible_log_difference(const char* name)
{
	LOGD << "difference at " << name;
}

static void Bible_log_difference(const char* name, int i)
{
	LOGD << "difference at " << name << "[" << i << "]";
}

#define CHECKLIST(VARIABLE, NAME) \
	if (VARIABLE.size() != other.VARIABLE.size()) \
	{ \
		Bible_log_difference(NAME); \
		return false; \
	} \
	for (size_t i = 0; i < VARIABLE.size(); i++) \
	{ \
		if (VARIABLE[i] != other.VARIABLE[i]) \
		{ \
			Bible_log_difference(NAME, i); \
			return false; \
		} \
	}

#define CHECK(VARIABLE, NAME) \
	if (VARIABLE != other.VARIABLE) \
	{ \
		Bible_log_difference(NAME); \
		return false; \
	} \

#define CHECKTILESTATS(VARIABLE, NAME) \
	for (size_t i = 0; i < TILETYPE_SIZE; i++) \
	{ \
		if (VARIABLE[i] != other.VARIABLE[i]) \
		{ \
			Bible_log_difference(NAME, i); \
			return false; \
		} \
	}

#define CHECKTILELISTS(VARIABLE, NAME) \
	for (size_t i = 0; i < TILETYPE_SIZE; i++) \
	{ \
		if (!is_permutation(VARIABLE[i], other.VARIABLE[i])) \
		{ \
			Bible_log_difference(NAME, i); \
			return false; \
		} \
	}

#define CHECKUNITSTATS(VARIABLE, NAME) \
	for (size_t i = 0; i < UNITTYPE_SIZE; i++) \
	{ \
		if (VARIABLE[i] != other.VARIABLE[i]) \
		{ \
			Bible_log_difference(NAME, i); \
			return false; \
		} \
	}

#define CHECKUNITLISTS(VARIABLE, NAME) \
	for (size_t i = 0; i < UNITTYPE_SIZE; i++) \
	{ \
		if (!is_permutation(VARIABLE[i], other.VARIABLE[i])) \
		{ \
			Bible_log_difference(NAME, i); \
			return false; \
		} \
	}

#define CHECKSEASONSTATS(VARIABLE, NAME) \
	for (size_t i = 0; i < SEASON_SIZE; i++) \
	{ \
		if (VARIABLE[i] != other.VARIABLE[i]) \
		{ \
			Bible_log_difference(NAME, i); \
			return false; \
		} \
	}

#define AUTOCHECKRANGESTATS(X, MAX) CHECKRANGESTATS(VARIABLE(X), NAME(X), MAX)
#define CHECKRANGESTATS(VARIABLE, NAME, MAX) \
	for (int i = 0; i <= MAX; i++) \
	{ \
		if (VARIABLE[i] != other.VARIABLE[i]) \
		{ \
			Bible_log_difference(NAME, i); \
			return false; \
		} \
	}

bool Bible::operator==(const Bible& other) const
{
	/* TYPES */
	AUTO(CHECKLIST, tiletypes)
	AUTO(CHECKLIST, unittypes)

	/* TILES */
	AUTO(CHECKTILESTATS, tileAccessible)
	AUTO(CHECKTILESTATS, tileWalkable)
	AUTO(CHECKTILESTATS, tileBuildable)
	AUTO(CHECKTILESTATS, tileDestructible)
	AUTO(CHECKTILESTATS, tileGrassy)
	AUTO(CHECKTILESTATS, tileNatural)
	AUTO(CHECKTILESTATS, tileLaboring)
	AUTO(CHECKTILESTATS, tileEnergizing)
	AUTO(CHECKTILESTATS, tilePowered)
	AUTO(CHECKTILESTATS, tileNeedsNiceness)
	AUTO(CHECKTILESTATS, tileNeedsLabor)
	AUTO(CHECKTILESTATS, tileNeedsLabor)
	AUTO(CHECKTILESTATS, tileOwnable)
	AUTO(CHECKTILESTATS, tileControllable)
	AUTO(CHECKTILESTATS, tileBinding)
	AUTO(CHECKTILESTATS, tileAutoCultivates)
	AUTO(CHECKTILESTATS, tilePlane)
	AUTO(CHECKTILESTATS, tileFlammable)
	AUTO(CHECKTILESTATS, tileWater)
	AUTO(CHECKTILESTATS, tileMountain)
	AUTO(CHECKTILESTATS, tileDesert)
	AUTO(CHECKTILESTATS, tileStone)
	AUTO(CHECKTILESTATS, tileTrenches)
	AUTO(CHECKTILESTATS, tileForceOccupy)
	AUTO(CHECKTILESTATS, tileChaosProtection)
	AUTO(CHECKTILESTATS, tileRegrowOnlyInSpring)
	AUTO(CHECKTILESTATS, tileGathersMorale)

	AUTO(CHECKTILESTATS, tileStacksBuilt)
	AUTO(CHECKTILESTATS, tileStacksMax)
	AUTO(CHECKTILESTATS, tilePowerBuilt)
	AUTO(CHECKTILESTATS, tilePowerMax)
	AUTO(CHECKTILESTATS, tileGrowthMax)
	AUTO(CHECKTILESTATS, tileVision)
	AUTO(CHECKTILESTATS, tileHitpoints)
	AUTO(CHECKTILESTATS, tileIncome)
	AUTO(CHECKTILESTATS, tileLeakGas)
	AUTO(CHECKTILESTATS, tileLeakRads)
	AUTO(CHECKTILESTATS, tileEmission)
	AUTO(CHECKTILESTATS, tilePollutionAmount)
	AUTO(CHECKTILESTATS, tilePollutionRadius)
	AUTO(CHECKTILESTATS, tileSlowAmount)
	AUTO(CHECKTILESTATS, tileSlowMaximum)
	AUTO(CHECKTILESTATS, tileRegrowthProbabilityDivisor)
	AUTO(CHECKTILESTATS, tileRegrowthAmount)
	AUTO(CHECKTILESTATS, tileFirestormResistance)
	AUTO(CHECKTILESTATS, tileMoraleGainWhenBuilt)
	AUTO(CHECKTILESTATS, tileMoraleGainWhenLost)
	AUTO(CHECKTILESTATS, tileMoraleGainWhenDestroyed)
	AUTO(CHECKTILESTATS, tileMoraleGainWhenCaptured)
	AUTO(CHECKTILESTATS, tileMoraleGainWhenRazed)
	AUTO(CHECKTILESTATS, tileMoraleGainWhenGathered)

	AUTO(CHECKTILELISTS, tileProduces)
	AUTO(CHECKTILELISTS, tileExpands)
	AUTO(CHECKTILELISTS, tileUpgrades)
	AUTO(CHECKTILELISTS, tileCultivates)

	AUTO(CHECKTILESTATS, tileCost)
	AUTO(CHECKTILESTATS, tileScoreBase)
	AUTO(CHECKTILESTATS, tileScoreStack)

	AUTO(CHECKTILESTATS, tileDestroyed)
	AUTO(CHECKTILESTATS, tileDegraded)
	AUTO(CHECKTILESTATS, tileDesertified)
	AUTO(CHECKTILESTATS, tileConsumed)
	AUTO(CHECKTILESTATS, tileRegrown)

	AUTO(CHECK, tileExpandRangeMin)
	AUTO(CHECK, tileExpandRangeMax)
	AUTO(CHECK, tileProduceRangeMin)
	AUTO(CHECK, tileProduceRangeMax)

	/* UNITS */
	AUTO(CHECKUNITSTATS, unitAir)
	AUTO(CHECKUNITSTATS, unitInfantry)
	AUTO(CHECKUNITSTATS, unitMechanical)
	AUTO(CHECKUNITSTATS, unitCanMove)
	AUTO(CHECKUNITSTATS, unitCanAttack)
	AUTO(CHECKUNITSTATS, unitCanGuard)
	AUTO(CHECKUNITSTATS, unitCanFocus)
	AUTO(CHECKUNITSTATS, unitCanLockdown)
	AUTO(CHECKUNITSTATS, unitCanShell)
	AUTO(CHECKUNITSTATS, unitCanBombard)
	AUTO(CHECKUNITSTATS, unitCanBomb)
	AUTO(CHECKUNITSTATS, unitCanCapture)
	AUTO(CHECKUNITSTATS, unitCanOccupy)

	AUTO(CHECKUNITSTATS, unitStacksMax)
	AUTO(CHECKUNITSTATS, unitSpeed)
	AUTO(CHECKUNITSTATS, unitVision)
	AUTO(CHECKUNITSTATS, unitHitpoints)
	AUTO(CHECKUNITSTATS, unitAttackShots)
	AUTO(CHECKUNITSTATS, unitAttackDamage)
	AUTO(CHECKUNITSTATS, unitTrampleShots)
	AUTO(CHECKUNITSTATS, unitTrampleDamage)
	AUTO(CHECKUNITSTATS, unitAbilityVolleys)
	AUTO(CHECKUNITSTATS, unitAbilityShots)
	AUTO(CHECKUNITSTATS, unitAbilityDamage)
	AUTO(CHECKUNITSTATS, unitAbilityGas)
	AUTO(CHECKUNITSTATS, unitAbilityRads)
	AUTO(CHECKUNITSTATS, unitAbilityRadius)
	AUTO(CHECKUNITSTATS, unitRangeMin)
	AUTO(CHECKUNITSTATS, unitRangeMax)
	AUTO(CHECKUNITSTATS, unitLeakGas)
	AUTO(CHECKUNITSTATS, unitLeakRads)
	AUTO(CHECKUNITSTATS, unitMoraleGainWhenLost)
	AUTO(CHECKUNITSTATS, unitMoraleGainWhenKilled)

	AUTO(CHECKUNITLISTS, unitShapes)
	AUTO(CHECKUNITLISTS, unitSettles)

	AUTO(CHECKUNITSTATS, unitCost)

	AUTO(CHECK, unitSizeMax)
	AUTO(CHECK, unitVisionMax)

	/* COMBAT */
	AUTO(CHECK, missCountGround)
	AUTO(CHECK, missCountAir)
	AUTO(CHECK, missCountTrenches)
	AUTO(CHECK, missHitpointsGround)
	AUTO(CHECK, missHitpointsAir)
	AUTO(CHECK, missHitpointsTrenches)

	/* WEATHER */
	AUTO(CHECKSEASONSTATS, seasonTemperatureSwing)
	AUTO(CHECKSEASONSTATS, seasonGlobalWarmingFactor)

	AUTO(CHECK, emissionDivisor)

	AUTO(CHECK, gasPollutionAmount)
	AUTO(CHECK, aridificationAmountHumid)
	AUTO(CHECK, aridificationAmountDegraded)

	AUTO(CHECK, aridificationRange)

	AUTO(CHECK, aridificationCount)
	AUTO(CHECK, firestormCount)
	AUTO(CHECK, deathCount)

	AUTO(CHECK, temperatureMax)
	AUTO(CHECK, temperatureMin)
	AUTO(CHECK, humidityMax)
	AUTO(CHECK, humidityMin)
	AUTO(CHECK, chaosMax)
	AUTO(CHECK, chaosMin)
	AUTO(CHECK, gasMax)
	AUTO(CHECK, gasMin)
	AUTO(CHECK, radiationMax)
	AUTO(CHECK, radiationMin)

	AUTO(CHECK, chaosThreshold)

	AUTO(CHECKSEASONSTATS, temperatureMinHotDeath)
	AUTO(CHECKSEASONSTATS, temperatureMinFirestorm)
	AUTO(CHECKSEASONSTATS, temperatureMinAridification)
	AUTO(CHECKSEASONSTATS, temperatureMaxComfortable)
	AUTO(CHECKSEASONSTATS, temperatureMinComfortable)
	AUTO(CHECKSEASONSTATS, temperatureMaxSnow)
	AUTO(CHECKSEASONSTATS, temperatureMaxFrostbite)
	AUTO(CHECKSEASONSTATS, temperatureMaxColdDeath)

	AUTO(CHECKSEASONSTATS, humidityMinWet)
	AUTO(CHECKSEASONSTATS, humidityMaxDegradation)
	AUTO(CHECKSEASONSTATS, humidityMaxDesertification)
	AUTO(CHECKSEASONSTATS, humidityMinSnow)
	AUTO(CHECKSEASONSTATS, humidityMinFrostbite)
	AUTO(CHECKSEASONSTATS, humidityMaxFirestorm)
	AUTO(CHECKSEASONSTATS, humidityMaxBonedrought)
	AUTO(CHECKSEASONSTATS, humidityMaxStonedrought)
	AUTO(CHECKSEASONSTATS, humidityMaxDeath)

	AUTO(CHECKSEASONSTATS, chaosMinDegradation)
	AUTO(CHECKSEASONSTATS, chaosMinDesertification)
	AUTO(CHECKSEASONSTATS, chaosMinAridification)
	AUTO(CHECKSEASONSTATS, chaosMinSnow)
	AUTO(CHECKSEASONSTATS, chaosMinFrostbite)
	AUTO(CHECKSEASONSTATS, chaosMinFirestorm)
	AUTO(CHECKSEASONSTATS, chaosMinBonedrought)
	AUTO(CHECKSEASONSTATS, chaosMinStonedrought)
	AUTO(CHECKSEASONSTATS, chaosMinDeath)

	AUTO(CHECK, frostbiteShots)
	AUTO(CHECK, frostbiteDamage)
	AUTO(CHECK, frostbiteThresholdDamage)
	AUTO(CHECK, frostbiteThresholdVulnerability)

	AUTO(CHECK, firestormShots)
	AUTO(CHECK, firestormDamage)
	AUTO(CHECK, firestormBasePercentage)
	AUTO(CHECK, firestormDroughtPercentage)

	AUTO(CHECK, gasShots)
	AUTO(CHECK, gasDamage)
	AUTO(CHECK, gasThresholdDamage)
	AUTO(CHECK, gasThresholdVulnerability)

	AUTO(CHECK, radiationShots)
	AUTO(CHECK, radiationDamage)
	AUTO(CHECK, radiationThresholdDamage)
	AUTO(CHECK, radiationThresholdVulnerability)
	AUTO(CHECK, radiationThresholdDeath)

	AUTO(CHECK, tempGenDefault)
	AUTO(CHECK, humGenDefault)

	AUTO(CHECK, tempGenGainRange)
	AUTO(CHECK, humGenGainRange)

	AUTOCHECKRANGESTATS(tempGenMountainGain, _tempGenGainRange)
	AUTOCHECKRANGESTATS(tempGenOceanGain, _tempGenGainRange)
	AUTOCHECKRANGESTATS(humGenLakeGain, _humGenGainRange)
	AUTOCHECKRANGESTATS(humGenOceanGain, _humGenGainRange)
	AUTOCHECKRANGESTATS(humGenDesertGain, _humGenGainRange)
	AUTOCHECKRANGESTATS(humGenMountainGain, _humGenGainRange)

	/* MECHANICS */
	AUTO(CHECK, powerAbolished)
	AUTO(CHECK, separatePowerStages)
	AUTO(CHECK, industryNicenessQuantitative)
	AUTO(CHECK, reactorNicenessQuantitative)
	AUTO(CHECK, forestChaosProtectionPermanent)
	AUTO(CHECK, collateralDamageKillsTiles)
	AUTO(CHECK, gasOnlyTargetsGroundUnits)
	AUTO(CHECK, frostbiteOnlyTargetsGroundUnits)
	AUTO(CHECK, trenchesProtectMovingUnits)
	AUTO(CHECK, trenchesProtectBypassingUnits)
	AUTO(CHECK, trenchesHideBypassedUnit)
	AUTO(CHECK, captureStrengthCheck)
	AUTO(CHECK, focusOnce)
	AUTO(CHECK, powerDrainScaled)
	AUTO(CHECK, powerCanDrop)
	AUTO(CHECK, upgradeKeepPower)
	AUTO(CHECK, upgradeKeepStacks)
	AUTO(CHECK, upgradeNotBlockedByFriendly)
	AUTO(CHECK, groundPollutionOnlyInAutumn)
	AUTO(CHECK, counterBasedWeather)
	AUTO(CHECK, quantitativeChaos)
	AUTO(CHECK, stackBasedFrostbite)
	AUTO(CHECK, emptyBasedFrostbite)
	AUTO(CHECK, planeBasedFrostbite)
	AUTO(CHECK, planeBasedAridification)
	AUTO(CHECK, flammableBasedFirestorm)
	AUTO(CHECK, percentageBasedFirestorm)
	AUTO(CHECK, randomizedFirestorm)
	AUTO(CHECK, randomizedAridification)
	AUTO(CHECK, cumulativeDeath)
	AUTO(CHECK, vulnerabilitiesStack)
	AUTO(CHECK, markersChangeAtNight)
	AUTO(CHECK, markersChangeAfterBuild)
	AUTO(CHECK, publicInitiative)
	AUTO(CHECK, weatherAfterNight)
	AUTO(CHECK, moraleReplacesMoney)
	AUTO(CHECK, moraleGatheredInSummer)
	AUTO(CHECK, moraleGatheredWhenBuilt)
	AUTO(CHECK, snowCoversNiceness)

	AUTO(CHECK, snowSlowAmount)
	AUTO(CHECK, snowSlowMaximum)

	/* COMMANDERS */
	AUTO(CHECK, startingMoney)
	AUTO(CHECK, maxMoney)
	AUTO(CHECK, minMoney)
	AUTO(CHECK, newOrderLimit)

	// No difference.
	return true;
}

bool Bible::operator==(const Json::Value& json) const
{
	return (*this == Bible("@json", json));
}

bool Bible::operator!=(const Bible& other) const
{
	return !(*this == other);
}

bool Bible::operator!=(const Json::Value& json) const
{
	return !(*this == json);
}
