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
#include "challenge.hpp"
#include "source.hpp"

#include "player.hpp"
#include "notice.hpp"
#include "cycle.hpp"
#include "bible.hpp"
#include "board.hpp"
#include "playerinfo.hpp"
#include "roundinfo.hpp"


static int tileCount(const Board& board, const TileType& tiletype)
{
	int count = 0;
	for (Cell index : board)
	{
		if (board.tile(index).type == tiletype)
		{
			count++;
		}
	}
	return count;
}

static int tileCount(const Board& board, const Player& owner)
{
	int count = 0;
	for (Cell index : board)
	{
		if (board.tile(index).owner == owner)
		{
			count++;
		}
	}
	return count;
}

Notice Challenge::check(const Bible& bible, const Board& board,
	const RoundInfo& rinfo, PlayerInfo& pinfo,
	std::vector<Player>& defeats)
{
	return Challenge::check(_id, bible, board, rinfo, pinfo, defeats);
}

Notice Challenge::check(const Id& id, const Bible&, const Board& board,
	const RoundInfo& rinfo, PlayerInfo& pinfo,
	std::vector<Player>& defeats)
{
	switch (id)
	{
		case SHOWCASE:
		case ELIMINATION:
		case EVERYTHINGISFREE:
		case TRAMPLE:
		case TRAMPLE2:
		case HIGHSPEED:
		case MORALE:
		{
			return Notice::NONE;
		}
		break;

		case INVESTMENT:
		{
			if (rinfo._year >= 3 && rinfo._phase == Phase::GROWTH)
			{
				for (const Player& player : pinfo._players)
				{
					if (pinfo._defeated[player]) continue;

					if (tileCount(board, player) == 0)
					{
						defeats.emplace_back(player);
					}
				}

				return Notice::ROUNDLIMIT;
			}
		}
		break;
	}

	return Notice::NONE;
}

void Challenge::score(const Bible& bible, const Board& board,
	PlayerInfo& info)
{
	return Challenge::score(_id, bible, board, info);
}

void Challenge::score(const Id& id, const Bible&, const Board&,
	PlayerInfo& info)
{
	switch (id)
	{
		case SHOWCASE:
		case ELIMINATION:
		case EVERYTHINGISFREE:
		case TRAMPLE:
		case TRAMPLE2:
		case HIGHSPEED:
		case MORALE:
		break;

		case INVESTMENT:
		{
			for (const Player& player : info._players)
			{
				if (!info._defeated[player])
				{
					info._score[player] = info._money[player];
				}
			}
		}
		break;
	}
}

void Challenge::award(const Bible& bible, const Board& board,
	PlayerInfo& info)
{
	return Challenge::award(_id, bible, board, info);
}

void Challenge::award(const Id& id, const Bible& bible, const Board& board,
	PlayerInfo& info)
{
	switch (id)
	{
		case SHOWCASE:
		{
			for (const Player& player : info._players)
			{
				int score = info._score[player];
				if (score >= 40)
				{
					info._award[player] = 3;
				}
				else if (score >= 25)
				{
					info._award[player] = 2;
				}
				else if (score >= 1)
				{
					info._award[player] = 1;
				}
				else
				{
					info._award[player] = 0;
				}
			}
		}
		break;
		case ELIMINATION:
		{
			for (const Player& player : info._players)
			{
				int score = info._score[player];
				if (info._defeated[player])
				{
					info._award[player] = 0;
				}
				else if (score >= 25)
				{
					if (tileCount(board, bible.tiletype("rubble")) == 0)
					{
						info._award[player] = 3;
					}
					else
					{
						info._award[player] = 2;
					}
				}
				else
				{
					info._award[player] = 1;
				}
			}
		}
		break;
		case EVERYTHINGISFREE:
		{
			for (const Player& player : info._players)
			{
				int score = info._score[player];
				if (score >= 80)
				{
					info._award[player] = 3;
				}
				else if (score >= 30)
				{
					info._award[player] = 2;
				}
				else if (score >= 1)
				{
					info._award[player] = 1;
				}
				else
				{
					info._award[player] = 0;
				}
			}
		}
		break;
		case TRAMPLE:
		case TRAMPLE2:
		{
			for (const Player& player : info._players)
			{
				int score = info._score[player];
				if (info._defeated[player])
				{
					info._award[player] = 0;
				}
				else if (score >= 10)
				{
					info._award[player] = 3;
				}
				else if (score >= 4)
				{
					info._award[player] = 2;
				}
				else
				{
					info._award[player] = 1;
				}
			}
		}
		break;
		case HIGHSPEED:
		{
			for (const Player& player : info._players)
			{
				int score = info._score[player];
				if (score >= 80)
				{
					info._award[player] = 3;
				}
				else if (score >= 30)
				{
					info._award[player] = 2;
				}
				else if (score >= 1)
				{
					info._award[player] = 1;
				}
				else
				{
					info._award[player] = 0;
				}
			}
		}
		break;
		case INVESTMENT:
		{
			for (const Player& player : info._players)
			{
				int money = info._money[player];
				if (money >= 132)
				{
					info._award[player] = 3;
				}
				else if (money >= 86)
				{
					info._award[player] = 2;
				}
				else if (money >= 21)
				{
					info._award[player] = 1;
				}
				else
				{
					info._award[player] = 0;
				}
			}
		}
		break;
		case MORALE:
		{
			for (const Player& player : info._players)
			{
				int score = info._score[player];
				if (info._defeated[player])
				{
					info._award[player] = 0;
				}
				else if (score >= 8)
				{
					info._award[player] = 3;
				}
				else if (score >= 5)
				{
					info._award[player] = 2;
				}
				else
				{
					info._award[player] = 1;
				}
			}
		}
		break;
	}
}
