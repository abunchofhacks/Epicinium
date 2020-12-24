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
#include "colorname.hpp"
#include "source.hpp"

#include "parseerror.hpp"


ColorName parseColorName(const std::string& str)
{
	for (size_t i = 0; i < COLORNAME_SIZE; i++)
	{
		ColorName colorname = (ColorName) i;
		if (str == stringify(colorname))
		{
			return colorname;
		}
	}

	throw ParseError("Unknown colorname '" + str + "'");
}

const char* stringify(const ColorName& colorname)
{
	switch (colorname)
	{
		case ColorName::NEUTRAL: return "neutral";
		case ColorName::RED: return "red";
		case ColorName::BLUE: return "blue";
		case ColorName::TEAL: return "teal";
		case ColorName::YELLOW: return "yellow";
		case ColorName::PINK: return "pink";
		case ColorName::BLACK: return "black";
		case ColorName::INDIGO: return "indigo";
		case ColorName::PURPLE: return "purple";
		case ColorName::METALNEUTRAL: return "metalneutral";
		case ColorName::METALRED: return "metalred";
		case ColorName::METALBLUE: return "metalblue";
		case ColorName::METALTEAL: return "metalteal";
		case ColorName::METALYELLOW: return "metalyellow";
		case ColorName::METALPINK: return "metalpink";
		case ColorName::METALBLACK: return "metalblack";
		case ColorName::METALINDIGO: return "metalindigo";
		case ColorName::METALPURPLE: return "metalpurple";
		case ColorName::STEELNEUTRAL: return "steelneutral";
		case ColorName::STEELRED: return "steelred";
		case ColorName::STEELBLUE: return "steelblue";
		case ColorName::STEELTEAL: return "steelteal";
		case ColorName::STEELYELLOW: return "steelyellow";
		case ColorName::STEELPINK: return "steelpink";
		case ColorName::STEELBLACK: return "steelblack";
		case ColorName::STEELINDIGO: return "steelindigo";
		case ColorName::STEELPURPLE: return "steelpurple";
		case ColorName::ARMORNEUTRAL: return "armorneutral";
		case ColorName::ARMORRED: return "armorred";
		case ColorName::ARMORBLUE: return "armorblue";
		case ColorName::ARMORTEAL: return "armorteal";
		case ColorName::ARMORYELLOW: return "armoryellow";
		case ColorName::ARMORPINK: return "armorpink";
		case ColorName::ARMORBLACK: return "armorblack";
		case ColorName::ARMORINDIGO: return "armorindigo";
		case ColorName::ARMORPURPLE: return "armorpurple";
		case ColorName::CANVASNEUTRAL: return "canvasneutral";
		case ColorName::CANVASRED: return "canvasred";
		case ColorName::CANVASBLUE: return "canvasblue";
		case ColorName::CANVASTEAL: return "canvasteal";
		case ColorName::CANVASYELLOW: return "canvasyellow";
		case ColorName::CANVASPINK: return "canvaspink";
		case ColorName::CANVASBLACK: return "canvasblack";
		case ColorName::CANVASINDIGO: return "canvasindigo";
		case ColorName::CANVASPURPLE: return "canvaspurple";
		case ColorName::STONENEUTRAL: return "stoneneutral";
		case ColorName::STONERED: return "stonered";
		case ColorName::STONEBLUE: return "stoneblue";
		case ColorName::STONETEAL: return "stoneteal";
		case ColorName::STONEYELLOW: return "stoneyellow";
		case ColorName::STONEPINK: return "stonepink";
		case ColorName::STONEBLACK: return "stoneblack";
		case ColorName::STONEINDIGO: return "stoneindigo";
		case ColorName::STONEPURPLE: return "stonepurple";
		case ColorName::MARBLENEUTRAL: return "marbleneutral";
		case ColorName::MARBLERED: return "marblered";
		case ColorName::MARBLEBLUE: return "marbleblue";
		case ColorName::MARBLETEAL: return "marbleteal";
		case ColorName::MARBLEYELLOW: return "marbleyellow";
		case ColorName::MARBLEPINK: return "marblepink";
		case ColorName::MARBLEBLACK: return "marbleblack";
		case ColorName::MARBLEINDIGO: return "marbleindigo";
		case ColorName::MARBLEPURPLE: return "marblepurple";
		case ColorName::SELECTIONNEUTRAL: return "selectionneutral";
		case ColorName::SELECTIONRED: return "selectionred";
		case ColorName::SELECTIONBLUE: return "selectionblue";
		case ColorName::SELECTIONTEAL: return "selectionteal";
		case ColorName::SELECTIONYELLOW: return "selectionyellow";
		case ColorName::SELECTIONPINK: return "selectionpink";
		case ColorName::SELECTIONBLACK: return "selectionblack";
		case ColorName::SELECTIONINDIGO: return "selectionindigo";
		case ColorName::SELECTIONPURPLE: return "selectionpurple";
		case ColorName::SKINTONE100: return "skintone100";
		case ColorName::SKINTONE170: return "skintone170";
		case ColorName::SKINTONE195: return "skintone195";
		case ColorName::SKINTONE205: return "skintone205";
		case ColorName::SKINTONE215: return "skintone215";
		case ColorName::SKINTONE225: return "skintone225";
		case ColorName::SKINTONE235: return "skintone235";
		case ColorName::SKINTONE245: return "skintone245";
		case ColorName::STAR: return "star";
		case ColorName::COIN: return "coin";
		case ColorName::DIAMOND: return "diamond";
		case ColorName::HEART: return "heart";
		case ColorName::WORK: return "work";
		case ColorName::ENERGY: return "energy";
		case ColorName::BROKEN: return "broken";
		case ColorName::DENIED: return "denied";
		case ColorName::FROST: return "frost";
		case ColorName::FROSTBITE: return "frostbite";
		case ColorName::BONEDROUGHT: return "bonedrought";
		case ColorName::DEATH: return "death";
		case ColorName::BLOOD: return "blood";
		case ColorName::BLOODDARK: return "blooddark";
		case ColorName::SPARK: return "spark";
		case ColorName::SPARKDARK: return "sparkdark";
		case ColorName::BARK: return "bark";
		case ColorName::BARKDARK: return "barkdark";
		case ColorName::SKULL: return "skull";
		case ColorName::SKULLDARK: return "skulldark";
		case ColorName::SKULLDARKDARK: return "skulldarkdark";
		case ColorName::FLAME: return "flame";
		case ColorName::FLAMEDARK: return "flamedark";
		case ColorName::DUSTLIGHT: return "dustlight";
		case ColorName::DUST: return "dust";
		case ColorName::DUSTDARK: return "dustdark";
		case ColorName::GAS: return "gas";
		case ColorName::GASDARK: return "gasdark";
		case ColorName::RAIN: return "rain";
		case ColorName::RAINDARK: return "raindark";
		case ColorName::HAIL: return "hail";
		case ColorName::HAILDARK: return "haildark";
		case ColorName::SNOWFALL: return "snowfall";
		case ColorName::SNOWTIP: return "snowtip";
		case ColorName::SNOW: return "snow";
		case ColorName::SNOWDARK: return "snowdark";
		case ColorName::GLASS: return "glass";
		case ColorName::RUBBLE: return "rubble";
		case ColorName::DOOR: return "door";
		case ColorName::SOOT: return "soot";
		case ColorName::LEAD: return "lead";
		case ColorName::TREAD: return "tread";
		case ColorName::CARBON: return "carbon";
		case ColorName::METAL: return "metal";
		case ColorName::LEAFWOOD: return "leafwood";
		case ColorName::PINEWOOD: return "pinewood";
		case ColorName::PALMWOOD: return "palmwood";
		case ColorName::DARKWOOD: return "darkwood";
		case ColorName::BIRCHWOOD: return "birchwood";
		case ColorName::LEAF: return "leaf";
		case ColorName::PINE: return "pine";
		case ColorName::PALM: return "palm";
		case ColorName::LEAFDRY: return "leafdry";
		case ColorName::PINEDRY: return "pinedry";
		case ColorName::SHIRT: return "shirt";
		case ColorName::SMOKE: return "smoke";
		case ColorName::SNOWFRONT: return "snowfront";
		case ColorName::MIDFRONT: return "midfront";
		case ColorName::LOWFRONT: return "lowfront";
		case ColorName::GRASSY: return "grassy";
		case ColorName::SNOWBACK: return "snowback";
		case ColorName::MIDBACK: return "midback";
		case ColorName::LOWBACK: return "lowback";
		case ColorName::GRASS: return "grass";
		case ColorName::GRASSHOT: return "grasshot";
		case ColorName::GRASSCOLD: return "grasscold";
		case ColorName::GRASSDRY: return "grassdry";
		case ColorName::DIRT: return "dirt";
		case ColorName::DIRTDRY: return "dirtdry";
		case ColorName::SOIL: return "soil";
		case ColorName::CROPS: return "crops";
		case ColorName::DESERT: return "desert";
		case ColorName::STONE: return "stone";
		case ColorName::ROCK: return "rock";
		case ColorName::WATER: return "water";
		case ColorName::LOGOFLAME: return "logoflame";
		case ColorName::LOGOFLAMEDARK: return "logoflamedark";
		case ColorName::LOGOLAVA: return "logolava";
		case ColorName::LOGOASH: return "logoash";
		case ColorName::LOGOSAND: return "logosand";
		case ColorName::LOGODIRT: return "logodirt";
		case ColorName::LOGOGRASS: return "logograss";
		case ColorName::LOGOWATER: return "logowater";
		case ColorName::LOGOSNOW: return "logosnow";
		case ColorName::LOGOICE: return "logoice";
		case ColorName::LOGOTEXT: return "logotext";
		case ColorName::LOGOTEXTBORDER: return "logotextborder";
		case ColorName::DAWN: return "dawn";
		case ColorName::DAWNDARK: return "dawndark";
		case ColorName::NOON: return "noon";
		case ColorName::NOONDARK: return "noondark";
		case ColorName::NIGHT: return "night";
		case ColorName::MOON: return "moon";
		case ColorName::SUN: return "sun";
		case ColorName::SUNDARK: return "sundark";
		case ColorName::HAND: return "hand";
		case ColorName::HANDDARK: return "handdark";
		case ColorName::SLEEVEPRIMARY: return "sleeveprimary";
		case ColorName::SLEEVEACCENT: return "sleeveaccent";
		case ColorName::CLOTH: return "cloth";
		case ColorName::CLOTHDARK: return "clothdark";
		case ColorName::FLASH: return "flash";
		case ColorName::IMPACT: return "impact";
		case ColorName::UIACCENT: return "uiaccent";
		case ColorName::UIPRIMARY: return "uiprimary";
		case ColorName::UISECONDARY: return "uisecondary";
		case ColorName::FRAMESTONE: return "framestone";
		case ColorName::FRAMESAND: return "framesand";
		case ColorName::FRAMEPAPER: return "framepaper";
		case ColorName::FRAME100: return "frame100";
		case ColorName::FRAME200: return "frame200";
		case ColorName::FRAME400: return "frame400";
		case ColorName::FRAME600: return "frame600";
		case ColorName::FRAME800: return "frame800";
		case ColorName::FRAMEITEM: return "frameitem";
		case ColorName::FRAMETRANSPARENTITEM: return "frametransparentitem";
		case ColorName::CURSORACTIVE: return "cursoractive";
		case ColorName::CURSORINVALID: return "cursorinvalid";
		case ColorName::CURSORBUSY: return "cursorbusy";
		case ColorName::CURSORWAITING: return "cursorwaiting";
		case ColorName::GUIDEMOVE: return "guidemove";
		case ColorName::GUIDEORDER: return "guideorder";
		case ColorName::GUIDEHALT: return "guidehalt";
		case ColorName::GUIDEVALIDTARGET: return "guidevalidtarget";
		case ColorName::GUIDESLOWED: return "guideslowed";
		case ColorName::GUIDEPOSTPONED: return "guidepostponed";
		case ColorName::GUIDEINVALIDTARGET: return "guideinvalidtarget";
		case ColorName::GUIDESHINE: return "guideshine";
		case ColorName::TEXT100: return "text100";
		case ColorName::TEXT200: return "text200";
		case ColorName::TEXT400: return "text400";
		case ColorName::TEXT600: return "text600";
		case ColorName::TEXT700: return "text700";
		case ColorName::TEXT800: return "text800";
		case ColorName::TEXT900: return "text900";
		case ColorName::TEXTOWNNAME: return "textownname";
		case ColorName::TEXTANNOUNCEMENT: return "textannouncement";
		case ColorName::TEXTLOBBYCHAT: return "textlobbychat";
		case ColorName::TEXTSELECTION: return "textselection";
		case ColorName::SCROLLBAR: return "scrollbar";
		case ColorName::EDITORFRAME: return "editorframe";
		case ColorName::EDITORTEXT: return "editortext";
		case ColorName::EDITORMAIN: return "editormain";
		case ColorName::EDITORACCENT: return "editoraccent";
		case ColorName::EDITORHIGHLIGHT: return "editorhighlight";
		case ColorName::UNITSHADOW: return "unitshadow";
		case ColorName::TREESHADOW: return "treeshadow";
		case ColorName::GLAREBLEND: return "glareblend";
		case ColorName::GLINEBLEND: return "glineblend";
		case ColorName::BLACKBLEND: return "blackblend";
		case ColorName::SHINEBLEND: return "shineblend";
		case ColorName::SHADEBLEND: return "shadeblend";
		case ColorName::HOVEREDBLEND: return "hoveredblend";
		case ColorName::PRESSEDBLEND: return "pressedblend";
		case ColorName::DISABLEDBLEND: return "disabledblend";
	}
	return "error";
}

std::ostream& operator<<(std::ostream& os, const ColorName& colorname)
{
	return os << stringify(colorname);
}
