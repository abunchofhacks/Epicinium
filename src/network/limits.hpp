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

/* This file defines some sanity limits for networking. */
constexpr size_t SEND_FILE_SIZE_LIMIT = 2147483647;
constexpr size_t SEND_FILE_SIZE_WARNING_LIMIT = 134217728;
constexpr size_t MESSAGE_SIZE_LIMIT = 524288;
constexpr size_t MESSAGE_SIZE_UNVERSIONED_LIMIT = 201;
constexpr size_t MESSAGE_SIZE_WARNING_LIMIT = 65537;
constexpr size_t SEND_FILE_CHUNK_SIZE = 65536;
constexpr size_t SEND_VIRTUAL_SIZE_LIMIT = 65536;
constexpr size_t SEND_VIRTUAL_SIZE_WARNING_LIMIT = 8193;
