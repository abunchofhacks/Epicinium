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
#include "tutorial.hpp"
#include "source.hpp"

#include "commander.hpp"
#include "bible.hpp"
#include "interfaceelement.hpp"
#include "input.hpp"
#include "paint.hpp"
#include "colorname.hpp"
#include "settings.hpp"
#include "graphics.hpp"
#include "language.hpp"


constexpr ColorName TEXTCOLORCONTROLS = ColorName::TEXT900;

enum class Tutorial::State
{
	NONE = 0,
	SELECTUNIT,
	SELECTUNITNOTTILE,
	SELECTUNITYOUCONTROL,
	SELECTUNITAGAIN,
	GIVEMOVECOMMAND,
	GIVEMOVECOMMANDNOTABILITY,
	STARTACTIONPHASE,
	WATCHACTIONPHASE,
	STARTNIGHTPHASE,
	WATCHNIGHTPHASE,
	SELECTTILE,
	SELECTTILENOTUNIT,
	SELECTTILEYOUCONTROL,
	SELECTTILEAGAIN,
	PRODUCESETTLER,
	PRODUCESETTLERNOTOTHER,
	STARTACTIONPHASE2,
	WATCHACTIONPHASE2,
	STARTWEATHERPHASE,
	WATCHWEATHERPHASE,
	TAKECONTROL,
	INCONTROL,
	WATCHACTIONPHASE3,
	USESETTLERS,
	INCONTROL2,
	WATCHACTIONPHASE4,
	USEINDUSTRY,
	INCONTROL3,
	WATCHACTIONPHASE5,
	USEBARRACKS,
	INCONTROL4,
	VICTORIOUS,
	DEFEATED,
};

Tutorial::Tutorial(Commander& commander, const Settings& settings) :
	_commander(commander),
	_settings(settings),
	_settlertype(_commander.bible().unittype("settler")),
	_industrytype(_commander.bible().tiletype("industry")),
	_barrackstype(_commander.bible().tiletype("barracks")),
	_state(State::NONE),
	_clickCount(0)
{}

void Tutorial::load()
{
	_commander.buildInfoBar();

	InterfaceElement& layout = _commander.getStatusBar();
	layout.setColor(0, Paint::alpha(ColorName::FRAMEPAPER, 200));
	layout.setColor(1,
		Paint::blend(ColorName::FRAMEPAPER, ColorName::SHADEBLEND, 0.5f));
	layout.setPowerColor(0, ColorName::FRAMEPAPER);
	layout.setShineColor(ColorName::GUIDESHINE);
	layout.power();
	prompt(State::SELECTUNIT);
}

void Tutorial::update()
{
	switch (_state)
	{
		case State::NONE:
		break;
		case State::SELECTUNIT:
		{
			_commander.getReadyButton().enableIf(false);

			if (Input::get()->wasKeyReleased(SDL_SCANCODE_LMB)
				&& _commander._hoversquare != nullptr)
			{
				std::vector<Order> neworders = _commander.listNewOrders();

				if (std::find_if(neworders.begin(), neworders.end(),
							[](const Order& order){

						return order.type == Order::Type::MOVE;

					}) != neworders.end())
				{
					prompt(State::STARTACTIONPHASE);
				}
				else if (_commander._selector.type == Descriptor::Type::GROUND)
				{
					prompt(State::GIVEMOVECOMMAND);
				}
				else if (_commander._selector.type == Descriptor::Type::TILE)
				{
					prompt(State::SELECTUNITNOTTILE);
				}
				else if (++_clickCount > 5)
				{
					prompt(State::SELECTUNITYOUCONTROL);
				}
			}
		}
		break;
		case State::SELECTUNITNOTTILE:
		case State::SELECTUNITYOUCONTROL:
		case State::SELECTUNITAGAIN:
		{
			_commander.getReadyButton().enableIf(false);

			if (Input::get()->wasKeyReleased(SDL_SCANCODE_LMB))
			{
				std::vector<Order> neworders = _commander.listNewOrders();

				if (std::find_if(neworders.begin(), neworders.end(),
							[](const Order& order){

						return order.type == Order::Type::MOVE;

					}) != neworders.end())
				{
					prompt(State::STARTACTIONPHASE);
				}
				else if (_commander._selectsquare != nullptr
					&& _commander._selector.type == Descriptor::Type::GROUND)
				{
					prompt(State::GIVEMOVECOMMAND);
				}
				else if (_commander._selectsquare != nullptr
					&& _commander._selector.type == Descriptor::Type::TILE)
				{
					prompt(State::SELECTUNITNOTTILE);
				}
				else if (_commander._hoversquare != nullptr)
				{
					prompt(State::SELECTUNITYOUCONTROL);
				}
			}
		}
		break;
		case State::GIVEMOVECOMMAND:
		case State::GIVEMOVECOMMANDNOTABILITY:
		{
			_commander.getReadyButton().enableIf(false);

			if (Input::get()->wasKeyReleased(SDL_SCANCODE_RMB)
				&& _commander._selectsquare != nullptr)
			{
				std::vector<Order> neworders = _commander.listNewOrders();

				if (std::find_if(neworders.begin(), neworders.end(),
							[](const Order& order){

						return order.type == Order::Type::MOVE;

					}) != neworders.end())
				{
					prompt(State::STARTACTIONPHASE);
				}
				else if (std::find_if(neworders.begin(), neworders.end(),
							[](const Order& order){

						return order.type != Order::Type::NONE;

					}) != neworders.end())
				{
					prompt(State::GIVEMOVECOMMANDNOTABILITY);
				}
			}
			else if (Input::get()->wasKeyReleased(SDL_SCANCODE_LMB))
			{
				std::vector<Order> neworders = _commander.listNewOrders();

				if (_commander._selectsquare == nullptr
					|| _commander._selector.type != Descriptor::Type::GROUND)
				{
					prompt(State::SELECTUNITAGAIN);
				}
				else if (std::find_if(neworders.begin(), neworders.end(),
							[](const Order& order){

						return order.type == Order::Type::MOVE;

					}) == neworders.end())
				{
					prompt(State::GIVEMOVECOMMANDNOTABILITY);
				}
			}
		}
		break;
		case State::STARTACTIONPHASE:
		{
			if (_commander._phase != Phase::PLANNING)
			{
				prompt(State::WATCHACTIONPHASE);
			}
			else if ((Input::get()->wasKeyReleased(SDL_SCANCODE_LMB)
					|| Input::get()->wasKeyReleased(SDL_SCANCODE_RMB))
				&& _commander._selectsquare != nullptr)
			{
				std::vector<Order> neworders = _commander.listNewOrders();

				if (std::find_if(neworders.begin(), neworders.end(),
							[](const Order& order){

						return order.type == Order::Type::MOVE;

					}) == neworders.end())
				{
					prompt(State::GIVEMOVECOMMANDNOTABILITY);
				}
			}
		}
		break;
		case State::WATCHACTIONPHASE:
		{
			if (_commander._phase == Phase::DECAY)
			{
				prompt(State::STARTNIGHTPHASE);
			}
		}
		break;
		case State::STARTNIGHTPHASE:
		{
			if (_commander._pauseOnce == false)
			{
				prompt(State::WATCHNIGHTPHASE);
			}
		}
		break;
		case State::WATCHNIGHTPHASE:
		{
			if (_commander._phase == Phase::PLANNING)
			{
				prompt(State::SELECTTILE);
			}
		}
		break;
		case State::SELECTTILE:
		{
			_commander.getReadyButton().enableIf(false);

			if (Input::get()->wasKeyReleased(SDL_SCANCODE_LMB)
				&& _commander._selectsquare != nullptr)
			{
				std::vector<Order> neworders = _commander.listNewOrders();
				const UnitType settlertype = _settlertype;

				if (std::find_if(neworders.begin(), neworders.end(),
							[settlertype](const Order& order) {

						return (order.type == Order::Type::PRODUCE
							&& order.unittype == settlertype);

					}) != neworders.end())
				{
					prompt(State::STARTACTIONPHASE2);
				}
				else if (_commander._selector.type == Descriptor::Type::TILE)
				{
					prompt(State::PRODUCESETTLER);
				}
				else if (_commander._selector.type == Descriptor::Type::GROUND)
				{
					prompt(State::SELECTTILENOTUNIT);
				}
				else
				{
					prompt(State::SELECTTILEYOUCONTROL);
				}
			}
		}
		break;
		case State::SELECTTILENOTUNIT:
		case State::SELECTTILEYOUCONTROL:
		case State::SELECTTILEAGAIN:
		{
			_commander.getReadyButton().enableIf(false);

			if (Input::get()->wasKeyReleased(SDL_SCANCODE_LMB))
			{
				std::vector<Order> neworders = _commander.listNewOrders();
				const UnitType settlertype = _settlertype;

				if (std::find_if(neworders.begin(), neworders.end(),
							[settlertype](const Order& order) {

						return (order.type == Order::Type::PRODUCE
							&& order.unittype == settlertype);

					}) != neworders.end())
				{
					prompt(State::STARTACTIONPHASE2);
				}
				else if (_commander._selectsquare != nullptr
					&& _commander._selector.type == Descriptor::Type::TILE)
				{
					prompt(State::PRODUCESETTLER);
				}
				else if (_commander._selectsquare != nullptr
					&& _commander._selector.type == Descriptor::Type::GROUND)
				{
					prompt(State::SELECTTILENOTUNIT);
				}
				else if (_commander._hoversquare != nullptr)
				{
					prompt(State::SELECTTILEYOUCONTROL);
				}
			}
		}
		break;
		case State::PRODUCESETTLER:
		case State::PRODUCESETTLERNOTOTHER:
		{
			_commander.getReadyButton().enableIf(false);

			if ((Input::get()->wasKeyReleased(SDL_SCANCODE_LMB)
					|| Input::get()->wasKeyReleased(SDL_SCANCODE_RMB)))
			{
				std::vector<Order> neworders = _commander.listNewOrders();
				const UnitType settlertype = _settlertype;

				if (std::find_if(neworders.begin(), neworders.end(),
							[settlertype](const Order& order) {

						return (order.type == Order::Type::PRODUCE
							&& order.unittype == settlertype);

					}) != neworders.end())
				{
					prompt(State::STARTACTIONPHASE2);
				}
				else if (_commander._selectsquare == nullptr
					|| _commander._selector.type != Descriptor::Type::TILE)
				{
					prompt(State::SELECTTILEAGAIN);
				}
				else if (_commander._hoversquare != nullptr)
				{
					prompt(State::PRODUCESETTLERNOTOTHER);
				}
			}
		}
		break;
		case State::STARTACTIONPHASE2:
		{
			if (_commander._phase != Phase::PLANNING)
			{
				prompt(State::WATCHACTIONPHASE2);
			}
			else if ((Input::get()->wasKeyReleased(SDL_SCANCODE_LMB)
					|| Input::get()->wasKeyReleased(SDL_SCANCODE_RMB))
				&& _commander._selectsquare != nullptr)
			{
				std::vector<Order> neworders = _commander.listNewOrders();
				const UnitType settlertype = _settlertype;

				if (std::find_if(neworders.begin(), neworders.end(),
							[settlertype](const Order& order) {

						return (order.type == Order::Type::PRODUCE
							&& order.unittype == settlertype);

					}) == neworders.end())
				{
					prompt(State::PRODUCESETTLERNOTOTHER);
				}
			}
		}
		break;
		case State::WATCHACTIONPHASE2:
		{
			if (_commander._phase == Phase::GROWTH
				&& _commander._daytime == Daytime::LATE)
			{
				prompt(State::STARTWEATHERPHASE);
			}
		}
		break;
		case State::STARTWEATHERPHASE:
		{
			if (_commander._pauseOnce == false)
			{
				prompt(State::WATCHWEATHERPHASE);
			}
		}
		break;
		case State::WATCHWEATHERPHASE:
		{
			if (_commander._phase == Phase::PLANNING)
			{
				prompt(State::TAKECONTROL);
			}
		}
		break;
		case State::TAKECONTROL:
		{
			_commander.getReadyButton().enableIf(false);

			if (Input::get()->wasKeyReleased(SDL_SCANCODE_LMB)
				&& _commander._selectsquare != nullptr)
			{
				if (_commander._selector.type == Descriptor::Type::GROUND
					|| _commander._selector.type == Descriptor::Type::TILE)
				{
					prompt(State::INCONTROL);
					_commander.getReadyButton().enable();
				}
			}
		}
		break;
		case State::INCONTROL:
		{
			if (_commander._gameover)
			{
				if (_commander._defeated)
				{
					prompt(State::DEFEATED);
				}
				else
				{
					prompt(State::VICTORIOUS);
				}
			}
			else if (_commander._phase == Phase::ACTION)
			{
				prompt(State::WATCHACTIONPHASE3);
			}
		}
		break;
		case State::WATCHACTIONPHASE3:
		{
			if (_commander._phase == Phase::PLANNING)
			{
				prompt(State::USESETTLERS);
			}
		}
		break;
		case State::USESETTLERS:
		{
			if ((Input::get()->wasKeyReleased(SDL_SCANCODE_LMB)
					|| Input::get()->wasKeyReleased(SDL_SCANCODE_RMB))
				&& _commander._selectsquare != nullptr)
			{
				std::vector<Order> neworders = _commander.listNewOrders();

				if (std::find_if(neworders.begin(), neworders.end(),
							[](const Order& order) {

						return (order.type == Order::Type::SETTLE);

					}) != neworders.end())
				{
					prompt(State::INCONTROL2);
				}
			}
		}
		break;
		case State::INCONTROL2:
		{
			if (_commander._gameover)
			{
				if (_commander._defeated)
				{
					prompt(State::DEFEATED);
				}
				else
				{
					prompt(State::VICTORIOUS);
				}
			}
			else if (_commander._phase == Phase::ACTION)
			{
				prompt(State::WATCHACTIONPHASE4);
			}
		}
		break;
		case State::WATCHACTIONPHASE4:
		{
			if (_commander._phase == Phase::PLANNING)
			{
				prompt(State::USEINDUSTRY);
			}
		}
		break;
		case State::USEINDUSTRY:
		{
			if (Input::get()->wasKeyReleased(SDL_SCANCODE_LMB)
				&& _commander._selectsquare != nullptr)
			{
				std::vector<Order> neworders = _commander.listNewOrders();
				const TileType industrytype = _industrytype;

				if (_commander._gameover)
				{
					if (_commander._defeated)
					{
						prompt(State::DEFEATED);
					}
					else
					{
						prompt(State::VICTORIOUS);
					}
				}
				else if (std::find_if(neworders.begin(), neworders.end(),
							[industrytype](const Order& order) {

						return (order.type == Order::Type::EXPAND
							&& (order.tiletype == industrytype));

					}) != neworders.end())
				{
					prompt(State::INCONTROL3);
				}
			}
		}
		break;
		case State::INCONTROL3:
		{
			if (_commander._gameover)
			{
				if (_commander._defeated)
				{
					prompt(State::DEFEATED);
				}
				else
				{
					prompt(State::VICTORIOUS);
				}
			}
			else if (_commander._phase == Phase::ACTION)
			{
				prompt(State::WATCHACTIONPHASE5);
			}
		}
		break;
		case State::WATCHACTIONPHASE5:
		{
			if (_commander._phase == Phase::PLANNING)
			{
				prompt(State::USEBARRACKS);
			}
		}
		break;
		case State::USEBARRACKS:
		{
			if ((Input::get()->wasKeyReleased(SDL_SCANCODE_LMB)
					|| Input::get()->wasKeyReleased(SDL_SCANCODE_RMB))
				&& _commander._selectsquare != nullptr)
			{
				std::vector<Order> neworders = _commander.listNewOrders();
				const TileType barrackstype = _barrackstype;

				if (_commander._gameover)
				{
					if (_commander._defeated)
					{
						prompt(State::DEFEATED);
					}
					else
					{
						prompt(State::VICTORIOUS);
					}
				}
				else if (std::find_if(neworders.begin(), neworders.end(),
							[barrackstype](const Order& order) {

						return (order.type == Order::Type::EXPAND
							&& (order.tiletype == barrackstype));

					}) != neworders.end())
				{
					prompt(State::INCONTROL4);
				}
			}
		}
		break;
		case State::INCONTROL4:
		{
			if (_commander._gameover)
			{
				if (_commander._defeated)
				{
					prompt(State::DEFEATED);
				}
				else
				{
					prompt(State::VICTORIOUS);
				}
			}
		}
		break;
		case State::VICTORIOUS:
		case State::DEFEATED:
		break;
	}
}

bool Tutorial::isEnglish() const
{
	return Language::isCurrentlyEnglish();
}

void Tutorial::prompt(State newstate)
{
	const int FONTSIZESMALL = _settings.getFontSizeTutorial();

	InterfaceElement& layout = _commander.getStatusBar();

	_state = newstate;

	switch (_state)
	{
		case State::NONE:
		break;
		case State::SELECTUNIT:
		{
			if (!isEnglish())
			{
				layout["texts"].setText(
					_(""
					"Hi! This tutorial will teach you how to play Epicinium."
					"")
				);
				layout["texts"].addText(
					"\n"
				);
				layout["texts"].addText(
					_(""
					"Epicinium is a strategy game played in rounds,"
					" where each round consists of multiple phases."
					" In the"
					" Planning phase"
					", you can select units"
					" by clicking on them"
					" with the"
					" LEFT MOUSE BUTTON"
					"."
					"")
				);
				layout.settle();
				layout.shine();
				break;
			}

			layout["texts"].setText(
				"Hi! This tutorial will teach you how to play Epicinium."
				"\n"
				"Epicinium is a strategy game played in rounds,"
				" where each round consists of multiple phases."
				" In the"
				" "
			);
			layout["texts"].addIcon("ui/ordericon_planning", ColorName::UIPRIMARY);
			layout["texts"].addText(
				" Planning phase"
				, FONTSIZESMALL, ColorName::UIPRIMARY
			);
			layout["texts"].addText(
				", you can select units"
				" by clicking on them"
				" with the"
			);
			layout["texts"].addText(
				" LEFT MOUSE BUTTON"
				, FONTSIZESMALL, TEXTCOLORCONTROLS
			);
			layout["texts"].addText(
				"."
			);
			layout.settle();
			layout.shine();
		}
		break;
		case State::SELECTUNITNOTTILE:
		{
			if (!isEnglish())
			{
				layout["texts"].setText(
					_(""
					"Good, you've selected your"
					" City"
					" tile."
					" Now try selecting one of your"
					" Militia"
					" units"
					" by clicking on it"
					" with the"
					" LEFT MOUSE BUTTON"
					"."
					"")
				);
				layout.settle();
				layout.shine();
				break;
			}

			layout["texts"].setText(
				"Good, you've selected your"
				" "
			);
			layout["texts"].addIcon("ui/ordericon_city", ColorName::UIPRIMARY);
			layout["texts"].addText(
				" City"
				, FONTSIZESMALL, ColorName::UIPRIMARY
			);
			layout["texts"].addText(
				" tile."
				" Now try selecting one of your"
				" "
			);
			layout["texts"].addIcon("ui/ordericon_militia", ColorName::UIPRIMARY);
			layout["texts"].addText(
				" Militia"
				, FONTSIZESMALL, ColorName::UIPRIMARY
			);
			layout["texts"].addText(
				" units"
				" by clicking on it"
				" with the"
			);
			layout["texts"].addText(
				" LEFT MOUSE BUTTON"
				, FONTSIZESMALL, TEXTCOLORCONTROLS
			);
			layout["texts"].addText(
				"."
			);
			layout.settle();
			layout.shine();
		}
		break;
		case State::SELECTUNITYOUCONTROL:
		{
			if (!isEnglish())
			{
				layout["texts"].setText(
					_(""
					"Your units are red."
					" Try selecting one of your"
					" Militia"
					" units"
					" by clicking on it"
					" with the"
					" LEFT MOUSE BUTTON"
					"."
					"")
				);
				layout.settle();
				layout.shine();
				break;
			}

			layout["texts"].setText(
				"Your units are red."
				" Try selecting one of your"
				" "
			);
			layout["texts"].addIcon("ui/ordericon_militia", ColorName::UIPRIMARY);
			layout["texts"].addText(
				" Militia"
				, FONTSIZESMALL, ColorName::UIPRIMARY
			);
			layout["texts"].addText(
				" units"
				" by clicking on it"
				" with the"
			);
			layout["texts"].addText(
				" LEFT MOUSE BUTTON"
				, FONTSIZESMALL, TEXTCOLORCONTROLS
			);
			layout["texts"].addText(
				"."
			);
			layout.settle();
			layout.shine();
		}
		break;
		case State::SELECTUNITAGAIN:
		{
			if (!isEnglish())
			{
				layout["texts"].setText(
					_(""
					"Select one of your"
					" Militia"
					" units"
					" by clicking on it"
					" with the"
					" LEFT MOUSE BUTTON"
					"."
					"")
				);
				layout.settle();
				layout.shine();
				break;
			}

			layout["texts"].setText(
				"Select one of your"
				" "
			);
			layout["texts"].addIcon("ui/ordericon_militia", ColorName::UIPRIMARY);
			layout["texts"].addText(
				" Militia"
				, FONTSIZESMALL, ColorName::UIPRIMARY
			);
			layout["texts"].addText(
				" units"
				" by clicking on it"
				" with the"
			);
			layout["texts"].addText(
				" LEFT MOUSE BUTTON"
				, FONTSIZESMALL, TEXTCOLORCONTROLS
			);
			layout["texts"].addText(
				"."
			);
			layout.settle();
			layout.shine();
		}
		break;
		case State::GIVEMOVECOMMAND:
		{
			if (!isEnglish())
			{
				layout["texts"].setText(
					_(""
					"Once selected, units can be ordered to move and attack"
					" by clicking on their destination"
					" with the"
					" RIGHT MOUSE BUTTON"
					"."
					"")
				);
				layout.settle();
				layout.shine();
				break;
			}

			layout["texts"].setText(
				"Once selected, units can be ordered to move and attack"
				" by clicking on their destination"
				" with the"
			);
			layout["texts"].addText(
				" RIGHT MOUSE BUTTON"
				, FONTSIZESMALL, TEXTCOLORCONTROLS
			);
			layout["texts"].addText(
				"."
			);
			layout.settle();
			layout.shine();
		}
		break;
		case State::GIVEMOVECOMMANDNOTABILITY:
		{
			if (!isEnglish())
			{
				layout["texts"].setText(
					_(""
					"Give your unit a Move order"
					" by clicking on a different location"
					" with the"
					" RIGHT MOUSE BUTTON"
					"."
					"")
				);
				layout.settle();
				layout.shine();
				break;
			}

			layout["texts"].setText(
				"Give your unit a Move order"
				" by clicking on a different location"
				" with the"
			);
			layout["texts"].addText(
				" RIGHT MOUSE BUTTON"
				, FONTSIZESMALL, TEXTCOLORCONTROLS
			);
			layout["texts"].addText(
				"."
			);
			layout.settle();
			layout.shine();
		}
		break;
		case State::STARTACTIONPHASE:
		{
			if (!isEnglish())
			{
				layout["texts"].setText(
					_(""
					"Excellent!"
					" The unit did not yet move,"
					" because the game is currently in the"
					" Planning phase"
					"."
					" Click the"
					" Ready"
					" button"
					" or press the"
					" SPACE BAR"
					" to start the"
					" Action phase"
					"."
					"")
				);
				layout.settle();
				layout.shine();
				_commander.getReadyButton().enable();
				break;
			}

			layout["texts"].setText(
				"Excellent!"
				" The unit did not yet move,"
				" because the game is currently in the"
				" "
			);
			layout["texts"].addIcon("ui/ordericon_planning", ColorName::UIPRIMARY);
			layout["texts"].addText(
				" Planning phase"
				, FONTSIZESMALL, ColorName::UIPRIMARY
			);
			layout["texts"].addText(
				"."
				" Click the"
			);
			layout["texts"].addText(
				" Ready"
				, FONTSIZESMALL, ColorName::TEXT900
			);
			layout["texts"].addText(
				" button"
				" or press the"
			);
			layout["texts"].addText(
				" SPACE BAR"
				, FONTSIZESMALL, TEXTCOLORCONTROLS
			);
			layout["texts"].addText(
				""
				" to start the"
				" "
			);
			layout["texts"].addIcon("ui/ordericon_action", ColorName::UIPRIMARY);
			layout["texts"].addText(
				" Action phase"
				, FONTSIZESMALL, ColorName::UIPRIMARY
			);
			layout["texts"].addText(
				"."
			);
			layout.settle();
			layout.shine();
			_commander.getReadyButton().enable();
		}
		break;
		case State::WATCHACTIONPHASE:
		{
			if (!isEnglish())
			{
				layout["texts"].setText(
					_(""
					"In the"
					" Action phase"
					","
					" players take turns automatically executing their orders."
					"")
				);
				layout.settle();
				layout.deshine();
				break;
			}

			layout["texts"].setText(
				"In the"
				" "
			);
			layout["texts"].addIcon("ui/ordericon_action", ColorName::UIPRIMARY);
			layout["texts"].addText(
				" Action phase"
				, FONTSIZESMALL, ColorName::UIPRIMARY
			);
			layout["texts"].addText(
				","
				" players take turns automatically executing their orders."
			);
			layout.settle();
			layout.deshine();
		}
		break;
		case State::STARTNIGHTPHASE:
		{
			if (!isEnglish())
			{
				layout["texts"].setText(
					_(""
					"In the"
					" Action phase"
					","
					" players take turns automatically executing their orders."
					"")
				);
				layout["texts"].addText(
					"\n"
				);
				layout["texts"].addText(
					_(""
					"Next, a"
					" Night phase"
					" will take place."
					" (Click the pause sign"
					" or press the"
					" SPACE BAR"
					" to unpause.)"
					"")
				);
				layout.settle();
				layout.shine();
				_commander._pauseOnce = true;
				break;
			}

			layout["texts"].setText(
				"In the"
				" "
			);
			layout["texts"].addIcon("ui/ordericon_action", ColorName::UIPRIMARY);
			layout["texts"].addText(
				" Action phase"
				, FONTSIZESMALL, ColorName::UIPRIMARY
			);
			layout["texts"].addText(
				","
				" players take turns automatically executing their orders."
				"\n"
				"Next, a"
				" "
			);
			layout["texts"].addIcon("ui/ordericon_night", ColorName::UIPRIMARY);
			layout["texts"].addText(
				" Night phase"
				, FONTSIZESMALL, ColorName::UIPRIMARY
			);
			layout["texts"].addText(
				" will take place."
				" (Click the pause sign"
				" or press the"
			);
			layout["texts"].addText(
				" SPACE BAR"
				, FONTSIZESMALL, TEXTCOLORCONTROLS
			);
			layout["texts"].addText(
				""
				" to unpause.)"
			);
			layout.settle();
			layout.shine();
			_commander._pauseOnce = true;
		}
		break;
		case State::WATCHNIGHTPHASE:
		{
			if (!isEnglish())
			{
				layout["texts"].setText(
					_(""
					"At night,"
					" City"
					" tiles can gain power"
					" if they are surrounded by enough"
					" Grass, Forest, Crops or Water tiles."
					" City"
					" tiles also generate income at night."
					"")
				);
				layout.settle();
				layout.deshine();
				break;
			}

			layout["texts"].setText(
				"At night,"
				""
				" "
			);
			layout["texts"].addIcon("ui/ordericon_city", ColorName::UIPRIMARY);
			layout["texts"].addText(
				" City"
				, FONTSIZESMALL, ColorName::UIPRIMARY
			);
			layout["texts"].addText(
				" tiles can gain power"
				" if they are surrounded by enough"
				" Grass, Forest, Crops or Water tiles."
				""
				" "
			);
			layout["texts"].addIcon("ui/ordericon_city", ColorName::UIPRIMARY);
			layout["texts"].addText(
				" City"
				, FONTSIZESMALL, ColorName::UIPRIMARY
			);
			layout["texts"].addText(
				" tiles also generate income at night."
			);
			layout.settle();
			layout.deshine();
		}
		break;
		case State::SELECTTILE:
		{
			if (!isEnglish())
			{
				layout["texts"].setText(
					_(""
					"At night,"
					" City"
					" tiles can gain power"
					" if they are surrounded by enough"
					" Grass, Forest, Crops or Water tiles."
					" City"
					" tiles also generate income at night."
					"")
				);
				layout["texts"].addText(
					"\n"
				);
				layout["texts"].addText(
					_(""
					"Afterwards, a new"
					" Planning phase"
					" begins."
					" Time to issue new orders to your tiles and units."
					"")
				);
				layout.settle();
				layout.shine();
				break;
			}

			layout["texts"].setText(
				"At night,"
				""
				" "
			);
			layout["texts"].addIcon("ui/ordericon_city", ColorName::UIPRIMARY);
			layout["texts"].addText(
				" City"
				, FONTSIZESMALL, ColorName::UIPRIMARY
			);
			layout["texts"].addText(
				" tiles can gain power"
				" if they are surrounded by enough"
				" Grass, Forest, Crops or Water tiles."
				""
				" "
			);
			layout["texts"].addIcon("ui/ordericon_city", ColorName::UIPRIMARY);
			layout["texts"].addText(
				" City"
				, FONTSIZESMALL, ColorName::UIPRIMARY
			);
			layout["texts"].addText(
				" tiles also generate income at night."
				"\n"
				"Afterwards, a new"
				" "
			);
			layout["texts"].addIcon("ui/ordericon_planning", ColorName::UIPRIMARY);
			layout["texts"].addText(
				" Planning phase"
				, FONTSIZESMALL, ColorName::UIPRIMARY
			);
			layout["texts"].addText(
				" begins."
				" Time to issue new orders to your tiles and units."
			);
			layout.settle();
			layout.shine();
		}
		break;
		case State::SELECTTILENOTUNIT:
		{
			if (!isEnglish())
			{
				layout["texts"].setText(
					_(""
					"Besides units, you can also give orders to tiles."
					" Try selecting one of your"
					" City"
					" tiles"
					" by clicking on it"
					" with the"
					" LEFT MOUSE BUTTON"
					"."
					"")
				);
				layout.settle();
				layout.shine();
				break;
			}

			layout["texts"].setText(
				"Besides units, you can also give orders to tiles."
				" Try selecting one of your"
				" "
			);
			layout["texts"].addIcon("ui/ordericon_city", ColorName::UIPRIMARY);
			layout["texts"].addText(
				" City"
				, FONTSIZESMALL, ColorName::UIPRIMARY
			);
			layout["texts"].addText(
				" tiles"
				" by clicking on it"
				" with the"
			);
			layout["texts"].addText(
				" LEFT MOUSE BUTTON"
				, FONTSIZESMALL, TEXTCOLORCONTROLS
			);
			layout["texts"].addText(
				"."
			);
			layout.settle();
			layout.shine();
		}
		break;
		case State::SELECTTILEYOUCONTROL:
		{
			if (!isEnglish())
			{
				layout["texts"].setText(
					_(""
					"Tiles you control have red highlights."
					" Try selecting one of your"
					" City"
					" tiles"
					" by clicking on it"
					" with the"
					" LEFT MOUSE BUTTON"
					"."
					"")
				);
				layout.settle();
				layout.shine();
				break;
			}

			layout["texts"].setText(
				"Tiles you control have red highlights."
				" Try selecting one of your"
				" "
			);
			layout["texts"].addIcon("ui/ordericon_city", ColorName::UIPRIMARY);
			layout["texts"].addText(
				" City"
				, FONTSIZESMALL, ColorName::UIPRIMARY
			);
			layout["texts"].addText(
				" tiles"
				" by clicking on it"
				" with the"
			);
			layout["texts"].addText(
				" LEFT MOUSE BUTTON"
				, FONTSIZESMALL, TEXTCOLORCONTROLS
			);
			layout["texts"].addText(
				"."
			);
			layout.settle();
			layout.shine();
		}
		break;
		case State::SELECTTILEAGAIN:
		{
			if (!isEnglish())
			{
				layout["texts"].setText(
					_(""
					"Select one of your"
					" City"
					" tiles"
					" by clicking on it"
					" with the"
					" LEFT MOUSE BUTTON"
					"."
					"")
				);
				layout.settle();
				layout.shine();
				break;
			}

			layout["texts"].setText(
				"Select one of your"
				" "
			);
			layout["texts"].addIcon("ui/ordericon_city", ColorName::UIPRIMARY);
			layout["texts"].addText(
				" City"
				, FONTSIZESMALL, ColorName::UIPRIMARY
			);
			layout["texts"].addText(
				" tiles"
				" by clicking on it"
				" with the"
			);
			layout["texts"].addText(
				" LEFT MOUSE BUTTON"
				, FONTSIZESMALL, TEXTCOLORCONTROLS
			);
			layout["texts"].addText(
				"."
			);
			layout.settle();
			layout.shine();
		}
		break;
		case State::PRODUCESETTLER:
		{
			if (!isEnglish())
			{
				layout["texts"].setText(
					_(""
					"Tiles can produce new units and build other buildings."
					" Once selected, access these abilities"
					" by clicking on the tile again"
					" with the"
					" RIGHT MOUSE BUTTON"
					"."
					" Please order one of your"
					" City"
					" tiles"
					" to produce a"
					" Settler"
					" unit."
					"")
				);
				layout.settle();
				layout.shine();
				break;
			}

			layout["texts"].setText(
				"Tiles can produce new units and build other buildings."
				" Once selected, access these abilities"
				" by clicking on the tile again"
				" with the"
			);
			layout["texts"].addText(
				" RIGHT MOUSE BUTTON"
				, FONTSIZESMALL, TEXTCOLORCONTROLS
			);
			layout["texts"].addText(
				"."
				" Please order one of your"
				" "
			);
			layout["texts"].addIcon("ui/ordericon_city", ColorName::UIPRIMARY);
			layout["texts"].addText(
				" City"
				, FONTSIZESMALL, ColorName::UIPRIMARY
			);
			layout["texts"].addText(
				" tiles"
				" to produce a"
				" "
			);
			layout["texts"].addIcon("ui/ordericon_settler", ColorName::UIPRIMARY);
			layout["texts"].addText(
				" Settler"
				, FONTSIZESMALL, ColorName::UIPRIMARY
			);
			layout["texts"].addText(
				" unit."
			);
			layout.settle();
			layout.shine();
		}
		break;
		case State::PRODUCESETTLERNOTOTHER:
		{
			if (!isEnglish())
			{
				layout["texts"].setText(
					_(""
					"Order your"
					" City"
					" tile to produce a"
					" Settler"
					" unit"
					" by selecting it"
					" with the"
					" LEFT MOUSE BUTTON"
					","
					" clicking on it again"
					" with the"
					" RIGHT MOUSE BUTTON"
					","
					" then clicking the upper panel"
					" and finally clicking on the tile"
					" where you want the"
					" Settler"
					" to be created."
					"")
				);
				layout.settle();
				layout.shine();
				break;
			}

			layout["texts"].setText(
				"Order your"
				" "
			);
			layout["texts"].addIcon("ui/ordericon_city", ColorName::UIPRIMARY);
			layout["texts"].addText(
				" City"
				, FONTSIZESMALL, ColorName::UIPRIMARY
			);
			layout["texts"].addText(
				" tile to produce a"
				" "
			);
			layout["texts"].addIcon("ui/ordericon_settler", ColorName::UIPRIMARY);
			layout["texts"].addText(
				" Settler"
				, FONTSIZESMALL, ColorName::UIPRIMARY
			);
			layout["texts"].addText(
				" unit"
				" by selecting it"
				" with the"
			);
			layout["texts"].addText(
				" LEFT MOUSE BUTTON"
				, FONTSIZESMALL, TEXTCOLORCONTROLS
			);
			layout["texts"].addText(
				","
				" clicking on it again"
				" with the"
			);
			layout["texts"].addText(
				" RIGHT MOUSE BUTTON"
				, FONTSIZESMALL, TEXTCOLORCONTROLS
			);
			layout["texts"].addText(
				","
				" then clicking the upper panel"
				" and finally clicking on the tile"
				" where you want the"
				" "
			);
			layout["texts"].addIcon("ui/ordericon_settler", ColorName::UIPRIMARY);
			layout["texts"].addText(
				" Settler"
				, FONTSIZESMALL, ColorName::UIPRIMARY
			);
			layout["texts"].addText(
				" to be created."
			);
			layout.settle();
			layout.shine();
		}
		break;
		case State::STARTACTIONPHASE2:
		{
			if (!isEnglish())
			{
				layout["texts"].setText(
					_(""
					"Great!"
					" Creating new units and tiles costs money and power,"
					" but they are only deducted"
					" when the order is executed"
					" in the"
					" Action phase"
					"."
					" Click the"
					" Ready"
					" button"
					" or press the"
					" SPACE BAR"
					" to start the next"
					" Action phase"
					"."
					"")
				);
				layout.settle();
				layout.shine();
				_commander.getReadyButton().enable();
				break;
			}

			layout["texts"].setText(
				"Great!"
				" Creating new units and tiles costs money and power,"
				" but they are only deducted"
				" when the order is executed"
				" in the"
				" "
			);
			layout["texts"].addIcon("ui/ordericon_action", ColorName::UIPRIMARY);
			layout["texts"].addText(
				" Action phase"
				, FONTSIZESMALL, ColorName::UIPRIMARY
			);
			layout["texts"].addText(
				"."
				" Click the"
			);
			layout["texts"].addText(
				" Ready"
				, FONTSIZESMALL, ColorName::TEXT900
			);
			layout["texts"].addText(
				" button"
				" or press the"
			);
			layout["texts"].addText(
				" SPACE BAR"
				, FONTSIZESMALL, TEXTCOLORCONTROLS
			);
			layout["texts"].addText(
				""
				" to start the next"
				" "
			);
			layout["texts"].addIcon("ui/ordericon_action", ColorName::UIPRIMARY);
			layout["texts"].addText(
				" Action phase"
				, FONTSIZESMALL, ColorName::UIPRIMARY
			);
			layout["texts"].addText(
				"."
			);
			layout.settle();
			layout.shine();
			_commander.getReadyButton().enable();
		}
		break;
		case State::WATCHACTIONPHASE2:
		{
			if (!isEnglish())
			{
				layout["texts"].setText(
					_(""
					"If units or tiles cannot complete their order"
					" in a single"
					" Action phase"
					","
					" they will keep their order and try again later."
					"")
				);
				layout.settle();
				layout.deshine();
				break;
			}

			layout["texts"].setText(
				"If units or tiles cannot complete their order"
				" in a single"
				" "
			);
			layout["texts"].addIcon("ui/ordericon_action", ColorName::UIPRIMARY);
			layout["texts"].addText(
				" Action phase"
				, FONTSIZESMALL, ColorName::UIPRIMARY
			);
			layout["texts"].addText(
				","
				" they will keep their order and try again later."
			);
			layout.settle();
			layout.deshine();
		}
		break;
		case State::STARTWEATHERPHASE:
		{
			if (!isEnglish())
			{
				layout["texts"].setText(
					_(""
					"If units or tiles cannot complete their order"
					" in a single"
					" Action phase"
					","
					" they will keep their order and try again later."
					"")
				);
				layout["texts"].addText(
					"\n"
				);
				layout["texts"].addText(
					_(""
					"After every"
					" Night phase"
					","
					" a new season arrives"
					" that begins with a"
					" Weather phase"
					"."
					" (Click the pause sign"
					" or press the"
					" SPACE BAR"
					" to unpause.)"
					"")
				);
				layout.settle();
				layout.shine();
				_commander._pauseOnce = true;
				break;
			}

			layout["texts"].setText(
				"If units or tiles cannot complete their order"
				" in a single"
				" "
			);
			layout["texts"].addIcon("ui/ordericon_action", ColorName::UIPRIMARY);
			layout["texts"].addText(
				" Action phase"
				, FONTSIZESMALL, ColorName::UIPRIMARY
			);
			layout["texts"].addText(
				","
				" they will keep their order and try again later."
				"\n"
				"After every"
				" "
			);
			layout["texts"].addIcon("ui/ordericon_night", ColorName::UIPRIMARY);
			layout["texts"].addText(
				" Night phase"
				, FONTSIZESMALL, ColorName::UIPRIMARY
			);
			layout["texts"].addText(
				","
				" a new season arrives"
				" that begins with a"
				" "
			);
			layout["texts"].addIcon("ui/ordericon_day", ColorName::UIPRIMARY);
			layout["texts"].addText(
				" Weather phase"
				, FONTSIZESMALL, ColorName::UIPRIMARY
			);
			layout["texts"].addText(
				"."
				" (Click the pause sign"
				" or press the"
			);
			layout["texts"].addText(
				" SPACE BAR"
				, FONTSIZESMALL, TEXTCOLORCONTROLS
			);
			layout["texts"].addText(
				""
				" to unpause.)"
			);
			layout.settle();
			layout.shine();
			_commander._pauseOnce = true;
		}
		break;
		case State::WATCHWEATHERPHASE:
		{
			if (!isEnglish())
			{
				layout["texts"].setText(
					_(""
					"In the"
					" Weather phase"
					","
					" weather effects change and plantlife may grow."
					"")
				);
				layout.settle();
				layout.deshine();
				break;
			}

			layout["texts"].setText(
				"In the"
				" "
			);
			layout["texts"].addIcon("ui/ordericon_day", ColorName::UIPRIMARY);
			layout["texts"].addText(
				" Weather phase"
				, FONTSIZESMALL, ColorName::UIPRIMARY
			);
			layout["texts"].addText(
				","
				" weather effects change and plantlife may grow."
			);
			layout.settle();
			layout.deshine();
		}
		break;
		case State::TAKECONTROL:
		{
			if (!isEnglish())
			{
				layout["texts"].setText(
					_(""
					"In the"
					" Weather phase"
					","
					" weather effects change and plantlife may grow."
					"")
				);
				layout["texts"].addText(
					"\n"
				);
				layout["texts"].addText(
					_(""
					"Then a new"
					" Planning phase"
					" begins as usual."
					" Select your units and give them some new orders!"
					"")
				);
				layout.settle();
				layout.shine();
				break;
			}

			layout["texts"].setText(
				"In the"
				" "
			);
			layout["texts"].addIcon("ui/ordericon_day", ColorName::UIPRIMARY);
			layout["texts"].addText(
				" Weather phase"
				, FONTSIZESMALL, ColorName::UIPRIMARY
			);
			layout["texts"].addText(
				","
				" weather effects change and plantlife may grow."
				"\n"
				"Then a new"
				" "
			);
			layout["texts"].addIcon("ui/ordericon_planning", ColorName::UIPRIMARY);
			layout["texts"].addText(
				" Planning phase"
				, FONTSIZESMALL, ColorName::UIPRIMARY
			);
			layout["texts"].addText(
				" begins as usual."
				" Select your units and give them some new orders!"
			);
			layout.settle();
			layout.shine();
		}
		break;
		case State::INCONTROL:
		{
			if (!isEnglish())
			{
				layout["texts"].setText(
					_(""
					"Once your units reach enemy tiles"
					" they will automatically occupy them."
					" As long as a tile is occupied,"
					" it cannot execute orders, gain power or provide income."
					" Occupy all enemy"
					" City"
					" tiles"
					" in order to defeat your opponent."
					"")
				);
				layout.settle();
				layout.shine();
				break;
			}

			layout["texts"].setText(
				"Once your units reach enemy tiles"
				" they will automatically occupy them."
				" As long as a tile is occupied,"
				" it cannot execute orders, gain power or provide income."
				" Occupy all enemy"
				" "
			);
			layout["texts"].addIcon("ui/ordericon_city", ColorName::UIPRIMARY);
			layout["texts"].addText(
				" City"
				, FONTSIZESMALL, ColorName::UIPRIMARY
			);
			layout["texts"].addText(
				" tiles"
				" in order to defeat your opponent."
			);
			layout.settle();
			layout.shine();
		}
		break;
		case State::WATCHACTIONPHASE3:
		{
			if (!isEnglish())
			{
				layout["texts"].setText(
					_(""
					"Militia"
					" units will attack when moving into enemy units."
					" They will also fire back if attacked"
					" and attack any units that move past them."
					"")
				);
				layout.settle();
				layout.shine();
				break;
			}

			layout["texts"].setText(
				""
			);
			layout["texts"].addIcon("ui/ordericon_militia", ColorName::UIPRIMARY);
			layout["texts"].addText(
				" Militia"
				, FONTSIZESMALL, ColorName::UIPRIMARY
			);
			layout["texts"].addText(
				" units will attack when moving into enemy units."
				" They will also fire back if attacked"
				" and attack any units that move past them."
			);
			layout.settle();
			layout.shine();
		}
		break;
		case State::USESETTLERS:
		{
			if (!isEnglish())
			{
				layout["texts"].setText(
					_(""
					"Settler"
					" units can be used to build tiles"
					" that generate income."
					" Once the"
					" Settler"
					" unit is surrounded by Grass and Forest tiles,"
					" select it, click on it again with the"
					" RIGHT MOUSE BUTTON"
					" and order it to build either a"
					" Farm"
					" tile or a"
					" Town"
					" tile."
					"")
				);
				layout.settle();
				layout.shine();
				break;
			}

			layout["texts"].setText(
				""
			);
			layout["texts"].addIcon("ui/ordericon_settler", ColorName::UIPRIMARY);
			layout["texts"].addText(
				" Settler"
				, FONTSIZESMALL, ColorName::UIPRIMARY
			);
			layout["texts"].addText(
				" units can be used to build tiles"
				" that generate income."
				" Once the"
				" "
			);
			layout["texts"].addIcon("ui/ordericon_settler", ColorName::UIPRIMARY);
			layout["texts"].addText(
				" Settler"
				, FONTSIZESMALL, ColorName::UIPRIMARY
			);
			layout["texts"].addText(
				" unit is surrounded by Grass and Forest tiles,"
				" select it, click on it again with the"
			);
			layout["texts"].addText(
				" RIGHT MOUSE BUTTON"
				, FONTSIZESMALL, TEXTCOLORCONTROLS
			);
			layout["texts"].addText(
				""
				" and order it to build either a"
				" "
			);
			layout["texts"].addIcon("ui/ordericon_farm", ColorName::UIPRIMARY);
			layout["texts"].addText(
				" Farm"
				, FONTSIZESMALL, ColorName::UIPRIMARY
			);
			layout["texts"].addText(
				" tile or a"
				" "
			);
			layout["texts"].addIcon("ui/ordericon_town", ColorName::UIPRIMARY);
			layout["texts"].addText(
				" Town"
				, FONTSIZESMALL, ColorName::UIPRIMARY
			);
			layout["texts"].addText(
				" tile."
			);
			layout.settle();
			layout.shine();
		}
		break;
		case State::INCONTROL2:
		{
			if (!isEnglish())
			{
				layout["texts"].setText(
					_(""
					"Excellent!"
					" Buildings generate income at night,"
					" and you can spend that money on new military units"
					" such as the"
					" Militia"
					" unit."
					"")
				);
				layout.settle();
				layout.shine();
				break;
			}

			layout["texts"].setText(
				"Excellent!"
				" Buildings generate income at night,"
				" and you can spend that money on new military units"
				" such as the"
				" "
			);
			layout["texts"].addIcon("ui/ordericon_militia", ColorName::UIPRIMARY);
			layout["texts"].addText(
				" Militia"
				, FONTSIZESMALL, ColorName::UIPRIMARY
			);
			layout["texts"].addText(
				" unit."
			);
			layout.settle();
			layout.shine();
		}
		break;
		case State::WATCHACTIONPHASE4:
		{
			if (!isEnglish())
			{
				layout["texts"].setText(
					_(""
					"Over the course of the game,"
					" global warming will make the world less habitable."
					" Destroying forests and building new tiles will"
					" accelerate this process."
					"")
				);
				layout.settle();
				layout.shine();
				break;
			}

			layout["texts"].setText(
				"Over the course of the game,"
				" global warming will make the world less habitable."
				" Destroying forests and building new tiles will"
				" accelerate this process."
			);
			layout.settle();
			layout.shine();
		}
		break;
		case State::USEINDUSTRY:
		{
			if (!isEnglish())
			{
				layout["texts"].setText(
					_(""
					"Industry"
					" tiles are a cheap way to generate income"
					" and can produce the powerful"
					" Tank"
					" unit."
					" However,"
					" they also lower the humidity of nearby tiles"
					" and accelerate global warming."
					" To build an"
					" Industry"
					" tile"
					" select one of your"
					" City"
					" tiles,"
					" click on it again with the"
					" RIGHT MOUSE BUTTON"
					","
					" click the left panel"
					" and then click on an adjacent tile."
					"")
				);
				layout.settle();
				layout.shine();
				break;
			}

			layout["texts"].setText(
				""
			);
			layout["texts"].addIcon("ui/ordericon_industry", ColorName::UIPRIMARY);
			layout["texts"].addText(
				" Industry"
				, FONTSIZESMALL, ColorName::UIPRIMARY
			);
			layout["texts"].addText(
				" tiles are a cheap way to generate income"
				" and can produce the powerful"
				" "
			);
			layout["texts"].addIcon("ui/ordericon_tank", ColorName::UIPRIMARY);
			layout["texts"].addText(
				" Tank"
				, FONTSIZESMALL, ColorName::UIPRIMARY
			);
			layout["texts"].addText(
				" unit."
				" However,"
				" they also lower the humidity of nearby tiles"
				" and accelerate global warming."
				" To build an"
				" "
			);
			layout["texts"].addIcon("ui/ordericon_industry", ColorName::UIPRIMARY);
			layout["texts"].addText(
				" Industry"
				, FONTSIZESMALL, ColorName::UIPRIMARY
			);
			layout["texts"].addText(
				" tile"
				" select one of your"
				" "
			);
			layout["texts"].addIcon("ui/ordericon_city", ColorName::UIPRIMARY);
			layout["texts"].addText(
				" City"
				, FONTSIZESMALL, ColorName::UIPRIMARY
			);
			layout["texts"].addText(
				" tiles,"
				" click on it again with the"
			);
			layout["texts"].addText(
				" RIGHT MOUSE BUTTON"
				, FONTSIZESMALL, TEXTCOLORCONTROLS
			);
			layout["texts"].addText(
				","
				" click the left panel"
				" and then click on an adjacent tile."
			);
			layout.settle();
			layout.shine();
		}
		break;
		case State::INCONTROL3:
		{
			if (!isEnglish())
			{
				layout["texts"].setText(
					_(""
					"Great!"
					" Industry"
					" and"
					" Barracks"
					" tiles can be upgraded"
					" to increase their size."
					" The more buildings a tile has,"
					" the stronger a unit produced there will be."
					"")
				);
				layout.settle();
				layout.shine();
				break;
			}

			layout["texts"].setText(
				"Great!"
				""
				" "
			);
			layout["texts"].addIcon("ui/ordericon_industry", ColorName::UIPRIMARY);
			layout["texts"].addText(
				" Industry"
				, FONTSIZESMALL, ColorName::UIPRIMARY
			);
			layout["texts"].addText(
				" and"
				" "
			);
			layout["texts"].addIcon("ui/ordericon_barracks", ColorName::UIPRIMARY);
			layout["texts"].addText(
				" Barracks"
				, FONTSIZESMALL, ColorName::UIPRIMARY
			);
			layout["texts"].addText(
				" tiles can be upgraded"
				" to increase their size."
				" The more buildings a tile has,"
				" the stronger a unit produced there will be."
			);
			layout.settle();
			layout.shine();
		}
		break;
		case State::WATCHACTIONPHASE5:
		{
			if (!isEnglish())
			{
				layout["texts"].setText(
					_(""
					"Tiles need power to produce units"
					" and only gain power at night."
					"")
				);
				layout.settle();
				layout.deshine();
				break;
			}

			layout["texts"].setText(
				"Tiles need power to produce units"
				" and only gain power at night."
			);
			layout.settle();
			layout.deshine();
		}
		break;
		case State::USEBARRACKS:
		{
			if (!isEnglish())
			{
				layout["texts"].setText(
					_(""
					"Tiles need power to produce units"
					" and only gain power at night."
					"")
				);
				layout["texts"].addText(
					"\n"
				);
				layout["texts"].addText(
					_(""
					"Barracks"
					" tiles can produce"
					" Rifleman"
					","
					" Gunner"
					" and"
					" Sapper"
					" units."
					" To build a"
					" Barracks"
					" tile"
					" select one of your"
					" City"
					" tiles,"
					" click on it again with the"
					" RIGHT MOUSE BUTTON"
					","
					" click the lower panel"
					" and then click on an adjacent tile."
					"")
				);
				layout.settle();
				layout.shine();
				break;
			}

			layout["texts"].setText(
				"Tiles need power to produce units"
				" and only gain power at night."
				"\n"
				""
			);
			layout["texts"].addIcon("ui/ordericon_barracks", ColorName::UIPRIMARY);
			layout["texts"].addText(
				" Barracks"
				, FONTSIZESMALL, ColorName::UIPRIMARY
			);
			layout["texts"].addText(
				" tiles can produce"
				" "
			);
			layout["texts"].addIcon("ui/ordericon_rifleman", ColorName::UIPRIMARY);
			layout["texts"].addText(
				" Rifleman"
				, FONTSIZESMALL, ColorName::UIPRIMARY
			);
			layout["texts"].addText(
				","
				" "
			);
			layout["texts"].addIcon("ui/ordericon_gunner", ColorName::UIPRIMARY);
			layout["texts"].addText(
				" Gunner"
				, FONTSIZESMALL, ColorName::UIPRIMARY
			);
			layout["texts"].addText(
				" and"
				" "
			);
			layout["texts"].addIcon("ui/ordericon_sapper", ColorName::UIPRIMARY);
			layout["texts"].addText(
				" Sapper"
				, FONTSIZESMALL, ColorName::UIPRIMARY
			);
			layout["texts"].addText(
				" units."
				" To build a"
				" "
			);
			layout["texts"].addIcon("ui/ordericon_barracks", ColorName::UIPRIMARY);
			layout["texts"].addText(
				" Barracks"
				, FONTSIZESMALL, ColorName::UIPRIMARY
			);
			layout["texts"].addText(
				" tile"
				" select one of your"
				" "
			);
			layout["texts"].addIcon("ui/ordericon_city", ColorName::UIPRIMARY);
			layout["texts"].addText(
				" City"
				, FONTSIZESMALL, ColorName::UIPRIMARY
			);
			layout["texts"].addText(
				" tiles,"
				" click on it again with the"
			);
			layout["texts"].addText(
				" RIGHT MOUSE BUTTON"
				, FONTSIZESMALL, TEXTCOLORCONTROLS
			);
			layout["texts"].addText(
				","
				" click the lower panel"
				" and then click on an adjacent tile."
			);
			layout.settle();
			layout.shine();
		}
		break;
		case State::INCONTROL4:
		{
			if (!isEnglish())
			{
				layout["texts"].setText(
					_(""
					"That's it!"
					" Remember: you must occupy or destroy all enemy"
					" City"
					" tiles"
					" in order to defeat your opponent."
					" Good luck!"
					"")
				);
				layout.settle();
				layout.depower();
				layout["texts"].power();
				layout.deshine();
				break;
			}

			layout["texts"].setText(
				"That's it!"
				" Remember: you must occupy or destroy all enemy"
				" "
			);
			layout["texts"].addIcon("ui/ordericon_city", ColorName::UIPRIMARY);
			layout["texts"].addText(
				" City"
				, FONTSIZESMALL, ColorName::UIPRIMARY
			);
			layout["texts"].addText(
				" tiles"
				" in order to defeat your opponent."
				" Good luck!"
			);
			layout.settle();
			layout.depower();
			layout["texts"].power();
			layout.deshine();
		}
		break;
		case State::VICTORIOUS:
		{
			if (!isEnglish())
			{
				layout["texts"].setText(
					_(""
					"Victory!"
					" After every game you win,"
					" you are rewarded with a score"
					" equal to the number of grass tiles left on the map."
					" If the score you obtain in a game is higher"
					" than the amount of rating points you have,"
					" your rating will rise!"
					"")
				);
				layout.settle();
				layout.power();
				layout.shine();
				break;
			}

			layout["texts"].setText(
				"Victory!"
				" After every game you win,"
				" you are rewarded with a score"
				" equal to the number of grass tiles left on the map."
				" If the score you obtain in a game is higher"
				" than the amount of rating points you have,"
				" your rating will rise!"
			);
			layout.settle();
			layout.power();
			layout.shine();
		}
		break;
		case State::DEFEATED:
		{
			if (!isEnglish())
			{
				layout["texts"].setText(
					_(""
					"Try again!"
					" Remember: you must occupy or destroy all enemy"
					" City"
					" tiles"
					" in order to defeat your opponent."
					"")
				);
				layout.settle();
				layout.power();
				layout.shine();
				break;
			}

			layout["texts"].setText(
				"Try again!"
				" Remember: you must occupy or destroy all enemy"
				" "
			);
			layout["texts"].addIcon("ui/ordericon_city", ColorName::UIPRIMARY);
			layout["texts"].addText(
				" City"
				, FONTSIZESMALL, ColorName::UIPRIMARY
			);
			layout["texts"].addText(
				" tiles"
				" in order to defeat your opponent."
			);
			layout.settle();
			layout.power();
			layout.shine();
		}
		break;
	}
}
