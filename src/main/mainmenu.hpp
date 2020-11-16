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
 * Can Ur (canur11@gmail.com)
 */
#pragma once
#include "header.hpp"

#include "menu.hpp"
#include "clienthandler.hpp"


class MainMenu final : public Menu, public ClientHandler
{
public:
	MainMenu(Owner& owner, GameOwner& gameowner);
	virtual ~MainMenu();

private:

	enum class ConnectionStatus : uint8_t
	{
		NONE,
		ACCESSING_PORTAL,
		CONNECTING,
		CONNECTED,
		PATCH_AVAILABLE,
		PATCH_DOWNLOADING,
		PATCH_PRIMED,
		PATCH_FAILED,
		RESETTING,
	};

	enum class LoginStatus : uint8_t
	{
		NONE,
		LOGGINGIN,
		LOGGEDIN,
		REGISTERING,
		REGISTERED,
		RESETTING_PW,
		RESET_PW,
		ACTIVATING_KEY,
		DISABLED,
	};

	enum class SteamStatus : uint8_t
	{
		NONE,
		CONNECTING,
		CONNECTED,
	};

	enum class DiscordStatus : uint8_t
	{
		NONE,
		CONNECTED,
	};

	ConnectionStatus _connectionStatus;
	LoginStatus _loginStatus;
	SteamStatus _steamStatus;
	DiscordStatus _discordStatus;
	bool _linkWasHovered;

	std::string _toForm = "";

	void resetting();
	void evaluatePlayable();

	bool hasLoginNub();
	bool hasSteamNub();
	bool hasDiscordNub();

	InterfaceElement& getForms();
	InterfaceElement& getLogo();
	InterfaceElement& getStamp();
	InterfaceElement& getEnterKeyButton();
	InterfaceElement& getFeedbackButton();
	InterfaceElement& getPlayOrDownloading();
	InterfaceElement& getPlay();
	InterfaceElement& getPlayButton();
	InterfaceElement& getConnectionNub();
	InterfaceElement& getLoginNub();
	InterfaceElement& getSteamNub();
	InterfaceElement& getDiscordNub();
	InterfaceElement& getSettingsButton();
	InterfaceElement& getCreditsButton();
	InterfaceElement& getQuitButton();
	InterfaceElement& getLoginButton();
	InterfaceElement& getLogoutButton();
	InterfaceElement& getRegisterButton();
	InterfaceElement& getCenteredError();

	static std::unique_ptr<InterfaceElement> makeButtonLarge(const std::string& text, int fontsize);
	static std::unique_ptr<InterfaceElement> makeButtonWide(const std::string& text, int fontsize);
	static std::unique_ptr<InterfaceElement> makeButtonSmall(const std::string& text, int fontsize);
	static std::unique_ptr<InterfaceElement> makeButtonTiny(const std::string& text, int fontsize);

	std::unique_ptr<InterfaceElement> makeUsernameInput();
	std::unique_ptr<InterfaceElement> makeEmailInput();
	std::unique_ptr<InterfaceElement> makePasswordInput();
	std::unique_ptr<InterfaceElement> makePassword2Input();
	std::unique_ptr<InterfaceElement> makePasswordResetTokenInput();
	std::unique_ptr<InterfaceElement> makeKeyTokenInput();

public:
	virtual void build() override;
	virtual void beforeFirstRefreshOfEachSecond() override;
	virtual void refresh() override;

	virtual void onOpen() override;
	virtual void onKill() override;
	virtual void onHide() override;

	virtual void debugHandler() const override;

	virtual void message(const std::string& message) override;

	virtual void displayStamp(const std::string& image,
		const std::string& tooltip, const std::string& url) override;

	virtual void connecting() override;
	virtual void connected() override;
	virtual void disconnected() override;
	virtual void systemFailure() override;
	virtual void serverOffline() override;
	virtual void serverShutdown() override;
	virtual void serverClosing() override;
	virtual void versionMismatch(const Version& version) override;
	virtual void patchAvailable(const Version& version) override;
	virtual void patchDownloading() override;
	virtual void patchPrimed(const Version& version) override;
	virtual void patchFailed() override;
	virtual void patchContinuing(const Version& version) override;
	virtual void patchContinued(const Version& version) override;
	virtual void downloading(const std::string& filename, float perc) override;

	virtual void accessingPortal() override;
	virtual void accessedPortal() override;
	virtual void accessingPortalFailed(const PortalStatus& status) override;

	virtual void loggingIn() override;
	virtual void loggedIn() override;
	virtual void loggedOut() override;
	virtual void loginFailed(const ResponseStatus& responsestatus) override;
	virtual void registering() override;
	virtual void registered() override;
	virtual void registrationFailed(const ResponseStatus& responsestatus) override;
	virtual void sessionFailed(const ResponseStatus& responsestatus) override;
	virtual void activatingKey() override;
	virtual void activatedKey() override;
	virtual void activatingKeyFailed(const ResponseStatus& responsestatus) override;
	virtual void resettingPassword() override;
	virtual void resetPassword() override;
	virtual void resetPasswordFailed(const ResponseStatus& responsestatus) override;
	virtual void sendingFeedback() override;
	virtual void sentFeedback(const std::string& link) override;
	virtual void feedbackFailed(const ResponseStatus& responsestatus) override;

	virtual void requestFulfilled(const std::string& filename) override;

	virtual void discordReady(const char* username, const char* discriminator, const std::string& picturename) override;
	virtual void discordDeactivated() override;
	virtual void discordDisconnected(int errorcode, const char* message) override;
	virtual void discordError(int errorcode, const char* message) override;

	virtual void hotJoin(const std::string& secret) override;
	virtual void hotSpectate(const std::string& secret) override;

	virtual void steamConnecting() override;
	virtual void steamConnected(const char* personaname,
		const std::string& avatarpicturename) override;
	virtual void steamDisconnected() override;
	virtual void steamMissing() override;
	virtual void steamAccountMergeTokenResult(
		const ResponseStatus& responsestatus) override;
};
