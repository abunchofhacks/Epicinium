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


enum class Notice : uint8_t
{
	NONE = 0,
	ORDERINVALID,
	HALTED,
	DESTINATIONOCCUPIED,
	SUBJECTOCCUPIED,
	TARGETOCCUPIED,
	SUBJECTKILLED,
	NOTARGET,
	UNBUILDABLE,
	UNWALKABLE,
	INACCESSIBLE,
	LACKINGSTACKS,
	LACKINGPOWER,
	LACKINGMONEY,
	OCCUPIEDBYENEMY,
	ACTIVEATTACK,
	RETALIATIONATTACK,
	FOCUSATTACK,
	TRIGGEREDFOCUSATTACK,
	TRIGGEREDLOCKDOWNATTACK,
	OPPORTUNITYATTACK,
	RESIGNED,
	ROUNDLIMIT,
};

Notice parseNotice(const std::string& str);
const char* stringify(const Notice& notice);

std::ostream& operator<<(std::ostream& os, const Notice& notice);
