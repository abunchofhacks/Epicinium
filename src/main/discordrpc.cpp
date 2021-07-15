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
#include "discordrpc.hpp"
#include "source.hpp"

#include <mutex>

#include "libs/SDL2/SDL_timer.h"

#include "libs/discord/discord_rpc.h"
#include "libs/discord/discord_register.h"

#include "clock.hpp"
#include "client.hpp"
#include "graphics.hpp"
#include "launchercommand.hpp"
#include "role.hpp"
#include "validation.hpp"


constexpr const char* APPLICATION_ID = "471648649751822348";

static std::mutex _mutex;
static DiscordRPC* _installed = nullptr;
static bool _active = false;
static int _dpPlayersMax = 0;
static int _dpPlayers = 0;
static int _dpBots = 0;
static bool _dpPublic = false;
static bool _dpIngame = false;
static std::string _dpMap;
static std::string _dpUserMapDescription;
static std::string _dpChallenge;
static std::vector<std::string> _dpListedChallengeKey; // (married)
static std::vector<std::string> _dpListedChallengeDisplayName; // (married)
static std::vector<std::string> _dpListedChallengeImageKey; // (married)
static std::string _dpReplay;
static std::string _dpPartyId;
static std::string _dpJoinSecret;
static std::string _dpSpectateSecret;
static DiscordEventHandlers _discordHandlers;
static DiscordRichPresence _discordPresence;
static std::string _discordRequestUserId;
static std::string _discordUserId;
static std::string _discordUsername;
static std::string _discordDiscriminator;
static std::string _discordAvatarPicture;

DiscordRPC::DiscordRPC(ClientHandler& owner, Client& client) :
	_owner(owner),
	_client(client),
	_discordCDN(_client.getCurlAccess())
{
	_client.registerHandler(this);
}

void DiscordRPC::install()
{
	LOGI << "Installing DiscordRPC";

	// If we're installing this DiscordRPC, we are actually using it, so there
	// can only be one. We lock a mutex just to prove this to ourselves.
	assert(_mutex.try_lock());
	// Install this DiscordRPC (for the handlers).
	assert(_installed == nullptr);
	_installed = this;

	// Initialize the Discord handlers, even if we don't use them.
	memset(&_discordHandlers, 0, sizeof(_discordHandlers));
	_discordHandlers.ready = [](const DiscordUser* request) {
		_installed->handleDiscordReady(request);
	};
	_discordHandlers.errored = [](int errorcode, const char* message) {
		_installed->handleDiscordError(errorcode, message);
	};
	_discordHandlers.disconnected = [](int errorcode, const char* message) {
		_installed->handleDiscordDisconnected(errorcode, message);
	};
	_discordHandlers.joinGame = [](const char* secret){
		_installed->handleDiscordJoinGame(secret);
	};
	_discordHandlers.spectateGame = [](const char* secret){
		_installed->handleDiscordSpectateGame(secret);
	};
	_discordHandlers.joinRequest = [](const DiscordUser* request) {
		_installed->handleDiscordJoinRequest(request);
	};

	// Initialize the Discord presence, even if we don't use it.
	memset(&_discordPresence, 0, sizeof(_discordPresence));
	_discordPresence.state = "In Menu";
	_discordPresence.details = "";
	_discordPresence.startTimestamp = 0;
	_discordPresence.endTimestamp = 0;
	_discordPresence.largeImageKey = "planetlogo";
	_discordPresence.largeImageText = "";
	_discordPresence.smallImageKey = "";
	_discordPresence.smallImageText = "";
	_discordPresence.partyId = "";
	_discordPresence.partySize = 0;
	_discordPresence.partyMax = 0;
	_discordPresence.joinSecret = nullptr;
	_discordPresence.spectateSecret = nullptr;
}

DiscordRPC::~DiscordRPC()
{
	_client.deregisterHandler(this);

	if (_installed == this)
	{
		// De-install this DiscordRPC.
		_mutex.unlock();
		_installed = nullptr;
	}
}

void DiscordRPC::activateDiscord()
{
	if (_active) return;

	// Install this DiscordRPC if necessary.
	if (_installed != this)
	{
		install();
	}

	LOGI << "Activating Discord";

	// Register the launcher to Discord.
#ifndef DEVELOPMENT
	Discord_Register(APPLICATION_ID, LauncherCommand::get());
#endif

	// Initialize Discord Rich Presence.
	Discord_Initialize(APPLICATION_ID, &_discordHandlers, 0, "");

	// Discord has been activated.
	_active = true;

	// Immediately update the presence.
	updatePresence();
}

void DiscordRPC::deactivateDiscord()
{
	if (!_active) return;

	LOGI << "Deactivating Discord";

	// We are deactivating Discord.
	_active = false;

	// Update the presence to indicate we are no longer playing.
	Discord_ClearPresence();

	// Quit Discord Rich Presence.
	Discord_Shutdown();
}

void DiscordRPC::updatePresence(bool dirty)
{
	if (!_active) return;

	// The Discord Rich Presence SDK documentation says, on the How To page,
	// that the SDK will handle the API's 15 second rate limit, and that
	// the developers (in casu Daan & Sander) do not have to do anything.
	// This is not true; either the docs are mistaken or there is a bug in the
	// Discord app, but when we update at a realistic pace (i.e. once every
	// time the presence changes), we are rate limited and ultimately the
	// presence breaks and is cleared. If intentional or until fixed, we will
	// have to do the rate limiting ourselves. The code below does that.
	// The 1 second delay is to ensure that when we do have the time to update,
	// we update the correct information; there might be multiple state-changing
	// events in sequence, in which case we want to update after the latter.
	// We never want to wait more than 1 second after the first state-changer.
	static constexpr uint32_t DISCORD_UPDATE_DELAY = 1000;
	static constexpr uint32_t DISCORD_UPDATE_COOLDOWN = 15000;
	static uint32_t _lastDirtyTime = 0;
	static uint32_t _lastCleanTime = uint32_t(-1);

	uint32_t now = SDL_GetTicks();

	if (dirty)
	{
		if (_lastDirtyTime <= _lastCleanTime)
		{
			_lastDirtyTime = now;
		}
		return;
	}

	if (now - _lastDirtyTime < DISCORD_UPDATE_DELAY) return;
	if (_lastCleanTime != uint32_t(-1))
	{
		if (_lastDirtyTime <= _lastCleanTime) return;
		if (now - _lastCleanTime < DISCORD_UPDATE_COOLDOWN) return;
	}

	_lastCleanTime = now;

	Discord_UpdatePresence(&_discordPresence);
}

void DiscordRPC::update()
{
	if (_active)
	{
		// Run Discord Rich Presence callbacks.
		Discord_RunCallbacks();

		// Update the Discord Presence iff necessary.
		updatePresence(/*dirty=*/false);
	}

	// Check if any avatars were downloaded from DiscordCDN.
	{
		std::vector<std::string> filenames;
		if (_discordCDN.update(filenames))
		{
			for (const std::string& filename : filenames)
			{
				_owner.requestFulfilled(filename);
			}
		}
	}
}

void DiscordRPC::inServer()
{
	if (!_installed) return;

	if (!_discordUserId.empty())
	{
		_client.tryLinkAccounts(_discordUserId);
	}

	_discordPresence.state = "Looking to Play";
	clearLobbyInfo();
	updatePresence();
}

void DiscordRPC::outServer()
{
	if (!_installed) return;

	_discordPresence.state = "In Menu";
	clearLobbyInfo();
	updatePresence();
}

void DiscordRPC::loggedOut()
{
	outServer();
}

void DiscordRPC::listOwnLobby(const std::string&, const Json::Value& metadata)
{
	if (!_installed) return;

	_dpPlayersMax = 0;
	if (metadata["max_players"].isInt())
	{
		_dpPlayersMax = metadata["max_players"].asInt();
	}
	DEBUG_ASSERT(_dpPlayersMax >= 0);

	int total = 0;
	if (metadata["num_players"].isInt())
	{
		total = metadata["num_players"].asInt();
	}
	DEBUG_ASSERT(total >= 0 && total <= _dpPlayersMax);
	_dpBots = 0;
	if (metadata["num_bot_players"].isInt())
	{
		_dpBots = metadata["num_bot_players"].asInt();
	}
	DEBUG_ASSERT(_dpBots >= 0 && _dpBots <= total);
	_dpPlayers = total - _dpBots;

	_dpPublic = false;
	if (metadata["is_public"].isBool())
	{
		_dpPublic = metadata["is_public"].asBool();
	}

	_discordPresence.partyMax = _dpPlayersMax;
	if (_dpIngame)
	{
		// Games can only be started if they are full and players cannot
		// be added afterwards, so the party should be considered full.
		_discordPresence.partySize = _dpPlayersMax;
	}
	else
	{
		_discordPresence.partySize = _dpPlayers + _dpBots;
	}
	updateLobbyInfo();
	updatePresence();
}

void DiscordRPC::pickMap(const std::string& mapname)
{
	if (!_installed) return;

	_dpMap = mapname;
	updateLobbyInfo();
	updatePresence();
}

void DiscordRPC::pickReplay(const std::string& replayname)
{
	if (!_installed) return;

	_dpReplay = replayname;
	_dpMap = "";
	updateLobbyInfo();
	updatePresence();
}

void DiscordRPC::pickChallenge(const std::string& key)
{
	if (!_installed) return;

	_dpChallenge = key;
	updateLobbyInfo();
	updatePresence();
}

void DiscordRPC::listChallenge(const std::string& key,
		const Json::Value& metadata)
{
	if (!_installed) return;

	size_t offset = _dpListedChallengeKey.size();
	auto iter = std::find(_dpListedChallengeKey.begin(),
		_dpListedChallengeKey.end(), key);
	if (iter < _dpListedChallengeKey.end())
	{
		offset = iter - _dpListedChallengeKey.begin();
	}
	else
	{
		_dpListedChallengeKey.emplace_back(key);
		_dpListedChallengeDisplayName.emplace_back();
		_dpListedChallengeImageKey.emplace_back();
	}
	if (metadata["display-name"].isString())
	{
		_dpListedChallengeDisplayName[offset] =
			metadata["display-name"].asString();
	}
	if (metadata["discord-image-key"].isString())
	{
		_dpListedChallengeImageKey[offset] =
			metadata["discord-image-key"].asString();
	}
}

void DiscordRPC::receiveSecrets(const Json::Value& metadata)
{
	if (!_installed) return;

	if (metadata["join-secret"].isString())
	{
		_dpJoinSecret = metadata["join-secret"].asString();
	}
	if (metadata["spectate-secret"].isString())
	{
		_dpSpectateSecret = metadata["spectate-secret"].asString();
	}

	if (_dpIngame)
	{
		if (!_dpSpectateSecret.empty())
		{
			_discordPresence.spectateSecret = _dpSpectateSecret.c_str();
		}
	}
	else
	{
		if (!_dpJoinSecret.empty())
		{
			_discordPresence.joinSecret = _dpJoinSecret.c_str();
		}
	}

	updatePresence();
}

void DiscordRPC::inLobby(const std::string& lobbyid)
{
	if (!_installed) return;

	if (_dpPlayersMax)
	{
		if (_dpIngame)
		{
			// Games can only be started if they are full and players cannot
			// be added afterwards, so the party should be considered full.
			_discordPresence.partySize = _dpPlayersMax;
		}
		else
		{
			_discordPresence.partySize = _dpPlayers + _dpBots;
		}
	}
	_dpChallenge = "";
	_dpReplay = "";
	_dpPublic = false;
	updateLobbyInfo();
	_discordPresence.state = "In a Lobby";
	_discordPresence.details = "";
	_dpPartyId = lobbyid;
	_discordPresence.partyId = _dpPartyId.c_str();
	// Wait for a subsequent ::listOwnLobby() call to update the presence.
}

void DiscordRPC::outLobby()
{
	inServer();
}

void DiscordRPC::startGame(const Role& role)
{
	if (!_installed) return;

	if (role == Role::PLAYER)
	{
		_discordPresence.state = "In Game";
	}
	else if (!_dpReplay.empty())
	{
		_discordPresence.state = "Watching Replay";
	}
	else
	{
		_discordPresence.state = "Spectating";
	}

	updateLobbyInfo();

	_dpIngame = true;
	if (_dpPlayersMax)
	{
		// Games can only be started if they are full and players cannot
		// be added afterwards, so the party should be considered full.
		_discordPresence.partySize = _dpPlayersMax;
	}
	_discordPresence.startTimestamp = EpochClock::seconds();
	_discordPresence.joinSecret = nullptr;
	if (_dpPublic && !_dpSpectateSecret.empty())
	{
		_discordPresence.spectateSecret = _dpSpectateSecret.c_str();
	}

	updatePresence();
}

void DiscordRPC::startTutorial()
{
	if (!_installed) return;

	updateLobbyInfo();

	_dpIngame = true;
	if (_dpPlayersMax)
	{
		// Games can only be started if they are full and players cannot
		// be added afterwards, so the party should be considered full.
		_discordPresence.partySize = _dpPlayersMax;
	}
	_discordPresence.state = "Playing Solo";
	_discordPresence.startTimestamp = EpochClock::seconds();
	_discordPresence.joinSecret = nullptr;

	updatePresence();
}

void DiscordRPC::checkDiscordReady()
{
	if (!_active) return;

	if (_discordUsername.empty()) return;

	_client.discordReady(_discordUsername.c_str(),
		_discordDiscriminator.c_str(),
		_discordAvatarPicture.c_str());
}

void DiscordRPC::handleDiscordReady(const DiscordUser* request)
{
	if (!_active) return;

	if (request)
	{
		_discordUserId = request->userId;
		_discordUsername = request->username;
		_discordDiscriminator = request->discriminator;
		_discordAvatarPicture = _discordCDN.getAvatarPicture(request->userId,
			request->avatar);

		_client.discordReady(_discordUsername.c_str(),
			_discordDiscriminator.c_str(),
			_discordAvatarPicture.c_str());

		_client.tryLinkAccounts(_discordUserId);
	}
}

void DiscordRPC::handleDiscordDisconnected(int errorcode, const char* message)
{
	if (!_active) return;

	if (message)
	{
		_client.discordDisconnected(errorcode, message);
	}
}

void DiscordRPC::handleDiscordError(int errorcode, const char* message)
{
	LOGE << "handleDiscordError"
		<< " (" << errorcode << "): "
		<< message;

	if (!_active) return;

	if (message)
	{
		_client.discordError(errorcode, message);
	}
}

void DiscordRPC::handleDiscordJoinGame(const char* secret)
{
	if (!_active) return;

	if (secret)
	{
		_client.discordJoinGame(secret);

		Graphics::get()->raiseWindow();
	}
}

void DiscordRPC::handleDiscordSpectateGame(const char* secret)
{
	if (!_active) return;

	if (secret)
	{
		_client.discordSpectateGame(secret);

		Graphics::get()->raiseWindow();
	}
}

void DiscordRPC::handleDiscordJoinRequest(const DiscordUser* request)
{
	if (!_active) return;

	if (request)
	{
		_discordRequestUserId = request->userId;

		_client.discordJoinRequest(request->username,
			request->discriminator,
			_discordCDN.getAvatarPicture(request->userId, request->avatar));
	}
}

void DiscordRPC::clearLobbyInfo()
{
	if (!_installed) return;

	_dpPartyId = "";
	_dpPlayers = 0;
	_dpBots = 0;
	_dpPlayersMax = 0;
	_dpPublic = false;
	_dpIngame = false;
	_dpJoinSecret = "";
	_dpSpectateSecret = "";
	_dpChallenge = "";
	_dpReplay = "";
	_dpMap = "";

	_discordPresence.details = "";
	_discordPresence.largeImageKey = "planetlogo";
	_discordPresence.largeImageText = "";
	_discordPresence.partyId = "";
	_discordPresence.partySize = 0;
	_discordPresence.partyMax = 0;
	_discordPresence.joinSecret = nullptr;
	_discordPresence.spectateSecret = nullptr;
	_discordPresence.startTimestamp = 0;
	_discordPresence.endTimestamp = 0;
}

void DiscordRPC::updateLobbyInfo()
{
	if (!_installed) return;

	if (_dpPartyId.empty())
	{
		// Wait for a subsequent ::inLobby() call to update the lobby info.
		return;
	}

	if (_dpMap == "tutorial")
	{
		_discordPresence.details = "Tutorial";
		_discordPresence.largeImageKey = "tutorial";
		_discordPresence.largeImageText = "";
	}
	else if (!_dpChallenge.empty())
	{
		_discordPresence.details = "Challenge";
		_discordPresence.largeImageKey = "versusai";
		_discordPresence.largeImageText = "";
	}
	else if (!_dpReplay.empty())
	{
		_discordPresence.details = "Replay Mode";
		_discordPresence.largeImageKey = "planetlogo";
		_discordPresence.largeImageText = "";
	}
	else if (_dpPlayersMax == 2
		&& _dpBots == 0)
	{
		_discordPresence.details = "One vs One";
		_discordPresence.largeImageKey = "onevsone";
		_discordPresence.largeImageText = "";
	}
	else if (_dpPlayersMax > 2
		&& _dpBots < _dpPlayersMax - 1)
	{
		_discordPresence.details = "Free For All";
		_discordPresence.largeImageKey = "freeforall";
		_discordPresence.largeImageText = "";
	}
	else if (_dpPlayers >= 1)
	{
		_discordPresence.details = "Versus AI";
		_discordPresence.largeImageKey = "versusai";
		_discordPresence.largeImageText = "";
	}
	else
	{
		_discordPresence.details = "AI vs AI";
		_discordPresence.largeImageKey = "versusai";
		_discordPresence.largeImageText = "";
	}

	if (!_dpChallenge.empty())
	{
		auto iter = std::find(_dpListedChallengeKey.begin(),
			_dpListedChallengeKey.end(), _dpChallenge);
		if (iter < _dpListedChallengeKey.end())
		{
			size_t offset = iter - _dpListedChallengeKey.begin();
			if (!_dpListedChallengeImageKey[offset].empty())
			{
				_discordPresence.largeImageKey =
					_dpListedChallengeImageKey[offset].c_str();
			}
			if (!_dpListedChallengeDisplayName[offset].empty())
			{
				_discordPresence.largeImageText =
					_dpListedChallengeDisplayName[offset].c_str();
			}
		}
	}
	else if (_dpMap.find_first_of('/') != std::string::npos)
	{
		size_t seppos = _dpMap.find_first_of("/");
		std::string mapname = _dpMap.substr(seppos + 1);
		DEBUG_ASSERT(isValidUserContentName(mapname));
		if (!isValidUserContentName(mapname))
		{
			mapname = "???";
		}
		std::string username = _dpMap.substr(0, seppos);
		DEBUG_ASSERT(isValidUsername(username));
		if (!isValidUsername(username))
		{
			username = "a user";
		}
		_dpUserMapDescription = "\"" + mapname + "\" by " + username;
		_discordPresence.largeImageText = _dpUserMapDescription.c_str();
	}
	else if (!_dpMap.empty())
	{
		_discordPresence.largeImageKey = _dpMap.c_str();
		_discordPresence.largeImageText = _dpMap.c_str();
	}
}

void DiscordRPC::acceptJoinRequest()
{
	if (!_installed) return;

	if (_active)
	{
		Discord_Respond(_discordRequestUserId.c_str(), DISCORD_REPLY_YES);
	}

	_discordRequestUserId.clear();
}

void DiscordRPC::denyJoinRequest()
{
	if (!_installed) return;

	if (_active)
	{
		Discord_Respond(_discordRequestUserId.c_str(), DISCORD_REPLY_NO);
	}

	_discordRequestUserId.clear();
}

void DiscordRPC::ignoreJoinRequest()
{
	if (!_installed) return;

	if (_active)
	{
		Discord_Respond(_discordRequestUserId.c_str(), DISCORD_REPLY_IGNORE);
	}

	_discordRequestUserId.clear();
}

void DiscordRPC::debugHandler() const
{
	LOGD << ((void*) this);
}
