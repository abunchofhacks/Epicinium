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


#if DICTATOR_ENABLED
/* ############################ DICTATOR_ENABLED ############################ */

#include <mutex>
#include <thread>
#include <queue>
#include <atomic>


class Dictator
{
public:
	explicit Dictator();
	explicit Dictator(bool live);
	~Dictator();
	Dictator(const Dictator& that) = delete;
	Dictator(Dictator&& that) = delete;
	Dictator& operator=(const Dictator& that) = delete;
	Dictator& operator=(Dictator&& that) = delete;

private:
	std::atomic<bool> _alive;
	std::atomic<bool> _threadrunning;
	std::mutex _mandatemutex;
	std::queue<std::string> _mandatebuffer;
	std::thread _thread;

public:
	std::vector<std::string> sendbuffer;
	std::vector<std::string> recvbuffer;

	std::string textInput;
	std::string textEdit;
	std::vector<int> scancodesUp;
	std::vector<int> scancodesDown;
	std::vector<int> scancodesRepeat;
	int mousex = 0;
	int mousey = 0;
	int scroll = 0;

	void update();

private:
	void runThread();
};

/* ############################ DICTATOR_ENABLED ############################ */
#else
/* ########################## not DICTATOR_ENABLED ########################## */

class Dictator
{
	// Nothing.
};

/* ########################## not DICTATOR_ENABLED ########################## */
#endif
