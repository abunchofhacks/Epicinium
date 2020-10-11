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

#include "tiletoken.hpp"
#include "animator.hpp"

class Square;
class Surface;
class Figure;
struct AnimationGroup;
struct Change;
class Bible;
class Skinner;
class PlacementBox;
enum class TreeType : uint8_t;
enum class TextureType : uint8_t;


class Tile : public TileToken, private Animator
{
public:
	Tile(Square* square, const Bible& bible, const Skinner& skinner);
	Tile(const Tile&) = delete;
	Tile(Tile&& that);
	~Tile();

	Tile& operator=(const Tile&) = delete;
	Tile& operator=(Tile&&) = delete;

private:
	Square* _square;
	const Bible& _bible;
	const Skinner& _skinner;
	TreeType _treetype;
	std::vector<std::shared_ptr<Surface>> _surfaces; // (unique ownership)
	std::vector<std::shared_ptr<Figure>> _figures; // (unique ownership)
	size_t _nfigures = 0;
	bool _selected = false;

	std::shared_ptr<Surface> newSurface();
	PlacementBox newPlacement();
	std::shared_ptr<Figure> newFigure();
	std::shared_ptr<Figure> newFenceFigure();
	TextureType getTextureType();
	bool joint(Square* other);
	void blendSurfaces(std::shared_ptr<Surface> surface,
		std::shared_ptr<Surface> oldsurface,
		std::shared_ptr<AnimationGroup> group);

	void changeStacksPower(int8_t stackdiff, int8_t powerdiff,
		const Player& newowner, std::shared_ptr<AnimationGroup> group);
	float audioPan();

public:
	void update();

	bool hovered() const;
	bool selected() const;

	void select();
	void deselect();

	void populate(std::shared_ptr<AnimationGroup> group);
	void enact(const Change& change, std::shared_ptr<AnimationGroup> group);
	void animateBlock(const Change& change,
		std::shared_ptr<AnimationGroup> group, float delay);
	float animateOccupy(const Change& change,
		std::shared_ptr<AnimationGroup> group, float delay);
	float animateDeoccupy(const Change& change,
		std::shared_ptr<AnimationGroup> group, float delay);
	void light(float light, float duration, float delay);
	void spotlight(float light, float duration, float delay);
	void flashlight(float light, float duration, float delay);

	void precleanup();
	void cleanup();

	int score(const TileToken& tile);

	void pickTreeType();
	void fixTreeType();
	void setTreeType(TreeType treetype);
	void unsetTreeType();
	bool hasTreeType();
	TreeType getTreeType();
};
