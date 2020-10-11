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
#include "particle.hpp"
#include "source.hpp"

#include "animation.hpp"
#include "sprite.hpp"
#include "point.hpp"
#include "collector.hpp"
#include "paint.hpp"
#include "colorname.hpp"


enum class Particle::State : uint8_t { UNSET, SET, DYING, DEAD };

Particle::Particle(std::shared_ptr<AnimationGroup> group,
		std::shared_ptr<Sprite> sprite,
		const Point* attachment, int xOffset, int yOffset, int height,
		float delay) :
	_state(State::UNSET),
	_yahooOffset(yOffset),
	_animation(new Animation(group, callback(attachment), 10, delay)),
	_sprite(sprite)
{
	_sprite->setOffset(xOffset, height);
	_sprite->setFinal(true);
}

Particle::Particle(std::shared_ptr<AnimationGroup> group,
		std::shared_ptr<Sprite> sprite,
		const Point& placement, int height,
		float delay) :
	_state(State::SET),
	_point(placement),
	_yahooOffset(0),
	_animation(new Animation(group, callback(), 10, delay)),
	_sprite(sprite)
{
	_sprite->setOffset(0, height);
	_sprite->setFinal(true);
}

Particle::~Particle() = default;

std::function<void(float)> Particle::callback(const Point* attachment)
{
	return [this, attachment](float) {

		if (_state == State::UNSET && attachment)
		{
			_state = State::SET;
			_point.xenon = attachment->xenon;
			_point.yahoo = attachment->yahoo + _yahooOffset;
		}

		_sprite->update();
		Collector::get()->addParticle(_sprite, _point);
	};
}

std::function<void(float)> Particle::callback()
{
	return [this](float) {

		_sprite->update();
		Collector::get()->addParticle(_sprite, _point);
	};
}

void Particle::update()
{
	switch (_state)
	{
		case State::UNSET:
		case State::SET:
		{
			if (!_animation) return;
			_animation->update();
			if (!_sprite->isVisible())
			{
				_state = State::DYING;
			}
		}
		break;

		case State::DYING:
		{
			_animation.reset();
			_sprite.reset();
			_state = State::DEAD;
		}
		break;

		case State::DEAD:
		break;
	}
}

static const char* randomblood()
{
	return "effects/hitmarker";
}

static const char* randomspark()
{
	return "effects/hitmarker";
}

static const char* randomspray()
{
	return "effects/dirtspray1";
}

static const char* randomboom()
{
	return "effects/dirtexplosion1";
}

static const char* randomexplosion()
{
	return "effects/explosion1";
}

static const char* randomsmokeexplosion()
{
	return "effects/dustexplosion1";
}

static const char* randomdustexplosion()
{
	return "effects/dustexplosion1";
}

static const char* randomflame()
{
	return "effects/flame1";
}

static const char* randomsmokeflame()
{
	switch (rand() % 4)
	{
		case 0:  return "effects/dustflame1";
		case 1:  return "effects/dustflame2";
		case 2:  return "effects/dustflame3";
		default: return "effects/dustflame4";
	}
}

static const char* randomdustflame()
{
	switch (rand() % 2)
	{
		case 0:  return "effects/dustburst1";
		default: return "effects/dustburst2";
	}
}

static const char* randomgasburst()
{
	switch (rand() % 4)
	{
		case 0:  return "effects/dustburst1";
		default: return "effects/dustburst2";
	}
}

static const char* randomember()
{
	switch (rand() % 2)
	{
		case 0:  return "effects/ember1";
		default: return "effects/ember2";
	}
}

static const char* randomstatik()
{
	switch (rand() % 2)
	{
		case 0:  return "effects/static1";
		default: return "effects/static2";
	}
}

static const char* randomfrost()
{
	return "effects/frost1_rising";
}

static const char* randomdeath()
{
	return "effects/skull3_rising";
}

static const char* randomgascloud()
{
	switch (rand() % 2)
	{
		case 0:  return "effects/gascloud1";
		default: return "effects/gascloud2";
	}
}

static const char* randomrubble()
{
	switch (rand() % 2)
	{
		case 0:  return "effects/gascloud1";
		default: return "effects/gascloud2";
	}
}

static const char* randombarkflake()
{
	return "effects/snowflake1";
}

static const char* randomgrassflake()
{
	switch (rand() % 2)
	{
		case 0:  return "effects/grassblade1";
		default: return "effects/grassblade2";
	}
}

static const char* randomraindropWeak()
{
	switch (rand() % 4)
	{
		case 0:  return "effects/raindrop1";
		case 1:  return "effects/raindrop1";
		case 2:  return "effects/raindrop2";
		default: return "effects/rainimpact";
	}
}

static const char* randomraindrop()
{
	switch (rand() % 4)
	{
		case 0:  return "effects/raindrop2";
		case 1:  return "effects/raindrop3";
		default: return "effects/rainimpact";
	}
}

static const char* randomraindropHeavy()
{
	switch (rand() % 2)
	{
		case 0:  return "effects/raindrop3";
		default: return "effects/rainimpact";
	}
}

static const char* randomhailstone()
{
	switch (rand() % 2)
	{
		case 0:  return "effects/raindrop1";
		default: return "effects/raindrop2";
	}
}

static const char* randomsnowflake()
{
	return "effects/snowflake1";
}

static const char* randomdustcloud()
{
	return "effects/dustcloud1";
}

static const char* randomfootprint()
{
	return "effects/footprint1";
}

static const char* randomflak()
{
	return "effects/flak1";
}

static const char* randomburp()
{
	return "effects/bubble1";
}

static const char* randomincome()
{
	return "effects/coin2_rising";
}

static const char* randomdiamondup()
{
	return "effects/diamond1_rising";
}

static const char* randomdiamonddown()
{
	return "effects/diamond1_broken";
}

static const char* randomheartup()
{
	return "effects/heart1_rising";
}

static const char* randomheartdown()
{
	return "effects/heart1_broken";
}

static const char* randomworkup()
{
	return "effects/work1_rising";
}

static const char* randomworkdown()
{
	return "effects/work1_broken";
}

static const char* randomboltup()
{
	return "effects/bolt1_rising";
}

static const char* randomboltdown()
{
	return "effects/bolt1_broken";
}

static const char* randomblocked()
{
	return "effects/blocked2";
}

static const char* randomnocoin()
{
	return "effects/nocoin2";
}

static const char* randommoon()
{
	return "effects/moon1";
}

static const char* randomexclamation()
{
	return "effects/exclamation1_rising";
}

static const char* randomquestion()
{
	return "effects/question1_rising";
}

std::unique_ptr<Particle> Particle::blood(
	std::shared_ptr<AnimationGroup> group,
	const Point* attachment, int xOffset, int yOffset, int height,
	float delay)
{
	static std::shared_ptr<Sprite::Palette> palette(new Sprite::Palette({
		Paint(ColorName::BLOOD),
		Paint(ColorName::BLOODDARK),
	}));

	auto sprite = std::make_shared<Sprite>(randomblood(), palette);
	sprite->setOriginAtCenter();
	return std::unique_ptr<Particle>(new Particle(group,
		sprite,
		attachment, xOffset, yOffset, height,
		delay));
}

std::unique_ptr<Particle> Particle::spark(
	std::shared_ptr<AnimationGroup> group,
	const Point* attachment, int xOffset, int yOffset, int height,
	float delay)
{
	static std::shared_ptr<Sprite::Palette> palette(new Sprite::Palette({
		Paint(ColorName::SPARK),
		Paint(ColorName::SPARKDARK),
	}));

	auto sprite = std::make_shared<Sprite>(randomspark(), palette);
	sprite->setOriginAtCenter();
	return std::unique_ptr<Particle>(new Particle(group,
		sprite,
		attachment, xOffset, yOffset, height,
		delay));
}

std::unique_ptr<Particle> Particle::woodchip(
	std::shared_ptr<AnimationGroup> group,
	const Point* attachment, int xOffset, int yOffset, int height,
	float delay)
{
	static std::shared_ptr<Sprite::Palette> palette(new Sprite::Palette({
		Paint(ColorName::BARK),
		Paint(ColorName::BARKDARK),
	}));

	auto sprite = std::make_shared<Sprite>(randomspark(), palette);
	sprite->setOriginAtCenter();
	return std::unique_ptr<Particle>(new Particle(group,
		sprite,
		attachment, xOffset, yOffset, height,
		delay));
}

std::unique_ptr<Particle> Particle::bloodspray(
	std::shared_ptr<AnimationGroup> group,
	const Point* attachment, int xOffset, int yOffset, int height,
	float delay)
{
	static std::shared_ptr<Sprite::Palette> palette(new Sprite::Palette({
		Paint(ColorName::BLOODDARK),
		Paint(ColorName::UNITSHADOW),
	}));

	auto sprite = std::make_shared<Sprite>(randomspray(), palette);
	sprite->setOriginAtBase();
	return std::unique_ptr<Particle>(new Particle(group,
		sprite,
		attachment, xOffset, yOffset, height,
		delay));
}

std::unique_ptr<Particle> Particle::bloodboom(
	std::shared_ptr<AnimationGroup> group,
	const Point* attachment, int xOffset, int yOffset, int height,
	float delay)
{
	static std::shared_ptr<Sprite::Palette> palette(new Sprite::Palette({
		Paint(ColorName::BLOODDARK),
		Paint(ColorName::UNITSHADOW),
		Paint(ColorName::DUSTDARK),
		Paint(ColorName::DUST),
		Paint(ColorName::SPARK),
	}));

	auto sprite = std::make_shared<Sprite>(randomboom(), palette);
	sprite->setOriginAtBase();
	return std::unique_ptr<Particle>(new Particle(group,
		sprite,
		attachment, xOffset, yOffset, height,
		delay));
}

std::unique_ptr<Particle> Particle::sparkboom(
	std::shared_ptr<AnimationGroup> group,
	const Point* attachment, int xOffset, int yOffset, int height,
	float delay)
{
	static std::shared_ptr<Sprite::Palette> palette(new Sprite::Palette({
		Paint(ColorName::SPARKDARK),
		Paint(ColorName::UNITSHADOW),
		Paint(ColorName::DUSTDARK),
		Paint(ColorName::DUST),
		Paint(ColorName::SPARK),
	}));

	auto sprite = std::make_shared<Sprite>(randomboom(), palette);
	sprite->setOriginAtBase();
	return std::unique_ptr<Particle>(new Particle(group,
		sprite,
		attachment, xOffset, yOffset, height,
		delay));
}

std::unique_ptr<Particle> Particle::woodboom(
	std::shared_ptr<AnimationGroup> group,
	const Point* attachment, int xOffset, int yOffset, int height,
	float delay)
{
	static std::shared_ptr<Sprite::Palette> palette(new Sprite::Palette({
		Paint(ColorName::BARKDARK),
		Paint(ColorName::UNITSHADOW),
		Paint(ColorName::DUSTDARK),
		Paint(ColorName::DUST),
		Paint(ColorName::SPARK),
	}));

	auto sprite = std::make_shared<Sprite>(randomboom(), palette);
	sprite->setOriginAtBase();
	return std::unique_ptr<Particle>(new Particle(group,
		sprite,
		attachment, xOffset, yOffset, height,
		delay));
}

std::unique_ptr<Particle> Particle::spray(
	std::shared_ptr<AnimationGroup> group,
	const Point* attachment, int xOffset, int yOffset, int height,
	const Color& color,
	float delay)
{
	/*local*/ std::shared_ptr<Sprite::Palette> palette(new Sprite::Palette({
		Color::mix(color, Paint::alpha(ColorName::SHADEBLEND, 255), 0.5f),
		Paint(ColorName::UNITSHADOW),
	}));

	auto sprite = std::make_shared<Sprite>(randomspray(), palette);
	sprite->setOriginAtBase();
	return std::unique_ptr<Particle>(new Particle(group,
		sprite,
		attachment, xOffset, yOffset, height,
		delay));
}

std::unique_ptr<Particle> Particle::spray(
	std::shared_ptr<AnimationGroup> group,
	const Point& placement,
	const Color& color,
	float delay)
{
	/*local*/ std::shared_ptr<Sprite::Palette> palette(new Sprite::Palette({
		Color::mix(color, Paint::alpha(ColorName::SHADEBLEND, 255), 0.5f),
		Paint(ColorName::UNITSHADOW),
	}));

	auto sprite = std::make_shared<Sprite>(randomspray(), palette);
	sprite->setOriginAtBase();
	return std::unique_ptr<Particle>(new Particle(group,
		sprite,
		placement, 0,
		delay));
}

std::unique_ptr<Particle> Particle::boom(
	std::shared_ptr<AnimationGroup> group,
	const Point* attachment, int xOffset, int yOffset, int height,
	const Color& color,
	float delay)
{
	/*local*/ std::shared_ptr<Sprite::Palette> palette(new Sprite::Palette({
		Color::mix(color, Paint::alpha(ColorName::SHADEBLEND, 255), 0.5f),
		Paint(ColorName::UNITSHADOW),
		Paint(ColorName::DUSTDARK),
		Paint(ColorName::DUST),
		Paint(ColorName::SPARK),
	}));

	auto sprite = std::make_shared<Sprite>(randomboom(), palette);
	sprite->setOriginAtBase();
	return std::unique_ptr<Particle>(new Particle(group,
		sprite,
		attachment, xOffset, yOffset, height,
		delay));
}

std::unique_ptr<Particle> Particle::boom(
	std::shared_ptr<AnimationGroup> group,
	const Point& placement,
	const Color& color,
	float delay)
{
	/*local*/ std::shared_ptr<Sprite::Palette> palette(new Sprite::Palette({
		Color::mix(color, Paint::alpha(ColorName::SHADEBLEND, 255), 0.5f),
		Paint(ColorName::UNITSHADOW),
		Paint(ColorName::DUSTDARK),
		Paint(ColorName::DUST),
		Paint(ColorName::SPARK),
	}));

	auto sprite = std::make_shared<Sprite>(randomboom(), palette);
	sprite->setOriginAtBase();
	return std::unique_ptr<Particle>(new Particle(group,
		sprite,
		placement, 0,
		delay));
}

std::unique_ptr<Particle> Particle::explosion(
	std::shared_ptr<AnimationGroup> group,
	const Point* attachment, int xOffset, int yOffset, int height,
	float delay)
{
	static std::shared_ptr<Sprite::Palette> palette(new Sprite::Palette({
		Paint(ColorName::FLASH),
		Paint(ColorName::FLAME),
		Paint(ColorName::FLAMEDARK),
		Paint(ColorName::DUSTDARK),
		Paint(ColorName::DUST),
	}));

	auto sprite = std::make_shared<Sprite>(randomexplosion(), palette);
	sprite->setOriginAtCenter();
	return std::unique_ptr<Particle>(new Particle(group,
		sprite,
		attachment, xOffset, yOffset, height,
		delay));
}

std::unique_ptr<Particle> Particle::smokeexplosion(
	std::shared_ptr<AnimationGroup> group,
	const Point* attachment, int xOffset, int yOffset, int height,
	float delay)
{
	static std::shared_ptr<Sprite::Palette> palette(new Sprite::Palette({
		Paint(ColorName::DUSTDARK),
		Paint(ColorName::DUST),
	}));

	auto sprite = std::make_shared<Sprite>(randomsmokeexplosion(), palette);
	sprite->setOriginAtCenter();
	return std::unique_ptr<Particle>(new Particle(group,
		sprite,
		attachment, xOffset, yOffset, height,
		delay));
}

std::unique_ptr<Particle> Particle::dustexplosion(
	std::shared_ptr<AnimationGroup> group,
	const Point* attachment, int xOffset, int yOffset, int height,
	float delay)
{
	static std::shared_ptr<Sprite::Palette> palette(new Sprite::Palette({
		Paint(ColorName::DUST),
		Paint(ColorName::DUSTLIGHT),
	}));

	auto sprite = std::make_shared<Sprite>(randomdustexplosion(), palette);
	sprite->setOriginAtCenter();
	return std::unique_ptr<Particle>(new Particle(group,
		sprite,
		attachment, xOffset, yOffset, height,
		delay));
}

std::unique_ptr<Particle> Particle::flame(
	std::shared_ptr<AnimationGroup> group,
	const Point* attachment, int xOffset, int yOffset, int height,
	float delay)
{
	static std::shared_ptr<Sprite::Palette> palette(new Sprite::Palette({
		Paint(ColorName::FLAME),
		Paint(ColorName::FLAMEDARK),
		Paint(ColorName::DUSTDARK),
		Paint(ColorName::DUST),
	}));

	auto sprite = std::make_shared<Sprite>(randomflame(), palette);
	sprite->setOriginAtBase();
	return std::unique_ptr<Particle>(new Particle(group,
		sprite,
		attachment, xOffset, yOffset, height,
		delay));
}

std::unique_ptr<Particle> Particle::smokeflame(
	std::shared_ptr<AnimationGroup> group,
	const Point* attachment, int xOffset, int yOffset, int height,
	float delay)
{
	static std::shared_ptr<Sprite::Palette> palette(new Sprite::Palette({
		Paint(ColorName::DUSTDARK),
		Paint(ColorName::DUST),
	}));

	auto sprite = std::make_shared<Sprite>(randomsmokeflame(), palette);
	sprite->setOriginAtBase();
	return std::unique_ptr<Particle>(new Particle(group,
		sprite,
		attachment, xOffset, yOffset, height,
		delay));
}

std::unique_ptr<Particle> Particle::dustflame(
	std::shared_ptr<AnimationGroup> group,
	const Point* attachment, int xOffset, int yOffset, int height,
	float delay)
{
	static std::shared_ptr<Sprite::Palette> palette(new Sprite::Palette({
		Paint(ColorName::DUST),
		Paint(ColorName::DUSTLIGHT),
	}));

	auto sprite = std::make_shared<Sprite>(randomdustflame(), palette);
	sprite->setOriginAtBase();
	return std::unique_ptr<Particle>(new Particle(group,
		sprite,
		attachment, xOffset, yOffset, height,
		delay));
}

std::unique_ptr<Particle> Particle::dustflame(
	std::shared_ptr<AnimationGroup> group,
	const Point& placement,
	float delay)
{
	static std::shared_ptr<Sprite::Palette> palette(new Sprite::Palette({
		Paint(ColorName::DUST),
		Paint(ColorName::DUSTLIGHT),
	}));

	auto sprite = std::make_shared<Sprite>(randomdustflame(), palette);
	sprite->setOriginAtBase();
	return std::unique_ptr<Particle>(new Particle(group,
		sprite,
		placement, 0,
		delay));
}

std::unique_ptr<Particle> Particle::frostflame(
	std::shared_ptr<AnimationGroup> group,
	const Point* attachment, int xOffset, int yOffset, int height,
	float delay)
{
	static std::shared_ptr<Sprite::Palette> palette(new Sprite::Palette({
		Paint::blend(ColorName::FROST, ColorName::SHINEBLEND),
		Paint(ColorName::FROST),
		Paint::blend(ColorName::FROST, ColorName::SHADEBLEND),
	}));

	auto sprite = std::make_shared<Sprite>(randomfrost(), palette);
	sprite->setOriginAtBase();
	return std::unique_ptr<Particle>(new Particle(group,
		sprite,
		attachment, xOffset, yOffset, height,
		delay));
}

std::unique_ptr<Particle> Particle::deathflame(
	std::shared_ptr<AnimationGroup> group,
	const Point* attachment, int xOffset, int yOffset, int height,
	float delay)
{
	static std::shared_ptr<Sprite::Palette> palette(new Sprite::Palette({
		Paint(ColorName::SKULL),
		Paint(ColorName::SKULLDARK),
		Paint(ColorName::DUST),
		Paint(ColorName::DUSTDARK),
	}));

	auto sprite = std::make_shared<Sprite>(randomdeath(), palette);
	sprite->setOriginAtBase();
	return std::unique_ptr<Particle>(new Particle(group,
		sprite,
		attachment, xOffset, yOffset, height,
		delay));
}

std::unique_ptr<Particle> Particle::gasburst(
	std::shared_ptr<AnimationGroup> group,
	const Point* attachment, int xOffset, int yOffset, int height,
	float delay)
{
	static std::shared_ptr<Sprite::Palette> palette(new Sprite::Palette({
		Paint(ColorName::GAS),
		Paint(ColorName::GASDARK),
	}));

	auto sprite = std::make_shared<Sprite>(randomgasburst(), palette);
	sprite->setOriginAtBase();
	return std::unique_ptr<Particle>(new Particle(group,
		sprite,
		attachment, xOffset, yOffset, height,
		delay));
}

std::unique_ptr<Particle> Particle::ember(
	std::shared_ptr<AnimationGroup> group,
	const Point& placement,
	float delay)
{
	static std::shared_ptr<Sprite::Palette> palette(new Sprite::Palette({
		Paint(ColorName::FLAME),
		Paint(ColorName::FLAMEDARK),
		Paint(ColorName::DUST),
	}));

	auto sprite = std::make_shared<Sprite>(randomember(), palette);
	sprite->setOriginAtBase();
	return std::unique_ptr<Particle>(new Particle(group,
		sprite,
		placement, 0,
		delay));
}

std::unique_ptr<Particle> Particle::statik(
	std::shared_ptr<AnimationGroup> group,
	const Point& placement,
	float delay)
{
	static std::shared_ptr<Sprite::Palette> palette(new Sprite::Palette({
		Paint(ColorName::SPARK),
		Paint(ColorName::DUST),
	}));

	auto sprite = std::make_shared<Sprite>(randomstatik(), palette);
	sprite->setOriginAtBase();
	return std::unique_ptr<Particle>(new Particle(group,
		sprite,
		placement, 0,
		delay));
}

std::unique_ptr<Particle> Particle::frost(
	std::shared_ptr<AnimationGroup> group,
	const Point& placement,
	float delay)
{
	static std::shared_ptr<Sprite::Palette> palette(new Sprite::Palette({
		Paint::blend(ColorName::FROST, ColorName::SHINEBLEND),
		Paint(ColorName::FROST),
		Paint::blend(ColorName::FROST, ColorName::SHADEBLEND),
	}));

	auto sprite = std::make_shared<Sprite>(randomfrost(), palette);
	sprite->setOriginAtBase();
	return std::unique_ptr<Particle>(new Particle(group,
		sprite,
		placement, 0,
		delay));
}

std::unique_ptr<Particle> Particle::death(
	std::shared_ptr<AnimationGroup> group,
	const Point& placement,
	float delay)
{
	static std::shared_ptr<Sprite::Palette> palette(new Sprite::Palette({
		Paint(ColorName::SKULL),
		Paint(ColorName::SKULLDARK),
		Paint(ColorName::DUST),
		Paint(ColorName::DUSTDARK),
	}));

	auto sprite = std::make_shared<Sprite>(randomdeath(), palette);
	sprite->setOriginAtBase();
	return std::unique_ptr<Particle>(new Particle(group,
		sprite,
		placement, 0,
		delay));
}

std::unique_ptr<Particle> Particle::gascloud(
	std::shared_ptr<AnimationGroup> group,
	const Point& placement,
	float delay)
{
	static std::shared_ptr<Sprite::Palette> palette(new Sprite::Palette({
		Paint::alpha(ColorName::GAS, 150),
		Paint::alpha(ColorName::GASDARK, 150),
		Paint::alpha(ColorName::DUST, 150),
	}));

	auto sprite = std::make_shared<Sprite>(randomgascloud(), palette);
	sprite->setOriginAtBase();
	return std::unique_ptr<Particle>(new Particle(group,
		sprite,
		placement, 0,
		delay));
}

std::unique_ptr<Particle> Particle::gascloudDark(
	std::shared_ptr<AnimationGroup> group,
	const Point& placement,
	float delay)
{
	static std::shared_ptr<Sprite::Palette> palette(new Sprite::Palette({
		Paint::alpha(ColorName::GAS, 50),
		Paint::alpha(ColorName::GASDARK, 50),
		Paint::alpha(ColorName::DUST, 50),
	}));

	auto sprite = std::make_shared<Sprite>(randomgascloud(), palette);
	sprite->setOriginAtBase();
	return std::unique_ptr<Particle>(new Particle(group,
		sprite,
		placement, 0,
		delay));
}

std::unique_ptr<Particle> Particle::raindrop(
	std::shared_ptr<AnimationGroup> group,
	const Point& placement,
	float delay)
{
	static std::shared_ptr<Sprite::Palette> palette(new Sprite::Palette({
		Paint::alpha(ColorName::RAIN, 80),
		Paint::alpha(ColorName::RAINDARK, 180),
	}));

	auto sprite = std::make_shared<Sprite>(randomraindrop(), palette);
	sprite->setOriginAtBase();
	return std::unique_ptr<Particle>(new Particle(group,
		sprite,
		placement, 0,
		delay));
}

std::unique_ptr<Particle> Particle::raindropWeak(
	std::shared_ptr<AnimationGroup> group,
	const Point& placement,
	float delay)
{
	static std::shared_ptr<Sprite::Palette> palette(new Sprite::Palette({
		Paint::alpha(ColorName::RAIN, 80),
		Paint::alpha(ColorName::RAINDARK, 180),
	}));

	auto sprite = std::make_shared<Sprite>(randomraindropWeak(), palette);
	sprite->setOriginAtBase();
	return std::unique_ptr<Particle>(new Particle(group,
		sprite,
		placement, 0,
		delay));
}

std::unique_ptr<Particle> Particle::raindropHeavy(
	std::shared_ptr<AnimationGroup> group,
	const Point& placement,
	float delay)
{
	static std::shared_ptr<Sprite::Palette> palette(new Sprite::Palette({
		Paint::alpha(ColorName::RAIN, 80),
		Paint::alpha(ColorName::RAINDARK, 180),
	}));

	auto sprite = std::make_shared<Sprite>(randomraindropHeavy(), palette);
	sprite->setOriginAtBase();
	return std::unique_ptr<Particle>(new Particle(group,
		sprite,
		placement, 0,
		delay));
}

std::unique_ptr<Particle> Particle::hailstone(
	std::shared_ptr<AnimationGroup> group,
	const Point& placement,
	float delay)
{
	static std::shared_ptr<Sprite::Palette> palette(new Sprite::Palette({
		Paint::alpha(ColorName::HAIL, 150),
		Paint::alpha(ColorName::HAILDARK, 150),
	}));

	auto sprite = std::make_shared<Sprite>(randomhailstone(), palette);
	sprite->setOriginAtBase();
	return std::unique_ptr<Particle>(new Particle(group,
		sprite,
		placement, 0,
		delay));
}

std::unique_ptr<Particle> Particle::snowflake(
	std::shared_ptr<AnimationGroup> group,
	const Point& placement,
	float delay)
{
	static std::shared_ptr<Sprite::Palette> palette(new Sprite::Palette({
		Paint::alpha(ColorName::SNOWFALL, 200),
	}));

	auto sprite = std::make_shared<Sprite>(randomsnowflake(), palette);
	sprite->setOriginAtBase();
	return std::unique_ptr<Particle>(new Particle(group,
		sprite,
		placement, 0,
		delay));
}

std::unique_ptr<Particle> Particle::grassflake(
	std::shared_ptr<AnimationGroup> group,
	const Point& placement,
	const Color& color,
	float delay)
{
	/*local*/ std::shared_ptr<Sprite::Palette> palette(new Sprite::Palette({
		color,
	}));

	auto sprite = std::make_shared<Sprite>(randomgrassflake(), palette);
	sprite->setOriginAtBase();
	return std::unique_ptr<Particle>(new Particle(group,
		sprite,
		placement, 0,
		delay));
}

std::unique_ptr<Particle> Particle::barkflake(
	std::shared_ptr<AnimationGroup> group,
	const Point* attachment, int xOffset, int yOffset, int height,
	float delay)
{
	static std::shared_ptr<Sprite::Palette> palette(new Sprite::Palette({
		Paint(ColorName::BARKDARK),
	}));

	auto sprite = std::make_shared<Sprite>(randombarkflake(), palette);
	sprite->setOriginAtBase();
	return std::unique_ptr<Particle>(new Particle(group,
		sprite,
		attachment, xOffset, yOffset, height,
		delay));
}

std::unique_ptr<Particle> Particle::rubble(
	std::shared_ptr<AnimationGroup> group,
	const Point* attachment, int xOffset, int yOffset, int height,
	float delay)
{
	static std::shared_ptr<Sprite::Palette> palette(new Sprite::Palette({
		Paint(ColorName::RUBBLE),
		Paint(ColorName::DUSTDARK),
		Paint(ColorName::DUST),
	}));

	auto sprite = std::make_shared<Sprite>(randomrubble(), palette);
	sprite->setOriginAtBase();
	return std::unique_ptr<Particle>(new Particle(group,
		sprite,
		attachment, xOffset, yOffset, height,
		delay));
}

std::unique_ptr<Particle> Particle::dustcloud(
	std::shared_ptr<AnimationGroup> group,
	const Point* attachment, int xOffset, int yOffset, int height,
	float delay)
{
	static std::shared_ptr<Sprite::Palette> palette(new Sprite::Palette({
		Paint(ColorName::DUSTDARK),
		Paint(ColorName::SOOT),
	}));

	auto sprite = std::make_shared<Sprite>(randomdustcloud(), palette);
	sprite->setOriginAtBase();
	return std::unique_ptr<Particle>(new Particle(group,
		sprite,
		attachment, xOffset, yOffset, height,
		delay));
}

std::unique_ptr<Particle> Particle::footprint(
	std::shared_ptr<AnimationGroup> group,
	const Point* attachment, int xOffset, int yOffset, int height,
	float delay)
{
	static std::shared_ptr<Sprite::Palette> palette(new Sprite::Palette({
		Paint(ColorName::UIPRIMARY),
		Paint(ColorName::UIPRIMARY),
	}));

	auto sprite = std::make_shared<Sprite>(randomfootprint(), palette);
	sprite->setOriginAtBase();
	return std::unique_ptr<Particle>(new Particle(group,
		sprite,
		attachment, xOffset, yOffset, height,
		delay));
}

std::unique_ptr<Particle> Particle::flak(
	std::shared_ptr<AnimationGroup> group,
	const Point* attachment, int xOffset, int yOffset, int height,
	float delay)
{
	static std::shared_ptr<Sprite::Palette> palette(new Sprite::Palette({
		Paint(ColorName::DUSTDARK),
		Paint(ColorName::SOOT),
	}));

	auto sprite = std::make_shared<Sprite>(randomflak(), palette);
	sprite->setOriginAtCenter();
	return std::unique_ptr<Particle>(new Particle(group,
		sprite,
		attachment, xOffset, yOffset, height,
		delay));
}

std::unique_ptr<Particle> Particle::flak(
	std::shared_ptr<AnimationGroup> group,
	const Point& placement, int height,
	float delay)
{
	static std::shared_ptr<Sprite::Palette> palette(new Sprite::Palette({
		Paint(ColorName::DUSTDARK),
		Paint(ColorName::SOOT),
	}));

	auto sprite = std::make_shared<Sprite>(randomflak(), palette);
	sprite->setOriginAtCenter();
	return std::unique_ptr<Particle>(new Particle(group,
		sprite,
		placement, height,
		delay));
}

std::unique_ptr<Particle> Particle::burp(
	std::shared_ptr<AnimationGroup> group,
	const Point* attachment, int xOffset, int yOffset, int height,
	float delay)
{
	static std::shared_ptr<Sprite::Palette> palette(new Sprite::Palette({
		Paint(ColorName::GAS),
		Paint::blend(ColorName::GAS, ColorName::SHINEBLEND),
	}));

	auto sprite = std::make_shared<Sprite>(randomburp(), palette);
	sprite->setOriginAtBase();
	return std::unique_ptr<Particle>(new Particle(group,
		sprite,
		attachment, xOffset, yOffset, height,
		delay));
}

std::unique_ptr<Particle> Particle::income(
	std::shared_ptr<AnimationGroup> group,
	const Point* attachment, int xOffset, int yOffset, int height,
	float delay)
{
	static std::shared_ptr<Sprite::Palette> palette(new Sprite::Palette({
		Paint::blend(ColorName::COIN, ColorName::SHINEBLEND),
		Paint(ColorName::COIN),
		Paint::blend(ColorName::COIN, ColorName::SHADEBLEND),
		Paint(ColorName::FLASH),
	}));

	auto sprite = std::make_shared<Sprite>(randomincome(), palette);
	sprite->setOriginAtBase();
	return std::unique_ptr<Particle>(new Particle(group,
		sprite,
		attachment, xOffset, yOffset, height,
		delay));
}

std::unique_ptr<Particle> Particle::diamondup(
	std::shared_ptr<AnimationGroup> group,
	const Point* attachment, int xOffset, int yOffset, int height,
	float delay)
{
	static std::shared_ptr<Sprite::Palette> palette(new Sprite::Palette({
		Paint::blend(ColorName::DIAMOND, ColorName::SHINEBLEND, 2.00f),
		Paint(ColorName::DIAMOND),
		Paint::blend(ColorName::DIAMOND, ColorName::SHADEBLEND, 2.00f),
		Paint(ColorName::FLASH),
	}));

	auto sprite = std::make_shared<Sprite>(randomdiamondup(), palette);
	sprite->setOriginAtBase();
	return std::unique_ptr<Particle>(new Particle(group,
		sprite,
		attachment, xOffset, yOffset, height,
		delay));
}

std::unique_ptr<Particle> Particle::diamonddown(
	std::shared_ptr<AnimationGroup> group,
	const Point* attachment, int xOffset, int yOffset, int height,
	float delay)
{
	static std::shared_ptr<Sprite::Palette> palette(new Sprite::Palette({
		Paint::blend(ColorName::DIAMOND, ColorName::SHINEBLEND),
		Paint(ColorName::DIAMOND),
		Paint::blend(ColorName::DIAMOND, ColorName::SHADEBLEND),
	}));

	auto sprite = std::make_shared<Sprite>(randomdiamonddown(), palette);
	sprite->setOriginAtBase();
	return std::unique_ptr<Particle>(new Particle(group,
		sprite,
		attachment, xOffset, yOffset, height,
		delay));
}

std::unique_ptr<Particle> Particle::heartup(
	std::shared_ptr<AnimationGroup> group,
	const Point* attachment, int xOffset, int yOffset, int height,
	float delay)
{
	static std::shared_ptr<Sprite::Palette> palette(new Sprite::Palette({
		Paint::blend(ColorName::HEART, ColorName::SHINEBLEND),
		Paint(ColorName::HEART),
		Paint::blend(ColorName::HEART, ColorName::SHADEBLEND),
	}));

	auto sprite = std::make_shared<Sprite>(randomheartup(), palette);
	sprite->setOriginAtBase();
	return std::unique_ptr<Particle>(new Particle(group,
		sprite,
		attachment, xOffset, yOffset, height,
		delay));
}

std::unique_ptr<Particle> Particle::heartdown(
	std::shared_ptr<AnimationGroup> group,
	const Point* attachment, int xOffset, int yOffset, int height,
	float delay)
{
	static std::shared_ptr<Sprite::Palette> palette(new Sprite::Palette({
		Paint::blend(ColorName::BROKEN, ColorName::SHINEBLEND),
		Paint(ColorName::BROKEN),
		Paint::blend(ColorName::BROKEN, ColorName::SHADEBLEND),
	}));

	auto sprite = std::make_shared<Sprite>(randomheartdown(), palette);
	sprite->setOriginAtBase();
	return std::unique_ptr<Particle>(new Particle(group,
		sprite,
		attachment, xOffset, yOffset, height,
		delay));
}

std::unique_ptr<Particle> Particle::workup(
	std::shared_ptr<AnimationGroup> group,
	const Point* attachment, int xOffset, int yOffset, int height,
	float delay)
{
	static std::shared_ptr<Sprite::Palette> palette(new Sprite::Palette({
		Paint::blend(ColorName::WORK, ColorName::SHINEBLEND),
		Paint(ColorName::WORK),
		Paint::blend(ColorName::WORK, ColorName::SHADEBLEND),
	}));

	auto sprite = std::make_shared<Sprite>(randomworkup(), palette);
	sprite->setOriginAtBase();
	return std::unique_ptr<Particle>(new Particle(group,
		sprite,
		attachment, xOffset, yOffset, height,
		delay));
}

std::unique_ptr<Particle> Particle::workdown(
	std::shared_ptr<AnimationGroup> group,
	const Point* attachment, int xOffset, int yOffset, int height,
	float delay)
{
	static std::shared_ptr<Sprite::Palette> palette(new Sprite::Palette({
		Paint::blend(ColorName::BROKEN, ColorName::SHINEBLEND),
		Paint(ColorName::BROKEN),
		Paint::blend(ColorName::BROKEN, ColorName::SHADEBLEND),
	}));

	auto sprite = std::make_shared<Sprite>(randomworkdown(), palette);
	sprite->setOriginAtBase();
	return std::unique_ptr<Particle>(new Particle(group,
		sprite,
		attachment, xOffset, yOffset, height,
		delay));
}

std::unique_ptr<Particle> Particle::boltup(
	std::shared_ptr<AnimationGroup> group,
	const Point* attachment, int xOffset, int yOffset, int height,
	float delay)
{
	static std::shared_ptr<Sprite::Palette> palette(new Sprite::Palette({
		Paint::blend(ColorName::ENERGY, ColorName::SHINEBLEND),
		Paint(ColorName::ENERGY),
		Paint::blend(ColorName::ENERGY, ColorName::SHADEBLEND),
	}));

	auto sprite = std::make_shared<Sprite>(randomboltup(), palette);
	sprite->setOriginAtBase();
	return std::unique_ptr<Particle>(new Particle(group,
		sprite,
		attachment, xOffset, yOffset, height,
		delay));
}

std::unique_ptr<Particle> Particle::boltdown(
	std::shared_ptr<AnimationGroup> group,
	const Point* attachment, int xOffset, int yOffset, int height,
	float delay)
{
	static std::shared_ptr<Sprite::Palette> palette(new Sprite::Palette({
		Paint::blend(ColorName::BROKEN, ColorName::SHINEBLEND),
		Paint(ColorName::BROKEN),
		Paint::blend(ColorName::BROKEN, ColorName::SHADEBLEND),
	}));

	auto sprite = std::make_shared<Sprite>(randomboltdown(), palette);
	sprite->setOriginAtBase();
	return std::unique_ptr<Particle>(new Particle(group,
		sprite,
		attachment, xOffset, yOffset, height,
		delay));
}

std::unique_ptr<Particle> Particle::exclamation(
	std::shared_ptr<AnimationGroup> group,
	const Point* attachment, int xOffset, int yOffset, int height,
	float delay)
{
	static std::shared_ptr<Sprite::Palette> palette(new Sprite::Palette({
		Paint::blend(ColorName::UIPRIMARY, ColorName::SHINEBLEND),
		Paint(ColorName::UIPRIMARY),
		Paint::blend(ColorName::UIPRIMARY, ColorName::SHADEBLEND),
	}));

	auto sprite = std::make_shared<Sprite>(randomexclamation(), palette);
	sprite->setOriginAtBase();
	return std::unique_ptr<Particle>(new Particle(group,
		sprite,
		attachment, xOffset, yOffset, height,
		delay));
}

std::unique_ptr<Particle> Particle::question(
	std::shared_ptr<AnimationGroup> group,
	const Point* attachment, int xOffset, int yOffset, int height,
	float delay)
{
	static std::shared_ptr<Sprite::Palette> palette(new Sprite::Palette({
		Paint::blend(ColorName::UIPRIMARY, ColorName::SHINEBLEND),
		Paint(ColorName::UIPRIMARY),
		Paint::blend(ColorName::UIPRIMARY, ColorName::SHADEBLEND),
		Color::transparent(),
	}));

	auto sprite = std::make_shared<Sprite>(randomquestion(), palette);
	sprite->setOriginAtBase();
	return std::unique_ptr<Particle>(new Particle(group,
		sprite,
		attachment, xOffset, yOffset, height,
		delay));
}

std::unique_ptr<Particle> Particle::blocked(
	std::shared_ptr<AnimationGroup> group,
	const Point& placement, int height,
	float delay)
{
	static std::shared_ptr<Sprite::Palette> palette(new Sprite::Palette({
		Paint::blend(ColorName::DENIED, ColorName::SHINEBLEND),
		Paint(ColorName::DENIED),
		Paint::blend(ColorName::DENIED, ColorName::SHADEBLEND),
	}));

	auto sprite = std::make_shared<Sprite>(randomblocked(), palette);
	sprite->setOriginAtBase();
	return std::unique_ptr<Particle>(new Particle(group,
		sprite,
		placement, height,
		delay));
}

std::unique_ptr<Particle> Particle::nocoin(
	std::shared_ptr<AnimationGroup> group,
	const Point& placement, int height,
	float delay)
{
	static std::shared_ptr<Sprite::Palette> palette(new Sprite::Palette({
		Paint::blend(ColorName::COIN, ColorName::SHINEBLEND),
		Paint(ColorName::COIN),
		Paint::blend(ColorName::COIN, ColorName::SHADEBLEND),
		Paint(ColorName::COIN),
	}));

	auto sprite = std::make_shared<Sprite>(randomnocoin(), palette);
	sprite->setOriginAtBase();
	return std::unique_ptr<Particle>(new Particle(group,
		sprite,
		placement, height,
		delay));
}

std::unique_ptr<Particle> Particle::heartup(
	std::shared_ptr<AnimationGroup> group,
	const Point& placement, int height,
	float delay)
{
	static std::shared_ptr<Sprite::Palette> palette(new Sprite::Palette({
		Paint::blend(ColorName::HEART, ColorName::SHINEBLEND),
		Paint(ColorName::HEART),
		Paint::blend(ColorName::HEART, ColorName::SHADEBLEND),
	}));

	auto sprite = std::make_shared<Sprite>(randomheartup(), palette);
	sprite->setOriginAtBase();
	return std::unique_ptr<Particle>(new Particle(group,
		sprite,
		placement, height,
		delay));
}

std::unique_ptr<Particle> Particle::heartdown(
	std::shared_ptr<AnimationGroup> group,
	const Point& placement, int height,
	float delay)
{
	static std::shared_ptr<Sprite::Palette> palette(new Sprite::Palette({
		Paint::blend(ColorName::BROKEN, ColorName::SHINEBLEND),
		Paint(ColorName::BROKEN),
		Paint::blend(ColorName::BROKEN, ColorName::SHADEBLEND),
	}));

	auto sprite = std::make_shared<Sprite>(randomheartdown(), palette);
	sprite->setOriginAtBase();
	return std::unique_ptr<Particle>(new Particle(group,
		sprite,
		placement, height,
		delay));
}

std::unique_ptr<Particle> Particle::workdown(
	std::shared_ptr<AnimationGroup> group,
	const Point& placement, int height,
	float delay)
{
	static std::shared_ptr<Sprite::Palette> palette(new Sprite::Palette({
		Paint::blend(ColorName::BROKEN, ColorName::SHINEBLEND),
		Paint(ColorName::BROKEN),
		Paint::blend(ColorName::BROKEN, ColorName::SHADEBLEND),
	}));

	auto sprite = std::make_shared<Sprite>(randomworkdown(), palette);
	sprite->setOriginAtBase();
	return std::unique_ptr<Particle>(new Particle(group,
		sprite,
		placement, height,
		delay));
}

std::unique_ptr<Particle> Particle::moon(
	std::shared_ptr<AnimationGroup> group,
	const Point& placement, int height,
	float delay)
{
	static std::shared_ptr<Sprite::Palette> palette(new Sprite::Palette({
		Paint::blend(ColorName::BROKEN, ColorName::SHINEBLEND),
		Paint(ColorName::BROKEN),
		Paint::blend(ColorName::BROKEN, ColorName::SHADEBLEND),
		Paint::alpha(ColorName::BROKEN, 128),
	}));

	auto sprite = std::make_shared<Sprite>(randommoon(), palette);
	sprite->setOriginAtBase();
	return std::unique_ptr<Particle>(new Particle(group,
		sprite,
		placement, height,
		delay));
}

std::unique_ptr<Particle> Particle::income(
	std::shared_ptr<AnimationGroup> group,
	const Point& placement, int height,
	float delay)
{
	static std::shared_ptr<Sprite::Palette> palette(new Sprite::Palette({
		Paint::blend(ColorName::COIN, ColorName::SHINEBLEND),
		Paint(ColorName::COIN),
		Paint::blend(ColorName::COIN, ColorName::SHADEBLEND),
		Paint(ColorName::FLASH),
	}));

	auto sprite = std::make_shared<Sprite>(randomincome(), palette);
	sprite->setOriginAtBase();
	return std::unique_ptr<Particle>(new Particle(group,
		sprite,
		placement, height,
		delay));
}

std::unique_ptr<Particle> Particle::diamondup(
	std::shared_ptr<AnimationGroup> group,
	const Point& placement, int height,
	float delay)
{
	static std::shared_ptr<Sprite::Palette> palette(new Sprite::Palette({
		Paint::blend(ColorName::DIAMOND, ColorName::SHINEBLEND, 2.00f),
		Paint(ColorName::DIAMOND),
		Paint::blend(ColorName::DIAMOND, ColorName::SHADEBLEND, 2.00f),
		Paint(ColorName::FLASH),
	}));

	auto sprite = std::make_shared<Sprite>(randomdiamondup(), palette);
	sprite->setOriginAtBase();
	return std::unique_ptr<Particle>(new Particle(group,
		sprite,
		placement, height,
		delay));
}

std::unique_ptr<Particle> Particle::diamonddown(
	std::shared_ptr<AnimationGroup> group,
	const Point& placement, int height,
	float delay)
{
	static std::shared_ptr<Sprite::Palette> palette(new Sprite::Palette({
		Paint::blend(ColorName::DIAMOND, ColorName::SHINEBLEND),
		Paint(ColorName::DIAMOND),
		Paint::blend(ColorName::DIAMOND, ColorName::SHADEBLEND),
	}));

	auto sprite = std::make_shared<Sprite>(randomdiamonddown(), palette);
	sprite->setOriginAtBase();
	return std::unique_ptr<Particle>(new Particle(group,
		sprite,
		placement, height,
		delay));
}
