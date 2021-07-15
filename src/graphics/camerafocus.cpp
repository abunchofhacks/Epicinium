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
#include "camerafocus.hpp"
#include "source.hpp"

#include "camera.hpp"
#include "input.hpp"
#include "loop.hpp"
#include "settings.hpp"
#include "mixer.hpp"


CameraFocus::CameraFocus(const Settings& settings, int measure) :
	_settings(settings),
	_measure(measure)
{}

void CameraFocus::load(const Point& topleft, const Point& bottomright)
{
	_scrollingWasd = _settings.cameraScrollingWasd.value();
	_scrollingArrows = _settings.cameraScrollingArrows.value();
	_scrollingEdge = _settings.cameraScrollingEdge.value();

	_panningAudibleId = Mixer::get()->loop(Clip::Type::UI_PANNING, 0.0f);

	Point center;
	center.xenon = (topleft.xenon + bottomright.xenon) / 2;
	center.yahoo = (topleft.yahoo + bottomright.yahoo) / 2;
	_topleftLimit.xenon = std::min(topleft.xenon + 2 * _measure, center.xenon);
	_topleftLimit.yahoo = std::min(topleft.yahoo + 2 * _measure, center.yahoo);
	_bottomrightLimit.xenon = std::max(bottomright.xenon - 2 * _measure,
		center.xenon);
	_bottomrightLimit.yahoo = std::max(bottomright.yahoo - 2 * _measure,
		center.yahoo);
}

static inline int randomshakeamount(float shake)
{
	int sign = ((rand() % 2) * 2 - 1);
	int lower = std::max(0, (int) shake);
	int upper = lower + 1;
	int permille = rand() % (upper * 1000);
	int amount = permille / 1000;
	if (permille > lower * 1000)
	{
		amount += ((permille % 1000) < (shake - lower) * 1000);
	}
	return sign * amount;
}

/* Converts WASD keyboard inputs to radians */
inline double wasdAngle(int8_t upminusdown, int8_t rightminusleft)
{
	return atan2(-upminusdown, rightminusleft);
}

void CameraFocus::update()
{
	Animator::update();

	Camera* camera = Camera::get();
	Input* input = Input::get();
	float delta = std::min(Loop::delta(), 0.1f);
	bool moving = false;

	if (_panning)
	{
		float spd = sqrt(_hspeed * _hspeed + _vspeed * _vspeed);

		// This is some nifty algebra. I'm writing this comment after the fact,
		// so I don't know how I got to this equation, but if correct then
		// the range is the total distance travelled if we start slowing down.
		float range = 0.5f * spd * spd / _decSpeed;

		// If distance is below 0.05f, it's basically 0.
		if (_target.distanceTo(_point) < std::max(0.05f, range))
		{
			_panning = false;
		}
		else
		{
			moving = true;
			float umd = _target.yahoo - _point.yahoo;
			float rml = _target.xenon - _point.xenon;
			float direction = atan2(umd, rml);
			_hspeed += _accSpeed * delta * cos(direction);
			_vspeed += _accSpeed * delta * sin(direction);
		}
	}

	if ((!_acting || _settings.allowScrollingWhileActing.value())
		&& (!_panning || _settings.allowScrollingWhilePanning.value())
		&& !input->isKeyHeld(SDL_SCANCODE_CTRL))
	{
		Pixel mousepixel = input->mousePixel();
		bool up =    (_keysEnabled && _scrollingWasd   && input->isKeyHeld(SDL_SCANCODE_W))
				||   (_keysEnabled && _scrollingArrows && input->isKeyHeld(SDL_SCANCODE_UP))
				||   (_scrollingEdge   && mousepixel.yahoo < 3);
		bool left =  (_keysEnabled && _scrollingWasd   && input->isKeyHeld(SDL_SCANCODE_A))
				||   (_keysEnabled && _scrollingArrows && input->isKeyHeld(SDL_SCANCODE_LEFT))
				||   (_scrollingEdge   && mousepixel.xenon < 3);
		bool down =  (_keysEnabled && _scrollingWasd   && input->isKeyHeld(SDL_SCANCODE_S))
				||   (_keysEnabled && _scrollingArrows && input->isKeyHeld(SDL_SCANCODE_DOWN))
				|| (_scrollingEdge && mousepixel.yahoo > camera->height() - 3);
		bool right = (_keysEnabled && _scrollingWasd   && input->isKeyHeld(SDL_SCANCODE_D))
				||   (_keysEnabled && _scrollingArrows && input->isKeyHeld(SDL_SCANCODE_RIGHT))
				|| (_scrollingEdge && mousepixel.xenon > camera->width() - 3);
		if (up    && _point.yahoo <= _topleftLimit.yahoo)         up    = false;
		if (left  && _point.xenon <= _topleftLimit.xenon)         left  = false;
		if (down  && _point.yahoo >= _bottomrightLimit.yahoo - 1) down  = false;
		if (right && _point.xenon >= _bottomrightLimit.xenon - 1) right = false;
		int8_t rml = right - left;
		int8_t umd = up - down;
		if (rml || umd)
		{
			moving = true;
			float direction = wasdAngle(umd, rml);
			_hspeed += _accSpeed * delta * cos(direction);
			_vspeed += _accSpeed * delta * sin(direction);
		}
	}

	float dir = atan2(_vspeed, _hspeed);
	float spd = sqrt(_hspeed * _hspeed + _vspeed * _vspeed);

	if (spd > _maxSpeed) spd = _maxSpeed;

	if (!moving)
	{
		spd -= _decSpeed * delta;
		if (spd < 0)
			spd = 0;
	}
	else if (_panning)
	{
		// This is some nifty algebra. See above.
		float range = 0.5f * spd * spd / _decSpeed;
		if (_target.distanceTo(_point) < std::max(0.05f, range))
		{
			_panning = false;

			// We do not want to overshoot.
			range = _target.distanceTo(_point);

			// Some more nifty algebra. We want to calculate the speed required
			// so we land exactly at the target if we stop moving now.
			spd = sqrt(_decSpeed * 2.0f * range);
		}
	}

	_hspeed = spd * cos(dir);
	_vspeed = spd * sin(dir);

	if (spd > 0)
	{
		_xenon += _hspeed * delta;
		if (_xenon < _topleftLimit.xenon)
			_xenon = _topleftLimit.xenon;
		if (_xenon > _bottomrightLimit.xenon - 1)
			_xenon = _bottomrightLimit.xenon - 1;

		_yahoo += _vspeed * delta;
		if (_yahoo < _topleftLimit.yahoo)
			_yahoo = _topleftLimit.yahoo;
		if (_yahoo > _bottomrightLimit.yahoo - 1)
			_yahoo = _bottomrightLimit.yahoo - 1;

		_point.xenon = (int) (_xenon + 0.5f);
		_point.yahoo = (int) (_yahoo + 0.5f);
	}

	if (Loop::tempo() < 0.5f)
	{
		// Keep camera offset the same.
	}
	else if (_shake > 0.001f)
	{
		camera->offset.xenon = randomshakeamount(camera->scale() * _shake);
		camera->offset.yahoo = randomshakeamount(camera->scale() * _shake);
	}
	else
	{
		camera->offset.xenon = 0;
		camera->offset.yahoo = 0;
	}

	camera->focus.xenon = _point.xenon;
	camera->focus.yahoo = _point.yahoo;

	Mixer::get()->fade(_panningAudibleId, (spd / _maxSpeed), 0.2f);

	/* IMGUI */
	static bool show = false;
	bool wasshown = show;

	if (Input::get()->isDebugKeyHeld())
	{
		if (ImGui::Begin("Windows", nullptr,
			ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse))
		{
			ImGui::Checkbox("Camera", &show);
		}
		ImGui::End();
	}

	if (show)
	{
		if (!wasshown) ImGui::SetNextWindowCollapsed(false);

		if (ImGui::Begin("Camera", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::PushItemWidth(80);
			ImGui::InputInt("Focus X", &(_point.xenon), 0, 0,
				ImGuiInputTextFlags_AutoSelectAll);
			ImGui::InputInt("Focus Y", &(_point.yahoo), 0, 0,
				ImGuiInputTextFlags_AutoSelectAll);
			ImGui::Value("Focus pixel-X", camera->_focusXenon);
			ImGui::Value("Focus pixel-Y", camera->_focusYahoo);
			ImGui::Value("Offset pixel-X", camera->offset.xenon);
			ImGui::Value("Offset pixel-Y", camera->offset.yahoo);
			ImGui::Value("Adjust pixel-X", camera->adjust.xenon);
			ImGui::Value("Adjust pixel-Y", camera->adjust.yahoo);
			ImGui::Value("Mouse X", Input::get()->mousePoint().xenon);
			ImGui::Value("Mouse Y", Input::get()->mousePoint().yahoo);
			ImGui::Value("Mouse pixel-X", Input::get()->mousePixel().xenon);
			ImGui::Value("Mouse pixel-Y", Input::get()->mousePixel().yahoo);
			ImGui::PopItemWidth();

			ImGui::Separator();
			ImGui::Checkbox("Move Camera with WASD", &_scrollingWasd);
			ImGui::Checkbox("Move Camera with Arrow Keys", &_scrollingArrows);
			ImGui::Checkbox("Move Camera with Edge of Screen", &_scrollingEdge);
			ImGui::DragFloat("Max. Speed", &_maxSpeed, 50.0f, 50.0f, 10000.0f);
			ImGui::DragFloat("Acc. Speed", &_accSpeed, 50.0f, 50.0f, 10000.0f);
			ImGui::DragFloat("Dec. Speed", &_decSpeed, 50.0f, 50.0f, 10000.0f);

			ImGui::Separator();
			ImGui::DragFloat("Camera Shake", &_shake, 0.25f, 0.0f, 10.0f);
		}
		ImGui::End();
	}
}

void CameraFocus::set(const Point& point)
{
	_hspeed = 0;
	_vspeed = 0;
	_panning = false;
	_target = point;
	_point = point;
	_xenon = _point.xenon;
	_yahoo = _point.yahoo;
}

void CameraFocus::pan(const Point& point)
{
	_hspeed = 0;
	_vspeed = 0;
	_target = point;
	_panning = (_point.distanceTo(_target) * 2 > _measure);
}

void CameraFocus::stop()
{
	_panning = false;
}

void CameraFocus::shake(std::shared_ptr<AnimationGroup> group,
	float amount, float durationSolid, float durationFalloff, float delay)
{
	float duration = durationSolid + durationFalloff;
	if (duration < 0.001f) return;

	float solid = durationSolid / duration;
	float falloff = std::max(1.0f - solid, 0.001f);

	addAnimation(Animation(group, [=](float progress) {

		if (progress <= solid) _shake = amount;
		else if (progress < 1.0f) _shake = amount * (1.0f - progress) / falloff;
		else _shake = 0.0f;
	}, duration, delay));
}
