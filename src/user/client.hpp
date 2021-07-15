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

#include <future>

typedef struct _TCPsocket* TCPsocket;

namespace femtozip
{
	class CompressionModel;
}

#include "account.hpp"
#include "responsestatus.hpp"
#include "response.hpp"

class Settings;
class Game;
class GameOwner;
class HostedGame;
class ClientHandler;
class StreamedMessage;
class ParsedMessage;
class Dictator;
class Firewall;
struct Version;
struct Download;
class Curl;
class PatchInProgress;
enum class PortalStatus:  uint8_t;
class Screenshot;


struct EmailPreference
{
	enum Preference : uint8_t
	{
		UNSET,
		NO,
		YES
	};
	Preference mailinglist = UNSET;
};

class Client
{
public:
	Client(ClientHandler& owner, GameOwner& gameowner, Settings& settings,
		Firewall* firewall = nullptr);
	~Client();

	Client(const Client&) = delete;
	Client(Client&&) = delete;
	Client& operator=(const Client&) = delete;
	Client& operator=(Client&&) = delete;

private:
	struct LoginData
	{
		ResponseStatus status;
		uint32_t accountId;
		std::string token;
		bool remember;
	};

	ClientHandler& _owner;
	GameOwner& _gameowner;
	Firewall* const _firewall;
	Settings& _settings;
	Account _account;
	TCPsocket _socket = nullptr;
	std::string _lobbyID = "";
	std::weak_ptr<Game> _game;
	std::weak_ptr<HostedGame> _hosted;
	bool _disconnected = false;
	bool _resetting = false;
	bool _initialized = false;
	bool _notifiedSlowResponse = false;
	ResponseStatus _sessionStatus = ResponseStatus::SUCCESS;
	double _lastsendtime = -1.0;
	double _lastreceivetime = -1.0;
	double _lastknownping = -1.0;
	double _pingsendtime = -1.0;
	std::string _fzmodelname;
	std::shared_ptr<femtozip::CompressionModel> _compressionModel;
	std::string _hotJoinSecret;
	std::string _hotSpectateSecret;

	std::shared_ptr<Curl> _curl;

	double _connectstarttime = -1.0;
	double _autoresettime = -1.0;
	std::thread _connectThread;
	std::future<TCPsocket> _futureSocket;

	bool _accessedPortal = false;
	bool _autoRetryPortal = false;
	std::future<Response> _futurePortal;

	bool _isSteamEnabled = false;
	uint64_t _steamId = 0;
	std::string _steamSessionTicket;
	std::string _usernameForSteam;
	std::string _steamAccountMergeToken;
	std::future<Response> _futureSteamAccountMergeToken;

	std::unique_ptr<Version> _patchVersion;
	std::string _manifestQuery;
	std::future<Response> _futurePatchManifest;
	std::future<Response> _futureAutoPatch;
	bool _autoPatchActivated = false;
	int _downloadsInProgress = 0;
	std::vector<Download> _downloads; // (married)
	std::vector<std::shared_ptr<std::atomic<float>>> _percentages; // (married)
	std::vector<std::future<Response>> _futureDownloads; // (married)
	std::unique_ptr<Json::Value> _mementoJson;
	std::future<Response> _futureFzmodelRequest;
	std::unique_ptr<Download> _activeFzmodelDownload;
	std::future<Response> _futureFzmodelDownload;

	std::future<Response> _futureRegistration;
	std::future<Response> _futureLogin;
	std::future<Response> _futureKey;
	std::future<Response> _futureReset;
	std::future<Response> _futureBug;
	std::future<Response> _futureLeaderboard;

	void handleMessage(const ParsedMessage& message);
	int safelyReceive(char* buffer, uint32_t length);
	void checkForNewMessages();
	void receiveMessage(const char* buffer, uint32_t length);
	void invalidateSession(uint32_t accountId, const std::string& sessionToken);
	void waitForClosure();

	bool checkConnection();
	void checkVitals();
	void checkPortal();
	void checkAutoPatch();
	void checkPatchManifest();
	void checkPatchProgress();
	void checkFzmodelRequest();
	void checkFzmodelDownload();
	void checkRegistration();
	void checkLogin();
	void checkKey();
	void checkReset();
	void checkBug();
	void checkLeaderboard();
	void checkSteamAccountMergeToken();

	bool isPatchPrimed(const Version& version);

	void requestFzmodel();
	void enableCompression();

	static bool fulfilRequest(const Download& download);

	void sendLogs();

	static void runConnectThread(std::string server, uint16_t port,
		std::promise<TCPsocket> promise);

	void handlePortalResponse(Response response);
	void handlePortalInfo(const std::string& host, uint16_t port);
	void handlePatchInfo(const Json::Value& info);
	void handleStamp(const Json::Value& json);

	void handlePatchManifestResponse(Response response);
	void handleAutoPatchResponse(Response response);

	static ResponseStatus parseRegistrationResponse(Response response);
	static LoginData parseLoginResponse(Response response);
	static ResponseStatus parseKeyActivationResponse(Response response);
	static ResponseStatus parsePasswordResetResponse(Response response);
	static ResponseStatus parseBugReportResponse(Response response);

	bool connectToPortal();
	void connect(const std::string& server, uint16_t port);

public:
	void connect();
	void disconnect(bool sendMessage = true);
	void join();
	void leave();
	void requestLeaderboard();

	void startPatch();

	void setUsernameForSteam(const std::string& username);
	void requestSteamAccountMergeToken(const std::string& email,
		const std::string& password);
	void cancelSteamAccountMergeToken();

	void registration(const std::string& username, const std::string& email,
		const std::string& password);
	void updateEmailPref(const std::string& email,
		const EmailPreference& preference);

	void login(const std::string& email, const std::string& password,
		bool remember);
	void logout(bool reset);

	void activateKey(const std::string& token);

	void forgotPassword(const std::string& email);
	void resetPassword(const std::string& email, const std::string& token,
		const std::string& password);

	void feedback(bool positive, const std::string& text, bool sendLogs);
	void bugReport(const std::string& text, bool sendLogs);

	void sendPulse();
	void send(StreamedMessage message);

private:
	void send(const std::string& jsonstr, bool compressible);
	void send(const char* data, uint32_t length);

public:
	void update();

	void obey(const Dictator& dictator);

	std::shared_ptr<Curl> getCurlAccess() { return _curl; }

	std::string getVersionDebugString() const;

	static std::string getWebsiteOrigin();

	bool accessedPortal() { return _accessedPortal; }
	bool isSteamEnabled() { return _isSteamEnabled; }
	bool disconnected() { return _disconnected; }
	bool resetting() { return _resetting; }
	bool autoresetActive();
	ResponseStatus sessionStatus() { return _sessionStatus; }

	void registerHandler(ClientHandler* handler);
	void deregisterHandler(ClientHandler* handler);

	bool hotJoining() { return !_hotJoinSecret.empty(); }
	bool hotSpectating() { return !_hotSpectateSecret.empty(); }

	void hotJoin();
	void hotSpectate();
	void hotClear();

	void acceptJoinRequest();
	void denyJoinRequest();
	void ignoreJoinRequest();

	void activateDiscord();
	void deactivateDiscord();

	void tryLinkAccounts(const std::string& discordUserId);

	void checkMementoAutoPatch();

	void checkDiscordReady();

	void discordAvailable();
	void discordJoinGame(const char* secret);
	void discordSpectateGame(const char* secret);
	void discordReady(const char* username, const char* discriminator,
		const std::string& picturename);
	void discordDeactivated();
	void discordDisconnected(int errorcode, const char* message);
	void discordError(int errorcode, const char* message);
	void discordJoinRequest(const char* username, const char* discriminator,
		const std::string& picturename);

	void steamConnecting();
	void steamConnected(const char* personaname,
		const std::string& avatarpicturename, uint64_t steamId,
		const std::string& ticket);
	void steamDisconnected();
	void steamMissing();
	void steamJoinGame(const std::string& secret);

	void openWorkshopForMap(const std::string& mapname);
	void openWorkshopForRuleset(const std::string& rulesetname);
	void openWorkshopForPalette(const std::string& palettename);
	void closeAllWorkshops();

	void openUrl(const std::string& url);

	void takeScreenshot(std::weak_ptr<Screenshot> screenshot);
	void takeScreenshotOfMap();

	void hostedGameStarted(std::weak_ptr<HostedGame> hostedGame);
};
