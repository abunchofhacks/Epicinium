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
#include "client.hpp"
#include "source.hpp"

#include <chrono>

	#include "libs/SDL2/SDL_net.h"

#if FEMTOZIP_ENABLED
#include "libs/femtozip/CompressionModel.h"
#include "libs/femtozip/DataIO.h"
#endif

#include "limits.hpp"
#include "message.hpp"
#include "clienthandler.hpp"
#include "gameowner.hpp"
#include "game.hpp"
#include "settings.hpp"
#include "writer.hpp"
#include "player.hpp"
#include "role.hpp"
#include "target.hpp"
#include "platform.hpp"
#include "download.hpp"
#include "patch.hpp"
#include "system.hpp"
#include "checksum.hpp"
#include "base32.hpp"
#include "parseerror.hpp"
#include "curl.hpp"
#include "ranking.hpp"
#include "compress.hpp"
#include "locator.hpp"
#include "library.hpp"
#include "dictator.hpp"
#include "firewall.hpp"
#include "loginstaller.hpp"


#ifdef DEVELOPMENT
#ifdef CANDIDATE
// The test website url for testing release candidates.
#define WEBSITE_ORIGIN "https://test.epicinium.nl"
#else
// During development, a non-https connection to localhost is fine.
#define WEBSITE_ORIGIN "http://epicinium.localhost"
#endif
#else
// The official website url. This url should always be kept alive
// and backwards compatibility with older game clients, in particular
// older api versions, should be preserved. It cannot be overridden.
#define WEBSITE_ORIGIN "https://server.epicinium.nl"
#endif

#if DISCORD_GUEST_ENABLED
static std::string _discordUserId;
static std::string _discordUsername;
static std::string _discordDiscriminator;
#endif

Client::Client(ClientHandler& owner, GameOwner& gameowner, Settings& settings,
		Firewall* firewall) :
	_owner(owner),
	_gameowner(gameowner),
#ifdef DEVELOPMENT
	_firewall(firewall),
#else
	// TODO hopefully the compiler can detect that _firewall is always nullptr
	_firewall((firewall) ? nullptr : nullptr),
#endif
	_settings(settings)
{
#if STEAM_ENABLED
	_isSteamEnabled = _settings.steam.value(true);
#endif

	// Initialize SDLNet.
	SDL_ClearError();
	if (SDLNet_Init())
	{
		throw std::runtime_error("SDLNet_Init failed: "
			+ std::string(SDLNet_GetError()));
	}

	// Create an asynchronous HTTP client.
	std::stringstream strm;
	strm << "epicinium/" << Version::current() << " (" << ::platform();
	switch (_settings.patchmode.value(PatchMode::NONE))
	{
		case PatchMode::ITCHIO: strm << "; itch.io"; break;
		case PatchMode::GAMEJOLT: strm << "; GameJolt"; break;
		case PatchMode::STEAM: strm << "; Steam"; break;
		case PatchMode::SERVER_BUT_DISABLED_DUE_TO_STORAGE_ISSUES:
		case PatchMode::NONE: strm << "; nopatch"; break;
		case PatchMode::SERVER: break;
	}
	strm << ")";
	_curl = std::make_shared<Curl>(strm.str());
}

void Client::discordAvailable()
{
	// Activate Discord only if someone has registered at least once or logged
	// in through Discord at least once.
	if (_settings.discord.value()
		&& (_account.existsSession() || _settings.allowDiscordLogin.value()))
	{
		activateDiscord();
	}
}

void Client::activateDiscord()
{
	_owner.activateDiscord();
}

Client::~Client()
{
	// Disconnect from server.
	disconnect();

	// Deactivate Discord.
	deactivateDiscord();

	// Quit SDLNet.
	SDLNet_Quit();
}

void Client::deactivateDiscord()
{
	_owner.deactivateDiscord();
	_owner.discordDeactivated();
}

void Client::registerHandler(ClientHandler* handler)
{
	_owner.registerHandler(handler);
}

void Client::deregisterHandler(ClientHandler* handler)
{
	_owner.deregisterHandler(handler);
}

void Client::sendPulse()
{
	return send("", 0);
}

void Client::send(StreamedMessage message)
{
	if (!_socket) return;

	if (_firewall) _firewall->screenOutgoing(message);

	std::ostringstream strm;
	strm << message;

	return send(strm.str(), message.compressible());
}

void Client::send(const std::string& jsonString, bool compressible)
{
	size_t stringlength = jsonString.length();
	if (stringlength >= MESSAGE_SIZE_LIMIT)
	{
		LOGF << "Cannot send messages larger than MESSAGE_SIZE_LIMIT.";
		throw std::runtime_error(
				"Cannot send messages larger than MESSAGE_SIZE_LIMIT.");
	}
	const char* data = jsonString.c_str();
	uint32_t length = stringlength;

	LOGD << "Sending message: \'" << jsonString << "\'";

#if FEMTOZIP_ENABLED
	auto model = _compressionModel;
	if (!model || !compressible)
#else
	(void) compressible;
#endif
	{
		return send(data, length);
	}

#if FEMTOZIP_ENABLED
	LOGD << "Compressing message of length " << (length);

	std::ostringstream zipstrm;
	zipstrm << "=";
	model->compress(data, length, zipstrm);
	std::string zipped = zipstrm.str();
	uint32_t ziplen = (uint32_t) zipped.length();

	{
		std::stringstream hash;
		hash << "=";
		hash << std::hex << std::setfill('0');
		for (size_t i = 0; i < ziplen; i++)
		{
			hash << " " << std::setw(2)
				<< (unsigned int) (uint8_t) zipped[i];
		}
		LOGD << "Compressed message length " << (1 + ziplen);
		LOGD << "Compressed message: \'" << hash.str() << "\'";
	}

	return send(zipped.data(), ziplen);
#endif
}

void Client::send(const char* data, uint32_t length)
{
	static std::mutex mut;

	// Get the network order (big-endian) byte representation of the length.
	uint8_t length_be_bytes[4] = {
		uint8_t(length >> 24),
		uint8_t(length >> 16),
		uint8_t(length >> 8),
		uint8_t(length),
	};

	// Reset the last send time so we know we don't need to send pulses.
	_lastsendtime = (double) SDL_GetTicks() * 0.001;

	if (length >= MESSAGE_SIZE_WARNING_LIMIT)
	{
		LOGW << "Sending very large message of length " << length;
	}

	LOGD << "Sending message of length " << length;
	bool success = true;
	{
		// We need a lock around sending the length of the message and sending
		// the message. We do not need a lock around the logs because they
		// mention the thread id.
		std::lock_guard<std::mutex> lock(mut);

		if (SDLNet_TCP_Send(_socket, length_be_bytes, 4) < 4
			|| SDLNet_TCP_Send(_socket, data, length) < int(length))
		{
			success = false;
		}
	}
	if (!success)
	{
		LOGW << "Error while sending message of length " << length;
		return;
	}
	LOGD << "Sent message of length " << length;
}

void Client::handleMessage(const ParsedMessage& message)
{
	switch (message.type())
	{
		case Message::Type::INVALID:
		case Message::Type::RESTORE:
		case Message::Type::REQUEST:
		case Message::Type::ENABLE_CUSTOM_MAPS:
		case Message::Type::LINK_ACCOUNTS:
		case Message::Type::ORDER_OLD:
		case Message::Type::ORDER_NEW:
		{
			LOGW << "Got invalid message (ignored): " << message;
			DEBUG_ASSERT(false);
		}
		break;
		case Message::Type::INIT:
		{
			_initialized = true;
		}
		break;
		case Message::Type::CHAT:
		{
			switch (message.target())
			{
				case Target::NONE:
				{
					LOGW << "Received chat message without target: " << message;
				}
				break;
				case Target::GENERAL:
				case Target::LOBBY:
				{
					if (message.sender() == "server")
					{
						_owner.message(
							GETTEXT_FROM_SERVER(message.content().c_str()));
						// We expect the following messages:
						(void) _("Replays are not available at the moment.");
					}
					else
					{
						_owner.chat(message.sender(), message.content(),
							message.target());
					}
				}
				break;
			}
		}
		break;
		case Message::Type::RATING_AND_STARS:
		{
			_owner.updateRating(message.content(), message.rating());
			_owner.updateStars(message.content(), message.time());
		}
		break;
		case Message::Type::RATING:
		{
			if (message.content().empty())
			{
				_owner.updateOwnRating(message.rating());
			}
			else
			{
				_owner.updateRating(message.content(), message.rating());
			}
		}
		break;
		case Message::Type::STARS:
		{
			if (message.content().empty())
			{
				_owner.updateStars(_account.username(), message.time());
			}
			else
			{
				_owner.updateStars(message.content(), message.time());
			}
		}
		break;
		case Message::Type::RECENT_STARS:
		{
			_owner.updateRecentStars(message.time());
		}
		break;
		case Message::Type::JOIN_SERVER:
		{
			std::string newname = message.content();

			if (newname.empty())
			{
				_sessionStatus = message.status();
				_owner.sessionFailed(_sessionStatus);
			}
			// This is information about successfully joining.
			else if (_account.username().empty())
			{
				// A previous JOIN_SERVER may have set _sessionStatus, but
				// this new attempt (without a login in between) was successful
				// because we entered a key or changed our username.
				_sessionStatus = ResponseStatus::SUCCESS;

				_account.setUsername(newname);
				_owner.inServer();
				if (_accessedPortal)
				{
					_owner.message(::format(
						// TRANSLATORS: The argument is a username.
						_("Hello %s, welcome to the Epicinium server!"),
						_account.username().c_str()));
				}
				else
				{
					DEBUG_ASSERT(_settings.server.defined());
					DEBUG_ASSERT(_settings.port.defined());
					_owner.message(::format(
						// TRANSLATORS: The first two arguments are a URL or an
						// IP address followed by a port number.
						// The last argument is a username.
						_("Connected to %s:%d as %s."),
						_settings.server.value("").c_str(),
						_settings.port.value(0),
						_account.username().c_str()));
				}
			}
			// This is information about who is online: us.
			else if (newname == _account.username())
			{
				_owner.addUser(newname, message.metadata(), true);
			}
			// This is information about who is online: someone else.
			else
			{
				_owner.addUser(newname, message.metadata(), false);
				if (_initialized)
				{
					_owner.message(::format(
						// TRANSLATORS: The argument is a username.
						_("%s is online."),
						newname.c_str()));
				}
			}
		}
		break;
		case Message::Type::LEAVE_SERVER:
		{
			std::string username = message.content();

			if (_account.username().empty() && !_account.sessionToken().empty())
			{
				LOGI << "Ghostbusting successful; attempting to join again...";
				join();
			}
			else if (username != _account.username())
			{
				_owner.removeUser(username);
				if (_initialized)
				{
					_owner.message(::format(
						// TRANSLATORS: The argument is a username.
						_("%s went offline."),
						username.c_str()));
				}
			}
			else
			{
				_account.setUsername("");
				_owner.outServer();
			}
		}
		break;
		case Message::Type::JOIN_LOBBY:
		{
			const std::string& content = message.content();
			const std::string& sender = message.sender();

			if (sender.empty())
			{
				if (content.empty())
				{
					_owner.message(
						_(""
						"Failed to join lobby"
						" because it is private or full."
						""));
				}
				else
				{
					_owner.message(
						_("You are already in a lobby."));
				}
				hotClear();
			}
			else if (content.empty())
			{
				_owner.joinsLobby(sender);
			}
			else if (_account.username() == sender)
			{
				_lobbyID = content;
				_owner.inLobby(content);
				_owner.joinsLobby(sender);
				_owner.joinsOwnLobby(sender, true);
				hotClear();
			}
			else if (content == _lobbyID)
			{
				_owner.joinsLobby(sender);
				_owner.joinsOwnLobby(sender, false);
				_owner.message(::format(
					// TRANSLATORS: The argument is a username.
					_("%s joined the lobby."),
					sender.c_str()));
			}
			else
			{
				_owner.joinsLobby(sender);
			}
		}
		break;
		case Message::Type::LEAVE_LOBBY:
		{
			const std::string& content = message.content();
			const std::string& sender = message.sender();

			if (sender.empty())
			{
				_owner.message(
					_("You are not in a lobby."));
			}
			else if (content.empty())
			{
				_owner.leavesLobby(sender);
			}
			else if (_account.username() == sender)
			{
				_lobbyID = "";
				_owner.leavesLobby(sender);
				_owner.outLobby();
			}
			else if (content == _lobbyID)
			{
				_owner.leavesLobby(sender);
				_owner.leavesOwnLobby(sender);
				_owner.message(::format(
					// TRANSLATORS: The argument is a username.
					_("%s left the lobby."),
					sender.c_str()));
			}
			else
			{
				_owner.leavesLobby(sender);
			}
		}
		break;
		case Message::Type::LIST_LOBBY:
		{
			const std::string& lobbyid = message.content();
			const std::string& lobbyname = message.sender();

			_owner.listLobby(lobbyid, lobbyname, message.metadata());
			if (lobbyid == _lobbyID)
			{
				_owner.listOwnLobby(lobbyname, message.metadata());
			}
		}
		break;
		case Message::Type::MAKE_LOBBY:
		{
			const std::string& content = message.content();

			if (content.empty())
			{
				_owner.message(
					_("You are already in a lobby."));
			}
			else
			{
				DEBUG_ASSERT(false && "deprecated");
			}
		}
		break;
		case Message::Type::DISBAND_LOBBY:
		{
			const std::string& content = message.content();
			_owner.removeLobby(content);
		}
		break;
		case Message::Type::EDIT_LOBBY:
		case Message::Type::SAVE_LOBBY:
		case Message::Type::LOCK_LOBBY:
		case Message::Type::UNLOCK_LOBBY:
		case Message::Type::NAME_LOBBY:
		case Message::Type::MAX_PLAYERS:
		case Message::Type::NUM_PLAYERS:
		{
			DEBUG_ASSERT(false && "deprecated");
		}
		break;
		case Message::Type::CLAIM_ROLE:
		{
			std::string rname = ::stringify(message.role());
			if (message.sender() == _account.username())
			{
				_owner.assignRole(message.sender(), message.role(), true);
			}
			else if (message.sender() != "")
			{
				_owner.assignRole(message.sender(), message.role(), false);
			}
			else
			{
				LOGE << "No sender set";
			}
		}
		break;
		case Message::Type::CLAIM_COLOR:
		{
			std::string cname = ::stringify(message.player());
			if (message.sender() == _account.username())
			{
				_owner.assignColor(message.sender(), message.player());
			}
			else if (message.sender() != "")
			{
				_owner.assignColor(message.sender(), message.player());
			}
			else
			{
				LOGE << "No sender set";
			}
		}
		break;
		case Message::Type::CLAIM_VISIONTYPE:
		{
			std::string vname = ::stringify(message.visiontype());
			if (message.sender() == _account.username())
			{
				_owner.assignVisionType(message.sender(), message.visiontype());
			}
			else if (message.sender() != "")
			{
				_owner.assignVisionType(message.sender(), message.visiontype());
			}
			else
			{
				LOGE << "No sender set";
			}
		}
		break;
		case Message::Type::CLAIM_AI:
		{
			_owner.assignAI(message.sender(), message.content());
		}
		break;
		case Message::Type::CLAIM_DIFFICULTY:
		{
			_owner.assignDifficulty(message.sender(),
				message.difficulty());
		}
		break;
		case Message::Type::PICK_MAP:
		{
			_owner.pickMap(message.content());
		}
		break;
		case Message::Type::PICK_TIMER:
		{
			_owner.pickTimer(message.time());
		}
		break;
		case Message::Type::PICK_REPLAY:
		{
			_owner.pickReplay(message.content());
		}
		break;
		case Message::Type::PICK_CHALLENGE:
		{
			_owner.pickChallenge(message.content());
		}
		break;
		case Message::Type::PICK_RULESET:
		{
			_owner.pickRuleset(message.content());
		}
		break;
		case Message::Type::ADD_BOT:
		{
			_owner.addBot(message.content());
		}
		break;
		case Message::Type::REMOVE_BOT:
		{
			_owner.removeBot(message.content());
		}
		break;
		case Message::Type::LIST_MAP:
		{
			_owner.listMap(message.content(), message.metadata());
		}
		break;
		case Message::Type::LIST_REPLAY:
		{
			_owner.listReplay(message.content(), message.metadata());
		}
		break;
		case Message::Type::LIST_RULESET:
		{
			_owner.listRuleset(message.content(), message.metadata());
		}
		break;
		case Message::Type::LIST_AI:
		{
			_owner.listAI(message.content(), message.metadata());
		}
		break;
		case Message::Type::LIST_CHALLENGE:
		{
			_owner.listChallenge(message.content(), message.metadata());
		}
		break;
		case Message::Type::RULESET_REQUEST:
		{
			// Not yet implemented. (#1304)
			send(Message::ruleset_unknown(message.content()));
		}
		break;
		case Message::Type::RULESET_DATA:
		{
			std::string rulesetname = message.content();
			std::string filename = Locator::rulesetFilename(rulesetname);
			if (Library::storeBible(rulesetname, message.data()))
			{
				_owner.requestFulfilled(filename);
			}
			else
			{
				_owner.requestFailed(filename);
			}
		}
		break;
		case Message::Type::RULESET_UNKNOWN:
		{
			std::string rulesetname = message.content();
			std::string filename = Locator::rulesetFilename(rulesetname);
			_owner.requestFailed(filename);
		}
		break;
		case Message::Type::SECRETS:
		{
			_owner.receiveSecrets(message.metadata());
		}
		break;
		case Message::Type::BRIEFING:
		{
			_owner.receiveBriefing(message.metadata());
		}
		break;
		case Message::Type::RESIGN:
		{
			if (message.content().empty())
			{
				LOGW << "Received invalid resign message from server";
			}
			else if (message.content() == _account.username())
			{
				_owner.message(
					_("You resigned."));
			}
			else
			{
				_owner.message(::format(
					// TRANSLATORS: The argument is a username.
					_("%s has resigned."),
					message.content().c_str()));
			}
		}
		break;
		case Message::Type::SKINS:
		{
			_owner.setSkins(message.metadata());
		}
		break;
		case Message::Type::IN_GAME:
		{
			_owner.inGame(message.content(), message.sender(), message.role());
		}
		break;
		case Message::Type::START:
		{
			_owner.message(
				_("Starting game failed."));
		}
		break;
		case Message::Type::GAME:
		{
			if (message.role() != Role::NONE || message.player() != Player::NONE)
			{
				_owner.message(
					_("Starting game..."));

				switch (message.role())
				{
					case Role::PLAYER:
					{
						_game = _gameowner.startGame(
							message.player(),
							message.content(),
							message.time());
					}
					break;
					default:
					{
						_game = _gameowner.startReplay(
							message.role(),
							message.content(),
							message.time());
					}
					break;
				}

				_owner.startGame(message.role());
			}
			else
			{
				_owner.message(
					_("Starting game failed."));
			}
		}
		break;
		case Message::Type::TUTORIAL:
		{
			if (message.role() != Role::NONE || message.player() != Player::NONE)
			{
				_owner.message(
					_("Starting tutorial..."));

				_game = _gameowner.startTutorial(
					message.player(),
					message.content(),
					message.time());

				_owner.startTutorial();
			}
			else
			{
				_owner.message(
					_("Starting tutorial failed."));
			}
		}
		break;
		case Message::Type::CHALLENGE:
		{
			_owner.message(
				_("Starting challenge failed."));
		}
		break;
		case Message::Type::REPLAY:
		{
			bool animation = (message.time() > 0);
			if (auto game = _game.lock()) game->setAnimation(animation);
			else LOGW << "Received replay speed message while not in a game or replay";
		}
		break;
		case Message::Type::CHANGE:
		{
			if (auto game = _game.lock())
			{
				game->receiveChanges(message.changes());
			}
			else LOGW << "Received change while not in a game or replay";
		}
		break;
		case Message::Type::SYNC:
		{
			if (auto game = _game.lock()) game->sync(message.time());
			else LOGW << "Received sync message while not in a game or replay";
		}
		break;
		case Message::Type::PULSE:
		{
			// The server just let us know that it is still breathing.
		}
		break;
		case Message::Type::PING:
		{
			// Pings must always be responded with pongs.
			send(Message::pong());
			return;
		}
		break;
		case Message::Type::PONG:
		{
			// Remember the ping time.
			if (_pingsendtime >= 0.0)
			{
				double currenttime = SDL_GetTicks() * 0.001;
				_lastknownping = currenttime - _pingsendtime;
				_pingsendtime = -1.0;
				if (_notifiedSlowResponse)
				{
					_notifiedSlowResponse = false;
					_owner.message(
						_("Connection to server resumed."));
				}
				LOGD << "We have "
					<< (int) (_lastknownping * 1000)
					<< "ms ping.";
			}
			return;
		}
		break;
		case Message::Type::QUIT:
		{
			disconnect(/*sendMessage=*/false);
		}
		break;
		case Message::Type::CLOSED:
		{
			// Disconnect first, then overwrite the message.
			disconnect();
			_owner.serverShutdown();
			_owner.message(
				_(""
				"The server has been shut down for maintenance."
				" We'll be back soon!"
				""));
		}
		break;
		case Message::Type::CLOSING:
		{
			_owner.serverClosing();
			_owner.message(
				_(""
				"The server is shutting down for maintenance."
				" (No new games)"
				""));
		}
		break;
		case Message::Type::VERSION:
		{
			Version myversion = Version::current();
			if (   message.version().major == myversion.major
				&& message.version().minor == myversion.minor)
			{
				_disconnected = false;
				// Throw a welcome message into the history.
				if (_accessedPortal)
				{
					_owner.message(
						_("The Epicinium server is online."));
				}
				else
				{
					DEBUG_ASSERT(_settings.server.defined());
					DEBUG_ASSERT(_settings.port.defined());
					std::stringstream strm;
					strm << "Connected to "
						<< _settings.server.value("")
						<< ":" << _settings.port.value(0) << ".";
					_owner.message(strm.str());
				}
				_owner.connected();
				_owner.promptUsername();
			}
			else
			{
				_owner.versionMismatch(message.version());
			}
		}
		break;
		case Message::Type::PATCH:
		case Message::Type::DOWNLOAD:
		case Message::Type::REQUEST_FULFILLED:
		case Message::Type::REQUEST_DENIED:
		case Message::Type::STAMP:
		{
			LOGW << "Message-based downloads disabled; ignoring message.";
		}
		break;
		case Message::Type::ENABLE_COMPRESSION:
		{
			if (!message.content().empty())
			{
				_fzmodelname = message.content();
				if (System::isFile(Locator::fzmodelFilename(_fzmodelname)))
				{
					enableCompression();
				}
				else
				{
					requestFzmodel();
				}
			}
			else LOGW << "Received invalid compression message from server";
		}
		break;
		case Message::Type::DISABLE_COMPRESSION:
		{
			_compressionModel.reset();
			LOGI << "Disabled compression";
		}
		break;
		case Message::Type::RANKINGS:
		{
			LOGI << "Received rankings from server";
			_owner.displayRankings(parseRankings(message.metadata()));
		}
		break;
	}
}

bool Client::checkConnection()
{
	if (_socket) return true;
	if (_connectstarttime < 0.0f) return false;
	double now = (double) SDL_GetTicks() * 0.001;
	if (now >= _connectstarttime + 10.0f) connect();
	if (!_connectThread.joinable()) return false;
	if (!_futureSocket.valid()) return false;
	bool ready = (_futureSocket.wait_for(std::chrono::seconds(0))
		== std::future_status::ready);

	if (!ready && now < _connectstarttime + 5.0f) return false;

	if (ready)
	{
		_connectThread.join();
		_socket = _futureSocket.get();
	}
	else
	{
		_connectThread.detach();
	}

	if (!ready || !_socket)
	{
		LOGI << "Failed to connect to "
			<< _settings.server.value("")
			<< ":" << _settings.port.value(0) << ".";
		// Throw an unwelcome message into the history.
		_owner.serverOffline();
		if (_accessedPortal)
		{
			_owner.message(
				_(""
				"The Epicinium server is not online at the moment."
				" Please try again later."
				""));
		}
		else
		{
			DEBUG_ASSERT(_settings.server.defined());
			DEBUG_ASSERT(_settings.port.defined());
			std::stringstream strm;
			strm << "Failed to connect to "
				<< _settings.server.value("")
				<< ":" << _settings.port.value(0) << ".";
			_owner.message(strm.str());
		}
		_futureSocket = std::future<TCPsocket>();

		// The server might have gone offline in the time between the portal
		// preparing its response and us receiving it. Try again, but only once.
		// Only do this if we accessed the portal in the first place.
		if (_accessedPortal && !_autoRetryPortal)
		{
			_autoRetryPortal = true;
			connectToPortal();
		}

		return false;
	}

	// Send our version number to the server.
	{
		Json::Value metadata(Json::objectValue);
		metadata["platform"] = ::stringify(::platform());
		DEBUG_ASSERT(_settings.patchmode.defined());
		switch (_settings.patchmode.value(PatchMode::NONE))
		{
			case PatchMode::SERVER_BUT_DISABLED_DUE_TO_STORAGE_ISSUES:
			case PatchMode::NONE:     metadata["patchmode"] = "none";     break;
			case PatchMode::SERVER:   metadata["patchmode"] = "server";   break;
			case PatchMode::ITCHIO:   metadata["patchmode"] = "itchio";   break;
			case PatchMode::GAMEJOLT: metadata["patchmode"] = "gamejolt"; break;
			case PatchMode::STEAM:    metadata["patchmode"] = "steam";    break;
		}
		send(Message::version(Version::current(), metadata));
	}

	// Send a ping message, just to get an estimated ping.
	if (_pingsendtime < 0)
	{
		_pingsendtime = (double) SDL_GetTicks() * 0.001;
		send(Message::ping());
	}

	// Attempt to load previous session.
	if (!_isSteamEnabled
		&& (!_account.sessionToken().empty()
			|| _account.loadSession()))
	{
		LOGI << "Loaded previous session";
		_owner.loggedIn();
		if (_settings.discord.value())
		{
			activateDiscord();
		}
	}

	// We successfully connected to the server, so we are allowed to retry
	// the portal again if this connection fails at some point in the future.
	_autoRetryPortal = false;

	return true;
}

void Client::checkVitals()
{
	double currenttime = SDL_GetTicks() * 0.001;
	// If we have never received any message from the server...
	if (_lastreceivetime < 0)
	{
		// ... consider our connection to be the first message.
		_lastreceivetime = currenttime;
	}
	// If the server does not respond to a ping within 33 seconds...
	else if (_pingsendtime >= 0
		&& _pingsendtime + 33 < currenttime)
	{
		// ... kill the connection...
		LOGW << "Disconnecting from inactive server.";
		disconnect();
		// ... and attempt to reconnect after the menus have settled down.
		_resetting = true;
	}
	// If the server does not respond to a ping within 10 seconds...
	else if (_pingsendtime >= 0
		&& _pingsendtime + 10 < currenttime && !_notifiedSlowResponse)
	{
		LOGW << "Server is taking longer than 10 seconds to answer ping.";
		_owner.message(
			_("Waiting for response from server..."));
		_notifiedSlowResponse = true;
	}
	// If the server has been silent for 5 seconds and we are not
	// already waiting for their ping message ...
	else if (_pingsendtime < 0
		&& _lastreceivetime + 5 < currenttime)
	{
		// ... send a ping message to make sure they are still alive.
		_pingsendtime = currenttime;
		send(Message::ping());
	}
	// If we sent something this step, the server knows we are breathing.
	else if (_lastsendtime < 0)
	{
		_lastsendtime = currenttime;
	}
	// If we have been silent for 4 seconds (slightly less than 5), we need
	// to let the server know we are still breathing.
	else if (_lastsendtime + 4 < currenttime)
	{
		sendPulse();
	}
}

void Client::checkPortal()
{
	if (!_futurePortal.valid()) return;
	bool ready = (_futurePortal.wait_for(std::chrono::seconds(0))
		== std::future_status::ready);
	if (!ready) return;

	handlePortalResponse(_futurePortal.get());
}

void Client::handlePortalInfo(const std::string& host, uint16_t port)
{
	// Store the server address and port in the settings,
	// but in the override slot to prevent them from being saved;
	// they will be queried again the next time the game is booted up.
	_settings.server.override(host);
	_settings.port.override(port);

	LOGI << "Accessed portal";
	_accessedPortal = true;
	_owner.accessedPortal();

	connect(_settings.server.value(), _settings.port.value());
}

void Client::checkPatchManifest()
{
	if (!_futurePatchManifest.valid()) return;
	bool ready = (_futurePatchManifest.wait_for(std::chrono::seconds(0))
		== std::future_status::ready);
	if (!ready) return;

	handlePatchManifestResponse(_futurePatchManifest.get());
}

void Client::checkAutoPatch()
{
	if (!_futureAutoPatch.valid()) return;
	bool ready = (_futureAutoPatch.wait_for(std::chrono::seconds(0))
		== std::future_status::ready);
	if (!ready) return;

	handleAutoPatchResponse(_futureAutoPatch.get());
}

void Client::checkPatchProgress()
{
	if (_downloadsInProgress <= 0) return;

	int inprogress = _downloadsInProgress;
	bool specific = false;

	for (size_t i = 0; i < _downloads.size(); i++)
	{
		std::future<Response>& future = _futureDownloads[i];
		if (!future.valid()) continue;

		bool ready = (future.wait_for(std::chrono::seconds(0))
			== std::future_status::ready);
		if (!ready)
		{
			_downloads[i].progress = *_percentages[i];

			if (!specific
				&& _downloads[i].progress >  0.001f
				&& _downloads[i].progress < 99.999f)
			{
				_owner.downloading(_downloads[i].targetfilename,
					_downloads[i].progress);
				specific = true;
			}

			continue;
		}

		Response response = future.get();
		if (response.errorcode)
		{
			LOGE << "Download failed due to connection failure";
			_owner.patchFailed();
			inprogress = 0;
			break;
		}

		if (response.statuscode != 200)
		{
			LOGE << "Download failed with status code " << response.statuscode;
			_owner.patchFailed();
			inprogress = 0;
			break;
		}

		LOGI << "Download successful: " << response.body;
		_downloads[i].progress = 100.0f;
		inprogress--;
	}

	if (inprogress == 0)
	{
		Version patchversion;
		if (_patchVersion)
		{
			patchversion = *_patchVersion;
		}

		if (isPatchPrimed(patchversion))
		{
			_owner.patchPrimed(patchversion);

			if (_autoPatchActivated)
			{
				_owner.patchContinued(patchversion);
			}
		}
		else
		{
			Patch::purge();
			_owner.patchFailed();
		}
	}
	else if (inprogress != _downloadsInProgress)
	{
		LOGI << "There are " << inprogress << " downloads in progress...";
		_downloadsInProgress = inprogress;

		if (!specific)
		{
			float percentage = 100.0f
				* (_downloads.size() - _downloadsInProgress)
				/ _downloads.size();
			_owner.downloading("", percentage);
		}
	}
}

void Client::requestFzmodel()
{
	if (!_accessedPortal)
	{
		LOGI << "Cannot request file; portal not accessed.";
		return;
	}

	LOGI << "Downloading new compression model"
		<< " \'" << _fzmodelname << "\'";

	constexpr const char* CSET = "abcdefghijklmnopqrstuvwxyz"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.-_";
	if (_fzmodelname.find_first_not_of(CSET) != std::string::npos)
	{
		LOGE << "Cannot request file: illegal characters in name.";
		DEBUG_ASSERT(false);
		return;
	}

	constexpr const char* BASE_URL = WEBSITE_ORIGIN "/api/v1/request"
		"?type=fzmodel&name=";
	std::string url = BASE_URL + _fzmodelname;
	_futureFzmodelRequest = _curl->get(url);
}

void Client::checkFzmodelRequest()
{
	if (!_futureFzmodelRequest.valid()) return;
	bool ready = (_futureFzmodelRequest.wait_for(std::chrono::seconds(0))
		== std::future_status::ready);
	if (!ready) return;

	Response response = _futureFzmodelRequest.get();
	if (response.errorcode)
	{
		LOGE << "Requesting fzmodel failed: connection failure";
		return;
	}

	LOGI << "Got a response from request server:"
		<< " [" << response.statuscode << "] "
		<< response.body;
	if (response.statuscode != 200)
	{
		LOGE << "Requesting fzmodel failed: invalid status code";
		return;
	}

	Json::Reader reader;
	Json::Value json;
	if (!reader.parse(response.body, json) || !json.isObject())
	{
		LOGE << "Requesting fzmodel failed: received malformed response";
		return;
	}

	Download download;
	try
	{
		download = Download::parse(json);
	}
	catch (Json::Exception& error)
	{
		LOGE << "Requesting fzmodel failed: invalid manifest";
		_owner.patchFailed();
		return;
	}

	System::touchFile(download.sourcefilename);

	if (json["download"].isString())
	{
		LOGI << "Downloading '" << download.targetfilename << "'...";
		std::string url = WEBSITE_ORIGIN + json["download"].asString();
		auto progressmeter = std::make_shared<std::atomic<float>>(0);
		_futureFzmodelDownload = _curl->download(url,
			download.sourcefilename, progressmeter);
		_activeFzmodelDownload = std::unique_ptr<Download>(new Download(
				std::move(download)));
		return;
	}
	else
	{
		LOGE << "Requesting fzmodel failed: no download link";
		return;
	}
}

void Client::checkFzmodelDownload()
{
	if (!_activeFzmodelDownload) return;
	if (!_futureFzmodelDownload.valid()) return;
	bool ready = (_futureFzmodelDownload.wait_for(std::chrono::seconds(0))
		== std::future_status::ready);
	if (!ready) return;

	Response response = _futureFzmodelDownload.get();
	if (response.errorcode)
	{
		LOGE << "Download failed due to connection failure";
		_activeFzmodelDownload.reset();
		return;
	}

	if (response.statuscode != 200)
	{
		LOGE << "Download failed with status code " << response.statuscode;
		_activeFzmodelDownload.reset();
		return;
	}

	LOGI << "Download successful: " << response.body;
	bool installed = fulfilRequest(*_activeFzmodelDownload);
	if (installed)
	{
		std::string filename = Locator::fzmodelFilename(_fzmodelname);
		if (filename != _activeFzmodelDownload->targetfilename)
		{
			LOGE << "Needed '" << filename << "' but downloaded"
				" '" << _activeFzmodelDownload->targetfilename << "'";
			_activeFzmodelDownload.reset();
			return;
		}
		enableCompression();
	}
	_activeFzmodelDownload.reset();
}

void Client::enableCompression()
{
#if FEMTOZIP_ENABLED
	LOGI << "Enabling compression with model '" << _fzmodelname << "'";
	{
		std::string filename = Locator::fzmodelFilename(_fzmodelname);
		std::ifstream file(filename, std::ios::binary);
		if (!file)
		{
			LOGE << "Failed to open '" << filename << "'";
			DEBUG_ASSERT(false);
			return;
		}
		femtozip::DataInput in(file);
		_compressionModel.reset(femtozip::CompressionModel::loadModel(in));
		_compressionModel->setCompressionLevel(0);
	}
	LOGI << "Enabled compression";
	send(Message::enable_compression(_fzmodelname));
#else
	LOGW << "Cannot enable compression without FemtoZip support";
#endif
}

void Client::checkRegistration()
{
	if (!_futureRegistration.valid()) return;
	bool ready = (_futureRegistration.wait_for(std::chrono::seconds(0))
		== std::future_status::ready);
	if (!ready) return;
	ResponseStatus status = parseRegistrationResponse(
		_futureRegistration.get());
	switch (status)
	{
		case ResponseStatus::SUCCESS:
		{
			LOGI << "Registered";
			_owner.registered();
		}
		break;
		case ResponseStatus::USERNAME_TAKEN:
		case ResponseStatus::EMAIL_TAKEN:
		{
			LOGI << "Registration failed due to user error"
				<< " with response status "
				<< std::to_string((int) status);
			_owner.registrationFailed(status);
		}
		break;
		case ResponseStatus::CONNECTION_FAILED:
		case ResponseStatus::CREDS_INVALID:
		case ResponseStatus::ACCOUNT_LOCKED:
		case ResponseStatus::ACCOUNT_DISABLED:
		case ResponseStatus::KEY_TAKEN:
		case ResponseStatus::IP_BLOCKED:
		case ResponseStatus::KEY_REQUIRED:
		case ResponseStatus::EMAIL_UNVERIFIED:
		case ResponseStatus::USERNAME_REQUIRED_NOUSER:
		case ResponseStatus::USERNAME_REQUIRED_INVALID:
		case ResponseStatus::USERNAME_REQUIRED_TAKEN:
		case ResponseStatus::METHOD_INVALID:
		case ResponseStatus::REQUEST_MALFORMED:
		case ResponseStatus::RESPONSE_MALFORMED:
		case ResponseStatus::DATABASE_ERROR:
		{
			LOGE << "Registration failed with unexpected response status "
				<< std::to_string((int) status);
			_owner.registrationFailed(status);
		}
		break;
		case ResponseStatus::UNKNOWN_ERROR:
		{
			LOGE << "Registration failed due to unknown error";
			_owner.registrationFailed(status);
		}
		break;
	}
}

void Client::checkLogin()
{
	if (!_account.username().empty()) return;
	if (!_futureLogin.valid()) return;
	bool ready = (_futureLogin.wait_for(std::chrono::seconds(0))
		== std::future_status::ready);
	if (!ready) return;
	LoginData data = parseLoginResponse(_futureLogin.get());
	switch (data.status)
	{
		case ResponseStatus::SUCCESS:
		{
			_account.set(data.accountId, data.token, data.remember);
			LOGI << "Logged in with account id " << data.accountId;
			_owner.loggedIn();
			if (_settings.discord.value())
			{
				activateDiscord();
			}
		}
		break;
		case ResponseStatus::CREDS_INVALID:
		case ResponseStatus::ACCOUNT_LOCKED:
		case ResponseStatus::ACCOUNT_DISABLED:
		{
			LOGI << "Login failed due to user error with response status "
				<< std::to_string((int) data.status);
			logout(true);
			_owner.loginFailed(data.status);
		}
		break;
		case ResponseStatus::KEY_TAKEN:
		case ResponseStatus::IP_BLOCKED:
		case ResponseStatus::KEY_REQUIRED:
		case ResponseStatus::EMAIL_UNVERIFIED:
		case ResponseStatus::USERNAME_REQUIRED_NOUSER:
		case ResponseStatus::USERNAME_REQUIRED_INVALID:
		case ResponseStatus::USERNAME_REQUIRED_TAKEN:
		case ResponseStatus::CONNECTION_FAILED:
		case ResponseStatus::METHOD_INVALID:
		case ResponseStatus::REQUEST_MALFORMED:
		case ResponseStatus::RESPONSE_MALFORMED:
		case ResponseStatus::USERNAME_TAKEN:
		case ResponseStatus::EMAIL_TAKEN:
		case ResponseStatus::DATABASE_ERROR:
		{
			LOGE << "Login failed with unexpected response status "
				<< std::to_string((int) data.status);
			logout(true);
			_owner.loginFailed(data.status);
		}
		break;
		case ResponseStatus::UNKNOWN_ERROR:
		{
			LOGE << "Login failed due to unknown error";
			logout(true);
			_owner.loginFailed(data.status);
		}
		break;
	}
}

void Client::checkKey()
{
	if (!_futureKey.valid()) return;
	bool ready = (_futureKey.wait_for(std::chrono::seconds(0))
		== std::future_status::ready);
	if (!ready) return;
	ResponseStatus status = parseKeyActivationResponse(_futureKey.get());
	switch (status)
	{
		case ResponseStatus::SUCCESS:
		{
			LOGI << "Successfully activated key";
			_owner.activatedKey();
		}
		break;
		case ResponseStatus::CREDS_INVALID:
		case ResponseStatus::KEY_TAKEN:
		case ResponseStatus::IP_BLOCKED:
		{
			LOGI << "key failed due to user error"
				<< " with response status "
				<< std::to_string(int(status));
			_owner.activatingKeyFailed(status);
		}
		break;
		case ResponseStatus::USERNAME_TAKEN:
		case ResponseStatus::EMAIL_TAKEN:
		case ResponseStatus::CONNECTION_FAILED:
		case ResponseStatus::ACCOUNT_LOCKED:
		case ResponseStatus::ACCOUNT_DISABLED:
		case ResponseStatus::KEY_REQUIRED:
		case ResponseStatus::EMAIL_UNVERIFIED:
		case ResponseStatus::USERNAME_REQUIRED_NOUSER:
		case ResponseStatus::USERNAME_REQUIRED_INVALID:
		case ResponseStatus::USERNAME_REQUIRED_TAKEN:
		case ResponseStatus::METHOD_INVALID:
		case ResponseStatus::REQUEST_MALFORMED:
		case ResponseStatus::RESPONSE_MALFORMED:
		case ResponseStatus::DATABASE_ERROR:
		{
			LOGE << "key failed with unexpected response status "
				<< std::to_string((int) status);
			_owner.activatingKeyFailed(status);
		}
		break;
		case ResponseStatus::UNKNOWN_ERROR:
		{
			LOGE << "key failed due to unknown error";
			_owner.activatingKeyFailed(status);
		}
		break;
	}
}

void Client::checkReset()
{
	if (!_futureReset.valid()) return;
	bool ready = (_futureReset.wait_for(std::chrono::seconds(0))
		== std::future_status::ready);
	if (!ready) return;
	ResponseStatus status = parsePasswordResetResponse(_futureReset.get());
	switch (status)
	{
		case ResponseStatus::SUCCESS:
		{
			LOGI << "Successfully reset password";
			_owner.resetPassword();
		}
		break;
		case ResponseStatus::CREDS_INVALID:
		{
			LOGI << "Password reset failed due to user error"
				<< " with response status "
				<< std::to_string(int(status));
			_owner.resetPasswordFailed(status);
		}
		break;
		case ResponseStatus::USERNAME_TAKEN:
		case ResponseStatus::EMAIL_TAKEN:
		case ResponseStatus::CONNECTION_FAILED:
		case ResponseStatus::ACCOUNT_LOCKED:
		case ResponseStatus::ACCOUNT_DISABLED:
		case ResponseStatus::KEY_TAKEN:
		case ResponseStatus::IP_BLOCKED:
		case ResponseStatus::KEY_REQUIRED:
		case ResponseStatus::EMAIL_UNVERIFIED:
		case ResponseStatus::USERNAME_REQUIRED_NOUSER:
		case ResponseStatus::USERNAME_REQUIRED_INVALID:
		case ResponseStatus::USERNAME_REQUIRED_TAKEN:
		case ResponseStatus::METHOD_INVALID:
		case ResponseStatus::REQUEST_MALFORMED:
		case ResponseStatus::RESPONSE_MALFORMED:
		case ResponseStatus::DATABASE_ERROR:
		{
			LOGE << "Password reset failed with unexpected response status "
				<< std::to_string((int) status);
			_owner.resetPasswordFailed(status);
		}
		break;
		case ResponseStatus::UNKNOWN_ERROR:
		{
			LOGE << "Password reset failed due to unknown error";
			_owner.resetPasswordFailed(status);
		}
		break;
	}
}

void Client::checkBug()
{
	if (!_futureBug.valid()) return;
	bool ready = (_futureBug.wait_for(std::chrono::seconds(0))
		== std::future_status::ready);
	if (!ready) return;
	ResponseStatus status = parseBugReportResponse(_futureBug.get());

	switch (status)
	{
		case ResponseStatus::SUCCESS:
		{
			LOGI << "Successfully submitted feedback";
			_owner.sentFeedback("");
		}
		break;
		case ResponseStatus::CREDS_INVALID:
		case ResponseStatus::USERNAME_TAKEN:
		case ResponseStatus::EMAIL_TAKEN:
		case ResponseStatus::CONNECTION_FAILED:
		case ResponseStatus::ACCOUNT_LOCKED:
		case ResponseStatus::ACCOUNT_DISABLED:
		case ResponseStatus::KEY_TAKEN:
		case ResponseStatus::IP_BLOCKED:
		case ResponseStatus::KEY_REQUIRED:
		case ResponseStatus::EMAIL_UNVERIFIED:
		case ResponseStatus::USERNAME_REQUIRED_NOUSER:
		case ResponseStatus::USERNAME_REQUIRED_INVALID:
		case ResponseStatus::USERNAME_REQUIRED_TAKEN:
		case ResponseStatus::METHOD_INVALID:
		case ResponseStatus::REQUEST_MALFORMED:
		case ResponseStatus::RESPONSE_MALFORMED:
		case ResponseStatus::DATABASE_ERROR:
		{
			LOGE << "Submitting feedback failed"
				<< " with unexpected response status "
				<< std::to_string(int(status));
			_owner.feedbackFailed(status);
		}
		break;
		case ResponseStatus::UNKNOWN_ERROR:
		{
			LOGE << "Submitting feedback failed due to unknown error";
			_owner.feedbackFailed(status);
		}
		break;
	}
}

void Client::checkLeaderboard()
{
	if (!_futureLeaderboard.valid()) return;
	bool ready = (_futureLeaderboard.wait_for(std::chrono::seconds(0))
		== std::future_status::ready);
	if (!ready) return;

	Response response = _futureLeaderboard.get();
	if (response.errorcode)
	{
		return;
	}

	LOGI << "Got a response from leaderboard server:"
		<< " [" << response.statuscode << "] "
		<< response.body;
	if (response.statuscode != 200)
	{
		LOGE << "Requesting leaderboard failed: invalid status code";
		return;
	}

	Json::Reader reader;
	Json::Value json;
	if (reader.parse(response.body, json) && json.isObject()
		&& json["rankings"].isArray())
	{
		std::vector<Ranking> rankings = parseRankings(json["rankings"]);
		if (rankings.empty())
		{
			LOGE << "Received malformed rankings from leaderboard server";
			return;
		}

		LOGI << "Received rankings from leaderboard";
		_owner.displayRankings(rankings);
	}
	else
	{
		LOGE << "Received malformed response from leaderboard server";
		return;
	}
}

void Client::checkMementoAutoPatch()
{
	if (!_settings.memento.defined()) return;

	std::string reference;
	{
		std::string memento = _settings.memento.value();
		Json::Reader reader;
		Json::Value json;
		if (reader.parse(memento, json) && json.isObject())
		{
			if (json["reference"].isString())
			{
				reference = json["reference"].asString();
			}
			else
			{
				LOGW << "Memento without reference, ignored";
				DEBUG_ASSERT(false);
				return;
			}
		}
		else
		{
			LOGE << "Invalid memento";
			DEBUG_ASSERT(false);
			return;
		}
	}

	LOGI << "Accessing memento reference: " << reference;
	std::string url = WEBSITE_ORIGIN + reference;
	_futureAutoPatch = _curl->get(url);
}

void Client::tryLinkAccounts(const std::string& discordUserId)
{
	DEBUG_ASSERT(!discordUserId.empty());
	if (discordUserId.empty()) return;

#if DISCORD_GUEST_ENABLED
	_discordUserId = discordUserId;
#endif

	if (!_socket) return;
	if (_account.username().empty()) return;

	Json::Value metadata = Json::objectValue;
	metadata["discord_user_id"] = discordUserId;
	send(Message::link_accounts(metadata));
}

void Client::requestSteamAccountMergeToken(const std::string& email,
	const std::string& password)
{
	if (!_accessedPortal)
	{
		LOGI << "Cannot request merge token; portal not accessed.";
		return;
	}
	else if (!_isSteamEnabled)
	{
		LOGI << "Cannot request merge token; steam integration not enabled.";
		return;
	}

	_steamAccountMergeToken = "";

	if (_steamId == 0)
	{
		LOGI << "Cannot request merge token; no Steam ID.";
		return;
	}

	LOGI << "Requesting merge token...";

	Json::Value payload(Json::objectValue);
	payload["steam_id_as_string"] = std::to_string(_steamId);
	payload["email"] = email;
	payload["p"] = Checksum::fromData(password).toHexString();

	constexpr const char* URL = WEBSITE_ORIGIN "/api/v1/merge_steam_account";
	_futureSteamAccountMergeToken = _curl->post(URL, payload);
}

void Client::cancelSteamAccountMergeToken()
{
	_steamAccountMergeToken = "";
	_futureSteamAccountMergeToken = std::future<Response>();
}

void Client::checkSteamAccountMergeToken()
{
	if (!_isSteamEnabled) return;
	if (!_futureSteamAccountMergeToken.valid()) return;
	bool ready = (_futureSteamAccountMergeToken.wait_for(
			std::chrono::seconds(0))
		== std::future_status::ready);
	if (!ready) return;

	Response response = _futureSteamAccountMergeToken.get();
	if (response.errorcode)
	{
		_owner.steamAccountMergeTokenResult(
			ResponseStatus::CONNECTION_FAILED);
		return;
	}

	LOGI << "Got a response from token server:"
		<< " [" << response.statuscode << "] "
		<< response.body;
	if (response.statuscode != 200)
	{
		LOGE << "Requesting token failed: invalid status code";
		if (response.statuscode >= 400 && response.statuscode <= 499)
		{
			_owner.steamAccountMergeTokenResult(
				ResponseStatus::REQUEST_MALFORMED);
		}
		else if (response.statuscode >= 500 && response.statuscode <= 599)
		{
			_owner.steamAccountMergeTokenResult(
				ResponseStatus::DATABASE_ERROR);
		}
		else
		{
			_owner.steamAccountMergeTokenResult(
				ResponseStatus::UNKNOWN_ERROR);
		}
		return;
	}

	Json::Reader reader;
	Json::Value json;
	if (reader.parse(response.body, json) && json.isObject()
		&& json["status"].isInt())
	{
		ResponseStatus status = fix(ResponseStatus(json["status"].asInt()));
		if (status == ResponseStatus::SUCCESS)
		{
			if (json["merge_token"].isString())
			{
				LOGI << "Received token from server";
				_steamAccountMergeToken = json["merge_token"].asString();
				_owner.steamAccountMergeTokenResult(status);
				return;
			}
			else
			{
				LOGE << "Login server reports success but data missing";
				_owner.steamAccountMergeTokenResult(
					ResponseStatus::RESPONSE_MALFORMED);
				return;
			}
		}
		else
		{
			LOGE << "Login server reports status code " << int(status);
			_owner.steamAccountMergeTokenResult(status);
			return;
		}
	}
	else
	{
		LOGE << "Received malformed response from token server";
		_owner.steamAccountMergeTokenResult(
			ResponseStatus::RESPONSE_MALFORMED);
		return;
	}
}

int Client::safelyReceive(char* buffer, uint32_t length)
{
	uint32_t receivedTotal = 0;
	while (receivedTotal < length)
	{
		int received = SDLNet_TCP_Recv(_socket, buffer + receivedTotal,
			length - receivedTotal);
		if (received <= 0)
		{
			LOGW << "Received too few bytes,"
				" expected " << length << ", received " << receivedTotal;
			return received;
		}
		receivedTotal += received;
	}
	return receivedTotal;
}

void Client::checkForNewMessages()
{
	if (!_socket) return;
	SDLNet_SocketSet set = SDLNet_AllocSocketSet(1);
	if (!set || SDLNet_TCP_AddSocket(set, _socket) < 0)
	{
		LOGW << "Not enough memory to allocate socket set or add socket";
		disconnect();
		return;
	}

	int active = 0;
	while (_socket && (active = SDLNet_CheckSockets(set, 0)) > 0)
	{
		if (!SDLNet_SocketReady(_socket)) return disconnect();

		// Get the network order (big-endian) byte representation of the length.
		uint8_t length_be_bytes[4] = {0, 0, 0, 0};
		int received = safelyReceive((char*) length_be_bytes, 4);
		if (received <= 0)
		{
			LOGW << "Error while safely receiving length,"
				" last received was " << received;
			disconnect();
			_resetting = true;
			return;
		}

		uint32_t length = (uint32_t(length_be_bytes[0]) << 24)
			| (uint32_t(length_be_bytes[1]) << 16)
			| (uint32_t(length_be_bytes[2]) << 8)
			| (uint32_t(length_be_bytes[3]));
		if (length >= MESSAGE_SIZE_LIMIT)
		{
			LOGW << "Receiving very large message of length " << length;
			LOGW << "Refusing to receive message of length " << length;
			disconnect();
			_resetting = true;
			return;
		}
		else if (length >= MESSAGE_SIZE_WARNING_LIMIT)
		{
			LOGW << "Receiving very large message of length " << length;
		}
		else if (length == 0)
		{
			LOGD << "Received empty message";

			// An empty message (i.e. without a body) is a pulse message.
			// Reset the last receive time so we know we are still connected.
			_lastreceivetime = (double) SDL_GetTicks() * 0.001;

			continue /*onto the next message*/;
		}

		LOGD << "Receiving message of length " << length;

		std::vector<char> buffer;
		buffer.resize(length);
		received = safelyReceive(buffer.data(), length);
		if (received <= 0)
		{
			LOGW << "Error while safely receiving message,"
				" last received was " << received;
			disconnect();
			_resetting = true;
			return;
		}

		// Reset the last receive time so we know we are still connected.
		_lastreceivetime = (double) SDL_GetTicks() * 0.001;

#if FEMTOZIP_ENABLED
		if (length > 0 && buffer[0] == '=')
		{
			{
				std::stringstream hash;
				hash << "=";
				hash << std::hex << std::setfill('0');
				for (size_t i = 1; i < length; i++)
				{
					hash << " " << std::setw(2)
						<< (unsigned int) (uint8_t) buffer[i];
				}
				LOGD << "Compressed message: \'" << hash.str() << "\'";
			}

			auto model = _compressionModel;
			if (!model)
			{
				std::stringstream hash;
				hash << "=";
				hash << std::hex << std::setfill('0');
				for (size_t i = 1; i < length; i++)
				{
					hash << " " << std::setw(2)
						<< (unsigned int) (uint8_t) buffer[i];
				}
				LOGE << "Cannot decompress message"
					" \'" << hash.str() << "\'"
					" without model";
				DEBUG_ASSERT(false);
				continue /*onto the next message*/;
			}

			std::ostringstream strm;
			model->decompress(buffer.data() + 1, length - 1, strm);
			std::string str = strm.str();
			buffer = std::vector<char>(str.begin(), str.end());
			length = str.length();
			LOGD << "Uncompressed message length " << (length);
		}
#endif

		LOGD << "Received message:"
			" \'" << std::string(buffer.data(), length) << "\'";

		receiveMessage(buffer.data(), length);
	}
	SDLNet_FreeSocketSet(set);
	if (active < 0)
	{
		LOGW << "Error while checking for new messages,"
			" " << active << " active sockets";
		disconnect();
		_resetting = true;
	}
}

void Client::receiveMessage(const char* data, uint32_t length)
{
	try
	{
		ParsedMessage message = Message::parse(data, length);
		if (_firewall) _firewall->screenIncoming(message);
		handleMessage(message);
	}
	catch (const ParseError& error)
	{
		LOGE << "Error while parsing message: "
			<< "\'" << std::string(data, length) << "\'"
			<< ", error "
			<< error.what();
		RETHROW_IF_DEV();
	}
	catch (const Json::Exception& error)
	{
		LOGE << "Error while parsing message: "
			<< "\'" << std::string(data, length) << "\'"
			<< ", error "
			<< error.what();
		RETHROW_IF_DEV();
	}
}

void Client::invalidateSession(uint32_t accountId,
	const std::string& sessionToken)
{
	if (!_accessedPortal)
	{
		LOGI << "Cannot invalidate session; portal not accessed.";
		return;
	}
	else if (_isSteamEnabled)
	{
		LOGI << "Cannot invalidate session; steam enabled.";
		return;
	}

	LOGI << "Invalidating session";

	Json::Value json(Json::objectValue);
	json["id"] = accountId;
	json["token"] = sessionToken;

	constexpr const char* URL = WEBSITE_ORIGIN "/invalidate_session.php";
	_curl->post(URL, std::move(json));
}

bool Client::autoresetActive()
{
	if (_autoresettime < 0) return true;

	double now = (double) SDL_GetTicks() * 0.001;
	return (now > _autoresettime);
}

bool Client::connectToPortal()
{
	if (_settings.server.defined() && !_autoRetryPortal)
	{
		// If the `server` setting is defined, we are in "LAN" mode; we use
		// the server and port as specified and disable all login activities.
		// This is because we do not want to send a real session token to an
		// unknown server and allow that server to hijack the session.
		LOGW << "Refusing to connect to portal in LAN mode";
		return false;
	}

	// The portal url. This url should always be kept alive
	// and backwards compatibility with older game clients, in particular
	// older api versions, should be preserved. It cannot be overridden.
	constexpr const char* URL = WEBSITE_ORIGIN "/api/v1/portal";

	LOGI << "Connecting to server portal...";
	_futurePortal = _curl->get(URL);
	_owner.accessingPortal();
	return true;
}

void Client::handlePortalResponse(Response response)
{
	if (response.errorcode)
	{
		LOGE << "Accessing portal failed due to connection failure";
		_owner.accessingPortalFailed(PortalStatus::CONNECTION_FAILED);
		return;
	}

	LOGI << "Got a response from portal server:"
		<< " [" << response.statuscode << "] "
		<< response.body;
	if (response.statuscode == 503)
	{
		LOGE << "Accessed portal but servers unavailable";
		_owner.accessingPortalFailed(PortalStatus::MAINTENANCE);
		return;
	}
	else if (response.statuscode != 200)
	{
		LOGE << "Accessed portal but received invalid status code";
		_owner.accessingPortalFailed(PortalStatus::CONNECTION_FAILED);
		return;
	}

	Json::Reader reader;
	Json::Value json;
	if (!reader.parse(response.body, json) || !json.isObject())
	{
		LOGE << "Accessed portal but received malformed response";
		_owner.accessingPortalFailed(PortalStatus::RESPONSE_MALFORMED);
		return;
	}

	// Is there a patch that we can download?
	if (!json["patch"].isNull())
	{
		handlePatchInfo(json["patch"]);

		// We always want to download the latest patch before connecting.
		return;
	}

	if (!json["stamp"].isNull())
	{
		handleStamp(json["stamp"]);
	}

	if (json["host"].isString()
		&& json["port"].isUInt()
		&& json["port"].asUInt() <= 0xFFFF)
	{
		handlePortalInfo(json["host"].asString(),
			(uint16_t) json["port"].asUInt());
	}
	else
	{
		LOGE << "Accessed portal but received malformed response";
		_owner.accessingPortalFailed(PortalStatus::RESPONSE_MALFORMED);
	}
}

void Client::handlePatchInfo(const Json::Value& json)
{
	Version patchversion;
	if (!json.isObject()
		|| !json["version"].isString()
		|| !patchversion.tryBecomeParsed(json["version"].asString())
		|| !json["manifest-query"].isString())
	{
		LOGE << "Accessed portal but response contains malformed patch info";
		_owner.accessingPortalFailed(PortalStatus::RESPONSE_MALFORMED);
		return;
	}

	Version finalversion = patchversion;
	if (json["final-version"].isString())
	{
		// Ignore parsing errors because this is cosmetic only.
		finalversion.tryBecomeParsed(json["final-version"].asString());
	}

	if (_settings.patchmode.value(PatchMode::NONE) != PatchMode::SERVER)
	{
		LOGI << "Patching disabled; user should update externally";
		_owner.versionMismatch(finalversion);
		return;
	}

	_patchVersion.reset(new Version(patchversion));
	_manifestQuery = json["manifest-query"].asString();

	LOGI << "Patch available: " << patchversion.toString()
		<< " (" << finalversion.toString() << ")";
	_owner.patchAvailable(finalversion);

	checkMementoAutoPatch();
}

void Client::handleStamp(const Json::Value& json)
{
	if (!json.isObject())
	{
		LOGE << "Ignoring invalid stamp";
		return;
	}

	std::string image;
	if (json["image"].isString())
	{
		image = json["image"].asString();
	}
	std::string tooltip;
	if (json["tooltip"].isString())
	{
		tooltip = json["tooltip"].asString();
	}
	std::string url;
	if (json["url"].isString())
	{
		url = json["url"].asString();
	}

	_owner.displayStamp(image, tooltip, url);
}

void Client::handlePatchManifestResponse(Response response)
{
	if (response.errorcode)
	{
		LOGE << "Downloading patch manifest failed: connection failure";
		_owner.patchFailed();
		return;
	}

	LOGI << "Got a response from patch server:"
		<< " [" << response.statuscode << "] "
		<< response.body;
	if (response.statuscode != 200)
	{
		LOGE << "Downloading patch manifest failed: invalid status code";
		_owner.patchFailed();
		return;
	}

	Json::Reader reader;
	Json::Value json;
	if (!reader.parse(response.body, json) || !json.isObject()
		|| !json["files"].isArray())
	{
		LOGE << "Dowloading patch manifest failed: received malformed response";
		_owner.patchFailed();
		return;
	}

	_downloads.clear();
	_percentages.clear();
	_futureDownloads.clear();
	size_t nfiles = json["files"].size();
	_downloads.reserve(nfiles);
	_percentages.reserve(nfiles);
	_futureDownloads.reserve(nfiles);

	for (const Json::Value& item : json["files"])
	{
		try
		{
			_downloads.push_back(Download::parse(item));
		}
		catch (Json::Exception& error)
		{
			LOGE << "Dowloading patch manifest failed: invalid manifest";
			_owner.patchFailed();
			return;
		}

		Download& download = _downloads.back();

		System::touchFile(download.sourcefilename);

		if (!download.sourcedata.empty())
		{
			std::promise<Response> promise;
			_futureDownloads.push_back(promise.get_future());
			_percentages.push_back(std::make_shared<std::atomic<float>>(100));

			std::ofstream file(download.sourcefilename,
				std::ios::binary | std::ios::trunc);
			if (file)
			{
				LOGI << "Writing download sourcedata to file"
					<< " '" << download.sourcefilename << "'...";
				file << download.sourcedata;
				promise.set_value({CURLE_OK, 200, "(from sourcedata)"});
			}
			else
			{
				LOGE << "Failed to write download sourcedata to file"
					<< " '" << download.sourcefilename << "'";
				promise.set_value({CURLE_FAILED_INIT, -1, "(from sourcedata)"});
			}
		}
		else if (item["download"].isString())
		{
			LOGI << "Downloading '" << download.targetfilename << "'...";
			std::string url = WEBSITE_ORIGIN + item["download"].asString();
			_percentages.push_back(std::make_shared<std::atomic<float>>(0));
			_futureDownloads.push_back(_curl->download(url,
				download.sourcefilename, _percentages.back()));
		}
		else
		{
			LOGE << "Dowloading patch manifest failed: no download link";
			_owner.patchFailed();
			return;
		}
	}

	if (!json["memento"].isNull())
	{
		_mementoJson.reset(new Json::Value(json["memento"]));
	}

	LOGI << "There are " << _downloads.size() << " downloads in progress...";
	_downloadsInProgress = _downloads.size();
}

void Client::handleAutoPatchResponse(Response response)
{
	if (response.errorcode)
	{
		LOGE << "Accessing autopatch ref failed: connection failure";
		return;
	}

	LOGI << "Got a response from autopatch reference:"
		<< " [" << response.statuscode << "] "
		<< response.body;
	if (response.statuscode != 200)
	{
		LOGE << "Accessing autopatch ref failed: invalid status code";
		return;
	}

	Json::Reader reader;
	Json::Value json;
	Version referenceversion;
	if (!reader.parse(response.body, json) || !json.isObject()
		|| !json["version"].isString()
		|| !referenceversion.tryBecomeParsed(json["version"].asString()))
	{
		LOGE << "Accessing autopatch ref failed: received malformed response";
		return;
	}

	Version patchversion;
	if (_patchVersion)
	{
		patchversion = *_patchVersion;
	}
	else
	{
		LOGE << "Cannot handle autopatch ref without patchversion";
		return;
	}

	if (patchversion != referenceversion)
	{
		LOGE << "Accessed autopatch ref but version mismatch";
		return;
	}

	_autoPatchActivated = true;
	_owner.patchContinuing(patchversion);
	startPatch();
}

void Client::connect(const std::string& server, uint16_t port)
{
	disconnect();
	_connectstarttime = (double) SDL_GetTicks() * 0.001;
	if (_resetting)
	{
		_autoresettime = _connectstarttime + 60.0f;
		_resetting = false;
	}
	else
	{
		_autoresettime = -1.0f;
	}

	if (_connectThread.joinable()) _connectThread.detach();
	std::promise<TCPsocket> promiseSocket;
	_futureSocket = promiseSocket.get_future();

	try
	{
		_connectThread = std::thread(&Client::runConnectThread,
			server, port,
			std::move(promiseSocket));
		_owner.connecting();
		_owner.message(
			_("Connecting..."));
	}
	catch (const std::system_error& e)
	{
		LOGW << "Could not start connect thread; system_error code " << e.code()
			<< ": " << e.what();
		_owner.systemFailure();
		_owner.message(
			_("Failed to connect to server."));
		disconnect(/*sendMessage=*/false);
	}
}

void Client::startPatch()
{
	if (_manifestQuery.empty())
	{
		LOGI << "Cannot start patch; empty manifest query.";
		return;
	}

	LOGI << "Downloading patch manifest...";

	constexpr const char* BASE_URL = WEBSITE_ORIGIN "/api/v1/manifest";
	std::string url = std::string(BASE_URL) + _manifestQuery;
	_futurePatchManifest = _curl->get(url);
	_owner.patchDownloading();
}

void Client::runConnectThread(std::string server, uint16_t port,
	std::promise<TCPsocket> promise)
{
	try
	{
		LOGI << "Connecting to server (" << server << ":" << port << ")...";
		// Make a client socket
		IPaddress ip;
		SDL_ClearError();
		if (SDLNet_ResolveHost(&ip, server.c_str(), port))
		{
			LOGW << "Connecting to server (" << server << ":" << port
				<< ") failed during resolution: " << SDLNet_GetError();
			promise.set_value(nullptr);
			return;
		}
		SDL_ClearError();
		TCPsocket socket = SDLNet_TCP_Open(&ip);
		if (!socket)
		{
			LOGW << "Connecting to server (" << server << ":" << port
				<< ") failed: " << SDLNet_GetError();
			promise.set_value(nullptr);
			return;
		}
		promise.set_value(socket);
	}
	catch (const std::exception& e)
	{
		LOGE << "Error while running connect thread: " << e.what();
		RETHROW_IF_DEV();
		promise.set_value(nullptr);
		return;
	}
	catch (...)
	{
		LOGE << "Unknown error while running connect thread";
		RETHROW_IF_DEV();
		promise.set_value(nullptr);
		return;
	}
}

void Client::connect()
{
	if (!_settings.server.defined())
	{
		// Determine the server address and port by asking the official portal.
		connectToPortal();
		return;
	}
	else if (!_settings.port.defined())
	{
		LOGF << "No port defined.";
		DEBUG_ASSERT(false && "No port defined.");
		return;
	}

	connect(_settings.server.value(), _settings.port.value());
}

void Client::disconnect(bool sendMessage)
{
	logout(false);

	if (_connectThread.joinable()) _connectThread.detach();
	if (_futureSocket.valid()) _futureSocket = std::future<TCPsocket>();

	// TODO do we really want to discard these?
	if (_futurePortal.valid()) _futurePortal = std::future<Response>();
	if (_futureAutoPatch.valid()) _futureAutoPatch = std::future<Response>();
	if (_futurePatchManifest.valid()) _futurePatchManifest = std::future<Response>();
	if (_downloadsInProgress > 0)
	{
		_downloadsInProgress = 0;
		_downloads.clear();
		_percentages.clear();
		_futureDownloads.clear();
	}
	if (_futureFzmodelRequest.valid()) _futureFzmodelRequest = std::future<Response>();
	if (_futureFzmodelDownload.valid()) _futureFzmodelDownload = std::future<Response>();
	if (_futureRegistration.valid()) _futureRegistration = std::future<Response>();
	if (_futureLogin.valid()) _futureLogin = std::future<Response>();
	if (_futureReset.valid()) _futureReset = std::future<Response>();
	// TODO because _futureBug is missing here; on purpose?
	// _futureLeaderboard is missing here on purpose.
	// _futureSteamAccountMergeToken is missing here on purpose.

	_lastreceivetime = -1;
	_lastknownping = -1;
	_lastsendtime = -1;
	_pingsendtime = -1;
	_connectstarttime = -1.0f;

	_compressionModel.reset();

	if (!_socket) return;

	if (sendMessage)
	{
		send(Message::quit());
		waitForClosure();
	}
	SDLNet_TCP_Close(_socket);
	_socket = nullptr;
	_lobbyID = "";
	_disconnected = true;
	_initialized = false;
	LOGI << "Disconnected from server";

	_owner.disconnected();
	if (sendMessage)
	{
		_owner.message(
			_("You have been disconnected from the server."));
	}
}

void Client::waitForClosure()
{
	constexpr int GARBAGESIZE = 1024;
	char garbage[GARBAGESIZE];
	int received;
	do
	{
		received = SDLNet_TCP_Recv(_socket, garbage, GARBAGESIZE);
		if (received > 0)
		{
			LOGD << "Received " << received << " bytes of garbage";
		}
	}
	while (received > 0);

	if (received < 0 || received > GARBAGESIZE)
	{
		LOGE << "Received " << received << " bytes of garbage";
	}
}

void Client::setUsernameForSteam(const std::string& username)
{
	_usernameForSteam = username;
}

void Client::join()
{
	if (!_socket) return;

	std::string token;
	std::string id;
	Json::Value metadata = Json::objectValue;
	if (!_accessedPortal)
	{
		uint32_t x;
		try
		{
			if (_settings.logname.value().find("alt") == 0)
			{
				x = std::stoi(_settings.logname.value().substr(3));
			}
			else x = (rand() % 999999) + 1;
		}
		catch (const std::logic_error&)
		{
			x = (rand() % 999999) + 1;
		}
		token = std::to_string(x);
		id = std::to_string(x);

		LOGI << "Joining locally with fake credentials: " << id;
	}
#if STEAM_ENABLED
	else if (_isSteamEnabled)
	{
		token = _steamSessionTicket;
		id = "!steam";
		if (!_steamAccountMergeToken.empty())
		{
			metadata["merge_token"] = _steamAccountMergeToken;
			_steamAccountMergeToken = "";
		}
		else if (!_usernameForSteam.empty())
		{
			metadata["desired_username"] = _usernameForSteam;
			_usernameForSteam = "";
		}
	}
#endif
#if DISCORD_GUEST_ENABLED
	else if (_settings.discord.value() && _settings.allowDiscordLogin.value()
		&& _account.id() == 0 && !_discordUsername.empty())
	{
		if (!_discordUserId.empty())
		{
			metadata["discord_id"] = _discordUserId;
		}
		token = "%""discord_indigo2020";
		id = _discordUsername + "#" + _discordDiscriminator;

		LOGI << "Joining as a guest using Discord id: " << id;
	}
#endif
	else
	{
		token = _account.sessionToken();
		id = std::to_string(_account.id());
	}

	send(Message::join_server(token, id, metadata));
	_owner.message(
		_("Authenticating..."));
}

void Client::leave()
{
	if (!_socket) return;
	_initialized = false;
	send(Message::leave_server());
}

void Client::registration(const std::string& username, const std::string& email,
	const std::string& password)
{
	if (!_accessedPortal)
	{
		LOGI << "Cannot register; portal not accessed.";
		return;
	}
	else if (_isSteamEnabled)
	{
		LOGI << "Cannot register; steam enabled.";
		return;
	}

	if (!_socket) return;
	logout(true);

	LOGI << "Registering...";

	Json::Value payload(Json::objectValue);
	payload["username"] = username;
	payload["email"] = email;
	payload["p"] = Checksum::fromData(password).toHexString();

	constexpr const char* URL = WEBSITE_ORIGIN "/register.php";
	_futureRegistration = _curl->post(URL, payload);
	_owner.registering();
}

ResponseStatus Client::parseRegistrationResponse(Response response)
{
	if (response.errorcode)
	{
		return ResponseStatus::CONNECTION_FAILED;
	}

	LOGI << "Got a response from registration server:"
		<< " [" << response.statuscode << "] "
		<< response.body;

	Json::Reader reader;
	Json::Value json;
	if (reader.parse(response.body, json) && json.isObject()
		&& json["status"].isInt())
	{
		return fix(ResponseStatus(json["status"].asInt()));
	}
	else
	{
		LOGE << "Received malformed response from registration server";
		return ResponseStatus::RESPONSE_MALFORMED;
	}
}

void Client::updateEmailPref(const std::string& email,
	const EmailPreference& preference)
{
	if (!_accessedPortal)
	{
		LOGI << "Cannot update email preferences; portal not accessed.";
		return;
	}
	else if (_isSteamEnabled)
	{
		LOGI << "Cannot update email preferences; steam enabled.";
		return;
	}

	LOGI << "Updating email preferences";

	Json::Value json(Json::objectValue);
	json["email"] = email;

	if (preference.mailinglist == EmailPreference::YES)
	{
		json["mailinglist"] = true;
		json["future_games"] = true;
	}
	else if (preference.mailinglist == EmailPreference::NO)
	{
		json["mailinglist"] = false;
		json["future_games"] = false;
	}
	else
	{
		// Do not specify.
	}

	constexpr const char* URL = WEBSITE_ORIGIN "/update_email_pref.php";
	_curl->post(URL, std::move(json));
}

void Client::login(const std::string& email, const std::string& password,
	bool remember)
{
	if (!_accessedPortal)
	{
		LOGI << "Cannot login; portal not accessed.";
		return;
	}
	else if (_isSteamEnabled)
	{
		LOGI << "Cannot login; steam enabled.";
		return;
	}

	if (!_socket) return;
	logout(true);

	LOGI << "Logging in...";

	Json::Value payload(Json::objectValue);
	payload["email"] = email;
	payload["p"] = Checksum::fromData(password).toHexString();
	payload["remember"] = remember;

	constexpr const char* URL = WEBSITE_ORIGIN "/process_login.php";
	_futureLogin = _curl->post(URL, payload);
	_sessionStatus = ResponseStatus::SUCCESS;
	_owner.loggingIn();
}

Client::LoginData Client::parseLoginResponse(Response response)
{
	if (response.errorcode)
	{
		return {ResponseStatus::CONNECTION_FAILED, 0, "", false};
	}

	LOGI << "Got a response from login server:"
		<< " [" << response.statuscode << "] "
		<< response.body;

	Json::Reader reader;
	Json::Value json;
	if (reader.parse(response.body, json) && json.isObject()
		&& json["status"].isInt())
	{
		ResponseStatus status = fix(ResponseStatus(json["status"].asInt()));
		if (status == ResponseStatus::SUCCESS)
		{
			if (json["id"].isUInt()
				&& json["token"].isString())
			{
				LOGI << "Successfully logged in";
				return {ResponseStatus::SUCCESS,
					json["id"].asUInt(),
					json["token"].asString(),
					json["remember"].asBool()};
			}
			else
			{
				LOGE << "Login server reports success but data missing";
				return {ResponseStatus::RESPONSE_MALFORMED, 0, "", false};
			}
		}
		else
		{
			LOGE << "Login server reports status code " << int(status);
			return {status, 0, "", false};
		}
	}
	else
	{
		LOGE << "Received malformed response from login server";
		return {ResponseStatus::RESPONSE_MALFORMED, 0, "", false};
	}
}

void Client::logout(bool reset)
{
	if (!_accessedPortal)
	{
		if (reset)
		{
			LOGI << "Cannot logout; portal not accessed.";
		}
		return;
	}
	else if (_isSteamEnabled)
	{
		if (reset)
		{
			LOGI << "Cannot logout; steam enabled.";
		}
		return;
	}

	if (_futureLogin.valid()) _futureLogin = std::future<Response>();
	if (!_account.sessionToken().empty()
		&& (!_account.rememberSession() || reset))
	{
		LOGI << "Session should either be forgotten"
				" or user manually logged out; invalidate it";
		invalidateSession(_account.id(), _account.sessionToken());
	}
	if (reset)
	{
		_account.reset();
		LOGI << "Logged out, forgot (possible) session";
	}
	else
	{
		_account.setUsername("");
		LOGI << "Logged out, remembered (possible) session";
	}
	_owner.loggedOut();
}

void Client::activateKey(const std::string& token)
{
	if (!_accessedPortal)
	{
		LOGI << "Cannot activate key; portal not accessed.";
		return;
	}
	else if (_isSteamEnabled)
	{
		LOGI << "Cannot activate key; steam enabled.";
		return;
	}

	if (_account.id() == 0)
	{
		LOGI << "Cannot activate key; no account id defined.";
		return;
	}

	LOGI << "Activating key...";

	Json::Value payload(Json::objectValue);
	payload["id"] = _account.id();
	payload["key"] = token;

	constexpr const char* URL = WEBSITE_ORIGIN "/activate_key.php";
	_futureKey = _curl->post(URL, payload);
	_owner.activatingKey();
}

ResponseStatus Client::parseKeyActivationResponse(Response response)
{
	if (response.errorcode)
	{
		return ResponseStatus::CONNECTION_FAILED;
	}

	LOGI << "Got a response from key activation server:"
		<< " [" << response.statuscode << "] "
		<< response.body;

	Json::Reader reader;
	Json::Value json;
	if (reader.parse(response.body, json) && json.isObject()
		&& json["status"].isInt())
	{
		return fix(ResponseStatus(json["status"].asInt()));
	}
	else
	{
		LOGE << "Received malformed response from key server";
		return ResponseStatus::RESPONSE_MALFORMED;
	}
}

void Client::forgotPassword(const std::string& email)
{
	if (!_accessedPortal)
	{
		LOGI << "Cannot request password reset; portal not accessed.";
		return;
	}
	else if (_isSteamEnabled)
	{
		LOGI << "Cannot request password reset; steam enabled.";
		return;
	}

	LOGI << "Requesting password reset";

	Json::Value json(Json::objectValue);
	json["email"] = email;

	constexpr const char* URL = WEBSITE_ORIGIN "/forgot_password.php";
	_curl->post(URL, std::move(json));
}

void Client::resetPassword(const std::string& email, const std::string& token,
	const std::string& password)
{
	if (!_accessedPortal)
	{
		LOGI << "Cannot reset password; portal not accessed.";
		return;
	}
	else if (_isSteamEnabled)
	{
		LOGI << "Cannot reset password; steam enabled.";
		return;
	}

	LOGI << "Resetting password...";

	Json::Value payload(Json::objectValue);
	payload["email"] = email;
	payload["token"] = token;
	payload["p"] = Checksum::fromData(password).toHexString();

	constexpr const char* URL = WEBSITE_ORIGIN "/reset_password.php";
	_futureReset = _curl->post(URL, payload);
	_owner.resettingPassword();
}

ResponseStatus Client::parsePasswordResetResponse(Response response)
{
	if (response.errorcode)
	{
		return ResponseStatus::CONNECTION_FAILED;
	}

	LOGI << "Got a response from password reset server:"
		<< " [" << response.statuscode << "] "
		<< response.body;

	Json::Reader reader;
	Json::Value json;
	if (reader.parse(response.body, json) && json.isObject()
		&& json["status"].isInt())
	{
		return fix(ResponseStatus(json["status"].asInt()));
	}
	else
	{
		LOGE << "Received malformed response from password reset server";
		return ResponseStatus::RESPONSE_MALFORMED;
	}
}

void Client::feedback(bool positive, const std::string& text, bool sendLogs)
{
	std::string truncText;
	if (text.length() > 120)
	{
		size_t space = text.find_last_of(" ", 117);
		if (space != std::string::npos && space >= 100)
		{
			truncText = text.substr(0, space) + "...";
		}
		else
		{
			truncText = text.substr(0, 117) + "...";
		}
	}
	else
	{
		truncText = text;
	}

	LOGI << "Sending stomt";

	_owner.sendStomt(positive, truncText);
	_owner.sendingFeedback();

	LOGI << "Submitting feedback";

	Json::Value json(Json::objectValue);
	json["positive"] = positive;
	json["text"] = text;
	json["debugversion"] = getVersionDebugString();
	if (_account.id() != 0)
	{
		json["id"] = _account.id();
	}
	if (!_account.username().empty())
	{
		json["username"] = _account.username();
	}
	if (_steamId != 0)
	{
		// TODO add persona name once we have UTF8 support on the website
		json["userdesc"] = "Steam user"
			" (" + std::to_string(_steamId) + ")";
	}

	constexpr const char* URL = WEBSITE_ORIGIN "/feedback.php";
	_curl->post(URL, std::move(json));

	if (sendLogs)
	{
		this->sendLogs();
	}
}

void Client::bugReport(const std::string& text, bool sendLogs)
{
	LOGI << "Submitting bugreport";

	Json::Value json(Json::objectValue);
	json["text"] = text;
	json["debugversion"] = getVersionDebugString();
	if (_account.id() != 0)
	{
		json["id"] = _account.id();
	}
	if (!_account.username().empty())
	{
		json["username"] = _account.username();
	}
	if (_steamId != 0)
	{
		// TODO add persona name once we have UTF8 support on the website
		json["userdesc"] = "Steam user"
			" (" + std::to_string(_steamId) + ")";
	}

	constexpr const char* URL = WEBSITE_ORIGIN "/feedback.php";
	_futureBug = _curl->post(URL, std::move(json));
	_owner.sendingFeedback();

	if (sendLogs)
	{
		this->sendLogs();
	}
}

ResponseStatus Client::parseBugReportResponse(Response response)
{
	if (response.errorcode)
	{
		return ResponseStatus::CONNECTION_FAILED;
	}

	LOGI << "Got a response from bugreport server:"
		<< " [" << response.statuscode << "] "
		<< response.body;

	Json::Reader reader;
	Json::Value json;
	if (reader.parse(response.body, json) && json.isObject()
		&& json["status"].isInt())
	{
		return fix(ResponseStatus(json["status"].asInt()));
	}
	else
	{
		LOGE << "Received malformed response from bugreport server";
		return ResponseStatus::RESPONSE_MALFORMED;
	}
}

void Client::requestLeaderboard()
{
	if (!_accessedPortal)
	{
		LOGI << "Cannot request leaderboard; portal not accessed.";
		return;
	}

	LOGI << "Requesting leaderboard...";

	constexpr const char* URL = WEBSITE_ORIGIN "/api/v1/leaderboard?amount=50";
	_futureLeaderboard = _curl->get(URL);
}

void Client::update()
{
	// Update all curl requests.
	_curl->update();

	// Check whether a query to the portal server completed.
	checkPortal();

	// Check whether a query to an autopatch reference has completed.
	checkAutoPatch();

	// Check whether a query for a patch manifest has completed.
	checkPatchManifest();

	// Check the progress of a patch.
	checkPatchProgress();

	// Check whether a query for a fzmodel request has completed.
	checkFzmodelRequest();

	// Check the progress of a fzmodel download.
	checkFzmodelDownload();

	// Check whether a merge token was created.
	checkSteamAccountMergeToken();

	// Check for connection with the server.
	// If there is no connection, there is nothing else to do.
	if (!checkConnection()) return;

	try
	{
		checkForNewMessages();
	}
	catch (const std::exception& e)
	{
		LOGW << "Error while checking for new messages in client: " << e.what();
		RETHROW_IF_DEV();
		disconnect();
		_resetting = true;
	}
	catch (...)
	{
		LOGW << "Unknown error while checking for new messages in client";
		RETHROW_IF_DEV();
		disconnect();
		_resetting = true;
	}

	// Check if the connection is still healthy.
	checkVitals();

	// Check whether a registration completed.
	checkRegistration();

	// Check whether we are logged in.
	checkLogin();

	// Check whether the user has activated a key.
	checkKey();

	// Check whether the user has reset their password.
	checkReset();

	// Check whether we submitted bugreport.
	checkBug();

	// Check whether we retrieved a leaderboard.
	checkLeaderboard();
}

bool Client::isPatchPrimed(const Version& version)
{
	// Avoid downgrade attacks.
	if (version == Version::undefined() || version <= Version::current())
	{
		LOGE << "Refusing to install non-upgrade version " << version;
		DEBUG_ASSERT(false);
		return false;
	}

#if SELF_PATCH_ENABLED
	// Verify that the downloads have been downloaded.
	for (const Download& download : _downloads)
	{
		if (!System::isFile(download.sourcefilename))
		{
			LOGE << "Cannot install patch: missing file"
				<< " '" << download.sourcefilename << "'";
			DEBUG_ASSERT(false);
			return false;
		}
	}

	// Verify the checksums.
	for (const Download& download : _downloads)
	{
		if (download.checksum.empty())
		{
			if (!download.sourcedata.empty())
			{
				// There is no checksum for data embedded in the manifest.
				continue;
			}

			LOGE << "Missing checksum for '" << download.sourcefilename << "'";
			DEBUG_ASSERT(false);
			return false;
		}

		auto checksum = Checksum::fromFile(download.sourcefilename);
		if (checksum != download.checksum)
		{
			LOGE << "Invalid checksum for '" << download.sourcefilename << "'";
			DEBUG_ASSERT(false);
			return false;
		}
	}

	// Write a memento, if any.
	if (_mementoJson)
	{
		std::ofstream mementofile(Patch::mementofilename(), std::ios::trunc);
		if (!mementofile.is_open())
		{
			LOGE << "Failed to open " << Patch::mementofilename();
			DEBUG_ASSERT(false);
			return false;
		}

		mementofile << Writer::write(*_mementoJson);
	}

	// Write the files to the primer.
	Patch::prime(std::move(_downloads));
	_percentages.clear();
	_futureDownloads.clear();

	return true;
#else
	LOGE << "Cannot prime patch: self patch not enabled.";
	return false;
#endif
}

bool Client::fulfilRequest(const Download& download)
{
	// Verify that the download has been downloaded.
	if (!System::isFile(download.sourcefilename))
	{
		LOGE << "Received illegal metadata from server";
		DEBUG_ASSERT(false);
		return false;
	}

	// Verify the checksum.
	{
		auto checksum = Checksum::fromFile(download.sourcefilename);
		if (checksum != download.checksum)
		{
			LOGE << "Invalid checksum for '" << download.sourcefilename << "'";
			DEBUG_ASSERT(false);
			return false;
		}
	}

	// Install the download.
	return Patch::install(download);
}

void Client::sendLogs()
{
	LOGI << "Sending logs";

	Json::Value json(Json::objectValue);
	if (_account.id() != 0)
	{
		json["id"] = _account.id();
	}
	if (!_account.username().empty())
	{
		json["username"] = _account.username();
	}
	if (_steamId != 0)
	{
		// TODO add persona name once we have UTF8 support on the website
		json["userdesc"] = "Steam user"
			" (" + std::to_string(_steamId) + ")";
	}

	// We can send up to 25 files (current limit in php.ini). We send up to
	// 10 error logs, up to 10 info logs and a settings file. If we want to
	// send a lot more, we need to change the max_file_uploads ini setting
	// again.
	std::vector<std::string> files;

	std::string logsfolder = LogInstaller::getLogsFolderWithSlash();

	{
		auto fname = logsfolder + "main.errors.log";
		if (System::isFile(fname))
		{
			std::string zipped = Compress::gzip(fname);
			if (!zipped.empty()) files.emplace_back(zipped);
		}
	}

	for (size_t i = 1; i < 10; i++)
	{
		auto fname = logsfolder + "main.errors." + std::to_string(i) + ".log";
		if (System::isFile(fname))
		{
			std::string zipped = Compress::gzip(fname);
			if (!zipped.empty()) files.emplace_back(zipped);
		}
	}

	{
		auto fname = logsfolder + "main.info.log";
		if (System::isFile(fname))
		{
			std::string zipped = Compress::gzip(fname);
			if (!zipped.empty()) files.emplace_back(zipped);
		}
	}

	for (size_t i = 1; i < 10; i++)
	{
		auto fname = logsfolder + "main.info." + std::to_string(i) + ".log";
		if (System::isFile(fname))
		{
			std::string zipped = Compress::gzip(fname);
			if (!zipped.empty()) files.emplace_back(zipped);
		}
	}

	auto settingsfilename = Settings::getPathFromFilename("settings.json");
	if (System::isFile(settingsfilename))
	{
		std::string zipped = Compress::gzip(settingsfilename);
		if (!zipped.empty()) files.emplace_back(zipped);
	}

	constexpr const char* URL = WEBSITE_ORIGIN "/upload_logs.php";
	_curl->post(URL, files, std::move(json));
}

std::string Client::getVersionDebugString() const
{
	std::stringstream strm;
	strm << "Version " << Version::current() << " ("
		<< ::platform();
	switch (_settings.patchmode.value(PatchMode::NONE))
	{
		case PatchMode::ITCHIO: strm << "; itch.io"; break;
		case PatchMode::GAMEJOLT: strm << "; GameJolt"; break;
		case PatchMode::STEAM: strm << "; Steam"; break;
		case PatchMode::SERVER_BUT_DISABLED_DUE_TO_STORAGE_ISSUES:
		case PatchMode::NONE: strm << "; nopatch"; break;
		case PatchMode::SERVER: break;
	}
	if (_settings.language.defined())
	{
		strm << "; " << _settings.language.value() << "";
	}
	strm << "; " << std::hex << std::setfill('0') << std::setw(8)
		<< Version::latest().asUint32();
	strm << "-";
#if STEAM_ENABLED
	strm << "S";
#endif
#if INTL_ENABLED
	strm << "I";
#endif
#if FEMTOZIP_ENABLED
	strm << "F";
#endif
#if LIBLOADER_ENABLED
	strm << "Lb";
#endif
#if DICTATOR_ENABLED
	strm << "D";
#endif
#if DISCORD_GUEST_ENABLED
	strm << "Dg";
#endif
#if COREDUMP_ENABLED
	strm << "C";
#endif
#if INCLUDE_IMGUI_ENABLED
	strm << "Im";
#endif
#if STATIC_WRITER_ENABLED
	strm << "W";
#endif
#if LOG_REPLACE_WITH_CALLBACK_ENABLED
	strm << "Lc";
#endif
#if VALGRIND_INTEGRATION_ENABLED
	strm << "V";
#endif
	strm << ")";
	return strm.str();
}

std::string Client::getWebsiteOrigin()
{
	return WEBSITE_ORIGIN;
}

void Client::checkDiscordReady()
{
	_owner.checkDiscordReady();
}

void Client::discordReady(const char* username, const char* discriminator,
	const std::string& picturename)
{
#if DISCORD_GUEST_ENABLED
	_discordUsername = username;
	_discordDiscriminator = discriminator;
#endif

	_owner.discordReady(username, discriminator, picturename);
}

void Client::discordDisconnected(int errorcode, const char* message)
{
	_owner.discordDisconnected(errorcode, message);
}

void Client::discordError(int errorcode, const char* message)
{
	_owner.discordError(errorcode, message);
}

void Client::discordJoinGame(const char* secret)
{
	_hotJoinSecret = secret;
	hotJoin();
}

void Client::discordSpectateGame(const char* secret)
{
	_hotSpectateSecret = secret;
	hotSpectate();
}

void Client::discordJoinRequest(const char* username, const char* discriminator,
	const std::string& picturename)
{
	_owner.discordJoinRequest(username, discriminator, picturename);
}

void Client::hotJoin()
{
	_owner.hotJoin(_hotJoinSecret);
}

void Client::hotSpectate()
{
	_owner.hotSpectate(_hotSpectateSecret);
}

void Client::hotClear()
{
	_hotJoinSecret.clear();
	_hotSpectateSecret.clear();
}

void Client::acceptJoinRequest()
{
	_owner.acceptJoinRequest();
}

void Client::denyJoinRequest()
{
	_owner.denyJoinRequest();
}

void Client::ignoreJoinRequest()
{
	_owner.ignoreJoinRequest();
}

void Client::steamConnecting()
{
	_owner.steamConnecting();
}

void Client::steamConnected(const char* personaname,
	const std::string& avatarpicturename,
	uint64_t steamId, const std::string& ticket)
{
	LOGI << "Connected as '" << personaname << "'"
		" (" << std::to_string(steamId) << ") on Steam";
	_steamId = steamId;
	_steamSessionTicket = ticket;

	_owner.steamConnected(personaname, avatarpicturename);
}

void Client::steamDisconnected()
{
	_owner.steamDisconnected();
}

void Client::steamMissing()
{
	_owner.steamMissing();
}

void Client::steamJoinGame(const std::string& secret)
{
	_hotJoinSecret = secret;
	hotJoin();
}

#if DICTATOR_ENABLED
/* ############################ DICTATOR_ENABLED ############################ */
void Client::obey(const Dictator& dictator)
{
	if (_socket)
	{
		for (const std::string& jsonstr : dictator.sendbuffer)
		{
			send(jsonstr, /*compressible=*/false);
		}
	}

	for (const std::string& jsonstr : dictator.recvbuffer)
	{
		LOGI << "Received message:"
			" \'" << jsonstr << "\'";
		receiveMessage(jsonstr.c_str(), jsonstr.size());
	}
}
/* ############################ DICTATOR_ENABLED ############################ */
#else
/* ########################## not DICTATOR_ENABLED ########################## */
void Client::obey(const Dictator& /**/)
{
	// Nothing to do.
}
/* ########################## not DICTATOR_ENABLED ########################## */
#endif
