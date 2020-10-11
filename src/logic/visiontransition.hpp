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

#include "cell.hpp"
#include "player.hpp"
#include "vision.hpp"

class Bible;
class Board;
class ChangeSet;
class PlayerInfo;
struct TileToken;
struct UnitToken;


// VisionTransitionMapreduceImplementation has 3 realizations.
template <class This>
class VisionTMRI
{
public:
	VisionTMRI(const Bible& bible, Board& board,
			PlayerInfo& info,
			ChangeSet& changeset);

protected:
	const Bible& _bible;
	Board& _board;
	PlayerInfo& _info;
	ChangeSet& _changeset;
	std::vector<Vision> _results;

	void map(Cell index);
	void reduce(Cell index)
	{
		static_cast<This*>(this)->reduce(index);
	}

	void provider(Cell from, const TileToken& tile);
	void provider(Cell from, const UnitToken& unit);

public:
	void execute();
	void executeAround(Cell near, int range);
};

class VisionTransition : public VisionTMRI<VisionTransition>
{
public:
	using VisionTMRI::VisionTMRI;

private:
	friend VisionTMRI;

	void reduce(Cell index);
};

class InitialVisionTransition : public VisionTMRI<InitialVisionTransition>
{
public:
	using VisionTMRI::VisionTMRI;

private:
	friend VisionTMRI;

	void reduce(Cell index);
};

class RejoinVisionTransition : public VisionTMRI<RejoinVisionTransition>
{
public:
	using VisionTMRI::VisionTMRI;

private:
	friend VisionTMRI;

	void reduce(Cell index);
};
