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

#include <mutex>
#include <condition_variable>
#include <thread>
#include <queue>
#include <atomic>

class Curl;


class DiscordCDN
{
public:
	DiscordCDN(std::shared_ptr<Curl> curl);
	~DiscordCDN();
	DiscordCDN(const DiscordCDN&) = delete;
	DiscordCDN(DiscordCDN&&) = delete;
	DiscordCDN& operator=(const DiscordCDN&) = delete;
	DiscordCDN& operator=(DiscordCDN&&) = delete;

private:
	std::atomic<bool> _alive;
	std::atomic<bool> _threadrunning;
	std::mutex _requestmutex;
	std::mutex _fulfilledmutex;
	std::condition_variable _requestnotifier;
	std::queue<std::string> _requestbuffer;
	std::vector<std::string> _fulfilled;
	std::shared_ptr<Curl> _curl;
	std::thread _thread;

public:
	std::string getAvatarPicture(const char* userid, const char* avatar);

	bool update(std::vector<std::string>& buffer);

private:
	void runThread();
};


