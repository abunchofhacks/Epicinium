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

#include "descriptor.hpp"
#include "tiletoken.hpp"
#include "unittoken.hpp"
#include "attacker.hpp"
#include "order.hpp"
#include "vision.hpp"

enum class Player : uint8_t;
enum class Season : uint8_t;
enum class Daytime : uint8_t;
enum class Phase : uint8_t;
enum class Notice : uint8_t;

class TypeNamer;


struct Change
{
public:
	enum class Type : uint8_t
	{
		NONE = 0,    // look at <subject> cell/tile/unit and see no changes
		STARTS,      // <subject> unit starts to move to <target> cellslot
		MOVES,       // <subject> unit moved to <target> cell
		REVEAL,      // reveal <subject> cell from fog of war and set <tile>, <snow>, <gas>, ...
		OBSCURE,     // cover <subject> cell in fog of war
		TRANSFORMED, // <subject> tile was naturally transformed into <tile>
		CONSUMED,    // <subject> tile was consumed into <tile>
		SHAPES,      // <subject> unit shapes a tile
		SHAPED,      // <subject> tile was shaped into <tile>
		SETTLES,     // <subject> unit settles a tile and is removed
		SETTLED,     // <subject> tile was settled into <tile>
		EXPANDS,     // <subject> tile expands a tile in <target> cell and spends <power>
		EXPANDED,    // <subject> tile was expanded into <tile>
		UPGRADES,    // <subject> tile upgrades itself and spends <power>
		UPGRADED,    // <subject> tile was upgraded into <tile>
		CULTIVATES,  // <subject> tile cultivates tiles around itself spends <power>
		CULTIVATED,  // <subject> tile was cultivated into <tile>
		CAPTURES,    // <subject> unit captures a tile
		CAPTURED,    // <subject> tile was captured now belongs to <player>
		PRODUCES,    // <subject> tile produces a unit and spends <power>
		PRODUCED,    // <subject> unit is a newly produced <unit>
		ENTERED,     // <subject> unit is a new <unit> that appeared out of fog of war
		EXITED,      // <subject> unit disappeared in the fog of war
		DIED,        // <subject> unit died because it lost all its figures
		DESTROYED,   // <subject> tile was destroyed into <tile>
		SURVIVED,    // <subject> unit survived a damage step
		AIMS,        // <subject> unit aims at something in <target> cell
		ATTACKS,     // <subject> unit's <figure> fires upon something in <target> cell
		ATTACKED,    // <subject> tile/unit's <figure> was hit by <attacker> and is <killed>
		TRAMPLES,    // <subject> unit's <figure> tramples its cell
		TRAMPLED,    // <subject> tile/unit's <figure> was hit by <bombarder> and is <killed>
		SHELLS,      // <subject> unit's <figure> fires upon something in <target> cell
		SHELLED,     // <subject> tile/unit's <figure> was hit by <attacker> and is <killed>
		BOMBARDS,    // <subject> unit's <figure> bombards some cell
		BOMBARDED,   // <subject> tile/unit's <figure> was hit by <bombarder> and is <killed>
		BOMBS,       // <subject> unit's <figure> bombs its cell
		BOMBED,      // <subject> tile/unit's <figure> was hit by <bombarder> and is <killed>
		FROSTBITTEN, // <subject> tile/unit's <figure> was hit by frostbite and is <killed>
		BURNED,      // <subject> tile/unit's <figure> was hit by firestorm and is <killed>
		GASSED,      // <subject> tile/unit's <figure> was hit by gas and is <killed>
		IRRADIATED,  // <subject> tile/unit's <figure> was hit by radiation and is <killed>
//		NUKED,       // <subject> tile/unit's <figure> was nuked and is <killed>
//		NUCLEAR,     // <subject> cell was the center of a nuclear explosion
		GROWS,       // <subject> tile gains <stacks> and <power>
		SNOW,        // set <snow> of <subject> cell
		FROSTBITE,   // set <frostbite> of <subject> cell
		FIRESTORM,   // set <firestorm> of <subject> cell
		BONEDROUGHT, // set <bonedrought> of <subject> cell
		DEATH,       // set <death> of <subject> cell
		GAS,         // change <gas> of <subject> cell
		RADIATION,   // change <radiation> of <subject> cell
		TEMPERATURE, // change <temperature> of <subject> cell
		HUMIDITY,    // change <humidity> of <subject> cell
		CHAOS,       // change <chaos> of <subject> cell
		CHAOSREPORT, // global warming has reached <level>
		YEAR,        // set the year to <year>
		SEASON,      // set the season to <season>
		DAYTIME,     // set the daytime to <daytime>
		PHASE,       // set the phase to <phase>
		INITIATIVE,  // <player> has <initiative> (1st, 2nd, etc)
		FUNDS,       // you/<player> gain <money> funds
		INCOME,      // you/<player> gain <money> income from <subject> tile
		EXPENDITURE, // you/<player> gain <money> from expenditures of <subject> tile/unit
		SLEEPING,    // you/<player> gave a sleep order, delaying other orders
		ACTING,      // your <subject> is currently acting out its order
		FINISHED,    // your order assigned to <subject> was finished
		DISCARDED,   // your order assigned to <subject> was discarded
		POSTPONED,   // <subject> still has <order> from you/<player> because it was postponed
		UNFINISHED,  // <subject> still has <order> from you/<player> and will continue later
		ORDERED,     // you/<player> gave an <order> to <subject>
		VISION,      // update the <vision> of <subject> cell
		CORNER,      // declare <subject> to be the bottom right corner
		BORDER,      // declare that the entire map has been announced
		SCORED,      // <player> gained <score> from <subject>
		DEFEAT,      // <player> was defeated and got <score>
		VICTORY,     // <player> was victorious and got <score>
		GAMEOVER,    // the game has ended; the world was worth <score>
		AWARD,       // <player> was awarded <level> stars for this challenge
	};

	Type type;
	Descriptor subject;
	union { int8_t __byteA;    Player player;        bool snow;              };
	union { int8_t __byteB;    int8_t stacks;        bool frostbite;         };
	union { int8_t __byteC;    int8_t power;         bool firestorm;         };
	union { int8_t __byteD;    int8_t figure;        bool bonedrought;       };
	union { int8_t __byteE;    bool killed;          bool death;             };
	union { int8_t __byteF;    bool depowered;       int8_t gas;             };
	union { int8_t __byteG;    Notice notice;        int8_t radiation;       };
	union { int8_t __byteH;    /*unused*/            int8_t temperature;     };
	union { int8_t __byteI;    /*unused*/            int8_t humidity;        };
	union { int8_t __byteJ;    /*unused*/            int8_t chaos;           };
	union
	{
		Descriptor target;
		TileToken tile;
		UnitToken unit;
		Attacker attacker;
		Bombarder bombarder;
		Season season;
		Daytime daytime;
		Phase phase;
		int16_t year;
		int16_t money;
		int16_t score;
		int8_t initiative;
		int8_t level;
		// If none of these are used, we zero-initialize 32 bits.
		int32_t __byteZ;
	};
	Order order;
	Vision vision;

	Change() :
		type(Type::NONE),
		subject(),
		__byteA(0),
		__byteB(0),
		__byteC(0),
		__byteD(0),
		__byteE(0),
		__byteF(0),
		__byteG(0),
		__byteH(0),
		__byteI(0),
		__byteJ(0),
		__byteZ(0),
		order(),
		vision()
	{}

	explicit Change(Type type) :
		type(type),
		subject(),
		__byteA(0),
		__byteB(0),
		__byteC(0),
		__byteD(0),
		__byteE(0),
		__byteF(0),
		__byteG(0),
		__byteH(0),
		__byteI(0),
		__byteJ(0),
		__byteZ(0),
		order(),
		vision()
	{}

	Change(Type type, const Descriptor& subject) :
		type(type),
		subject(subject),
		__byteA(0),
		__byteB(0),
		__byteC(0),
		__byteD(0),
		__byteE(0),
		__byteF(0),
		__byteG(0),
		__byteH(0),
		__byteI(0),
		__byteJ(0),
		__byteZ(0),
		order(),
		vision()
	{}

	Change(Type type, const Descriptor& subject, const Descriptor& target) :
		type(type),
		subject(subject),
		__byteA(0),
		__byteB(0),
		__byteC(0),
		__byteD(0),
		__byteE(0),
		__byteF(0),
		__byteG(0),
		__byteH(0),
		__byteI(0),
		__byteJ(0),
		target(target),
		order(),
		vision()
	{}

	Change(Type type, const Descriptor& subject, const TileToken& tile) :
		type(type),
		subject(subject),
		__byteA(0),
		__byteB(0),
		__byteC(0),
		__byteD(0),
		__byteE(0),
		__byteF(0),
		__byteG(0),
		__byteH(0),
		__byteI(0),
		__byteJ(0),
		tile(tile),
		order(),
		vision()
	{}

	Change(Type type, const Descriptor& subject, const UnitToken& unit) :
		type(type),
		subject(subject),
		__byteA(0),
		__byteB(0),
		__byteC(0),
		__byteD(0),
		__byteE(0),
		__byteF(0),
		__byteG(0),
		__byteH(0),
		__byteI(0),
		__byteJ(0),
		unit(unit),
		order(),
		vision()
	{}

	Change(Type type, const Descriptor& subject, const Player& player) :
		type(type),
		subject(subject),
		player(player),
		__byteB(0),
		__byteC(0),
		__byteD(0),
		__byteE(0),
		__byteF(0),
		__byteG(0),
		__byteH(0),
		__byteI(0),
		__byteJ(0),
		__byteZ(0),
		order(),
		vision()
	{}

	Change(Type type, const Descriptor& subject, const Player& player,
			const Order& order) :
		type(type),
		subject(subject),
		player(player),
		__byteB(0),
		__byteC(0),
		__byteD(0),
		__byteE(0),
		__byteF(0),
		__byteG(0),
		__byteH(0),
		__byteI(0),
		__byteJ(0),
		__byteZ(0),
		order(order),
		vision()
	{}

	Change(Type type, const Descriptor& subject, const Vision& vision) :
		type(type),
		subject(subject),
		__byteA(0),
		__byteB(0),
		__byteC(0),
		__byteD(0),
		__byteE(0),
		__byteF(0),
		__byteG(0),
		__byteH(0),
		__byteI(0),
		__byteJ(0),
		__byteZ(0),
		order(),
		vision(vision)
	{}

	Change(Type type, const Descriptor& subject, const TileToken& tile,
			bool snow, bool frostbite, bool firestorm, bool bonedrought,
			bool death, int8_t gas, int8_t radiation,
			int8_t temperature, int8_t humidity, int8_t chaos) :
		type(type),
		subject(subject),
		snow(snow),
		frostbite(frostbite),
		firestorm(firestorm),
		bonedrought(bonedrought),
		death(death),
		gas(gas),
		radiation(radiation),
		temperature(temperature),
		humidity(humidity),
		chaos(chaos),
		tile(tile),
		order(),
		vision()
	{}

	Change(Type type, const Descriptor& subject, int8_t stacks, int8_t power) :
		type(type),
		subject(subject),
		__byteA(0),
		stacks(stacks),
		power(power),
		__byteD(0),
		__byteE(0),
		__byteF(0),
		__byteG(0),
		__byteH(0),
		__byteI(0),
		__byteJ(0),
		__byteZ(0),
		order(),
		vision()
	{}

	Change(Type type, const Descriptor& subject, const Attacker& unit,
			int8_t figure, bool killed, bool depowered) :
		type(type),
		subject(subject),
		__byteA(0),
		__byteB(0),
		__byteC(0),
		figure(figure),
		killed(killed),
		depowered(depowered),
		__byteG(0),
		__byteH(0),
		__byteI(0),
		__byteJ(0),
		attacker(unit),
		order(),
		vision()
	{}

	Change(Type type, const Descriptor& subject, const Bombarder& unit,
			int8_t figure, bool killed, bool depowered) :
		type(type),
		subject(subject),
		__byteA(0),
		__byteB(0),
		__byteC(0),
		figure(figure),
		killed(killed),
		depowered(depowered),
		__byteG(0),
		__byteH(0),
		__byteI(0),
		__byteJ(0),
		bombarder(unit),
		order(),
		vision()
	{}

	Change(Type type, const Descriptor& subject,
			int8_t figure, bool killed, bool depowered) :
		type(type),
		subject(subject),
		__byteA(0),
		__byteB(0),
		__byteC(0),
		figure(figure),
		killed(killed),
		depowered(depowered),
		__byteG(0),
		__byteH(0),
		__byteI(0),
		__byteJ(0),
		__byteZ(0),
		order(),
		vision()
	{}

	Change(Type type, const Descriptor& subject, const Descriptor& target,
			int8_t figure) :
		type(type),
		subject(subject),
		__byteA(0),
		__byteB(0),
		__byteC(0),
		figure(figure),
		__byteE(0),
		__byteF(0),
		__byteG(0),
		__byteH(0),
		__byteI(0),
		__byteJ(0),
		target(target),
		order(),
		vision()
	{}

	Change(Type type, const Descriptor& subject, int8_t figure) :
		type(type),
		subject(subject),
		__byteA(0),
		__byteB(0),
		__byteC(0),
		figure(figure),
		__byteE(0),
		__byteF(0),
		__byteG(0),
		__byteH(0),
		__byteI(0),
		__byteJ(0),
		__byteZ(0),
		order(),
		vision()
	{}

	Change(Type type, const Player& player) :
		type(type),
		subject(),
		player(player),
		__byteB(0),
		__byteC(0),
		__byteD(0),
		__byteE(0),
		__byteF(0),
		__byteG(0),
		__byteH(0),
		__byteI(0),
		__byteJ(0),
		score(0),
		order(),
		vision()
	{}

	Change(Type type, const Player& player, const Notice& notice) :
		type(type),
		subject(),
		player(player),
		__byteB(0),
		__byteC(0),
		__byteD(0),
		__byteE(0),
		__byteF(0),
		notice(notice),
		__byteH(0),
		__byteI(0),
		__byteJ(0),
		score(0),
		order(),
		vision()
	{}

	Change(Type type, const Descriptor& subject, const Descriptor& target,
			const Notice& notice) :
		type(type),
		subject(subject),
		__byteA(0),
		__byteB(0),
		__byteC(0),
		__byteD(0),
		__byteE(0),
		__byteF(0),
		notice(notice),
		__byteH(0),
		__byteI(0),
		__byteJ(0),
		target(target),
		order(),
		vision()
	{}

	Change& xPower(int8_t x)
	{
		power = x;
		return *this;
	}

	Change& xSnow(bool x)
	{
		snow = x;
		return *this;
	}

	Change& xFrostbite(bool x)
	{
		frostbite = x;
		return *this;
	}

	Change& xFirestorm(bool x)
	{
		firestorm = x;
		return *this;
	}

	Change& xBonedrought(bool x)
	{
		bonedrought = x;
		return *this;
	}

	Change& xDeath(bool x)
	{
		death = x;
		return *this;
	}

	Change& xGas(int8_t x)
	{
		gas = x;
		return *this;
	}

	Change& xRadiation(int8_t x)
	{
		radiation = x;
		return *this;
	}

	Change& xTemperature(int8_t x)
	{
		temperature = x;
		return *this;
	}

	Change& xHumidity(int8_t x)
	{
		humidity = x;
		return *this;
	}

	Change& xChaos(int8_t x)
	{
		chaos = x;
		return *this;
	}

	Change& xLevel(int16_t x)
	{
		level = x;
		return *this;
	}

	Change& xYear(int16_t x)
	{
		year = x;
		return *this;
	}

	Change& xMoney(int16_t x)
	{
		money = x;
		return *this;
	}

	Change& xScore(int16_t x)
	{
		score = x;
		return *this;
	}

	Change& xInitiative(int8_t x)
	{
		initiative = x;
		return *this;
	}

	Change(Type type, Season season) :
		type(type),
		__byteA(0),
		__byteB(0),
		__byteC(0),
		__byteD(0),
		__byteE(0),
		__byteF(0),
		__byteG(0),
		__byteH(0),
		__byteI(0),
		__byteJ(0),
		season(season)
	{}

	Change(Type type, Daytime daytime) :
		type(type),
		__byteA(0),
		__byteB(0),
		__byteC(0),
		__byteD(0),
		__byteE(0),
		__byteF(0),
		__byteG(0),
		__byteH(0),
		__byteI(0),
		__byteJ(0),
		daytime(daytime)
	{}

	Change(Type type, Phase phase) :
		type(type),
		__byteA(0),
		__byteB(0),
		__byteC(0),
		__byteD(0),
		__byteE(0),
		__byteF(0),
		__byteG(0),
		__byteH(0),
		__byteI(0),
		__byteJ(0),
		phase(phase)
	{}

	static Change::Type parseType(const std::string& str);
	static const char* stringify(const Change::Type& type);

	explicit Change(const TypeNamer& namer, const Json::Value& json);

	friend std::ostream& operator<<(std::ostream& os, const Change::Type& type);
	friend std::ostream& operator<<(std::ostream& os, const Change& change);

	static std::vector<Change> parseChanges(const TypeNamer& typenamer,
		const Json::Value& json);
	static std::vector<Change> parseChanges(const TypeNamer& typenamer,
		const std::string& json);

private:
	void parseDeadPoweredKilledDepowered(const Json::Value& json);
};
