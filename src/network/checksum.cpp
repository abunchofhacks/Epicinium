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
#include "checksum.hpp"
#include "source.hpp"

#include "libs/openssl/sha.h"


Checksum::Checksum(uint8_t digest[]) :
	std::vector<uint8_t>(digest, digest + SHA512_DIGEST_LENGTH)
{}

Checksum Checksum::fromFile(const std::string& filename)
{
	size_t bufferlen = 64;
	char buffer[64];

	uint8_t digest[SHA512_DIGEST_LENGTH];

	SHA512_CTX ctx;
	SHA512_Init(&ctx);

	{
		std::ifstream file(filename, std::ios::binary);
		while (file)
		{
			file.read(buffer, bufferlen);
			if (!file) bufferlen = file.gcount();
			SHA512_Update(&ctx, buffer, bufferlen);
		}
	}

	SHA512_Final(digest, &ctx);

	return Checksum(digest);
}

Checksum Checksum::fromData(const std::string& data)
{
	uint8_t digest[SHA512_DIGEST_LENGTH];

	SHA512_CTX ctx;
	SHA512_Init(&ctx);

	SHA512_Update(&ctx, data.data(), data.size());

	SHA512_Final(digest, &ctx);

	return Checksum(digest);
}

std::string Checksum::toHexString()
{
	char buffer[SHA512_DIGEST_LENGTH * 2 + 1];
	for (size_t i = 0; i < SHA512_DIGEST_LENGTH; i++)
	{
		sprintf(buffer + i * 2, "%02x", (*this)[i]);
	}
	return std::string(buffer);
}
