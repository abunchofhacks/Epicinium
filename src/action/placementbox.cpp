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
#include "placementbox.hpp"
#include "source.hpp"

#include "surface.hpp"


Point PlacementBox::random()
{
	Point point(_topleft);
	int width  = _bottomright.xenon - _topleft.xenon;
	int height = _bottomright.yahoo - _topleft.yahoo;
	point.xenon += width / 2 - 1 + (rand() % 3);
	if (_entrenched)
	{
		point.yahoo += height / 2;
	}
	else if (_enfenced)
	{
		point.yahoo += height;
	}
	else if (_raised)
	{
		point.yahoo += 3 * height / 10 - 1 + (rand() % 3);
	}
	else if (_lowered)
	{
		point.yahoo += 7 * height / 10 - 1 + (rand() % 3);
	}
	else
	{
		point.yahoo +=     height /  2 - 1 + (rand() % 3);
	}
	return point;
}

void PlacementBox::centralize(int8_t pE, int8_t pS, int8_t pW, int8_t pN)
{
	// Find the center of the surface that this box is placed on.
	Point center = Surface::convertCenter(Surface::convert(_topleft));

	// If the left border is west of the center, we need to take percentW,
	// otherwise percentE.
	int8_t pH = (_topleft.xenon < center.xenon) ? pW : pE;
	int8_t pV = (_topleft.yahoo < center.yahoo) ? pN : pS;
	// Linear interpolation.
	_topleft.xenon = (_topleft.xenon * (100 - pH) + center.xenon * pH) / 100;
	_topleft.yahoo = (_topleft.yahoo * (100 - pV) + center.yahoo * pV) / 100;

	// Same for the other corner.
	pH = (_bottomright.xenon < center.xenon) ? pW : pE;
	pV = (_bottomright.yahoo < center.yahoo) ? pN : pS;
	_bottomright.xenon = (_bottomright.xenon * (100 - pH) + center.xenon * pH) / 100;
	_bottomright.yahoo = (_bottomright.yahoo * (100 - pV) + center.yahoo * pV) / 100;
}
