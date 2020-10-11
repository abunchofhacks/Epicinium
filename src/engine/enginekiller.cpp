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
#include "enginekiller.hpp"
#include "source.hpp"

#include <csignal>

#include "engine.hpp"
#include "exitcode.hpp"


static volatile sig_atomic_t killcount = 0;

void EngineKiller::shutdown(int signum, int threshold)
{
	LOGI << "Received signal " << signum
		<< " (threshold " << killcount << "/" << threshold << ")";
	if (killcount < threshold)
	{
		++killcount;
	}
	else
	{
		signal(signum, SIG_DFL);
		raise(signum);
	}
}

void EngineKiller::shutdown1(int signum)
{
	shutdown(signum, 1);
}

void EngineKiller::shutdown10(int signum)
{
	shutdown(signum, 10);
}

EngineKiller::EngineKiller(Engine& engine) :
	_engine(engine)
{
	// Make sure we can properly shut down the game by calling 'kill <pid>',
	// and that closing the terminal also shuts down the game properly.
	{
#ifdef PLATFORMUNIX
		signal(SIGTERM, shutdown1);
		signal(SIGHUP, shutdown10);
#else
		signal(SIGTERM, shutdown1);
		signal(SIGBREAK, shutdown1);
#endif
	}
}

EngineKiller::~EngineKiller()
{
	// The engine is already shutting down, so there is nothing we can do
	// if we were to catch a signal now. Therefore we revert to default.
	{
#ifdef PLATFORMUNIX
		signal(SIGTERM, SIG_DFL);
		signal(SIGHUP, SIG_DFL);
#else
		signal(SIGTERM, SIG_DFL);
		signal(SIGBREAK, SIG_DFL);
#endif
	}
}

void EngineKiller::check()
{
	if (killcount > 0)
	{
		_engine.quit(ExitCode::TERMINATE);
	}
}
