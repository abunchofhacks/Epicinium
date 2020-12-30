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

#include "ailibrary.hpp"
#include "bible.hpp"
#include "board.hpp"
#include "cycle.hpp"
#include "order.hpp"

enum class Player : uint8_t;
enum class Difficulty : uint8_t;
struct Change;


class AICommander : public AILibrary
{
public:
	AICommander(const Player& player, const Difficulty& difficulty,
		const std::string& rulesetname, char character);
	AICommander(const AICommander&) = delete;
	AICommander(AICommander&&) = delete;
	AICommander& operator=(const AICommander&) = delete;
	AICommander& operator=(AICommander&&) = delete;
	virtual ~AICommander() = default;

protected:
	Bible _bible;
	const Player _player;
	const Difficulty _difficulty;
	const char _character;

	Board _board;
	int _money;
	int _year;
	Season _season;
	Daytime _daytime;
	Phase _phase;
	int _score;
	bool _gameover;
	bool _defeated;
	bool _finished;

	std::vector<Order> _unfinishedOrders;
	std::vector<Order> _newOrders;
	size_t _newOrdersConfirmed;

	friend class NewtBrain;
	friend class NeuralNewtBrain;

	Order hasUnfinished(const Descriptor& subject) const;
	Order hasNew(const Descriptor& subject) const;

public:
	const Bible& bible() const { return _bible; }

	// hiding AILibrary::receiveChanges()
	void receiveChanges(const std::vector<Change>& changes);
	void receiveChangesAsJson(const Json::Value& json);
	virtual void receiveChangesAsString(const std::string& changes) override;

	virtual void preprocess() {};
	virtual void process() = 0;
	virtual bool postprocess() { return true; }

	virtual void prepareOrders() override;

	// hiding AILibrary::orders()
	std::vector<Order> orders();
	virtual std::string ordersAsString() override;

	bool wantsToPrepareOrders() const;

	// hiding AILibrary::players() and ::difficulty()
	Player player() const { return _player; }
	Difficulty difficulty() const { return _difficulty; }

	virtual std::string playerAsString() const override;
	virtual std::string difficultyAsString() const override;

	virtual std::string characterstring() const override
	{
		return std::string(1, _character);
	}

	virtual std::string displayname() const override;
	// virtual std::string ainame() const = 0;
	// virtual std::string authors() const = 0;
	virtual std::string descriptivename() const override;
};

class AIDummy : public AICommander
{
	using AICommander::AICommander;

	virtual void process() override {};

	virtual std::string ainame() const override;
	virtual std::string authors() const override;
};
