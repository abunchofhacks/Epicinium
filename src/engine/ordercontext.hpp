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

#include "context.hpp"
#include "cell.hpp"
#include "descriptor.hpp"
#include "player.hpp"

class Bible;
class Board;
class Skinner;
class Settings;
enum class TileType : uint8_t;
enum class UnitType : uint8_t;
struct Order;
enum class Move : uint8_t;
class Paint;


class OrderContext : public Context
{
public:
	OrderContext(const Bible& bible, const Board& board, const Skinner& skinner,
		const Settings& settings,
		Cell cell, const Descriptor& subject,
		const Player& player, bool hasOldOrder, bool hasNewOrder,
		bool canGiveOrders);
	OrderContext(const OrderContext&) = delete;
	OrderContext(OrderContext&&) = delete;
	OrderContext& operator=(const OrderContext&) = delete;
	OrderContext& operator=(OrderContext&&) = delete;
	virtual ~OrderContext() = default;

private:
	const Bible& _bible;
	const Board& _board;
	const Skinner& _skinner;
	const Settings& _settings;
	const Cell _cell;
	const Descriptor _subject;
	const Player _player;
	const bool _hasOldOrder;
	const bool _hasNewOrder;
	const bool _istile;
	const TileType _tiletype;
	const UnitType _unittype;
	std::vector<std::pair<Move, Order>> _abilities;

	virtual std::unique_ptr<InterfaceElement> createPanel(
		const Position& position,
		const Move& index, const char* panelname,
		const char* iconname, const Paint& iconcolor,
		const std::string& text, const std::string& subtext) override;

	bool filled(const Move& index) const;
	using InterfaceElement::get;
	Order get(const Move& index) const;

	bool checkBible(const Order& order) const;
	bool checkValidity(const Order& order) const;
	const char* determineIconName(const Order& order) const;
	Paint determineIconColor(const Order& order) const;
	std::string determineText(const Order& order) const;
	void determineCosts(const Order& order, std::string& cost) const;

	void determineAbilities();
	void fillAbilitiesUnit();
	void fillAbilitiesTile();

public:
	Order resolve() const;
	Order peek() const;
};
