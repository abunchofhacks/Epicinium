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
#include "figure.hpp"
#include "source.hpp"

#include "collector.hpp"
#include "square.hpp"
#include "aim.hpp"
#include "surface.hpp"
#include "animationgroup.hpp"
#include "camerafocus.hpp"
#include "change.hpp"
#include "sprite.hpp"
#include "aim.hpp"
#include "move.hpp"
#include "particle.hpp"
#include "particlebuffer.hpp"
#include "camera.hpp"
#include "input.hpp"
#include "palette.hpp"
#include "colorname.hpp"
#include "mixer.hpp"
#include "notice.hpp"
#include "skin.hpp"
#include "mixer.hpp"
#include "loop.hpp"

Figure::Figure(PlacementBox&& box, 	const Skin& skin, const Player& player,
		const PowerType& powertype) :
	Actor(skin.randomspritename(), box.random()),
	_placementbox(std::move(box)),
	_palettesize(std::min(skin.palettesize, _sprite->paletteSize())),
	_basicPalette(skin.palette("basic")),
	_powerPalette(skin.palette("power")),
	_dryPalette(skin.palette("dry")),
	_snowPalette(skin.palette("snow")),
	_skintone(randomSkintone()),
	_player(player),
	_lastDestination(_point),
	_footprint(_point, skin.selectionspritename),
	_yahooOffset(skin.yahooOffset),
	_selected(false),
	_dying(false),
	_chilled(false),
	_moveGroup(false),
	_deathSound(skin.deathsound),
	_formtype(skin.formtype),
	_powertype(powertype),
	_chillTimer(0),
	_moved1(0),
	_moved2(0)
{
	for (size_t i = 0; i <= PLAYER_MAX; i++)
	{
		_teamPalettes[i] = skin.palette(stringref(::stringify((Player) i)));
	}

	_sprite->setOriginAtBase();
	_sprite->setOffset(0,
		(_formtype == FormType::BALLOON)
			? (skin.baseOffset + Surface::HEIGHT / 3)
			:  skin.baseOffset);
	_sprite->setTag("Idle");
	_sprite->setTag("East Idle");
	if (_placementbox.entrenched())
	{
		_sprite->setTag("East Trench Idle");
	}
#ifdef DEVELOPMENT
	if (player == Player::BLUE)
	{
		_sprite->setTag("West Idle");
		if (_placementbox.entrenched())
		{
			_sprite->setTag("West Trench Idle");
		}
	}
#endif
	_sprite->randomizeAnimationStart();

	_footprint.setPlayer(player);

	if (!skin.shadowspritename.empty())
	{
		_shadowsprite.reset(new Sprite(skin.shadowspritename));
		_shadowsprite->setOriginAtCenter();
		_shadowsprite->setTag("Horizontal");
		// TODO use a recipe
		_shadowsprite->setColor(0, Palette::get(ColorName::UNITSHADOW));
		_shadowsprite->setColor(1, Color::transparent());
	}

	// The zeppelin's windows should be lit at night.
	if (_formtype == FormType::BALLOON)
	{
		set(TRANSITION_POWER, 1);
	}

	set(TRANSITION_MOVE_X, _point.xenon);
	set(TRANSITION_MOVE_Y, _point.yahoo);
}

Figure::Figure(PlacementBox&& box,const Skin& skin, const Player& player) :
	Figure(std::move(box), skin, player, PowerType::HEART)
{}

void Figure::update()
{
	// Points have integer x and y, but the transition is float-based.
	// So we have to round off to the nearest integer.
	Point next;
	next.xenon = (int) lround(_transitions[TRANSITION_MOVE_X]
		+                     _transitions[TRANSITION_DISPLACE_X]);
	next.yahoo = (int) lround(_transitions[TRANSITION_MOVE_Y]
		+                     _transitions[TRANSITION_DISPLACE_Y]);

	// Is there a noticable displacement?
	// At high framerate and/or slow-motion, the different between _transition[TRANS._MOVE_X]
	// in two consecutive update() calls might be very small. Therefore this difference cannot
	// be used to determine movement. Still, there will be a call where rounding causes our
	// x or y position to move by 1.
	int dx = next.xenon - _point.xenon;
	int dy = next.yahoo - _point.yahoo;
	bool stunned = (_transitions[TRANSITION_STUN] > 0.01f);
	bool selfmove = (dx != 0 || dy != 0)
		&& _moveGroup
		&& !stunned;

	// Start or continue movement animation.
	if (!selfmove) {}
	else if ( dx > 0 &&  dx >= dy &&  dx > -dy)
	{
		_sprite->setTagActive("East Move", /*restart=*/false);
	}
	else if (-dx > 0 && -dx >= dy && -dx > -dy)
	{
		_sprite->setTagActive("West Move", /*restart=*/false);
	}
	else if ( dy > 0 &&  dy >= dx &&  dy > -dx)
	{
		_sprite->setTagActive("South Move", /*restart=*/false);
	}
	else if (-dy > 0 && -dy >= dx && -dy > -dx)
	{
		_sprite->setTagActive("North Move", /*restart=*/false);
	}

	// Enact move in figure (e.g. dustclouds for tanks).
	if (selfmove) moved(abs(dx) + abs(dy));
	else if (stunned) moved(-1);

	// Update our position.
	_footprint.point().xenon = _point.xenon = next.xenon;
	_footprint.point().yahoo = _point.yahoo = next.yahoo;

	Actor::update();
	_footprint.update();

	if (_shadowsprite)
	{
		const char* tag = _sprite->getTag().c_str();
		bool horizontal = (strncmp(tag, "East Xyz", 4) == 0
			||             strncmp(tag, "West Xyz", 4) == 0);
		bool vertical   = (strncmp(tag, "South Xy", 5) == 0
			||             strncmp(tag, "North Xy", 5) == 0);
		if    (horizontal) _shadowsprite->setTag("Horizontal");
		else if (vertical) _shadowsprite->setTag("Vertical");
		else               _shadowsprite->setTagActive("Death");

		_shadowsprite->update();
	}

	if (_chilled)
	{
		_chillTimer -= Loop::delta() * Loop::tempo();
		if (_chillTimer <= 0)
		{
			bool shiver = (rand() % 100 < 40);
			animateChill(nullptr, shiver);
		}
	}
}

void Figure::animateChill(std::shared_ptr<AnimationGroup> group,
	bool shiver, float delay)
{
	_chillTimer = 0.6f + 0.4 * (rand() % 1000) * 0.001;
	_particlebuffer.add(Particle::frostflame(group, &_point,
		(rand() % 7) - 3, 0, 4, 0.0f));

	if (shiver)
	{
		int offset = (1 - 2 * (rand() % 2)) * 2;
		float duration = 0.020f;
		transition(group, TRANSITION_STUN, 1, 0.0f, delay);
		transition(group, TRANSITION_DISPLACE_X, offset,
			duration, delay); delay += duration;
		transition(group, TRANSITION_DISPLACE_X, 0,
			duration, delay); delay += duration;
		transition(group, TRANSITION_DISPLACE_X, -offset,
			duration, delay); delay += duration;
		transition(group, TRANSITION_DISPLACE_X, 0,
			duration, delay); delay += duration;
		transition(group, TRANSITION_DISPLACE_X, offset,
			duration, delay); delay += duration;
		transition(group, TRANSITION_DISPLACE_X, 0,
			duration, delay); delay += duration;
		transition(group, TRANSITION_STUN, 0, 0.0f, delay + 0.020f);
	}
}

void Figure::drawSprite()
{
	Point point = _point;
	point.yahoo += _yahooOffset;
	Collector::get()->addFigure(_sprite, point);

	if (_shadowsprite)
	{
		Collector::get()->addShadow(_shadowsprite, _point);
	}
}

void Figure::mix()
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

		if (!mixed)
		{
			mixed = _skintone;
		}

		if (_dryPalette && _dryPalette[i])
		{
			float hum = humidityTransitionCap(_transitions[TRANSITION_HUMIDITY]);
			mixed = Color::blend(mixed, _dryPalette[i], 1.0f - hum);
		}

		if (_snowPalette && _snowPalette[i])
		{
			mixed = Color::blend(mixed, _snowPalette[i], _transitions[TRANSITION_SNOW]);
		}

		float power = 0;
		if (_powerPalette && _powerPalette[i])
		{
			power = introduceFlicker(_transitions[TRANSITION_POWER]);
			mixed = Color::mix(mixed, _powerPalette[i], power);
		}

		float lum = std::max(0.0f, std::min(2.0f, std::max(power,
			(_transitions[TRANSITION_LIGHT] + _transitions[TRANSITION_FLASHLIGHT]))));
		if (lum < 1.0f)
		{
			mixed = Color::blend(mixed, Palette::get(ColorName::BLACKBLEND), 1.0f - lum);
		}
		else if (lum > 1.0f)
		{
			mixed = Color::blend(mixed, Palette::get(ColorName::GLAREBLEND), lum - 1.0f);
		}

		mixed = Color::mix(mixed, Palette::get(ColorName::IMPACT),
			_transitions[TRANSITION_IMPACT]);

		_sprite->setColor(i, mixed);
	}
}

float Figure::humidityTransitionCap(float humidity)
{
	if (humidity < 0.1f)
	{
		return 0.0f;
	}
	else if (humidity < 0.9f)
	{
		return 0.700f * (humidity - 0.1f) / 0.8f;
	}
	else if (humidity < 1.1f)
	{
		return 0.700f;
	}
	else if (humidity < 1.9f)
	{
		return 0.700f + 0.300f * (humidity - 1.1f) / 0.8f;
	}
	else
	{
		return 1.0f;
	}
}


bool Figure::hovered() const
{
	Point point = _point;
	point.yahoo += _yahooOffset;
	Pixel origin = Camera::get()->convert(point, Layer::FIGURE);
	Pixel mouse = Input::get()->mousePixel();
	if (mouse.proximity.layer() > origin.proximity.layer()) return false;
	if (_sprite->onMask(origin, mouse))
	{
		Input::get()->layerMouse(origin.proximity.layer());
		return true;
	}
	else return false;
}

void Figure::select()
{
	_selected = true;
	_footprint.select();
}

void Figure::deselect()
{
	_selected = false;
	_footprint.deselect();
}

void Figure::reposition(std::shared_ptr<AnimationGroup> group,
	PlacementBox&& box)
{
	_placementbox = std::move(box);
	_chilled = false;

	// Get a new spawn point now, before _placementbox is overridden again.
	Point spawnpoint = _placementbox.random();

	// But only reposition when the time comes.
	addAnimation(Animation(group, [this, spawnpoint](float) {

		// Update position.
		_footprint.point().xenon = _point.xenon = spawnpoint.xenon;
		_footprint.point().yahoo = _point.yahoo = spawnpoint.yahoo;
		_transitions[TRANSITION_MOVE_X] = spawnpoint.xenon;
		_transitions[TRANSITION_MOVE_Y] = spawnpoint.yahoo;
	}, 0, group ? group->delay : 0));
}

float Figure::doMoving(std::shared_ptr<AnimationGroup> group,
	PlacementBox&& box, Move direction, float speed)
{
	_placementbox = std::move(box);
	return move(group, direction, speed, /*premoving=*/false);
}

float Figure::startMoving(std::shared_ptr<AnimationGroup> group,
	PlacementBox&& box, Move direction, float speed)
{
	_placementbox = std::move(box);
	return move(group, direction, speed, /*premoving=*/true);
}

float Figure::finishMoving(std::shared_ptr<AnimationGroup> group,
	Move direction, float speed)
{
	return move(group, direction, speed, /*premoving=*/false);
}

float Figure::move(std::shared_ptr<AnimationGroup> group,
	Move direction, float speed, bool premoving)
{
	float delay = group ? group->delay : 0;
	float totalduration = 0;

	addAnimation(Animation(group, [this](float) {

		moved(-1);
	}, 0, delay));

	// Acquire the pathing that the PlacementPather determined for us.
	PlacementPathing pathing;
	pathing.swap(_placementbox.pathing());

	int lasttime = 0;
	for (auto it = pathing.begin(); it != pathing.end(); ++it)
	{
		const Point& point = it->first;
		int time = (int) it->second;

		if (premoving && time > 3)
		{
			// If we are premoving, we want to stop midway through.
			if (lasttime < 3)
			{
				Point midway = _lastDestination;
				int dx = point.xenon - _lastDestination.xenon;
				int dy = point.yahoo - _lastDestination.yahoo;
				midway.xenon += dx * (3 - lasttime) / (time - lasttime);
				midway.yahoo += dy * (3 - lasttime) / (time - lasttime);
				totalduration += move(group, delay + totalduration, direction,
					speed, midway);
			}

			// We remember the remaining pathing stops for later.
			std::move(it, pathing.end(),
				std::back_inserter(_placementbox.pathing()));
			return totalduration;
		}

		totalduration += move(group, delay + totalduration, direction, speed,
			point);
	}

	totalduration += move(group, delay + totalduration, direction, speed,
		_placementbox.random());
	return totalduration;
}

float Figure::move(std::shared_ptr<AnimationGroup> group,
	float delay, /*mutable*/ Move direction, float speed,
	/*mutable*/ Point destination)
{
	int dx = destination.xenon - _lastDestination.xenon;
	int dy = destination.yahoo - _lastDestination.yahoo;

	if (dx == 0 && dy == 0) return 0;

	if (direction == Move::X)
	{
		if      ( dx > 0 &&  dx >  dy &&  dx > -dy) direction = Move::E;
		else if (-dx > 0 && -dx >  dy && -dx > -dy) direction = Move::W;
		else if ( dy > 0 &&  dy >  dx &&  dy > -dx) direction = Move::S;
		else if (-dy > 0 && -dy >  dx && -dy > -dx) direction = Move::N;
	}

	if (direction == Move::E || direction == Move::W)
	{
		if (dy >= -2 && dy <= 2)
		{
			destination.yahoo = _lastDestination.yahoo;
			dy = 0;
		}
	}
	else
	{
		if (dx >= -2 && dx <= 2)
		{
			destination.xenon = _lastDestination.xenon;
			dx = 0;
		}
	}

	if (dx == 0 && dy == 0) return 0;

	float visualspeed = 40.0f * std::max(1.0f, speed);
	float durationX = abs(dx) / visualspeed;
	float durationY = abs(dy) / visualspeed;
	float duration;

	_chilled = false;

	if (group)
	{
		if (direction == Move::E || direction == Move::W)
		{
			transition(group, TRANSITION_MOVE_X, destination.xenon, durationX, delay);
			transition(group, TRANSITION_MOVE_Y, destination.yahoo, durationY, delay + durationX);
		}
		else
		{
			transition(group, TRANSITION_MOVE_Y, destination.yahoo, durationY, delay);
			transition(group, TRANSITION_MOVE_X, destination.xenon, durationX, delay + durationY);
		}
		_moveGroup = true;
		duration = durationX + durationY;
	}
	else
	{
		set(TRANSITION_MOVE_X, destination.xenon);
		set(TRANSITION_MOVE_Y, destination.yahoo);
		_moveGroup = false;
		duration = 0;
	}

	if (direction == Move::E)
	{
		setTag(group, "East Idle", delay);
		if (_placementbox.entrenched())
		{
			setTag(group, "East Trench Idle", delay + duration);
		}
	}
	else if (direction == Move::W)
	{
		setTag(group, "West Idle", delay);
		if (_placementbox.entrenched())
		{
			setTag(group, "West Trench Idle", delay + duration);
		}
	}
	else if (direction == Move::N)
	{
		setTag(group, "North Idle", delay);
		if (_placementbox.entrenched())
		{
			setTag(group, "North Trench Idle", delay + duration);
		}
	}
	else
	{
		setTag(group, "South Idle", delay);
		if (_placementbox.entrenched())
		{
			setTag(group, "South Trench Idle", delay + duration);
		}
	}

	moverumble(group, duration);

	_lastDestination = destination;
	return duration;
}

void Figure::moved(int distance)
{
	switch (_formtype)
	{
		case FormType::VEHICLE:
		{
			if (distance < 0)
			{
				_moved1 = 0.0f;
				_moved2 = 120.0f;
				return;
			}

			_moved1 += distance;
			if (_moved1 >= 6.0f)
			{
				_moved1 -= 1.2f + 1.2f * (rand() % 9);
				_particlebuffer.add(Particle::dustcloud(nullptr,
					&_point, 0, 0, 0, 0));
			}

			_moved2 += distance;
			if (_moved2 >= 120.0f)
			{
				_moved2 = 0.0f;

				// queue the audio
				Mixer::get()->queue(Clip::Type::ENGINE, 0, _point);
			}
		}
		break;

		case FormType::BALLOON:
		{
			if (distance < 0)
			{
				_moved1 = 0.0f;
				_moved2 = 120.0f;
				return;
			}

			_moved1 += distance;
			if (_moved1 >= 6.0f)
			{
				_moved1 = 0;//-= 1.2f + 1.2f * (rand() % 9);
			}

			_moved2 += distance;
			if (_moved2 >= 15.0f)
			{
				_moved2 = 0.0f;

				// queue the audio
				Mixer::get()->queue(Clip::Type::ZEPPELIN, 0, _point);
			}
		}
		break;

		case FormType::PERSON:
		{
			if (distance < 0)
			{
				_moved1 = 0;
				_moved2 = 0;
				return;
			}

			_moved1 += distance;
			_moved2 += distance;

			if (_moved1 >= 12.0f)
			{
				_moved1 = 0;

				// queue the audio
				Mixer::get()->queue(Clip::Type::FOOTSTEP, 0, _point);
			}

			if (_moved2 >= 3.0f)
			{
				_moved2 = 0;

				// TODO add footprint (#1103)
			}
		}
		break;

		default:
		break;
	}
}

void Figure::moverumble(std::shared_ptr<AnimationGroup> group, float duration)
{
	if (!group) return;

	switch (_formtype)
	{
		case FormType::VEHICLE:
		{
			if (auto camerafocus = group->camerafocus.lock())
			{
				camerafocus->shake(group, 0.025f, duration, 0.025f, group->delay);
			}
		}
		break;

		default:
		break;
	}
}

Paint Figure::randomSkintone()
{
	int i = rand() % 8;

	static Paint SKINCOLORS[8] =
	{
		ColorName::SKINTONE100,
		ColorName::SKINTONE170,
		ColorName::SKINTONE195,
		ColorName::SKINTONE205,
		ColorName::SKINTONE215,
		ColorName::SKINTONE225,
		ColorName::SKINTONE235,
		ColorName::SKINTONE245,
	};

	return SKINCOLORS[i];
}

inline float introduceFlanger(float x)
{
	return x - 2 * sin(x * 2 * 2 * ::pi());
}

float Figure::introduceFlicker(float x)
{
	static constexpr float begin = 0.65f;
	static constexpr float end = 0.95f;
	static constexpr float len = end - begin;

	if (x > begin && x < end)
	{
		return std::max(0.0f, std::min(1.0f,
			begin + len * introduceFlanger((x - begin) / len)));
	}
	else return x;
}

void Figure::addFlash(std::shared_ptr<AnimationGroup> group,
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

void Figure::setPlayer(const Player& player)
{
	_player = player;
	_footprint.setPlayer(player);
}

void Figure::impact(const Change& change,
		std::shared_ptr<AnimationGroup> group, Square* square, float delay)
{
	int basewidth = 4;
	switch (_formtype)
	{
		case FormType::PERSON:     basewidth = 5; break;
		case FormType::VEHICLE:    basewidth = 5; break;
		case FormType::BALLOON:    basewidth = 5; break;
		case FormType::BUILDING:   basewidth = 8; break;
		default:                                  break;
	}
	int xOffset = (rand() % basewidth) - (basewidth / 2);

	if (_dying)
	{
		switch (_formtype)
		{
			case FormType::PERSON:
			{
				_particlebuffer.add(Particle::bloodspray(group,
					&_point, xOffset, _yahooOffset + 1, 0, delay));
				return;
			}
			break;

			case FormType::VEHICLE:
			{
				int height = rand() % 4 + 4;
				_particlebuffer.add(Particle::explosion(group,
					&_point, xOffset, _yahooOffset + 1, height, delay));
				return;
			}
			break;

			case FormType::BALLOON:
			{
				int height = rand() % 4 - 3 + Surface::HEIGHT / 3;
				_particlebuffer.add(Particle::flak(group,
					&_point, xOffset, _yahooOffset + 1, height, delay));
				return;
			}
			break;

			default:
			break;
		}
	}

	bool headshot = ((rand() % 2) == 0);

	switch (change.type)
	{
		case Change::Type::ATTACKED:
		{
			if (_formtype == FormType::PERSON)
			{
				if (isLethal(change) && headshot)
				{
					int height = rand() % 2 + 6;
					_particlebuffer.add(Particle::blood(group,
						&_point, xOffset, _yahooOffset + 1, height, delay));
				}
				else if (isLethal(change))
				{
					int height = rand() % 3 + 4;
					_particlebuffer.add(Particle::blood(group,
						&_point, xOffset, _yahooOffset + 1, height, delay));
				}
				else
				{
					int height = rand() % 4 + 8;
					_particlebuffer.add(Particle::spark(group,
						&_point, xOffset, _yahooOffset + 1, height, delay));
				}
			}
			else if (_formtype == FormType::VEHICLE)
			{
				int height = rand() % 4 + 4;
				_particlebuffer.add(Particle::spark(group,
					&_point, xOffset, _yahooOffset + 1, height, delay));
			}
			else if (_formtype == FormType::BUILDING)
			{
				int height = rand() % 12 + 5;
				_particlebuffer.add(Particle::spark(group,
					&_point, xOffset, _yahooOffset + 1, height, delay));
			}
			else if (_formtype == FormType::TREE
				|| _formtype == FormType::FENCE)
			{
				int height = rand() % 8 + 3;
				_particlebuffer.add(Particle::woodchip(group,
					&_point, xOffset, _yahooOffset + 1, height, delay));
			}
			else
			{
				int height = rand() % 4 + 2;
				_particlebuffer.add(Particle::spark(group,
					&_point, xOffset, _yahooOffset + 1, height, delay));
			}
		}
		break;

		case Change::Type::TRAMPLED:
		break;

		case Change::Type::SHELLED:
		{
			if (_formtype == FormType::PERSON)
			{
				int height = rand() % 3 + 4;
				_particlebuffer.add(Particle::blood(group,
					&_point, xOffset, _yahooOffset + 1, height, delay));
			}
			else if (_formtype == FormType::VEHICLE)
			{
				int height = rand() % 4 + 4;
				_particlebuffer.add(Particle::explosion(group,
					&_point, xOffset, _yahooOffset + 1, height, delay));
			}
			else if (_formtype == FormType::BALLOON)
			{
				int height = rand() % 4 + 6 + Surface::HEIGHT / 3;
				_particlebuffer.add(Particle::explosion(group,
					&_point, xOffset, _yahooOffset + 1, height, delay));
			}
			else if (_formtype == FormType::BUILDING)
			{
				int height = rand() % 12 + 5;
				_particlebuffer.add(Particle::explosion(group,
					&_point, xOffset, _yahooOffset + 1, height, delay));
			}
			else
			{
				int height = rand() % 4 + 2;
				_particlebuffer.add(Particle::explosion(group,
					&_point, xOffset, _yahooOffset + 1, height, delay));
			}
		}
		break;

		case Change::Type::BOMBARDED:
		case Change::Type::BOMBED:
		{
			if (_formtype == FormType::PERSON)
			{
				_particlebuffer.add(Particle::bloodboom(group,
					&_point, xOffset, _yahooOffset + 1, 0, delay));
			}
			else if (_formtype == FormType::VEHICLE)
			{
				Color metal = Palette::get(ColorName::METALNEUTRAL);
				_particlebuffer.add(Particle::boom(group,
					&_point, xOffset, _yahooOffset + 1, 0, metal, delay));
			}
			else if (_formtype == FormType::BALLOON)
			{
				int height = rand() % 4 + 6 + Surface::HEIGHT / 3;
				_particlebuffer.add(Particle::explosion(group,
					&_point, xOffset, _yahooOffset + 1, height, delay));
			}
			else if (_formtype == FormType::BUILDING)
			{
				_particlebuffer.add(Particle::sparkboom(group,
					&_point, xOffset, _yahooOffset + 1, 0, delay));
			}
			else if (_formtype == FormType::TREE || _formtype == FormType::FENCE)
			{
				_particlebuffer.add(Particle::woodboom(group,
					&_point, xOffset, _yahooOffset + 1, 0, delay));
			}
			else
			{
				Color stone = Palette::get(ColorName::ROCK);
				_particlebuffer.add(Particle::boom(group,
					&_point, xOffset, _yahooOffset + 1, 0, stone, delay));
			}
		}
		break;

		case Change::Type::FROSTBITTEN:
		{
			if (_formtype == FormType::PERSON)
			{
				int height = rand() % 3 + 2;
				_particlebuffer.add(Particle::frostflame(group,
					&_point, xOffset, _yahooOffset + 1, height, delay));
			}
			else
			{
				int height = rand() % 4 + 2;
				_particlebuffer.add(Particle::frostflame(group,
					&_point, xOffset, _yahooOffset + 1, height, delay));
			}
		}
		break;

		case Change::Type::BURNED:
		{
			if (_formtype == FormType::PERSON)
			{
				int height = rand() % 3 + 2;
				_particlebuffer.add(Particle::flame(group,
					&_point, xOffset, _yahooOffset + 1, height, delay));
			}
			else if (_formtype == FormType::VEHICLE)
			{
				int height = rand() % 4 + 2;
				_particlebuffer.add(Particle::flame(group,
					&_point, xOffset, _yahooOffset + 1, height, delay));
			}
			else if (_formtype == FormType::BALLOON)
			{
				int height = rand() % 4 + 2 + Surface::HEIGHT / 3;
				_particlebuffer.add(Particle::flame(group,
					&_point, xOffset, _yahooOffset + 1, height, delay));
			}
			else if (_formtype == FormType::BUILDING)
			{
				int height = rand() % 12 + 1;
				_particlebuffer.add(Particle::flame(group,
					&_point, xOffset, _yahooOffset + 1, height, delay));
			}
			else
			{
				int height = 2;
				_particlebuffer.add(Particle::flame(group,
					&_point, xOffset, _yahooOffset + 1, height, delay));
			}
		}
		break;

		case Change::Type::GASSED:
		{
			if (_formtype == FormType::PERSON)
			{
				int height = rand() % 2 + 6;
				_particlebuffer.add(Particle::burp(group,
					&_point, xOffset, _yahooOffset + 1, height, delay));
			}
			else
			{
				int height = rand() % 4 + 4;
				_particlebuffer.add(Particle::burp(group,
					&_point, xOffset, _yahooOffset + 1, height, delay));
			}
		}
		break;

		case Change::Type::DIED:
		{
			if (_formtype == FormType::PERSON)
			{
				int height = 8;
				_particlebuffer.add(Particle::deathflame(group,
					&_point, 0, _yahooOffset + 1, height, delay));
			}
		}
		break;

		default:
		break;
	}

	if (!isLethal(change)) return;

	if (_formtype == FormType::PERSON)
	{
		switch (change.type)
		{
			case Change::Type::ATTACKED:
			{
				if (headshot)
				{
					triggerDeathHeadshotAnimation(group, delay);
				}
				else
				{
					triggerDeathAnimation(group, delay);
				}
			}
			break;

			case Change::Type::SHELLED:
			case Change::Type::BOMBARDED:
			case Change::Type::BOMBED:
			{
				triggerDeathExplodeAnimation(group, delay);
			}
			break;

			default:
			{
				triggerDeathAnimation(group, delay);
			}
			break;
		}
	}
	else if (_formtype == FormType::VEHICLE)
	{
		// Tanks have no death animation, so we just use explosions instead.
		xOffset = 0;
		int height = rand() % 4 + 4;
		_particlebuffer.add(Particle::explosion(group,
			&_point, xOffset, _yahooOffset + 1, height, delay + 0.150f));
		xOffset = -2;
		height = rand() % 4 + 4;
		_particlebuffer.add(Particle::explosion(group,
			&_point, xOffset, _yahooOffset + 1, height, delay + 0.200f));
		xOffset = 2;
		height = rand() % 4 + 4;
		_particlebuffer.add(Particle::explosion(group,
			&_point, xOffset, _yahooOffset + 1, height, delay + 0.250f));

		// The tank disappears at the height of the explosions: after 250ms + 200ms.
		triggerDeathAnimation(group, delay + 0.250f + 0.200f);

		// Add a flash of light to nearby squares.
		// TODO magic number 0.150f matches 0.050f + 0.100f from SHELLED.
		addFlash(group, square, 0.20f, delay + 0.150f, 0.100f, 0.300f);

		if (group)
		{
			if (auto camerafocus = group->camerafocus.lock())
			{
				camerafocus->shake(group, 2, 0.020, 0.080f, delay + 0.250f);
			}

			// queue the audio
			Mixer::get()->queue(Clip::Type::EXPLOSION, delay + 0.250f, _point);
		}
	}
	else if (_formtype == FormType::BALLOON)
	{
		bool horizontal;
		{
			const char* tag = _sprite->getTag().c_str();
			horizontal = (strncmp(tag, "East Xyz", 4) == 0
				||        strncmp(tag, "West Xyz", 4) == 0);
		}

		// Zeppelins have no death animation, so we just use explosions instead.
		xOffset = 0;
		int height = rand() % 4 + 8 + Surface::HEIGHT / 3;
		_particlebuffer.add(Particle::explosion(group,
			&_point, xOffset, _yahooOffset + 1, height, delay + 0.150f));
		xOffset = -3 - 5 * horizontal;
		height = rand() % 4 + 8 + Surface::HEIGHT / 3 + 3 * (!horizontal);
		_particlebuffer.add(Particle::explosion(group,
			&_point, xOffset, _yahooOffset + 1, height, delay + 0.200f));
		xOffset =  3 + 5 * horizontal;
		height = rand() % 4 + 8 + Surface::HEIGHT / 3 - 3 * (!horizontal);
		_particlebuffer.add(Particle::explosion(group,
			&_point, xOffset, _yahooOffset + 1, height, delay + 0.200f));
		xOffset = rand() % 5 - 2;
		height = rand() % 4 + 5 + Surface::HEIGHT / 3;
		_particlebuffer.add(Particle::explosion(group,
			&_point, xOffset, _yahooOffset + 1, height, delay + 0.300f));
		xOffset = rand() % 5 - 2;
		height = rand() % 4 + 2 + Surface::HEIGHT / 3;
		_particlebuffer.add(Particle::explosion(group,
			&_point, xOffset, _yahooOffset + 1, height, delay + 0.400f));
		xOffset = rand() % 5 - 2;
		height = rand() % 4 - 2 + Surface::HEIGHT / 3;
		_particlebuffer.add(Particle::explosion(group,
			&_point, xOffset, _yahooOffset + 1, height, delay + 0.500f));
		xOffset = rand() % 5 - 2;
		height = rand() % 4 - 6 + Surface::HEIGHT / 3;
		_particlebuffer.add(Particle::explosion(group,
			&_point, xOffset, _yahooOffset + 1, height, delay + 0.600f));

		// The zeppelin disappears at the height of the explosions: after 250ms + 200ms.
		triggerDeathAnimation(group, delay + 0.250f + 0.200f);

		// Add a flash of light to nearby squares.
		// TODO magic number 0.150f matches 0.050f + 0.100f from SHELLED.
		addFlash(group, square, 0.30f, delay + 0.150f, 0.100f, 0.300f);

		if (group)
		{
			if (auto camerafocus = group->camerafocus.lock())
			{
				camerafocus->shake(group, 4, 0.020, 0.480f, delay + 0.250f);
			}

			// queue the audio
			Mixer::get()->queue(Clip::Type::EXPLOSION, delay + 0.250f, _point);
		}
	}
	else if (_formtype == FormType::BUILDING)
	{
		// Rubble particles at the bottom of the building.
		xOffset = -2;
		_particlebuffer.add(Particle::rubble(group,
			&_point, xOffset, _yahooOffset + 1, 0, delay + 0.100f));
		xOffset = 2;
		_particlebuffer.add(Particle::rubble(group,
			&_point, xOffset, _yahooOffset + 1, 0, delay + 0.100f));
		xOffset = -2;
		_particlebuffer.add(Particle::rubble(group,
			&_point, xOffset, _yahooOffset + 1, 0, delay + 0.200f));
		xOffset = 2;
		_particlebuffer.add(Particle::rubble(group,
			&_point, xOffset, _yahooOffset + 1, 0, delay + 0.200f));

		// Collapse animation.
		triggerDeathAnimation(group, delay + 0.300f);

		// queue the audio
		if (group) Mixer::get()->queue(Clip::Type::COLLAPSE, delay + 0.300f, _point);
	}
	else if (_formtype == FormType::TREE)
	{
		// Trees have no death animation, so we just use falling bark instead.
		xOffset = 0;
		int height = 0;
		_particlebuffer.add(Particle::barkflake(group,
			&_point, xOffset, _yahooOffset + 1, height, delay + 0.100f));
		xOffset = -2;
		_particlebuffer.add(Particle::barkflake(group,
			&_point, xOffset, _yahooOffset + 1, height, delay + 0.150f));
		xOffset = 2;
		_particlebuffer.add(Particle::barkflake(group,
			&_point, xOffset, _yahooOffset + 1, height, delay + 0.200f));

		// Disappear at the height of the explosion animation.
		triggerInvisibleDeathAnimation(group, delay + 0.200f);

		// queue the audio
		if (group) Mixer::get()->queue(Clip::Type::TREEFELL, delay, _point);
	}
	else if (_formtype == FormType::FENCE)
	{
		// Collapse animation.
		triggerDeathAnimation(group, delay + 0.300f);
	}
	else
	{
		triggerDefaultDeathAnimation(group, delay);
	}
}

void Figure::displace(const Change& change,
		std::shared_ptr<AnimationGroup> group, float delay)
{
	Move direction;
	float amount = 1.45f;
	float duration = 0.050f;

	switch (change.type)
	{
		case Change::Type::ATTACKED:
		{
			direction = Aim(change.attacker.position, change.subject.position).direction();
		}
		break;

		case Change::Type::SHELLED:
		{
			amount = 2.45;
			direction = Aim(change.attacker.position, change.subject.position).direction();
		}
		break;

		case Change::Type::TRAMPLED:
		{
			direction = Move::X;
		}
		break;

		case Change::Type::BOMBARDED:
		case Change::Type::BOMBED:
		{
			amount = 2.45;
			direction = Move::X;
		}
		break;

		case Change::Type::FROSTBITTEN:
		case Change::Type::BURNED:
		case Change::Type::GASSED:
		case Change::Type::IRRADIATED:
		{
			direction = Move::X;
		}
		break;

		default:
		{
			LOGW << "Missing case for '" << change.type << "'";
			DEBUG_ASSERT(false);
			return;
		}
		break;
	}

	if (direction == Move::X)
	{
		switch (rand() % 2)
		{
			case 0: direction = Move::E; break;
			case 1: direction = Move::W; break;
		}
	}

	switch (direction)
	{
		case Move::E:
		{
			transition(group, TRANSITION_DISPLACE_X, 0 + amount, duration, delay);
			transition(group, TRANSITION_DISPLACE_X, 0, duration, delay + duration);
		}
		break;

		case Move::S:
		{
			transition(group, TRANSITION_DISPLACE_Y, 0 + amount, duration, delay);
			transition(group, TRANSITION_DISPLACE_Y, 0, duration, delay + duration);
		}
		break;

		case Move::W:
		{
			transition(group, TRANSITION_DISPLACE_X, 0 - amount, duration, delay);
			transition(group, TRANSITION_DISPLACE_X, 0, duration, delay + duration);
		}
		break;

		case Move::N:
		{
			transition(group, TRANSITION_DISPLACE_Y, 0 - amount, duration, delay);
			transition(group, TRANSITION_DISPLACE_Y, 0, duration, delay + duration);
		}
		break;

		case Move::X:
		break;
	}

	transition(group, TRANSITION_STUN, 1, 0.0f, delay);
	transition(group, TRANSITION_STUN, 0, 2 * duration, delay + 0.020f);
}

void Figure::animateMood(const Change& change,
		std::shared_ptr<AnimationGroup> group, float delay)
{
	int height = _sprite->height();
	if (change.power > 0)
	{
		switch (_powertype)
		{
			case PowerType::LABOR:
			{
				_particlebuffer.add(Particle::workup(group,
					&_point, 0, _yahooOffset + 1, height, delay));
			}
			break;

			case PowerType::ENERGY:
			{
				_particlebuffer.add(Particle::boltup(group,
					&_point, 0, _yahooOffset + 1, height, delay));
			}
			break;

			case PowerType::HEART:
			default:
			{
				_particlebuffer.add(Particle::heartup(group,
					&_point, 0, _yahooOffset + 1, height, delay));
			}
			break;
		}

		// queue the audio
		if (group) Mixer::get()->queue(Clip::Type::HEART, delay, _point);
	}
	else
	{
		switch (_powertype)
		{
			case PowerType::LABOR:
			{
				_particlebuffer.add(Particle::workdown(group,
					&_point, 0, _yahooOffset + 1, height, delay));
			}
			break;

			case PowerType::ENERGY:
			{
				_particlebuffer.add(Particle::boltdown(group,
					&_point, 0, _yahooOffset + 1, height, delay));
			}
			break;

			case PowerType::HEART:
			default:
			{
				_particlebuffer.add(Particle::heartdown(group,
					&_point, 0, _yahooOffset + 1, height, delay));
			}
			break;
		}

		// queue the audio
		if (group) Mixer::get()->queue(Clip::Type::NOHEART, delay, _point);
	}
}

bool Figure::isLethal(const Change& change)
{
	switch (change.type)
	{
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
			return change.killed;
		}
		break;

		case Change::Type::DIED:
		case Change::Type::DESTROYED:
		case Change::Type::TRANSFORMED:
		case Change::Type::CONSUMED:
		case Change::Type::SHAPED:
		case Change::Type::SETTLED:
		case Change::Type::EXPANDED:
		case Change::Type::UPGRADED:
		case Change::Type::CULTIVATED:
		{
			return true;
		}
		break;

		default:
		{
			return false;
		}
		break;
	}
}

void Figure::triggerDefaultDeathAnimation(std::shared_ptr<AnimationGroup> group, float delay)
{
	_chilled = false;
	setTrigger(group, "Death", delay);
	addAnimation(Animation(group, [this](float) {

		lock(TRANSITION_MOVE_X);
		lock(TRANSITION_MOVE_Y);
		_sprite->setFinal(true);
		_footprint.deselect();

	}, 0, delay));
}

void Figure::triggerInvisibleDeathAnimation(std::shared_ptr<AnimationGroup> group, float delay)
{
	_chilled = false;
	setVisible(group, false, delay);
	addAnimation(Animation(group, [this](float) {

		_footprint.deselect();

	}, 0, delay));
}

void Figure::triggerDeathAnimation(std::shared_ptr<AnimationGroup> group, float delay)
{
	_chilled = false;
	setTrigger(group, "Death", delay);
	if (_placementbox.entrenched())
	{
		setTrigger(group, "Death Trench", delay);
	}
	addAnimation(Animation(group, [this](float) {

		lock(TRANSITION_MOVE_X);
		lock(TRANSITION_MOVE_Y);
		_sprite->setFinal(true);
		_footprint.deselect();

	}, 0, delay));

	if (group)
	{
		// queue the audio
		if (_deathSound) Mixer::get()->queue(Clip::Type::SPLASH, delay, _point);
	}
}

void Figure::triggerDeathHeadshotAnimation(std::shared_ptr<AnimationGroup> group, float delay)
{
	_chilled = false;
	setTrigger(group, "Death", delay);
	setTrigger(group, "Death Headshot", delay);
	if (_placementbox.entrenched())
	{
		setTrigger(group, "Death Trench Headshot", delay);
	}
	addAnimation(Animation(group, [this](float) {

		lock(TRANSITION_MOVE_X);
		lock(TRANSITION_MOVE_Y);
		_sprite->setFinal(true);
		_footprint.deselect();

	}, 0, delay));

	if (group)
	{
		// queue the audio
		if (_deathSound) Mixer::get()->queue(Clip::Type::SPLASH, delay, _point);
	}
}

void Figure::triggerDeathExplodeAnimation(std::shared_ptr<AnimationGroup> group, float delay)
{
	_chilled = false;
	setTrigger(group, "Death", delay);
	setTrigger(group, "Death Explode", delay);
	if (_placementbox.entrenched())
	{
		setTrigger(group, "Death Trench Explode", delay);
	}
	addAnimation(Animation(group, [this](float) {

		lock(TRANSITION_MOVE_X);
		lock(TRANSITION_MOVE_Y);
		_sprite->setFinal(true);
		_footprint.deselect();

	}, 0, delay));

	if (group)
	{
		// queue the audio
		if (_deathSound) Mixer::get()->queue(Clip::Type::SPLASH, delay, _point);
	}
}

void Figure::animate(const Change& change,
	std::shared_ptr<AnimationGroup> group, Square* square)
{
	float delay = group ? group->delay : 0;
	switch (change.type)
	{
		case Change::Type::DIED:
		{
			impact(change, group, square, delay);
			set(TRANSITION_IMPACT, 0);
			transition(group, TRANSITION_IMPACT, 1, 0.100f, delay);
			transition(group, TRANSITION_IMPACT, 0, 0.100f, delay + 0.100f);

			// Add a bit of hitstop.
			// TODO magic number 0.100f matches the same above
			addHitstop(group, delay + 0.100f);
		}
		break;

		case Change::Type::TRANSFORMED:
		case Change::Type::CONSUMED:
		case Change::Type::SHAPED:
		case Change::Type::SETTLED:
		case Change::Type::EXPANDED:
		case Change::Type::UPGRADED:
		case Change::Type::CULTIVATED:
		{
			impact(change, group, square, delay);
		}
		break;

		case Change::Type::DESTROYED:
		{
			impact(change, group, square, delay);

			// Add a bit of hitstop.
			addHitstop(group, delay);
		}
		break;

		case Change::Type::AIMS:
		{
			switch (change.notice)
			{
				case Notice::ACTIVEATTACK:
				case Notice::RETALIATIONATTACK:
				case Notice::FOCUSATTACK:
				break;

				case Notice::TRIGGEREDFOCUSATTACK:
				case Notice::TRIGGEREDLOCKDOWNATTACK:
				case Notice::OPPORTUNITYATTACK:
				{
					int height = 14;
					_particlebuffer.add(Particle::exclamation(group, &_point,
						0, _yahooOffset + 1, height, delay));
				}
				break;

				default:
				break;
			}

			switch (Aim(change.subject.position, change.target.position).direction())
			{
				case Move::E:
				{
					setTrigger(group, "East Aim", "East Aim", delay);
					if (_placementbox.entrenched())
					{
						setTrigger(group, "East Trench Aim",
							"East Trench Aim", delay);
					}
				}
				break;
				case Move::S:
				{
					setTrigger(group, "South Aim", "South Aim", delay);
					if (_placementbox.entrenched())
					{
						setTrigger(group, "South Trench Aim",
							"South Trench Aim", delay);
					}
				}
				break;
				case Move::W:
				{
					setTrigger(group, "West Aim", "West Aim", delay);
					if (_placementbox.entrenched())
					{
						setTrigger(group, "West Trench Aim",
							"West Trench Aim", delay);
					}
				}
				break;
				case Move::N:
				{
					setTrigger(group, "North Aim", "North Aim", delay);
					if (_placementbox.entrenched())
					{
						setTrigger(group, "North Trench Aim",
							"North Trench Aim", delay);
					}
				}
				break;
				case Move::X: break;
			}

			if (group)
			{
				// Before any ATTACKS changes, players that see the Attacker
				// see all its figures aiming near-simultaneously.
				delay = (group->delay += 0.025f);
			}
		}
		break;

		case Change::Type::ATTACKS:
		{
			switch (Aim(change.subject.position, change.target.position).direction())
			{
				case Move::E:
				{
					setTrigger(group, "East Shoot", "East Idle", delay);
					if (_placementbox.entrenched())
					{
						setTrigger(group, "East Trench Shoot",
							"East Trench Idle", delay);
					}
				}
				break;
				case Move::S:
				{
					setTrigger(group, "South Shoot", "South Idle", delay);
					if (_placementbox.entrenched())
					{
						setTrigger(group, "South Trench Shoot",
							"South Trench Idle", delay);
					}
				}
				break;
				case Move::W:
				{
					setTrigger(group, "West Shoot", "West Idle", delay);
					if (_placementbox.entrenched())
					{
						setTrigger(group, "West Trench Shoot",
							"West Trench Idle", delay);
					}
				}
				break;
				case Move::N:
				{
					setTrigger(group, "North Shoot", "North Idle", delay);
					if (_placementbox.entrenched())
					{
						setTrigger(group, "North Trench Shoot",
							"North Trench Idle", delay);
					}
				}
				break;
				case Move::X: break;
			}

			// Add a flash of light to nearby squares.
			addFlash(group, square, 0.05f, delay - 0.050f, 0.050f, 0.050f);

			if (group)
			{
				// queue the audio
				Mixer::get()->queue(Clip::Type::GUNSHOT, delay, _point);

				// The ATTACKS change determines the time between attacks.
				// In case there is no ATTACKS change, the ATTACKED change will
				// determine the time between attacks instead.
				// The magic number 180 is slightly less than the duration of
				// the gunners attack animation, which is 200ms, which prevents
				// the idle animation from appearing in between shots.
				delay = (group->delay += 0.180f);
				group->attacker = true;
			}
		}
		break;

		case Change::Type::ATTACKED:
		{
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
			displace(change, group, impacttime);
			impact(change, group, square, impacttime);
			set(TRANSITION_IMPACT, 0);
			transition(group, TRANSITION_IMPACT, 1, 0.050f, impacttime);
			transition(group, TRANSITION_IMPACT, 0, 0.050f, impacttime + 0.050f);

			// Add a bit of hitstop.
			// TODO magic number 0.050f matches the same above
			addHitstop(group, impacttime + 0.050f);
		}
		break;

		case Change::Type::TRAMPLES:
		break;

		case Change::Type::TRAMPLED:
		{
			displace(change, group, delay);
			impact(change, group, square, delay);
			set(TRANSITION_IMPACT, 0);
			transition(group, TRANSITION_IMPACT, 1, 0.100f, delay);
			transition(group, TRANSITION_IMPACT, 0, 0.100f, delay + 0.100f);
			if (group)
			{
				if (auto camerafocus = group->camerafocus.lock())
				{
					camerafocus->shake(group, 1, 0.100f, 0.100f, delay);
				}
			}
			if (group) group->delay += 0.200f;
		}
		break;

		case Change::Type::SHELLS:
		{
			// We check for a personal delay because there might have been
			// a second volley.
			if (group)
			{
				personalDelay(group);
				delay = group->delay;
			}

			switch (Aim(change.subject.position, change.target.position).direction())
			{
				case Move::E:
				{
					_sprite->setTag("East Idle");
					setTrigger(group, "East Shell", delay);
				}
				break;
				case Move::S:
				{
					_sprite->setTag("South Idle");
					setTrigger(group, "South Shell", delay);
				}
				break;
				case Move::W:
				{
					_sprite->setTag("West Idle");
					setTrigger(group, "West Shell", delay);
				}
				break;
				case Move::N:
				{
					_sprite->setTag("North Idle");
					setTrigger(group, "North Shell", delay);
				}
				break;
				case Move::X: break;
			}

			// Fire time is 100ms after the attack animation starts (because of aiming).
			float firetime = delay + 0.100f;
			if (group)
			{
				if (auto camerafocus = group->camerafocus.lock())
				{
					camerafocus->shake(group, 1, 0.020f, 0.030f, firetime);
				}

				// queue the audio
				Mixer::get()->queue(Clip::Type::GUNSHOT, delay, _point);

				// The SHELLS change determines the time between attacks.
				// In case there is no SHELLS change, the SHELLED change will
				// determine the time between attacks instead.
				delay = (group->delay += 0.400f);
				group->attacker = true;

				// We add a personal delay because there might be
				// a second volley.
				personalDelay(group, group->delay + 0.400f + 1.000f);
			}
		}
		break;

		case Change::Type::SHELLED:
		{
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
			displace(change, group, impacttime);
			impact(change, group, square, impacttime);
			set(TRANSITION_IMPACT, 0);
			transition(group, TRANSITION_IMPACT, 1, 0.050f, impacttime);
			transition(group, TRANSITION_IMPACT, 0, 0.050f, impacttime + 0.050f);

			// Add a flash of light to nearby squares.
			// TODO magic number 0.050f matches the same above
			addFlash(group, square, 0.20f, impacttime, 0.050f, 0.100f);

			// Add a bit of hitstop.
			// TODO magic number 0.050f matches the same above
			addHitstop(group, impacttime + 0.050f);

			if (group)
			{
				if (auto camerafocus = group->camerafocus.lock())
				{
					camerafocus->shake(group, 2, 0.020f, 0.180f, impacttime);
				}

				// queue the audio
				Mixer::get()->queue(Clip::Type::EXPLOSION, impacttime, _point);
			}
		}
		break;

		case Change::Type::BOMBARDS:
		{
			setTrigger(group, "Bombard", delay);
			if (_placementbox.entrenched()) setTrigger(group, "Bombard Trench", delay);

			// Fire time is 500ms after the attack animation starts (because of aiming).
			float firetime = delay + 0.500f;
			if (group)
			{
				if (auto camerafocus = group->camerafocus.lock())
				{
					camerafocus->shake(group, 1, 0.020f, 0.010f, firetime);
				}

				// queue the audio
				Mixer::get()->queue(Clip::Type::GUNSHOT, firetime, _point);
				Mixer::get()->queue(Clip::Type::UP, firetime, _point);

				delay = (group->delay += 0.500f + 0.500f);
			}
		}
		break;

		case Change::Type::BOMBARDED:
		{
			if (group)
			{
				// queue the audio
				Mixer::get()->queue(Clip::Type::DOWN, delay, _point);

				delay = (group->delay += 0.500f);
			}

			displace(change, group, delay);
			impact(change, group, square, delay);
			set(TRANSITION_IMPACT, 0);
			transition(group, TRANSITION_IMPACT, 1, 0.050f, delay);
			transition(group, TRANSITION_IMPACT, 0, 0.050f, delay + 0.050f);

			// Add a flash of light to nearby squares.
			// TODO magic number 0.050f matches the same above
			addFlash(group, square, 0.20f, delay, 0.050f, 0.100f);

			// Add a bit of hitstop.
			// TODO magic number 0.050f matches the same above
			addHitstop(group, delay + 0.050f);

			if (group)
			{
				if (auto camerafocus = group->camerafocus.lock())
				{
					camerafocus->shake(group, 2, 0.020f, 0.180f, delay);
				}

				// queue the audio
				Mixer::get()->queue(Clip::Type::EXPLOSION, delay, _point);
			}

			pause(group, 0.200f);
			if (group) group->delay += 0.400f;
		}
		break;

		case Change::Type::BOMBS:
		{
			setTrigger(group, "Bomb", delay);
			if (group) delay = (group->delay += 0.100f);
			int height = 0;
			_particlebuffer.add(Particle::gasburst(group,
				&_point, 0, _yahooOffset + 1, height, delay));
		}
		break;

		case Change::Type::BOMBED:
		{
			displace(change, group, delay);
			impact(change, group, square, delay);
			set(TRANSITION_IMPACT, 0);
			transition(group, TRANSITION_IMPACT, 1, 0.050f, delay);
			transition(group, TRANSITION_IMPACT, 0, 0.050f, delay + 0.050f);

			// Add a flash of light to nearby squares.
			// TODO magic number 0.050f matches the same above
			addFlash(group, square, 0.20f, delay, 0.050f, 0.100f);

			// Add a bit of hitstop.
			// TODO magic number 0.050f matches the same above
			addHitstop(group, delay + 0.050f);

			if (group)
			{
				if (auto camerafocus = group->camerafocus.lock())
				{
					camerafocus->shake(group, 2, 0.020f, 0.180f, delay);
				}
			}

			pause(group, 0.200f);
			if (group) group->delay += 0.400f;
		}
		break;

		case Change::Type::FROSTBITTEN:
		{
			Mixer::get()->queue(Clip::Type::FROSTHURT, delay, _point);

			displace(change, group, delay);
			impact(change, group, square, delay);
			set(TRANSITION_IMPACT, 0);
			transition(group, TRANSITION_IMPACT, 1, 0.050f, delay);
			transition(group, TRANSITION_IMPACT, 0, 0.050f, delay + 0.050f);

			// Add a bit of hitstop.
			// TODO magic number 0.050f matches the same above
			addHitstop(group, delay + 0.050f, 0.5f);

			pause(group, 0.200f);
			if (group) group->delay += 0.350f;
		}
		break;

		case Change::Type::BURNED:
		{
			displace(change, group, delay);
			impact(change, group, square, delay);
			set(TRANSITION_IMPACT, 0);
			transition(group, TRANSITION_IMPACT, 1, 0.050f, delay);
			transition(group, TRANSITION_IMPACT, 0, 0.050f, delay + 0.050f);

			// Add a flash of light to nearby squares.
			// TODO magic number 0.050f matches the same above
			addFlash(group, square, 0.10f, delay, 0.050f, 0.100f);

			// Add a bit of hitstop.
			// TODO magic number 0.050f matches the same above
			addHitstop(group, delay + 0.050f, 0.5f);

			pause(group, 0.200f);
			if (group) group->delay += 0.350f;
		}
		break;

		case Change::Type::GASSED:
		{
			displace(change, group, delay);
			impact(change, group, square, delay);
			set(TRANSITION_IMPACT, 0);
			transition(group, TRANSITION_IMPACT, 1, 0.050f, delay);
			transition(group, TRANSITION_IMPACT, 0, 0.050f, delay + 0.050f);

			// Add a bit of hitstop.
			// TODO magic number 0.050f matches the same above
			addHitstop(group, delay + 0.050f, 0.5f);

			pause(group, 0.200f);
			if (group) group->delay += 0.350f;
		}
		break;

		case Change::Type::IRRADIATED:
		{
			displace(change, group, delay);
			impact(change, group, square, delay);
			set(TRANSITION_IMPACT, 0);
			transition(group, TRANSITION_IMPACT, 1, 0.050f, delay);
			transition(group, TRANSITION_IMPACT, 0, 0.050f, delay + 0.050f);

			// Add a bit of hitstop.
			// TODO magic number 0.050f matches the same above
			addHitstop(group, delay + 0.050f, 0.5f);

			pause(group, 0.200f);
			if (group) group->delay += 0.350f;
		}
		break;

		case Change::Type::SNOW:
		{
			if (change.snow)
			{
				transition(group, TRANSITION_SNOW, 1, 1.000f);
			}
			else
			{
				transition(group, TRANSITION_SNOW, 0, 1.000f);
			}
		}
		break;

		case Change::Type::REVEAL:
		case Change::Type::FROSTBITE:
		{
			if (change.frostbite && group && group->coldfeet)
			{
				_chilled = true;
			}
			else if (!change.frostbite)
			{
				_chilled = false;
			}
		}
		break;

		case Change::Type::FIRESTORM:
		break;

		case Change::Type::BONEDROUGHT:
		break;

		case Change::Type::DEATH:
		break;

		case Change::Type::GAS:
		case Change::Type::RADIATION:
		case Change::Type::TEMPERATURE:
		break;

		case Change::Type::HUMIDITY:
		{
			float val = (float) square->humidity();
			transition(group, TRANSITION_HUMIDITY, val, 1);
		}
		break;

		case Change::Type::GROWS:
		{
			if (group && (group->player == _player || group->player == Player::OBSERVER))
			{
				animateMood(change, group, delay);
			}
		}
		break;

		case Change::Type::INCOME:
		{
			int height = _sprite->height();

			_particlebuffer.add(Particle::income(group,
				&_point, 0, _yahooOffset + 1, height, delay));

			if (group)
			{
				// queue the audio
				Mixer::get()->queue(Clip::Type::COIN, delay, _point);
			}
		}
		break;

		case Change::Type::SCORED:
		{
			int height = _sprite->height();
			_particlebuffer.add(Particle::diamondup(group,
				&_point, 0, _yahooOffset + 1, height, delay));

			if (group)
			{
				// queue the audio
				Mixer::get()->queue(Clip::Type::COIN, delay, _point);
			}
		}
		break;

		default:
		{
			LOGW << "Missing case \"" + std::string(Change::stringify(change.type))
				+ "\" in Figure::enact";
		}
		break;
	}
}

PlacementBox& Figure::box()
{
	return _placementbox;
}

const PlacementBox& Figure::box() const
{
	return _placementbox;
}

Point Figure::getSpawnPoint()
{
	return _point;
}

void Figure::setSpawnPoint(const Point& spawnpoint)
{
	_footprint.point().xenon = _point.xenon = spawnpoint.xenon;
	_footprint.point().yahoo = _point.yahoo = spawnpoint.yahoo - 2;
	_transitions[TRANSITION_MOVE_X] = _point.xenon;
	_transitions[TRANSITION_MOVE_Y] = _point.yahoo;
	_lastDestination = _point;
}

float Figure::spawnMoving(std::shared_ptr<AnimationGroup> group,
	float speed)
{
	Point destination(_point);
	destination.yahoo += 5 * Camera::get()->scale();
	return move(group, group ? group->delay : 0, Move::X, speed, destination);
}

void Figure::spawn(const Change& change,
	std::shared_ptr<AnimationGroup> group, float delay)
{
	if (!group) return;

	switch (change.type)
	{
		case Change::Type::PRODUCED:
		{
			switch (_formtype)
			{
				case FormType::VEHICLE:
				{
					// queue the audio
					Mixer::get()->queue(Clip::Type::ENGINE, delay, _point);
				}
				break;

				case FormType::BALLOON:
				{
					// TODO zeppelin spawn sound effect
				}
				break;

				case FormType::PERSON:
				{
					// queue the audio
					Mixer::get()->queue(Clip::Type::FOOTSTEP, delay, _point);
					Mixer::get()->queue(Clip::Type::FOOTSTEP, delay + 0.1, _point);
					Mixer::get()->queue(Clip::Type::FOOTSTEP, delay + 0.2, _point);
				}
				break;

				default:
				break;
			}
		}
		break;

		case Change::Type::UPGRADED:
		{
			for (int i = 0; i < 2; i++)
			{
				int xOffset = -6 + 12 * i;
				int height = -2;
				_particlebuffer.add(Particle::dustflame(group,
					&_point, xOffset, _yahooOffset + 1, height, delay));
			}
		}
		break;

		default:
		break;
	}
}

void Figure::prepareForReposition()
{
	_placementbox.release();
}

void Figure::die()
{
	_placementbox.release();
	_dying = true;
}

void Figure::revive()
{
	_dying = false;
}

void Figure::idle(float delay)
{
	int height = 14;
	_particlebuffer.add(Particle::question(nullptr,
		&_point, 0, _yahooOffset + 1, height, delay));
}

void Figure::setBorder()
{
	_sprite->setBorder(true);
}

std::shared_ptr<Skin> Figure::missingSkin()
{
	auto skinptr = std::make_shared<Skin>();
	Skin& skin = *skinptr;
	skin.name = "figures/missing";
	skin.spritename = "figures/missing";
	skin.selectionspritename = "ui/footprint_unit12";
	skin.formtype = FormType::PERSON;
	skin.yahooOffset = 1;
	skin._palettedata = {
		Paint::blend(Color::broken(), ColorName::SHINEBLEND),
		Color::broken(),
		Paint::blend(Color::broken(), ColorName::SHADEBLEND),
		ColorName::UNITSHADOW,
		Color::transparent(),
	};
	skin.palettes = {skin._palettedata.data()};
	skin.palettenames = {"basic"};
	skin.palettesize = skin._palettedata.size();
	return skinptr;
}
