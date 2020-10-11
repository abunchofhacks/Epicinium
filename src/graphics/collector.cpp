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
#include "collector.hpp"
#include "source.hpp"

#include "camera.hpp"
#include "sprite.hpp"
#include "picture.hpp"
#include "text.hpp"
#include "primitive.hpp"
#include "graphics.hpp"


Collector* Collector::_installed = nullptr;

void Collector::install()
{
	_installed = this;
}

Collector::Collector(const Graphics& graphics) :
	_graphics(graphics)
{}

void Collector::addAlways(std::vector<CoSprite>& drawlist,
		const std::shared_ptr<Sprite> sprite, const Point& point)
{
	Camera* camera = Camera::get();
	Pixel pixel = camera->convert(point);

	drawlist.emplace_back(sprite, pixel);
}

void Collector::addOnScreen(std::vector<CoSprite>& drawlist,
		const std::shared_ptr<Sprite> sprite, const Point& point)
{
	Camera* camera = Camera::get();
	Pixel pixel = camera->convert(point);

	if (sprite->topleftXenon(pixel.xenon) + sprite->scaledWidth()
			>                   -5 * camera->SCALE
		&&  sprite->topleftXenon(pixel.xenon)
			< camera->WINDOW_W + 5 * camera->SCALE
		&&  sprite->topleftYahoo(pixel.yahoo) + sprite->scaledHeight()
			>                   -5 * camera->SCALE
		&&  sprite->topleftYahoo(pixel.yahoo)
			< camera->WINDOW_H + 5 * camera->SCALE)
	{
		drawlist.emplace_back(sprite, pixel);
	}
}

void Collector::addBackgroundSurface(const std::shared_ptr<Sprite> sprite,
	const Point& point)
{
	addAlways(_backgroundSurfaces, sprite, point);
}

void Collector::addSurfaceUnmasked(const std::shared_ptr<Sprite> sprite,
	const Point& point)
{
	addOnScreen(_unmaskedSurfaces, sprite, point);
}

void Collector::addSurfaceMasked(const std::shared_ptr<Sprite> sprite,
	const Point& point)
{
	addOnScreen(_maskedSurfaces, sprite, point);
}

void Collector::addBorderSurfaceUnmasked(const std::shared_ptr<Sprite> sprite,
	const Point& point)
{
	addOnScreen(_unmaskedBorderSurfaces, sprite, point);
}

void Collector::addBorderSurfaceMasked(const std::shared_ptr<Sprite> sprite,
	const Point& point)
{
	addOnScreen(_maskedBorderSurfaces, sprite, point);
}

void Collector::addSortedSurface(const std::shared_ptr<Sprite> sprite,
	const Point& point)
{
	addOnScreen(_sortedSurfaces, sprite, point);
}

void Collector::addShadow(const std::shared_ptr<Sprite> sprite,
	const Point& point)
{
	addOnScreen(_shadows, sprite, point);
}

void Collector::addFootprint(const std::shared_ptr<Sprite> sprite,
	const Point& point)
{
	addOnScreen(_footprints, sprite, point);
}

void Collector::addGuide(const std::shared_ptr<Sprite> sprite,
	const Point& point)
{
	addOnScreen(_guides, sprite, point);
}

void Collector::addHovercursor(const std::shared_ptr<Sprite> sprite,
	const Point& point)
{
	addOnScreen(_hovers, sprite, point);
}

void Collector::addFigure(const std::shared_ptr<Sprite> sprite,
	const Point& point)
{
	addOnScreen(_figures, sprite, point);
}

void Collector::addParticle(const std::shared_ptr<Sprite> sprite,
	const Point& point)
{
	// Particles and figures are drawn at the same time.
	addOnScreen(_figures, sprite, point);
}

void Collector::addGuidestamp(const std::shared_ptr<Sprite> sprite,
	const Point& point)
{
	addOnScreen(_guidestamps, sprite, point);
}

void Collector::addBusycursor(const std::shared_ptr<Sprite> sprite,
	const Pixel& pixel)
{
	_busycursors.emplace_back(sprite, pixel);
}

Collector::CoLayer& Collector::addLayer(const Proximity& proximity)
{
	for (CoLayer& layer : _layers)
	{
		if (layer.proximity == proximity)
		{
			return layer;
		}
	}

	_layers.emplace_back();
	_layers.back().proximity = proximity;
	return _layers.back();
}

void Collector::addFrame(const std::shared_ptr<Sprite> sprite,
	const Pixel& pixel)
{
	CoLayer& layer = addLayer(pixel.proximity);
	layer.frames.emplace_back(sprite, pixel);
}

void Collector::addImage(const std::shared_ptr<Sprite> sprite,
	const Pixel& pixel)
{
	CoLayer& layer = addLayer(pixel.proximity);
	layer.images.emplace_back(sprite, pixel);
}

void Collector::add(const std::shared_ptr<Picture> picture, const Pixel& pixel)
{
	CoLayer& layer = addLayer(pixel.proximity);
	layer.pictures.emplace_back(picture, pixel);
}

void Collector::add(const std::shared_ptr<Text> text, const Pixel& pixel)
{
	CoLayer& layer = addLayer(pixel.proximity);
	layer.texts.emplace_back(text, pixel);
}

void Collector::add(const std::shared_ptr<Primitive> prim, const Pixel& pixel)
{
	CoLayer& layer = addLayer(pixel.proximity);
	layer.primitives.emplace_back(prim, pixel);
}

bool Collector::lessYahooOrXenon(const CoSprite& a, const CoSprite& b)
{
	// the one with the smaller y must be drawn first
	return (a.pixel.yahoo  < b.pixel.yahoo)
		|| (a.pixel.yahoo == b.pixel.yahoo && a.pixel.xenon < b.pixel.xenon);
}

bool Collector::lessProximity(const CoLayer& a, const CoLayer& b)
{
	// the one with the smaller proximity must be drawn first
	return (a.proximity < b.proximity);
}

void Collector::render()
{
	renderSprites();
	renderLayers();

	glUseProgram(0);
}

void Collector::renderSprites()
{
	GLuint spriteshader = _graphics.getSpriteShader();
	glUseProgram(spriteshader);

	/* Draw the surfaces. */

	for (const CoSprite& item : _backgroundSurfaces)
	{
		item.sprite->render(spriteshader, item.pixel.xenon, item.pixel.yahoo,
				SurfaceDrawFlag::UNMASKED);
	}
	_backgroundSurfaces.clear();

	for (const CoSprite& item : _unmaskedBorderSurfaces)
	{
		item.sprite->render(spriteshader, item.pixel.xenon, item.pixel.yahoo,
				SurfaceDrawFlag::UNMASKED);
	}
	_unmaskedBorderSurfaces.clear();

	for (const CoSprite& item : _unmaskedSurfaces)
	{
		item.sprite->render(spriteshader, item.pixel.xenon, item.pixel.yahoo,
				SurfaceDrawFlag::UNMASKED);
	}
	_unmaskedSurfaces.clear();

	for (const CoSprite& item : _maskedSurfaces)
	{
		item.sprite->render(spriteshader, item.pixel.xenon, item.pixel.yahoo,
				SurfaceDrawFlag::MASKED);
	}
	_maskedSurfaces.clear();

	for (const CoSprite& item : _maskedBorderSurfaces)
	{
		item.sprite->render(spriteshader, item.pixel.xenon, item.pixel.yahoo,
				SurfaceDrawFlag::MASKED);
	}
	_maskedBorderSurfaces.clear();

	std::sort(_sortedSurfaces.begin(), _sortedSurfaces.end(), lessYahooOrXenon);
	for (const CoSprite& item : _sortedSurfaces)
	{
		item.sprite->render(spriteshader, item.pixel.xenon, item.pixel.yahoo,
				SurfaceDrawFlag::UNMASKED | SurfaceDrawFlag::MASKED);
	}
	_sortedSurfaces.clear();

	/* Draw the shadows and guides on top of the surfaces. */

	std::sort(_shadows.begin(), _shadows.end(), lessYahooOrXenon);
	renderAndClear(spriteshader, _shadows);

	std::sort(_footprints.begin(), _footprints.end(), lessYahooOrXenon);
	renderAndClear(spriteshader, _footprints);

	renderAndClear(spriteshader, _guides);
	renderAndClear(spriteshader, _hovers);

	/* Draw the figures and particles in y,x-order. */

	std::sort(_figures.begin(), _figures.end(), lessYahooOrXenon);
	renderAndClear(spriteshader, _figures);

	/* Draw the guidestamps and busycursor. */

	renderAndClear(spriteshader, _guidestamps);
	renderAndClear(spriteshader, _busycursors);
}

void Collector::renderLayers()
{
	/* Draw the interface elements. */

	std::sort(_layers.begin(), _layers.end(), lessProximity);
	for (const CoLayer& layer : _layers)
	{
		renderLayerPictures(layer);
		renderLayerFrames(layer);
		renderLayerPrimitives(layer);
		renderLayerImages(layer);
		renderLayerTexts(layer);
	}
	_layers.clear();
}

void Collector::renderLayerPictures(const CoLayer& layer)
{
	GLuint shader = _graphics.getPictureShader();
	glUseProgram(shader);

	for (const CoPicture& item : layer.pictures)
	{
		item.picture->render(shader, item.pixel.xenon, item.pixel.yahoo);
	}
}

void Collector::renderLayerFrames(const CoLayer& layer)
{
	GLuint shader = _graphics.getSpriteShader();
	glUseProgram(shader);

	for (const CoSprite& item : layer.frames)
	{
		item.sprite->render(shader, item.pixel.xenon, item.pixel.yahoo);
	}
}

void Collector::renderLayerPrimitives(const CoLayer& layer)
{
	GLuint shader = _graphics.getPrimitiveShader();
	glUseProgram(shader);

	for (const CoPrimitive& item : layer.primitives)
	{
		item.primitive->render(shader, item.pixel.xenon, item.pixel.yahoo);
	}
}

void Collector::renderLayerImages(const CoLayer& layer)
{
	GLuint shader = _graphics.getSpriteShader();
	glUseProgram(shader);

	for (const CoSprite& item : layer.images)
	{
		item.sprite->render(shader, item.pixel.xenon, item.pixel.yahoo);
	}
}

void Collector::renderLayerTexts(const CoLayer& layer)
{
	GLuint shader = _graphics.getTextShader();
	glUseProgram(shader);

	for (const CoText& item : layer.texts)
	{
		item.text->render(shader, item.pixel.xenon, item.pixel.yahoo);
	}
}

void Collector::renderAndClear(GLuint spriteshader,
	std::vector<CoSprite>& drawlist)
{
	for (const CoSprite& item : drawlist)
	{
		item.sprite->render(spriteshader, item.pixel.xenon, item.pixel.yahoo);
	}
	drawlist.clear();
}

void Collector::empty()
{
	_backgroundSurfaces.clear();
	_unmaskedBorderSurfaces.clear();
	_unmaskedSurfaces.clear();
	_maskedSurfaces.clear();
	_maskedBorderSurfaces.clear();
	_sortedSurfaces.clear();
	_shadows.clear();
	_footprints.clear();
	_guides.clear();
	_hovers.clear();
	_figures.clear();
	_guidestamps.clear();
	_busycursors.clear();
	_layers.clear();
}
