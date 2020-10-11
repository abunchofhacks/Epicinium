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
#include "guide.hpp"
#include "source.hpp"

#include "cell.hpp"
#include "color.hpp"
#include "surface.hpp"
#include "sprite.hpp"
#include "collector.hpp"
#include "move.hpp"
#include "loop.hpp"
#include "paint.hpp"
#include "colorname.hpp"
#include "mixer.hpp"

Guide Guide::move(Cell index, const Move& move)
{
	static std::shared_ptr<Sprite::Palette> palette(new Sprite::Palette({
		Paint(ColorName::GUIDEMOVE)
	}));

	return Guide(index,
		std::make_shared<Sprite>(spritenameMove(move), palette),
		nullptr,
		/*shine=*/true, /*pulse*/false);
}

Guide Guide::slow(Cell index, const Move& move)
{
	static std::shared_ptr<Sprite::Palette> palette(new Sprite::Palette({
		Paint(ColorName::GUIDESLOWED)
	}));

	return Guide(index,
		std::make_shared<Sprite>(spritenameMove(move), palette),
		nullptr,
		/*shine=*/true, /*pulse*/false);
}

Guide Guide::postmove(Cell index, const Move& move)
{
	static std::shared_ptr<Sprite::Palette> palette(new Sprite::Palette({
		Paint(ColorName::GUIDEPOSTPONED)
	}));

	return Guide(index,
		std::make_shared<Sprite>(spritenameMove(move), palette),
		nullptr,
		/*shine=*/true, /*pulse*/false);
}

Guide Guide::order(Cell index, const char* stampspritename)
{
	static std::shared_ptr<Sprite::Palette> palette(new Sprite::Palette({
		Paint(ColorName::GUIDEORDER)
	}));

	Mixer::get()->queue(Clip::Type::PLANMENU, 0.25f);

	return Guide(index,
		std::make_shared<Sprite>("ui/cursor_hover", palette),
		std::make_shared<Sprite>(stampspritename, palette),
		/*shine=*/true, /*pulse=*/false);
}

Guide Guide::halt(Cell index)
{
	static std::shared_ptr<Sprite::Palette> palette(new Sprite::Palette({
		Paint(ColorName::GUIDEHALT)
	}));

	return Guide(index,
		std::make_shared<Sprite>("ui/cursor_hover", palette),
		std::make_shared<Sprite>("ui/ordericon_halt", palette),
		/*shine=*/true, /*pulse=*/false);
}

Guide Guide::validtarget(Cell index)
{
	static std::shared_ptr<Sprite::Palette> palette(new Sprite::Palette({
		Paint::alpha(ColorName::GUIDEVALIDTARGET, 255),
		Paint::alpha(ColorName::GUIDEVALIDTARGET, 255),
		Paint::alpha(ColorName::GUIDEVALIDTARGET,  55),
	}));

	return Guide(index,
		std::make_shared<Sprite>("ui/targetguide_valid", palette),
		nullptr,
		/*shine=*/false, /*pulse=*/true);
}

Guide Guide::invalidtarget(Cell index)
{
	static std::shared_ptr<Sprite::Palette> palette(new Sprite::Palette({
		Paint::alpha(ColorName::GUIDEINVALIDTARGET, 255),
		Paint::alpha(ColorName::GUIDEINVALIDTARGET, 255),
		Paint::alpha(ColorName::GUIDEINVALIDTARGET, 105),
	}));

	return Guide(index,
		std::make_shared<Sprite>("ui/targetguide_invalid", palette),
		nullptr,
		/*shine=*/false, /*pulse=*/true);
}

Guide Guide::skiptarget(Cell index)
{
	static std::shared_ptr<Sprite::Palette> palette(new Sprite::Palette({
		Paint(ColorName::GUIDEINVALIDTARGET)
	}));

	return Guide(index,
		std::make_shared<Sprite>("ui/targetguide_invalid", palette),
		nullptr,
		/*shine=*/true, /*pulse=*/false);
}

Guide Guide::frostbite(Cell index)
{
	static std::shared_ptr<Sprite::Palette> hoverpalette(new Sprite::Palette({
		Paint::alpha(ColorName::FROSTBITE, 10),
		Color::transparent(),
	}));

	return Guide(index,
		std::make_shared<Sprite>("ui/hazardguide", hoverpalette),
		nullptr,
		/*shine=*/false, /*pulse=*/false);
}

Guide Guide::firestorm(Cell index)
{
	static std::shared_ptr<Sprite::Palette> hoverpalette(new Sprite::Palette({
		Paint::alpha(ColorName::FLAMEDARK, 60),
		Color::transparent(),
	}));

	return Guide(index,
		std::make_shared<Sprite>("ui/hazardguide", hoverpalette),
		nullptr,
		/*shine=*/false, /*pulse=*/false);
}

Guide Guide::bonedrought(Cell index)
{
	static std::shared_ptr<Sprite::Palette> hoverpalette(new Sprite::Palette({
		Paint::alpha(ColorName::BONEDROUGHT, 30),
		Color::transparent(),
	}));

	return Guide(index,
		std::make_shared<Sprite>("ui/hazardguide", hoverpalette),
		nullptr,
		/*shine=*/false, /*pulse=*/false);
}

Guide Guide::death(Cell index)
{
	static std::shared_ptr<Sprite::Palette> hoverpalette(new Sprite::Palette({
		Paint::alpha(ColorName::DEATH, 30),
		Color::transparent(),
	}));

	return Guide(index,
		std::make_shared<Sprite>("ui/hazardguide", hoverpalette),
		nullptr,
		/*shine=*/false, /*pulse=*/false);
}

Guide Guide::gas(Cell index)
{
	static std::shared_ptr<Sprite::Palette> hoverpalette(new Sprite::Palette({
		Paint::alpha(ColorName::GASDARK, 60),
		Color::transparent(),
	}));

	return Guide(index,
		std::make_shared<Sprite>("ui/hazardguide", hoverpalette),
		nullptr,
		/*shine=*/false, /*pulse=*/false);
}

Guide Guide::radiation(Cell index)
{
	static std::shared_ptr<Sprite::Palette> hoverpalette(new Sprite::Palette({
		Paint::alpha(ColorName::BLOOD, 30),
		Color::transparent(),
	}));

	return Guide(index,
		std::make_shared<Sprite>("ui/hazardguide", hoverpalette),
		nullptr,
		/*shine=*/false, /*pulse=*/false);
}

Guide::Guide(Cell index,
		std::shared_ptr<Sprite> hoversprite, // (unique ownership)
		std::shared_ptr<Sprite> stampsprite, // (unique ownership)
		bool shine, bool pulse) :
	_hoverorigin(Surface::convertOrigin(index.pos())),
	_stamporigin(_hoverorigin.xenon - Surface::WIDTH / 3 + 1,
		_hoverorigin.yahoo),
	_hoversprite(hoversprite),
	_stampsprite(stampsprite),
	_pulsecolors(pulse)
{
	if (_hoversprite)
	{
		_hoversprite->setOriginAtBase();
	}
	if (_stampsprite)
	{
		_stampsprite->setOriginAtBase();
		_stampsprite->setOffset(0, 3);
	}

	if (shine)
	{
		if (_hoversprite)
		{
			_hoversprite->setShineColor(Paint(ColorName::GUIDESHINE));
			_hoversprite->setShine(1);
			_hoversprite->setThetaOffset(-0.4f - Loop::theta());
		}
		if (_stampsprite)
		{
			_stampsprite->setShineColor(Paint(ColorName::GUIDESHINE));
			_stampsprite->setShine(1);
			_stampsprite->setThetaOffset(-0.4f - Loop::theta());
		}
	}
}

Guide::Guide(Guide&& that) :
	_hoverorigin(that._hoverorigin),
	_stamporigin(that._stamporigin),
	_hoversprite(std::move(that._hoversprite)),
	_stampsprite(std::move(that._stampsprite)),
	_pulsecolors(that._pulsecolors)
{}

Guide::Guide() :
	_pulsecolors(false)
{}

void Guide::update()
{
	if (_pulsecolors)
	{
		if (std::fmod(Loop::theta(), 0.5f) < 0.25f)
		{
			if (_hoversprite)
			{
				_hoversprite->setColor(0, _hoversprite->getColor(1));
			}
			if (_stampsprite)
			{
				_stampsprite->setColor(0, _stampsprite->getColor(1));
			}
		}
		else
		{
			if (_hoversprite)
			{
				_hoversprite->setColor(0, _hoversprite->getColor(2));
			}
			if (_stampsprite)
			{
				_stampsprite->setColor(0, _stampsprite->getColor(2));
			}
		}
	}

	if (_hoversprite) _hoversprite->update();
	if (_stampsprite) _stampsprite->update();

	if (_hoversprite && _hoversprite->isVisible())
	{
		Collector::get()->addGuide(_hoversprite, _hoverorigin);
	}
	if (_stampsprite && _stampsprite->isVisible())
	{
		Collector::get()->addGuidestamp(_stampsprite, _stamporigin);
	}
}

void Guide::setVisible(bool visible)
{
	if (_hoversprite) _hoversprite->setVisible(visible);
	if (_stampsprite) _stampsprite->setVisible(visible);
}

Guide::~Guide() = default;

const char* Guide::spritenameMove(const Move& move)
{
	switch (move)
	{
		case Move::X: return "ui/amove_origin";
		case Move::N: return "ui/amove_north";
		case Move::E: return "ui/amove_east";
		case Move::S: return "ui/amove_south";
		case Move::W: return "ui/amove_west";
	}
	return nullptr;
}
