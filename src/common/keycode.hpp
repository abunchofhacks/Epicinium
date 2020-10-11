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


// Create a lossy keycode by interleaving a 16 bit key and a 44 bit data element
// into a 60 bit number that is then encoded as a big-endian base32-word of
// length 12 (since 32^12 = 2^60).
inline std::string keycode(uint16_t key, /*uint44_t=*/uint64_t data)
{
	static const char* alphabet = "0123456789" // a = 10
	                              "abcdefgh"   // skip i
	                              "jk"         // skip l
	                              "mn"         // skip o
	                              "pqrst"      // skip u
	                              "vwxyz"      ;

	static const int8_t bites[12] = {5, 4, 3, 2, 1, 1, 0, 0, 0, 0, 0, 0};
	char word[13];

	for (int i = 11; i >= 0; i--)
	{
		uint8_t index = ((key & ((1 << bites[i]) - 1)) << (5 - bites[i]))
			| (data & ((1 << (5 - bites[i])) - 1));
		key >>= bites[i];
		data >>= 5 - bites[i];

		word[i] = alphabet[index];
	}

	word[12] = '\0';

	return std::string(word);
}
