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
#include "clock.hpp"
#include "source.hpp"

#include <chrono>


uint64_t EpochClock::seconds()
{
	// For some uses we care about time relative to earlier runs of the program,
	// so we cannot use steady_clock and use system_clock instead.
	auto currentTimestampMs =
		std::chrono::duration_cast<std::chrono::seconds>(
			std::chrono::system_clock::now().time_since_epoch());
	return currentTimestampMs.count();
}

uint64_t EpochClock::milliseconds()
{
	// For some uses we care about time relative to earlier runs of the program,
	// so we cannot use steady_clock and use system_clock instead.
	auto currentTimestampMs =
		std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::system_clock::now().time_since_epoch());
	return currentTimestampMs.count();
}

uint64_t SteadyClock::milliseconds()
{
	// We use steady_clock instead of system_clock, because we only care about
	// time relative to the start of the program, not about timezones or dates.
	auto currentTimestampMs =
		std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::steady_clock::now().time_since_epoch());
	return currentTimestampMs.count();
}
