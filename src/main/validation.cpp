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
#include "validation.hpp"
#include "source.hpp"

#include "base32.hpp"


bool isValidUserContentName(const std::string& name)
{
	if (name.size() < 3)
	{
		return false;
	}
	if (name.size() > 100)
	{
		return false;
	}
	size_t ill = name.find_first_not_of(
		" !"
		// not "#$%&
		"'()"
		// not *+
		",-."
		// not /
		"0123456789"
		// not :;<=>
		"?"
		// not @
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		// not [\]^
		"_"
		// not `
		"abcdefghijklmnopqrstuvwxyz"
		// not {|}
		"~"
		"");
	if (ill != std::string::npos)
	{
		return false;
	}
	return true;
}

bool isValidUsername(const std::string& username)
{
	if (username.size() < 3)
	{
		return false;
	}
	if (username.size() > 36)
	{
		return false;
	}
	size_t ill = username.find_first_not_of(
		// not space
		// not !"#$%&'()*+,
		"-."
		// not /
		"0123456789"
		// not :;<=>?@
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		// not [\]^
		"_"
		// not `
		"abcdefghijklmnopqrstuvwxyz"
		// not {|}
		"~"
		"");
	if (ill != std::string::npos)
	{
		return false;
	}
	return true;
}

bool isValidEmail(const std::string& email)
{
	size_t at = email.find_last_of('@');
	if (at == std::string::npos)
	{
		return false;
	}
	size_t dot = email.find_first_of('.', at);
	if (dot == std::string::npos)
	{
		return false;
	}
	return true;
}

bool isValidPassword(const std::string& password)
{
	if (password.size() < 8)
	{
		return false;
	}
	// We have few restrictions on passwords because it is blind anyway.
	// However we do not allow control characters, DEL or upper-half ASCII.
	// This is because having those characters in a password is not widely
	// supported and entering UTF8 on different keyboards or in different OSes
	// is not entirely deterministic, so doing it blind is crazy.
	auto inv = std::find_if(password.begin(), password.end(), [](char c){
		uint8_t x = (uint8_t) c;
		return (x <= 31 || x >= 127);
	});
	if (inv != password.end())
	{
		return false;
	}
	// We do require one letter (a-zA-Z) and one non-letter (0-9 or other).
	size_t let = password.find_first_of(
		"abcdefghijklmnopqrstuvwxyz"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"");
	if (let == std::string::npos)
	{
		return false;
	}
	size_t non = password.find_first_not_of(
		"abcdefghijklmnopqrstuvwxyz"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"");
	if (non == std::string::npos)
	{
		return false;
	}
	return true;
}

bool isValidPasswordResetToken(const std::string& token)
{
	if (token.size() != 6)
	{
		return false;
	}
	size_t ill = token.find_first_not_of(
		"23456789"
		"ABCDEFGHJKMNPQRSTVWXYZ"
		"abcdefghjkmnpqrstvwxyz"
		"");
	if (ill != std::string::npos)
	{
		return false;
	}
	return true;
}

bool isValidKeyToken(const std::string& token)
{
	// Key tokens consist of a number of blocks separated by dashes; each block
	// is an uppercase Crockford's Base32 encoded string of length five.
	// The token should be valid after being canonized by the method below.
	if (token.size() != 29)
	{
		return false;
	}
	size_t ill = token.find_first_not_of(
		"0123456789"
		"ABCDEFGHJKMNPQRSTVWXYZ"
		"-"
		"");
	if (ill != std::string::npos)
	{
		return false;
	}
	return true;
}

std::string canonizeKeyToken(const std::string& token)
{
	// With Crockford's Base32, all of 0-9, A-Z, a-z have a valid meaning.
	static const char* decodables = ""
		"0123456789"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz"
		"";
	static const char* valids = ""
		" "
		"0123456789"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz"
		"-"
		"";

	// Trim away dashes and spaces. Skip but keep invalid characters.
	// Canonize each of the Base32 characters and intersperse some dashes.
	// Contrary to how we use Base32 elsewhere, key tokens are uppercase.
	std::stringstream strm;
	size_t start = 0;
	size_t pre = 0;
	size_t end = 0;
	size_t count = 0;
	while (end != std::string::npos)
	{
		pre = token.find_first_of(valids, end);
		strm << token.substr(end, pre - end);
		if (pre == std::string::npos)
		{
			break;
		}

		start = token.find_first_of(decodables, pre);
		if (start == std::string::npos)
		{
			strm << token.substr(pre);
			break;
		}

		end = token.find_first_not_of(decodables, start);
		for (char x : token.substr(start, end - start))
		{
			if (count > 0 && (count % 5) == 0)
			{
				strm << '-';
			}
			uint8_t nickel = Base32::charToNickel(x);
			char y = std::toupper(Base32::nickelToChar(nickel));
			strm << y;
			count++;
		}
	}

	return strm.str();
}
