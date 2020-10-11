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


enum class ResponseStatus : uint8_t
{
	SUCCESS            =  0,
	CREDS_INVALID      =  1,
	ACCOUNT_LOCKED     =  2,
	USERNAME_TAKEN     =  3,
	EMAIL_TAKEN        =  4,
	ACCOUNT_DISABLED   =  5,
	KEY_TAKEN          =  6, // only used for key activation (for now)
	IP_BLOCKED         =  7, // only used for key activation (for now)
	KEY_REQUIRED       =  8,
	EMAIL_UNVERIFIED   =  9,
	USERNAME_REQUIRED_NOUSER = 10,
	USERNAME_REQUIRED_INVALID = 11,
	USERNAME_REQUIRED_TAKEN = 12,

	DATABASE_ERROR     = 94,
	METHOD_INVALID     = 95,
	REQUEST_MALFORMED  = 96,
	RESPONSE_MALFORMED = 97,
	CONNECTION_FAILED  = 98,
	UNKNOWN_ERROR      = 99,
};

inline ResponseStatus fix(const ResponseStatus& status)
{
	switch (status)
	{
		case ResponseStatus::SUCCESS:
		case ResponseStatus::CREDS_INVALID:
		case ResponseStatus::ACCOUNT_LOCKED:
		case ResponseStatus::USERNAME_TAKEN:
		case ResponseStatus::EMAIL_TAKEN:
		case ResponseStatus::ACCOUNT_DISABLED:
		case ResponseStatus::KEY_TAKEN:
		case ResponseStatus::IP_BLOCKED:
		case ResponseStatus::KEY_REQUIRED:
		case ResponseStatus::EMAIL_UNVERIFIED:
		case ResponseStatus::USERNAME_REQUIRED_NOUSER:
		case ResponseStatus::USERNAME_REQUIRED_INVALID:
		case ResponseStatus::USERNAME_REQUIRED_TAKEN:
		case ResponseStatus::DATABASE_ERROR:
		case ResponseStatus::METHOD_INVALID:
		case ResponseStatus::REQUEST_MALFORMED:
		case ResponseStatus::RESPONSE_MALFORMED:
		case ResponseStatus::CONNECTION_FAILED:
		case ResponseStatus::UNKNOWN_ERROR:
		return status;
	}
	return ResponseStatus::UNKNOWN_ERROR;
}
