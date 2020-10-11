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

#include "actor.hpp"
#include "position.hpp"
#include "point.hpp"
#include "guide.hpp"
#include "paint.hpp"
#include "player.hpp"

struct Change;
struct AnimationGroup;
class Square;
class Border;
enum class TextureType : uint8_t;
struct Skin;

class Surface : public Actor
{
private:
	enum class WeatherType : uint8_t
	{
		NONE = 0,
		RAINWEAK,
		RAIN,
		RAINHEAVY,
		SNOW,
		FROSTBITE,
		FIRESTORM,
		BONEDROUGHT,
		DEATH,
		GAS,
		RADIATION,
	};

	enum class HazardType : uint8_t
	{
		DEATH,
		FIRESTORM,
		RADIATION,
		GAS,
		FROSTBITE,
		SIZE
	};

public:
	static constexpr int WIDTH  = 60;
	static constexpr int HEIGHT = 60;

	Surface(const Square* square, const Skin& skin, const Player& player);
	Surface(const Border* border, const Skin& skin);

	Surface(const Surface&) = delete;
	Surface(Surface&&) = delete;
	Surface& operator=(const Surface&) = delete;
	Surface& operator=(Surface&&) = delete;

	virtual ~Surface() = default;

private:
	Player _player;

	TextureType _texturetype;

	int _palettesize;
	const Paint* _basicPalette;
	const Paint* _dryPalette;
	const Paint* _hotPalette;
	const Paint* _coldPalette;
	const Paint* _snowPalette;
	std::array<const Paint*, PLAYER_MAX + 1> _teamPalettes;

	std::bitset<(size_t) HazardType::SIZE> _hazards;
	std::array<Guide, (size_t) HazardType::SIZE> _hazardguides;
	int _raininess;
	float _climateModifier;
	float _weathercountdown;

protected:
	virtual void drawSprite() override;

	virtual void mix() override;

	void impact(const Change& change,
		std::shared_ptr<AnimationGroup> group, float delay);
	void boom(const Change& change,
		std::shared_ptr<AnimationGroup> group, float delay);

public:
	void mowGrass(std::shared_ptr<AnimationGroup> group);
	void generateStatik(std::shared_ptr<AnimationGroup> group);

protected:
	void weather(std::shared_ptr<AnimationGroup> group,
		const WeatherType& type, float percentage);

	void addFlash(std::shared_ptr<AnimationGroup> group,
		Square* square, float amount,
		float delay, float upduration, float downduration);

	const char* blendnameTop(TextureType top, TextureType bottom);
	const char* blendnameLeft(TextureType left, TextureType right);

public:
	bool inheritBlends(std::shared_ptr<Surface> old);
	void blendTop(std::shared_ptr<Surface> other);
	void blendLeft(std::shared_ptr<Surface> other);

	void update();
	void updateWeather(const Square* square);
	void setBorder();
	void setAsBackground();
	void setPlayer(const Player& player);

	TextureType texturetype() const { return _texturetype; }

	static float temperatureTransitionCap(float fraction);
	static float humidityTransitionCap(float fraction);

	void animate(const Change& change,
		std::shared_ptr<AnimationGroup> group, Square* square);
	void animateScoreLoss(const Change& change,
		std::shared_ptr<AnimationGroup> group, float delay);
	void animateBlock(const Change& change,
		std::shared_ptr<AnimationGroup> group, float delay);

	static std::shared_ptr<Skin> missingSkin();

	static Position convert(const Point& point)
	{
		Position position;
		convertTo(point, position);
		return position;
	}

	// TODO move this to header maybe? or to separate custom math header?
	static constexpr int floordiv(int x, int y)
	{
		// Calculate quotient and remainder in single machine instruction.
		// Floor the quotient if necessary.
		return (x / y) - ((x % y) && ((x < 0) ^ (y < 0)));
	}

	static void convertTo(const Point& point, Position& position)
	{
		position.col = floordiv(point.xenon, WIDTH);
		position.row = floordiv(point.yahoo, HEIGHT);
	}

	static Point convertTopleft(const Position& position)
	{
		Point point;
		convertToTopleft(position, point);
		return point;
	}

	static void convertToTopleft(const Position& position, Point& point)
	{
		point.xenon = position.col * WIDTH;
		point.yahoo = position.row * HEIGHT;
	}

	static Point convertCenter(const Position& position)
	{
		Point point;
		convertToCenter(position, point);
		return point;
	}

	static void convertToCenter(const Position& position, Point& point)
	{
		point.xenon = position.col * WIDTH  + WIDTH  / 2;
		point.yahoo = position.row * HEIGHT + HEIGHT / 2;
	}

	static Point convertOrigin(const Position& position)
	{
		Point point;
		convertToOrigin(position, point);
		return point;
	}

	static void convertToOrigin(const Position& position, Point& point)
	{
		point.xenon = position.col * WIDTH  + WIDTH / 2;
		point.yahoo = position.row * HEIGHT + HEIGHT;
	}
};
