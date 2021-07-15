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
#include <thread>

#include "libs/SDL2/SDL_net.h"

#include "coredump.hpp"
#include "clock.hpp"
#include "limits.hpp"
#include "version.hpp"
#include "platform.hpp"
#include "loginstaller.hpp"
#include "settings.hpp"
#include "message.hpp"
#include "writer.hpp"
#include "parseerror.hpp"
#include "curl.hpp"
#include "slackapi.hpp"


class PulseSDL
{
public:
	PulseSDL();
	~PulseSDL();
};

class Pulse
{
public:
	Pulse(Settings& settings);
	~Pulse();

	Pulse(const Pulse&) = delete;
	Pulse(Pulse&&) = delete;
	Pulse& operator=(const Pulse&) = delete;
	Pulse& operator=(Pulse&&) = delete;

private:
	PulseSDL _sdl;
	Settings& _settings;
	Writer _writer;
	std::shared_ptr<Curl> _curl;
	SlackAPI _slack;

	TCPsocket _socket;

	uint64_t _mstime; // Apps Hungarian notation.

	bool checkForActivity();
	int safelyReceive(TCPsocket socket, char* buffer, uint32_t length);

	void sendMessage(const StreamedMessage& message,
		std::string& warning, uint64_t& warningTTL);
	void sendMessage(const char* data, uint32_t length,
		std::string& warning, uint64_t& warningTTL);
	void checkForResponse(std::string& warning, uint64_t& warningTTL);
	void connect(std::string& warning, uint64_t& warningTTL);
	void disconnect();

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
	// And that closing the window also shuts down the pulse properly.
	{
#ifdef PLATFORMUNIX
		signal(SIGTERM, shutdown);
		signal(SIGHUP, shutdown);
#else
		signal(SIGTERM, shutdown);
		signal(SIGBREAK, shutdown);
#endif
	}

	CoreDump::enable();

	std::string logname = "pulse";

	Settings settings("settings-pulse.json", argc, argv);

	if (settings.logname.defined())
	{
		logname = settings.logname.value();
	}
	else settings.logname.override(logname);

	std::cout << "[ Epicinium Pulse ]";
	std::cout << " (" << logname << " v" << Version::current() << ")";
	std::cout << std::endl << std::endl;

	if (settings.dataRoot.defined())
	{
		LogInstaller::setRoot(settings.dataRoot.value());
	}

#ifdef RELEASESERVER
	settings.logrollback.override(2000);
	LogInstaller(settings).withSeparateVerboseRollback(2000).install();
#else
	settings.logrollback.override(100);
	LogInstaller(settings).install();
#endif

	LOGI << "Start " << logname << " v" << Version::current();

	{
		Pulse pulse(settings);
		pulse.run();
	}

	LOGI << "End " << logname << " v" << Version::current();

	std::cout << std::endl << std::endl << "[ Done ]" << std::endl;
	return 0;
}

PulseSDL::PulseSDL()
{
	// This prevents converting SIGINT into SDL_QUIT, because otherwise it is
	// impossible to quit the game with ctrl+c when the game freezes.
	// TODO find out why this does not work on Sanders desktop
	SDL_SetHint(SDL_HINT_NO_SIGNAL_HANDLERS, "1");

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

PulseSDL::~PulseSDL()
{
	SDLNet_Quit();
	SDL_Quit();
}

static std::string generateUserAgentString()
{
	std::stringstream strm;
	strm << "epicinium-pulse/" << Version::current()
		<< " (" << ::platform()	<< "; cpp)";
	return strm.str();
}

Pulse::Pulse(Settings& settings) :
	_settings(settings),
	_curl(std::make_shared<Curl>(generateUserAgentString())),
	_slack(_curl, _settings.slackurl, "server-notifications",
		_settings.slackname, Version::current().toString()),
	_socket(nullptr),
	_mstime(SteadyClock::milliseconds())
{
	_writer.install();

	_slack.post("Pulse started.");
}

Pulse::~Pulse()
{
	if (_socket)
	{
		SDLNet_TCP_Close(_socket);
	}

	_slack.post("Pulse stopped.");
}

bool Pulse::checkForActivity()
{
	// TODO maybe it is better to reuse the same socket set?
	SDL_ClearError();
	SDLNet_SocketSet set = SDLNet_AllocSocketSet(1);
	if (!set)
	{
		throw std::runtime_error("Failed to allocate socket set: "
			+ std::string(SDLNet_GetError()));
	}
	SDLNet_TCP_AddSocket(set, _socket);

	SDL_ClearError();
	int active = SDLNet_CheckSockets(set, 1);
	SDLNet_FreeSocketSet(set);
	if (active < 0)
	{
		throw std::runtime_error("Error while checking for socket activity: "
			+ std::string(SDLNet_GetError()));
	}
	else return (active > 0);
}

int Pulse::safelyReceive(TCPsocket socket, char* buffer, uint32_t length)
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

void Pulse::run()
{
	if (!_settings.server.defined() && !_settings.port.defined())
	{
		throw std::runtime_error("No server or port defined.");
	}

	int criticalErrors = 0;

	uint64_t msLastPulse = 0;
	uint64_t msLastWarning = 0;
	uint64_t msPulseTTL = 0;
	uint64_t msWarningTTL = 0;
	std::string warning;
	std::string serverdescription = _settings.server.value()
		+ ":" + std::to_string(_settings.port.value());

	// The pulse server shuts down at the next possibility once killed.
	while (killcount == 0)
	{
		try
		{
			_curl->update();

			_mstime = SteadyClock::milliseconds();

			if (!_socket)
			{
				if (_mstime >= msLastPulse + msPulseTTL)
				{
					// Attempt to connect
					if (!warning.empty()
						&& _mstime >= msLastWarning + msWarningTTL)
					{
						_slack.error(warning, serverdescription);
						LOGI << "Posting warning: " + warning;
						msLastWarning = _mstime;
						msWarningTTL = std::max(uint64_t(60 * 1000),
							std::min(uint64_t(3600 * 1000), msWarningTTL * 2));
					}
					LOGI << "Attempting to connect";
					msLastPulse = _mstime;
					msPulseTTL = 4000;
					connect(warning, msWarningTTL);
					if (!_socket) continue;
					warning = "";
					msWarningTTL = 0;
					sendMessage(
						Message::version(Version::current()),
						warning, msWarningTTL);
				}
				else
				{
					std::this_thread::sleep_for(
						std::chrono::microseconds(1000000 / 60));
					continue;
				}
			}

			if (checkForActivity())
			{
				LOGI << "Activity detected";
				checkForResponse(warning, msWarningTTL);
			}
			else if (!warning.empty()
				&& _mstime >= msLastWarning + msWarningTTL)
			{
				_slack.error(warning, serverdescription);
				LOGI << "Posting warning: " + warning;
				msLastWarning = _mstime;
				msWarningTTL = std::max(uint64_t(60 * 1000),
					std::min(uint64_t(3600 * 1000), msWarningTTL * 2));
			}
			else if (_mstime >= msLastPulse + msPulseTTL)
			{
				// Pulse.
				LOGI << "Pulsing";
				sendMessage("", 0, warning, msWarningTTL);
				msLastPulse = _mstime;
				msPulseTTL = 4000;
				// Unless we receive a response within 66 seconds, we raise the
				// alarm.
				if (warning.empty())
				{
					warning = "No response!";
					msLastWarning = _mstime;
					msWarningTTL = 66 * 1000;
				}
			}
		}
		catch (const std::exception& e)
		{
			LOGW << "Error while running pulse server: " << e.what();
			LOGW << "Error count: " << ++criticalErrors;
			if (criticalErrors >= 4)
			{
				_slack.post("This pulse server is gravely ill"
					" and needs to be cured!");
				LOGF << "This pulse server is gravely ill"
					" and needs to be cured!";
				throw e;
			}
		}
		catch (...)
		{
			LOGW << "Unknown error while running pulse server";
			LOGW << "Error count: " << ++criticalErrors;
			if (criticalErrors >= 4)
			{
				_slack.post("This pulse server is gravely ill"
					" and needs to be cured!");
				LOGF << "This pulse server is gravely ill"
					" and needs to be cured!";
				throw std::runtime_error(
					"Unknown error while running pulse server");
			}
		}
	}
}

void Pulse::sendMessage(const StreamedMessage& message,
	std::string& warning, uint64_t& warningTTL)
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

	return sendMessage(jsonString.c_str(), length, warning, warningTTL);
}

void Pulse::sendMessage(const char* data, uint32_t length,
	std::string& warning, uint64_t& warningTTL)
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
		LOGW << "Error while sending message \'" << data << "\'";
		warning = "Connection loss!";
		warningTTL = 0;
		return;
	}
	LOGI << "Sent message: \'" << data << "\'";
}

void Pulse::checkForResponse(std::string& warning, uint64_t& warningTTL)
{
	SDLNet_SocketSet set = SDLNet_AllocSocketSet(1);
	if (!set)
	{
		warning = "Pulse Server broken!";
		warningTTL = 0;
		LOGE << "Pulse Server broken";
		disconnect();
		return;
	}
	SDLNet_TCP_AddSocket(set, _socket);

	int active;
	while ((active = SDLNet_CheckSockets(set, 0)) > 0
		&& SDLNet_SocketReady((TCPsocket) _socket))
	{
		// Get the network order (big-endian) byte representation of the length.
		uint8_t length_be_bytes[4] = {0, 0, 0, 0};
		if (safelyReceive(_socket, (char*) length_be_bytes, 4) <= 0)
		{
			warning = "Too few bytes in message length!";
			warningTTL = 0;
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
			warning = "Refusing to receive message";
			warningTTL = 0;
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
			LOGD << "Received empty message";

			// An empty message (i.e. without a body) is a pulse message.
			{
				// Disable the automatic 'No response' warning.
				warning = "";
				LOGD << "Pulse received";
			}

			continue /*onto the next message*/;
		}

		LOGD << "Receiving message of length " << length;

		std::vector<char> buffer;
		buffer.resize(length);
		if (safelyReceive(_socket, buffer.data(), length) <= 0)
		{
			warning = "Too few bytes in message!";
			warningTTL = 0;
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
					warning = "Received hard quit";
					warningTTL = 0;
					LOGW << "Received hard quit";
				}
				break;

				case Message::Type::CLOSED:
				{
					LOGI << "Server is shutting down, so kill";
					disconnect();
					++killcount;
				}
				break;

				case Message::Type::CLOSING:
				{
					LOGI << "Server is closing, so kill";
					disconnect();
					++killcount;
				}
				break;

				case Message::Type::PULSE:
				{
					// Disable the automatic 'No response' warning.
					warning = "";
					LOGD << "Pulse received";
				}
				break;
				case Message::Type::PING:
				{
					// Pings must always be responded with pongs.
					sendMessage(Message::pong(), warning, warningTTL);
					return;
				}
				break;
				case Message::Type::PONG:
				{
					// Disable the automatic 'No response' warning.
					warning = "";
					LOGD << "Pulse acknowledged";
					return;
				}
				break;

				case Message::Type::VERSION:
				{
					Version myversion = Version::current();
					if (   message.version().major == myversion.major
						&& message.version().minor == myversion.minor)
					{
						// Disable the automatic 'No response' warning.
						warning = "";
						LOGD << "Pulse acknowledged";
						return;
					}
					else
					{
						warning = "Version mismatch";
						warningTTL = 0;
						LOGW << "Version mismatch: " << message.version();
						return;
					}
				}
				break;

				case Message::Type::PATCH:
				case Message::Type::CHAT:
				case Message::Type::STAMP:
				case Message::Type::ENABLE_COMPRESSION:
				case Message::Type::DISABLE_COMPRESSION:
				{
					LOGD << "Ignored";
				}
				break;

				case Message::Type::INIT:
				case Message::Type::RATING_AND_STARS:
				case Message::Type::RATING:
				case Message::Type::STARS:
				case Message::Type::RECENT_STARS:
				case Message::Type::JOIN_SERVER:
				case Message::Type::LEAVE_SERVER:
				case Message::Type::JOIN_LOBBY:
				case Message::Type::LEAVE_LOBBY:
				case Message::Type::LIST_LOBBY:
				case Message::Type::MAKE_LOBBY:
				case Message::Type::DISBAND_LOBBY:
				case Message::Type::EDIT_LOBBY:
				case Message::Type::SAVE_LOBBY:
				case Message::Type::LOCK_LOBBY:
				case Message::Type::UNLOCK_LOBBY:
				case Message::Type::NAME_LOBBY:
				case Message::Type::MAX_PLAYERS:
				case Message::Type::NUM_PLAYERS:
				case Message::Type::SECRETS:
				case Message::Type::SKINS:
				case Message::Type::IN_GAME:
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
				case Message::Type::LIST_RULESET:
				case Message::Type::LIST_CHALLENGE:
				case Message::Type::RULESET_REQUEST:
				case Message::Type::RULESET_DATA:
				case Message::Type::RULESET_UNKNOWN:
				case Message::Type::START:
				case Message::Type::GAME:
				case Message::Type::TUTORIAL:
				case Message::Type::CHALLENGE:
				case Message::Type::RESTORE:
				case Message::Type::REPLAY:
				case Message::Type::RESIGN:
				case Message::Type::BRIEFING:
				case Message::Type::CHANGE:
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
					warning = "Invalid message!";
					warningTTL = 0;
					LOGW << "Invalid message";
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
			warning = "Broken message!";
			warningTTL = 0;
			return;
		}
		catch (Json::Exception& error)
		{
			LOGE << "Error while parsing message: "
				<< "\'" << std::string(buffer.data(), length) << "\', error "
				<< error.what();
			warning = "Broken message!";
			warningTTL = 0;
			return;
		}
	}
	SDLNet_FreeSocketSet(set);
	if (active < 0)
	{
		warning = "Disconnected!";
		warningTTL = 0;
		LOGW << "Disconnected";
		disconnect();
		return;
	}
}

void Pulse::connect(std::string& warning, uint64_t& /**/)
{
	if (!_settings.server.defined() || !_settings.port.defined())
	{
		throw std::runtime_error("No server or port defined.");
	}
	IPaddress ip;
	SDL_ClearError();
	if (SDLNet_ResolveHost(&ip, _settings.server.value().c_str(),
		_settings.port.value()))
	{
		LOGE << "Pulse connection failed: " << SDLNet_GetError();
		warning = "Pulse connection failed: " + std::string(SDLNet_GetError());
		// do not set warningTTL
		return;
	}
	SDL_ClearError();
	_socket = SDLNet_TCP_Open(&ip);
	if (!_socket)
	{
		LOGE << "Pulse connection failed: " << SDLNet_GetError();
		warning = "Pulse connection failed: " + std::string(SDLNet_GetError());
		// do not set warningTTL
		return;
	}
	LOGI << "Pulse connected";
	_slack.post("Pulse connected.");
}

void Pulse::disconnect()
{
	if (!_socket) return;
	std::string dummyS = "";
	uint64_t dummyMs = 0;
	sendMessage(Message::quit(), dummyS, dummyMs);
	SDLNet_TCP_Close(_socket);
	_socket = nullptr;
	LOGI << "Pulse disconnected";
	_slack.post("Pulse disconnected.");
}
