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

#include "libs/jsoncpp/json.h"

#include "writer.hpp"
#include "library.hpp"

enum class Player : uint8_t;
enum class Difficulty : uint8_t;
class Settings;


class Tracker;

class EssAI
{
public:
	EssAI(Settings& settings,
		const std::string& ruleset,
		const std::vector<std::string>& ainames,
		const std::vector<Difficulty>& aidifficulties,
		const std::string& mapname, bool enableRecordings);

	EssAI(const EssAI& /**/) = delete;
	EssAI(EssAI&& /**/) = delete;
	EssAI& operator=(const EssAI& /**/) = delete;
	EssAI& operator=(EssAI&& /**/) = delete;
	~EssAI();

private:
	struct Result
	{
		std::vector<uint8_t> initiatives;
		std::vector<Player> positions;
		std::vector<int> scores;
		std::vector<char> defeated;
		int totalScore;
		int globalScore;
		bool draw;
		bool mutualDestruction;
		size_t turns;
		Json::Value metadata;
		std::string recordingName;
	};

	Settings& _settings;
	Writer _writer;
	Library _library;
	std::string _ruleset;
	std::vector<uint8_t> _placements;
	std::vector<Player> _players;
	std::vector<std::string> _ainames;
	std::vector<Difficulty> _aidifficulties;
	std::string _mapname;
	bool _enableRecordings;

	uint64_t ms_phase_start;
	uint64_t ms_action_phase_total = 0;
	uint64_t ms_resting_phase_total = 0;
	uint64_t ms_planning_phase_total = 0;
	uint64_t ms_staging_phase_total = 0;
	size_t total_divisor = 0;

	Json::Value resultsToJson(const std::vector<Result>& results,
		time_t starttime, const char* humantime);
	void writeMatchLengthTsv(const std::vector<Result>& results,
		const std::string& filename);
	Result playGame(size_t offset, Tracker& tracker);

public:
	void run(size_t games);
};
