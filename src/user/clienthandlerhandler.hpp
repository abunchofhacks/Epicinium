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


// TODO Can we avoid using a macro while keeping this file header only?
#define IMPLEMENT_CHH_METHOD_VOID(SIGNATURE, CALL) \
	virtual void SIGNATURE override \
	{ \
		for (auto* handler : _handlers) handler->CALL; \
	}
// There should not be any non-void method because there is no conclusive way
// to merge the result from multiple calls.

class ClientHandlerHandler : public ClientHandler
{
private:
	std::vector<ClientHandler*> _handlers;

public:
	virtual void debugHandler() const override
	{
		for (const ClientHandler* handler : _handlers)
		{
			handler->debugHandler();
		}
	}

	virtual void registerHandler(ClientHandler* handler) override
	{
		_handlers.emplace_back(handler);
	}

	virtual void deregisterHandler(ClientHandler* handler) override
	{
		_handlers.erase(
			std::remove(_handlers.begin(), _handlers.end(), handler),
			_handlers.end());
	}

	IMPLEMENT_CHH_METHOD_VOID(
		message(const std::string& m),
		message(m));
	IMPLEMENT_CHH_METHOD_VOID(
		chat(const std::string& u, const std::string& m, const Target& t),
		chat(u, m, t));

	IMPLEMENT_CHH_METHOD_VOID(
		addUser(const std::string& u, const Json::Value& m, bool i),
		addUser(u, m, i));
	IMPLEMENT_CHH_METHOD_VOID(
		removeUser(const std::string& u),
		removeUser(u));
	IMPLEMENT_CHH_METHOD_VOID(
		removeLobby(const std::string& l),
		removeLobby(l));
	IMPLEMENT_CHH_METHOD_VOID(
		joinsLobby(const std::string& s),
		joinsLobby(s));
	IMPLEMENT_CHH_METHOD_VOID(
		inGame(const std::string& l, const std::string& s, const Role& r),
		inGame(l, s, r));
	IMPLEMENT_CHH_METHOD_VOID(
		leavesLobby(const std::string& s),
		leavesLobby(s));
	IMPLEMENT_CHH_METHOD_VOID(
		listLobby(const std::string& l, const std::string& n, const Json::Value& m),
		listLobby(l, n, m));
	IMPLEMENT_CHH_METHOD_VOID(
		listOwnLobby(const std::string& n, const Json::Value& m),
		listOwnLobby(n, m));
	IMPLEMENT_CHH_METHOD_VOID(
		joinsOwnLobby(const std::string& s, bool i),
		joinsOwnLobby(s, i));
	IMPLEMENT_CHH_METHOD_VOID(
		leavesOwnLobby(const std::string& s),
		leavesOwnLobby(s));
	IMPLEMENT_CHH_METHOD_VOID(
		assignHost(const std::string& u, bool i),
		assignHost(u, i));
	IMPLEMENT_CHH_METHOD_VOID(
		assignRole(const std::string& u, const Role& r, bool i),
		assignRole(u, r, i));
	IMPLEMENT_CHH_METHOD_VOID(
		assignColor(const std::string& u, const Player& c),
		assignColor(u, c));
	IMPLEMENT_CHH_METHOD_VOID(
		assignVisionType(const std::string& u, const VisionType& t),
		assignVisionType(u, t));
	IMPLEMENT_CHH_METHOD_VOID(
		assignAI(const std::string& u, const std::string& n),
		assignAI(u, n));
	IMPLEMENT_CHH_METHOD_VOID(
		assignDifficulty(const std::string& u, Difficulty d),
		assignDifficulty(u, d));
	IMPLEMENT_CHH_METHOD_VOID(
		pickMap(const std::string& n),
		pickMap(n));
	IMPLEMENT_CHH_METHOD_VOID(
		pickTimer(uint32_t t),
		pickTimer(t));
	IMPLEMENT_CHH_METHOD_VOID(
		pickReplay(const std::string& n),
		pickReplay(n));
	IMPLEMENT_CHH_METHOD_VOID(
		pickChallenge(const std::string& n),
		pickChallenge(n));
	IMPLEMENT_CHH_METHOD_VOID(
		pickRuleset(const std::string& n),
		pickRuleset(n));
	IMPLEMENT_CHH_METHOD_VOID(
		addBot(const std::string& n),
		addBot(n));
	IMPLEMENT_CHH_METHOD_VOID(
		removeBot(const std::string& n),
		removeBot(n));
	IMPLEMENT_CHH_METHOD_VOID(
		listMap(const std::string& n, const Json::Value& m),
		listMap(n, m));
	IMPLEMENT_CHH_METHOD_VOID(
		listReplay(const std::string& n, const Json::Value& m),
		listReplay(n, m));
	IMPLEMENT_CHH_METHOD_VOID(
		listRuleset(const std::string& n, const Json::Value& m),
		listRuleset(n, m));
	IMPLEMENT_CHH_METHOD_VOID(
		listAI(const std::string& n, const Json::Value& m),
		listAI(n, m));
	IMPLEMENT_CHH_METHOD_VOID(
		listChallenge(const std::string& n, const Json::Value& m),
		listChallenge(n, m));
	IMPLEMENT_CHH_METHOD_VOID(
		receiveSecrets(const Json::Value& m),
		receiveSecrets(m));
	IMPLEMENT_CHH_METHOD_VOID(
		startGame(const Role& r),
		startGame(r));
	IMPLEMENT_CHH_METHOD_VOID(
		startTutorial(),
		startTutorial());
	IMPLEMENT_CHH_METHOD_VOID(
		startHostedGame(),
		startHostedGame());
	IMPLEMENT_CHH_METHOD_VOID(
		setSkins(const Json::Value& m),
		setSkins(m));
	IMPLEMENT_CHH_METHOD_VOID(
		receiveBriefing(const Json::Value& m),
		receiveBriefing(m));

	IMPLEMENT_CHH_METHOD_VOID(
		updateOwnRating(float r),
		updateOwnRating(r));
	IMPLEMENT_CHH_METHOD_VOID(
		updateRating(const std::string& u, float r),
		updateRating(u, r));
	IMPLEMENT_CHH_METHOD_VOID(
		updateStars(const std::string& u, int s),
		updateStars(u, s));
	IMPLEMENT_CHH_METHOD_VOID(
		updateRecentStars(const std::string& k, int s),
		updateRecentStars(k, s));

	IMPLEMENT_CHH_METHOD_VOID(
		displayRankings(const std::vector<Ranking>& r),
		displayRankings(r));

	IMPLEMENT_CHH_METHOD_VOID(
		displayStamp(const std::string& i, const std::string& t, const std::string& u),
		displayStamp(i, t, u));

	IMPLEMENT_CHH_METHOD_VOID(
		inServer(),
		inServer());
	IMPLEMENT_CHH_METHOD_VOID(
		outServer(),
		outServer());
	IMPLEMENT_CHH_METHOD_VOID(
		inLobby(const std::string& l),
		inLobby(l));
	IMPLEMENT_CHH_METHOD_VOID(
		outLobby(),
		outLobby());

	IMPLEMENT_CHH_METHOD_VOID(
		promptUsername(),
		promptUsername());
	IMPLEMENT_CHH_METHOD_VOID(
		usernameTaken(),
		usernameTaken());
	IMPLEMENT_CHH_METHOD_VOID(
		identified(),
		identified());

	IMPLEMENT_CHH_METHOD_VOID(
		loggingIn(),
		loggingIn());
	IMPLEMENT_CHH_METHOD_VOID(
		loggedIn(),
		loggedIn());
	IMPLEMENT_CHH_METHOD_VOID(
		loggedOut(),
		loggedOut());
	IMPLEMENT_CHH_METHOD_VOID(
		loginFailed(const ResponseStatus& r),
		loginFailed(r));
	IMPLEMENT_CHH_METHOD_VOID(
		accessingPortal(),
		accessingPortal());
	IMPLEMENT_CHH_METHOD_VOID(
		accessedPortal(),
		accessedPortal());
	IMPLEMENT_CHH_METHOD_VOID(
		accessingPortalFailed(const PortalStatus& x),
		accessingPortalFailed(x));
	IMPLEMENT_CHH_METHOD_VOID(
		registering(),
		registering());
	IMPLEMENT_CHH_METHOD_VOID(
		registered(),
		registered());
	IMPLEMENT_CHH_METHOD_VOID(
		registrationFailed(const ResponseStatus& r),
		registrationFailed(r));
	IMPLEMENT_CHH_METHOD_VOID(
		sessionFailed(const ResponseStatus& r),
		sessionFailed(r));
	IMPLEMENT_CHH_METHOD_VOID(
		activatingKey(),
		activatingKey());
	IMPLEMENT_CHH_METHOD_VOID(
		activatedKey(),
		activatedKey());
	IMPLEMENT_CHH_METHOD_VOID(
		activatingKeyFailed(const ResponseStatus& r),
		activatingKeyFailed(r));
	IMPLEMENT_CHH_METHOD_VOID(
		resettingPassword(),
		resettingPassword());
	IMPLEMENT_CHH_METHOD_VOID(
		resetPassword(),
		resetPassword());
	IMPLEMENT_CHH_METHOD_VOID(
		resetPasswordFailed(const ResponseStatus& r),
		resetPasswordFailed(r));
	IMPLEMENT_CHH_METHOD_VOID(
		sendingFeedback(),
		sendingFeedback());
	IMPLEMENT_CHH_METHOD_VOID(
		sentFeedback(const std::string& l),
		sentFeedback(l));
	IMPLEMENT_CHH_METHOD_VOID(
		feedbackFailed(const ResponseStatus& r),
		feedbackFailed(r));

	IMPLEMENT_CHH_METHOD_VOID(
		connecting(),
		connecting());
	IMPLEMENT_CHH_METHOD_VOID(
		connected(),
		connected());
	IMPLEMENT_CHH_METHOD_VOID(
		disconnected(),
		disconnected());
	IMPLEMENT_CHH_METHOD_VOID(
		systemFailure(),
		systemFailure());
	IMPLEMENT_CHH_METHOD_VOID(
		serverOffline(),
		serverOffline());
	IMPLEMENT_CHH_METHOD_VOID(
		serverShutdown(),
		serverShutdown());
	IMPLEMENT_CHH_METHOD_VOID(
		serverClosing(),
		serverClosing());
	IMPLEMENT_CHH_METHOD_VOID(
		versionMismatch(const Version& v),
		versionMismatch(v));
	IMPLEMENT_CHH_METHOD_VOID(
		patchAvailable(const Version& v),
		patchAvailable(v));
	IMPLEMENT_CHH_METHOD_VOID(
		patchDownloading(),
		patchDownloading());
	IMPLEMENT_CHH_METHOD_VOID(
		patchPrimed(const Version& v),
		patchPrimed(v));
	IMPLEMENT_CHH_METHOD_VOID(
		patchFailed(),
		patchFailed());
	IMPLEMENT_CHH_METHOD_VOID(
		patchContinuing(const Version& v),
		patchContinuing(v));
	IMPLEMENT_CHH_METHOD_VOID(
		patchContinued(const Version& v),
		patchContinued(v));
	IMPLEMENT_CHH_METHOD_VOID(
		requestFulfilled(const std::string& f),
		requestFulfilled(f));
	IMPLEMENT_CHH_METHOD_VOID(
		requestFailed(const std::string& f),
		requestFailed(f));
	IMPLEMENT_CHH_METHOD_VOID(
		downloading(const std::string& f, float p),
		downloading(f, p));

	IMPLEMENT_CHH_METHOD_VOID(
		activateDiscord(),
		activateDiscord());
	IMPLEMENT_CHH_METHOD_VOID(
		deactivateDiscord(),
		deactivateDiscord());

	IMPLEMENT_CHH_METHOD_VOID(
		checkDiscordReady(),
		checkDiscordReady());

	IMPLEMENT_CHH_METHOD_VOID(
		discordReady(const char* u, const char* d, const std::string& p),
		discordReady(u, d, p));
	IMPLEMENT_CHH_METHOD_VOID(
		discordDeactivated(),
		discordDeactivated());
	IMPLEMENT_CHH_METHOD_VOID(
		discordDisconnected(int e, const char* m),
		discordDisconnected(e, m));
	IMPLEMENT_CHH_METHOD_VOID(
		discordError(int e, const char* m),
		discordError(e, m));
	IMPLEMENT_CHH_METHOD_VOID(
		discordJoinRequest(const char* u, const char* d, const std::string& p),
		discordJoinRequest(u, d, p));

	IMPLEMENT_CHH_METHOD_VOID(
		acceptJoinRequest(),
		acceptJoinRequest());
	IMPLEMENT_CHH_METHOD_VOID(
		denyJoinRequest(),
		denyJoinRequest());
	IMPLEMENT_CHH_METHOD_VOID(
		ignoreJoinRequest(),
		ignoreJoinRequest());

	IMPLEMENT_CHH_METHOD_VOID(
		hotJoin(const std::string& s),
		hotJoin(s));
	IMPLEMENT_CHH_METHOD_VOID(
		hotSpectate(const std::string& s),
		hotSpectate(s));

	IMPLEMENT_CHH_METHOD_VOID(
		steamConnecting(),
		steamConnecting());
	IMPLEMENT_CHH_METHOD_VOID(
		steamConnected(const char* n, const std::string& a),
		steamConnected(n, a));
	IMPLEMENT_CHH_METHOD_VOID(
		steamDisconnected(),
		steamDisconnected());
	IMPLEMENT_CHH_METHOD_VOID(
		steamMissing(),
		steamMissing());
	IMPLEMENT_CHH_METHOD_VOID(
		steamAccountMergeTokenResult(const ResponseStatus& r),
		steamAccountMergeTokenResult(r));

	IMPLEMENT_CHH_METHOD_VOID(
		openWorkshopForMap(const std::string& n),
		openWorkshopForMap(n));
	IMPLEMENT_CHH_METHOD_VOID(
		openWorkshopForRuleset(const std::string& n),
		openWorkshopForRuleset(n));
	IMPLEMENT_CHH_METHOD_VOID(
		openWorkshopForPalette(const std::string& n),
		openWorkshopForPalette(n));
	IMPLEMENT_CHH_METHOD_VOID(
		closeAllWorkshops(),
		closeAllWorkshops());

	IMPLEMENT_CHH_METHOD_VOID(
		openUrl(const std::string& u),
		openUrl(u));

	IMPLEMENT_CHH_METHOD_VOID(
		sendStomt(bool p, const std::string& t),
		sendStomt(p, t));

	IMPLEMENT_CHH_METHOD_VOID(
		takeScreenshot(std::weak_ptr<Screenshot> s),
		takeScreenshot(s));
	IMPLEMENT_CHH_METHOD_VOID(
		takeScreenshotOfMap(),
		takeScreenshotOfMap());
	IMPLEMENT_CHH_METHOD_VOID(
		screenshotTaken(std::weak_ptr<Screenshot> s),
		screenshotTaken(s));
};
