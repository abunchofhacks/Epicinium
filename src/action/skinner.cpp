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
#include "skinner.hpp"
#include "source.hpp"

#include "skin.hpp"
#include "bible.hpp"
#include "treetype.hpp"
#include "player.hpp"


Skinner::Skinner(const Bible& bible) :
	_bible(bible),
	climateModifier(0.0f),
	raininess(0.0f)
{}

void Skinner::clear()
{
	climateModifier = 0.0f;
	raininess = 0.0f;
	treetypes.clear();
}

void Skinner::load(const Json::Value& json)
{
	if (json["climateModifier"].isDouble())
	{
		climateModifier = json["climateModifier"].asFloat();
	}

	if (json["raininess"].isDouble())
	{
		raininess = json["raininess"].asFloat();
	}

	if (json["treetypes"].isArray())
	{
		treetypes.clear();
		for (auto item : json["treetypes"])
		{
			if (item.isString())
			{
				TreeType treetype = parseTreeType(item.asString());
				treetypes.push_back(treetype);
			}
		}
	}
}

void Skinner::store(Json::Value& json)
{
	json["climateModifier"] = climateModifier;
	json["raininess"] = raininess;
	json["treetypes"] = Json::arrayValue;
	for (TreeType treetype : treetypes)
	{
		json["treetypes"].append(::stringify(treetype));
	}
}

const Skin& Skinner::figure(const UnitType& unittype,
	const Player& /**/) const
{
	std::string skinname = "figures/";
	skinname += ::stringify(_bible.typeword(unittype));
	return Skin::get(skinname);
}

const Skin& Skinner::figure(const TileType& tiletype,
	const TreeType& treetype, const Player& /**/) const
{
	TypeWord typeword = _bible.typeword(tiletype);

	if (typeword == "forest")
	{
		if (treetype != TreeType::UNSET)
		{
			std::string skinname = "figures/";
			skinname += ::stringify(treetype);
			return Skin::get(skinname);
		}
		else
		{
			// Backwards compatibility: old recordings don't have a BORDER
			// change and therefore the TreeType of forest tiles is not set.
			// Also this is used for the icon spritename.
			return Skin::get("figures/oak");
		}
	}

	std::string skinname = "figures/";
	skinname += ::stringify(typeword);
	return Skin::get(skinname);
}

const Skin& Skinner::fence(const TileType& /**/, const Player& /**/) const
{
	return Skin::get("figures/fence");
}

const Skin& Skinner::surface(const TileType& tiletype,
	const Player& /**/) const
{
	if (_bible.tileBinding(tiletype))
	{
		return Skin::get("surfaces/home");
	}

	std::string skinname = "surfaces/";
	skinname += ::stringify(_bible.typeword(tiletype));
	return Skin::get(skinname);
}

const Skin& Skinner::border() const
{
	return Skin::get("surfaces/border");
}

const std::string& Skinner::iconname(const UnitType& unittype) const
{
	return figure(unittype, Player::NONE).iconspritename;
}

const std::string& Skinner::iconname(const TileType& tiletype) const
{
	if (_bible.tileStacksMax(tiletype) > 0)
	{
		return figure(tiletype, TreeType::UNSET, Player::NONE).iconspritename;
	}
	else
	{
		return surface(tiletype, Player::NONE).iconspritename;
	}
}
