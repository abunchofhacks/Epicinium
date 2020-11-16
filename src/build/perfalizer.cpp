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

#include "libs/termcolor/termcolor.hpp"

#include "settings.hpp"
#include "loginstaller.hpp"
#include "system.hpp"


#define PARSEINT(VARIABLE) \
	if (line.compare(namepos, namelen, #VARIABLE) == 0) \
	{ \
		std::string valstr = line.substr(valpos); \
		result.VARIABLE = atoi(valstr.c_str()); \
	}

#define PARSEFLOAT(VARIABLE) \
	if (line.compare(namepos, namelen, #VARIABLE) == 0) \
	{ \
		std::string valstr = line.substr(valpos); \
		result.VARIABLE = (float) atof(valstr.c_str()); \
	}

struct Result
{
	int pid = 0;

	// General results.
	int timeframe_in_seconds = 0;

	// Graphical results.
	float framerate_average = 0;
	float framerate_percentage = 0;
	float avg_start_delay = 0;
	float avg_runtime_flip = 0;
	float avg_runtime_update = 0;
	float avg_runtime_draw = 0;
	float avg_runtime_finish = 0;
	float avg_duration = 0;

	// Logical results.
	float avg_automaton_load = 0;
	float avg_automaton_replay = 0;
	float avg_action_phase = 0;
	float avg_resting_phase = 0;
	float avg_planning_phase = 0;
	float avg_staging_phase = 0;
};

static inline Result parseResult(const std::string& fname, int pid, int skippid)
{
	Result result;

	std::ifstream file = System::ifstream(fname);
	if (!file)
	{
		throw std::runtime_error("failed to open file '" + fname + "'");
	}

	bool empty = true;

	std::string pidstr = (pid > 0) ? std::to_string(pid) : "";
	std::string skipstr = (skippid > 0) ? std::to_string(skippid) : "";

	std::string line;
	while (std::getline(file, line))
	{
		size_t pos = line.find("INFO");
		if (pos == std::string::npos) continue;

		pos = line.find_first_of('[', pos);
		assert(pos != std::string::npos);
		pos++;
		assert(pos < line.size());
		size_t endpos = line.find_first_of(']', pos);
		assert(endpos != std::string::npos);

		if (pid > 0)
		{
			if (line.compare(pos, endpos - pos, pidstr) != 0)
			{
				continue;
			}
		}
		else if (skippid > 0 && line.compare(pos, endpos - pos, skipstr) == 0)
		{
			continue;
		}
		else if (pidstr.empty())
		{
			pidstr = line.substr(pos, endpos - pos);
		}
		else if (line.compare(pos, endpos - pos, pidstr) != 0)
		{
			pidstr = line.substr(pos, endpos - pos);
			result = Result();
		}

		assert(endpos + 1 < line.size());
		endpos = line.find_first_of(']', endpos + 1);
		assert(endpos != std::string::npos);
		size_t namepos = endpos + 2;
		assert(namepos < line.size());

		size_t valpos = line.find_first_of('=', namepos);
		if (valpos == std::string::npos) continue;

		assert(valpos > namepos);
		size_t namelen = valpos - namepos - 1;
		valpos++;
		assert(valpos < line.size());

		// We now have a name and a value, separated by an assignment operator.
		empty = false;

		// General results.
		PARSEINT(timeframe_in_seconds)

		// Graphical results.
		PARSEFLOAT(framerate_average)
		PARSEFLOAT(framerate_percentage)
		PARSEFLOAT(avg_start_delay)
		PARSEFLOAT(avg_runtime_flip)
		PARSEFLOAT(avg_runtime_update)
		PARSEFLOAT(avg_runtime_draw)
		PARSEFLOAT(avg_runtime_finish)
		PARSEFLOAT(avg_duration)

		// Logical results.
		PARSEFLOAT(avg_automaton_load)
		PARSEFLOAT(avg_automaton_replay)
		PARSEFLOAT(avg_action_phase)
		PARSEFLOAT(avg_resting_phase)
		PARSEFLOAT(avg_planning_phase)
		PARSEFLOAT(avg_staging_phase)
	}

	if (empty)
	{
		if (pid > 0)
		{
			throw std::runtime_error("failed to find"
				" pid " + pidstr + ""
				" in file '" + fname + "'");
		}
		else if (skippid > 0)
		{
			throw std::runtime_error("failed to find"
				" anything other than " + skipstr + ""
				" in file '" + fname + "'");
		}
		else
		{
			throw std::runtime_error("failed to find"
				" anything"
				" in file '" + fname + "'");
		}
	}
	else
	{
		result.pid = atoi(pidstr.c_str());
	}

	return result;
}

#define BASECOLOR termcolor::yellow
#define TESTCOLOR termcolor::cyan

#define WARNIFBELOW_(NAME, VARIABLE, NORMAL, CRITICAL, WARNINGTEXT) \
	{ \
		if (NAME.VARIABLE < NORMAL) \
		{ \
			std::cout << NAME##COLOR << #NAME "." #VARIABLE \
				<< termcolor::white << " = "; \
			if (NAME.VARIABLE < CRITICAL) \
			{ \
				std::cout << termcolor::white << termcolor::on_red; \
			} \
			else if (NAME.VARIABLE < NORMAL) \
			{ \
				std::cout << termcolor::red; \
			} \
			std::cout << termcolor::bold << " " \
				<< NAME.VARIABLE \
				<< " " << termcolor::reset; \
			std::cout << termcolor::white << "; " WARNINGTEXT; \
			std::cout << std::endl; \
		} \
	}

#define WARNIFBELOW(VARIABLE, NORMAL, CRITICAL, WARNINGTEXT) \
	{ \
		WARNIFBELOW_(BASE, VARIABLE, NORMAL, CRITICAL, WARNINGTEXT) \
		WARNIFBELOW_(TEST, VARIABLE, NORMAL, CRITICAL, WARNINGTEXT) \
	}

#define PRINTHEADER() \
	{ \
	std::cout << std::setfill(' ') << std::left << std::setw(30) << " " \
		<< std::setfill('=') \
		<< " " << BASECOLOR << "==" << std::setw(10) << "BASE" \
		<< " " << TESTCOLOR << "==" << std::setw(10) << "TEST" \
		<< termcolor::white \
		<< std::endl; \
	}

#define PRINTGOODNESS(VARIABLE, SIGNIFICANT, EXTREME) \
	if (BASE.VARIABLE != 0 || TEST.VARIABLE != 0) \
	{ \
	std::cout << std::setfill('.') << std::left << termcolor::white \
		<< std::setw(30) << #VARIABLE << std::setfill(' ') << std::right \
		<< " " << std::fixed << std::setprecision(2) \
		<< " " << std::setw(10) << BASE.VARIABLE << " " \
		<< " " << std::fixed << std::setprecision(2); \
	if (TEST.VARIABLE < BASE.VARIABLE - EXTREME) \
	{ \
		std::cout << termcolor::bold; \
		std::cout << termcolor::white << termcolor::on_red; \
	} \
	else if (TEST.VARIABLE < BASE.VARIABLE - SIGNIFICANT) \
	{ \
		std::cout << termcolor::bold; \
		std::cout << termcolor::red; \
	} \
	else if (TEST.VARIABLE < BASE.VARIABLE + SIGNIFICANT) \
	{ \
		std::cout << termcolor::white; \
	} \
	else if (TEST.VARIABLE < BASE.VARIABLE + EXTREME) \
	{ \
		std::cout << termcolor::bold; \
		std::cout << termcolor::green; \
	} \
	else \
	{ \
		std::cout << termcolor::bold; \
		std::cout << termcolor::white << termcolor::on_green; \
	} \
	std::cout \
		<< " " << std::setw(10) << TEST.VARIABLE << " " \
		<< termcolor::reset \
		<< std::endl; \
	}

#define PRINTBADNESS(VARIABLE, SIGNIFICANT, EXTREME) \
	if (BASE.VARIABLE != 0 || TEST.VARIABLE != 0) \
	{ \
	std::cout << std::setfill('.') << std::left << termcolor::white \
		<< std::setw(30) << #VARIABLE << std::setfill(' ') << std::right \
		<< " " << std::fixed << std::setprecision(2) \
		<< " " << std::setw(10) << BASE.VARIABLE << " " \
		<< " " << std::fixed << std::setprecision(2); \
	if (TEST.VARIABLE < BASE.VARIABLE - EXTREME) \
	{ \
		std::cout << termcolor::bold; \
		std::cout << termcolor::white << termcolor::on_green; \
	} \
	else if (TEST.VARIABLE < BASE.VARIABLE - SIGNIFICANT) \
	{ \
		std::cout << termcolor::bold; \
		std::cout << termcolor::green; \
	} \
	else if (TEST.VARIABLE < BASE.VARIABLE + SIGNIFICANT) \
	{ \
		std::cout << termcolor::white; \
	} \
	else if (TEST.VARIABLE < BASE.VARIABLE + EXTREME) \
	{ \
		std::cout << termcolor::bold; \
		std::cout << termcolor::red; \
	} \
	else \
	{ \
		std::cout << termcolor::bold; \
		std::cout << termcolor::white << termcolor::on_red; \
	} \
	std::cout \
		<< " " << std::setw(10) << TEST.VARIABLE << " " \
		<< termcolor::reset \
		<< std::endl; \
	}

int main(int argc, char* argv[])
{
	Settings settings("settings-perfalizer.json", argc, argv);

	if (!settings.logname.defined())
	{
		throw std::runtime_error("no logname defined");
	}
	else if (!settings.perflog.defined())
	{
		throw std::runtime_error("no perflog defined");
	}

	std::string logname = settings.logname.value();
	std::string testname = settings.perflog.value();
	std::string basename = testname;
	int basepid = 0;
	int testpid = 0;
	for (int i = 1; i < argc; i++)
	{
		const char* arg = argv[i];
		size_t arglen = strlen(arg);
		if (strncmp(arg, "-", 1) == 0)
		{
			// Setting argument, will be handled by Settings.
		}
		else if (strspn(arg, "0123456789") < arglen)
		{
			basename = arg;
			if (basename == testname)
			{
				throw std::runtime_error("basename argument same as testname");
			}
		}
		else if (testpid == 0)
		{
			testpid = atoi(arg);
		}
		else if (basepid == 0)
		{
			basepid = testpid;
			testpid = atoi(arg);
		}
		else
		{
			throw std::runtime_error("unknown argument "
				"'" + std::string(arg) + "'");
		}
	}

	Result TEST;
	{
		if (testpid < 0)
		{
			throw std::runtime_error("invalid pid " + std::to_string(testpid));
		}
		std::string testfname = LogInstaller(settings).perflogfilename();
		TEST = parseResult(testfname, testpid, testpid);
		testpid = TEST.pid;
	}

	Result BASE;
	{
		Json::Value json = settings.flattenIntoJson();
		Settings basesettings(json);
		if (basename != testname)
		{
			basesettings.perflog.override(basename);
			basepid = 0;
		}
		std::string basefname = LogInstaller(basesettings).perflogfilename();
		BASE = parseResult(basefname, basepid, testpid);
	}

	std::cout << std::endl;
	std::cout << termcolor::white << "Comparing "
		<< BASECOLOR
		<< basename << "/" << std::to_string(BASE.pid) << " (BASE)"
		<< termcolor::white
		<< " with "
		<< TESTCOLOR
		<< testname << "/" << std::to_string(TEST.pid) << " (TEST)"
		<< termcolor::white
		<< std::endl;
	WARNIFBELOW(timeframe_in_seconds, 10, 5, "performance might be unreliable")
	std::cout << std::endl;

	PRINTHEADER()

	// Graphical results.
	PRINTGOODNESS(framerate_average, 2, 10)
	PRINTGOODNESS(framerate_percentage, 1, 5)
	PRINTBADNESS(avg_start_delay, 0.5f, 1.0f)
	PRINTBADNESS(avg_runtime_flip, 0.5f, 1.0f)
	PRINTBADNESS(avg_runtime_update, 1.0f, 2.0f)
	PRINTBADNESS(avg_runtime_draw, 1.0f, 2.0f)
	PRINTBADNESS(avg_runtime_finish, 0.5f, 1.0f)
	PRINTBADNESS(avg_duration, 1.0f, 2.0f)

	// Logical results.
	PRINTBADNESS(avg_automaton_load, 0.5f, 1.0f)
	PRINTBADNESS(avg_automaton_replay, 2.0f, 5.0f)
	PRINTBADNESS(avg_action_phase, 0.5f, 1.0f)
	PRINTBADNESS(avg_resting_phase, 0.5f, 1.0f)
	PRINTBADNESS(avg_planning_phase, 0.5f, 1.0f)
	PRINTBADNESS(avg_staging_phase, 0.5f, 1.0f)

	std::cout << std::endl;

	return 0;
}
