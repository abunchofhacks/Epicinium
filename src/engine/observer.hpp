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

#include <queue>

#include "cell.hpp"
#include "animator.hpp"
#include "bible.hpp"
#include "skinner.hpp"
#include "board.hpp"
#include "arranger.hpp"
#include "level.hpp"
#include "camerafocus.hpp"
#include "cycle.hpp"
#include "change.hpp"
#include "descriptor.hpp"
#include "guide.hpp"
#include "padding.hpp"
#include "stackedlayout.hpp"
#include "color.hpp"
#include "paint.hpp"

struct AnimationGroup;
class Square;
class Cursor;
class Settings;
class Game;


class Observer : protected Animator
{
protected:
	struct ObservedChange
	{
		const Change* change;
		Position focus;
		Point point;

		ObservedChange(const Change* c, const Position& f, const Point& p) :
			change(c),
			focus(f),
			point(p)
		{}
	};

public:
	Observer(Settings& settings, Game& game,
		const Player& player, const std::string& rulesetname);
	Observer(Settings& settings, Game& game,
		const std::string& rulesetname);
	Observer(const Observer&) = delete;
	Observer(Observer&&) = delete;
	Observer& operator=(const Observer&) = delete;
	Observer& operator=(Observer&&) = delete;

	virtual ~Observer();

protected:
	Settings& _settings;
	Game& _game;
	Arranger _arranger;
	Bible _bible;
	Skinner _skinner;
	Player _player;

	Board _board;
	Level _level;
	int _year;
	Season _season;
	Daytime _daytime;
	Phase _phase;
	int _score;
	bool _defeated;
	bool _gameover;

	StackedLayout _underlayout;
	Padding _layout;
	StackedLayout _popuplayout;

	std::vector<std::string> _chatmodeTarget;
	std::vector<std::string> _chatmodeLabel;
	std::vector<Paint> _chatmodeColor;
	size_t _chatmode;

	std::shared_ptr<CameraFocus> _camerafocus; // (unique ownership)

	float _pauseJuiceDisplayMax;
	float _pauseJuice;
	float _panningJuice;
	bool _panning;
	bool _animating;
	bool _pause;
	bool _pauseOnce;
	bool _skipanimations;
	bool _skiplighting;
	bool _skippanning;
	bool _silentplanningbell;
	bool _hideidleanimations;
	bool _delayedEnabler;
	bool _hideLayouts;
	bool _showViewport;

	std::queue<std::vector<ObservedChange>> _unprocessedChunks;
	std::queue<std::vector<Change>> _unprocessedChanges;
	std::weak_ptr<AnimationGroup> _animations;

	Square* _hoversquare;
	Square* _selectsquare;
	Descriptor _selector;
	std::unique_ptr<Cursor> _cursor;
	std::vector<Guide> _guides;

	void adjustCameraToViewport();
	int viewportWidth();
	int viewportHeight();
	Player getSubjectOwner(const Change& change);
	bool isChangePannable(const Change& change);
	bool isChangeSwappable(const Change& change);
	bool isChangeDeferable(const Change& change);
	void fillFoci(const Change& change, std::vector<Cell>& foci);

	virtual void fillActingFoci(const Descriptor&, std::vector<Cell>&) {}

	void separateIntoChunks();
	void panCamera(bool backToBase = false);
	void processChanges();
	void processChange(const std::shared_ptr<AnimationGroup> group,
		const Change& change);

	virtual void prepareAnimationGroup();
	virtual void handleInitiative(const Change&);
	virtual void handleWallet(const Change&);
	virtual void handleOrder(const Change&);
	virtual void blinkWallet() {}

	virtual void checkIdleUnits() {};
	virtual void uncheckIdleUnits() {};

	void deselectUnitOrTile();
	void selectUnitOrTile();

	std::string unitName(const UnitType& unittype);
	std::string tileName(const TileType& tiletype);
	std::string description(const Order& order);

	const char* spritenameOrderIcon(const UnitType& unittype);
	const char* spritenameOrderIcon(const TileType& tiletype);
	const char* spritenameOrderIcon(const Order::Type& ordertype);
	const char* spritenameOrderIcon(const Order& order);

	std::unique_ptr<InterfaceElement> makeOrderIcon(const char* spritename);
	std::unique_ptr<InterfaceElement> makeOrderButton(const char* tagname);
	std::unique_ptr<InterfaceElement> makeOrderGrip();
	std::unique_ptr<InterfaceElement> makeEmptyOrderListElement();
	std::unique_ptr<InterfaceElement> makeOrderListElement(const UnitType& unittype,
		const Order& order, const char* buttontag);
	std::unique_ptr<InterfaceElement> makeOrderListElement(const TileType& tiletype,
		const Order& order, const char* buttontag);
	std::unique_ptr<InterfaceElement> makeOrderListElement(const char* subjectspritename,
		const std::string& desc, const Order& order, const char* buttontag);
	std::unique_ptr<InterfaceElement> makeOrderListFrame();

	std::unique_ptr<InterfaceElement> makeCard(
		std::unique_ptr<InterfaceElement> icon,
		const std::string& title, const std::string& description);

	std::unique_ptr<InterfaceElement> makeTurnSequenceElement(
		int sequence, const Player& player);

	std::unique_ptr<InterfaceElement> makePhaseIcon(Daytime daytime,
		Phase phase);
	std::unique_ptr<InterfaceElement> makeIcon(const char* spritename);
	std::unique_ptr<InterfaceElement> makeIcon(const char* spritename,
		const Paint& color);
	std::unique_ptr<InterfaceElement> makeIconWithTooltip(
		std::unique_ptr<InterfaceElement> icon,
		const std::string& tooltip);
	std::unique_ptr<InterfaceElement> makeIconWithTooltipCube(
		std::unique_ptr<InterfaceElement> icon,
		const std::string& tooltip);
	std::unique_ptr<InterfaceElement> makeIconWithTooltipTall(
		std::unique_ptr<InterfaceElement> icon,
		const std::string& tooltip, bool isTop, bool isBottom);
	std::unique_ptr<InterfaceElement> makeIconWithTooltipWide(
		std::unique_ptr<InterfaceElement> icon,
		const std::string& tooltip, bool isTop, bool isBottom);
	std::unique_ptr<InterfaceElement> makeIconStat(
		std::unique_ptr<InterfaceElement> icon,
		const std::string& tooltip);

	std::unique_ptr<InterfaceElement> makeMenu();
	std::unique_ptr<InterfaceElement> makeMenuButton(const std::string& text);
	std::unique_ptr<InterfaceElement> makeMenuNub();
	std::unique_ptr<InterfaceElement> makeStatusBar();
	std::unique_ptr<InterfaceElement> makeSeasonBox();
	std::unique_ptr<InterfaceElement> makePhaseGraphic();
	std::unique_ptr<InterfaceElement> makeChatBox();
	std::unique_ptr<InterfaceElement> makeChatPreview();
	std::unique_ptr<InterfaceElement> makeDiplomacyBox();
	std::unique_ptr<InterfaceElement> makeDiplomacyNub();
	std::unique_ptr<InterfaceElement> makeReportBox();
	std::unique_ptr<InterfaceElement> makeMissionBox();
	std::unique_ptr<InterfaceElement> makeMissionNub();
	std::unique_ptr<InterfaceElement> makeQuitOverlay();
	std::unique_ptr<InterfaceElement> makePauseOverlay();
	std::unique_ptr<InterfaceElement> makeViewport();
	std::unique_ptr<InterfaceElement> makeCardView();
	std::unique_ptr<InterfaceElement> makeNothing();

	virtual std::unique_ptr<InterfaceElement> makeWallet();
	virtual std::unique_ptr<InterfaceElement> makeOldWallet();
	virtual std::unique_ptr<InterfaceElement> makeReadyButton();
	virtual std::unique_ptr<InterfaceElement> makeOrderList();

	std::unique_ptr<InterfaceElement> makeDefeatPopup();
	std::unique_ptr<InterfaceElement> makeVictoryPopup();
	std::unique_ptr<InterfaceElement> makeQuitPopup();

	std::unique_ptr<InterfaceElement> makeButton(
		const std::string& text, int fontsize);

	static Paint getPlayerItemColor(const Player& player);
	static Paint getPlayerTextColor(const Player& player);

	void addPlayerToDiplomacyBox(const Player& player,
		const std::string& name);

	bool isChallenge();
	void fillMissionBox(const Json::Value& json);

	void fillReportBox(const std::string& headline,
		const std::string& message1, const std::string& message2,
		const std::string& message3);
	void updateReportBoxDate();
	void updateReportBoxPrice();

	InterfaceElement& getViewport();
	InterfaceElement& getViewBoxHolder();
	InterfaceElement& getCardView();
	InterfaceElement& getMenu();
	InterfaceElement& getDiplomacyBox();
	InterfaceElement& getReportBox();
	InterfaceElement& getMissionBox();
	InterfaceElement& getChatHistory();
	InterfaceElement& getChatHistoryPreview();
	InterfaceElement& getChatPreview();
	InterfaceElement& getStatusBar();
	InterfaceElement& getSeasonBar();
	InterfaceElement& getSeasonTextField();
	InterfaceElement& getWallet();
	InterfaceElement& getDiplomacyNub();
	InterfaceElement& getMissionNub();
	InterfaceElement& getMenuNub();
	InterfaceElement& getPhaseGraphic();

	bool doesViewportHaveFocus();

	void openQuitPopup();

	virtual void selectActiveOrder(const Descriptor&) {}
	virtual void deselectActiveOrder() {}
	virtual void disableActiveOrder() {}

	virtual void touchReadyButton() {}

	virtual bool quitSelectorContext() { return false; }
	virtual bool quitOrderContext() { return false; }
	virtual bool quitTargetCursor() { return false; }

	virtual Order findOrder(const Descriptor&);
	virtual bool hasOrder(const Descriptor&);
	virtual	void visualizeOrder(const Order&) {}
	virtual void clearGuides() {}

	virtual void control();

	virtual void hoveredContexts() {}
	virtual bool updateOtherCursor() { return false; }
	virtual void updateWallet() {}

	void updateCards();
	void checkMarkerCard(stringref name, bool on, const char* iconname,
		const std::string& title, const std::string& description);
	void checkAirUnitCard(const UnitType& unittype);
	void checkGroundUnitCard(const UnitType& unittype);
	void checkTileCard(const TileType& tiletype);
	void checkOrderCard(const Order::Type& ordertype, bool revoke);

	virtual Order getHoveredOrder();

	void updatePhaseGraphic();

	void setYear(int value, const std::shared_ptr<AnimationGroup> group);
	void setSeason(Season value, const std::shared_ptr<AnimationGroup> group);
	void setDaytime(Daytime value, const std::shared_ptr<AnimationGroup> group);
	void setPhase(Phase value, const std::shared_ptr<AnimationGroup> group);
	void setLight(const std::shared_ptr<AnimationGroup> group);
	void light(float light, float duration);
	void setupLight(const std::shared_ptr<AnimationGroup> group);

	void setActionSpotlight(std::vector<Cell> foci);
	void setTargetSpotlight(std::vector<Cell> foci);
	void resetActionSpotlight();
	void resetTargetSpotlight();

	bool isChatEnabled();
	size_t getChatmode(const std::string& target);

public:
	void addChatmode(const std::string& target, const std::string& label,
		const Paint& color);
	void setChatmode(const std::string& target);

	void message(const std::string& message);
	void chat(const std::string& user, const std::string& message,
		const std::string& target);

	void assignColor(const std::string& name,
		const Player& player);
	void setSkins(const Json::Value& metadata);
	void receiveBriefing(const Json::Value& metadata);

	void disconnected();
	void outLobby();

	const Bible& bible() { return _bible; }

private:
	void build();

protected:
	virtual void postbuild();

public:
	void load();
	void receiveChanges(const std::vector<Change>& changes);
	void update();
	void attemptQuit();

	void setAnimation(bool animate);

	const Phase& phase() const { return _phase; }
	bool gameover() const { return _gameover; }
	bool animating() const;
};
