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

#include <bitset>

#include "version.hpp"
#include "tiletype.hpp"
#include "unittype.hpp"
#include "typenamer.hpp"

enum class Season : uint8_t;


class Bible : public TypeNamer
{
public:
	struct TileBuild
	{
		TileType type;
		int16_t cost;

private:
		TileBuild(TileType tiletype);
		TileBuild(TileType tiletype, int16_t cost);

public:
		bool operator==(const TileBuild& other) const;

		friend Bible;
	};

	struct UnitBuild
	{
		UnitType type;
		int16_t cost;

private:
		UnitBuild(UnitType unittype);
		UnitBuild(UnitType unittype, int16_t cost);

public:
		bool operator==(const UnitBuild& other) const;

		friend Bible;
	};

private:
	Bible(const std::string& name, const Version& version);

	void initialize();
	void customize();
	void finalize();

private:
	friend class Library;
	friend class Editor;

	Bible();

	static Bible createDefault();
	static Bible createCustom();

	Bible(const std::string& name, const Json::Value& json);

public:
	Json::Value toJson() const;

	bool operator==(const Bible& other) const;
	bool operator!=(const Bible& other) const;
	bool operator==(const Json::Value& json) const;
	bool operator!=(const Json::Value& json) const;

private:
	/* NAME */
	std::string _name;

	/* VERSION */
	Version _version;

	/* TILES */
	std::bitset<TILETYPE_SIZE> _tileAccessible;
	std::bitset<TILETYPE_SIZE> _tileWalkable;
	std::bitset<TILETYPE_SIZE> _tileBuildable;
	std::bitset<TILETYPE_SIZE> _tileDestructible;
	std::bitset<TILETYPE_SIZE> _tileGrassy;
	std::bitset<TILETYPE_SIZE> _tileNatural;
	std::bitset<TILETYPE_SIZE> _tileLaboring;
	std::bitset<TILETYPE_SIZE> _tileEnergizing;
	std::bitset<TILETYPE_SIZE> _tilePowered;
	std::bitset<TILETYPE_SIZE> _tileNeedsNiceness;
	std::bitset<TILETYPE_SIZE> _tileNeedsLabor;
	std::bitset<TILETYPE_SIZE> _tileNeedsEnergy;
	std::bitset<TILETYPE_SIZE> _tileNeedsTime;
	std::bitset<TILETYPE_SIZE> _tileOwnable;
	std::bitset<TILETYPE_SIZE> _tileControllable;
	std::bitset<TILETYPE_SIZE> _tileBinding;
	std::bitset<TILETYPE_SIZE> _tileAutoCultivates;
	std::bitset<TILETYPE_SIZE> _tilePlane;
	std::bitset<TILETYPE_SIZE> _tileFlammable;
	std::bitset<TILETYPE_SIZE> _tileWater;
	std::bitset<TILETYPE_SIZE> _tileMountain;
	std::bitset<TILETYPE_SIZE> _tileDesert;
	std::bitset<TILETYPE_SIZE> _tileStone;
	std::bitset<TILETYPE_SIZE> _tileTrenches;
	std::bitset<TILETYPE_SIZE> _tileForceOccupy;
	std::bitset<TILETYPE_SIZE> _tileChaosProtection;
	std::bitset<TILETYPE_SIZE> _tileRegrowOnlyInSpring;
	std::bitset<TILETYPE_SIZE> _tileGathersMorale;

	std::vector<int8_t> _tileStacksBuilt;
	std::vector<int8_t> _tileStacksMax;
	std::vector<int8_t> _tilePowerBuilt;
	std::vector<int8_t> _tilePowerMax;
	std::vector<int8_t> _tileGrowthMax;
	std::vector<int8_t> _tileVision;
	std::vector<int8_t> _tileHitpoints;
	std::vector<int8_t> _tileIncome;
	std::vector<int8_t> _tileLeakGas;
	std::vector<int8_t> _tileLeakRads;
	std::vector<int8_t> _tileEmission;
	std::vector<int8_t> _tilePollutionAmount;
	std::vector<int8_t> _tilePollutionRadius;
	std::vector<int8_t> _tileSlowAmount;
	std::vector<int8_t> _tileSlowMaximum;
	std::vector<int8_t> _tileRegrowthProbabilityDivisor;
	std::vector<int8_t> _tileRegrowthAmount;
	std::vector<int8_t> _tileMoraleGainWhenBuilt;
	std::vector<int8_t> _tileMoraleGainWhenLost;
	std::vector<int8_t> _tileMoraleGainWhenDestroyed;
	std::vector<int8_t> _tileMoraleGainWhenCaptured;
	std::vector<int8_t> _tileMoraleGainWhenRazed;
	std::vector<int8_t> _tileMoraleGainWhenGathered;

	std::vector<std::vector<UnitBuild>> _tileProduces;
	std::vector<std::vector<TileBuild>> _tileExpands;
	std::vector<std::vector<TileBuild>> _tileUpgrades;
	std::vector<std::vector<TileBuild>> _tileCultivates;

	std::vector<int16_t> _tileCost;
	std::vector<int16_t> _tileScoreBase;
	std::vector<int16_t> _tileScoreStack;

	std::vector<TileType> _tileDestroyed;
	std::vector<TileType> _tileDegraded;
	std::vector<TileType> _tileDesertified;
	std::vector<TileType> _tileConsumed;
	std::vector<TileType> _tileRegrown;

	int8_t _tileExpandRangeMin;
	int8_t _tileExpandRangeMax;
	int8_t _tileProduceRangeMin;
	int8_t _tileProduceRangeMax;

	/* UNITS */
	std::bitset<UNITTYPE_SIZE> _unitAir;
	std::bitset<UNITTYPE_SIZE> _unitInfantry;
	std::bitset<UNITTYPE_SIZE> _unitMechanical;
	std::bitset<UNITTYPE_SIZE> _unitCanMove;
	std::bitset<UNITTYPE_SIZE> _unitCanAttack;
	std::bitset<UNITTYPE_SIZE> _unitCanGuard;
	std::bitset<UNITTYPE_SIZE> _unitCanFocus;
	std::bitset<UNITTYPE_SIZE> _unitCanLockdown;
	std::bitset<UNITTYPE_SIZE> _unitCanShell;
	std::bitset<UNITTYPE_SIZE> _unitCanBombard;
	std::bitset<UNITTYPE_SIZE> _unitCanBomb;
	std::bitset<UNITTYPE_SIZE> _unitCanCapture;
	std::bitset<UNITTYPE_SIZE> _unitCanOccupy;

	std::vector<int8_t> _unitStacksMax;
	std::vector<int8_t> _unitSpeed;
	std::vector<int8_t> _unitVision;
	std::vector<int8_t> _unitHitpoints;
	std::vector<int8_t> _unitAttackShots;
	std::vector<int8_t> _unitAttackDamage;
	std::vector<int8_t> _unitTrampleShots;
	std::vector<int8_t> _unitTrampleDamage;
	std::vector<int8_t> _unitAbilityVolleys;
	std::vector<int8_t> _unitAbilityShots;
	std::vector<int8_t> _unitAbilityDamage;
	std::vector<int8_t> _unitAbilityGas;
	std::vector<int8_t> _unitAbilityRads;
	std::vector<int8_t> _unitAbilityRadius;
	std::vector<int8_t> _unitRangeMin;
	std::vector<int8_t> _unitRangeMax;
	std::vector<int8_t> _unitLeakGas;
	std::vector<int8_t> _unitLeakRads;
	std::vector<int8_t> _unitMoraleGainWhenLost;
	std::vector<int8_t> _unitMoraleGainWhenKilled;

	std::vector<std::vector<TileBuild>> _unitShapes;
	std::vector<std::vector<TileBuild>> _unitSettles;

	std::vector<int16_t> _unitCost;

	int8_t _unitSizeMax;
	int8_t _unitVisionMax;

	/* COMBAT */
	int8_t _missCountGround;
	int8_t _missCountAir;
	int8_t _missCountTrenches;
	int8_t _missHitpointsGround;
	int8_t _missHitpointsAir;
	int8_t _missHitpointsTrenches;

	/* WEATHER */
	std::vector<int8_t> _seasonTemperatureSwing;
	std::vector<int8_t> _seasonGlobalWarmingFactor;

	int8_t _emissionDivisor;

	int8_t _gasPollutionAmount;
	int8_t _aridificationAmountHumid;
	int8_t _aridificationAmountDegraded;

	int8_t _aridificationRange;

	int8_t _aridificationCount;
	int8_t _firestormCount;
	int8_t _deathCount;

	int8_t _temperatureMax;
	int8_t _temperatureMin;
	int8_t _humidityMax;
	int8_t _humidityMin;
	int8_t _chaosMax;
	int8_t _chaosMin;
	int8_t _gasMax;
	int8_t _gasMin;
	int8_t _radiationMax;
	int8_t _radiationMin;

	int8_t _chaosThreshold;

	std::vector<int8_t> _temperatureMinHotDeath;
	std::vector<int8_t> _temperatureMinFirestorm;
	std::vector<int8_t> _temperatureMinAridification;
	std::vector<int8_t> _temperatureMaxComfortable;
	std::vector<int8_t> _temperatureMinComfortable;
	std::vector<int8_t> _temperatureMaxSnow;
	std::vector<int8_t> _temperatureMaxFrostbite;
	std::vector<int8_t> _temperatureMaxColdDeath;

	std::vector<int8_t> _humidityMinWet;
	std::vector<int8_t> _humidityMaxDegradation;
	std::vector<int8_t> _humidityMaxDesertification;
	std::vector<int8_t> _humidityMinSnow;
	std::vector<int8_t> _humidityMinFrostbite;
	std::vector<int8_t> _humidityMaxFirestorm;
	std::vector<int8_t> _humidityMaxBonedrought;
	std::vector<int8_t> _humidityMaxStonedrought;
	std::vector<int8_t> _humidityMaxDeath;

	std::vector<int8_t> _chaosMinDegradation;
	std::vector<int8_t> _chaosMinDesertification;
	std::vector<int8_t> _chaosMinAridification;
	std::vector<int8_t> _chaosMinSnow;
	std::vector<int8_t> _chaosMinFrostbite;
	std::vector<int8_t> _chaosMinFirestorm;
	std::vector<int8_t> _chaosMinBonedrought;
	std::vector<int8_t> _chaosMinStonedrought;
	std::vector<int8_t> _chaosMinDeath;

	int8_t _frostbiteShots;
	int8_t _frostbiteDamage;
	int8_t _frostbiteThresholdDamage;
	int8_t _frostbiteThresholdVulnerability;

	int8_t _firestormShots;
	int8_t _firestormDamage;

	int8_t _gasShots;
	int8_t _gasDamage;
	int8_t _gasThresholdDamage;
	int8_t _gasThresholdVulnerability;

	int8_t _radiationShots;
	int8_t _radiationDamage;
	int8_t _radiationThresholdDamage;
	int8_t _radiationThresholdVulnerability;
	int8_t _radiationThresholdDeath;

	int8_t _tempGenDefault;
	int8_t _humGenDefault;

	int8_t _tempGenGainRange;
	int8_t _humGenGainRange;

	std::vector<int8_t> _tempGenMountainGain;
	std::vector<int8_t> _tempGenOceanGain;
	std::vector<int8_t> _humGenLakeGain;
	std::vector<int8_t> _humGenOceanGain;
	std::vector<int8_t> _humGenDesertGain;
	std::vector<int8_t> _humGenMountainGain;

	/* MECHANICS */
	bool _powerAbolished;
	bool _separatePowerStages;
	bool _industryNicenessQuantitative;
	bool _reactorNicenessQuantitative;
	bool _forestChaosProtectionPermanent;
	bool _collateralDamageKillsTiles;
	bool _gasOnlyTargetsGroundUnits;
	bool _frostbiteOnlyTargetsGroundUnits;
	bool _trenchesProtectMovingUnits;
	bool _trenchesProtectBypassingUnits;
	bool _trenchesHideBypassedUnit;
	bool _captureStrengthCheck;
	bool _focusOnce;
	bool _powerDrainScaled;
	bool _powerCanDrop;
	bool _upgradeKeepPower;
	bool _upgradeKeepStacks;
	bool _upgradeNotBlockedByFriendly;
	bool _groundPollutionOnlyInAutumn;
	bool _counterBasedWeather;
	bool _quantitativeChaos;
	bool _stackBasedFrostbite;
	bool _emptyBasedFrostbite;
	bool _planeBasedFrostbite;
	bool _planeBasedAridification;
	bool _flammableBasedFirestorm;
	bool _randomizedFirestorm;
	bool _randomizedAridification;
	bool _cumulativeDeath;
	bool _vulnerabilitiesStack;
	bool _markersChangeAtNight;
	bool _markersChangeAfterBuild;
	bool _publicInitiative;
	bool _weatherAfterNight;
	bool _moraleReplacesMoney;
	bool _moraleGatheredInSummer;
	bool _moraleGatheredWhenBuilt;
	bool _snowCoversNiceness;

	int8_t _snowSlowAmount;
	int8_t _snowSlowMaximum;

	/* COMMANDERS */
	int16_t _startingMoney;
	int16_t _maxMoney;
	int16_t _minMoney;
	uint8_t _newOrderLimit;

public:
	/* NAME & VERSION */
	const std::string& name() const { return _name; }
	const Version& version() const { return _version; }

	/* TILES */
	bool tileAccessible(  const TileType& x) const { return _tileAccessible[(size_t) x];   }
	bool tileWalkable(    const TileType& x) const { return _tileWalkable[(size_t) x];     }
	bool tileBuildable(   const TileType& x) const { return _tileBuildable[(size_t) x];    }
	bool tileDestructible(const TileType& x) const { return _tileDestructible[(size_t) x]; }
	bool tileGrassy(      const TileType& x) const { return _tileGrassy[(size_t) x];       }
	bool tileNatural(     const TileType& x) const { return _tileNatural[(size_t) x];      }
	bool tileLaboring(    const TileType& x) const { return _tileLaboring[(size_t) x];     }
	bool tileEnergizing(  const TileType& x) const { return _tileEnergizing[(size_t) x];   }
	bool tilePowered(     const TileType& x) const { return _tilePowered[(size_t) x];      }
	bool tileNeedsNiceness(const TileType& x) const { return _tileNeedsNiceness[(size_t) x]; }
	bool tileNeedsLabor(const TileType& x) const { return _tileNeedsLabor[(size_t) x]; }
	bool tileNeedsEnergy(const TileType& x) const { return _tileNeedsEnergy[(size_t) x]; }
	bool tileNeedsTime(const TileType& x) const { return _tileNeedsTime[(size_t) x]; }
	bool tileOwnable(     const TileType& x) const { return _tileOwnable[(size_t) x];      }
	bool tileControllable(const TileType& x) const { return _tileControllable[(size_t) x]; }
	bool tileBinding(const TileType& x) const { return _tileBinding[(size_t) x]; }
	bool tileAutoCultivates(const TileType& x) const { return _tileAutoCultivates[(size_t) x]; }
	bool tilePlane(const TileType& x) const { return _tilePlane[(size_t) x]; }
	bool tileFlammable(const TileType& x) const { return _tileFlammable[(size_t) x]; }
	bool tileWater(const TileType& x) const { return _tileWater[(size_t) x]; }
	bool tileMountain(const TileType& x) const { return _tileMountain[(size_t) x]; }
	bool tileDesert(const TileType& x) const { return _tileDesert[(size_t) x]; }
	bool tileStone(const TileType& x) const { return _tileStone[(size_t) x]; }
	bool tileTrenches(const TileType& x) const { return _tileTrenches[(size_t) x]; }
	bool tileForceOccupy(const TileType& x) const { return _tileForceOccupy[(size_t) x]; }
	bool tileChaosProtection(const TileType& x) const { return _tileChaosProtection[(size_t) x]; }
	bool tileRegrowOnlyInSpring(const TileType& x) const { return _tileRegrowOnlyInSpring[(size_t) x]; }
	bool tileGathersMorale(const TileType& x) const { return _tileGathersMorale[(size_t) x]; }

	int8_t tileStacksBuilt(  const TileType& x) const { return _tileStacksBuilt[(size_t) x];   }
	int8_t tileStacksMax(    const TileType& x) const { return _tileStacksMax[(size_t) x];     }
	int8_t tilePowerBuilt(   const TileType& x) const { return _tilePowerBuilt[(size_t) x];    }
	int8_t tilePowerMax(     const TileType& x) const { return _tilePowerMax[(size_t) x];      }
	int8_t tileGrowthMax(    const TileType& x) const { return _tileGrowthMax[(size_t) x];     }
	int8_t tileVision(       const TileType& x) const { return _tileVision[(size_t) x];        }
	int8_t tileHitpoints(    const TileType& x) const { return _tileHitpoints[(size_t) x];     }
	int8_t tileIncome(       const TileType& x) const { return _tileIncome[(size_t) x];        }
	int8_t tileLeakGas(      const TileType& x) const { return _tileLeakGas[(size_t) x];       }
	int8_t tileLeakRads(     const TileType& x) const { return _tileLeakRads[(size_t) x];      }
	int8_t tileEmission(     const TileType& x) const { return _tileEmission[(size_t) x];      }
	int8_t tilePollutionAmount(const TileType& x) const { return _tilePollutionAmount[(size_t) x]; }
	int8_t tilePollutionRadius(const TileType& x) const { return _tilePollutionRadius[(size_t) x]; }
	int8_t tileSlowAmount(   const TileType& x) const { return _tileSlowAmount[(size_t) x];    }
	int8_t tileSlowMaximum(  const TileType& x) const { return _tileSlowMaximum[(size_t) x];   }
	int8_t tileRegrowthProbabilityDivisor(const TileType& x) const { return _tileRegrowthProbabilityDivisor[(size_t) x]; }
	int8_t tileRegrowthAmount(const TileType& x) const { return _tileRegrowthAmount[(size_t) x]; }
	int8_t tileMoraleGainWhenBuilt(const TileType& x) const { return _tileMoraleGainWhenBuilt[(size_t) x]; }
	int8_t tileMoraleGainWhenLost(const TileType& x) const { return _tileMoraleGainWhenLost[(size_t) x]; }
	int8_t tileMoraleGainWhenDestroyed(const TileType& x) const { return _tileMoraleGainWhenDestroyed[(size_t) x]; }
	int8_t tileMoraleGainWhenCaptured(const TileType& x) const { return _tileMoraleGainWhenCaptured[(size_t) x]; }
	int8_t tileMoraleGainWhenRazed(const TileType& x) const { return _tileMoraleGainWhenRazed[(size_t) x]; }
	int8_t tileMoraleGainWhenGathered(const TileType& x) const { return _tileMoraleGainWhenGathered[(size_t) x]; }

	const std::vector<UnitBuild>& tileProduces(const TileType& x) const
	{
		return _tileProduces[(size_t) x];
	}
	const std::vector<TileBuild>& tileExpands(const TileType& x) const
	{
		return _tileExpands[(size_t) x];
	}
	const std::vector<TileBuild>& tileUpgrades(const TileType& x) const
	{
		return _tileUpgrades[(size_t) x];
	}
	const std::vector<TileBuild>& tileCultivates(const TileType& x) const
	{
		return _tileCultivates[(size_t) x];
	}

/* This has been made obsolete by TileBuild.
	int16_t tileCost(      const TileType& x) const { return _tileCost[(size_t) x];       }
*/

	int16_t tileScoreBase( const TileType& x) const { return _tileScoreBase[(size_t) x];  }
	int16_t tileScoreStack(const TileType& x) const { return _tileScoreStack[(size_t) x]; }

	TileType tileDestroyed(const TileType& x) const { return _tileDestroyed[(size_t) x]; }
	TileType tileDegraded(const TileType& x) const { return _tileDegraded[(size_t) x]; }
	TileType tileDesertified(const TileType& x) const { return _tileDesertified[(size_t) x]; }
	TileType tileConsumed(const TileType& x) const { return _tileConsumed[(size_t) x]; }
	TileType tileRegrown(const TileType& x) const { return _tileRegrown[(size_t) x]; }

	int8_t tileExpandRangeMin() const { return _tileExpandRangeMin; }
	int8_t tileExpandRangeMax() const { return _tileExpandRangeMax; }
	int8_t tileProduceRangeMin() const { return _tileProduceRangeMin; }
	int8_t tileProduceRangeMax() const { return _tileProduceRangeMax; }

	/* UNITS */
	bool unitAir(       const UnitType& x) const { return _unitAir[(size_t) x];        }
	bool unitInfantry(  const UnitType& x) const { return _unitInfantry[(size_t) x];   }
	bool unitMechanical(const UnitType& x) const { return _unitMechanical[(size_t) x]; }
	bool unitCanMove(   const UnitType& x) const { return _unitCanMove[(size_t) x];    }
	bool unitCanAttack( const UnitType& x) const { return _unitCanAttack[(size_t) x];  }
	bool unitCanGuard(  const UnitType& x) const { return _unitCanGuard[(size_t) x];   }
	bool unitCanFocus(  const UnitType& x) const { return _unitCanFocus[(size_t) x];   }
	bool unitCanLockdown(const UnitType& x) const { return _unitCanLockdown[(size_t) x]; }
	bool unitCanShell(  const UnitType& x) const { return _unitCanShell[(size_t) x];   }
	bool unitCanBombard(const UnitType& x) const { return _unitCanBombard[(size_t) x]; }
	bool unitCanBomb(   const UnitType& x) const { return _unitCanBomb[(size_t) x];    }
	bool unitCanCapture(const UnitType& x) const { return _unitCanCapture[(size_t) x]; }
	bool unitCanOccupy( const UnitType& x) const { return _unitCanOccupy[(size_t) x];  }

	int8_t unitStacksMax(    const UnitType& x) const { return _unitStacksMax[(size_t) x];     }
	int8_t unitSpeed(        const UnitType& x) const { return _unitSpeed[(size_t) x];         }
	int8_t unitVision(       const UnitType& x) const { return _unitVision[(size_t) x];        }
	int8_t unitHitpoints(    const UnitType& x) const { return _unitHitpoints[(size_t) x];     }
	int8_t unitAttackShots(  const UnitType& x) const { return _unitAttackShots[(size_t) x];   }
	int8_t unitAttackDamage( const UnitType& x) const { return _unitAttackDamage[(size_t) x];  }
	int8_t unitTrampleShots( const UnitType& x) const { return _unitTrampleShots[(size_t) x];  }
	int8_t unitTrampleDamage(const UnitType& x) const { return _unitTrampleDamage[(size_t) x]; }
	int8_t unitAbilityShots( const UnitType& x) const { return _unitAbilityShots[(size_t) x];  }
	int8_t unitAbilityVolleys(const UnitType& x) const {return _unitAbilityVolleys[(size_t) x];}
	int8_t unitAbilityDamage(const UnitType& x) const { return _unitAbilityDamage[(size_t) x]; }
	int8_t unitAbilityGas(   const UnitType& x) const { return _unitAbilityGas[(size_t) x];    }
	int8_t unitAbilityRads(  const UnitType& x) const { return _unitAbilityRads[(size_t) x];   }
	int8_t unitAbilityRadius(const UnitType& x) const { return _unitAbilityRadius[(size_t) x]; }
	int8_t unitRangeMin(     const UnitType& x) const { return _unitRangeMin[(size_t) x];      }
	int8_t unitRangeMax(     const UnitType& x) const { return _unitRangeMax[(size_t) x];      }
	int8_t unitLeakGas(      const UnitType& x) const { return _unitLeakGas[(size_t) x];       }
	int8_t unitLeakRads(     const UnitType& x) const { return _unitLeakRads[(size_t) x];      }
	int8_t unitMoraleGainWhenLost(const UnitType& x) const { return _unitMoraleGainWhenLost[(size_t) x]; }
	int8_t unitMoraleGainWhenKilled(const UnitType& x) const { return _unitMoraleGainWhenKilled[(size_t) x]; }

	const std::vector<TileBuild>& unitShapes(const UnitType& x) const
	{
		return _unitShapes[(size_t) x];
	}

	const std::vector<TileBuild>& unitSettles(const UnitType& x) const
	{
		return _unitSettles[(size_t) x];
	}

/* This has been made obsolete by UnitBuild.
	int16_t unitCost(const UnitType& x) const { return _unitCost[(size_t) x]; }
*/

	int8_t unitSizeMax() const { return _unitSizeMax; }
	int8_t unitVisionMax() const { return _unitVisionMax; }

	/* COMBAT */
	int8_t missCountGround()       const { return _missCountGround;       }
	int8_t missCountAir()          const { return _missCountAir;          }
	int8_t missCountTrenches()     const { return _missCountTrenches;     }
	int8_t missHitpointsGround()   const { return _missHitpointsGround;   }
	int8_t missHitpointsAir()      const { return _missHitpointsAir;      }
	int8_t missHitpointsTrenches() const { return _missHitpointsTrenches; }

	/* WEATHER */
	int8_t seasonTemperatureSwing(const Season& season) const
	{
		return _seasonTemperatureSwing[(size_t) season];
	}

	int8_t seasonGlobalWarmingFactor(const Season& season) const
	{
		return _seasonGlobalWarmingFactor[(size_t) season];
	}

	int8_t emissionDivisor() const { return _emissionDivisor; }

	int8_t gasPollutionAmount() const              { return _gasPollutionAmount;               }
	int8_t aridificationAmountHumid() const        { return _aridificationAmountHumid;         }
	int8_t aridificationAmountDegraded() const     { return _aridificationAmountDegraded;      }

	int8_t aridificationRange() const              { return _aridificationRange;               }

	int8_t aridificationCount() const              { return _aridificationCount;               }
	int8_t firestormCount() const                  { return _firestormCount;                   }
	int8_t deathCount() const                      { return _deathCount;                       }

	int8_t temperatureMax() const                  { return _temperatureMax;                   }
	int8_t temperatureMin() const                  { return _temperatureMin;                   }
	int8_t humidityMax() const                     { return _humidityMax;                      }
	int8_t humidityMin() const                     { return _humidityMin;                      }
	int8_t chaosMax() const                        { return _chaosMax;                         }
	int8_t chaosMin() const                        { return _chaosMin;                         }
	int8_t gasMax() const                          { return _gasMax;                           }
	int8_t gasMin() const                          { return _gasMin;                           }
	int8_t radiationMax() const                    { return _radiationMax;                     }
	int8_t radiationMin() const                    { return _radiationMin;                     }

	int8_t chaosThreshold() const                  { return _chaosThreshold;                   }

	int8_t temperatureMinHotDeath(const Season& season)      const { return _temperatureMinHotDeath[(size_t) season];      }
	int8_t temperatureMinFirestorm(const Season& season)     const { return _temperatureMinFirestorm[(size_t) season];     }
	int8_t temperatureMinAridification(const Season& season) const { return _temperatureMinAridification[(size_t) season]; }
	int8_t temperatureMaxComfortable(const Season& season)   const { return _temperatureMaxComfortable[(size_t) season];   }
	int8_t temperatureMinComfortable(const Season& season)   const { return _temperatureMinComfortable[(size_t) season];   }
	int8_t temperatureMaxSnow(const Season& season)          const { return _temperatureMaxSnow[(size_t) season];          }
	int8_t temperatureMaxFrostbite(const Season& season)     const { return _temperatureMaxFrostbite[(size_t) season];     }
	int8_t temperatureMaxColdDeath(const Season& season)     const { return _temperatureMaxColdDeath[(size_t) season];     }

	int8_t humidityMinWet(const Season& season)             const { return _humidityMinWet[(size_t) season];             }
	int8_t humidityMaxDegradation(const Season& season)     const { return _humidityMaxDegradation[(size_t) season];     }
	int8_t humidityMaxDesertification(const Season& season) const { return _humidityMaxDesertification[(size_t) season]; }
	int8_t humidityMinSnow(const Season& season)            const { return _humidityMinSnow[(size_t) season];            }
	int8_t humidityMinFrostbite(const Season& season)       const { return _humidityMinFrostbite[(size_t) season];       }
	int8_t humidityMaxFirestorm(const Season& season)       const { return _humidityMaxFirestorm[(size_t) season];       }
	int8_t humidityMaxBonedrought(const Season& season)     const { return _humidityMaxBonedrought[(size_t) season];     }
	int8_t humidityMaxStonedrought(const Season& season)    const { return _humidityMaxStonedrought[(size_t) season];    }
	int8_t humidityMaxDeath(const Season& season)           const { return _humidityMaxDeath[(size_t) season];           }

	int8_t chaosMinDegradation(const Season& season)     const { return _chaosMinDegradation[(size_t) season];     }
	int8_t chaosMinDesertification(const Season& season) const { return _chaosMinDesertification[(size_t) season]; }
	int8_t chaosMinAridification(const Season& season)   const { return _chaosMinAridification[(size_t) season];   }
	int8_t chaosMinSnow(const Season& season)            const { return _chaosMinSnow[(size_t) season];            }
	int8_t chaosMinFrostbite(const Season& season)       const { return _chaosMinFrostbite[(size_t) season];       }
	int8_t chaosMinFirestorm(const Season& season)       const { return _chaosMinFirestorm[(size_t) season];       }
	int8_t chaosMinBonedrought(const Season& season)     const { return _chaosMinBonedrought[(size_t) season];     }
	int8_t chaosMinStonedrought(const Season& season)    const { return _chaosMinStonedrought[(size_t) season];    }
	int8_t chaosMinDeath(const Season& season)           const { return _chaosMinDeath[(size_t) season];           }

	int8_t frostbiteShots()                  const { return _frostbiteShots;                  }
	int8_t frostbiteDamage()                 const { return _frostbiteDamage;                 }
	int8_t frostbiteThresholdDamage()        const { return _frostbiteThresholdDamage;        }
	int8_t frostbiteThresholdVulnerability() const { return _frostbiteThresholdVulnerability; }

	int8_t firestormShots()  const { return _firestormShots;  }
	int8_t firestormDamage() const { return _firestormDamage; }

	int8_t gasShots()                  const { return _gasShots;                  }
	int8_t gasDamage()                 const { return _gasDamage;                 }
	int8_t gasThresholdDamage()        const { return _gasThresholdDamage;        }
	int8_t gasThresholdVulnerability() const { return _gasThresholdVulnerability; }

	int8_t radiationShots()                  const { return _radiationShots;                  }
	int8_t radiationDamage()                 const { return _radiationDamage;                 }
	int8_t radiationThresholdDamage()        const { return _radiationThresholdDamage;        }
	int8_t radiationThresholdVulnerability() const { return _radiationThresholdVulnerability; }
	int8_t radiationThresholdDeath()         const { return _radiationThresholdDeath;         }

	int8_t tempGenDefault()      const { return _tempGenDefault;      }
	int8_t humGenDefault()       const { return _humGenDefault;       }
	int8_t tempGenGainRange()    const { return _tempGenGainRange;    }
	int8_t humGenGainRange()     const { return _humGenGainRange;     }

	int8_t tempGenMountainGain(size_t dist) const    { return _tempGenMountainGain[dist];     }
	int8_t tempGenOceanGain(size_t dist)    const    { return _tempGenOceanGain[dist];        }
	int8_t humGenLakeGain(size_t dist)     const     { return _humGenLakeGain[dist];          }
	int8_t humGenOceanGain(size_t dist)    const     { return _humGenOceanGain[dist];         }
	int8_t humGenDesertGain(size_t dist)   const     { return _humGenDesertGain[dist];        }
	int8_t humGenMountainGain(size_t dist) const     { return _humGenMountainGain[dist];      }

	/* MECHANICS */
	bool powerAbolished()                    const { return _powerAbolished;                   }
	bool separatePowerStages()               const { return _separatePowerStages;              }
	bool industryNicenessQuantitative()      const { return _industryNicenessQuantitative;     }
	bool reactorNicenessQuantitative()       const { return _reactorNicenessQuantitative;      }
	bool forestChaosProtectionPermanent()    const { return _forestChaosProtectionPermanent;   }
	bool collateralDamageKillsTiles()        const { return _collateralDamageKillsTiles;       }
	bool gasOnlyTargetsGroundUnits()         const { return _gasOnlyTargetsGroundUnits;        }
	bool frostbiteOnlyTargetsGroundUnits()   const { return _frostbiteOnlyTargetsGroundUnits;  }
	bool trenchesProtectMovingUnits()        const { return _trenchesProtectMovingUnits;       }
	bool trenchesProtectBypassingUnits()     const { return _trenchesProtectBypassingUnits;    }
	bool trenchesHideBypassedUnit()          const { return _trenchesHideBypassedUnit;         }
	bool captureStrengthCheck()              const { return _captureStrengthCheck;             }
	bool focusOnce()                         const { return _focusOnce;                        }
	bool powerDrainScaled()                  const { return _powerDrainScaled;                 }
	bool powerCanDrop()                      const { return _powerCanDrop;                     }
	bool upgradeKeepPower()                  const { return _upgradeKeepPower;                 }
	bool upgradeKeepStacks()                 const { return _upgradeKeepStacks;                }
	bool upgradeNotBlockedByFriendly()       const { return _upgradeNotBlockedByFriendly;      }
	bool groundPollutionOnlyInAutumn()       const { return _groundPollutionOnlyInAutumn;      }
	bool counterBasedWeather()               const { return _counterBasedWeather;              }
	bool quantitativeChaos()                 const { return _quantitativeChaos;                }
	bool stackBasedFrostbite()               const { return _stackBasedFrostbite;              }
	bool emptyBasedFrostbite()               const { return _emptyBasedFrostbite;              }
	bool planeBasedFrostbite()               const { return _planeBasedFrostbite;              }
	bool planeBasedAridification()           const { return _planeBasedAridification;          }
	bool flammableBasedFirestorm()           const { return _flammableBasedFirestorm;          }
	bool randomizedFirestorm()               const { return _randomizedFirestorm;              }
	bool randomizedAridification()           const { return _randomizedAridification;          }
	bool cumulativeDeath()                   const { return _cumulativeDeath;                  }
	bool vulnerabilitiesStack()              const { return _vulnerabilitiesStack;             }
	bool markersChangeAtNight()              const { return _markersChangeAtNight;             }
	bool markersChangeAfterBuild()           const { return _markersChangeAfterBuild;          }
	bool publicInitiative()                  const { return _publicInitiative;                 }
	bool weatherAfterNight()                 const { return _weatherAfterNight;                }
	bool moraleReplacesMoney()               const { return _moraleReplacesMoney;           }
	bool moraleGatheredInSummer()            const { return _moraleGatheredInSummer;           }
	bool moraleGatheredWhenBuilt()           const { return _moraleGatheredWhenBuilt;          }
	bool snowCoversNiceness()                const { return _snowCoversNiceness;      }

	int8_t snowSlowAmount()            const { return _snowSlowAmount;              }
	int8_t snowSlowMaximum()           const { return _snowSlowMaximum;             }

	/* COMMANDERS */
	int16_t startingMoney() const { return _startingMoney; }
	int16_t maxMoney() const { return _maxMoney; }
	int16_t minMoney() const { return _minMoney; }
	uint8_t newOrderLimit()  const { return _newOrderLimit;  }
};
