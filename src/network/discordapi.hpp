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

#include <atomic>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <queue>

#include "setting.hpp"

class Curl;


class DiscordAPI
{
private:
	struct Post
	{
		std::string text;
		std::string name;

		Post()
		{}

		Post(const std::string& t, const std::string& n) :
			text(t),
			name(n)
		{}
	};

	static DiscordAPI* _installed;

public:
	static DiscordAPI* get() { return _installed; }

	DiscordAPI(std::shared_ptr<Curl> curl, const Setting<std::string>& url);
	~DiscordAPI();
	DiscordAPI(const DiscordAPI&) = delete;
	DiscordAPI(DiscordAPI&&) = delete;
	DiscordAPI& operator=(const DiscordAPI&) = delete;
	DiscordAPI& operator=(DiscordAPI&&) = delete;

private:
	std::shared_ptr<Curl> _curl;
	std::string _url;
	std::string _botname;
	std::atomic<bool> _enabled;

	std::mutex _buffermutex;
	std::condition_variable _buffernotifier;
	std::queue<Post> _buffer;
	std::thread _sendthread;

	int _retryAfter = 0;

	void runSendThread();

	bool send(const std::string& text, const std::string& name);

	void post(const std::string& text, const std::string& name);

public:
	void post(const std::string& text);
};
