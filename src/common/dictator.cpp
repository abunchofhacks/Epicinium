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
#include "dictator.hpp"
#include "source.hpp"


#if DICTATOR_ENABLED
/* ############################ DICTATOR_ENABLED ############################ */

#include <unistd.h>


Dictator::Dictator() :
	Dictator(true)
{}

Dictator::Dictator(bool live) :
	_alive(live),
	_threadrunning(false)
{
	// If this program is run directly from the terminal, then trying to peek
	// from stdin will block until the user inputs either a character or
	// manually hits Ctrl+D to enter an EOF. So in that case we are disabled.
	if (_alive && isatty(fileno(stdin)))
	{
		_alive = false;
	}

	if (_alive)
	{
		// Start the thread.
		_threadrunning = true;
		_thread = std::thread(&Dictator::runThread, this);
	}
}

Dictator::~Dictator()
{
	// We are shutting down.
	_alive = false;

	// We need to wait for the thread to finish or risk memory crashes.
	if (_threadrunning)
	{
		LOGI << "Waiting until thread finishes...";
	}
	if (_thread.joinable()) _thread.join();

	LOGI << "Thread finished.";
}

void Dictator::update()
{
	sendbuffer.clear();
	recvbuffer.clear();

	textInput.clear();
	textEdit.clear();
	scancodesUp.clear();
	scancodesDown.clear();
	scancodesRepeat.clear();
	mousex = 0;
	mousey = 0;
	scroll = 0;

	std::string mandate;
	{
		// We need a lock around the buffer to prevent it from being
		// written to as we are reading from it.
		std::lock_guard<std::mutex> lock(_mandatemutex);

		if (_mandatebuffer.empty()) return;

		mandate = std::move(_mandatebuffer.front());
		_mandatebuffer.pop();
	}

	mandate += ";";
	size_t start = 0;
	size_t startofcommand;
	while ((startofcommand = mandate.find_first_not_of(" ;", start))
		!= std::string::npos)
	{
		size_t endofcommand = mandate.find_first_of(" ;", startofcommand);
		size_t startofdata = mandate.find_first_not_of(' ', endofcommand);
		size_t endofdata = mandate.find_first_of(';', startofdata);
		std::string command = mandate.substr(startofcommand,
			endofcommand - startofcommand);
		if (command.compare("print") == 0)
		{
			std::cout
				<< mandate.substr(startofdata, endofdata - startofdata)
				<< std::endl;
		}
		else if (command.compare("send") == 0)
		{
			sendbuffer.emplace_back(
				mandate.substr(startofdata, endofdata - startofdata));
		}
		else if (command.compare("recv") == 0)
		{
			recvbuffer.emplace_back(
				mandate.substr(startofdata, endofdata - startofdata));
		}
		else if (command.compare("input") == 0)
		{
			textInput = mandate.substr(startofdata, endofdata - startofdata);
		}
		else if (command.compare("edit") == 0)
		{
			textEdit = mandate.substr(startofdata, endofdata - startofdata);
		}
		else if (command.compare("up") == 0)
		{
			int scancode = atoi(mandate.c_str() + startofdata);
			scancodesUp.push_back(scancode);
		}
		else if (command.compare("down") == 0)
		{
			int scancode = atoi(mandate.c_str() + startofdata);
			scancodesDown.push_back(scancode);
		}
		else if (command.compare("repeat") == 0)
		{
			int scancode = atoi(mandate.c_str() + startofdata);
			scancodesRepeat.push_back(scancode);
		}
		else if (command.compare("mousex") == 0)
		{
			int amount = atoi(mandate.c_str() + startofdata);
			mousex += amount;
		}
		else if (command.compare("mousey") == 0)
		{
			int amount = atoi(mandate.c_str() + startofdata);
			mousey += amount;
		}
		else if (command.compare("scroll") == 0)
		{
			int amount = atoi(mandate.c_str() + startofdata);
			scroll += amount;
		}
		start = endofdata;
	}
}

void Dictator::runThread()
{
	// This peek blocks if there are no characters left to read, but if the
	// standard in was never opened, it should return EOF immediately.
	if (std::cin.peek() == std::char_traits<char>::eof())
	{
		LOGI << "No instructions";

		// Finish this thread.
		_threadrunning = false;
		return;
	}

	LOGI << "Awaiting instructions";

	std::string line;
	// This getline blocks until there is a newline character.
	while (std::getline(std::cin, line))
	{
		// We need a lock around the buffer to prevent it from being
		// read from as we are writing from it.
		std::lock_guard<std::mutex> lock(_mandatemutex);

		_mandatebuffer.emplace(std::move(line));
	}

	LOGI << "Finished instructions";

	// Finish this thread.
	_threadrunning = false;
	return;
}

/* ############################ DICTATOR_ENABLED ############################ */
#endif
