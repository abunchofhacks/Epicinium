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
#include "cell.hpp"
#include "source.hpp"


std::ostream& operator<<(std::ostream& os, Cell index)
{
	return os << "[" << index.ix() << " 0x" << std::hex
		<< std::setfill('0') << std::setw(2) << int(index._rows)
		<< std::setfill('0') << std::setw(2) << int(index._cols)
		<< std::setfill('0') << std::setw(4) << int(index._ix)
		<< std::dec << "]";
}
