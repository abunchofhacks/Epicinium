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
#include "recordingiterator.hpp"
#include "source.hpp"

#include "recording.hpp"


RecordingIterator::RecordingIterator(
		const TypeNamer& typenamer, const Recording& recording) :
	_typenamer(typenamer),
	_name(recording.name()),
	_file(recording.filename()),
	_linenumber(0)
{
	if (!_file) return;

	if (!std::getline(_file, _line) || !_reader.parse(_line, _json)
		|| (!_json.isObject() && !_json.isArray()))
	{
		_file.close();
		return;
	}

	// Pre-historic recording?
	if (!_json.isObject())
	{
		_changeset = ChangeSet(_typenamer, _json);
	}
	else
	{
		++(*this);
	}
}

RecordingIterator::~RecordingIterator() = default;

RecordingIterator& RecordingIterator::operator++()
{
	if (!_file) return *this;

	if (!std::getline(_file, _line) || !_reader.parse(_line, _json)
		|| !_json.isArray())
	{
		_file.close();
		return *this;
	}

	_changeset = ChangeSet(_typenamer, _json);
	++_linenumber;

	return *this;
}
