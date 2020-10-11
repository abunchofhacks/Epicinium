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

enum class Player : uint8_t;


class Recording
{
public:
	explicit Recording();
	explicit Recording(const std::string& name);
	~Recording();
	Recording(const Recording&) = delete;
	Recording(Recording&& that);
	Recording& operator=(const Recording&) = delete;
	Recording& operator=(Recording&& that);

private:
	std::string _name;
	std::string _filename;
	std::unique_ptr<Json::Value> _metadata;
	bool _listed = true;

public:
	const std::string& name()     const { return _name;     }
	const std::string& filename() const { return _filename; }

	static std::string filename(const std::string& name);

	explicit operator bool() const { return !_name.empty(); }

	bool operator==(const Recording& other) { return _name == other._name; }

	void start();
	void start(const std::string& name);
	void end();

	void addMetadata(Json::Value& metadata);

	Json::Value metadata();

	std::vector<Player> getPlayers();
	std::string getRuleset();

	static bool exists(const std::string& name);

	static std::vector<Recording> list(int count);

private:
	static std::string _recordingsfolder;
	static std::string _historyfilename;

public:
	static void setRoot(const std::string& root);
};
