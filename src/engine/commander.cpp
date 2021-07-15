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
 * Can Ur (canur11@gmail.com)
 */
#include "commander.hpp"
#include "source.hpp"

#include "loop.hpp"
#include "camera.hpp"
#include "surface.hpp"
#include "animationgroup.hpp"
#include "input.hpp"
#include "targetcursor.hpp"
#include "selectorcontext.hpp"
#include "ordercontext.hpp"
#include "move.hpp"
#include "horizontallayout.hpp"
#include "scrollablelayout.hpp"
#include "stackedlayout.hpp"
#include "textfield.hpp"
#include "dynamictextfield.hpp"
#include "frame.hpp"
#include "image.hpp"
#include "filler.hpp"
#include "padding.hpp"
#include "game.hpp"
#include "settings.hpp"
#include "paint.hpp"
#include "colorname.hpp"
#include "pathingflowfield.hpp"
#include "parseerror.hpp"
#include "mixer.hpp"


Commander::Commander(Settings& settings, Game& game,
		const Player& player,
		const std::string& rulesetname) :
	Observer(settings, game, player, rulesetname),
	_money(0),
	_displaymoney(0),
	_displaymoneydelay(0),
	_targetcursor(nullptr),
	_selectorcontext(nullptr),
	_ordercontext(nullptr)
{}

Commander::~Commander()
{}

void Commander::postbuild()
{
	Observer::postbuild();

	InterfaceElement& wallet = getWallet();
	wallet["text"].setText("0");

	_layout["right"]["readybutton"].fixWidth(
		_layout["right"]["orderlist"].width());

	_layout["right"]["orderlist"]["old"].fixHeight();
	_layout["right"]["orderlist"]["old"].content().fixHeight();

	numberOrderList();
}

InterfaceElement& Commander::getOrderlistOld()
{
	return _layout["right"]["orderlist"]["old"];
}

InterfaceElement& Commander::getOrderlistNew()
{
	return _layout["right"]["orderlist"]["new"];
}

InterfaceElement& Commander::getInitiativeSequencer()
{
	return _layout["right"]["orderlist"]["sequence"];
}

InterfaceElement& Commander::getReadyButton()
{
	return _layout["right"]["readybutton"];
}

std::unique_ptr<InterfaceElement> Commander::makeReadyButton()
{
	std::unique_ptr<InterfaceElement> element(new Frame("ui/frame_button_9"));

	(*element).put(new HorizontalLayout());
	(*element).add("text", new TextField(
		_("Ready"),
		_settings.getFontSizeReadyButton()));
	(*element)["text"].align(HorizontalAlignment::CENTER);
	(*element)["text"].setMarginLeft(11 * InterfaceElement::scale());
	(*element).add("filler", new HorizontalFiller());
	(*element).add("box", new Image("ui/checkbox"));
	(*element)["box"].setColor(0,
		Paint::blend(ColorName::UIACCENT, ColorName::SHINEBLEND));
	(*element)["box"].setColor(1,
		ColorName::UIACCENT);
	(*element)["box"].setColor(2,
		Paint::blend(ColorName::UIACCENT, ColorName::SHADEBLEND));
	(*element)["box"].setColor(3,
		ColorName::FRAMESTONE);
	(*element)["box"].setColor(4,
		Paint::blend(ColorName::FRAMESTONE, ColorName::SHADEBLEND));
	(*element)["box"].setColor(5,
		Color::transparent());
	(*element)["box"].setColor(6,
		Color::transparent());
	(*element)["box"].setTag("Empty");
	(*element).setPadding(6 * InterfaceElement::scale());
	(*element).setMargin(5 * InterfaceElement::scale());
	(*element).align(VerticalAlignment::MIDDLE);
	(*element).settle();
	(*element).makeClickable();

	return element;
}

std::unique_ptr<InterfaceElement> Commander::makeInitiativeSequencer()
{
	std::unique_ptr<InterfaceElement> element(new Frame("ui/frame_window_9"));
	InterfaceElement& it = *element;
	it.put(new HorizontalLayout());
	it.setMarginVertical(5 * InterfaceElement::scale());
	it.setBasicColor(0, Paint::alpha(ColorName::FRAME200, 128));
	it.setBasicColor(1, Color::alpha(Paint::blend(ColorName::FRAME200, ColorName::SHADEBLEND, 0.5f), 128));
	it.setBasicColor(2, Color::alpha(Paint::blend(ColorName::FRAME600, ColorName::SHINEBLEND), 128));
	it.setBasicColor(3, Color::alpha(Paint::blend(ColorName::FRAME600, ColorName::SHADEBLEND), 128));
	{
		int seq = 1;
		Player player = Player::NONE;
		std::string str = std::to_string(seq);
		it.add(str, makeTurnSequenceElement(seq, player));
	}
	it.setPaddingHorizontal(5 * InterfaceElement::scale());
	it.setPaddingVertical(5 * InterfaceElement::scale());
	return element;
}

std::unique_ptr<InterfaceElement> Commander::makeOrderList()
{
	std::unique_ptr<InterfaceElement> element(new VerticalLayout());

	(*element).add("sequence", makeInitiativeSequencer());
	(*element).add("old", makeOrderListOld());
	(*element).add("new", makeOrderListNew());
	(*element)["old"].add("filler", makeEmptyOrderListElement());
	(*element).settleWidth();
	(*element).fixWidth();
	(*element)["sequence"].fixWidth((*element)["new"].width());
	(*element)["old"].fixWidth();
	(*element)["new"].fixWidth();
	for (int i = 1; i <= _bible.newOrderLimit(); i++)
	{
		std::string str = std::to_string(i);
		(*element)["new"][str].disable();
	}
	(*element)["old"].settleHeight();
	(*element)["old"].remove("filler");

	return element;
}

std::unique_ptr<InterfaceElement> Commander::makeOrderListOld()
{
	std::unique_ptr<InterfaceElement> element(new Frame("ui/frame_window_9"));
	InterfaceElement& it = *element;
	it.put(new ScrollableLayout(true));
	it.align(VerticalAlignment::BOTTOM);
	it.setMarginVertical(5 * InterfaceElement::scale());
	it.setBasicColor(0, Paint::alpha(ColorName::FRAME200, 128));
	it.setBasicColor(1, Color::alpha(
		Paint::blend(ColorName::FRAME200, ColorName::SHADEBLEND, 0.5f),
		128));
	it.setBasicColor(2, Color::alpha(
		Paint::blend(ColorName::FRAME600, ColorName::SHINEBLEND),
		128));
	it.setBasicColor(3, Color::alpha(
		Paint::blend(ColorName::FRAME600, ColorName::SHADEBLEND),
		128));
	// TODO this is because scrollablelayout doesn't work with margins
	// TODO but now this is also because scrollablelayout has a scrollbar
	it.setPaddingHorizontal(3 * InterfaceElement::scale());
	it.setPaddingVertical(4 * InterfaceElement::scale());
	it.align(HorizontalAlignment::CENTER);
	return element;
}

std::unique_ptr<InterfaceElement> Commander::makeOrderListNew()
{
	std::unique_ptr<InterfaceElement> element(new Frame("ui/frame_window_9"));
	InterfaceElement& it = *element;
	it.put(new VerticalLayout());
	it.setMarginVertical(5 * InterfaceElement::scale());
	it.setPowerColor(0, ColorName::UIPRIMARY);
	it.setPowerColor(1,
		Paint::blend(ColorName::UIPRIMARY, ColorName::SHADEBLEND, 0.5f));
	it.setPowerColor(2,
		Paint::blend(ColorName::FRAME800, ColorName::SHINEBLEND));
	it.setPowerColor(3,
		Paint::blend(ColorName::FRAME800, ColorName::SHADEBLEND));
	for (int i = 1; i <= _bible.newOrderLimit(); i++)
	{
		std::string str = std::to_string(i);
		it.add(str, makeEmptyOrderListElement());
	}
	it.settleWidth();
	it.settleHeight();
	it.setPaddingHorizontal(8 * InterfaceElement::scale());
	it.setPaddingVertical(5 * InterfaceElement::scale());
	return element;
}

void Commander::buildInfoBar()
{
	const int FONTSIZESMALL = _settings.getFontSizeTutorial();

	_layout["center"]["top"].fixHeight();

	int statusbarwidth = _layout["center"]["top"]["statusbar"].width();

	std::unique_ptr<InterfaceElement> element(new Frame("ui/frame_window_9"));

	(*element).put(new StackedLayout());
	(*element).add("filler", new Filler());
	(*element).add("texts", new DynamicTextField(FONTSIZESMALL));
	(*element).align(HorizontalAlignment::LEFT);
	(*element).align(VerticalAlignment::MIDDLE);
	(*element).setPaddingHorizontal(6 * InterfaceElement::scale());
	(*element).setPaddingVertical(4 * InterfaceElement::scale());
	(*element).setMargin(5 * InterfaceElement::scale());
	(*element).settle();
	(*element).fixWidth(statusbarwidth);

	_layout["center"]["top"].replace("statusbar", std::move(element));
	_layout.settle();
}

void Commander::prepareAnimationGroup()
{
	Observer::prepareAnimationGroup();

	if (auto group = _animations.lock())
	{
		// We let the Tiles determine the money animations.
		group->money = _money;
	}
}

void Commander::handleInitiative(const Change& change)
{
	InterfaceElement& sequence = getInitiativeSequencer();

	int x = change.initiative;

	int width = sequence.width();
	if (x <= 1)
	{
		sequence.reset();
	}

	std::string index = std::to_string(x);
	sequence.add(index, makeTurnSequenceElement(x, change.player));

	sequence.fixWidth(width);
	sequence.settleHeight();
	sequence.place(sequence.topleft());
}

void Commander::handleWallet(const Change& change)
{
	_money += change.money;

	// Animate the money changes.
	switch (change.type)
	{
		case Change::Type::FUNDS:
		case Change::Type::EXPENDITURE:
		{
			if (auto group = _animations.lock())
			{
				int money = (int) change.money;

				// We want to update group->money inside an Animation, but we
				// do not want the lambda function to keep group alive.
				std::weak_ptr<AnimationGroup> weak = group;
				addAnimation(
					Animation(group, [money, weak](float /**/){
						if (auto g = weak.lock())
						{
							g->money += money;
						}
				}, 0, group->delay));
			}
		}
		break;

		// case Change::Type::INCOME:
		// {
		//   We let the Tiles determine the INCOME animations.
		// }
		// break;

		default:
		break;
	}
}

void Commander::handleOrder(const Change& change)
{
	switch (change.type)
	{
		case Change::Type::PHASE:
		{
			if (change.phase == Phase::ACTION)
			{
				unlistNewOrders();
			}
			else if (change.phase == Phase::RESTING)
			{
				tagOrdersGuiltyUntilProvenInnocent();
				_needunlisting = true;
			}
		}
		break;

		case Change::Type::FINISHED:
		case Change::Type::DISCARDED:
		break;

		case Change::Type::POSTPONED:
		case Change::Type::UNFINISHED:
		{
			if (_phase == Phase::RESTING)
			{
				keepOrder(change.subject, change.order);
			}
		}
		break;

		case Change::Type::ORDERED:
		{
			listOrder(change.order);
		}
		break;

		default:
		break;
	}
}

void Commander::checkIdleUnits()
{
	if (_hideidleanimations) return;

	// If there are any ground units without orders, give them question marks.
	for (Square& square : _level)
	{
		if (square.ground() && square.ground().owner == _player)
		{
			Descriptor desc = Descriptor::ground(square.position());
			if (!hasOrder(desc))
			{
				square.ground().idle();
			}
		}
	}
}

void Commander::uncheckIdleUnits()
{
	// Prevent question marks.
	for (Square& square : _level)
	{
		if (square.ground() && square.ground().owner == _player)
		{
			square.ground().unidle();
		}
	}
}

void Commander::unlistDiscardedOrders()
{
	std::vector<std::unique_ptr<InterfaceElement>> keepers;
	{
		InterfaceElement& oldlist = getOrderlistOld();
		for (size_t i = 0; i < oldlist.size(); i++)
		{
			std::string name = oldlist.name(i);
			if (name[0] >= '0' && name[0] <= '9') continue;

			if (oldlist[name]["data"].text() != "$discarded")
			{
				keepers.emplace_back(oldlist.replace(name, new Filler()));
			}
		}
	}
	{
		InterfaceElement& newlist = getOrderlistNew();
		for (size_t i = 0; i < newlist.size(); i++)
		{
			std::string name = newlist.name(i);
			if (name[0] >= '0' && name[0] <= '9') continue;

			if (newlist[name]["data"].text() != "$discarded")
			{
				keepers.emplace_back(newlist.replace(name, new Filler()));
			}
		}
	}

	getOrderlistOld().reset();
	getOrderlistNew().reset();

	{
		InterfaceElement& oldlist = getOrderlistOld();
		for (size_t i = 0; i < keepers.size(); i++)
		{
			InterfaceElement& element = *keepers[i];
			element.enable();

			Order order = parseOrderData(element["data"].text());

			// In the future, we might want/have to change ordericons here.

			element["item"]["remove"].setTag("Box");

			std::stringstream strm;
			strm << order.subject;
			std::string index = strm.str();
			oldlist.add(index, std::move(keepers[i]));
		}
	}

	{
		InterfaceElement& newlist = getOrderlistNew();
		for (int i = 1; i <= _bible.newOrderLimit(); i++)
		{
			std::string str = std::to_string(i);
			newlist.add(str, makeEmptyOrderListElement());
		}
	}

	numberOrderList();
	getOrderlistOld().settle();
	getOrderlistNew().settle();
}

void Commander::numberOrderList()
{
	InterfaceElement& oldlist = getOrderlistOld();
	InterfaceElement& newlist = getOrderlistNew();

	int number = 1;
	for (size_t i = 0; i < oldlist.size(); i++)
	{
		if (!oldlist[oldlist.name(i)].enabled()) continue;
		oldlist[oldlist.name(i)]["backing"]["number"].setText(
			std::to_string(number++));
	}

	for (size_t i = 0; i < newlist.size(); i++)
	{
		newlist[newlist.name(i)]["backing"]["number"].setText(
			std::to_string(number++));
	}
}

bool Commander::controlSelector()
{
	Input* input = Input::get();
	if (input->wasKeyPressed(SDL_SCANCODE_LMB))
	{
		if (!canGiveOrders()) return false;

		if (_hoversquare == nullptr || _hoversquare->edge()) return false;

		// Deselect the current unit (this resets _selectsquare to null).
		deselectUnitOrTile();

		_selectsquare = _hoversquare;
		Cell index = _selectsquare->cell();

		switch (_settings.selectormode.value())
		{
			case SelectorMode::CONTEXT:
			{
				_selectorcontext.reset(
					new SelectorContext(_board, _settings, index, _player));
				if (_selectorcontext->empty())
				{
					controlSelectorContext();
				}
			}
			break;

			case SelectorMode::FIGURE:
			{
				bool tile   = (_selectsquare->tile().owner   == _player
					&& _bible.tileControllable(_selectsquare->tile().type));
				bool ground = (_selectsquare->ground().owner == _player);
				bool air    = (_selectsquare->air().owner    == _player);

				// The ordering of the six if-statements below is intentional.
				// The air unit has a specific position on the screen that other figures avoid,
				// so if we might be hovering it we are most likely hovering it.
				if (air && _selectsquare->air().hovered())
				{
					_selector = Descriptor::air(_selectsquare->position());
					selectUnitOrTile();
				}
				// Ground units are smaller so take precedence over buildings.
				else if (ground && _selectsquare->ground().hovered())
				{
					_selector = Descriptor::ground(_selectsquare->position());
					selectUnitOrTile();
				}
				// Tiles have the largest figures so the are easier to hover.
				else if (tile && _selectsquare->tile().hovered())
				{
					_selector = Descriptor::tile(_selectsquare->position());
					selectUnitOrTile();
				}
				// Unless we're hovering a building specifically, we usually want ground units.
				else if (ground)
				{
					_selector = Descriptor::ground(_selectsquare->position());
					selectUnitOrTile();
				}
				// Air units have precedence over tiles.
				else if (air)
				{
					_selector = Descriptor::air(_selectsquare->position());
					selectUnitOrTile();
				}
				// We only want to auto-select tiles if they are otherwise empty.
				else if (tile)
				{
					_selector = Descriptor::tile(_selectsquare->position());
					selectUnitOrTile();
				}
				else
				{
					_selector = Descriptor();
				}
			}
			break;
		}

		if (_selector.type == Descriptor::Type::NONE)
		{
			_selectsquare = nullptr;
		}

		return true;
	}

	return false;
}

bool Commander::controlSelectorContext()
{
	if (_selectorcontext == nullptr) return false;

	Input* input = Input::get();
	if (input->wasKeyPressed(SDL_SCANCODE_LMB)
		|| input->wasKeyLongReleased(SDL_SCANCODE_LMB)
		|| (input->wasKeyReleased(SDL_SCANCODE_LMB) && _selectorcontext->moved()))
	{
		_selector = _selectorcontext->resolve();
		_selectsquare = _level.cell(_selector.position);
		_selectorcontext.reset(nullptr);

		selectUnitOrTile();
	}

	return true;
}

bool Commander::quitSelectorContext()
{
	if (_selectorcontext)
	{
		_selectorcontext.reset(nullptr);
		return true;
	}
	else return false;
}

bool Commander::quitOrderContext()
{
	if (_ordercontext)
	{
		_ordercontext.reset(nullptr);
		return true;
	}
	else return false;
}

bool Commander::quitTargetCursor()
{
	if (_targetcursor)
	{
		_targetcursor.reset(nullptr);
		resetTargetSpotlight();
		return true;
	}
	else return false;
}

void Commander::clearGuides()
{
	if (_selectorcontext)
	{
		_selectorcontext.reset(nullptr);
	}

	if (_ordercontext)
	{
		_ordercontext.reset(nullptr);
	}

	if (_targetcursor)
	{
		_targetcursor.reset(nullptr);
		resetTargetSpotlight();
	}

	_guides.clear();
}

void Commander::selectActiveOrder(const Descriptor& subject)
{
	std::stringstream strm;
	strm << subject;
	std::string index = strm.str();

	{
		InterfaceElement& newlist = getOrderlistNew();
		if (newlist.contains(index))
		{
			_activeOrderIndexNew = index;
			newlist[index].power();
		}
		else
		{
			_activeOrderIndexNew = "";
		}
	}

	{
		InterfaceElement& oldlist = getOrderlistOld();
		if (oldlist.contains(index))
		{
			_activeOrderIndexOld = index;
			oldlist[index].power();
		}
		else
		{
			_activeOrderIndexOld = "";
		}
	}
}

void Commander::deselectActiveOrder()
{
	if (!_activeOrderIndexOld.empty())
	{
		InterfaceElement& oldlist = getOrderlistOld();
		if (oldlist.contains(_activeOrderIndexOld))
		{
			oldlist[_activeOrderIndexOld].depower();
		}
	}

	if (!_activeOrderIndexNew.empty())
	{
		InterfaceElement& newlist = getOrderlistNew();
		if (newlist.contains(_activeOrderIndexNew))
		{
			newlist[_activeOrderIndexNew].depower();
		}
	}
}

void Commander::disableActiveOrder()
{
	if (!_activeOrderIndexOld.empty())
	{
		InterfaceElement& oldlist = getOrderlistOld();
		if (oldlist.contains(_activeOrderIndexOld))
		{
			oldlist[_activeOrderIndexOld].depower();
			oldlist[_activeOrderIndexOld].disable();
		}
	}

	if (!_activeOrderIndexNew.empty())
	{
		InterfaceElement& newlist = getOrderlistNew();
		if (newlist.contains(_activeOrderIndexNew))
		{
			newlist[_activeOrderIndexNew].depower();
			newlist[_activeOrderIndexNew].disable();
		}
	}
}

size_t Commander::countNewOrders()
{
	InterfaceElement& newlist = getOrderlistNew();
	int countUnused = 0;
	for (int i = 1; i <= _bible.newOrderLimit(); i++)
	{
		std::string index = std::to_string(i);
		if (newlist.contains(index)
			&& !newlist[index].enabled())
		{
			countUnused++;
		}
	}
	return _bible.newOrderLimit() - countUnused;
}

bool Commander::canGiveOrders()
{
	return (_phase == Phase::PLANNING && !_defeated && !_gameover);
}

void Commander::touchReadyButton()
{
	InterfaceElement& readybutton = getReadyButton();
	if (canGiveOrders())
	{
		readybutton.enable();
		readybutton.depower();
		readybutton["box"].setTag("Empty");
		checkIdleUnits();
	}
	else
	{
		readybutton.disable();
		readybutton.depower();
		readybutton["box"].setTag("Empty");
		uncheckIdleUnits();
	}
}

bool Commander::controlUnitOrders()
{
	if (_selectsquare == nullptr) return false;
	if (!_selectsquare->unit(_selector.type)) return false;
	if (!canGiveOrders()) return false;

	Cell index = _selectsquare ? _selectsquare->cell() : Cell::undefined();

	Input* input = Input::get();
	if (input->wasKeyPressed(SDL_SCANCODE_RMB))
	{
		if (_hoversquare == nullptr || _hoversquare->edge()) return false;

		bool hasOld = hasOldOrder(_selector);
		bool hasNew = hasNewOrder(_selector);
		bool full = countNewOrders() >= _bible.newOrderLimit();
		bool can = hasNew || !full;

		if (_hoversquare->position() == _selectsquare->position())
		{
			_ordercontext.reset(new OrderContext(_bible, _board,
				_skinner, _settings, _season,
				index, _selector, _player,
				hasOld, hasNew, can));
		}
		else if (_bible.unitCanMove(_selectsquare->unit(_selector.type).type))
		{
			if (can) giveMoveOrder();
		}

		if (!hasNew && full)
		{
			blinkOrderListFull();
		}

		return true;
	}

	return false;
}

bool Commander::controlTileOrders()
{
	if (_selectsquare == nullptr) return false;
	if (_selector.type != Descriptor::Type::TILE) return false;
	if (!_selectsquare->tile()) return false;
	if (!canGiveOrders()) return false;

	Cell index = _selectsquare ? _selectsquare->cell() : Cell::undefined();

	Input* input = Input::get();
	if (input->wasKeyPressed(SDL_SCANCODE_RMB))
	{
		if (_hoversquare == nullptr || _hoversquare->edge()) return false;

		bool hasOld = hasOldOrder(_selector);
		bool hasNew = hasNewOrder(_selector);
		bool full = countNewOrders() >= _bible.newOrderLimit();
		bool can = hasNew || !full;

		if (_hoversquare->position() == _selectsquare->position())
		{
			_ordercontext.reset(new OrderContext(_bible, _board,
				_skinner, _settings, _season,
				index, _selector, _player,
				hasOld, hasNew, can));
		}

		if (!hasNew && full)
		{
			blinkOrderListFull();
		}
	}

	return false;
}

bool Commander::controlOrderContext()
{
	if (_ordercontext == nullptr) return false;

	Input* input = Input::get();
	if (input->wasKeyPressed(SDL_SCANCODE_LMB)
		|| input->wasKeyPressed(SDL_SCANCODE_RMB)
		|| input->wasKeyLongReleased(SDL_SCANCODE_RMB)
		|| (input->wasKeyReleased(SDL_SCANCODE_RMB) && _ordercontext->moved()))
	{
		Order order = _ordercontext->resolve();
		_ordercontext.reset(nullptr);

		Cell index = _selectsquare ? _selectsquare->cell() : Cell::undefined();

		switch (order.type)
		{
			case Order::Type::GUARD:
			case Order::Type::FOCUS:
			case Order::Type::LOCKDOWN:
			case Order::Type::SHELL:
			case Order::Type::BOMBARD:
			case Order::Type::EXPAND:
			{
				_targetcursor.reset(new TargetCursor(_bible, _board, index,
					order, spritenameOrderIcon(order)));
				setTargetSpotlight(_targetcursor->foci());
			}
			break;
			case Order::Type::BOMB:
			case Order::Type::CAPTURE:
			case Order::Type::SHAPE:
			case Order::Type::SETTLE:
			case Order::Type::UPGRADE:
			case Order::Type::CULTIVATE:
			{
				giveOrder(order);
				visualizeOrder(order);
			}
			break;
			case Order::Type::PRODUCE:
			{
				if (_bible.tileProduceRangeMax() > 0)
				{
					_targetcursor.reset(new TargetCursor(_bible, _board, index,
						order, spritenameOrderIcon(order)))	;
					setTargetSpotlight(_targetcursor->foci());
				}
				else
				{
					giveOrder(order);
					visualizeOrder(order);
				}
			}
			break;
			case Order::Type::HALT:
			{
				giveHaltOrderOrRevoke();
			}
			break;
			case Order::Type::NONE:
			{
				visualizeOrder(findOrder(_selector));
			}
			break;
			case Order::Type::MOVE:
			break;
		}
	}

	return true;
}

bool Commander::controlTargetCursor()
{
	if (_targetcursor == nullptr) return false;

	Input* input = Input::get();
	if (input->wasKeyPressed(SDL_SCANCODE_LMB)
		|| input->wasKeyPressed(SDL_SCANCODE_RMB))
	{
		if (_targetcursor->valid())
		{
			Order order = _targetcursor->resolve();
			_targetcursor.reset(nullptr);
			resetTargetSpotlight();

			giveOrder(order);
			visualizeOrder(order);
		}
		else
		{
			_targetcursor.reset(nullptr);
			resetTargetSpotlight();
			visualizeOrder(findOrder(_selector));
		}
	}

	return true;
}

void Commander::blinkWallet()
{
	std::shared_ptr<AnimationGroup> group = _animations.lock();

	addAnimation(Animation(group, [this](float) {

		InterfaceElement& wallet = getWallet();
		wallet.power();

	}, 0, (group ? group->delay : 0) + 0.500f + 0.000f));

	addAnimation(Animation(group, [this](float) {

		InterfaceElement& wallet = getWallet();
		wallet.depower();

	}, 0, (group ? group->delay : 0) + 0.500f + 0.150f));

	addAnimation(Animation(group, [this](float) {

		InterfaceElement& wallet = getWallet();
		wallet.power();

	}, 0, (group ? group->delay : 0) + 0.500f + 0.300f));

	addAnimation(Animation(group, [this](float) {

		InterfaceElement& wallet = getWallet();
		wallet.depower();
		wallet["text"].power();

	}, 0, (group ? group->delay : 0) + 0.500f + 0.450f));
}

void Commander::blinkOrderListFull()
{
	{
		InterfaceElement& newlist = getOrderlistNew();
		for (size_t i = 0; i < newlist.size(); i++)
		{
			newlist[newlist.name(i)].power();
		}
		newlist.power();
	}

	addAnimation(Animation(nullptr, [this](float) {

		InterfaceElement& newlist = getOrderlistNew();
		for (size_t i = 0; i < newlist.size(); i++)
		{
			newlist[newlist.name(i)].depower();
		}
		newlist.depower();

	}, 0, 0.150f));

	addAnimation(Animation(nullptr, [this](float) {

		InterfaceElement& newlist = getOrderlistNew();
		for (size_t i = 0; i < newlist.size(); i++)
		{
			newlist[newlist.name(i)].power();
		}
		newlist.power();

	}, 0, 0.300f));

	addAnimation(Animation(nullptr, [this](float) {

		InterfaceElement& newlist = getOrderlistNew();
		for (size_t i = 0; i < newlist.size(); i++)
		{
			newlist[newlist.name(i)].depower();
		}
		newlist.depower();

	}, 0, 0.450f));
}

void Commander::controlOrderList()
{
	{
		InterfaceElement& newlist = getOrderlistNew();
		for (size_t i = 0; i < newlist.size(); i++)
		{
			std::string name = newlist.name(i);
			newlist[name]["item"]["remove"].enableIf(canGiveOrders());
		}
	}
	{
		InterfaceElement& oldlist = getOrderlistOld();
		for (size_t i = 0; i < oldlist.size(); i++)
		{
			std::string name = oldlist.name(i);
			oldlist[name]["item"]["remove"].enableIf(canGiveOrders());
		}
	}

	if (_phase == Phase::RESTING && _needunlisting)
	{
		unlistDiscardedOrders();
		_needunlisting = false;
	}

	if (!canGiveOrders()) return;

	Descriptor click;
	Descriptor doubleclick;
	Descriptor hold;
	Descriptor stop;
	Descriptor revert;
	std::string indexOld = "";
	std::string indexNew = "";

	{
		InterfaceElement& newlist = getOrderlistNew();
		for (size_t i = 0; i < newlist.size(); i++)
		{
			std::string name = newlist.name(i);
			if (newlist[name]["item"]["remove"].clicked())
			{
				revert = parseDescriptorData(name);
				indexNew = name;
				break;
			}
			else if (newlist[name].doubleclicked())
			{
				click = parseDescriptorData(name);
				doubleclick = parseDescriptorData(name);
				indexNew = name;
				break;
			}
			else if (newlist[name].clicked())
			{
				click = parseDescriptorData(name);
				indexNew = name;
				break;
			}
			else if (newlist[name]["item"].held())
			{
				hold = parseDescriptorData(name);
				indexNew = name;
				break;
			}
		}
	}
	{
		InterfaceElement& oldlist = getOrderlistOld();
		for (size_t i = 0; i < oldlist.size(); i++)
		{
			std::string name = oldlist.name(i);
			if (oldlist[name]["item"]["remove"].clicked())
			{
				stop = parseDescriptorData(name);
				indexOld = name;
				break;
			}
			else if (oldlist[name].doubleclicked())
			{
				click = parseDescriptorData(name);
				doubleclick = parseDescriptorData(name);
				indexOld = name;
				break;
			}
			else if (oldlist[name].clicked())
			{
				click = parseDescriptorData(name);
				indexOld = name;
				break;
			}
			else if (oldlist[name]["item"].held())
			{
				hold = parseDescriptorData(name);
				indexOld = name;
				break;
			}
		}
	}
	if (indexNew.empty() && !indexOld.empty()
		&& getOrderlistNew().contains(indexOld))
	{
		indexNew = indexOld;
	}
	else if (indexOld.empty() && !indexNew.empty()
		&& getOrderlistOld().contains(indexNew))
	{
		indexOld = indexNew;
	}

	if (click.type != Descriptor::Type::NONE)
	{
		deselectUnitOrTile();
		_selectsquare = _level.cell(click.position);
		_selector = click;
		_activeOrderIndexNew = indexNew;
		_activeOrderIndexOld = indexOld;
		selectUnitOrTile();
		if (_selector.type == Descriptor::Type::NONE)
		{
			LOGW << "invalid order in order list; new: " << indexNew
				<< "; old: " << indexOld;
		}
		else if (doubleclick == _selector)
		{
			panCamera();
		}
	}
	else if (hold.type != Descriptor::Type::NONE)
	{
		InterfaceElement& newlist = getOrderlistNew();
		InterfaceElement& oldlist = getOrderlistOld();
		bool has = hasNewOrder(hold);
		bool full = countNewOrders() >= _bible.newOrderLimit();
		bool can = canGiveOrders() && (has || !full);

		if (!indexNew.empty() && newlist.overed() && can)
		{
			size_t newi = newlist.size();
			size_t over = newlist.size();
			std::string indexOver;
			for (size_t i = 0; i < newlist.size(); i++)
			{
				std::string name = newlist.name(i);
				if (name == indexNew)
				{
					newi = i;
				}
				else if (over >= newlist.size() && newlist[name].overed())
				{
					over = i;
					indexOver = name;
				}
			}
			if (over < newlist.size())
			{
				std::vector<std::string> names;
				std::vector<std::unique_ptr<InterfaceElement>> elements;
				names.emplace_back(indexNew);
				for (size_t i = 0; i < newlist.size(); i++)
				{
					std::string name = newlist.name(i);
					if (name == indexNew) continue;
					if (i == over && newi < over) continue;
					if (i < over) continue;
					names.emplace_back(name);
				}
				for (const std::string& name : names)
				{
					elements.emplace_back(newlist.remove(name));
				}
				for (size_t i = 0; i < names.size(); i++)
				{
					newlist.add(names[i], std::move(elements[i]));
				}
				newlist.place(newlist.topleft());
				numberOrderList();
				updateReadyButton();

				deselectUnitOrTile();
				_selectsquare = _level.cell(hold.position);
				_selector = hold;
				_activeOrderIndexNew = indexNew;
				_activeOrderIndexOld = indexOld;
				selectUnitOrTile();
			}
		}
		else if (!indexNew.empty() && oldlist.overed())
		{
			if (oldlist.contains(indexNew)
				&& oldlist[indexNew]["data"].text()
					== newlist[indexNew]["data"].text())
			{
				std::unique_ptr<InterfaceElement> element = newlist.remove(indexNew);
				oldlist.replace(indexNew, std::move(element));
				for (int i = 1; i <= _bible.newOrderLimit(); i++)
				{
					if (!newlist.contains(std::to_string(i)))
					{
						newlist.add(std::to_string(i), makeEmptyOrderListElement());
						break;
					}
				}
				oldlist.place(oldlist.topleft());
				newlist.place(newlist.topleft());
				oldlist[indexNew]["item"]["remove"].setTag("Box");
				numberOrderList();
				updateReadyButton();

				deselectUnitOrTile();
				_selectsquare = _level.cell(hold.position);
				_selector = hold;
				_activeOrderIndexNew = indexNew;
				_activeOrderIndexOld = indexOld;
				selectUnitOrTile();
			}
		}
		else if (!indexOld.empty() && newlist.overed() && can)
		{
			size_t over = newlist.size();
			std::string indexOver;
			for (size_t i = 0; i < newlist.size(); i++)
			{
				std::string name = newlist.name(i);
				if (over >= newlist.size()
					&& newlist[name].overed())
				{
					over = i;
					indexOver = name;
				}
			}
			if (over < newlist.size())
			{
				std::vector<std::string> names;
				std::vector<std::unique_ptr<InterfaceElement>> elements;
				for (size_t i = 0; i < newlist.size(); i++)
				{
					std::string name = newlist.name(i);
					if (i <= over && i + 1 < newlist.size()) continue;
					names.emplace_back(name);
				}
				for (const std::string& name : names)
				{
					elements.emplace_back(newlist.remove(name));
				}
				names.pop_back();
				Descriptor desc = parseDescriptorData(indexOld);
				{
					std::unique_ptr<InterfaceElement> element
						= makeOrderListElement(findOldOrder(desc), "Box");
					element = oldlist.replace(indexOld, std::move(element));
					newlist.add(indexOld, std::move(element));
				}
				for (size_t i = 0; i < names.size(); i++)
				{
					newlist.add(names[i], std::move(elements[i]));
				}
				newlist.place(newlist.topleft());
				newlist[indexOld]["item"]["remove"].setTag("Cross");
				crossOldOrder(desc);
				updateReadyButton();

				deselectUnitOrTile();
				_selectsquare = _level.cell(hold.position);
				_selector = hold;
				_activeOrderIndexNew = indexNew;
				_activeOrderIndexOld = indexOld;
				selectUnitOrTile();
			}
		}
		else if (!indexOld.empty() && oldlist.overed())
		{
			// Nothing.
		}
	}
	else if (revert.type != Descriptor::Type::NONE)
	{
		Mixer::get()->queue(Clip::Type::UI_BLOCKED, 0.0f);
		revertOrder(revert);
		visualizeOrder(findOrder(revert));
		updateReadyButton();

		deselectUnitOrTile();
		_selectsquare = _level.cell(revert.position);
		_selector = revert;
		_activeOrderIndexNew = indexNew;
		_activeOrderIndexOld = indexOld;
		selectUnitOrTile();
	}
	else if (stop.type != Descriptor::Type::NONE)
	{
		bool has = hasNewOrder(stop);
		bool full = countNewOrders() >= _bible.newOrderLimit();
		bool can = canGiveOrders() && (has || !full);
		if (can)
		{
			Order order(Order::Type::HALT, stop);
			giveOrder(order);
			updateReadyButton();

			deselectUnitOrTile();
			_selectsquare = _level.cell(stop.position);
			_selector = stop;
			_activeOrderIndexNew = indexNew;
			_activeOrderIndexOld = indexOld;
			selectUnitOrTile();
		}
		else if (canGiveOrders())
		{
			blinkOrderListFull();
		}
	}
}

Order Commander::findOrder(const Descriptor& subject)
{
	Order order = findNewOrder(subject);
	if (order.type != Order::Type::NONE) return order;
	else return findOldOrder(subject);
}

Order Commander::findNewOrder(const Descriptor& subject)
{
	std::stringstream strm;
	strm << subject;
	std::string subjectname = strm.str();

	InterfaceElement& newlist = getOrderlistNew();
	for (size_t i = 0; i < newlist.size(); i++)
	{
		std::string name = newlist.name(i);
		if (subjectname == name)
		{
			return parseOrderData(newlist[name]["data"].text());
		}
	}

	return Order();
}

Order Commander::findOldOrder(const Descriptor& subject)
{
	std::stringstream strm;
	strm << subject;
	std::string subjectname = strm.str();

	InterfaceElement& oldlist = getOrderlistOld();
	for (size_t i = 0; i < oldlist.size(); i++)
	{
		std::string name = oldlist.name(i);
		if (subjectname == name)
		{
			return parseOrderData(oldlist[name]["data"].text());
		}
	}

	return Order();
}

bool Commander::hasOrder(const Descriptor& subject)
{
	return (hasNewOrder(subject) || hasOldOrder(subject));
}

bool Commander::hasNewOrder(const Descriptor& subject)
{
	std::stringstream strm;
	strm << subject;
	std::string subjectname = strm.str();

	InterfaceElement& newlist = getOrderlistNew();
	for (size_t i = 0; i < newlist.size(); i++)
	{
		std::string name = newlist.name(i);
		if (subjectname == name)
		{
			return (newlist[name]["data"].text() != "$discarded");
		}
	}

	return false;
}

bool Commander::hasOldOrder(const Descriptor& subject)
{
	std::stringstream strm;
	strm << subject;
	std::string subjectname = strm.str();

	InterfaceElement& oldlist = getOrderlistOld();
	for (size_t i = 0; i < oldlist.size(); i++)
	{
		std::string name = oldlist.name(i);
		if (subjectname == name)
		{
			return (oldlist[name]["data"].text() != "$discarded");
		}
	}

	return false;
}

void Commander::crossOldOrder(const Descriptor& subject)
{
	std::stringstream strm;
	strm << subject;
	std::string subjectname = strm.str();

	InterfaceElement& oldlist = getOrderlistOld();
	for (size_t i = 0; i < oldlist.size(); i++)
	{
		std::string name = oldlist.name(i);
		if (subjectname == name)
		{
			oldlist[name].disable();
			break;
		}
	}

	numberOrderList();
}

void Commander::dropNewOrder(const Descriptor& subject)
{
	std::stringstream strm;
	strm << subject;
	std::string subjectname = strm.str();

	std::string foundname = "";

	InterfaceElement& newlist = getOrderlistNew();
	for (size_t i = 0; i < newlist.size(); i++)
	{
		std::string name = newlist.name(i);
		if (subjectname == name)
		{
			foundname = name;
			break;
		}
	}

	if (foundname.empty()) return;

	newlist.remove(foundname);

	{
		std::string missing;
		for (int i = 1; i <= _bible.newOrderLimit(); i++)
		{
			if (!newlist.contains(std::to_string(i)))
			{
				missing = std::to_string(i);
				break;
			}
		}
		std::vector<std::string> names;
		std::vector<std::unique_ptr<InterfaceElement>> elements;
		for (size_t i = 0; i < newlist.size(); i++)
		{
			std::string name = newlist.name(i);
			if (name[0] >= '0' && name[0] <= '9')
			{
				names.emplace_back(name);
			}
		}
		for (const std::string& name : names)
		{
			elements.emplace_back(newlist.remove(name));
		}
		newlist.add(missing, makeEmptyOrderListElement());
		for (size_t i = 0; i < names.size(); i++)
		{
			newlist.add(names[i], std::move(elements[i]));
		}
	}

	numberOrderList();
	getOrderlistOld().settle();
	getOrderlistNew().settle();
}

void Commander::insertNewOrder(const Descriptor& subject,
		std::unique_ptr<InterfaceElement> orderlistelement)
{
	std::stringstream strm;
	strm << subject;
	std::string subjectname = strm.str();

	std::string foundname = "";

	InterfaceElement& newlist = getOrderlistNew();
	for (size_t i = 0; i < newlist.size(); i++)
	{
		std::string name = newlist.name(i);

		if (!newlist[name].enabled())
		{
			foundname = name;
			break;
		}
	}

	if (foundname.empty())
	{
		LOGE << "No more room to insert new order.";
		DEBUG_ASSERT(false);
		return;
	}

	newlist.replace(foundname, std::move(orderlistelement), subjectname);

	numberOrderList();
	getOrderlistOld().settle();
	getOrderlistNew().settle();
}

std::unique_ptr<InterfaceElement> Commander::makeOrderListElement(
	const Order& order, const char* buttontag)
{
	switch (order.subject.type)
	{
		case Descriptor::Type::TILE:
		{
			const Square* square = _level.cell(order.subject.position);
			if (square == nullptr)
			{
				LOGE << "unexpected nullptr";
				break;
			}
			const TileToken& tiletoken = square->tile();
			if (!tiletoken)
			{
				LOGE << "unexpected tile not found";
				break;
			}

			return makeOrderListElement(tiletoken.type, order, buttontag);
		}
		break;
		case Descriptor::Type::GROUND:
		{
			const Square* square = _level.cell(order.subject.position);
			if (square == nullptr)
			{
				LOGE << "unexpected nullptr";
				break;
			}
			const UnitToken& unittoken = square->ground();
			if (!unittoken)
			{
				LOGE << "unexpected unit not found";
				break;
			}

			return makeOrderListElement(unittoken.type, order, buttontag);
		}
		break;
		case Descriptor::Type::AIR:
		{
			const Square* square = _level.cell(order.subject.position);
			if (square == nullptr)
			{
				LOGE << "unexpected nullptr";
				break;
			}
			const UnitToken& unittoken = square->air();
			if (!unittoken)
			{
				LOGE << "unexpected unit not found";
				break;
			}

			return makeOrderListElement(unittoken.type, order, buttontag);
		}
		break;
		case Descriptor::Type::NONE:
		{
			if (order.type == Order::Type::NONE)
			{
				// Detect little hack from Commander::listOrder().
				if (order.subject.position.col != 0)
				{
					// Enable it so that the next order is placed after it.
					auto element = makeEmptyOrderListElement();
					element->enable();
					return element;
				}

				return makeEmptyOrderListElement();
			}
			else
			{
				LOGE << "missing case";
				break;
			}
		}
		break;
		default:
		{
			LOGE << "missing case";
			break;
		}
	}

	LOGE << "order " << TypeEncoder(&_bible) << order;
	DEBUG_ASSERT(false);
	return InterfaceElement::makeGarbage();
}

void Commander::tagOrdersGuiltyUntilProvenInnocent()
{
	{
		InterfaceElement& oldlist = getOrderlistOld();
		for (size_t i = 0; i < oldlist.size(); i++)
		{
			const std::string& name = oldlist.name(i);
			if (name[0] >= '0' && name[0] <= '9') continue;
			oldlist[name]["data"].setText("$discarded");
		}
	}

	{
		InterfaceElement& newlist = getOrderlistNew();
		for (size_t i = 0; i < newlist.size(); i++)
		{
			const std::string& name = newlist.name(i);
			if (name[0] >= '0' && name[0] <= '9') continue;
			newlist[name]["data"].setText("$discarded");
		}
	}
}

void Commander::keepOrder(const Descriptor& oldsubject, const Order& neworder)
{
	std::string index;
	{
		std::stringstream strm;
		strm << oldsubject;
		index = strm.str();
	}

	std::string orderstr;
	{
		std::stringstream strm;
		strm << TypeEncoder(&_bible) << neworder;
		orderstr = strm.str();
	}

	// Note newlist first, because there might be an overridden old order that
	// we don't want to keep.

	{
		InterfaceElement& newlist = getOrderlistNew();
		if (newlist.contains(index))
		{
			newlist[index]["data"].setText(orderstr);
			return;
		}
	}

	{
		InterfaceElement& oldlist = getOrderlistOld();
		if (oldlist.contains(index))
		{
			oldlist[index]["data"].setText(orderstr);
			return;
		}
	}

	LOGW << "Cannot find old subject " << index
		<< " for new order " << orderstr;
	DEBUG_ASSERT(false);
}

void Commander::listOrder(const Order& order)
{
	// Little hack to give each NONE ("SLEEP") order a different subject.
	if (order.type == Order::Type::NONE)
	{
		Order sleep = order;
		sleep.subject.position.col = countNewOrders() + 1;
		insertNewOrder(sleep.subject, makeOrderListElement(sleep, "Cross"));
		return;
	}

	crossOldOrder(order.subject);
	insertNewOrder(order.subject, makeOrderListElement(order, "Cross"));
}

void Commander::giveOrder(const Order& order)
{
	std::unique_ptr<InterfaceElement> orderlistelement
		= makeOrderListElement(order, "Cross");

	{
		Position position = order.subject.position;
		Pixel pixel = Camera::get()->convert(
			Surface::convertOrigin(position));
		pixel.xenon -= orderlistelement->width() / 2;
		pixel.yahoo += 5 * Camera::get()->scale();
		pixel.proximity = Layer::INTERFACE;
		orderlistelement->place(pixel);
	}

	giveNewOrder(order.subject, std::move(orderlistelement));
}

void Commander::giveNewOrder(const Descriptor& subject,
		std::unique_ptr<InterfaceElement> orderlistelement)
{
	switch (subject.type)
	{
		case Descriptor::Type::TILE:
		case Descriptor::Type::GROUND:
		case Descriptor::Type::AIR:
		{
			crossOldOrder(subject);
			dropNewOrder(subject);
			insertNewOrder(subject, std::move(orderlistelement));
			deselectActiveOrder();
			selectActiveOrder(subject);
		}
		break;
		case Descriptor::Type::BYPASS:
		case Descriptor::Type::CELL:
		{
			LOGE << "Invalid subject " << subject;
		}
		break;
		case Descriptor::Type::NONE:
		break;
	}

	updateReadyButton();
}

void Commander::updateReadyButton()
{
	InterfaceElement& readybutton = getReadyButton();
	if (canGiveOrders() && countNewOrders() > 0 && _game.haveSentOrders())
	{
		readybutton.power();
		readybutton["box"].setTag("Empty");
	}
	else if (canGiveOrders() && countNewOrders() > 0 && !_game.haveSentOrders())
	{
		readybutton.power();
		readybutton["box"].setTag("Dotted");
	}
	else
	{
		readybutton.depower();
		readybutton["box"].setTag("Empty");
	}
}

void Commander::revertOrder(const Descriptor& subject)
{
	dropNewOrder(subject);

	InterfaceElement& oldlist = getOrderlistOld();
	for (size_t i = 0; i < oldlist.size(); i++)
	{
		std::string name = oldlist.name(i);
		std::stringstream strm;
		strm << subject;
		if (strm.str() == name)
		{
			oldlist[name].enable();
			if (_activeOrderIndexOld == name)
			{
				oldlist[name].power();
			}
			break;
		}
	}
	numberOrderList();
}

Descriptor Commander::parseDescriptorData(const std::string& str)
{
	Json::Reader reader;
	Json::Value descjson;

	if (!reader.parse(str, descjson))
	{
		LOGE << "corrupt descriptor data '" << str << "'";
		DEBUG_ASSERT(false);
		return Descriptor();
	}

	try
	{
		return Descriptor(descjson);
	}
	catch (const ParseError& error)
	{
		LOGE << "corrupt descriptor data: " << error.what();
		DEBUG_ASSERT(false);
		return Descriptor();
	}
	catch (const Json::Exception& error)
	{
		LOGE << "corrupt descriptor data: " << error.what();
		DEBUG_ASSERT(false);
		return Descriptor();
	}
}

Order Commander::parseOrderData(const std::string& str)
{
	Json::Reader reader;
	Json::Value orderjson;

	if (str == "$discarded")
	{
		return Order();
	}

	if (!reader.parse(str, orderjson))
	{
		LOGE << "corrupt order data '" << str << "'";
		DEBUG_ASSERT(false);
		return Order();
	}

	try
	{
		return Order(_bible, orderjson);
	}
	catch (const ParseError& error)
	{
		LOGE << "corrupt order data: " << error.what();
		DEBUG_ASSERT(false);
		return Order();
	}
	catch (const Json::Exception& error)
	{
		LOGE << "corrupt order data: " << error.what();
		DEBUG_ASSERT(false);
		return Order();
	}
}

void Commander::giveHaltOrderOrRevoke()
{
	if (!hasNewOrder(_selector))
	{
		Order order(Order::Type::HALT, _selector);
		giveOrder(order);
		visualizeOrder(order);
	}
	else
	{
		revertOrder(_selector);
		visualizeOrder(findOrder(_selector));
	}
}

void Commander::giveMoveOrder()
{
	Cell from = _selectsquare->cell();

	std::vector<Move> moves;
	PathingFlowfield pathing(_bible, _board);

	bool air = _bible.unitAir(_selectsquare->unit(_selector.type).type);
	if (air) pathing.fly();

	pathing.put(from, Move::X);

	Order existingorder = findOrder(_selector);
	if (!existingorder.moves.empty())
	{
		Cell at = from;
		for (size_t i = 0; i < existingorder.moves.size(); i++)
		{
			at = at + existingorder.moves[i];
			pathing.put(at, ::flip(existingorder.moves[i]));
		}
	}

	pathing.execute();

	{
		Cell at = _hoversquare->cell();
		Move move;
		while ((move = pathing.step(at)) != Move::X)
		{
			moves.emplace_back(::flip(move));
			at = at + move;
		}
		std::reverse(moves.begin(), moves.end());
	}

	Descriptor target = Descriptor::cell(from.pos());
	{
		size_t keep = 0;
		Cell at = from;
		for (const Move& move : moves)
		{
			at = at + move;
			if (!_level.cell(at.pos())->tile()) break;
			if (!_bible.tileAccessible(_level.cell(at.pos())->tile().type)) break;
			if (!_bible.tileWalkable(_level.cell(at.pos())->tile().type) && !air) break;
			target = Descriptor::cell(at.pos());
			keep++;
		}
		moves.resize(keep);
	}

	if (moves.size() < 1)
	{
		revertOrder(_selector);
		visualizeOrder(findOrder(_selector));
		return;
	}

	Order order = Order(Order::Type::MOVE, _selector, target, moves);
	giveOrder(order);
	visualizeOrder(order);

}

void Commander::visualizeOrder(const Order& order)
{
	clearGuides();

	switch (order.type)
	{
		case Order::Type::MOVE:    visualizeMoveOrder(order);    break;
		case Order::Type::GUARD:   visualizeGuardOrder(order);   break;
		case Order::Type::FOCUS:   visualizeFocusOrder(order);   break;
		case Order::Type::LOCKDOWN: visualizeLockdownOrder(order); break;
		case Order::Type::SHELL:   visualizeShellOrder(order);   break;
		case Order::Type::BOMBARD: visualizeBombardOrder(order); break;
		case Order::Type::BOMB:    visualizeBombOrder(order);    break;
		case Order::Type::CAPTURE: visualizeCaptureOrder(order); break;
		case Order::Type::SHAPE:   visualizeShapeOrder(order);   break;
		case Order::Type::SETTLE:  visualizeSettleOrder(order);  break;
		case Order::Type::EXPAND:  visualizeExpandOrder(order);  break;
		case Order::Type::UPGRADE: visualizeUpgradeOrder(order); break;
		case Order::Type::CULTIVATE: visualizeCultivateOrder(order); break;
		case Order::Type::PRODUCE: visualizeProduceOrder(order); break;
		case Order::Type::HALT:    visualizeHaltOrder(order);    break;
		case Order::Type::NONE:                                  break;
	}
}

void Commander::visualizeMoveOrder(const Order& order)
{
	// Determine how many movesteps we might be able to take.
	const Square* subjectsquare = _level.cell(order.subject.position);
	const UnitToken& movingunit = subjectsquare->unit(order.subject.type);
	int basespeed = _bible.unitSpeed(movingunit.type);
	int speed = basespeed;
	int slow = 0;
	bool air = _bible.unitAir(movingunit.type);
	int steps = 0;

	Mixer::get()->queue(Clip::Type::WHISTLE, 0.0f);

	const Square* sq = subjectsquare;
	_guides.emplace_back(Guide::move(sq->cell(), Move::X));
	for (const Move& move : order.moves)
	{
		// Check if there is anything slowing us down. (1)
		if (!air && sq->snow()
			&& _bible.snowSlowAmount() > 0
			&& slow < _bible.snowSlowMaximum())
		{
			// Increase slow.
			slow = std::min(slow + _bible.snowSlowAmount(),
				(int) _bible.snowSlowMaximum());

			// Reduce movement speed.
			speed = std::min(speed, std::max(basespeed - slow, 1));
		}

		// Check if there is anything slowing us down. (2)
		TileType fromtype = sq->tile().type;
		if (!air && _bible.tileSlowAmount(fromtype) > 0
			&& slow < _bible.tileSlowMaximum(fromtype))
		{
			// Increase slow.
			slow = std::min(slow + _bible.tileSlowAmount(fromtype),
				(int) _bible.tileSlowMaximum(fromtype));

			// Reduce movement speed.
			speed = std::min(speed, std::max(basespeed - slow, 1));
		}

		sq = sq->eswn(move);
		Cell to = sq->cell();
		if (steps < speed) _guides.emplace_back(Guide::move(to, move));
		else if (steps < basespeed) _guides.emplace_back(Guide::slow(to, move));
		else _guides.emplace_back(Guide::postmove(to, move));
		steps += 1;

		// Are we forced to stop at an unoccupied trenches tile?
		if (_bible.tileForceOccupy(sq->tile().type)
			&& !air
			&& !_bible.unitMechanical(movingunit.type)
			&& !sq->unit(order.subject.type))
		{
			// Reduce movement speed to 1.
			speed = std::min(speed, 1);
		}
	}
}

void Commander::visualizeGuardOrder(const Order& order)
{
	const Square* targetsquare = _level.cell(order.target.position);
	_guides.emplace_back(Guide::order(targetsquare->cell(),
		spritenameOrderIcon(order)));
}

void Commander::visualizeFocusOrder(const Order& order)
{
	const Square* targetsquare = _level.cell(order.target.position);
	_guides.emplace_back(Guide::order(targetsquare->cell(),
		spritenameOrderIcon(order)));
}

void Commander::visualizeLockdownOrder(const Order& order)
{
	const Square* targetsquare = _level.cell(order.target.position);
	_guides.emplace_back(Guide::order(targetsquare->cell(),
		spritenameOrderIcon(order)));
}

void Commander::visualizeShellOrder(const Order& order)
{
	const Square* targetsquare = _level.cell(order.target.position);
	_guides.emplace_back(Guide::order(targetsquare->cell(),
		spritenameOrderIcon(order)));
}

void Commander::visualizeBombardOrder(const Order& order)
{
	const Square* targetsquare = _level.cell(order.target.position);
	_guides.emplace_back(Guide::order(targetsquare->cell(),
		spritenameOrderIcon(order)));
}

void Commander::visualizeBombOrder(const Order& order)
{
	const Square* subjectsquare = _level.cell(order.subject.position);
	_guides.emplace_back(Guide::order(subjectsquare->cell(),
		spritenameOrderIcon(order)));
}

void Commander::visualizeCaptureOrder(const Order& order)
{
	const Square* subjectsquare = _level.cell(order.subject.position);
	_guides.emplace_back(Guide::order(subjectsquare->cell(),
		spritenameOrderIcon(order)));
}

void Commander::visualizeShapeOrder(const Order& order)
{
	const Square* subjectsquare = _level.cell(order.subject.position);
	_guides.emplace_back(Guide::order(subjectsquare->cell(),
		spritenameOrderIcon(order)));
	checkAutoCultivate(subjectsquare, order.tiletype);
}

void Commander::visualizeSettleOrder(const Order& order)
{
	const Square* subjectsquare = _level.cell(order.subject.position);
	_guides.emplace_back(Guide::order(subjectsquare->cell(),
		spritenameOrderIcon(order)));
	checkAutoCultivate(subjectsquare, order.tiletype);
}

void Commander::visualizeExpandOrder(const Order& order)
{
	const Square* targetsquare = _level.cell(order.target.position);
	_guides.emplace_back(Guide::order(targetsquare->cell(),
		spritenameOrderIcon(order)));
	checkAutoCultivate(targetsquare, order.tiletype);
}

void Commander::visualizeUpgradeOrder(const Order& order)
{
	const Square* subjectsquare = _level.cell(order.subject.position);
	_guides.emplace_back(Guide::order(subjectsquare->cell(),
		spritenameOrderIcon(order)));
	checkAutoCultivate(subjectsquare, order.tiletype);
}

void Commander::visualizeCultivateOrder(const Order& order)
{
	const Square* subjectsquare = _level.cell(order.subject.position);
	for (Cell index : subjectsquare->area(1, 2))
	{
		const Square* target = _level.cell(index.pos());
		bool success = true;

		// Only some tiles (grass, dirt) can be built on.
		if (!_bible.tileBuildable(target->tile().type))
		{
			success = false;
		}

		// If there is a ground unit occupying the tile, we cannot build on it.
		// But they might move so we assume that we can build.

		// If the tile is owned by the player, we do not build on it.
		if (target->tile().owner == _player)
		{
			success = false;
		}

		if (success)
		{
			_guides.emplace_back(Guide::order(index,
				spritenameOrderIcon(order)));
		}
		else
		{
			_guides.emplace_back(Guide::skiptarget(index));
		}
	}
}

void Commander::visualizeProduceOrder(const Order& order)
{
	const Square* targetsquare = _level.cell(order.target.position);
	_guides.emplace_back(Guide::order(targetsquare->cell(),
		spritenameOrderIcon(order)));
}

void Commander::visualizeHaltOrder(const Order& order)
{
	const Square* subjectsquare = _level.cell(order.subject.position);
	_guides.emplace_back(Guide::halt(subjectsquare->cell()));
}

void Commander::checkAutoCultivate(const Square* targetsquare,
	const TileType& cultivatortype)
{
	if (_bible.tileAutoCultivates(cultivatortype)
		&& !_bible.tileCultivates(cultivatortype).empty())
	{
		TileType newtype = _bible.tileCultivates(cultivatortype)[0].type;
		visualizeCultivateOrder(Order(Order::Type::CULTIVATE,
			Descriptor::tile(targetsquare->position()),
			newtype));
	}
}

std::vector<Order> Commander::orders()
{
	deselectUnitOrTile();

	return listNewOrders();
}

std::vector<Order> Commander::listNewOrders()
{
	InterfaceElement& newlist = getOrderlistNew();

	size_t size = 0;
	std::vector<Order> newOrders(newlist.size());
	for (size_t i = 0; i < newlist.size(); i++)
	{
		std::string name = newlist.name(i);
		if (newlist[name].enabled())
		{
			newOrders[i] = parseOrderData(newlist[name]["data"].text());
			size = i + 1;
		}
	}

	// Keep empty orders between real orders, but trim empty orders at the end.
	newOrders.resize(size);

	return newOrders;
}

void Commander::unlistNewOrders()
{
	InterfaceElement& newlist = getOrderlistNew();
	std::vector<std::string> names;
	for (size_t i = 0; i < newlist.size(); i++)
	{
		std::string name = newlist.name(i);
		if (name[0] >= '0' && name[0] <= '9') continue;
		names.emplace_back(name);
	}
	for (const std::string& name : names)
	{
		revertOrder(parseDescriptorData(name));
	}
}

void Commander::control()
{
	if (_targetcursor)
	{
		if (_hoversquare != nullptr) _targetcursor->set(_hoversquare->cell());
		else _targetcursor->set(Cell::undefined());
	}

	// Try each of the control() methods in order. If a method returns true, it handled input.
	if      (controlSelectorContext()) {}
	else if (controlOrderContext())    {}
	else if (controlTargetCursor())    {}
	else if (controlUnitOrders())      {}
	else if (controlTileOrders())      {}
	else if (controlSelector())        {}

	controlOrderList();

	Observer::control();

	InterfaceElement& readybutton = getReadyButton();
	if (canGiveOrders()
		&& readybutton.enabled()
		&& (readybutton.clicked()))
	{
		Mixer::get()->queue(Clip::Type::UI_CLICK, 0.0f);
		_game.sendOrders();

		readybutton.depower();
		readybutton["box"].setTag("Checked");
	}

	if (canGiveOrders() && doesViewportHaveFocus())
	{
		readybutton.setHotkeyScancode(SDL_SCANCODE_SPACE);
	}
	else
	{
		readybutton.setHotkeyScancode(0);
	}
}

void Commander::hoveredContexts()
{
	// Because some items might occlude others, we must know which layer the mouse operates in.
	// We therefore need to check all UI items to see if they are hovered.
	if (_selectorcontext) _selectorcontext->hovered();
	if (_ordercontext) _ordercontext->hovered();
}

bool Commander::updateOtherCursor()
{
	if      (_targetcursor) _targetcursor->update();
	else if (_selectorcontext) _selectorcontext->refresh();
	else if (_ordercontext) _ordercontext->refresh();
	else return false;

	return true;
}

void Commander::updateWallet()
{
	int oldvalue = _displaymoney;
	int newvalue;
	int min = _bible.minMoney();
	int max = (_bible.maxMoney() > 0) ? _bible.maxMoney() : 999;
	if (auto group = _animations.lock())
	{
		newvalue = std::max(min, std::min((int) group->money, max));
	}
	else
	{
		newvalue = std::max(min, std::min(_money, max));
	}
	if (newvalue == oldvalue) return;

	InterfaceElement& wallet = getWallet();

	float dt = Loop::delta() * Loop::tempo();
	_displaymoneydelay -= dt;
	if (_displaymoneydelay > 0) return;

	_displaymoney += signum(newvalue - oldvalue);
	wallet["text"].setText(std::to_string(_displaymoney));

	if (wallet.contains("icon"))
	{
		if (newvalue >= oldvalue)
		{
			wallet["icon"]["coin"].setTagActive("Up", /*restart=*/true);
		}
		else
		{
			wallet["icon"]["coin"].setTagActive("Down", /*restart=*/true);
		}
	}
	else if (wallet.contains("coin"))
	{
		wallet["coin"].setTagActive("Jiggle", /*restart=*/true);
	}

	int diff = std::abs(newvalue - oldvalue);
	_displaymoneydelay = 0.035f + 0.003f * std::max(0, 21 - diff);
}

Order Commander::getHoveredOrder()
{
	if (_ordercontext)
	{
		Order order = _ordercontext->peek();
		if (order.type == Order::Type::HALT)
		{
			if (hasNewOrder(order.subject))
			{
				// This is a little hack to let the Observer know it is a
				// revoke pseudo-order. See Observer::updateCards().
				order.target = order.subject;
			}
		}
		return order;
	}
	else return Order();
}

void Commander::fillActingFoci(const Descriptor& subject,
	std::vector<Cell>& foci)
{
	Order order = findOrder(subject);
	switch (order.type)
	{
		case Order::Type::BOMBARD:
		{
			// Special case for bombard because its target may be outside
			// of the vision radius of the subject.
			DEBUG_ASSERT(order.target);
			foci.emplace_back(_level.index(order.target.position));
		}
		break;

		default:
		break;
	}
}
