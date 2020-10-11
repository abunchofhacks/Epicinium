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

#include "setting.hpp"



class Settings
{
private:
	// Pre-initialized.
	Settings(std::unique_ptr<Settings> fallback);

	// Default initialized.
	Settings();

	// The layer below the top level Settings.
	Settings(const std::string& fname, std::unique_ptr<Settings> fallback);

public:
	// Standard application settings.
	Settings(const std::string& fname, int argc, const char* const argv[]);

	// Unsaveable library settings.
	Settings(int argc, const char* const argv[]);

	// Unsaveable copied settings.
	explicit Settings(Json::Value& json);

	Settings(const Settings&) = delete;
	Settings(Settings&&) = delete;
	Settings& operator=(const Settings&) = delete;
	Settings& operator=(Settings&&) = delete;
	~Settings() = default;

private:
	std::unique_ptr<Settings> _fallback;

	std::vector<Setting<bool>*> _boollist;
	std::vector<Setting<ScreenMode>*> _screenmodelist;
	std::vector<Setting<SelectorMode>*> _selectormodelist;
	std::vector<Setting<ArtPanMode>*> _artpanmodelist;
	std::vector<Setting<PatchMode>*> _patchmodelist;
	std::vector<Setting<int>*> _intlist;
	std::vector<Setting<float>*> _floatlist;
	std::vector<Setting<std::string>*> _stringlist;

	friend Setting<bool>;
	friend Setting<ScreenMode>;
	friend Setting<SelectorMode>;
	friend Setting<ArtPanMode>;
	friend Setting<PatchMode>;
	friend Setting<int>;
	friend Setting<float>;
	friend Setting<std::string>;

	size_t push(Setting<bool>*);
	size_t push(Setting<ScreenMode>*);
	size_t push(Setting<SelectorMode>*);
	size_t push(Setting<ArtPanMode>*);
	size_t push(Setting<PatchMode>*);
	size_t push(Setting<int>*);
	size_t push(Setting<float>*);
	size_t push(Setting<std::string>*);

	Setting<bool>& get(size_t index, bool dummy);
	Setting<ScreenMode>& get(size_t index, ScreenMode dummy);
	Setting<SelectorMode>& get(size_t index, SelectorMode dummy);
	Setting<ArtPanMode>& get(size_t index, ArtPanMode dummy);
	Setting<PatchMode>& get(size_t index, PatchMode dummy);
	Setting<int>& get(size_t index, int dummy);
	Setting<float>& get(size_t index, float dummy);
	Setting<std::string>& get(size_t index, const std::string& dummy);

	std::string _filename;

	bool _askedForHelp;
	std::vector<std::string> _unknowns;

	bool saveable();

public:
	Setting<std::string> logname;
	Setting<std::string> loglevel;
	Setting<int> logrollback;
	Setting<std::string> perflog;
	Setting<std::string> configRoot;
	Setting<std::string> dataRoot;
	Setting<std::string> cacheRoot;
	Setting<std::string> resourceRoot;
	Setting<int> seed;
	Setting<int> display;
	Setting<ScreenMode> screenmode;
	Setting<int> windowX;
	Setting<int> windowY;
	Setting<int> width;
	Setting<int> height;
	Setting<int> scale;
	Setting<int> framerate;
	Setting<bool> finishRendering;
	Setting<bool> steam;
	Setting<bool> discord;
	Setting<bool> allowDiscordLogin;
	Setting<bool> useApi;
	Setting<std::string> server;
	Setting<int> port;
	Setting<PatchMode> patchmode;
	Setting<SelectorMode> selectormode;
	Setting<ArtPanMode> artpanmode;
	Setting<std::string> stomtToken;
	Setting<std::string> memento;
	Setting<bool> cameraScrollingWasd;
	Setting<bool> cameraScrollingArrows;
	Setting<bool> cameraScrollingEdge;
	Setting<bool> allowScrollingWhileActing;
	Setting<bool> allowScrollingWhilePanning;
	Setting<bool> allowRedundantStop;
	Setting<bool> allowRedundantCultivate;
	Setting<bool> allowRedundantUpgrade;
	Setting<bool> allowPowerless;
	Setting<std::string> slackname;
	Setting<std::string> slackurl;
	Setting<std::string> discordurl;
	Setting<int> audioBufferSize;
	Setting<float> masterVolume;
	Setting<float> gameplayVolume;
	Setting<float> musicVolume;
	Setting<bool> skipAnimations;
	Setting<bool> skipLighting;
	Setting<bool> skipPanning;
	Setting<bool> silentPlanningBell;
	Setting<bool> hideIdleAnimations;
	Setting<bool> hideLayouts;
	Setting<bool> showViewport;
	Setting<std::string> language;
	Setting<std::string> palette;
	Setting<std::string> fontFilename;
	Setting<int> fontSize;
	Setting<int> fontSizeTextInput;
	Setting<int> fontSizeMenuHeader;
	Setting<int> fontSizeMenuButton;
	Setting<int> fontSizePlayButton;
	Setting<int> fontSizeReadyButton;
	Setting<int> fontSizeWallet;
	Setting<int> fontSizeTutorial;
	Setting<int> fontSizeHeadline;

private:
	Setting<std::string> settings;

	static PatchMode detectPatchMode();

	void setFallback(const std::string& filename);

	bool parse(const Json::Value& json);

	void store(Json::Value& json);

public:
	bool override(const char* arg);

	Settings& defaults();

	void load();
	void save();

	Json::Value flattenIntoJson();

	bool askedForHelp() const { return _askedForHelp; }
	void help() const;

	int getScaleBasedFontSize() const;
	int getFontSize() const;
	int getFontSizeTextInput() const;
	int getFontSizeMenuHeader() const;
	int getFontSizeMenuButton() const;
	int getFontSizePlayButton() const;
	int getFontSizeReadyButton() const;
	int getFontSizeWallet() const;
	int getFontSizeTutorial() const;
	int getFontSizeHeadline() const;

private:
	static std::string _configroot;

public:
	static void setRoot(const std::string& root);

	static std::string getPathFromFilename(const std::string& filename)
	{
		return _configroot + filename;
	}

	static void determineCompatibleRoot(const std::string& filename,
		int argc, const char* const argv[]);
};
