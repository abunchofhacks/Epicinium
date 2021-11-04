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
#include "settingsmenu.hpp"
#include "source.hpp"

#include "frame.hpp"
#include "layout.hpp"
#include "horizontallayout.hpp"
#include "verticallayout.hpp"
#include "scrollablelayout.hpp"
#include "tooltiplayout.hpp"
#include "slideshowlayout.hpp"
#include "textfield.hpp"
#include "textinput.hpp"
#include "filler.hpp"
#include "paint.hpp"
#include "colorname.hpp"
#include "input.hpp"
#include "settings.hpp"
#include "displaysettings.hpp"
#include "graphics.hpp"
#include "client.hpp"
#include "language.hpp"
#include "palette.hpp"


void SettingsMenu::fixScreenOptions()
{
	_layout["bot"]["apply"].enable();
	_layout["bot"]["return"].setTag("discard");

	ScreenMode mode = ScreenMode(stoi(_layout["top"]["left"]["screenmode"]["options"]["content"].getTag()));
	switch (mode)
	{
		case ScreenMode::WINDOWED:
		case ScreenMode::BORDERLESS:
		{
			auto& dropdowns = _layout["top"]["left"]["resolution"]["options"]["dropdowns"];
			dropdowns.setTag("full");
			auto& full = dropdowns["full"];
			if (!full.contains(_layout["top"]["left"]["resolution"]["options"]["content"].getTag()))
			{
				_layout["top"]["left"]["resolution"]["options"]["content"].setTag("custom");
			}
			_layout["top"]["left"]["resolution"]["options"].enable();
		}
		break;
		case ScreenMode::FULLSCREEN:
		{
			auto& dropdowns = _layout["top"]["left"]["resolution"]["options"]["dropdowns"];
			dropdowns.setTag("detected");
			auto& detected = dropdowns["detected"];
			if (!detected.contains(_layout["top"]["left"]["resolution"]["options"]["content"].getTag()))
			{
				std::string newname = detected.name(detected.size() - 1);
				_layout["top"]["left"]["resolution"]["options"]["content"].setTag(newname);
			}
			_layout["top"]["left"]["resolution"]["options"].enable();
			_layout["top"]["left"]["width"]["options"].disable();
			_layout["top"]["left"]["height"]["options"].disable();
		}
		break;
		case ScreenMode::DESKTOP:
		{
			_layout["top"]["left"]["resolution"]["options"]["dropdowns"].setTag("full");
			_layout["top"]["left"]["resolution"]["options"]["content"].setTag("custom");
			_layout["top"]["left"]["width"]["options"].setText("");
			_layout["top"]["left"]["height"]["options"].setText("");
			_layout["top"]["left"]["resolution"]["options"].disable();
			_layout["top"]["left"]["width"]["options"].disable();
			_layout["top"]["left"]["height"]["options"].disable();
			return;
		}
		break;
	}

	std::string name = _layout["top"]["left"]["resolution"]["options"]["content"].getTag();
	if (name == "custom")
	{
		_layout["top"]["left"]["width"]["options"].enable();
		_layout["top"]["left"]["height"]["options"].enable();
	}
	else
	{
		try
		{
			size_t offset;
			int w = std::stoi(name, &offset);
			int h = std::stoi(name.substr(offset + 1));
			_settings.width = w;
			_settings.height = h;
			_layout["top"]["left"]["width"]["options"].setText(
				std::to_string(w));
			_layout["top"]["left"]["height"]["options"].setText(
				std::to_string(h));
			_layout["top"]["left"]["width"]["options"].disable();
			_layout["top"]["left"]["height"]["options"].disable();
		}
		catch (const std::logic_error&)
		{
			LOGE << "Cannot parse dropdown option '" << name << "'";
		}
	}
}

void SettingsMenu::build()
{
	Menu::build();

	_margins = 2 * InterfaceElement::scale();

	_layout.content().put(new VerticalLayout());
	_layout.add("top", new HorizontalLayout());
	_layout.add("bot", new HorizontalLayout());

	buildTop();
	buildBot();

	_layout.setWidth(InterfaceElement::windowW());
	_layout.setHeight(InterfaceElement::windowH());
	_layout["top"]["left"].align(HorizontalAlignment::LEFT);
	_layout["top"]["right"].align(HorizontalAlignment::LEFT);
	_layout.place(Pixel(0, 0, Layer::INTERFACE));

	load();
}

void SettingsMenu::buildTop()
{
	buildTopLeft();
	buildTopRight();
}

void SettingsMenu::buildTopLeft()
{
	_layout["top"].add("left", new Frame("ui/frame_screen_9"));
	_layout["top"]["left"].put(new ScrollableLayout());
	_layout["top"]["left"].setPadding(
		(InterfaceElement::windowH() < 400 * InterfaceElement::scale())
		? 5 * InterfaceElement::scale()
		: 10 * InterfaceElement::scale());
	_layout["top"]["left"].setMargin(
		(InterfaceElement::windowH() < 400 * InterfaceElement::scale())
		? 5 * InterfaceElement::scale()
		: 20 * InterfaceElement::scale());

	buildTopLeftDisplay();
	_layout["top"]["left"].add("separator1", new HorizontalFiller());
	buildTopLeftAudio();
}

void SettingsMenu::buildTopRight()
{
	_layout["top"].add("right", new Frame("ui/frame_screen_9"));
	_layout["top"]["right"].put(new ScrollableLayout());
	_layout["top"]["right"].setPadding(
		(InterfaceElement::windowH() < 400 * InterfaceElement::scale())
		? 5 * InterfaceElement::scale()
		: 10 * InterfaceElement::scale());
	_layout["top"]["right"].setMargin(
		(InterfaceElement::windowH() < 400 * InterfaceElement::scale())
		? 5 * InterfaceElement::scale()
		: 20 * InterfaceElement::scale());

	buildTopRightInterface();
	_layout["top"]["right"].add("separator1", new HorizontalFiller());
	buildTopRightControls();
	_layout["top"]["right"].add("separator2", new HorizontalFiller());
	buildTopRightSocial();
}

void SettingsMenu::buildTopLeftDisplay()
{
	const int FONTSIZEHEADER = _settings.getFontSizeMenuHeader();

	_layout["top"]["left"].add("header_display", new TextField(
		// TRANSLATORS: The header of the video settings section.
		_("Video"),
		FONTSIZEHEADER));
	_layout["top"]["left"]["header_display"].setMargin(
		_margins);

	buildTopLeftDisplayScreenmode();
	buildTopLeftDisplayResolution();
	buildTopLeftDisplayScale();
	buildTopLeftDisplayFramerate();
	buildTopLeftDisplayWindow();
}

inline std::string translatedScreenModeName(const ScreenMode& mode)
{
	switch (mode)
	{
		case ScreenMode::WINDOWED: return _("Windowed");
		case ScreenMode::BORDERLESS: return _("Windowed Borderless");
		case ScreenMode::FULLSCREEN: return _("Fullscreen");
		case ScreenMode::DESKTOP: return _("Fullscreen Desktop");
	}
	return "";
}

void SettingsMenu::buildTopLeftDisplayScreenmode()
{
	const int FONTSIZE = _settings.getFontSize();

	_layout["top"]["left"].add("screenmode", new HorizontalLayout());
	_layout["top"]["left"]["screenmode"].align(
		VerticalAlignment::MIDDLE);
	_layout["top"]["left"]["screenmode"].add("title", new TextField(
		_("Screen Mode:"),
		FONTSIZE));
	_layout["top"]["left"]["screenmode"].add("options",
		new TooltipLayout(/*dropdown=*/true));
	{
		InterfaceElement& options = _layout["top"]["left"]["screenmode"]["options"];
		options.add("content", new SlideshowLayout());
		auto& content = options["content"];

		options.add("dropdown", Frame::makeTooltip(/*dropdown=*/true));
		auto& dropdown = options["dropdown"];
		dropdown.put(new VerticalLayout());

		for (ScreenMode mode : {
			ScreenMode::WINDOWED,
			ScreenMode::BORDERLESS,
			ScreenMode::FULLSCREEN,
			ScreenMode::DESKTOP})
		{
			std::string tagname = std::to_string((int) mode);
			std::string name = translatedScreenModeName(mode);
			content.add(tagname, Frame::makeItem(ColorName::FRAMEITEM));
			content[tagname].put(new TextField(name, FONTSIZE));
			content[tagname].align(HorizontalAlignment::LEFT);
			content[tagname].makeClickable();
			dropdown.add(tagname, Frame::makeItem());
			dropdown[tagname].put(new TextField(name, FONTSIZE));
			dropdown[tagname].align(HorizontalAlignment::LEFT);
			dropdown[tagname].makeClickable();
		}
		content.settleWidth();
		content.settleHeight();
		content.fixWidth();
		dropdown.settleWidth();
		dropdown.settleHeight();
		dropdown.fixWidth();
	}
	_layout["top"]["left"]["screenmode"].setMargin(_margins);
}

void SettingsMenu::buildTopLeftDisplayResolution()
{
	const int FONTSIZE = _settings.getFontSize();

	_layout["top"]["left"].add("resolution", new HorizontalLayout());
	_layout["top"]["left"]["resolution"].align(
		VerticalAlignment::MIDDLE);
	_layout["top"]["left"]["resolution"].add("title", new TextField(
		_("Screen Resolution:"),
		FONTSIZE));
	_layout["top"]["left"]["resolution"].add("options",
		new TooltipLayout(/*dropdown=*/true));
	{
		InterfaceElement& options = _layout["top"]["left"]["resolution"]["options"];
		options.add("content", new SlideshowLayout());
		auto& content = options["content"];

		options.add("dropdowns", new SlideshowLayout());
		auto& dropdowns = options["dropdowns"];

		dropdowns.add("full", Frame::makeTooltip(/*dropdown=*/true));
		auto& full = dropdowns["full"];
		full.put(new VerticalLayout());
		for (std::string tagname : {
			"1024x768",
			"1280x720",
			"1280x800",
			"1280x1024",
			"1360x786",
			"1366x786",
			"1440x900",
			"1600x900",
			"1680x1050",
			"1920x1080",
			"1920x1200",
			"custom"})
		{
			std::string name = tagname;
			if (name == "custom") name = _("Custom");
			content.add(tagname, Frame::makeItem(ColorName::FRAMEITEM));
			content[tagname].put(new TextField(name, FONTSIZE));
			content[tagname].align(HorizontalAlignment::LEFT);
			content[tagname].makeClickable();
			full.add(tagname, Frame::makeItem());
			full[tagname].put(new TextField(name, FONTSIZE));
			full[tagname].align(HorizontalAlignment::LEFT);
			full[tagname].makeClickable();
		}

		dropdowns.add("detected", Frame::makeTooltip(/*dropdown=*/true));
		auto& detected = dropdowns["detected"];
		detected.put(new VerticalLayout());
		for (const std::string& tagname : Graphics::get()->resolutions())
		{
			std::string name = tagname;
			if (!content.contains(tagname))
			{
				content.add(tagname, Frame::makeItem(ColorName::FRAMEITEM));
				content[tagname].put(new TextField(name, FONTSIZE));
				content[tagname].align(HorizontalAlignment::LEFT);
				content[tagname].makeClickable();
			}
			detected.add(tagname, Frame::makeItem());
			detected[tagname].put(new TextField(name, FONTSIZE));
			detected[tagname].align(HorizontalAlignment::LEFT);
			detected[tagname].makeClickable();
		}

		content.settleWidth();
		content.settleHeight();
		content.fixWidth();
		full.settleWidth();
		full.settleHeight();
		full.fixWidth();
		detected.settleWidth();
		detected.settleHeight();
		detected.fixWidth();
		dropdowns.settleWidth();
		dropdowns.settleHeight();
		dropdowns.fixWidth();
	}
	_layout["top"]["left"]["resolution"].setMargin(_margins);

	_layout["top"]["left"].add("width", new HorizontalLayout());
	_layout["top"]["left"]["width"].align(
		VerticalAlignment::MIDDLE);
	_layout["top"]["left"]["width"].add("title", new TextField(
		_("Screen Width:"),
		FONTSIZE));
	_layout["top"]["left"]["width"].add("options", new Frame("ui/frame_window_9"));
	_layout["top"]["left"]["width"]["options"].put(
		new TextInput(
			_settings.getFontSizeTextInput(),
			/*maxlength=*/5));
	_layout["top"]["left"]["width"]["options"].setText("1920");
	_layout["top"]["left"]["width"]["options"].content().fixWidth(
		25 * InterfaceElement::scale());
	_layout["top"]["left"]["width"]["options"].content().setMarginHorizontal(
		4 * InterfaceElement::scale());
	_layout["top"]["left"]["width"]["options"].content().setMarginVertical(
		1 * InterfaceElement::scale());
	_layout["top"]["left"]["width"]["options"].settleWidth();
	_layout["top"]["left"]["width"]["options"].fixWidth();
	_layout["top"]["left"]["width"].setMargin(_margins);

	_layout["top"]["left"].add("height", new HorizontalLayout());
	_layout["top"]["left"]["height"].align(
		VerticalAlignment::MIDDLE);
	_layout["top"]["left"]["height"].add("title", new TextField(
		_("Screen Height:"),
		FONTSIZE));
	_layout["top"]["left"]["height"].add("options", new Frame("ui/frame_window_9"));
	_layout["top"]["left"]["height"]["options"].put(
		new TextInput(
			_settings.getFontSizeTextInput(),
			/*maxlength=*/5));
	_layout["top"]["left"]["height"]["options"].setText("1080");
	_layout["top"]["left"]["height"]["options"].content().fixWidth(
		25 * InterfaceElement::scale());
	_layout["top"]["left"]["height"]["options"].content().setMarginHorizontal(
		4 * InterfaceElement::scale());
	_layout["top"]["left"]["height"]["options"].content().setMarginVertical(
		1 * InterfaceElement::scale());
	_layout["top"]["left"]["height"]["options"].settleWidth();
	_layout["top"]["left"]["height"]["options"].fixWidth();
	_layout["top"]["left"]["height"].setMargin(_margins);
}

void SettingsMenu::buildTopLeftDisplayScale()
{
	const int FONTSIZE = _settings.getFontSize();

	_layout["top"]["left"].add("scale", new HorizontalLayout());
	_layout["top"]["left"]["scale"].align(
		VerticalAlignment::MIDDLE);
	_layout["top"]["left"]["scale"].add("title", new TextField(
		_("Scale:"),
		FONTSIZE));
	_layout["top"]["left"]["scale"].add("options",
		new TooltipLayout(/*dropdown=*/true));
	{
		InterfaceElement& options = _layout["top"]["left"]["scale"]["options"];
		options.add("content", new SlideshowLayout());
		auto& content = options["content"];

		options.add("dropdown", Frame::makeTooltip(/*dropdown=*/true));
		auto& dropdown = options["dropdown"];
		dropdown.put(new VerticalLayout());

		for (int scale : {1, 2, 3, 4})
		{
			std::string tagname = std::to_string(scale);
			std::string name = std::to_string(scale);
			content.add(tagname, Frame::makeItem(ColorName::FRAMEITEM));
			content[tagname].put(new TextField(name, FONTSIZE));
			content[tagname].align(HorizontalAlignment::RIGHT);
			content[tagname].makeClickable();
			dropdown.add(tagname, Frame::makeItem());
			dropdown[tagname].put(new TextField(name, FONTSIZE));
			dropdown[tagname].align(HorizontalAlignment::RIGHT);
			dropdown[tagname].makeClickable();
		}
		content.settleWidth();
		content.settleHeight();
		content.fixWidth();
		dropdown.settleWidth();
		dropdown.settleHeight();
		dropdown.fixWidth();
	}
	_layout["top"]["left"]["scale"].setMargin(_margins);
}

void SettingsMenu::buildTopLeftDisplayFramerate()
{
	const int FONTSIZE = _settings.getFontSize();

	_layout["top"]["left"].add("framerate", new HorizontalLayout());
	_layout["top"]["left"]["framerate"].align(
		VerticalAlignment::MIDDLE);
	_layout["top"]["left"]["framerate"].add("title", new TextField(
		_("Framerate:"),
		FONTSIZE));
	_layout["top"]["left"]["framerate"].add("options",
		new TooltipLayout(/*dropdown=*/true));
	{
		InterfaceElement& options = _layout["top"]["left"]["framerate"]["options"];
		options.add("content", new SlideshowLayout());
		auto& content = options["content"];

		options.add("dropdown", Frame::makeTooltip(/*dropdown=*/true));
		auto& dropdown = options["dropdown"];
		dropdown.put(new VerticalLayout());

		for (int rate : {30, 60, 120})
		{
			std::string tagname = std::to_string(rate);
			std::string name = std::to_string(rate);
			content.add(tagname, Frame::makeItem(ColorName::FRAMEITEM));
			content[tagname].put(new TextField(name, FONTSIZE));
			content[tagname].align(HorizontalAlignment::RIGHT);
			content[tagname].makeClickable();
			dropdown.add(tagname, Frame::makeItem());
			dropdown[tagname].put(new TextField(name, FONTSIZE));
			dropdown[tagname].align(HorizontalAlignment::RIGHT);
			dropdown[tagname].makeClickable();
		}
		content.settleWidth();
		content.settleHeight();
		content.fixWidth();
		dropdown.settleWidth();
		dropdown.settleHeight();
		dropdown.fixWidth();
	}
	_layout["top"]["left"]["framerate"].setMargin(_margins);
}

void SettingsMenu::buildTopLeftDisplayWindow()
{
	const int FONTSIZE = _settings.getFontSize();

	_layout["top"]["left"].add("display", new HorizontalLayout());
	_layout["top"]["left"]["display"].align(
		VerticalAlignment::MIDDLE);
	_layout["top"]["left"]["display"].add("title", new TextField(
		_("Monitor:"),
		FONTSIZE));
	_layout["top"]["left"]["display"].add("options",
		new TooltipLayout(/*dropdown=*/true));
	{
		InterfaceElement& options =
			_layout["top"]["left"]["display"]["options"];
		options.add("content", new SlideshowLayout());
		auto& content = options["content"];

		options.add("dropdown", Frame::makeTooltip(/*dropdown=*/true));
		auto& dropdown = options["dropdown"];
		dropdown.put(new VerticalLayout());

		for (int i = 1; i <= (int) _displaysettings.displayBounds.size(); i++)
		{
			std::string tagname = std::to_string((int) i);
			std::string name = tagname;
			content.add(tagname, Frame::makeItem(ColorName::FRAMEITEM));
			content[tagname].put(new TextField(name, FONTSIZE));
			content[tagname].align(HorizontalAlignment::LEFT);
			content[tagname].makeClickable();
			dropdown.add(tagname, Frame::makeItem());
			dropdown[tagname].put(new TextField(name, FONTSIZE));
			dropdown[tagname].align(HorizontalAlignment::LEFT);
			dropdown[tagname].makeClickable();
		}
		content.settleWidth();
		content.settleHeight();
		content.fixWidth();
		dropdown.settleWidth();
		dropdown.settleHeight();
		dropdown.fixWidth();
	}
	_layout["top"]["left"]["display"].setMargin(_margins);

	_layout["top"]["left"].add("window-position", new HorizontalLayout());
	_layout["top"]["left"]["window-position"].align(
		VerticalAlignment::MIDDLE);
	_layout["top"]["left"]["window-position"].add("title", new TextField(
		_("Window Position:"),
		FONTSIZE));
	_layout["top"]["left"]["window-position"].add("options",
		new TooltipLayout(/*dropdown=*/true));
	{
		InterfaceElement& options =
			_layout["top"]["left"]["window-position"]["options"];
		options.add("content", new SlideshowLayout());
		auto& content = options["content"];

		options.add("dropdown", Frame::makeTooltip(/*dropdown=*/true));
		auto& dropdown = options["dropdown"];
		dropdown.put(new VerticalLayout());

		const char* TAGNAMES[] = {
			"centered",
			"custom",
		};
		std::string NAMES[array_size(TAGNAMES)] = {
			_("Centered"),
			_("Custom"),
		};
		for (size_t i = 0; i < array_size(TAGNAMES); i++)
		{
			std::string tagname = TAGNAMES[i];
			const std::string& name = NAMES[i];
			content.add(tagname, Frame::makeItem(ColorName::FRAMEITEM));
			content[tagname].put(new TextField(name, FONTSIZE));
			content[tagname].align(HorizontalAlignment::LEFT);
			content[tagname].makeClickable();
			dropdown.add(tagname, Frame::makeItem());
			dropdown[tagname].put(new TextField(name, FONTSIZE));
			dropdown[tagname].align(HorizontalAlignment::LEFT);
			dropdown[tagname].makeClickable();
		}
		content.settleWidth();
		content.settleHeight();
		content.fixWidth();
		dropdown.settleWidth();
		dropdown.settleHeight();
		dropdown.fixWidth();
	}
	_layout["top"]["left"]["window-position"].setMargin(_margins);

	_layout["top"]["left"].add("window-x", new HorizontalLayout());
	_layout["top"]["left"]["window-x"].align(
		VerticalAlignment::MIDDLE);
	_layout["top"]["left"]["window-x"].add("title", new TextField(
		_("Window Position (X):"),
		FONTSIZE));
	_layout["top"]["left"]["window-x"].add("options",
		new Frame("ui/frame_window_9"));
	_layout["top"]["left"]["window-x"]["options"].put(
		new TextInput(
			_settings.getFontSizeTextInput(),
			/*maxlength=*/5));
	_layout["top"]["left"]["window-x"]["options"].setText("");
	_layout["top"]["left"]["window-x"]["options"].content().fixWidth(
		25 * InterfaceElement::scale());
	_layout["top"]["left"]["window-x"]["options"].content().setMarginHorizontal(
		4 * InterfaceElement::scale());
	_layout["top"]["left"]["window-x"]["options"].content().setMarginVertical(
		1 * InterfaceElement::scale());
	_layout["top"]["left"]["window-x"]["options"].settleWidth();
	_layout["top"]["left"]["window-x"]["options"].fixWidth();
	_layout["top"]["left"]["window-x"].setMargin(_margins);

	_layout["top"]["left"].add("window-y", new HorizontalLayout());
	_layout["top"]["left"]["window-y"].align(
		VerticalAlignment::MIDDLE);
	_layout["top"]["left"]["window-y"].add("title", new TextField(
		_("Window Position (Y):"),
		FONTSIZE));
	_layout["top"]["left"]["window-y"].add("options",
		new Frame("ui/frame_window_9"));
	_layout["top"]["left"]["window-y"]["options"].put(
		new TextInput(
			_settings.getFontSizeTextInput(),
			/*maxlength=*/5));
	_layout["top"]["left"]["window-y"]["options"].setText("");
	_layout["top"]["left"]["window-y"]["options"].content().fixWidth(
		25 * InterfaceElement::scale());
	_layout["top"]["left"]["window-y"]["options"].content().setMarginHorizontal(
		4 * InterfaceElement::scale());
	_layout["top"]["left"]["window-y"]["options"].content().setMarginVertical(
		1 * InterfaceElement::scale());
	_layout["top"]["left"]["window-y"]["options"].settleWidth();
	_layout["top"]["left"]["window-y"]["options"].fixWidth();
	_layout["top"]["left"]["window-y"].setMargin(_margins);
}

void SettingsMenu::buildTopLeftAudio()
{
	const int FONTSIZE = _settings.getFontSize();
	const int FONTSIZEHEADER = _settings.getFontSizeMenuHeader();

	_layout["top"]["left"].add("header_audio", new TextField(
		// TRANSLATORS: The header of the audio settings section.
		_("Audio"),
		FONTSIZEHEADER));
	_layout["top"]["left"]["header_audio"].setMargin(_margins);

	_layout["top"]["left"].add("masterVolume", new HorizontalLayout());
	_layout["top"]["left"]["masterVolume"].align(
		VerticalAlignment::MIDDLE);
	_layout["top"]["left"]["masterVolume"].add("title", new TextField(
		_("Master Volume:"),
		FONTSIZE));
	_layout["top"]["left"]["masterVolume"].add("options",
		Menu::makeSlider(100 * InterfaceElement::scale()));
	_layout["top"]["left"]["masterVolume"].setMargin(_margins);

	_layout["top"]["left"].add("gameplayVolume", new HorizontalLayout());
	_layout["top"]["left"]["gameplayVolume"].align(
		VerticalAlignment::MIDDLE);
	_layout["top"]["left"]["gameplayVolume"].add("title", new TextField(
		_("Gameplay Volume:"),
		FONTSIZE));
	_layout["top"]["left"]["gameplayVolume"].add("options",
		Menu::makeSlider(100 * InterfaceElement::scale()));
	_layout["top"]["left"]["gameplayVolume"].setMargin(_margins);

	_layout["top"]["left"].add("musicVolume", new HorizontalLayout());
	_layout["top"]["left"]["musicVolume"].align(
		VerticalAlignment::MIDDLE);
	_layout["top"]["left"]["musicVolume"].add("title", new TextField(
		_("Music Volume:"),
		FONTSIZE));
	_layout["top"]["left"]["musicVolume"].add("options",
		Menu::makeSlider(100 * InterfaceElement::scale()));
	_layout["top"]["left"]["musicVolume"].setMargin(_margins);

	_layout["top"]["left"].add("audioBufferSize", new HorizontalLayout());
	_layout["top"]["left"]["audioBufferSize"].align(
		VerticalAlignment::MIDDLE);
	_layout["top"]["left"]["audioBufferSize"].add("title", new TextField(
		_("Audio Buffer Size:"),
		FONTSIZE));
	_layout["top"]["left"]["audioBufferSize"].add("options",
		new TooltipLayout(/*dropdown=*/true));
	{
		InterfaceElement& options = _layout["top"]["left"]["audioBufferSize"]["options"];
		options.add("content", new SlideshowLayout());
		auto& content = options["content"];

		options.add("dropdown", Frame::makeTooltip(/*dropdown=*/true));
		auto& dropdown = options["dropdown"];
		dropdown.put(new VerticalLayout());

		for (int rate : {512, 1024, 2048, 4096, 8192, 16384})
		{
			std::string tagname = std::to_string(rate);
			std::string name = std::to_string(rate);
			content.add(tagname, Frame::makeItem(ColorName::FRAMEITEM));
			content[tagname].put(new TextField(name, FONTSIZE));
			content[tagname].align(HorizontalAlignment::RIGHT);
			content[tagname].makeClickable();
			dropdown.add(tagname, Frame::makeItem());
			dropdown[tagname].put(new TextField(name, FONTSIZE));
			dropdown[tagname].align(HorizontalAlignment::RIGHT);
			dropdown[tagname].makeClickable();
		}
		content.settleWidth();
		content.settleHeight();
		content.fixWidth();
		dropdown.settleWidth();
		dropdown.settleHeight();
		dropdown.fixWidth();
	}
	_layout["top"]["left"]["audioBufferSize"].setMargin(_margins);
}

void SettingsMenu::buildTopRightControls()
{
	const int FONTSIZE = _settings.getFontSize();
	const int FONTSIZEHEADER = _settings.getFontSizeMenuHeader();

	_layout["top"]["right"].add("header_controls", new TextField(
		// TRANSLATORS: The header of the control settings section.
		_("Controls"),
		FONTSIZEHEADER));
	_layout["top"]["right"]["header_controls"].setMargin(
		_margins);

	_layout["top"]["right"].add("selectormode", new HorizontalLayout());
	_layout["top"]["right"]["selectormode"].align(
		VerticalAlignment::MIDDLE);
	_layout["top"]["right"]["selectormode"].add("title", new TextField(
		_("Selection Mode:"),
		FONTSIZE));
	_layout["top"]["right"]["selectormode"].add("options",
		new TooltipLayout(/*dropdown=*/true));
	{
		InterfaceElement& options = _layout["top"]["right"]["selectormode"]["options"];
		options.add("content", new SlideshowLayout());
		auto& content = options["content"];

		options.add("dropdown", Frame::makeTooltip(/*dropdown=*/true));
		auto& dropdown = options["dropdown"];
		dropdown.put(new VerticalLayout());

		for (SelectorMode mode : {
			SelectorMode::FIGURE,
			SelectorMode::CONTEXT})
		{
			std::string tagname = std::to_string((int) mode);
			std::string name;
			switch (mode)
			{
				case SelectorMode::FIGURE: name = _("Figure-Based"); break;
				case SelectorMode::CONTEXT: name = _("Context Menu"); break;
			}
			content.add(tagname, Frame::makeItem(ColorName::FRAMEITEM));
			content[tagname].put(new TextField(name, FONTSIZE));
			content[tagname].align(HorizontalAlignment::LEFT);
			content[tagname].makeClickable();
			dropdown.add(tagname, Frame::makeItem());
			dropdown[tagname].put(new TextField(name, FONTSIZE));
			dropdown[tagname].align(HorizontalAlignment::LEFT);
			dropdown[tagname].makeClickable();
		}
		content.settleWidth();
		content.settleHeight();
		content.fixWidth();
		dropdown.settleWidth();
		dropdown.settleHeight();
		dropdown.fixWidth();
	}
	_layout["top"]["right"]["selectormode"].setMargin(_margins);

	_layout["top"]["right"].add("cameraScrollingWasd", new HorizontalLayout());
	_layout["top"]["right"]["cameraScrollingWasd"].align(
		VerticalAlignment::MIDDLE);
	_layout["top"]["right"]["cameraScrollingWasd"].add("title", new TextField(
		_("View Scrolling with WASD:"),
		FONTSIZE));
	_layout["top"]["right"]["cameraScrollingWasd"].add("options",
		Menu::makeCheckbox());
	_layout["top"]["right"]["cameraScrollingWasd"].setMarginHorizontal(_margins);
	_layout["top"]["right"]["cameraScrollingWasd"]["title"].setMarginVertical(_margins);

	_layout["top"]["right"].add("cameraScrollingArrows", new HorizontalLayout());
	_layout["top"]["right"]["cameraScrollingArrows"].align(
		VerticalAlignment::MIDDLE);
	_layout["top"]["right"]["cameraScrollingArrows"].add("title", new TextField(
		_("View Scrolling with Arrow Keys:"),
		FONTSIZE));
	_layout["top"]["right"]["cameraScrollingArrows"].add("options",
		Menu::makeCheckbox());
	_layout["top"]["right"]["cameraScrollingArrows"].setMarginHorizontal(_margins);
	_layout["top"]["right"]["cameraScrollingArrows"]["title"].setMarginVertical(_margins);

	_layout["top"]["right"].add("cameraScrollingEdge", new HorizontalLayout());
	_layout["top"]["right"]["cameraScrollingEdge"].align(
		VerticalAlignment::MIDDLE);
	_layout["top"]["right"]["cameraScrollingEdge"].add("title", new TextField(
		_("View Scrolling at Edge of Screen:"),
		FONTSIZE));
	_layout["top"]["right"]["cameraScrollingEdge"].add("options",
		Menu::makeCheckbox());
	_layout["top"]["right"]["cameraScrollingEdge"].setMarginHorizontal(_margins);
	_layout["top"]["right"]["cameraScrollingEdge"]["title"].setMarginVertical(_margins);

	_layout["top"]["right"].add("margin_advancedMode", new HorizontalFiller());
	_layout["top"]["right"].add("advancedMode", new HorizontalLayout());
	_layout["top"]["right"]["advancedMode"].align(
		VerticalAlignment::MIDDLE);
	_layout["top"]["right"]["advancedMode"].add("title", new TextField(
		_("Advanced Mode:"),
		FONTSIZE));
	_layout["top"]["right"]["advancedMode"].add("options",
		Menu::makeCheckbox());
	_layout["top"]["right"]["advancedMode"].setMarginHorizontal(_margins);
	_layout["top"]["right"]["advancedMode"]["title"].setMarginVertical(_margins);

	_layout["top"]["right"].add("allowPowerless", new HorizontalLayout());
	_layout["top"]["right"]["allowPowerless"].align(
		VerticalAlignment::MIDDLE);
	_layout["top"]["right"]["allowPowerless"].add("title", new TextField(
		_("Allow orders for tiles without power:"),
		FONTSIZE));
	_layout["top"]["right"]["allowPowerless"].add("options",
		Menu::makeCheckbox());
	_layout["top"]["right"]["allowPowerless"].setMarginHorizontal(_margins);
	_layout["top"]["right"]["allowPowerless"]["title"].setMarginVertical(_margins);

	_layout["top"]["right"].add("allowRedundantStop", new HorizontalLayout());
	_layout["top"]["right"]["allowRedundantStop"].align(
		VerticalAlignment::MIDDLE);
	_layout["top"]["right"]["allowRedundantStop"].add("title", new TextField(
		_("Allow redundant Stop orders:"),
		FONTSIZE));
	_layout["top"]["right"]["allowRedundantStop"].add("options",
		Menu::makeCheckbox());
	_layout["top"]["right"]["allowRedundantStop"].setMarginHorizontal(_margins);
	_layout["top"]["right"]["allowRedundantStop"]["title"].setMarginVertical(_margins);

	_layout["top"]["right"].add("allowRedundantCultivate", new HorizontalLayout());
	_layout["top"]["right"]["allowRedundantCultivate"].align(
		VerticalAlignment::MIDDLE);
	_layout["top"]["right"]["allowRedundantCultivate"].add("title", new TextField(
		_("Allow redundant Cultivate orders:"),
		FONTSIZE));
	_layout["top"]["right"]["allowRedundantCultivate"].add("options",
		Menu::makeCheckbox());
	_layout["top"]["right"]["allowRedundantCultivate"].setMarginHorizontal(_margins);
	_layout["top"]["right"]["allowRedundantCultivate"]["title"].setMarginVertical(_margins);

	_layout["top"]["right"].add("allowRedundantUpgrade", new HorizontalLayout());
	_layout["top"]["right"]["allowRedundantUpgrade"].align(
		VerticalAlignment::MIDDLE);
	_layout["top"]["right"]["allowRedundantUpgrade"].add("title", new TextField(
		_("Allow redundant Upgrade orders:"),
		FONTSIZE));
	_layout["top"]["right"]["allowRedundantUpgrade"].add("options",
		Menu::makeCheckbox());
	_layout["top"]["right"]["allowRedundantUpgrade"].setMarginHorizontal(_margins);
	_layout["top"]["right"]["allowRedundantUpgrade"]["title"].setMarginVertical(_margins);

	_layout["top"]["right"].add("allowScrollingWhileActing", new HorizontalLayout());
	_layout["top"]["right"]["allowScrollingWhileActing"].align(
		VerticalAlignment::MIDDLE);
	_layout["top"]["right"]["allowScrollingWhileActing"].add("title", new TextField(
		_("Allow view scrolling while acting:"),
		FONTSIZE));
	_layout["top"]["right"]["allowScrollingWhileActing"].add("options",
		Menu::makeCheckbox());
	_layout["top"]["right"]["allowScrollingWhileActing"].setMarginHorizontal(_margins);
	_layout["top"]["right"]["allowScrollingWhileActing"]["title"].setMarginVertical(_margins);

	_layout["top"]["right"].add("allowScrollingWhilePanning", new HorizontalLayout());
	_layout["top"]["right"]["allowScrollingWhilePanning"].align(
		VerticalAlignment::MIDDLE);
	_layout["top"]["right"]["allowScrollingWhilePanning"].add("title", new TextField(
		_("Allow view scrolling while panning:"),
		FONTSIZE));
	_layout["top"]["right"]["allowScrollingWhilePanning"].add("options",
		Menu::makeCheckbox());
	_layout["top"]["right"]["allowScrollingWhilePanning"].setMarginHorizontal(_margins);
	_layout["top"]["right"]["allowScrollingWhilePanning"]["title"].setMarginVertical(_margins);
}

void SettingsMenu::buildTopRightSocial()
{
	const int FONTSIZE = _settings.getFontSize();
	const int FONTSIZEHEADER = _settings.getFontSizeMenuHeader();

	_layout["top"]["right"].add("header_social", new TextField(
		// TRANSLATORS: The header of the social settings section.
		_("Social"),
		FONTSIZEHEADER));
	_layout["top"]["right"]["header_social"].setMargin(
		_margins);

	_layout["top"]["right"].add("discord", new HorizontalLayout());
	_layout["top"]["right"]["discord"].align(
		VerticalAlignment::MIDDLE);
	_layout["top"]["right"]["discord"].add("title", new TextField(
		_("Enable Discord Rich Presence:"),
		FONTSIZE));
	_layout["top"]["right"]["discord"].add("options",
		Menu::makeCheckbox());
	_layout["top"]["right"]["discord"].setMarginHorizontal(_margins);
	_layout["top"]["right"]["discord"]["title"].setMarginVertical(_margins);

	_layout["top"]["right"].add("all-chat", new HorizontalLayout());
	_layout["top"]["right"]["all-chat"].align(
		VerticalAlignment::MIDDLE);
	_layout["top"]["right"]["all-chat"].add("title", new TextField(
		_("Enable General Text Chat ('ALL'):"),
		FONTSIZE));
	_layout["top"]["right"]["all-chat"].add("options",
		Menu::makeCheckbox());
	_layout["top"]["right"]["all-chat"].setMarginHorizontal(_margins);
	_layout["top"]["right"]["all-chat"]["title"].setMarginVertical(_margins);

	_layout["top"]["right"].add("lobby-chat", new HorizontalLayout());
	_layout["top"]["right"]["lobby-chat"].align(
		VerticalAlignment::MIDDLE);
	_layout["top"]["right"]["lobby-chat"].add("title", new TextField(
		_("Enable Lobby Text Chat:"),
		FONTSIZE));
	_layout["top"]["right"]["lobby-chat"].add("options",
		Menu::makeCheckbox());
	_layout["top"]["right"]["lobby-chat"].setMarginHorizontal(_margins);
	_layout["top"]["right"]["lobby-chat"]["title"].setMarginVertical(_margins);
}

void SettingsMenu::buildTopRightInterface()
{
	const int FONTSIZE = _settings.getFontSize();
	const int FONTSIZEHEADER = _settings.getFontSizeMenuHeader();

	_layout["top"]["right"].add("header_interface", new TextField(
		// TRANSLATORS: The header of the interface settings section.
		_("Interface"),
		FONTSIZEHEADER));
	_layout["top"]["right"]["header_interface"].setMargin(
		_margins);

	_layout["top"]["right"].add("language", new HorizontalLayout());
	_layout["top"]["right"]["language"].align(
		VerticalAlignment::MIDDLE);
	_layout["top"]["right"]["language"].add("title", new TextField(
		_("Language:"),
		FONTSIZE));
	_layout["top"]["right"]["language"].add("options",
		new TooltipLayout(/*dropdown=*/true));
	{
		InterfaceElement& options = _layout["top"]["right"]["language"]["options"];
		options.add("content", new SlideshowLayout());
		auto& content = options["content"];

		options.add("dropdown", Frame::makeTooltip(/*dropdown=*/true));
		auto& dropdown = options["dropdown"];
		dropdown.put(new VerticalLayout());

		for (std::string tag : Language::supportedTags())
		{
			std::string name = Language::getNameInOwnLanguage(tag);
			content.add(tag, Frame::makeItem(ColorName::FRAMEITEM));
			content[tag].put(new TextField(name, FONTSIZE));
			content[tag].align(HorizontalAlignment::LEFT);
			content[tag].makeClickable();
			dropdown.add(tag, Frame::makeItem());
			dropdown[tag].put(new TextField(name, FONTSIZE));
			dropdown[tag].align(HorizontalAlignment::LEFT);
			dropdown[tag].makeClickable();
		}
		for (std::string tag : Language::experimentalTags())
		{
			std::string name = Language::getNameInOwnLanguage(tag);
			{
				Language::ScopedOverride override(tag);
				name = ::format(
					_("%s [experimental]"),
					name.c_str());
			}
			content.add(tag, Frame::makeItem(ColorName::FRAMEITEM));
			content[tag].put(new TextField(name, FONTSIZE));
			content[tag].align(HorizontalAlignment::LEFT);
			content[tag].makeClickable();
			dropdown.add(tag, Frame::makeItem());
			dropdown[tag].put(new TextField(name, FONTSIZE));
			dropdown[tag].align(HorizontalAlignment::LEFT);
			dropdown[tag].makeClickable();
		}
		for (std::string tag : Language::incompleteTags())
		{
			std::string name = Language::getNameInOwnLanguage(tag);
			{
				Language::ScopedOverride override(tag);
				name = ::format(
					_("%s [incomplete]"),
					name.c_str());
			}
			content.add(tag, Frame::makeItem(ColorName::FRAMEITEM));
			content[tag].put(new TextField(name, FONTSIZE));
			content[tag].align(HorizontalAlignment::LEFT);
			content[tag].makeClickable();
			dropdown.add(tag, Frame::makeItem());
			dropdown[tag].put(new TextField(name, FONTSIZE));
			dropdown[tag].align(HorizontalAlignment::LEFT);
			dropdown[tag].makeClickable();
		}
		for (std::string tag : Language::allDetectedTags())
		{
			if (content.contains(tag)) continue;

			std::string name = Language::getNameInOwnLanguage(tag);
			name += " <" + tag + ".po>";
			content.add(tag, Frame::makeItem(ColorName::FRAMEITEM));
			content[tag].put(new TextField(name, FONTSIZE));
			content[tag].align(HorizontalAlignment::LEFT);
			content[tag].makeClickable();
			dropdown.add(tag, Frame::makeItem());
			dropdown[tag].put(new TextField(name, FONTSIZE));
			dropdown[tag].align(HorizontalAlignment::LEFT);
			dropdown[tag].makeClickable();
		}
		content.settleWidth();
		content.settleHeight();
		content.fixWidth();
		dropdown.settleWidth();
		dropdown.settleHeight();
		dropdown.fixWidth();
	}
	_layout["top"]["right"]["language"].setMargin(_margins);

	_layout["top"]["right"].add("textscale", new HorizontalLayout());
	_layout["top"]["right"]["textscale"].align(
		VerticalAlignment::MIDDLE);
	_layout["top"]["right"]["textscale"].add("title", new TextField(
		_("Text scaling:"),
		FONTSIZE));
	_layout["top"]["right"]["textscale"].add("options",
		new TooltipLayout(/*dropdown=*/true));
	{
		InterfaceElement& options = _layout["top"]["right"]["textscale"]["options"];
		options.add("content", new SlideshowLayout());
		auto& content = options["content"];

		options.add("dropdown", Frame::makeTooltip(/*dropdown=*/true));
		auto& dropdown = options["dropdown"];
		dropdown.put(new VerticalLayout());

		for (int percent : {100, 150})
		{
			std::string tag = std::to_string(percent);
			std::string name = std::to_string(percent) + "%";
			content.add(tag, Frame::makeItem(ColorName::FRAMEITEM));
			content[tag].put(new TextField(name, FONTSIZE));
			content[tag].align(HorizontalAlignment::LEFT);
			content[tag].makeClickable();
			dropdown.add(tag, Frame::makeItem());
			dropdown[tag].put(new TextField(name, FONTSIZE));
			dropdown[tag].align(HorizontalAlignment::LEFT);
			dropdown[tag].makeClickable();
		}
		content.settleWidth();
		content.settleHeight();
		content.fixWidth();
		dropdown.settleWidth();
		dropdown.settleHeight();
		dropdown.fixWidth();
	}
	_layout["top"]["right"]["textscale"].setMarginHorizontal(_margins);

	_layout["top"]["right"].add("highcontrast", new HorizontalLayout());
	_layout["top"]["right"]["highcontrast"].align(
		VerticalAlignment::MIDDLE);
	_layout["top"]["right"]["highcontrast"].add("title", new TextField(
		_("High contrast colors:"),
		FONTSIZE));
	_layout["top"]["right"]["highcontrast"].add("options",
		Menu::makeCheckbox());
	_layout["top"]["right"]["highcontrast"].setMarginHorizontal(_margins);
	_layout["top"]["right"]["highcontrast"]["title"].setMarginVertical(_margins);

	_layout["top"]["right"].add("palette", new HorizontalLayout());
	_layout["top"]["right"]["palette"].align(
		VerticalAlignment::MIDDLE);
	_layout["top"]["right"]["palette"].add("title", new TextField(
		_("Color palette:"),
		FONTSIZE));
	_layout["top"]["right"]["palette"].add("options",
		new TooltipLayout(/*dropdown=*/true));
	{
		InterfaceElement& options = _layout["top"]["right"]["palette"]["options"];
		options.add("content", new SlideshowLayout());
		auto& content = options["content"];

		options.add("dropdown", Frame::makeTooltip(/*dropdown=*/true));
		auto& dropdown = options["dropdown"];
		dropdown.put(new VerticalLayout());

		for (std::string name : Palette::indexedNames())
		{
			if (name == "custom") continue;
			std::string tagname = name;
			content.add(tagname, Frame::makeItem(ColorName::FRAMEITEM));
			content[tagname].put(new TextField(name, FONTSIZE));
			content[tagname].align(HorizontalAlignment::LEFT);
			content[tagname].makeClickable();
			if (name == "default") continue;
			dropdown.add(tagname, Frame::makeItem());
			dropdown[tagname].put(new TextField(name, FONTSIZE));
			dropdown[tagname].align(HorizontalAlignment::LEFT);
			dropdown[tagname].makeClickable();
		}
		for (const auto& item : Palette::externalItems())
		{
			std::string tagname = item.uniqueTag;
			content.add(tagname, Frame::makeItem(ColorName::FRAMEITEM));
			content[tagname].put(new TextField(item.quotedName, FONTSIZE));
			content[tagname].align(HorizontalAlignment::LEFT);
			content[tagname].makeClickable();
			dropdown.add(tagname, Frame::makeItem());
			dropdown[tagname].put(new TextField(item.quotedName, FONTSIZE));
			dropdown[tagname].align(HorizontalAlignment::LEFT);
			dropdown[tagname].makeClickable();
		}
		{
			content.add("custom", Frame::makeItem(ColorName::FRAMEITEM));
			content["custom"].put(new TextField("custom", FONTSIZE));
			content["custom"].align(HorizontalAlignment::LEFT);
			content["custom"].makeClickable();
			dropdown.add("custom", Frame::makeItem());
			dropdown["custom"].put(new TextField("custom...", FONTSIZE));
			dropdown["custom"].align(HorizontalAlignment::LEFT);
			dropdown["custom"].makeClickable();
		}
		content.settleWidth();
		content.settleHeight();
		content.fixWidth();
		dropdown.settleWidth();
		dropdown.settleHeight();
		dropdown.fixWidth();
	}
	_layout["top"]["right"]["palette"].setMargin(_margins);

	_layout["top"]["right"].add("artpanmode", new HorizontalLayout());
	_layout["top"]["right"]["artpanmode"].align(
		VerticalAlignment::MIDDLE);
	_layout["top"]["right"]["artpanmode"].add("title", new TextField(
		_("Main menu art panning:"),
		FONTSIZE));
	_layout["top"]["right"]["artpanmode"].add("options",
		new TooltipLayout(/*dropdown=*/true));
	{
		InterfaceElement& options = _layout["top"]["right"]["artpanmode"]["options"];
		options.add("content", new SlideshowLayout());
		auto& content = options["content"];

		options.add("dropdown", Frame::makeTooltip(/*dropdown=*/true));
		auto& dropdown = options["dropdown"];
		dropdown.put(new VerticalLayout());

		for (ArtPanMode mode : {
			ArtPanMode::AUTO,
			ArtPanMode::CURSOR,
			ArtPanMode::NONE})
		{
			std::string tagname = std::to_string((int) mode);
			std::string name;
			switch (mode)
			{
				case ArtPanMode::NONE: name = _("Off"); break;
				case ArtPanMode::AUTO: name = _("Automatic"); break;
				case ArtPanMode::CURSOR: name = _("Cursor"); break;
			}
			content.add(tagname, Frame::makeItem(ColorName::FRAMEITEM));
			content[tagname].put(new TextField(name, FONTSIZE));
			content[tagname].align(HorizontalAlignment::LEFT);
			content[tagname].makeClickable();
			dropdown.add(tagname, Frame::makeItem());
			dropdown[tagname].put(new TextField(name, FONTSIZE));
			dropdown[tagname].align(HorizontalAlignment::LEFT);
			dropdown[tagname].makeClickable();
		}
		content.settleWidth();
		content.settleHeight();
		content.fixWidth();
		dropdown.settleWidth();
		dropdown.settleHeight();
		dropdown.fixWidth();
	}
	_layout["top"]["right"]["artpanmode"].setMargin(_margins);
}

void SettingsMenu::buildBot()
{
	const int FONTSIZE = _settings.getFontSizeMenuButton();

	_layout["bot"].add("filler1", new HorizontalFiller());
	_layout["bot"].add("apply", makeButton(
		_("apply and restart"),
		FONTSIZE));
	_layout["bot"]["apply"].setAltHotkeyScancode(SDL_SCANCODE_A);
	_layout["bot"]["apply"].settleWidth();
	_layout["bot"]["apply"].fixWidth();
	_layout["bot"]["apply"].setMargin(
		5 * InterfaceElement::scale());
	_layout["bot"].add("return", new SlideshowLayout());
	_layout["bot"]["return"].add("discard", makeButton(
		_("discard and return to main menu"),
		FONTSIZE));
	_layout["bot"]["return"]["discard"].setAltHotkeyScancode(SDL_SCANCODE_D);
	_layout["bot"]["return"]["discard"].setHotkeyScancode(SDL_SCANCODE_ESCAPE);
	_layout["bot"]["return"].add("apply", makeButton(
		_("apply and return to main menu"),
		FONTSIZE));
	_layout["bot"]["return"]["apply"].setAltHotkeyScancode(SDL_SCANCODE_A);
	_layout["bot"]["return"]["apply"].setHotkeyScancode(SDL_SCANCODE_ESCAPE);
	_layout["bot"]["return"].settleWidth();
	_layout["bot"]["return"].fixWidth();
	_layout["bot"]["return"].setMargin(
		5 * InterfaceElement::scale());
}

void SettingsMenu::refresh()
{
	// Because some items might occlude others, we must know which layer the mouse operates in.
	// We therefore need to check all items to see if they are hovered.
	_layout.hovered();

	_layout.refresh();

	if (_layout["top"]["left"]["display"]["options"]["dropdown"].clicked())
	{
		auto& options = _layout["top"]["left"]["display"]["options"];
		auto& content = options["content"];
		auto& dropdown = options["dropdown"];
		for (size_t i = 0; i < content.size(); i++)
		{
			std::string name = content.name(i);
			if (dropdown.contains(name) && dropdown[name].clicked()
				&& content.getTag() != name)
			{
				try
				{
					int index = stoi(name);
					_settings.display = index;
					content.setTag(name);

					_layout["bot"]["apply"].enable();
					_layout["bot"]["return"].setTag("discard");
				}
				catch (const std::logic_error&)
				{
					LOGE << "Cannot parse dropdown option '" << name << "'";
				}
			}
		}
	}

	if (_layout["top"]["left"]["window-position"]["options"]["dropdown"].clicked())
	{
		auto& options = _layout["top"]["left"]["window-position"]["options"];
		auto& content = options["content"];
		auto& dropdown = options["dropdown"];
		for (size_t i = 0; i < content.size(); i++)
		{
			std::string name = content.name(i);
			if (dropdown.contains(name) && dropdown[name].clicked()
				&& content.getTag() != name)
			{
				if (name == "custom")
				{
					_layout["top"]["left"]["window-x"].enable();
					_layout["top"]["left"]["window-y"].enable();
					content.setTag(name);
				}
				else // if (name == "centered")
				{
					_settings.windowX = -1;
					_settings.windowY = -1;
					_layout["top"]["left"]["window-x"]["options"].setText("");
					_layout["top"]["left"]["window-y"]["options"].setText("");
					_layout["top"]["left"]["window-x"].disable();
					_layout["top"]["left"]["window-y"].disable();
					content.setTag(name);

					_layout["bot"]["apply"].enable();
					_layout["bot"]["return"].setTag("discard");
				}
			}
		}
	}

	if (_layout["top"]["left"]["window-x"].enabled())
	{
		InterfaceElement& options = _layout["top"]["left"]["window-x"]["options"];
		if (options.clicked())
		{
			options.content().power();
		}
		else if (options.content().powered()
			&& Input::get()->wasKeyPressed(SDL_SCANCODE_LMB))
		{
			options.content().depower();
			std::string name = options.content().text();
			try
			{
				int x = stoi(name);
				_settings.windowX = x;

				_layout["bot"]["apply"].enable();
				_layout["bot"]["return"].setTag("discard");
			}
			catch (const std::logic_error&)
			{
				LOGE << "Cannot parse input '" << name << "'";
			}
		}
	}

	if (_layout["top"]["left"]["window-y"].enabled())
	{
		InterfaceElement& options = _layout["top"]["left"]["window-y"]["options"];
		if (options.clicked())
		{
			options.content().power();
		}
		else if (options.content().powered()
			&& Input::get()->wasKeyPressed(SDL_SCANCODE_LMB))
		{
			options.content().depower();
			std::string name = options.content().text();
			try
			{
				int y = stoi(name);
				_settings.windowY = y;

				_layout["bot"]["apply"].enable();
				_layout["bot"]["return"].setTag("discard");
			}
			catch (const std::logic_error&)
			{
				LOGE << "Cannot parse input '" << name << "'";
			}
		}
	}

	if (_layout["top"]["left"]["screenmode"]["options"]["dropdown"].clicked())
	{
		auto& options = _layout["top"]["left"]["screenmode"]["options"];
		auto& content = options["content"];
		auto& dropdown = options["dropdown"];
		for (size_t i = 0; i < content.size(); i++)
		{
			std::string name = content.name(i);
			if (dropdown.contains(name) && dropdown[name].clicked()
				&& content.getTag() != name)
			{
				try
				{
					int index = stoi(name);
					_settings.screenmode = (ScreenMode) index;
					content.setTag(name);
					fixScreenOptions();

					_layout["bot"]["apply"].enable();
					_layout["bot"]["return"].setTag("discard");
				}
				catch (const std::logic_error&)
				{
					LOGE << "Cannot parse dropdown option '" << name << "'";
				}
			}
		}
	}

	if (_layout["top"]["left"]["resolution"]["options"]["dropdowns"].clicked())
	{
		auto& options = _layout["top"]["left"]["resolution"]["options"];
		auto& content = options["content"];
		auto& dropdown = options["dropdowns"][options["dropdowns"].getTag()];
		for (size_t i = 0; i < content.size(); i++)
		{
			std::string name = content.name(i);
			if (dropdown.contains(name) && dropdown[name].clicked()
				&& content.getTag() != name)
			{
				content.setTag(name);
				fixScreenOptions();
			}
		}
	}

	if (_layout["top"]["left"]["width"].enabled())
	{
		InterfaceElement& options = _layout["top"]["left"]["width"]["options"];
		if (options.clicked())
		{
			options.content().power();
		}
		else if (options.content().powered()
			&& Input::get()->wasKeyPressed(SDL_SCANCODE_LMB))
		{
			options.content().depower();
			std::string name = options.content().text();
			try
			{
				int w = stoi(name);
				_settings.width = w;

				_layout["bot"]["apply"].enable();
				_layout["bot"]["return"].setTag("discard");
			}
			catch (const std::logic_error&)
			{
				LOGE << "Cannot parse input '" << name << "'";
			}
		}
	}

	if (_layout["top"]["left"]["height"].enabled())
	{
		InterfaceElement& options = _layout["top"]["left"]["height"]["options"];
		if (options.clicked())
		{
			options.content().power();
		}
		else if (options.content().powered()
			&& Input::get()->wasKeyPressed(SDL_SCANCODE_LMB))
		{
			options.content().depower();
			std::string name = options.content().text();
			try
			{
				int h = stoi(name);
				_settings.height = h;

				_layout["bot"]["apply"].enable();
				_layout["bot"]["return"].setTag("discard");
			}
			catch (const std::logic_error&)
			{
				LOGE << "Cannot parse input '" << name << "'";
			}
		}
	}

	if (_layout["top"]["left"]["scale"]["options"]["dropdown"].clicked())
	{
		auto& options = _layout["top"]["left"]["scale"]["options"];
		auto& content = options["content"];
		auto& dropdown = options["dropdown"];
		for (size_t i = 0; i < content.size(); i++)
		{
			std::string name = content.name(i);
			if (dropdown.contains(name) && dropdown[name].clicked()
				&& content.getTag() != name)
			{
				try
				{
					int index = stoi(name);
					_settings.scale = index;
					content.setTag(name);

					_layout["bot"]["apply"].enable();
					_layout["bot"]["return"].setTag("discard");
				}
				catch (const std::logic_error&)
				{
					LOGE << "Cannot parse dropdown option '" << name << "'";
				}
			}
		}
	}

	if (_layout["top"]["left"]["framerate"]["options"]["dropdown"].clicked())
	{
		auto& options = _layout["top"]["left"]["framerate"]["options"];
		auto& content = options["content"];
		auto& dropdown = options["dropdown"];
		for (size_t i = 0; i < content.size(); i++)
		{
			std::string name = content.name(i);
			if (dropdown.contains(name) && dropdown[name].clicked()
				&& content.getTag() != name)
			{
				try
				{
					int index = stoi(name);
					_settings.framerate = index;
					content.setTag(name);

					_layout["bot"]["apply"].enable();
					_layout["bot"]["return"].setTag("discard");
				}
				catch (const std::logic_error&)
				{
					LOGE << "Cannot parse dropdown option '" << name << "'";
				}
			}
		}
	}

	if (_layout["top"]["left"]["masterVolume"]["options"].held())
	{
		auto& options = _layout["top"]["left"]["masterVolume"]["options"];
		int mx = Input::get()->mousePixel().xenon;
		int sx = options["slider"].topleft().xenon;
		int value = std::max(0, std::min(100,
			(mx - sx) / InterfaceElement::scale()));
		_settings.masterVolume = 0.01f * value;
		options["slider"].fixWidth(value * InterfaceElement::scale());
		options["slider"].showIf(value > 0);
	}

	if (_layout["top"]["left"]["gameplayVolume"]["options"].held())
	{
		auto& options = _layout["top"]["left"]["gameplayVolume"]["options"];
		int mx = Input::get()->mousePixel().xenon;
		int sx = options["slider"].topleft().xenon;
		int value = std::max(0, std::min(100,
			(mx - sx) / InterfaceElement::scale()));
		_settings.gameplayVolume = 0.01f * value;
		options["slider"].fixWidth(value * InterfaceElement::scale());
		options["slider"].showIf(value > 0);
	}

	if (_layout["top"]["left"]["musicVolume"]["options"].held())
	{
		auto& options = _layout["top"]["left"]["musicVolume"]["options"];
		int mx = Input::get()->mousePixel().xenon;
		int sx = options["slider"].topleft().xenon;
		int value = std::max(0, std::min(100,
			(mx - sx) / InterfaceElement::scale()));
		_settings.musicVolume = 0.01f * value;
		options["slider"].fixWidth(value * InterfaceElement::scale());
		options["slider"].showIf(value > 0);
	}

	if (_layout["top"]["left"]["audioBufferSize"]["options"]["dropdown"].clicked())
	{
		auto& options = _layout["top"]["left"]["audioBufferSize"]["options"];
		auto& content = options["content"];
		auto& dropdown = options["dropdown"];
		for (size_t i = 0; i < content.size(); i++)
		{
			std::string name = content.name(i);
			if (dropdown.contains(name) && dropdown[name].clicked()
				&& content.getTag() != name)
			{
				try
				{
					int index = stoi(name);
					_settings.audioBufferSize = index;
					content.setTag(name);

					_layout["bot"]["apply"].enable();
					_layout["bot"]["return"].setTag("discard");
				}
				catch (const std::logic_error&)
				{
					LOGE << "Cannot parse dropdown option '" << name << "'";
				}
			}
		}
	}

	if (_layout["top"]["right"]["cameraScrollingWasd"]["options"].clicked())
	{
		auto& options = _layout["top"]["right"]["cameraScrollingWasd"]["options"];
		if (options.getTag() == "Empty")
		{
			_settings.cameraScrollingWasd = true;
			options.setTag("Checked");
		}
		else
		{
			_settings.cameraScrollingWasd = false;
			options.setTag("Empty");
		}
	}

	if (_layout["top"]["right"]["cameraScrollingArrows"]["options"].clicked())
	{
		auto& options = _layout["top"]["right"]["cameraScrollingArrows"]["options"];
		if (options.getTag() == "Empty")
		{
			_settings.cameraScrollingArrows = true;
			options.setTag("Checked");
		}
		else
		{
			_settings.cameraScrollingArrows = false;
			options.setTag("Empty");
		}
	}

	if (_layout["top"]["right"]["cameraScrollingEdge"]["options"].clicked())
	{
		auto& options = _layout["top"]["right"]["cameraScrollingEdge"]["options"];
		if (options.getTag() == "Empty")
		{
			_settings.cameraScrollingEdge = true;
			options.setTag("Checked");
		}
		else
		{
			_settings.cameraScrollingEdge = false;
			options.setTag("Empty");
		}
	}

	if (_layout["top"]["right"]["advancedMode"]["options"].clicked())
	{
		auto& options = _layout["top"]["right"]["advancedMode"]["options"];
		if (options.getTag() == "Empty")
		{
			_settings.allowScrollingWhileActing = true;
			_settings.allowScrollingWhilePanning = true;
			_settings.allowRedundantStop = true;
			_settings.allowRedundantCultivate = true;
			_settings.allowRedundantUpgrade = true;
			_settings.allowPowerless = true;
			_layout["top"]["right"]["allowScrollingWhileActing"]["options"].setTag("Checked");
			_layout["top"]["right"]["allowScrollingWhilePanning"]["options"].setTag("Checked");
			_layout["top"]["right"]["allowRedundantStop"]["options"].setTag("Checked");
			_layout["top"]["right"]["allowRedundantCultivate"]["options"].setTag("Checked");
			_layout["top"]["right"]["allowRedundantUpgrade"]["options"].setTag("Checked");
			_layout["top"]["right"]["allowPowerless"]["options"].setTag("Checked");
			_layout["top"]["right"]["allowScrollingWhileActing"]["options"].show();
			_layout["top"]["right"]["allowScrollingWhilePanning"]["options"].show();
			_layout["top"]["right"]["allowRedundantStop"]["options"].show();
			_layout["top"]["right"]["allowRedundantCultivate"]["options"].show();
			_layout["top"]["right"]["allowRedundantUpgrade"]["options"].show();
			_layout["top"]["right"]["allowPowerless"]["options"].show();
			_layout["top"]["right"]["allowScrollingWhileActing"].enable();
			_layout["top"]["right"]["allowScrollingWhilePanning"].enable();
			_layout["top"]["right"]["allowRedundantStop"].enable();
			_layout["top"]["right"]["allowRedundantCultivate"].enable();
			_layout["top"]["right"]["allowRedundantUpgrade"].enable();
			_layout["top"]["right"]["allowPowerless"].enable();
			options.setTag("Checked");
		}
		else
		{
			_settings.allowScrollingWhileActing = false;
			_settings.allowScrollingWhilePanning = false;
			_settings.allowRedundantStop = false;
			_settings.allowRedundantCultivate = false;
			_settings.allowRedundantUpgrade = false;
			_settings.allowPowerless = false;
			_layout["top"]["right"]["allowScrollingWhileActing"]["options"].setTag("Empty");
			_layout["top"]["right"]["allowScrollingWhilePanning"]["options"].setTag("Empty");
			_layout["top"]["right"]["allowRedundantStop"]["options"].setTag("Empty");
			_layout["top"]["right"]["allowRedundantCultivate"]["options"].setTag("Empty");
			_layout["top"]["right"]["allowRedundantUpgrade"]["options"].setTag("Empty");
			_layout["top"]["right"]["allowPowerless"]["options"].setTag("Empty");
			_layout["top"]["right"]["allowScrollingWhileActing"]["options"].hide();
			_layout["top"]["right"]["allowScrollingWhilePanning"]["options"].hide();
			_layout["top"]["right"]["allowRedundantStop"]["options"].hide();
			_layout["top"]["right"]["allowRedundantCultivate"]["options"].hide();
			_layout["top"]["right"]["allowRedundantUpgrade"]["options"].hide();
			_layout["top"]["right"]["allowPowerless"]["options"].hide();
			_layout["top"]["right"]["allowScrollingWhileActing"].disable();
			_layout["top"]["right"]["allowScrollingWhilePanning"].disable();
			_layout["top"]["right"]["allowRedundantStop"].disable();
			_layout["top"]["right"]["allowRedundantCultivate"].disable();
			_layout["top"]["right"]["allowRedundantUpgrade"].disable();
			_layout["top"]["right"]["allowPowerless"].disable();
			options.setTag("Empty");
		}
	}

	if (_layout["top"]["right"]["allowScrollingWhileActing"]["options"].clicked())
	{
		auto& options = _layout["top"]["right"]["allowScrollingWhileActing"]["options"];
		if (options.getTag() == "Empty")
		{
			_settings.allowScrollingWhileActing = true;
			options.setTag("Checked");
		}
		else
		{
			_settings.allowScrollingWhileActing = false;
			options.setTag("Empty");
		}
	}

	if (_layout["top"]["right"]["allowScrollingWhilePanning"]["options"].clicked())
	{
		auto& options = _layout["top"]["right"]["allowScrollingWhilePanning"]["options"];
		if (options.getTag() == "Empty")
		{
			_settings.allowScrollingWhilePanning = true;
			options.setTag("Checked");
		}
		else
		{
			_settings.allowScrollingWhilePanning = false;
			options.setTag("Empty");
		}
	}

	if (_layout["top"]["right"]["allowRedundantStop"]["options"].clicked())
	{
		auto& options = _layout["top"]["right"]["allowRedundantStop"]["options"];
		if (options.getTag() == "Empty")
		{
			_settings.allowRedundantStop = true;
			options.setTag("Checked");
		}
		else
		{
			_settings.allowRedundantStop = false;
			options.setTag("Empty");
		}
	}

	if (_layout["top"]["right"]["allowRedundantCultivate"]["options"].clicked())
	{
		auto& options = _layout["top"]["right"]["allowRedundantCultivate"]["options"];
		if (options.getTag() == "Empty")
		{
			_settings.allowRedundantCultivate = true;
			options.setTag("Checked");
		}
		else
		{
			_settings.allowRedundantCultivate = false;
			options.setTag("Empty");
		}
	}

	if (_layout["top"]["right"]["allowRedundantUpgrade"]["options"].clicked())
	{
		auto& options = _layout["top"]["right"]["allowRedundantUpgrade"]["options"];
		if (options.getTag() == "Empty")
		{
			_settings.allowRedundantUpgrade = true;
			options.setTag("Checked");
		}
		else
		{
			_settings.allowRedundantUpgrade = false;
			options.setTag("Empty");
		}
	}

	if (_layout["top"]["right"]["allowPowerless"]["options"].clicked())
	{
		auto& options = _layout["top"]["right"]["allowPowerless"]["options"];
		if (options.getTag() == "Empty")
		{
			_settings.allowPowerless = true;
			options.setTag("Checked");
		}
		else
		{
			_settings.allowPowerless = false;
			options.setTag("Empty");
		}
	}

	if (_layout["top"]["right"]["selectormode"]["options"]["dropdown"].clicked())
	{
		auto& options = _layout["top"]["right"]["selectormode"]["options"];
		auto& content = options["content"];
		auto& dropdown = options["dropdown"];
		for (size_t i = 0; i < content.size(); i++)
		{
			std::string name = content.name(i);
			if (dropdown.contains(name) && dropdown[name].clicked()
				&& content.getTag() != name)
			{
				try
				{
					int index = stoi(name);
					_settings.selectormode = (SelectorMode) index;
					content.setTag(name);
				}
				catch (const std::logic_error&)
				{
					LOGE << "Cannot parse dropdown option '" << name << "'";
				}
			}
		}
	}

	if (_layout["top"]["right"]["discord"]["options"].clicked())
	{
		auto& options = _layout["top"]["right"]["discord"]["options"];
		if (options.getTag() == "Empty")
		{
			_settings.discord = true;
			options.setTag("Checked");
			// Re-enabling this checkbox shows some amount of consent.
			_settings.allowDiscordLogin = true;
			_client.activateDiscord();
		}
		else
		{
			_settings.discord = false;
			options.setTag("Empty");
			_client.deactivateDiscord();
		}
	}

	if (_layout["top"]["right"]["all-chat"]["options"].clicked())
	{
		auto& options = _layout["top"]["right"]["all-chat"]["options"];
		if (options.getTag() == "Empty")
		{
			_settings.enableGeneralChat = true;
			options.setTag("Checked");
		}
		else
		{
			_settings.enableGeneralChat = false;
			options.setTag("Empty");
		}
	}

	if (_layout["top"]["right"]["lobby-chat"]["options"].clicked())
	{
		auto& options = _layout["top"]["right"]["lobby-chat"]["options"];
		if (options.getTag() == "Empty")
		{
			_settings.enableLobbyChat = true;
			options.setTag("Checked");
		}
		else
		{
			_settings.enableLobbyChat = false;
			options.setTag("Empty");
		}
	}

	if (_layout["top"]["right"]["artpanmode"]["options"]["dropdown"].clicked())
	{
		auto& options = _layout["top"]["right"]["artpanmode"]["options"];
		auto& content = options["content"];
		auto& dropdown = options["dropdown"];
		for (size_t i = 0; i < content.size(); i++)
		{
			std::string name = content.name(i);
			if (dropdown.contains(name) && dropdown[name].clicked()
				&& content.getTag() != name)
			{
				try
				{
					int index = stoi(name);
					_settings.artpanmode = (ArtPanMode) index;
					content.setTag(name);

					_layout["bot"]["apply"].enable();
					_layout["bot"]["return"].setTag("discard");
				}
				catch (const std::logic_error&)
				{
					LOGE << "Cannot parse dropdown option '" << name << "'";
				}
			}
		}
	}

	if (_layout["top"]["right"]["language"]["options"]["dropdown"].clicked())
	{
		auto& options = _layout["top"]["right"]["language"]["options"];
		auto& content = options["content"];
		auto& dropdown = options["dropdown"];
		for (size_t i = 0; i < content.size(); i++)
		{
			std::string name = content.name(i);
			if (dropdown.contains(name) && dropdown[name].clicked()
				&& content.getTag() != name)
			{
				_settings.language = name;
				content.setTag(name);

				_layout["bot"]["apply"].enable();
				_layout["bot"]["return"].setTag("discard");
			}
		}
	}

	if (_layout["top"]["right"]["textscale"]["options"]["dropdown"].clicked())
	{
		auto& options = _layout["top"]["right"]["textscale"]["options"];
		auto& content = options["content"];
		auto& dropdown = options["dropdown"];
		for (size_t i = 0; i < content.size(); i++)
		{
			std::string name = content.name(i);
			if (dropdown.contains(name) && dropdown[name].clicked()
				&& content.getTag() != name)
			{
				if (name == "100")
				{
					_settings.fontSize.clear();
				}
				else
				{
					int normalsize = _settings.getScaleBasedFontSize();
					try
					{
						int percent = std::stoi(name);
						int textsize = normalsize * percent / 100;
						_settings.fontSize = textsize;
					}
					catch (const std::logic_error&)
					{
						LOGE << "Cannot parse dropdown option '" << name << "'";
					}
				}
				content.setTag(name);

				_layout["bot"]["apply"].enable();
				_layout["bot"]["return"].setTag("discard");
			}
		}
	}

	if (_layout["top"]["right"]["highcontrast"]["options"].clicked())
	{
		auto& options = _layout["top"]["right"]["highcontrast"]["options"];
		if (options.getTag() == "Empty")
		{
			_settings.palette = "highcontrast";
			Palette::installNamed("highcontrast");
			options.setTag("Checked");
		}
		else
		{
			_settings.palette.clear();
			Palette::installDefault();
			options.setTag("Empty");

			auto& palette = _layout["top"]["right"]["palette"]["options"];
			palette["content"].setTag("default");
		}
	}

	_layout["top"]["right"]["palette"]["options"].enableIf(
		_layout["top"]["right"]["highcontrast"]["options"].getTag() == "Empty");

	if (_layout["top"]["right"]["palette"]["options"]["dropdown"].clicked())
	{
		auto& options = _layout["top"]["right"]["palette"]["options"];
		auto& content = options["content"];
		auto& dropdown = options["dropdown"];
		for (size_t i = 0; i < content.size(); i++)
		{
			std::string name = content.name(i);
			if (dropdown.contains(name) && dropdown[name].clicked())
			{
				if (name == "custom")
				{
					if (!Palette::exists("custom"))
					{
						Palette::saveInstalledAs("custom");
					}
					_owner.openPaletteEditor("custom");
				}

				if (content.getTag() != name)
				{
					if (Palette::exists(name))
					{
						_settings.palette = name;
						Palette::installNamed(name);
						content.setTag(name);
					}
				}
			}
		}
	}

	if (_layout["bot"]["apply"].clicked())
	{
		_settings.rememberPreviousContents();
		_settings.save();
		_owner.quit(ExitCode::APPLY_SETTINGS_AND_RESTART);
	}
	else if (_layout["bot"]["return"].getTag() == "apply"
		&& _layout["bot"]["return"]["apply"].clicked())
	{
		_settings.rememberedPreviousContents.clear();
		_settings.save();
		quit();
	}
	else if (_layout["bot"]["return"].getTag() == "discard"
		&& _layout["bot"]["return"]["discard"].clicked())
	{
		_settings.load();
		quit();
	}
}

void SettingsMenu::onOpen()
{
	Menu::onOpen();

	load();
}

void SettingsMenu::load()
{
	const int FONTSIZE = _settings.getFontSize();

	{
		InterfaceElement& options =
			_layout["top"]["left"]["display"]["options"];
		InterfaceElement& content = options["content"];
		std::string tag = std::to_string(
			(int) _settings.display.value());
		if (content.contains(tag))
		{
			content.setTag(tag);
		}
	}

	{
		InterfaceElement& options =
			_layout["top"]["left"]["window-position"]["options"];
		InterfaceElement& content = options["content"];
		if (_settings.windowX.value() < 0
			|| _settings.windowY.value() < 0)
		{
			content.setTag("centered");
		}
		else
		{
			content.setTag("custom");
		}
	}

	if (_settings.windowX.value() >= 0)
	{
		_layout["top"]["left"]["window-x"]["options"].setText(
			std::to_string(_settings.windowX.value()));
	}
	else
	{
		_layout["top"]["left"]["window-x"]["options"].setText("");
	}
	_layout["top"]["left"]["window-x"].enableIf(
		_layout["top"]["left"]["window-position"]["options"]["content"].getTag()
			== "custom");

	if (_settings.windowY.value() >= 0)
	{
		_layout["top"]["left"]["window-y"]["options"].setText(
			std::to_string(_settings.windowY.value()));
	}
	else
	{
		_layout["top"]["left"]["window-y"]["options"].setText("");
	}
	_layout["top"]["left"]["window-y"].enableIf(
		_layout["top"]["left"]["window-position"]["options"]["content"].getTag()
			== "custom");

	{
		InterfaceElement& options = _layout["top"]["left"]["screenmode"]["options"];
		InterfaceElement& content = options["content"];
		content.setTag(std::to_string(
			(int) _settings.screenmode.value()));
	}

	{
		InterfaceElement& options = _layout["top"]["left"]["resolution"]["options"];
		InterfaceElement& content = options["content"];
		std::string mode = _layout["top"]["left"]["screenmode"]["options"]["content"].getTag();
		if (mode == std::to_string((int) ScreenMode::FULLSCREEN))
		{
			options["dropdowns"].setTag("detected");
			std::string reso = std::to_string(_settings.width.value())
				+ "x" + std::to_string(_settings.height.value());
			if (content.contains(reso))
			{
				content.setTag(reso);
			}
			else
			{
				auto& detected = options["dropdowns"]["detected"];
				std::string newname = detected.name(detected.size() - 1);
				content.setTag(newname);
			}
		}
		else
		{
			options["dropdowns"].setTag("full");
			std::string reso = std::to_string(_settings.width.value())
				+ "x" + std::to_string(_settings.height.value());
			if (content.contains(reso))
			{
				content.setTag(reso);
			}
			else
			{
				content.setTag("custom");
			}
		}
		options.enableIf(mode != std::to_string((int) ScreenMode::DESKTOP));
	}

	_layout["top"]["left"]["width"]["options"].setText(
		std::to_string(_settings.width.value()));
	_layout["top"]["left"]["width"]["options"].enableIf(
		_layout["top"]["left"]["resolution"]["options"]["content"].getTag()
			== "custom");

	_layout["top"]["left"]["height"]["options"].setText(
		std::to_string(_settings.height.value()));
	_layout["top"]["left"]["height"]["options"].enableIf(
		_layout["top"]["left"]["resolution"]["options"]["content"].getTag()
			== "custom");

	// TODO is all the above necessary when we also do this?
	fixScreenOptions();

	{
		InterfaceElement& options = _layout["top"]["left"]["scale"]["options"];
		InterfaceElement& content = options["content"];
		std::string tag = std::to_string(
			(int) _settings.scale.value());
		if (content.contains(tag))
		{
			content.setTag(tag);
		}
	}

	{
		InterfaceElement& options = _layout["top"]["left"]["framerate"]["options"];
		InterfaceElement& content = options["content"];
		std::string tag = std::to_string(
			(int) _settings.framerate.value());
		if (content.contains(tag))
		{
			content.setTag(tag);
		}
	}

	{
		int value = std::max(0, std::min(100,
			(int) (100.0f * _settings.masterVolume.value())));
		_layout["top"]["left"]["masterVolume"]["options"]["slider"].fixWidth(
			value * InterfaceElement::scale());
		_layout["top"]["left"]["masterVolume"]["options"]["slider"].showIf(
			value > 0);
	}

	{
		int value = std::max(0, std::min(100,
			(int) (100.0f * _settings.gameplayVolume.value())));
		_layout["top"]["left"]["gameplayVolume"]["options"]["slider"].fixWidth(
			value * InterfaceElement::scale());
		_layout["top"]["left"]["gameplayVolume"]["options"]["slider"].showIf(
			value > 0);
	}

	{
		int value = std::max(0, std::min(100,
			(int) (100.0f * _settings.musicVolume.value())));
		_layout["top"]["left"]["musicVolume"]["options"]["slider"].fixWidth(
			value * InterfaceElement::scale());
		_layout["top"]["left"]["musicVolume"]["options"]["slider"].showIf(
			value > 0);
	}

	{
		InterfaceElement& options = _layout["top"]["left"]["audioBufferSize"]["options"];
		InterfaceElement& content = options["content"];
		content.setTag(std::to_string(
			(int) _settings.audioBufferSize.value()));
	}

	{
		InterfaceElement& options = _layout["top"]["right"]["selectormode"]["options"];
		InterfaceElement& content = options["content"];
		content.setTag(std::to_string(
			(int) _settings.selectormode.value()));
	}

	if (_settings.cameraScrollingWasd.value())
	{
		_layout["top"]["right"]["cameraScrollingWasd"]["options"].setTag("Checked");
	}
	else
	{
		_layout["top"]["right"]["cameraScrollingWasd"]["options"].setTag("Empty");
	}

	if (_settings.cameraScrollingArrows.value())
	{
		_layout["top"]["right"]["cameraScrollingArrows"]["options"].setTag("Checked");
	}
	else
	{
		_layout["top"]["right"]["cameraScrollingArrows"]["options"].setTag("Empty");
	}

	if (_settings.cameraScrollingEdge.value())
	{
		_layout["top"]["right"]["cameraScrollingEdge"]["options"].setTag("Checked");
	}
	else
	{
		_layout["top"]["right"]["cameraScrollingEdge"]["options"].setTag("Empty");
	}

	if (_settings.allowScrollingWhileActing.value()
		|| _settings.allowScrollingWhilePanning.value()
		|| _settings.allowRedundantStop.value()
		|| _settings.allowRedundantCultivate.value()
		|| _settings.allowRedundantUpgrade.value()
		|| _settings.allowPowerless.value())
	{
		_layout["top"]["right"]["advancedMode"]["options"].setTag("Checked");
	}
	else
	{
		_layout["top"]["right"]["advancedMode"]["options"].setTag("Empty");
		_layout["top"]["right"]["allowScrollingWhileActing"]["options"].hide();
		_layout["top"]["right"]["allowScrollingWhilePanning"]["options"].hide();
		_layout["top"]["right"]["allowRedundantStop"]["options"].hide();
		_layout["top"]["right"]["allowRedundantCultivate"]["options"].hide();
		_layout["top"]["right"]["allowRedundantUpgrade"]["options"].hide();
		_layout["top"]["right"]["allowPowerless"]["options"].hide();
		_layout["top"]["right"]["allowScrollingWhileActing"].disable();
		_layout["top"]["right"]["allowScrollingWhilePanning"].disable();
		_layout["top"]["right"]["allowRedundantStop"].disable();
		_layout["top"]["right"]["allowRedundantCultivate"].disable();
		_layout["top"]["right"]["allowRedundantUpgrade"].disable();
		_layout["top"]["right"]["allowPowerless"].disable();
	}

	if (_settings.allowScrollingWhileActing.value())
	{
		_layout["top"]["right"]["allowScrollingWhileActing"]["options"].setTag("Checked");
	}
	else
	{
		_layout["top"]["right"]["allowScrollingWhileActing"]["options"].setTag("Empty");
	}

	if (_settings.allowScrollingWhilePanning.value())
	{
		_layout["top"]["right"]["allowScrollingWhilePanning"]["options"].setTag("Checked");
	}
	else
	{
		_layout["top"]["right"]["allowScrollingWhilePanning"]["options"].setTag("Empty");
	}

	if (_settings.allowRedundantStop.value())
	{
		_layout["top"]["right"]["allowRedundantStop"]["options"].setTag("Checked");
	}
	else
	{
		_layout["top"]["right"]["allowRedundantStop"]["options"].setTag("Empty");
	}

	if (_settings.allowRedundantCultivate.value())
	{
		_layout["top"]["right"]["allowRedundantCultivate"]["options"].setTag("Checked");
	}
	else
	{
		_layout["top"]["right"]["allowRedundantCultivate"]["options"].setTag("Empty");
	}

	if (_settings.allowRedundantUpgrade.value())
	{
		_layout["top"]["right"]["allowRedundantUpgrade"]["options"].setTag("Checked");
	}
	else
	{
		_layout["top"]["right"]["allowRedundantUpgrade"]["options"].setTag("Empty");
	}

	if (_settings.allowPowerless.value())
	{
		_layout["top"]["right"]["allowPowerless"]["options"].setTag("Checked");
	}
	else
	{
		_layout["top"]["right"]["allowPowerless"]["options"].setTag("Empty");
	}

	if (_settings.discord.value())
	{
		_layout["top"]["right"]["discord"]["options"].setTag("Checked");
	}
	else
	{
		_layout["top"]["right"]["discord"]["options"].setTag("Empty");
	}

	if (_settings.enableGeneralChat.value())
	{
		_layout["top"]["right"]["all-chat"]["options"].setTag("Checked");
	}
	else
	{
		_layout["top"]["right"]["all-chat"]["options"].setTag("Empty");
	}

	if (_settings.enableLobbyChat.value())
	{
		_layout["top"]["right"]["lobby-chat"]["options"].setTag("Checked");
	}
	else
	{
		_layout["top"]["right"]["lobby-chat"]["options"].setTag("Empty");
	}

	if (_settings.palette.value("") == "highcontrast")
	{
		_layout["top"]["right"]["highcontrast"]["options"].setTag("Checked");
	}
	else
	{
		_layout["top"]["right"]["highcontrast"]["options"].setTag("Empty");
	}

	{
		InterfaceElement& x = _layout["top"]["right"]["palette"];
		InterfaceElement& options = x["options"];
		InterfaceElement& content = options["content"];
		InterfaceElement& dropdown = options["dropdown"];
		for (size_t i = 0; i < dropdown.size(); i++)
		{
			const std::string& name = dropdown.name(i);
			if (name == "default") continue;
			if (name == "custom") continue;
			if (!Palette::exists(name))
			{
				dropdown.remove(name);
				break;
			}
		}
		bool anychanges = false;
		for (const auto& item : Palette::externalItems())
		{
			const std::string& tagname = item.uniqueTag;
			if (!content.contains(tagname))
			{
				anychanges = true;
				content.add(tagname, Frame::makeItem(ColorName::FRAMEITEM));
				content[tagname].put(new TextField(item.quotedName, FONTSIZE));
				content[tagname].align(HorizontalAlignment::LEFT);
				content[tagname].makeClickable();
			}
			if (!dropdown.contains(tagname))
			{
				anychanges = true;
				dropdown.add(tagname, Frame::makeItem());
				dropdown[tagname].put(new TextField(item.quotedName, FONTSIZE));
				dropdown[tagname].align(HorizontalAlignment::LEFT);
				dropdown[tagname].makeClickable();
			}
		}
		if (anychanges)
		{
			int w = x.width();
			content.unfixWidth();
			content.settleWidth();
			content.fixWidth();
			options.settleWidth();
			x.setWidth(w);
			x.place(x.topleft());
			auto custom = dropdown.remove("custom");
			if (custom)
			{
				dropdown.add("custom", std::move(custom));
			}
			dropdown.unfixWidth();
			dropdown.settle();
			dropdown.fixWidth();
		}
	}

	if (_settings.palette.defined())
	{
		InterfaceElement& x = _layout["top"]["right"]["palette"];
		InterfaceElement& options = x["options"];
		InterfaceElement& content = options["content"];
		if (content.contains(_settings.palette.value()))
		{
			content.setTag(_settings.palette.value());
		}
		else
		{
			content.setTag("custom");
		}
	}

	{
		InterfaceElement& x = _layout["top"]["right"]["artpanmode"];
		InterfaceElement& options = x["options"];
		InterfaceElement& content = options["content"];
		content.setTag(std::to_string(
			(int) _settings.artpanmode.value()));
	}

	{
		InterfaceElement& x = _layout["top"]["right"]["language"];
		InterfaceElement& options = x["options"];
		InterfaceElement& content = options["content"];
		bool found = false;
		for (size_t i = 0; i < content.size(); i++)
		{
			const std::string& name = content.name(i);
			if (name == _settings.language.value("en_US"))
			{
				content.setTag(name);
				found = true;
			}
		}
		if (!found)
		{
			x.disable();
		}
	}

	{
		InterfaceElement& x = _layout["top"]["right"]["textscale"];
		InterfaceElement& options = x["options"];
		InterfaceElement& content = options["content"];
		int normalsize = _settings.getScaleBasedFontSize();
		int percent = 100;
		if (_settings.fontSize.defined())
		{
			percent = 100 * _settings.fontSize.value() / normalsize;
		}
		std::string name = std::to_string(percent);
		if (content.contains(name))
		{
			content.setTag(name);
		}
		else
		{
			x.disable();
		}
	}

	// We only need to restart if the video settings are changed.
	_layout["bot"]["apply"].disable();
	_layout["bot"]["return"].setTag("apply");
}
