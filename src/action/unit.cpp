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
#include "unit.hpp"
#include "source.hpp"

#include "square.hpp"
#include "animationgroup.hpp"
#include "change.hpp"
#include "aim.hpp"
#include "bible.hpp"
#include "loop.hpp"
#include "notice.hpp"
#include "placementpather.hpp"
#include "figure.hpp"
#include "skinner.hpp"
#include "mixer.hpp"

Unit::Unit(Square* square, const Bible& bible, const Skinner& skinner) :
	_square(square),
	_bible(bible),
	_skinner(skinner)
{}

Unit::Unit(Unit&&) = default;

Unit::~Unit() = default;

Unit& Unit::operator=(Unit&& other)
{
	UnitToken::operator=((UnitToken) other);
	Animator::operator=((Animator) other);

	_square = other._square;
	// _bible
	// _skinner
	_figures = std::move(other._figures);
	_selected = other._selected;
	_dying = other._dying;
	_air = other._air;
	_premoving = other._premoving;
	_premoveDuration = other._premoveDuration;
	_idleTimer = other._idleTimer;

	return *this;
}

PlacementBox Unit::newPlacement()
{
	_air = _bible.unitAir(type);
	Descriptor::Type goa = _air ? Descriptor::Type::AIR : Descriptor::Type::GROUND;
	PlacementBox box = _square->place(goa);
	if (isEntrenched(_square, goa)) box.entrench();
	else box.raise();
	return box;
}

std::shared_ptr<Figure> Unit::newFigure()
{
	PlacementBox box = newPlacement();
	return std::make_shared<Figure>(std::move(box),
		_skinner.figure(type, owner),
		owner);
}

void Unit::update()
{
	if (_idleTimer >= 0)
	{
		_idleTimer -= Loop::delta();
		if (_idleTimer < 0)
		{
			for (size_t i = 0; i < (size_t) std::max(0, (int) stacks); i++)
			{
				if (i >= _figures.size() || !_figures[i])
				{
					LOGW << "nullptr detected in _figures";
					continue;
				}
				std::shared_ptr<Figure>& figure = _figures[i];
				figure->idle(i * 0.050f);
			}
			_idleTimer = 5.0f;
		}
	}

	Animator::update();

	// All figures need to be updated, not just those in [0, stacks).
	for (auto& figure : _figures)
	{
		if (!figure)
		{
			LOGW << "nullptr detected in _figures in unit update";
			continue;
		}
		figure->update();
	}
}

bool Unit::hovered() const
{
	// Because some items might occlude others, we must know which layer the mouse operates in.
	// We therefore need to check all items to see if they are hovered.
	bool result = false;

	for (size_t i = 0; i < (size_t) std::max(0, (int) stacks); i++)
	{
		if (i >= _figures.size() || !_figures[i])
		{
			LOGW << "nullptr detected in _figures";
			continue;
		}
		const std::shared_ptr<Figure>& figure = _figures[i];
		if (figure->hovered()) result = true;
	}

	return result;
}

bool Unit::selected() const
{
	return _selected;
}

void Unit::select()
{
	_selected = true;
	for (size_t i = 0; i < (size_t) std::max(0, (int) stacks); i++)
	{
		if (i >= _figures.size() || !_figures[i])
		{
			LOGW << "nullptr detected in _figures";
			continue;
		}
		std::shared_ptr<Figure>& figure = _figures[i];
		figure->select();
	}
	Mixer::get()->queue(Clip::Type::UI_HOVERHIGHLIGHT, 0.0f);
	unidle();
}

void Unit::deselect()
{
	_selected = false;
	for (size_t i = 0; i < (size_t) std::max(0, (int) stacks); i++)
	{
		if (i >= _figures.size() || !_figures[i])
		{
			LOGW << "nullptr detected in _figures";
			continue;
		}
		std::shared_ptr<Figure>& figure = _figures[i];
		figure->deselect();
	}
}

void Unit::resetSquare(Square* dest)
{
	_square = dest;
}

void Unit::populate(std::shared_ptr<AnimationGroup> group)
{
	// There might still be some figures that have animations playing.
	// Instead of removing them, we will rotate them out of range.
	size_t leftovers = _figures.size();

	for (size_t i = 0; i < (size_t) std::max(0, (int) stacks); i++)
	{
		std::shared_ptr<Figure> figure(newFigure());
		/*
		// Disabled (#1583)
		figure->set(TRANSITION_OBSCURED, !_square->current());
		*/
		figure->set(TRANSITION_LIGHT, _square->light());
		figure->set(TRANSITION_SPOTLIGHT, _square->spotlight());
		figure->set(TRANSITION_FLASHLIGHT, _square->flashlight());
		if (group)
		{
			figure->setVisible(false);
			std::weak_ptr<Figure> weakfigure = figure;
			addAnimation(Animation(group, [weakfigure](float) {

				auto f = weakfigure.lock();
				if (!f) return;

				f->setVisible(true);
			}, 0, group->delay));
		}
		_figures.emplace_back(figure);
	}

	if (leftovers)
	{
		std::rotate(_figures.begin(), _figures.begin() + leftovers, _figures.end());
	}
}

void Unit::reanimate(std::shared_ptr<AnimationGroup> group)
{
	if (!group) return populate(group);

	// Have we moved away since the last time we disappeared?
	bool moved = (group->unitslot > 0
			&& _square->position() == Position(group->torow, group->tocol));
	if (moved)
	{
		group->delay += 0.500f + 0.800f;
	}

	// There might still be some figures that have animations playing.
	// Instead of removing them, we will rotate them out of range.
	size_t leftovers = _figures.size();
	size_t stackfigures = (size_t) std::max(0, (int) stacks);
	size_t revivedfigures = std::min(leftovers, stackfigures);

	// Revive as many figures as we can and need.
	for (size_t i = 0; i < revivedfigures; i++)
	{
		if (!_figures[i])
		{
			LOGW << "nullptr detected in _figures";
			continue;
		}

		std::shared_ptr<Figure>& figure = _figures[i];
		std::weak_ptr<Figure> weakfigure = figure;

		figure->revive();
		if (moved)
		{
			figure->reposition(group, newPlacement());
		}
		figure->setVisible(group, true, group->delay + 0.500f);
	}

	// If we need more figures than we used to have, create new figures.
	// This should never happen since units cannot gain figures mid-changeset.
	for (size_t i = revivedfigures; i < stackfigures; i++)
	{
		std::shared_ptr<Figure> figure(newFigure());
		/*
		// Disabled (#1583)
		figure->set(TRANSITION_OBSCURED, !_square->current());
		*/
		figure->set(TRANSITION_LIGHT, _square->light());
		figure->set(TRANSITION_SPOTLIGHT, _square->spotlight());
		figure->set(TRANSITION_FLASHLIGHT, _square->flashlight());
		figure->setVisible(false);
		std::weak_ptr<Figure> weakfigure = figure;
		addAnimation(Animation(group, [weakfigure](float) {

			auto f = weakfigure.lock();
			if (!f) return;

			f->setVisible(true);
		}, 0, group->delay));
		_figures.emplace_back(figure);
	}

	// Move any old figures between the revived ones and the added ones back.
	if (leftovers > stackfigures)
	{
		std::rotate(_figures.begin() + revivedfigures,
			_figures.begin() + leftovers, _figures.end());
	}

	if (moved)
	{
		group->delay += 0.200f;
	}
}

bool Unit::isEntrenched(const Square* square, const Descriptor::Type& slot)
{
	return (_bible.tileTrenches(square->tile().type)
		&& !_bible.unitMechanical(type)
		&& slot == Descriptor::Type::GROUND);
}

float Unit::animateMove(const Change& change, std::shared_ptr<AnimationGroup> group)
{
	bool start = (_premoving && change.type == Change::Type::STARTS);
	bool finish = (_premoving && !start);

	Move direction = Aim(change.subject.position, change.target.position).direction();
	Descriptor::Type goa = _air ? Descriptor::Type::AIR : Descriptor::Type::GROUND;
	Square* targetsquare = start ? _square->eswn(direction) : _square;
	PlacementPather pather(direction);
	float maxDuration = 0;
	float visualspeed = 1.0f * _bible.unitSpeed(type);
	if (!_air && _square->snow())
	{
		visualspeed = std::max(1.0f, visualspeed * 0.50f);
	}

	for (size_t i = 0; i < (size_t) std::max(0, (int) stacks); i++)
	{
		if (i >= _figures.size() || !_figures[i])
		{
			LOGW << "nullptr detected in _figures";
			continue;
		}
		std::shared_ptr<Figure>& figure = _figures[i];

		if (start)
		{
			figure->prepareForReposition();
		}
		else if (finish)
		{}
		else
		{
			figure->prepareForReposition();
		}
	}

	for (size_t i = 0; i < (size_t) std::max(0, (int) stacks); i++)
	{
		if (i >= _figures.size() || !_figures[i])
		{
			LOGW << "nullptr detected in _figures";
			continue;
		}
		std::shared_ptr<Figure>& figure = _figures[i];

		float duration;
		if (start)
		{
			PlacementBox box = targetsquare->place(goa, figure->box(), pather);
			if (isEntrenched(targetsquare, change.target.type)) box.entrench();
			else box.raise();
			duration = figure->startMoving(group, std::move(box), direction,
				visualspeed);
		}
		else if (finish)
		{
			duration = figure->finishMoving(group, direction, visualspeed);
		}
		else
		{
			PlacementBox box = targetsquare->place(goa, figure->box(), pather);
			if (isEntrenched(targetsquare, change.target.type)) box.entrench();
			duration = figure->doMoving(group, std::move(box), direction,
				visualspeed);
		}
		if (maxDuration < duration) maxDuration = duration;
	}

	return maxDuration;
}

void Unit::enact(const Change& change, std::shared_ptr<AnimationGroup> group)
{
	switch (change.type)
	{
		case Change::Type::NONE:
		{
			if (change.target.type != Descriptor::Type::NONE)
			{
				Aim aim(_square->position(), change.target.position);
				Square* targetsquare = _square;
				for (Move move : aim.deconstruct())
				{
					targetsquare = targetsquare->eswn(move);
				}
				targetsquare->tile().animateBlock(change, group, group ? group->delay : 0);
			}
			else
			{
				_square->tile().animateBlock(change, group, group ? group->delay : 0);
			}

			switch (change.notice)
			{
				case Notice::COLDFEET:
				{
					for (size_t i = 0; i < (size_t) std::max(0, (int) stacks); i++)
					{
						if (i >= _figures.size() || !_figures[i])
						{
							LOGW << "nullptr detected in _figures";
							continue;
						}
						std::shared_ptr<Figure>& figure = _figures[i];
						figure->animateChill(group, /*shiver=*/true,
							group ? group->delay : 0);
						if (group)
						{
							group->delay += 0.1f;
						}
					}
				}
				break;

				default:
				break;
			}

			if (group)
			{
				group->delay += 0.5f;
				pause(group, group->delay);
			}
		}
		break;

		case Change::Type::STARTS:
		{
			// pre-move for attack of opportunity
			_premoving = true;
			float maxDuration = animateMove(change, group);
			_premoveDuration = maxDuration;

			// Remember this unit in the case that it exits and enters again.
			if (group)
			{
				group->unitslot = (int8_t) change.target.type;
				group->fromrow = change.target.position.row;
				group->fromcol = change.target.position.col;
			}
		}
		break;

		case Change::Type::MOVES:
		{
			if (_premoving)
			{
				if (group) group->delay += _premoveDuration;
			}

			float maxDuration = animateMove(change, group);
			if (group) group->delay += maxDuration;

			if (_premoving)
			{
				_premoving = false;
			}

			// A tile might become occupied or deoccupied as a result.
			if (_bible.unitCanOccupy(type))
			{
				Square* to = _square;
				Move movedir = Aim(change.target.position,
					change.subject.position).direction();
				Square* from = to->eswn(movedir);
				float d = 0;
				if (change.subject.type != Descriptor::Type::BYPASS
					&& from->tile().owner != Player::NONE
					&& from->tile().owner != owner)
				{
					d = from->tile().animateDeoccupy(change, group,
						group ? group->delay : 0);
				}
				if (change.target.type != Descriptor::Type::BYPASS
					&& to->tile().owner != Player::NONE
					&& to->tile().owner != owner)
				{
					float d1 = to->tile().animateOccupy(change, group,
						group ? group->delay : 0);
					d = std::max(d, d1);
				}
				if (group && d > 0)
				{
					group->delay += d;
				}
			}

			// Remember this unit in the case that it exits and enters again.
			if (group)
			{
				group->unitslot = (int8_t) change.target.type;
				group->fromrow = change.target.position.row;
				group->fromcol = change.target.position.col;
			}
		}
		break;

		case Change::Type::CAPTURES:
		{
			if (group)
			{
				group->delay += 0.2f;
				pause(group, group->delay);
			}
		}
		break;

		case Change::Type::SHAPES:
		{
			Descriptor::Type goa = _air ? Descriptor::Type::AIR : Descriptor::Type::GROUND;
			float maxDuration = 0;
			float visualspeed = 1.0f * _bible.unitSpeed(type);
			if (!_air && _square->snow())
			{
				visualspeed = std::max(1.0f, visualspeed * 0.50f);
			}
			for (size_t i = 0; i < (size_t) std::max(0, (int) stacks); i++)
			{
				if (i >= _figures.size() || !_figures[i])
				{
					LOGW << "nullptr detected in _figures";
					continue;
				}
				std::shared_ptr<Figure>& figure = _figures[i];
				PlacementBox box = _square->place(goa);
				if (isEntrenched(_square, goa)) box.entrench();
				else box.raise();
				float duration = figure->doMoving(group, std::move(box),
					Move::X, visualspeed);
				if (maxDuration < duration) maxDuration = duration;
			}
			if (group) group->delay += maxDuration;
		}
		break;

		case Change::Type::SETTLES:
		{
			UnitToken::operator=(UnitToken());
			for (size_t i = 0; i < _figures.size(); i++)
			{
				if (i >= _figures.size() || !_figures[i])
				{
					LOGW << "nullptr detected in _figures";
					continue;
				}
				std::shared_ptr<Figure>& figure = _figures[i];
				figure->die();
			}
		}
		break;

		case Change::Type::PRODUCED:
		{
			UnitToken::operator=(change.unit);
			populate(group);

			for (size_t i = 0; i < (size_t) std::max(0, (int) stacks); i++)
			{
				if (i >= _figures.size() || !_figures[i])
				{
					LOGW << "nullptr detected in _figures";
					continue;
				}
				std::shared_ptr<Figure>& figure = _figures[i];
				if (group && i < group->placements.size())
				{
					// Use the spawn point that the tile prepared for us when
					// enacting PRODUCES. We will use the "box index" later.
					figure->setSpawnPoint(group->placements[i]);
					figure->setVisible(false);
					figure->setVisible(group, true, group->delay);
					figure->spawnMoving(group, group->delay);
					group->delay += 0.10f;
				}
			}

			if (group)
			{
				group->delay += 0.15f;
			}

			float visualspeed = 1.0f * _bible.unitSpeed(type);
			if (!_air && _square->snow())
			{
				visualspeed = std::max(1.0f, visualspeed * 0.50f);
			}

			float maxDuration = 0;
			PlacementPather pather(Move::X);
			for (size_t i = 0; i < (size_t) std::max(0, (int) stacks); i++)
			{
				if (i >= _figures.size() || !_figures[i])
				{
					LOGW << "nullptr detected in _figures";
					continue;
				}
				std::shared_ptr<Figure>& figure = _figures[i];
				figure->prepareForReposition();
			}
			for (size_t i = 0; i < (size_t) std::max(0, (int) stacks); i++)
			{
				if (i >= _figures.size() || !_figures[i])
				{
					LOGW << "nullptr detected in _figures";
					continue;
				}
				std::shared_ptr<Figure>& figure = _figures[i];
				figure->spawn(change, group, group ? group->delay : 0);
				if (group && i < group->indices.size())
				{
					// Use the "box index" that the tile prepared for us when
					// enacting PRODUCES. We used the spawn point above.
					_square->spawn(figure->box(), group->indices[i], pather);
				}
				if (group)
				{
					float duration = figure->finishMoving(group,
						Move::X, visualspeed);
					if (maxDuration < duration) maxDuration = duration;
				}
			}

			if (group)
			{
				group->delay += maxDuration;
				pause(group, group->delay);
			}
		}
		break;

		case Change::Type::ENTERED:
		{
			UnitToken::operator=(change.unit);
			if (!group || group->delay == 0)
			{
				populate(group);
				/*
				// Disabled (#1583)
				for (size_t i = 0; i < (size_t) std::max(0, (int) stacks); i++)
				{
					if (i >= _figures.size() || !_figures[i])
					{
						LOGW << "nullptr detected in _figures";
						continue;
					}
					std::shared_ptr<Figure>& figure = _figures[i];
					figure->set(TRANSITION_OBSCURED, 1);
					figure->transition(group, TRANSITION_OBSCURED, 0, 0.5f);
				}
				*/
			}
			else
			{
				// Because humans have object permanence, we want to keep any
				// random data the same when a Unit leaves and enters vision.
				// Therefore we reuse the same figures when it enters again.
				reanimate(group);
				/*
				// Disabled (#1583)
				for (size_t i = 0; i < (size_t) std::max(0, (int) stacks); i++)
				{
					if (i >= _figures.size() || !_figures[i])
					{
						LOGW << "nullptr detected in _figures";
						continue;
					}
					std::shared_ptr<Figure>& figure = _figures[i];
					std::weak_ptr<Figure> weakfigure = figure;
					addAnimation(Animation(group,
							[group, weakfigure](float) {

						auto f = weakfigure.lock();
						if (!f) return;

						f->set(TRANSITION_OBSCURED, 1);
						f->transition(group, TRANSITION_OBSCURED, 0, 0.5f, 0);
					}, 0, group->delay));
				}
				*/
			}

			// A tile might become occupied as a result.
			if (_bible.unitCanOccupy(type))
			{
				if (_square->tile().owner != Player::NONE
					&& _square->tile().owner != owner)
				{
					_square->tile().animateOccupy(change, group,
						group ? (group->delay + 0.250f) : 0);
				}
			}
		}
		break;

		case Change::Type::EXITED:
		{
			UnitToken::operator=(UnitToken());
			for (size_t i = 0; i < _figures.size(); i++)
			{
				if (i >= _figures.size() || !_figures[i])
				{
					LOGW << "nullptr detected in _figures";
					continue;
				}
				std::shared_ptr<Figure>& figure = _figures[i];
				constexpr float duration = 0.5f;
				/*
				// Disabled (#1583)
				figure->transition(group, TRANSITION_OBSCURED, 1, duration);
				*/
				figure->setVisible(group, false,
					(group ? group->delay : 0) + duration);
				figure->die();
			}
		}
		break;

		case Change::Type::DIED:
		{
			UnitToken old = (*this);
			UnitToken::operator=(UnitToken());
			for (size_t i = 0; i < _figures.size(); i++)
			{
				if (i >= _figures.size() || !_figures[i])
				{
					LOGW << "nullptr detected in _figures";
					continue;
				}
				std::shared_ptr<Figure>& figure = _figures[i];
				if (!figure->isDying())
				{
					figure->animate(change, group, _square);
					figure->die();

					if (group) group->delay += 0.100f;
				}
			}

			// A tile might become deoccupied as a result.
			if (_bible.unitCanOccupy(old.type)
				&& change.subject.type != Descriptor::Type::BYPASS
				&& _square->tile().owner != Player::NONE
				&& _square->tile().owner != old.owner)
			{
				float d = _square->tile().animateDeoccupy(change, group,
					group ? group->delay : 0);
				if (group && d > 0)
				{
					group->delay += d;
				}
			}
		}
		break;

		case Change::Type::SURVIVED:
		{
			precleanup();
		}
		break;

		case Change::Type::AIMS:
		{
			if (change.notice == Notice::OPPORTUNITYATTACK)
			{
				if (group) group->delay += 0.250f;
			}

			for (size_t i = 0; i < (size_t) std::max(0, (int) stacks); i++)
			{
				if (i >= _figures.size() || !_figures[i])
				{
					LOGW << "nullptr detected in _figures";
					continue;
				}
				std::shared_ptr<Figure>& figure = _figures[i];
				figure->animate(change, group, _square);
			}
			if (group) group->delay += 0.250f;
		}
		break;

		case Change::Type::ATTACKS:
		{
			if ((size_t) change.figure >= _figures.size() || !_figures[change.figure])
			{
				LOGW << "nullptr detected in _figures in unit enact attacks change";
				return;
			}
			std::shared_ptr<Figure>& figure = _figures[change.figure];
			figure->animate(change, group, _square);
		}
		break;

		case Change::Type::TRAMPLES:
		case Change::Type::SHELLS:
		case Change::Type::BOMBARDS:
		case Change::Type::BOMBS:
		{
			if ((size_t) change.figure >= _figures.size() || !_figures[change.figure])
			{
				LOGW << "nullptr detected in _figures in unit enact attacks change";
				return;
			}
			std::shared_ptr<Figure>& figure = _figures[change.figure];
			figure->animate(change, group, _square);
		}
		break;

		case Change::Type::ATTACKED:
		case Change::Type::TRAMPLED:
		case Change::Type::SHELLED:
		case Change::Type::BOMBARDED:
		case Change::Type::BOMBED:
		case Change::Type::FROSTBITTEN:
		case Change::Type::BURNED:
		case Change::Type::GASSED:
		case Change::Type::IRRADIATED:
		{
			if (change.killed)
			{
				stacks -= 1;
			}
			if ((size_t) change.figure >= _figures.size() || !_figures[change.figure])
			{
				LOGW << "nullptr detected in _figures in unit enact attacks change";
				return;
			}
			std::shared_ptr<Figure>& figure = _figures[change.figure];
			figure->animate(change, group, _square);
			if (change.killed)
			{
				figure->die();
			}
		}
		break;

		case Change::Type::REVEAL:
		case Change::Type::SNOW:
		case Change::Type::FROSTBITE:
		case Change::Type::FIRESTORM:
		case Change::Type::BONEDROUGHT:
		case Change::Type::DEATH:
		{
			for (size_t i = 0; i < (size_t) std::max(0, (int) stacks); i++)
			{
				if (i >= _figures.size() || !_figures[i])
				{
					LOGW << "nullptr detected in _figures";
					continue;
				}
				std::shared_ptr<Figure>& figure = _figures[i];
				figure->animate(change, group, _square);
			}
		}
		break;

		case Change::Type::GAS:
		case Change::Type::RADIATION:
		case Change::Type::TEMPERATURE:
		case Change::Type::HUMIDITY:
		case Change::Type::CHAOS:
		break;

		default:
		{
			LOGW << "Missing case \"" + std::string(Change::stringify(change.type))
				+ "\" in Unit::enact";
		}
		break;
	}
}

void Unit::light(float light, float duration, float delay)
{
	for (size_t i = 0; i < (size_t) std::max(0, (int) stacks); i++)
	{
		if (i >= _figures.size() || !_figures[i])
		{
			LOGW << "nullptr detected in _figures";
			continue;
		}
		std::shared_ptr<Figure>& figure = _figures[i];
		figure->transition(TRANSITION_LIGHT, light, duration, delay);
	}
}

void Unit::spotlight(float light, float duration, float delay)
{
	for (size_t i = 0; i < (size_t) std::max(0, (int) stacks); i++)
	{
		if (i >= _figures.size() || !_figures[i])
		{
			LOGW << "nullptr detected in _figures";
			continue;
		}
		std::shared_ptr<Figure>& figure = _figures[i];
		figure->transition(TRANSITION_SPOTLIGHT, light, duration, delay);
	}
}

void Unit::flashlight(float light, float duration, float delay)
{
	for (size_t i = 0; i < (size_t) std::max(0, (int) stacks); i++)
	{
		if (i >= _figures.size() || !_figures[i])
		{
			LOGW << "nullptr detected in _figures";
			continue;
		}
		std::shared_ptr<Figure>& figure = _figures[i];
		figure->transition(TRANSITION_FLASHLIGHT, light, duration, delay);
	}
}

void Unit::precleanup()
{
	// The precleanup moves all "bad" figures (null or dying) to the end of the vector.
	// Through this process, the figures in the range [start, figure) are bad.
	auto start = _figures.begin();
	for (auto figure = _figures.begin(); figure != _figures.end(); figure++)
	{
		if (!*figure)
		{
			LOGW << "nullptr detected in _figures in unit precleanup";
			// Implicitly append this to the range of bad figures.
		}
		else if ((*figure)->isDying())
		{
			// Implicitly append this to the range of bad figures.
		}
		else
		{
			// Rotate this element in front of the range of bad figures.
			if (start < figure) std::rotate(start, figure, figure + 1);
			// This element is not bad.
			start++;
		}
	}
}

void Unit::cleanup()
{
	_premoving = false;
	auto figure = _figures.begin();
	while (figure != _figures.end())
	{
		if (!*figure)
		{
			LOGW << "nullptr detected in _figures in unit cleanup";
			figure = _figures.erase(figure);
		}
		else if ((*figure)->isDying()) figure = _figures.erase(figure);
		else figure++;
	}
}

void Unit::idle()
{
	_idleTimer = 0.01f * (rand() % 200);
}

void Unit::unidle()
{
	_idleTimer = -1;
}
