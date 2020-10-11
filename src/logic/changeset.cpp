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
#include "changeset.hpp"
#include "source.hpp"

#include "typenamer.hpp"


ChangeSet::ChangeSet(const TypeNamer& namer, const Json::Value& json)
{
	for (auto& pair : json)
	{
		_data.emplace_back(std::piecewise_construct,
			std::forward_as_tuple(namer, pair["change"]),
			std::forward_as_tuple(pair["vision"]));
	}
}

std::ostream& operator<<(std::ostream& os, const ChangeSet& changeset)
{
	os << "[";
	bool empty = true;
	for (auto& datum : changeset._data)
	{
		if (empty) empty = false;
		else os << ",";
		os << "{"
			"\"change\":" << datum.first << ""
			","
			"\"vision\":" << datum.second << ""
			"}";
	}
	return os << "]";
}

bool ChangeSet::equal(const TypeNamer& namer,
	const ChangeSet& a, const ChangeSet& b)
{
	size_t i = 0;
	for (; i < a._data.size() && i < b._data.size(); i++)
	{
		const auto& aa = a._data[i];
		const auto& bb = b._data[i];

		std::string aacs;
		{
			std::stringstream strm;
			strm << TypeEncoder(&namer) << aa.first;
			aacs = strm.str();
		}
		std::string bbcs;
		{
			std::stringstream strm;
			strm << TypeEncoder(&namer) << bb.first;
			bbcs = strm.str();
		}

		if (aacs != bbcs)
		{
			LOGD << "difference at [" << i << "]:" << TypeEncoder(&namer)
				<< " " << aacs << " vs " << bbcs;
			return false;
		}
		else if (aa.second != bb.second)
		{
			LOGD << "difference at [" << i << "]:" << TypeEncoder(&namer)
				<< " " << aa.second << " vs " << bb.second;
			return false;
		}
	}

	if (i <  a._data.size() && i >= b._data.size())
	{
		LOGD << "difference at [" << i << "]:" << TypeEncoder(&namer)
			<< " " << a._data[i].first << " vs nothing";
		return false;
	}
	else if (i >= a._data.size() && i <  b._data.size())
	{
		LOGD << "difference at [" << i << "]:" << TypeEncoder(&namer)
			<< " nothing vs " << b._data[i].first;
		return false;
	}
	else return true;
}
