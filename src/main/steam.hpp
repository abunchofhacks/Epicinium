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

#if STEAM_ENABLED
/* ############################# STEAM_ENABLED ############################# */

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"
#pragma GCC diagnostic ignored "-Wpedantic"
#include "libs/steam/steam_api.h"
#pragma GCC diagnostic pop

#include "clienthandler.hpp"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"


#define AUTO_STEAM_CALLBACK_GET(X) \
	_STEAM_CALLBACK_3(/**/, Steam, handle##X, Get##X##_t)
#define AUTO_STEAM_CALLBACK_NEW(X) \
	_STEAM_CALLBACK_3(/**/, Steam, handle##X, New##X##_t)

class Client;

class Steam final : public ClientHandler
{
private:
	class SessionTicket
	{
	private:
		HAuthTicket _handle;
		char _buffer[1024];
		uint32_t _length = 0;

	public:
		void retrieve();
		HAuthTicket handle() const
		{
			return _handle;
		}
		std::vector<uint8_t> data() const
		{
			return std::vector<uint8_t>(_buffer, _buffer + _length);
		}
	};

	Steam(Client& client);

public:
	~Steam();

	Steam(const Steam& that) = delete;
	Steam(Steam&& that) = delete;
	Steam& operator=(const Steam& that) = delete;
	Steam& operator=(Steam&& that) = delete;

private:
	Client& _client;

	SessionTicket _serverSessionTicket;
	std::vector<int> _cachedImageHandles;

	bool _isOnline = false;

	std::string _lobbyId;
	std::string _lobbyType;
	std::vector<std::string> _lobbyMemberUsernames;
	int _maxPlayersAndBots;
	int _numPlayersAndBots;
	int _numBots;
	int _numPlayers;
	std::string _challengeDisplayName;
	bool _isPlaying;
	bool _isSpectating;

	std::string _groupId;
	std::string _groupSize;
	std::string _legacyStatus;
	std::string _localizedStatus;
	std::string _challengeKey;
	std::string _lobbyTypeKey;
	std::string _lobbyMaxSize;
	std::string _lobbyCurSize;
	std::string _connectCommand;

	AUTO_STEAM_CALLBACK_GET(AuthSessionTicketResponse);
	AUTO_STEAM_CALLBACK_NEW(UrlLaunchParameters);

	void ready();

	std::string getAvatarPictureName(CSteamID steamid);

	void clearLobbyInfo();
	void updateLobbyInfo();
	void updatePresence();

public:
	static bool shouldRestart();
	static std::unique_ptr<Steam> create(Client& client);

	virtual void debugHandler() const override;

	virtual void disconnected() override;

	virtual void inServer() override;
	virtual void outServer() override;
	virtual void loggedOut() override;
	virtual void inLobby(const std::string& lobbyid) override;
	virtual void outLobby() override;
	virtual void listOwnLobby(const std::string& lobbyname,
		const Json::Value& metadata) override;
	virtual void joinsOwnLobby(const std::string& name, bool isSelf) override;
	virtual void leavesOwnLobby(const std::string& name) override;
	virtual void listChallenge(const std::string& name, const Json::Value& metadata) override;
	virtual void receiveSecrets(const Json::Value& metadata) override;
	virtual void startGame(const Role& role) override;
	virtual void startTutorial() override;

	void update();
};

#pragma GCC diagnostic pop

/* ############################# STEAM_ENABLED ############################# */
#else
class Steam
{
	// Empty.
};
#endif
