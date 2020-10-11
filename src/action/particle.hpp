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

#include <functional>

#include "point.hpp"

struct AnimationGroup;
struct Color;
class Animation;
class Sprite;


class Particle
{
private:
	enum class State : uint8_t;

	Particle(std::shared_ptr<AnimationGroup> group,
		std::shared_ptr<Sprite> sprite,
		const Point* attachment, int xOffset, int yOffset, int height,
		float delay);

	Particle(std::shared_ptr<AnimationGroup> group,
		std::shared_ptr<Sprite> sprite,
		const Point& placement, int height,
		float delay);

public:
	~Particle();

private:
	State _state;
	Point _point;
	int _yahooOffset;
	std::unique_ptr<Animation> _animation;
	std::shared_ptr<Sprite> _sprite; // (unique ownership)

	std::function<void(float)> callback();
	std::function<void(float)> callback(const Point* attachment);

public:
	void update();

	static std::unique_ptr<Particle> blood(
		std::shared_ptr<AnimationGroup> group,
		const Point* attachment, int xOffset, int yOffset, int height,
		float delay);

	static std::unique_ptr<Particle> spark(
		std::shared_ptr<AnimationGroup> group,
		const Point* attachment, int xOffset, int yOffset, int height,
		float delay);

	static std::unique_ptr<Particle> woodchip(
		std::shared_ptr<AnimationGroup> group,
		const Point* attachment, int xOffset, int yOffset, int height,
		float delay);

	static std::unique_ptr<Particle> bloodspray(
		std::shared_ptr<AnimationGroup> group,
		const Point* attachment, int xOffset, int yOffset, int height,
		float delay);

	static std::unique_ptr<Particle> bloodboom(
		std::shared_ptr<AnimationGroup> group,
		const Point* attachment, int xOffset, int yOffset, int height,
		float delay);

	static std::unique_ptr<Particle> sparkboom(
		std::shared_ptr<AnimationGroup> group,
		const Point* attachment, int xOffset, int yOffset, int height,
		float delay);

	static std::unique_ptr<Particle> woodboom(
		std::shared_ptr<AnimationGroup> group,
		const Point* attachment, int xOffset, int yOffset, int height,
		float delay);

	static std::unique_ptr<Particle> spray(
		std::shared_ptr<AnimationGroup> group,
		const Point* attachment, int xOffset, int yOffset, int height,
		const Color& color, float delay = 0);
	static std::unique_ptr<Particle> spray(
		std::shared_ptr<AnimationGroup> group,
		const Point& placement,
		const Color& color, float delay = 0);

	static std::unique_ptr<Particle> boom(
		std::shared_ptr<AnimationGroup> group,
		const Point* attachment, int xOffset, int yOffset, int height,
		const Color& color, float delay = 0);
	static std::unique_ptr<Particle> boom(
		std::shared_ptr<AnimationGroup> group,
		const Point& placement,
		const Color& color, float delay = 0);

	static std::unique_ptr<Particle> explosion(
		std::shared_ptr<AnimationGroup> group,
		const Point* attachment, int xOffset, int yOffset, int height,
		float delay);

	static std::unique_ptr<Particle> smokeexplosion(
		std::shared_ptr<AnimationGroup> group,
		const Point* attachment, int xOffset, int yOffset, int height,
		float delay);

	static std::unique_ptr<Particle> dustexplosion(
		std::shared_ptr<AnimationGroup> group,
		const Point* attachment, int xOffset, int yOffset, int height,
		float delay);

	static std::unique_ptr<Particle> flame(
		std::shared_ptr<AnimationGroup> group,
		const Point* attachment, int xOffset, int yOffset, int height,
		float delay);

	static std::unique_ptr<Particle> smokeflame(
		std::shared_ptr<AnimationGroup> group,
		const Point* attachment, int xOffset, int yOffset, int height,
		float delay);

	static std::unique_ptr<Particle> dustflame(
		std::shared_ptr<AnimationGroup> group,
		const Point* attachment, int xOffset, int yOffset, int height,
		float delay);
	static std::unique_ptr<Particle> dustflame(
		std::shared_ptr<AnimationGroup> group,
		const Point& placement,
		float delay = 0);

	static std::unique_ptr<Particle> frostflame(
		std::shared_ptr<AnimationGroup> group,
		const Point* attachment, int xOffset, int yOffset, int height,
		float delay);

	static std::unique_ptr<Particle> deathflame(
		std::shared_ptr<AnimationGroup> group,
		const Point* attachment, int xOffset, int yOffset, int height,
		float delay);

	static std::unique_ptr<Particle> gasburst(
		std::shared_ptr<AnimationGroup> group,
		const Point* attachment, int xOffset, int yOffset, int height,
		float delay);

	static std::unique_ptr<Particle> ember(
		std::shared_ptr<AnimationGroup> group,
		const Point& placement,
		float delay = 0);

	static std::unique_ptr<Particle> statik(
		std::shared_ptr<AnimationGroup> group,
		const Point& placement,
		float delay = 0);

	static std::unique_ptr<Particle> frost(
		std::shared_ptr<AnimationGroup> group,
		const Point& placement,
		float delay = 0);

	static std::unique_ptr<Particle> death(
		std::shared_ptr<AnimationGroup> group,
		const Point& placement,
		float delay = 0);

	static std::unique_ptr<Particle> gascloud(
		std::shared_ptr<AnimationGroup> group,
		const Point& placement,
		float delay = 0);

	static std::unique_ptr<Particle> gascloudDark(
		std::shared_ptr<AnimationGroup> group,
		const Point& placement,
		float delay = 0);

	static std::unique_ptr<Particle> raindrop(
		std::shared_ptr<AnimationGroup> group,
		const Point& placement,
		float delay = 0);

	static std::unique_ptr<Particle> raindropWeak(
		std::shared_ptr<AnimationGroup> group,
		const Point& placement,
		float delay = 0);

	static std::unique_ptr<Particle> raindropHeavy(
		std::shared_ptr<AnimationGroup> group,
		const Point& placement,
		float delay = 0);

	static std::unique_ptr<Particle> hailstone(
		std::shared_ptr<AnimationGroup> group,
		const Point& placement,
		float delay = 0);

	static std::unique_ptr<Particle> snowflake(
		std::shared_ptr<AnimationGroup> group,
		const Point& placement,
		float delay = 0);

	static std::unique_ptr<Particle> grassflake(
		std::shared_ptr<AnimationGroup> group,
		const Point& placement,
		const Color& color,
		float delay = 0);

	static std::unique_ptr<Particle> barkflake(
		std::shared_ptr<AnimationGroup> group,
		const Point* attachment, int xOffset, int yOffset, int height,
		float delay);

	static std::unique_ptr<Particle> rubble(
		std::shared_ptr<AnimationGroup> group,
		const Point* attachment, int xOffset, int yOffset, int height,
		float delay);

	static std::unique_ptr<Particle> dustcloud(
		std::shared_ptr<AnimationGroup> group,
		const Point* attachment, int xOffset, int yOffset, int height,
		float delay);

	static std::unique_ptr<Particle> footprint(
		std::shared_ptr<AnimationGroup> group,
		const Point* attachment, int xOffset, int yOffset, int height,
		float delay);

	static std::unique_ptr<Particle> flak(
		std::shared_ptr<AnimationGroup> group,
		const Point* attachment, int xOffset, int yOffset, int height,
		float delay);
	static std::unique_ptr<Particle> flak(
		std::shared_ptr<AnimationGroup> group,
		const Point& placement, int height,
		float delay = 0);

	static std::unique_ptr<Particle> burp(
		std::shared_ptr<AnimationGroup> group,
		const Point* attachment, int xOffset, int yOffset, int height,
		float delay);

	static std::unique_ptr<Particle> income(
		std::shared_ptr<AnimationGroup> group,
		const Point* attachment, int xOffset, int yOffset, int height,
		float delay);
	static std::unique_ptr<Particle> income(
		std::shared_ptr<AnimationGroup> group,
		const Point& placement, int height,
		float delay = 0);

	static std::unique_ptr<Particle> diamondup(
		std::shared_ptr<AnimationGroup> group,
		const Point* attachment, int xOffset, int yOffset, int height,
		float delay);
	static std::unique_ptr<Particle> diamondup(
		std::shared_ptr<AnimationGroup> group,
		const Point& placement, int height,
		float delay = 0);

	static std::unique_ptr<Particle> diamonddown(
		std::shared_ptr<AnimationGroup> group,
		const Point* attachment, int xOffset, int yOffset, int height,
		float delay);
	static std::unique_ptr<Particle> diamonddown(
		std::shared_ptr<AnimationGroup> group,
		const Point& placement, int height,
		float delay = 0);

	static std::unique_ptr<Particle> heartup(
		std::shared_ptr<AnimationGroup> group,
		const Point* attachment, int xOffset, int yOffset, int height,
		float delay);
	static std::unique_ptr<Particle> heartup(
		std::shared_ptr<AnimationGroup> group,
		const Point& placement, int height,
		float delay = 0);

	static std::unique_ptr<Particle> heartdown(
		std::shared_ptr<AnimationGroup> group,
		const Point* attachment, int xOffset, int yOffset, int height,
		float delay);
	static std::unique_ptr<Particle> heartdown(
		std::shared_ptr<AnimationGroup> group,
		const Point& placement, int height,
		float delay = 0);

	static std::unique_ptr<Particle> workup(
		std::shared_ptr<AnimationGroup> group,
		const Point* attachment, int xOffset, int yOffset, int height,
		float delay);

	static std::unique_ptr<Particle> workdown(
		std::shared_ptr<AnimationGroup> group,
		const Point* attachment, int xOffset, int yOffset, int height,
		float delay);
	static std::unique_ptr<Particle> workdown(
		std::shared_ptr<AnimationGroup> group,
		const Point& placement, int height,
		float delay = 0);

	static std::unique_ptr<Particle> boltup(
		std::shared_ptr<AnimationGroup> group,
		const Point* attachment, int xOffset, int yOffset, int height,
		float delay);

	static std::unique_ptr<Particle> boltdown(
		std::shared_ptr<AnimationGroup> group,
		const Point* attachment, int xOffset, int yOffset, int height,
		float delay);

	static std::unique_ptr<Particle> exclamation(
		std::shared_ptr<AnimationGroup> group,
		const Point* attachment, int xOffset, int yOffset, int height,
		float delay);

	static std::unique_ptr<Particle> question(
		std::shared_ptr<AnimationGroup> group,
		const Point* attachment, int xOffset, int yOffset, int height,
		float delay);

	static std::unique_ptr<Particle> blocked(
		std::shared_ptr<AnimationGroup> group,
		const Point& placement, int height,
		float delay = 0);

	static std::unique_ptr<Particle> nocoin(
		std::shared_ptr<AnimationGroup> group,
		const Point& placement, int height,
		float delay = 0);

	static std::unique_ptr<Particle> moon(
		std::shared_ptr<AnimationGroup> group,
		const Point& placement, int height,
		float delay = 0);
};
