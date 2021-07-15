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
#include "ordercontext.hpp"
#include "source.hpp"

#include "bible.hpp"
#include "board.hpp"
#include "skinner.hpp"
#include "settings.hpp"
#include "move.hpp"
#include "order.hpp"
#include "stackedlayout.hpp"
#include "image.hpp"
#include "verticallayout.hpp"
#include "horizontallayout.hpp"
#include "textfield.hpp"
#include "padding.hpp"
#include "filler.hpp"
#include "paint.hpp"
#include "colorname.hpp"
#include "graphics.hpp"
#include "cycle.hpp"


OrderContext::OrderContext(const Bible& bible, const Board& board,
		const Skinner& skinner,
		const Settings& settings,
		Season season,
		Cell cell, const Descriptor& desc,
		const Player& player, bool hasOldOrder, bool hasNewOrder,
		bool canGiveOrders) :
	Context(cell.pos()),
	_bible(bible),
	_board(board),
	_skinner(skinner),
	_settings(settings),
	_season(season),
	_cell(cell),
	_subject(desc),
	_player(player),
	_hasOldOrder(hasOldOrder),
	_hasNewOrder(hasNewOrder),
	_istile(desc.type == Descriptor::Type::TILE),
	_tiletype(_board.tile(cell).type),
	_unittype(_board.unit(cell, desc.type).type),
	_abilities()
{
	if ( _istile)
	{
		if (_board.tile(cell).owner != player) return;
	}
	else
	{
		if (_board.unit(cell, desc.type).owner != player) return;
	}

	if (_hasNewOrder)
	{
		add(Move::X, nullptr, canGiveOrders,
			"ui/ordericon_revoke",
			Paint::mix(ColorName::UIPRIMARY, ColorName::FRAME600, 0.75f),
			_("REVOKE"));
	}
	else if (_hasOldOrder || _settings.allowRedundantStop.value())
	{
		add(Move::X, nullptr, canGiveOrders,
			"ui/ordericon_halt",
			Paint::mix(ColorName::UIPRIMARY, ColorName::FRAME600, 0.75f),
			_("STOP"));
	}
	else
	{
		add(Move::X, nullptr, false, nullptr, ColorName::FRAME400, "");
	}

	determineAbilities();

	for (const Move& index : {Move::E, Move::S, Move::W, Move::N})
	{
		Order order = get(index);
		bool valid = canGiveOrders && checkValidity(order);
		std::string costtext;
		determineCosts(order, costtext);
		add(index, nullptr, valid,
			determineIconName(order), determineIconColor(order),
			determineText(order), costtext);
	}
}

std::unique_ptr<InterfaceElement> OrderContext::createPanel(const Position&,
	const Move& index, const char*,
	const char* iconname, const Paint& iconcolor,
	const std::string& text, const std::string& subtext)
{
	std::unique_ptr<InterfaceElement> panel(new StackedLayout());

	(*panel).add("panel", new Image(spritenamePanel(index)));

	(*panel)["panel"].makeClickable();

	(*panel)["panel"].setBasicColor(0, ColorName::FRAME200);
	(*panel)["panel"].setBasicColor(1, Paint::mix(ColorName::FRAME200, ColorName::FRAME400, 0.5f));
	(*panel)["panel"].setBasicColor(2, Paint::blend(ColorName::FRAME400, ColorName::SHINEBLEND));
	(*panel)["panel"].setBasicColor(3, ColorName::FRAME400);
	(*panel)["panel"].setBasicColor(4, Paint::alpha(ColorName::FRAME400, 127));

	(*panel)["panel"].setColor(5, Color::transparent());

	if (iconname || !text.empty())
	{
		(*panel).add("texts", new Padding());
		(*panel)["texts"].put(new VerticalLayout());

		if (iconname)
		{
			(*panel)["texts"].add("icon", new Image(iconname));
			(*panel)["texts"]["icon"].setBasicColor(0, iconcolor);
			(*panel)["texts"]["icon"].setBasicColor(1, iconcolor);
			(*panel)["texts"]["icon"].setBasicColor(2, iconcolor);
			(*panel)["texts"]["icon"].setMarginBottom(
				2 * InterfaceElement::scale());
		}

		if (!text.empty())
		{
			(*panel)["texts"].add("text",
				new TextField(text,
					_settings.getFontSizeMenuButton(),
					ColorName::TEXT700));
			(*panel)["texts"]["text"].setMarginBottom(
				1 * InterfaceElement::scale());
		}

		if (!subtext.empty())
		{
			(*panel)["texts"].add("subtext", new HorizontalLayout());
		}

		if (!subtext.empty() && !_bible.moraleReplacesMoney())
		{
			(*panel)["texts"]["subtext"].add("coin", new Padding());
			(*panel)["texts"]["subtext"]["coin"].put(new Image("effects/coin2_still"));
			(*panel)["texts"]["subtext"]["coin"].setMarginRight(2 * InterfaceElement::scale());
			(*panel)["texts"]["subtext"]["coin"].setPaddingTop(1 * InterfaceElement::scale());
			(*panel)["texts"]["subtext"]["coin"].setColor(0, Paint::blend(ColorName::COIN, ColorName::SHINEBLEND));
			(*panel)["texts"]["subtext"]["coin"].setColor(1, ColorName::COIN);
			(*panel)["texts"]["subtext"]["coin"].setColor(2, Paint::blend(ColorName::COIN, ColorName::SHADEBLEND));
			(*panel)["texts"]["subtext"]["coin"].setTag("Still");
		}
		else if (!subtext.empty())
		{
			(*panel)["texts"]["subtext"].add("heart", new Padding());
			(*panel)["texts"]["subtext"]["heart"].put(new Image("effects/heart1_still"));
			(*panel)["texts"]["subtext"]["heart"].setMarginRight(2 * InterfaceElement::scale());
			(*panel)["texts"]["subtext"]["heart"].setPaddingTop(2 * InterfaceElement::scale());
			(*panel)["texts"]["subtext"]["heart"].setColor(0, Paint::blend(ColorName::HEART, ColorName::SHINEBLEND));
			(*panel)["texts"]["subtext"]["heart"].setColor(1, ColorName::HEART);
			(*panel)["texts"]["subtext"]["heart"].setColor(2, Paint::blend(ColorName::HEART, ColorName::SHADEBLEND));
			(*panel)["texts"]["subtext"]["heart"].setTag("Still");
		}

		if (!subtext.empty())
		{
			(*panel)["texts"]["subtext"].add("text",
				new TextField(subtext,
					_settings.getFontSize(),
					ColorName::TEXT700));
		}

		(*panel)["texts"].align(HorizontalAlignment::CENTER);

		switch (index)
		{
			case Move::X:
			break;

			case Move::E:
			{
				(*panel)["texts"].setPaddingRight(8 * scale());
			}
			break;

			case Move::S:
			{
				(*panel)["texts"].setPaddingBottom(12 * scale());
			}
			break;

			case Move::W:
			{
				(*panel)["texts"].setPaddingLeft(8 * scale());
			}
			break;

			case Move::N:
			{
				(*panel)["texts"].setPaddingTop(16 * scale());
			}
			break;
		}

		(*panel)["texts"].settle();
	}

	(*panel).align(HorizontalAlignment::CENTER);
	(*panel).align(VerticalAlignment::MIDDLE);
	(*panel).settle();

	return panel;
}

Order OrderContext::resolve() const
{
	if (!_hoverValid) return Order();

	Move index = _hoverIndex;
	Order order = get(index);
	order.subject = _subject;
	return order;
}

Order OrderContext::peek() const
{
	Move index = _overIndex;
	Order order = get(index);
	order.subject = _subject;
	return order;
}

bool OrderContext::filled(const Move& index) const
{
	for (auto& kv : _abilities)
	{
		if (kv.first == index)
		{
			return true;
		}
	}
	return false;
}

Order OrderContext::get(const Move& index) const
{
	if (index == Move::X) return Order(Order::Type::HALT, _subject);

	for (auto& kv : _abilities)
	{
		if (kv.first != index) continue;
		return kv.second;
	}

	return Order();
}

void OrderContext::determineAbilities()
{
	if (_istile) fillAbilitiesTile();
	else fillAbilitiesUnit();
}

void OrderContext::fillAbilitiesUnit()
{
	if (_bible.unitCanCapture(_unittype))
	{
		for (const Move& index : {Move::E, Move::N, Move::W, Move::S})
		{
			if (filled(index)) continue;
			_abilities.push_back(
				{index, {Order::Type::CAPTURE, {}}});
			break;
		}
	}

	if (_bible.unitCanGuard(_unittype))
	{
		for (const Move& index : {Move::W, Move::E, Move::N, Move::S})
		{
			if (filled(index)) continue;
			_abilities.push_back(
				{index, {Order::Type::GUARD, {}}});
			break;
		}
	}

	if (_bible.unitCanFocus(_unittype))
	{
		for (const Move& index : {Move::W, Move::E, Move::N, Move::S})
		{
			if (filled(index)) continue;
			_abilities.push_back(
				{index, {Order::Type::FOCUS, {}}});
			break;
		}
	}

	if (_bible.unitCanLockdown(_unittype))
	{
		for (const Move& index : {Move::N, Move::E, Move::W, Move::S})
		{
			if (filled(index)) continue;
			_abilities.push_back(
				{index, {Order::Type::LOCKDOWN, {}}});
			break;
		}
	}

	if (_bible.unitCanShell(_unittype))
	{
		for (const Move& index : {Move::N, Move::E, Move::W, Move::S})
		{
			if (filled(index)) continue;
			_abilities.push_back(
				{index, {Order::Type::SHELL, {}}});
			break;
		}
	}

	if (_bible.unitCanBombard(_unittype))
	{
		for (const Move& index : {Move::N, Move::E, Move::W, Move::S})
		{
			if (filled(index)) continue;
			_abilities.push_back(
				{index, {Order::Type::BOMBARD, {}}});
			break;
		}
	}

	if (_bible.unitCanBomb(_unittype))
	{
		for (const Move& index : {Move::N, Move::E, Move::W, Move::S})
		{
			if (filled(index)) continue;
			_abilities.push_back(
				{index, {Order::Type::BOMB, {}}});
			break;
		}
	}

	for (const auto& build : _bible.unitShapes(_unittype))
	{
		for (const Move& index : {Move::S, Move::E, Move::N, Move::W})
		{
			if (filled(index)) continue;
			_abilities.push_back({index,
				{Order::Type::SHAPE, {}, build.type}});
			break;
		}
	}

	for (const auto& build : _bible.unitSettles(_unittype))
	{
		for (const Move& index : {Move::S, Move::E, Move::N, Move::W})
		{
			if (filled(index)) continue;
			_abilities.push_back({index,
				{Order::Type::SETTLE, {}, build.type}});
			break;
		}
	}
}

void OrderContext::fillAbilitiesTile()
{
	for (const auto& build : _bible.tileUpgrades(_tiletype))
	{
		for (const Move& index : {Move::S, Move::E, Move::N, Move::W})
		{
			if (filled(index)) continue;
			_abilities.push_back({index,
				{Order::Type::UPGRADE, {}, build.type}});
			break;
		}
	}

	for (const auto& build : _bible.tileCultivates(_tiletype))
	{
		for (const Move& index : {Move::W, Move::E, Move::N, Move::S})
		{
			if (filled(index)) continue;
			_abilities.push_back({index,
				{Order::Type::CULTIVATE, {}, build.type}});
			break;
		}
	}

	for (const auto& build : _bible.tileProduces(_tiletype))
	{
		for (const Move& index : {Move::E, Move::N, Move::W, Move::S})
		{
			if (filled(index)) continue;
			_abilities.push_back({index,
				{Order::Type::PRODUCE, {}, {}, build.type}});
			break;
		}
	}

	for (const auto& build : _bible.tileExpands(_tiletype))
	{
		for (const Move& index : {Move::E, Move::N, Move::W, Move::S})
		{
			if (filled(index)) continue;
			_abilities.push_back({index,
				{Order::Type::EXPAND, {}, {}, build.type}});
			break;
		}
	}
}

bool OrderContext::checkBible(const Order& order) const
{
	switch (order.type)
	{
		case Order::Type::NONE:
		{
			// NONE orders cannot be given.
			return false;
		}
		break;

		case Order::Type::GUARD:
		{
			// Check if the unit can guard.
			return (_bible.unitCanGuard(_unittype));
		}
		break;

		case Order::Type::FOCUS:
		{
			// Check if the unit can focus.
			return (_bible.unitCanFocus(_unittype));
		}
		break;

		case Order::Type::LOCKDOWN:
		{
			// Check if the unit can focus.
			return (_bible.unitCanLockdown(_unittype));
		}
		break;

		case Order::Type::SHELL:
		{
			// Check if the unit can capture.
			return (_bible.unitCanShell(_unittype));
		}
		break;

		case Order::Type::BOMBARD:
		{
			// Check if the unit can capture.
			return (_bible.unitCanBombard(_unittype));
		}
		break;

		case Order::Type::BOMB:
		{
			// Check if the unit can bomb.
			return (_bible.unitCanBomb(_unittype));
		}
		break;

		case Order::Type::CAPTURE:
		{
			// Check if the unit can capture.
			return (_bible.unitCanCapture(_unittype));
		}
		break;

		case Order::Type::SHAPE:
		{
			// Check if the unit can shape this type of tile.
			TileType newtype = order.tiletype;
			const auto& builds = _bible.unitShapes(_unittype);
			return (std::find_if(builds.begin(), builds.end(),
				[newtype](const Bible::TileBuild& build){

					return (build.type == newtype);
				}) != builds.end());
		}
		break;

		case Order::Type::SETTLE:
		{
			// Check if the unit can settle this type of tile.
			TileType newtype = order.tiletype;
			const auto& builds = _bible.unitSettles(_unittype);
			return (std::find_if(builds.begin(), builds.end(),
				[newtype](const Bible::TileBuild& build){

					return (build.type == newtype);
				}) != builds.end());
		}
		break;

		case Order::Type::EXPAND:
		{
			// Check if the tile can build this type of tile.
			TileType newtype = order.tiletype;
			const auto& builds = _bible.tileExpands(_tiletype);
			return (std::find_if(builds.begin(), builds.end(),
				[newtype](const Bible::TileBuild& build){

					return (build.type == newtype);
				}) != builds.end());
		}
		break;

		case Order::Type::UPGRADE:
		{
			// Check if the tile can build this type of tile.
			TileType newtype = order.tiletype;
			const auto& builds = _bible.tileUpgrades(_tiletype);
			return (std::find_if(builds.begin(), builds.end(),
				[newtype](const Bible::TileBuild& build){

					return (build.type == newtype);
				}) != builds.end());
		}
		break;

		case Order::Type::CULTIVATE:
		{
			// Check if the tile can build this type of tile.
			TileType newtype = order.tiletype;
			const auto& builds = _bible.tileCultivates(_tiletype);
			return (std::find_if(builds.begin(), builds.end(),
				[newtype](const Bible::TileBuild& build){

					return (build.type == newtype);
				}) != builds.end());
		}
		break;

		case Order::Type::PRODUCE:
		{
			// Check if the tile can produce this type of unit.
			UnitType newtype = order.unittype;
			const auto& builds = _bible.tileProduces(_tiletype);
			return (std::find_if(builds.begin(), builds.end(),
				[newtype](const Bible::UnitBuild& build){

					return (build.type == newtype);
				}) != builds.end());
		}
		break;

		case Order::Type::HALT:
		{
			// Halt orders can always be given.
			return true;
		}
		break;

		case Order::Type::MOVE: return false;
	}
	return false;
}

bool OrderContext::checkValidity(const Order& order) const
{
	// Chilled ground units cannot use abilities (i.e. orders other than Move).
	// The Chilled status effect is implemented by having Frostbite stick
	// around in Spring, so we have to detect that.
	bool coldfeet = (_bible.frostbiteGivesColdFeet()
		&& _subject.type == Descriptor::Type::GROUND
		&& _board.frostbite(_cell)
		&& _bible.chaosMinFrostbite(_season) < 0);

	switch (order.type)
	{
		case Order::Type::NONE:
		{
			// NONE orders cannot be given.
			return false;
		}
		break;

		case Order::Type::GUARD:
		{
			// Depends on the target, but a priori valid.
			return !coldfeet;
		}
		break;

		case Order::Type::FOCUS:
		{
			// Depends on the target, but a priori valid.
			return !coldfeet;
		}
		break;

		case Order::Type::LOCKDOWN:
		{
			// Depends on the target, but a priori valid.
			return !coldfeet;
		}
		break;

		case Order::Type::SHELL:
		{
			// Depends on the target, but a priori valid.
			return !coldfeet;
		}
		break;

		case Order::Type::BOMBARD:
		{
			// Depends on the target, but a priori valid.
			return !coldfeet;
		}
		break;

		case Order::Type::BOMB:
		{
			// Always valid.
			return !coldfeet;
		}
		break;

		case Order::Type::CAPTURE:
		{
			if (coldfeet) return false;
			// If we can already predict failure, we do not allow the player to give the order.
			const UnitToken& conqueror = _board.unit(_cell, _subject.type);
			const TileToken& tile = _board.tile(_cell);
			return (tile.owner != conqueror.owner && _bible.tileOwnable(tile.type));
		}
		break;

		case Order::Type::SHAPE:
		{
			if (coldfeet) return false;
			// The tile to be built on must be buildable.
			if (!_board.tile(_cell)) return false;
			return _bible.tileBuildable(_board.tile(_cell).type);
		}
		break;

		case Order::Type::SETTLE:
		{
			if (coldfeet) return false;
			// The tile to be built on must be buildable.
			if (!_board.tile(_cell)) return false;
			return _bible.tileBuildable(_board.tile(_cell).type);
		}
		break;

		case Order::Type::EXPAND:
		{
			if (coldfeet) return false;
			// Depends on the target, but a priori valid.
			if (!_settings.allowPowerless.value())
			{
				if (!_bible.powerAbolished() && _board.tile(_cell).power <= 0)
				{
					return false;
				}
			}
			return true;
		}
		break;

		case Order::Type::UPGRADE:
		{
			if (coldfeet) return false;
			// Upgrade orders can always be given, but will be delayed if necessary.
			if (!_settings.allowPowerless.value())
			{
				if (!_bible.powerAbolished() && _board.tile(_cell).power <= 0)
				{
					return false;
				}
			}
			// However we prevent the user from giving stack growth upgrades
			// if the tile is already at max stacks or below growthMax.
			if (!_settings.allowRedundantUpgrade.value())
			{
				if (order.tiletype == TileType::NONE)
				{
					TileType type = _board.tile(_cell).type;
					int stacks = _board.tile(_cell).stacks;
					return (stacks >= _bible.tileGrowthMax(type)
						&& stacks < _bible.tileStacksMax(type));
				}
			}
			return true;
		}
		break;

		case Order::Type::CULTIVATE:
		{
			if (coldfeet) return false;
			// Cultivate orders can always be given, but will be delayed if necessary.
			if (!_settings.allowPowerless.value())
			{
				if (!_bible.powerAbolished() && _board.tile(_cell).power <= 0)
				{
					return false;
				}
			}
			// However we prevent the user from giving stack growth upgrades
			// if the tile is already at max stacks.
			if (!_settings.allowRedundantCultivate.value())
			{
				int targetcount = 0;
				for (Cell target : _board.area(_cell, 1, 2))
				{
					if (_bible.tileBuildable(_board.tile(target).type)
						&& _board.tile(target).owner != _player)
					{
						targetcount++;
					}
				}
				if (targetcount == 0)
				{
					return false;
				}
			}
			return true;
		}
		break;

		case Order::Type::PRODUCE:
		{
			if (coldfeet) return false;
			// Produce orders can always be given, but will be delayed if necessary.
			if (!_settings.allowPowerless.value())
			{
				if (!_bible.powerAbolished() && _board.tile(_cell).power <= 0)
				{
					return false;
				}
			}
			return true;
		}
		break;

		case Order::Type::HALT:
		{
			// Halt orders are _always_ valid, even if the unit is Chilled.
			return true;
		}
		break;

		case Order::Type::MOVE: return false;
	}
	return false;
}

const char* OrderContext::determineIconName(const Order& order) const
{
	switch (order.type)
	{
		case Order::Type::GUARD:   return "ui/ordericon_guard";
		case Order::Type::FOCUS:   return "ui/ordericon_focus";
		case Order::Type::LOCKDOWN: return "ui/ordericon_guard";
		case Order::Type::SHELL:   return "ui/ordericon_bombard";
		case Order::Type::BOMBARD: return "ui/ordericon_bombard";
		case Order::Type::BOMB:    return "ui/ordericon_bomb";
		case Order::Type::CAPTURE: return "ui/ordericon_capture";
		case Order::Type::SHAPE:   return "ui/ordericon_trenches";
		case Order::Type::CULTIVATE: return "ui/ordericon_soil";

		case Order::Type::SETTLE:
		case Order::Type::EXPAND:
		{
			const char* iconname = _skinner.iconname(order.tiletype).c_str();
			if (*iconname) return iconname;
			else return "ui/ordericon_missing";
		}
		break;

		case Order::Type::UPGRADE:
		{
			if (order.tiletype == TileType::NONE) return "ui/ordericon_upgrade";
			const char* iconname = _skinner.iconname(order.tiletype).c_str();
			if (*iconname) return iconname;
			else return "ui/ordericon_missing";
		}
		break;

		case Order::Type::PRODUCE:
		{
			const char* iconname = _skinner.iconname(order.unittype).c_str();
			if (*iconname) return iconname;
			else return "ui/ordericon_missing";
		}
		break;

		case Order::Type::HALT: return "ui/ordericon_halt";

		case Order::Type::MOVE: return nullptr;

		case Order::Type::NONE: return nullptr;
	}

	return "ui/ordericon_revoke";
}

Paint OrderContext::determineIconColor(const Order& order) const
{
	switch (order.type)
	{
		case Order::Type::GUARD:
		case Order::Type::FOCUS:
		case Order::Type::LOCKDOWN:
		case Order::Type::SHELL:
		case Order::Type::BOMBARD:
		case Order::Type::BOMB:
		case Order::Type::CAPTURE:
		case Order::Type::SHAPE:
		case Order::Type::CULTIVATE:
		return ColorName::UIACCENT;

		case Order::Type::SETTLE:
		case Order::Type::EXPAND:
		case Order::Type::UPGRADE:
		case Order::Type::PRODUCE:
		return ColorName::FRAME600;

		case Order::Type::HALT:
		return Paint::mix(ColorName::UIPRIMARY, ColorName::FRAME600, 0.75f);

		case Order::Type::MOVE:
		case Order::Type::NONE:
		return ColorName::FRAME600;
	}

	return ColorName::FRAME600;
}

constexpr const char* KNOWN_UNIT_TYPES[] = {
	"none",
	"rifleman",
	"gunner",
	"sapper",
	"tank",
	"settler",
	"militia",
	"zeppelin",
};

constexpr const char* KNOWN_TILE_TYPES[] = {
	"none",
	"grass",
	"dirt",
	"desert",
	"rubble",
	"ridge",
	"mountain",
	"water",
	"forest",
	"city",
	"town",
	"outpost",
	"industry",
	"barracks",
	"airfield",
	"farm",
	"soil",
	"crops",
	"trenches",
};

inline std::string unitName(const Bible& bible, const UnitType& unittype)
{
	std::string unitnames[array_size(KNOWN_UNIT_TYPES)] = {
		"-", // none
		_("RIFLEMAN"),
		_("GUNNER"),
		_("SAPPER"),
		_("TANK"),
		_("SETTLER"),
		_("MILITIA"),
		_("ZEPPELIN"),
	};

	for (size_t i = 0; i < array_size(KNOWN_UNIT_TYPES); i++)
	{
		if (bible.typeword(unittype) == KNOWN_UNIT_TYPES[i])
		{
			return unitnames[i];
		}
	}

	LOGW << "Untranslated unit type: " << bible.typeword(unittype);
	return ::toupper(::stringify(bible.typeword(unittype)));
}

inline std::string tileName(const Bible& bible, const TileType& tiletype)
{
	std::string tilenames[array_size(KNOWN_TILE_TYPES)] = {
		"-", // none
		_("GRASS"),
		_("DIRT"),
		_("DESERT"),
		_("RUBBLE"),
		_("RIDGE"),
		_("MOUNTAIN"),
		_("WATER"),
		_("FOREST"),
		_("CITY"),
		_("TOWN"),
		_("OUTPOST"),
		_("INDUSTRY"),
		_("BARRACKS"),
		_("AIRFIELD"),
		_("FARM"),
		_("SOIL"),
		_("CROPS"),
		_("TRENCHES"),
	};

	for (size_t i = 0; i < array_size(KNOWN_TILE_TYPES); i++)
	{
		if (bible.typeword(tiletype) == KNOWN_TILE_TYPES[i])
		{
			return tilenames[i];
		}
	}

	LOGW << "Untranslated tiletype type: " << bible.typeword(tiletype);
	return ::toupper(::stringify(bible.typeword(tiletype)));
}

std::string OrderContext::determineText(const Order& order) const
{
	switch (order.type)
	{
		case Order::Type::GUARD:      return _("LOCKDOWN");
		case Order::Type::FOCUS:      return _("FOCUS");
		case Order::Type::LOCKDOWN:   return _("LOCKDOWN");
		case Order::Type::SHELL:      return _("SHELL");
		case Order::Type::BOMBARD:    return _("BOMBARD");
		case Order::Type::BOMB:       return _("DROP GAS");
		case Order::Type::CAPTURE:    return _("CAPTURE");
		case Order::Type::SHAPE:      return _("DIG TRENCHES");
		case Order::Type::CULTIVATE:  return _("CULTIVATE");

		case Order::Type::SETTLE:
		case Order::Type::EXPAND:
		case Order::Type::UPGRADE:
		{
			if (order.tiletype == TileType::NONE)
			{
				return _("UPGRADE");
			}
			else
			{
				return ::tileName(_bible, order.tiletype);
			}
		}
		break;

		case Order::Type::PRODUCE:
		{
			return ::unitName(_bible, order.unittype);
		}
		break;

		case Order::Type::HALT: return _("STOP");

		case Order::Type::MOVE: return _("MOVE");

		case Order::Type::NONE: return "";
	}

	return "error";
}

void OrderContext::determineCosts(const Order& order,
		std::string& costtext) const
{
	int cost = 0;

	switch (order.type)
	{
		case Order::Type::EXPAND:
		{
			TileType newtype = order.tiletype;
			const auto& builds = _bible.tileExpands(_tiletype);
			auto iter = std::find_if(builds.begin(), builds.end(),
				[newtype](const Bible::TileBuild& build){

					return (build.type == newtype);
				});
			if (iter == builds.end())
			{
				LOGE << "Missing build for " << TypeEncoder(&_bible) << newtype;
				DEBUG_ASSERT(false);
				return;
			}
			cost = iter->cost;
		}
		break;

		case Order::Type::UPGRADE:
		{
			TileType newtype = order.tiletype;
			const auto& builds = _bible.tileUpgrades(_tiletype);
			auto iter = std::find_if(builds.begin(), builds.end(),
				[newtype](const Bible::TileBuild& build){

					return (build.type == newtype);
				});
			if (iter == builds.end())
			{
				LOGE << "Missing build for " << TypeEncoder(&_bible) << newtype;
				DEBUG_ASSERT(false);
				return;
			}
			cost = iter->cost;
		}
		break;

		case Order::Type::SETTLE:
		{
			TileType newtype = order.tiletype;
			const auto& builds = _bible.unitSettles(_unittype);
			auto iter = std::find_if(builds.begin(), builds.end(),
				[newtype](const Bible::TileBuild& build){

					return (build.type == newtype);
				});
			if (iter == builds.end())
			{
				LOGE << "Missing build for " << TypeEncoder(&_bible) << newtype;
				DEBUG_ASSERT(false);
				return;
			}
			cost = iter->cost;
		}
		break;

		case Order::Type::PRODUCE:
		{
			UnitType newtype = order.unittype;
			const auto& builds = _bible.tileProduces(_tiletype);
			auto iter = std::find_if(builds.begin(), builds.end(),
				[newtype](const Bible::UnitBuild& build){

					return (build.type == newtype);
				});
			if (iter == builds.end())
			{
				LOGE << "Missing build for " << TypeEncoder(&_bible) << newtype;
				DEBUG_ASSERT(false);
				return;
			}
			cost = iter->cost;
		}
		break;

		default: return;
	}

	costtext = std::to_string(cost);
}
