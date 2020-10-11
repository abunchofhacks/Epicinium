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

#include "observer.hpp"
#include "order.hpp"

enum class Player : uint8_t;
class TargetCursor;
class SelectorContext;
class OrderContext;


class Commander : public Observer
{
public:
	Commander(Settings& settings, Game& game,
		const Player& player, const std::string& rulesetname);
	Commander(const Commander&) = delete;
	Commander(Commander&&) = delete;
	Commander& operator=(const Commander&) = delete;
	Commander& operator=(Commander&&) = delete;

	virtual ~Commander();

private:
	int _money;
	int _displaymoney;
	float _displaymoneydelay;

	std::string _activeOrderIndexOld;
	std::string _activeOrderIndexNew;

	std::unique_ptr<TargetCursor> _targetcursor;
	std::unique_ptr<SelectorContext> _selectorcontext;
	std::unique_ptr<OrderContext> _ordercontext;

	bool _needunlisting = false;

	virtual void fillActingFoci(const Descriptor&, std::vector<Cell>&) override;

	virtual void prepareAnimationGroup() override;
	virtual void handleInitiative(const Change& change) override;
	virtual void handleWallet(const Change& change) override;
	virtual void handleOrder(const Change& change) override;

	virtual void checkIdleUnits() override;
	virtual void uncheckIdleUnits() override;

	void unlistDiscardedOrders();
	void unlistNewOrders();

	virtual std::unique_ptr<InterfaceElement> makeReadyButton() override;
	virtual std::unique_ptr<InterfaceElement> makeOrderList() override;

	using Observer::makeOrderListElement;
	std::unique_ptr<InterfaceElement> makeOrderListElement(
		const Order& order, const char* buttontag);

	std::unique_ptr<InterfaceElement> makeOrderListOld();
	std::unique_ptr<InterfaceElement> makeOrderListNew();
	std::unique_ptr<InterfaceElement> makeInitiativeSequencer();

	bool canGiveOrders();

	bool controlSelector();
	bool controlSelectorContext();

	bool controlUnitOrders();
	bool controlTileOrders();
	bool controlOrderContext();
	bool controlTargetCursor();

	virtual bool quitSelectorContext() override;
	virtual bool quitOrderContext() override;
	virtual bool quitTargetCursor() override;

	virtual void blinkWallet() override;

	void blinkOrderListFull();
	void controlOrderList();
	void numberOrderList();

	virtual Order findOrder(const Descriptor& subject) override;
	virtual bool hasOrder(const Descriptor& subject) override;

	Order findNewOrder(const Descriptor& subject);
	Order findOldOrder(const Descriptor& subject);
	bool hasNewOrder(const Descriptor& subject);
	bool hasOldOrder(const Descriptor& subject);

	void tagOrdersGuiltyUntilProvenInnocent();
	void keepOrder(const Descriptor& oldsubject, const Order& neworder);

	void listOrder(const Order& order);
	void giveOrder(const Order& order);
	void giveNewOrder(const Descriptor& subject,
		std::unique_ptr<InterfaceElement> orderlistelement);
	void crossOldOrder(const Descriptor& subject);
	void dropNewOrder(const Descriptor& subject);
	void insertNewOrder(const Descriptor& subject,
		std::unique_ptr<InterfaceElement> orderlistelement);
	void revertOrder(const Descriptor& subject);

	Descriptor parseDescriptorData(const std::string& string);
	Order parseOrderData(const std::string& string);

	void giveMoveOrder();
	void giveHaltOrderOrRevoke();

	virtual void selectActiveOrder(const Descriptor& subject) override;
	virtual void deselectActiveOrder() override;
	virtual void disableActiveOrder() override;

	size_t countNewOrders();
	std::vector<Order> listNewOrders();

	virtual void visualizeOrder(const Order& order) override;

	void visualizeMoveOrder(const Order& order);
	void visualizeGuardOrder(const Order& order);
	void visualizeFocusOrder(const Order& order);
	void visualizeLockdownOrder(const Order& order);
	void visualizeShellOrder(const Order& order);
	void visualizeBombardOrder(const Order& order);
	void visualizeBombOrder(const Order& order);
	void visualizeCaptureOrder(const Order& order);
	void visualizeShapeOrder(const Order& order);
	void visualizeSettleOrder(const Order& order);
	void visualizeExpandOrder(const Order& order);
	void visualizeUpgradeOrder(const Order& order);
	void visualizeCultivateOrder(const Order& order);
	void visualizeProduceOrder(const Order& order);
	void visualizeHaltOrder(const Order& order);
	void checkAutoCultivate(const Square* targetsquare,
		const TileType& cultivatortype);

	virtual void clearGuides() override;

	virtual void touchReadyButton() override;

	void updateReadyButton();

	virtual void control() override;

	virtual void hoveredContexts() override;
	virtual bool updateOtherCursor() override;
	virtual void updateWallet() override;

	virtual Order getHoveredOrder() override;

	virtual void postbuild() override;

	InterfaceElement& getOrderlistOld();
	InterfaceElement& getOrderlistNew();
	InterfaceElement& getInitiativeSequencer();
	InterfaceElement& getReadyButton();

public:
	const Player& player() const { return _player; }
	std::vector<Order> orders();

	void buildInfoBar();

private:
	friend class Tutorial;
};
