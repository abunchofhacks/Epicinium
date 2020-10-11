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


namespace Base32
{
	inline char nickelToChar(uint8_t value)
	{
		static const char* alphabet = "0123456789" // a = 10
		                              "abcdefgh"   // skip i
		                              "jk"         // skip l
		                              "mn"         // skip o
		                              "pqrst"      // skip u
		                              "vwxyz"      ;
		// Based on Douglas Crockford's Base32 alphabet.
		return alphabet[value & 0x1f];
	}

	inline uint8_t charToNickel(char x)
	{
		uint8_t value = 0;
		if      (x >= '0' && x <= '9') value = (x - '0'     ); // a = 10
		else if (x >= 'a' && x <= 'h') value = (x - 'a' + 10); // skip i
		else if (x >= 'j' && x <= 'k') value = (x - 'j' + 18); // skip l
		else if (x >= 'm' && x <= 'n') value = (x - 'm' + 20); // skip o
		else if (x >= 'p' && x <= 't') value = (x - 'p' + 22); // skip u
		else if (x >= 'v' && x <= 'z') value = (x - 'v' + 27); // continue with capitals
		else if (x >= 'A' && x <= 'H') value = (x - 'A' + 10); // skip I
		else if (x >= 'J' && x <= 'K') value = (x - 'J' + 18); // skip L
		else if (x >= 'M' && x <= 'N') value = (x - 'M' + 20); // skip O
		else if (x >= 'P' && x <= 'T') value = (x - 'P' + 22); // skip U
		else if (x >= 'V' && x <= 'Z') value = (x - 'V' + 27); // continue with skipped
		else if (x == 'i' || x == 'I') value = 1;  // i and I are confused with 1
		else if (x == 'l' || x == 'L') value = 1;  // l and L are confused with 1
		else if (x == 'o' || x == 'O') value = 0;  // o and O are confused with 0
		else if (x == 'u' || x == 'U') value = 27; // u and U are confused with v
		return value;
	}

	// Convert a big-endian base256 number to a big-endian base32 representation.
	inline std::string encode(const std::vector<uint8_t>& data)
	{
		// Calculate the length of the resulting word, rounding up.
		// E.g. a single byte takes 2 * 5 bits, five byte take exactly 8 * 5 bits.
		int datalength = data.size();
		int wordlength = (datalength * 8 + 4) / 5;
		std::string word;
		word.resize(wordlength);

		// If necessary, we can prepend 0 bits to the front of the big-endian number.
		// E.g. if we want to encode a single uint8_t, we use 2 * 5 = 10 >= 8 = 1 * 8 bits,
		// so we prepend 2 zeroes to the big-endian number.
		// If we want to encode five uint8_t, we use 8 * 5 = 40 >= 40 = 5 * 8 bits,
		// so we do not need to prepend anything.
		int nbits = (5 - (datalength * 8) % 5) % 5;
		uint16_t buffer = 0;

		// Create the word from left to right.
		int datapos = 0;
		for (int wordpos = 0; wordpos < wordlength; wordpos++)
		{
			// Do we need to add fresh bits?
			if (nbits < 5)
			{
				// Move the bits to bitpositions nbits, ..., nbits + 7.
				buffer |= data[datapos++] << (8 - nbits);
				nbits += 8;
			}

			// Consume the five left-most bits.
			word[wordpos] = nickelToChar(buffer >> 11);
			buffer <<= 5;
			nbits -= 5;
		}

		return word;
	}

	// Convert a big-endian base32 representation back into a big-endian base256 number.
	// A byte array of size S=5N+K is encoded as a word of length l(S)=8N+f(K),
	// where f(0) = 0, f(1) = 2, f(2) = 4, f(3) = 5 and f(4) = 7.
	// Note that l() is surjective, so we can determine the size S of a byte array given l(S).
	inline std::vector<uint8_t> decode(const std::string& word)
	{
		// Because decode is the inverse of encode, we want to determine how long the original
		// data array was, and we will drop the first few bits of this word; we round down.
		int wordlength = word.length();
		int datalength = (wordlength * 5) / 8;
		std::vector<uint8_t> data;
		data.resize(datalength);

		// If necessary, we can drop bits from the front of the representation.
		// E.g. if we had encoded a single uint8_t, we are now decoding two characters,
		// which is 10 bits, but we can freely ignore the first two bits.
		int nbits = - ((wordlength * 5) % 8);
		uint16_t buffer = 0;

		// Decode the word one character at a time.
		int datapos = 0;
		for (int wordpos = 0; wordpos < wordlength; wordpos++)
		{
			char x = word[wordpos];
			uint8_t value = charToNickel(x);

			// Add the fresh bits.
			buffer |= value << (11 - nbits);
			nbits += 5;

			// Can we consume eight bits?
			if (nbits >= 8)
			{
				// Consume the eight left-most bits.
				data[datapos++] = buffer >> 8;
				buffer <<= 8;
				nbits -= 8;
			}
		}

		return data;
	}

	// Convert from big-endian numbers.
	inline std::string encode64bits(uint64_t data)
	{
		return encode({(uint8_t) (data >> 56),
			(uint8_t) (data >> 48),
			(uint8_t) (data >> 40),
			(uint8_t) (data >> 32),
			(uint8_t) (data >> 24),
			(uint8_t) (data >> 16),
			(uint8_t) (data >> 8),
			(uint8_t) data});
	}

	inline std::string encode32bits(uint32_t data)
	{
		return encode({(uint8_t) (data >> 24),
			(uint8_t) (data >> 16),
			(uint8_t) (data >> 8),
			(uint8_t) data});
	}

	// Convert back to big-endian numbers.
	inline uint64_t decode64bits(const std::string& word)
	{
		auto data = decode(word);
		uint64_t result = 0;
		for (uint8_t byte : data)
		{
			result <<= 8;
			result |= byte;
		}
		return result;
	}

	inline uint32_t decode32bits(const std::string& word)
	{
		auto data = decode(word);
		uint32_t result = 0;
		for (uint8_t byte : data)
		{
			result <<= 8;
			result |= byte;
		}
		return result;
	}
}
