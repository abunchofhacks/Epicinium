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

#include "engine.hpp"
#include "client.hpp"
#include "clienthandlerhandler.hpp"
#include "epicdn.hpp"
#include "discordrpc.hpp"
#include "stomt.hpp"
#include "mainmenu.hpp"
#include "gameowner.hpp"
#include "dictator.hpp"
#include "messageinjector.hpp"

class Steam;


class Main final
	: public Engine
	, public ClientHandlerHandler
	, public Menu::Owner
	, public GameOwner
{
public:
	Main(Settings& settings);
	Main(const Main&) = delete;
	Main(Main&&) = delete;
	Main& operator=(const Main&) = delete;
	Main& operator=(Main&&) = delete;
	virtual ~Main();

private:
	Dictator _dictator;
	MessageInjector _messageinjector;
	Client _client;
	EpiCDN _epiCDN;
	DiscordRPC _discordRPC;
	Stomt _stomt;
	MainMenu _menu;

	std::unique_ptr<Steam> _steam;
	imploding_ptr<HostedGame> _hosted;

	virtual void doFirst() override;

	virtual void startUpdates() override;
	virtual void endUpdates() override;

	virtual void quit(ExitCode exitcode) override;

	virtual void getPicture(const std::string& name) override;

	virtual std::string activePaletteName() const override;
	virtual void openPaletteEditor(const std::string& palettename) override;

	virtual void openUrl(const std::string& text) override;

	virtual void sendStomt(bool positive, const std::string& text) override;

	virtual void takeScreenshot(std::weak_ptr<Screenshot> screenshot) override;

	virtual bool isTakingScreenshot() override;

public:
	using ClientHandlerHandler::startGame;
	virtual std::weak_ptr<Game> startGame(imploding_ptr<Game> game) override;
	virtual std::weak_ptr<Game> startChallenge(
		const Challenge& challenge) override;
	virtual std::weak_ptr<Game> startGame(
		const Player& player, const std::string& rulesetname,
		uint32_t planningTime) override;
	using ClientHandlerHandler::startTutorial;
	virtual std::weak_ptr<Game> startTutorial(
		const Player& player, const std::string& rulesetname,
		uint32_t planningTime) override;
	virtual std::weak_ptr<Game> startReplay(
		const Role& role, const std::string& rulesetname,
		uint32_t planningTime) override;
	virtual std::weak_ptr<Game> startDiorama() override;

	virtual std::weak_ptr<HostedGame> hostGame(
		std::shared_ptr<Challenge> challenge,
		const std::vector<Player>& colors,
		const std::vector<VisionType>& visiontypes,
		const std::vector<std::string>& usernames,
		const std::vector<Bot>& bots,
		bool hasObservers,
		const std::string& mapname, const std::string& rulesetname) override;

	virtual void stopGame() override;

	Settings& settings() override { return _settings; }
	DisplaySettings& displaysettings() override { return _displaysettings; }
	Client& client() override { return _client; }
};
