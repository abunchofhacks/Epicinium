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
#include "coredump.hpp"
#include "source.hpp"

#if COREDUMP_ENABLED
#include <sys/resource.h>
#endif


void CoreDump::enable()
{
#if COREDUMP_ENABLED
	{
		struct rlimit core_limit;
		core_limit.rlim_cur = RLIM_INFINITY;
		core_limit.rlim_max = RLIM_INFINITY;

		if (setrlimit(RLIMIT_CORE, &core_limit) < 0)
		{
			// Plog has not yet been installed at this point.
			fprintf(stderr, "setrlimit: %s\n"
				"Warning: core dumps may be truncated or non-existant\n",
				strerror(errno));
		}
	}
#endif
}
