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
#include "source.hpp"

#include "clock.hpp"
#include "coredump.hpp"
#include "version.hpp"
#include "settings.hpp"
#include "loginstaller.hpp"
#include "library.hpp"
#include "system.hpp"
#include "writer.hpp"
#include "recording.hpp"
#include "automaton.hpp"
#include "cycle.hpp"


static uint64_t ms_automaton_start;
static uint64_t ms_automaton_load_total = 0;
static uint64_t ms_automaton_replay_total = 0;
static size_t automaton_total_divisor = 0;

static uint64_t ms_phase_start;
static uint64_t ms_action_phase_total = 0;
static uint64_t ms_resting_phase_total = 0;
static size_t phase_total_divisor = 0;

static inline void replay(Recording& recording, bool reenactFromOrders)
{
	uint64_t ms = SteadyClock::milliseconds();
	ms_automaton_start = ms;

	Automaton automaton(recording.getPlayers(), recording.getRuleset());
	automaton.replay(recording, reenactFromOrders);

	ms = SteadyClock::milliseconds();
	ms_automaton_load_total += ms - ms_automaton_start;
	ms_automaton_start = ms;
	ms_phase_start = ms;

	Phase phase = Phase::GROWTH;
	while (phase != Phase::DECAY)
	{
		switch (phase)
		{
			case Phase::GROWTH:
			case Phase::ACTION:
			{
				if (automaton.active())
				{
					automaton.act();
				}
				else
				{
					phase = Phase::RESTING;
				}
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
				}
				else if (automaton.replaying())
				{
					automaton.act();
					phase = Phase::ACTION;
				}

				ms = SteadyClock::milliseconds();
				ms_resting_phase_total += ms - ms_phase_start;
				ms_phase_start = ms;

				phase_total_divisor++;
			}
			break;

			case Phase::PLANNING:
			case Phase::STAGING:
			{
				assert(false);
			}
			break;

			case Phase::DECAY:
			break;
		}
	}

	ms = SteadyClock::milliseconds();
	ms_automaton_replay_total += ms - ms_automaton_start;

	automaton_total_divisor++;
}

int main(int argc, char* argv[])
{
	CoreDump::enable();

	std::string logname = "replaytest";

	Settings settings("settings-replaytest.json", argc, argv);

	if (settings.logname.defined())
	{
		logname = settings.logname.value();
	}
	else settings.logname.override(logname);

	std::cout << "[ Epicinium Test ]";
	std::cout << " (" << logname << " v" << Version::current() << ")";
	std::cout << std::endl << std::endl;

	Writer writer;
	writer.install();

	// Defining the data-folder has specific ramifications for this test; we
	// want to be able to specify the location of recordings without affecting
	// the location of the logfiles.
	// TODO resolve this in a better way
	if (settings.dataRoot.defined())
	{
		// LogInstaller::setRoot(settings.dataRoot.value());
		Recording::setRoot(settings.dataRoot.value());
	}

	if (!settings.logrollback.defined()) settings.logrollback.override(5);
	LogInstaller(settings).install();

	LOGI << "Start v" << Version::current();

	Library library;
	library.load();
	library.install();

	std::string indexfilename;
	std::vector<std::string> recnames;
	bool reenactFromOrders = false;

	for (int i = 1; i < argc; i++)
	{
		const char* arg = argv[i];
		size_t arglen = strlen(arg);
		if (strncmp(arg, "-", 1) == 0)
		{
			// Setting argument, will be handled by Settings.
		}
		else if (arglen > 11 + 4
			&& strncmp(arg, "recordings/", 11) == 0
			&& strncmp(arg + arglen - 4, ".rec", 4) == 0)
		{
			recnames.emplace_back(arg + 11, arglen - 11 - 4);
		}
		else if (arglen > 11 + 5
			&& strncmp(arg, "recordings/", 11) == 0
			&& strncmp(arg + arglen - 5, ".list", 5) == 0)
		{
			indexfilename = std::string(arg + 11, arglen - 11 - 4);
		}
		else if (arglen >= 6
			&& strncmp(arg + arglen - 6, "orders", 6) == 0)
		{
			reenactFromOrders = true;
		}
		else
		{
			throw std::runtime_error("unknown argument "
				"'" + std::string(arg) + "'");
		}
	}

	if (!indexfilename.empty() && !recnames.empty())
	{
		LOGE << "Cannot specify both index and filenames";
		throw std::runtime_error("cannot specify both index and filenames");
	}

	if (!indexfilename.empty())
	{
		std::ifstream file = System::ifstream(indexfilename);
		if (!file)
		{
			LOGE << "Failed to open '" << indexfilename << "'";
			throw std::runtime_error("Failed to open '" + indexfilename + "'");
		}

		std::string line;
		while (std::getline(file, line))
		{
			size_t extpos = line.find(".rec");
			if (extpos != std::string::npos)
			{
				recnames.push_back(line.substr(0, extpos));
			}
			else recnames.push_back(line);
		}
	}

	if (settings.seed.defined())
	{
		srand(settings.seed.value());
	}

	if (!recnames.empty())
	{
		for (const std::string& recname : recnames)
		{
			Recording recording(recname);
			replay(recording, reenactFromOrders);
		}
	}
	else if (settings.dataRoot.defined())
	{
		// The recordings might be saved on a harddisk instead of on an SSD;
		// to prevent the difference in diskspeed from affecting the tests,
		// we copy them to a hidden file in the 'local' recordings folder.
		std::vector<Recording> datarecordings = Recording::list(1000);
		if (datarecordings.empty())
		{
			throw std::runtime_error("Failed to open data");
		}

		Recording::setRoot("");
		for (Recording& data : datarecordings)
		{
			Recording recording(".replaytest");
			System::copyFile(data.filename(), recording.filename());
			replay(recording, reenactFromOrders);
		}
	}
	else
	{
		std::vector<Recording> recordings = Recording::list(1000);
		if (recordings.empty())
		{
			throw std::runtime_error("Failed to open history");
		}

		for (Recording& recording : recordings)
		{
			replay(recording, reenactFromOrders);
		}
	}

	if (automaton_total_divisor > 0 && phase_total_divisor > 0)
	{
		PERFLOGI << "timeframe_in_seconds = "
			<< ((ms_automaton_load_total + ms_automaton_replay_total) / 1000);
		PERFLOGI << "avg_automaton_load = "
			<< (1.0 * ms_automaton_load_total / automaton_total_divisor);
		PERFLOGI << "avg_automaton_replay = "
			<< (1.0 * ms_automaton_replay_total / automaton_total_divisor);
		PERFLOGI << "avg_action_phase = "
			<< (1.0 * ms_action_phase_total / phase_total_divisor);
		PERFLOGI << "avg_resting_phase = "
			<< (1.0 * ms_resting_phase_total / phase_total_divisor);
	}

	LOGI << "OK";

	std::cout << std::endl << "[ Done ]" << std::endl;
	return 0;
}
