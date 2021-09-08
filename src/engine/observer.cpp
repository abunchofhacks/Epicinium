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
#include "observer.hpp"
#include "source.hpp"

#include "libs/SDL2/SDL_mouse.h"
#include "libs/SDL2/SDL_events.h"

#include "loop.hpp"
#include "camera.hpp"
#include "cursor.hpp"
#include "surface.hpp"
#include "animationgroup.hpp"
#include "input.hpp"
#include "library.hpp"
#include "verticallayout.hpp"
#include "horizontallayout.hpp"
#include "scrollablelayout.hpp"
#include "stackedlayout.hpp"
#include "slideshowlayout.hpp"
#include "alignedlayout.hpp"
#include "tooltiplayout.hpp"
#include "textinput.hpp"
#include "textfield.hpp"
#include "multitextfield.hpp"
#include "frame.hpp"
#include "image.hpp"
#include "filler.hpp"
#include "padding.hpp"
#include "clickanddrag.hpp"
#include "backing.hpp"
#include "hiddentag.hpp"
#include "game.hpp"
#include "settings.hpp"
#include "paint.hpp"
#include "colorname.hpp"
#include "mixer.hpp"
#include "notice.hpp"
#include "clip.hpp"
#include "skin.hpp"


Observer::Observer(Settings& settings, Game& game,
		const std::string& rulesetname) :
	Observer(settings, game, Player::OBSERVER, rulesetname)
{}

Observer::Observer(Settings& settings, Game& game,
		const Player& player,
		const std::string& rulesetname) :
	_settings(settings),
	_game(game),
	_arranger(110, 4),
	_bible(Library::getBible(rulesetname)),
	_skinner(_bible),
	_player(player),
	_board(_bible),
	_level(_bible, _skinner),
	_year(0),
	_season(Season::SPRING),
	_daytime(Daytime::LATE),
	_phase(Phase::GROWTH),
	_score(0),
	_defeated(false),
	_gameover(false),
	_chatmodeTarget({""}),
	_chatmodeLabel({
		_("CHAT")
	}),
	_chatmodeColor({ColorName::TEXT800}),
	_chatmode(0),
	_camerafocus(new CameraFocus(_settings, Surface::WIDTH)),
	_pauseJuiceDisplayMax(1),
	_pauseJuice(0),
	_panningJuice(0),
	_panning(false),
	_animating(false),
	_pause(false),
	_pauseOnce(false),
	_skipanimations(_settings.skipAnimations.value(false)),
	_skiplighting(_settings.skipLighting.value(_game.test())),
	_skippanning(_settings.skipPanning.value(_game.test())),
	_silentplanningbell(_settings.silentPlanningBell.value(_game.test())),
	_hideidleanimations(_settings.hideIdleAnimations.value(_game.test())),
	_delayedEnabler(false),
	_hideLayouts(_settings.hideLayouts.value(false)),
	_showViewport(_settings.showViewport.value(false)),
	_hoversquare(nullptr),
	_selectsquare(nullptr),
	_cursor(nullptr)
{}

void Observer::build()
{
	_layout.put(new HorizontalLayout());
	_layout.setPadding(10 * InterfaceElement::scale());

	_layout.add("center", new VerticalLayout());
	_layout.add("right", new VerticalLayout());

	_layout["center"].add("top", new HorizontalLayout());

	_layout["center"]["top"].add("menu", makeMenuNub());
	_layout["center"]["top"].add("diplomacy", makeDiplomacyNub());
	_layout["center"]["top"]["diplomacy"].disable();
	_layout["center"]["top"].add("mission", makeNothing());
	_layout["center"]["top"].add("filler1", new HorizontalFiller());
	_layout["center"]["top"]["filler1"].fixWidth(
		40 * InterfaceElement::scale());
	_layout["center"]["top"].add("statusbar", makeStatusBar());
	_layout["center"]["top"].add("filler2", new HorizontalFiller());
	_layout["center"]["top"]["filler2"].fixWidth(
		40 * InterfaceElement::scale());
	_layout["center"]["top"].add("seasonbox", makeSeasonBox());
	_layout["center"]["top"].settle();
	_layout["center"]["top"].fixHeight();

	_layout["right"].add("top", new StackedLayout());
	_layout["right"]["top"].add("phasegraphic", makePhaseGraphic());
	_layout["right"]["top"].add("other", new VerticalLayout());
	_layout["right"]["top"]["other"].add("filler", new VerticalFiller());
	_layout["right"]["top"]["other"].add("wallet", makeWallet());
	_layout["right"]["top"].align(VerticalAlignment::TOP);
	_layout["right"]["top"].align(HorizontalAlignment::RIGHT);
	_layout["right"]["top"].settle();
	_layout["right"]["top"].fixHeight(
		_layout["right"]["top"]["phasegraphic"].height()
			+ 10 * InterfaceElement::scale());
	_layout["right"].add("orderlist", makeOrderList());
	_layout["right"].add("readybutton", makeReadyButton());
	_layout["right"].fixWidth();
	_layout["right"].align(HorizontalAlignment::RIGHT);

	_layout["center"].add("mid", new SlideshowLayout());
	_layout["center"]["mid"].add("main", makeCardView());
	_layout["center"]["mid"].add("viewport", makeViewport());
	_layout["center"]["mid"].add("chat", makeChatBox());
	_layout["center"]["mid"].add("diplomacy", makeDiplomacyBox());
	_layout["center"]["mid"].add("report", makeReportBox());
	_layout["center"]["mid"].add("mission", makeMissionBox());
	_layout["center"]["mid"].add("menu", makeMenu());
	_layout["center"]["mid"].setTag("main");
	_layout["center"]["mid"].align(HorizontalAlignment::LEFT);
	_layout["center"]["mid"].align(VerticalAlignment::TOP);
	_layout["center"].add("bot", new HorizontalLayout());
	_layout["center"]["bot"].add("chat", makeChatPreview());
	_layout["center"]["bot"].align(VerticalAlignment::BOTTOM);

	_layout["right"]["readybutton"].settleHeight();
	_layout["right"]["readybutton"].fixHeight();
	_layout["center"]["bot"]["chat"].fixHeight(
		_layout["right"]["readybutton"].height());

	_layout.fixWidth(InterfaceElement::windowW());
	_layout.fixHeight(InterfaceElement::windowH());
	_layout.place(Pixel(0, 0, Layer::INTERFACE));

	if (isChatEnabled())
	{
		getChatHistoryPreview().fixWidth();
		getChatHistoryPreview().fixHeight();
	}
	getChatHistory().fixWidth();
	getChatHistory().fixHeight();

	adjustCameraToViewport();

	_underlayout.add("filler", new Filler());
	_underlayout.add("pauseoverlay", makePauseOverlay());
	_underlayout.align(HorizontalAlignment::CENTER);
	_underlayout.align(VerticalAlignment::MIDDLE);
	_underlayout.fixWidth(InterfaceElement::windowW());
	_underlayout.fixHeight(InterfaceElement::windowH());
	_underlayout.place(Pixel(0, 0, Layer::UNDERLAY));

	_popuplayout.add("filler", new Filler());
	_popuplayout.add("quitoverlay", makeQuitOverlay());
	_popuplayout.add("defeat", makeDefeatPopup());
	_popuplayout.add("victory", makeVictoryPopup());
	_popuplayout.add("quit", makeQuitPopup());
	_popuplayout.align(HorizontalAlignment::CENTER);
	_popuplayout.align(VerticalAlignment::MIDDLE);
	_popuplayout.fixWidth(InterfaceElement::windowW());
	_popuplayout.fixHeight(InterfaceElement::windowH());
	_popuplayout.place(Pixel(0, 0, Layer::POPUP));

	postbuild();
}

void Observer::postbuild()
{
	getStatusBar().fixWidth();
	getCardView().fixHeight();

	if (isChatEnabled())
	{
		getChatHistoryPreview().remove("filler");
	}
	else
	{
		InterfaceElement& menu = getMenu();
		menu["buttons"]["chat"].disable();
	}
	getChatHistory().remove("filler");

	std::string HOPELINES[] = {
		_("FUTURE PROMISES PEACE AND PROSPERITY"),
		_("ECONOMY ON THE RISE"),
	};

	std::string RANDOMLINES[] = {
		_("SEVENTEEN NEW SPECIES OF FROG DISCOVERED"),
		_("RECORD NUMBER OF CYCLISTS ON CITY STREETS"),
		_("SURVEY SHOWS CATS MORE POPULAR THAN DOGS"),
		_("SURVEY SHOWS DOGS MORE POPULAR THAN CATS"),
		_("BEE POPULATION LARGER THAN EVER ACCORDING TO ENTOMOLOGISTS"),
		_("WOMAN DISCOVERS SAFE CONTAINING 1500 CANS OF MUSHROOM SOUP"),
		_("MAN FINALLY HAPPY AFTER SOLVING CHILDHOOD MATH PROBLEM"),
		_("COUPLE CELEBRATES FIVE YEARS WITHOUT USING THE LETTER R"),
	};

	fillReportBox(
		_("A NEW ERA BEGINS"),
		_("BRAVE PIONEERS SETTLE NEW CITIES"),
		HOPELINES[rand() % array_size(HOPELINES)],
		RANDOMLINES[rand() % array_size(RANDOMLINES)]);

	updateReportBoxDate();
	updateReportBoxPrice();
}

Observer::~Observer()
{
	Mixer::get()->stop();

	SDL_ShowCursor(SDL_ENABLE);
}

Player Observer::getSubjectOwner(const Change& change)
{
	const Square* square = _level.cell(change.subject.position);
	return square->tile().owner;
}

bool Observer::isChangePannable(const Change& change)
{
	switch (change.type)
	{
		// The NONE change indicates that a tile or order cannot act.
		// This is useful information and therefore pannable.
		case Change::Type::NONE: return true;

		// Moving is important.
		case Change::Type::STARTS: return true;
		case Change::Type::MOVES: return true;

		// Vision changes are only interesting when accompanying another
		// change, but then that change is already panned to.
		case Change::Type::REVEAL: return false;
		case Change::Type::OBSCURE: return false;

		// Natural transformations can be viewed afterwards.
		case Change::Type::TRANSFORMED: return false;

		// Consumations are only interesting when accompanying another
		// change, but then that change is already panned to.
		case Change::Type::CONSUMED: return false;

		// Abilities are important.
		case Change::Type::SHAPES: return true;
		case Change::Type::SHAPED: return true;
		case Change::Type::SETTLES: return true;
		case Change::Type::SETTLED: return true;
		case Change::Type::EXPANDS: return true;
		case Change::Type::EXPANDED: return true;
		case Change::Type::UPGRADES: return true;
		case Change::Type::UPGRADED: return true;
		case Change::Type::CULTIVATES: return true;
		case Change::Type::CULTIVATED: return true;
		case Change::Type::CAPTURES: return true;
		case Change::Type::CAPTURED: return true;
		case Change::Type::PRODUCES: return true;
		case Change::Type::PRODUCED: return true;

		// Vision changes are only interesting when accompanying another
		// change, but then that change is already panned to.
		case Change::Type::ENTERED: return false;
		case Change::Type::EXITED: return false;

		// Dying is usually accompanying something else, unless due to Death.
		case Change::Type::DIED: return true;
		case Change::Type::DESTROYED: return true;

		// Survival is always accompanying something else.
		case Change::Type::SURVIVED: return false;

		// Attacks and damage are important.
		case Change::Type::AIMS: return true;
		case Change::Type::ATTACKS: return true;
		case Change::Type::ATTACKED: return true;
		case Change::Type::TRAMPLES: return true;
		case Change::Type::TRAMPLED: return true;
		case Change::Type::SHELLS: return true;
		case Change::Type::SHELLED: return true;
		case Change::Type::BOMBARDS: return true;
		case Change::Type::BOMBARDED: return true;
		case Change::Type::BOMBS: return true;
		case Change::Type::BOMBED: return true;
		case Change::Type::FROSTBITTEN: return true;
		case Change::Type::BURNED: return true;
		case Change::Type::GASSED: return true;
		case Change::Type::IRRADIATED: return true;

		// Power growth at night is important.
		case Change::Type::GROWS: return true;

		// Weather changes can be viewed afterwards.
		case Change::Type::SNOW: return false;
		case Change::Type::FROSTBITE: return false;
		case Change::Type::FIRESTORM: return false;
		case Change::Type::BONEDROUGHT: return false;
		case Change::Type::DEATH: return false;
		case Change::Type::GAS: return false;
		case Change::Type::RADIATION: return false;
		case Change::Type::TEMPERATURE: return false;
		case Change::Type::HUMIDITY: return false;
		case Change::Type::CHAOS: return false;

		// News reports don't have a subject.
		case Change::Type::CHAOSREPORT: return false;

		// Phase changes don't have a subject.
		case Change::Type::YEAR: return false;
		case Change::Type::SEASON: return false;
		case Change::Type::DAYTIME: return false;
		case Change::Type::PHASE: return false;

		// Initiative changes don't have a subject.
		case Change::Type::INITIATIVE: return false;

		// Phase changes don't have a subject.
		case Change::Type::FUNDS: return false;

		// Income at night is important.
		case Change::Type::INCOME: return true;

		// Expenditure is always accompanying something else.
		case Change::Type::EXPENDITURE: return false;

		// Sleep changes don't have a subject.
		case Change::Type::SLEEPING: return false;

		// Acting notifiers always accompany something else.
		case Change::Type::ACTING: return false;
		case Change::Type::FINISHED: return false;
		case Change::Type::DISCARDED: return false;
		case Change::Type::POSTPONED: return false;
		case Change::Type::UNFINISHED: return false;

		// Order confirmations are not pannable.
		case Change::Type::ORDERED: return false;

		// Vision changes are only interesting when accompanying another
		// change, but then that change is already panned to.
		case Change::Type::VISION: return false;

		// This declaration does not have a subject.
		case Change::Type::CORNER: return false;
		case Change::Type::BORDER: return false;

		// Score counting is important.
		case Change::Type::SCORED: return true;

		// Game result changes don't have a subject.
		case Change::Type::DEFEAT: return false;
		case Change::Type::VICTORY: return false;
		case Change::Type::GAMEOVER: return false;
		case Change::Type::AWARD: return false;
	}

	return false;
}

bool Observer::isChangeSwappable(const Change& change)
{
	switch (change.type)
	{
		// No.
		case Change::Type::NONE: return false;
		case Change::Type::STARTS: return false;
		case Change::Type::MOVES: return false;

		// Non-pannable changes can be swapped if necessary,
		// but only if all other changes are also swappable.
		case Change::Type::REVEAL: return true;
		case Change::Type::OBSCURE: return true;
		case Change::Type::TRANSFORMED: return true;
		case Change::Type::CONSUMED: return true;

		// No.
		case Change::Type::SHAPES: return false;
		case Change::Type::SHAPED: return false;
		case Change::Type::SETTLES: return false;
		case Change::Type::SETTLED: return false;
		case Change::Type::EXPANDS: return false;
		case Change::Type::EXPANDED: return false;
		case Change::Type::UPGRADES: return false;
		case Change::Type::UPGRADED: return false;
		case Change::Type::CULTIVATES: return false;
		case Change::Type::CULTIVATED: return false;
		case Change::Type::CAPTURES: return false;
		case Change::Type::CAPTURED: return false;
		case Change::Type::PRODUCES: return false;
		case Change::Type::PRODUCED: return false;
		case Change::Type::ENTERED: return false;
		case Change::Type::EXITED: return false;
		case Change::Type::DIED: return false;
		case Change::Type::DESTROYED: return false;
		case Change::Type::SURVIVED: return false;
		case Change::Type::AIMS: return false;
		case Change::Type::ATTACKS: return false;
		case Change::Type::ATTACKED: return false;
		case Change::Type::TRAMPLES: return false;
		case Change::Type::TRAMPLED: return false;
		case Change::Type::SHELLS: return false;
		case Change::Type::SHELLED: return false;
		case Change::Type::BOMBARDS: return false;
		case Change::Type::BOMBARDED: return false;
		case Change::Type::BOMBS: return false;
		case Change::Type::BOMBED: return false;
		case Change::Type::FROSTBITTEN: return false;
		case Change::Type::BURNED: return false;
		case Change::Type::GASSED: return false;
		case Change::Type::IRRADIATED: return false;

		// Each niceness layer is in its own separate changesets,
		// but within the layer the order does not matter.
		case Change::Type::GROWS: return true;

		// Non-pannable changes can be swapped if necessary,
		// but only if all other changes are also swappable.
		case Change::Type::SNOW: return true;
		case Change::Type::FROSTBITE: return true;
		case Change::Type::FIRESTORM: return true;
		case Change::Type::BONEDROUGHT: return true;
		case Change::Type::DEATH: return true;
		case Change::Type::GAS: return true;
		case Change::Type::RADIATION: return true;
		case Change::Type::TEMPERATURE: return true;
		case Change::Type::HUMIDITY: return true;
		case Change::Type::CHAOS: return true;

		// No.
		case Change::Type::CHAOSREPORT: return false;
		case Change::Type::YEAR: return false;
		case Change::Type::SEASON: return false;
		case Change::Type::DAYTIME: return false;
		case Change::Type::PHASE: return false;
		case Change::Type::INITIATIVE: return false;
		case Change::Type::FUNDS: return false;

		// Income is in a changeset where the order does not matter.
		case Change::Type::INCOME: return true;

		// No.
		case Change::Type::EXPENDITURE: return false;
		case Change::Type::SLEEPING: return false;
		case Change::Type::ACTING: return false;
		case Change::Type::FINISHED: return false;
		case Change::Type::DISCARDED: return false;
		case Change::Type::POSTPONED: return false;
		case Change::Type::UNFINISHED: return false;
		case Change::Type::ORDERED: return false;

		// Non-pannable changes can be swapped if necessary,
		// but only if all other changes are also swappable.
		case Change::Type::VISION: return true;

		// This declaration does not have a subject.
		case Change::Type::CORNER: return false;
		case Change::Type::BORDER: return false;

		// Score counting is in its own changeset; the order does not matter.
		case Change::Type::SCORED: return true;

		// No.
		case Change::Type::DEFEAT: return false;
		case Change::Type::VICTORY: return false;
		case Change::Type::GAMEOVER: return false;
		case Change::Type::AWARD: return false;
	}

	return false;
}

bool Observer::isChangeDeferable(const Change& change)
{
	switch (change.type)
	{
		// No.
		case Change::Type::NONE: return false;
		case Change::Type::STARTS: return false;
		case Change::Type::MOVES: return false;
		case Change::Type::REVEAL: return false;
		case Change::Type::OBSCURE: return false;
		case Change::Type::TRANSFORMED: return false;

		// Crops consumption at night is not important to other players,
		// so we want to move them to the end to avoid delays.
		case Change::Type::CONSUMED:
		{
			if (_player == Player::OBSERVER) return false;
			else if (_player == getSubjectOwner(change)) return false;
			else return true;
		}
		break;

		// No.
		case Change::Type::SHAPES: return false;
		case Change::Type::SHAPED: return false;
		case Change::Type::SETTLES: return false;
		case Change::Type::SETTLED: return false;
		case Change::Type::EXPANDS: return false;
		case Change::Type::EXPANDED: return false;
		case Change::Type::UPGRADES: return false;
		case Change::Type::UPGRADED: return false;
		case Change::Type::CULTIVATES: return false;
		case Change::Type::CULTIVATED: return false;
		case Change::Type::CAPTURES: return false;
		case Change::Type::CAPTURED: return false;
		case Change::Type::PRODUCES: return false;
		case Change::Type::PRODUCED: return false;
		case Change::Type::ENTERED: return false;
		case Change::Type::EXITED: return false;
		case Change::Type::DIED: return false;
		case Change::Type::DESTROYED: return false;
		case Change::Type::SURVIVED: return false;
		case Change::Type::AIMS: return false;
		case Change::Type::ATTACKS: return false;
		case Change::Type::ATTACKED: return false;
		case Change::Type::TRAMPLES: return false;
		case Change::Type::TRAMPLED: return false;
		case Change::Type::SHELLS: return false;
		case Change::Type::SHELLED: return false;
		case Change::Type::BOMBARDS: return false;
		case Change::Type::BOMBARDED: return false;
		case Change::Type::BOMBS: return false;
		case Change::Type::BOMBED: return false;
		case Change::Type::FROSTBITTEN: return false;
		case Change::Type::BURNED: return false;
		case Change::Type::GASSED: return false;
		case Change::Type::IRRADIATED: return false;

		// Power growth at night is not important to other players,
		// so we want to move them to the end to avoid delays.
		case Change::Type::GROWS:
		{
			if (_player == Player::OBSERVER) return false;
			else if (_player == getSubjectOwner(change)) return false;
			else return true;
		}
		break;

		// No.
		case Change::Type::SNOW: return false;
		case Change::Type::FROSTBITE: return false;
		case Change::Type::FIRESTORM: return false;
		case Change::Type::BONEDROUGHT: return false;
		case Change::Type::DEATH: return false;
		case Change::Type::GAS: return false;
		case Change::Type::RADIATION: return false;
		case Change::Type::TEMPERATURE: return false;
		case Change::Type::HUMIDITY: return false;
		case Change::Type::CHAOS: return false;
		case Change::Type::CHAOSREPORT: return false;
		case Change::Type::YEAR: return false;
		case Change::Type::SEASON: return false;
		case Change::Type::DAYTIME: return false;
		case Change::Type::PHASE: return false;
		case Change::Type::INITIATIVE: return false;
		case Change::Type::FUNDS: return false;

		// Income at night is not important to other players,
		// so we want to move them to the end to avoid delays.
		case Change::Type::INCOME:
		{
			if (_player == Player::OBSERVER) return false;
			else if (_player == change.player) return false;
			else return true;
		}
		break;

		// No.
		case Change::Type::EXPENDITURE: return false;
		case Change::Type::SLEEPING: return false;
		case Change::Type::ACTING: return false;
		case Change::Type::FINISHED: return false;
		case Change::Type::DISCARDED: return false;
		case Change::Type::POSTPONED: return false;
		case Change::Type::UNFINISHED: return false;
		case Change::Type::ORDERED: return false;
		case Change::Type::VISION: return false;
		case Change::Type::CORNER: return false;
		case Change::Type::BORDER: return false;
		case Change::Type::SCORED: return false;
		case Change::Type::DEFEAT: return false;
		case Change::Type::VICTORY: return false;
		case Change::Type::GAMEOVER: return false;
		case Change::Type::AWARD: return false;
	}

	return false;
}

void Observer::separateIntoChunks()
{
	// If there are no pannable changes, we don't pan at all.
	bool pannable = false;

	// Most changesets are not separatable.
	bool separatable = true;

	// We need to know each change's focus.
	std::vector<ObservedChange> targets;

	// Are there any changes?
	if (_unprocessedChanges.front().empty()) return;

	// Find the focus of each change in the changeset.
	for (const Change& change : _unprocessedChanges.front())
	{
		if (isChangePannable(change))
		{
			pannable = true;
		}

		if (!isChangeSwappable(change))
		{
			separatable = false;
			break;
		}

		DEBUG_ASSERT(change.subject);
		Point center = Surface::convertCenter(change.subject.position);

		targets.emplace_back(&change, change.subject.position, center);
	}

	if (!pannable || !separatable) return;

	int w = viewportWidth();
	int h = viewportHeight();

	if (w <= 0 || h <= 0) return;

	// We will defer some changes to a final chunk.
	std::vector<ObservedChange> deferchunk;

	// Filter out deferable changes and changes on the same position.
	{
		// We need to keep groups with the same position together.
		auto start = targets.begin();
		auto end = targets.begin();
		bool deferable = false;

		while (start != targets.end() && end != targets.end())
		{
			if (isChangeDeferable(*(end->change)))
			{
				deferable = true;
			}

			end++;

			if (end == targets.end()
				|| end->focus != start->focus)
			{
				if (deferable)
				{
					// Move the group from targets into the deferchunk.
					std::move(start, end, std::back_inserter(deferchunk));
					end = targets.erase(start, end);
				}

				// Start a new group.
				start = end;
				deferable = false;
			}
		}
	}

	// SmartPan
	std::stable_sort(targets.begin(), targets.end(),
			[](const ObservedChange& a, const ObservedChange& b){

		return (a.point.yahoo < b.point.yahoo);
	});

	int totalh = targets.back().point.yahoo - targets.front().point.yahoo;
	int rowcount = totalh / h + 1;

	int startofrow = 0;
	for (int row = 0; row < rowcount; row++)
	{
		if (startofrow >= (int) targets.size()) break;
		int top = targets[startofrow].point.yahoo;
		int rowh = ((row + 1 < rowcount) ? (totalh / rowcount) : h);

		int endofrow = targets.size();
		while (endofrow >= (int) targets.size()
			|| targets[endofrow].point.yahoo >= top + rowh
			|| (endofrow + 1 < (int) targets.size()
				&& targets[endofrow].focus == targets[endofrow + 1].focus))
		{
			endofrow--;
		}

		std::stable_sort(targets.begin() + startofrow,
				targets.begin() + endofrow + 1,
				[](const ObservedChange& a, const ObservedChange& b){

			return (a.point.xenon < b.point.xenon);
		});

		int totalw = targets[endofrow].point.xenon
			- targets[startofrow].point.xenon;
		int colcount = totalw / w + 1;

		std::vector<std::vector<ObservedChange>> rowofchunks;

		int startofcol = startofrow;
		for (int col = 0; col < colcount; col++)
		{
			if (startofcol > endofrow) break;
			int left = targets[startofcol].point.xenon;
			int colw = ((col + 1 < colcount) ? (totalw / colcount) : w);

			int endofcol = endofrow;
			while (endofcol >= (int) targets.size()
				|| endofcol > endofrow
				|| targets[endofcol].point.xenon >= left + colw
				|| (endofcol + 1 < (int) targets.size()
					&& targets[endofcol].focus == targets[endofcol + 1].focus))
			{
				endofcol--;
			}

			std::stable_sort(targets.begin() + startofcol,
					targets.begin() + endofcol + 1,
					[](const ObservedChange& a, const ObservedChange& b){

				return (a.point.yahoo < b.point.yahoo);
			});

			rowofchunks.emplace_back(targets.begin() + startofcol,
				targets.begin() + endofcol + 1);

			startofcol = endofcol + 1;
		}

//		if (row % 2 == 1)
//		{
//			std::reverse(rowofchunks.begin(), rowofchunks.end());
//		}

		for (auto& chunk : rowofchunks)
		{
			_unprocessedChunks.push(chunk);
		}

		startofrow = endofrow + 1;
	}

	_unprocessedChunks.push(deferchunk);
}

void Observer::panCamera(bool backToBase)
{
	Point min = _level.bottomrightPoint();
	Point max = _level.topleftPoint();
	std::vector<Cell> foci;

	if (backToBase)
	{
		for (Square& square : _level)
		{
			if (!((square.tile() && square.tile().owner == _player)
				|| (square.ground() && square.ground().owner == _player)
				|| (square.air() && square.air().owner == _player))) continue;
			foci.emplace_back(square.cell());
		}
	}
	else if (!_unprocessedChunks.empty())
	{
		for (const ObservedChange& change : _unprocessedChunks.front())
		{
			if (isChangeDeferable(*(change.change))) continue;
			fillFoci(*(change.change), foci);
		}
	}
	else if (!_unprocessedChanges.empty())
	{
		for (const Change& change : _unprocessedChanges.front())
		{
			fillFoci(change, foci);
		}
	}
	else if (_selectsquare != nullptr)
	{
		foci.emplace_back(_selectsquare->cell());
	}
	else
	{
		LOGW << "Tried to pan with nothing to pan toward.";
		DEBUG_ASSERT(false);
		return;
	}

	// There might be no pannable foci.
	if (foci.size() == 0) return;

	for (Cell focus : foci)
	{
		Point center = Surface::convertCenter(focus.pos());
		min.xenon = std::min(min.xenon, center.xenon);
		max.xenon = std::max(max.xenon, center.xenon);
		min.yahoo = std::min(min.yahoo, center.yahoo);
		max.yahoo = std::max(max.yahoo, center.yahoo);
	}
	int panX = (min.xenon + max.xenon) / 2;
	int panY = (min.yahoo + max.yahoo) / 2;
	_camerafocus->pan(Point(panX, panY));
	LOGV << "Panning camera to (" << panX << ", " << panY << ")";
}

void Observer::fillFoci(const Change& change, std::vector<Cell>& foci)
{
	// If the target of a BOMBARD order lies outside of the player's vision,
	// there is no BOMBARDED change but we still want to highlight the target.
	// We can detect the preceding ACTING change and look up the order.
	if (change.type == Change::Type::ACTING)
	{
		fillActingFoci(change.subject, foci);
		// Continue below.
	}

	if (!isChangePannable(change)) return;

	DEBUG_ASSERT(change.subject);
	foci.emplace_back(_level.index(change.subject.position));

	switch (change.type)
	{
		case Change::Type::NONE:
		{
			// Some NONE changes, such as those generated by postponed GUARD
			// orders, have a target to help animate this.
			if (change.target)
			{
				foci.emplace_back(_level.index(change.target.position));
			}
		}
		break;

		case Change::Type::STARTS:
		case Change::Type::MOVES:
		case Change::Type::EXPANDS:
		case Change::Type::AIMS:
		case Change::Type::ATTACKS:
		case Change::Type::SHELLS:
		{
			DEBUG_ASSERT(change.target);
			foci.emplace_back(_level.index(change.target.position));
		}
		break;

		case Change::Type::ATTACKED:
		case Change::Type::SHELLED:
		{
			DEBUG_ASSERT(change.attacker.unittype != UnitType::NONE);
			foci.emplace_back(_level.index(change.attacker.position));
		}
		break;

		case Change::Type::GROWS:
		{
			Square* square = _level.cell(change.subject.position);
			TileType tiletype = square->tile().type;
			if (_bible.tileNeedsNiceness(tiletype))
			{
				for (Cell index : _level.area(square->cell(), 1, 2))
				{
					foci.emplace_back(index);
				}
				// TODO highlight snowed, grassy and natural tiles?
			}
			else if (_bible.tileNeedsLabor(tiletype))
			{
				for (Cell index : _level.area(square->cell(), 1, 2))
				{
					foci.emplace_back(index);
				}
				// TODO highlight laboring tiles?
			}
			else if (_bible.tileNeedsEnergy(tiletype))
			{
				for (Cell index : _level.area(square->cell(), 1, 2))
				{
					foci.emplace_back(index);
				}
				// TODO highlight energizing tiles?
			}
		}
		break;

		default:
		break;
	}
}

void Observer::prepareAnimationGroup()
{
	if (auto group = _animations.lock())
	{
		// Some figures may need to know if they belong to the player that is
		// actually playing.
		group->player = _player;

		// If we want to do camera shake, we need the camera focus.
		group->camerafocus = _camerafocus;

		// Wait after panning for the player's eyes to focus.
		group->delay += 0.500f;

		// Prevent flashlights if lighting is disabled.
		group->skiplighting = _skiplighting;

		// Prevent Frostbite hazard markers if frostbite is used to mark
		// the "Chilled" status effect on units in Spring.
		group->coldfeet = (_bible.frostbiteGivesColdFeet()
			&& _bible.chaosMinFrostbite(_season) < 0);
	}
}

void Observer::processChanges()
{
	LOGV << "Processing set of changes";

	// Create an animation group. Remember it so we can wait for it to finish
	// before processing further, and so we can use it in our own animations.
	std::shared_ptr<AnimationGroup> group(
		(_skipanimations) ? nullptr : new AnimationGroup());
	if (group)
	{
		_animations = group;
		prepareAnimationGroup();
	}

	// Process some changes.
	if (_unprocessedChunks.empty())
	{
		{
			std::vector<Cell> foci;
			for (const Change& change : _unprocessedChanges.front())
			{
				fillFoci(change, foci);
			}
			setActionSpotlight(std::move(foci));
		}

		for (const Change& change : _unprocessedChanges.front())
		{
			processChange(group, change);
		}
		_unprocessedChanges.pop();
	}
	else
	{
		{
			std::vector<Cell> foci;
			for (const ObservedChange& change : _unprocessedChunks.front())
			{
				fillFoci(*(change.change), foci);
			}
			setActionSpotlight(std::move(foci));
		}

		for (const ObservedChange& change : _unprocessedChunks.front())
		{
			processChange(group, *(change.change));
		}
		_unprocessedChunks.pop();
		if (_unprocessedChunks.empty()) _unprocessedChanges.pop();
	}

	// Virtualize all the hitstops in the Mixer.
	if (group) Mixer::get()->virtualizeHitstops(group->hitstops);
}

void Observer::processChange(const std::shared_ptr<AnimationGroup> group,
	const Change& change)
{
	LOGV << "Processing change: " << TypeEncoder(&_bible) << change;
	switch (change.type)
	{
		case Change::Type::NONE:
		{
			switch (change.notice)
			{
				case Notice::LACKINGMONEY:
				{
					blinkWallet();
				}
				break;

				default: break;
			}
		}
		break;

		case Change::Type::INITIATIVE:
		{
			handleInitiative(change);
		}
		break;

		case Change::Type::FUNDS:
		{
			handleWallet(change);
			panCamera(/*ToBase=*/true);
		}
		break;

		case Change::Type::INCOME:
		case Change::Type::EXPENDITURE:
		{
			handleWallet(change);
		}
		break;

		case Change::Type::SLEEPING:
		{
			// Nothing to display.
		}
		break;

		case Change::Type::ACTING:
		{
			// It is possible that the subject has been killed earlier and that
			// afterwards another unit move into its place. We do not want to
			// select that other unit, but we do want to highlight the order.
			// TODO this does not work if that unit does have an order (#879)
			if (!hasOrder(change.subject))
			{
				deselectActiveOrder();
				selectActiveOrder(change.subject);
				break;
			}

			_selectsquare = _level.cell(change.subject.position);
			_selector = change.subject;
			selectUnitOrTile();
			// If the unit has been killed, the selectUnitOrTile() won't cause
			// that unit's order to be highlighted, so we highlight it (again).
			selectActiveOrder(change.subject);
		}
		break;

		case Change::Type::FINISHED:
		case Change::Type::DISCARDED:
		case Change::Type::POSTPONED:
		case Change::Type::UNFINISHED:
		{
			// These changes are used to confirm the old-orderlist.
			handleOrder(change);

			addAnimation(Animation(group, [this](float) {

				deselectUnitOrTile();
				disableActiveOrder();

			}, 0, group ? group->delay : 0));
		}
		break;

		case Change::Type::ORDERED:
		{
			// These changes are used to confirm the new-orderlist.
			handleOrder(change);
		}
		break;

		case Change::Type::SCORED:
		{
			// TODO increase a running score counter or something
		}
		break;

		case Change::Type::DEFEAT:
		{
			if (change.player == _player)
			{
				Mixer::get()->queue(Clip::Type::FAIL, 0.0f);

				_score = change.score;
				_defeated = true;
				if (!_game.test())
				{
					_popuplayout["defeat"]["subtext"]["score"].setText(
						std::to_string(_score));
					_popuplayout["defeat"].bear();
				}
				if (_popuplayout["quit"].born())
				{
					_popuplayout["quit"].kill();
				}

				_popuplayout["quit"]["title"].setText(
					_("Quit?"));
				_popuplayout["quit"]["text"].setText(
					_("Are you sure you want to quit?"));
				_popuplayout["quit"]["buttons"]["quit"].setText(
					_("quit"));
				_popuplayout["quit"].unfixWidth();
				_popuplayout["quit"].settleWidth();
				_popuplayout["quit"].fixWidth();
				_popuplayout.settle();

				InterfaceElement& menu = getMenu();
				menu["buttons"]["quit"].setText(
					_("Quit"));
			}
			else if (_player == Player::OBSERVER && isChallenge()
				&& change.player == Player::RED)
			{
				if (!_game.test())
				{
					_popuplayout["defeat"]["subtext"]["score"].setText(
						std::to_string(change.score));
					_popuplayout["defeat"].bear();
				}
				if (_popuplayout["quit"].born())
				{
					_popuplayout["quit"].kill();
				}
			}

			std::string name = ::colorPlayerName(change.player);
			{
				InterfaceElement& dbox = getDiplomacyBox();
				InterfaceElement& plist = dbox["players"];
				std::string index = ::stringify(change.player);
				if (plist.contains(index))
				{
					plist[index]["defeated"].show();
					name = plist[index]["name"].text();
				}
			}

			{
				message(::format(
					// TRANSLATORS: The argument is a username.
					_("%s was defeated."),
					name.c_str()));
			}
		}
		break;

		case Change::Type::VICTORY:
		{
			if (change.player == _player)
			{
				Mixer::get()->queue(Clip::Type::WIN, 0.0f);

				_score = change.score;
				if (!_game.test())
				{
					_popuplayout["victory"]["subtext"]["score"].setText(
						std::to_string(_score));
					_popuplayout["victory"].bear();
				}
				if (_popuplayout["quit"].born())
				{
					_popuplayout["quit"].kill();
				}

				_popuplayout["quit"]["title"].setText(
					_("Quit?"));
				_popuplayout["quit"]["text"].setText(
					_("Are you sure you want to quit?"));
				_popuplayout["quit"]["buttons"]["quit"].setText(
					_("quit"));
				_popuplayout["quit"].unfixWidth();
				_popuplayout["quit"].settleWidth();
				_popuplayout["quit"].fixWidth();
				_popuplayout.settle();

				InterfaceElement& menu = getMenu();
				menu["buttons"]["quit"].setText(
					_("Quit"));
			}
			else if (_player == Player::OBSERVER
				&& (!isChallenge() || change.player == Player::RED))
			{
				_score = change.score;
				if (!_game.test())
				{
					_popuplayout["victory"]["subtext"]["score"].setText(
						std::to_string(_score));
					_popuplayout["victory"].bear();
				}
				if (_popuplayout["quit"].born())
				{
					_popuplayout["quit"].kill();
				}
			}

			std::string name = ::colorPlayerName(change.player);
			{
				InterfaceElement& dbox = getDiplomacyBox();
				InterfaceElement& plist = dbox["players"];
				std::string index = ::stringify(change.player);
				if (plist.contains(index))
				{
					name = plist[index]["name"].text();
				}
			}

			{
				message(::format(
					// TRANSLATORS: The first argument is a username.
					// The second argument is a numerical score.
					_("%s was victorious. Score: %d."),
					name.c_str(),
					change.score));
			}
		}
		break;

		case Change::Type::GAMEOVER:
		{
			_arranger.fadeOutMusic();

			_gameover = true;
			if (!_skiplighting) light(0.4, 1);

			if (_player == Player::OBSERVER
				&& !_popuplayout["defeat"].born()
				&& !_popuplayout["victory"].born())
			{
				_score = change.score;
				if (!_game.test())
				{
					_popuplayout["defeat"]["subtext"]["score"].setText(
						std::to_string(_score));
					_popuplayout["defeat"].bear();
				}
				if (_popuplayout["quit"].born())
				{
					_popuplayout["quit"].kill();
				}
			}

			if (_player != Player::OBSERVER)
			{
				_popuplayout["quit"]["title"].setText(
					_("Quit?"));
				_popuplayout["quit"]["text"].setText(
					_("Are you sure you want to quit?"));
				_popuplayout["quit"]["buttons"]["quit"].setText(
					_("quit"));
				_popuplayout["quit"].unfixWidth();
				_popuplayout["quit"].settleWidth();
				_popuplayout["quit"].fixWidth();
				_popuplayout.settle();

				InterfaceElement& menu = getMenu();
				menu["buttons"]["quit"].setText(
					_("Quit"));
			}
		}
		break;

		case Change::Type::AWARD:
		{
			for (auto str : {stringref("defeat"), stringref("victory")})
			{
				int maxstars = 3;
				{
					auto& briefing = getMissionBox();
					if (briefing.contains("lines"))
					{
						if (!briefing["lines"].contains("2"))
						{
							maxstars = 1;
						}
						else if (!briefing["lines"].contains("3"))
						{
							maxstars = 2;
						}
					}
				}
				int amount = std::min((int) change.level, 3);
				maxstars = std::max(amount, maxstars);
				if (strncmp(str, "defeat", 1) == 0 && _player == Player::OBSERVER
					&& isChallenge() && change.player != Player::RED)
				{
					amount = 0;
				}

				InterfaceElement& element = _popuplayout[str];
				if (!element.contains("stars"))
				{
					element.add("stars", new HorizontalLayout());
					element["stars"].add("filler1", new HorizontalFiller());
					for (int i = 1; i <= maxstars; i++)
					{
						std::string index = std::to_string(i);
						element["stars"].add(index, new Image("effects/star1"));

						auto& icon = element["stars"][index];
						icon.setColor(0, Paint::blend(ColorName::FRAME600,
							ColorName::SHINEBLEND, 0.2f));
						icon.setColor(1, ColorName::FRAME600);
						icon.setColor(2, Paint::blend(ColorName::FRAME600,
							ColorName::SHADEBLEND));
						icon.setPowerColor(0, Paint::blend(ColorName::STAR,
							ColorName::SHINEBLEND));
						icon.setPowerColor(1, ColorName::STAR);
						icon.setPowerColor(2, Paint::blend(ColorName::STAR,
							ColorName::SHADEBLEND, 0.2f));
						icon.setMarginHorizontal(8 * InterfaceElement::scale());
					}
					element["stars"].add("filler2", new HorizontalFiller());
					element["stars"].setMargin(5 * InterfaceElement::scale());
					element.unfixHeight();
					element.settleHeight();
					element.fixHeight();
				}
				for (int i = 1; i <= amount; i++)
				{
					std::string index = std::to_string(i);
					auto& icon = element["stars"][index];
					icon.power();
				}
			}
			_popuplayout.place(_popuplayout.topleft());
		}
		break;

		case Change::Type::CHAOSREPORT:
		{
			_arranger.fadeOutNightAmbience();
			Mixer::get()->queue(Clip::Type::CHAOSREPORT, 0.0f);
			Mixer::get()->queue(Clip::Type::HEADSUP, 0.0f);

			if (change.level == 1)
			{
				fillReportBox(
					_(""
					"GLOBAL WARMING WORSENS"
					""),
					_(""
					"HEAT WAVES CAUSE AVERAGE HUMIDITY TO DROP"
					""),
					_(""
					"DROUGHT CURBS GROWTH OF TREES AND CROPS, WARN BOTANISTS"
					""),
					_(""
					"FROSTBITE EXPECTED TO TERRORIZE OPEN FIELDS IN WINTER"
					""));
			}
			else if (change.level == 2)
			{
				fillReportBox(
					_(""
					"GLOBAL WARMING WORSENS"
					""),
					_(""
					"DRY AREAS WITHOUT VEGETATION SUFFER FROM DESERTIFICATION"
					""),
					_(""
					"DESERT SAND UNSUITABLE FOR BUILDING, SAY CONTRACTORS"
					""),
					_(""
					"SUMMER FIRESTORM ALERT LIKELY"
					""));
			}
			else if (change.level == 3)
			{
				fillReportBox(
					_(""
					"GLOBAL WARMING WORSENS"
					""),
					_(""
					"EXTREME DROUGHT WEAKENS TROOPS IN DESERTS"
					""),
					_(""
					"DROUGHT LEAVES TANKS MORE VULNERABLE TO EXTREME WEATHER,"
					" SAY MECHANICS"
					""),
					_(""
					"DEFORESTATION SPREADS AS WAR CONTINUES"
					""));
			}
			else if (change.level == 4)
			{
				fillReportBox(
					_(""
					"GLOBAL WARMING WORSENS"
					""),
					_(""
					"EXTREME DROUGHT ALSO AFFECTS STONE, ASPHALT AND RUBBLE"
					""),
					_(""
					"STRUCTURAL INTEGRITY OF BUILDINGS FALTERS, WARN ENGINEERS"
					""),
					_(""
					"INCREASED RISK OF FIRESTORM IN SUMMER"
					""));
			}
			else if (change.level >= 5)
			{
				fillReportBox(
					_(""
					"GLOBAL WARMING WORSENS"
					""),
					_(""
					"CITIES VULNERABLE TO INFERNO AS HEATWAVE CONTINUES"
					""),
					_(""
					"THE END OF DAYS HAS ARRIVED, SAY CULT LEADERS"
					""),
					_(""
					"CITIZENS IN AFFECTED REGIONS ARE ADVISED TO EVACUATE"
					" IMMEDIATELY"
					""));
			}

			updateReportBoxDate();
			updateReportBoxPrice();

			if (!_skipanimations)
			{
				_camerafocus->enableKeys();
				InterfaceElement& viewboxholder = getViewBoxHolder();
				viewboxholder.setTag("report");
			}
		}
		break;

		case Change::Type::YEAR:    setYear(   change.year,    group); break;
		case Change::Type::SEASON:  setSeason( change.season,  group); break;
		case Change::Type::DAYTIME: setDaytime(change.daytime, group); break;

		case Change::Type::PHASE:
		{
			setPhase(change.phase, group);

			// The orderlists may need resetting.
			handleOrder(change);
		}
		break;

		default: break;
	}

	_board.enact(change);
	_level.enact(change, group);

	switch (change.type)
	{
		case Change::Type::REVEAL:
		case Change::Type::CHAOS:
		{
			updateReportBoxPrice();
		}
		break;

		case Change::Type::CORNER:
		{
			_camerafocus->set(_level.centerPoint());
			_camerafocus->load(_level.topleftPoint(),
				_level.bottomrightPoint());
		}
		break;

		case Change::Type::BORDER:
		{
			setupLight(group);

			if (isChallenge() && _player != Player::OBSERVER)
			{
				// Open the mission box.
				_camerafocus->enableKeys();
				InterfaceElement& viewboxholder = getViewBoxHolder();
				viewboxholder.setTag("mission");
			}
		}
		break;

		default: break;
	}
}

void Observer::handleInitiative(const Change& /**/)
{}

void Observer::handleWallet(const Change& change)
{
	InterfaceElement& dbox = getDiplomacyBox();
	InterfaceElement& plist = dbox["players"];
	std::string index = ::stringify(change.player);

	if (!plist.contains(index)) return;
	if (!plist[index].contains("money")) return;

	int money;
	try
	{
		money = stoi(plist[index]["money"].text());
	}
	catch (const std::logic_error& ignored)
	{
		money = 0;
	}

	money += change.money;

	plist[index]["money"].setText(std::to_string(money));
}

void Observer::handleOrder(const Change& /**/)
{}

void Observer::setYear(int value, const std::shared_ptr<AnimationGroup>)
{
	_year = value;
}

inline std::string formatDate(Season season, int year)
{
	switch (season)
	{
		case Season::SPRING:
		{
			return ::format(
				// TRANSLATORS: The argument is a year number.
				_("Spring, Year %d"),
				year);
		}
		break;
		case Season::SUMMER:
		{
			return ::format(
				// TRANSLATORS: The argument is a year number.
				_("Summer, Year %d"),
				year);
		}
		break;
		case Season::AUTUMN:
		{
			return ::format(
				// TRANSLATORS: The argument is a year number.
				_("Autumn, Year %d"),
				year);
		}
		break;
		case Season::WINTER:
		{
			return ::format(
				// TRANSLATORS: The argument is a year number.
				_("Winter, Year %d"),
				year);
		}
		break;
	}
	return "error";
}

void Observer::setSeason(Season value, const std::shared_ptr<AnimationGroup>)
{
	_season = value;
	InterfaceElement& seasontextfield = getSeasonTextField();
	seasontextfield.setText(formatDate(_season, _year));

	_arranger.newSeasonPhase(_season, Phase::PLANNING);
}

void Observer::setDaytime(Daytime value, const std::shared_ptr<AnimationGroup> group)
{
	_daytime = value;

	setLight(group);

	switch (_daytime)
	{
		//Day time
		case Daytime::LATE:
		{
			// There are no pannable changes in the Weather phase.
			_camerafocus->stopActing();
			_arranger.fadeOutNightAmbience();
		}
		break;

		//Night time
		case Daytime::EARLY:
			_arranger.fadeInNightAmbience();
		break;
	}

	updatePhaseGraphic();
}

void Observer::setPhase(Phase value,
	const std::shared_ptr<AnimationGroup> group)
{
	_phase = value;

	setLight(group);

	switch (_phase)
	{
		case Phase::GROWTH:
		break;
		case Phase::RESTING:
		{
			_camerafocus->stopActing();
			if (!_gameover)
			{
				_cursor->setState(Cursor::State::WAITING);
			}
			else
			{
				_cursor->setState(Cursor::State::ACTIVE);
			}
			if (_game.planningTimeTotal() > 0)
			{
				_pauseJuice = ((int) _game.planningTimeTotal()) / 5;
				if (_pauseJuice < 5) _pauseJuiceDisplayMax = _pauseJuice;
				else _pauseJuiceDisplayMax = _pauseJuice * 5 / 6;
			}
			else _pauseJuice = 999;
		}
		break;
		case Phase::PLANNING:
		{
			deselectUnitOrTile();
			_camerafocus->stopActing();
			_cursor->setState(Cursor::State::ACTIVE);
			if (_game.planningTimeTotal() > 0)
			{
				InterfaceElement& statusbar = getStatusBar();
				statusbar.enable();
				statusbar["timer"].show();
				statusbar["timer"].fixWidth(0);
				statusbar.settle();
			}
			touchReadyButton();

			// queue the audio
 			if (group && !_silentplanningbell)
			{
				Mixer::get()->queue(Clip::Type::PLANNING, 0);
			}
		}
		break;
		case Phase::STAGING:
		{
			_cursor->setState(Cursor::State::WAITING);
			if (_game.planningTimeTotal() > 0)
			{
				InterfaceElement& statusbar = getStatusBar();
				statusbar.disable();
				statusbar["timer"].hide();
			}
			touchReadyButton();
		}
		break;
		case Phase::ACTION:
		{
			deselectUnitOrTile();
			_camerafocus->startActing();
			_cursor->setState(Cursor::State::BUSY);

			if(_unprocessedChanges.size() > 19) _arranger.newSeasonPhase(_season, Phase::ACTION);
		}
		break;
		case Phase::DECAY:
		break;
	}

	updatePhaseGraphic();
}

void Observer::updatePhaseGraphic()
{
	InterfaceElement& graphic = getPhaseGraphic();

	// Night and noon are completely transparent by default and become visible
	// when powered.
	graphic["image"]["night"].powerIf(_phase == Phase::GROWTH
		&& _daytime == Daytime::EARLY);
	graphic["image"]["noon"].powerIf(_phase == Phase::ACTION);

	// Dawn is replaced by dusk while noon and night are still opaque.
	graphic["image"]["dusk"].bearIf(_phase == Phase::DECAY
		|| (_phase == Phase::GROWTH && _daytime == Daytime::EARLY));

	InterfaceElement& moon = graphic["image"]["moon"];
	InterfaceElement& hand_top = graphic["image"]["hand_top"];
	InterfaceElement& sun = graphic["image"]["sun"];
	InterfaceElement& circle = graphic["image"]["circle"];
	InterfaceElement& hand_mid = graphic["image"]["hand_mid"];
	InterfaceElement& soldier = graphic["image"]["soldier"];
	InterfaceElement& hand_bot = graphic["image"]["hand_bot"];
	InterfaceElement& skulls = graphic["image"]["skulls"];
	InterfaceElement& grass = graphic["image"]["grass"];

	// Hide these elements based on the true phase, because their presence
	// changes which element the mouse is currently hovering.
	hand_mid.bearIf(_phase != Phase::ACTION);
	hand_top.bearIf(_phase == Phase::PLANNING);
	skulls["blood"].bearIf(_phase == Phase::DECAY);

	// Alter which phase is highlighted if the user is hovering an element.
	// This is hardcoded because (a) all elements are part of a StackedLayout
	// and thus part of the same Proximity layer, (b) each element is itself
	// a StackedLayout containing multiple images (e.g. base and blood), and we
	// do not want an image to be highlighted separately from the others, and
	// (c) multiple elements represent the same phase and we do not want one of
	// them to be highlighted separately from the others.
	bool planning = false;
	bool action = false;
	bool decay = false;
	bool night = false;
	bool weather = false;
	if (getWallet().hovered())
	{
		// The wallet partially obscures the phase graphic, but the mouse works
		// in proximity layers, so we need to do this manually.
	}
	else if (moon.hovered())
	{
		night = true;
	}
	else if (hand_top.hovered())
	{
		planning = true;
	}
	else if (sun.hovered())
	{
		weather = true;
	}
	else if (circle.hovered())
	{
		// Default.
	}
	else if (hand_mid.hovered())
	{
		planning = true;
	}
	else if (soldier.hovered())
	{
		action = true;
	}
	else if (hand_bot.hovered())
	{
		planning = true;
	}
	else if (skulls.hovered())
	{
		decay = true;
	}
	else if (grass.hovered())
	{
		action = true;
	}

	if (!planning && !action && !decay && !night && !weather)
	{
		planning = (_phase == Phase::PLANNING);
		action = (_phase == Phase::ACTION);
		decay = (_phase == Phase::DECAY);
		night = (_phase == Phase::GROWTH && _daytime == Daytime::EARLY);
		weather = (_phase == Phase::GROWTH && _daytime == Daytime::LATE);
	}

	hand_top.enableIf(planning);
	hand_mid.enableIf(planning);
	hand_bot.enableIf(planning);
	soldier["shadow"].bearIf(planning);
	soldier.enableIf(action || planning);
	grass.enableIf(action);
	skulls.enableIf(decay);
	moon.enableIf(night);
	sun.enableIf(!night);
	sun.powerIf(weather);

	bool hovered = graphic.hovered() && !getWallet().hovered();
	graphic["tooltip_planning"].bearIf(planning && hovered);
	graphic["tooltip_action"].bearIf(action && hovered);
	graphic["tooltip_decay"].bearIf(decay && hovered);
	graphic["tooltip_night"].bearIf(night && hovered);
	graphic["tooltip_weather"].bearIf(weather && hovered);
}

void Observer::setLight(const std::shared_ptr<AnimationGroup> group)
{
	if (_skiplighting || _gameover) return;

	if (group)
	{
		addAnimation(Animation(group, [this](float /**/) {

			resetActionSpotlight();
		}, 0, group->delay));
	}
	else resetActionSpotlight();

	switch (_daytime)
	{
		case Daytime::EARLY:
		{
			switch (_phase)
			{
				case Phase::GROWTH:   light(0.2, 1); pause(group, 2); break;
				case Phase::RESTING:  light(0.6, 1);                  break;
				case Phase::PLANNING: light(0.7, 1);                  break;
				case Phase::STAGING:  light(0.7, 1);                  break;
				case Phase::ACTION:   light(0.8, 1);                  break;
				case Phase::DECAY:    light(1.0, 1); pause(group, 1); break;
			}
		}
		break;

		case Daytime::LATE:
		{
			switch (_phase)
			{
				case Phase::GROWTH:   light(1.1, 1); pause(group, 1); break;
				case Phase::RESTING:  light(1.1, 1); break;
				case Phase::PLANNING: light(1.0, 1); break;
				case Phase::STAGING:  light(1.0, 1); break;
				case Phase::ACTION:   light(0.8, 1); break;
				case Phase::DECAY:    light(0.6, 1); pause(group, 1); break;
			}
		}
		break;
	}
}

void Observer::light(float light, float duration)
{
	_level.setLight(light, duration);
}

void Observer::setupLight(const std::shared_ptr<AnimationGroup> group)
{
	// Even if (_skiplighting || _gameover).

	for (Square& square : _level)
	{
		square.setSpotlight(1, 0);
	}
	_level.setBorderSpotlight(1, 0);

	// TODO Magic number 0.500f matches the same in prepareAnimationGroup().
	_level.setLight(1.0, 0.600f, 0.500f);
	pause(group, 0.600f + 0.500f);
}

void Observer::setActionSpotlight(std::vector<Cell> foci)
{
	if (_skiplighting || _gameover) return;
	if (foci.empty()) return;

	for (Square& square : _level)
	{
		float spot = 1.0f;
		for (Cell focus : foci)
		{
			if (focus == square.cell())
			spot = 1.2f;
		}
		square.setSpotlight(spot, 0.500f);
	}
	_level.setBorderSpotlight(1.0f, 0.500f);
}

void Observer::setTargetSpotlight(std::vector<Cell> foci)
{
	if (_skiplighting || _gameover) return;
	// Even if (foci.empty()).

	for (Square& square : _level)
	{
		float spot = 0.8f;
		for (Cell focus : foci)
		{
			if (focus == square.cell())
			spot = 1.0f;
		}
		square.setSpotlight(spot, 0.100f);
	}
	_level.setBorderSpotlight(0.8f, 0.100f);
}

void Observer::resetActionSpotlight()
{
	// Even if (_skiplighting || _gameover).

	for (Square& square : _level)
	{
		square.setSpotlight(1, 0.500f);
	}
	_level.setBorderSpotlight(1, 0.500f);
}

void Observer::resetTargetSpotlight()
{
	// Even if (_skiplighting || _gameover).

	for (Square& square : _level)
	{
		square.setSpotlight(1, 0.100f);
	}
	_level.setBorderSpotlight(1, 0.100f);
}

void Observer::deselectUnitOrTile()
{
	for (Square& square : _level)
	{
		if (square.tile())   square.tile().deselect();
		if (square.ground()) square.ground().deselect();
		if (square.air())    square.air().deselect();
		if (square.bypass()) square.bypass().deselect();
	}

	deselectActiveOrder();

	_selectsquare = nullptr;
	_selector = Descriptor();

	clearGuides();
}

Order Observer::findOrder(const Descriptor&)
{
	return Order();
}

bool Observer::hasOrder(const Descriptor&)
{
	return false;
}

void Observer::selectUnitOrTile()
{
	switch (_selector.type)
	{
		case Descriptor::Type::GROUND:
		case Descriptor::Type::AIR:
		{
			Unit& unit = _selectsquare->unit(_selector.type);
			if (unit && (unit.owner == _player || _player == Player::OBSERVER))
			{
				unit.select();
				visualizeOrder(findOrder(_selector));
				deselectActiveOrder();
				selectActiveOrder(_selector);
			}
			else
			{
				_selectsquare = nullptr;
				_selector = Descriptor();
			}
		}
		break;

		case Descriptor::Type::TILE:
		{
			Tile& tile = _selectsquare->tile();
			if (tile && (tile.owner == _player || _player == Player::OBSERVER))
			{
				tile.select();
				visualizeOrder(findOrder(_selector));
				deselectActiveOrder();
				selectActiveOrder(_selector);
			}
			else
			{
				_selectsquare = nullptr;
				_selector = Descriptor();
			}
		}
		break;

		case Descriptor::Type::BYPASS:
		{
			LOGE << "Cannot select bypass units";
			DEBUG_ASSERT(_selector.type != Descriptor::Type::BYPASS);
		}
		break;

		default: break;
	}
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

std::string Observer::unitName(const UnitType& unittype)
{
	std::string unitnames[array_size(KNOWN_UNIT_TYPES)] = {
		"-", // none
		_("Rifleman"),
		_("Gunner"),
		_("Sapper"),
		_("Tank"),
		_("Settler"),
		_("Militia"),
		_("Zeppelin"),
	};

	for (size_t i = 0; i < array_size(KNOWN_UNIT_TYPES); i++)
	{
		if (_bible.typeword(unittype) == KNOWN_UNIT_TYPES[i])
		{
			return unitnames[i];
		}
	}

	LOGW << "Untranslated unit type: " << _bible.typeword(unittype);
	return ::toupper1(::stringify(_bible.typeword(unittype)));
}

std::string Observer::tileName(const TileType& tiletype)
{
	std::string tilenames[array_size(KNOWN_TILE_TYPES)] = {
		"-", // none
		_("Grass"),
		_("Dirt"),
		_("Desert"),
		_("Rubble"),
		_("Ridge"),
		_("Mountain"),
		_("Water"),
		_("Forest"),
		_("City"),
		_("Town"),
		_("Outpost"),
		_("Industry"),
		_("Barracks"),
		_("Airfield"),
		_("Farm"),
		_("Soil"),
		_("Crops"),
		_("Trenches"),
	};

	for (size_t i = 0; i < array_size(KNOWN_TILE_TYPES); i++)
	{
		if (_bible.typeword(tiletype) == KNOWN_TILE_TYPES[i])
		{
			return tilenames[i];
		}
	}

	LOGW << "Untranslated tiletype type: " << _bible.typeword(tiletype);
	return ::toupper1(::stringify(_bible.typeword(tiletype)));
}

std::string Observer::description(const Order& order)
{
	switch (order.type)
	{
		case Order::Type::GUARD:     return _("Lockdown");
		case Order::Type::FOCUS:     return _("Focus");
		case Order::Type::LOCKDOWN:  return _("Lockdown");
		case Order::Type::SHELL:     return _("Shell");
		case Order::Type::BOMBARD:   return _("Bombard");
		case Order::Type::BOMB:      return _("Drop Gas");
		case Order::Type::CAPTURE:   return _("Capture");
		case Order::Type::SHAPE:     return _("Dig Trenches");

		case Order::Type::SETTLE:
		{
			return ::format(
				// TRANSLATORS: The argument is a tiletype.
				_("Settle %s"),
				tileName(order.tiletype).c_str());
		}
		break;

		case Order::Type::EXPAND:
		{
			return ::format(
				// TRANSLATORS: The argument is a tiletype.
				_("Build %s"),
				tileName(order.tiletype).c_str());
		}
		break;

		case Order::Type::UPGRADE:
		{
			if (order.tiletype == TileType::NONE)
			{
				return _("Upgrade");
			}
			else
			{
				return ::format(
					// TRANSLATORS: The argument is a tiletype.
					_("Upgrade to %s"),
					tileName(order.tiletype).c_str());
			}
		}
		break;

		case Order::Type::CULTIVATE:
		{
			return ::format(
				// TRANSLATORS: The argument is a tiletype.
				_("Cultivate %s"),
				tileName(order.tiletype).c_str());
		}
		break;

		case Order::Type::PRODUCE:
		{
			return ::format(
				// TRANSLATORS: The argument is a unittype.
				_("Produce %s"),
				unitName(order.unittype).c_str());
		}
		break;

		case Order::Type::HALT: return _("Stop");

		case Order::Type::MOVE: return _("Move");

		case Order::Type::NONE: return "";
	}
	return "error";
}

const char* Observer::spritenameOrderIcon(const UnitType& unittype)
{
	const char* iconname = _skinner.iconname(unittype).c_str();
	if (*iconname) return iconname;
	else return "ui/ordericon_missing";
}

const char* Observer::spritenameOrderIcon(const TileType& tiletype)
{
	const char* iconname = _skinner.iconname(tiletype).c_str();
	if (*iconname) return iconname;
	else return "ui/ordericon_missing";
}

const char* Observer::spritenameOrderIcon(const Order::Type& ordertype)
{
	switch (ordertype)
	{
		case Order::Type::MOVE:      return "ui/ordericon_move";
		case Order::Type::GUARD:     return "ui/ordericon_guard";
		case Order::Type::FOCUS:     return "ui/ordericon_focus";
		case Order::Type::LOCKDOWN:  return "ui/ordericon_guard";
		case Order::Type::SHELL:     return "ui/ordericon_bombard";
		case Order::Type::BOMBARD:   return "ui/ordericon_bombard";
		case Order::Type::BOMB:      return "ui/ordericon_bomb";
		case Order::Type::CAPTURE:   return "ui/ordericon_capture";
		case Order::Type::SHAPE:     // error
		case Order::Type::SETTLE:    // error
		case Order::Type::EXPAND:    // error
		case Order::Type::UPGRADE:   return "ui/ordericon_upgrade";
		case Order::Type::CULTIVATE: // error
		case Order::Type::PRODUCE:   // error
		case Order::Type::HALT:      return "ui/ordericon_halt";
		case Order::Type::NONE:      return "ui/ordericon_sleep";
	}
	return nullptr;
}

const char* Observer::spritenameOrderIcon(const Order& order)
{
	switch (order.type)
	{
		case Order::Type::SHAPE:
		case Order::Type::SETTLE:
		case Order::Type::EXPAND:
		case Order::Type::CULTIVATE:
		{
			return spritenameOrderIcon(order.tiletype);
		}
		break;
		case Order::Type::UPGRADE:
		{
			if (order.tiletype == TileType::NONE)
			{
				return spritenameOrderIcon(order.type);
			}
			else
			{
				return spritenameOrderIcon(order.tiletype);
			}
		}
		break;
		case Order::Type::PRODUCE:
		{
			return spritenameOrderIcon(order.unittype);
		}
		break;
		case Order::Type::MOVE:
		case Order::Type::GUARD:
		case Order::Type::FOCUS:
		case Order::Type::LOCKDOWN:
		case Order::Type::SHELL:
		case Order::Type::BOMBARD:
		case Order::Type::BOMB:
		case Order::Type::CAPTURE:
		case Order::Type::HALT:
		case Order::Type::NONE:
		{
			return spritenameOrderIcon(order.type);
		}
		break;
	}
	return nullptr;
}

std::unique_ptr<InterfaceElement> Observer::makeOrderIcon(const char* spritename)
{
	std::unique_ptr<InterfaceElement> ordericon(new StackedLayout());

	ordericon->add("filler", new Filler());
	ordericon->add("icon", new Image(spritename));

	{
		InterfaceElement& icon = (*ordericon)["icon"];

		icon.setColor(0, ColorName::UIPRIMARY);
		icon.setColor(1, ColorName::UIPRIMARY);
		icon.setColor(2, ColorName::UIPRIMARY);

		icon.setPowerColor(0, ColorName::FRAME100);
		icon.setPowerColor(1, ColorName::FRAME100);
		icon.setPowerColor(2, ColorName::FRAME100);

		if (strcmp(spritename, "ui/ordericon_sleep") == 0)
		{
			icon.setColor(0, Color::transparent());
		}
	}

	ordericon->fixWidth(16 * InterfaceElement::scale());
	ordericon->fixHeight(12 * InterfaceElement::scale());
	ordericon->align(HorizontalAlignment::CENTER);
	ordericon->align(VerticalAlignment::MIDDLE);

	ordericon->setMarginHorizontal(3 * InterfaceElement::scale());
	ordericon->setMarginVertical(4 * InterfaceElement::scale());

	return ordericon;
}

std::unique_ptr<InterfaceElement> Observer::makeOrderButton(const char* tagname)
{
	std::unique_ptr<InterfaceElement> ordericon(new Image("ui/orderbutton"));

	ordericon->setColor(0, Paint::blend(ColorName::FRAMESTONE, ColorName::SHINEBLEND));
	ordericon->setColor(1, ColorName::FRAMESTONE);
	ordericon->setColor(2, Color::transparent());

	ordericon->setPowerColor(0, Paint::blend(ColorName::FRAMESAND, ColorName::SHINEBLEND));
	ordericon->setPowerColor(1, ColorName::FRAMESAND);

	ordericon->setMarginHorizontal(1 * InterfaceElement::scale());
	ordericon->setMarginVertical(2 * InterfaceElement::scale());

	ordericon->makeClickable();
	ordericon->setTag(tagname);

	return ordericon;
}

std::unique_ptr<InterfaceElement> Observer::makeOrderGrip()
{
	std::unique_ptr<InterfaceElement> ordericon(new Image("ui/orderbutton"));

	ordericon->setColor(0, Paint::blend(ColorName::FRAMESTONE, ColorName::SHINEBLEND));
	ordericon->setColor(1, ColorName::FRAMESTONE);
	ordericon->setColor(2, Color::transparent());

	ordericon->setPowerColor(0, Paint::blend(ColorName::FRAMESAND, ColorName::SHINEBLEND));
	ordericon->setPowerColor(1, ColorName::FRAMESAND);

	ordericon->setMarginHorizontal(1 * InterfaceElement::scale());
	ordericon->setMarginVertical(2 * InterfaceElement::scale());

	ordericon->setTag("Grip");

	return ordericon;
}

std::unique_ptr<InterfaceElement> Observer::makeEmptyOrderListElement()
{
	return makeOrderListElement(spritenameOrderIcon(Order()), "", Order(), "Empty");
}

std::unique_ptr<InterfaceElement> Observer::makeOrderListElement(const UnitType& unittype,
		const Order& order, const char* buttontag)
{
	std::string name = unitName(unittype);
	std::string desc = name + ": " + description(order);
	return makeOrderListElement(spritenameOrderIcon(unittype), desc, order,
		buttontag);
}

std::unique_ptr<InterfaceElement> Observer::makeOrderListElement(const TileType& tiletype,
		const Order& order, const char* buttontag)
{
	std::string name = tileName(tiletype);
	std::string desc = name + ": " + description(order);
	return makeOrderListElement(spritenameOrderIcon(tiletype), desc, order,
		buttontag);
}

std::unique_ptr<InterfaceElement> Observer::makeOrderListElement(const char* subjectspritename,
		const std::string& desc, const Order& order, const char* buttontag)
{
	const int FONTSIZESMALL = _settings.getFontSize();

	std::unique_ptr<InterfaceElement> element(
		new ClickAndDrag(/*movable=*/true));
	element->put(new TooltipLayout());

	element->add("item", new Frame("ui/frame_item_9"));
	auto& item = (*element)["item"];
	item.put(new HorizontalLayout());
	item.align(VerticalAlignment::MIDDLE);
	item.add("grip", makeOrderGrip());
	item.add("left", makeOrderIcon(subjectspritename));
	item.add("right", makeOrderIcon(spritenameOrderIcon(order)));
	item.add("remove", makeOrderButton(buttontag));
	item.setPaddingLeft(2 * InterfaceElement::scale());
	item.setPaddingRight(3 * InterfaceElement::scale());
	item.setPaddingVertical(4 * InterfaceElement::scale());
	item["left"].setMarginLeft(4 * InterfaceElement::scale());
	item["right"].setMarginRight(4 * InterfaceElement::scale());

	item.setBasicColor(0, ColorName::FRAME100);
	item.setBasicColor(1, Paint::blend(ColorName::FRAME100, ColorName::SHADEBLEND, 0.5f));
	item.setBasicColor(2, Paint::blend(ColorName::FRAME600, ColorName::SHINEBLEND));
	item.setBasicColor(3, Paint::blend(ColorName::FRAME600, ColorName::SHADEBLEND));
	item.setPowerColor(0, ColorName::UIPRIMARY);
	item.setPowerColor(1, Paint::blend(ColorName::UIPRIMARY, ColorName::SHADEBLEND, 0.5f));
	item.setPowerColor(2, Paint::blend(ColorName::UIPRIMARY, ColorName::SHINEBLEND));
	item.setPowerColor(3, Paint::blend(ColorName::UIPRIMARY, ColorName::SHADEBLEND));

	{
		std::unique_ptr<InterfaceElement> swap(new Backing());
		swap->add("backing", makeOrderListFrame());
		swap->put(std::move(element));
		element = std::move(swap);
	}

	element->settle();
	element->fixWidth();
	element->setMargin(4 * InterfaceElement::scale());

	element->content().makeClickable();

	if (order.type != Order::Type::NONE)
	{
		element->enable();

		element->add("tooltip", Frame::makeTooltip());
		(*element)["tooltip"].put(new TextField(desc, FONTSIZESMALL));
		(*element)["tooltip"].setMargin(4 * InterfaceElement::scale());
	}
	else
	{
		element->disable();
		element->content().hide();
	}

	std::stringstream strm;
	strm << TypeEncoder(&_bible) << order;
	element->add("data", new HiddenTag(strm.str()));

	return element;
}

std::unique_ptr<InterfaceElement> Observer::makeOrderListFrame()
{
	// This font size is not a setting because it brakes the frames.
	const int FONTSIZEFIXED = 2 * _settings.getScaleBasedFontSize();

	std::unique_ptr<InterfaceElement> element(
		new Frame("ui/frame_item_9"));

	element->setColor(0, Paint::blend(ColorName::FRAME200, ColorName::DISABLEDBLEND));
	element->setColor(1, Paint::blend(Paint::blend(ColorName::FRAME200, ColorName::DISABLEDBLEND), ColorName::SHADEBLEND, 0.5f));
	element->setColor(2, Paint::blend(Paint::blend(ColorName::FRAME600, ColorName::DISABLEDBLEND), ColorName::SHINEBLEND));
	element->setColor(3, Paint::blend(Paint::blend(ColorName::FRAME600, ColorName::DISABLEDBLEND), ColorName::SHADEBLEND));

	element->setDisabledColor(0, Paint::blend(ColorName::FRAME200, ColorName::DISABLEDBLEND, 0.8f));
	element->setDisabledColor(1, Paint::blend(Paint::blend(ColorName::FRAME200, ColorName::DISABLEDBLEND, 0.8f), ColorName::SHADEBLEND, 0.5f));
	element->setDisabledColor(2, Paint::blend(Paint::blend(ColorName::FRAME600, ColorName::DISABLEDBLEND), ColorName::SHINEBLEND));
	element->setDisabledColor(3, Paint::blend(Paint::blend(ColorName::FRAME600, ColorName::DISABLEDBLEND), ColorName::SHADEBLEND));

	element->put(new StackedLayout());
	element->add("filler", new Filler());
	element->add("number", new TextField(
		"",
		FONTSIZEFIXED,
		Paint::blend(ColorName::FRAME200, ColorName::DISABLEDBLEND, 0.8f),
		InterfaceElement::scale(),
		Paint::mix(Paint::blend(ColorName::FRAME200,
			ColorName::DISABLEDBLEND), ColorName::FRAME600, 0.4f)));
	(*element)["number"].align(HorizontalAlignment::CENTER);
	element->align(HorizontalAlignment::CENTER);
	element->align(VerticalAlignment::MIDDLE);

	return element;
}

std::unique_ptr<InterfaceElement> Observer::makeCard(
	std::unique_ptr<InterfaceElement> icon,
	const std::string& title, const std::string& description)
{
	const int FONTSIZESMALL = _settings.getFontSizeTutorial();

	std::unique_ptr<InterfaceElement> element(new Frame("ui/frame_window_9"));

	(*element).put(new HorizontalLayout());
	(*element).add("icon", new StackedLayout());
	(*element)["icon"].align(HorizontalAlignment::CENTER);
	(*element)["icon"].align(VerticalAlignment::MIDDLE);
	(*element)["icon"].add("filler", new Frame("ui/canvas_9"));
	(*element)["icon"].add("icon", std::move(icon));
	(*element)["icon"]["filler"].fixWidth(24 * InterfaceElement::scale());
	(*element)["icon"]["filler"].fixHeight(24 * InterfaceElement::scale());
	(*element)["icon"]["filler"].hide();
	(*element)["icon"].setMarginRight(4 * InterfaceElement::scale());
	(*element).add("texts", new VerticalLayout());
	(*element)["texts"].add("title", new TextField(title, FONTSIZESMALL));
	(*element)["texts"].add("desc", new MultiTextField(description,
		FONTSIZESMALL, ColorName::TEXT700));
	(*element)["texts"].fixWidth(120 * InterfaceElement::scale());
	(*element)["texts"].align(HorizontalAlignment::LEFT);
	(*element).align(VerticalAlignment::MIDDLE);
	(*element).setPadding(4 * InterfaceElement::scale());
	(*element).setMargin(5 * InterfaceElement::scale());
	(*element).settle();
	(*element).fixHeight();
	(*element).fixWidth();

	return element;
}

std::unique_ptr<InterfaceElement> Observer::makeTurnSequenceElement(
	int sequence, const Player& player)
{
	const int FONTSIZESMALL = _settings.getFontSize();

	std::unique_ptr<InterfaceElement> element(new TooltipLayout());

	Paint uicolor = getPlayerItemColor(player);
	element->add("item", Frame::makeMini(uicolor));
	auto& item = (*element)["item"];
	item.put(new Filler());
	item.content().fixHeight(1 * InterfaceElement::scale());

	element->settleWidth();
	element->settleHeight();
	element->setMarginHorizontal(2 * InterfaceElement::scale());
	element->setMarginVertical(2 * InterfaceElement::scale());
	element->makeClickable();

	std::string ordinal;
	switch (sequence)
	{
		// TRANSLATORS: The ordinal numeral, not the adverb.
		case 1: ordinal = _("first"); break;
		// TRANSLATORS: The ordinal numeral, not the measurement of time.
		case 2: ordinal = _("second"); break;
		case 3: ordinal = _("third"); break;
		case 4: ordinal = _("fourth"); break;
		case 5: ordinal = _("fifth"); break;
		case 6: ordinal = _("sixth"); break;
		case 7: ordinal = _("seventh"); break;
		case 8: ordinal = _("eighth"); break;

		default:
		{
			LOGE << "Missing ordinal " << sequence;
			DEBUG_ASSERT(false);
			ordinal = _("eighth");
		}
		break;
	}

	std::string desc;
	if (player == _player || player == Player::NONE)
	{
		desc = ::format(
			// TRANSLATORS: The argument is an ordinal numeral.
			_("You go %s."),
			ordinal.c_str());
	}
	else
	{
		std::string name;

		InterfaceElement& dbox = getDiplomacyBox();
		InterfaceElement& plist = dbox["players"];
		std::string index = ::stringify(player);
		if (plist.contains(index))
		{
			name = plist[index]["name"].text();
		}
		else
		{
			name = ::colorPlayerName(player);
		}

		desc = ::format(
			// TRANSLATORS: The first argument is a name and the second
			// argument is an ordinal numeral.
			_("%s goes %s."),
			name.c_str(),
			ordinal.c_str());
	}

	element->add("tooltip", Frame::makeTooltip());
	(*element)["tooltip"].put(new TextField(desc, FONTSIZESMALL));
	(*element)["tooltip"].setMargin(4 * InterfaceElement::scale());

	return element;
}

std::unique_ptr<InterfaceElement> Observer::makePhaseIcon(Daytime daytime, Phase phase)
{
	switch (phase)
	{
		case Phase::GROWTH:
		{
			switch (daytime)
			{
				case Daytime::LATE:
				{
					return makeIcon("ui/phaseicon_day");
				}
				break;
				case Daytime::EARLY:
				{
					return makeIcon("ui/phaseicon_night");
				}
				break;
			}
			break;
		}
		break;
		case Phase::PLANNING:
		{
			return makeIcon("ui/phaseicon_planning");
		}
		break;
		case Phase::ACTION:
		{
			return makeIcon("ui/phaseicon_action");
		}
		break;
		case Phase::DECAY:
		{
			return makeIcon("ui/phaseicon_decay");
		}
		break;
		case Phase::RESTING:
		case Phase::STAGING:
		{
			LOGE << "Non-exising phase icon";
			DEBUG_ASSERT(false);
		}
		break;
	}

	return makeIcon("ui/phaseicon_action");
}

std::unique_ptr<InterfaceElement> Observer::makeIcon(const char* spritename)
{
	std::unique_ptr<InterfaceElement> ordericon(new Image(spritename));

	ordericon->setBasicColor(0, Paint::blend(ColorName::UISECONDARY, ColorName::SHINEBLEND));
	ordericon->setBasicColor(1, ColorName::UISECONDARY);
	ordericon->setBasicColor(2, Paint::blend(ColorName::UISECONDARY, ColorName::SHADEBLEND));

	ordericon->setPowerColor(0, Paint::blend(ColorName::UIPRIMARY, ColorName::SHINEBLEND));
	ordericon->setPowerColor(1, ColorName::UIPRIMARY);
	ordericon->setPowerColor(2, Paint::blend(ColorName::UIPRIMARY, ColorName::SHADEBLEND));

	return ordericon;
}

std::unique_ptr<InterfaceElement> Observer::makeIcon(const char* spritename,
	const Paint& color)
{
	std::unique_ptr<InterfaceElement> ordericon(new Image(spritename));

	Color muted = Paint::mix(ColorName::FRAME600, ColorName::FRAME400, 0.4f);
	ordericon->setBasicColor(0, Paint::blend(muted, ColorName::SHINEBLEND, 0.5f));
	ordericon->setBasicColor(1, muted);
	ordericon->setBasicColor(2, Paint::blend(muted, ColorName::SHADEBLEND, 0.5f));

	ordericon->setPowerColor(0, Paint::blend(color, ColorName::SHINEBLEND));
	ordericon->setPowerColor(1, color);
	ordericon->setPowerColor(2, Paint::blend(color, ColorName::SHADEBLEND));

	return ordericon;
}

std::unique_ptr<InterfaceElement> Observer::makeIconWithTooltip(
	std::unique_ptr<InterfaceElement> icon,
	const std::string& tooltip)
{
	const int FONTSIZESMALL = _settings.getFontSize();

	std::unique_ptr<InterfaceElement> element(new TooltipLayout());

	std::string desc = tooltip;
	(*element).add("it", new StackedLayout());
	(*element)["it"].align(HorizontalAlignment::CENTER);
	(*element)["it"].align(VerticalAlignment::MIDDLE);
	(*element)["it"].add("filler", new Frame("ui/canvas_9"));
	(*element)["it"].add("icon", std::move(icon));
	(*element)["it"]["filler"].fixWidth(11 * InterfaceElement::scale());
	(*element)["it"]["filler"].fixHeight(11 * InterfaceElement::scale());
	(*element)["it"]["filler"].hide();
	(*element).add("tooltip", Frame::makeTooltip());
	(*element)["tooltip"].put(new VerticalLayout());
	TextField* lasttextfield = new TextField(desc, FONTSIZESMALL);
	(*element)["tooltip"].add("0", lasttextfield);
	(*element)["tooltip"].setMargin(4 * InterfaceElement::scale());
	(*element).settle();
	(*element)["tooltip"].fixWidth(std::min((*element)["tooltip"].width(),
		InterfaceElement::windowW() / 3));
	for (int i = 0; i < 10; i++)
	{
		desc = lasttextfield->textLeftover();
		if (desc.empty()) break;
		lasttextfield = new TextField(desc, FONTSIZESMALL);
		(*element)["tooltip"].add(std::to_string(i + 1), lasttextfield);
		(*element)["tooltip"][std::to_string(i + 1)].fixWidth(
			(*element)["tooltip"][std::to_string(i)].width());
	}

	return element;
}

std::unique_ptr<InterfaceElement> Observer::makeIconWithTooltipCube(
	std::unique_ptr<InterfaceElement> icon,
	const std::string& tooltip)
{
	const int FONTSIZESMALL = _settings.getFontSize();

	std::unique_ptr<InterfaceElement> element(new TooltipLayout());

	std::string desc = tooltip;
	(*element).add("it", new Frame("ui/frame_window_9"));
	(*element)["it"].put(new StackedLayout());
	(*element)["it"].align(HorizontalAlignment::CENTER);
	(*element)["it"].align(VerticalAlignment::MIDDLE);
	(*element)["it"].add("filler", new Filler());
	(*element)["it"].add("icon", std::move(icon));
	(*element)["it"].setPadding(2 * InterfaceElement::scale());
	(*element)["it"]["filler"].fixWidth(11 * InterfaceElement::scale());
	(*element)["it"]["filler"].fixHeight(11 * InterfaceElement::scale());
	(*element)["it"].setMargin(1 * InterfaceElement::scale());
	(*element).add("tooltip", Frame::makeTooltip());
	(*element)["tooltip"].put(new VerticalLayout());
	TextField* lasttextfield = new TextField(desc, FONTSIZESMALL);
	(*element)["tooltip"].add("0", lasttextfield);
	(*element)["tooltip"].setMargin(4 * InterfaceElement::scale());
	(*element).settle();
	(*element)["tooltip"].fixWidth(std::min((*element)["tooltip"].width(),
		InterfaceElement::windowW() / 3));
	for (int i = 0; i < 10; i++)
	{
		desc = lasttextfield->textLeftover();
		if (desc.empty()) break;
		lasttextfield = new TextField(desc, FONTSIZESMALL);
		(*element)["tooltip"].add(std::to_string(i + 1), lasttextfield);
		(*element)["tooltip"][std::to_string(i + 1)].fixWidth(
			(*element)["tooltip"][std::to_string(i)].width());
	}

	return element;
}

std::unique_ptr<InterfaceElement> Observer::makeIconWithTooltipTall(
	std::unique_ptr<InterfaceElement> icon,
	const std::string& tooltip,
	bool /**/, bool /**/)
{
	const int FONTSIZESMALL = _settings.getFontSize();

	std::unique_ptr<InterfaceElement> element(new TooltipLayout());

	std::string desc = tooltip;
	(*element).add("it", new Frame("ui/frame_window_9"));
	(*element)["it"].put(new StackedLayout());
	(*element)["it"].align(HorizontalAlignment::CENTER);
	(*element)["it"].align(VerticalAlignment::MIDDLE);
	(*element)["it"].add("filler", new Filler());
	(*element)["it"].add("icon", std::move(icon));
	(*element)["it"].setPadding(2 * InterfaceElement::scale());
	(*element)["it"]["filler"].fixWidth(9 * InterfaceElement::scale());
	(*element)["it"]["filler"].fixHeight(26 * InterfaceElement::scale());
	(*element)["it"].setMarginHorizontal(6 * InterfaceElement::scale());
	(*element)["it"].setMarginVertical(1 * InterfaceElement::scale());
	(*element).add("tooltip", Frame::makeTooltip());
	(*element)["tooltip"].put(new VerticalLayout());
	TextField* lasttextfield = new TextField(desc, FONTSIZESMALL);
	(*element)["tooltip"].add("0", lasttextfield);
	(*element)["tooltip"].setMargin(4 * InterfaceElement::scale());
	(*element).settle();
	(*element)["tooltip"].fixWidth(std::min((*element)["tooltip"].width(),
		InterfaceElement::windowW() / 3));
	for (int i = 0; i < 10; i++)
	{
		desc = lasttextfield->textLeftover();
		if (desc.empty()) break;
		lasttextfield = new TextField(desc, FONTSIZESMALL);
		(*element)["tooltip"].add(std::to_string(i + 1), lasttextfield);
		(*element)["tooltip"][std::to_string(i + 1)].fixWidth(
			(*element)["tooltip"][std::to_string(i)].width());
	}

	return element;
}

std::unique_ptr<InterfaceElement> Observer::makeIconWithTooltipWide(
	std::unique_ptr<InterfaceElement> icon,
	const std::string& tooltip,
	bool /**/, bool /**/)
{
	const int FONTSIZESMALL = _settings.getFontSize();

	std::unique_ptr<InterfaceElement> element(new TooltipLayout());

	std::string desc = tooltip;
	(*element).add("it", new Frame("ui/frame_window_9"));
	(*element)["it"].put(new StackedLayout());
	(*element)["it"].align(HorizontalAlignment::CENTER);
	(*element)["it"].align(VerticalAlignment::MIDDLE);
	(*element)["it"].add("filler", new Filler());
	(*element)["it"].add("icon", std::move(icon));
	(*element)["it"].setPadding(2 * InterfaceElement::scale());
	(*element)["it"]["filler"].fixWidth(
		24 * InterfaceElement::scale());
	(*element)["it"]["filler"].fixHeight(
		8 * InterfaceElement::scale());
	(*element)["it"].setMarginHorizontal(1 * InterfaceElement::scale());
	(*element)["it"].setMarginVertical(0);
	(*element).add("tooltip", Frame::makeTooltip());
	(*element)["tooltip"].put(new VerticalLayout());
	TextField* lasttextfield = new TextField(desc, FONTSIZESMALL);
	(*element)["tooltip"].add("0", lasttextfield);
	(*element)["tooltip"].setMargin(4 * InterfaceElement::scale());
	(*element).settle();
	(*element)["tooltip"].fixWidth(std::min((*element)["tooltip"].width(),
		InterfaceElement::windowW() / 3));
	for (int i = 0; i < 10; i++)
	{
		desc = lasttextfield->textLeftover();
		if (desc.empty()) break;
		lasttextfield = new TextField(desc, FONTSIZESMALL);
		(*element)["tooltip"].add(std::to_string(i + 1), lasttextfield);
		(*element)["tooltip"][std::to_string(i + 1)].fixWidth(
			(*element)["tooltip"][std::to_string(i)].width());
	}

	return element;
}

std::unique_ptr<InterfaceElement> Observer::makeStatusBar()
{
	std::unique_ptr<InterfaceElement> element(new Frame("ui/frame_window_9"));

	(*element).put(new StackedLayout());
	(*element).align(HorizontalAlignment::LEFT);
	(*element).align(VerticalAlignment::MIDDLE);
	(*element).add("timer", new Frame("ui/gauge_liquid_9"));
	(*element)["timer"].setColor(0, ColorName::UIACCENT);
	(*element)["timer"].setColor(1, Paint::blend(ColorName::UIACCENT, ColorName::SHINEBLEND));
	(*element)["timer"].put(new Filler());
	(*element)["timer"].fixWidth(0);
	(*element)["timer"].hide();
	(*element).add("juice", new Frame("ui/gauge_liquid_9"));
	(*element)["juice"].setColor(0, ColorName::UIPRIMARY);
	(*element)["juice"].setColor(1, Paint::blend(ColorName::UIPRIMARY, ColorName::SHINEBLEND));
	(*element)["juice"].put(new Filler());
	(*element)["juice"].fixWidth(0);
	(*element)["juice"].hide();
	(*element).add("tempo", new Padding());
	(*element)["tempo"].put(new Image("ui/speedbutton"));
	(*element)["tempo"].setColor(0, ColorName::FRAME100);
	(*element)["tempo"].setColor(1, ColorName::FRAME100);
	(*element)["tempo"].setColor(2, Color::transparent());
	(*element)["tempo"].setDisabledColor(0, ColorName::FRAME100);
	(*element)["tempo"].setDisabledColor(1, ColorName::FRAME100);
	(*element)["tempo"].setTag("Empty");
	(*element)["tempo"].setPaddingHorizontal(2 * InterfaceElement::scale());
	(*element).add("filler", new Filler());
	(*element).setPadding(1 * InterfaceElement::scale());
	(*element).setMargin(5 * InterfaceElement::scale());
	(*element).settleHeight();
	(*element).fixHeight();

	(*element).enableIf(_game.planningTimeTotal() > 0);

	return element;
}

std::unique_ptr<InterfaceElement> Observer::makeIconStat(
	std::unique_ptr<InterfaceElement> icon,
	const std::string& tooltip)
{
	const int FONTSIZESMALL = _settings.getFontSize();

	std::unique_ptr<InterfaceElement> element(new HorizontalLayout());

	(*element).add("icon", makeIconWithTooltip(std::move(icon), tooltip));
	(*element).add("text",
		new TextField("9", FONTSIZESMALL));

	(*element)["icon"]["it"]["filler"].fixWidth(8 * InterfaceElement::scale());
	(*element)["icon"]["it"]["filler"].fixHeight(8 * InterfaceElement::scale());
	(*element)["text"].setMarginLeft(3 * InterfaceElement::scale());
	(*element)["text"].align(HorizontalAlignment::LEFT);
	(*element)["text"].settle();
	(*element)["text"].fixWidth();
	(*element).align(VerticalAlignment::MIDDLE);

	return element;
}

std::unique_ptr<InterfaceElement> Observer::makeSeasonBox()
{
	std::unique_ptr<InterfaceElement> element(
		new Frame("ui/frame_button_9"));

	(*element).put(new StackedLayout());
	(*element).add("season", new TextField(
		formatDate(Season::SPRING, 99),
		_settings.getFontSizeMenuButton()));
	(*element).setPaddingHorizontal(3 * InterfaceElement::scale());
	(*element).setPaddingVertical(1 * InterfaceElement::scale());
	(*element)["season"].settleWidth();
	int maxw = (*element)["season"].width();
	for (Season season : {Season::SUMMER, Season::AUTUMN, Season::WINTER})
	{
		(*element)["season"].setText(formatDate(season, 99));
		maxw = std::max(maxw, (*element)["season"].width());
	}
	(*element)["season"].fixWidth(maxw);
	(*element)["season"].setText(formatDate(Season::SPRING, 1));
	(*element)["season"].align(HorizontalAlignment::CENTER);
	(*element).add("filler", new Filler());
	(*element).align(HorizontalAlignment::CENTER);
	(*element).align(VerticalAlignment::MIDDLE);
	(*element).setMargin(5 * InterfaceElement::scale());
	(*element).setMarginRight(1 * InterfaceElement::scale());
	(*element).settle();
	(*element).fixWidth();
	(*element).makeClickable();

	return element;
}

std::unique_ptr<InterfaceElement> Observer::makePhaseGraphic()
{
	const Skin& skin = Skin::get("ui/phase_graphic");

	std::unique_ptr<InterfaceElement> element(
		new TooltipLayout(TooltipLayout::Type::MANUAL));
	(*element).add("image", new StackedLayout());
	InterfaceElement& graphic = (*element)["image"];
	{
		graphic.add("dawn", new Image(skin));
		graphic["dawn"].setTag("Dawn");
	}
	{
		graphic.add("dusk", new Image(skin));
		graphic["dusk"].setTag("Dusk");
	}
	{
		graphic.add("noon", new Image(skin));
		graphic["noon"].setTag("Noon");
	}
	{
		graphic.add("night", new Image(skin));
		graphic["night"].setTag("Night");
	}
	{
		graphic.add("grass", new Image(skin));
		graphic["grass"].setTag("Grass");
	}
	{
		graphic.add("skulls", new StackedLayout());
		graphic["skulls"].add("base", new Image(skin));
		graphic["skulls"]["base"].setTag("Skull Colour");
		graphic["skulls"].add("blood", new Image(skin));
		graphic["skulls"]["blood"].setTag("Skull Blood");
		graphic["skulls"].add("lines", new Image(skin));
		graphic["skulls"]["lines"].setTag("Skull Lines");
	}
	{
		graphic.add("hand_bot", new StackedLayout());
		graphic["hand_bot"].add("base", new Image(skin));
		graphic["hand_bot"]["base"].setTag("Hand Colour");
		graphic["hand_bot"].add("lines", new Image(skin));
		graphic["hand_bot"]["lines"].setTag("Hand Lines");
		graphic["hand_bot"].disable();
	}
	{
		graphic.add("soldier", new StackedLayout());
		graphic["soldier"].add("base", new Image(skin));
		graphic["soldier"]["base"].setTag("Soldier Colour");
		graphic["soldier"].add("shadow", new Image(skin));
		graphic["soldier"]["shadow"].setTag("Soldier Shadow Planning");
		graphic["soldier"]["shadow"].kill();
		graphic["soldier"].add("lines", new Image(skin));
		graphic["soldier"]["lines"].setTag("Soldier Lines");
	}
	{
		graphic.add("hand_mid", new StackedLayout());
		graphic["hand_mid"].add("base", new Image(skin));
		graphic["hand_mid"]["base"].setTag("Hand Colour");
		graphic["hand_mid"].add("lines", new Image(skin));
		graphic["hand_mid"]["lines"].setTag("Hand Lines");
		graphic["hand_mid"].disable();
	}
	{
		graphic.add("circle", new Image(skin));
		graphic["circle"].setTag("Circle Line");
	}
	{
		graphic.add("sun", new StackedLayout());
		graphic["sun"].add("base", new Image(skin));
		graphic["sun"]["base"].setTag("Sun Colour");
		graphic["sun"].add("lines", new Image(skin));
		graphic["sun"]["lines"].setTag("Sun Line Light");
	}
	{
		graphic.add("hand_top", new StackedLayout());
		graphic["hand_top"].add("base", new Image(skin));
		graphic["hand_top"]["base"].setTag("Hand Colour");
		graphic["hand_top"].add("lines", new Image(skin));
		graphic["hand_top"]["lines"].setTag("Hand Lines");
		graphic["hand_top"].kill();
	}
	{
		graphic.add("moon", new Image(skin));
		graphic["moon"].setTag("Moon");
	}
	graphic.setMargin(5 * InterfaceElement::scale());

	const int FONTSIZESMALL = _settings.getFontSize();
	(*element).add("tooltip_planning", Frame::makeTooltip());
	(*element)["tooltip_planning"].put(new MultiTextField(
		phaseDescription(Daytime::EARLY, Phase::PLANNING), FONTSIZESMALL));
	(*element)["tooltip_planning"].setMargin(4 * InterfaceElement::scale());
	(*element)["tooltip_planning"].settle();
	(*element)["tooltip_planning"].fixWidth(std::min(
		(*element)["tooltip_planning"].width(),
		InterfaceElement::windowW() * 4 / 5));
	(*element).add("tooltip_action", Frame::makeTooltip());
	(*element)["tooltip_action"].put(new MultiTextField(
		phaseDescription(Daytime::EARLY, Phase::ACTION), FONTSIZESMALL));
	(*element)["tooltip_action"].setMargin(4 * InterfaceElement::scale());
	(*element)["tooltip_action"].settle();
	(*element)["tooltip_action"].fixWidth(std::min(
		(*element)["tooltip_action"].width(),
		InterfaceElement::windowW() * 4 / 5));
	(*element).add("tooltip_decay", Frame::makeTooltip());
	(*element)["tooltip_decay"].put(new MultiTextField(
		phaseDescription(Daytime::EARLY, Phase::DECAY), FONTSIZESMALL));
	(*element)["tooltip_decay"].setMargin(4 * InterfaceElement::scale());
	(*element)["tooltip_decay"].settle();
	(*element)["tooltip_decay"].fixWidth(std::min(
		(*element)["tooltip_decay"].width(),
		InterfaceElement::windowW() * 4 / 5));
	(*element).add("tooltip_night", Frame::makeTooltip());
	(*element)["tooltip_night"].put(new MultiTextField(
		phaseDescription(Daytime::EARLY, Phase::GROWTH), FONTSIZESMALL));
	(*element)["tooltip_night"].setMargin(4 * InterfaceElement::scale());
	(*element)["tooltip_night"].settle();
	(*element)["tooltip_night"].fixWidth(std::min(
		(*element)["tooltip_night"].width(),
		InterfaceElement::windowW() * 4 / 5));
	(*element).add("tooltip_weather", Frame::makeTooltip());
	(*element)["tooltip_weather"].put(new MultiTextField(
		phaseDescription(Daytime::LATE, Phase::GROWTH), FONTSIZESMALL));
	(*element)["tooltip_weather"].setMargin(4 * InterfaceElement::scale());
	(*element)["tooltip_weather"].settle();
	(*element)["tooltip_weather"].fixWidth(std::min(
		(*element)["tooltip_weather"].width(),
		InterfaceElement::windowW() * 4 / 5));

	return element;
}

std::unique_ptr<InterfaceElement> Observer::makeChatBox()
{
	const int FONTSIZESMALL = _settings.getFontSize();
	const int FONTSIZE_MENUBUTTON = _settings.getFontSizeMenuButton();

	std::unique_ptr<InterfaceElement> element(new Padding());

	(*element).put(new Frame("ui/frame_window_9"));
	(*element).content().put(new VerticalLayout());
	(*element).setPadding(20 * InterfaceElement::scale());
	(*element).content().setPadding(20 * InterfaceElement::scale());
	(*element).add("history",
		new ScrollableLayout(true));

	(*element).add("inputline", new HorizontalLayout());
	(*element)["inputline"].add("indicator", new TooltipLayout());
	(*element)["inputline"]["indicator"].add("button", new Frame("ui/frame_button_9"));
	(*element)["inputline"]["indicator"]["button"].put(new TextField(
		_("CHAT"),
		FONTSIZE_MENUBUTTON, ColorName::TEXT800));
	(*element)["inputline"]["indicator"]["button"].align(HorizontalAlignment::CENTER);
	(*element)["inputline"]["indicator"]["button"].setMarginVertical(4 * InterfaceElement::scale());
	(*element)["inputline"]["indicator"]["button"].setPaddingHorizontal(6 * InterfaceElement::scale());
	(*element)["inputline"]["indicator"]["button"].setPaddingVertical(4 * InterfaceElement::scale());
	int chatw = InterfaceElement::textW(
		TextStyle(FONTSIZE_MENUBUTTON, Color::broken()),
		_("CHAT"));
	int allw = InterfaceElement::textW(
		TextStyle(FONTSIZE_MENUBUTTON, Color::broken()),
		_("ALL"));
	int gamew = InterfaceElement::textW(
		TextStyle(FONTSIZE_MENUBUTTON, Color::broken()),
		_("GAME"));
	(*element)["inputline"]["indicator"]["button"].content().fixWidth(
		std::max(chatw, std::max(allw, gamew)));
	(*element)["inputline"]["indicator"]["button"].settleWidth();
	(*element)["inputline"]["indicator"]["button"].fixWidth();
	(*element)["inputline"]["indicator"]["button"].settleHeight();
	(*element)["inputline"]["indicator"]["button"].makeClickable();
	(*element)["inputline"]["indicator"]["button"].setHotkeyScancode(
		SDL_SCANCODE_TAB);
	(*element)["inputline"]["indicator"].add("tooltip", Frame::makeTooltip());
	(*element)["inputline"]["indicator"]["tooltip"].put(new TextField(
		_("The chat target. Click to switch (also TAB key)."),
		FONTSIZESMALL));
	(*element)["inputline"].add("input", new Frame("ui/frame_window_9"));
	(*element)["inputline"]["input"].put(new TextInput(
		_settings.getFontSizeTextInput(),
		/*maxlength=*/255));
	(*element)["inputline"]["input"].setMargin(4 * InterfaceElement::scale());
	(*element)["inputline"]["input"].setPadding(4 * InterfaceElement::scale());
	(*element)["inputline"]["input"].power();
	(*element)["inputline"].add("sendbutton", new Frame("ui/frame_button_9"));
	(*element)["inputline"]["sendbutton"].put(new TextField(
		_("Send"),
		FONTSIZE_MENUBUTTON, ColorName::TEXT800));
	(*element)["inputline"]["sendbutton"].align(HorizontalAlignment::CENTER);
	(*element)["inputline"]["sendbutton"].setMarginVertical(4 * InterfaceElement::scale());
	(*element)["inputline"]["sendbutton"].setPaddingHorizontal(6 * InterfaceElement::scale());
	(*element)["inputline"]["sendbutton"].setPaddingVertical(4 * InterfaceElement::scale());
	(*element)["inputline"]["sendbutton"].settleWidth();
	(*element)["inputline"]["sendbutton"].fixWidth();
	(*element)["inputline"]["sendbutton"].settleHeight();
	(*element)["inputline"]["sendbutton"].makeClickable();
	(*element)["inputline"]["sendbutton"].setHotkeyScancode(SDL_SCANCODE_ENTER);
	(*element)["inputline"]["sendbutton"].disable();
	(*element)["inputline"].settleHeight();

	(*element).content().setBasicColor(0, Paint::alpha(ColorName::FRAME200, 128));
	(*element).content().setBasicColor(1, Color::alpha(Paint::blend(ColorName::FRAME200, ColorName::SHADEBLEND, 0.5f), 128));
	(*element).content().setBasicColor(2, Color::alpha(Paint::blend(ColorName::FRAME600, ColorName::SHINEBLEND), 128));
	(*element).content().setBasicColor(3, Color::alpha(Paint::blend(ColorName::FRAME600, ColorName::SHADEBLEND), 128));

	(*element)["history"].add("filler", new Filler());
	(*element)["history"]["filler"].fixHeight(
		InterfaceElement::fontH(FONTSIZESMALL));

	return element;
}

std::unique_ptr<InterfaceElement> Observer::makeChatPreview()
{
	if (!isChatEnabled())
	{
		return makeNothing();
	}

	const int FONTSIZESMALL = _settings.getFontSize();

	std::unique_ptr<InterfaceElement> element(new Frame("ui/frame_window_9"));

	(*element).put(new TooltipLayout());
	(*element).add("history", new ScrollableLayout(true));
	(*element).setMargin(4 * InterfaceElement::scale());
	(*element).setPaddingHorizontal(4 * InterfaceElement::scale());
	(*element).setPaddingVertical(1 * InterfaceElement::scale());

	(*element).add("tooltip", Frame::makeTooltip());
	(*element)["tooltip"].put(new TextField(
		_("Click to open chat."),
		FONTSIZESMALL));
	(*element)["tooltip"].setMargin(4 * InterfaceElement::scale());

	(*element).setBasicColor(0, Paint::alpha(ColorName::FRAME200, 128));
	(*element).setBasicColor(1, Color::alpha(Paint::blend(ColorName::FRAME200, ColorName::SHADEBLEND, 0.5f), 128));
	(*element).setBasicColor(2, Color::alpha(Paint::blend(ColorName::FRAME600, ColorName::SHINEBLEND), 128));
	(*element).setBasicColor(3, Color::alpha(Paint::blend(ColorName::FRAME600, ColorName::SHADEBLEND), 128));

	(*element)["history"].add("filler", new Filler());
	(*element)["history"]["filler"].fixHeight(
		InterfaceElement::fontH(FONTSIZESMALL));

	(*element).makeClickable();
	(*element).setHotkeyScancode(SDL_SCANCODE_ENTER);

	return element;
}

std::unique_ptr<InterfaceElement> Observer::makeMenu()
{
	std::unique_ptr<InterfaceElement> element(new Padding());

	(*element).put(new Frame("ui/frame_window_9"));
	(*element).content().put(new VerticalLayout());
	(*element).setPadding(5 * InterfaceElement::scale());
	(*element).content().setPadding(5 * InterfaceElement::scale());
	(*element).add("buttons", new VerticalLayout());

	(*element)["buttons"].add("chat", makeMenuButton(
		_("Open Chat")));
	(*element)["buttons"].add("quit", makeMenuButton(
		_("Quit")));
	if (_player != Player::OBSERVER)
	{
		(*element)["buttons"]["quit"].setText(
			_("Resign & Quit"));
	}
	(*element)["buttons"].add("continue", makeMenuButton(
		_("Close")));
	(*element)["buttons"]["continue"].setMarginTop(
		10 * InterfaceElement::scale());

	(*element).content().setBasicColor(0, ColorName::FRAME400);
	(*element).content().setBasicColor(1, Paint::blend(ColorName::FRAME400, ColorName::SHADEBLEND, 0.5f));
	(*element).content().setBasicColor(2, Paint::blend(ColorName::FRAME600, ColorName::SHINEBLEND));
	(*element).content().setBasicColor(3, Paint::blend(ColorName::FRAME600, ColorName::SHADEBLEND));

	(*element).settleWidth();
	(*element).fixWidth((*element).width());

	return element;
}

std::unique_ptr<InterfaceElement> Observer::makeMenuButton(
	const std::string& text)
{
	std::unique_ptr<InterfaceElement> frame;
	frame.reset(new Frame("ui/frame_button_9"));
	frame->put(new TextField(text,
		_settings.getFontSize()));
	frame->align(HorizontalAlignment::CENTER);
	frame->setPaddingHorizontal(6 * InterfaceElement::scale());
	frame->setPaddingVertical(4 * InterfaceElement::scale());
	frame->setMargin(2 * InterfaceElement::scale());
	frame->settleHeight();
	frame->makeClickable();
	return frame;
}

std::unique_ptr<InterfaceElement> Observer::makeDiplomacyBox()
{
	std::unique_ptr<InterfaceElement> element(new Padding());

	(*element).put(new Frame("ui/frame_window_9"));
	(*element).content().put(new VerticalLayout());
	(*element).setPadding(5 * InterfaceElement::scale());
	(*element).content().setPadding(20 * InterfaceElement::scale());
	(*element).add("players", new VerticalLayout());

	if (_player == Player::OBSERVER)
	{
		auto& plist = (*element)["players"];
		plist.add("#header", new Padding());
		plist["#header"].put(new HorizontalLayout());
		plist["#header"].add("filler", new HorizontalFiller());
		plist["#header"].add("money", new HorizontalLayout());
		plist["#header"]["money"].add("filler", new HorizontalFiller());
		if (!_bible.moraleReplacesMoney())
		{
			plist["#header"]["money"].add("icon", new Image("effects/coin2_still"));
			plist["#header"]["money"]["icon"].setColor(0, Paint::blend(ColorName::COIN, ColorName::SHINEBLEND));
			plist["#header"]["money"]["icon"].setColor(1, ColorName::COIN);
			plist["#header"]["money"]["icon"].setColor(2, Paint::blend(ColorName::COIN, ColorName::SHADEBLEND));
			plist["#header"]["money"]["icon"].setTag("Still");
		}
		else
		{
			plist["#header"]["money"].add("icon", new Image("effects/heart1_still"));
			plist["#header"]["money"]["icon"].setColor(0, Paint::blend(ColorName::HEART, ColorName::SHINEBLEND));
			plist["#header"]["money"]["icon"].setColor(1, ColorName::HEART);
			plist["#header"]["money"]["icon"].setColor(2, Paint::blend(ColorName::HEART, ColorName::SHADEBLEND));
			plist["#header"]["money"]["icon"].setTag("Still");
		}
		plist["#header"].setPaddingHorizontal(4 * InterfaceElement::scale());
	}

	(*element).content().setBasicColor(0, Paint::alpha(ColorName::FRAME200, 128));
	(*element).content().setBasicColor(1, Color::alpha(Paint::blend(ColorName::FRAME200, ColorName::SHADEBLEND, 0.5f), 128));
	(*element).content().setBasicColor(2, Color::alpha(Paint::blend(ColorName::FRAME600, ColorName::SHINEBLEND), 128));
	(*element).content().setBasicColor(3, Color::alpha(Paint::blend(ColorName::FRAME600, ColorName::SHADEBLEND), 128));

	(*element).settleWidth();
	(*element).fixWidth(std::max((*element).width(),
		(int) (0.25f * InterfaceElement::windowW())));

	return element;
}

inline std::string formatPrice(int cents)
{
	return ::format(
		// TRANSLATORS: The price of a fictional newspaper. The first argument
		// is the number of whole dollars, the next two arguments are the two
		// digits in the number of cents.
		// Feel free to change the valuta if that makes sense.
		_("Price: $%d.%d%d"),
		cents / 100, (cents % 100) / 10, cents % 10);
}

std::unique_ptr<InterfaceElement> Observer::makeReportBox()
{
	const int FONTSIZESMALL = _settings.getFontSize();
	const int FONTSIZEMEDIUM = _settings.getFontSizeMenuHeader();

	std::unique_ptr<InterfaceElement> element(
		new StackedLayout());

	(*element).add("it", new Padding());
	(*element)["it"].put(new Frame("ui/frame_newspaper_9"));
	(*element)["it"].setColor(0, ColorName::FRAMEPAPER);
	(*element)["it"].setColor(1, Paint::blend(ColorName::FRAMEPAPER, ColorName::SHINEBLEND));
	(*element)["it"].setColor(2, Paint::blend(ColorName::FRAMEPAPER, ColorName::SHADEBLEND));
	(*element)["it"].setColor(3, Paint::alpha(ColorName::SHADEBLEND, 100));
	(*element)["it"].setPadding(5 * InterfaceElement::scale());
	(*element)["it"].content().setPadding(20 * InterfaceElement::scale());
	(*element).add("filler", new Filler());
	(*element).align(HorizontalAlignment::CENTER);
	(*element).align(VerticalAlignment::MIDDLE);
	(*element)["it"].content().put(new VerticalLayout());
	InterfaceElement& it = (*element)["it"];

	it.add("title", new TextField(
		_("The Planet"),
		FONTSIZEMEDIUM, Paint::alpha(ColorName::TEXT800, 200)));
	it["title"].align(HorizontalAlignment::CENTER);

	it.add("dateline", new HorizontalLayout());
	it["dateline"].add("date", new TextField(
		formatDate(Season::SPRING, 1),
		FONTSIZESMALL, Paint::alpha(ColorName::TEXT800, 200)));
	it["dateline"].add("filler", new HorizontalFiller());
	it["dateline"].add("price", new TextField(
		formatPrice(999),
		FONTSIZESMALL, Paint::alpha(ColorName::TEXT800, 200)));
	it["dateline"]["price"].settleWidth();
	it["dateline"]["price"].fixWidth();

	it.add("logoline", new HorizontalLayout());
	it["logoline"].align(VerticalAlignment::MIDDLE);
	it["logoline"].add("left", new Frame("ui/gauge_liquid_9"));
	it["logoline"]["left"].setColor(0, Paint::alpha(ColorName::TEXT800, 120));
	it["logoline"]["left"].setColor(1, Paint::alpha(ColorName::TEXT800,  80));
	it["logoline"]["left"].put(new Filler());
	it["logoline"]["left"].fixHeight(2 * InterfaceElement::scale());
	it["logoline"].add("planet", new Image("ui/planet"));
	{
		InterfaceElement& icon = it["logoline"]["planet"];
		int i = 0;
		icon.setColor(i++, Color::transparent());
		icon.setColor(i++, Color::transparent());
		icon.setColor(i++, Color::transparent());
		icon.setColor(i++, Paint::alpha(ColorName::TEXT800, 100));
		icon.setColor(i++, Paint::alpha(ColorName::TEXT800, 180));
		icon.setColor(i++, Paint::alpha(ColorName::TEXT800, 180));
		icon.setColor(i++, Paint::alpha(ColorName::TEXT800,  80));
		icon.setColor(i++, Paint::alpha(ColorName::TEXT800, 160));
		icon.setColor(i++, Paint::alpha(ColorName::TEXT800, 170));
		icon.setColor(i++, Paint::alpha(ColorName::TEXT800,  60));
		icon.setColor(i++, Paint::alpha(ColorName::TEXT800, 130));
		icon.setColor(i++, Paint::alpha(ColorName::TEXT800, 160));
		icon.setColor(i++, Color::transparent());
		icon.setTag("Empty");
	}
	it["logoline"]["planet"].setMarginHorizontal(
		5 * InterfaceElement::scale());
	it["logoline"].add("right", new Frame("ui/gauge_liquid_9"));
	it["logoline"]["right"].setColor(0, Paint::alpha(ColorName::TEXT800, 120));
	it["logoline"]["right"].setColor(1, Paint::alpha(ColorName::TEXT800,  80));
	it["logoline"]["right"].put(new Filler());
	it["logoline"]["right"].fixHeight(2 * InterfaceElement::scale());

	it.add("filler", new Filler());

	return element;
}

void Observer::fillReportBox(const std::string& headline,
		const std::string& message1, const std::string& message2,
		const std::string& message3)
{
	const int FONTSIZESMALL = _settings.getFontSize();
	const int FONTSIZELARGE = _settings.getFontSizeHeadline();

	InterfaceElement& it = getReportBox();

	if (it.contains("headline")) it.remove("headline");
	it.add("headline", new TextField(
		headline,
		FONTSIZELARGE, Paint::alpha(ColorName::TEXT800, 240)));
	it["headline"].align(HorizontalAlignment::CENTER);
	it["headline"].setMarginVertical(5 * InterfaceElement::scale());

	if (it.contains("columns")) it.remove("columns");
	it.add("columns", new HorizontalLayout());
	it["columns"].add("left", new VerticalLayout());
	it["columns"]["left"].align(HorizontalAlignment::LEFT);
	it["columns"]["left"].setMarginHorizontal(10 * InterfaceElement::scale());
	it["columns"].add("mid", new VerticalLayout());
	it["columns"]["mid"].align(HorizontalAlignment::LEFT);
	it["columns"]["mid"].setMarginHorizontal(10 * InterfaceElement::scale());
	it["columns"].add("right", new VerticalLayout());
	it["columns"]["right"].align(HorizontalAlignment::LEFT);
	it["columns"]["right"].setMarginHorizontal(10 * InterfaceElement::scale());

	static const char* LINES[] = {
		"  xxxxxxxxxxxxxx",
		"xxxxxxxxxxxxxxxx",
		"xxxxxx          ",
		"xxxxxxxxxxxx    ",
		"xxxxxxxxx       ",
		"xxxxxxxxxxxxxx  ",
		"xxxxxxxxxxx     ",
	};
	std::unique_ptr<InterfaceElement> example(new TextField(
		LINES[1], FONTSIZESMALL));
	example->settleWidth();
	int colwidth = example->width();

	static const int len = 15;
	int head = 0;
	int next = 0;
	for (auto tag : {stringref("left"), stringref("mid"), stringref("right")})
	{
		for (int i = 0; i < len; i++)
		{
			std::string index = std::to_string(i);
			if (i == head)
			{
				std::string message;
				if (strncmp(tag, "left", 1) == 0) message = message1;
				else if (strncmp(tag, "mid", 1) == 0) message = message2;
				else message = message3;

				it["columns"][tag].add(index, new TextField("", FONTSIZESMALL));

				TextField* textField = new TextField(message, FONTSIZESMALL,
					Paint::alpha(ColorName::TEXT800, 240));
				textField->setWidth(colwidth);
				it["columns"][tag].add(index, textField);

				while (!textField->textLeftover().empty())
				{
					i++;
					index = std::to_string(i);
					textField = new TextField(
						textField->textLeftover(), FONTSIZESMALL,
						Paint::alpha(ColorName::TEXT800, 240));
					textField->setWidth(colwidth);
					it["columns"][tag].add(index, textField);
				}

				head = 100;
			}
			else if (i + 1 == next)
			{
				size_t x = (rand() % (array_size(LINES) - 2)) + 2;
				it["columns"][tag].add(index, new TextField(
					LINES[x], FONTSIZESMALL,
					Paint::alpha(ColorName::TEXT800, 100)));
				it["columns"][tag][index].fixWidth(colwidth);
			}
			else if (i >= next)
			{
				it["columns"][tag].add(index, new TextField(
					LINES[0], FONTSIZESMALL,
					Paint::alpha(ColorName::TEXT800, 100)));
				it["columns"][tag][index].fixWidth(colwidth);
				it["columns"][tag][index].align(HorizontalAlignment::RIGHT);
				next = i + (rand() % 3) + (rand() % 2) + (rand() % 2) + 2;
				if (next + 1 >= head) next = head;
				if (strncmp(tag, "right", 1) == 0 && next + 1 >= len) next = len;
			}
			else
			{
				it["columns"][tag].add(index, new TextField(
					LINES[1], FONTSIZESMALL,
					Paint::alpha(ColorName::TEXT800, 100)));
				it["columns"][tag][index].fixWidth(colwidth);
			}
		}
		next = next % len;
		head = next + 2 * ((rand() % 2) + (rand() % 2));
	}

	it.settleWidth();
	it.fixWidth();
	it.settleHeight();
	it.fixHeight();

	InterfaceElement& viewboxholder = getViewBoxHolder();
	viewboxholder.place(viewboxholder.topleft());
}

void Observer::updateReportBoxDate()
{
	InterfaceElement& seasontextfield = getSeasonTextField();
	std::string summary = seasontextfield.text();

	InterfaceElement& it = getReportBox();
	it["dateline"]["date"].setText(summary);
}

void Observer::updateReportBoxPrice()
{
	InterfaceElement& it = getReportBox();

	int cents = 100;
	for (const Square& sq : _level)
	{
		cents += sq.chaos();
	}

	std::string text = formatPrice(cents);
	it["dateline"]["price"].setText(text);
}

std::unique_ptr<InterfaceElement> Observer::makeMissionBox()
{
	const int FONTSIZEMEDIUM = _settings.getFontSizeMenuHeader();

	std::unique_ptr<InterfaceElement> element(
		new StackedLayout());

	(*element).add("it", new Padding());
	(*element)["it"].put(new Frame("ui/frame_newspaper_9"));
	(*element)["it"].setColor(0, ColorName::FRAMEPAPER);
	(*element)["it"].setColor(1, Paint::blend(ColorName::FRAMEPAPER, ColorName::SHINEBLEND));
	(*element)["it"].setColor(2, Paint::blend(ColorName::FRAMEPAPER, ColorName::SHADEBLEND));
	(*element)["it"].setColor(3, Paint::alpha(ColorName::SHADEBLEND, 100));
	(*element)["it"].setPadding(5 * InterfaceElement::scale());
	(*element)["it"].content().setPadding(20 * InterfaceElement::scale());
	(*element).add("filler", new Filler());
	(*element).align(HorizontalAlignment::CENTER);
	(*element).align(VerticalAlignment::MIDDLE);
	(*element)["it"].content().put(new VerticalLayout());
	InterfaceElement& it = (*element)["it"];

	it.add("title", new TextField(
		_("Mission Briefing"),
		FONTSIZEMEDIUM, Paint::alpha(ColorName::TEXT800, 200)));
	it["title"].align(HorizontalAlignment::CENTER);

	it.add("logoline", new HorizontalLayout());
	it["logoline"].align(VerticalAlignment::MIDDLE);
	it["logoline"].add("left", new Frame("ui/gauge_liquid_9"));
	it["logoline"]["left"].setColor(0, Paint::alpha(ColorName::TEXT800, 120));
	it["logoline"]["left"].setColor(1, Paint::alpha(ColorName::TEXT800,  80));
	it["logoline"]["left"].put(new Filler());
	it["logoline"]["left"].fixHeight(2 * InterfaceElement::scale());
	for (int i = 1; i <= 5; i++)
	{
		std::string index = "star" + std::to_string(i);
		it["logoline"].add(index, new Image("ui/reward_star"));
		{
			InterfaceElement& icon = it["logoline"][index];
			icon.setColor(0, Paint::alpha(ColorName::TEXT800,  80));
			icon.setColor(1, Paint::alpha(ColorName::TEXT800, 100));
			icon.setColor(2, Paint::alpha(ColorName::TEXT800, 120));
		}
		it["logoline"][index].setMargin(
			6 * InterfaceElement::scale());
	}
	it["logoline"].add("right", new Frame("ui/gauge_liquid_9"));
	it["logoline"]["right"].setColor(0, Paint::alpha(ColorName::TEXT800, 120));
	it["logoline"]["right"].setColor(1, Paint::alpha(ColorName::TEXT800,  80));
	it["logoline"]["right"].put(new Filler());
	it["logoline"]["right"].fixHeight(2 * InterfaceElement::scale());
	it["logoline"].setMarginBottom(
		12 * InterfaceElement::scale());

	it.add("filler", new Filler());

	return element;
}

void Observer::fillMissionBox(const Json::Value& json)
{
	InterfaceElement& it = getMissionBox();

	if (it.contains("lines")) it.remove("lines");
	it.add("lines", new VerticalLayout());

	if (json["greeting"].isString() && !json["greeting"].asString().empty())
	{
		std::string text = json["greeting"].asString();
		it["lines"].add("greeting", new MultiTextField(
			GETTEXT_FROM_SERVER(text.c_str()),
			_settings.getFontSize(),
			Paint::alpha(ColorName::TEXT800, 200)));
		it["lines"]["greeting"].setMarginVertical(
			12 * InterfaceElement::scale());
	}

	if (json["description"].isString()
		&& !json["description"].asString().empty())
	{
		std::string text = json["description"].asString();
		it["lines"].add("description", new MultiTextField(
			GETTEXT_FROM_SERVER(text.c_str()),
			_settings.getFontSizeTutorial(),
			Paint::alpha(ColorName::TEXT800, 240)));
		it["lines"]["description"].setMarginVertical(
			12 * InterfaceElement::scale());
	}

	if (json["objective"].isString() && !json["objective"].asString().empty())
	{
		std::string text = json["objective"].asString();
		it["lines"].add("objective", new MultiTextField(
			GETTEXT_FROM_SERVER(text.c_str()),
			_settings.getFontSizeTutorial(),
			Paint::alpha(ColorName::TEXT800, 240)));
		it["lines"]["objective"].setMarginVertical(
			12 * InterfaceElement::scale());
	}

	int maxstars = 1;
	if (json["3"].isString()) maxstars = 3;
	else if (json["2"].isString()) maxstars = 2;

	for (int x = 0; x < 3; x++)
	{
		std::string index = std::to_string(x + 1);
		if (!json[index].isString()) continue;
		it["lines"].add(index, new HorizontalLayout());
		it["lines"][index].add("filler", new HorizontalFiller());
		it["lines"][index]["filler"].fixWidth(
			8 * InterfaceElement::scale());
		for (int i = 1; i <= maxstars; i++)
		{
			std::string subindex = "star" + std::to_string(i);
			it["lines"][index].add(subindex, new Image("effects/star1"));
			auto& icon = it["lines"][index][subindex];
			icon.setColor(0, Paint::alpha(ColorName::FRAME600, 180));
			icon.setColor(1, Paint::alpha(ColorName::FRAME600, 200));
			icon.setColor(2, Paint::alpha(ColorName::FRAME600, 220));
			icon.setPowerColor(0, Paint::blend(ColorName::STAR,
				ColorName::SHINEBLEND));
			icon.setPowerColor(1, ColorName::STAR);
			icon.setPowerColor(2, Paint::blend(ColorName::STAR,
				ColorName::SHADEBLEND, 0.2f));
			icon.powerIf(i <= x + 1);
			icon.setMargin(
				2 * InterfaceElement::scale());
		}
		std::string text = json[index].asString();
		if (x == 0 && text.empty())
		{
			text = "Defeat the enemy.";
		}
		it["lines"][index].add("text", new TextField(
			GETTEXT_FROM_SERVER(text.c_str()),
			_settings.getFontSizeTutorial(),
			Paint::alpha(ColorName::TEXT800, 240)));
		it["lines"][index]["text"].setMarginLeft(
			8 * InterfaceElement::scale());
		it["lines"][index].setMarginVertical(
			4 * InterfaceElement::scale());
	}

	if (json["sendoff"].isString() && !json["sendoff"].asString().empty())
	{
		std::string text = json["sendoff"].asString();
		it["lines"].add("sendoff", new MultiTextField(
			GETTEXT_FROM_SERVER(text.c_str()),
			_settings.getFontSize(),
			Paint::alpha(ColorName::TEXT800, 200)));
		it["lines"]["sendoff"].setMarginVertical(
			12 * InterfaceElement::scale());
	}

	it.fixWidth(
		std::max(InterfaceElement::windowW() * 2 / 5,
			std::min(640, InterfaceElement::windowW() * 3 / 5)));
	it.settleHeight();
	it.fixHeight();

	InterfaceElement& viewboxholder = getViewBoxHolder();
	viewboxholder.place(viewboxholder.topleft());
}

std::unique_ptr<InterfaceElement> Observer::makeMissionNub()
{
	std::unique_ptr<InterfaceElement> element(
		new Frame("ui/frame_button_9"));

	(*element).put(new HorizontalLayout());
	(*element).add("filler1", new VerticalFiller());
	for (int i = 1; i <= 1; i++)
	{
		std::string index = std::to_string(i);
		(*element).add(index, new Image("effects/star1"));

		auto& icon = (*element)[index];
		icon.setColor(0, Paint::blend(ColorName::STAR,
			ColorName::SHINEBLEND));
		icon.setColor(1, ColorName::STAR);
		icon.setColor(2, Paint::blend(ColorName::STAR,
			ColorName::SHADEBLEND, 0.2f));
		icon.setMarginHorizontal(4 * InterfaceElement::scale());
	}
	(*element).add("filler2", new VerticalFiller());
	(*element).setPadding(4 * InterfaceElement::scale());
	(*element).align(VerticalAlignment::MIDDLE);
	(*element).setMargin(5 * InterfaceElement::scale());
	(*element).settle();
	(*element).fixWidth();
	(*element).makeClickable();

	return element;
}

Paint Observer::getPlayerItemColor(const Player& player)
{
	switch (player)
	{
		case Player::RED:    return ColorName::RED;    break;
		case Player::BLUE:   return ColorName::BLUE;   break;
		case Player::TEAL:   return ColorName::TEAL;   break;
		case Player::YELLOW: return ColorName::YELLOW; break;
		case Player::PINK:   return ColorName::PINK;   break;
		case Player::BLACK:  return ColorName::BLACK;  break;
		case Player::INDIGO: return ColorName::INDIGO; break;
		case Player::PURPLE: return ColorName::PURPLE; break;

		default:
		{
			return ColorName::FRAMESAND;
		}
		break;
	}
}

Paint Observer::getPlayerTextColor(const Player& player)
{
	switch (player)
	{
		case Player::RED:
		case Player::BLUE:
		case Player::TEAL:
		case Player::YELLOW:
		case Player::PINK:
		case Player::BLACK:
		case Player::INDIGO:
		case Player::PURPLE:
		{
			return ColorName::TEXT100;
		}
		break;

		default:
		{
			return ColorName::TEXT800;
		}
		break;
	}
}

void Observer::addPlayerToDiplomacyBox(const Player& player,
	const std::string& name)
{
	const int FONTSIZESMALL = _settings.getFontSize();

	InterfaceElement& dbox = getDiplomacyBox();
	InterfaceElement& plist = dbox["players"];

	std::string index = ::stringify(player);
	Paint uicolor = getPlayerItemColor(player);
	Paint textcolor = getPlayerTextColor(player);

	plist.add(index, Frame::makeMini(uicolor));
	plist[index].put(new HorizontalLayout());
	plist[index].add("name", new TextField(name, FONTSIZESMALL,
		textcolor));
	plist[index]["name"].settle();
	plist[index]["name"].fixWidth();
	plist[index].add("defeated", makeIconWithTooltip(
		makeIcon("ui/markericon_death", textcolor),
		_("This player has been defeated.")));
	plist[index]["defeated"].setMarginHorizontal(
		2 * InterfaceElement::scale());
	plist[index]["defeated"].power();
	plist[index]["defeated"].hide();
	plist[index].add("filler", new HorizontalFiller());
	if (_player == Player::OBSERVER)
	{
		plist[index].add("money", new TextField("00999", FONTSIZESMALL,
			textcolor));
		plist[index]["money"].fixWidth();
		plist["#header"]["money"].fixWidth(plist[index]["money"].width());
		plist[index]["money"].align(HorizontalAlignment::RIGHT);
		plist[index]["money"].setText("0");
		plist["#header"].settleWidth();
	}
	plist[index].align(VerticalAlignment::MIDDLE);
	plist[index].setPadding(4 * InterfaceElement::scale());
	plist[index].setMargin(2 * InterfaceElement::scale());

	dbox.unfixWidth();
	plist.unfixWidth();
	dbox.settle();
	dbox.fixWidth(std::max(dbox.width(),
		(int) (0.25f * InterfaceElement::windowW())));
	plist.fixWidth();
	dbox.settle();

	InterfaceElement& button = getDiplomacyNub();
	button.enable();
}

std::unique_ptr<InterfaceElement> Observer::makeMenuNub()
{
	std::unique_ptr<InterfaceElement> element(new Frame("ui/frame_button_9"));

	(*element).setPowerColor(0, ColorName::UIPRIMARY);
	(*element).setPowerColor(1, Paint::blend(ColorName::UIPRIMARY, ColorName::SHADEBLEND, 0.5f));
	(*element).setPowerColor(2, Paint::blend(ColorName::FRAME800, ColorName::SHINEBLEND));
	(*element).setPowerColor(3, Paint::blend(ColorName::FRAME800, ColorName::SHADEBLEND));
	(*element).put(new StackedLayout());
	(*element).add("icon", new Image("effects/bolt1_still"));
	(*element)["icon"].setMargin(4 * InterfaceElement::scale());
	(*element)["icon"].setColor(0, Paint::blend(ColorName::ENERGY, ColorName::SHINEBLEND));
	(*element)["icon"].setColor(1, ColorName::ENERGY);
	(*element)["icon"].setColor(2, Paint::blend(ColorName::ENERGY, ColorName::SHADEBLEND));
	(*element)["icon"].setTag("Still");
	(*element).add("filler", new VerticalFiller());
	(*element).align(VerticalAlignment::MIDDLE);
	(*element).setPaddingHorizontal(8 * InterfaceElement::scale());
	(*element).setPaddingVertical(2 * InterfaceElement::scale());
	(*element).setMargin(5 * InterfaceElement::scale());
	(*element).fixWidth();
	(*element).settle();
	(*element).makeClickable();

	return element;
}

std::unique_ptr<InterfaceElement> Observer::makeDiplomacyNub()
{
	std::unique_ptr<InterfaceElement> element(new Frame("ui/frame_button_9"));

	(*element).setPowerColor(0, ColorName::UIPRIMARY);
	(*element).setPowerColor(1, Paint::blend(ColorName::UIPRIMARY, ColorName::SHADEBLEND, 0.5f));
	(*element).setPowerColor(2, Paint::blend(ColorName::FRAME800, ColorName::SHINEBLEND));
	(*element).setPowerColor(3, Paint::blend(ColorName::FRAME800, ColorName::SHADEBLEND));
	(*element).put(new StackedLayout());
	(*element).add("icon", new Image("effects/work1_still"));
	(*element)["icon"].setMargin(4 * InterfaceElement::scale());
	(*element)["icon"].setColor(0, Paint::blend(ColorName::WORK, ColorName::SHINEBLEND));
	(*element)["icon"].setColor(1, ColorName::WORK);
	(*element)["icon"].setColor(2, Paint::blend(ColorName::WORK, ColorName::SHADEBLEND));
	(*element)["icon"].setTag("Still");
	(*element).add("filler", new VerticalFiller());
	(*element).align(VerticalAlignment::MIDDLE);
	(*element).setPaddingHorizontal(8 * InterfaceElement::scale());
	(*element).setPaddingVertical(2 * InterfaceElement::scale());
	(*element).setMargin(5 * InterfaceElement::scale());
	(*element).fixWidth();
	(*element).settle();
	(*element).makeClickable();

	return element;
}

std::unique_ptr<InterfaceElement> Observer::makeWallet()
{
	const int FONTSIZEMEDIUM = _settings.getFontSizeWallet();

	if (_player == Player::OBSERVER)
	{
		return makeNothing();
	}
	else if (_bible.moraleReplacesMoney())
	{
		return makeOldWallet();
	}

	std::unique_ptr<InterfaceElement> element(new StackedLayout());
	(*element).add("coin", new Image("effects/coin3"));
	(*element)["coin"].setColor(0, Color::broken());
	(*element)["coin"].setColor(1, Paint::blend(ColorName::COIN, ColorName::SHINEBLEND));
	(*element)["coin"].setColor(2, ColorName::COIN);
	(*element)["coin"].setColor(3, Paint::blend(ColorName::COIN, ColorName::SHADEBLEND));
	(*element)["coin"].setColor(4, ColorName::FRAME800);
	(*element)["coin"].setColor(5, ColorName::FRAME800);
	(*element)["coin"].setColor(6, Paint::blend(ColorName::FRAME800, ColorName::SHADEBLEND));
	(*element)["coin"].setPowerColor(1, Paint::blend(ColorName::UIPRIMARY, ColorName::SHINEBLEND));
	(*element)["coin"].setPowerColor(2, ColorName::UIPRIMARY);
	(*element)["coin"].setPowerColor(3, Paint::blend(ColorName::UIPRIMARY, ColorName::SHADEBLEND));
	(*element)["coin"].setTag("Still");
	(*element).add("text", new TextField("999", FONTSIZEMEDIUM));
	(*element)["text"].align(HorizontalAlignment::CENTER);
	// All TextFields are slightly off-center because there is whitespace after
	// the end of the last letter.
	(*element)["text"].fixWidth(
		(*element)["coin"].width() - 2 * InterfaceElement::scale());
	(*element).align(HorizontalAlignment::RIGHT);
	(*element).align(VerticalAlignment::MIDDLE);
	(*element).setMargin(5 * InterfaceElement::scale());
	(*element).fixWidth();
	(*element).settle();

	return element;
}

std::unique_ptr<InterfaceElement> Observer::makeOldWallet()
{
	const int FONTSIZEMEDIUM = _settings.getFontSizeWallet();

	std::unique_ptr<InterfaceElement> element(new Frame("ui/frame_window_9"));

	(*element).setBasicColor(2, ColorName::FRAME800);
	(*element).setBasicColor(3, Paint::blend(ColorName::FRAME800, ColorName::SHADEBLEND));
	(*element).setPowerColor(0, ColorName::UIPRIMARY);
	(*element).setPowerColor(1, Paint::blend(ColorName::UIPRIMARY, ColorName::SHADEBLEND, 0.5f));
	(*element).setPowerColor(2, ColorName::FRAME800);
	(*element).setPowerColor(3, Paint::blend(ColorName::FRAME800, ColorName::SHADEBLEND));
	(*element).put(new HorizontalLayout());
	(*element).add("icon", new SlideshowLayout());
	if (_bible.moraleReplacesMoney())
	{
		(*element)["icon"].add("coin", new Image("effects/heart1_still"));
		(*element)["icon"]["coin"].setMargin(4 * InterfaceElement::scale());
		(*element)["icon"]["coin"].setColor(0, Paint::blend(ColorName::HEART, ColorName::SHINEBLEND));
		(*element)["icon"]["coin"].setColor(1, ColorName::HEART);
		(*element)["icon"]["coin"].setColor(2, Paint::blend(ColorName::HEART, ColorName::SHADEBLEND));
		(*element)["icon"]["coin"].setTag("Still");
	}
	else
	{
		(*element)["icon"].add("coin", new Image("effects/coin2_still"));
		(*element)["icon"]["coin"].setMargin(4 * InterfaceElement::scale());
		(*element)["icon"]["coin"].setColor(0, Paint::blend(ColorName::COIN, ColorName::SHINEBLEND));
		(*element)["icon"]["coin"].setColor(1, ColorName::COIN);
		(*element)["icon"]["coin"].setColor(2, Paint::blend(ColorName::COIN, ColorName::SHADEBLEND));
		(*element)["icon"]["coin"].setTag("Still");
	}
	(*element)["icon"].setTag("coin");
	if (_player != Player::OBSERVER)
	{
		(*element).add("text", new TextField("999", FONTSIZEMEDIUM));
		(*element)["text"].align(HorizontalAlignment::RIGHT);
	}
	(*element).align(VerticalAlignment::MIDDLE);
	(*element).setPaddingHorizontal(6 * InterfaceElement::scale());
	(*element).setPaddingVertical(2 * InterfaceElement::scale());
	(*element).setMargin(5 * InterfaceElement::scale());
	(*element).fixWidth();
	(*element).settle();

	return element;
}

std::unique_ptr<InterfaceElement> Observer::makeQuitOverlay()
{
	std::unique_ptr<InterfaceElement> element(new Frame("ui/canvas_9"));

	(*element).put(new Filler());
	(*element).setColor(0, Paint::alpha(ColorName::SHADEBLEND, 150));
	(*element).setColor(1, Color::transparent());
	(*element).setColor(2, Color::transparent());
	(*element).setColor(3, Color::transparent());
	(*element).kill();

	return element;
}

std::unique_ptr<InterfaceElement> Observer::makePauseOverlay()
{
	std::unique_ptr<InterfaceElement> element(new Image("ui/pauseprint"));

	(*element).setColor(0, Paint::alpha(ColorName::GUIDESLOWED, 100));
	(*element).setColor(1, Paint::alpha(ColorName::GUIDESHINE, 200));
	(*element).setColor(2, Color::transparent());
	(*element).setColor(3, Color::transparent());
	(*element).setShineColor(ColorName::GUIDESHINE);
	(*element).shine();
	(*element).kill();

	return element;
}

std::unique_ptr<InterfaceElement> Observer::makeViewport()
{
	std::unique_ptr<InterfaceElement> element(new Padding());
	(*element).put(new Frame("ui/debug_9"));
	(*element).content().setColor(0, Paint::alpha(ColorName::GUIDESLOWED,  20));
	(*element).content().setColor(1, Paint::alpha(ColorName::GUIDESLOWED,  30));
	(*element).content().setColor(2, Paint::alpha(ColorName::GUIDESLOWED, 120));
	(*element).content().setColor(3, Paint::alpha(ColorName::GUIDESLOWED, 170));
	(*element).content().put(new Filler());
	(*element).content().makeMassless();
	(*element).content().kill();
	(*element).setPadding(Camera::get()->scale() * Surface::WIDTH / 2);

	return element;
}

std::unique_ptr<InterfaceElement> Observer::makeCardView()
{
	std::unique_ptr<InterfaceElement> element(new VerticalLayout());
	(*element).add("filler", new VerticalFiller());
	(*element).align(HorizontalAlignment::LEFT);

	return element;
}

std::unique_ptr<InterfaceElement> Observer::makeReadyButton()
{
	return makeNothing();
}

std::unique_ptr<InterfaceElement> Observer::makeOrderList()
{
	return makeNothing();
}

std::unique_ptr<InterfaceElement> Observer::makeNothing()
{
	std::unique_ptr<InterfaceElement> element(new Filler());

	element->fixWidth(0);
	element->fixHeight(0);

	return element;
}

std::unique_ptr<InterfaceElement> Observer::makeDefeatPopup()
{
	const int FONTSIZESMALL = _settings.getFontSize();
	const int FONTSIZELARGE = _settings.getFontSizeHeadline();

	std::unique_ptr<InterfaceElement> element(new Frame("ui/frame_window_9"));

	(*element).setBasicColor(0, Paint::alpha(ColorName::FRAME200, 100));
	(*element).put(new VerticalLayout());
	(*element).add("title", new TextField(
		_("Defeat!"),
		FONTSIZELARGE, ColorName::TEXT900));
	(*element)["title"].align(HorizontalAlignment::CENTER);
	(*element)["title"].setMargin(5 * InterfaceElement::scale());
	(*element).add("text", new TextField(
		_("All players are defeated!"),
		FONTSIZESMALL, ColorName::TEXT800));
	if (_player != Player::OBSERVER)
	{
		(*element)["text"].setText(
			_("You have been defeated."));
	}
	(*element)["text"].setMargin(3 * InterfaceElement::scale());
	(*element)["text"].align(HorizontalAlignment::CENTER);
	(*element).add("subtext", new HorizontalLayout());
	(*element)["subtext"].add("scoretext", new TextField(
		_("Score:"),
		FONTSIZESMALL, ColorName::TEXT800));
	(*element)["subtext"]["scoretext"].align(HorizontalAlignment::LEFT);
	(*element)["subtext"].add("score", new TextField(
		"0",
		FONTSIZESMALL, ColorName::TEXT800));
	(*element)["subtext"]["score"].align(HorizontalAlignment::RIGHT);
	(*element)["subtext"].settleHeight();
	(*element)["subtext"].setMargin(3 * InterfaceElement::scale());
	(*element).align(HorizontalAlignment::CENTER);
	(*element).setPadding(10 * InterfaceElement::scale());
	(*element).settleWidth();
	(*element).settleHeight();
	(*element).fixWidth();
	(*element).fixHeight();
	(*element).kill();

	return element;
}

std::unique_ptr<InterfaceElement> Observer::makeVictoryPopup()
{
	const int FONTSIZESMALL = _settings.getFontSize();
	const int FONTSIZELARGE = _settings.getFontSizeHeadline();

	std::unique_ptr<InterfaceElement> element(new Frame("ui/frame_window_9"));

	(*element).setBasicColor(0, Paint::alpha(ColorName::FRAME200, 100));
	(*element).put(new VerticalLayout());
	(*element).add("title",
		new TextField("Victory!", FONTSIZELARGE, ColorName::TEXT900));
	(*element)["title"].align(HorizontalAlignment::CENTER);
	(*element)["title"].setMargin(5 * InterfaceElement::scale());
	(*element).add("text", new TextField(
		_("A player is victorious!"),
		FONTSIZESMALL, ColorName::TEXT800));
	if (_player != Player::OBSERVER)
	{
		(*element)["text"].setText(
			_("You are victorious."));
	}
	(*element)["text"].setMargin(3 * InterfaceElement::scale());
	(*element)["text"].align(HorizontalAlignment::CENTER);
	(*element).add("subtext", new HorizontalLayout());
	(*element)["subtext"].add("scoretext", new TextField(
		_("Score:"),
		FONTSIZESMALL, ColorName::TEXT800));
	(*element)["subtext"]["scoretext"].align(HorizontalAlignment::LEFT);
	(*element)["subtext"].add("score", new TextField(
		"0",
		FONTSIZESMALL, ColorName::TEXT800));
	(*element)["subtext"]["score"].align(HorizontalAlignment::RIGHT);
	(*element)["subtext"].settleHeight();
	(*element)["subtext"].setMargin(3 * InterfaceElement::scale());
	(*element).align(HorizontalAlignment::CENTER);
	(*element).setPadding(10 * InterfaceElement::scale());
	(*element).settleWidth();
	(*element).settleHeight();
	(*element).fixWidth();
	(*element).fixHeight();
	(*element).kill();

	return element;
}

std::unique_ptr<InterfaceElement> Observer::makeQuitPopup()
{
	const int FONTSIZESMALL = _settings.getFontSize();
	const int FONTSIZELARGE = _settings.getFontSizeHeadline();

	std::unique_ptr<InterfaceElement> element(new Frame("ui/frame_window_9"));

	(*element).put(new VerticalLayout());
	(*element).add("title", new TextField(
		_("Quit?"),
		FONTSIZELARGE, ColorName::TEXT900));
	(*element)["title"].align(HorizontalAlignment::CENTER);
	(*element)["title"].setMargin(5 * InterfaceElement::scale());
	(*element).add("text", new TextField(
		_("Are you sure you want to quit?"),
		FONTSIZESMALL, ColorName::TEXT700));
	(*element)["text"].setMargin(3 * InterfaceElement::scale());
	(*element).add("buttons", new HorizontalLayout());
	(*element)["buttons"].add("quit", makeButton(
		_("quit"),
		_settings.getFontSizeMenuButton()));
	(*element)["buttons"]["quit"].setMargin(3 * InterfaceElement::scale());
	(*element)["buttons"].add("cancel", makeButton(
		_("cancel"),
		_settings.getFontSizeMenuButton()));
	(*element)["buttons"]["cancel"].setMargin(3 * InterfaceElement::scale());
	(*element)["buttons"].setMarginTop(10 * InterfaceElement::scale());
	(*element).align(HorizontalAlignment::CENTER);
	(*element).setPadding(10 * InterfaceElement::scale());
	if (_player != Player::OBSERVER)
	{
		(*element)["title"].setText(
			_("Resign?"));
		(*element)["text"].setText(
			_("Are you sure you want to resign?"));
		(*element)["buttons"]["quit"].setText(
			_("resign"));
	}
	(*element).settleWidth();
	(*element).settleHeight();
	(*element).fixWidth();
	(*element).fixHeight();
	(*element).kill();
	(*element).unfixWidth();
	(*element).settleWidth();
	(*element).fixWidth();

	return element;
}

std::unique_ptr<InterfaceElement> Observer::makeButton(
	const std::string& text, int fontsize)
{
	std::unique_ptr<InterfaceElement> frame;
	frame.reset(new Frame("ui/frame_button_9"));
	frame->put(new TextField(text, fontsize));
	frame->align(HorizontalAlignment::CENTER);
	frame->setPadding(6 * InterfaceElement::scale());
	frame->settleHeight();
	frame->makeClickable();
	return frame;
}

void Observer::load()
{
	_camerafocus->set(_level.centerPoint());
	_camerafocus->load(_level.topleftPoint(), _level.bottomrightPoint());
	_cursor.reset(new Cursor(_board, _bible));

	build();
}

void Observer::receiveChanges(const std::vector<Change>& changes)
{
	_unprocessedChanges.push(changes);
}

InterfaceElement& Observer::getViewport()
{
	InterfaceElement& viewboxholder = getViewBoxHolder();
	return viewboxholder["viewport"].content();
}

InterfaceElement& Observer::getViewBoxHolder()
{
	return _layout["center"]["mid"];
}

InterfaceElement& Observer::getCardView()
{
	InterfaceElement& viewboxholder = getViewBoxHolder();
	return viewboxholder["main"];
}

InterfaceElement& Observer::getMenu()
{
	InterfaceElement& viewboxholder = getViewBoxHolder();
	return viewboxholder["menu"];
}

InterfaceElement& Observer::getDiplomacyBox()
{
	InterfaceElement& viewboxholder = getViewBoxHolder();
	return viewboxholder["diplomacy"];
}

InterfaceElement& Observer::getReportBox()
{
	InterfaceElement& viewboxholder = getViewBoxHolder();
	return viewboxholder["report"]["it"];
}

InterfaceElement& Observer::getMissionBox()
{
	InterfaceElement& viewboxholder = getViewBoxHolder();
	return viewboxholder["mission"]["it"];
}

InterfaceElement& Observer::getChatHistory()
{
	InterfaceElement& viewboxholder = getViewBoxHolder();
	return viewboxholder["chat"]["history"];
}

InterfaceElement& Observer::getChatHistoryPreview()
{
	InterfaceElement& preview = getChatPreview();
	DEBUG_ASSERT(isChatEnabled());
	return preview["history"];
}

InterfaceElement& Observer::getChatPreview()
{
	return _layout["center"]["bot"]["chat"];
}

InterfaceElement& Observer::getStatusBar()
{
	return _layout["center"]["top"]["statusbar"];
}

InterfaceElement& Observer::getSeasonBar()
{
	return _layout["center"]["top"]["seasonbox"];
}

InterfaceElement& Observer::getSeasonTextField()
{
	InterfaceElement& seasonbar = getSeasonBar();
	return seasonbar["season"];
}

InterfaceElement& Observer::getWallet()
{
	return _layout["right"]["top"]["other"]["wallet"];
}

InterfaceElement& Observer::getDiplomacyNub()
{
	return _layout["center"]["top"]["diplomacy"];
}

InterfaceElement& Observer::getMissionNub()
{
	return _layout["center"]["top"]["mission"];
}

InterfaceElement& Observer::getMenuNub()
{
	return _layout["center"]["top"]["menu"];
}

InterfaceElement& Observer::getPhaseGraphic()
{
	return _layout["right"]["top"]["phasegraphic"];
}

bool Observer::doesViewportHaveFocus()
{
	InterfaceElement& viewboxholder = getViewBoxHolder();
	std::string tag = viewboxholder.getTag();
	return (tag == "viewport" || tag == "main");
}

void Observer::adjustCameraToViewport()
{
	InterfaceElement& viewport = getViewport();
	Pixel center = viewport.topleft();
	center.xenon += viewport.width() / 2;
	center.yahoo += viewport.height() / 2;

	Camera::get()->adjustFocus(center);
}

int Observer::viewportWidth()
{
	InterfaceElement& viewport = getViewport();
	return viewport.width() / Camera::get()->scale();
}

int Observer::viewportHeight()
{
	InterfaceElement& viewport = getViewport();
	return viewport.height() / Camera::get()->scale();
}

void Observer::update()
{
	Animator::update();

	adjustCameraToViewport();
	_camerafocus->update();

	// Because some items might occlude others, we must know which layer the mouse operates in.
	// We therefore need to check all UI items to see if they are hovered.
	_underlayout.hovered();
	_layout.hovered();
	_popuplayout.hovered();
	hoveredContexts();

	updatePhaseGraphic();
	updateCards();

	if (updateOtherCursor()) {}
	else
	{
		_cursor->update();

		for (Guide& guide : _guides)
		{
			guide.update();
		}
	}

	updateWallet();

	_underlayout.refresh();
	_layout.refresh();
	_popuplayout.refresh();

	_level.update();

	_arranger.update();

	// Determine hovered square.
	bool cursorOnSurface;
	{
		Input* input = Input::get();
		Pixel pixel(input->mousePixel());
		Point point(input->mousePoint());
		Position position(Surface::convert(point));
		Square* square = _level.cell(position);

		cursorOnSurface = (pixel.proximity.layer() <= Layer::SURFACE);

		if (square->edge())
		{
			_hoversquare = nullptr;
		}
		else if (cursorOnSurface)
		{
			input->layerMouse(Layer::SURFACE);
			_hoversquare = square;
		}
		else
		{
			_hoversquare = nullptr;
		}

		// Figures can extend beyond the top of the square, so we might be hovering elsewhere.
		if (_settings.selectormode.value() == SelectorMode::FIGURE &&
			(  square->south()->air().hovered()
			|| square->south()->ground().hovered()
			|| square->south()->bypass().hovered()
			|| square->south()->tile().hovered()))
		{
			_hoversquare = square->south();
		}
	}

	if (_cursor)
	{
		if (_hoversquare != nullptr) _cursor->set(_hoversquare->cell());
		else _cursor->set(Cell::undefined());
	}

	if (_cursor && !_cursor->showRealCursor()
		&& !_underlayout.hovered()
		&& !_layout.hovered()
		&& !_popuplayout.hovered()
		&& !ImGui::GetIO().WantCaptureMouse)
	{
		SDL_ShowCursor(SDL_DISABLE);
	}
	else SDL_ShowCursor(SDL_ENABLE);

	InterfaceElement& statusbar = getStatusBar();
	if (!statusbar.contains("tempo") || !statusbar.contains("juice"))
	{
		// Nothing.
	}
	else if (_cursor && cursorOnSurface
		&& _cursor->state() == Cursor::State::BUSY
		&& !ImGui::GetIO().WantCaptureMouse)
	{
		if (Input::get()->isKeyHeld(SDL_SCANCODE_RMB) && _pauseJuice > 0)
		{
			if (_game.planningTimeTotal() > 0)
			{
				_pauseJuice -= Loop::delta();
			}
			Loop::changeTempo(0);
			statusbar["tempo"].setTag("Pause");
			statusbar["juice"].showIf(true);
			float value = _pauseJuice / std::max(1.0f, _pauseJuiceDisplayMax);
			int width = std::max(0.0f, std::min(value, 1.0f))
				* statusbar["filler"].width();
			statusbar["juice"].fixWidth(width);
			statusbar.settle();
		}
		else if (Input::get()->isKeyHeld(SDL_SCANCODE_LMB))
		{
			Loop::changeTempo(3);
			statusbar["tempo"].setTag("Fast");
			statusbar["juice"].showIf(false);
		}
		else
		{
			Loop::changeTempo(1);
			statusbar["tempo"].setTag("Empty");
			statusbar["juice"].showIf(false);
		}
	}
	else
	{
		Loop::changeTempo(1);
		statusbar["tempo"].setTag("Empty");
		statusbar["juice"].showIf(false);
	}

	/* IMGUI */
	static bool show = false;
	bool wasshown = show;

	if (Input::get()->isDebugKeyHeld())
	{
		if (ImGui::Begin("Windows", nullptr,
			ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse))
		{
			ImGui::Checkbox("Animations", &show);
		}
		ImGui::End();
	}

	if (show)
	{
		if (!wasshown) ImGui::SetNextWindowCollapsed(false);

		if (ImGui::Begin("Animations", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::Text("There are %ld animations playing...", _animations.use_count());
			ImGui::Checkbox("Pause after changeset until Space", &_pause);
			ImGui::Checkbox("Skip animations", &_skipanimations);
			ImGui::Checkbox("Skip lighting", &_skiplighting);
			ImGui::Checkbox("Skip panning", &_skippanning);
			ImGui::Checkbox("Mute planning bell", &_silentplanningbell);
			{
				bool old = _hideidleanimations;
				ImGui::Checkbox("Hide idle animations", &_hideidleanimations);
				if (_hideidleanimations && !old) uncheckIdleUnits();
				if (!_hideidleanimations && old) checkIdleUnits();
			}
			ImGui::Checkbox("Hide UI", &_hideLayouts);
			ImGui::Checkbox("Show viewport", &_showViewport);
			ImGui::Separator();
			float oldlight = _level.cell(Position(0, 0))->light();
			float newlight = oldlight;
			ImGui::DragFloat("Lighting", &newlight, 0.05f, 0.0f, 2.0f);
			if (newlight != oldlight)
			{
				light(newlight, 0);
			}
		}
		ImGui::End();
	}

	_underlayout.showIf(!_hideLayouts);
	_layout.showIf(!_hideLayouts);
	_popuplayout.showIf(!_hideLayouts);
	getViewport().bearIf(_showViewport);
	{
		InterfaceElement& holder = getViewBoxHolder();
		bool showCards = (!_showViewport
			&& _cursor->state() == Cursor::State::ACTIVE);
		if (showCards && holder.getTag() == "viewport")
		{
			holder.setTag("main");
		}
		else if (!showCards && holder.getTag() == "main")
		{
			holder.setTag("viewport");
		}
	}

	// Wait with processing next change set until all animations are done.
	if (_animations.expired())
	{
		if (!_panning)
		{
			if (_animating)
			{
				// Destroy the dying figures and units.
				for (Square& square : _level)
				{
					square.cleanup();
				}
				_animating = false;
				if (_phase != Phase::PLANNING)
				{
					if (!_gameover)
					{
						_cursor->setState(Cursor::State::WAITING);
					}
					else
					{
						_cursor->setState(Cursor::State::ACTIVE);
					}
				}
			}

			InterfaceElement& viewboxholder = getViewBoxHolder();
			if (!_unprocessedChunks.empty())
			{
				if ((_pause || _pauseOnce)
					&& !Input::get()->wasKeyPressed(SDL_SCANCODE_SPACE)
					&& !_underlayout["pauseoverlay"].clicked())
				{
					if (_pauseOnce)
					{
						_underlayout["pauseoverlay"].bearIf(true);
					}
				}
				else if (_cursor->state() == Cursor::State::READING
					&& (viewboxholder.getTag() == "report"
						|| viewboxholder.getTag() == "mission"))
				{
					// Wait for the user to read or for the timer to run out.
				}
				else if (viewboxholder.getTag() == "report"
					|| viewboxholder.getTag() == "mission")
				{
					// Wait for user to read and close report.
					_cursor->setState(Cursor::State::READING);

					if (_game.planningTimeTotal() > 0)
					{
						// Wait for at most 5 seconds.
						addAnimation(Animation(nullptr, [this](float) {

							InterfaceElement& holder = getViewBoxHolder();
							if (_cursor->state() == Cursor::State::READING
								&& (holder.getTag() == "report"
									|| holder.getTag() == "mission"))
							{
								_cursor->setState(Cursor::State::BUSY);
								holder.setTag("main");
							}
						}, 0, 5));
					}
				}
				else
				{
					_pauseOnce = false;
					_underlayout["pauseoverlay"].kill();

					if (!_skipanimations && !_skippanning)
					{
						panCamera();
					}
					_panningJuice = 10;
					_panning = true;
					_animating = true;
					_cursor->setState(Cursor::State::BUSY);
				}
			}
			else if (!_unprocessedChanges.empty())
			{
				if ((_pause || _pauseOnce)
					&& !Input::get()->wasKeyPressed(SDL_SCANCODE_SPACE)
					&& !_underlayout["pauseoverlay"].clicked())
				{
					if (_pauseOnce)
					{
						_underlayout["pauseoverlay"].bearIf(true);
					}
				}
				else if (_cursor->state() == Cursor::State::READING
					&& (viewboxholder.getTag() == "report"
						|| viewboxholder.getTag() == "mission"))
				{
					// Wait for the user to read or for the timer to run out.
				}
				else if (viewboxholder.getTag() == "report"
					|| viewboxholder.getTag() == "mission")
				{
					// Wait for user to read and close report.
					_cursor->setState(Cursor::State::READING);

					if (_game.planningTimeTotal() > 0)
					{
						// Wait for at most 5 seconds.
						addAnimation(Animation(nullptr, [this](float) {

							InterfaceElement& holder = getViewBoxHolder();
							if (_cursor->state() == Cursor::State::READING
								&& (holder.getTag() == "report"
									|| holder.getTag() == "mission"))
							{
								_cursor->setState(Cursor::State::BUSY);
								holder.setTag("main");
							}
						}, 0, 5));
					}
				}
				else
				{
					_pauseOnce = false;
					_underlayout["pauseoverlay"].kill();

					if (!_skipanimations && !_skippanning)
					{
						separateIntoChunks();
						panCamera();
					}
					_panningJuice = 10;
					_panning = true;
					_animating = true;
					_cursor->setState(Cursor::State::BUSY);
				}
			}
			else if (_delayedEnabler)
			{
				_skipanimations = false;
				_delayedEnabler = false;
			}
		}
		else
		{
			if (_panningJuice > 0)
			{
				_panningJuice -= Loop::delta();
			}
			else _camerafocus->stop();

			if (!_camerafocus->panning())
			{
				_panning = false;
				processChanges();
			}
		}
	}

	control();
}

void Observer::checkMarkerCard(stringref name, bool on,
	const char* iconname,
	const std::string& title, const std::string& description)
{
	InterfaceElement& cards = getCardView();
	if (cards.contains(name))
	{
		cards[name].bearIf(on);
	}
	else if (on)
	{
		cards.add(name, makeCard(makeIcon(iconname), title, description));
	}
}

void Observer::checkAirUnitCard(const UnitType& unittype)
{
	InterfaceElement& cards = getCardView();
	std::string unitname = ::stringify(_bible.typeword(unittype));
	std::string prefix = "air/";
	std::string name = prefix + unitname;

	for (size_t i = 0; i < cards.size(); i++)
	{
		if (cards.name(i).find(prefix) == 0)
		{
			cards[cards.name(i)].bearIf(cards.name(i) == name);
		}
	}

	if (unittype == UnitType::NONE)
	{
		return;
	}

	if (!cards.contains(name))
	{
		auto icon = makeOrderIcon(spritenameOrderIcon(unittype));
		std::string title = unitName(unittype);
		std::string description =
			_("Slow air unit that deploys gas in an area.");
		cards.add(name, makeCard(std::move(icon), title, description));
	}
}

void Observer::checkGroundUnitCard(const UnitType& unittype)
{
	InterfaceElement& cards = getCardView();
	std::string prefix = "ground/";
	std::string name = prefix + ::stringify(_bible.typeword(unittype));

	for (size_t i = 0; i < cards.size(); i++)
	{
		if (cards.name(i).find(prefix) == 0)
		{
			cards[cards.name(i)].bearIf(cards.name(i) == name);
		}
	}

	if (unittype == UnitType::NONE)
	{
		return;
	}

	if (!cards.contains(name))
	{
		auto icon = makeOrderIcon(spritenameOrderIcon(unittype));
		std::string title = unitName(unittype);
		std::string description = "";
		std::string DESCRIPTIONS[array_size(KNOWN_UNIT_TYPES)] = {
			// NONE:
			_("All-round infantry unit that can dig trenches."),
			// rifleman:
			_("All-round infantry unit that can dig trenches."),
			// gunner:
			_("Shoots three times and can lock down a space."),
			// sapper:
			_("Can bombard far-away tiles."),
			// tank:
			_("Does not attack but has a powerful shell ability."),
			// settler:
			_("Can build settlements."),
			// militia:
			_("Fast but fragile infantry unit that can build outposts."),
		};
		for (size_t i = 0; i < array_size(KNOWN_UNIT_TYPES); i++)
		{
			if (_bible.typeword(unittype) == KNOWN_UNIT_TYPES[i])
			{
				description = DESCRIPTIONS[i];
				break;
			}
		}
		cards.add(name, makeCard(std::move(icon), title, description));
	}
}

void Observer::checkTileCard(const TileType& tiletype)
{
	InterfaceElement& cards = getCardView();
	std::string prefix = "tile/";
	std::string name = prefix + ::stringify(_bible.typeword(tiletype));

	for (size_t i = 0; i < cards.size(); i++)
	{
		if (cards.name(i).find(prefix) == 0)
		{
			cards[cards.name(i)].bearIf(cards.name(i) == name);
		}
	}

	if (tiletype == TileType::NONE)
	{
		return;
	}

	if (!cards.contains(name))
	{
		auto icon = makeOrderIcon(spritenameOrderIcon(tiletype));
		std::string title = tileName(tiletype);
		std::string description = "";
		std::string DESCRIPTIONS[array_size(KNOWN_TILE_TYPES)] = {
			// NONE:
			_("Inaccessible to ground units."),
			// grass:
			_("An inspiring grassland."),
			// dirt:
			_("Left over after grass was destroyed."),
			// desert:
			_("An arid wasteland."),
			// rubble:
			_("Left over after a structure was destroyed."),
			// ridge:
			_("Inaccessible to ground units."),
			// mountain:
			_("Inaccessible to ground units."),
			// water:
			_("Inaccessible to ground units."),
			// forest:
			_("Trees provide cover for your troops."),
			// city:
			_("Capture or destroy all enemy Cities to win."),
			// town:
			_("Generates income at night."),
			// outpost:
			_("Produces militia and settlers."),
			// industry:
			_("Produces tanks and generates income at night."),
			// barracks:
			_("Produces specialized military units."),
			// airfield:
			_("Produces zeppelins."),
			// farm:
			_("Cultivates surrounding spaces when built."),
			// soil:
			_("Crops can grow here."),
			// crops:
			_("Harvested for income at night."),
			// trenches:
			_("Provides cover from bullets."),
		};
		for (size_t i = 0; i < array_size(KNOWN_TILE_TYPES); i++)
		{
			if (_bible.typeword(tiletype) == KNOWN_TILE_TYPES[i])
			{
				description = DESCRIPTIONS[i];
				break;
			}
		}
		cards.add(name, makeCard(std::move(icon), title, description));
	}
}

void Observer::checkOrderCard(const Order::Type& ordertype, bool revoke)
{
	InterfaceElement& cards = getCardView();
	std::string prefix = "order/";
	std::string name = prefix + Order::stringify(ordertype);
	if (revoke)
	{
		name = prefix + "revoke";
	}

	for (size_t i = 0; i < cards.size(); i++)
	{
		if (cards.name(i).find(prefix) == 0)
		{
			cards[cards.name(i)].bearIf(cards.name(i) == name);
		}
	}

	if (ordertype == Order::Type::NONE)
	{
		return;
	}

	if (cards.contains(name))
	{
		return;
	}

	auto icon = makeOrderIcon(spritenameOrderIcon(ordertype));
	std::string title = "";
	std::string description = "";
	switch (ordertype)
	{
		case Order::Type::FOCUS:
		{
			title = _("Focus");
			description = _(""
				"Attack an enemy."
				" All friendly ground units adjacent to the target"
				" join the attack."
				"");
		}
		break;
		case Order::Type::GUARD:
		case Order::Type::LOCKDOWN:
		{
			title = _("Lockdown");
			description = _(""
				"Attack an enemy and lock down a space."
				" Any enemy entering that space"
				" is attacked immediately and stops moving."
				"");
		}
		break;
		case Order::Type::SHELL:
		{
			title = _("Shell");
			description = _(""
				"Fire two volleys of explosive rounds"
				" at an adjacent space,"
				" dealing indiscriminate damage."
				"");
		}
		break;
		case Order::Type::BOMBARD:
		{
			title = _("Bombard");
			description = _(""
				"Fire mortar rounds"
				" at a space at long range,"
				" dealing indiscriminate damage."
				"");
		}
		break;
		case Order::Type::BOMB:
		{
			title = _("Drop Gas");
			description = _(""
				"Release a lethal gas in the space below."
				" Gas spreads to surrounding spaces"
				" at the end of the Decay phase."
				"");
		}
		break;
		case Order::Type::CAPTURE:
		{
			title = _("Capture");
			description = _(""
				"Take control of an enemy tile."
				"");
		}
		break;
		case Order::Type::HALT:
		{
			title =  _("Stop");
			description = _(""
				"Halt previous activities."
				"");
		}
		break;
		case Order::Type::UPGRADE:
		{
			title =  _("Upgrade");
			description = _(""
				"Increase the number of buildings on this tile."
				"");
		}
		break;
		case Order::Type::MOVE:
		case Order::Type::SHAPE:
		case Order::Type::SETTLE:
		case Order::Type::EXPAND:
		case Order::Type::CULTIVATE:
		case Order::Type::PRODUCE:
		case Order::Type::NONE:
		break;
	}

	if (revoke)
	{
		icon = makeOrderIcon("ui/ordericon_revoke");
		title =  _("Revoke");
		description = _(""
			"Cancel an order."
			"");
	}

	cards.add(name, makeCard(std::move(icon), title, description));
}

Order Observer::getHoveredOrder()
{
	return Order();
}

void Observer::updateCards()
{
	if (getViewBoxHolder().getTag() != "main") return;
	if (getCardView().hovered()) return;

	Order order = getHoveredOrder();

	bool drought = false;
	bool snow = false;
	bool frostbite = false;
	bool coldfeet = false;
	bool firestorm = false;
	bool bonedrought = false;
	bool gas = false;
	if (order.type != Order::Type::NONE)
	{
		gas = (order.type == Order::Type::BOMB);
		coldfeet = (_bible.frostbiteGivesColdFeet()
			&& _selector.type == Descriptor::Type::GROUND
			&& _selectsquare->frostbite()
			&& _bible.chaosMinFrostbite(_season) < 0);
	}
	else if (_hoversquare)
	{
		drought = (_hoversquare->humidity() <= _bible.humidityMin()
				&& (_bible.tileGrassy(_hoversquare->tile().type)
					|| _bible.tileBuildable(_hoversquare->tile().type)));
		snow = _hoversquare->snow();
		frostbite = _hoversquare->frostbite() && _hoversquare->current();
		if (_bible.frostbiteGivesColdFeet() && frostbite
			// Show a different card in Spring than in Winter.
			&& _bible.chaosMinFrostbite(_season) < 0)
		{
			frostbite = false;
			coldfeet = true;
		}
		firestorm = _hoversquare->firestorm() && _hoversquare->current();
		bonedrought = _hoversquare->bonedrought() && _hoversquare->current();
		gas = _hoversquare->gas() && _hoversquare->current();
	}

	checkMarkerCard("drought", drought,
		"ui/markericon_death", // TODO #1371 change
		_("Drought"),
		_(""
		"Crops and trees will no longer grow here."
		""));

	// TODO #1371 add a card for tileGrassy()?
	checkMarkerCard("snow", snow,
		"ui/markericon_snow",
		_("Snow"),
		_(""
		"Ground units are slowed."
		""));
	checkMarkerCard("frostbite", frostbite,
		"ui/markericon_frostbite",
		_("Frostbite"),
		_(""
		"Ground units that remain here become Chilled in the Decay phase."
		" Chilled units cannot attack or use abilities until they move."
		""));
	checkMarkerCard("coldfeet", coldfeet,
		"ui/markericon_frostbite",
		_("Chilled"),
		_(""
		"This unit cannot attack or use abilities until it moves."
		""));
	checkMarkerCard("firestorm", firestorm,
		"ui/markericon_firestorm",
		_("Firestorm"),
		_(""
		"Units that remain here take damage in the Decay phase."
		""));
	checkMarkerCard("bonedrought", bonedrought,
		"ui/markericon_bonedrought",
		_("Bonedrought"),
		_(""
		"Units and tiles have fewer hitpoints."
		""));
	checkMarkerCard("gas", gas,
		"ui/markericon_gas",
		_("Gas"),
		_(""
		"Ground units have fewer hitpoints."
		" Ground units that remain here take damage in the Decay phase."
		""));

	Order::Type ordertype = Order::Type::NONE;
	bool revoke = false;
	UnitType airtype = UnitType::NONE;
	UnitType groundtype = UnitType::NONE;
	TileType tiletype = TileType::NONE;
	if (order.type != Order::Type::NONE)
	{
		switch (order.type)
		{
			case Order::Type::SHAPE:
			case Order::Type::SETTLE:
			case Order::Type::EXPAND:
			case Order::Type::CULTIVATE:
			{
				tiletype = order.tiletype;
			}
			break;

			case Order::Type::UPGRADE:
			{
				if (order.tiletype == TileType::NONE)
				{
					ordertype = order.type;
				}
				else
				{
					tiletype = order.tiletype;
				}
			}
			break;

			case Order::Type::PRODUCE:
			{
				if (_bible.unitAir(order.unittype))
				{
					airtype = order.unittype;
				}
				else
				{
					groundtype = order.unittype;
				}
			}
			break;

			case Order::Type::GUARD:
			case Order::Type::FOCUS:
			case Order::Type::LOCKDOWN:
			case Order::Type::SHELL:
			case Order::Type::BOMBARD:
			case Order::Type::BOMB:
			case Order::Type::CAPTURE:
			case Order::Type::HALT:
			{
				ordertype = order.type;

				if (order.target)
				{
					// This is a little hack so we know it is a revoke
					// pseudo-order. See Commander::getHoveredOrder().
					revoke = true;
				}
			}
			break;

			case Order::Type::MOVE:
			case Order::Type::NONE:
			break;
		}
	}
	else if (_hoversquare)
	{
		airtype = _hoversquare->air().type;
		groundtype = _hoversquare->ground().type;
		tiletype = _hoversquare->tile().type;
	}

	checkOrderCard(ordertype, revoke);
	checkAirUnitCard(airtype);
	checkGroundUnitCard(groundtype);
	checkTileCard(tiletype);

	InterfaceElement& cards = getCardView();
	std::vector<std::string> removals;
	for (size_t i = 0; i < cards.size(); i++)
	{
		if (!cards[cards.name(i)].born())
		{
			removals.emplace_back(cards.name(i));
		}
	}
	for (const std::string& name : removals)
	{
		cards.remove(name);
	}
	int h = getViewBoxHolder().height();
	cards.unfixHeight();
	cards.setHeight(h);
	cards.place(cards.topleft());
}

void Observer::control()
{
	if (_phase == Phase::PLANNING && _game.planningTimeTotal() > 0)
	{
		float fraction = std::min(_game.planningTimeSpent() / _game.planningTimeTotal(), 1.0f);
		InterfaceElement& statusbar = getStatusBar();
		int width = fraction * statusbar["filler"].width();
		statusbar["timer"].fixWidth(width);
	}

	if (_popuplayout["quit"]["buttons"]["quit"].clicked())
	{
		_game.confirmQuit();
		return;
	}

	if (_popuplayout["quit"]["buttons"]["cancel"].clicked())
	{
		_popuplayout["quitoverlay"].kill();
		_popuplayout["quit"].kill();
	}

	InterfaceElement& viewboxholder = getViewBoxHolder();
	InterfaceElement& chatpreview = getChatPreview();
	InterfaceElement& diplomacybutton = getDiplomacyNub();
	InterfaceElement& reportbutton = getSeasonBar();
	InterfaceElement& missionbutton = getMissionNub();
	InterfaceElement& menubutton = getMenuNub();
	InterfaceElement& menu = getMenu();
	if (viewboxholder.getTag() == "chat")
	{
		InterfaceElement& chat = viewboxholder["chat"];

		if (_chatmodeTarget.size() > 1)
		{
			InterfaceElement& button = chat["inputline"]["indicator"]["button"];
			if (button.clicked())
			{
				_chatmode = (_chatmode + 1) % _chatmodeTarget.size();
				button.setText(_chatmodeLabel[_chatmode]);
				button.setTextColor(_chatmodeColor[_chatmode]);
			}
		}

		InterfaceElement& sendbutton = chat["inputline"]["sendbutton"];
		sendbutton.enableIf(!chat["inputline"]["input"].text().empty());

		if (sendbutton.clicked())
		{
			std::string input = chat["inputline"]["input"].text();
			if (!input.empty())
			{
				_game.chat(input, _chatmodeTarget[_chatmode]);
			}
			chat["inputline"]["input"].reset();
			sendbutton.disable();

			if (Input::get()->wasKeyPressed(SDL_SCANCODE_ENTER)
				&& !Input::get()->isKeyHeld(SDL_SCANCODE_SHIFT))
			{
				_camerafocus->enableKeys();
				viewboxholder.setTag("main");
			}
		}
		else if (chatpreview.clicked())
		{
			_camerafocus->enableKeys();
			viewboxholder.setTag("main");
		}
		else if (menubutton.clicked())
		{
			_camerafocus->enableKeys();
			viewboxholder.setTag("menu");
		}
		else if (diplomacybutton.clicked())
		{
			_camerafocus->enableKeys();
			viewboxholder.setTag("diplomacy");
		}
		else if (reportbutton.clicked())
		{
			Mixer::get()->queue(Clip::Type::CHAOSREPORT, 0.0f);
			_camerafocus->enableKeys();
			viewboxholder.setTag("report");
		}
		else if (missionbutton.clicked())
		{
			_camerafocus->enableKeys();
			viewboxholder.setTag("mission");
		}
		else if (Input::get()->wasKeyPressed(SDL_SCANCODE_LMB)
			&& !chatpreview.clicked()
			&& !viewboxholder["chat"].clicked())
		{
			_camerafocus->enableKeys();
			viewboxholder.setTag("main");
		}

		if (sendbutton.enabled())
		{
			chatpreview.setHotkeyScancode(0);
		}
		else
		{
			chatpreview.setHotkeyScancode(SDL_SCANCODE_ENTER);
		}
	}
	else
	{
		if (isChatEnabled() && chatpreview.clicked())
		{
			_camerafocus->disableKeys();
			viewboxholder.setTag("chat");
		}
		else if (menubutton.clicked())
		{
			if (viewboxholder.getTag() != "menu")
			{
				viewboxholder.setTag("menu");
			}
			else
			{
				viewboxholder.setTag("main");
			}
		}
		else if (diplomacybutton.clicked())
		{
			if (viewboxholder.getTag() != "diplomacy")
			{
				viewboxholder.setTag("diplomacy");
			}
			else
			{
				viewboxholder.setTag("main");
			}
		}
		else if (reportbutton.clicked())
		{
			if (viewboxholder.getTag() != "report")
			{
				Mixer::get()->queue(Clip::Type::CHAOSREPORT, 0.0f);
				viewboxholder.setTag("report");
			}
			else
			{
				viewboxholder.setTag("main");
			}
		}
		else if (missionbutton.clicked())
		{
			if (viewboxholder.getTag() != "mission")
			{
				viewboxholder.setTag("mission");
			}
			else
			{
				viewboxholder.setTag("main");
			}
		}
		else if (menu.clicked())
		{
			if (menu["buttons"]["quit"].clicked())
			{
				viewboxholder.setTag("main");
				openQuitPopup();
			}
			else if (menu["buttons"]["chat"].clicked())
			{
				_camerafocus->disableKeys();
				viewboxholder.setTag("chat");
			}
			else if (menu["buttons"]["continue"].clicked())
			{
				viewboxholder.setTag("main");
			}
		}
		else if (Input::get()->wasKeyPressed(SDL_SCANCODE_SPACE)
			|| Input::get()->wasKeyPressed(SDL_SCANCODE_LMB))
		{
			if (viewboxholder.getTag() != "main"
				&& viewboxholder.getTag() != "viewport")
			{
				viewboxholder.setTag("main");
			}
		}

		chatpreview.setHotkeyScancode(SDL_SCANCODE_ENTER);
	}
}

void Observer::setAnimation(bool animate)
{
	if (!animate) _skipanimations = true;
	// we only want to enable animations after we have processed all changes
	else if (_unprocessedChunks.empty() && _unprocessedChanges.empty())
	{
		_skipanimations = false;
	}
	else _delayedEnabler = true;
}

bool Observer::animating() const
{
	return _panning || _animating
		|| !_unprocessedChunks.empty()
		|| !_unprocessedChanges.empty();
}

void Observer::attemptQuit()
{
	InterfaceElement& viewboxholder = getViewBoxHolder();
	if (viewboxholder.getTag() == "chat")
	{
		_camerafocus->enableKeys();
		viewboxholder.setTag("main");
		return;
	}
	else if (viewboxholder.getTag() != "main"
		&& viewboxholder.getTag() != "viewport")
	{
		viewboxholder.setTag("main");
		return;
	}

	if (quitSelectorContext())
	{
		return;
	}

	if (quitOrderContext())
	{
		return;
	}

	if (quitTargetCursor())
	{
		return;
	}

	openQuitPopup();
}

void Observer::openQuitPopup()
{
	if (_popuplayout["defeat"].born())
	{
		_popuplayout["defeat"].kill();
	}

	if (_popuplayout["victory"].born())
	{
		_popuplayout["victory"].kill();
	}

	if (_popuplayout["quit"].born())
	{
		return;
	}

	_popuplayout["quitoverlay"].bear();
	_popuplayout["quit"].bear();
}

void Observer::addChatmode(const std::string& target, const std::string& label,
		const Paint& color)
{
	if (_chatmodeTarget.size() == 1 && _chatmodeTarget[0].empty())
	{
		_chatmodeTarget[0] = target;
		_chatmodeLabel[0] = label;
		_chatmodeColor[0] = color;
	}
	else
	{
		_chatmodeTarget.push_back(target);
		_chatmodeLabel.push_back(label);
		_chatmodeColor.push_back(color);
	}
}

void Observer::setChatmode(const std::string& target)
{
	_chatmode = getChatmode(target);

	InterfaceElement& viewboxholder = getViewBoxHolder();
	InterfaceElement& chat = viewboxholder["chat"];
	InterfaceElement& button = chat["inputline"]["indicator"]["button"];

	button.setText(_chatmodeLabel[_chatmode]);
	button.setTextColor(_chatmodeColor[_chatmode]);
}

size_t Observer::getChatmode(const std::string& target)
{
	for (size_t i = 0; i < _chatmodeTarget.size(); i++)
	{
		if (target == _chatmodeTarget[i])
		{
			return i;
		}
	}
	return 0;
}

bool Observer::isChatEnabled()
{
	return (_game.online() || _game.test());
}

static void emplaceInHistory(InterfaceElement& history,
	const std::string& message, int fontsize, const Paint& color)
{
	TextField* textField = new TextField(message, fontsize, color);
	history.add(std::to_string(history.size()), textField);
	history.setWidth(history.width());

	while (!textField->textLeftover().empty())
	{
		textField = new TextField(textField->textLeftover(), fontsize, color);
		history.add(std::to_string(history.size()), textField);
		history.setWidth(history.width());
	}
}

void Observer::message(const std::string& message)
{
	const int FONTSIZE = _settings.getFontSize();

	if (!isChatEnabled()) return;

	emplaceInHistory(getChatHistoryPreview(),
		message, FONTSIZE, ColorName::TEXTANNOUNCEMENT);
	emplaceInHistory(getChatHistory(),
		message, FONTSIZE, ColorName::TEXTANNOUNCEMENT);
}

void Observer::chat(const std::string& user, const std::string& message,
	const std::string& target)
{
	const int FONTSIZE = _settings.getFontSize();

	if (!isChatEnabled()) return;

	size_t m = getChatmode(target);
	Paint color = _chatmodeColor[m];
	std::string string = user + ": " + message;
	emplaceInHistory(getChatHistoryPreview(),
		string, FONTSIZE, color);
	emplaceInHistory(getChatHistory(),
		string, FONTSIZE, color);
}

void Observer::assignColor(const std::string& name,
		const Player& player)
{
	addPlayerToDiplomacyBox(player, name);
}

void Observer::setSkins(const Json::Value& metadata)
{
	_skinner.load(metadata);
}

bool Observer::isChallenge()
{
	InterfaceElement& mission = getMissionBox();
	return mission.contains("lines");
}

void Observer::receiveBriefing(const Json::Value& metadata)
{
	fillMissionBox(metadata);

	int w = _layout["center"]["top"].width();
	_layout["center"]["top"].replace("mission", makeMissionNub());
	_layout["center"]["top"].unfixHeight();
	_layout["center"]["top"].settleHeight();
	_layout["center"]["top"].fixHeight();
	_layout["center"]["top"]["statusbar"].unfixWidth();
	_layout["center"]["top"].setWidth(w);
	_layout["center"]["top"]["statusbar"].fixWidth();

	_layout.settle();

	// Adjust the victory and defeat messages.
	if (_player == Player::OBSERVER)
	{
		_popuplayout["victory"]["text"].setText(
			_("The player was victorious!"));
		_popuplayout["victory"]["text"].unfixWidth();
		_popuplayout["victory"]["text"].settleWidth();
		_popuplayout["victory"]["text"].fixWidth();

		_popuplayout["defeat"]["text"].setText(
			_("The player was defeated."));
		_popuplayout["defeat"]["text"].unfixWidth();
		_popuplayout["defeat"]["text"].settleWidth();
		_popuplayout["defeat"]["text"].fixWidth();
	}
}
