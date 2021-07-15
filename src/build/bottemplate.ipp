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
#include "source.hpp"

#include <csignal>
#include <chrono>
#include <unordered_map>

#include "libs/SDL2/SDL_net.h"

#include "coredump.hpp"
#include "clock.hpp"
#include "limits.hpp"
#include "version.hpp"
#include "platform.hpp"
#include "loginstaller.hpp"
#include "system.hpp"
#include "settings.hpp"
#include "message.hpp"
#include "writer.hpp"
#include "parseerror.hpp"
#include "curl.hpp"
#include "library.hpp"

#include "player.hpp"
#include "difficulty.hpp"
#include "order.hpp"
#include "change.hpp"

// In order to use this implementation, define the following variables:
//
// BOTNAME         e.g.   #define BOTNAME "example"
//   The technical name that is prepended with "bot" and then used for logs
//   and settings files. Preferably all lowercase letters.
//
// AICLASS         e.g.   #define AICLASS AIExample
//   The name of a C++ class that extends AICommander, without quotes.
//
// AIHPPFILE       e.g.   #define AIHPPFILE "aiexample.hpp"
//   The C++ header file that defines that class. If included in src/ai/,
//   only the filename is needed, otherwise it should be a full path.
//
// AINAME          e.g.   #define AINAME "Exampl-o-Matic"
//   The public name of this AI, between 3 and 16 characters consisting only of
//   letters (a-zA-Z), numbers, dashes, underscores and periods.
//
// AUTHORS         e.g.   #define AUTHORS "SLiV"
//   Your Epicinium username.

#ifdef BOTNAME
#ifdef AICLASS
#ifdef AIHPPFILE
#ifdef AINAME
#ifdef AUTHORS
/* ########################################################################## */

#include AIHPPFILE


struct Key
{
	char slot;
	std::string lobbyid;

	static Key undefined()
	{
		return Key{'\0', ""};
	}

	bool isWellDefined() const
	{
		return slot >= 'A' && slot <= 'Z' && !lobbyid.empty();
	}

	class Hash
	{
	public:
		char operator()(const Key& a) const
		{
			// Skip hashing the lobby id and just create 26 buckets that the keys
			// will be spread uniformly across.
			return a.slot;
		}
	};

	class Equals
	{
	public:
		bool operator()(const Key& a, const Key& b) const
		{
			return (a.slot == b.slot && a.lobbyid == b.lobbyid);
		}
	};
};

class ConnectedBotSDL
{
public:
	ConnectedBotSDL();
	~ConnectedBotSDL();
};

class ConnectedBot
{
public:
	ConnectedBot(Settings& settings);
	~ConnectedBot();

	ConnectedBot(const ConnectedBot&) = delete;
	ConnectedBot(ConnectedBot&&) = delete;
	ConnectedBot& operator=(const ConnectedBot&) = delete;
	ConnectedBot& operator=(ConnectedBot&&) = delete;

private:
	ConnectedBotSDL _sdl;
	Settings& _settings;
	Writer _writer;
	Library _library;
	std::shared_ptr<Curl> _curl;

	TCPsocket _socket = nullptr;
	SDLNet_SocketSet _socketset = nullptr;

	uint64_t _msNow; // Apps Hungarian notation.

	uint64_t _msLastPulse = 0;
	std::string _username;
	bool _listed = false;
	int _numActiveGames = 0;

	std::unordered_map<Key, std::unique_ptr<AICLASS>,
		Key::Hash, Key::Equals> _ais;

	std::vector<std::string> _rulesetsRequested;
	std::vector<std::pair<std::string, std::string>> _lobbiesWaiting;

	bool checkForActivity();
	int safelyReceive(TCPsocket socket, char* buffer, uint32_t length);

	void sendMessage(const StreamedMessage& message);
	void sendMessage(const char* data, uint32_t length);

	bool accessPortal(const std::string& url);
	bool connect();

	void checkForResponse();
	void disconnect();
	void waitForClosure();
	void join();

	void listRulesetForLobby(const std::string& rulesetname,
		const std::string& lobbyid);
	void confirmRuleset(const std::string& rulesetname);

public:
	void run();
};


static volatile sig_atomic_t killcount = 0;

static void shutdown(int signum)
{
	if (killcount <= 1) ++killcount;
	else
	{
		signal(signum, SIG_DFL);
		raise(signum);
	}
}

int main(int argc, char* argv[])
{
	// Make sure we can try to properly shut down by calling 'kill <pid>' once.
	{
		signal(SIGTERM, shutdown);
	}

	CoreDump::enable();

	std::string logname = "bot" BOTNAME;

	Settings settings("settings-bot" BOTNAME ".json", argc, argv);

	if (settings.logname.defined())
	{
		logname = settings.logname.value();
	}
	else settings.logname.override(logname);

	std::cout << "[ Epicinium Bot ]";
	std::cout << " (" << logname << " v" << Version::current() << ")";
	std::cout << std::endl << std::endl;

	if (settings.dataRoot.defined())
	{
		LogInstaller::setRoot(settings.dataRoot.value());
	}

	settings.logrollback.override(100);
	LogInstaller(settings).install();

	LOGI << "Start " << logname << " v" << Version::current();

	{
		ConnectedBot bot(settings);
		bot.run();
	}

	LOGI << "End " << logname << " v" << Version::current();

	std::cout << std::endl << std::endl << "[ Done ]" << std::endl;
	return 0;
}

ConnectedBotSDL::ConnectedBotSDL()
{
	// This prevents converting SIGINT into SDL_QUIT, because otherwise it is
	// impossible to quit the game with ctrl+c when the game freezes.

	// Initialize the timer SDL subsystem
	SDL_ClearError();
	if (SDL_Init(SDL_INIT_TIMER))
	{
		throw std::runtime_error("SDL_Init failed: "
			+ std::string(SDL_GetError()));
	}

	// Initialize SDLNet
	SDL_ClearError();
	if (SDLNet_Init())
	{
		throw std::runtime_error("SDLNet_Init failed: "
			+ std::string(SDLNet_GetError()));
	}
}

ConnectedBotSDL::~ConnectedBotSDL()
{
	SDLNet_Quit();
	SDL_Quit();
}

static std::string generateUserAgentString()
{
	std::stringstream strm;
	strm << "epicinium-bot/" << Version::current()
		<< " (" << ::platform()	<< "; cpp; " BOTNAME ")";
	return strm.str();
}

ConnectedBot::ConnectedBot(Settings& settings) :
	_settings(settings),
	_curl(std::make_shared<Curl>(generateUserAgentString())),
	_socket(nullptr),
	_msNow(SteadyClock::milliseconds())
{
	_writer.install();

	_library.load();
	_library.install();
}

ConnectedBot::~ConnectedBot()
{
	if (_socketset)
	{
		SDLNet_FreeSocketSet(_socketset);
	}

	if (_socket)
	{
		SDLNet_TCP_Close(_socket);
	}
}

bool ConnectedBot::checkForActivity()
{
	SDL_ClearError();
	int active = SDLNet_CheckSockets(_socketset, 1);
	if (active < 0)
	{
		throw std::runtime_error("Error while checking for socket activity: "
			+ std::string(SDLNet_GetError()));
	}
	else return (active > 0);
}

int ConnectedBot::safelyReceive(TCPsocket socket, char* buffer, uint32_t length)
{
	uint32_t receivedTotal = 0;
	while (receivedTotal < length)
	{
		int received = SDLNet_TCP_Recv(socket, buffer + receivedTotal,
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

inline Json::Value createAiMetadata()
{
	Json::Value metadata = Json::objectValue;
	assert(strlen(AUTHORS) >= 3 && strlen(AUTHORS) <= 30);
	metadata["authors"] = AUTHORS;
	return metadata;
}

void ConnectedBot::run()
{
	if (!_settings.port.defined() && _settings.server.defined()
		&& _settings.server.value().substr(0, 4) == "http")
	{
		std::string url = _settings.server.value() + "/api/v1/portal";
		bool accessed = accessPortal(url);
		if (!accessed)
		{
			return;
		}
	}

	bool connected = connect();
	if (!connected)
	{
		return;
	}

	sendMessage(Message::version(Version::current()));

	// The pulse server shuts down at the next possibility once killed.
	while (killcount < 2 && (killcount == 0 || _numActiveGames > 0))
	{
		if (killcount > 0 && _listed)
		{
			sendMessage(Message::list_ai(""));
			_listed = false;
		}

		_curl->update();

		_msNow = SteadyClock::milliseconds();

		if (checkForActivity())
		{
			LOGI << "Activity detected...";
			checkForResponse();
		}
		else if (_msNow >= _msLastPulse + 4000)
		{
			// Pulse.
			sendMessage("", 0);
			_msLastPulse = _msNow;
		}
	}

	if (_socket)
	{
		disconnect();
	}
}

bool ConnectedBot::accessPortal(const std::string& url)
{
	LOGI << "Accessing \'" << url << "\'...";
	auto future = _curl->get(url);
	bool ready = false;
	while (future.valid() && !ready)
	{
		_curl->update();
		ready = (future.wait_for(std::chrono::seconds(0))
					== std::future_status::ready);
	}
	if (!ready)
	{
		LOGF << "Failed to access portal at \'" << url << "\'.";
		std::cout << "Failed to access portal." << std::endl;
		return false;
	}

	Response response = future.get();
	if (response.errorcode)
	{
		LOGF << "Failed to access portal at \'" << url << "\':"
			" connection failure.";
		std::cout << "Connection failure while accessing portal." << std::endl;
		return false;
	}

	LOGI << "Got a response from portal server:"
		<< " [" << response.statuscode << "] "
		<< response.body;
	if (response.statuscode == 503)
	{
		LOGF << "Accessed portal but servers unavailable.";
		std::cout << "Server unavailable." << std::endl;
		return false;
	}
	else if (response.statuscode != 200)
	{
		LOGF << "Accessed portal but received invalid status code.";
		std::cout << "Unexpected status while accessing portal." << std::endl;
		return false;
	}

	Json::Reader reader;
	Json::Value json;
	if (!reader.parse(response.body, json) || !json.isObject())
	{
		LOGF << "Accessed portal but received malformed response.";
		std::cout << "Malformed response from portal." << std::endl;
		return false;
	}

	if (json["host"].isString()
		&& json["port"].isUInt()
		&& json["port"].asUInt() <= 0xFFFF)
	{
		LOGI << "Accessed portal.";
		_settings.server.override(json["host"].asString());
		_settings.port.override((uint16_t) json["port"].asUInt());
		return true;
	}
	else
	{
		LOGF << "Accessed portal but received empty response.";
		std::cout << "Empty response from portal." << std::endl;
		return false;
	}
}

void ConnectedBot::sendMessage(const StreamedMessage& message)
{
	if (!_socket) return;

	std::ostringstream strm;
	strm << message;
	std::string jsonString = strm.str();
	size_t stringlength = jsonString.length();
	if (stringlength >= MESSAGE_SIZE_LIMIT)
	{
		LOGF << "Cannot send messages larger than MESSAGE_SIZE_LIMIT.";
		throw std::runtime_error(
				"Cannot send messages larger than MESSAGE_SIZE_LIMIT.");
	}
	else if (stringlength >= MESSAGE_SIZE_WARNING_LIMIT)
	{
		LOGW << "Sending very large message of length " << stringlength;
	}
	uint32_t length = stringlength;

	return sendMessage(jsonString.c_str(), length);
}

void ConnectedBot::sendMessage(const char* data, uint32_t length)
{
	// Get the network order (big-endian) byte representation of the length.
	uint8_t length_be_bytes[4] = {
		uint8_t(length >> 24),
		uint8_t(length >> 16),
		uint8_t(length >> 8),
		uint8_t(length),
	};

	LOGI << "Sending message of length " << length;
	bool success = true;
	{
		if (SDLNet_TCP_Send(_socket, length_be_bytes, 4) < 4
			|| SDLNet_TCP_Send(_socket, data, length) < int(length))
		{
			success = false;
		}
	}
	if (!success)
	{
		LOGF << "Error while sending message \'" << data << "\'";
		throw std::runtime_error("Error while sending message.");
	}
	LOGI << "Sent message: \'" << data << "\'";
}

inline Key extractKeyFromMetadata(const Json::Value& metadata)
{
	Key key = Key::undefined();
	if (metadata["lobby_id"].isString() && metadata["slot"].isString()
		&& metadata["slot"].asString().length() == 2
		&& metadata["slot"].asString()[0] == '%'
		&& metadata["slot"].asString()[1] >= 'A'
		&& metadata["slot"].asString()[2] <= 'Z')
	{
		key.slot = metadata["slot"].asString()[1];
		key.lobbyid = metadata["lobby_id"].asString();
	}
	else
	{
		LOGE << "Failed to extract key from " << Writer::write(metadata);
	}
	return key;
}

void ConnectedBot::checkForResponse()
{
	int active;
	while ((active = SDLNet_CheckSockets(_socketset, 0)) > 0
		&& SDLNet_SocketReady((TCPsocket) _socket))
	{
		// Get the network order (big-endian) byte representation of the length.
		uint8_t length_be_bytes[4] = {0, 0, 0, 0};
		if (safelyReceive(_socket, (char*) length_be_bytes, 4) <= 0)
		{
			LOGW << "Too few bytes in message length";
			disconnect();
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
			return;
		}
		else if (length >= MESSAGE_SIZE_WARNING_LIMIT)
		{
			LOGW << "Receiving very large message of length " << length;
		}
		else if (length == 0)
		{
			// An empty message (i.e. without a body) is a pulse message.
			LOGD << "Received empty message (pulse)";

			continue /*onto the next message*/;
		}

		LOGD << "Receiving message of length " << length;

		std::vector<char> buffer;
		buffer.resize(length);
		if (safelyReceive(_socket, buffer.data(), length) <= 0)
		{
			LOGW << "Too few bytes in message";
			disconnect();
			return;
		}

		if (length > 0 && buffer[0] == '=')
		{
			LOGW << "Received compressed message in pulse server";
			disconnect();
			return;
		}

		LOGD << "Received message: " << std::string(buffer.data(), length);

		try
		{
			ParsedMessage message = Message::parse(buffer.data(), length);
			switch (message.type())
			{
				case Message::Type::QUIT:
				{
					LOGW << "Received hard quit";
				}
				break;

				case Message::Type::CLOSED:
				{
					LOGI << "Server is shutting down";
					disconnect();
					++killcount;
					return;
				}
				break;

				case Message::Type::CLOSING:
				{
					LOGI << "Server is closing";
					disconnect();
					++killcount;
					return;
				}
				break;

				case Message::Type::PULSE:
				{
					LOGD << "Pulse received";
				}
				break;
				case Message::Type::PING:
				{
					// Pings must always be responded with pongs.
					sendMessage(Message::pong());
				}
				break;
				case Message::Type::PONG:
				break;

				case Message::Type::VERSION:
				{
					Version myversion = Version::current();
					if (   message.version().major == myversion.major
						&& message.version().minor == myversion.minor)
					{
						join();
					}
					else
					{
						LOGW << "Version mismatch: " << message.version();
						disconnect();
						++killcount;
						return;
					}
				}
				break;

				case Message::Type::PATCH:
				case Message::Type::CHAT:
				case Message::Type::STAMP:
				case Message::Type::ENABLE_COMPRESSION:
				case Message::Type::DISABLE_COMPRESSION:
				case Message::Type::INIT:
				case Message::Type::STARS:
				case Message::Type::RECENT_STARS:
				case Message::Type::RATING_AND_STARS:
				case Message::Type::RATING:
				case Message::Type::LIST_CHALLENGE:
				case Message::Type::LIST_LOBBY:
				case Message::Type::DISBAND_LOBBY:
				case Message::Type::JOIN_LOBBY:
				case Message::Type::LEAVE_LOBBY:
				case Message::Type::IN_GAME:
				case Message::Type::LEAVE_SERVER:
				{
					LOGD << "Ignored";
				}
				break;

				case Message::Type::JOIN_SERVER:
				{
					std::string newname = message.content();

					if (newname.empty())
					{
						LOGE << "Failed to join server";
						throw std::runtime_error("Failed to join server");
					}
					// This is information about successfully joining.
					else if (_username.empty())
					{
						LOGD << "Joined server as '" << newname << "'";
						_username = newname;

						// This is a stupid hack because the server might kick
						// us for no reason if we send the LIST_AI message
						// too soon after it sent us the JOIN_SERVER message.
						std::this_thread::sleep_for(std::chrono::seconds(2));

						assert(strlen(AINAME) >= 3 && strlen(AINAME) <= 16);
						sendMessage(Message::list_ai(AINAME,
							createAiMetadata()));
						_listed = true;
					}
					// This is information about who is online: us.
					else if (newname == _username)
					{
						LOGD << "Ignored";
					}
					// This is information about who is online: someone else.
					else
					{
						LOGD << "Ignored";
					}
				}
				break;

				case Message::Type::LIST_RULESET:
				{
					if (message.metadata()["lobby_id"].isString())
					{
						listRulesetForLobby(message.content(),
							message.metadata()["lobby_id"].asString());
					}
					else
					{
						LOGD << "Ignored";
					}
				}
				break;

				case Message::Type::RULESET_DATA:
				{
					std::string rulesetname = message.content();
					if (Library::storeBible(rulesetname, message.data()))
					{
						confirmRuleset(rulesetname);
					}
					else
					{
						LOGE << "Failed to install ruleset " << rulesetname;
					}
				}
				break;

				case Message::Type::RULESET_UNKNOWN:
				{
					LOGE << "Failed to request ruleset " << message.content();
				}
				break;

				case Message::Type::GAME:
				{
					Key key = extractKeyFromMetadata(message.metadata());
					if (!key.isWellDefined())
					{
						LOGE << "GAME message without key";
						break;
					}
					else if (_ais[key])
					{
						LOGE << "GAME message with duplicate key";
						break;
					}

					Difficulty difficulty = message.difficulty();
					if (difficulty == Difficulty::NONE)
					{
						LOGW << "GAME messaeg without difficulty";
					}

					_ais[key].reset(new AICLASS(message.player(),
						difficulty,
						message.content(),
						key.slot));
				}
				break;

				case Message::Type::CHANGE:
				{
					Key key = extractKeyFromMetadata(message.metadata());
					if (!key.isWellDefined())
					{
						LOGE << "CHANGE message without key";
						break;
					}
					else if (!_ais[key])
					{
						LOGE << "CHANGE message with invalid key";
						break;
					}

					_ais[key]->receiveChangesAsJson(message.changes());

					if (_ais[key]->wantsToPrepareOrders())
					{
						_ais[key]->prepareOrders();
						sendMessage(Message::order_new(_ais[key]->bible(),
							_ais[key]->orders(),
							message.metadata()));
					}
				}
				break;

				case Message::Type::MAKE_LOBBY:
				case Message::Type::EDIT_LOBBY:
				case Message::Type::SAVE_LOBBY:
				case Message::Type::LOCK_LOBBY:
				case Message::Type::UNLOCK_LOBBY:
				case Message::Type::NAME_LOBBY:
				case Message::Type::MAX_PLAYERS:
				case Message::Type::NUM_PLAYERS:
				case Message::Type::SECRETS:
				case Message::Type::SKINS:
				case Message::Type::CLAIM_HOST:
				case Message::Type::CLAIM_ROLE:
				case Message::Type::CLAIM_COLOR:
				case Message::Type::CLAIM_VISIONTYPE:
				case Message::Type::CLAIM_AI:
				case Message::Type::CLAIM_DIFFICULTY:
				case Message::Type::ENABLE_CUSTOM_MAPS:
				case Message::Type::PICK_MAP:
				case Message::Type::PICK_TIMER:
				case Message::Type::PICK_REPLAY:
				case Message::Type::PICK_CHALLENGE:
				case Message::Type::PICK_RULESET:
				case Message::Type::ADD_BOT:
				case Message::Type::REMOVE_BOT:
				case Message::Type::LIST_AI:
				case Message::Type::LIST_MAP:
				case Message::Type::LIST_REPLAY:
				case Message::Type::RULESET_REQUEST:
				case Message::Type::START:
				case Message::Type::TUTORIAL:
				case Message::Type::CHALLENGE:
				case Message::Type::RESTORE:
				case Message::Type::REPLAY:
				case Message::Type::RESIGN:
				case Message::Type::BRIEFING:
				case Message::Type::ORDER_OLD:
				case Message::Type::ORDER_NEW:
				case Message::Type::SYNC:
				case Message::Type::HOST_SYNC:
				case Message::Type::HOST_REJOIN_REQUEST:
				case Message::Type::HOST_REJOIN_CHANGES:
				case Message::Type::DOWNLOAD:
				case Message::Type::REQUEST:
				case Message::Type::REQUEST_DENIED:
				case Message::Type::REQUEST_FULFILLED:
				case Message::Type::RANKINGS:
				case Message::Type::LINK_ACCOUNTS:
				case Message::Type::INVALID:
				{
					LOGW << "Invalid message: " << message;
					return;
				}
				break;
			}
		}
		catch (ParseError& error)
		{
			LOGE << "Error while parsing message: "
				<< "\'" << std::string(buffer.data(), length) << "\', error "
				<< error.what();
			throw;
		}
		catch (Json::Exception& error)
		{
			LOGE << "Error while parsing message: "
				<< "\'" << std::string(buffer.data(), length) << "\', error "
				<< error.what();
			throw;
		}
	}
	if (active < 0)
	{
		LOGW << "Server disconnected";
		disconnect();
		return;
	}
}

bool ConnectedBot::connect()
{
	LOGI << "Connecting...";
	if (!_settings.server.defined() || !_settings.port.defined())
	{
		LOGF << "No server defined or port defined.";
		std::cout << "No server or port defined." << std::endl;
		return false;
	}
	IPaddress ip;
	SDL_ClearError();
	if (SDLNet_ResolveHost(&ip, _settings.server.value().c_str(),
		_settings.port.value()))
	{
		LOGE << "Failed to resolve host: " << SDLNet_GetError();
		std::cout << "Failed to resolve host." << std::endl;
		return false;
	}
	SDL_ClearError();
	_socket = SDLNet_TCP_Open(&ip);
	if (!_socket)
	{
		LOGE << "Connection failed: " << SDLNet_GetError();
		std::cout << "Connection failed." << std::endl;
		return false;
	}
	LOGI << "Connected.";

	SDL_ClearError();
	_socketset = SDLNet_AllocSocketSet(1);
	if (!_socketset)
	{
		LOGF << "Failed to allocate socket set: " << SDLNet_GetError();
		std::cout << "Failed to allocate socket set." << std::endl;
		return false;
	}
	SDLNet_TCP_AddSocket(_socketset, _socket);
	return true;
}

void ConnectedBot::disconnect()
{
	if (!_socket) return;
	LOGI << "Disconnecting...";
	sendMessage(Message::quit());
	waitForClosure();
	SDLNet_TCP_Close(_socket);
	_socket = nullptr;
	LOGI << "Disconnected.";
}

void ConnectedBot::waitForClosure()
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

void ConnectedBot::join()
{
	// TODO _accountsfolder (?)
	std::string filename = "accounts/bot" BOTNAME ".acc";
	std::ifstream file;
	try
	{
		file = System::ifstream(filename);
	}
	catch (const std::ifstream::failure& error)
	{
		LOGE << "Failed to open '" << filename << "'";
		throw;
	}
	if (!file.is_open())
	{
		LOGE << "Failed to open '" << filename << "'";
		throw std::runtime_error("Failed to open session file.");
	}

	Json::Reader reader;
	Json::Value json;
	std::string line;
	if (!std::getline(file, line) || !reader.parse(line, json)
		|| !json.isObject() || !json["id"].isInt()
		|| !json["session_token"].isString())
	{
		LOGE << "Failed to parse '" << filename << "'";
		throw std::runtime_error("Failed to parse session file.");
	}

	std::string token = json["session_token"].asString();
	std::string id = std::to_string(json["id"].asInt());
	Json::Value metadata = Json::objectValue;

	sendMessage(Message::join_server(token, id, metadata));
}

void ConnectedBot::listRulesetForLobby(const std::string& rulesetname,
	const std::string& lobbyid)
{
	// Do we have this ruleset in our library, either cached or saved?
	if (!Library::existsBible(rulesetname))
	{
		if (std::find_if(_lobbiesWaiting.begin(), _lobbiesWaiting.end(),
				[lobbyid](const std::pair<std::string, std::string>& pair) {
					return std::get<0>(pair) == lobbyid;
				})
				== _lobbiesWaiting.end())
		{
			_lobbiesWaiting.emplace_back(lobbyid, rulesetname);
		}

		if (std::find(_rulesetsRequested.begin(), _rulesetsRequested.end(),
					rulesetname)
				!= _rulesetsRequested.end())
		{
			return;
		}

		sendMessage(Message::ruleset_request(rulesetname));
		_rulesetsRequested.emplace_back(rulesetname);
	}
	else
	{
		// Confirm that we have the ruleset.
		Json::Value metadata = Json::objectValue;
		metadata["lobby_id"] = lobbyid;
		sendMessage(Message::list_ruleset(rulesetname, metadata));
	}
}

void ConnectedBot::confirmRuleset(const std::string& rulesetname)
{
	auto split = std::partition(_lobbiesWaiting.begin(), _lobbiesWaiting.end(),
		[rulesetname](const std::pair<std::string, std::string>& pair) {
			return std::get<1>(pair) != rulesetname;
		});
	for (auto iter = split; iter != _lobbiesWaiting.end(); iter++)
	{
		Json::Value metadata = Json::objectValue;
		metadata["lobby_id"] = std::get<0>(*iter);
		sendMessage(Message::list_ruleset(rulesetname, metadata));
	}
	_lobbiesWaiting.erase(split, _lobbiesWaiting.end());
}

/* ########################################################################## */
#endif
#endif
#endif
#endif
#endif
