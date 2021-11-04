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



const std::vector<Challenge::Id>& Challenge::pool()
{
	static std::vector<Challenge::Id> pool = {
		Challenge::Id::SHOWCASE,
		Challenge::Id::ELIMINATION,
		Challenge::Id::TRAMPLE,
		Challenge::Id::MORALE,
		Challenge::Id::EVERYTHINGISFREE,
		Challenge::Id::HIGHSPEED,
		Challenge::Id::INVESTMENT,
	};
	return pool;
}

const std::vector<Challenge::Id>& Challenge::campaign()
{
	static std::vector<Challenge::Id> pool = {
		Challenge::Id::ACT1A, Challenge::Id::ACT1B, Challenge::Id::ACT1C,
		Challenge::Id::ACT1D, Challenge::Id::ACT1E, Challenge::Id::ACT1F,
		Challenge::Id::ACT2A, Challenge::Id::ACT2B, Challenge::Id::ACT2C,
		Challenge::Id::ACT2D, Challenge::Id::ACT2E,
		Challenge::Id::ACT3A, Challenge::Id::ACT3B, Challenge::Id::ACT3C,
		Challenge::Id::ACT3D, Challenge::Id::ACT3E, Challenge::Id::ACT3F,
		Challenge::Id::ACT3G,
	};
	return pool;
}

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

static int tileCount(const Board& board,
	const TileType& tiletype, const Player& owner)
{
	int count = 0;
	for (Cell index : board)
	{
		if (board.tile(index).type == tiletype
			&& board.tile(index).owner == owner)
		{
			count++;
		}
	}
	return count;
}

static int unitCount(const Board& board,
	const UnitType& unittype, const Player& owner)
{
	int count = 0;
	for (Cell index : board)
	{
		if (board.ground(index).type == unittype
			&& board.ground(index).owner == owner)
		{
			count++;
		}

		if (board.air(index).type == unittype
			&& board.air(index).owner == owner)
		{
			count++;
		}
	}
	return count;
}

static int unitReachesTile(const Board& board,
	const UnitType& unittype, const TileType& tiletype, const Player& owner)
{
	int count = 0;
	for (Cell index : board)
	{
		if (board.ground(index).type == unittype
			&& board.ground(index).owner == owner
			&& board.tile(index).type == tiletype
			&& board.tile(index).owner == owner)
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

Notice Challenge::check(const Id& id, const Bible& bible, const Board& board,
	const RoundInfo& rinfo, PlayerInfo& pinfo,
	std::vector<Player>& defeats)
{
	switch (id)
	{
		case CUSTOM:
		case SHOWCASE:
		case ELIMINATION:
		case EVERYTHINGISFREE:
		case TRAMPLE:
		case HIGHSPEED:
		case MORALE:
		case ACT1A:
		case ACT1B:
		case ACT1C:
		case ACT1D:
		case ACT1E:
		case ACT1F:
		case ACT2A:
		case ACT2B:
		case ACT2C:
		case ACT2D:
		case ACT2E:
		case ACT3C:
		case ACT3E:
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

		case ACT3A:
		{
			TileType citytype = bible.tiletype("city");
			if (citytype == TileType::NONE)
			{
				LOGE << "Missing type 'city'";
				return Notice::NONE;
			}

			// Capture (not destroy) neutral city. Lose if blue captures it or
			// your city. Also capture the enemy city. Rephrased: you win when
			// you control all 3 cities, and you lose if blue controls 2 cities
			// or if any city is destroyed.(Settlers cannot build cities so
			// that's fine.)
			if (tileCount(board, citytype, Player::RED) >= 3)
			{
				for (const Player& player : pinfo._players)
				{
					if (player != Player::BLUE) continue;
					if (pinfo._defeated[player]) continue;

					defeats.emplace_back(player);
				}
			}
			else if (tileCount(board, citytype) < 3
				|| tileCount(board, citytype, Player::BLUE) >= 2)
			{
				for (const Player& player : pinfo._players)
				{
					if (player != Player::RED) continue;
					if (pinfo._defeated[player]) continue;

					defeats.emplace_back(player);
				}
			}
			return Notice::NONE;
		}
		break;
		case ACT3B:
		{
			TileType towntype = bible.tiletype("town");
			if (towntype == TileType::NONE)
			{
				LOGE << "Missing type 'town'";
				return Notice::NONE;
			}

			// Capture 3 towns within 4 years. If not, lose.
			if (rinfo._year > 4 && rinfo._phase == Phase::GROWTH)
			{
				for (const Player& player : pinfo._players)
				{
					if (player != Player::RED) continue;
					if (pinfo._defeated[player]) continue;

					defeats.emplace_back(player);
				}

				return Notice::ROUNDLIMIT;
			}

			if (tileCount(board, towntype, Player::RED) >= 3)
			{
				for (const Player& player : pinfo._players)
				{
					if (player != Player::BLUE) continue;
					if (pinfo._defeated[player]) continue;

					defeats.emplace_back(player);
				}
			}
			else if (tileCount(board, towntype) < 3)
			{
				for (const Player& player : pinfo._players)
				{
					if (player != Player::RED) continue;
					if (pinfo._defeated[player]) continue;

					defeats.emplace_back(player);
				}
			}

			return Notice::NONE;
		}
		break;
		case ACT3D:
		{
			// Occupy southern airport (only possible by capturing
			// northern airport and sending zeppelin down).
			// This is achieved by simply making the airfields binding.
			return Notice::NONE;
		}
		break;
		case ACT3F:
		{
			UnitType gunnertype = bible.unittype("gunner");
			if (gunnertype == UnitType::NONE)
			{
				LOGE << "Missing type 'gunner'";
				return Notice::NONE;
			}
			TileType citytype = bible.tiletype("city");
			if (citytype == TileType::NONE)
			{
				LOGE << "Missing type 'city'";
				return Notice::NONE;
			}

			// Kill enemy gunners.
			if (unitCount(board, gunnertype, Player::BLUE) == 0)
			{
				for (const Player& player : pinfo._players)
				{
					if (player != Player::BLUE) continue;
					if (pinfo._defeated[player]) continue;

					defeats.emplace_back(player);
				}
			}

			// If gunners reach blue city, you lose.
			if (unitReachesTile(board, gunnertype, citytype, Player::BLUE))
			{
				for (const Player& player : pinfo._players)
				{
					if (player != Player::RED) continue;
					if (pinfo._defeated[player]) continue;

					defeats.emplace_back(player);
				}

				return Notice::NONE;
			}
		}
		break;
		case ACT3G:
		{
			UnitType gunnertype = bible.unittype("gunner");
			if (gunnertype == UnitType::NONE)
			{
				LOGE << "Missing type 'gunner'";
				return Notice::NONE;
			}

			// Kill enemy gunners.
			if (unitCount(board, gunnertype, Player::BLUE) == 0)
			{
				for (const Player& player : pinfo._players)
				{
					if (player != Player::BLUE) continue;
					if (pinfo._defeated[player]) continue;

					defeats.emplace_back(player);
				}
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
		case CUSTOM:
		case SHOWCASE:
		case ELIMINATION:
		case EVERYTHINGISFREE:
		case TRAMPLE:
		case HIGHSPEED:
		case MORALE:
		case ACT1A:
		case ACT1B:
		case ACT1C:
		case ACT1D:
		case ACT1E:
		case ACT1F:
		case ACT2A:
		case ACT2B:
		case ACT2C:
		case ACT2D:
		case ACT2E:
		case ACT3A:
		case ACT3B:
		case ACT3C:
		case ACT3D:
		case ACT3E:
		case ACT3F:
		case ACT3G:
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
		case CUSTOM:
		case ACT1A:
		case ACT1B:
		case ACT1C:
		case ACT1D:
		case ACT2A:
		case ACT2B:
		case ACT2C:
		case ACT2D:
		case ACT2E:
		case ACT3A:
		case ACT3B:
		case ACT3C:
		case ACT3D:
		case ACT3E:
		case ACT3F:
		case ACT3G:
		{
			for (const Player& player : info._players)
			{
				if (info._defeated[player])
				{
					info._award[player] = 0;
				}
				else
				{
					info._award[player] = 1;
				}
			}
		}
		break;
		case ACT1E:
		{
			for (const Player& player : info._players)
			{
				int score = info._score[player];
				if (score >= 75)
				{
					info._award[player] = 2;
				}
				else if (info._defeated[player])
				{
					info._award[player] = 0;
				}
				else
				{
					info._award[player] = 1;
				}
			}
		}
		break;
		case ACT1F:
		{
			for (const Player& player : info._players)
			{
				int score = info._score[player];
				if (score >= 100)
				{
					info._award[player] = 3;
				}
				else if (score >= 80)
				{
					info._award[player] = 2;
				}
				else if (info._defeated[player])
				{
					info._award[player] = 0;
				}
				else
				{
					info._award[player] = 1;
				}
			}
		}
		break;
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
