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

#include "libs/GLEW/glew.h"

#include "pixel.hpp"

class Sprite;
class Picture;
class Text;
class Primitive;
struct Point;
class Graphics;


class Collector
{
private:
	static Collector* _installed;

	struct CoSprite
	{
		Pixel pixel;
		std::shared_ptr<Sprite> sprite;

		CoSprite(const std::shared_ptr<Sprite> spr, const Pixel& px) :
			pixel(px),
			sprite(spr)
		{}
	};

	struct CoPicture
	{
		Pixel pixel;
		std::shared_ptr<Picture> picture;

		CoPicture(const std::shared_ptr<Picture> pic, const Pixel& px) :
			pixel(px),
			picture(pic)
		{}
	};

	struct CoText
	{
		Pixel pixel;
		std::shared_ptr<Text> text;

		CoText(const std::shared_ptr<Text> txt, const Pixel& px) :
			pixel(px),
			text(txt)
		{}
	};

	struct CoPrimitive
	{
		Pixel pixel;
		std::shared_ptr<Primitive> primitive;

		CoPrimitive(const std::shared_ptr<Primitive> prm, const Pixel& px) :
			pixel(px),
			primitive(prm)
		{}
	};

	struct CoLayer
	{
		Proximity proximity;
		std::vector<CoPicture> pictures;
		std::vector<CoSprite> frames;
		std::vector<CoPrimitive> primitives;
		std::vector<CoSprite> images;
		std::vector<CoText> texts;
	};

	const Graphics& _graphics;

	// Surfaces are drawn in separated draw steps. Within each draw step, the
	// parts of the surface that are drawn are non-overlapping, so their
	// relative order does not matter.
	std::vector<CoSprite> _backgroundSurfaces;
	std::vector<CoSprite> _unmaskedBorderSurfaces;
	std::vector<CoSprite> _unmaskedSurfaces;
	std::vector<CoSprite> _maskedSurfaces;
	std::vector<CoSprite> _maskedBorderSurfaces;

	// Alternatively, surfaces and are sorted in y,x order.
	std::vector<CoSprite> _sortedSurfaces;

	// Shadows, footprints, guides and hovers also belong to Layer::SURFACE.
	std::vector<CoSprite> _shadows;
	std::vector<CoSprite> _footprints;
	std::vector<CoSprite> _guides;
	std::vector<CoSprite> _hovers;

	// Figures (and particles) belong to Layer::FIGURE and are sorted in y,x
	// order to make the faux 3D popup effect work.
	std::vector<CoSprite> _figures;

	// Guidestamps and busycursors also belong to Layer::FIGURE.
	std::vector<CoSprite> _guidestamps;
	std::vector<CoSprite> _busycursors;

	// A vector of interface layers for all of the interface elements.
	std::vector<CoLayer> _layers;

public:
	static Collector* get() { return _installed; }

	Collector(const Graphics& graphics);

	void install();

private:
	void addAlways(std::vector<CoSprite>& drawlist,
		const std::shared_ptr<Sprite> sprite, const Point& point);
	void addOnScreen(std::vector<CoSprite>& drawlist,
		const std::shared_ptr<Sprite> sprite, const Point& point);

	CoLayer& addLayer(const Proximity& proximity);

public:
	void addBackgroundSurface(const std::shared_ptr<Sprite> sprite,
		const Point& point);
	void addSurfaceUnmasked(const std::shared_ptr<Sprite> sprite,
		const Point& point);
	void addSurfaceMasked(const std::shared_ptr<Sprite> sprite,
		const Point& point);
	void addBorderSurfaceUnmasked(const std::shared_ptr<Sprite> sprite,
		const Point& point);
	void addBorderSurfaceMasked(const std::shared_ptr<Sprite> sprite,
		const Point& point);
	void addSortedSurface(const std::shared_ptr<Sprite> sprite,
		const Point& point);
	void addShadow(const std::shared_ptr<Sprite> sprite,
		const Point& point);
	void addFootprint(const std::shared_ptr<Sprite> sprite,
		const Point& point);
	void addGuide(const std::shared_ptr<Sprite> sprite,
		const Point& point);
	void addHovercursor(const std::shared_ptr<Sprite> sprite,
		const Point& point);
	void addFigure(const std::shared_ptr<Sprite> sprite,
		const Point& point);
	void addParticle(const std::shared_ptr<Sprite> sprite,
		const Point& point);
	void addGuidestamp(const std::shared_ptr<Sprite> sprite,
		const Point& point);
	void addBusycursor(const std::shared_ptr<Sprite> sprite,
		const Pixel& pixel);
	void addFrame(const std::shared_ptr<Sprite> sprite,
		const Pixel& pixel);
	void addImage(const std::shared_ptr<Sprite> sprite,
		const Pixel& pixel);
	void add(const std::shared_ptr<Picture> picture, const Pixel& pixel);
	void add(const std::shared_ptr<Text> text, const Pixel& pixel);
	void add(const std::shared_ptr<Primitive> primitive, const Pixel& pixel);
	void render();
	void empty();

private:
	void renderSprites();
	void renderAndClear(GLuint spriteshader, std::vector<CoSprite>& drawlist);

	void renderLayers();
	void renderLayerPictures(const CoLayer& layer);
	void renderLayerFrames(const CoLayer& layer);
	void renderLayerPrimitives(const CoLayer& layer);
	void renderLayerImages(const CoLayer& layer);
	void renderLayerTexts(const CoLayer& layer);

	static bool lessYahooOrXenon(const CoSprite& a, const CoSprite& b);
	static bool lessProximity(const CoLayer& a, const CoLayer& b);
};
