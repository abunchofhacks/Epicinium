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
#include "launchercommand.hpp"
#include "source.hpp"


#ifdef PLATFORMOSX
/* ################################## OSX ################################## */

#include <mach-o/dyld.h>


const char* LauncherCommand::get()
{
	static std::string command;
	if (command.empty())
	{
		char exePath[1024];
		uint32_t size = sizeof(exePath);
		_NSGetExecutablePath(exePath, &size);
		if (size <= 0)
		{
			return nullptr;
		}
		command = std::string(exePath, size);

		size_t pos = command.find("bin/game");
		if (pos >= command.size())
		{
			return nullptr;
		}
		command.replace(pos, std::string::npos, "epicinium");
	}
	return command.c_str();
}

/* ################################## OSX ################################## */
#endif

#ifdef PLATFORMDEBIAN
/* ################################# LINUX ################################# */

#include <unistd.h>


const char* LauncherCommand::get()
{
	static std::string command;
	if (command.empty())
	{
		char exePath[1024];
		size_t size = readlink("/proc/self/exe", exePath, sizeof(exePath));
		if (size <= 0)
		{
			return nullptr;
		}
		command = std::string(exePath, size);

		size_t pos = command.find("bin/game");
		if (pos >= command.size())
		{
			return nullptr;
		}
		command.replace(pos, std::string::npos, "epicinium");
	}
	return command.c_str();
}

/* ################################# LINUX ################################# */
#endif

#ifdef PLATFORMWINDOWS
/* ################################ WINDOWS ################################ */

#include <windows.h>


const char* LauncherCommand::get()
{
	static std::string command;
	if (command.empty())
	{
		char exePath[1024];
		size_t size = GetModuleFileName(nullptr, exePath, sizeof(exePath));
		if (size <= 0)
		{
			return nullptr;
		}
		command = std::string(exePath, size);

		size_t pos = command.find("bin\\game.exe");
		if (pos >= command.size())
		{
			return nullptr;
		}
		command.replace(pos, std::string::npos, "epicinium.exe");
	}
	return command.c_str();
}

/* ################################ WINDOWS ################################ */
#endif
