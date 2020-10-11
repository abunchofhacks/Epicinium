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
#include "tile.hpp"
#include "source.hpp"

#include "square.hpp"
#include "animationgroup.hpp"
#include "camerafocus.hpp"
#include "change.hpp"
#include "aim.hpp"
#include "bible.hpp"
#include "skin.hpp"
#include "skinner.hpp"
#include "mixer.hpp"
#include "surface.hpp"
#include "figure.hpp"
#include "treetype.hpp"



Tile::Tile(Square* square, const Bible& bible, const Skinner& skinner) :
	_square(square),
	_bible(bible),
	_skinner(skinner),
	_treetype(TreeType::UNSET),
	_nfigures(0)
{}

Tile::Tile(Tile&&) = default;

Tile::~Tile() = default;

std::shared_ptr<Surface> Tile::newSurface()
{
	return std::make_shared<Surface>(_square,
		_skinner.surface(type, owner),
		owner);
}

PlacementBox Tile::newPlacement()
{
	PlacementBox box = _square->place(Descriptor::Type::TILE);
	if (getTextureType() == TextureType::MOUNTAIN)
	{
		int east  = joint(_square->east())  ? -10 : 30;
		int south = joint(_square->south()) ? -15 :  5;
		int west  = joint(_square->west())  ? -10 : 30;
		int north = joint(_square->north()) ?   0 : 20;
		box.centralize(east, south, west, north);
	}
	else
	{
		box.lower();
	}
	return box;
}

std::shared_ptr<Figure> Tile::newFigure()
{
	PlacementBox box = newPlacement();

	PowerType powertype;
	if (_bible.tileNeedsTime(type)) powertype = PowerType::LABOR;
	else if (_bible.tileNeedsLabor(type)) powertype = PowerType::LABOR;
	else if (_bible.tileNeedsEnergy(type)) powertype = PowerType::ENERGY;
	else powertype = PowerType::HEART;

	return std::make_shared<Figure>(std::move(box),
		_skinner.figure(type, _treetype, owner),
		owner, powertype);
}

std::shared_ptr<Figure> Tile::newFenceFigure()
{
	PlacementBox box = _square->place(Descriptor::Type::CELL);
	box.enfence();
	return std::make_shared<Figure>(std::move(box),
		_skinner.fence(type, owner),
		owner);
}

void Tile::pickTreeType()
{
	if (_skinner.treetypes.empty())
	{
		_treetype = TreeType::OAK;
		return;
	}

	size_t index = rand() % _skinner.treetypes.size();
	_treetype = _skinner.treetypes[index];
}

void Tile::fixTreeType()
{
	if (_square->humidity() >= 4)
	{
		_treetype = TreeType::SPRUCE;
	}
	else if (_square->humidity() <= 1)
	{
		_treetype = TreeType::PALM;
	}
	else if (_treetype == TreeType::UNSET)
	{
		pickTreeType();
	}
}

void Tile::setTreeType(TreeType treetype)
{
	_treetype = treetype;
}

void Tile::unsetTreeType()
{
	_treetype = TreeType::UNSET;
}

bool Tile::hasTreeType()
{
	return (_treetype != TreeType::UNSET);
}

TreeType Tile::getTreeType()
{
	return _treetype;
}

TextureType Tile::getTextureType()
{
	for (size_t i = 0; i == 0; i++)
	{
		if (_surfaces.empty() || !_surfaces[0])
		{
			if (type != TileType::NONE)
			{
				LOGW << "nullptr detected in _surfaces";
			}
			continue;
		}
		std::shared_ptr<Surface>& surface = _surfaces[0];
		return surface->texturetype();
	}
	return TextureType::NONE;
}

bool Tile::joint(Square* other)
{
	if (!other) return false;

	switch (getTextureType())
	{
		case TextureType::NONE:
		case TextureType::MOUNTAIN:
		{
			switch (other->tile().getTextureType())
			{
				case TextureType::NONE:
				case TextureType::MOUNTAIN:
				{
					return true;
				}
				break;

				default: return false;
			}
		}
		break;

		default: return false;
	}
}

void Tile::blendSurfaces(std::shared_ptr<Surface> newsurface,
	std::shared_ptr<Surface> oldsurface,
	std::shared_ptr<AnimationGroup> group)
{
	if (oldsurface && newsurface->inheritBlends(oldsurface))
	{
		// The newsurface has inherited the blends from the oldsurface.
		// We want to keep the blends of the right and bottom surfaces intact.
		return;
	}

	{
		std::shared_ptr<Surface> topsurface;
		if (_square->north() && !_square->north()->tile()._surfaces.empty())
		{
			topsurface = _square->north()->tile()._surfaces[0];
		}
		newsurface->blendTop(topsurface);
	}

	{
		std::shared_ptr<Surface> leftsurface;
		if (_square->west() && !_square->west()->tile()._surfaces.empty())
		{
			leftsurface = _square->west()->tile()._surfaces[0];
		}
		newsurface->blendLeft(leftsurface);
	}

	if (_square->south() && !_square->south()->tile()._surfaces.empty())
	{
		auto bottomsurface = _square->south()->tile()._surfaces[0];
		if (group)
		{
			std::weak_ptr<Surface> weaksurface = bottomsurface;
			std::weak_ptr<Surface> weaktop = newsurface;
			addAnimation(Animation(group, [weaksurface, weaktop](float) {

				auto surface = weaksurface.lock();
				if (!surface) return;

				surface->blendTop(weaktop.lock());
			}, 0, group->delay));
		}
		else bottomsurface->blendTop(newsurface);
	}

	if (_square->east() && !_square->east()->tile()._surfaces.empty())
	{
		auto rightsurface = _square->east()->tile()._surfaces[0];
		if (group)
		{
			std::weak_ptr<Surface> weaksurface = rightsurface;
			std::weak_ptr<Surface> weakleft = newsurface;
			addAnimation(Animation(group, [weaksurface, weakleft](float) {

				auto surface = weaksurface.lock();
				if (!surface) return;

				surface->blendLeft(weakleft.lock());
			}, 0, group->delay));
		}
		else rightsurface->blendLeft(newsurface);
	}
}

void Tile::populate(std::shared_ptr<AnimationGroup> group)
{
	{
		std::shared_ptr<Surface> surface(newSurface());
		{
			std::shared_ptr<Surface> oldsurface;
			if (!_surfaces.empty())
			{
				oldsurface = _surfaces[0];
			}
			blendSurfaces(surface, oldsurface, group);
		}
		surface->set(TRANSITION_LIGHT, _square->light());
		surface->set(TRANSITION_SPOTLIGHT, _square->spotlight());
		surface->set(TRANSITION_FLASHLIGHT, _square->flashlight());
		surface->set(TRANSITION_OBSCURED, !_square->current());
		surface->set(TRANSITION_SNOW, _square->snow());
		surface->set(TRANSITION_TEMPERATURE, (float) _square->temperature());
		surface->set(TRANSITION_HUMIDITY, (float) _square->humidity());
		surface->set(TRANSITION_CHAOS,
			(_square->chaos() == 0 && _bible.tileChaosProtection(type))
				? -1.0f : (float) _square->chaos());
		if (group)
		{
			surface->setVisible(false);
			std::weak_ptr<Surface> weaksurface = surface;
			addAnimation(Animation(group, [weaksurface](float) {

				auto s = weaksurface.lock();
				if (!s) return;

				s->setVisible(true);
			}, 0, group->delay));
		}
		_surfaces.emplace(_surfaces.begin(), surface);
	}

	// Instead of removing the old surface, it was pushed back to [1] and we hide it.
	for (size_t i = 1; i == 1 && i < _surfaces.size(); i++)
	{
		if (!_surfaces[1])
		{
			LOGW << "nullptr detected in _surfaces";
			continue;
		}
		std::shared_ptr<Surface>& surface = _surfaces[1];
		if (group)
		{
			std::weak_ptr<Surface> weaksurface = surface;
			addAnimation(Animation(group, [weaksurface](float) {

				auto s = weaksurface.lock();
				if (!s) return;

				s->setVisible(false);
			}, 0, group->delay));
		}
		else
		{
			surface->setVisible(false);
		}
	}

	// There might still be some figures that have animations playing.
	// Instead of removing them, we will rotate them out of range.
	size_t leftovers = _figures.size();
	if (leftovers)
	{
		for (size_t i = 0; i < leftovers; i++)
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

	_nfigures = (size_t) std::max(0, (int) stacks);
	if (_nfigures == 0 && owner != Player::NONE) _nfigures = 1;

	for (size_t i = 0; i < _nfigures; i++)
	{
		std::shared_ptr<Figure> figure;
		if (i < (size_t) std::max(0, (int) stacks))
		{
			figure = newFigure();
		}
		else
		{
			figure = newFenceFigure();
		}
		if (_bible.powerAbolished() || i < (size_t) std::max(0, (int) power))
		{
			figure->transition(group, TRANSITION_POWER, 1, 0.500f);
		}
		figure->set(TRANSITION_LIGHT, _square->light());
		figure->set(TRANSITION_SPOTLIGHT, _square->spotlight());
		figure->set(TRANSITION_FLASHLIGHT, _square->flashlight());
		figure->set(TRANSITION_OBSCURED, !_square->current());
		figure->set(TRANSITION_HUMIDITY, (float) _square->humidity());
		figure->set(TRANSITION_SNOW, _square->snow());
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

void Tile::update()
{
	Animator::update();

	// Only a single surface should generate new weather particles.
	// This needs to happen before Surface::update() is called.
	for (int i = 0; i == 0; i++)
	{
		if (_surfaces.empty() || !_surfaces[0])
		{
			continue;
		}
		std::shared_ptr<Surface>& surface = _surfaces[0];

		surface->updateWeather(_square);
	}

	// All surfaces need to be updated, not just 0.
	for (auto& surface : _surfaces)
	{
		if (!surface)
		{
			LOGW << "nullptr detected in _surfaces in tile update";
			continue;
		}

		surface->update();
	}

	// All figures need to be updated, not just those in [0, _nfigures).
	for (auto& figure : _figures)
	{
		if (!figure)
		{
			LOGW << "nullptr detected in _figures in tile update";
			continue;
		}

		figure->update();
	}
}

bool Tile::hovered() const
{
	// Because some items might occlude others, we must know which layer the mouse operates in.
	// We therefore need to check all items to see if they are hovered.
	bool result = false;

	for (size_t i = 0; i < _nfigures; i++)
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

bool Tile::selected() const
{
	return _selected;
}

void Tile::select()
{
	_selected = true;
	for (size_t i = 0; i < _nfigures; i++)
	{
		if (i >= _figures.size() || !_figures[i])
		{
			LOGW << "nullptr detected in _figures";
			continue;
		}
		std::shared_ptr<Figure>& figure = _figures[i];
		figure->select();
	}
	Mixer::get()->queue(Clip::Type::UI_HOVERBUILDING, 0.0f);
}

void Tile::deselect()
{
	_selected = false;
	for (size_t i = 0; i < _nfigures; i++)
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

void Tile::changeStacksPower(int8_t stackdiff, int8_t powerdiff, const Player& newowner,
	std::shared_ptr<AnimationGroup> group)
{
	// Kill fence figure by killing all figures in interval [stacks, _nfigures).
	for (size_t i = stacks; i < _nfigures; i++)
	{
		if (!_figures[i])
		{
			LOGW << "nullptr detected in _figures in tile enact relative";
			continue;
		}
		std::shared_ptr<Figure>& figure = _figures[i];
		if (group)
		{
			figure->setVisible(group, false);
		}
		else figure->setVisible(false);
		figure->die();
	}

	_nfigures = (size_t) std::max(0, (int) stacks);

	while (stackdiff > 0 && powerdiff > 0)
	{
		// Make new empowered figure.
		std::shared_ptr<Figure> newfigure(newFigure());
		newfigure->set(TRANSITION_OBSCURED, !_square->current());
		newfigure->set(TRANSITION_LIGHT, _square->light());
		newfigure->set(TRANSITION_SPOTLIGHT, _square->spotlight());
		newfigure->set(TRANSITION_FLASHLIGHT, _square->flashlight());
		newfigure->set(TRANSITION_HUMIDITY, _square->humidity());
		newfigure->set(TRANSITION_SNOW, _square->snow());
		newfigure->transition(group, TRANSITION_POWER, 1, 0.500f);
		if (group)
		{
			newfigure->setVisible(false);
			std::weak_ptr<Figure> weakfigure = newfigure;
			addAnimation(Animation(group, [weakfigure](float) {

				auto figure = weakfigure.lock();
				if (!figure) return;

				figure->setVisible(true);
			}, 0, group->delay));
		}
		_figures.emplace(_figures.begin() + power, newfigure);
		power++;
		powerdiff--;
		stacks++;
		stackdiff--;
	}

	while (powerdiff > 0)
	{
		if (!_figures[power])
		{
			LOGW << "nullptr detected in _figures in tile enact relative";
			continue;
		}
		// Power up existing figure.
		_figures[power]->transition(group, TRANSITION_POWER, 1, 0.500f);
		power++;
		powerdiff--;
	}

	while (stackdiff > 0)
	{
		// Make new unpowered figure.
		std::shared_ptr<Figure> newfigure(newFigure());
		newfigure->set(TRANSITION_OBSCURED, !_square->current());
		newfigure->set(TRANSITION_LIGHT, _square->light());
		newfigure->set(TRANSITION_SPOTLIGHT, _square->spotlight());
		newfigure->set(TRANSITION_FLASHLIGHT, _square->flashlight());
		newfigure->set(TRANSITION_HUMIDITY, _square->humidity());
		newfigure->set(TRANSITION_SNOW, _square->snow());
		if (_bible.powerAbolished())
		{
			newfigure->transition(group, TRANSITION_POWER, 1, 0.500f);
		}
		if (group)
		{
			newfigure->setVisible(false);
			std::weak_ptr<Figure> weakfigure = newfigure;
			addAnimation(Animation(group, [weakfigure](float) {

				auto figure = weakfigure.lock();
				if (!figure) return;

				figure->setVisible(true);
			}, 0, group->delay));
		}
		_figures.emplace_back(newfigure);
		stacks++;
		stackdiff--;
	}

	while (stackdiff < 0 && stackdiff < powerdiff)
	{
		if (!_figures[stacks - 1])
		{
			LOGW << "nullptr detected in _figures in tile enact relative";
			continue;
		}
		// Kill unpowered figure.
		std::shared_ptr<Figure>& figure = _figures[stacks - 1];
		if (group)
		{
			figure->setVisible(group, false);
		}
		else figure->setVisible(false);
		figure->die();
		stacks--;
		stackdiff++;
	}

	while (stackdiff < 0 && powerdiff < 0)
	{
		if (!_figures[power - 1])
		{
			LOGW << "nullptr detected in _figures in tile enact relative";
			continue;
		}
		// Kill empowered figure.
		std::shared_ptr<Figure>& figure = _figures[power - 1];
		if (group)
		{
			figure->setVisible(group, false);
		}
		else figure->setVisible(false);
		figure->die();
		power--;
		powerdiff++;
		stacks--;
		stackdiff++;
		// Note that this is kind of weird because now _figures[stacks]
		// has not called die() and _figures[power] has called die() instead.
		// This will be resolved by a precleanup later.
	}

	while (powerdiff < 0)
	{
		if (!_figures[power - 1])
		{
			LOGW << "nullptr detected in _figures in tile enact relative";
			continue;
		}
		// Power down existing figure.
		_figures[power - 1]->transition(group, TRANSITION_POWER, 0, 0.500f);
		power--;
		powerdiff++;
	}

	_nfigures = (size_t) std::max(0, (int) stacks);

	if (_nfigures == 0 && newowner != Player::NONE)
	{
		// Make new fence figure.
		std::shared_ptr<Figure> newfigure(newFenceFigure());
		newfigure->set(TRANSITION_OBSCURED, !_square->current());
		newfigure->set(TRANSITION_LIGHT, _square->light());
		newfigure->set(TRANSITION_SPOTLIGHT, _square->spotlight());
		newfigure->set(TRANSITION_FLASHLIGHT, _square->flashlight());
		newfigure->set(TRANSITION_HUMIDITY, _square->humidity());
		newfigure->set(TRANSITION_SNOW, _square->snow());
		if (group)
		{
			newfigure->setVisible(false);
			std::weak_ptr<Figure> weakfigure = newfigure;
			addAnimation(Animation(group, [weakfigure](float) {

				auto figure = weakfigure.lock();
				if (!figure) return;

				figure->setVisible(true);
			}, 0, group->delay));
		}
		_figures.emplace(_figures.begin(), newfigure);

		_nfigures = 1;
	}

	// Do a precleanup so the figures that have called die() are exactly
	// _figures[stacks], _figures[stacks + 1], ...
	precleanup();
}

void Tile::enact(const Change& change, std::shared_ptr<AnimationGroup> group)
{
	switch (change.type)
	{
		case Change::Type::NONE:
		{
			if (change.target.type != Descriptor::Type::NONE)
			{
				Aim aim(_square->position(), change.target.position);
				Square* targetsquare = _square->eswn(aim.direction());
				targetsquare->tile().animateBlock(change, group, group ? group->delay : 0);
			}
			else
			{
				animateBlock(change, group, group ? group->delay : 0);
			}

			if (group)
			{
				group->delay += 0.5f;
				pause(group, group->delay);
			}
		}
		break;

		case Change::Type::REVEAL:
		{
			if (type != change.tile.type)
			{
				size_t leftovers = _nfigures;
				for (size_t i = 0; i < leftovers; i++)
				{
					if (i >= _figures.size() || !_figures[i])
					{
						LOGW << "nullptr detected in _figures";
						continue;
					}
					std::shared_ptr<Figure>& figure = _figures[i];
					if (group) figure->setVisible(group, false, group->delay);
					else figure->setVisible(false);
					figure->die();
				}

				TileToken::operator=(change.tile);

				if (_surfaces.empty() || !group || group->delay == 0)
				{
					populate(group);
					for (size_t i = 0; i == 0; i++)
					{
						if (_surfaces.empty() || !_surfaces[0])
						{
							LOGW << "nullptr detected in _surfaces";
							continue;
						}
						std::shared_ptr<Surface>& surface = _surfaces[0];
						surface->animate(change, group, _square);
						surface->set(TRANSITION_OBSCURED, 1);
						surface->transition(group, TRANSITION_OBSCURED, 0, 0.5f);
					}
					for (size_t i = 0; i < _nfigures; i++)
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
				}
				else
				{
					populate(group);
					for (size_t i = 0; i == 0; i++)
					{
						if (_surfaces.empty() || !_surfaces[0])
						{
							LOGW << "nullptr detected in _surfaces";
							continue;
						}
						std::shared_ptr<Surface>& surface = _surfaces[0];
						surface->animate(change, group, _square);
						std::weak_ptr<Surface> weaksurface = surface;
						addAnimation(Animation(group,
								[group, weaksurface](float) {

							auto s = weaksurface.lock();
							if (!s) return;

							s->set(TRANSITION_OBSCURED, 1);
							s->transition(group, TRANSITION_OBSCURED, 0,
								0.5f, /*delay=*/0);
						}, 0, group->delay));
					}
					for (size_t i = 0; i < _nfigures; i++)
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
							f->transition(group, TRANSITION_OBSCURED, 0,
								0.5f, /*delay=*/0);
						}, 0, group->delay));
					}
				}

				break;
			}

			changeStacksPower(change.tile.stacks - stacks, change.tile.power - power,
				change.tile.owner, group);

			if (owner != change.tile.owner)
			{
				for (size_t i = 0; i < _nfigures; i++)
				{
					if (i >= _figures.size() || !_figures[i])
					{
						LOGW << "nullptr detected in _figures";
						continue;
					}
					std::shared_ptr<Figure>& figure = _figures[i];
					figure->setPlayer(change.tile.owner);
				}

				for (size_t i = 0; i == 0; i++)
				{
					if (_surfaces.empty() || !_surfaces[0])
					{
						LOGW << "nullptr detected in _surfaces";
						continue;
					}
					std::shared_ptr<Surface>& surface = _surfaces[0];
					surface->setPlayer(change.tile.owner);
				}

				owner = change.tile.owner;
			}

			for (size_t i = 0; i == 0; i++)
			{
				if (_surfaces.empty() || !_surfaces[0])
				{
					LOGW << "nullptr detected in _surfaces";
					continue;
				}
				std::shared_ptr<Surface>& surface = _surfaces[0];
				surface->animate(change, group, _square);
				surface->set(TRANSITION_OBSCURED, 1);
				surface->transition(group, TRANSITION_OBSCURED, 0, 0.5f);
				surface->transition(group, TRANSITION_SNOW,
					_square->snow(), 0.02f);
				surface->transition(group, TRANSITION_TEMPERATURE,
					(float) _square->temperature(), 0.02f);
				surface->transition(group, TRANSITION_HUMIDITY,
					(float) _square->humidity(), 0.02f);
				surface->transition(group, TRANSITION_CHAOS,
					(!_square->chaos() && _bible.tileChaosProtection(type))
						? -1.0f : (float) _square->chaos(),
					0.02f);
			}
			for (size_t i = 0; i < _nfigures; i++)
			{
				if (i >= _figures.size() || !_figures[i])
				{
					LOGW << "nullptr detected in _figures";
					continue;
				}
				std::shared_ptr<Figure>& figure = _figures[i];
				figure->set(TRANSITION_OBSCURED, 1);
				figure->transition(group, TRANSITION_OBSCURED, 0, 0.5f);
				figure->transition(group, TRANSITION_HUMIDITY,
					(float) _square->humidity(), 0.02f);
				figure->transition(group, TRANSITION_SNOW, _square->snow(), 0.02f);
			}
		}
		break;

		case Change::Type::OBSCURE:
		{
			for (size_t i = 0; i == 0; i++)
			{
				if (_surfaces.empty() || !_surfaces[0])
				{
					LOGW << "nullptr detected in _surfaces";
					continue;
				}
				std::shared_ptr<Surface>& surface = _surfaces[0];
				surface->animate(change, group, _square);
				surface->transition(group, TRANSITION_OBSCURED, 1, 0.5f);
			}
			for (size_t i = 0; i < _nfigures; i++)
			{
				if (i >= _figures.size() || !_figures[i])
				{
					LOGW << "nullptr detected in _figures";
					continue;
				}
				std::shared_ptr<Figure>& figure = _figures[i];
				figure->transition(group, TRANSITION_OBSCURED, 1, 0.5f);
			}
		}
		break;

		case Change::Type::TRANSFORMED:
		{

			if (type != change.tile.type)
			{
				size_t leftovers = _nfigures;
				for (size_t i = 0; i < leftovers; i++)
				{
					// Don't destroy the fence if the owner stays the same.
					// TODO It is a bit ugly to have this specific check here.
					if (i >= (size_t) std::max(0, (int) stacks)
						&& change.tile.stacks == 0
						&& change.tile.owner == owner)
					{
						continue;
					}

					if (i >= _figures.size() || !_figures[i])
					{
						LOGW << "nullptr detected in _figures";
						continue;
					}
					std::shared_ptr<Figure>& figure = _figures[i];
					figure->animate(change, group, _square);
					figure->die();
				}

				for (size_t i = 0; i == 0; i++)
				{
					if (_surfaces.empty() || !_surfaces[0])
					{
						LOGW << "nullptr detected in _surfaces";
						continue;
					}
					std::shared_ptr<Surface>& surface = _surfaces[0];
					surface->animate(change, group, _square);
					// TODO this code was moved from Surface::animate() because
					// it does not have access to _bible or type. (#1191)
					if (_bible.tileGrassy(type)
						&& !_bible.tileGrassy(change.tile.type))
					{
						surface->mowGrass(group);
					}
					else if (_bible.tileDesert(change.tile.type))
					{
						surface->generateStatik(group);
					}
				}

				TileToken::operator=(change.tile);
				populate(group);

				if (group) group->delay += 0.100f;

				break;
			}

			changeStacksPower(change.tile.stacks - stacks, change.tile.power - power,
				change.tile.owner, group);

			if (owner != change.tile.owner)
			{
				for (size_t i = 0; i < _nfigures; i++)
				{
					if (i >= _figures.size() || !_figures[i])
					{
						LOGW << "nullptr detected in _figures";
						continue;
					}
					std::shared_ptr<Figure>& figure = _figures[i];
					figure->setPlayer(change.tile.owner);
				}

				for (size_t i = 0; i == 0; i++)
				{
					if (_surfaces.empty() || !_surfaces[0])
					{
						LOGW << "nullptr detected in _surfaces";
						continue;
					}
					std::shared_ptr<Surface>& surface = _surfaces[0];
					surface->setPlayer(change.tile.owner);
				}

				owner = change.tile.owner;
			}

			if (group) group->delay += 0.1;
		}
		break;

		case Change::Type::CONSUMED:
		{
			if (type != change.tile.type)
			{
				size_t leftovers = _nfigures;
				for (size_t i = 0; i < leftovers; i++)
				{
					// Don't destroy the fence if the owner stays the same.
					// TODO It is a bit ugly to have this specific check here.
					if (i >= (size_t) std::max(0, (int) stacks)
						&& change.tile.stacks == 0
						&& change.tile.owner == owner)
					{
						continue;
					}

					if (i >= _figures.size() || !_figures[i])
					{
						LOGW << "nullptr detected in _figures";
						continue;
					}
					std::shared_ptr<Figure>& figure = _figures[i];
					figure->animate(change, group, _square);
					figure->die();
				}

				TileToken::operator=(change.tile);
				populate(group);

				break;
			}

			changeStacksPower(change.tile.stacks - stacks, change.tile.power - power,
				change.tile.owner, group);

			if (owner != change.tile.owner)
			{
				for (size_t i = 0; i < _nfigures; i++)
				{
					if (i >= _figures.size() || !_figures[i])
					{
						LOGW << "nullptr detected in _figures";
						continue;
					}
					std::shared_ptr<Figure>& figure = _figures[i];
					figure->setPlayer(change.tile.owner);
				}

				for (size_t i = 0; i == 0; i++)
				{
					if (_surfaces.empty() || !_surfaces[0])
					{
						LOGW << "nullptr detected in _surfaces";
						continue;
					}
					std::shared_ptr<Surface>& surface = _surfaces[0];
					surface->setPlayer(change.tile.owner);
				}

				owner = change.tile.owner;
			}
		}
		break;

		case Change::Type::CAPTURED:
		{
			if (owner != change.player)
			{
				for (size_t i = 0; i < _nfigures; i++)
				{
					if (i >= _figures.size() || !_figures[i])
					{
						LOGW << "nullptr detected in _figures";
						continue;
					}
					Player newowner = change.player;
					if (group)
					{
						std::weak_ptr<Figure> weakfigure = _figures[i];
						addAnimation(Animation(group,
								[weakfigure, newowner](float) {

							auto figure = weakfigure.lock();
							if (!figure) return;

							figure->setPlayer(newowner);

						}, 0, group->delay));
					}
					else _figures[i]->setPlayer(newowner);

					if (_bible.powerAbolished()
						|| i < (size_t) std::max(0, (int) power))
					{
						std::shared_ptr<Figure> figure = _figures[i];
						figure->transition(group, TRANSITION_POWER, 1, 0.300f);
					}
				}

				for (size_t i = 0; i == 0; i++)
				{
					if (_surfaces.empty() || !_surfaces[0])
					{
						LOGW << "nullptr detected in _surfaces";
						continue;
					}
					Player newowner = change.player;
					if (group)
					{
						std::weak_ptr<Surface> weaksurface = _surfaces[0];
						addAnimation(Animation(group,
								[weaksurface, newowner](float) {

							auto surface = weaksurface.lock();
							if (!surface) return;

							surface->setPlayer(newowner);

						}, 0, group->delay));
					}
					else _surfaces[0]->setPlayer(newowner);
				}

				owner = change.player;

				if (_nfigures == 0)
				{
					// Make new fence figure.
					std::shared_ptr<Figure> newfigure(newFenceFigure());
					newfigure->set(TRANSITION_OBSCURED, !_square->current());
					newfigure->set(TRANSITION_LIGHT, _square->light());
					newfigure->set(TRANSITION_SPOTLIGHT, _square->spotlight());
					newfigure->set(TRANSITION_FLASHLIGHT, _square->flashlight());
					if (group)
					{
						newfigure->setVisible(false);
						std::weak_ptr<Figure> weakfigure = newfigure;
						addAnimation(Animation(group, [weakfigure](float) {

							auto figure = weakfigure.lock();
							if (!figure) return;

							figure->setVisible(true);
						}, 0, group->delay));
					}
					_figures.emplace(_figures.begin(), newfigure);

					_nfigures = 1;
				}
			}

			if (group)
			{
				group->delay += 0.600f;
				pause(group, group->delay);
			}
		}
		break;

		case Change::Type::TRAMPLED:
		{
			if (change.subject.type == Descriptor::Type::CELL)
			{
				if (_surfaces.empty() || !_surfaces[0])
				{
					LOGW << "nullptr detected in _surfaces";
					break;
				}
				std::shared_ptr<Surface>& surface = _surfaces[0];
				surface->animate(change, group, _square);
				break;
			}

			int x = change.figure;
			if (change.killed)
			{
				stacks--;
				if (_nfigures > 0) _nfigures--;
			}
			if (change.depowered)
			{
				power--;

				// Tiny hack that used to be in the Damage class; we can only
				// depower the figure in the middle, because we do not remember
				// anything that let's us renormalize during precleanup.
				x = power;
			}

			if (x >= 0)
			{
				size_t i = x;
				if (i >= _figures.size() || !_figures[i])
				{
					LOGW << "nullptr detected in _figures";
					break;
				}
				std::shared_ptr<Figure>& figure = _figures[i];

				figure->animate(change, group, _square);

				if (change.killed)
				{
					figure->die();
				}
				else if (change.depowered)
				{
					figure->transition(group, TRANSITION_POWER, 0, 0.500f);
					if (group) group->delay += 0.500f;
				}
			}
			else
			{
				if (_surfaces.empty() || !_surfaces[0])
				{
					LOGW << "nullptr detected in _surfaces";
					break;
				}
				std::shared_ptr<Surface>& surface = _surfaces[0];

				if (group)
				{
					group->placements.emplace_back(_square->place(
						Descriptor::Type::GROUND).random());

					Mixer::get()->queue(Clip::Type::TRAMPLE, group->delay,
					Surface::convertOrigin(_square->position()));
				}
				surface->animate(change, group, _square);
			}
		}
		break;

		case Change::Type::BURNED:
		{
						if (change.subject.type == Descriptor::Type::CELL)
			{
				if (_surfaces.empty() || !_surfaces[0])
				{
					LOGW << "nullptr detected in _surfaces";
					break;
				}
				std::shared_ptr<Surface>& surface = _surfaces[0];
				surface->animate(change, group, _square);
				break;
			}

			int x = change.figure;
			if (change.killed)
			{
				stacks--;
				if (_nfigures > 0) _nfigures--;
			}
			if (change.depowered)
			{
				power--;

				// Tiny hack that used to be in the Damage class; we can only
				// depower the figure in the middle, because we do not remember
				// anything that let's us renormalize during precleanup.
				x = power;
			}

			if (x >= 0)
			{
				size_t i = x;
				if (i >= _figures.size() || !_figures[i])
				{
					LOGW << "nullptr detected in _figures";
					break;
				}
				std::shared_ptr<Figure>& figure = _figures[i];

				figure->animate(change, group, _square);

				if (change.killed)
				{
					figure->die();
				}
				else if (change.depowered)
				{
					figure->transition(group, TRANSITION_POWER, 0, 0.500f);
					if (group) group->delay += 0.500f;
				}
			}
			else
			{
				if (_surfaces.empty() || !_surfaces[0])
				{
					LOGW << "nullptr detected in _surfaces";
					break;
				}
				std::shared_ptr<Surface>& surface = _surfaces[0];

				if (group)
				{
					group->placements.emplace_back(_square->place(
						Descriptor::Type::GROUND).random());

					Mixer::get()->queue(Clip::Type::CROPSBURN, group->delay,
					Surface::convertOrigin(_square->position()));
				}
				surface->animate(change, group, _square);
			}
		}
		break;
		case Change::Type::ATTACKED:
		case Change::Type::SHELLED:
		case Change::Type::BOMBARDED:
		case Change::Type::BOMBED:
		case Change::Type::FROSTBITTEN:
		case Change::Type::GASSED:
		case Change::Type::IRRADIATED:
		{
			if (change.subject.type == Descriptor::Type::CELL)
			{
				if (_surfaces.empty() || !_surfaces[0])
				{
					LOGW << "nullptr detected in _surfaces";
					break;
				}
				std::shared_ptr<Surface>& surface = _surfaces[0];
				surface->animate(change, group, _square);
				break;
			}

			int x = change.figure;
			if (change.killed)
			{
				stacks--;
				if (_nfigures > 0) _nfigures--;
			}
			if (change.depowered)
			{
				power--;

				// Tiny hack that used to be in the Damage class; we can only
				// depower the figure in the middle, because we do not remember
				// anything that let's us renormalize during precleanup.
				x = power;
			}

			if (x >= 0)
			{
				size_t i = x;
				if (i >= _figures.size() || !_figures[i])
				{
					LOGW << "nullptr detected in _figures";
					break;
				}
				std::shared_ptr<Figure>& figure = _figures[i];

				figure->animate(change, group, _square);

				if (change.killed)
				{
					figure->die();
				}
				else if (change.depowered)
				{
					figure->transition(group, TRANSITION_POWER, 0, 0.500f);
					if (group) group->delay += 0.500f;
				}
			}
			else
			{
				if (_surfaces.empty() || !_surfaces[0])
				{
					LOGW << "nullptr detected in _surfaces";
					break;
				}
				std::shared_ptr<Surface>& surface = _surfaces[0];

				if (group)
				{
					group->placements.emplace_back(_square->place(
						Descriptor::Type::GROUND).random());
				}
				surface->animate(change, group, _square);
			}
		}
		break;

		case Change::Type::SHAPED:
		{
			size_t leftovers = _nfigures;
			for (size_t i = 0; i < leftovers; i++)
			{
				if (i >= _figures.size() || !_figures[i])
				{
					LOGW << "nullptr detected in _figures";
					continue;
				}
				std::shared_ptr<Figure>& figure = _figures[i];
				figure->animate(change, group, _square);
				figure->die();

				if (group) group->delay += 0.100f;
			}

			TileToken::operator=(change.tile);
			populate(group);

			for (size_t i = 0; i == 0; i++)
			{
				if (_surfaces.empty() || !_surfaces[0])
				{
					LOGW << "nullptr detected in _surfaces";
					continue;
				}
				std::shared_ptr<Surface>& surface = _surfaces[0];
				surface->animate(change, group, _square);
			}

			if (group)
			{
				if (auto camerafocus = group->camerafocus.lock())
				{
					camerafocus->shake(group, 1, 0.020f, 0.130f, group->delay);
				}

				// queue the audio
				Mixer::get()->queue(Clip::Type::TREEFELL, group->delay,
					Surface::convertOrigin(_square->position()));

				group->delay += 0.5;
			}
		}
		break;

		case Change::Type::SETTLED:
		{
			size_t leftovers = _nfigures;
			for (size_t i = 0; i < leftovers; i++)
			{
				if (i >= _figures.size() || !_figures[i])
				{
					LOGW << "nullptr detected in _figures";
					continue;
				}
				std::shared_ptr<Figure>& figure = _figures[i];
				figure->animate(change, group, _square);
				figure->die();

				if (group) group->delay += 0.100f;
			}

			TileToken::operator=(change.tile);
			populate(group);

			for (size_t i = 0; i == 0; i++)
			{
				if (_surfaces.empty() || !_surfaces[0])
				{
					LOGW << "nullptr detected in _surfaces";
					continue;
				}
				std::shared_ptr<Surface>& surface = _surfaces[0];
				surface->animate(change, group, _square);
			}

			if (group)
			{
				if (auto camerafocus = group->camerafocus.lock())
				{
					camerafocus->shake(group, 3, 0.020f, 0.130f, group->delay);
				}

				// queue the audio
				Mixer::get()->queue(Clip::Type::PLACEMENT, group->delay,
					Surface::convertOrigin(_square->position()));

				group->delay += 0.5;
			}
		}
		break;

		case Change::Type::EXPANDS:
		{
			changeStacksPower(0, change.power, owner, group);

			if (group) group->delay += 0.5;

			Mixer::get()->queue(Clip::Type::PLACE, 0.0f);
		}
		break;

		case Change::Type::EXPANDED:
		{
			size_t leftovers = _nfigures;
			for (size_t i = 0; i < leftovers; i++)
			{
				if (i >= _figures.size() || !_figures[i])
				{
					LOGW << "nullptr detected in _figures";
					continue;
				}
				std::shared_ptr<Figure>& figure = _figures[i];
				figure->animate(change, group, _square);
				figure->die();

				if (group) group->delay += 0.100f;
			}

			TileToken::operator=(change.tile);
			populate(group);

			for (size_t i = 0; i == 0; i++)
			{
				if (_surfaces.empty() || !_surfaces[0])
				{
					LOGW << "nullptr detected in _surfaces";
					continue;
				}
				std::shared_ptr<Surface>& surface = _surfaces[0];
				surface->animate(change, group, _square);
			}

			if (group)
			{
				if (auto camerafocus = group->camerafocus.lock())
				{
					camerafocus->shake(group, 3, 0.020f, 0.130f, group->delay);
				}

				// queue the audio
				Mixer::get()->queue(Clip::Type::PLACEMENT, group->delay,
					Surface::convertOrigin(_square->position()));

				group->delay += 0.5;
			}
		}
		break;

		case Change::Type::UPGRADES:
		{
			changeStacksPower(0, change.power, owner, group);

			if (group) group->delay += 0.5;
		}
		break;

		case Change::Type::UPGRADED:
		{
			if (change.tile.type == type)
			{
				changeStacksPower(change.tile.stacks - stacks,
					change.tile.power - power,
					change.tile.owner, group);

				for (size_t i = (size_t) (stacks - 1); i < _nfigures; i++)
				{
					if (i >= _figures.size() || !_figures[i])
					{
						LOGW << "nullptr detected in _figures";
						continue;
					}
					std::shared_ptr<Figure>& figure = _figures[i];
					figure->spawn(change, group, group ? group->delay : 0);
				}

				if (group)
				{
					if (auto camerafocus = group->camerafocus.lock())
					{
						camerafocus->shake(group, 3, 0.020f, 0.130f, group->delay);
					}

					// queue the audio
					Mixer::get()->queue(Clip::Type::PLACEMENT, group->delay,
						Surface::convertOrigin(_square->position()));

					group->delay += 0.5;
				}

				break;
			}

			size_t leftovers = _nfigures;
			for (size_t i = 0; i < leftovers; i++)
			{
				if (i >= _figures.size() || !_figures[i])
				{
					LOGW << "nullptr detected in _figures";
					continue;
				}
				std::shared_ptr<Figure>& figure = _figures[i];
				figure->animate(change, group, _square);
				figure->die();

				if (group) group->delay += 0.100f;
			}

			TileToken::operator=(change.tile);
			populate(group);

			if (group)
			{
				if (auto camerafocus = group->camerafocus.lock())
				{
					camerafocus->shake(group, 3, 0.020f, 0.130f, group->delay);
				}

				// queue the audio
				Mixer::get()->queue(Clip::Type::PLACEMENT, group->delay,
					Surface::convertOrigin(_square->position()));

				group->delay += 0.5;
			}
		}
		break;

		case Change::Type::CULTIVATES:
		{
			changeStacksPower(0, change.power, owner, group);

			if (group && group->multiculti == false)
			{
				group->delay += 0.500f;
				group->multiculti = true;
			}
		}
		break;

		case Change::Type::CULTIVATED:
		{
			size_t leftovers = _nfigures;
			for (size_t i = 0; i < leftovers; i++)
			{
				if (i >= _figures.size() || !_figures[i])
				{
					LOGW << "nullptr detected in _figures";
					continue;
				}
				std::shared_ptr<Figure>& figure = _figures[i];
				figure->animate(change, group, _square);
				figure->die();

				if (group) group->delay += 0.100f;
			}

			TileToken::operator=(change.tile);
			populate(group);

			for (size_t i = 0; i == 0; i++)
			{
				if (_surfaces.empty() || !_surfaces[0])
				{
					LOGW << "nullptr detected in _surfaces";
					continue;
				}
				std::shared_ptr<Surface>& surface = _surfaces[0];
				surface->animate(change, group, _square);
			}

			if (group)
			{
				if (auto camerafocus = group->camerafocus.lock())
				{
					camerafocus->shake(group, 3, 0.020f, 0.130f, group->delay);
				}

				// queue the audio
				Mixer::get()->queue(Clip::Type::PLACEMENT, group->delay,
					Surface::convertOrigin(_square->position()));

				group->delay += 0.500f;
			}
		}
		break;

		case Change::Type::PRODUCES:
		{
			changeStacksPower(0, change.power, owner, group);

			if (group)
			{
				size_t from = 0;
				size_t to = stacks;
				// We spent -change.power (i.e. "gained" change.power) power,
				// and the figures in the range [current power, old power)
				// are the figures where the units should spawn.
				if (!_bible.powerAbolished())
				{
					from = (size_t) std::max(0, (int) power);
					to = (size_t) std::max(0, power - change.power);
				}
				for (size_t i = from; i < to; i++)
				{
					if (i >= _figures.size() || !_figures[i])
					{
						LOGW << "nullptr detected in _figures";
						continue;
					}
					std::shared_ptr<Figure>& figure = _figures[i];
					// Remember the spawn point and the "box index" of this
					// figure, as the Unit will need it when enacting PRODUCED.
					group->placements.emplace_back(figure->getSpawnPoint());
					group->indices.emplace_back(figure->box().index());
				}
				// Fisher-Yates shuffle the placements and indices together.
				if (_bible.powerAbolished())
				{
					size_t len = (to - from);
					for (size_t i = len - 1; i > 0; i--)
					{
						size_t j = rand() % (i + 1);
						if (i != j)
						{
							std::swap(
								*(group->placements.end() - len + i),
								*(group->placements.end() - len + j));
							std::swap(
								*(group->indices.end() - len + i),
								*(group->indices.end() - len + j));
						}
					}
				}
			}

			if (group) group->delay += 0.5;
		}
		break;

		case Change::Type::DESTROYED:
		{
			size_t leftovers = _nfigures;
			for (size_t i = 0; i < leftovers; i++)
			{
				if (i >= _figures.size() || !_figures[i])
				{
					LOGW << "nullptr detected in _figures";
					continue;
				}
				std::shared_ptr<Figure>& figure = _figures[i];
				figure->animate(change, group, _square);
				figure->die();

				if (group) group->delay += 0.100f;
			}

			for (size_t i = 0; i == 0; i++)
			{
				if (_surfaces.empty() || !_surfaces[0])
				{
					LOGW << "nullptr detected in _surfaces";
					continue;
				}
				std::shared_ptr<Surface>& surface = _surfaces[0];
				surface->animate(change, group, _square);
			}

			TileToken::operator=(change.tile);
			populate(group);

			if (group)
			{
				if (auto camerafocus = group->camerafocus.lock())
				{
					camerafocus->shake(group, 1, 0.020f, 0.030f, group->delay);
				}
			}

			if (group) group->delay += 0.500f;
		}
		break;

		case Change::Type::SURVIVED:
		{
			precleanup();
		}
		break;

		case Change::Type::GROWS:
		{
			// The tile either gains a powered Figure or empowers one of its
			// existing Figures.
			changeStacksPower(change.stacks, change.power, owner, group);

			size_t from;
			size_t to;
			if (change.power > 0)
			{
				from = power - change.power;
				to = power;
			}
			else if (change.power == 0)
			{
				from = (power < stacks) ? power : power - 1;
				to = from + 1;
			}
			else // (change.power < 0)
			{
				from = power;
				to = power - change.power;
			}
			for (size_t i = from; i < to; i++)
			{
				if (i >= _figures.size() || !_figures[i])
				{
					LOGW << "nullptr detected in _figures";
					continue;
				}
				std::shared_ptr<Figure>& figure = _figures[i];
				figure->animate(change, group, _square);
			}

			if (group
				&& (group->player == owner
					|| group->player == Player::OBSERVER))
			{
				group->delay += 0.40f * 5 / (5 + (group->ticks++));
			}
		}
		break;

		case Change::Type::GAS:
		{
		for (size_t i = 0; i == 0; i++)
			{
				if (_surfaces.empty() || !_surfaces[0])
				{
					LOGW << "nullptr detected in _surfaces";
					continue;
				}
				std::shared_ptr<Surface>& surface = _surfaces[0];
				surface->animate(change, group, _square);
			}
			for (size_t i = 0; i < _nfigures; i++)
			{
				if (i >= _figures.size() || !_figures[i])
				{
					LOGW << "nullptr detected in _figures";
					continue;
				}
				std::shared_ptr<Figure>& figure = _figures[i];
				figure->animate(change, group, _square);
			}

			if((int)change.gas == 2) {
				Mixer::get()->queue(Clip::Type::GAS, 0.7f);
				Mixer::get()->queue(Clip::Type::EMINATE, 1.2f);
			}
			else if((int)change.gas == 1 && group) {
				Mixer::get()->queue(Clip::Type::EMINATE, group->delay, Surface::convertOrigin(_square->position()));
				group->delay += 0.1f;
			}
		}
		break;
		case Change::Type::FROSTBITE:
		{
			for (size_t i = 0; i == 0; i++)
			{
				if (_surfaces.empty() || !_surfaces[0])
				{
					LOGW << "nullptr detected in _surfaces";
					continue;
				}
				std::shared_ptr<Surface>& surface = _surfaces[0];
				surface->animate(change, group, _square);
			}
			for (size_t i = 0; i < _nfigures; i++)
			{
				if (i >= _figures.size() || !_figures[i])
				{
					LOGW << "nullptr detected in _figures";
					continue;
				}
				std::shared_ptr<Figure>& figure = _figures[i];
				figure->animate(change, group, _square);
			}

			if(group && change.frostbite) {
				Mixer::get()->queue(Clip::Type::FROSTBITE, group->delay, Surface::convertOrigin(_square->position()));
				group->delay += 0.1f;
			}
		}
		break;
		case Change::Type::FIRESTORM:
		{
			for (size_t i = 0; i == 0; i++)
			{
				if (_surfaces.empty() || !_surfaces[0])
				{
					LOGW << "nullptr detected in _surfaces";
					continue;
				}
				std::shared_ptr<Surface>& surface = _surfaces[0];
				surface->animate(change, group, _square);
			}
			for (size_t i = 0; i < _nfigures; i++)
			{
				if (i >= _figures.size() || !_figures[i])
				{
					LOGW << "nullptr detected in _figures";
					continue;
				}
				std::shared_ptr<Figure>& figure = _figures[i];
				figure->animate(change, group, _square);
			}

			if(group && change.firestorm) {
				Mixer::get()->queue(Clip::Type::FIRESTORM, group->delay, Surface::convertOrigin(_square->position()));
				group->delay += 0.1f;
			}
		}
		break;
		case Change::Type::SNOW:
		case Change::Type::BONEDROUGHT:
		case Change::Type::DEATH:
		case Change::Type::RADIATION:
		case Change::Type::TEMPERATURE:
		case Change::Type::HUMIDITY:
		{
			for (size_t i = 0; i == 0; i++)
			{
				if (_surfaces.empty() || !_surfaces[0])
				{
					LOGW << "nullptr detected in _surfaces";
					continue;
				}
				std::shared_ptr<Surface>& surface = _surfaces[0];
				surface->animate(change, group, _square);
			}
			for (size_t i = 0; i < _nfigures; i++)
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

		case Change::Type::CHAOS:
		{
			float val = (!_square->chaos() && _bible.tileChaosProtection(type))
				? -1.0f : (float) _square->chaos();
			for (size_t i = 0; i == 0; i++)
			{
				if (_surfaces.empty() || !_surfaces[0])
				{
					LOGW << "nullptr detected in _surfaces";
					continue;
				}
				std::shared_ptr<Surface>& surface = _surfaces[0];
				surface->transition(group, TRANSITION_CHAOS, val, 1);
			}
		}
		break;

		case Change::Type::INCOME:
		{
			if (group)
			{
				group->morale = (_bible.moraleReplacesMoney());
			}

			int totalamount = std::abs(change.money);
			// Most tiles produce income per (powered) building.
			int ngenerators = _bible.powerAbolished() ? stacks : power;
			// Certain morale changes use CELL instead of unit.
			if (change.subject.type == Descriptor::Type::CELL)
			{
				ngenerators = 0;
			}
			// Crops generate income but are not powered.
			int count = std::max(1, ngenerators);
			// The reactor generates more than one coin per power.
			for (int x = 0; x < totalamount; x += count)
			{
				int amount = std::max(0, std::min(totalamount - x, count));
				for (size_t i = 0; i < (size_t) amount; i++)
				{
					if (i >= (size_t) std::max(0, ngenerators)
						|| _bible.moraleReplacesMoney())
					{
						if (_surfaces.empty() || !_surfaces[0])
						{
							LOGW << "nullptr detected in _surfaces";
							continue;
						}
						std::shared_ptr<Surface>& surface = _surfaces[0];
						surface->animate(change, group, _square);
					}
					else
					{
						if (i >= _figures.size() || !_figures[i])
						{
							LOGW << "nullptr detected in _figures";
							continue;
						}
						std::shared_ptr<Figure>& figure = _figures[i];
						figure->animate(change, group, _square);
					}

					if (group && group->player == change.player)
					{
						// We want to update group->money inside an Animation,
						// but we do not want the lambda function to keep group
						// alive.
						std::weak_ptr<AnimationGroup> weak = group;
						addAnimation(Animation(group, [weak](float /**/){

							if (auto g = weak.lock())
							{
								g->money += 1;
							}
						}, 0, group->delay));
					}

					if (group
						&& (group->player == change.player
							|| group->player == Player::OBSERVER))
					{
						group->delay += 0.090f * 5 / (5 + (group->ticks++));
					}
				}

				if (group
					&& (group->player == change.player
						|| group->player == Player::OBSERVER))
				{
					group->delay += 0.400f * 5 / (5 + (group->ticks++));
				}
			}
		}
		break;

		case Change::Type::SCORED:
		{
			// Perhaps some tiles generate more than one point per stack.
			for (int x = 0; x < change.score; x++)
			{
				if (_surfaces.empty() || !_surfaces[0])
				{
					LOGW << "nullptr detected in _surfaces";
					continue;
				}
				std::shared_ptr<Surface>& surface = _surfaces[0];
				surface->animate(change, group, _square);

				if (group) group->delay += 0.400f * 5 / (5 + (group->ticks++));
			}
		}
		break;

		default:
		{
			LOGW << "Missing case \""
				+ std::string(Change::stringify(change.type))
				+ "\" in Tile::enact";
		}
		break;
	}
}

void Tile::animateBlock(const Change& change,
	std::shared_ptr<AnimationGroup> group, float delay)
{
	if (_surfaces.empty() || !_surfaces[0])
	{
		LOGW << "nullptr detected in _surfaces";
		return;
	}
	std::shared_ptr<Surface>& surface = _surfaces[0];
	surface->animateBlock(change, group, delay);
}

float Tile::animateOccupy(const Change& /**/,
	std::shared_ptr<AnimationGroup> group, float delay)
{
	float duration = 0;
	for (size_t i = 0; i < _nfigures; i++)
	{
		if (i >= _figures.size() || !_figures[i])
		{
			LOGW << "nullptr detected in _figures";
			continue;
		}
		std::shared_ptr<Figure>& figure = _figures[i];

		if (_bible.powerAbolished() || i < (size_t) std::max(0, (int) power))
		{
			figure->transition(group, TRANSITION_POWER, 0.6f, 0.300f, delay);
			duration = 0.300f;
		}
	}
	return duration;
}

float Tile::animateDeoccupy(const Change& /**/,
	std::shared_ptr<AnimationGroup> group, float delay)
{
	float duration = 0;
	for (size_t i = 0; i < _nfigures; i++)
	{
		if (i >= _figures.size() || !_figures[i])
		{
			LOGW << "nullptr detected in _figures";
			continue;
		}
		std::shared_ptr<Figure>& figure = _figures[i];

		if (_bible.powerAbolished() || i < (size_t) std::max(0, (int) power))
		{
			figure->transition(group, TRANSITION_POWER, 1, 0.300f, delay);
			duration = 0.300f;
		}
	}
	return duration;
}

void Tile::light(float light, float duration, float delay)
{
	if (type == TileType::NONE) return;

	for (size_t i = 0; i == 0; i++)
	{
		if (i >= _surfaces.size() || !_surfaces[i])
		{
			LOGW << "nullptr detected in _surfaces";
			continue;
		}
		std::shared_ptr<Surface>& surface = _surfaces[i];
		surface->transition(TRANSITION_LIGHT, light, duration, delay);
	}
	for (size_t i = 0; i < _nfigures; i++)
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

void Tile::spotlight(float light, float duration, float delay)
{
	if (type == TileType::NONE) return;

	for (size_t i = 0; i == 0; i++)
	{
		if (i >= _surfaces.size() || !_surfaces[i])
		{
			LOGW << "nullptr detected in _surfaces";
			continue;
		}
		std::shared_ptr<Surface>& surface = _surfaces[i];
		surface->transition(TRANSITION_SPOTLIGHT, light, duration, delay);
	}
	for (size_t i = 0; i < _nfigures; i++)
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

void Tile::flashlight(float light, float duration, float delay)
{
	if (type == TileType::NONE) return;

	for (size_t i = 0; i == 0; i++)
	{
		if (i >= _surfaces.size() || !_surfaces[i])
		{
			LOGW << "nullptr detected in _surfaces";
			continue;
		}
		std::shared_ptr<Surface>& surface = _surfaces[i];
		surface->transition(TRANSITION_FLASHLIGHT, light, duration, delay);
	}
	for (size_t i = 0; i < _nfigures; i++)
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

void Tile::precleanup()
{
	// The precleanup moves all "bad" figures (null or dying) to the end of the vector.
	// Through this process, the figures in the range [start, figure) are bad.
	auto start = _figures.begin();
	for (auto figure = _figures.begin(); figure != _figures.end(); figure++)
	{
		if (!*figure)
		{
			LOGW << "nullptr detected in _figures in tile precleanup";
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

void Tile::cleanup()
{
	if (!_surfaces.empty())
	{
		_surfaces.resize(1);
	}

	auto figure = _figures.begin();
	while (figure != _figures.end())
	{
		if (!*figure)
		{
			LOGW << "nullptr detected in _figures in tile cleanup";
			figure = _figures.erase(figure);
		}
		else if ((*figure)->isDying()) figure = _figures.erase(figure);
		else figure++;
	}
}

int Tile::score(const TileToken& tile)
{
	return _bible.tileScoreBase(tile.type)
		+ tile.stacks * _bible.tileScoreStack(tile.type);
}
