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
#include "notice.hpp"
#include "source.hpp"

#include "parseerror.hpp"


Notice parseNotice(const std::string& str)
{
	if      (str == "none")                return Notice::NONE;
	else if (str == "orderinvalid")        return Notice::ORDERINVALID;
	else if (str == "halted")              return Notice::HALTED;
	else if (str == "destinationoccupied") return Notice::DESTINATIONOCCUPIED;
	else if (str == "subjectoccupied")     return Notice::SUBJECTOCCUPIED;
	else if (str == "targetoccupied")      return Notice::TARGETOCCUPIED;
	else if (str == "subjectkilled")       return Notice::SUBJECTKILLED;
	else if (str == "notarget")            return Notice::NOTARGET;
	else if (str == "unbuildable")         return Notice::UNBUILDABLE;
	else if (str == "unwalkable")          return Notice::UNWALKABLE;
	else if (str == "inaccessible")        return Notice::INACCESSIBLE;
	else if (str == "lackingstacks")       return Notice::LACKINGSTACKS;
	else if (str == "lackingpower")        return Notice::LACKINGPOWER;
	else if (str == "lackingmoney")        return Notice::LACKINGMONEY;
	else if (str == "occupiedbyenemy")     return Notice::OCCUPIEDBYENEMY;
	else if (str == "activeattack") return Notice::ACTIVEATTACK;
	else if (str == "retaliationattack") return Notice::RETALIATIONATTACK;
	else if (str == "focusattack") return Notice::FOCUSATTACK;
	else if (str == "triggeredfocusattack") return Notice::TRIGGEREDFOCUSATTACK;
	else if (str == "triggeredlockdownattack") return Notice::TRIGGEREDLOCKDOWNATTACK;
	else if (str == "opportunityattack") return Notice::OPPORTUNITYATTACK;
	else if (str == "resigned")            return Notice::RESIGNED;
	else if (str == "roundlimit")          return Notice::ROUNDLIMIT;
	else throw ParseError("Unknown notice '" + str + "'");
}

const char* stringify(const Notice& notice)
{
	switch (notice)
	{
		case Notice::NONE:                return "none";
		case Notice::ORDERINVALID:        return "orderinvalid";
		case Notice::HALTED:              return "halted";
		case Notice::DESTINATIONOCCUPIED: return "destinationoccupied";
		case Notice::SUBJECTOCCUPIED:     return "subjectoccupied";
		case Notice::TARGETOCCUPIED:      return "targetoccupied";
		case Notice::SUBJECTKILLED:       return "subjectkilled";
		case Notice::NOTARGET:            return "notarget";
		case Notice::UNBUILDABLE:         return "unbuildable";
		case Notice::UNWALKABLE:          return "unwalkable";
		case Notice::INACCESSIBLE:        return "inaccessible";
		case Notice::LACKINGSTACKS:       return "lackingstacks";
		case Notice::LACKINGPOWER:        return "lackingpower";
		case Notice::LACKINGMONEY:        return "lackingmoney";
		case Notice::OCCUPIEDBYENEMY:     return "occupiedbyenemy";
		case Notice::ACTIVEATTACK: return "activeattack";
		case Notice::RETALIATIONATTACK: return "retaliationattack";
		case Notice::FOCUSATTACK: return "focusattack";
		case Notice::TRIGGEREDFOCUSATTACK: return "triggeredfocusattack";
		case Notice::TRIGGEREDLOCKDOWNATTACK: return "triggeredlockdownattack";
		case Notice::OPPORTUNITYATTACK: return "opportunityattack";
		case Notice::RESIGNED:            return "resigned";
		case Notice::ROUNDLIMIT:          return "roundlimit";
	}
	return "error";
}

std::ostream& operator<<(std::ostream& os, const Notice& notice)
{
	return os << stringify(notice);
}
