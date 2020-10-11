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

#include "version.hpp"
#include "loginstaller.hpp"
#include "bible.hpp"
#include "library.hpp"
#include "writer.hpp"


int main(int /**/, char* /**/[])
{
	std::cout << "[ Epicinium Bible Saver ]";
	std::cout << " (v" << Version::current() << ")";
	std::cout << std::endl << std::endl;

	Writer writer;
	writer.install();

	LogInstaller("biblesaver", 5).install();

	LOGI << "Start v" << Version::current();

	Library library;
	library.loadAndUpdateIndex();

	{
		// Make sure the current bible can be saved and loaded correctly.
		Bible currentbible = library.get(library.currentRuleset());
		LOGI << "Checking " << currentbible.toJson();
		assert(currentbible == currentbible.toJson());

		// Get the name of the previous bible in the library.
		std::string previous = library.previousRuleset();

		// It should exist.
		LOGI << "Checking existence of " << previous;
		assert(library.exists(previous));

		// Make sure this previous version is also saved and loaded correctly.
		Bible olderbible = library.get(previous);
		LOGI << "Checking " << olderbible.toJson();
		assert(olderbible == olderbible.toJson());

		if (Version::current().isWellDefined())
		{
			// Make sure the "correctly"-checker is working correctly.
			LOGI << "Checking " << olderbible.name()
				<< " != " << currentbible.name();
			assert(olderbible != currentbible);
		}
	}

	LOGI << "OK";

	std::cout << std::endl << "[ Done ]" << std::endl;
	return 0;
}
