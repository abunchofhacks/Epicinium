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
#include "border.hpp"
#include "source.hpp"

#include "bible.hpp"
#include "skinner.hpp"
#include "tiletype.hpp"
#include "treetype.hpp"
#include "placementgrid.hpp"
#include "surface.hpp"
#include "figure.hpp"


Border::Border(const Position& position,
		const Bible& bible, const Skinner& skinner,
		int distance) :
	_position(position),
	_bible(bible),
	_skinner(skinner),
	_distance(std::max(0, std::min(distance, 5))),
	_type(TileType::NONE),
	_east(TileType::NONE),
	_south(TileType::NONE),
	_west(TileType::NONE),
	_north(TileType::NONE),
	_treetype(TreeType::UNSET),
	_nfigures(0)
{
	Point topleft = Surface::convertTopleft(_position);
	_placementgrid.reset(new PlacementGrid(Point(topleft.xenon + 5, topleft.yahoo + 4),
			Point(topleft.xenon + Surface::WIDTH - 5, topleft.yahoo + Surface::HEIGHT - 1)));
}

Border::Border(Border&&) = default;

Border::~Border() = default;

void Border::set(const TileType& tiletype,
		const TileType& e, const TileType& s,
		const TileType& w, const TileType& n,
		const TreeType& treetype)
{
	_type = tiletype;
	_east = e;
	_south = s;
	_west = w;
	_north = n;
	_treetype = treetype;
}

std::shared_ptr<Surface> Border::newSurface()
{
	auto surface = std::make_shared<Surface>(this, _skinner.border());
	surface->blendTop(nullptr);
	surface->blendLeft(nullptr);
	surface->set(TRANSITION_LIGHT, 0);
	surface->set(TRANSITION_SPOTLIGHT, 1);
	surface->set(TRANSITION_FLASHLIGHT, 0);
	surface->setBorder();
	return surface;
}

std::shared_ptr<Figure> Border::newFigure()
{
	PlacementBox box = _placementgrid->place(Descriptor::Type::TILE);
	int east  = (_type == _east)  ? -10 : 30;
	int south = (_type == _south) ? -15 :  5;
	int west  = (_type == _west)  ? -10 : 30;
	int north = (_type == _north) ?   0 : 20;
	box.centralize(east, south, west, north);
	auto figure = std::make_shared<Figure>(std::move(box),
		_skinner.figure(_type, _treetype, Player::NONE),
		Player::NONE);
	figure->set(TRANSITION_LIGHT, 0);
	figure->set(TRANSITION_SPOTLIGHT, 1);
	figure->set(TRANSITION_FLASHLIGHT, 0);
	figure->setBorder();
	return figure;
}

void Border::setAsBackground()
{
	for (size_t i = 0; i == 0; i++)
	{
		if (_surfaces.empty() || !_surfaces[0])
		{
			LOGW << "nullptr detected in _surfaces";
			continue;
		}
		std::shared_ptr<Surface>& surface = _surfaces[0];
		surface->setAsBackground();
	}
}

void Border::populate()
{
	{
		std::shared_ptr<Surface> surface(newSurface());
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
		surface->setVisible(false);
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

	if (_bible.tileStacksMax(_type) > 0 && !_bible.tileControllable(_type))
	{
		_nfigures = (size_t) std::max(0, std::min(5,
			(int) _bible.tileStacksMax(_type) - _distance + (rand() % 2)));
	}
	else _nfigures = 0;

	for (size_t i = 0; i < _nfigures; i++)
	{
		std::shared_ptr<Figure> figure(newFigure());
		_figures.emplace_back(figure);
	}

	if (leftovers)
	{
		std::rotate(_figures.begin(), _figures.begin() + leftovers, _figures.end());
	}

	cleanup();
}

void Border::update()
{
	Animator::update();

	// All surfaces need to be updated, not just 0.
	for (auto& surface : _surfaces)
	{
		if (!surface)
		{
			LOGW << "nullptr detected in _surfaces in border update";
			continue;
		}

		surface->update();
	}

	// All figures need to be updated, not just those in [0, _nfigures).
	for (auto& figure : _figures)
	{
		if (!figure)
		{
			LOGW << "nullptr detected in _figures in border update";
			continue;
		}

		figure->update();
	}
}

void Border::setLight(float light, float duration, float delay)
{
	for (size_t i = 0; i == 0; i++)
	{
		if (_surfaces.empty() || !_surfaces[0])
		{
			continue;
		}
		std::shared_ptr<Surface>& surface = _surfaces[0];
		surface->transition(TRANSITION_LIGHT, light, duration, delay);
	}
	for (size_t i = 0; i < _nfigures; i++)
	{
		if (i >= _figures.size() || !_figures[i])
		{
			continue;
		}
		std::shared_ptr<Figure>& figure = _figures[i];
		figure->transition(TRANSITION_LIGHT, light, duration, delay);
	}
}

void Border::setSpotlight(float light, float duration, float delay)
{
	for (size_t i = 0; i == 0; i++)
	{
		if (_surfaces.empty() || !_surfaces[0])
		{
			continue;
		}
		std::shared_ptr<Surface>& surface = _surfaces[0];
		surface->transition(TRANSITION_SPOTLIGHT, light, duration, delay);
	}
	for (size_t i = 0; i < _nfigures; i++)
	{
		if (i >= _figures.size() || !_figures[i])
		{
			continue;
		}
		std::shared_ptr<Figure>& figure = _figures[i];
		figure->transition(TRANSITION_SPOTLIGHT, light, duration, delay);
	}
}

void Border::cleanup()
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
			LOGW << "nullptr detected in _figures in border cleanup";
			figure = _figures.erase(figure);
		}
		else if ((*figure)->isDying()) figure = _figures.erase(figure);
		else figure++;
	}
}
