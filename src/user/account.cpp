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
#include "account.hpp"
#include "source.hpp"

#include "system.hpp"


std::string Account::_accountsfolder = "accounts/";
std::string Account::_historyfilename = "accounts/history.list";

void Account::setRoot(const std::string& root)
{
	if (root.empty())
	{
		_accountsfolder = "accounts/";
	}
	else if (root.back() == '/')
	{
		_accountsfolder = root + "accounts/";
	}
	else
	{
		_accountsfolder = root + "/accounts/";
	}

	_historyfilename = _accountsfolder + "history.list";
}

Account::Account() :
	_id(0),
	_sessionToken(""),
	_rememberSession(false)
{}

std::ostream& operator<<(std::ostream& os, const Account& account);

void Account::save()
{
	if (_id == 0)
	{
		LOGW << "Trying to save account while not logged in";
		return;
	}

	System::touchDirectory(_accountsfolder);

	std::ofstream file = System::ofstream(
		_accountsfolder + std::to_string(_id) + ".acc",
		std::ofstream::out | std::ofstream::trunc);
	file << (*this) << std::endl;

	std::ofstream index = System::ofstream(_historyfilename,
		std::ofstream::out | std::ofstream::trunc);
	index << std::to_string(_id) << std::endl;
}

std::string Account::lastSessionId()
{
	static constexpr int LENGTH = 80;
	static char buffer[LENGTH + 1] = {0};

	std::ifstream index = System::ifstream(_historyfilename,
		std::ifstream::in | std::ifstream::binary);
	index.seekg(0, std::ifstream::end);

	std::string id = "";
	while (index.tellg() > 0)
	{
		int i = LENGTH;
		while (index.tellg() > 0 && i >= 0)
		{
			index.seekg(-1, std::ifstream::cur);
			if (index.peek() != '\r' && index.peek() != '\n')
			{
				--i;
				buffer[i] = index.peek();
			}
			else
			{
				break;
			}
		}
		if (i < LENGTH)
		{
			id = std::string(buffer + i);
		}
	}
	return id;
}

bool Account::existsSession()
{
	std::string id = lastSessionId();
	return (!id.empty());
}

bool Account::loadSession()
{
	std::string id = lastSessionId();
	if (id.empty()) return false;

	std::ifstream file;
	try
	{
		file = System::ifstream(_accountsfolder + id + ".acc");
	}
	catch (const std::ifstream::failure&)
	{
		return false;
	}
	if (!file.is_open()) return false;

	Json::Reader reader;
	Json::Value json;
	std::string line;
	if (!std::getline(file, line)) return false;
	if (!reader.parse(line, json)) return false;
	if (!json.isObject()) return false;
	fromJson(json);
	if (_id == 0 || _sessionToken.empty())
	{
		reset();
		return false;
	}
	_rememberSession = true;

	return true;
}

void Account::set(uint32_t id, const std::string& token, bool rememberSession)
{
	reset();
	_id = id;
	_sessionToken = token;
	_rememberSession = rememberSession;
	if (_rememberSession) save();
}

void Account::reset()
{
	_sessionToken = "";
	_username = "";
	if (_rememberSession) save();
	_id = 0;
	_linked = false;
	_rememberSession = false;
}

void Account::setUsername(const std::string& username)
{
	_username = username;
	if (!username.empty() && _rememberSession) save();
}

bool Account::linked() const
{
	return _linked;
}

void Account::link()
{
	_linked = true;
}

Json::Value Account::toJson() const
{
	Json::Value json(Json::objectValue);

	json["id"] = _id;
	json["session_token"] = _sessionToken;
	json["username"] = _username;

	return json;
}

void Account::fromJson(const Json::Value& json)
{
	if (json["id"].isUInt())
	{
		_id = json["id"].asUInt();
	}
	if (json["session_token"].isString())
	{
		_sessionToken = json["session_token"].asString();
	}
}

std::ostream& operator<<(std::ostream& os, const Account& account)
{
	return os << "{"
		"\"id\":" << account.id() << ""
		","
		"\"session_token\":\"" << account.sessionToken() << "\""
		","
		"\"username\":\"" << account.username() << "\""
		"}";
}
