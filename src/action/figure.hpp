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
#include "placementbox.hpp"
#include "footprint.hpp"
#include "descriptor.hpp"
#include "move.hpp"
#include "paint.hpp"
#include "player.hpp"

struct Change;
struct AnimationGroup;
class Square;
enum class FormType : uint8_t;
struct Skin;


enum class PowerType : uint8_t
{
	HEART,
	LABOR,
	ENERGY,
};

class Figure : public Actor
{
public:
	Figure(PlacementBox&& box, const Skin& skin, const Player& player,
		const PowerType& powertype);
	Figure(PlacementBox&& box, const Skin& skin, const Player& player);

	Figure(const Figure&) = delete;
	Figure(Figure&&) = delete;
	Figure& operator=(const Figure&) = delete;
	Figure& operator=(Figure&&) = delete;

	virtual ~Figure() = default;

protected:
	PlacementBox _placementbox;

	int _palettesize;
	const Paint* _basicPalette;
	const Paint* _powerPalette;
	const Paint* _dryPalette;
	const Paint* _snowPalette;
	std::array<const Paint*, PLAYER_MAX + 1> _teamPalettes;

	Paint _skintone;
	Player _player;
	Point _lastDestination;
	Footprint _footprint;
	std::shared_ptr<Sprite> _shadowsprite; // (unique ownership)
	int _yahooOffset;
	bool _selected;
	bool _dying;
	bool _moveGroup;
	bool _deathSound;
	FormType _formtype;
	PowerType _powertype;
	float _moved1;
	float _moved2;

	virtual void drawSprite() override;

	virtual void mix() override;

	void impact(const Change& change,
		std::shared_ptr<AnimationGroup> group, Square* square, float delay = 0);
	void moved(int distance);
	void moverumble(std::shared_ptr<AnimationGroup> group, float duration);
	void animateMood(const Change& change,
		std::shared_ptr<AnimationGroup> group, float delay = 0);

	static bool isLethal(const Change& change);

	float move(std::shared_ptr<AnimationGroup> group,
		Move direction, float speed, bool premoving);
	float move(std::shared_ptr<AnimationGroup> group,
		float delay, Move direction, float speed, Point destination);
	void displace(const Change& change,
		std::shared_ptr<AnimationGroup> group, float delay = 0);

	void triggerDefaultDeathAnimation(std::shared_ptr<AnimationGroup> group, float delay = 0);
	void triggerInvisibleDeathAnimation(std::shared_ptr<AnimationGroup> group, float delay = 0);
	void triggerDeathAnimation(std::shared_ptr<AnimationGroup> group, float delay = 0);
	void triggerDeathHeadshotAnimation(std::shared_ptr<AnimationGroup> group, float delay = 0);
	void triggerDeathExplodeAnimation(std::shared_ptr<AnimationGroup> group, float delay = 0);

	static Paint randomSkintone();
	static float introduceFlicker(float x);
	static float humidityTransitionCap(float fraction);

	void addFlash(std::shared_ptr<AnimationGroup> group,
		Square* square, float amount,
		float delay, float upduration, float downduration);

public:
	void update();

	bool hovered() const;

	void select();
	void deselect();

	void prepareForReposition();
	float doMoving(std::shared_ptr<AnimationGroup> group,
		PlacementBox&& box, Move direction, float speed);
	float startMoving(std::shared_ptr<AnimationGroup> group,
		PlacementBox&& box, Move direction, float speed);
	float finishMoving(std::shared_ptr<AnimationGroup> group,
		Move direction, float speed);
	float spawnMoving(std::shared_ptr<AnimationGroup> group,
		float speed);
	void reposition(std::shared_ptr<AnimationGroup> group,
		PlacementBox&& box);
	void setPlayer(const Player& player);
	void animate(const Change& change,
		std::shared_ptr<AnimationGroup> group, Square* square);

	PlacementBox& box();
	const PlacementBox& box() const;
	Point getSpawnPoint();
	void setSpawnPoint(const Point& spawnpoint);

	void spawn(const Change& change,
		std::shared_ptr<AnimationGroup> group, float delay = 0);

	void die();
	void revive();
	bool isDying() { return _dying; }

	void idle(float delay);

	void setBorder();

	static std::shared_ptr<Skin> missingSkin();
};
