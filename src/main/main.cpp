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
#include "main.hpp"
#include "source.hpp"

#include "coredump.hpp"
#include "version.hpp"
#include "loginstaller.hpp"
#include "game.hpp"
#include "localgame.hpp"
#include "localreplay.hpp"
#include "localdiorama.hpp"
#include "onlinegame.hpp"
#include "onlinetutorial.hpp"
#include "onlinereplay.hpp"
#include "onlinediorama.hpp"
#include "onlinelocalgame.hpp"
#include "hostedgame.hpp"
#include "aichallenge.hpp"
#include "settings.hpp"
#include "exitcode.hpp"
#include "system.hpp"
#include "patch.hpp"
#include "bot.hpp"
#include "difficulty.hpp"
#include "ai.hpp"
#include "aicommander.hpp"
#include "ailibrary.hpp"
#include "map.hpp"
#include "account.hpp"
#include "recording.hpp"
#include "steam.hpp"
#include "language.hpp"
#include "download.hpp"
#include "locator.hpp"
#include "clip.hpp"
#include "skin.hpp"
#include "palette.hpp"
#include "spritepattern.hpp"
#include "curl.hpp"
#include "openurl.hpp"
#include "message.hpp"


static void help(const Settings& settings)
{
	std::cout << "Usage:" << std::endl
		<< "  epicinium [OPTIONS]" << std::endl
		<< "  epicinium [OPTIONS] MAPNAME [NUMPLAYERS BOTS]"
			" [rulesets/RULESETNAME.json] [record]" << std::endl
		<< "  epicinium [OPTIONS] recordings/RECORDINGNAME.rec" << std::endl
		<< std::endl
		<< "Bot:" << std::endl
		<< "  AINAME DIFFICULTY"
		<< std::endl
		<< "Help:" << std::endl
		<< "  -h, -?, --help       Show this help output." << std::endl
		<< std::endl;

	settings.help();
}

int main(int argc, char* argv[])
{
	CoreDump::enable();

	std::string settingsfilename = "settings.json";
	Settings::determineCompatibleRoot(settingsfilename, argc, argv);
	std::string settingspath = Settings::getPathFromFilename(settingsfilename);
	Settings settings(settingspath, argc, argv);

	std::string mapname = "";
	std::string recname = "";
	std::string rulname = "";
	std::vector<std::string> ainames;
	std::vector<Difficulty> aidifficulties;
	bool record = false;
	int nplayers = 2;
	int challengeid = -1;
	int launcherversion = 0;
	std::vector<std::string> invalid_arguments;

	for (int i = 1; i < argc; i++)
	{
		const char* arg = argv[i];
		size_t arglen = strlen(arg);
		if (strncmp(arg, "-", 1) == 0)
		{
			// Setting argument, will be handled by Settings.
		}
		else if (ainames.size() > aidifficulties.size())
		{
			aidifficulties.emplace_back(parseDifficulty(arg));
		}
		else if (arglen > 9
			&& strncmp(arg, "launcher=", 9) == 0
			&& strspn(arg + 9, "0123456789") == arglen - 9)
		{
			launcherversion = atoi(arg + 9);
		}
		else if (!mapname.empty() && strspn(arg, "0123456789") == arglen)
		{
			nplayers = atoi(arg);
		}
		else if (arglen > 11 + 4
			&& strncmp(arg, "recordings/", 11) == 0
			&& strncmp(arg + arglen - 4, ".rec", 4) == 0)
		{
			recname = std::string(arg + 11, arglen - 11 - 4);
		}
		else if (arglen > 9 + 5
			&& strncmp(arg, "rulesets/", 9) == 0
			&& strncmp(arg + arglen - 5, ".json", 5) == 0)
		{
			rulname = std::string(arg + 9, arglen - 9 - 5);
		}
		else if (strcmp(arg, "record") == 0)
		{
			record = true;
		}
		else if (Map::exists(arg))
		{
			mapname = arg;
		}
		else if (AI::exists(arg))
		{
			ainames.emplace_back(arg);
		}
		else if (AILibrary::exists(arg))
		{
			ainames.emplace_back(arg);
		}
		else if (arglen > 10
			&& strncmp(arg, "challenge=", 10) == 0
			&& strspn(arg + 10, "0123456789") == arglen - 10)
		{
			challengeid = atoi(arg + 10);
		}
		else if (arglen > 8 + 3
			&& strncmp(arg, "discord-", 8) == 0
			&& strncmp(arg + arglen - 3, "://", 3) == 0)
		{
			// This is a discord-rpc quirk: it launches through a desktop
			// shortcut called (e.g.) discord-458745295492:// and appends
			// that string as the first argument. For some reason.
		}
		else
		{
			invalid_arguments.push_back(arg);
		}
	}

	std::string logname = "main";
	if (settings.logname.defined())
	{
		logname = settings.logname.value();
	}
	else settings.logname.override(logname);

	std::cout << "[ Epicinium ]";
	std::cout << " (" << logname << " v" << Version::current() << ")";
	std::cout << std::endl << std::endl;

	if (settings.dataRoot.defined())
	{
		LogInstaller::setRoot(settings.dataRoot.value());
	}

	LogInstaller(settings).install();

	if (settings.patchmode.value() == PatchMode::SERVER
		|| launcherversion == 0)
	{
		if (System::hasStorageIssuesForSelfPatch())
		{
			LOGE << "Patching disabled due to storage issues.";
			settings.patchmode.override(
				PatchMode::SERVER_BUT_DISABLED_DUE_TO_STORAGE_ISSUES);
		}
		else
		{
			settings.cacheRoot.override("");
		}
	}

	if (settings.cacheRoot.defined())
	{
		Download::setRoot(settings.cacheRoot.value());
		Locator::setCacheRoot(settings.cacheRoot.value());
	}

	if (settings.resourceRoot.defined())
	{
		Curl::setRoot(settings.resourceRoot.value());
		Clip::setRoot(settings.resourceRoot.value());
		Language::setRoot(settings.resourceRoot.value());
		Map::setResourceRoot(settings.resourceRoot.value());
		Locator::setResourceRoot(settings.resourceRoot.value());
		Skin::setResourceRoot(settings.resourceRoot.value());
		Palette::setResourceRoot(settings.resourceRoot.value());
		SpritePattern::setResourceRoot(settings.resourceRoot.value());
	}

	if (settings.dataRoot.defined())
	{
		Map::setAuthoredRoot(settings.dataRoot.value());
		Locator::setAuthoredRoot(settings.dataRoot.value());
		Palette::setAuthoredRoot(settings.dataRoot.value());
		Account::setRoot(settings.dataRoot.value());
		Recording::setRoot(settings.dataRoot.value());
	}

	LOGI << "Start v" << Version::current();

	if (settings.askedForHelp())
	{
		help(settings);
		std::cout << std::endl << "[ Done ]" << std::endl;
		return 0;
	}
	else if (ainames.size() != aidifficulties.size())
	{
		std::cerr << "Each AI must be followed by a difficulty" << std::endl;
		std::cerr << std::endl;

		help(settings);
		std::cout << std::endl << "[ Done ]" << std::endl;
		return 0;
	}
	else if (!invalid_arguments.empty())
	{
		for (const std::string& arg : invalid_arguments)
		{
			std::cerr << "Invalid argument: " << arg << std::endl;
		}
		std::cout << std::endl;

		help(settings);

		if (mapname.empty() && recname.empty())
		{
			// Continue anyway because this might be a quirk, e.g. see above.
		}
		else
		{
			std::cout << std::endl << "[ Done ]" << std::endl;
			return 0;
		}
	}

	// Enable internationalization.
	Language::use(settings);

	// Disable Steam and Discord integration when skipping the main menu.
	if (!mapname.empty() || !recname.empty())
	{
		settings.steam.override(false);
		settings.discord.override(false);
	}

	if (launcherversion >= 0)
	{
		LOGD << "Launcher version: " << launcherversion;
	}
	else
	{
#if STEAM_ENABLED
		LOGD << "Launcher replaced by Steam.";
#else
		LOGD << "Launcherversion invalid";
#endif
	}

#if SELF_PATCH_ENABLED
	if (System::isFile(Patch::mementofilename()))
	{
		std::ifstream file = System::ifstream(Patch::mementofilename());
		std::string line;
		if (file && std::getline(file, line))
		{
			LOGI << "Memento '" << line << "'";
			settings.memento.override(line);
		}
	}
#endif

	Patch::purge();

#if STEAM_ENABLED
	if (settings.steam.value(true))
	{
		if (Steam::shouldRestart())
		{
			LOGI << "Steam requested an immediate restart through steam...";
			std::cout << "[ Done ]" << std::endl;
			return 0;
		}
	}
#endif

	ExitCode exitcode;

	try
	{
		Main engine(settings);

		if (mapname == Map::DIORAMA_MAPNAME)
		{
			engine.startGame(imploding_ptr<Game>(new LocalDiorama(
				engine, settings,
				mapname,
				/*silentQuit=*/true)));
		}
		else if (!mapname.empty())
		{
			std::vector<Bot> bots;
			std::string slotname = "%""A";
			assert(ainames.size() == aidifficulties.size());
			for (size_t i = 0; i < ainames.size(); i++)
			{
				slotname[1] = 'A' + i;
				bots.emplace_back(slotname, ainames[i], aidifficulties[i]);
			}

			engine.startGame(imploding_ptr<Game>(new LocalGame(
				engine, settings,
				getPlayers(nplayers), bots, mapname, rulname,
				nplayers - ainames.size(),
				/*silentQuit=*/true, record)));
		}
		else if (!recname.empty())
		{
			engine.startGame(imploding_ptr<Game>(new LocalReplay(
				engine, settings,
				recname,
				/*silentQuit=*/true)));
		}
		else if (challengeid >= 0
			&& ((size_t) challengeid) < Challenge::ID_SIZE)
		{
			engine.startChallenge((Challenge::Id) challengeid, "");
		}

		exitcode = engine.run();
	}
	catch (const std::exception& e)
	{
		LOGE << "Error while running game: " << e.what();
		Patch::purge();
		throw;
	}
	catch (...)
	{
		LOGW << "Unknown error while running game";
		Patch::purge();
		throw;
	}

	std::cout << std::endl << std::endl;
	switch (exitcode)
	{
		case ExitCode::APPLY_PATCH_AND_RESTART:
		{
			LOGI << "Applying patch and restarting...";

			try
			{
#if SELF_PATCH_ENABLED
				if (Patch::install())
				{
					LOGI << "Patch installed. Restarting...";
				}
				else
				{
					LOGE << "Failed to install patch.";
					Patch::purge();
					throw std::runtime_error("Failed to install patch.");
				}
#else
				throw std::runtime_error("self patch not enabled");
#endif
			}
			catch (const std::exception& e)
			{
				LOGE << "Error while patching: " << e.what();
				Patch::purge();
				throw;
			}
			catch (...)
			{
				LOGW << "Unknown error while patching.";
				Patch::purge();
				throw;
			}

			if (launcherversion >= 1)
			{
				std::cout << "[ Restarting... ]" << std::endl;
				return 2;
			}
			else
			{
				Patch::restart();
				std::cout << "[ Restarting... ]" << std::endl;
				return 0;
			}
		}
		break;

		case ExitCode::APPLY_SETTINGS_AND_RESTART:
		{
			LOGI << "Applying settings and restarting...";

			if (launcherversion >= 1)
			{
				std::cout << "[ Restarting... ]" << std::endl;
				return 2;
			}
			else
			{
				Patch::restart();
				std::cout << "[ Restarting... ]" << std::endl;
				return 0;
			}
		}
		break;

		case ExitCode::DONE:
		{
			LOGI << "Done";
			std::cout << "[ Done ]" << std::endl;
			return 0;
		}
		break;

		case ExitCode::TERMINATE:
		{
			LOGI << "Terminated";
			std::cout << "[ Terminated ]" << std::endl;
			return 0;
		}
		break;
	}

	// Invalid exit code.
	LOGE << "Invalid exit code: " << ((int) exitcode);
	Patch::purge();
	std::cout << "[ Error ]" << std::endl;
	return 1;
}

Main::Main(Settings& settings) :
	Engine(settings),
	_dictator(),
	_messageinjector(_dictator),
	_client(*this, *this, _settings),
	_epiCDN(*this, Client::getWebsiteOrigin(), _client.getCurlAccess()),
	_discordRPC(*this, _client),
	_stomt(*this, _settings, _client.getCurlAccess()),
	_menu(*this, *this)
{
	_menu.init();
	_menu.open();

#if STEAM_ENABLED
	if (settings.steam.value(true))
	{
		_steam = Steam::create(_client);
	}
#endif

	_client.discordAvailable();
	_client.connect();
}

Main::~Main()
{
	// We have to make sure _game deregisters itself before _menu and _client,
	// because we do not want the backtrace
	//        (segfault)
	//    _menu.show() in Main::stopGame()
	//    _owner.stopGame() in OnlineGame::disconnected()
	//    handler->disconnected() in ClientHandlerHandler::disconnected()
	//    _owner.disconnected() in Client::disconnected()
	//    disconnect() in ~Client()
	//        in ~Main()
	// which occurs after _menu has been destroyed.
	if (auto game = _game.lock()) game->confirmQuit();
}

void Main::doFirst()
{
	Engine::doFirst();

	_menu.beforeFirstUpdateOfEachSecond();
}

void Main::startUpdates()
{
#if DICTATOR_ENABLED
	_dictator.update();
	_messageinjector.update();
	_input.obey(_dictator);
	_client.obey(_dictator);
#endif

	if (_screenshot)
	{
		// Tell all ClientHandler implementors that a screenshot was taken.
		// The screenshot will be cleared by Engine::startUpdates().
		// Afterwards, we can create new a new screenshot.
		ClientHandlerHandler::screenshotTaken(_screenshot);
	}

	Engine::startUpdates();

#if STEAM_ENABLED
	if (_steam) _steam->update();
#endif

	_epiCDN.update();
	_discordRPC.update();
	_client.update();
	_stomt.update();
}

void Main::endUpdates()
{
	Engine::endUpdates();

	_menu.update();
}

void Main::quit(ExitCode exitcode)
{
	Engine::quit(exitcode);
}

void Main::getPicture(const std::string& name)
{
	_epiCDN.getPicture(name);
}

std::string Main::activePaletteName() const
{
	return _skineditor.activePaletteName();
}

void Main::openPaletteEditor(const std::string& palettename)
{
	return _skineditor.openPaletteEditor(palettename);
}

void Main::openUrl(const std::string& url)
{
#if STEAM_ENABLED
	if (_steam)
	{
		_steam->openUrl(url);
	}
	else
	{
		System::openURL(url);
	}
#else
	System::openURL(url);
#endif
}

void Main::sendStomt(bool positive, const std::string& text)
{
	_stomt.sendStomt(positive, text);
}

std::weak_ptr<Game> Main::startGame(imploding_ptr<Game> game)
{
	_menu.hide();
	return Engine::startGame(std::move(game));
}

std::weak_ptr<Game> Main::startChallenge(const Challenge& challenge,
	const std::string& name)
{
	if (name.empty())
	{
		return startGame(new LocalGame(*this, _settings,
			std::make_shared<AIChallenge>(challenge),
			/*silentQuit=*/false));
	}
	else
	{
		std::string mapname = name;
		std::string rulesetname = Library::nameCurrentBible();
		if (Library::existsBible(name))
		{
			rulesetname = name;
		}
		return startGame(new OnlineLocalGame(*this, _settings, _client,
			std::make_shared<AIChallenge>(challenge),
			mapname, rulesetname));
	}
}

std::weak_ptr<Game> Main::startGame(
		const Player& player, const std::string& rulesetname,
		uint32_t planningtime)
{
	return startGame(new OnlineGame(*this, _settings, _client,
		player, rulesetname, planningtime));
}

std::weak_ptr<Game> Main::startTutorial(
		const Player& player, const std::string& rulesetname,
		uint32_t planningtime)
{
	return startGame(new OnlineTutorial(*this, _settings, _client,
		player, rulesetname, planningtime));
}

std::weak_ptr<Game> Main::startReplay(
		const Role& role, const std::string& rulesetname,
		uint32_t planningtime)
{
	return startGame(new OnlineReplay(*this, _settings, _client,
		role, rulesetname, planningtime));
}

std::weak_ptr<Game> Main::startDiorama()
{
	return startGame(new OnlineDiorama(*this, _settings, _client,
		Map::DIORAMA_MAPNAME));
}

std::weak_ptr<HostedGame> Main::startHostedGame(
		const std::vector<Player>& playercolors,
		const std::vector<VisionType>& visiontypes,
		const std::vector<std::string>& usernames,
		const std::vector<Bot>& bots,
		bool hasObservers,
		const std::string& mapname, const std::string& rulesetname)
{
	_hosted = new HostedGame(_client,
		playercolors, visiontypes, usernames, bots, hasObservers,
		mapname, rulesetname);
	return _hosted.remember();
}

void Main::stopGame()
{
	Engine::stopGame();
	_hosted = nullptr;
	_menu.show();
}

void Main::reportAwardedStars(int amount)
{
	Json::Value metadata = Json::objectValue;
	metadata["game_over"] = true;
	metadata["stars"] = amount;
	_client.send(Message::host_sync(metadata));
}

void Main::takeScreenshot(std::weak_ptr<Screenshot> screenshot)
{
	// The screenshot will be rendered by the Engine during the next frame.
	_nextScreenshot = screenshot.lock();
}

bool Main::isTakingScreenshot()
{
	return (_screenshot != nullptr);
}
