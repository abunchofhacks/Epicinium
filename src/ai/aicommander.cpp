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
#include "aicommander.hpp"
#include "source.hpp"

#include "library.hpp"
#include "difficulty.hpp"
#include "change.hpp"
#include "notice.hpp"


const std::vector<std::string> NAMEPOOL = {"Alice", "Bob", "Carol", "Dave", "Emma", "Frank",
	"Gwen", "Harold", "Iris", "Justin", "Kate", "Leopold", "Mary", "Nick", "Olivia", "Peter",
	"Quintin", "Rachel", "Sasha", "Timothy", "Ursula", "Victor", "Wendy", "Xerxes", "Yara",
	"Zach"};

AICommander::AICommander(const Player& player, const Difficulty& difficulty,
		const std::string& rulesetname, char x) :
	_bible(Library::getBible(rulesetname)),
	_player(player),
	_difficulty(difficulty),
	_character(std::max('A', std::min(x, 'Z'))),
	_board(_bible),
	_money(0),
	_year(0),
	_season(Season::SPRING),
	_daytime(Daytime::LATE),
	_phase(Phase::GROWTH),
	_score(0),
	_gameover(false),
	_defeated(false),
	_finished(false)
{
	DEBUG_ASSERT(x >= 'A' && x <= 'Z');
}

Order AICommander::hasUnfinished(const Descriptor& subject) const
{
	for (const Order& order : _unfinishedOrders)
	{
		if (order.subject == subject) return order;
	}
	return Order();
}

Order AICommander::hasNew(const Descriptor& subject) const
{
	for (const Order& order : _newOrders)
	{
		if (order.subject == subject) return order;
	}
	return Order();
}

void AICommander::receiveChanges(const std::vector<Change>& changes)
{
	for (auto& change : changes)
	{
		switch (change.type)
		{
			case Change::Type::YEAR:         _year    = change.year;    break;
			case Change::Type::SEASON:       _season  = change.season;  break;
			case Change::Type::DAYTIME:      _daytime = change.daytime; break;

			case Change::Type::PHASE:
			{
				_phase = change.phase;
				if (_phase == Phase::RESTING)
				{
					_unfinishedOrders.clear();
				}
				else if (_phase == Phase::PLANNING)
				{
					_newOrders.clear();
					_finished = false;
				}
				else if (_phase == Phase::ACTION)
				{
					_newOrdersConfirmed = 0;
				}
				else if (_phase == Phase::DECAY)
				{
					if (_newOrdersConfirmed < _newOrders.size())
					{
						LOGE << descriptivename() << " issued invalid orders";
					}
				}
			}
			break;

			case Change::Type::INITIATIVE:
			break;

			case Change::Type::UNFINISHED:
			{
				if (_phase == Phase::RESTING)
				{
					_unfinishedOrders.emplace_back(change.order);
				}
			}
			break;

			case Change::Type::FUNDS:
			case Change::Type::INCOME:
			case Change::Type::EXPENDITURE:
			{
				_money += change.money;
			}
			break;

			case Change::Type::GAMEOVER:
			{
				_gameover = true;
			}
			break;

			case Change::Type::DEFEAT:
			{
				if (change.player == _player)
				{
					_defeated = true;
				}
			}
			break;

			case Change::Type::ORDERED:
			{
				if (_newOrdersConfirmed < _newOrders.size()
					&& change.order == _newOrders[_newOrdersConfirmed])
				{
					_newOrdersConfirmed += 1;
				}
			}
			break;

			case Change::Type::NONE:
			{
				if (change.notice == Notice::ORDERINVALID)
				{
					LOGE << descriptivename() << " issued an invalid order";
				}
			}
			break;

			default:
			break;
		}

		_board.enact(change);
	}
}

void AICommander::receiveChangesAsJson(const Json::Value& changes)
{
	receiveChanges(Change::parseChanges(_bible, changes));
}

void AICommander::receiveChangesAsString(const std::string& changes)
{
	receiveChanges(Change::parseChanges(_bible, changes));
}

bool AICommander::wantsToPrepareOrders() const
{
	return (_phase == Phase::PLANNING && !_finished
			&& !_gameover && !_defeated);
}

void AICommander::prepareOrders()
{
	if (_finished) return;

	_newOrders.clear();
	if (_gameover || _defeated)
	{
		_finished = true;
		return;
	}

	bool processed = false;
	preprocess();
	while (!processed)
	{
		process();
		processed = postprocess();
	}
	_finished = true;
}

std::vector<Order> AICommander::orders()
{
	std::vector<Order> orders = _newOrders;
	return orders;
}

std::string AICommander::ordersAsString()
{
	std::stringstream strm;
	strm << TypeEncoder(&_bible) << "[";
	const char* sep = "";
	for (const Order& order : orders())
	{
		strm << sep << order;
		sep = ",";
	}
	strm << "]";
	return strm.str();
}

std::string AICommander::playerAsString() const
{
	return ::stringify(_player);
}

std::string AICommander::difficultyAsString() const
{
	return ::stringify(_difficulty);
}

std::string AICommander::displayname() const
{
	return NAMEPOOL[_character - 'A'];
}

std::string AICommander::descriptivename() const
{
	std::string str = ""
		"" + displayname() + ""
		" ("
		"";
	if (difficulty() != Difficulty::NONE)
	{
		str += ""
			"" + ::toupper1(::stringify(difficulty())) + ""
			" "
			"";
	}
	str += ""
		"" + ainame() + ""
		")"
		"";
	return str;
}

std::string AIDummy::ainame() const
{
	return "dummy";
}

std::string AIDummy::authors() const
{
	return "Sander in 't Veld";
}
