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

#include <chrono>
#include <thread>

#include "base32.hpp"
#include "keycode.hpp"


int main(int /**/, char* /**/[])
{
	auto timestamp = std::chrono::system_clock::now().time_since_epoch();
	auto timestampMs = std::chrono::duration_cast<std::chrono::milliseconds>(
		timestamp).count();
	uint64_t ms = timestampMs;

	for (int i = 0; i < 20; i++)
	{
		uint64_t timebits = ms & 0x00000FFFFFFFFFFFull;
		uint16_t genbits = 1 & 0x001F;
		uint16_t seqbits = 1 & 0x003F;
		uint64_t snowflake = (timebits << 19) | (genbits << 10) | seqbits;
		std::string word = Base32::encode64bits(snowflake);
		std::string dashed = word.substr(0, 4) + "-" + word.substr(4, 4)
			+ "-" + word.substr(8);
		std::string rearranged = word.substr(5, 4) + "." + word.substr(0, 5)
			+ "." + word.substr(9);
		std::string prefixed = word.substr(7, 2) + word.substr(5, 2)
			+ ":" + word;
		uint16_t randbits = rand() & 0xFFFF;
		std::string keycode = ::keycode(randbits, timebits);
		std::string dashedkey = keycode.substr(0, 4)
			+ "-" + keycode.substr(4, 4)
			+ "-" + keycode.substr(8);
		uint64_t ranflake = (timebits << 20) | (rand() & 0xFFFFF);
		std::string ranword = Base32::encode64bits(ranflake);
		std::string randashed = ranword.substr(0, 5)
			+ "-" + ranword.substr(5, 4)
			+ "-" + ranword.substr(9);
		std::string ranged = ranword.substr(9, 4) + "-" + ranword.substr(0, 9);

		std::cout << std::setw(5) << (ms - timestampMs)
			<< " => " << snowflake
			<< " or 0x" << std::hex << snowflake << std::dec
			<< " or " << word
			<< " or " << dashed
			<< " or " << rearranged
			<< " or " << prefixed
			<< " or " << keycode
			<< " or " << ranflake
			<< " or " << std::hex << (ranflake & 0xFFFFFFFF) << "-"
				<< (ranflake >> 32) << std::dec
			<< " or " << randashed
			<< " or " << ranged
			<< std::endl;

		ms += 1 + i * i * i;
	}

	return 0;
}
