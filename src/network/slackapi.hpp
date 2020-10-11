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

#include "setting.hpp"

class Curl;


class SlackAPI
{
private:
	struct Post
	{
		std::string text;
		std::string name;
		const char* emoji;

		Post()
		{}

		Post(const std::string& t, const std::string& n, const char* e) :
			text(t),
			name(n),
			emoji(e)
		{}
	};

public:
	SlackAPI(std::shared_ptr<Curl> curl,
		const Setting<std::string>& url,
		const std::string& channelname,
		const Setting<std::string>& botname,
		const std::string& suffix);
	~SlackAPI();
	SlackAPI(const SlackAPI&) = delete;
	SlackAPI(SlackAPI&&) = delete;
	SlackAPI& operator=(const SlackAPI&) = delete;
	SlackAPI& operator=(SlackAPI&&) = delete;

private:
	std::shared_ptr<Curl> _curl;
	std::string _url;
	std::string _channelname;
	std::string _botname;
	std::atomic<bool> _enabled;

	void post(const std::string& text, const std::string& botname,
		const char* emoji);

public:
	void post(const std::string& text);
	void error(const std::string& text, const std::string& name);
};
