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

#include <fstream>
#include "libs/jsoncpp/json.h"

#include "changeset.hpp"

class TypeNamer;
class Recording;


class RecordingIterator
{
public:
	RecordingIterator(const TypeNamer& typenamer, const Recording& recording);
	~RecordingIterator();
	RecordingIterator(const RecordingIterator&) = delete;
	RecordingIterator(RecordingIterator&&) = delete;
	RecordingIterator& operator=(const RecordingIterator&) = delete;
	RecordingIterator& operator=(RecordingIterator&&) = delete;

private:
	const TypeNamer& _typenamer;
	std::string _name;
	std::ifstream _file;
	size_t _linenumber;

	Json::Reader _reader;
	Json::Value _json;
	std::string _line;

	ChangeSet _changeset;

public:
	const ChangeSet& operator*() const
	{
		return _changeset;
	}

	explicit operator bool() const
	{
		return _file.is_open();
	}

	RecordingIterator& operator++();

	size_t linenumber() const { return _linenumber; }
};
