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

#include "menu.hpp"
#include "clienthandler.hpp"

enum class InputMode;
class Paint;


class MultiplayerMenu final : public Menu, public ClientHandler
{
private:
	std::unique_ptr<InterfaceElement> makeMapDropdown();
	std::unique_ptr<InterfaceElement> makeRulesetDropdown();
	std::unique_ptr<InterfaceElement> makeTimerDropdown();

	static std::string formatRating(float rating);

public:
	using Menu::Menu;
	virtual ~MultiplayerMenu();

private:
	InputMode _inputMode;
	std::string _mapBuffer;
	std::string _ownLobby = "";
	std::string _username;
	std::vector<std::string> _ainames; // (married)
	std::vector<std::string> _aidescriptions; // (married)
	bool _isSelfHosting = false;
	std::vector<std::string> _challengekeys; // (married)
	std::vector<int> _challengestars; // (married)
	bool _linkWasHovered = false;

	void addPlayer(const std::string& username, bool isSelf);
	void addObserver(const std::string& username, bool isSelf);

	void addAddBot();

	void resetMapDropdown();
	void straightenLobbySettings();
	void restrictLobbySettingsForOneVsOne();
	void rethinkLobbySettingsForSelfHostedContent();
	void reloadChallengeList();

	void becomeGameLobby();
	void becomeReplayLobby();

	bool joinSpectatable(bool dryrun = false);
	bool joinOneVsOne();
	bool joinFreeForAll();

	std::unique_ptr<InterfaceElement> makePanel(const std::string& text,
		const std::string& background, const Paint& framecolor);
	std::unique_ptr<InterfaceElement> makeChallengePanel(
		const std::string& text, int maxStars);

public:
	virtual void build() override;
	virtual void refresh() override;

	virtual void onOpen() override;
	virtual void onKill() override;

	virtual void onShow() override;
	virtual void onHide() override;

	virtual void debugHandler() const override;

	virtual void message(const std::string& message) override;
	virtual void chat(const std::string& user, const std::string& message,
		const Target& target) override;

	virtual void addUser(const std::string& user, const Json::Value& metadata, bool isSelf) override;
	virtual void removeUser(const std::string& user) override;
	void addLobby(const std::string& lobby);
	virtual void removeLobby(const std::string& lobby) override;
	virtual void joinsLobby(const std::string& sender) override;
	virtual void inGame(const std::string& lobby, const std::string& sender, const Role& role) override;
	virtual void leavesLobby(const std::string& sender) override;
	virtual void listLobby(const std::string& lobby, const std::string& name, const Json::Value& metadata) override;
	void nameLobby(const std::string& lobby,
		const std::string& name);
	void tagLobby(const std::string& lobby,
		const std::string& name);
	void lockLobby(const std::string& lobby);
	void unlockLobby(const std::string& lobby);

	virtual void joinsOwnLobby(const std::string& sender, bool isSelf) override;
	virtual void leavesOwnLobby(const std::string& sender) override;
	void nameOwnLobby(const std::string& name);
	void lockOwnLobby();
	void unlockOwnLobby();
	void maxPlayers(const std::string& lobby, uint32_t count);
	void numPlayers(const std::string& lobby, uint32_t count);
	virtual void assignHost(const std::string& user, bool isSelf) override;
	virtual void assignRole(const std::string& user, const Role& role, bool isSelf) override;
	virtual void assignColor(const std::string& user, const Player& color) override;
	virtual void assignVisionType(const std::string& user, const VisionType& type) override;
	virtual void assignAI(const std::string& bot, const std::string& ainame) override;
	virtual void assignDifficulty(const std::string& bot, Difficulty difficulty) override;
	virtual void pickMap(const std::string& mapname) override;
	virtual void pickTimer(uint32_t timer) override;
	virtual void pickReplay(const std::string& mapname) override;
	virtual void pickRuleset(const std::string& rulesetname) override;
	virtual void addBot(const std::string& name) override;
	virtual void removeBot(const std::string& name) override;
	virtual void listMap(const std::string& name, const Json::Value& metadata) override;
	virtual void listReplay(const std::string& name, const Json::Value& metadata) override;
	virtual void listRuleset(const std::string& name, const Json::Value& metadata) override;
	virtual void listAI(const std::string& name, const Json::Value& metadata) override;
	virtual void listChallenge(const std::string& name, const Json::Value& metadata) override;

	virtual void startHostedGame() override;

	virtual void updateOwnRating(float rating) override;
	virtual void updateRating(const std::string& name, float rating) override;
	virtual void updateStars(const std::string& name, int stars) override;
	virtual void updateRecentStars(const std::string& key, int stars) override;

	virtual void displayRankings(const std::vector<Ranking>& rankings) override;

	virtual void inServer() override;
	virtual void outServer() override;
	virtual void inLobby(const std::string& lobby) override;
	virtual void outLobby() override;

	virtual void identified() override;
	virtual void sessionFailed(const ResponseStatus&) override;
	virtual void sendingFeedback() override;
	virtual void sentFeedback(const std::string& link) override;
	virtual void feedbackFailed(const ResponseStatus& responsestatus) override;

	virtual void requestFulfilled(const std::string& filename) override;

	virtual void serverClosing() override;

	virtual void discordJoinRequest(const char* username, const char* discriminator, const std::string& picturename) override;

	virtual void hotJoin(const std::string& secret) override;
	virtual void hotSpectate(const std::string& secret) override;
};
