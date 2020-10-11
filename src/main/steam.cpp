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
#include "steam.hpp"
#include "source.hpp"

#if STEAM_ENABLED
/* ############################# STEAM_ENABLED ############################# */

#include <mutex>

#include "libs/SDL2/SDL_image.h"

#include "client.hpp"
#include "system.hpp"
#include "locator.hpp"
#include "role.hpp"


static std::mutex _singleton_mutex;
constexpr uint32_t APPLICATION_ID = 1286730;

extern "C" void __cdecl steam_log_callback(int severity, const char* text);

Steam::Steam(Client& client) :
	_client(client)
{
	// If we're creating this Steam, of which there can only be one.
	// We lock a mutex just to prove this to ourselves.
	assert(_singleton_mutex.try_lock());

	_client.registerHandler(this);
}

Steam::~Steam()
{
	if (_serverSessionTicket.handle() != k_HAuthTicketInvalid)
	{
		auto oldhandle = _serverSessionTicket.handle();
		LOGD << "Cancelling ticket " << (int) oldhandle << ".";
		SteamUser()->CancelAuthTicket(oldhandle);
	}

	SteamAPI_Shutdown();

	_client.deregisterHandler(this);

	_singleton_mutex.unlock();
}

bool Steam::shouldRestart()
{
	return SteamAPI_RestartAppIfNecessary(APPLICATION_ID);
}

std::unique_ptr<Steam> Steam::create(Client& client)
{
	if (SteamAPI_Init())
	{
		LOGI << "Initialized Steam integration.";
	}
	else
	{
		LOGE << "Steam is not running, or failed to detect steam.";
		client.steamMissing();
		return nullptr;
	}

	SteamUtils()->SetWarningMessageHook(&steam_log_callback);

	std::unique_ptr<Steam> ptr;
	ptr.reset(new Steam(client));

	ptr->ready();

	return ptr;
}

void Steam::ready()
{
	_serverSessionTicket.retrieve();
	if (_serverSessionTicket.handle() == k_HAuthTicketInvalid)
	{
		_client.steamDisconnected();
	}
	else
	{
		_client.steamConnecting();
	}

	clearLobbyInfo();
	updatePresence();
}

void Steam::update()
{
	SteamAPI_RunCallbacks();
}

void Steam::clearLobbyInfo()
{
	_lobbyId = "";
	_lobbyType = "";
	_lobbyMemberUsernames.clear();
	_maxPlayersAndBots = 0;
	_numPlayersAndBots = 0;
	_numBots = 0;
	_numPlayers = 0;
	_isPlaying = false;
	_isSpectating = false;

	_groupId = "";
	_groupSize = "";

	_connectCommand = "";

	if (_isOnline)
	{
		_legacyStatus = "Looking to Play";
		_localizedStatus = "#looking_to_play";
	}
	else
	{
		_legacyStatus = "In Menu";
		_localizedStatus = "#in_menu";
	}
}

void Steam::updateLobbyInfo()
{
	if (_lobbyId.empty())
	{
		// Wait for a subsequent ::inLobby() call to update the lobby info.
		return;
	}

	_groupId = _lobbyId;
	_groupSize = std::to_string(_lobbyMemberUsernames.size());

	if (_lobbyType == "tutorial")
	{
		_legacyStatus = "Playing Tutorial";
		_localizedStatus = "#playing_tutorial";
	}
	else if (_lobbyType == "challenge")
	{
		if (!_challengeKey.empty() && !_challengeDisplayName.empty())
		{
			_legacyStatus = "Playing " + _challengeDisplayName + " Challenge";
			_localizedStatus = "#playing_challenge_keyed";
		}
		else
		{
			_legacyStatus = "Playing Challenge";
			_localizedStatus = "#playing_challenge";
		}
	}
	else if (_lobbyType == "replay")
	{
		_legacyStatus = "Watching Replay";
		_localizedStatus = "#watching_replay";
	}
	else
	{
		std::string typedesc;
		if (_maxPlayersAndBots == 2 && _numBots == 0)
		{
			typedesc = "One vs One";
			_lobbyTypeKey = "one_vs_one";
		}
		else if (_maxPlayersAndBots > 2 && _numBots < _maxPlayersAndBots - 1)
		{
			typedesc = "Free For All";
			_lobbyTypeKey = "free_for_all";
		}
		else if (_numPlayers >= 1)
		{
			typedesc = "Versus AI";
			_lobbyTypeKey = "versus_ai";
		}
		else
		{
			typedesc = "AI vs AI";
			_lobbyTypeKey = "ai_vs_ai";
		}

		if (_isPlaying)
		{
			_legacyStatus = "Playing " + typedesc;
			_localizedStatus = "#playing";
		}
		else if (_isSpectating)
		{
			_legacyStatus = "Spectating " + typedesc;
			_localizedStatus = "#spectating";
		}
		else
		{
			_lobbyCurSize = std::to_string(_numPlayersAndBots);
			_lobbyMaxSize = std::to_string(_maxPlayersAndBots);
			_legacyStatus = "In " + typedesc + " Lobby"
				" (" + _lobbyCurSize + "/" + _lobbyMaxSize + ")";
			_localizedStatus = "#in_lobby";
		}
	}
}

void Steam::updatePresence()
{
	SteamFriends()->ClearRichPresence();
	if (!_groupId.empty())
	{
		SteamFriends()->SetRichPresence("steam_player_group", _groupId.c_str());
		SteamFriends()->SetRichPresence("steam_player_group_size",
			_groupSize.c_str());
	}
	if (!_legacyStatus.empty())
	{
		SteamFriends()->SetRichPresence("status", _legacyStatus.c_str());
	}
	if (!_localizedStatus.empty())
	{
		SteamFriends()->SetRichPresence("steam_display",
			_localizedStatus.c_str());
	}
	if (!_challengeKey.empty())
	{
		SteamFriends()->SetRichPresence("challenge_key",
			_challengeKey.c_str());
	}
	if (!_lobbyTypeKey.empty())
	{
		SteamFriends()->SetRichPresence("lobby_type_key",
			_lobbyTypeKey.c_str());
	}
	if (!_lobbyCurSize.empty())
	{
		SteamFriends()->SetRichPresence("lobby_cur_size",
			_lobbyCurSize.c_str());
	}
	if (!_lobbyMaxSize.empty())
	{
		SteamFriends()->SetRichPresence("lobby_max_size",
			_lobbyMaxSize.c_str());
	}
	if (!_connectCommand.empty())
	{
		SteamFriends()->SetRichPresence("connect",
			_connectCommand.c_str());
	}
}

void Steam::SessionTicket::retrieve()
{
	_handle = SteamUser()->GetAuthSessionTicket((void*) _buffer,
		array_size(_buffer), &_length);
	if (_handle == k_HAuthTicketInvalid)
	{
		LOGE << "Failed to generate ticket.";
	}
	else
	{
		LOGD << "Validating ticket " << (int) _handle << "...";
	}
}

void Steam::handleAuthSessionTicketResponse(
	GetAuthSessionTicketResponse_t* response)
{
	auto handle = response->m_hAuthTicket;
	if (handle == _serverSessionTicket.handle())
	{
		if (response->m_eResult == k_EResultOK)
		{
			LOGD << "Successfully validated ticket " << (int) handle << ".";
			std::stringstream strm;
			strm << std::hex << std::setfill('0');
			for (uint8_t x : _serverSessionTicket.data())
			{
				strm << std::setw(2) << (int) x;
			}
			std::string ticket = strm.str();

			CSteamID steamid = SteamUser()->GetSteamID();

			std::string avatarpicturename = getAvatarPictureName(steamid);

			_client.steamConnected(SteamFriends()->GetPersonaName(),
				avatarpicturename, steamid.ConvertToUint64(), ticket);

			handleUrlLaunchParameters(nullptr);
		}
		else
		{
			LOGE << "EResult = " << (int) response->m_eResult;

			_client.steamDisconnected();
		}
	}
	else
	{
		if (response->m_eResult != k_EResultOK)
		{
			LOGE << "EResult = " << (int) response->m_eResult;
		}
		LOGD << "Discarding ticket " << (int) handle << ".";
		SteamUser()->CancelAuthTicket(handle);
	}
}

std::string Steam::getAvatarPictureName(CSteamID steamid)
{
	int imagehandle = SteamFriends()->GetSmallFriendAvatar(steamid);
	std::string avatarpicturename = "steam/avatars/"
		+ std::to_string(steamid.ConvertToUint64()) + "/small";
	std::string filename = Locator::pictureFilename(avatarpicturename);

	if (std::find(_cachedImageHandles.begin(),
			_cachedImageHandles.end(), imagehandle)
		== _cachedImageHandles.end())
	{
		LOGD << "Getting " << avatarpicturename << " from Steam...";
		uint32_t width = 0;
		uint32_t height = 0;
		if (SteamUtils()->GetImageSize(imagehandle, &width, &height))
		{
			// According to the Steam SDK documentation, GetImageRGBA will
			// return the data in "RGBA format", which I assume means that the
			// first 8 bits represent the red value of the top left pixel.
			// Therefore I would say that the masks passed to SDL should
			// always be the big-endian masks. However that does not seem to
			// work (on my machine), so maybe I just don't understand SDL.
			uint32_t rmask, gmask, bmask, amask;
			#if SDL_BYTEORDER == SDL_BIG_ENDIAN
				rmask = 0xff000000;
				gmask = 0x00ff0000;
				bmask = 0x0000ff00;
				amask = 0x000000ff;
			#else
				rmask = 0x000000ff;
				gmask = 0x0000ff00;
				bmask = 0x00ff0000;
				amask = 0xff000000;
			#endif

			// Write the image (probably obtained from a PNG inside Steam)
			// onto an SDL surface, then write it to a PNG, so that later
			// we can load it back onto a surface. Oh well.
			SDL_Surface* surface = SDL_CreateRGBSurface(0, width, height, 32,
				rmask, gmask, bmask, amask);
			if (surface == nullptr)
			{
				LOGE << "Failed to create surface: " << SDL_GetError();
			}
			else
			{
				SteamUtils()->GetImageRGBA(imagehandle,
					(uint8_t*) surface->pixels,
					surface->h * surface->pitch);
				System::touchFile(filename);
				IMG_SavePNG(surface, filename.c_str());
				SDL_FreeSurface(surface);
				LOGD << "Saved " << filename << ".";
			}
			_cachedImageHandles.push_back(imagehandle);
		}
		else
		{
			LOGW << "Failed to load avatar.";
		}
	}

	// Also pass the correct name in case of an error, because it might have
	// been written to PNG before.
	return avatarpicturename;
}

void Steam::handleUrlLaunchParameters(NewUrlLaunchParameters_t* /*nullable*/)
{
	std::string line;
	{
		std::vector<char> buf;
		buf.resize(1024);
		int len = SteamApps()->GetLaunchCommandLine(buf.data(), buf.size());
		if (len >= 0)
		{
			line = std::string(buf.data(), len);
		}
		else
		{
			LOGE << "Launched from Steam but failed to get command line";
			return;
		}
	}
	LOGI << "Launched from Steam with: " << line;

	std::string secret;
	{
		constexpr const char* arg = "hot-join-secret=";
		size_t start = line.find(arg);
		if (start != std::string::npos)
		{
			start += strlen(arg);
			size_t end = line.find_first_not_of(
				"0123456789"
				"abcdefghijklmnopqrstuvwxyz"
				"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
				"-"
				"", start);
			if (end != std::string::npos)
			{
				secret = line.substr(start, end - start);
			}
			else
			{
				secret = line.substr(start);
			}
		}
	}
	if (!secret.empty())
	{
		_client.steamJoinGame(secret);
	}
}

void Steam::debugHandler() const
{
	LOGD << ((void*) this);
}

void Steam::disconnected()
{
	if (_serverSessionTicket.handle() != k_HAuthTicketInvalid)
	{
		auto oldhandle = _serverSessionTicket.handle();
		LOGD << "Cancelling ticket " << (int) oldhandle << ".";
		SteamUser()->CancelAuthTicket(oldhandle);

		_serverSessionTicket.retrieve();
		if (_serverSessionTicket.handle() == k_HAuthTicketInvalid)
		{
			_client.steamDisconnected();
		}
		else
		{
			_client.steamConnecting();
		}
	}
}

void Steam::inServer()
{
	_isOnline = true;
	clearLobbyInfo();
	updatePresence();
}

void Steam::outServer()
{
	_isOnline = false;
	clearLobbyInfo();
	updatePresence();
}

void Steam::loggedOut()
{
	outServer();
}

void Steam::inLobby(const std::string& lobbyid)
{
	_lobbyId = lobbyid;
	// Wait for a subsequent ::listOwnLobby() call to update the presence.
}

void Steam::outLobby()
{
	clearLobbyInfo();
	updatePresence();
}

void Steam::listOwnLobby(const std::string&, const Json::Value& metadata)
{
	_lobbyType = "";
	if (metadata["lobby_type"].isString())
	{
		_lobbyType = metadata["lobby_type"].asString();
	}

	_maxPlayersAndBots = 0;
	if (metadata["max_players"].isInt())
	{
		_maxPlayersAndBots = metadata["max_players"].asInt();
	}
	DEBUG_ASSERT(_maxPlayersAndBots >= 0);

	_numPlayersAndBots = 0;
	if (metadata["num_players"].isInt())
	{
		_numPlayersAndBots = metadata["num_players"].asInt();
	}
	DEBUG_ASSERT(_numPlayersAndBots >= 0
		&& _numPlayersAndBots <= _maxPlayersAndBots);
	_numBots = 0;
	if (metadata["num_bot_players"].isInt())
	{
		_numBots = metadata["num_bot_players"].asInt();
	}
	DEBUG_ASSERT(_numBots >= 0 && _numBots <= _numPlayersAndBots);
	_numPlayers = _numPlayersAndBots - _numBots;

	updateLobbyInfo();
	updatePresence();
}

void Steam::joinsOwnLobby(const std::string& name, bool isSelf)
{
	auto found = std::find(_lobbyMemberUsernames.begin(),
		_lobbyMemberUsernames.end(),
		name);
	if (found != _lobbyMemberUsernames.end())
	{
		return;
	}

	_lobbyMemberUsernames.push_back(name);

	if (isSelf)
	{
		// Wait for a subsequent ::listOwnLobby() call to update the presence.
		return;
	}

	updateLobbyInfo();
	updatePresence();
}

void Steam::leavesOwnLobby(const std::string& name)
{
	auto found = std::find(_lobbyMemberUsernames.begin(),
		_lobbyMemberUsernames.end(),
		name);
	if (found == _lobbyMemberUsernames.end())
	{
		return;
	}

	_lobbyMemberUsernames.erase(found);

	updateLobbyInfo();
	updatePresence();
}

void Steam::listChallenge(const std::string&, const Json::Value& metadata)
{
	if (metadata["steam-short-key"].isString())
	{
		_challengeKey = metadata["steam-short-key"].asString();
	}
	else
	{
		LOGW << "Missing 'steam-short-key'";
	}

	if (metadata["display-name"].isString())
	{
		// The display name is used for the (English-only) legacy status,
		// so we leave it untranslated.
		_challengeDisplayName = metadata["display-name"].asString();
	}
	else
	{
		LOGW << "Missing 'display-name'";
	}
}

void Steam::receiveSecrets(const Json::Value& metadata)
{
	if (metadata["join-secret"].isString())
	{
		std::string secret = metadata["join-secret"].asString();
		// The secret should be 58 characters, but this number might change.
		if (secret.size() < 10 || secret.size() > 100)
		{
			LOGW << "Ignoring invalid join secret: " << secret;
			return;
		}
		size_t ill = secret.find_first_not_of(
			"0123456789"
			"abcdefghijklmnopqrstuvwxyz"
			"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
			"-"
			"");
		if (ill != std::string::npos)
		{
			LOGW << "Ignoring invalid join secret: " << secret;
			return;
		}

		std::stringstream strm;
		strm << "hot-join-secret=" << secret;
		_connectCommand = strm.str();
	}
	// For Steam there is no separate spectate secret, because Steam's Join Game
	// feature is only available for friends and looks the same when the player
	// is already playing a game.

	updatePresence();
}

void Steam::startGame(const Role& role)
{
	if (role == Role::PLAYER)
	{
		_isPlaying = true;
	}
	else
	{
		_isSpectating = true;
	}

	updateLobbyInfo();
	updatePresence();
}

void Steam::startTutorial()
{
	_isPlaying = true;
	updateLobbyInfo();
	updatePresence();
}

extern "C" void __cdecl steam_log_callback(int severity, const char* text)
{
	switch (severity)
	{
		case 0: LOGI << text; break;
		case 1: LOGW << text; break;
		default: LOGE << text; break;
	}
}

/* ############################# STEAM_ENABLED ############################# */
#endif
