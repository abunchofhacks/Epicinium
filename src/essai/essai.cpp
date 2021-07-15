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
#include "essai.hpp"
#include "source.hpp"

#include "libs/termcolor/termcolor.hpp"

#include "coredump.hpp"
#include "version.hpp"
#include "settings.hpp"
#include "loginstaller.hpp"
#include "ai.hpp"
#include "difficulty.hpp"
#include "player.hpp"
#include "aicommander.hpp"
#include "ailibrary.hpp"
#include "automaton.hpp"
#include "map.hpp"
#include "library.hpp"
#include "recording.hpp"
#include "clock.hpp"
#include "system.hpp"


static const char* ordinal(uint8_t x);

static constexpr int MIN_TURNS_DRAW = 5;
static constexpr int ABSOLUTE_MAX_TURNS = 100;

class Tracker
{
public:
	static constexpr int NUM_BUCKETS = 101;
	static constexpr int GLOBAL_SCORE_PER_BUCKET = 1;
	static constexpr int GLOBAL_WARMING_PER_BUCKET = 5;

	std::array<std::array<size_t, NUM_BUCKETS>,
		ABSOLUTE_MAX_TURNS + 1> globalScorePerTurn = {0};
	std::array<std::array<size_t, NUM_BUCKETS>,
		ABSOLUTE_MAX_TURNS + 1> globalWarmingPerTurn = {0};

	void trackGlobalScorePerTurn(int score, int turn)
	{
		int bucket = std::max(0, std::min(
			score / GLOBAL_SCORE_PER_BUCKET,
			NUM_BUCKETS - 1));
		globalScorePerTurn[turn][bucket] += 1;
	}

	void trackGlobalWarmingPerTurn(int value, int turn)
	{
		int bucket = std::max(0, std::min(
			value / GLOBAL_WARMING_PER_BUCKET,
			NUM_BUCKETS - 1));
		globalWarmingPerTurn[turn][bucket] += 1;
	}
};

EssAI::EssAI(Settings& settings,
		const std::string& ruleset,
		const std::vector<std::string>& ainames,
		const std::vector<Difficulty>& aidifficulties,
		const std::string& mapname, bool enableRecordings) :
	_settings(settings),
	_ruleset(ruleset),
	_players(getPlayers(ainames.size())),
	_ainames(ainames),
	_aidifficulties(aidifficulties),
	_mapname(mapname),
	_enableRecordings(enableRecordings)
{
	_writer.install();
	_library.load();
	_library.install();

	if (_ruleset.empty())
	{
		_ruleset = _library.currentRuleset();
	}
	else if (!Library::existsBible(ruleset))
	{
		LOGF << "Invalid ruleset name '" << ruleset << "'";
		throw std::runtime_error("invalid ruleset name");
	}

	_placements.resize(_players.size());
	for (size_t i = 0; i < _players.size(); i++)
	{
		_placements[i] = i;
	}

	if (!Map::exists(mapname))
	{
		LOGF << "Invalid map name '" << mapname << "'";
		throw std::runtime_error("invalid map name");
	}

	for (const std::string& ainame : _ainames)
	{
		if (AI::exists(ainame))
		{
			// Nothing to do.
		}
		else if (AILibrary::exists(ainame))
		{
			// Preload the library.
			AILibrary::preload(ainame);
		}
		else
		{
			LOGF << "Invalid AI name '" << ainame << "'";
			throw std::runtime_error("invalid AI name");
		}
	}

	if (_settings.seed.defined())
	{
		srand(_settings.seed.value());
	}
	else
	{
		auto timestampMs = SteadyClock::milliseconds();
		srand(timestampMs);
	}
}

EssAI::~EssAI()
{
	if (total_divisor > 0)
	{
		PERFLOGI << "timeframe_in_seconds = "
			<< ((ms_action_phase_total + ms_resting_phase_total
				+ ms_planning_phase_total + ms_staging_phase_total) / 1000);
		PERFLOGI << "avg_action_phase = "
			<< (1.0 * ms_action_phase_total / total_divisor);
		PERFLOGI << "avg_resting_phase = "
			<< (1.0 * ms_resting_phase_total / total_divisor);
		PERFLOGI << "avg_planning_phase = "
			<< (1.0 * ms_planning_phase_total / total_divisor);
		PERFLOGI << "avg_staging_phase = "
			<< (1.0 * ms_staging_phase_total / total_divisor);
	}
}

EssAI::Result EssAI::playGame(size_t offset, Tracker& tracker)
{
	if (_players.size() <= 2)
	{
		if (offset % 2 == 0)
		{
			std::reverse(_placements.begin(), _placements.end());
		}
		else
		{
			std::reverse(_players.begin(), _players.end());
		}
	}
	else
	{
		// TODO replace with Steinhaus-Johnson-Trotter algorithm
		if (offset % 2 == 0)
		{
			std::random_shuffle(_placements.begin(), _placements.end());
		}
		else
		{
			std::random_shuffle(_players.begin(), _players.end());
		}
	}

	Json::Value metadata = Json::objectValue;
	metadata["map"] = _mapname;
	metadata["online"] = false;
	metadata["planningtime"] = 0;
	metadata["bots"] = Json::arrayValue;

	std::vector<Player> aiplayers;
	std::vector<std::shared_ptr<AICommander>> aicommanders; // (unique own.)
	std::vector<std::shared_ptr<AILibrary>> ailibraries; // (unique ownership)
	for (size_t i = 0; i < _ainames.size(); i++)
	{
		aiplayers.emplace_back(_players[_placements[i]]);

		if (AILibrary::isLibraryReminder(_ainames[i]))
		{
			std::shared_ptr<AILibrary> ptr = AILibrary::create(_ainames[i],
				_players[_placements[i]], _aidifficulties[i],
				_ruleset, 'A' + i);

			if (AILibrary::hasFastDirective(_ainames[i]))
			{
				auto result = std::dynamic_pointer_cast<AICommander>(ptr);
				if (result)
				{
					aicommanders.emplace_back(result);
				}
				else ailibraries.emplace_back(ptr);
			}
			else ailibraries.emplace_back(ptr);

			const AILibrary& ai = *ptr;
			Json::Value json = Json::objectValue;
			json["player"] = ::stringify(ai.player());
			json["difficulty"] = ::stringify(ai.difficulty());
			json["character"] = ai.characterstring();
			json["displayname"] = ai.displayname();
			json["ainame"] = ai.ainame();
			json["authors"] = ai.authors();
			json["libraryname"] = _ainames[i];
			metadata["bots"].append(json);
		}
		else
		{
			aicommanders.emplace_back(AI::create(_ainames[i],
				_players[_placements[i]], _aidifficulties[i],
				_ruleset, 'A' + i));

			const AICommander& ai = *aicommanders.back();
			Json::Value json = Json::objectValue;
			json["player"] = ::stringify(ai.player());
			json["difficulty"] = ::stringify(ai.difficulty());
			json["character"] = ai.characterstring();
			json["displayname"] = ai.displayname();
			json["ainame"] = ai.ainame();
			json["authors"] = ai.authors();
			metadata["bots"].append(json);
		}
	}

	Automaton automaton(_players, _ruleset);
	Phase phase = Phase::GROWTH;

	automaton.load(_mapname, false);
	if (_enableRecordings)
	{
		automaton.startRecording(metadata);
	}
	size_t turns = 0;
	bool draw = false;

	uint64_t ms = SteadyClock::milliseconds();
	ms_phase_start = ms;

	while (phase != Phase::DECAY)
	{
		switch (phase)
		{
			case Phase::GROWTH:
			case Phase::ACTION:
			{
				if (automaton.active())
				{
					ChangeSet changeset = automaton.act();
					for (const auto& aicommander : aicommanders)
					{
						aicommander->receiveChanges(
							changeset.get(aicommander->player()));
					}
					for (const auto& ailibrary : ailibraries)
					{
						ailibrary->receiveChanges(
							changeset.get(ailibrary->player()),
							automaton.bible());
					}
				}
				else phase = Phase::RESTING;
			}
			break;

			case Phase::RESTING:
			{
				ms = SteadyClock::milliseconds();
				ms_action_phase_total += ms - ms_phase_start;
				ms_phase_start = ms;

				if (automaton.gameover())
				{
					phase = Phase::DECAY;
					break;
				}
				else if (automaton.globalScore() <= 0
					&& turns >= MIN_TURNS_DRAW)
				{
					draw = true;
					phase = Phase::DECAY;
					break;
				}
				else if (turns >= ABSOLUTE_MAX_TURNS)
				{
					draw = true;
					phase = Phase::DECAY;
					break;
				}

				tracker.trackGlobalScorePerTurn(automaton.globalScore(), turns);
				tracker.trackGlobalWarmingPerTurn(automaton.globalWarming(),
					turns);

				ChangeSet changeset = automaton.hibernate();
				for (const auto& aicommander : aicommanders)
				{
					aicommander->receiveChanges(
						changeset.get(aicommander->player()));
				}
				for (const auto& ailibrary : ailibraries)
				{
					ailibrary->receiveChanges(
						changeset.get(ailibrary->player()),
						automaton.bible());
				}
				phase = Phase::PLANNING;
			}
			break;

			case Phase::PLANNING:
			{
				ms = SteadyClock::milliseconds();
				ms_resting_phase_total += ms - ms_phase_start;
				ms_phase_start = ms;

				for (const auto& aicommander : aicommanders)
				{
					aicommander->prepareOrders();
				}
				for (const auto& ailibrary : ailibraries)
				{
					ailibrary->prepareOrders();
				}

				ChangeSet changeset = automaton.awake();
				for (const auto& aicommander : aicommanders)
				{
					aicommander->receiveChanges(
						changeset.get(aicommander->player()));
				}
				for (const auto& ailibrary : ailibraries)
				{
					ailibrary->receiveChanges(
						changeset.get(ailibrary->player()),
						automaton.bible());
				}
				phase = Phase::STAGING;
			}
			break;

			case Phase::STAGING:
			{
				ms = SteadyClock::milliseconds();
				ms_planning_phase_total += ms - ms_phase_start;
				ms_phase_start = ms;

				for (const auto& aicommander : aicommanders)
				{
					automaton.receive(aicommander->player(),
						aicommander->orders());
				}
				for (const auto& ailibrary : ailibraries)
				{
					automaton.receive(ailibrary->player(),
						ailibrary->orders(automaton.bible()));
				}

				ChangeSet changeset = automaton.prepare();
				for (const auto& aicommander : aicommanders)
				{
					aicommander->receiveChanges(
						changeset.get(aicommander->player()));
				}
				for (const auto& ailibrary : ailibraries)
				{
					ailibrary->receiveChanges(
						changeset.get(ailibrary->player()),
						automaton.bible());
				}

				ms = SteadyClock::milliseconds();
				ms_staging_phase_total += ms - ms_phase_start;
				ms_phase_start = ms;

				phase = Phase::ACTION;
				turns++;
			}
			break;

			case Phase::DECAY:
			break;
		}
	}

	total_divisor += turns;

	Result result;
	{
		for (const Player& player : aiplayers)
		{
			for (size_t i = 0; i < _players.size(); i++)
			{
				if (_players[i] == player)
				{
					result.initiatives.emplace_back(i + 1);
					break;
				}
			}
			result.positions.emplace_back(player);
			result.scores.emplace_back(draw ? 0 : automaton.score(player));
			result.defeated.emplace_back(automaton.defeated(player));
		}
	}
	result.totalScore = 0;
	for (int score : result.scores)
	{
		result.totalScore += score;
	}
	result.globalScore = automaton.globalScore();
	result.draw = draw;
	result.mutualDestruction = true;
	for (bool defeated : result.defeated)
	{
		result.mutualDestruction &= defeated;
	}
	result.turns = turns;
	result.metadata = metadata;
	result.recordingName = automaton.identifier();
	return result;
}

Json::Value EssAI::resultsToJson(const std::vector<Result>& results,
	time_t starttime, const char* humantime)
{
	size_t totalWon = 0;
	size_t totalLost = 0;
	size_t totalDraw = 0;
	size_t totalMutual = 0;
	size_t totalPeace = 0;
	int totalScore = 0;
	int totalScoreAgainst = 0;
	size_t totalTurns = 0;
	size_t minTurns = 0;
	size_t maxTurns = 0;

	size_t totalWonIntv[PLAYER_MAX] = {0};
	size_t totalLostIntv[PLAYER_MAX] = {0};
	size_t totalDrawIntv[PLAYER_MAX] = {0};
	size_t totalMutualIntv[PLAYER_MAX] = {0};
	size_t totalPeaceIntv[PLAYER_MAX] = {0};
	int totalScoreIntv[PLAYER_MAX] = {0};
	int totalScoreAgainstIntv[PLAYER_MAX] = {0};
	size_t totalTurnsIntv[PLAYER_MAX] = {0};
	size_t minTurnsIntv[PLAYER_MAX] = {0};
	size_t maxTurnsIntv[PLAYER_MAX] = {0};
	size_t gamesIntv[PLAYER_MAX] = {0};

	size_t totalWonFrom[PLAYER_MAX] = {0};
	size_t totalLostFrom[PLAYER_MAX] = {0};
	size_t totalDrawFrom[PLAYER_MAX] = {0};
	size_t totalMutualFrom[PLAYER_MAX] = {0};
	size_t totalPeaceFrom[PLAYER_MAX] = {0};
	int totalScoreFrom[PLAYER_MAX] = {0};
	int totalScoreAgainstFrom[PLAYER_MAX] = {0};
	size_t totalTurnsFrom[PLAYER_MAX] = {0};
	size_t minTurnsFrom[PLAYER_MAX] = {0};
	size_t maxTurnsFrom[PLAYER_MAX] = {0};
	size_t gamesFrom[PLAYER_MAX] = {0};

	Json::Value json(Json::objectValue);
	json["starttime"] = uint64_t(starttime);
	json["localtime"] = uint64_t(humantime);

	json["games"] = Json::arrayValue;
	for (const Result& result : results)
	{
		Json::Value game = Json::objectValue;
		game["initiatives"] = Json::arrayValue;
		for (uint8_t x : result.initiatives)
		{
			game["initiatives"].append(x);
		}
		game["positions"] = Json::arrayValue;
		for (const Player& x : result.positions)
		{
			game["positions"].append(::stringify(x));
		}
		game["scores"] = Json::arrayValue;
		for (int x : result.scores)
		{
			game["scores"].append(x);
		}
		game["defeated"] = Json::arrayValue;
		for (bool x : result.defeated)
		{
			game["defeated"].append(x);
		}
		game["total_score"] = result.totalScore;
		game["global_score"] = result.globalScore;
		game["draw"] = result.draw;
		game["mutual_destruction"] = result.mutualDestruction;
		game["turns"] = result.turns;
		game["metadata"] = result.metadata;
		game["recording_name"] = result.recordingName;
		json["games"].append(game);

		if (result.mutualDestruction)
		{
			totalMutual++;
		}
		else if (result.draw)
		{
			totalDraw++;
		}
		else if (result.defeated[0])
		{
			totalLost++;
		}
		// TODO is it possible to broker peace when the totalscore is 0?
		else if (result.scores[0] == result.totalScore)
		{
			totalWon++;
		}
		else
		{
			totalPeace++;
		}
		totalTurns += result.turns;
		if (minTurns == 0 || minTurns > result.turns)
			minTurns = result.turns;
		if (maxTurns < result.turns) maxTurns = result.turns;
		totalScore += result.scores[0];
		totalScoreAgainst += result.totalScore - result.scores[0];

		{
			int i = result.initiatives[0] - 1;
			if (result.mutualDestruction)
			{
				totalMutualIntv[i]++;
			}
			else if (result.draw)
			{
				totalDrawIntv[i]++;
			}
			else if (result.defeated[0])
			{
				totalLostIntv[i]++;
			}
			// TODO is it possible to broker peace when the totalscore is 0?
			else if (result.scores[0] == result.totalScore)
			{
				totalWonIntv[i]++;
			}
			else
			{
				totalPeaceIntv[i]++;
			}
			totalTurnsIntv[i] += result.turns;
			if (minTurnsIntv[i] == 0 || minTurnsIntv[i] > result.turns)
				minTurnsIntv[i] = result.turns;
			if (maxTurnsIntv[i] < result.turns) maxTurnsIntv[i] = result.turns;
			totalScoreIntv[i] += result.scores[0];
			totalScoreAgainstIntv[i] += result.totalScore - result.scores[0];
			gamesIntv[i]++;
		}

		{
			int i = (int) result.positions[0] - 1;
			if (result.mutualDestruction)
			{
				totalMutualFrom[i]++;
			}
			else if (result.draw)
			{
				totalDrawFrom[i]++;
			}
			else if (result.defeated[0])
			{
				totalLostFrom[i]++;
			}
			// TODO is it possible to broker peace when the totalscore is 0?
			else if (result.scores[0] == result.totalScore)
			{
				totalWonFrom[i]++;
			}
			else
			{
				totalPeaceFrom[i]++;
			}
			totalTurnsFrom[i] += result.turns;
			if (minTurnsFrom[i] == 0 || minTurnsFrom[i] > result.turns)
				minTurnsFrom[i] = result.turns;
			if (maxTurnsFrom[i] < result.turns) maxTurnsFrom[i] = result.turns;
			totalScoreFrom[i] += result.scores[0];
			totalScoreAgainstFrom[i] += result.totalScore - result.scores[0];
			gamesFrom[i]++;
		}
	}

	json["statistics"] = Json::objectValue;
	json["statistics"]["total_won"] = totalWon;
	json["statistics"]["total_lost"] = totalLost;
	json["statistics"]["total_draw"] = totalDraw;
	json["statistics"]["total_mutual"] = totalMutual;
	json["statistics"]["total_peace"] = totalPeace;
	json["statistics"]["total_score"] = totalScore;
	json["statistics"]["total_score_against"] = totalScoreAgainst;
	json["statistics"]["total_turns"] = totalTurns;
	json["statistics"]["min_turns"] = minTurns;
	json["statistics"]["max_turns"] = maxTurns;
	json["statistics"]["games"] = results.size();

	for (int i = 0; i < (int) _players.size() && i < (int) PLAYER_MAX; i++)
	{
		std::string tag = "statistics_" + std::string(ordinal(i + 1));
		json[tag] = Json::objectValue;
		json[tag]["total_won"] = totalWonIntv[i];
		json[tag]["total_lost"] = totalLostIntv[i];
		json[tag]["total_draw"] = totalDrawIntv[i];
		json[tag]["total_mutual"] = totalMutualIntv[i];
		json[tag]["total_peace"] = totalPeaceIntv[i];
		json[tag]["total_score"] = totalScoreIntv[i];
		json[tag]["total_score_against"] = totalScoreAgainstIntv[i];
		json[tag]["total_turns"] = totalTurnsIntv[i];
		json[tag]["min_turns"] = minTurnsIntv[i];
		json[tag]["max_turns"] = maxTurnsIntv[i];
		json[tag]["games"] = gamesIntv[i];
	}

	for (int i = 0; i < (int) _players.size() && i < (int) PLAYER_MAX; i++)
	{
		std::string tag = "statistics_"
			+ std::string(::stringify((Player) (i + 1)));
		json[tag] = Json::objectValue;
		json[tag]["total_won"] = totalWonFrom[i];
		json[tag]["total_lost"] = totalLostFrom[i];
		json[tag]["total_draw"] = totalDrawFrom[i];
		json[tag]["total_mutual"] = totalMutualFrom[i];
		json[tag]["total_peace"] = totalPeaceFrom[i];
		json[tag]["total_score"] = totalScoreFrom[i];
		json[tag]["total_score_against"] = totalScoreAgainstFrom[i];
		json[tag]["total_turns"] = totalTurnsFrom[i];
		json[tag]["min_turns"] = minTurnsFrom[i];
		json[tag]["max_turns"] = maxTurnsFrom[i];
		json[tag]["games"] = gamesFrom[i];
	}

	return json;
}

void EssAI::writeMatchLengthTsv(const std::vector<Result>& results,
	const std::string& filename)
{
	int lastTurnWin[ABSOLUTE_MAX_TURNS + 1] = {0};
	int lastTurnLoss[ABSOLUTE_MAX_TURNS + 1] = {0};
	int lastTurnDraw[ABSOLUTE_MAX_TURNS + 1] = {0};

	for (const Result& result : results)
	{
		if (result.mutualDestruction)
		{
			lastTurnDraw[result.turns] += 1;
		}
		else if (result.draw)
		{
			lastTurnDraw[result.turns] += 1;
		}
		else if (result.defeated[0])
		{
			lastTurnLoss[result.turns] += 1;
		}
		else if (result.scores[0] == result.totalScore)
		{
			lastTurnWin[result.turns] += 1;
		}
		else
		{
			lastTurnDraw[result.turns] += 1;
		}
	}

	LOGD << "Writing TSV to " << filename;
	System::touchFile(filename);
	std::ofstream record = System::ofstream(filename);
	if (!record)
	{
		LOGE << "Failed to open " << filename;
		return;
	}
	record << "#\tW\tL\tD" << std::endl;
	for (size_t t = 0; t < array_size(lastTurnWin); t++)
	{
		record << std::to_string(t)
			<< "\t" << lastTurnWin[t]
			<< "\t" << lastTurnLoss[t]
			<< "\t" << lastTurnDraw[t]
			<< std::endl;
	}
	LOGD << "Finished writing TSV";
}

static void writeTrackerGridTsv(
	const std::array<std::array<size_t, Tracker::NUM_BUCKETS>,
		ABSOLUTE_MAX_TURNS + 1>& grid,
	size_t amount_per_bucket,
	size_t num_buckets,
	const std::string& filename)
{
	LOGD << "Writing TSV to " << filename;
	System::touchFile(filename);
	std::ofstream record = System::ofstream(filename);
	if (!record)
	{
		LOGE << "Failed to open " << filename;
		return;
	}
	assert(num_buckets <= Tracker::NUM_BUCKETS);
	for (size_t b = 0; b < num_buckets; b++)
	{
		if (b == num_buckets - 1)
		{
			record << (b * amount_per_bucket) << "+";
		}
		else if (amount_per_bucket > 1)
		{
			record << (b * amount_per_bucket)
				<< "-" << ((b + 1) * amount_per_bucket - 1);
		}
		else
		{
			record << (b * amount_per_bucket);
		}

		for (size_t t = 0; t < grid.size(); t++)
		{
			record << "\t" << grid[t][b];
		}
		record << std::endl;
	}
	LOGD << "Finished writing TSV";
}

static void writeGlobalScoreTsv(const Tracker& tracker,
	const std::string& filename)
{
	writeTrackerGridTsv(tracker.globalScorePerTurn,
		Tracker::GLOBAL_SCORE_PER_BUCKET,
		Tracker::NUM_BUCKETS,
		filename);
}

static void writeGlobalWarmingTsv(const Tracker& tracker,
	const std::string& filename)
{
	writeTrackerGridTsv(tracker.globalWarmingPerTurn,
		Tracker::GLOBAL_WARMING_PER_BUCKET,
		std::min(260 / Tracker::GLOBAL_WARMING_PER_BUCKET + 1,
			Tracker::NUM_BUCKETS),
		filename);
}

static const char* ordinal(uint8_t x)
{
	switch (x)
	{
		case 1: return "first"; break;
		case 2: return "second"; break;
		case 3: return "third"; break;
		case 4: return "fourth"; break;
		case 5: return "fifth"; break;
		case 6: return "sixth"; break;
		case 7: return "seventh"; break;
		case 8: return "eighth"; break;
		default: return "last"; break;
	}
}

void EssAI::run(size_t games)
{
	time_t starttime;
	std::time(&starttime);
	char humantime[100];
	std::strftime(humantime, sizeof(humantime), "%Y-%m-%dT%Hh%Mm%S",
		std::localtime(&starttime));
	std::string desc;
	for (size_t i = 0; i < _players.size(); i++)
	{
		if (i > 0) desc += "-";
		desc += ::sanitize(_ainames[i]);
	}
	std::string logname = LogInstaller::getLogsFolderWithSlash() + "essai/"
		+ desc + "-" + humantime + ".log";
	System::touchFile(logname);
	std::ofstream log = System::ofstream(logname,
		std::ofstream::out | std::ofstream::app);
	if (!log)
	{
		LOGE << "Failed to open " << logname << " for writing";
		throw std::runtime_error("failed to open " + logname + " for writing");
	}

	Tracker tracker;

	std::vector<Result> results;
	for (size_t i = 0; i < games; i++)
	{
		results.emplace_back(playGame(i, tracker));
		Result& result = results.back();
		std::cout << "Result of game " << i
			<< " (" << result.recordingName << ")"
			<< ": "
			<< "Challenger"
			<< " started " << ordinal(result.initiatives[0])
			<< " from the " << ::stringify(result.positions[0]) << " position"
			<< " and";
		if (result.mutualDestruction)
		{
			std::cout << " "
				<< termcolor::yellow
				<< "achieved mutual destruction"
				<< termcolor::reset;
		}
		else if (result.draw)
		{
			std::cout << " "
				<< termcolor::yellow
				<< "drew"
				<< termcolor::reset;
		}
		else if (result.defeated[0])
		{
			std::cout << " "
				<< termcolor::bold << termcolor::red
				<< "lost"
				<< termcolor::reset;
		}
		// TODO is it possible to broker peace when the totalscore is 0?
		else if (result.scores[0] == result.totalScore)
		{
			std::cout << " "
				<< termcolor::bold << termcolor::green
				<< "won"
				<< termcolor::reset;
		}
		else
		{
			std::cout << " "
				<< termcolor::bold << termcolor::cyan
				<< "achieved peace"
				<< termcolor::reset;
		}
		std::cout << " in " << result.turns << " turns";
		std::cout << " with " << result.scores[0] << " against "
			<< (result.totalScore - result.scores[0]) << " points."
			<< std::endl;
	}

	Json::Value json = resultsToJson(results, starttime, humantime);
	log << Writer::write(json);
	log.close();

	float gamesAll = json["statistics"]["games"].asInt();
	std::cout << std::endl << "FINAL RESULTS:" << std::endl;
	std::cout << "Number of games: " << json["statistics"]["games"]
		<< std::endl;
	std::cout << "Won: " << json["statistics"]["total_won"].asInt() << " ("
		<< json["statistics"]["total_won"].asInt() / gamesAll * 100 << "%)"
		<< std::endl;
	std::cout << "Lost: " << json["statistics"]["total_lost"] << " ("
		<< json["statistics"]["total_lost"].asInt() / gamesAll * 100 << "%)"
		<< std::endl;
	std::cout << "Draws: " << json["statistics"]["total_draw"]
		<< " (" << json["statistics"]["total_draw"].asInt() / gamesAll * 100
		<< "%)" << std::endl;
	std::cout << "Mutual destruction: " << json["statistics"]["total_mutual"]
		<< " (" << json["statistics"]["total_mutual"].asInt() / gamesAll * 100
		<< "%)" << std::endl;
	std::cout << "Peace: " << json["statistics"]["total_peace"]
		<< " (" << json["statistics"]["total_peace"].asInt() / gamesAll  * 100
		<< "%)" << std::endl;
	std::cout << "Average score: "
		<< json["statistics"]["total_score"].asInt() / gamesAll << std::endl;
	std::cout << "Average score against: "
		<< json["statistics"]["total_score_against"].asInt() / gamesAll
		<< std::endl;
	std::cout << "Average number of turns: "
		<< json["statistics"]["total_turns"].asInt() / gamesAll << std::endl;
	std::cout << "Minimum number of turns: "
		<< json["statistics"]["min_turns"].asInt() << std::endl;
	std::cout << "Maximum number of turns: "
		<< json["statistics"]["max_turns"].asInt() << std::endl;


	for (int i = 0; i < 1; i++)
	{
		std::string tag = "statistics_" + std::string(ordinal(i + 1));
	 	float ngames = json[tag]["games"].asInt();
		std::cout << std::endl << "FINAL RESULTS ("
			<< toupper(ordinal(i + 1))
			<< "):" << std::endl;
		std::cout << "Number of games: " << json[tag]["games"]
			<< std::endl;
		std::cout << "Won: " << json[tag]["total_won"].asInt() << " ("
			<< json[tag]["total_won"].asInt() / ngames * 100 << "%)"
			<< std::endl;
		std::cout << "Lost: " << json[tag]["total_lost"] << " ("
			<< json[tag]["total_lost"].asInt() / ngames * 100 << "%)"
			<< std::endl;
		std::cout << "Draws: "
			<< json[tag]["total_draw"] << " ("
			<< json[tag]["total_draw"].asInt() / ngames * 100
			<< "%)" << std::endl;
		std::cout << "Mutual destruction: "
			<< json[tag]["total_mutual"] << " ("
			<< json[tag]["total_mutual"].asInt() / ngames * 100
			<< "%)" << std::endl;
		std::cout << "Peace: " << json[tag]["total_peace"] << " ("
			<< json[tag]["total_peace"].asInt() / ngames * 100
			<< "%)" << std::endl;
		std::cout << "Average score: "
			<< json[tag]["total_score"].asInt() / ngames
			<< std::endl;
		std::cout << "Average score against: "
			<< json[tag]["total_score_against"].asInt() / ngames
			<< std::endl;
		std::cout << "Average number of turns: "
			<< json[tag]["total_turns"].asInt() / ngames
			<< std::endl;
		std::cout << "Minimum number of turns: "
			<< json[tag]["min_turns"].asInt() << std::endl;
		std::cout << "Maximum number of turns: "
			<< json[tag]["max_turns"].asInt() << std::endl;
	}

	for (int i = 0; i < (int) _players.size() && i < (int) PLAYER_MAX; i++)
	{
		std::string tag = "statistics_"
			+ std::string(::stringify((Player) (i + 1)));
		float ngames = json[tag]["games"].asInt();
		std::cout << std::endl << "FINAL RESULTS ("
			<< toupper(::stringify((Player) (i + 1)))
			<< "):" << std::endl;
		std::cout << "Number of games: " << json[tag]["games"]
			<< std::endl;
		std::cout << "Won: " << json[tag]["total_won"].asInt() << " ("
			<< json[tag]["total_won"].asInt() / ngames * 100 << "%)"
			<< std::endl;
		std::cout << "Lost: " << json[tag]["total_lost"] << " ("
			<< json[tag]["total_lost"].asInt() / ngames * 100 << "%)"
			<< std::endl;
		std::cout << "Draws: "
			<< json[tag]["total_draw"] << " ("
			<< json[tag]["total_draw"].asInt() / ngames * 100
			<< "%)" << std::endl;
		std::cout << "Mutual destruction: "
			<< json[tag]["total_mutual"] << " ("
			<< json[tag]["total_mutual"].asInt() / ngames * 100
			<< "%)" << std::endl;
		std::cout << "Peace: " << json[tag]["total_peace"] << " ("
			<< json[tag]["total_peace"].asInt() / ngames * 100
			<< "%)" << std::endl;
		std::cout << "Average score: "
			<< json[tag]["total_score"].asInt() / ngames
			<< std::endl;
		std::cout << "Average score against: "
			<< json[tag]["total_score_against"].asInt() / ngames
			<< std::endl;
		std::cout << "Average number of turns: "
			<< json[tag]["total_turns"].asInt() / ngames
			<< std::endl;
		std::cout << "Minimum number of turns: "
			<< json[tag]["min_turns"].asInt() << std::endl;
		std::cout << "Maximum number of turns: "
			<< json[tag]["max_turns"].asInt() << std::endl;
	}

	{
		std::string filename = LogInstaller::getLogsFolderWithSlash()
			+ "graphs/matchlength-output.tsv";
		writeMatchLengthTsv(results, filename);
	}
	{
		std::string filename = LogInstaller::getLogsFolderWithSlash()
			+ "graphs/globalscore-output.tsv";
		writeGlobalScoreTsv(tracker, filename);
	}
	{
		std::string filename = LogInstaller::getLogsFolderWithSlash()
			+ "graphs/globalwarming-output.tsv";
		writeGlobalWarmingTsv(tracker, filename);
	}

	if (!_fullsuitelogname.empty())
	{
		System::touchFile(_fullsuitelogname);
		std::ofstream out = System::ofstream(_fullsuitelogname,
			std::ofstream::out | std::ofstream::app);
		if (!out)
		{
			LOGE << "Failed to open " << _fullsuitelogname << " for writing";
			throw std::runtime_error("failed to open " + _fullsuitelogname
				+ " for writing");
		}
		for (const Result& result : results)
		{
			Json::Value entry = Json::objectValue;
			entry["map"] = _mapname;
			bool winner = false;
			entry["usernames"] = Json::arrayValue;
			for (size_t i = 0; i < _ainames.size(); i++)
			{
				std::string username = "[bot]" + _ainames[i] + ":"
					+ stringify(_aidifficulties[i]);
				entry["usernames"].append(username);
				if (result.scores[i] > 0)
				{
					winner = true;
					entry["winner"] = username;
					entry["score"] = result.scores[i];
				}
			}
			if (!winner)
			{
				entry["winner"] = Json::nullValue;
				entry["score"] = result.totalScore;
			}
			out << Writer::write(entry) << std::endl;
		}
	}
}

void EssAI::runFullSuite(size_t ngames)
{
	{
		time_t starttime;
		std::time(&starttime);
		char humantime[100];
		std::strftime(humantime, sizeof(humantime), "%Y-%m-%dT%Hh%Mm%S",
			std::localtime(&starttime));
		_fullsuitelogname = LogInstaller::getLogsFolderWithSlash() + "essai/"
			+ "fullsuite_game_results_" + humantime + ".log";
	}

	std::vector<std::string> ainames = {"RampantRhino", "HungryHippo",
		"NeuralNewt", "ChargingCheetah"};
	std::vector<Difficulty> aidiffs = {Difficulty::HARD,
		Difficulty::MEDIUM, Difficulty::EASY};

	for (const std::string& mapname : Map::pool())
	{
		Json::Value mapmetadata = Map::loadMetadata(mapname);
		if (mapmetadata["playercount"] != 2) continue;
		_mapname = mapname;
		for (size_t i = 0; i < ainames.size(); i++)
		{
			for (size_t j = i + 1; j < ainames.size(); j++)
			{
				_ainames = {ainames[i], ainames[j]};
				for (size_t u = 0; u < aidiffs.size(); u++)
				{
					for (size_t v = 0; v < aidiffs.size(); v++)
					{
						_aidifficulties = {aidiffs[u], aidiffs[v]};
						run(ngames);
					}
				}
			}

			_ainames = {ainames[i], ainames[i]};
			for (size_t u = 0; u < aidiffs.size(); u++)
			{
				for (size_t v = u + 1; v < aidiffs.size(); v++)
				{
					_aidifficulties = {aidiffs[u], aidiffs[v]};
					run(ngames);
				}
			}

			_ainames = {ainames[i], "Dummy"};
			for (size_t u = 0; u < aidiffs.size(); u++)
			{
				_aidifficulties = {aidiffs[u], Difficulty::NONE};
				run(ngames);
			}
		}
	}
}

int main(int argc, char* argv[])
{
	CoreDump::enable();

	std::string logname = "essai";
	std::string rulname = "";
	std::vector<std::string> ainames;
	std::vector<Difficulty> aidifficulties;
	std::string mapname = "";
	bool record = false;
	bool fullsuite = false;
	int ngames = 0;

	Settings settings("settings-essai.json", argc, argv);

	for (int i = 1; i < argc; i++)
	{
		const char* arg = argv[i];
		size_t arglen = strlen(arg);
		if (strncmp(arg, "-", 1) == 0)
		{
			// Setting argument, will be handled by Settings.
		}
		else if (ainames.size() > aidifficulties.size())
		{
			aidifficulties.emplace_back(parseDifficulty(arg));
		}
		else if (strspn(arg, "0123456789") == arglen)
		{
			ngames = atoi(arg);
		}
		else if (strcmp(arg, "record") == 0)
		{
			record = true;
		}
		else if (strcmp(arg, "fullsuite") == 0)
		{
			fullsuite = true;
		}
		else if (arglen > 9 + 5
			&& strncmp(arg, "rulesets/", 9) == 0
			&& strncmp(arg + arglen - 5, ".json", 5) == 0)
		{
			rulname = std::string(arg + 9, arglen - 9 - 5);
		}
		else if (AI::exists(arg))
		{
			ainames.emplace_back(arg);
		}
		else if (AILibrary::exists(arg))
		{
			ainames.emplace_back(arg);
		}
		else
		{
			mapname = arg;
		}
	}

	if (settings.logname.defined())
	{
		logname = settings.logname.value();
	}
	else settings.logname.override(logname);

	std::cout << "[ Epicinium essAI ]";
	std::cout << " (" << logname << " v" << Version::current() << ")";
	std::cout << std::endl << std::endl;

	if (settings.dataRoot.defined())
	{
		LogInstaller::setRoot(settings.dataRoot.value());
		Recording::setRoot(settings.dataRoot.value());
	}

	LogInstaller(settings).install();

	LOGI << "Start " << logname << " v" << Version::current();

	if (ainames.empty() && aidifficulties.empty())
	{
		ainames = {
			"RampantRhino",
			"HungryHippo",
		};
		aidifficulties =
		{
			Difficulty::HARD,
			Difficulty::HARD,
		};
	}
	else if (ainames.size() != aidifficulties.size())
	{
		LOGF << "Each AI should be followed by a difficulty";
		throw std::runtime_error("each AI should be followed by a difficulty");
	}

	if (mapname.empty())
	{
		mapname = "oceanside1v1";
	}

	if (ngames == 0)
	{
		if (fullsuite)
		{
			ngames = 1;
		}
		else
		{
			ngames = 1000;
		}
	}

	EssAI essai(settings,
		rulname,
		ainames,
		aidifficulties,
		mapname,
		record);

	if (fullsuite)
	{
		essai.runFullSuite(ngames);
	}
	else
	{
		essai.run(ngames);
	}

	std::cout << std::endl << std::endl << "[ Done ]" << std::endl;
	return 0;
}
