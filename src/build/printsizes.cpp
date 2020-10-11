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
#include "source.hpp"

#include "version.hpp"
#include "position.hpp"
#include "aim.hpp"
#include "move.hpp"
#include "descriptor.hpp"
#include "unittype.hpp"
#include "tiletype.hpp"
#include "unittoken.hpp"
#include "tiletoken.hpp"
#include "player.hpp"
#include "vision.hpp"
#include "cycle.hpp"
#include "notice.hpp"
#include "attacker.hpp"
#include "cell.hpp"
#include "space.hpp"
#include "order.hpp"
#include "change.hpp"
#include "changeset.hpp"
#include "board.hpp"
#include "area.hpp"
#include "bible.hpp"
#include "playerinfo.hpp"
#include "automaton.hpp"


#define PRINTSIZE(NAME) \
	std::cout << "sizeof(" #NAME ") = " << sizeof(NAME) << std::endl;

int main(int /**/, char* /**/[])
{
	PRINTSIZE(Version);
	PRINTSIZE(Position);
	PRINTSIZE(Aim);
	PRINTSIZE(Move);
	PRINTSIZE(Descriptor);
	PRINTSIZE(UnitType);
	PRINTSIZE(TileType);
	PRINTSIZE(UnitToken);
	PRINTSIZE(TileToken);
	PRINTSIZE(UnitTokenWithId);
	PRINTSIZE(TileTokenWithId);
	PRINTSIZE(Player);
	PRINTSIZE(Vision);
	PRINTSIZE(Phase);
	PRINTSIZE(Daytime);
	PRINTSIZE(Season);
	PRINTSIZE(Notice);
	PRINTSIZE(Attacker);
	PRINTSIZE(Bombarder);
	PRINTSIZE(Cell);
	PRINTSIZE(Space);
	PRINTSIZE(Order);
	PRINTSIZE(Change);
	PRINTSIZE(ChangeSet);
	PRINTSIZE(Board);
	PRINTSIZE(Area);
	PRINTSIZE(Bible);
	PRINTSIZE(Automaton);
	PRINTSIZE(PlayerInfo);
	return 0;
}
