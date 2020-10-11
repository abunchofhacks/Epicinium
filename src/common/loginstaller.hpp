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

#include "logcallback.h"

class Settings;


class LogInstaller
{
public:
	explicit LogInstaller(const Settings& settings);
	explicit LogInstaller(const std::string& name, int rollback,
		const std::string& level = "", const std::string& perf = "");
	explicit LogInstaller(log_callback_fn callback, uint8_t severity);

private:
	const std::string _name;
	const std::string _level;
	const std::string _perf;
	const int _rollback;

	int _verboserollback = 0;

public:
	void install();

	LogInstaller& withSeparateVerboseRollback(int verboserollback)
	{
		_verboserollback = verboserollback;
		return *this;
	}

	std::string perflogfilename() const;

private:
	static std::string _logsfolder;

public:
	static void setRoot(const std::string& root);
	static std::string getLogsFolderWithSlash() { return _logsfolder; }
};
