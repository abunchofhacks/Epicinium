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


class Account
{
private:
	static std::string _accountsfolder;
	static std::string _historyfilename;

public:
	static void setRoot(const std::string& root);

	Account();

private:
	uint32_t _id;
	std::string _sessionToken;
	bool _rememberSession;
	std::string _username;

	bool _linked = false;

	void save();

	std::string lastSessionId();

public:
	bool existsSession();
	bool loadSession();

	void set(uint32_t id, const std::string& token, bool rememberSession);
	void reset();

	void setUsername(const std::string& username);

	uint32_t id() const { return _id; }
	const std::string& sessionToken() const { return _sessionToken; }
	bool rememberSession() const { return _rememberSession; }
	const std::string& username() const { return _username; }
	bool linked() const;
	void link();

private:
	Json::Value toJson() const;
	void fromJson(const Json::Value& json);
};
