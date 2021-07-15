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
#include "automaton.hpp"
#include "source.hpp"

#include "aim.hpp"
#include "damage.hpp"
#include "gastransition.hpp"
#include "radiationtransition.hpp"
#include "powertransition.hpp"
#include "transformtransition.hpp"
#include "weathertransition.hpp"
#include "chaostransition.hpp"
#include "markertransition.hpp"
#include "visiontransition.hpp"
#include "elevationtransition.hpp"
#include "freshwatertransition.hpp"
#include "watertransition.hpp"
#include "randomizer.hpp"
#include "library.hpp"
#include "notice.hpp"
#include "map.hpp"
#include "recording.hpp"
#include "recordingiterator.hpp"
#include "challenge.hpp"
#include "typenamer.hpp"
#include "system.hpp"


Automaton::Automaton(size_t playercount, const std::string& rulesetname) :
	PlayerInfo(playercount),
	RoundInfo(),
	_bible(Library::getBible(rulesetname)),
	_board(_bible),
	_sequencer(playercount)
{
	DEBUG_ASSERT(InitiativeSequencer::MAX_NUM >= PLAYER_MAX);
}

Automaton::Automaton(const std::vector<Player>& players,
		const std::string& rulesetname) :
	Automaton(players.size(), rulesetname)
{
	for (const Player& player : players)
	{
		if (player != Player::NONE)
		{
			addPlayer(player);
		}
	}
}

void Automaton::addPlayer(const Player& player)
{
	DEBUG_ASSERT(_players.size() < _playercount);
	_players.emplace_back(player);
}

void Automaton::startRecording(Json::Value metadata,
	const char* recnameOrNull)
{
	Recording recording;
	if (recnameOrNull != nullptr)
	{
		const char* recname = recnameOrNull;
		recording.start(recname);
	}
	else
	{
		recording.start();
	}
	_identifier = recording.name();
	_recording = System::ofstream(recording.filename(),
		std::ofstream::out | std::ofstream::app);
	if (!_recording)
	{
		LOGW << "Failed to open " << recording.filename() << " for writing";
	}

	recording.addMetadata(metadata);
	metadata["automaton-version"] = Version::current().toString();
	metadata["ruleset"] = _bible.name();
	_recording << Json::FastWriter().write(metadata);
	// Newline already added by FastWriter.
	_recording << TypeEncoder(&_bible);
}

Automaton::~Automaton()
{
	if (_recording.is_open())
	{
		_recording.close();
		Recording(_identifier).end();
	}
}

ChangeSet Automaton::setupMarkersOnBoard(const Bible& bible, Board& board)
{
	ChangeSet cset;
	if (bible.counterBasedWeather())
	{
		WaterTransition(bible, board, cset).execute();
	}
	else
	{
		ElevationTransition(bible, board, cset).execute();
		FreshwaterTransition(bible, board, cset).execute();
	}
	MarkerTransition(bible, board, cset).execute();
	return cset;
}

ChangeSet Automaton::adjustMarkersOnBoard(const Bible& bible, Board& board)
{
	ChangeSet cset;
	MarkerTransition(bible, board, cset).execute();
	return cset;
}

void Automaton::load(const std::string& mapname, bool shufflePlayers)
{
	_board.load(mapname);
	{
		ChangeSet dummy = setupMarkersOnBoard(_bible, _board);
	}

	if (shufflePlayers)
	{
		// Shuffle the player list so that the player at the top of the lobby
		// is not necessarily the player to move first, which is _player[0].
		std::random_shuffle(_players.begin(), _players.end());

		// Assign the players to random starting positions on the board,
		// independent of player order. The board shuffles again internally.
		_board.assignPlayers(_players);
	}
	else _board.setPlayers(_players);

	ChangeSet initialchanges;

	initialchanges.push(Change(Change::Type::YEAR).xYear(_year), Vision::all(_players));
	initialchanges.push(Change(Change::Type::SEASON,  _season),  Vision::all(_players));
	initialchanges.push(Change(Change::Type::DAYTIME, _daytime), Vision::all(_players));
	initialchanges.push(Change(Change::Type::PHASE,   _phase),   Vision::all(_players));

	// Declare the bottom right corner of the map.
	initialchanges.push(Change(Change::Type::CORNER, Descriptor::cell(
			Position(_board.rows() - 1, _board.cols() - 1))),
		Vision::all(_players));

	// Reveal the entire map to all players so the starting positions are known.
	// Units are only ever revealed if they were in the players vision.
	InitialVisionTransition(_bible, _board, *this, initialchanges).execute();

	// Declare that the entire map has been announced.
	initialchanges.push(Change(Change::Type::BORDER), Vision::all(_players));

	for (const Player& player : _players)
	{
		_money[player] += _bible.startingMoney();
		Change moneychange(Change::Type::FUNDS, player);
		moneychange.xMoney(_bible.startingMoney());
		initialchanges.push(moneychange, Vision::only(player));
	}

	_changesets.push(initialchanges);

	for (const Player& player : _players)
	{
		// Count the number of cities this player controls.
		size_t cities = 0;
		for (Cell index : _board)
		{
			const TileToken& tile = _board.tile(index);
			if (_bible.tileBinding(tile.type) && tile.owner == player)
			{
				cities++;
			}
		}

		// Players that start without cities are not citybound.
		_citybound[player] = (cities > 0);
	}

	ChangeSet announcementchanges;
	_phase = Phase::RESTING;
	announcementchanges.push(Change(Change::Type::PHASE, _phase),
		Vision::all(_players));
	_changesets.push(announcementchanges);
}

void Automaton::replay(Recording& recording, bool fromOrders)
{
	// Open recording.
	_replay.reset(new RecordingIterator{_bible, recording});
	if (!(_replay && *_replay))
	{
		LOGF << "Failed to open " << recording.filename();
		throw std::runtime_error("Failed to open " + recording.filename());
	}
	_identifier = recording.name();
	_players = recording.getPlayers();
	_oldstyleUnfinished = false;
	_reenactFromOrders = fromOrders;

	// Read version information.
	Version myversion = Version::current();
	Version automatonversion;

	Json::Value json = recording.metadata();
	if (json.isObject())
	{
		automatonversion = Version(json["automaton-version"]);
	}
	else
	{
		// Prehistoric recording.
		automatonversion = Version::prehistoric();
		LOGW << "Missing players or bots while reading " << _identifier;
	}

	// Check compatibility.
	if (automatonversion.major > myversion.major)
	{
		// TODO replace with friendly error message in the UI
		LOGE << "Version mismatch while reading " << _identifier;
		throw std::runtime_error("Version mismatch while reading " + _identifier);
	}

	// Do we need to activate backwards compatibility modes?
	if (automatonversion < Version(0, 32, 0, 0))
	{
		if (_reenactFromOrders)
		{
			LOGE << "Reenacting from orders not supported for " << _identifier;
			DEBUG_ASSERT(false);
			_reenactFromOrders = false;
		}

		_oldstyleUnfinished = true;
	}

	// Check compatibility.
	Version bibleversion = _bible.version();
	if (bibleversion.major > myversion.major
			|| (bibleversion.minor == myversion.minor
				&& bibleversion.minor > myversion.minor))
	{
		// TODO replace with friendly error message in the UI
		LOGE << "Version mismatch while reading " << _identifier;
		throw std::runtime_error("Version mismatch while reading " + _identifier);
	}
}

void Automaton::enactOnPlayerInfo(const Change& change)
{
	switch (change.type)
	{
		case Change::Type::FUNDS:
		case Change::Type::INCOME:
		case Change::Type::EXPENDITURE:
		{
			_money[change.player] += change.money;
		}
		break;

		case Change::Type::INITIATIVE:
		{
			_initiative[change.player] = change.initiative;
		}
		break;

		case Change::Type::YEAR:
		{
			_year = change.year;
		}
		break;

		case Change::Type::SEASON:
		{
			_season = change.season;
		}
		break;

		case Change::Type::DAYTIME:
		{
			_daytime = change.daytime;
		}
		break;

		case Change::Type::PHASE:
		{
			if (change.phase == Phase::ACTION)
			{
				for (auto& kv : _activeorders) kv.second.clear();
				for (auto& kv : _activeidentifiers) kv.second.clear();
				for (auto& kv : _neworders) kv.second.clear();
			}
			_phase = change.phase;
		}
		break;

		case Change::Type::DEFEAT:
		{
			_defeated[change.player] = true;
			_score[change.player] = change.score;
		}
		break;

		case Change::Type::VICTORY:
		{
			_score[change.player] = change.score;
		}
		break;

		case Change::Type::GAMEOVER:
		{
			_gameover = true;
		}
		break;

		case Change::Type::AWARD:
		{
			_award[change.player] = change.level;
		}
		break;

		case Change::Type::POSTPONED:
		case Change::Type::UNFINISHED:
		{
			if (change.player == Player::NONE)
			{
				if (!_oldstyleUnfinished)
				{
					LOGW << "Ignoring change of type "
						<< Change::stringify(change.type)
						<< " without player";
				}
				DEBUG_ASSERT(_oldstyleUnfinished);
			}
			else if (_phase == Phase::ACTION)
			{
				enactUnfinishedOrder(change, change.player);
			}
		}
		break;

		default:
		break;
	}
}

void Automaton::enactUnfinishedOrder(const Change&, const Player&)
{
	// TODO implement for resume from replay / restore functionality (#965)
}

void Automaton::reenact(const ChangeSet& changeset)
{
	if (_reenactFromOrders)
	{
		return reenactFromOrders(changeset);
	}
	else
	{
		return reenactFromChanges(changeset);
	}
}

void Automaton::reenactFromChanges(const ChangeSet& changeset)
{
	for (Change& change : changeset.get(Player::OBSERVER))
	{
		enactOnPlayerInfo(change);

		_board.enact(change);
	}

	if (_oldstyleUnfinished && _phase == Phase::ACTION)
	{
		for (Player& player : _players)
		{
			for (Change& change : changeset.get(player))
			{
				if (change.type == Change::Type::POSTPONED
					|| change.type == Change::Type::UNFINISHED)
				{
					enactUnfinishedOrder(change, player);
				}
			}
		}
	}
}

void Automaton::reenactFromOrders(const ChangeSet& changeset)
{
	LOGV << "Reenacting " << TypeEncoder(&_bible) << changeset;

	bool triggerSetup = false;
	bool triggerRest = false;
	bool triggerHibernate = false;
	bool triggerAwake = false;
	bool triggerPrepare = false;
	for (Change& change : changeset.get(Player::OBSERVER))
	{
		switch (change.type)
		{
			case Change::Type::PHASE:
			{
				LOGV << "Triggered by " << TypeEncoder(&_bible) << change;
				switch (change.phase)
				{
					case Phase::GROWTH:
					{
						if (_round == 0) triggerSetup = true;
					}
					break;

					case Phase::RESTING:
					{
						triggerRest = true;
					}
					break;

					case Phase::PLANNING:
					{
						triggerHibernate = true;
					}
					break;

					case Phase::STAGING:
					{
						triggerAwake = true;
					}
					break;

					case Phase::ACTION:
					{
						triggerPrepare = true;
					}
					break;

					case Phase::DECAY:
					break;
				}
			}
			break;

			case Change::Type::ORDERED:
			{
				Player player = change.player;
				Order neworder = change.order;
				LOGV << "Reenacting order " << TypeEncoder(&_bible) << neworder
					<< " for player " << player;

				if (!triggerPrepare)
				{
					LOGW << "Reenacting order but not preparing for action";
				}
				DEBUG_ASSERT(triggerPrepare);

				issue(player, neworder);
			}
			break;

			default:
			break;
		}
	}

	if (triggerSetup)
	{
		return reenactFromChanges(changeset);
	}
	else if (triggerRest)
	{
		return;
	}

	ChangeSet results;
	if (triggerHibernate)
	{
		results = hibernate();
	}
	else if (triggerAwake)
	{
		results = awake();
	}
	else if (triggerPrepare)
	{
		results = prepare();
	}
	else
	{
		results = actAsGame();
	}
	LOGD << "Reenacted " << TypeEncoder(&_bible) << results;

	if (!ChangeSet::equal(_bible, results, changeset))
	{
		LOGW << "Changesets differ";
		DEBUG_ASSERT(false);
	}
}

ChangeSet Automaton::rejoin(const Player& perspective)
{
	ChangeSet cset;

	// Declare the current season, daytime and phase.
	cset.push(Change(Change::Type::YEAR).xYear(_year), Vision::all(_players));
	cset.push(Change(Change::Type::SEASON,  _season),  Vision::all(_players));
	cset.push(Change(Change::Type::DAYTIME, _daytime), Vision::all(_players));
	cset.push(Change(Change::Type::PHASE,   _phase),   Vision::all(_players));

	{
		// Simulate the board from the perspective of the player/observer.
		Board simulation(_bible);

		// Open the recording and reenact the changes that that player saw.
		for (RecordingIterator iter{_bible, Recording{_identifier}}; iter; ++iter)
		{
			if (_replay && *_replay
				&& iter.linenumber() == _replay->linenumber())
			{
				break;
			}

			for (const Change& change : (*iter).get(perspective))
			{
				simulation.enact(change);
			}
		}

		// Declare the bottom right corner of the map.
		cset.push(Change(Change::Type::CORNER, Descriptor::cell(
				Position(simulation.rows() - 1, simulation.cols() - 1))),
			Vision::all(_players));

		// Reveal the entire map to the rejoining player.
		RejoinVisionTransition(_bible, simulation, *this, cset).execute();

		// Declare that the entire map has been announced.
		cset.push(Change(Change::Type::BORDER), Vision::all(_players));
	}

	// Reveal the last chaos report.
	if (_bible.quantitativeChaos() && _board.mass() > 0
		&& _bible.chaosThreshold() > 0)
	{
		int total = 0;
		for (Cell index : _board)
		{
			total += _board.chaos(index);
		}
		int level = total / (_board.mass() * _bible.chaosThreshold());

		Change report(Change::Type::CHAOSREPORT);
		report.xLevel(level);
		cset.push(report, Vision::all(_board.players()));
	}

	// Reveal the current status of resources.
	for (const Player& player : _players)
	{
		Change moneychange(Change::Type::FUNDS, player);
		moneychange.xMoney(_money[player]);
		cset.push(moneychange, Vision::only(player));
	}

	// Reveal the current orderlists.
	for (const Player& player : _players)
	{
		for (const Order& order : _orderlists[player])
		{
			// We don't remember the correct oldsubject, but we are sending this
			// to someone who is rejoining, so it does not really matter.
			Descriptor oldsubject = order.subject;
			Change change(Change::Type::UNFINISHED, oldsubject, player, order);
			cset.push(change, Vision::only(player));
		}
	}

	// Reveal the current initiative (in ascending order).
	{
		std::vector<std::pair<int, Player>> ips;
		for (const Player& player : _players)
		{
			int initiative = _initiative[player];
			if (initiative > 0)
			{
				ips.emplace_back(initiative, player);
			}
		}
		std::sort(ips.begin(), ips.end());
		for (const auto& ip : ips)
		{
			int initiative = ip.first;
			const Player& player = ip.second;
			Change change(Change::Type::INITIATIVE, player);
			change.xInitiative(initiative);
			cset.push(change, Vision::all(_players));
		}
	}

	// Reveal the game results, if any.
	for (const Player& player : _players)
	{
		if (_defeated[player])
		{
			Change change(Change::Type::DEFEAT, player);
			change.xScore(_score[player]);
			cset.push(change, Vision::all(_players));
		}
		else if (_gameover)
		{
			Change change(Change::Type::VICTORY, player);
			change.xScore(_score[player]);
			cset.push(change, Vision::all(_players));
		}

		if (_award[player])
		{
			Change change(Change::Type::AWARD, player);
			change.xLevel(_award[player]);
			cset.push(change, Vision::only(player));
		}
	}
	if (_gameover)
	{
		Change change(Change::Type::GAMEOVER);
		change.xScore(globalScore());
		cset.push(change, Vision::all(_players));
	}

	return cset;
}

void Automaton::setChallenge(std::shared_ptr<Challenge> challenge)
{
	_challenge = challenge;
}

void Automaton::resign(const Player& player)
{
	if (_defeated[player])
	{
		LOGD << "player " << player << " is already defeated";
		return;
	}
	else if (std::find(_resignations.begin(), _resignations.end(), player)
		!= _resignations.end())
	{
		LOGW << "player " << player << " has already resigned";
		return;
	}

	_resignations.emplace_back(player);
}

void Automaton::receive(const Player& player, std::vector<Order> neworders)
{
	if (_defeated[player])
	{
		for (auto& neworder : neworders)
		{
			LOGW << "player " << player << " is already defeated,"
				" discarding " << TypeEncoder(&_bible) << neworder;
		}
		return;
	}

	size_t count = 0;
	for (auto& neworder : neworders)
	{
		if (count >= _bible.newOrderLimit()
			|| _neworders[player].size() >= _bible.newOrderLimit())
		{
			LOGW << "player " << player << " cannot issue more orders,"
				" discarding " << TypeEncoder(&_bible) << neworder;
			continue;
		}

		issue(player, neworder);

		// Keep a separate count because we do not want players submitting
		// multiple new orders with the same subject.
		count++;
	}
}

static void eraseBySubject(std::vector<Order>& list, const Descriptor& subject)
{
	for (auto iter = list.begin(); iter < list.end(); ++iter)
	{
		const Order& order = *iter;
		if (order.subject == subject)
		{
			list.erase(iter);
			return;
		}
	}
}

void Automaton::issue(const Player& player, const Order& neworder)
{
	LOGV << "player " << player << " issues order "
		<< TypeEncoder(&_bible) << neworder;

	if (neworder.type != Order::Type::NONE)
	{
		// Verify that the subject is owned by the player, because we will
		// assign ids to all of the subjects later, and we do not want a player
		// to be able to invalidate their opponent's subjects by giving bogus
		// orders. More thorough checks are done when the order is processed.
		if (!verifySubject(neworder.subject, player))
		{
			return;
		}

		// Each subject can only be given one order, so we immediately silently
		// discard any old order. If the player tries to give multiple orders
		// to the same subject, all but the last are silently discarded as well.
		// However, the player may give multiple NONE ("SLEEP") orders.
		eraseBySubject(_neworders[player], neworder.subject);
		eraseBySubject(_orderlists[player], neworder.subject);

	}

	_neworders[player].push_back(neworder);
	_orderlists[player].push_back(neworder);
}

void Automaton::gatherUnfinishedOrders(const Player& player, ChangeSet& cset)
{
	_orderlists[player].reserve(_activeorders[player].size());
	for (size_t j = 0; j < _activeorders[player].size(); j++)
	{
		Order order = std::move(_activeorders[player][j]);
		uint32_t id = _activeidentifiers[player][j];
		// Make sure the subject of the order was not killed or captured after
		// the order was declared unfinished; if it was, the order is no longer
		// valid and must be discarded.
		if (!order.finished() && findSubjectId(order.subject) == id)
		{
			Descriptor oldsubject = _activesubjects[id];
			Change change(Change::Type::UNFINISHED, oldsubject, player, order);
			cset.push(change, Vision::only(player));

			LOGV << "keeping old order " << TypeEncoder(&_bible) << order;
			_orderlists[player].emplace_back(std::move(order));
		}
	}
	_activeorders[player].clear();
	_activeidentifiers[player].clear();
}

void Automaton::grantGlobalVision(const Player& player)
{
	_visionaries.emplace_back(player);
}

ChangeSet Automaton::awake()
{
	ChangeSet announcement;
	_phase = Phase::STAGING;
	announcement.push(Change(Change::Type::PHASE, _phase), Vision::all(_players));
	record(announcement);
	return announcement;
}

void Automaton::prepareForAction()
{
	for (Cell index : _board)
	{
		_board.tile(index).resetId();
		_board.ground(index).resetId();
		_board.air(index).resetId();
		_board.bypass(index).resetId();
	}

	_activeresignations.swap(_resignations);

	_activesubjects.clear();
	_lockdowns.clear();
	uint32_t assigned = 0;

	for (size_t i = 0; i < _players.size(); i++)
	{
		Player player = _players[_sequencer.getPlayerIndex(_round, i)];

		// Players without orders are inactive.
		if (_orderlists[player].empty()) continue;

		// Queue all of this players orders.
		_activeorders[player] = std::move(_orderlists[player]);
		_orderlists[player].clear();

		// We will assign an identifier to each active order.
		_activeidentifiers[player].resize(_activeorders[player].size(), 0);

		// Assign ids to the subjects of the orders, so that if the subject is
		// killed or captured before or after (as opposed to during) the order's
		// execution, the order can be discarded as it would no longer be valid.
		for (size_t j = 0; j < _activeorders[player].size(); j++)
		{
			const Order& order = _activeorders[player][j];
			if (order.type != Order::Type::NONE)
			{
				// Start with id 1 and work up from there.
				assigned++;

				// If the subject is invalid, the id is skipped.
				if (assignSubjectId(order.subject, assigned))
				{
					_activesubjects[assigned] = order.subject;
					_activeidentifiers[player][j] = assigned;
				}
			}
		}

		// Because 0 is not a valid id, it should have an empty descriptor.
		_activesubjects[0] = Descriptor();

		// This player is an active player.
		_activeplayers.push(player);

		// Start with this player's first queued order.
		_activeorderindices[player] = 0;
	}
	_round++;
}

ChangeSet Automaton::prepare()
{
	prepareForAction();

	ChangeSet announcement;
	_phase = Phase::ACTION;
	announcement.push(Change(Change::Type::PHASE, _phase), Vision::all(_players));

	// Tell each player their new-orderlist.
	for (const Player& player : _players)
	{
		for (const Order& order : _neworders[player])
		{
			Change change(Change::Type::ORDERED, order.subject, player, order);
			announcement.push(change, Vision::only(player));
		}
		_neworders[player].clear();
	}

	record(announcement);
	return announcement;
}

ChangeSet Automaton::hibernate()
{
	ChangeSet announcement;

	if (_bible.publicInitiative())
	{
		int initiative = 1;
		for (size_t i = 0; i < _players.size(); i++)
		{
			Player player = _players[_sequencer.getPlayerIndex(_round, i)];
			_initiative[player] = initiative;
			Change change(Change::Type::INITIATIVE, player);
			change.xInitiative(initiative);
			announcement.push(change, Vision::all(_players));
			initiative++;
		}
	}

	_phase = Phase::PLANNING;
	announcement.push(Change(Change::Type::PHASE, _phase), Vision::all(_players));
	record(announcement);
	return announcement;
}

bool Automaton::active() const
{
	switch (_phase)
	{
		case Phase::GROWTH: return true;
		case Phase::RESTING: return (!replaying() && !_changesets.empty());
		case Phase::PLANNING: return replaying();
		case Phase::STAGING: return replaying();
		case Phase::ACTION: return true;
		case Phase::DECAY: return true;
	}
	return false;
}

bool Automaton::replaying() const
{
	return (_replay != nullptr);
}

ChangeSet Automaton::act()
{
	if (_replay)
	{
		return actAsReplay();
	}
	else
	{
		return actAsGame();
	}
}

ChangeSet Automaton::actAsGame()
{
	if (!_changesets.empty())
	{
		ChangeSet changeset(_changesets.front());
		_changesets.pop();
		record(changeset);
		return changeset;
	}

	if (_gameover)
	{
		rest();
	}
	else if (_phase == Phase::ACTION)
	{
		if (!_activeresignations.empty())
		{
			doDefeat(_activeresignations, Notice::RESIGNED);
			_activeresignations.clear();
		}
		else if (!_activeplayers.empty())
		{
			play();
		}
		else
		{
			decay();
			checkRegularDefeat();
			checkChallengeDefeat();
		}
	}
	else if (_phase == Phase::DECAY)
	{
		grow();
		checkChallengeDefeat();
	}
	else if (_phase == Phase::GROWTH)
	{
		rest();
	}

	if (!_changesets.empty())
	{
		ChangeSet changeset(_changesets.front());
		_changesets.pop();
		record(changeset);
		return changeset;
	}
	else
	{
		LOGW << "Acting without a changeset.";
		ChangeSet changeset;
		record(changeset);
		return changeset;
	}
}

ChangeSet Automaton::actAsReplay()
{
	if (*_replay)
	{
		ChangeSet cset = **_replay;
		++(*_replay);
		reenact(cset);
		return cset;
	}
	else
	{
		_replay.reset(nullptr);
		ChangeSet cset;
		if (!_gameover)
		{
			Change change(Change::Type::GAMEOVER);
			change.xScore(globalScore());
			cset.push(change, Vision::all(_players));
		}
		_gameover = true;
		return cset;
	}
}

void Automaton::play()
{
	Player player = _activeplayers.front();
	_activeplayers.pop();

	{
		size_t index = _activeorderindices[player];
		Order& order = _activeorders[player][index];

		process(player, order);
	}

	_activeorderindices[player]++;
	if (_activeorderindices[player] < _activeorders[player].size())
	{
		_activeplayers.push(player);
	}
}

bool Automaton::assignSubjectId(const Descriptor& subject, uint32_t id)
{
	Cell index = _board.cell(subject.position);
	switch (subject.type)
	{
		case Descriptor::Type::TILE:
		{
			TileTokenWithId& tile = _board.tile(index);
			if (!tile)
			{
				LOGV << "could not find subject " << subject;
				return 0;
			}
			tile.assignId(id);
			LOGV << "assigning identifier " << id << " to " << subject;
			return tile.id();
		}
		break;

		case Descriptor::Type::GROUND:
		case Descriptor::Type::AIR:
		{
			UnitTokenWithId& unit = _board.unit(index, subject.type);
			if (!unit)
			{
				LOGV << "could not find subject " << subject;
				return 0;
			}
			unit.assignId(id);
			LOGV << "assigning identifier " << id << " to " << subject;
			return unit.id();
		}
		break;

		default:
		{
			LOGW << "could not find broken subject " << subject;
			return 0;
		}
		break;
	}
}

uint32_t Automaton::findSubjectId(const Descriptor& subject)
{
	Cell index = _board.cell(subject.position);
	switch (subject.type)
	{
		case Descriptor::Type::TILE:
		{
			const TileTokenWithId& tile = _board.tile(index);
			if (!tile)
			{
				LOGV << "could not find subject " << subject;
				return 0;
			}
			LOGV << "matching identifier " << tile.id() << " with " << subject;
			return tile.id();
		}
		break;

		case Descriptor::Type::GROUND:
		case Descriptor::Type::AIR:
		{
			const UnitTokenWithId& unit = _board.unit(index, subject.type);
			if (!unit)
			{
				LOGV << "could not find subject " << subject;
				return 0;
			}
			LOGV << "matching identifier " << unit.id() << " with " << subject;
			return unit.id();
		}
		break;

		default:
		{
			LOGW << "could not find broken subject " << subject;
			return 0;
		}
		break;
	}
}

bool Automaton::verifySubject(const Descriptor& subject, const Player& player)
{
	Cell index = _board.cell(subject.position);
	switch (subject.type)
	{
		case Descriptor::Type::TILE:
		{
			const TileTokenWithId& tile = _board.tile(index);
			if (tile && tile.owner == player)
			{
				LOGV << "verified " << subject << " for " << player;
				return true;
			}
			else
			{
				LOGV << "could not verify " << subject << " for " << player;
				return false;
			}
		}
		break;

		case Descriptor::Type::GROUND:
		case Descriptor::Type::AIR:
		{
			const UnitTokenWithId& unit = _board.unit(index, subject.type);
			if (unit && unit.owner == player)
			{
				LOGV << "verified " << subject << " for " << player;
				return true;
			}
			else
			{
				LOGV << "could not verify " << subject << " for " << player;
				return false;
			}
		}
		break;

		default:
		{
			LOGW << "could not verify broken " << subject << " for " << player;
			return false;
		}
		break;
	}
}

void Automaton::process(const Player& player, Order& order)
{
	LOGV << "processing " << TypeEncoder(&_bible) << order << " from " << player;
	switch (order.type)
	{
		case Order::Type::NONE:    return processSleep(player, order);
		case Order::Type::MOVE:    return processMove(player, order);
		case Order::Type::GUARD:   return processGuard(player, order);
		case Order::Type::FOCUS:   return processFocus(player, order);
		case Order::Type::LOCKDOWN: return processLockdown(player, order);
		case Order::Type::SHELL:   return processShell(player, order);
		case Order::Type::BOMBARD: return processBombard(player, order);
		case Order::Type::BOMB:    return processBomb(player, order);
		case Order::Type::CAPTURE: return processCapture(player, order);
		case Order::Type::SHAPE:   return processShape(player, order);
		case Order::Type::SETTLE:  return processSettle(player, order);
		case Order::Type::EXPAND:  return processExpand(player, order);
		case Order::Type::UPGRADE: return processUpgrade(player, order);
		case Order::Type::CULTIVATE: return processCultivate(player, order);
		case Order::Type::PRODUCE: return processProduce(player, order);
		case Order::Type::HALT:    return processHalt(player, order);
	}
}

void Automaton::processSleep(const Player& player, Order& order)
{
	// Show a sleep change.
	ChangeSet cset;
	cset.push(Change(Change::Type::SLEEPING, player), Vision::only(player));
	_changesets.push(cset);

	// This order is done.
	order = Order();
}

bool Automaton::validateMoveSteps(const Order& order)
{
	std::vector<Position> visits;
	visits.reserve(order.moves.size() + 1);
	visits.emplace_back(order.subject.position);

	Cell at = _board.cell(order.subject.position);
	for (Move move : order.moves)
	{
		at = at + move;
		visits.emplace_back(at.pos());
	}

	// The list of moves must end at the target space.
	if (visits.back() != order.target.position) return false;

	// Furthermore, two positions visited must be adjacent iff they are
	// connected by a move.
	for (size_t j = 0; j + 1 < visits.size(); j++)
	{
		// Check i=0,...,j-1 with j+1 for non-adjacency.
		for (size_t i = 0; i < j; i++)
		{
			if (Aim(visits[i], visits[j + 1]).length() == 1) return false;
		}

		// Check j with j+1 for adjacency.
		if (Aim(visits[j], visits[j + 1]).length() != 1) return false;
	}

	return true;
}

void Automaton::processMove(const Player& player, Order& order)
{
	// Declare we are processing this order.
	ChangeSet cset;
	cset.push(Change(Change::Type::ACTING, order.subject), Vision::only(player));

	// Find the moving unit based on its description.
	Descriptor::Type desctype = order.subject.type;
	Cell current = _board.cell(order.subject.position);
	const UnitTokenWithId& movingunit = _board.unit(current, desctype);

	// Is the unit the subject of the order?
	if (_activesubjects[movingunit.id()] != order.subject)
	{
		LOGV << "subject id does not match, order discarded";
		return discarded(player, order, cset);
	}

	// Does the unit exist?
	if (!movingunit)
	{
		LOGE << "verified subject does not exist, order discarded";
		DEBUG_ASSERT(false && "subject verification is not watertight");
		return discarded(player, order, cset);
	}

	// Is the unit owned by the player?
	if (movingunit.owner != player)
	{
		LOGE << "verified subject not controlled by player, order discarded";
		DEBUG_ASSERT(false && "subject verification is not watertight");
		return discarded(player, order, cset);
	}

	// Can the unit move?
	if (!_bible.unitCanMove(movingunit.type))
	{
		LOGD << "subject cannot move, order discarded"
			": " << TypeEncoder(&_bible) << order << " from " << player;
		return discarded(player, order, cset, Notice::ORDERINVALID);
	}

	// Is the list of moves non-empty?
	if (order.moves.empty())
	{
		LOGD << "move order without moves, order discarded"
			": " << TypeEncoder(&_bible) << order << " from " << player;
		return discarded(player, order, cset, Notice::ORDERINVALID);
	}

	// Is the list of moves valid?
	if (!validateMoveSteps(order))
	{
		LOGD << "move order with invalid moves, order discarded"
			": " << TypeEncoder(&_bible) << order << " from " << player;
		return discarded(player, order, cset, Notice::ORDERINVALID);
	}

	// Determine how many movesteps we might be able to take.
	int basespeed = _bible.unitSpeed(movingunit.type);
	int speed = basespeed;
	int slow = 0;
	int maxmoves = order.moves.size();

	// Air units crash into enemy air units. Gliders also crash into friendly air units.
	bool air = _bible.unitAir(movingunit.type);
	bool gliding = false;

	// When a unit has Cold Feet, they cannot attack until after they have
	// moved. We check whether they currently have Cold Feet.
	bool coldfeet = hasColdFeet(current);

	// Before actually moving, we will determine how far we walk and if we will attack an enemy.
	int moves = 0;
	int attack = 0;
	int bypassattack = 0;
	int blocked = 0;

	// We will need to remember what steps we will take when determining this.
	std::vector<Descriptor> steps;
	steps.push_back(order.subject);
	Descriptor attackstep;

	// Can the unit move?
	if (basespeed <= 0)
	{
		LOGD << "subject has no speed, order discarded"
			": " << TypeEncoder(&_bible) << order << " from " << player;
		return discarded(player, order, cset, Notice::ORDERINVALID);
	}

	// Try to move until we hit an obstacle.
	for (int i = 0; i < speed && i < maxmoves; i++)
	{
		// Check if there is anything slowing us down. (1)
		// NOTE: Changes to this code should be copied to Commander::visualizeMoveOrder().
		if (!air && _board.snow(current)
			&& _bible.snowSlowAmount() > 0
			&& slow < _bible.snowSlowMaximum())
		{
			// Increase slow.
			slow = std::min(slow + _bible.snowSlowAmount(),
				(int) _bible.snowSlowMaximum());

			// Reduce movement speed.
			speed = std::min(speed, std::max(basespeed - slow, 1));

			// Check if we have enough left to move.
			if (i >= speed) break;
		}

		// Check if there is anything slowing us down. (2)
		// NOTE: Changes to this code should be copied to Commander::visualizeMoveOrder().
		TileType fromtype = _board.tile(current).type;
		if (!air && _bible.tileSlowAmount(fromtype) > 0
			&& slow < _bible.tileSlowMaximum(fromtype))
		{
			// Increase slow.
			slow = std::min(slow + _bible.tileSlowAmount(fromtype),
				(int) _bible.tileSlowMaximum(fromtype));

			// Reduce movement speed.
			speed = std::min(speed, std::max(basespeed - slow, 1));

			// Check if we have enough left to move.
			if (i >= speed) break;
		}

		// Update the position.
		current = current + order.moves[i];

		// Are we out of bounds?
		const TileToken& othertile = _board.tile(current);
		if (!othertile)
		{
			LOGW << "tile is null, order discarded"
				": " << TypeEncoder(&_bible) << order << " from " << player;
			return discarded(player, order, cset, Notice::ORDERINVALID);
		}

		// Is the tile accessible?
		if (!_bible.tileAccessible(othertile.type))
		{
			LOGV << "tile inaccessible, order discarded";
			return discarded(player, order, cset, Notice::INACCESSIBLE);
		}

		// Is the tile accessible by ground? If not, is the unit an air unit?
		if (!air && !_bible.tileWalkable(othertile.type))
		{
			LOGV << "tile not walkable, order discarded";
			return discarded(player, order, cset, Notice::UNWALKABLE);
		}

		// Is another unit already occupying the space we want to move to?
		// Note that ground units do not collide with air units or vice versa.
		const UnitToken& otherunit = _board.unit(current, desctype);

		// If the space is free, we can proceed as usual.
		if (!otherunit)
		{
			steps.push_back(Descriptor(desctype, current.pos()));
			moves = i + 1;
		}
		// If the unit is friendly, we might be able to bypass them...
		else if (!air && otherunit.owner == movingunit.owner)
		{
			// Remember where we were blocked.
			blocked = i + 1;

			// This move is only possible if we can end up on an unoccupied
			// space later this turn, so we can not yet update moves.
			// We will continue walking to see what happens.
			steps.push_back(Descriptor(Descriptor::Type::BYPASS, current.pos()));
		}
		// ... unless we are an air unit, in which case we must stop.
		else if (air && !gliding && otherunit.owner == movingunit.owner)
		{
			// Remember where we were blocked.
			blocked = i + 1;

			// We cannot move here.
			break;
		}
		// If the unit is not friendly, we will stop moving and try to attack it...
		else if (!air) // && otherunit.owner != movingunit.owner
		{
			// Remember where we were blocked.
			blocked = i + 1;

			// Only some units can attack.
			if (!_bible.unitCanAttack(movingunit.type)) break;
			if (coldfeet) break;

			// We can only attack if we can stop, or if we do a "bypass attack";
			// in the latter case we can only have bypassed a single unit.
			if (!(moves == i || moves + 1 == i)) break;

			// We will attack.
			attackstep = Descriptor(desctype, current.pos());
			attack = i;

			// In the case of a bypass attack, we need to move back.
			if (moves != i)
			{
				// Go back to where we came from.
				steps.push_back(steps[moves]);
				bypassattack = attack + 1;
			}

			// We cannot continue moving after attacking.
			break;
		}
		// ... unless we are an air unit, in which case we just crash.
		else // (air && (gliding || otherunit.owner != movingunit.owner))
		{
			// We crash by moving into the bypass slot as an air unit.
			steps.push_back(Descriptor(Descriptor::Type::BYPASS, current.pos()));
			moves = i + 1;
		}

		// Are we forced to stop at an unoccupied trenches tile?
		// NOTE: Changes to this code should be copied to Commander::visualizeMoveOrder().
		if (_bible.tileForceOccupy(_board.tile(current).type)
			&& !air
			&& !_bible.unitMechanical(movingunit.type)
			&& !otherunit)
		{
			// The unit must stop and occupy the trenches.
			break;
		}
	}

	// If we cannot move and cannot attack, we show the player that the unit could not move.
	if (moves <= 0 && attackstep.type == Descriptor::Type::NONE)
	{
		LOGV << "cannot move or attack, order postponed";
		Cell target = _board.cell(order.subject.position);
		for (int i = 0; i < std::max(1, blocked); i++)
		{
			target = target + order.moves[i];
		}
		return postponed(player, order, cset, Notice::DESTINATIONOCCUPIED,
			Descriptor(order.subject.type, target.pos()));
	}

	// Lockdown may force us to stop moving.
	bool lockdown = false;

	// Now we actually perform the moves.
	for (int i = 0; i < std::max(moves, attack); i++)
	{
		Cell from = _board.cell(steps[i].position);
		Cell to   = _board.cell(steps[i + 1].position);
		doMove(from, to, steps[i], steps[i + 1], cset);

		// The moving unit might have been killed during the attack of opportunity.
		if (!_board.unit(to, steps[i + 1].type))
		{
			// We clear the order because the unit does not exist anymore.
			LOGV << "unit killed during attack of opportunity, order discarded";
			return discarded(player, order, cset, Notice::SUBJECTKILLED);
		}

		// The unit moving might trigger lockdown.
		if (checkLockdown(to, steps[i + 1], cset))
		{
			// The moving unit might have been killed during the attack of opportunity.
			if (!_board.unit(to, steps[i + 1].type))
			{
				// We clear the order because the unit does not exist anymore.
				LOGV << "unit killed during attack of opportunity, order discarded";
				return discarded(player, order, cset, Notice::SUBJECTKILLED);
			}

			// Lockdown has occurred, so we must stop moving.
			lockdown = true;
			break;
		}
	}

	// We might want to attack a target.
	if (attackstep.type != Descriptor::Type::NONE && !lockdown)
	{
		{
			Cell from = _board.cell(steps[attack].position);
			Cell to   = _board.cell(attackstep.position);
			doActiveAttack(from, to, steps[attack], attackstep, cset);

			// The moving unit might have been killed during the reactive attack.
			if (!_board.unit(from, steps[attack].type))
			{
				// We clear the order because the unit does not exist anymore.
				LOGV << "unit killed during reactive attack, order discarded";
				return discarded(player, order, cset, Notice::SUBJECTKILLED);
			}
		}

		// Perform bypass attack moves, if any.
		for (int i = attack; i < bypassattack; i++)
		{
			Cell from = _board.cell(steps[i].position);
			Cell to   = _board.cell(steps[i + 1].position);
			doMove(from, to, steps[i], steps[i + 1], cset);

			// The moving unit might have been killed during the attack of opportunity.
			if (!_board.unit(to, steps[i + 1].type))
			{
				// We clear the order because the unit does not exist anymore.
				LOGV << "unit killed during attack of opportunity, order discarded";
				return discarded(player, order, cset, Notice::SUBJECTKILLED);
			}

			// Bypass protects against lockdown, so we do not check it here.
		}
	}

	if ((size_t) moves < order.moves.size())
	{
		// If we had to stop moving early because of our movement speed limit,
		// we update the list of moves still left to make.
		// The subject of the UNFINISHED change is its original position.
		Descriptor oldsubject = order.subject;
		// Update the order by adjusting the subject and list of moves.
		order.subject = steps[moves];
		order.moves.erase(order.moves.begin(), order.moves.begin() + moves);
		// We will continue this order later.
		LOGV << "order to be continued: " << TypeEncoder(&_bible) << order;
		return unfinished(player, oldsubject, order, cset);
	}
	else
	{
		// Otherwise the are done moving.
		LOGV << "order completed";
		// The subject of the FINISHED change is its original position.
		return finished(player, order, cset);
	}
}

void Automaton::processGuard(const Player& player, Order& order)
{
	// Declare we are processing this order.
	ChangeSet cset;
	cset.push(Change(Change::Type::ACTING, order.subject), Vision::only(player));

	// Find the guarding unit based on its description.
	Descriptor::Type desctype = order.subject.type;
	Cell from = _board.cell(order.subject.position);
	const UnitTokenWithId& guardingunit = _board.unit(from, desctype);

	// Is the unit the subject of the order?
	if (_activesubjects[guardingunit.id()] != order.subject)
	{
		LOGV << "subject id does not match, order discarded";
		return discarded(player, order, cset);
	}

	// Does the unit exist?
	if (!guardingunit)
	{
		LOGE << "verified subject does not exist, order discarded";
		DEBUG_ASSERT(false && "subject verification is not watertight");
		return discarded(player, order, cset);
	}

	// Is the unit owned by the player?
	if (guardingunit.owner != player)
	{
		LOGE << "verified subject not controlled by player, order discarded";
		DEBUG_ASSERT(false && "subject verification is not watertight");
		return discarded(player, order, cset);
	}

	// Can the unit guard?
	if (!_bible.unitCanGuard(guardingunit.type))
	{
		LOGD << "subject cannot guard, order discarded"
			": " << TypeEncoder(&_bible) << order << " from " << player;
		return discarded(player, order, cset, Notice::ORDERINVALID);
	}

	// Units can only attack (hence guard) adjacent cells.
	int distance = Aim(order.target.position, order.subject.position).sumofsquares();
	if (distance != 1)
	{
		LOGD << "target is out of range, order discarded"
			": " << TypeEncoder(&_bible) << order << " from " << player;
		return discarded(player, order, cset, Notice::ORDERINVALID);
	}

	// Before attacking, we look for a target.
	Cell to = _board.cell(order.target.position);
	const UnitToken& otherunit = _board.unit(to, desctype);
	const TileToken& tile = _board.tile(to);

	// Are we out of bounds?
	if (!tile)
	{
		LOGW << "tile is null, order discarded"
			": " << TypeEncoder(&_bible) << order << " from " << player;
		return discarded(player, order, cset, Notice::ORDERINVALID);
	}

	// Does the unit get cold feet?
	if (hasColdFeet(from))
	{
		LOGV << "unit has cold feet, order discarded";
		return discarded(player, order, cset, Notice::COLDFEET);
	}

	// If the cell is empty, we cannot attack.
	// If the unit is friendly, we cannot attack.
	if (!otherunit || otherunit.owner == guardingunit.owner)
	{
		LOGV << "no unit to attack, order postponed";
		return postponed(player, order, cset, Notice::NOTARGET, order.target);
	}

	Descriptor target(desctype, order.target.position);
	doActiveAttack(from, to, order.subject, target, cset);

	// The guarding unit might have been killed during the reactive attack.
	if (!_board.unit(from, order.subject.type))
	{
		// We clear the order because the unit does not exist anymore.
		LOGV << "unit killed during reactive attack, order discarded";
		return discarded(player, order, cset, Notice::SUBJECTKILLED);
	}

	// This order never completes.
	LOGV << "order to be continued";
	return unfinished(player, order.subject, order, cset);
}

void Automaton::processFocus(const Player& player, Order& order)
{
	// Declare we are processing this order.
	ChangeSet cset;
	cset.push(Change(Change::Type::ACTING, order.subject), Vision::only(player));

	// Find the focussing unit based on its description.
	Descriptor::Type desctype = order.subject.type;
	Cell from = _board.cell(order.subject.position);
	const UnitTokenWithId& focussingunit = _board.unit(from, desctype);

	// Is the unit the subject of the order?
	if (_activesubjects[focussingunit.id()] != order.subject)
	{
		LOGV << "subject id does not match, order discarded";
		return discarded(player, order, cset);
	}

	// Does the unit exist?
	if (!focussingunit)
	{
		LOGE << "verified subject does not exist, order discarded";
		DEBUG_ASSERT(false && "subject verification is not watertight");
		return discarded(player, order, cset);
	}

	// Is the unit owned by the player?
	if (focussingunit.owner != player)
	{
		LOGE << "verified subject not controlled by player, order discarded";
		DEBUG_ASSERT(false && "subject verification is not watertight");
		return discarded(player, order, cset);
	}

	// Can the unit focus?
	if (!_bible.unitCanFocus(focussingunit.type))
	{
		LOGD << "subject cannot focus, order discarded"
			": " << TypeEncoder(&_bible) << order << " from " << player;
		return discarded(player, order, cset, Notice::ORDERINVALID);
	}

	// Units can only attack (hence focus) adjacent cells.
	Cell to = _board.cell(order.target.position);
	int distance = Aim(to.pos(), from.pos()).sumofsquares();
	if (distance != 1)
	{
		LOGD << "target is out of range, order discarded"
			": " << TypeEncoder(&_bible) << order << " from " << player;
		return discarded(player, order, cset, Notice::ORDERINVALID);
	}

	// Before attacking, we look for a target.
	const UnitToken& otherunit = _board.unit(to, desctype);
	const TileToken& tile = _board.tile(to);

	// Are we out of bounds?
	if (!tile)
	{
		LOGW << "tile is null, order discarded"
			": " << TypeEncoder(&_bible) << order << " from " << player;
		return discarded(player, order, cset, Notice::ORDERINVALID);
	}

	// Does the unit get cold feet?
	if (hasColdFeet(from))
	{
		LOGV << "unit has cold feet, order discarded";
		return discarded(player, order, cset, Notice::COLDFEET);
	}

	// If the cell is empty, we cannot attack.
	// If the unit is friendly, we cannot attack.
	if (!otherunit || otherunit.owner == focussingunit.owner)
	{
		if (!_bible.focusOnce())
		{
			LOGV << "no unit to attack, order postponed";
			return postponed(player, order, cset, Notice::NOTARGET, order.target);
		}

		// We show that the unit did not attack anything. Other players do not
		// get to see this because they are not informed of intentions.
		cset.push(Change(Change::Type::NONE,
				order.subject, order.target, Notice::NOTARGET),
			Vision::only(player));

		LOGV << "order completed";
		return finished(player, order, cset);
	}

	Descriptor target(desctype, order.target.position);
	doFocussedAttack(from, to, order.subject, target, cset);

	// The focussing unit might have been killed during the reactive attack.
	if (!_board.unit(from, order.subject.type))
	{
		// We clear the order because the unit does not exist anymore.
		LOGV << "unit killed during reactive attack, order discarded";
		return discarded(player, order, cset, Notice::SUBJECTKILLED);
	}

	if (!_bible.focusOnce())
	{
		// This order never completes.
		LOGV << "order to be continued";
		return unfinished(player, order.subject, order, cset);
	}

	// This order was completed.
	LOGV << "order completed";
	return finished(player, order, cset);
}

void Automaton::processLockdown(const Player& player, Order& order)
{
	// Declare we are processing this order.
	ChangeSet cset;
	cset.push(Change(Change::Type::ACTING, order.subject), Vision::only(player));

	// Find the focussing unit based on its description.
	Descriptor::Type desctype = order.subject.type;
	Cell from = _board.cell(order.subject.position);
	const UnitTokenWithId& activeunit = _board.unit(from, desctype);

	// Is the unit the subject of the order?
	if (_activesubjects[activeunit.id()] != order.subject)
	{
		LOGV << "subject id does not match, order discarded";
		return discarded(player, order, cset);
	}

	// Does the unit exist?
	if (!activeunit)
	{
		LOGE << "verified subject does not exist, order discarded";
		DEBUG_ASSERT(false && "subject verification is not watertight");
		return discarded(player, order, cset);
	}

	// Is the unit owned by the player?
	if (activeunit.owner != player)
	{
		LOGE << "verified subject not controlled by player, order discarded";
		DEBUG_ASSERT(false && "subject verification is not watertight");
		return discarded(player, order, cset);
	}

	// Can the unit lockdown?
	if (!_bible.unitCanLockdown(activeunit.type))
	{
		LOGD << "subject cannot lockdown, order discarded"
			": " << TypeEncoder(&_bible) << order << " from " << player;
		return discarded(player, order, cset, Notice::ORDERINVALID);
	}

	// Units can only attack (hence lockdown) adjacent cells.
	Cell to = _board.cell(order.target.position);
	int distance = Aim(to.pos(), from.pos()).sumofsquares();
	if (distance != 1)
	{
		LOGD << "target is out of range, order discarded"
			": " << TypeEncoder(&_bible) << order << " from " << player;
		return discarded(player, order, cset, Notice::ORDERINVALID);
	}

	// Before attacking, we look for a target.
	const UnitToken& otherunit = _board.unit(to, desctype);
	const TileToken& tile = _board.tile(to);

	// Are we out of bounds?
	if (!tile)
	{
		LOGW << "tile is null, order discarded"
			": " << TypeEncoder(&_bible) << order << " from " << player;
		return discarded(player, order, cset, Notice::ORDERINVALID);
	}

	// Does the unit get cold feet?
	if (hasColdFeet(from))
	{
		LOGV << "unit has cold feet, order discarded";
		return discarded(player, order, cset, Notice::COLDFEET);
	}

	// This unit is now in lockdown mode.
	_lockdowns.emplace_back(activeunit.id(), from, to);

	// If the cell is empty, we cannot attack.
	// If the unit is friendly, we cannot attack.
	if (!otherunit || otherunit.owner == activeunit.owner)
	{
		// We show that the unit did not attack anything. Other players do not
		// get to see this because they are not informed of intentions.
		cset.push(Change(Change::Type::NONE,
				order.subject, order.target, Notice::NOTARGET),
			Vision::only(player));

		LOGV << "order completed";
		return finished(player, order, cset);
	}

	Descriptor target(desctype, order.target.position);
	doActiveAttack(from, to, order.subject, target, cset);

	// The active unit might have been killed during the reactive attack.
	if (!_board.unit(from, order.subject.type))
	{
		// We clear the order because the unit does not exist anymore.
		LOGV << "unit killed during reactive attack, order discarded";
		return discarded(player, order, cset, Notice::SUBJECTKILLED);
	}

	// This order was completed.
	LOGV << "order completed";
	return finished(player, order, cset);
}

void Automaton::processShell(const Player& player, Order& order)
{
	// Declare we are processing this order.
	ChangeSet cset;
	cset.push(Change(Change::Type::ACTING, order.subject), Vision::only(player));

	// Find the shelling unit based on its description.
	Descriptor::Type desctype = order.subject.type;
	Cell from = _board.cell(order.subject.position);
	const UnitTokenWithId& shellingunit = _board.unit(from, desctype);

	// Is the unit the subject of the order?
	if (_activesubjects[shellingunit.id()] != order.subject)
	{
		LOGV << "subject id does not match, order discarded";
		return discarded(player, order, cset);
	}

	// Does the unit exist?
	if (!shellingunit)
	{
		LOGE << "verified subject does not exist, order discarded";
		DEBUG_ASSERT(false && "subject verification is not watertight");
		return discarded(player, order, cset);
	}

	// Is the unit owned by the player?
	if (shellingunit.owner != player)
	{
		LOGE << "verified subject not controlled by player, order discarded";
		DEBUG_ASSERT(false && "subject verification is not watertight");
		return discarded(player, order, cset);
	}

	// Can the unit shell?
	UnitType unittype = shellingunit.type;
	if (!_bible.unitCanShell(unittype))
	{
		LOGD << "subject cannot shell, order discarded"
			": " << TypeEncoder(&_bible) << order << " from " << player;
		return discarded(player, order, cset, Notice::ORDERINVALID);
	}

	// Is the ability out of range?
	int distance = Aim(order.target.position, order.subject.position).sumofsquares();
	if (distance < _bible.unitRangeMin(unittype) || distance > _bible.unitRangeMax(unittype))
	{
		LOGD << "target is out of range, order discarded"
			": " << TypeEncoder(&_bible) << order << " from " << player;
		return discarded(player, order, cset, Notice::ORDERINVALID);
	}

	// Before shelling, we look for targets.
	Cell to = _board.cell(order.target.position);
	const TileToken& tile = _board.tile(to);

	// Are we out of bounds?
	if (!tile)
	{
		LOGW << "tile is null, order discarded"
			": " << TypeEncoder(&_bible) << order << " from " << player;
		return discarded(player, order, cset, Notice::ORDERINVALID);
	}

	// Does the unit get cold feet?
	if (hasColdFeet(from))
	{
		LOGV << "unit has cold feet, order discarded";
		return discarded(player, order, cset, Notice::COLDFEET);
	}

	// Deal the damage.
	for (int volley = 0; volley < _bible.unitAbilityVolleys(unittype); volley++)
	{
		doShelling(from, to, order.subject, cset);
	}

	// This order was completed.
	LOGV << "order completed";
	return finished(player, order, cset);
}

void Automaton::processBombard(const Player& player, Order& order)
{
	// Declare we are processing this order.
	ChangeSet cset;
	cset.push(Change(Change::Type::ACTING, order.subject), Vision::only(player));

	// Find the bombarding unit based on its description.
	Descriptor::Type desctype = order.subject.type;
	Cell from = _board.cell(order.subject.position);
	const UnitTokenWithId& bombardingunit = _board.unit(from, desctype);

	// Is the unit the subject of the order?
	if (_activesubjects[bombardingunit.id()] != order.subject)
	{
		LOGV << "subject id does not match, order discarded";
		return discarded(player, order, cset);
	}

	// Does the unit exist?
	if (!bombardingunit)
	{
		LOGE << "verified subject does not exist, order discarded";
		DEBUG_ASSERT(false && "subject verification is not watertight");
		return discarded(player, order, cset);
	}

	// Is the unit owned by the player?
	if (bombardingunit.owner != player)
	{
		LOGE << "verified subject not controlled by player, order discarded";
		DEBUG_ASSERT(false && "subject verification is not watertight");
		return discarded(player, order, cset);
	}

	// Can the unit bombard?
	UnitType unittype = bombardingunit.type;
	if (!_bible.unitCanBombard(unittype))
	{
		LOGD << "subject cannot bombard, order discarded"
			": " << TypeEncoder(&_bible) << order << " from " << player;
		return discarded(player, order, cset, Notice::ORDERINVALID);
	}

	// Is the ability out of range?
	int distance = Aim(order.target.position, order.subject.position).sumofsquares();
	if (distance < _bible.unitRangeMin(unittype) || distance > _bible.unitRangeMax(unittype))
	{
		LOGD << "target is out of range, order discarded"
			": " << TypeEncoder(&_bible) << order << " from " << player;
		return discarded(player, order, cset, Notice::ORDERINVALID);
	}

	// Before bombarding, we look for targets.
	Cell to = _board.cell(order.target.position);
	const TileToken& tile = _board.tile(to);

	// Are we out of bounds?
	if (!tile)
	{
		LOGW << "tile is null, order discarded"
			": " << TypeEncoder(&_bible) << order << " from " << player;
		return discarded(player, order, cset, Notice::ORDERINVALID);
	}

	// Does the unit get cold feet?
	if (hasColdFeet(from))
	{
		LOGV << "unit has cold feet, order discarded";
		return discarded(player, order, cset, Notice::COLDFEET);
	}

	// Deal the damage.
	for (int volley = 0; volley < _bible.unitAbilityVolleys(unittype); volley++)
	{
		doBombardment(from, to, order.subject, cset);
	}

	// This order was completed.
	LOGV << "order completed";
	return finished(player, order, cset);
}

void Automaton::processBomb(const Player& player, Order& order)
{
	// Declare we are processing this order.
	ChangeSet cset;
	cset.push(Change(Change::Type::ACTING, order.subject), Vision::only(player));

	// Find the bombarding unit based on its description.
	Descriptor::Type desctype = order.subject.type;
	Cell at = _board.cell(order.subject.position);
	const UnitTokenWithId& bombingunit = _board.unit(at, desctype);

	// Is the unit the subject of the order?
	if (_activesubjects[bombingunit.id()] != order.subject)
	{
		LOGV << "subject id does not match, order discarded";
		return discarded(player, order, cset);
	}

	// Does the unit exist?
	if (!bombingunit)
	{
		LOGE << "verified subject does not exist, order discarded";
		DEBUG_ASSERT(false && "subject verification is not watertight");
		return discarded(player, order, cset);
	}

	// Is the unit owned by the player?
	if (bombingunit.owner != player)
	{
		LOGE << "verified subject not controlled by player, order discarded";
		DEBUG_ASSERT(false && "subject verification is not watertight");
		return discarded(player, order, cset);
	}

	// Can the unit bomb?
	UnitType unittype = bombingunit.type;
	if (!_bible.unitCanBomb(unittype))
	{
		LOGD << "subject cannot bomb, order discarded"
			": " << TypeEncoder(&_bible) << order << " from " << player;
		return discarded(player, order, cset, Notice::ORDERINVALID);
	}

	// Before bombing, we look for targets.
	const TileToken& tile = _board.tile(at);

	// Are we out of bounds?
	if (!tile)
	{
		LOGW << "tile is null, order discarded"
			": " << TypeEncoder(&_bible) << order << " from " << player;
		return discarded(player, order, cset, Notice::ORDERINVALID);
	}

	// Does the unit get cold feet?
	if (hasColdFeet(at))
	{
		LOGV << "unit has cold feet, order discarded";
		return discarded(player, order, cset, Notice::COLDFEET);
	}

	// Deal the damage.
	for (int volley = 0; volley < _bible.unitAbilityVolleys(unittype); volley++)
	{
		doBombing(at, at, order.subject, cset);
	}

	// This order was completed.
	LOGV << "order completed";
	return finished(player, order, cset);
}

void Automaton::processCapture(const Player& player, Order& order)
{
	// Declare we are processing this order.
	ChangeSet cset;
	cset.push(Change(Change::Type::ACTING, order.subject), Vision::only(player));

	// Find the capturing unit based on its description.
	Cell at = _board.cell(order.subject.position);
	Descriptor desc = order.subject;
	const UnitTokenWithId& conqueror = _board.unit(at, desc.type);
	UnitType conquerortype = conqueror.type;
	const TileToken& tile = _board.tile(at);
	TileType oldtype = tile.type;
	Player oldowner = tile.owner;

	// Is the unit the subject of the order?
	if (_activesubjects[conqueror.id()] != order.subject)
	{
		LOGV << "subject id does not match, order discarded";
		return discarded(player, order, cset);
	}

	// Does the unit exist?
	if (!conqueror)
	{
		LOGE << "verified subject does not exist, order discarded";
		DEBUG_ASSERT(false && "subject verification is not watertight");
		return discarded(player, order, cset);
	}

	// Is the unit owned by the player?
	if (conqueror.owner != player)
	{
		LOGE << "verified subject not controlled by player, order discarded";
		DEBUG_ASSERT(false && "subject verification is not watertight");
		return discarded(player, order, cset);
	}

	// Can the unit capture?
	if (!_bible.unitCanCapture(conqueror.type))
	{
		LOGD << "subject cannot capture, order discarded"
			": " << TypeEncoder(&_bible) << order << " from " << player;
		return discarded(player, order, cset, Notice::ORDERINVALID);
	}

	// Are we out of bounds?
	if (!tile)
	{
		LOGW << "tile is null, order discarded"
			": " << TypeEncoder(&_bible) << order << " from " << player;
		return discarded(player, order, cset, Notice::ORDERINVALID);
	}

	// Does the unit get cold feet?
	if (hasColdFeet(at))
	{
		LOGV << "unit has cold feet, order discarded";
		return discarded(player, order, cset, Notice::COLDFEET);
	}

	// We can't capture a tile that is already ours.
	if (tile.owner == conqueror.owner)
	{
		// We show the player that the unit failed to capture anything.
		LOGV << "tile already owned, order discarded";
		return discarded(player, order, cset, Notice::NOTARGET);
	}

	// We can only capture ownable tiles (e.g. city, industry, crops).
	if (!_bible.tileOwnable(tile.type))
	{
		// We show the player that the unit failed to capture anything.
		LOGV << "tile not capturable, order discarded";
		return discarded(player, order, cset, Notice::NOTARGET);
	}

	// A unit must have at least as many stacks as the tile has powered stacks.
	// Because both the unit and the tile can lose stacks and power during the action phase,
	// this is not an illegal order. Therefore we just postpone the order.
	if (_bible.captureStrengthCheck() && tile.power > conqueror.stacks)
	{
		// We show the player that the unit failed to capture anything.
		LOGV << "unit not strong enough to capture, order postponed";
		return postponed(player, order, cset, Notice::LACKINGSTACKS);
	}

	// The capture is successful.
	Change captureschange(Change::Type::CAPTURES, desc);
	_board.enact(captureschange);
	cset.push(captureschange, _board.vision(at));

	Change capturedchange(Change::Type::CAPTURED,
		Descriptor::tile(at.pos()), player);
	_board.enact(capturedchange);
	cset.push(capturedchange, _board.vision(at));

	// Update vision.
	VisionTransition(_bible, _board, *this, cset).executeAround(at,
		std::max(_bible.tileVision(oldtype),
			_bible.unitVision(conquerortype)));

	// Check for morale boost.
	checkTileCaptureMoraleGain(at, oldowner, player, cset);

	// The capture order was completed.
	LOGV << "order completed";
	return finished(player, order, cset);
}

void Automaton::processShape(const Player& player, Order& order)
{
	// Declare we are processing this order.
	ChangeSet cset;
	cset.push(Change(Change::Type::ACTING, order.subject), Vision::only(player));

	// Find the settling unit based on its description.
	Cell at = _board.cell(order.subject.position);
	const UnitTokenWithId& shaper = _board.unit(at, order.subject.type);
	UnitType shapertype = shaper.type;
	const TileToken& tile = _board.tile(at);
	TileType oldtype = tile.type;
	TileType newtype = order.tiletype;

	// Is the unit the subject of the order?
	if (_activesubjects[shaper.id()] != order.subject)
	{
		LOGV << "subject id does not match, order discarded";
		return discarded(player, order, cset);
	}

	// Does the unit exist?
	if (!shaper)
	{
		LOGE << "verified subject does not exist, order discarded";
		DEBUG_ASSERT(false && "subject verification is not watertight");
		return discarded(player, order, cset);
	}

	// Is the unit owned by the player?
	if (shaper.owner != player)
	{
		LOGE << "verified subject not controlled by player, order discarded";
		DEBUG_ASSERT(false && "subject verification is not watertight");
		return discarded(player, order, cset);
	}

	// Can the unit settle this tile?
	const auto& builds = _bible.unitShapes(shaper.type);
	auto iter = std::find_if(builds.begin(), builds.end(),
		[newtype](const Bible::TileBuild& build){

			return (build.type == newtype);
		});
	if (iter == builds.end())
	{
		LOGD << "subject cannot build this type of tile, order discarded"
			": " << TypeEncoder(&_bible) << order << " from " << player;
		return discarded(player, order, cset, Notice::ORDERINVALID);
	}
	const Bible::TileBuild& build = *iter;

	// Are we out of bounds?
	if (!tile)
	{
		LOGW << "tile is null, order discarded"
			": " << TypeEncoder(&_bible) << order << " from " << player;
		return discarded(player, order, cset, Notice::ORDERINVALID);
	}

	// Only some tiles (grass, dirt, desert) can be built on.
	if (!_bible.tileBuildable(tile.type))
	{
		// We show the player that the unit failed to build anything.
		LOGV << "tile not buildable, order discarded";
		return discarded(player, order, cset, Notice::UNBUILDABLE);
	}

	// Does the unit get cold feet?
	if (hasColdFeet(at))
	{
		LOGV << "unit has cold feet, order discarded";
		return discarded(player, order, cset, Notice::COLDFEET);
	}

	// If the player cannot afford the unit cost, the order is postponed.
	if (_money[player] < build.cost)
	{
		// We show the player that the tile failed to build anything.
		LOGV << "not enough money, order postponed";
		return postponed(player, order, cset, Notice::LACKINGMONEY);
	}

	// Spend the money.
	if (build.cost > 0)
	{
		_money[player] -= build.cost;
		Change moneychange(Change::Type::EXPENDITURE, order.subject,
			shaper.owner);
		moneychange.xMoney(-build.cost);
		cset.push(moneychange, Vision::only(player));
	}

	// If the new tile is ownable (e.g. city, crops), then its owner is the shaper's owner.
	TileToken newtoken;
	newtoken.type = newtype;
	if (_bible.tileOwnable(newtype)) newtoken.owner = shaper.owner;
	newtoken.stacks = _bible.tileStacksBuilt(newtype);
	newtoken.power = _bible.tilePowerBuilt(newtype);

	// Build the tile.
	Descriptor tiledesc = Descriptor::tile(order.subject.position);
	Change buildchange(Change::Type::SHAPED, tiledesc, newtoken);
	_board.enact(buildchange);
	cset.push(buildchange, _board.vision(at));

	// The shaper has shaped.
	Change shapechange(Change::Type::SHAPES, order.subject);
	_board.enact(shapechange);
	cset.push(shapechange, _board.vision(at));

	// Update vision.
	VisionTransition(_bible, _board, *this, cset).executeAround(at,
		std::max(_bible.unitVision(shapertype),
			std::max(_bible.tileVision(oldtype), _bible.tileVision(newtype))));

	// Check for morale boost.
	checkTileBuildMoraleGain(at, cset);

	// Check for cleanses.
	checkTileBuildCleanse(at, cset);

	// Check for auto-cultivate triggers.
	checkAutoCultivate(at, cset);

	// The order has been completed.
	LOGV << "order completed";
	return finished(player, order, cset);
}

void Automaton::processSettle(const Player& player, Order& order)
{
	// Declare we are processing this order.
	ChangeSet cset;
	cset.push(Change(Change::Type::ACTING, order.subject), Vision::only(player));

	// Find the settling unit based on its description.
	Cell at = _board.cell(order.subject.position);
	const UnitTokenWithId& settler = _board.unit(at, order.subject.type);
	UnitType settlertype = settler.type;
	const TileToken& tile = _board.tile(at);
	TileType oldtype = tile.type;
	TileType newtype = order.tiletype;

	// Is the unit the subject of the order?
	if (_activesubjects[settler.id()] != order.subject)
	{
		LOGV << "subject id does not match, order discarded";
		return discarded(player, order, cset);
	}

	// Does the unit exist?
	if (!settler)
	{
		LOGE << "verified subject does not exist, order discarded";
		DEBUG_ASSERT(false && "subject verification is not watertight");
		return discarded(player, order, cset);
	}

	// Is the unit owned by the player?
	if (settler.owner != player)
	{
		LOGE << "verified subject not controlled by player, order discarded";
		DEBUG_ASSERT(false && "subject verification is not watertight");
		return discarded(player, order, cset);
	}

	// Can the unit settle this tile?
	const auto& builds = _bible.unitSettles(settler.type);
	auto iter = std::find_if(builds.begin(), builds.end(),
		[newtype](const Bible::TileBuild& build){

			return (build.type == newtype);
		});
	if (iter == builds.end())
	{
		LOGD << "subject cannot build this type of tile, order discarded"
			": " << TypeEncoder(&_bible) << order << " from " << player;
		return discarded(player, order, cset, Notice::ORDERINVALID);
	}
	const Bible::TileBuild& build = *iter;

	// Are we out of bounds?
	if (!tile)
	{
		LOGW << "tile is null, order discarded"
			": " << TypeEncoder(&_bible) << order << " from " << player;
		return discarded(player, order, cset, Notice::ORDERINVALID);
	}

	// Only some tiles (grass, dirt, desert) can be built on.
	if (!_bible.tileBuildable(tile.type))
	{
		// We show the player that the unit failed to build anything.
		LOGV << "tile not buildable, order discarded";
		return discarded(player, order, cset, Notice::UNBUILDABLE);
	}

	// Does the unit get cold feet?
	if (hasColdFeet(at))
	{
		LOGV << "unit has cold feet, order discarded";
		return discarded(player, order, cset, Notice::COLDFEET);
	}

	// If the player cannot afford the unit cost, the order is postponed.
	if (_money[player] < build.cost)
	{
		// We show the player that the tile failed to build anything.
		LOGV << "not enough money, order postponed";
		return postponed(player, order, cset, Notice::LACKINGMONEY);
	}

	// Spend the money.
	if (build.cost > 0)
	{
		_money[player] -= build.cost;
		Change moneychange(Change::Type::EXPENDITURE, order.subject,
			settler.owner);
		moneychange.xMoney(-build.cost);
		cset.push(moneychange, Vision::only(player));
	}

	// If the new tile is ownable (e.g. city, crops), then its owner is the settler's owner.
	TileToken newtoken;
	newtoken.type = newtype;
	if (_bible.tileOwnable(newtype)) newtoken.owner = settler.owner;
	newtoken.stacks = _bible.tileStacksBuilt(newtype);
	newtoken.power = _bible.tilePowerBuilt(newtype);

	// Build the tile.
	Descriptor tiledesc = Descriptor::tile(order.subject.position);
	Change buildchange(Change::Type::SETTLED, tiledesc, newtoken);
	_board.enact(buildchange);
	cset.push(buildchange, _board.vision(at));

	// The settler has settled.
	Change exitchange(Change::Type::SETTLES, order.subject);
	_board.enact(exitchange);
	cset.push(exitchange, _board.vision(at));

	// Update vision.
	VisionTransition(_bible, _board, *this, cset).executeAround(at,
		std::max(_bible.unitVision(settlertype),
			std::max(_bible.tileVision(oldtype), _bible.tileVision(newtype))));

	// Check for morale boost.
	checkTileBuildMoraleGain(at, cset);

	// Check for cleanses.
	checkColdFeetCleanse(at, cset);
	checkTileBuildCleanse(at, cset);

	// Check for auto-cultivate triggers.
	checkAutoCultivate(at, cset);

	// The order has been completed.
	LOGV << "order completed";
	return finished(player, order, cset);
}

void Automaton::processExpand(const Player& player, Order& order)
{
	// Declare we are processing this order.
	ChangeSet cset;
	cset.push(Change(Change::Type::ACTING, order.subject), Vision::only(player));

	// Find the expander based on its description.
	Cell from = _board.cell(order.subject.position);
	const TileTokenWithId& expander = _board.tile(from);
	Cell at = _board.cell(order.target.position);
	const TileToken& tile = _board.tile(at);
	TileType oldtype = tile.type;
	TileType newtype = order.tiletype;

	// Is the unit the subject of the order?
	if (_activesubjects[expander.id()] != order.subject)
	{
		LOGV << "subject id does not match, order discarded";
		return discarded(player, order, cset);
	}

	// Are we out of bounds?
	if (!expander)
	{
		LOGE << "verified subject tile is null, order discarded";
		DEBUG_ASSERT(false && "subject verification is not watertight");
		return discarded(player, order, cset);
	}

	// Is the tile owned by the player?
	if (expander.owner != player)
	{
		LOGE << "verified subject not controlled by player, order discarded";
		DEBUG_ASSERT(false && "subject verification is not watertight");
		return discarded(player, order, cset);
	}

	// Can the tile build this tile?
	const auto& builds = _bible.tileExpands(expander.type);
	auto iter = std::find_if(builds.begin(), builds.end(),
		[newtype](const Bible::TileBuild& build){

			return (build.type == newtype);
		});
	if (iter == builds.end())
	{
		LOGD << "subject cannot build this type of tile, order discarded"
			": " << TypeEncoder(&_bible) << order << " from " << player;
		return discarded(player, order, cset, Notice::ORDERINVALID);
	}
	const Bible::TileBuild& build = *iter;

	// Are we out of bounds?
	if (!tile)
	{
		LOGW << "tile is null, order discarded"
			": " << TypeEncoder(&_bible) << order << " from " << player;
		return discarded(player, order, cset, Notice::ORDERINVALID);
	}

	// Is the ability out of range?
	int distance = Aim(order.target.position, order.subject.position).sumofsquares();
	if (distance < _bible.tileExpandRangeMin() || distance > _bible.tileExpandRangeMax())
	{
		LOGD << "target is out of range, order discarded"
			": " << TypeEncoder(&_bible) << order << " from " << player;
		return discarded(player, order, cset, Notice::ORDERINVALID);
	}

	// Only some tiles (grass, dirt, desert) can be built on.
	if (!_bible.tileBuildable(tile.type))
	{
		// We show the player that the unit failed to build anything.
		LOGV << "tile not buildable, order discarded";
		return discarded(player, order, cset, Notice::UNBUILDABLE, order.target);
	}

	// If the tile has no powered stacks, the order is postponed.
	if (!_bible.powerAbolished() && expander.power <= 0)
	{
		// We show the player that the tile failed to build anything.
		LOGV << "subject has no power, order postponed";
		return postponed(player, order, cset, Notice::LACKINGPOWER);
	}

	// If the tile is occupied by an enemy unit, the order is postponed.
	if ((_board.ground(from)
				&& _board.ground(from).owner != expander.owner
				&& _bible.unitCanOccupy(_board.ground(from).type))
		|| (_board.air(from)
				&& _board.air(from).owner != expander.owner
				&& _bible.unitCanOccupy(_board.air(from).type)))
	{
		// We show the player that the tile failed to build anything.
		LOGV << "tile occupied by enemy, order postponed";
		return postponed(player, order, cset, Notice::OCCUPIEDBYENEMY);
	}

	// If there is a ground unit occupying the target tile, the order is postponed.
	if (_board.ground(at))
	{
		// We show the player that the tile failed to build anything.
		LOGV << "target occupied, order postponed";
		return postponed(player, order, cset, Notice::TARGETOCCUPIED, order.target);
	}

	// If the player cannot afford the unit cost, the order is postponed.
	if (_money[player] < build.cost)
	{
		// We show the player that the tile failed to build anything.
		LOGV << "not enough money, order postponed";
		return postponed(player, order, cset, Notice::LACKINGMONEY);
	}

	// Spend the money.
	if (build.cost > 0)
	{
		_money[player] -= build.cost;
		Change moneychange(Change::Type::EXPENDITURE, order.subject,
			expander.owner);
		moneychange.xMoney(-build.cost);
		cset.push(moneychange, Vision::only(player));
	}

	// If the new tile is ownable (e.g. city, crops), then its owner is the builder's owner.
	TileToken newtoken;
	newtoken.type = newtype;
	if (_bible.tileOwnable(newtype)) newtoken.owner = expander.owner;
	newtoken.stacks = _bible.tileStacksBuilt(newtype);
	newtoken.power = _bible.tilePowerBuilt(newtype);

	// Spend the power.
	Change powerchange(Change::Type::EXPANDS, order.subject, order.target);
	if (!_bible.powerAbolished())
	{
		powerchange.xPower(_bible.powerDrainScaled()
			? -std::min((int) newtoken.stacks, (int) expander.power)
			: -1);
	}
	_board.enact(powerchange);
	cset.push(powerchange, _board.vision(from));

	// Build the tile.
	Descriptor tiledesc = Descriptor::tile(order.target.position);
	Change buildchange(Change::Type::EXPANDED, tiledesc, newtoken);
	_board.enact(buildchange);
	cset.push(buildchange, _board.vision(at));

	// Update vision.
	VisionTransition(_bible, _board, *this, cset).executeAround(at,
		std::max(_bible.tileVision(oldtype), _bible.tileVision(newtype)));

	// Check for morale boost.
	checkTileBuildMoraleGain(at, cset);

	// Check for cleanses.
	checkTileBuildCleanse(at, cset);

	// Check for auto-cultivate triggers.
	checkAutoCultivate(at, cset);

	// The order has been completed.
	LOGV << "order completed";
	return finished(player, order, cset);
}

void Automaton::processUpgrade(const Player& player, Order& order)
{
	// Declare we are processing this order.
	ChangeSet cset;
	cset.push(Change(Change::Type::ACTING, order.subject), Vision::only(player));

	// Find the upgrader based on its description.
	Cell at = _board.cell(order.subject.position);
	const TileTokenWithId& upgrader = _board.tile(at);
	TileType oldtype = upgrader.type;
	TileType newtype = order.tiletype;

	// If the new tiletype is NONE, the tile gains a stack instead of changing.
	bool upstack = (newtype == TileType::NONE);

	// Is the unit the subject of the order?
	if (_activesubjects[upgrader.id()] != order.subject)
	{
		LOGV << "subject id does not match, order discarded";
		return discarded(player, order, cset);
	}

	// Are we out of bounds?
	if (!upgrader)
	{
		LOGE << "verified subject tile is null, order discarded";
		DEBUG_ASSERT(false && "subject verification is not watertight");
		return discarded(player, order, cset);
	}

	// Is the tile owned by the player?
	if (upgrader.owner != player)
	{
		LOGE << "verified subject not controlled by player, order discarded";
		DEBUG_ASSERT(false && "subject verification is not watertight");
		return discarded(player, order, cset);
	}

	// Can the tile build this tile?
	const auto& builds = _bible.tileUpgrades(upgrader.type);
	auto iter = std::find_if(builds.begin(), builds.end(),
		[newtype](const Bible::TileBuild& build){

			return (build.type == newtype);
		});
	if (iter == builds.end())
	{
		LOGD << "subject cannot build this type of tile, order discarded"
			": " << TypeEncoder(&_bible) << order << " from " << player;
		return discarded(player, order, cset, Notice::ORDERINVALID);
	}
	const Bible::TileBuild& build = *iter;

	// Note that tiles can upgrade themselves even though they are not
	// tileBuildable().

	if (upstack
		&& upgrader.stacks >= (int) _bible.tileStacksMax(upgrader.type))
	{
		// We show the player that the tile did not need to upgrade.
		LOGV << "subject has max stacks, order discarded";
		return discarded(player, order, cset, Notice::NOTARGET);
	}

	// If the tile has no powered stacks, the order is postponed.
	if (!_bible.powerAbolished() && upgrader.power <= 0)
	{
		// We show the player that the tile failed to build anything.
		LOGV << "subject has no power, order postponed";
		return postponed(player, order, cset, Notice::LACKINGPOWER);
	}

	// If the tile is occupied by an enemy unit, the order is postponed.
	if ((_board.ground(at)
				&& _board.ground(at).owner != upgrader.owner
				&& _bible.unitCanOccupy(_board.ground(at).type))
		|| (_board.air(at)
				&& _board.air(at).owner != upgrader.owner
				&& _bible.unitCanOccupy(_board.air(at).type)))
	{
		// We show the player that the tile failed to build anything.
		LOGV << "tile occupied by enemy, order postponed";
		return postponed(player, order, cset, Notice::OCCUPIEDBYENEMY);
	}

	// If there is a ground unit occupying the tile, the order is postponed.
	if (_board.ground(at) && !_bible.upgradeNotBlockedByFriendly())
	{
		// We show the player that the tile failed to build anything.
		LOGV << "subject occupied, order postponed";
		return postponed(player, order, cset, Notice::SUBJECTOCCUPIED);
	}

	// If the player cannot afford the unit cost, the order is postponed.
	if (_money[player] < build.cost)
	{
		// We show the player that the tile failed to build anything.
		LOGV << "not enough money, order postponed";
		return postponed(player, order, cset, Notice::LACKINGMONEY);
	}

	// Spend the money.
	if (build.cost > 0)
	{
		_money[player] -= build.cost;
		Change moneychange(Change::Type::EXPENDITURE, order.subject,
			upgrader.owner);
		moneychange.xMoney(-build.cost);
		cset.push(moneychange, Vision::only(player));
	}

	// If the new tiletype is NONE, the tile gains a stack instead of changing.
	if (upstack) newtype = upgrader.type;

	// If the new tile is ownable (e.g. city, crops), then its owner is the builder's owner.
	TileToken newtoken;
	newtoken.type = newtype;
	if (_bible.tileOwnable(newtype)) newtoken.owner = upgrader.owner;
	newtoken.stacks = (upstack)
		? std::min(upgrader.stacks + 1,
			(int) _bible.tileStacksMax(upgrader.type))
		: (_bible.upgradeKeepStacks())
		? std::min(upgrader.stacks + 0,
			(int) _bible.tileStacksMax(newtype))
		: _bible.tileStacksBuilt(newtype);
	if (!_bible.powerAbolished())
	{
		newtoken.power = (upstack)
			? (_bible.upgradeKeepPower())
				? std::min(upgrader.power - 1,
					(int) _bible.tilePowerMax(newtype))
				: 0
			: (_bible.upgradeKeepPower())
				? std::max(
					std::min(upgrader.power - 1,
						(int) _bible.tilePowerMax(newtype)),
					(int) _bible.tilePowerBuilt(newtype))
				: _bible.tilePowerBuilt(newtype);
	}

	// Spend the power.
	Change powerchange(Change::Type::UPGRADES, order.subject);
	if (!_bible.powerAbolished())
	{
		powerchange.xPower((_bible.powerDrainScaled() && !_bible.upgradeKeepPower())
			? -std::min((int) newtoken.stacks, (int) upgrader.power)
			: -1);
	}
	_board.enact(powerchange);
	cset.push(powerchange, _board.vision(at));

	// Build the tile.
	Change buildchange(Change::Type::UPGRADED, order.subject, newtoken);
	_board.enact(buildchange);
	cset.push(buildchange, _board.vision(at));

	// Update vision.
	VisionTransition(_bible, _board, *this, cset).executeAround(at,
		std::max(_bible.tileVision(oldtype), _bible.tileVision(newtype)));

	// Upgrading does not grant a morale boost because it is repeatable.

	// Check for cleanses.
	checkTileBuildCleanse(at, cset);

	// Check for auto-cultivate triggers.
	checkAutoCultivate(at, cset);

	// The order has been completed.
	LOGV << "order completed";
	return finished(player, order, cset);
}

void Automaton::processCultivate(const Player& player, Order& order)
{
	// Declare we are processing this order.
	ChangeSet cset;
	cset.push(Change(Change::Type::ACTING, order.subject), Vision::only(player));

	// Find the cultivator based on its description.
	Cell at = _board.cell(order.subject.position);
	const TileTokenWithId& cultivator = _board.tile(at);
	TileType newtype = order.tiletype;

	// Is the unit the subject of the order?
	if (_activesubjects[cultivator.id()] != order.subject)
	{
		LOGV << "subject id does not match, order discarded";
		return discarded(player, order, cset);
	}

	// Are we out of bounds?
	if (!cultivator)
	{
		LOGE << "verified subject tile is null, order discarded";
		DEBUG_ASSERT(false && "subject verification is not watertight");
		return discarded(player, order, cset);
	}

	// Is the tile owned by the player?
	if (cultivator.owner != player)
	{
		LOGE << "verified subject not controlled by player, order discarded";
		DEBUG_ASSERT(false && "subject verification is not watertight");
		return discarded(player, order, cset);
	}

	// Can the tile build this tile?
	const auto& builds = _bible.tileCultivates(cultivator.type);
	auto iter = std::find_if(builds.begin(), builds.end(),
		[newtype](const Bible::TileBuild& build){

			return (build.type == newtype);
		});
	if (iter == builds.end())
	{
		LOGD << "subject cannot build this type of tile, order discarded"
			": " << TypeEncoder(&_bible) << order << " from " << player;
		return discarded(player, order, cset, Notice::ORDERINVALID);
	}
	const Bible::TileBuild& build = *iter;

	// Find target tiles from the surrounding area.
	std::vector<Cell> targets;
	for (Cell target : _board.area(at, 1, 2))
	{
		// Only some tiles (grass, dirt) can be built on.
		if (!_bible.tileBuildable(_board.tile(target).type))
		{
			continue;
		}

		// If there is a ground unit occupying the tile, we cannot build on it.
		if (_board.ground(target))
		{
			continue;
		}

		// If the tile is owned by the player, we do not build on it.
		if (_board.tile(target).owner == player)
		{
			continue;
		}

		targets.emplace_back(target);
	}

	// Are there any buildable targets?
	if (targets.empty())
	{
		// We show the player that the unit failed to build anything.
		LOGV << "no buildable targets, order discarded";
		return discarded(player, order, cset, Notice::NOTARGET);
	}

	// If the tile is occupied by an enemy unit, the order is postponed.
	if ((_board.ground(at)
				&& _board.ground(at).owner != cultivator.owner
				&& _bible.unitCanOccupy(_board.ground(at).type))
		|| (_board.air(at)
				&& _board.air(at).owner != cultivator.owner
				&& _bible.unitCanOccupy(_board.air(at).type)))
	{
		// We show the player that the tile failed to build anything.
		LOGV << "tile occupied by enemy, order postponed";
		return postponed(player, order, cset, Notice::OCCUPIEDBYENEMY);
	}

	// If the player cannot afford the unit cost, the order is postponed.
	if (_money[player] < build.cost)
	{
		// We show the player that the tile failed to build anything.
		LOGV << "not enough money, order postponed";
		return postponed(player, order, cset, Notice::LACKINGMONEY);
	}

	// Spend the money.
	if (build.cost > 0)
	{
		_money[player] -= build.cost;
		Change moneychange(Change::Type::EXPENDITURE, order.subject,
			cultivator.owner);
		moneychange.xMoney(-build.cost);
		cset.push(moneychange, Vision::only(player));
	}

	// Cultivate each target tile in random order.
	std::random_shuffle(targets.begin(), targets.end());
	for (Cell target : targets)
	{
		// Get the old type.
		TileType oldtype = _board.tile(target).type;

		// If the tile has no powered stacks, the order is postponed.
		if (!_bible.powerAbolished() && cultivator.power <= 0)
		{
			// We show the player that the tile failed to build anything.
			LOGV << "subject has no power, order postponed";
			return postponed(player, order, cset, Notice::LACKINGPOWER);
		}

		// Individual cultivations do not cost anything.

		// If the new tile is ownable (e.g. city, crops), then its owner is the builder's owner.
		TileToken newtoken;
		newtoken.type = newtype;
		if (_bible.tileOwnable(newtype)) newtoken.owner = cultivator.owner;
		newtoken.stacks = _bible.tileStacksBuilt(newtype);
		newtoken.power = _bible.tilePowerBuilt(newtype);

		// Spend the power.
		Change powerchange(Change::Type::CULTIVATES, order.subject);
		if (!_bible.powerAbolished())
		{
			powerchange.xPower(_bible.powerDrainScaled()
				? -std::min((int) newtoken.stacks, (int) cultivator.power)
				: -1);
		}
		_board.enact(powerchange);
		cset.push(powerchange, _board.vision(at));

		// Build the tile.
		Change buildchange(Change::Type::CULTIVATED,
			Descriptor::tile(target.pos()), newtoken);
		_board.enact(buildchange);
		cset.push(buildchange, _board.vision(target));

		// Update vision.
		VisionTransition(_bible, _board, *this, cset).executeAround(target,
			std::max(_bible.tileVision(oldtype), _bible.tileVision(newtype)));

		// Cultivating might in the future give a morale boost, but currently
		// it does not, and we do not want to waste the CPU cycles checking it.

		// Check for cleanses.
		checkTileBuildCleanse(target, cset);

		// Check for auto-cultivate triggers.
		checkAutoCultivate(target, cset);
	}

	// The order has been completed.
	LOGV << "order completed";
	return finished(player, order, cset);
}

void Automaton::checkAutoCultivate(Cell from, ChangeSet& cset)
{
	// Get the subject.
	const TileTokenWithId& cultivator = _board.tile(from);

	// Does it auto-cultivate?
	if (!_bible.tileAutoCultivates(cultivator.type))
	{
		return;
	}

	// Get the types of tiles it can cultivate.
	const std::vector<Bible::TileBuild>& builds = _bible.tileCultivates(
		cultivator.type);

	// This should not happen.
	DEBUG_ASSERT(!builds.empty());
	if (builds.empty())
	{
		return;
	}

	// Auto-cultivation uses the first cultivatable type.
	TileType newtype = builds[0].type;

	// When auto-cultivating, the cultivation cost is included in the tile cost.

	// List all target tiles from the surrounding area.
	std::vector<Cell> targets;
	for (Cell target : _board.area(from, 1, 2))
	{
		targets.emplace_back(target);
	}

	// Cultivate each target tile in random order.
	std::random_shuffle(targets.begin(), targets.end());
	for (Cell target : targets)
	{
		doAutoCultivate(cultivator.owner, from, target, newtype, cset);
	}
}

void Automaton::doAutoCultivate(const Player& player, Cell from,
	Cell target, const TileType& newtype, ChangeSet& cset)
{
	// Only some tiles (grass, dirt) can be built on.
	if (!_bible.tileBuildable(_board.tile(target).type))
	{
		return;
	}

	// If there is a ground unit occupying the tile, we cannot build on it.
	if (_board.ground(target))
	{
		return;
	}

	// If the tile is owned by the player, we do not build on it.
	if (_board.tile(target).owner == player)
	{
		return;
	}

	// Get the subject.
	const Descriptor& subject = Descriptor::tile(from.pos());
	const TileTokenWithId& cultivator = _board.tile(from);
	TileType oldtype = _board.tile(target).type;

	// If the tile has no powered stacks, it cannot auto-cultivate.
	if (!_bible.powerAbolished() && cultivator.power <= 0)
	{
		return;
	}

	// Individual cultivations do not cost anything.

	// If the new tile is ownable (e.g. city, crops), then its owner is the builder's owner.
	TileToken newtoken;
	newtoken.type = newtype;
	if (_bible.tileOwnable(newtype)) newtoken.owner = cultivator.owner;
	newtoken.stacks = _bible.tileStacksBuilt(newtype);
	newtoken.power = _bible.tilePowerBuilt(newtype);

	// Spend the power.
	Change powerchange(Change::Type::CULTIVATES, subject);
	if (!_bible.powerAbolished())
	{
		powerchange.xPower(_bible.powerDrainScaled()
			? -std::min((int) newtoken.stacks, (int) cultivator.power)
			: -1);
	}
	_board.enact(powerchange);
	cset.push(powerchange, _board.vision(from));

	// Build the tile.
	Change buildchange(Change::Type::CULTIVATED,
		Descriptor::tile(target.pos()), newtoken);
	_board.enact(buildchange);
	cset.push(buildchange, _board.vision(target));

	// Update vision.
	VisionTransition(_bible, _board, *this, cset).executeAround(target,
		std::max(_bible.tileVision(oldtype), _bible.tileVision(newtype)));

	// Cultivating might in the future give a morale boost, but currently
	// it does not, and we do not want to waste the CPU cycles checking it.

	// Check for cleanses.
	checkTileBuildCleanse(target, cset);

	// Check for auto-cultivate triggers.
	checkAutoCultivate(target, cset);
}

void Automaton::processProduce(const Player& player, Order& order)
{
	// Declare we are processing this order.
	ChangeSet cset;
	cset.push(Change(Change::Type::ACTING, order.subject), Vision::only(player));

	// Find the producer based on its description.
	Cell at = _board.cell(order.subject.position);
	const TileTokenWithId& producer = _board.tile(at);
	Cell to = _board.cell(order.target.position);
	const TileToken& targettile = _board.tile(to);
	UnitType newtype = order.unittype;

	// Is the unit the subject of the order?
	if (_activesubjects[producer.id()] != order.subject)
	{
		LOGV << "subject id does not match, order discarded";
		return discarded(player, order, cset);
	}

	// Are we out of bounds?
	if (!producer)
	{
		LOGE << "verified subject does not exist, order discarded";
		DEBUG_ASSERT(false && "subject verification is not watertight");
		return discarded(player, order, cset);
	}

	// Is the tile owned by the player?
	if (producer.owner != player)
	{
		LOGE << "verified subject not controlled by player, order discarded";
		DEBUG_ASSERT(false && "subject verification is not watertight");
		return discarded(player, order, cset);
	}

	// Can the unit settle this tile?
	const auto& builds = _bible.tileProduces(producer.type);
	auto iter = std::find_if(builds.begin(), builds.end(),
		[newtype](const Bible::UnitBuild& build){

			return (build.type == newtype);
		});
	if (iter == builds.end())
	{
		LOGD << "subject cannot produce this type of unit, order discarded"
			": " << TypeEncoder(&_bible) << order << " from " << player;
		return discarded(player, order, cset, Notice::ORDERINVALID);
	}
	const Bible::UnitBuild& build = *iter;

	// Are we out of bounds?
	if (!targettile)
	{
		LOGW << "tile is null, order discarded"
			": " << TypeEncoder(&_bible) << order << " from " << player;
		return discarded(player, order, cset, Notice::ORDERINVALID);
	}

	// Is the ability out of range?
	int distance = Aim(order.target.position, order.subject.position).sumofsquares();
	if (distance < _bible.tileProduceRangeMin() || distance > _bible.tileProduceRangeMax())
	{
		LOGD << "target is out of range, order discarded"
			": " << TypeEncoder(&_bible) << order << " from " << player;
		return discarded(player, order, cset, Notice::ORDERINVALID);
	}

	// Is the tile accessible?
	if (!_bible.tileAccessible(targettile.type))
	{
		LOGV << "tile inaccessible, order discarded";
		return discarded(player, order, cset, Notice::INACCESSIBLE);
	}

	// Is the tile accessible by ground? If not, is the unit an air unit?
	if (!_bible.tileWalkable(targettile.type) && !_bible.unitAir(newtype))
	{
		LOGV << "tile not walkable, order discarded";
		return discarded(player, order, cset, Notice::UNWALKABLE);
	}

	// If the tile has no powered stacks, the order is postponed.
	if (!_bible.powerAbolished() && producer.power <= 0)
	{
		// We show the player that the tile failed to produce anything.
		LOGV << "subject has no power, order postponed";
		return postponed(player, order, cset, Notice::LACKINGPOWER);
	}

	// If the tile is occupied by an enemy unit, the order is postponed.
	if ((_board.ground(at)
			&& _board.ground(at).owner != producer.owner
			&& _bible.unitCanOccupy(_board.ground(at).type))
		|| (_board.air(at)
				&& _board.air(at).owner != producer.owner
				&& _bible.unitCanOccupy(_board.air(at).type)))
	{
		// We show the player that the tile failed to build anything.
		LOGV << "tile occupied by enemy, order postponed";
		return postponed(player, order, cset, Notice::OCCUPIEDBYENEMY);
	}

	// Get a description of the newly trained unit and its destination.
	Descriptor unitdesc(Descriptor::Type::GROUND, order.subject.position);
	Descriptor targetdesc(Descriptor::Type::GROUND, order.target.position);
	if (_bible.unitAir(newtype))
	{
		unitdesc.type = Descriptor::Type::AIR;
		targetdesc.type = Descriptor::Type::AIR;
	}

	// Are we producing in place...
	if (unitdesc.position == targetdesc.position)
	{
		// If the space is occupied by another unit, the order is postponed.
		if (_board.unit(at, unitdesc.type))
		{
			// We show the player that the tile failed to produce anything.
			LOGV << "subject occupied, order postponed";
			return postponed(player, order, cset, Notice::SUBJECTOCCUPIED);
		}
	}
	// ... or are we rallying?
	else
	{
		// Are other units already occupying the spaces involved?
		// Note that ground units do not collide with air units or vice versa.
		const UnitToken& atunit = _board.unit(at, unitdesc.type);
		const UnitToken& tounit = _board.unit(to, unitdesc.type);

		// If the space is occupied by an enemy unit, the order is postponed.
		if (atunit && atunit.owner != producer.owner)
		{
			// We show the player that the tile failed to produce anything.
			LOGV << "subject occupied, order postponed";
			return postponed(player, order, cset, Notice::SUBJECTOCCUPIED);
		}

		// If both spaces are occupied, the order is postponed.
		if (atunit && tounit)
		{
			// We show the player that the tile failed to produce anything.
			LOGV << "target occupied, order postponed";
			return postponed(player, order, cset, Notice::TARGETOCCUPIED,
					order.target);
		}

		// If the producer's space is occupied by a unit, we need to bypass it,
		if (atunit)
		{
			unitdesc.type = Descriptor::Type::BYPASS;
		}
		// If the target space is occupied by a unit, we do not move there.
		else if (tounit)
		{
			targetdesc.position = unitdesc.position;
		}
	}

	// If the player cannot afford the unit cost, the order is postponed.
	if (_money[player] < build.cost)
	{
		// We show the player that the tile failed to produce anything.
		LOGV << "not enough money, order postponed";
		return postponed(player, order, cset, Notice::LACKINGMONEY);
	}

	// Spend the money.
	if (build.cost > 0)
	{
		_money[player] -= build.cost;
		Change moneychange(Change::Type::EXPENDITURE, order.subject,
			producer.owner);
		moneychange.xMoney(-build.cost);
		cset.push(moneychange, Vision::only(player));
	}

	// The size of the new unit is the same as the size of the tile that built it.
	// Unless this number exceeds the maximum number of stacks the unit can have.
	UnitToken unittoken;
	unittoken.type = order.unittype;
	unittoken.owner = producer.owner;
	int size = (_bible.powerAbolished())
		? producer.stacks
		: producer.power;
	unittoken.stacks = std::min(size, (int) _bible.unitStacksMax(newtype));

	// Spend the power.
	Change powerchange(Change::Type::PRODUCES, order.subject);
	if (!_bible.powerAbolished())
	{
		powerchange.xPower(_bible.powerDrainScaled()
			? -std::min((int) unittoken.stacks, (int) producer.power)
			: -1);
	}
	_board.enact(powerchange);
	cset.push(powerchange, _board.vision(at));

	// Create the unit.
	Change spawnchange(Change::Type::PRODUCED, unitdesc, unittoken);
	_board.enact(spawnchange);
	cset.push(spawnchange, _board.vision(at));

	// Update vision.
	VisionTransition(_bible, _board, *this, cset).executeAround(at,
		_bible.unitVision(order.unittype));

	// The unit spawning might trigger lockdown.
	bool lockdown = checkLockdown(at, unitdesc, cset);

	// If we are rallying, move the unit.
	if (targetdesc.position != unitdesc.position && !lockdown)
	{
		doMove(at, to, unitdesc, targetdesc, cset);

		// The produced unit might have been killed in an attack of opportunity.
		if (_board.unit(to, targetdesc.type))
		{
			// The unit moving might trigger lockdown.
			checkLockdown(to, targetdesc, cset);
		}
	}

	// This order was completed.
	LOGV << "order completed";
	return finished(player, order, cset);
}

void Automaton::processHalt(const Player& player, Order& order)
{
	// Declare we are processing this order.
	ChangeSet cset;
	cset.push(Change(Change::Type::ACTING, order.subject),
		Vision::only(player));

	// Find the activeunit based on its description.
	// The Halt order is the only order that can be given both to a tile and
	// to a unit.
	Cell at = _board.cell(order.subject.position);
	if (order.subject.type == Descriptor::Type::TILE)
	{
		const TileTokenWithId& activetile = _board.tile(at);

		// Is the tile the subject of the order?
		if (_activesubjects[activetile.id()] != order.subject)
		{
			LOGV << "subject id does not match, order discarded";
			return discarded(player, order, cset);
		}

		// Are we out of bounds?
		if (!activetile)
		{
			LOGE << "verified subject does not exist, order discarded";
			DEBUG_ASSERT(false && "subject verification is not watertight");
			return discarded(player, order, cset);
		}

		// Is the tile owned by the player?
		if (activetile.owner != player)
		{
			LOGE << "verified subject not controlled by player, order discarded";
			DEBUG_ASSERT(false && "subject verification is not watertight");
			return discarded(player, order, cset);
		}
	}
	else
	{
		const UnitTokenWithId& activeunit = _board.unit(at, order.subject.type);

		// Is the unit the subject of the order?
		if (_activesubjects[activeunit.id()] != order.subject)
		{
			LOGV << "subject id does not match, order discarded";
			return discarded(player, order, cset);
		}

		// Does the unit exist?
		if (!activeunit)
		{
			LOGE << "verified subject does not exist, order discarded";
			DEBUG_ASSERT(false && "subject verification is not watertight");
			return discarded(player, order, cset);
		}

		// Is the unit owned by the player?
		if (activeunit.owner != player)
		{
			LOGE << "verified subject not controlled by player, order discarded";
			DEBUG_ASSERT(false && "subject verification is not watertight");
			return discarded(player, order, cset);
		}
	}

	// We show the player that the tile or unit did indeed stop
	// moving/building/producing. Other players do not get to see this because
	// they are not informed of intentions.
	cset.push(Change(Change::Type::NONE, order.subject, {}, Notice::HALTED),
		Vision::only(player));

	LOGV << "order completed";
	return finished(player, order, cset);
}

void Automaton::finished(const Player& player, Order& order, ChangeSet& changes)
{
	// Other players do not get to see this because they are not informed of intentions.
	changes.push(Change(Change::Type::FINISHED, order.subject), Vision::only(player));
	_changesets.push(changes);
	order = Order();
}

void Automaton::discarded(const Player& player, Order& order, ChangeSet& changes)
{
	// Other players do not get to see this because they are not informed of intentions.
	changes.push(Change(Change::Type::DISCARDED, order.subject), Vision::only(player));
	_changesets.push(changes);
	order = Order();
}

void Automaton::discarded(const Player& player, Order& order, ChangeSet& changes,
	const Notice& notice)
{
	discarded(player, order, changes, notice, Descriptor());
}

void Automaton::discarded(const Player& player, Order& order, ChangeSet& changes,
	const Notice& notice, const Descriptor& target)
{
	// Other players do not get to see this because they are not informed of intentions.
	changes.push(Change(Change::Type::NONE, order.subject, target, notice),
		Vision::only(player));
	changes.push(Change(Change::Type::DISCARDED, order.subject), Vision::only(player));
	_changesets.push(changes);
	order = Order();
}

void Automaton::postponed(const Player& player, const Order& order,
	ChangeSet& changes, const Notice& notice)
{
	postponed(player, order, changes, notice, Descriptor());
}

void Automaton::postponed(const Player& player, const Order& order,
	ChangeSet& changes, const Notice& notice, const Descriptor& target)
{
	// Other players do not get to see this because they are not informed of intentions.
	changes.push(Change(Change::Type::NONE, order.subject, target, notice),
		Vision::only(player));
	changes.push(Change(Change::Type::POSTPONED, order.subject, player, order),
		Vision::only(player));
	_changesets.push(changes);
}

void Automaton::unfinished(const Player& player, const Descriptor& oldsubject,
	const Order& neworder, ChangeSet& changes)
{
	// Other players do not get to see this because they are not informed of intentions.
	changes.push(Change(Change::Type::UNFINISHED, oldsubject, player, neworder),
		Vision::only(player));
	_changesets.push(changes);
}

void Automaton::doMove(Cell from, Cell to,
	const Descriptor& fromdesc, const Descriptor& todesc, ChangeSet& changes)
{
	// Indicate that the unit is about to move.
	Change startschange(Change::Type::STARTS, fromdesc, todesc);
	_board.enact(startschange);
	changes.push(startschange, _board.vision(from));

	// Before we actually move, we check for attacks of opportunity.
	// If there is any combat, it takes place in the same changeset.
	checkAttackOfOpportunity(from, fromdesc, changes);

	// The moving unit might have been killed during the attack of opportunity.
	if (!_board.unit(from, fromdesc.type)) return;

	// Move the unit.
	Change movechange(Change::Type::MOVES, fromdesc, todesc);
	_board.enact(movechange);

	// Get the unit from its current position.
	const UnitToken& movingunit = _board.unit(to, todesc.type);
	UnitType movingunittype = movingunit.type;

	// Update vision.
	// Note that the owner of the moving unit is the only player
	// that can gain or lose vision of certain spaces during this vision
	// transition. Other players' vision does not change, even though the
	// moving might move into or out of their vision.
	// Also note that a circle around point A with radius R + distance(A, B)
	// covers two circles with radius R around point A and around point B.
	// However we work in squared Euclidean distance, so we need to calculate
	// (x + 1)^2 = x^2 + 2x + 1 <= x^2 + (x^2 + 1) + 1 = 2 x^2 + 2 = 2 R + 2.
	// Also see VisionTransition::executeAround for more algebra.
	VisionTransition(_bible, _board, *this, changes).executeAround(to,
		2 * _bible.unitVision(movingunittype) + 2);

	// Some players might see the moving unit appear from the fog of war.
	Vision newsees = _board.vision(to).minus(_board.vision(from));
	if (!newsees.empty())
	{
		// We have the moving unit appear in the fog of war and then walk out of it.
		Change enterchange(Change::Type::ENTERED, fromdesc, movingunit);
		changes.push(enterchange, newsees);
	}

	// All players that have vision of either cell can see the unit moving.
	Vision movesees = _board.vision(from).plus(_board.vision(to));
	changes.push(movechange, movesees);

	// Some players might see the moving unit disappear into the fog of war.
	Vision unsees = _board.vision(from).minus(_board.vision(to));
	if (!unsees.empty())
	{
		// We have the moving unit walk into fog of war and then disappear.
		Change exitchange(Change::Type::EXITED, todesc);
		changes.push(exitchange, unsees);
	}

	// After we move, cold feet wears off.
	checkColdFeetCleanse(from, changes);

	// After we move, we check for trample damage.
	// If there is any trample damage, it takes place in the same changeset.
	checkTrample(to, todesc, changes);

	// Air units can't attack or bypass; instead they crash into enemy units.
	if (todesc.type == Descriptor::Type::BYPASS && _bible.unitAir(movingunit.type))
	{
		// Remember the type of the crashee.
		UnitType crasheetype = _board.air(to).type;

		// The moving air unit dies.
		doUnitDeath(to, movingunit, todesc, changes);

		// The crashee dies.
		doUnitDeath(to, _board.air(to), Descriptor::air(to.pos()), changes);

		// Their owners lose vision.
		VisionTransition(_bible, _board, *this, changes).executeAround(to,
			std::max(_bible.unitVision(movingunittype),
				_bible.unitVision(crasheetype)));
	}
}

void Automaton::doActiveAttack(Cell from, Cell to,
	const Descriptor& attacker, const Descriptor& taker, ChangeSet& changes)
{
	// The attacker aims.
	changes.push(Change(Change::Type::AIMS,
			attacker,
			Descriptor::cell(taker.position),
			Notice::ACTIVEATTACK),
		_board.vision(from));

	// Do a damage step.
	Damage damage(_bible, _board);

	// Declare the attacker.
	damage.addAttacker(attacker, _board.unit(from, attacker.type));

	// Declare the taker.
	damage.addTaker(taker, _board.unit(to, taker.type));

	// Get the unit type of the taker.
	UnitType takerunittype = _board.unit(to, taker.type).type;

	// Declare the background.
	// During attacks, trenches provide extra miss chances unless occupied by a mechanical unit.
	if (_bible.tileTrenches(_board.tile(to).type)
		&& !_bible.unitMechanical(takerunittype))
	{
		damage.addTrenches(Descriptor::tile(to.pos()), _board.tile(to));
	}
	else
	{
		damage.addBackground(Descriptor::tile(to.pos()), _board.tile(to));
	}

	// Resolve damage and unit deaths.
	damage.resolve();
	checkAttackDamage(to, damage, changes);
	bool deaths = checkDeaths(to, changes);

	// Vision might have changes due to units dying.
	if (deaths)
	{
		VisionTransition(_bible, _board, *this, changes).executeAround(to,
			_bible.unitVision(takerunittype));
	}

	// If the defending unit was not killed during the attack and can retaliate, it will.
	const UnitToken& takingunit = _board.unit(to, taker.type);
	if (takingunit && _bible.unitCanAttack(takingunit.type) && !hasColdFeet(to))
	{
		doRetaliationAttack(to, from, taker, attacker, changes);
	}
}

void Automaton::doRetaliationAttack(Cell from, Cell to,
	const Descriptor& attacker, const Descriptor& taker, ChangeSet& changes)
{
	// The attacker aims.
	changes.push(Change(Change::Type::AIMS,
			attacker,
			Descriptor::cell(taker.position),
			Notice::RETALIATIONATTACK),
		_board.vision(from));

	// Do a damage step.
	Damage damage(_bible, _board);

	// Declare the attacker.
	damage.addAttacker(attacker, _board.unit(from, attacker.type));

	// Declare the taker.
	damage.addTaker(taker, _board.unit(to, taker.type));

	// Declare a bypassed background unit, if any.
	// Entrenched units do not provide a background.
	bool bypassdefense = (taker.type == Descriptor::Type::BYPASS
			&& (!_bible.tileTrenches(_board.tile(to).type)
				|| !_bible.trenchesHideBypassedUnit()
				|| _bible.unitMechanical(_board.ground(to).type)));
	if (bypassdefense)
	{
		damage.addBackground(Descriptor::ground(to.pos()), _board.ground(to));
	}

	// Remember the unit type of the taker.
	UnitType takerunittype = _board.unit(to, taker.type).type;
	UnitType otherunittype = (bypassdefense)
		? _board.ground(to).type : UnitType::NONE;

	// Declare the background.
	// During attacks, trenches provide extra miss chances unless occupied
	// by a mechanical unit.
	// During bypass attacks, trenches don't provide extra miss chances.
	if (_bible.tileTrenches(_board.tile(to).type)
		&& !_bible.unitMechanical(takerunittype)
		&& (taker.type != Descriptor::Type::BYPASS
			|| (_bible.trenchesProtectBypassingUnits()
				&& !_bible.unitMechanical(otherunittype))))
	{
		damage.addTrenches(Descriptor::tile(to.pos()), _board.tile(to));
	}
	else
	{
		damage.addBackground(Descriptor::tile(to.pos()), _board.tile(to));
	}

	// Resolve damage and unit deaths.
	damage.resolve();
	checkAttackDamage(to, damage, changes);
	bool deaths = checkDeaths(to, changes);

	// Vision might have changes due to units dying.
	if (deaths)
	{
		VisionTransition(_bible, _board, *this, changes).executeAround(to,
			std::max(_bible.unitVision(takerunittype),
				_bible.unitVision(otherunittype)));
	}
}


void Automaton::doFocussedAttack(Cell from, Cell to,
	const Descriptor& attacker, const Descriptor& taker, ChangeSet& changes)
{
	// The attacker aims.
	changes.push(Change(Change::Type::AIMS,
			attacker,
			Descriptor::cell(taker.position),
			Notice::FOCUSATTACK),
		_board.vision(from));

	// Do a damage step.
	Damage damage(_bible, _board);

	// Declare the attacker.
	const UnitToken& focussingunit = _board.unit(from, attacker.type);
	damage.addAttacker(attacker, focussingunit);

	// Look for additional attackers within the four adjacent cells.
	for (Cell at : _board.area(to, 1, 1))
	{
		// Skip the attacker we already added.
		if (at == from) continue;

		// Get the ground unit of that tile (only ground units can attack).
		const UnitToken& unit = _board.ground(at);
		if (!unit || !_bible.unitCanAttack(unit.type)
			|| hasColdFeet(at)) continue;

		// Only units belonging to the player that gave the order respond to it.
		if (unit.owner != focussingunit.owner) continue;

		// The attacker aims.
		changes.push(Change(Change::Type::AIMS,
				Descriptor::ground(at.pos()),
				Descriptor::cell(taker.position),
				Notice::TRIGGEREDFOCUSATTACK),
			_board.vision(from));

		// Add it as an attacker.
		Descriptor triggeredattacker = Descriptor::ground(at.pos());
		damage.addAttacker(triggeredattacker, unit);
	}

	// Remember the taker type.
	UnitType takerunittype = _board.unit(to, taker.type).type;

	// Declare the unit type of the taker.
	damage.addTaker(taker, _board.unit(to, taker.type));

	// Declare the background.
	// During attacks, trenches provide extra miss chances unless occupied by a mechanical unit.
	if (_bible.tileTrenches(_board.tile(to).type)
		&& !_bible.unitMechanical(_board.unit(to, taker.type).type))
	{
		damage.addTrenches(Descriptor::tile(to.pos()), _board.tile(to));
	}
	else
	{
		damage.addBackground(Descriptor::tile(to.pos()), _board.tile(to));
	}

	// Resolve damage and unit deaths.
	damage.resolve();
	checkAttackDamage(to, damage, changes);
	bool deaths = checkDeaths(to, changes);

	// Vision might have changes due to units dying.
	if (deaths)
	{
		VisionTransition(_bible, _board, *this, changes).executeAround(to,
			_bible.unitVision(takerunittype));
	}

	// If the defending unit was not killed during the attack and can retaliate, it will.
	const UnitToken& takingunit = _board.unit(to, taker.type);
	if (takingunit && _bible.unitCanAttack(takingunit.type) && !hasColdFeet(to))
	{
		doRetaliationAttack(to, from, taker, attacker, changes);
	}
}

void Automaton::checkAttackOfOpportunity(Cell at,
		const Descriptor& mover, ChangeSet& changes)
{
	// Bypassing a friendly unit protects you from attacks of opportunity.
	// This is mostly because it would be visually confusing,
	// but it also makes sense lorewise.
	if (mover.type == Descriptor::Type::BYPASS) return;

	// Air units cannot be attacked.
	if (mover.type == Descriptor::Type::AIR) return;

	// Get the moving unit.
	const UnitToken& movingunit = _board.unit(at, mover.type);
	UnitType movingunittype = movingunit.type;

	// Start a damage step.
	Damage damage(_bible, _board);

	// Look for possible attackers within the four adjacent cells.
	for (Cell from : _board.area(at, 1, 1))
	{
		// Get the ground unit of that tile (only ground units can attack).
		const UnitToken& unit = _board.ground(from);
		if (!unit || !_bible.unitCanAttack(unit.type)
			|| hasColdFeet(from)) continue;
		if (unit.owner == movingunit.owner) continue;

		// The attacker aims.
		changes.push(Change(Change::Type::AIMS,
				Descriptor::ground(from.pos()),
				Descriptor::cell(mover.position),
				Notice::OPPORTUNITYATTACK),
			_board.vision(from));

		// If it can attack, add it as an attacker.
		Descriptor attacker = Descriptor::ground(from.pos());
		damage.addAttacker(attacker, unit);
	}

	// If there were no attackers, there is no attack of opportunity.
	if (!damage.shots()) return;

	// The moving unit is the defender.
	damage.addTaker(mover, movingunit);

	// Declare the background.
	// Trenches no longer provide extra miss chances to moving units.
	if (_bible.trenchesProtectMovingUnits()
		&& _bible.tileTrenches(_board.tile(at).type)
		&& !_bible.unitMechanical(movingunittype))
	{
		damage.addTrenches(Descriptor::tile(at.pos()), _board.tile(at));
	}
	else
	{
		damage.addBackground(Descriptor::tile(at.pos()), _board.tile(at));
	}

	// Resolve damage and unit deaths.
	damage.resolve();
	checkAttackDamage(at, damage, changes);
	bool deaths = checkDeaths(at, changes);

	// Vision might have changes due to units dying.
	if (deaths)
	{
		VisionTransition(_bible, _board, *this, changes).executeAround(at,
			_bible.unitVision(movingunittype));
	}
}

bool Automaton::checkLockdown(Cell at,
		const Descriptor& mover, ChangeSet& changes)
{
	// Bypassing a friendly unit protects you from lockdown.
	// This is mostly because it would be visually confusing,
	// but it also makes sense lorewise.
	if (mover.type == Descriptor::Type::BYPASS) return false;

	// Air units cannot be attacked, hence cannot be locked down.
	if (mover.type == Descriptor::Type::AIR) return false;

	// Get the moving unit.
	DEBUG_ASSERT(mover.type == Descriptor::Type::GROUND);
	const UnitToken& movingunit = _board.unit(at, mover.type);
	UnitType movingunittype = movingunit.type;

	// Start a damage step.
	Damage damage(_bible, _board);

	// Look for possible attackers within the four adjacent cells.
	bool lockdown = false;
	for (const auto& ist : _lockdowns)
	{
		if (std::get<2>(ist) != at) continue;

		uint32_t identifier = std::get<0>(ist);
		Cell from = std::get<1>(ist);

		// Get the ground unit of that tile (only ground units can attack).
		const UnitTokenWithId& unit = _board.ground(from);
		if (!unit) continue;
		if (unit.id() != identifier) continue;
		if (unit.owner == movingunit.owner) continue;

		// Lockdown has occurred.
		lockdown = true;

		// The lockdown unit may or may not also be able to attack.
		if (!_bible.unitCanAttack(unit.type) || hasColdFeet(from)) continue;

		// The attacker aims.
		changes.push(Change(Change::Type::AIMS,
				Descriptor::ground(from.pos()),
				Descriptor::cell(mover.position),
				Notice::TRIGGEREDLOCKDOWNATTACK),
			_board.vision(from));

		// If it can attack, add it as an attacker.
		Descriptor attacker = Descriptor::ground(from.pos());
		damage.addAttacker(attacker, unit);
	}

	// If there were no attackers, lockdown might have still occurred.
	if (!damage.shots()) return lockdown;

	// The moving unit is the defender.
	damage.addTaker(mover, movingunit);

	// Declare the background.
	// Trenches no longer provide extra miss chances to moving units.
	if (_bible.trenchesProtectMovingUnits()
		&& _bible.tileTrenches(_board.tile(at).type)
		&& !_bible.unitMechanical(movingunittype))
	{
		damage.addTrenches(Descriptor::tile(at.pos()), _board.tile(at));
	}
	else
	{
		damage.addBackground(Descriptor::tile(at.pos()), _board.tile(at));
	}

	// Resolve damage and unit deaths.
	damage.resolve();
	checkAttackDamage(at, damage, changes);
	bool deaths = checkDeaths(at, changes);

	// Vision might have changes due to units dying.
	if (deaths)
	{
		VisionTransition(_bible, _board, *this, changes).executeAround(at,
			_bible.unitVision(movingunittype));
	}

	return lockdown;
}

void Automaton::checkTrample(Cell at, const Descriptor& mover,
		ChangeSet& changes)
{
	const UnitToken& movingunit = _board.unit(at, mover.type);
	TileType tiletype = _board.tile(at).type;

	// If the unit deals no trample damage, there is no trample attack.
	if (_bible.unitTrampleShots(movingunit.type) == 0) return;

	// Start a damage step.
	Damage damage(_bible, _board);

	// The moving unit is the defender.
	damage.addTrampler(mover, movingunit);

	// If the unit deals no trample damage, there is no trample attack.
	if (!damage.shots()) return;

	// Declare the background.
	damage.addTaker(Descriptor::tile(at.pos()), _board.tile(at));

	// Resolve damage and deaths.
	damage.resolve();
	checkTrampleDamage(at, damage, changes);
	bool deaths = checkDeaths(at, changes);

	// Vision might have changes due to units dying.
	if (deaths)
	{
		VisionTransition(_bible, _board, *this, changes).executeAround(at,
			_bible.tileVision(tiletype));
	}
}

void Automaton::doShelling(Cell from, Cell to,
	const Descriptor& sheller, ChangeSet& changes)
{
	// Do a damage step.
	Damage damage(_bible, _board);

	// Declare the sheller.
	const UnitToken& shellingunit = _board.unit(from, sheller.type);
	damage.addCaster(sheller, shellingunit);

	// Declare takers.
	if (_board.tile(to))   damage.addTaker(Descriptor::tile(  to.pos()), _board.tile(to));
	if (_board.ground(to)) damage.addTaker(Descriptor::ground(to.pos()), _board.ground(to));
	if (_board.air(to))    damage.addTaker(Descriptor::air(   to.pos()), _board.air(to));

	// Remember the types.
	TileType tiletype = _board.tile(to).type;
	UnitType groundtype = _board.ground(to).type;
	UnitType airtype = _board.air(to).type;

	// Resolve damage and possible unit and tile deaths.
	damage.resolve();
	checkShellDamage(to, damage, changes);
	bool deaths = checkDeaths(to, changes);

	// Create gas and radiation, if any.
	int gasleak = _bible.unitAbilityGas( shellingunit.type);
	int radleak = _bible.unitAbilityRads(shellingunit.type);
	doLeak(to, gasleak, radleak, changes);

	// Vision might have changes due to units dying.
	if (deaths)
	{
		VisionTransition(_bible, _board, *this, changes).executeAround(to,
			std::max(_bible.tileVision(tiletype), std::max(
				_bible.unitVision(groundtype), _bible.unitVision(airtype))));
	}
}

void Automaton::doBombardment(Cell from, Cell to,
	const Descriptor& bombarder, ChangeSet& changes)
{
	const UnitToken& bombardingunit = _board.unit(from, bombarder.type);
	int radius = _bible.unitAbilityRadius(bombardingunit.type);
	if (radius > 0)
	{
		for (Cell pos : _board.area(to, 0, radius))
		{
			doBombardment1(from, pos, bombarder, changes);
		}
	}
	else
	{
		doBombardment1(from, to, bombarder, changes);
	}

	// Vision might have changes due to units dying.
	VisionTransition(_bible, _board, *this, changes).executeAround(to,
		radius + _bible.unitVisionMax());
}

void Automaton::doBombardment1(Cell from, Cell to,
	const Descriptor& bombarder, ChangeSet& changes)
{
	// Do a damage step.
	Damage damage(_bible, _board);

	// Declare the bombarder.
	const UnitToken& bombardingunit = _board.unit(from, bombarder.type);
	damage.addCaster(bombarder, bombardingunit);

	// Declare takers.
	if (_board.tile(to))   damage.addTaker(Descriptor::tile(  to.pos()), _board.tile(to));
	if (_board.ground(to)) damage.addTaker(Descriptor::ground(to.pos()), _board.ground(to));
	if (_board.air(to))    damage.addTaker(Descriptor::air(   to.pos()), _board.air(to));

	// Resolve damage and possible unit and tile deaths.
	damage.resolve();
	checkBombardDamage(to, damage, changes);
	checkDeaths(to, changes);

	// Create gas and radiation, if any.
	int gasleak = _bible.unitAbilityGas( bombardingunit.type);
	int radleak = _bible.unitAbilityRads(bombardingunit.type);
	doLeak(to, gasleak, radleak, changes);
}

void Automaton::doBombing(Cell from, Cell to,
	const Descriptor& bomber, ChangeSet& changes)
{
	const UnitToken& bombingunit = _board.unit(from, bomber.type);
	int radius = _bible.unitAbilityRadius(bombingunit.type);
	if (radius > 0)
	{
		for (Cell pos : _board.area(to, 0, radius))
		{
			doBombing1(from, pos, bomber, changes);
		}
	}
	else
	{
		doBombing1(from, to, bomber, changes);
	}

	// Vision might have changes due to units dying.
	VisionTransition(_bible, _board, *this, changes).executeAround(to,
		radius + _bible.unitVisionMax());
}

void Automaton::doBombing1(Cell from, Cell to,
	const Descriptor& bomber, ChangeSet& changes)
{
	// Do a damage step.
	Damage damage(_bible, _board);

	// Declare the bombarder.
	const UnitToken& bombingunit = _board.unit(from, bomber.type);
	damage.addCaster(bomber, bombingunit);

	// Declare takers.
	if (_board.tile(to))   damage.addTaker(Descriptor::tile(  to.pos()), _board.tile(to));
	if (_board.ground(to)) damage.addTaker(Descriptor::ground(to.pos()), _board.ground(to));

	// Resolve damage and possible unit and tile deaths.
	damage.resolve();
	checkBombDamage(to, damage, changes);
	checkDeaths(to, changes);

	// Create gas and radiation, if any.
	int gasleak = _bible.unitAbilityGas( bombingunit.type);
	int radleak = _bible.unitAbilityRads(bombingunit.type);
	doLeak(to, gasleak, radleak, changes);
}

void Automaton::checkAttackDamage(Cell at, const Damage& damage,
		ChangeSet& changes)
{
	for (const Damage::Hit& hit : damage)
	{
		const Damage::Shot& shot = hit.shot;
		Attacker attacker(shot.unit->type, shot.desc.position);
		const Damage::Body& body = hit.body;

		// The ATTACKS change tells a player that a unit attacks something else.
		// The location, target and figure number of the attacker is known, but nothing else.
		{
			Cell index = _board.cell(shot.desc.position);
			Change change(Change::Type::ATTACKS, shot.desc, body.desc, shot.figure);
			_board.enact(change);
			changes.push(change, _board.vision(index));
		}

		// The ATTACKED change tells a player that a unit was attacked by some Attacker.
		// The location and unittype of the Attacker is known, but not its owner or its stacks.
		if (shot.damage > 0)
		{
			Change change(Change::Type::ATTACKED, body.desc, attacker, body.figure,
				hit.killing, hit.depowering);
			_board.enact(change);
			changes.push(change, _board.vision(at));
		}
	}
}

void Automaton::checkTrampleDamage(Cell at, const Damage& damage,
		ChangeSet& changes)
{
	for (const Damage::Hit& hit : damage)
	{
		const Damage::Shot& shot = hit.shot;
		Bombarder bombarder(shot.unit->type);
		const Damage::Body& body = hit.body;

		// The TRAMPLES change tells a player that a unit tramples something else.
		{
			Cell index = _board.cell(shot.desc.position);
			Change change(Change::Type::TRAMPLES, shot.desc, shot.figure);
			_board.enact(change);
			changes.push(change, _board.vision(index));
		}

		// The TRAMPLED change tells a player that a unit was trampled by some Bombarder.
		// The unittype of the Bombarder is specified but the rest of the data is clear.
		if (shot.damage > 0)
		{
			Change change(Change::Type::TRAMPLED, body.desc, bombarder, body.figure,
				hit.killing, hit.depowering);
			_board.enact(change);
			changes.push(change, _board.vision(at));
		}
	}
}

void Automaton::checkShellDamage(Cell at, const Damage& damage,
		ChangeSet& changes)
{
	for (const Damage::Hit& hit : damage)
	{
		const Damage::Shot& shot = hit.shot;
		Attacker attacker(shot.unit->type, shot.desc.position);
		const Damage::Body& body = hit.body;

		// The SHELLS change tells a player that a unit shells something else.
		// The location, target and figure number of the sheller is known, but nothing else.
		{
			Cell index = _board.cell(shot.desc.position);
			Change change(Change::Type::SHELLS, shot.desc, body.desc, shot.figure);
			_board.enact(change);
			changes.push(change, _board.vision(index));
		}

		// The SHELLED change tells a player that a unit was shelled by some Attacker.
		// The location and unittype of the Attacker is known, but not its owner or its stacks.
		if (shot.damage > 0)
		{
			Change change(Change::Type::SHELLED, body.desc, attacker, body.figure,
				hit.killing, hit.depowering);
			_board.enact(change);
			changes.push(change, _board.vision(at));
		}
	}
}

void Automaton::checkBombardDamage(Cell at, const Damage& damage,
		ChangeSet& changes)
{
	for (const Damage::Hit& hit : damage)
	{
		const Damage::Shot& shot = hit.shot;
		Bombarder bombarder(shot.unit->type);
		const Damage::Body& body = hit.body;

		// The BOMBARDS change tells a player that a unit bombards something else.
		// The location and figure number of the bombarder are known to players that have
		// vision of the bombarder, but nothing else. The bombarded need not have vision.
		{
			Cell index = _board.cell(shot.desc.position);
			Change change(Change::Type::BOMBARDS, shot.desc, shot.figure);
			_board.enact(change);
			changes.push(change, _board.vision(index));
		}

		// The BOMBARDED change tells a player that a unit was bombarded by some Bombarder.
		// The unittype of the Bombarder is known, but not its location, owner or stacks.
		if (shot.damage > 0)
		{
			Change change(Change::Type::BOMBARDED, body.desc, bombarder, body.figure,
				hit.killing, hit.depowering);
			_board.enact(change);
			changes.push(change, _board.vision(at));
		}
	}
}

void Automaton::checkBombDamage(Cell at, const Damage& damage,
		ChangeSet& changes)
{
	for (const Damage::Hit& hit : damage)
	{
		const Damage::Shot& shot = hit.shot;
		Bombarder bombarder(shot.unit->type);
		const Damage::Body& body = hit.body;

		// The BOMBS change tells a player that a unit bombs something else.
		{
			Cell index = _board.cell(shot.desc.position);
			Change change(Change::Type::BOMBS, shot.desc, shot.figure);
			_board.enact(change);
			changes.push(change, _board.vision(index));
		}

		// The BOMBED change tells a player that a unit was bombed by some Bombarder.
		// The unittype of the Bombarder is specified but the rest of the data is clear.
		if (shot.damage > 0)
		{
			Change change(Change::Type::BOMBED, body.desc, bombarder, body.figure,
				hit.killing, hit.depowering);
			_board.enact(change);
			changes.push(change, _board.vision(at));
		}
	}
}

bool Automaton::checkDeaths(Cell at, ChangeSet& changes)
{
	int deaths = 0;
	deaths += checkUnitDeath(at, Descriptor::ground(at.pos()), changes);
	deaths += checkUnitDeath(at, Descriptor::bypass(at.pos()), changes);
	deaths += checkUnitDeath(at, Descriptor::air(at.pos()), changes);
	deaths += checkTileDeath(at, Descriptor::tile(at.pos()), changes);
	return (deaths > 0);
}

bool Automaton::checkUnitDeath(Cell at, const Descriptor& desc,
		ChangeSet& changes)
{
	// Because checkXyzDamage() was called immediately before,
	// the unit's stacks might be 0.
	const UnitToken& unit = _board.unit(at, desc.type);

	// Is there a unit?
	if (!unit) return false;

	// Did the unit survive?
	if (unit.stacks > 0)
	{
		Change survivechange(Change::Type::SURVIVED, desc);
		_board.enact(survivechange);
		changes.push(survivechange, _board.vision(at));
		return false;
	}

	doUnitDeath(at, unit, desc, changes);
	return true;
}

void Automaton::doUnitDeath(Cell at, const UnitToken& unit,
		const Descriptor& desc, ChangeSet& changes)
{
	// Remember the type because we will destroy the unit.
	UnitType oldtype = unit.type;
	Player oldowner = unit.owner;

	// Remove the unit.
	Change deathchange(Change::Type::DIED, desc);
	_board.enact(deathchange);
	changes.push(deathchange, _board.vision(at));

	// Check cleanses.
	checkColdFeetCleanse(at, changes);

	// Some units release gas or radation on destruction.
	checkUnitDeathLeak(at, oldtype, changes);

	// Check for morale boost.
	checkUnitKillMoraleGain(at, oldtype, oldowner, changes);
}

bool Automaton::checkTileDeath(Cell at, const Descriptor& desc,
		ChangeSet& changes)
{
	// Because checkXyzDamage() was called immediately before,
	// the tile's stacks might be 0.
	const TileToken& tile = _board.tile(at);

	// Is there a tile (I hope so)?
	if (!tile) return false;

	// Only some types of tiles can be destroyed.
	if (!_bible.tileDestructible(tile.type)) return false;

	// Did the tile survive?
	if (tile.stacks > 0)
	{
		Change survivechange(Change::Type::SURVIVED, desc);
		_board.enact(survivechange);
		changes.push(survivechange, _board.vision(at));
		return false;
	}

	doTileDeath(at, tile, desc, changes);
	return true;
}

void Automaton::doTileDeath(Cell at, const TileToken& tile,
		const Descriptor& desc, ChangeSet& changes)
{
	// Only some types of tiles can be destroyed.
	if (!_bible.tileDestructible(tile.type)) return;

	// Remember the type because we will destroy the tile.
	TileType oldtype = tile.type;
	Player oldowner = tile.owner;

	// Destroy the tile.
	TileToken destroyed;
	destroyed.type = _bible.tileDestroyed(tile.type);
	Change destructchange(Change::Type::DESTROYED, desc, destroyed);
	_board.enact(destructchange);
	changes.push(destructchange, _board.vision(at));

	// Some tiles release gas or radation on destruction.
	checkTileDeathLeak(at, oldtype, changes);

	// Check for morale boost.
	checkTileDestroyMoraleGain(at, oldtype, oldowner, changes);
}

void Automaton::checkUnitDeathLeak(Cell at, const UnitType& unittype,
		ChangeSet& changes)
{
	int gasleak = _bible.unitLeakGas(unittype);
	int radleak = _bible.unitLeakRads(unittype);
	doLeak(at, gasleak, radleak, changes);
}

void Automaton::checkTileDeathLeak(Cell at, const TileType& tiletype,
		ChangeSet& changes)
{
	int gasleak = _bible.tileLeakGas(tiletype);
	int radleak = _bible.tileLeakRads(tiletype);
	doLeak(at, gasleak, radleak, changes);
}

void Automaton::doLeak(Cell at, int gasleak, int radleak,
		ChangeSet& changes)
{
	if (gasleak)
	{
		if (_board.gas(at) < std::min(gasleak, (int) _bible.gasMax()))
		{
			int increase = gasleak - _board.gas(at);
			Change change(Change::Type::GAS, Descriptor::cell(at.pos()));
			change.xGas(increase);
			_board.enact(change);
			changes.push(change, _board.vision(at));
		}
	}

	if (radleak)
	{
		if (_board.radiation(at) < std::min(radleak, (int) _bible.radiationMax()))
		{
			int increase = radleak - _board.radiation(at);
			Change change(Change::Type::RADIATION, Descriptor::cell(at.pos()));
			change.xRadiation(increase);
			_board.enact(change);
			changes.push(change, _board.vision(at));
		}
	}
}

void Automaton::checkTileBuildMoraleGain(Cell at, ChangeSet& cset)
{
	const TileToken& citytile = _board.tile(at);
	if (citytile.owner == Player::NONE) return;

	{
		int gain = _bible.tileMoraleGainWhenBuilt(citytile.type);
		if (gain)
		{
			// Generate the income.
			_money[citytile.owner] += gain;
			Change moralechange(Change::Type::INCOME,
				Descriptor::tile(at.pos()),
				citytile.owner);
			moralechange.xMoney(gain);
			cset.push(moralechange, Vision::only(citytile.owner));
		}
	}

	if (!_bible.moraleGatheredWhenBuilt()) return;
	if (!_bible.tileGathersMorale(citytile.type)) return;

	for (Cell neighbor : _board.area(at, 1, 2))
	{
		const TileToken& grasstile = _board.tile(neighbor);
		int gain = _bible.tileMoraleGainWhenGathered(grasstile.type);
		if (!gain) continue;

		// Generate the income.
		_money[citytile.owner] += gain;
		Change moralechange(Change::Type::INCOME,
			Descriptor::tile(neighbor.pos()),
			citytile.owner);
		moralechange.xMoney(gain);
		cset.push(moralechange, Vision::only(citytile.owner));
	}
}

void Automaton::checkTileCaptureMoraleGain(Cell at,
		Player oldowner, Player activeplayer, ChangeSet& cset)
{
	const TileToken& citytile = _board.tile(at);

	if (oldowner != Player::NONE)
	{
		int gain = _bible.tileMoraleGainWhenLost(citytile.type);
		if (gain)
		{
			// Generate the income.
			_money[oldowner] += gain;
			Change moralechange(Change::Type::INCOME,
				Descriptor::cell(at.pos()),
				oldowner);
			moralechange.xMoney(gain);
			cset.push(moralechange, Vision::only(oldowner));
		}
	}

	if (activeplayer != Player::NONE)
	{
		int gain = _bible.tileMoraleGainWhenCaptured(citytile.type);
		if (gain)
		{
			// Generate the income.
			_money[activeplayer] += gain;
			Change moralechange(Change::Type::INCOME,
				Descriptor::cell(at.pos()),
				activeplayer);
			moralechange.xMoney(gain);
			cset.push(moralechange, Vision::only(activeplayer));
		}
	}
}

void Automaton::checkTileDestroyMoraleGain(Cell at, TileType oldtype,
		Player oldowner, ChangeSet& cset)
{
	if (oldowner != Player::NONE)
	{
		int gain = _bible.tileMoraleGainWhenDestroyed(oldtype);
		if (gain)
		{
			// Generate the income.
			_money[oldowner] += gain;
			Change moralechange(Change::Type::INCOME,
				Descriptor::cell(at.pos()),
				oldowner);
			moralechange.xMoney(gain);
			cset.push(moralechange, Vision::only(oldowner));
		}
	}

	{
		int gain = _bible.tileMoraleGainWhenRazed(oldtype);
		if (gain)
		{
			// Everyone who can see this tile getting razed may gain morale.
			for (Player player : _players)
			{
				if (player == oldowner) continue;
				if (!_board.vision(at).contains(player)) continue;

				// Generate the income.
				_money[player] += gain;
				Change moralechange(Change::Type::INCOME,
					Descriptor::cell(at.pos()),
					player);
				moralechange.xMoney(gain);
				cset.push(moralechange, Vision::only(player));
			}
		}
	}
}

void Automaton::checkUnitKillMoraleGain(Cell at, UnitType oldtype,
		Player oldowner, ChangeSet& cset)
{
	if (oldowner != Player::NONE)
	{
		int gain = _bible.unitMoraleGainWhenLost(oldtype);
		if (gain)
		{
			// Generate the income.
			_money[oldowner] += gain;
			Change moralechange(Change::Type::INCOME,
				Descriptor::cell(at.pos()),
				oldowner);
			moralechange.xMoney(gain);
			cset.push(moralechange, Vision::only(oldowner));
		}
	}

	{
		int gain = _bible.unitMoraleGainWhenKilled(oldtype);
		if (gain)
		{
			// Everyone who can see this tile getting razed may gain morale.
			for (Player player : _players)
			{
				if (player == oldowner) continue;
				if (!_board.vision(at).contains(player)) continue;

				// Generate the income.
				_money[player] += gain;
				Change moralechange(Change::Type::INCOME,
					Descriptor::cell(at.pos()),
					player);
				moralechange.xMoney(gain);
				cset.push(moralechange, Vision::only(player));
			}
		}
	}
}

void Automaton::checkTileBuildCleanse(Cell at, ChangeSet& changes)
{
	if (!_bible.markersChangeAfterBuild()) return;

	if (_bible.planeBasedFrostbite())
	{
		// In particular digging Trenches on a plane tile to protect your gunner
		// is something that you want to do.
		if (_board.frostbite(at)
			&& !_bible.tilePlane(_board.tile(at).type))
		{
			Change change(Change::Type::FROSTBITE,
				Descriptor::cell(at.pos()));
			change.xFrostbite(false);
			_board.enact(change);
			changes.push(change, _board.vision(at));
		}
	}

	if (_bible.flammableBasedFirestorm())
	{
		// Even though firestorm only prefers flammable tiles and can occur on
		// nonflammables, it feels bad if you pave over a burning forest and
		// your newly built industry gets destroyed.
		if (_board.firestorm(at)
			&& !_bible.tileFlammable(_board.tile(at).type))
		{
			Change change(Change::Type::FIRESTORM,
				Descriptor::cell(at.pos()));
			change.xFirestorm(false);
			_board.enact(change);
			changes.push(change, _board.vision(at));
		}
	}
}

void Automaton::doDeathEffect(Cell at)
{
	// If nothing can die, this effect can be skipped.
	if (! ((_board.tile(at) && _bible.tileDestructible(_board.tile(at).type))
		||  _board.ground(at)
		||  _board.air(at)))
	{
		return;
	}

	// Create the changeset.
	ChangeSet cset;

	// Remember the types.
	TileType tiletype = _board.tile(at).type;
	UnitType groundtype = _board.ground(at).type;
	UnitType airtype = _board.air(at).type;

	// Kill everything.
	if (_board.tile(at))   doTileDeath(at, _board.tile(at),   Descriptor::tile(  at.pos()), cset);
	if (_board.ground(at)) doUnitDeath(at, _board.ground(at), Descriptor::ground(at.pos()), cset);
	if (_board.air(at))    doUnitDeath(at, _board.air(at),    Descriptor::air(   at.pos()), cset);

	// Vision might have changes due to units dying.
	VisionTransition(_bible, _board, *this, cset).executeAround(at,
		std::max(_bible.tileVision(tiletype), std::max(
			_bible.unitVision(groundtype), _bible.unitVision(airtype))));

	_changesets.push(cset);
}

void Automaton::doFrostbiteEffect(Cell at)
{
	if (_bible.frostbiteGivesColdFeet()
		&& _board.frostbite(at) && _board.ground(at)
		// Frostbite's Cold Feet is applied in Winter, not in Spring.
		&& _bible.chaosMinFrostbite(_season) >= 0)
	{
		ChangeSet cset;
		for (int figure = 0; figure < _board.ground(at).stacks; figure++)
		{
			// The FROSTBITTEN change tells a player that a unit was bitten
			// by frostbite.
			Change change(Change::Type::FROSTBITTEN,
				Descriptor::ground(at.pos()),
				figure, /*killing=*/false, /*depowering=*/false);
			_board.enact(change);
			cset.push(change, _board.vision(at));
		}
		_changesets.push(cset);
	}

	// Is there enough frostbite to do damage?
	if ((_board.frostbite(at) ? 1 : 0) < _bible.frostbiteThresholdDamage()) return;

	// Frostbite no longer affects tiles or air.
	bool hittiles = !_bible.frostbiteOnlyTargetsGroundUnits();
	bool hitair = !_bible.frostbiteOnlyTargetsGroundUnits();

	// If nothing can take damage, this effect can be skipped.
	if (! ((hittiles && _board.tile(at) && _bible.tileDestructible(_board.tile(at).type))
		|| _board.ground(at))
		|| (hitair && _board.air(at)))
	{
		return;
	}

	// Declare a damage step.
	Damage damage(_bible, _board);

	// Declare takers.
	if (_board.tile(at) && hittiles)
	{
		damage.addTaker(Descriptor::tile(  at.pos()), _board.tile(at));
	}
	if (_board.ground(at))
	{
		damage.addTaker(Descriptor::ground(at.pos()), _board.ground(at));
	}
	if (_board.air(at) && hitair)
	{
		damage.addTaker(Descriptor::air(   at.pos()), _board.air(at));
	}

	// If there are no bodies, this effect can be skipped.
	if (!damage.bodies()) return;

	// Declare damage effect.
	Descriptor celldesc = Descriptor::cell(at.pos());
	damage.addEffect(celldesc, _bible.frostbiteShots(), _bible.frostbiteDamage());

	// Remember the types.
	TileType tiletype = hittiles ? _board.tile(at).type : TileType::NONE;
	UnitType groundtype = _board.ground(at).type;
	UnitType airtype = hitair ? _board.air(at).type : UnitType::NONE;

	// Resolve damage.
	damage.resolve();

	// Handle changes.
	ChangeSet cset;
	for (const Damage::Hit& hit : damage)
	{
		const Damage::Body& body = hit.body;

		// The FROSTBITTEN change tells a player that a unit was bitten by frostbite.
		Change change(Change::Type::FROSTBITTEN, body.desc, body.figure,
				hit.killing, hit.depowering);
		_board.enact(change);
		cset.push(change, _board.vision(at));
	}

	// Check for unit deaths.
	if (_board.tile(at) && hittiles) checkTileDeath(at, Descriptor::tile(  at.pos()), cset);
	if (_board.ground(at))           checkUnitDeath(at, Descriptor::ground(at.pos()), cset);
	if (_board.air(at) && hitair)    checkUnitDeath(at, Descriptor::air(   at.pos()), cset);

	// Vision might have changes due to units dying.
	VisionTransition(_bible, _board, *this, cset).executeAround(at,
		std::max(_bible.tileVision(tiletype), std::max(
			_bible.unitVision(groundtype), _bible.unitVision(airtype))));

	_changesets.push(cset);
}

void Automaton::doFirestormEffect(Cell at)
{
	// If nothing can take damage, this effect can be skipped.
	if (! ((_board.tile(at) && _bible.tileDestructible(_board.tile(at).type))
		||  _board.ground(at)
		||  _board.air(at)))
	{
		return;
	}

	// Declare a damage step.
	Damage damage(_bible, _board);

	// Declare takers.
	if (_board.tile(at))   damage.addTaker(Descriptor::tile(  at.pos()), _board.tile(at));
	if (_board.ground(at)) damage.addTaker(Descriptor::ground(at.pos()), _board.ground(at));
	if (_board.air(at))    damage.addTaker(Descriptor::air(   at.pos()), _board.air(at));

	// If there are no bodies, this effect can be skipped.
	if (!damage.bodies()) return;

	// Declare damage effect.
	Descriptor celldesc = Descriptor::cell(at.pos());
	damage.addEffect(celldesc, _bible.firestormShots(), _bible.firestormDamage());

	// Remember the types.
	TileType tiletype = _board.tile(at).type;
	UnitType groundtype = _board.ground(at).type;
	UnitType airtype = _board.air(at).type;

	// Resolve damage.
	damage.resolve();

	// Handle changes.
	ChangeSet cset;
	for (const Damage::Hit& hit : damage)
	{
		const Damage::Body& body = hit.body;

		// The BURNED change tells a player that a unit was burned by firestorm.
		Change change(Change::Type::BURNED, body.desc, body.figure,
				hit.killing, hit.depowering);
		_board.enact(change);
		cset.push(change, _board.vision(at));
	}

	// Check for unit deaths.
	if (_board.tile(at))   checkTileDeath(at, Descriptor::tile(  at.pos()), cset);
	if (_board.ground(at)) checkUnitDeath(at, Descriptor::ground(at.pos()), cset);
	if (_board.air(at))    checkUnitDeath(at, Descriptor::air(   at.pos()), cset);

	// Vision might have changes due to units dying.
	VisionTransition(_bible, _board, *this, cset).executeAround(at,
		std::max(_bible.tileVision(tiletype), std::max(
			_bible.unitVision(groundtype), _bible.unitVision(airtype))));

	_changesets.push(cset);
}

void Automaton::doGasEffect(Cell at)
{
	// Is there enough gas to do damage?
	if (_board.gas(at) < _bible.gasThresholdDamage()) return;

	// Gas no longer hits tiles.
	bool hittiles = !_bible.gasOnlyTargetsGroundUnits();

	// If nothing can take damage or lose power, this effect can be skipped.
	if (! ((hittiles && _board.tile(at)
			&& !_bible.powerAbolished()
			&& _bible.tilePowered(_board.tile(at).type))
		||  _board.ground(at)))
	{
		return;
	}

	// Declare a damage step.
	Damage damage(_bible, _board);

	// Declare takers.
	if (_board.ground(at)) damage.addTaker(Descriptor::ground(at.pos()), _board.ground(at));

	// Declare background.
	if (hittiles)
	{
		if (_board.tile(at)) damage.addBackground(Descriptor::tile(at.pos()), _board.tile(at));
	}

	// If there are no bodies, this effect can be skipped.
	if (!damage.bodies()) return;

	// Declare damage effect.
	Descriptor celldesc = Descriptor::cell(at.pos());
	damage.addEffect(celldesc, _bible.gasShots(), _bible.gasDamage());

	// Remember the types.
	TileType tiletype = hittiles ? _board.tile(at).type : TileType::NONE;
	UnitType groundtype = _board.ground(at).type;

	// Resolve damage.
	damage.resolve();

	// Handle changes.
	ChangeSet cset;
	for (const Damage::Hit& hit : damage)
	{
		const Damage::Body& body = hit.body;

		// The GASSED change tells a player that a unit was gassed by gas.
		Change change(Change::Type::GASSED, body.desc, body.figure,
				hit.killing, hit.depowering);
		_board.enact(change);
		cset.push(change, _board.vision(at));
	}

	// Check for unit deaths.
	if (_board.tile(at) && hittiles) checkTileDeath(at, Descriptor::tile(at.pos()), cset);
	if (_board.ground(at)) checkUnitDeath(at, Descriptor::ground(at.pos()), cset);

	// Vision might have changes due to units dying.
	VisionTransition(_bible, _board, *this, cset).executeAround(at,
		std::max(_bible.tileVision(tiletype),
			_bible.unitVision(groundtype)));

	_changesets.push(cset);
}

void Automaton::doRadiationEffect(Cell at)
{
	// Is there enough radiation to do damage?
	if (_board.radiation(at) < _bible.radiationThresholdDamage()) return;

	// If nothing can take damage or lose power, this effect can be skipped.
	if (! ((_board.tile(at) && _bible.tileDestructible(_board.tile(at).type))
		||  _board.ground(at)
		||  _board.air(at)))
	{
		return;
	}

	// Declare a damage step.
	Damage damage(_bible, _board);

	// Declare takers.
	if (_board.ground(at)) damage.addTaker(Descriptor::ground(at.pos()), _board.ground(at));
	if (_board.air(at))    damage.addTaker(Descriptor::air(   at.pos()), _board.air(at));

	// Declare background.
	if (_board.tile(at)) damage.addBackground(Descriptor::tile(at.pos()), _board.tile(at));

	// If there are no bodies, this effect can be skipped.
	if (!damage.bodies()) return;

	// Declare damage effect.
	Descriptor celldesc = Descriptor::cell(at.pos());
	damage.addEffect(celldesc, _bible.radiationShots(), _bible.radiationDamage());

	// Remember the types.
	TileType tiletype = _board.tile(at).type;
	UnitType groundtype = _board.ground(at).type;
	UnitType airtype =_board.air(at).type;

	// Resolve damage.
	damage.resolve();

	// Handle changes.
	ChangeSet cset;
	for (const Damage::Hit& hit : damage)
	{
		const Damage::Body& body = hit.body;

		// The IRRADIATED change tells a player that a unit was irradiated by radiation.
		Change change(Change::Type::IRRADIATED, body.desc, body.figure,
			hit.killing, hit.depowering);
		_board.enact(change);
		cset.push(change, _board.vision(at));
	}

	// Check for unit deaths.
	if (_board.tile(at))   checkTileDeath(at, Descriptor::tile(  at.pos()), cset);
	if (_board.ground(at)) checkUnitDeath(at, Descriptor::ground(at.pos()), cset);
	if (_board.air(at))    checkUnitDeath(at, Descriptor::air(   at.pos()), cset);

	// Vision might have changes due to units dying.
	VisionTransition(_bible, _board, *this, cset).executeAround(at,
		std::max(_bible.tileVision(tiletype), std::max(
			_bible.unitVision(groundtype), _bible.unitVision(airtype))));

	_changesets.push(cset);
}

bool Automaton::hasColdFeet(Cell at)
{
	return (_bible.frostbiteGivesColdFeet()
		&& _board.frostbite(at) && _board.ground(at)
		// Frostbite's Cold Feet effect applies in Spring, not in Winter.
		&& _bible.chaosMinFrostbite(_season) < 0);
}

void Automaton::checkColdFeetCleanse(Cell at, ChangeSet& changes)
{
	if (_bible.frostbiteGivesColdFeet()
		// Once the unit moves away or is killed, Cold Feet wears off.
		&& _board.frostbite(at) && !_board.ground(at)
		// But only in Spring, not in Winter.
		&& _bible.chaosMinFrostbite(_season) < 0)
	{
		Change change(Change::Type::FROSTBITE,
			Descriptor::cell(at.pos()));
		change.xFrostbite(false);
		_board.enact(change);
		changes.push(change, _board.vision(at));
	}
}

void Automaton::decay()
{
	{
		ChangeSet announcement;
		_phase = Phase::DECAY;
		announcement.push(Change(Change::Type::PHASE, _phase), Vision::all(_players));
		_changesets.push(announcement);
	}

	Randomizer<Cell> indices;
	for (Cell index : _board)
	{
		indices.push(index);
	}

	while (indices)
	{
		Cell index = indices.pop();
		if (_board.death(index))
		{
			// Do a death effect.
			doDeathEffect(index);

			// No need to proceed since we just killed everything.
			continue;
		}

		// Do frostbite, firestorm, gas and radiation effects as needed.
		if (_board.frostbite(index)) doFrostbiteEffect(index);
		if (_board.firestorm(index)) doFirestormEffect(index);
		if (_board.gas(index)) doGasEffect(index);
		if (_board.radiation(index)) doRadiationEffect(index);
	}

	{
		ChangeSet cset;
		GasTransition(_bible, _board, cset).execute();
		RadiationTransition(_board, cset).execute();
		_changesets.push(cset);
	}
}

void Automaton::grow()
{
	{
		ChangeSet announcement;
		nextDaytime(announcement);
		_phase = Phase::GROWTH;
		announcement.push(Change(Change::Type::PHASE, _phase), Vision::all(_players));
		_changesets.push(announcement);
	}

	switch (_daytime)
	{
		case Daytime::EARLY:
		{
			growCities();
			tax();
			cap();
			if (_bible.markersChangeAtNight()) updateMarkers();
			else if (_bible.weatherAfterNight())
			{
				ChangeSet announcement;
				nextDaytime(announcement);
				_changesets.push(announcement);

				weather();
				updateMarkers();
				growPlants();
				updateMorale();
				cap();
			}
		}
		break;
		case Daytime::LATE:
		{
			weather();
			updateMarkers();
			growPlants();
			updateMorale();
			cap();
		}
		break;
	}
}

void Automaton::rest()
{
	ChangeSet announcement;
	_phase = Phase::RESTING;
	announcement.push(Change(Change::Type::PHASE, _phase), Vision::all(_players));

	// Tell each player their old-orderlist.
	for (const Player& player : _players)
	{
		gatherUnfinishedOrders(player, announcement);
	}

	_changesets.push(announcement);
}

void Automaton::tax()
{
	ChangeSet cset;

	for (Cell index : _board)
	{
		const TileToken& tile = _board.tile(index);

		// Some ownable tiles (city, industry) provide income to their owner,
		// and crops tiles are consumed at night.
		if (!_bible.tileOwnable(tile.type)) continue;

		// Calculate the amount of income this tile generates.
		int incomePerPower = _bible.tileIncome(tile.type);
		int count = (_bible.powerAbolished())
			? (_bible.tileStacksMax(tile.type) ? tile.stacks : 1)
			: (_bible.tilePowered(tile.type) ? tile.power : 1);
		int income = count * incomePerPower;

		// If there is no owner, no one gains income.
		if (tile.owner == Player::NONE)
		{
			income = 0;
		}

		// Tiles that are occupied by an enemy unit do not generate income.
		if ((_board.ground(index) && _board.ground(index).owner != tile.owner
					&& _bible.unitCanOccupy(_board.ground(index).type))
			|| (_board.air(index) && _board.air(index).owner != tile.owner
					&& _bible.unitCanOccupy(_board.air(index).type)))
		{
			income = 0;
		}

		// Does this powered tile provide income?
		if (income != 0)
		{
			// Generate the income.
			_money[tile.owner] += income;
			Change moneychange(Change::Type::INCOME,
				Descriptor::tile(index.pos()),
				tile.owner);
			moneychange.xMoney(income);
			cset.push(moneychange, Vision::only(tile.owner));
		}

		// Consume crops.
		TileType consumedtype = _bible.tileConsumed(tile.type);
		if (consumedtype != TileType::NONE)
		{
			TileToken consumed;
			consumed.type = consumedtype;
			if (_bible.tileOwnable(consumedtype)) consumed.owner = tile.owner;
			Change consumechange(Change::Type::CONSUMED,
				Descriptor::tile(index.pos()),
				consumed);
			_board.enact(consumechange);
			cset.push(consumechange, _board.vision(index));
		}
	}

	_changesets.push(cset);
}

void Automaton::updateMorale()
{
	// Morale updates only happen in summer.
	if (!_bible.moraleGatheredInSummer()) return;
	if (_season != Season::SUMMER) return;

	ChangeSet cset;

	for (Cell index : _board)
	{
		// Does this tile affect morale?
		int8_t value = _bible.tileMoraleGainWhenGathered(
			_board.tile(index).type);
		if (value == 0)
		{
			continue;
		}

		// Get all affected players, i.e. players with a city nearby.
		std::vector<Player> affected;
		for (Cell from : _board.area(index, 1, 2))
		{
			Player owner = _board.tile(from).owner;
			if (owner != Player::NONE
				&& _bible.tileGathersMorale(_board.tile(from).type)
				&& std::find(affected.begin(), affected.end(), owner)
					== affected.end())
			{
				affected.push_back(owner);
			}
		}

		// Update morale for those players.
		for (Player player : affected)
		{
			_money[player] += value;
			Change moralechange(Change::Type::INCOME,
				Descriptor::tile(index.pos()),
				player);
			moralechange.xMoney(value);
			cset.push(moralechange, Vision::only(player));
		}
	}

	_changesets.push(cset);
}

void Automaton::cap()
{
	ChangeSet cset;

	for (const Player& player : _players)
	{
		int moneydiff = 0;

		if (_bible.maxMoney() > 0 && _money[player] > _bible.maxMoney())
		{
			moneydiff = _bible.maxMoney() - _money[player];
			_money[player] += moneydiff;
		}
		else if (_money[player] < _bible.minMoney())
		{
			moneydiff = _bible.minMoney() - _money[player];
			_money[player] += moneydiff;
		}

		// Issue a correction so UI's and AI's do not stray.
		if (moneydiff)
		{
			Change correction(Change::Type::FUNDS, player);
			correction.xMoney(moneydiff);
			cset.push(correction, Vision::only(player));
		}
	}

	_changesets.push(cset);
}

void Automaton::growCities()
{
	if (_bible.powerAbolished()) return;

	// We separate the power transition stages into three changesets.
	// This is a mechanical change, since it means that cities powering up in the first stage
	// can provide the power necessary to power up industry tiles in the second stage.
	if (_bible.separatePowerStages())
	{
		for (PowerTransition::Stage stage : PowerTransition::stages)
		{
			ChangeSet cset;
			PowerTransition(_bible, _board, cset, stage).execute();
			_changesets.push(cset);
		}
	}
	else
	{
		ChangeSet cset;
		PowerTransition(_bible, _board, cset, PowerTransition::Stage::ALL).execute();
		_changesets.push(cset);
	}
}

void Automaton::growPlants()
{
	ChangeSet cset;
	TransformTransition(_bible, _board, cset, _season).execute();
	_changesets.push(cset);
}

void Automaton::nextDaytime(ChangeSet& changes)
{
	switch (_daytime)
	{
		case Daytime::LATE:
		{
			_daytime = Daytime::EARLY;
			changes.push(Change(Change::Type::DAYTIME, _daytime),
				Vision::all(_players));
		}
		break;

		case Daytime::EARLY:
		{
			nextSeason(changes);
			_daytime = Daytime::LATE;
			changes.push(Change(Change::Type::DAYTIME, _daytime),
				Vision::all(_players));
		}
		break;
	}
}

void Automaton::nextSeason(ChangeSet& changes)
{
	switch (_season)
	{
		case Season::SPRING: _season = Season::SUMMER; break;
		case Season::SUMMER: _season = Season::AUTUMN; break;
		case Season::AUTUMN: _season = Season::WINTER; break;
		case Season::WINTER: _season = Season::SPRING; break;
	}
	if (_season == Season::SPRING)
	{
		_year++;
		changes.push(Change(Change::Type::YEAR).xYear(_year), Vision::all(_players));
	}
	changes.push(Change(Change::Type::SEASON, _season), Vision::all(_players));
}

void Automaton::weather()
{
	ChangeSet cset;

	// Temperature and humidity change due to seasons and global warming.
	WeatherTransition(_bible, _board, cset, _season).execute();

	// Note that chaos is altered *AFTER* temperatures are altered based on chaos.
	// This is intentional and makes sure that autumn is always the same temperature.
	if (_season == Season::AUTUMN) ChaosTransition(_bible, _board, cset).execute();

	_changesets.push(cset);
}

void Automaton::updateMarkers()
{
	ChangeSet cset;

	MarkerTransition(_bible, _board, cset, _season).execute();

	_changesets.push(cset);
}

int Automaton::globalScore()
{
	int score = 0;
	for (Cell index : _board)
	{
		const TileToken& tile = _board.tile(index);
		score += _bible.tileScoreBase(tile.type);
		score += tile.stacks * _bible.tileScoreStack(tile.type);
	}
	return score;
}

int Automaton::globalWarming()
{
	int total = 0;
	for (Cell index : _board)
	{
		total += (int) _board.chaos(index);
	}
	return total;
}

void Automaton::checkRegularDefeat()
{
	// Check if any players are defeated.
	std::vector<Player> defeats;
	for (const Player& player : _players)
	{
		if (!_defeated[player])
		{
			// Count the number of unoccupied homes this player controls.
			size_t homes = 0;
			size_t occupiedhomes = 0;
			size_t heroes = 0;
			for (Cell index : _board)
			{
				// By default, homes are city tiles.
				// For non-citybound players all controllable tiles are homes.
				const TileToken& tile = _board.tile(index);
				if (tile.owner == player
					&& _bible.tileControllable(tile.type)
					&& (_bible.tileBinding(tile.type) || !_citybound[player]))
				{
					if ((_board.ground(index) && _board.ground(index).owner != tile.owner
								&& _bible.unitCanOccupy(_board.ground(index).type))
						|| (_board.air(index) && _board.air(index).owner != tile.owner
								&& _bible.unitCanOccupy(_board.air(index).type)))
					{
						occupiedhomes++;
					}

					homes++;
				}

				// Non-citybound players do not lose as long as they have units.
				if (!_citybound[player])
				{
					if (_board.ground(index).owner == player)
					{
						heroes++;
					}

					if (_board.air(index).owner == player)
					{
						heroes++;
					}
				}
			}

			// If a player controls no unoccupied homes and no heroes
			// at the end of the decay phase, they lose.
			if (homes <= occupiedhomes && heroes == 0)
			{
				defeats.emplace_back(player);
			}
		}
	}

	if (!defeats.empty())
	{
		doDefeat(defeats, Notice::NONE);
	}
}

void Automaton::checkChallengeDefeat()
{
	if (!_challenge) return;

	// Check for defeat and for time limit.
	std::vector<Player> defeats;
	Notice notice = _challenge->check(_bible, _board, *this, *this, defeats);
	if (!defeats.empty())
	{
		doDefeat(defeats, notice);
	}
}

void Automaton::doDefeat(const std::vector<Player>& defeats,
		const Notice& defeatnotice)
{
	// This changeset can occur at any point during the Action phase;
	// usually at the start because the Action phase only lasts a few ms.
	ChangeSet cset;

	// If players were defeated while still having some homes, they all lose
	// control of their homes simultaneously.
	for (Cell index : _board)
	{
		const TileToken& tile = _board.tile(index);
		if (tile.owner != Player::NONE
				&& (_bible.tileBinding(tile.type) || !_citybound[tile.owner])
				&& std::find(defeats.begin(), defeats.end(),
						tile.owner) != defeats.end())
		{
			// The city is lost, i.e. captured by nature.
			Change capturedchange(Change::Type::CAPTURED,
					Descriptor::tile(index.pos()),
					Player::NONE);
			_board.enact(capturedchange);
			cset.push(capturedchange, _board.vision(index));
		}
	}

	if (cset.any())
	{
		// Update vision for all players simultaneously.
		VisionTransition(_bible, _board, *this, cset).execute();
	}

	// Announce which players are defeated.
	for (const Player& player : defeats)
	{
		_defeated[player] = true;
		_score[player] = 0;

		Change change(Change::Type::DEFEAT, player, defeatnotice);
		change.xScore(0);
		cset.push(change, Vision::all(_players));

		// Throw away their old orders.
		_activeorders[player].clear();
		_activeidentifiers[player].clear();
		// And filter them out of _activeplayers to avoid processing order
		// _activeorders[_activeorderindices] on the next play() call.

		// We will reveal the entire map to them.
		if (std::find(_visionaries.begin(), _visionaries.end(), player)
			== _visionaries.end())
		{
			_visionaries.push_back(player);
		}
	}

	// Filter the defeated players out of _activeplayers.
	std::queue<Player> newactiveplayers;
	while (!_activeplayers.empty())
	{
		Player player = _activeplayers.front();
		if (!_defeated[player])
		{
			newactiveplayers.push(player);
		}
		_activeplayers.pop();
	}
	_activeplayers = std::move(newactiveplayers);

	// How many players are left undefeated?
	std::vector<Player> survivors;
	for (const Player& player : _players)
	{
		if (!_defeated[player])
		{
			survivors.push_back(player);
		}
	}

	// If a player is the only undefeated player, they are victorious.
	// If no one survives, it is game over.
	if (survivors.size() <= 1)
	{
		_gameover = true;
	}

	if (_gameover)
	{
		int worldscore = globalScore();
		for (const Player& survivor : survivors)
		{
			_score[survivor] = worldscore;
		}
		if (_challenge)
		{
			_challenge->score(_bible, _board, *this);
		}
		for (const Player& survivor : survivors)
		{
			Change change(Change::Type::VICTORY, survivor);
			change.xScore(_score[survivor]);
			cset.push(change, Vision::all(_players));
		}
		if (_challenge)
		{
			_challenge->award(_bible, _board, *this);
			for (const Player& player : _players)
			{
				Change change(Change::Type::AWARD, player);
				change.xLevel(_award[player]);
				cset.push(change, Vision::only(player));
			}
		}
		{
			Change change(Change::Type::GAMEOVER);
			change.xScore(worldscore);
			cset.push(change, Vision::all(_players));
		}
		_gameover = true;
	}

	if (cset.any())
	{
		_changesets.push(cset);
	}
	else
	{
		return;
	}

	if (_gameover)
	{
		// In a separate homogeneous changeset.
		ChangeSet vset;

		// Give all players vision of the entire map.
		_visionaries = _players;
		VisionTransition(_bible, _board, *this, vset).execute();
		_changesets.push(vset);
	}
	else if (!defeats.empty())
	{
		// In a separate homogeneous changeset.
		ChangeSet vset;

		// Give all defeated players vision of the entire map.
		VisionTransition(_bible, _board, *this, vset).execute();
		_changesets.push(vset);
	}

	if (_gameover)
	{
		// In a separate homogeneous changeset.
		ChangeSet changes;

		// Score each scoreworthy tile.
		for (Cell index : _board)
		{
			const TileToken& tile = _board.tile(index);
			int score = _bible.tileScoreBase(tile.type);
			score += tile.stacks * _bible.tileScoreStack(tile.type);
			if (score != 0)
			{
				for (const Player& survivor : survivors)
				{
					Change change(Change::Type::SCORED,
						Descriptor::tile(index.pos()), survivor);
					change.xScore(score);
					_board.enact(change);
					changes.push(change, _board.vision(index));
				}
			}
		}
		_changesets.push(changes);
	}
}

void Automaton::record(const ChangeSet& changes)
{
	if (_recording.is_open())
	{
		_recording << changes << std::endl;
	}
}
