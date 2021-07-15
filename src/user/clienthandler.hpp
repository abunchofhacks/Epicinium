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

enum class Player : uint8_t;
enum class Role : uint8_t;
enum class VisionType : uint8_t;
enum class Target : uint8_t;
enum class Difficulty : uint8_t;
enum class ResponseStatus : uint8_t;
struct Version;
struct Ranking;
class Screenshot;

enum class PortalStatus : uint8_t
{
	SUCCESS,
	BAD_REQUEST,
	MAINTENANCE,

	RESPONSE_MALFORMED,
	CONNECTION_FAILED,
};

enum class PatchServerStatus : uint8_t
{
	NO_PATCH_AVAILABLE,
	PATCH_AVAILABLE,

	RESPONSE_MALFORMED,
	CONNECTION_FAILED,
};


class ClientHandler
{
protected:
	ClientHandler() = default;

public:
	ClientHandler(const ClientHandler& /**/) = delete;
	ClientHandler(ClientHandler&& /**/) = delete;
	ClientHandler& operator=(const ClientHandler& /**/) = delete;
	ClientHandler& operator=(ClientHandler&& /**/) = delete;
	virtual ~ClientHandler() = default;

	virtual void debugHandler() const = 0;

	virtual void registerHandler(ClientHandler* /*handler*/) {};
	virtual void deregisterHandler(ClientHandler* /*handler*/) {};

	virtual void message(const std::string& /*message*/) {};
	virtual void chat(const std::string& /*user*/, const std::string& /*message*/, const Target& /*target*/) {};

	virtual void addUser(const std::string& /*user*/, const Json::Value& /*metadata*/, bool /*isSelf*/) {};
	virtual void removeUser(const std::string& /*user*/) {};
	virtual void removeLobby(const std::string& /*lobby*/) {};
	virtual void joinsLobby(const std::string& /*sender*/) {};
	virtual void inGame(const std::string& /*lobby*/, const std::string& /*sender*/, const Role& /*role*/) {};
	virtual void leavesLobby(const std::string& /*sender*/) {};
	virtual void listLobby(const std::string& /*lobby*/, const std::string& /*name*/, const Json::Value& /*metadata*/) {};
	virtual void listOwnLobby(const std::string& /*name*/, const Json::Value& /*metadata*/) {};
	virtual void joinsOwnLobby(const std::string& /*sender*/, bool /*isSelf*/) {};
	virtual void leavesOwnLobby(const std::string& /*sender*/) {};
	virtual void assignHost(const std::string& /*user*/, bool /*isSelf*/) {};
	virtual void assignRole(const std::string& /*user*/, const Role& /*role*/, bool /*isSelf*/) {};
	virtual void assignColor(const std::string& /*user*/, const Player& /*color*/) {};
	virtual void assignVisionType(const std::string& /*user*/, const VisionType& /*type*/) {};
	virtual void assignAI(const std::string& /*user*/, const std::string& /*ainame*/) {};
	virtual void assignDifficulty(const std::string& /*user*/, Difficulty /*difficulty*/) {};
	virtual void pickMap(const std::string& /*mapname*/) {};
	virtual void pickTimer(uint32_t /*timer*/) {};
	virtual void pickReplay(const std::string& /*replayname*/) {};
	virtual void pickChallenge(const std::string& /*name*/) {};
	virtual void pickRuleset(const std::string& /*replayname*/) {};
	virtual void addBot(const std::string& /*name*/) {};
	virtual void removeBot(const std::string& /*name*/) {};
	virtual void listMap(const std::string& /*name*/, const Json::Value& /*metadata*/) {};
	virtual void listReplay(const std::string& /*name*/, const Json::Value& /*metadata*/) {};
	virtual void listRuleset(const std::string& /*name*/, const Json::Value& /*metadata*/) {};
	virtual void listAI(const std::string& /*name*/, const Json::Value& /*metadata*/) {};
	virtual void listChallenge(const std::string& /*name*/, const Json::Value& /*metadata*/) {};
	virtual void receiveSecrets(const Json::Value& /*metadata*/) {};
	virtual void startGame(const Role& /*role*/) {};
	virtual void startTutorial() {};
	virtual void startHostedGame() {};
	virtual void setSkins(const Json::Value& /*metadata*/) {};
	virtual void receiveBriefing(const Json::Value& /*metadata*/) {};

	virtual void updateOwnRating(float /*rank*/) {};
	virtual void updateRating(const std::string& /*user*/, float /*rating*/) {};
	virtual void updateStars(const std::string& /*user*/, int /*stars*/) {};
	virtual void updateRecentStars(const std::string& /*key*/, int /*stars*/) {};

	virtual void displayRankings(const std::vector<Ranking>& /*rankings*/) {};

	virtual void displayStamp(const std::string& /*image*/, const std::string& /*tooltip*/, const std::string& /*url*/) {};

	virtual void inServer() {};
	virtual void outServer() {};
	virtual void inLobby(const std::string& /*lobby*/) {};
	virtual void outLobby() {};

	virtual void promptUsername() {};
	virtual void usernameTaken() {};
	virtual void identified() {};

	virtual void loggingIn() {};
	virtual void loggedIn() {};
	virtual void loggedOut() {};
	virtual void loginFailed(const ResponseStatus& /*responsestatus*/) {};
	virtual void accessingPortal() {};
	virtual void accessedPortal() {};
	virtual void accessingPortalFailed(const PortalStatus& /**/) {};
	virtual void registering() {};
	virtual void registered() {};
	virtual void registrationFailed(const ResponseStatus& /*responsestatus*/) {};
	virtual void sessionFailed(const ResponseStatus& /*responsestatus*/) {};
	virtual void activatingKey() {};
	virtual void activatedKey() {};
	virtual void activatingKeyFailed(const ResponseStatus& /*responsestatus*/) {};
	virtual void resettingPassword() {};
	virtual void resetPassword() {};
	virtual void resetPasswordFailed(const ResponseStatus& /*responsestatus*/) {};
	virtual void sendingFeedback() {};
	virtual void sentFeedback(const std::string& /*link*/) {};
	virtual void feedbackFailed(const ResponseStatus& /*responsestatus*/) {};

	virtual void connecting() {};
	virtual void connected() {};
	virtual void disconnected() {};
	virtual void systemFailure() {};
	virtual void serverOffline() {};
	virtual void serverShutdown() {};
	virtual void serverClosing() {};
	virtual void versionMismatch(const Version& /*version*/) {};
	virtual void patchAvailable(const Version& /*version*/) {};
	virtual void patchDownloading() {};
	virtual void patchPrimed(const Version& /*version*/) {};
	virtual void patchFailed() {};
	virtual void patchContinuing(const Version& /*version*/) {};
	virtual void patchContinued(const Version& /*version*/) {};
	virtual void requestFulfilled(const std::string& /*filename*/) {};
	virtual void requestFailed(const std::string& /*filename*/) {};
	virtual void downloading(const std::string& /*filename*/, float /*p*/) {};

	virtual void activateDiscord() {};
	virtual void deactivateDiscord() {};

	virtual void checkDiscordReady() {};

	virtual void discordReady(const char* /*username*/, const char* /*discriminator*/, const std::string& /*picturename*/) {}
	virtual void discordDeactivated() {}
	virtual void discordDisconnected(int /*errorcode*/, const char* /*message*/) {}
	virtual void discordError(int /*errorcode*/, const char* /*message*/) {}
	virtual void discordJoinRequest(const char* /*username*/, const char* /*discriminator*/, const std::string& /*picturename*/) {}

	virtual void acceptJoinRequest() {};
	virtual void denyJoinRequest() {};
	virtual void ignoreJoinRequest() {};

	virtual void hotJoin(const std::string& /*secret*/) {};
	virtual void hotSpectate(const std::string& /*secret*/) {};

	virtual void steamConnecting() {}
	virtual void steamConnected(const char* /*personaname*/, const std::string& /*avatarpicturename*/) {}
	virtual void steamDisconnected() {}
	virtual void steamMissing() {}
	virtual void steamAccountMergeTokenResult(const ResponseStatus& /*responsestatus*/) {}

	virtual void openWorkshopForMap(const std::string& /*mapname*/) {};
	virtual void openWorkshopForRuleset(const std::string& /*rulesetname*/) {};
	virtual void openWorkshopForPalette(const std::string& /*palettename*/) {};
	virtual void closeAllWorkshops() {};

	virtual void openUrl(const std::string& /*url*/) {};

	virtual void sendStomt(bool /*positive*/, const std::string& /*text*/) {};

	virtual void takeScreenshot(std::weak_ptr<Screenshot> /*screenshot*/) {}
	virtual void takeScreenshotOfMap() {}
	virtual void screenshotTaken(std::weak_ptr<Screenshot> /*screenshot*/) {}
};
