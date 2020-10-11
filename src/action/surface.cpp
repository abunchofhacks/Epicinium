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
#include "surface.hpp"
#include "source.hpp"

#include "loop.hpp"
#include "collector.hpp"
#include "change.hpp"
#include "notice.hpp"
#include "sprite.hpp"
#include "randomizer.hpp"
#include "particle.hpp"
#include "animationgroup.hpp"
#include "camerafocus.hpp"
#include "square.hpp"
#include "border.hpp"
#include "aim.hpp"
#include "mixer.hpp"
#include "skin.hpp"
#include "skinner.hpp"
#include "palette.hpp"
#include "colorname.hpp"


Surface::Surface(const Square* square, const Skin& skin, const Player& player) :
	Actor(skin.spritename, convertOrigin(square->position())),
	_player(player),
	_texturetype(skin.texturetype),
	_palettesize(std::min(skin.palettesize, _sprite->paletteSize())),
	_basicPalette(skin.palette("basic")),
	_dryPalette(skin.palette("dry")),
	_hotPalette(skin.palette("hot")),
	_coldPalette(skin.palette("cold")),
	_snowPalette(skin.palette("snow")),
	_hazardguides({
			Guide::death(square->cell()),
			Guide::firestorm(square->cell()),
			Guide::radiation(square->cell()),
			Guide::gas(square->cell()),
			Guide::frostbite(square->cell()),
		}),
	_raininess(5 * square->skinner().raininess),
	_climateModifier(square->skinner().climateModifier),
	_weathercountdown(0)
{
	for (size_t i = 0; i <= PLAYER_MAX; i++)
	{
		_teamPalettes[i] = skin.palette(stringref(::stringify((Player) i)));
	}

	_sprite->setOriginAtBase();
	_sprite->randomizeAnimationStart();
	_particlebuffer.resize(100);

	for (size_t i = 0; i < (size_t) HazardType::SIZE; i++)
	{
		HazardType type = (HazardType) i;
		switch (type)
		{
			case HazardType::FROSTBITE:
			{
				_hazards[i] = square->frostbite();
			}
			break;
			case HazardType::FIRESTORM:
			{
				_hazards[i] = square->firestorm();
			}
			break;
			// Bonedrought does not count as a Hazard, see below.
			case HazardType::DEATH:
			{
				_hazards[i] = square->death();
			}
			break;
			case HazardType::GAS:
			{
				_hazards[i] = square->gas();
			}
			break;
			case HazardType::RADIATION:
			{
				_hazards[i] = square->radiation();
			}
			break;
			case HazardType::SIZE:
			break;
		}
	}
}

Surface::Surface(const Border* border, const Skin& skin) :
	Actor(skin.spritename, convertOrigin(border->position())),
	_player(Player::NONE),
	_texturetype(skin.texturetype),
	_palettesize(std::min(skin.palettesize, _sprite->paletteSize())),
	_basicPalette(skin.palette("basic")),
	_dryPalette(nullptr),
	_hotPalette(nullptr),
	_coldPalette(nullptr),
	_snowPalette(nullptr),
	_raininess(0),
	_climateModifier(0),
	_weathercountdown(0)
{
	for (size_t i = 0; i <= PLAYER_MAX; i++)
	{
		_teamPalettes[i] = nullptr;
	}

	_sprite->setOriginAtBase();
	_sprite->randomizeAnimationStart();
	_particlebuffer.resize(100);
}

bool Surface::inheritBlends(std::shared_ptr<Surface> old)
{
	if (old && old->_texturetype == _texturetype)
	{
		_sprite->inheritBlends(old->_sprite);
		return true;
	}
	else return false;
}

void Surface::blendTop(std::shared_ptr<Surface> other)
{
	TextureType othertype = other ? other->_texturetype : TextureType::NONE;
	_sprite->setBlendTop(blendnameTop(othertype, _texturetype));
}

void Surface::blendLeft(std::shared_ptr<Surface> other)
{
	TextureType othertype = other ? other->_texturetype : TextureType::NONE;
	_sprite->setBlendLeft(blendnameLeft(othertype, _texturetype));
}

void Surface::update()
{
	Actor::update();

	bool visib = (_sprite->isVisible());
	bool night = (_transitions[TRANSITION_LIGHT] < 0.55);
	bool first = true;
	for (size_t i = 0; i < (size_t) HazardType::SIZE; i++)
	{
		if (visib && !night && first && _hazards[i])
		{
			_hazardguides[i].setVisible(true);
			first = false;
		}
		else _hazardguides[i].setVisible(false);
		_hazardguides[i].update();
	}
}

void Surface::drawSprite()
{
	static constexpr bool OLDSTYLE = true;

	if (_sprite->isSetAsBackground())
	{
		Collector::get()->addBackgroundSurface(_sprite, _point);
	}
	else if (OLDSTYLE)
	{
		Collector::get()->addSortedSurface(_sprite, _point);
	}
	else if (_sprite->isBorder())
	{
		Collector::get()->addBorderSurfaceUnmasked(_sprite, _point);
		Collector::get()->addBorderSurfaceMasked(_sprite, _point);
	}
	else
	{
		Collector::get()->addSurfaceUnmasked(_sprite, _point);
		Collector::get()->addSurfaceMasked(_sprite, _point);
	}
}

void Surface::mix()
{
	size_t team = (size_t) _player;

	for (int i = 0; i < _palettesize; i++)
	{
		Color mixed = _basicPalette[i];

		if (!_teamPalettes.empty())
		{
			if (_teamPalettes[team] && _teamPalettes[team][i])
			{
				mixed = _teamPalettes[team][i];
			}
		}

		if (_hotPalette && _hotPalette[i] && _climateModifier > 0)
		{
			mixed = Color::mix(mixed, _hotPalette[i], _climateModifier);
		}
		else if (_coldPalette && _coldPalette[i] && _climateModifier < 0)
		{
			mixed = Color::mix(mixed, _coldPalette[i], -_climateModifier);
		}

		if (_dryPalette && _dryPalette[i])
		{
			float hum = humidityTransitionCap(_transitions[TRANSITION_HUMIDITY]);
			mixed = Color::mix(mixed, _dryPalette[i], 1.0f - hum);
		}

		if (_snowPalette && _snowPalette[i])
		{
			mixed = Color::blend(mixed, _snowPalette[i], _transitions[TRANSITION_SNOW]);
		}

		float lum = std::max(0.0f, std::min(2.0f,
			(_transitions[TRANSITION_LIGHT] + _transitions[TRANSITION_FLASHLIGHT])
			* _transitions[TRANSITION_SPOTLIGHT]));
		if (lum < 1.0f)
		{
			mixed = Color::blend(mixed, Palette::get(ColorName::BLACKBLEND), 1.0f - lum);
		}
		else if (lum > 1.0f)
		{
			mixed = Color::blend(mixed, Palette::get(ColorName::GLAREBLEND), lum - 1.0f);
		}

		if (_visualizer)
		{
			float fraction = _transitions[_visualizer.transitionIndex()];
			mixed = _visualizer.mix(mixed, fraction);
		}

		_sprite->setColor(i, mixed);
	}
}

float Surface::temperatureTransitionCap(float temperature)
{
	if (temperature < 4.5f)
	{
		return 0.0f;
	}
	else if (temperature < 5.5f)
	{
		return 0.200f * (temperature - 4.5f) / 1.0f;
	}
	else if (temperature < 9.5f)
	{
		return 0.200f;
	}
	else if (temperature < 10.5f)
	{
		return 0.200f + 0.200f * (temperature - 9.5f) / 1.0f;
	}
	else if (temperature < 14.5f)
	{
		return 0.400f;
	}
	else if (temperature < 15.5f)
	{
		return 0.400f + 0.100f * (temperature - 14.5f) / 1.0f;
	}
	else if (temperature < 19.5f)
	{
		return 0.500f;
	}
	else if (temperature < 20.5f)
	{
		return 0.500f + 0.100f * (temperature - 19.5f) / 1.0f;
	}
	else if (temperature < 24.5f)
	{
		return 0.600f;
	}
	else if (temperature < 25.5f)
	{
		return 0.600f + 0.200f * (temperature - 24.5f) / 1.0f;
	}
	else if (temperature < 29.5f)
	{
		return 0.800f;
	}
	else if (temperature < 30.5f)
	{
		return 0.800f + 0.200f * (temperature - 29.5f) / 1.0f;
	}
	else
	{
		return 1.0f;
	}
}

float Surface::humidityTransitionCap(float humidity)
{
	if (humidity < 0.1f)
	{
		return 0.0f;
	}
	else if (humidity < 0.9f)
	{
		return 0.200f * (humidity - 0.1f) / 0.8f;
	}
	else if (humidity < 1.1f)
	{
		return 0.200f;
	}
	else if (humidity < 1.9f)
	{
		return 0.200f + 0.400f * (humidity - 1.1f) / 0.8f;
	}
	else if (humidity < 2.1f)
	{
		return 0.600f;
	}
	else if (humidity < 2.9f)
	{
		return 0.600f + 0.400f * (humidity - 2.1f) / 0.8f;
	}
	else
	{
		return 1.0f;
	}
}

void Surface::animate(const Change& change,
	std::shared_ptr<AnimationGroup> group, Square* square)
{
	float delay = group ? group->delay : 0;

	switch (change.type)
	{
		case Change::Type::REVEAL:
		{
			bool frostbite = change.frostbite;
			bool firestorm = change.firestorm;
			bool bonedrought = change.bonedrought;
			bool death = change.death;
			bool gas = change.gas;
			bool radiation = change.radiation;

			addAnimation(Animation(group, [this,
					frostbite, firestorm, death, gas, radiation](float /**/){

				if (frostbite) _hazards[(size_t) HazardType::FROSTBITE] = true;
				if (firestorm) _hazards[(size_t) HazardType::FIRESTORM] = true;
				// Bonedrought does not count as a Hazard, see below.
				if (death)     _hazards[(size_t) HazardType::DEATH] = true;
				if (gas)       _hazards[(size_t) HazardType::GAS] = true;
				if (radiation) _hazards[(size_t) HazardType::RADIATION] = true;

			}, 0, delay));

			if (frostbite)   weather(group, WeatherType::FROSTBITE, 0.25);
			if (firestorm)   weather(group, WeatherType::FIRESTORM, 0.5);
			if (bonedrought) weather(group, WeatherType::BONEDROUGHT, 0.25);
			if (death)       weather(group, WeatherType::DEATH, 0.25);
			if (gas)         weather(group, WeatherType::GAS, 0.5);
			if (radiation)   weather(group, WeatherType::RADIATION, 0.5);
			_weathercountdown = delay + 1.000f;
		}
		break;

		case Change::Type::OBSCURE:
		{
			addAnimation(Animation(group, [this](float /**/){

				_hazards.reset();

			}, 0, delay));
		}
		break;

		case Change::Type::SETTLED:
		case Change::Type::EXPANDED:
		case Change::Type::CULTIVATED:
		case Change::Type::SHAPED:
		{
			for (int i = 0; i < 5; i++)
			{
				for (int j = 0; j < 5; j++)
				{
					if (i > 0 && i < 5 - 1 && j > 0 && j < 5 - 1) continue;
					Point placement(_point);
					placement.xenon += -WIDTH / 2 + (int) (0.025f
						* (10 * i - 2 + (rand() % 5)) * WIDTH);
					placement.yahoo += -HEIGHT + (int) (0.025f
						* (10 * j - 2 + (rand() % 5)) * HEIGHT);
					_particlebuffer.add(Particle::dustflame(group,
						placement, delay));
				}
			}
		}
		break;

		case Change::Type::TRANSFORMED:
		{
			Mixer::get()->queue(Clip::Type::CROPS, 0.5f);
		}
		break;
		case Change::Type::DESTROYED:
		break;

		case Change::Type::ATTACKED:
		{
			if (change.subject.type == Descriptor::Type::CELL)
			{
				Point placement(_point);
				placement.xenon += -(WIDTH - 20) / 2
					+ (rand() % (WIDTH - 20 + 1));
				placement.yahoo += -HEIGHT
					+ 10 + (rand() % (HEIGHT - 20 + 1));
				_particlebuffer.add(Particle::flak(group,
					placement, HEIGHT / 3, delay));
				break;
			}

			// In case there is no ATTACKS change, the ATTACKED change will
			// determine the time between attacks instead.
			if (group)
			{
				if (group->attacker == false)
				{
					// queue the audio
					Mixer::get()->queue(Clip::Type::GUNSHOT, delay, _point);

					// TODO magic number 0.180f matches the same in Figure
					delay = (group->delay += 0.180f);
				}
				else group->attacker = false;
			}

			// Impact time is 100ms after the attack animation starts (because of aiming).
			// TODO magic number 0.180f matches the same in Figure
			float impacttime = delay - 0.180f + 0.100f;
			impact(change, group, impacttime);

			// No hitstop.
			//addHitstop(group, impacttime, 0.5f);
		}
		break;

		case Change::Type::TRAMPLED:
		{
			if (group)
			{
				if (auto camerafocus = group->camerafocus.lock())
				{
					camerafocus->shake(group, 1, 0.100f, 0.100f, delay);
				}
			}
		}
		break;

		case Change::Type::SHELLED:
		{
			if (change.subject.type == Descriptor::Type::CELL)
			{
				Point placement(_point);
				placement.xenon += -(WIDTH - 20) / 2
					+ (rand() % (WIDTH - 20 + 1));
				placement.yahoo += -HEIGHT
					+ 10 + (rand() % (HEIGHT - 20 + 1));
				_particlebuffer.add(Particle::flak(group,
					placement, HEIGHT / 3, delay));
				break;
			}

			// In case there is no SHELLS change, the SHELLED change will
			// determine the time between attacks instead.
			if (group)
			{
				if (group->attacker == false)
				{
					// TODO magic number 0.400f matches the same in Figure
					delay = (group->delay += 0.400f);
				}
				else group->attacker = false;
			}

			// Impact time is 100ms after the attack animation starts (because of aiming).
			// TODO magic number 0.400f matches the same in Figure
			float impacttime = delay - 0.400f + 0.100f;
			boom(change, group, impacttime);

			// Add a flash of light to nearby squares.
			addFlash(group, square, 0.10f, impacttime, 0.050f, 0.100f);

			// No hitstop.
			//addHitstop(group, impacttime, 0.5f);

			if (group)
			{
				// queue the audio
				Mixer::get()->queue(Clip::Type::EXPLOSION, impacttime, _point);

				if (auto camerafocus = group->camerafocus.lock())
				{
					camerafocus->shake(group, 2, 0.020f, 0.180f, impacttime);
				}
			}
		}
		break;

		case Change::Type::BOMBARDED:
		case Change::Type::BOMBED:
		{
			if (change.subject.type == Descriptor::Type::CELL)
			{
				Point placement(_point);
				placement.xenon += -(WIDTH - 20) / 2
					+ (rand() % (WIDTH - 20 + 1));
				placement.yahoo += -HEIGHT
					+ 10 + (rand() % (HEIGHT - 20 + 1));
				_particlebuffer.add(Particle::flak(group,
					placement, HEIGHT / 3, delay));
				break;
			}

			if (group)
			{
				// queue the audio
				Mixer::get()->queue(Clip::Type::DOWN, delay, _point);

				delay = (group->delay += 0.500f);
			}

			if (_texturetype == TextureType::WATER)
			{
				impact(change, group, delay);

				// TODO add splash sound effect
				break;
			}

			boom(change, group, delay);

			// Add a flash of light to nearby squares.
			addFlash(group, square, 0.10f, delay, 0.050f, 0.100f);

			// No hitstop.
			//addHitstop(group, delay, 0.5f);

			if (group)
			{
				// queue the audio
				Mixer::get()->queue(Clip::Type::EXPLOSION, delay, _point);

				if (auto camerafocus = group->camerafocus.lock())
				{
					camerafocus->shake(group, 2, 0.020f, 0.180f, delay);
				}

				delay = (group->delay += 0.300f);
			}
		}
		break;

		case Change::Type::BURNED:
		break;

		case Change::Type::GASSED:
		break;

		case Change::Type::IRRADIATED:
		break;

		case Change::Type::SNOW:
		{
			if (change.snow)
			{
				// For aesthetics we also do snow transitions in the fog of war,
				// but we do not want to trigger weather particles there.
				if (change.subject.type != Descriptor::Type::NONE)
				{
					weather(group, WeatherType::SNOW, 2.0);
					_weathercountdown = delay + 1.000f;
				}

				transition(group, TRANSITION_SNOW, 1, 1.000f);
			}
			else
			{
				transition(group, TRANSITION_SNOW, 0, 1.000f);
			}
		}
		break;

		case Change::Type::FROSTBITE:
		{
			bool frostbite = change.frostbite;
			addAnimation(Animation(group, [this, frostbite](float /**/){

				_hazards[size_t(HazardType::FROSTBITE)] = frostbite;

			}, 0, delay + 0.500f));

			if (!change.frostbite) break;

			weather(group, WeatherType::FROSTBITE, 1.0);
			_weathercountdown = delay + 1.000f;
		}
		break;

		case Change::Type::FIRESTORM:
		{
			bool firestorm = change.firestorm;
			addAnimation(Animation(group, [this, firestorm](float /**/){

				_hazards[size_t(HazardType::FIRESTORM)] = firestorm;

			}, 0, delay + 0.500f));

			if (!change.firestorm) break;

			// Add a flash of light to nearby squares.
			addFlash(group, square, 0.15f, delay, 0.100f, 0.200f);

			weather(group, WeatherType::FIRESTORM, 2.0);
			_weathercountdown = delay + 1.000f;
		}
		break;

		case Change::Type::BONEDROUGHT:
		{
			// Bonedrought does not count as a Hazard because it does not deal
			// damage, because it is permanent and because its placement is
			// already indicated by the color of the desert and stone surface.

			if (!change.bonedrought) break;

			weather(group, WeatherType::BONEDROUGHT, 2.0);
			_weathercountdown = delay + 1.000f;
		}
		break;

		case Change::Type::DEATH:
		{
			bool death = change.death;
			addAnimation(Animation(group, [this, death](float /**/){

				_hazards[size_t(HazardType::DEATH)] = death;

			}, 0, delay + 0.500f));

			if (!change.death) break;

			weather(group, WeatherType::DEATH, 1.0);
			_weathercountdown = delay + 1.000f;
		}
		break;

		case Change::Type::GAS:
		{
			bool gas = square->gas();
			addAnimation(Animation(group, [this, gas](float /**/){

				_hazards[size_t(HazardType::GAS)] = gas;

			}, 0, delay + 0.500f));

			if (!square->gas()) break;

			if (square->gas() >= 2) weather(group, WeatherType::GAS, 1.0);
			else                 weather(group, WeatherType::GAS, 0.4);
			_weathercountdown = delay + 1.000f;
		}
		break;

		case Change::Type::RADIATION:
		{
			bool radiation = square->radiation();
			addAnimation(Animation(group, [this, radiation](float /**/){

				_hazards[size_t(HazardType::RADIATION)] = radiation;

			}, 0, delay + 0.500f));

			if (!square->radiation()) break;

			weather(group, WeatherType::RADIATION, 1.0);
			_weathercountdown = delay + 1.000f;
		}
		break;

		case Change::Type::HUMIDITY:
		{
			float val = (float) square->humidity();
			transition(group, TRANSITION_HUMIDITY, val, 1);
		}
		break;

		case Change::Type::TEMPERATURE:
		{
			float val = (float) square->temperature();
			transition(group, TRANSITION_TEMPERATURE, val, 1);
		}
		break;

		case Change::Type::INCOME:
		{
			Point point(_point);
			point.yahoo += 5;

			bool moraleReplacesMoney = (group && group->morale);

			if (change.money && !moraleReplacesMoney)
			{
				_particlebuffer.add(
					Particle::income(group, point, HEIGHT / 2, delay));

				if (group)
				{
					// queue the audio
					Mixer::get()->queue(Clip::Type::COIN, delay, _point);
				}
			}
			else if (change.money > 0)
			{
				_particlebuffer.add(
					Particle::heartup(group, point, HEIGHT / 2, delay));

				if (group)
				{
					// queue the audio
					Mixer::get()->queue(Clip::Type::HEART, delay, _point);
				}
			}
			else
			{
				_particlebuffer.add(
					Particle::heartdown(group, point, HEIGHT / 2, delay));

				if (group)
				{
					// queue the audio
					Mixer::get()->queue(Clip::Type::NOHEART, delay, _point);
				}
			}
		}
		break;

		case Change::Type::SCORED:
		{
			Point point(_point);
			point.yahoo += 5;
			_particlebuffer.add(Particle::diamondup(group, point, HEIGHT / 2, delay));

			if (group)
			{
				// queue the audio
				Mixer::get()->queue(Clip::Type::COIN, delay, _point);
			}
		}
		break;

		default:
		{
			LOGW << "Missing case for '" << change.type << "'";
			DEBUG_ASSERT(false);
		}
		break;
	}
}

void Surface::animateScoreLoss(const Change& /*change*/,
	std::shared_ptr<AnimationGroup> group, float delay)
{
	Point point(_point);
	point.yahoo += 5;
	_particlebuffer.add(Particle::diamonddown(group, point, HEIGHT / 2, delay));

	if (group)
	{
		// queue the audio
		Mixer::get()->queue(Clip::Type::NOCOIN, delay, _point);
	}
}

void Surface::animateBlock(const Change& change,
	std::shared_ptr<AnimationGroup> group, float delay)
{
	switch (change.notice)
	{
		case Notice::SUBJECTOCCUPIED:
		case Notice::TARGETOCCUPIED:
		case Notice::DESTINATIONOCCUPIED:
		case Notice::UNBUILDABLE:
		case Notice::LACKINGSTACKS:
		case Notice::OCCUPIEDBYENEMY:
		{
			Point point(_point);
			point.yahoo += 5;
			_particlebuffer.add(Particle::blocked(group, point, HEIGHT / 2, delay));
		}
		break;

		case Notice::LACKINGPOWER:
		{
			Point point(_point);
			point.yahoo += 5;
			_particlebuffer.add(Particle::moon(group, point, HEIGHT / 2, delay));
		}
		break;

		case Notice::LACKINGMONEY:
		{
			Point point(_point);
			point.yahoo += 5;
			_particlebuffer.add(Particle::nocoin(group, point, HEIGHT / 2, delay));

			if (group)
			{
				// queue the audio
				Mixer::get()->queue(Clip::Type::NOCOIN, delay, _point);
			}
		}
		break;

		default:
		break;
	}
}

void Surface::impact(const Change&,
	std::shared_ptr<AnimationGroup> group, float delay)
{
	// TODO this is not great
	constexpr int DARK = 1;

	Point placement(_point);
	if (group && !group->placements.empty())
	{
		placement = group->placements.back();
		group->placements.pop_back();
	}
	else
	{
		placement.xenon += -(WIDTH - 8) / 2 + (rand() % (WIDTH - 8 + 1));
		placement.yahoo -= 4 + (rand() % (HEIGHT - 8 + 1));
	}
	_particlebuffer.add(Particle::spray(group, placement,
		_sprite->getColor(DARK), delay));
}

void Surface::boom(const Change&,
	std::shared_ptr<AnimationGroup> group, float delay)
{
	// TODO this is not great
	constexpr int DARK = 1;

	Point placement(_point);
	if (group && !group->placements.empty())
	{
		placement = group->placements.back();
		group->placements.pop_back();
	}
	else
	{
		placement.xenon += -(WIDTH - 8) / 2 + (rand() % (WIDTH - 8 + 1));
		placement.yahoo -= 4 + (rand() % (HEIGHT - 8 + 1));
	}
	_particlebuffer.add(Particle::boom(group, placement,
		_sprite->getColor(DARK), delay));
}

void Surface::mowGrass(std::shared_ptr<AnimationGroup> group)
{
	float delay = group ? group->delay : 0;

	constexpr int BASE = 0;
	Color mixed = _basicPalette[BASE];

	if (_hotPalette && _hotPalette[BASE] && _climateModifier > 0)
	{
		mixed = Color::mix(mixed, _hotPalette[BASE], _climateModifier);
	}
	else if (_coldPalette && _coldPalette[BASE] && _climateModifier < 0)
	{
		mixed = Color::mix(mixed, _coldPalette[BASE], -_climateModifier);
	}

	if (_dryPalette && _dryPalette[BASE])
	{
		float hum = humidityTransitionCap(_transitions[TRANSITION_HUMIDITY]);
		mixed = Color::mix(mixed, _dryPalette[BASE], 1.0f - hum);
	}

	if (_snowPalette && _snowPalette[BASE])
	{
		// Make it darker so it has more contrast with the white snow.
		mixed = Color::blend(mixed, Palette::get(ColorName::SHADEBLEND),
			_transitions[TRANSITION_SNOW]);
	}

	for (int i = 0; i < 5; i++)
	{
		for (int j = 0; j < 5; j++)
		{
			Point point(_point);
			point.xenon += -WIDTH / 2 + (int) (0.025f
				* (8 * i + 1 + (rand() % 7)) * WIDTH);
			point.yahoo += -HEIGHT + (int) (0.025f
				* (8 * j + 1 + (rand() % 7)) * HEIGHT);
			_particlebuffer.add(Particle::grassflake(group, point,
				mixed, delay));
		}
	}
}

void Surface::generateStatik(std::shared_ptr<AnimationGroup> group)
{
	float delay = group ? group->delay : 0;

	for (int i = 0; i < 5; i++)
	{
		for (int j = 0; j < 5; j++)
		{
			Point point(_point);
			point.xenon += -WIDTH / 2 + (int) (0.025f
				* (8 * i + 1 + (rand() % 7)) * WIDTH);
			point.yahoo += -HEIGHT + (int) (0.025f
				* (8 * j + 1 + (rand() % 7)) * HEIGHT);
			_particlebuffer.add(Particle::statik(group, point, delay));
		}
	}
}

void Surface::weather(std::shared_ptr<AnimationGroup> group,
	const WeatherType& weathertype, float percentage)
{
	bool night = (_transitions[TRANSITION_LIGHT] < 0.55);
	switch (weathertype)
	{
		case WeatherType::RAINWEAK:
		case WeatherType::RAIN:
		case WeatherType::RAINHEAVY:
		{
			if (night) return;
		}
		break;

		case WeatherType::SNOW:
		case WeatherType::FROSTBITE:
		case WeatherType::FIRESTORM:
		case WeatherType::BONEDROUGHT:
		case WeatherType::DEATH:
		{
			if (night && percentage < 0.9f) return;
		}
		break;

		case WeatherType::GAS:
		case WeatherType::RADIATION:
		{
			// We will use darker weather particles.
		}
		break;

		case WeatherType::NONE:
		{
			return;
		}
		break;
	}

	Randomizer<int> placements;
	int gridsize = 5;
	for (int r = 0; r < gridsize; r++)
	{
		for (int c = 0; c < gridsize; c++)
		{
			if ((r == 0 || r == gridsize - 1) && (c == 0 || c == gridsize - 1)) continue;
			placements.push(r * gridsize + c);
		}
	}

	size_t nplacements = placements.count();
	size_t amount = (size_t) lround(percentage * nplacements);
	if (amount <= 0) return;

	float groupdelay = group ? group->delay : 0;
	Randomizer<float> delays;
	for (size_t i = 0; i < amount; i++)
	{
		delays.push(groupdelay + 1.0f * i / std::min(amount, nplacements));
	}

	int w = WIDTH / gridsize;
	int h = HEIGHT / gridsize;
	for (size_t i = 0; i < amount && i < nplacements; i++)
	{
		int placement = placements.pop();
		int r = placement / gridsize;
		int c = placement % gridsize;
		float delay = delays.pop();

		for (size_t t = i; t < amount; t += gridsize * gridsize, delay += 0.5f)
		{
			Point point(_point);
			point.xenon += -WIDTH / 2 + c * w + 1 + (rand() % (w - 2));
			point.yahoo += -HEIGHT    + r * h + 1 + (rand() % (h - 2));
			switch (weathertype)
			{
				case WeatherType::RAINWEAK:
				{
					_particlebuffer.add(Particle::raindropWeak(group, point, delay));
				}
				break;

				case WeatherType::RAIN:
				{
					_particlebuffer.add(Particle::raindrop(group, point, delay));
				}
				break;

				case WeatherType::RAINHEAVY:
				{
					_particlebuffer.add(Particle::raindropHeavy(group, point, delay));
				}
				break;

				case WeatherType::SNOW:
				{
					_particlebuffer.add(Particle::snowflake(group, point, delay));
				}
				break;

				case WeatherType::FROSTBITE:
				{
					_particlebuffer.add(Particle::frost(group, point, delay));
				}
				break;

				case WeatherType::FIRESTORM:
				{
					_particlebuffer.add(Particle::ember(group, point, delay));
				}
				break;

				case WeatherType::BONEDROUGHT:
				{
					_particlebuffer.add(Particle::statik(group, point, delay));
				}
				break;

				case WeatherType::DEATH:
				{
					_particlebuffer.add(Particle::death(group, point, delay));
				}
				break;

				case WeatherType::GAS:
				{
					if (night)
					{
						_particlebuffer.add(Particle::gascloudDark(group, point, delay));
					}
					else
					{
						_particlebuffer.add(Particle::gascloud(group, point, delay));
					}
				}
				break;

				case WeatherType::RADIATION:
				{
					// TODO
				}
				break;

				case WeatherType::NONE:
				break;
			}
		}
	}
}

void Surface::updateWeather(const Square* square)
{
	if (!square->current()) return;

	if (!_sprite->isVisible()) return;

	_weathercountdown -= Loop::delta() * Loop::tempo();
	if (_weathercountdown <= 0)
	{
		_weathercountdown = 0.8f + 0.4 * (rand() % 1000) * 0.001;
		if (_raininess > 0 && square->humidity() > 0 && square->humidity() < 4
			&& !square->firestorm() && !square->gas())
		{
			if (square->humidity() + _raininess >= 6)
			{
				weather(nullptr, WeatherType::RAINHEAVY, 1.0);
			}
			else if (square->humidity() + _raininess >= 5)
			{
				weather(nullptr, WeatherType::RAIN, 0.5);
			}
			else if (square->humidity() + _raininess >= 4)
			{
				weather(nullptr, WeatherType::RAINWEAK, 0.25);
			}
		}
		if (square->frostbite())   weather(nullptr, WeatherType::FROSTBITE, 0.1);
		if (square->firestorm())   weather(nullptr, WeatherType::FIRESTORM, 0.2);
		if (square->bonedrought()) weather(nullptr, WeatherType::BONEDROUGHT, 0.1);
		if (square->death())       weather(nullptr, WeatherType::DEATH, 0.05);
		if (square->gas() > 0)
		{
			if (square->gas() >= 2) weather(nullptr, WeatherType::GAS, 1.0);
			else                    weather(nullptr, WeatherType::GAS, 0.2);
		}
		if (square->radiation())
		{
			if      (square->radiation() >= 3) weather(nullptr, WeatherType::RADIATION, 0.5);
			else if (square->radiation() >= 2) weather(nullptr, WeatherType::RADIATION, 0.3);
			else                               weather(nullptr, WeatherType::RADIATION, 0.1);
		}
	}
}

void Surface::addFlash(std::shared_ptr<AnimationGroup> group,
	Square* square, float amount,
	float delay, float up, float down)
{
	if (!group) return;
	if (!square) return;
	if (group->skiplighting) return;

	for (Cell index : square->area(0, 5))
	{
		Square* sq = square->get(index.pos());
		int sqdis = Aim(square->position(), sq->position()).sumofsquares();
		float y = amount * (1 - float(sqrt(sqdis / 5.1f)));
		sq->setFlashlight(y, up, delay);
		sq->setFlashlight(0, down, delay + up);
	}
}

void Surface::setPlayer(const Player& player)
{
	_player = player;
}

void Surface::setBorder()
{
	_sprite->setBorder(true);
}

void Surface::setAsBackground()
{
	_sprite->setAsBackground();
}

static const char* randomTop()
{
	switch (rand() % 4)
	{
		case 0: return "tiles/blend1_top";
		case 1: return "tiles/blend2_top";
		case 2: return "tiles/blend3_top";
		case 3: return "tiles/blend4_top";
	}
	return "tiles/blend3_top";
}

static const char* randomLeft()
{
	switch (rand() % 4)
	{
		case 0: return "tiles/blend1_left";
		case 1: return "tiles/blend2_left";
		case 2: return "tiles/blend3_left";
		case 3: return "tiles/blend4_left";
	}
	return "tiles/blend3_left";
}

static const char* randomTopGS()
{
	return "tiles/blend_gs1_top";
}

static const char* randomLeftGS()
{
	return "tiles/blend_gs1_left";
}

static const char* randomTopSG()
{
	return "tiles/blend_sg1_top";
}

static const char* randomLeftSG()
{
	return "tiles/blend_sg1_left";
}

static const char* randomTopGD()
{
	switch (rand() % 2)
	{
		case 0: return "tiles/blend_gd1_top";
		case 1: return "tiles/blend_gd2_top";
	}
	return "tiles/blend1_top";
}

static const char* randomLeftGD()
{
	switch (rand() % 2)
	{
		case 0: return "tiles/blend_gd1_left";
		case 1: return "tiles/blend_gd2_left";
	}
	return "tiles/blend1_left";
}

static const char* randomTopDG()
{
	switch (rand() % 2)
	{
		case 0: return "tiles/blend_dg1_top";
		case 1: return "tiles/blend_dg2_top";
	}
	return "tiles/blend1_top";
}

static const char* randomLeftDG()
{
	switch (rand() % 2)
	{
		case 0: return "tiles/blend_dg1_left";
		case 1: return "tiles/blend_dg2_left";
	}
	return "tiles/blend1_left";
}

static const char* randomTopGM()
{
	switch (rand() % 2)
	{
		case 0: return "tiles/blend_gm1_top";
		case 1: return "tiles/blend_gm2_top";
	}
	return "tiles/blend1_top";
}

static const char* randomLeftGM()
{
	switch (rand() % 2)
	{
		case 0: return "tiles/blend_gm1_left";
		case 1: return "tiles/blend_gm2_left";
	}
	return "tiles/blend1_left";
}

static const char* randomTopMG()
{
	switch (rand() % 2)
	{
		case 0: return "tiles/blend_mg1_top";
		case 1: return "tiles/blend_mg2_top";
	}
	return "tiles/blend1_top";
}

static const char* randomLeftMG()
{
	switch (rand() % 2)
	{
		case 0: return "tiles/blend_mg1_left";
		case 1: return "tiles/blend_mg2_left";
	}
	return "tiles/blend1_left";
}

static const char* randomTopGW()
{
	switch (rand() % 4)
	{
		case 0: return "tiles/blend_gw1_top";
		case 1: return "tiles/blend_gw2_top";
		case 2: return "tiles/blend_gw3_top";
		case 3: return "tiles/blend_gw4_top";
	}
	return "tiles/blend1_top";
}

static const char* randomLeftGW()
{
	switch (rand() % 4)
	{
		case 0: return "tiles/blend_gw1_left";
		case 1: return "tiles/blend_gw2_left";
		case 2: return "tiles/blend_gw3_left";
		case 3: return "tiles/blend_gw4_left";
	}
	return "tiles/blend1_left";
}

static const char* randomTopWG()
{
	switch (rand() % 4)
	{
		case 0: return "tiles/blend_wg1_top";
		case 1: return "tiles/blend_wg2_top";
		case 2: return "tiles/blend_wg3_top";
		case 3: return "tiles/blend_wg4_top";
	}
	return "tiles/blend1_top";
}

static const char* randomLeftWG()
{
	switch (rand() % 4)
	{
		case 0: return "tiles/blend_wg1_left";
		case 1: return "tiles/blend_wg2_left";
		case 2: return "tiles/blend_wg3_left";
		case 3: return "tiles/blend_wg4_left";
	}
	return "tiles/blend1_left";
}

const char* Surface::blendnameTop(TextureType top, TextureType bottom)
{
	switch (top)
	{
		case TextureType::NONE:
		{
			return randomTop();
		}
		break;

		case TextureType::GRASS:
		{
			switch (bottom)
			{
				case TextureType::NONE: return randomTop();
				case TextureType::GRASS: return randomTop();
				case TextureType::DIRT: return randomTopDG();
				case TextureType::DESERT: return randomTopDG();
				case TextureType::STONE: return randomTopSG();
				case TextureType::MOUNTAIN: return randomTopMG();
				case TextureType::WATER: return randomTopWG();
			}
		}
		break;

		case TextureType::DIRT:
		{
			switch (bottom)
			{
				case TextureType::NONE: return randomTop();
				case TextureType::GRASS: return randomTopGD();
				case TextureType::DIRT: return randomTop();
				case TextureType::DESERT: return randomTopGD();
				case TextureType::STONE: return randomTopSG();
				case TextureType::MOUNTAIN: return randomTopMG();
				case TextureType::WATER: return randomTopWG();
			}
		}
		break;

		case TextureType::DESERT:
		{
			switch (bottom)
			{
				case TextureType::NONE: return randomTop();
				case TextureType::GRASS: return randomTopDG();
				case TextureType::DIRT: return randomTopDG();
				case TextureType::DESERT: return randomTop();
				case TextureType::STONE: return randomTopSG();
				case TextureType::MOUNTAIN: return randomTopMG();
				case TextureType::WATER: return randomTopWG();
			}
		}
		break;

		case TextureType::STONE:
		{
			switch (bottom)
			{
				case TextureType::NONE: return randomTop();
				case TextureType::GRASS: return randomTopGS();
				case TextureType::DIRT: return randomTopGS();
				case TextureType::DESERT: return randomTopGS();
				case TextureType::STONE: return randomTop();
				case TextureType::MOUNTAIN: return randomTopMG();
				case TextureType::WATER: return randomTopWG();
			}
		}
		break;

		case TextureType::MOUNTAIN:
		{
			switch (bottom)
			{
				case TextureType::NONE: return randomTop();
				case TextureType::GRASS: return randomTopGM();
				case TextureType::DIRT: return randomTopGM();
				case TextureType::DESERT: return randomTopGM();
				case TextureType::STONE: return randomTopGM();
				case TextureType::MOUNTAIN: return randomTop();
				case TextureType::WATER: return randomTopWG();
			}
		}
		break;

		case TextureType::WATER:
		{
			switch (bottom)
			{
				case TextureType::NONE: return randomTop();
				case TextureType::GRASS: return randomTopGW();
				case TextureType::DIRT: return randomTopGW();
				case TextureType::DESERT: return randomTopGW();
				case TextureType::STONE: return randomTopGW();
				case TextureType::MOUNTAIN: return randomTopGW();
				case TextureType::WATER: return randomTop();
			}
		}
		break;
	}

	LOGE << "Missing blend top";
	DEBUG_ASSERT(false);
	return "tiles/blend0_top";
}

const char* Surface::blendnameLeft(TextureType left, TextureType right)
{
	switch (left)
	{
		case TextureType::NONE:
		{
			return randomLeft();
		}
		break;

		case TextureType::GRASS:
		{
			switch (right)
			{
				case TextureType::NONE: return randomLeft();
				case TextureType::GRASS: return randomLeft();
				case TextureType::DIRT: return randomLeftDG();
				case TextureType::DESERT: return randomLeftDG();
				case TextureType::STONE: return randomLeftSG();
				case TextureType::MOUNTAIN: return randomLeftMG();
				case TextureType::WATER: return randomLeftWG();
			}
		}
		break;

		case TextureType::DIRT:
		{
			switch (right)
			{
				case TextureType::NONE: return randomLeft();
				case TextureType::GRASS: return randomLeftGD();
				case TextureType::DIRT: return randomLeft();
				case TextureType::DESERT: return randomLeftGD();
				case TextureType::STONE: return randomLeftSG();
				case TextureType::MOUNTAIN: return randomLeftMG();
				case TextureType::WATER: return randomLeftWG();
			}
		}
		break;

		case TextureType::DESERT:
		{
			switch (right)
			{
				case TextureType::NONE: return randomLeft();
				case TextureType::GRASS: return randomLeftDG();
				case TextureType::DIRT: return randomLeftDG();
				case TextureType::DESERT: return randomLeft();
				case TextureType::STONE: return randomLeftSG();
				case TextureType::MOUNTAIN: return randomLeftMG();
				case TextureType::WATER: return randomLeftWG();
			}
		}
		break;

		case TextureType::STONE:
		{
			switch (right)
			{
				case TextureType::NONE: return randomLeft();
				case TextureType::GRASS: return randomLeftGS();
				case TextureType::DIRT: return randomLeftGS();
				case TextureType::DESERT: return randomLeftGS();
				case TextureType::STONE: return randomLeft();
				case TextureType::MOUNTAIN: return randomLeftMG();
				case TextureType::WATER: return randomLeftWG();
			}
		}
		break;

		case TextureType::MOUNTAIN:
		{
			switch (right)
			{
				case TextureType::NONE: return randomLeft();
				case TextureType::GRASS: return randomLeftGM();
				case TextureType::DIRT: return randomLeftGM();
				case TextureType::DESERT: return randomLeftGM();
				case TextureType::STONE: return randomLeftGM();
				case TextureType::MOUNTAIN: return randomLeft();
				case TextureType::WATER: return randomLeftWG();
			}
		}
		break;

		case TextureType::WATER:
		{
			switch (right)
			{
				case TextureType::NONE: return randomLeft();
				case TextureType::GRASS: return randomLeftGW();
				case TextureType::DIRT: return randomLeftGW();
				case TextureType::DESERT: return randomLeftGW();
				case TextureType::STONE: return randomLeftGW();
				case TextureType::MOUNTAIN: return randomLeftGW();
				case TextureType::WATER: return randomLeft();
			}
		}
		break;
	}

	LOGE << "Missing blend left";
	DEBUG_ASSERT(false);
	return "tiles/blend0_left";
}

std::shared_ptr<Skin> Surface::missingSkin()
{
	auto skinptr = std::make_shared<Skin>();
	Skin& skin = *skinptr;
	skin.name = "surfaces/missing";
	skin.spritename = "tiles/stone";
	skin.texturetype = TextureType::NONE;
	skin._palettedata = {
		Color::broken(),
	};
	skin.palettes = {skin._palettedata.data()};
	skin.palettenames = {"basic"};
	skin.palettesize = skin._palettedata.size();
	return skinptr;
}

