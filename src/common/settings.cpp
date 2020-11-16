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
#include "settings.hpp"
#include "source.hpp"

#include "system.hpp"


std::string Settings::_configroot = "";

void Settings::setRoot(const std::string& root)
{
	if (root.empty())
	{
		_configroot = "";
	}
	else if (root.back() == '/')
	{
		_configroot = root;
	}
	else
	{
		_configroot = root + "/";
	}
}

void Settings::help() const
{
	std::cout << "Settings:" << std::endl
		<< "  TODO" << std::endl;
}

Settings::Settings(std::unique_ptr<Settings> fallback) :
	_fallback(std::move(fallback)),
	_askedForHelp(false),
	logname(this, "logname"),
	loglevel(this, "loglevel"),
	logrollback(this, "logrollback"),
	perflog(this, "perflog"),
	configRoot(this, "config-root"),
	dataRoot(this, "data-root", "data-folder"),
	cacheRoot(this, "cache-root"),
	resourceRoot(this, "resource-root"),
	seed(this, "seed"),
	display(this, "display"),
	screenmode(this, "screenmode"),
	windowX(this, "window-x"),
	windowY(this, "window-y"),
	width(this, "width"),
	height(this, "height"),
	scale(this, "scale"),
	framerate(this, "framerate"),
	finishRendering(this, "finish-rendering"),
	steam(this, "steam"),
	discord(this, "discord"),
	allowDiscordLogin(this, "allow-discord-login"),
	useApi(this, "use-api"),
	server(this, "server"),
	port(this, "port"),
	patchmode(this, "patchmode"),
	selectormode(this, "selectormode"),
	artpanmode(this, "artpanmode"),
	stomtToken(this, "stomt-token"),
	memento(this, nullptr),
	cameraScrollingWasd(this, "camera-scrolling-wasd"),
	cameraScrollingArrows(this, "camera-scrolling-arrows"),
	cameraScrollingEdge(this, "camera-scrolling-edge"),
	allowScrollingWhileActing(this, "allow-scrolling-while-acting"),
	allowScrollingWhilePanning(this, "allow-scrolling-while-panning"),
	allowRedundantStop(this, "allow-redundant-stop"),
	allowRedundantCultivate(this, "allow-redundant-cultivate"),
	allowRedundantUpgrade(this, "allow-redundant-upgrade"),
	allowPowerless(this, "allow-powerless"),
	slackname(this, "slackname"),
	slackurl(this, "slackurl"),
	discordurl(this, "discordurl"),
	audioBufferSize(this, "audio-buffer-size"),
	masterVolume(this, "master-volume"),
	gameplayVolume(this, "gameplay-volume"),
	musicVolume(this, "music-volume"),
	skipAnimations(this, "skip-animations"),
	skipLighting(this, "skip-lighting"),
	skipPanning(this, "skip-panning"),
	silentPlanningBell(this, "silent-planning-bell"),
	hideIdleAnimations(this, "hide-idle-animations"),
	hideLayouts(this, "hide-layouts"),
	showViewport(this, "show-viewport"),
	language(this, "language"),
	palette(this, "palette"),
	fontFilename(this, "font-filename"),
	fontSize(this, "font-size"),
	fontSizeTextInput(this, "font-size-text-input"),
	fontSizeMenuHeader(this, "font-size-menu-header"),
	fontSizeMenuButton(this, "font-size-menu-button"),
	fontSizePlayButton(this, "font-size-play-button"),
	fontSizeReadyButton(this, "font-size-ready-button"),
	fontSizeWallet(this, "font-size-wallet"),
	fontSizeTutorial(this, "font-size-tutorial"),
	fontSizeHeadline(this, "font-size-headline"),
	rememberedPreviousContents(this, "remembered-previous-contents"),
	settings(this, "settings", "defaults")
{}

Settings::Settings() :
	Settings(nullptr)
{
	display = 1;
	screenmode = ScreenMode::DESKTOP;
	scale = 2;
	framerate = 60;
	discord = true;
	allowDiscordLogin = false;
	patchmode = detectPatchMode();
	selectormode = SelectorMode::FIGURE;
	artpanmode = ArtPanMode::AUTO;
	cameraScrollingWasd = true;
	cameraScrollingArrows = true;
	cameraScrollingEdge = true;
	allowScrollingWhileActing = false;
	allowScrollingWhilePanning = false;
	allowRedundantStop = false;
	allowRedundantCultivate = false;
	allowRedundantUpgrade = false;
	allowPowerless = false;
	audioBufferSize = 4096;
	masterVolume = 1.0f;
	gameplayVolume = 1.0f;
	musicVolume = 1.0f;

#ifdef DEVELOPMENT
#ifdef CANDIDATE
	dataRoot = System::getPersistentDataRoot();
	cacheRoot = System::getPersistentCacheRoot();
#else
	// Use the working directory.
#endif
#else
	dataRoot = System::getPersistentDataRoot();
	cacheRoot = System::getPersistentCacheRoot();
#endif

#ifdef PLATFORMOSX
	// In OSX we now use a proper epicinium.app file.
	if (System::isDirectory("../Resources"))
	{
		resourceRoot = "../Resources/";
	}
#endif
}

Settings::Settings(const std::string& fname,
		std::unique_ptr<Settings> fallback) :
	Settings(std::move(fallback))
{
	_filename = fname;

	load();
}

Settings::Settings(const std::string& fname,
		int argc, const char* const argv[]) :
	Settings(std::unique_ptr<Settings>(new Settings(fname,
		std::unique_ptr<Settings>(new Settings()))))
{
	for (int i = 1; i < argc; i++)
	{
		const char* arg = argv[i];
		if (strncmp(arg, "--", 2) == 0 && override(arg + 2))
		{
			// Override successful.
		}
		else if (strncmp(arg, "-", 1) == 0)
		{
			if (strcmp(arg, "-h") == 0 || strcmp(arg, "-?") == 0
				|| strcmp(arg, "--help") == 0)
			{
				_askedForHelp = true;
			}
			else _unknowns.emplace_back(arg);
		}
		else
		{
			// Not a setting, will be handled by application.
		}
	}

	for (const std::string& unknown : _unknowns)
	{
		std::cerr << "Unknown setting '" << unknown << "'" << std::endl;
	}
}

Settings::Settings(int argc, const char* const argv[]) :
	Settings(std::unique_ptr<Settings>(new Settings()))
{
	for (int i = 1; i < argc; i++)
	{
		const char* arg = argv[i];
		if (strncmp(arg, "--", 2) == 0 && override(arg + 2))
		{
			// Override successful.
		}
		else if (strncmp(arg, "-", 1) == 0)
		{
			_unknowns.emplace_back(arg);
		}
		else
		{
			// Not a setting, will be handled by application.
		}
	}

	for (const std::string& unknown : _unknowns)
	{
		std::cerr << "Unknown setting '" << unknown << "'" << std::endl;
	}
}

Settings::Settings(Json::Value& json) :
	Settings(std::unique_ptr<Settings>(new Settings()))
{
	parse(json);
}

size_t Settings::push(Setting<bool>* it)
{
	size_t index = _boollist.size();
	_boollist.emplace_back(it);
	return index;
}

size_t Settings::push(Setting<ScreenMode>* it)
{
	size_t index = _screenmodelist.size();
	_screenmodelist.emplace_back(it);
	return index;
}

size_t Settings::push(Setting<SelectorMode>* it)
{
	size_t index = _selectormodelist.size();
	_selectormodelist.emplace_back(it);
	return index;
}

size_t Settings::push(Setting<ArtPanMode>* it)
{
	size_t index = _artpanmodelist.size();
	_artpanmodelist.emplace_back(it);
	return index;
}

size_t Settings::push(Setting<PatchMode>* it)
{
	size_t index = _patchmodelist.size();
	_patchmodelist.emplace_back(it);
	return index;
}

size_t Settings::push(Setting<int>* it)
{
	size_t index = _intlist.size();
	_intlist.emplace_back(it);
	return index;
}

size_t Settings::push(Setting<float>* it)
{
	size_t index = _floatlist.size();
	_floatlist.emplace_back(it);
	return index;
}

size_t Settings::push(Setting<std::string>* it)
{
	size_t index = _stringlist.size();
	_stringlist.emplace_back(it);
	return index;
}

Setting<bool>& Settings::get(size_t index, bool /**/)
{
	return *_boollist[index];
}

Setting<ScreenMode>& Settings::get(size_t index, ScreenMode /**/)
{
	return *_screenmodelist[index];
}

Setting<SelectorMode>& Settings::get(size_t index, SelectorMode /**/)
{
	return *_selectormodelist[index];
}

Setting<ArtPanMode>& Settings::get(size_t index, ArtPanMode /**/)
{
	return *_artpanmodelist[index];
}

Setting<PatchMode>& Settings::get(size_t index, PatchMode /**/)
{
	return *_patchmodelist[index];
}

Setting<int>& Settings::get(size_t index, int /**/)
{
	return *_intlist[index];
}

Setting<float>& Settings::get(size_t index, float /**/)
{
	return *_floatlist[index];
}

Setting<std::string>& Settings::get(size_t index, const std::string& /**/)
{
	return *_stringlist[index];
}

Settings& Settings::defaults()
{
	if (_fallback != nullptr)
	{
		return _fallback->defaults();
	}
	else
	{
		return *this;
	}
}

void Settings::setFallback(const std::string& fname)
{
	// TODO check against circular settings
	_fallback.reset(new Settings(fname, std::move(_fallback)));
}

void Settings::load()
{
	if (!saveable())
	{
		if (_fallback)
		{
			_fallback->load();
		}
		return;
	}

	Json::Reader reader;
	Json::Value root;
	std::ifstream file = System::ifstream(_filename);
	if (!file.is_open())
	{
		std::cerr << "Settings file " << _filename
			<< " could not be loaded, using defaults" << std::endl;
		return;
	}
	if (!reader.parse(file, root))
	{
		std::cerr << "Settings file " << _filename
			<< " could not be read" << std::endl;
		return;
	}

	parse(root);
}

bool Settings::parse(const Json::Value& root)
{
	bool used = false;

	for (auto setting : _boollist)
	{
		used |= setting->parse(root);
	}
	for (auto setting : _screenmodelist)
	{
		used |= setting->parse(root);
	}
	for (auto setting : _selectormodelist)
	{
		used |= setting->parse(root);
	}
	for (auto setting : _artpanmodelist)
	{
		used |= setting->parse(root);
	}
	for (auto setting : _patchmodelist)
	{
		used |= setting->parse(root);
	}
	for (auto setting : _intlist)
	{
		used |= setting->parse(root);
	}
	for (auto setting : _floatlist)
	{
		used |= setting->parse(root);
	}
	for (auto setting : _stringlist)
	{
		used |= setting->parse(root);
	}

	// Note _defined instead of defined(), because we want to know if this
	// itself is the Settings layer that has a fallback defined.
	if (settings._defined)
	{
		setFallback(settings.value());
	}

	return used;
}

bool Settings::override(const char* arg)
{
	const char* equals = strchr(arg, '=');
	if (equals == nullptr) return false;
	std::string left(arg, equals - arg);
	std::string right(equals + 1);
	Json::Value json;
	{
		json[left] = right;
	}
	return parse(json);
}

PatchMode Settings::detectPatchMode()
{
#if STEAM_ENABLED
	return PatchMode::STEAM;
#else
	if (System::isDirectory(".itch"))
	{
		return PatchMode::ITCHIO;
	}
	else if (System::isFile(".gj-credentials"))
	{
		return PatchMode::GAMEJOLT;
	}
	else
	{
#if SELF_PATCH_ENABLED
		return PatchMode::SERVER;
#else
		return PatchMode::NONE;
#endif
	}
#endif
}

void Settings::store(Json::Value& root)
{
	for (auto setting : _boollist)
	{
		setting->store(root);
	}
	for (auto setting : _screenmodelist)
	{
		setting->store(root);
	}
	for (auto setting : _selectormodelist)
	{
		setting->store(root);
	}
	for (auto setting : _artpanmodelist)
	{
		setting->store(root);
	}
	for (auto setting : _patchmodelist)
	{
		setting->store(root);
	}
	for (auto setting : _intlist)
	{
		setting->store(root);
	}
	for (auto setting : _floatlist)
	{
		setting->store(root);
	}
	for (auto setting : _stringlist)
	{
		setting->store(root);
	}
}

void Settings::save()
{
	if (!saveable())
	{
		if (_fallback)
		{
			_fallback->save();
		}
		return;
	}

	Json::Value root(Json::objectValue);

	{
		Json::Reader reader;
		std::ifstream file = System::ifstream(_filename);
		if (!file.is_open())
		{
			System::touchFile(_filename);
		}
		else if (!reader.parse(file, root))
		{
			std::cerr << "Settings file " << _filename
				<< " could not be read, using defaults" << std::endl;
			root = Json::Value(Json::objectValue);
		}
	}

	store(root);

	{
		std::ofstream file = System::ofstream(_filename);
		if (!file.is_open())
		{
			std::cerr << "Settings file " << _filename
				<< " could not be opened for writing" << std::endl;
			return;
		}
		file << root << std::endl;
	}
}

bool Settings::saveable()
{
	// We don't want to save the overrides, so only the Settings layer that
	// has a filename defined is saveable.
	return !_filename.empty();
}

void Settings::rememberPreviousContents()
{
	if (!saveable())
	{
		if (_fallback)
		{
			_fallback->rememberPreviousContents();
		}
		return;
	}

	if (System::isFile(_filename))
	{
		Json::Reader reader;
		std::ifstream file = System::ifstream(_filename);
		if (file)
		{
			std::stringstream strm;
			strm << file.rdbuf();
			rememberedPreviousContents = strm.str();
		}
		else
		{
			std::cerr << "Settings file " << _filename
				<< " could not be read, remembering defaults" << std::endl;
			rememberedPreviousContents = "{}";
		}
	}
	else
	{
		// If there were no previous settings saved, we want the player to
		// either confirm the new settings or revert to default, so we remember
		// an empty JSON.
		rememberedPreviousContents = "{}";
	}
}

void Settings::revertToPreviousContents()
{
	if (!saveable())
	{
		if (_fallback)
		{
			_fallback->revertToPreviousContents();
		}
		return;
	}

	System::touchFile(_filename);

	{
		std::ofstream file = System::ofstream(_filename);
		if (!file.is_open())
		{
			std::cerr << "Settings file " << _filename
				<< " could not be opened for writing" << std::endl;
			return;
		}
		file << rememberedPreviousContents.value("{}");
	}

	load();

	rememberedPreviousContents.clear();
}

Json::Value Settings::flattenIntoJson()
{
	Json::Value root(Json::objectValue);

	for (auto setting : _boollist)
	{
		if (setting->defined()) setting->definition().store(root);
	}
	for (auto setting : _screenmodelist)
	{
		if (setting->defined()) setting->definition().store(root);
	}
	for (auto setting : _selectormodelist)
	{
		if (setting->defined()) setting->definition().store(root);
	}
	for (auto setting : _artpanmodelist)
	{
		if (setting->defined()) setting->definition().store(root);
	}
	for (auto setting : _patchmodelist)
	{
		if (setting->defined()) setting->definition().store(root);
	}
	for (auto setting : _intlist)
	{
		if (setting->defined()) setting->definition().store(root);
	}
	for (auto setting : _floatlist)
	{
		if (setting->defined()) setting->definition().store(root);
	}
	for (auto setting : _stringlist)
	{
		if (setting->defined()) setting->definition().store(root);
	}

	return root;
}

int Settings::getScaleBasedFontSize() const
{
	return 8 * scale.value(2);
}

int Settings::getFontSize() const
{
	return fontSize.value(getScaleBasedFontSize());
}

int Settings::getFontSizeTextInput() const
{
	return fontSizeTextInput.value(getFontSize());
}

int Settings::getFontSizeMenuHeader() const
{
	return fontSizeMenuHeader.value(2 * getFontSize());
}

int Settings::getFontSizeMenuButton() const
{
	return fontSizeMenuButton.value(getFontSize());
}

int Settings::getFontSizePlayButton() const
{
	return fontSizePlayButton.value(3 * getFontSize());
}

int Settings::getFontSizeReadyButton() const
{
	return fontSizeReadyButton.value(2 * getFontSize());
}

int Settings::getFontSizeWallet() const
{
	return fontSizeWallet.value(2 * getFontSize());
}

int Settings::getFontSizeTutorial() const
{
	return fontSizeTutorial.value(getFontSize());
}

int Settings::getFontSizeHeadline() const
{
	return fontSizeHeadline.value(3 * getFontSize());
}

void Settings::determineCompatibleRoot(const std::string& settingsfilename,
	int argc, const char* const argv[])
{
	try
	{
		Settings argsettings(argc, argv);
		if (argsettings.configRoot.defined())
		{
			Settings::setRoot(argsettings.configRoot.value());
			return;
		}
	}
	catch (...)
	{
		// Ignored.
	}

#ifdef DEVELOPMENT
#ifndef CANDIDATE
	Settings::setRoot("");
	return;
#endif
#endif

	if (!System::isFile(settingsfilename))
	{
		Settings::setRoot(System::getPersistentConfigRoot());
		return;
	}

	try
	{
		Settings localsettings(settingsfilename, argc, argv);
		if (localsettings.configRoot.defined()
			&& localsettings.configRoot.value() == "")
		{
			Settings::setRoot("");
			return;
		}
	}
	catch (...)
	{
		// Ignored.
	}

	Settings::setRoot(System::getPersistentConfigRoot());
}
