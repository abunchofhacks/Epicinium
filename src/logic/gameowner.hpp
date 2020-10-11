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

#include "implodingptr.hpp"

class Game;
class Challenge;
enum class Player : uint8_t;
enum class Role : uint8_t;


class GameOwner
{
protected:
	GameOwner() = default;

public:
	GameOwner(const GameOwner& /**/) = delete;
	GameOwner(GameOwner&& /**/) = delete;
	GameOwner& operator=(const GameOwner& /**/) = delete;
	GameOwner& operator=(GameOwner&& /**/) = delete;
	virtual ~GameOwner() = default;

	virtual std::weak_ptr<Game> startGame(imploding_ptr<Game> game) = 0;
	virtual std::weak_ptr<Game> startChallenge(const Challenge& challenge) = 0;
	virtual std::weak_ptr<Game> startGame(
		const Player& player, const std::string& rulesetname,
		uint32_t planningTime) = 0;
	virtual std::weak_ptr<Game> startTutorial(
		const Player& player, const std::string& rulesetname,
		uint32_t planningTime) = 0;
	virtual std::weak_ptr<Game> startReplay(
		const Role& role, const std::string& rulesetname,
		uint32_t planningTime) = 0;
	virtual std::weak_ptr<Game> startDiorama() = 0;

	virtual void stopGame() = 0;
};
