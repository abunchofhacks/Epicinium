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

#include "clienthandler.hpp"
#include "discordcdn.hpp"


struct DiscordUser;

class Client;
enum class Role : uint8_t;

class DiscordRPC final : public ClientHandler
{
public:
	DiscordRPC(ClientHandler& owner, Client& client);
	~DiscordRPC();

	DiscordRPC(const DiscordRPC& that) = delete;
	DiscordRPC(DiscordRPC&& that) = delete;
	DiscordRPC& operator=(const DiscordRPC& that) = delete;
	DiscordRPC& operator=(DiscordRPC&& that) = delete;

private:
	ClientHandler& _owner;
	Client& _client;
	DiscordCDN _discordCDN;

	void handleDiscordReady(const DiscordUser* request);
	void handleDiscordDisconnected(int errorcode, const char* message);
	void handleDiscordError(int errorcode, const char* message);
	void handleDiscordJoinGame(const char* secret);
	void handleDiscordSpectateGame(const char* secret);
	void handleDiscordJoinRequest(const DiscordUser* request);

	void clearLobbyInfo();
	void updateLobbyInfo();
	void updatePresence(bool dirty = true);

	void install();

public:
	virtual void debugHandler() const override;

	virtual void activateDiscord() override;
	virtual void deactivateDiscord() override;

	virtual void inServer() override;
	virtual void outServer() override;
	virtual void loggedOut() override;
	virtual void listOwnLobby(const std::string& lobbyname,
		const Json::Value& metadata) override;
	virtual void pickMap(const std::string& mapname) override;
	virtual void pickReplay(const std::string& replayname) override;
	virtual void pickChallenge(const std::string& name) override;
	virtual void listChallenge(const std::string& name, const Json::Value& metadata) override;
	virtual void receiveSecrets(const Json::Value& metadata) override;
	virtual void inLobby(const std::string& lobbyid) override;
	virtual void outLobby() override;
	virtual void startGame(const Role& role) override;
	virtual void startTutorial() override;

	virtual void checkDiscordReady() override;
	virtual void acceptJoinRequest() override;
	virtual void denyJoinRequest() override;
	virtual void ignoreJoinRequest() override;

	void update();
};
