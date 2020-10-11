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
#include "passwordinput.hpp"
#include "source.hpp"

#include "text.hpp"


PasswordInput::PasswordInput(int fontsize) :
	TextInput(fontsize, -1)
{
	_copyable = false;
}

void PasswordInput::prerenderText()
{
	constexpr char OBFUSCHAR = '*';

	// Display a byte-for-byte obfuscated replacement of _text.
	// This is fine w.r.t. UTF8 because we do not want unicode in passwords.
	std::string faketext = std::string(_text.size(), OBFUSCHAR);

	TextInput::prerenderText(faketext);
}

void PasswordInput::filterUserInput(std::string& input)
{
	// Replace whitespace characters with spaces, as in TextInput.
	std::replace(input.begin(), input.end(), '\t', ' ');
	std::replace(input.begin(), input.end(), '\r', ' ');
	std::replace(input.begin(), input.end(), '\n', ' ');

	// Replace all other control bytes (0 through 31), as well as DEL (127)
	// and all upper half bytes (128 through 255, might be ISO or UTF8),
	// with the ASCII substitution control character SUB.
	// This is because having those characters in a password is not widely
	// supported and entering UTF8 on different keyboards or in different OSes
	// is not entirely deterministic, so doing it blind is crazy.
	// This character will then be rejected by validatePassword().
	constexpr char SUBCHAR = 0x1A;
	std::replace_if(input.begin(), input.end(), [](char c){
		uint8_t x = (uint8_t) c;
		return (c <= 31 || x >= 127);
	}, SUBCHAR);
}
