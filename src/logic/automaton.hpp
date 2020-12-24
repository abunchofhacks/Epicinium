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

#include <queue>
#include <fstream>

#include "playerinfo.hpp"
#include "roundinfo.hpp"
#include "order.hpp"
#include "bible.hpp"
#include "board.hpp"
#include "initiativesequencer.hpp"
#include "changeset.hpp"

enum class Player : uint8_t;
enum class Season : uint8_t;
enum class Daytime : uint8_t;
enum class Phase : uint8_t;
struct Descriptor;
class Damage;
class Recording;
class RecordingIterator;
class Challenge;


class Automaton : private PlayerInfo, private RoundInfo
{
public:
	Automaton(size_t playercount, const std::string& ruleset);
	Automaton(const std::vector<Player>& players, const std::string& ruleset);
	~Automaton();

	Automaton(const Automaton&) = delete;
	Automaton(Automaton&&) = delete;

	Automaton& operator=(const Automaton&) = delete;
	Automaton& operator=(Automaton&&) = delete;

private:
	Bible _bible;
	Board _board;
	InitiativeSequencer _sequencer;

	std::map<Player, std::vector<Order>> _neworders;
	std::vector<Player> _resignations;
	std::vector<Player> _activeresignations;

	std::queue<Player> _activeplayers;
	std::map<Player, std::vector<Order>> _activeorders; // (married)
	std::map<Player, std::vector<uint32_t>> _activeidentifiers; // (married)
	std::map<Player, size_t> _activeorderindices; // (index into married)
	std::map<uint32_t, Descriptor> _activesubjects;
	std::queue<ChangeSet> _changesets;

	std::vector<std::tuple<uint32_t, Cell, Cell>> _lockdowns;

	std::string _identifier;
	std::ofstream _recording;
	std::unique_ptr<RecordingIterator> _replay;
	bool _oldstyleUnfinished = false;
	bool _reenactFromOrders = false;

	std::shared_ptr<Challenge> _challenge;

	void play();
	void decay();
	void grow();
	void rest();

	void growCities();
	void tax();
	void cap();
	void growPlants();
	void weather();
	void updateMarkers();
	void updateMorale();

	void nextDaytime(ChangeSet& changes);
	void nextSeason(ChangeSet& changes);

	bool assignSubjectId(const Descriptor& subject, uint32_t id);
	uint32_t findSubjectId(const Descriptor& subject);

	bool verifySubject(const Descriptor& subject, const Player& player);

	void process(const Player& player, Order& order);
	void processSleep(const Player& player, Order& order);
	void processMove(const Player& player, Order& order);
	void processGuard(const Player& player, Order& order);
	void processFocus(const Player& player, Order& order);
	void processLockdown(const Player& player, Order& order);
	void processShell(const Player& player, Order& order);
	void processBombard(const Player& player, Order& order);
	void processBomb(const Player& player, Order& order);
	void processCapture(const Player& player, Order& order);
	void processShape(const Player& player, Order& order);
	void processSettle(const Player& player, Order& order);
	void processExpand(const Player& player, Order& order);
	void processUpgrade(const Player& player, Order& order);
	void processCultivate(const Player& player, Order& order);
	void processProduce(const Player& player, Order& order);
	void processHalt(const Player& player, Order& order);

	bool validateMoveSteps(const Order& order);

	void checkAutoCultivate(Cell from, ChangeSet& changes);
	void doAutoCultivate(const Player& player, Cell from, Cell target,
		const TileType& newtype, ChangeSet& changes);

	void checkGatherMorale(Cell from, ChangeSet& changes);

	void finished(const Player& player, Order& order, ChangeSet& changes);
	void discarded(const Player& player, Order& order, ChangeSet& changes);
	void discarded(const Player& player, Order& order, ChangeSet& changes,
		const Notice& notice);
	void discarded(const Player& player, Order& order, ChangeSet& changes,
		const Notice& notice, const Descriptor& target);
	void postponed(const Player& player, const Order& order,
		ChangeSet& changes, const Notice& notice);
	void postponed(const Player& player, const Order& order,
		ChangeSet& changes, const Notice& notice, const Descriptor& target);
	void unfinished(const Player& player, const Descriptor& oldsubject,
		const Order& neworder, ChangeSet& changes);

	void doMove(Cell from, Cell to, const Descriptor& fromdesc,
		const Descriptor& todesc, ChangeSet& changes);
	void doActiveAttack(Cell from, Cell to, const Descriptor& dealer,
		const Descriptor& taker, ChangeSet& changes);
	void doRetaliationAttack(Cell from, Cell to, const Descriptor& dealer,
		const Descriptor& taker, ChangeSet& changes);
	void doFocussedAttack(Cell from, Cell to, const Descriptor& dealer,
		const Descriptor& taker, ChangeSet& changes);
	void checkAttackOfOpportunity(Cell at, const Descriptor& mover, ChangeSet& changes);
	bool checkLockdown(Cell at, const Descriptor& mover, ChangeSet& changes);
	void checkTrample(Cell at, const Descriptor& mover, ChangeSet& changes);
	void doShelling(Cell from, Cell to, const Descriptor& dealer, ChangeSet& changes);
	void doBombardment(Cell from, Cell to, const Descriptor& dealer, ChangeSet& changes);
	void doBombardment1(Cell from, Cell to, const Descriptor& dealer, ChangeSet& changes);
	void doBombing(Cell from, Cell to, const Descriptor& dealer, ChangeSet& changes);
	void doBombing1(Cell from, Cell to, const Descriptor& dealer, ChangeSet& changes);
	void checkAttackDamage(Cell at, const Damage& damage, ChangeSet& changes);
	void checkTrampleDamage(Cell at, const Damage& damage, ChangeSet& changes);
	void checkShellDamage(Cell at, const Damage& damage, ChangeSet& changes);
	void checkBombardDamage(Cell at, const Damage& damage, ChangeSet& changes);
	void checkBombDamage(Cell at, const Damage& damage, ChangeSet& changes);
	bool checkDeaths(Cell at, ChangeSet& changes);
	bool checkUnitDeath(Cell at, const Descriptor& desc, ChangeSet& changes);
	void doUnitDeath(Cell at, const UnitToken& unit, const Descriptor& desc,
		ChangeSet& changes);
	bool checkTileDeath(Cell at, const Descriptor& desc, ChangeSet& changes);
	void doTileDeath(Cell at, const TileToken& tile, const Descriptor& desc,
		ChangeSet& changes);
	void checkUnitDeathLeak(Cell at, const UnitType& unittype, ChangeSet& changes);
	void checkTileDeathLeak(Cell at, const TileType& tiletype, ChangeSet& changes);
	void doLeak(Cell at, int gasleak, int radleak, ChangeSet& changes);
	void checkUnitKillMoraleGain(Cell at, UnitType oldtype, Player oldowner, ChangeSet& changes);
	void checkTileBuildMoraleGain(Cell at, ChangeSet& changes);
	void checkTileCaptureMoraleGain(Cell at, Player oldowner, Player activeplayer, ChangeSet& changes);
	void checkTileDestroyMoraleGain(Cell at, TileType oldtype, Player oldowner, ChangeSet& changes);
	void checkTileBuildCleanse(Cell at, ChangeSet& changes);
	void doDeathEffect(Cell at);
	void doFrostbiteEffect(Cell at);
	void doFirestormEffect(Cell at);
	void doGasEffect(Cell at);
	void doRadiationEffect(Cell at);

	void checkRegularDefeat();
	void checkChallengeDefeat();
	void doDefeat(const std::vector<Player>& defeats, const Notice& notice);

	void issue(const Player& player, const Order& order);
	void prepareForAction();
	void gatherUnfinishedOrders(const Player& player, ChangeSet& changes);

	void record(const ChangeSet& changes);
	void reenact(const ChangeSet& changes);
	void reenactFromChanges(const ChangeSet& changes);
	void reenactFromOrders(const ChangeSet& changes);
	void enactOnPlayerInfo(const Change& change);
	void enactUnfinishedOrder(const Change& change, const Player& player);

	ChangeSet actAsGame();
	ChangeSet actAsReplay();

	uint32_t ThueMorse(uint32_t n, uint32_t b);

public:
	void addPlayer(const Player& player);
	void grantGlobalVision(const Player& player);
	void load(const std::string& mapname, bool shufflePlayers);
	void replay(Recording& recording, bool fromOrders = false);
	void startRecording(Json::Value metadata,
		const char* recnameOrNull = nullptr);
	ChangeSet rejoin(const Player& player);

	void setChallenge(std::shared_ptr<Challenge> challenge);

	void resign(const Player& player);
	void receive(const Player& player, std::vector<Order> orders);
	bool active() const;
	bool replaying() const;
	ChangeSet awake();
	ChangeSet prepare();
	ChangeSet act();
	ChangeSet hibernate();

	int round() const { return _round; }

	int globalWarming();
	int globalScore();
	int score(const Player& player) { return _score[player]; }
	int award(const Player& player) { return _award[player]; }
	bool defeated(const Player& player) { return _defeated[player]; }
	bool gameover() { return _gameover; }

	const Bible& bible() { return _bible; }
	const std::string& identifier() { return _identifier; }

	/**/ATTRIBUTE_WARN_UNUSED_RESULT/**/
	static ChangeSet setupMarkersOnBoard(const Bible& bible, Board& board);

	/**/ATTRIBUTE_WARN_UNUSED_RESULT/**/
	static ChangeSet adjustMarkersOnBoard(const Bible& bible, Board& board);
};
