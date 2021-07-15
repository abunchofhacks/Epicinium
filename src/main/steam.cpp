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
#include "steam.hpp"
#include "source.hpp"

#if STEAM_ENABLED
/* ############################# STEAM_ENABLED ############################# */

#include <mutex>

#include "libs/SDL2/SDL_image.h"
#include "libs/SDL2/SDL2_gfxPrimitives.h"
#include "libs/stb/stb_image_write.h"

#include "client.hpp"
#include "system.hpp"
#include "locator.hpp"
#include "role.hpp"
#include "screenshot.hpp"
#include "keycode.hpp"
#include "clock.hpp"
#include "camera.hpp"
#include "graphics.hpp"
#include "map.hpp"
#include "palette.hpp"
#include "download.hpp"
#include "writer.hpp"
#include "colorname.hpp"
#include "openurl.hpp"
#include "validation.hpp"
#include "parseerror.hpp"


static std::mutex _singleton_mutex;
constexpr uint32_t APPLICATION_ID = 1286730;

extern "C" void __cdecl steam_log_callback(int severity, const char* text);

Steam::Steam(Client& client) :
	_client(client)
{
	// If we're creating this Steam, of which there can only be one.
	// We lock a mutex just to prove this to ourselves.
	assert(_singleton_mutex.try_lock());

	_client.registerHandler(this);
}

Steam::~Steam()
{
	if (_serverSessionTicket.handle() != k_HAuthTicketInvalid)
	{
		auto oldhandle = _serverSessionTicket.handle();
		LOGD << "Cancelling ticket " << (int) oldhandle << ".";
		SteamUser()->CancelAuthTicket(oldhandle);
	}

	SteamAPI_Shutdown();

	_client.deregisterHandler(this);

	_singleton_mutex.unlock();
}

bool Steam::shouldRestart()
{
	return SteamAPI_RestartAppIfNecessary(APPLICATION_ID);
}

std::unique_ptr<Steam> Steam::create(Client& client)
{
	if (SteamAPI_Init())
	{
		LOGI << "Initialized Steam integration.";
	}
	else
	{
		LOGE << "Steam is not running, or failed to detect steam.";
		client.steamMissing();
		return nullptr;
	}

	SteamUtils()->SetWarningMessageHook(&steam_log_callback);

	std::unique_ptr<Steam> ptr;
	ptr.reset(new Steam(client));

	ptr->ready();

	return ptr;
}

void Steam::ready()
{
	_serverSessionTicket.retrieve();
	if (_serverSessionTicket.handle() == k_HAuthTicketInvalid)
	{
		_client.steamDisconnected();
	}
	else
	{
		_client.steamConnecting();
	}

	clearLobbyInfo();
	updatePresence();
}

void Steam::update()
{
	SteamAPI_RunCallbacks();

	updateWorkshop();
}

void Steam::clearLobbyInfo()
{
	_lobbyId = "";
	_lobbyType = "";
	_lobbyMemberUsernames.clear();
	_maxPlayersAndBots = 0;
	_numPlayersAndBots = 0;
	_numBots = 0;
	_numPlayers = 0;
	_isPlaying = false;
	_isSpectating = false;

	_groupId = "";
	_groupSize = "";

	_connectCommand = "";

	if (_isOnline)
	{
		_legacyStatus = "Looking to Play";
		_localizedStatus = "#looking_to_play";
	}
	else
	{
		_legacyStatus = "In Menu";
		_localizedStatus = "#in_menu";
	}
}

void Steam::updateLobbyInfo()
{
	if (_lobbyId.empty())
	{
		// Wait for a subsequent ::inLobby() call to update the lobby info.
		return;
	}

	_groupId = _lobbyId;
	_groupSize = std::to_string(_lobbyMemberUsernames.size());

	if (_lobbyType == "tutorial")
	{
		_legacyStatus = "Playing Tutorial";
		_localizedStatus = "#playing_tutorial";
	}
	else if (_lobbyType == "challenge")
	{
		_legacyStatus = "Playing Challenge";
		_localizedStatus = "#playing_challenge";
	}
	else if (_lobbyType == "replay")
	{
		_legacyStatus = "Watching Replay";
		_localizedStatus = "#watching_replay";
	}
	else
	{
		std::string typedesc;
		if (_maxPlayersAndBots == 2 && _numBots == 0)
		{
			typedesc = "One vs One";
			_lobbyTypeKey = "one_vs_one";
		}
		else if (_maxPlayersAndBots > 2 && _numBots < _maxPlayersAndBots - 1)
		{
			typedesc = "Free For All";
			_lobbyTypeKey = "free_for_all";
		}
		else if (_numPlayers >= 1)
		{
			typedesc = "Versus AI";
			_lobbyTypeKey = "versus_ai";
		}
		else
		{
			typedesc = "AI vs AI";
			_lobbyTypeKey = "ai_vs_ai";
		}

		if (_isPlaying)
		{
			_legacyStatus = "Playing " + typedesc;
			_localizedStatus = "#playing";
		}
		else if (_isSpectating)
		{
			_legacyStatus = "Spectating " + typedesc;
			_localizedStatus = "#spectating";
		}
		else
		{
			_lobbyCurSize = std::to_string(_numPlayersAndBots);
			_lobbyMaxSize = std::to_string(_maxPlayersAndBots);
			_legacyStatus = "In " + typedesc + " Lobby"
				" (" + _lobbyCurSize + "/" + _lobbyMaxSize + ")";
			_localizedStatus = "#in_lobby";
		}
	}
}

void Steam::updatePresence()
{
	SteamFriends()->ClearRichPresence();
	if (!_groupId.empty())
	{
		SteamFriends()->SetRichPresence("steam_player_group", _groupId.c_str());
		SteamFriends()->SetRichPresence("steam_player_group_size",
			_groupSize.c_str());
	}
	if (!_legacyStatus.empty())
	{
		SteamFriends()->SetRichPresence("status", _legacyStatus.c_str());
	}
	if (!_localizedStatus.empty())
	{
		SteamFriends()->SetRichPresence("steam_display",
			_localizedStatus.c_str());
	}
	if (!_lobbyTypeKey.empty())
	{
		SteamFriends()->SetRichPresence("lobby_type_key",
			_lobbyTypeKey.c_str());
	}
	if (!_lobbyCurSize.empty())
	{
		SteamFriends()->SetRichPresence("lobby_cur_size",
			_lobbyCurSize.c_str());
	}
	if (!_lobbyMaxSize.empty())
	{
		SteamFriends()->SetRichPresence("lobby_max_size",
			_lobbyMaxSize.c_str());
	}
	if (!_connectCommand.empty())
	{
		SteamFriends()->SetRichPresence("connect",
			_connectCommand.c_str());
	}
}

void Steam::SessionTicket::retrieve()
{
	_handle = SteamUser()->GetAuthSessionTicket((void*) _buffer,
		array_size(_buffer), &_length);
	if (_handle == k_HAuthTicketInvalid)
	{
		LOGE << "Failed to generate ticket.";
	}
	else
	{
		LOGD << "Validating ticket " << (int) _handle << "...";
	}
}

void Steam::handleAuthSessionTicketResponse(
	GetAuthSessionTicketResponse_t* response)
{
	auto handle = response->m_hAuthTicket;
	if (handle == _serverSessionTicket.handle())
	{
		if (response->m_eResult == k_EResultOK)
		{
			LOGD << "Successfully validated ticket " << (int) handle << ".";
			std::stringstream strm;
			strm << std::hex << std::setfill('0');
			for (uint8_t x : _serverSessionTicket.data())
			{
				strm << std::setw(2) << (int) x;
			}
			std::string ticket = strm.str();

			CSteamID steamid = SteamUser()->GetSteamID();

			std::string avatarpicturename = getAvatarPictureName(steamid);

			_client.steamConnected(SteamFriends()->GetPersonaName(),
				avatarpicturename, steamid.ConvertToUint64(), ticket);

			handleUrlLaunchParameters(nullptr);
			refreshSubscribedWorkshopItems();
			retrievePublishedWorkshopItems();
		}
		else
		{
			LOGE << "EResult = " << (int) response->m_eResult;

			_client.steamDisconnected();
		}
	}
	else
	{
		if (response->m_eResult != k_EResultOK)
		{
			LOGE << "EResult = " << (int) response->m_eResult;
		}
		LOGD << "Discarding ticket " << (int) handle << ".";
		SteamUser()->CancelAuthTicket(handle);
	}
}

std::string Steam::getAvatarPictureName(CSteamID steamid)
{
	int imagehandle = SteamFriends()->GetSmallFriendAvatar(steamid);
	std::string avatarpicturename = "steam/avatars/"
		+ std::to_string(steamid.ConvertToUint64()) + "/small";
	std::string filename = Locator::pictureFilename(avatarpicturename);

	if (std::find(_cachedImageHandles.begin(),
			_cachedImageHandles.end(), imagehandle)
		== _cachedImageHandles.end())
	{
		LOGD << "Getting " << avatarpicturename << " from Steam...";
		uint32_t width = 0;
		uint32_t height = 0;
		if (SteamUtils()->GetImageSize(imagehandle, &width, &height))
		{
			// According to the Steam SDK documentation, GetImageRGBA will
			// return the data in "RGBA format", which I assume means that the
			// first 8 bits represent the red value of the top left pixel.
			// Therefore I would say that the masks passed to SDL should
			// always be the big-endian masks. However that does not seem to
			// work (on my machine), so maybe I just don't understand SDL.
			uint32_t rmask, gmask, bmask, amask;
			#if SDL_BYTEORDER == SDL_BIG_ENDIAN
				rmask = 0xff000000;
				gmask = 0x00ff0000;
				bmask = 0x0000ff00;
				amask = 0x000000ff;
			#else
				rmask = 0x000000ff;
				gmask = 0x0000ff00;
				bmask = 0x00ff0000;
				amask = 0xff000000;
			#endif

			// Write the image (probably obtained from a PNG inside Steam)
			// onto an SDL surface, then write it to a PNG, so that later
			// we can load it back onto a surface. Oh well.
			SDL_Surface* surface = SDL_CreateRGBSurface(0, width, height, 32,
				rmask, gmask, bmask, amask);
			if (surface == nullptr)
			{
				LOGE << "Failed to create surface: " << SDL_GetError();
			}
			else
			{
				SteamUtils()->GetImageRGBA(imagehandle,
					(uint8_t*) surface->pixels,
					surface->h * surface->pitch);
				System::touchFile(filename);
				IMG_SavePNG(surface, filename.c_str());
				SDL_FreeSurface(surface);
				LOGD << "Saved " << filename << ".";
			}
			_cachedImageHandles.push_back(imagehandle);
		}
		else
		{
			LOGW << "Failed to load avatar.";
		}
	}

	// Also pass the correct name in case of an error, because it might have
	// been written to PNG before.
	return avatarpicturename;
}

void Steam::handleUrlLaunchParameters(NewUrlLaunchParameters_t* /*nullable*/)
{
	std::string line;
	{
		std::vector<char> buf;
		buf.resize(1024);
		int len = SteamApps()->GetLaunchCommandLine(buf.data(), buf.size());
		if (len >= 0)
		{
			line = std::string(buf.data(), len);
		}
		else
		{
			LOGE << "Launched from Steam but failed to get command line";
			return;
		}
	}
	LOGI << "Launched from Steam with: " << line;

	std::string secret;
	{
		constexpr const char* arg = "hot-join-secret=";
		size_t start = line.find(arg);
		if (start != std::string::npos)
		{
			start += strlen(arg);
			size_t end = line.find_first_not_of(
				"0123456789"
				"abcdefghijklmnopqrstuvwxyz"
				"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
				"-"
				"", start);
			if (end != std::string::npos)
			{
				secret = line.substr(start, end - start);
			}
			else
			{
				secret = line.substr(start);
			}
		}
	}
	if (!secret.empty())
	{
		_client.steamJoinGame(secret);
	}
}

void Steam::debugHandler() const
{
	LOGD << ((void*) this);
}

void Steam::disconnected()
{
	if (_serverSessionTicket.handle() != k_HAuthTicketInvalid)
	{
		auto oldhandle = _serverSessionTicket.handle();
		LOGD << "Cancelling ticket " << (int) oldhandle << ".";
		SteamUser()->CancelAuthTicket(oldhandle);

		_serverSessionTicket.retrieve();
		if (_serverSessionTicket.handle() == k_HAuthTicketInvalid)
		{
			_client.steamDisconnected();
		}
		else
		{
			_client.steamConnecting();
		}
	}
}

void Steam::inServer()
{
	_isOnline = true;
	clearLobbyInfo();
	updatePresence();
}

void Steam::outServer()
{
	_isOnline = false;
	clearLobbyInfo();
	updatePresence();
}

void Steam::loggedOut()
{
	outServer();
}

void Steam::inLobby(const std::string& lobbyid)
{
	_lobbyId = lobbyid;
	// Wait for a subsequent ::listOwnLobby() call to update the presence.
}

void Steam::outLobby()
{
	clearLobbyInfo();
	updatePresence();
}

void Steam::listOwnLobby(const std::string&, const Json::Value& metadata)
{
	_lobbyType = "";
	if (metadata["lobby_type"].isString())
	{
		_lobbyType = metadata["lobby_type"].asString();
	}

	_maxPlayersAndBots = 0;
	if (metadata["max_players"].isInt())
	{
		_maxPlayersAndBots = metadata["max_players"].asInt();
	}
	DEBUG_ASSERT(_maxPlayersAndBots >= 0);

	_numPlayersAndBots = 0;
	if (metadata["num_players"].isInt())
	{
		_numPlayersAndBots = metadata["num_players"].asInt();
	}
	DEBUG_ASSERT(_numPlayersAndBots >= 0
		&& _numPlayersAndBots <= _maxPlayersAndBots);
	_numBots = 0;
	if (metadata["num_bot_players"].isInt())
	{
		_numBots = metadata["num_bot_players"].asInt();
	}
	DEBUG_ASSERT(_numBots >= 0 && _numBots <= _numPlayersAndBots);
	_numPlayers = _numPlayersAndBots - _numBots;

	updateLobbyInfo();
	updatePresence();
}

void Steam::joinsOwnLobby(const std::string& name, bool isSelf)
{
	auto found = std::find(_lobbyMemberUsernames.begin(),
		_lobbyMemberUsernames.end(),
		name);
	if (found != _lobbyMemberUsernames.end())
	{
		return;
	}

	_lobbyMemberUsernames.push_back(name);

	if (isSelf)
	{
		// Wait for a subsequent ::listOwnLobby() call to update the presence.
		return;
	}

	updateLobbyInfo();
	updatePresence();
}

void Steam::leavesOwnLobby(const std::string& name)
{
	auto found = std::find(_lobbyMemberUsernames.begin(),
		_lobbyMemberUsernames.end(),
		name);
	if (found == _lobbyMemberUsernames.end())
	{
		return;
	}

	_lobbyMemberUsernames.erase(found);

	updateLobbyInfo();
	updatePresence();
}

void Steam::receiveSecrets(const Json::Value& metadata)
{
	if (metadata["join-secret"].isString())
	{
		std::string secret = metadata["join-secret"].asString();
		// The secret should be 58 characters, but this number might change.
		if (secret.size() < 10 || secret.size() > 100)
		{
			LOGW << "Ignoring invalid join secret: " << secret;
			return;
		}
		size_t ill = secret.find_first_not_of(
			"0123456789"
			"abcdefghijklmnopqrstuvwxyz"
			"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
			"-"
			"");
		if (ill != std::string::npos)
		{
			LOGW << "Ignoring invalid join secret: " << secret;
			return;
		}

		std::stringstream strm;
		strm << "hot-join-secret=" << secret;
		_connectCommand = strm.str();
	}
	// For Steam there is no separate spectate secret, because Steam's Join Game
	// feature is only available for friends and looks the same when the player
	// is already playing a game.

	updatePresence();
}

void Steam::startGame(const Role& role)
{
	if (role == Role::PLAYER)
	{
		_isPlaying = true;
	}
	else
	{
		_isSpectating = true;
	}

	updateLobbyInfo();
	updatePresence();
}

void Steam::startTutorial()
{
	_isPlaying = true;
	updateLobbyInfo();
	updatePresence();
}

inline std::string buildUniqueTag(const char* title, PublishedFileId_t fileId)
{
	std::string tag = title;
	if (!isValidUserContentName(tag))
	{
		LOGE << "Failed to validate item title in time: " << title;
		tag = "Unnamed";
	}
	tag += "@WORKSHOP/" + std::to_string(fileId);
	return tag;
}

bool Steam::resetWorkshopItem(const WorkshopItemType& type)
{
	switch (_workshopItem.state)
	{
		case WorkshopItemState::NONE:
		case WorkshopItemState::READY:
		case WorkshopItemState::CREATION_FAILED:
		case WorkshopItemState::UPLOAD_FAILED:
		{
			_workshopItem.type = type;
			_workshopItem.state = WorkshopItemState::NONE;
			_workshopItem.title[0] = '\0';
			_workshopItem.description[0] = '\0';
			_workshopItem.previewScreenshot.reset();
			_workshopItem.previewScreenshotPath.clear();
			_workshopItem.panelScreenshot.reset();
			_workshopItem.panelScreenshotPath.clear();
			return true;
		}
		break;
		case WorkshopItemState::CREATING:
		case WorkshopItemState::UPLOADING:
		{
			// Cannot change the state right now.
			return false;
		}
		break;
	}
	return false;
}

inline std::shared_ptr<Screenshot> createPalettePreviewScreenshot()
{
	int boxw = 20;
	int boxh = 20;
	int cols = 18;
	int rows = (COLORNAME_SIZE + cols - 1) / cols;
	int w = cols * boxw;
	int h = rows * boxh;

	SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormat(0,
		nearestPowerOfTwo(w), nearestPowerOfTwo(h),
		32, SDL_PIXELFORMAT_RGBA8888);
	if (!surface)
	{
		LOGW << "SDL generated invalid surface";
		DEBUG_ASSERT(false);
		return nullptr;
	}

	SDL_Renderer* renderer = SDL_CreateSoftwareRenderer(surface);
	if (!renderer)
	{
		LOGW << "SDL generated invalid renderer";
		DEBUG_ASSERT(false);
		return nullptr;
	}

	for (size_t i = 0; i < COLORNAME_SIZE; i++)
	{
		ColorName colorname = (ColorName) i;
		Color color = Palette::get(colorname);

		int x = (i % cols) * boxw;
		int y = (i / cols) * boxh;
		boxRGBA(renderer,
			x, y, x + boxw - 1, y + boxh - 1,
			color.r, color.g, color.b, color.a);
	}

	SDL_DestroyRenderer(renderer);

	auto screenshot = std::make_shared<Screenshot>(w, h, "palette", false);

	glBindTexture(GL_TEXTURE_2D, screenshot->textureID());
	glPixelStorei(GL_UNPACK_ROW_LENGTH, surface->w);
	glTexImage2D(GL_TEXTURE_2D, 0, surface->format->BytesPerPixel,
		w, h, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8,
		surface->pixels);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	SDL_FreeSurface(surface);

	return screenshot;
}

static int filterValidUserContentChar(ImGuiInputTextCallbackData* data)
{
	bool discarded = !isValidUserContentChar(data->EventChar);
	return discarded;
}

void Steam::updateWorkshop()
{
	if (_workshopItem.type == WorkshopItemType::NONE)
	{
		return;
	}

	bool takePreviewScreenshot = false;
	bool keepopen = true;
	if (ImGui::Begin("Steam Workshop", &keepopen,
			ImGuiWindowFlags_AlwaysAutoResize))
	{
		{
			ImGui::Text("By submitting items, you agree to the ");
			ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
			ImGui::PushStyleColor(ImGuiCol_Text,
				ImGui::GetStyle().Colors[ImGuiCol_ButtonHovered]);
			ImGui::Text("Steam Workshop terms of service");
			ImGui::PopStyleColor();
			ImVec2 min = ImGui::GetItemRectMin();
			ImVec2 max = ImGui::GetItemRectMax();
			min.y = max.y;
			auto underlinecolor = ImGuiCol_Button;
			if (ImGui::IsItemHovered())
			{
				const char* URL = "https://steamcommunity.com/sharedfiles/workshoplegalagreement";
				if (ImGui::IsMouseClicked(0))
				{
					openUrl(URL);
				}
				ImGui::SetTooltip("Open with Steam\n%s", URL);
				underlinecolor = ImGuiCol_ButtonHovered;
			}
			ImGui::GetWindowDrawList()->AddLine(min, max,
				ImGui::GetColorU32(ImGui::GetStyle().Colors[underlinecolor]),
				1.0f);
			ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
			ImGui::Text(".");
		}
		ImGui::Separator();

		switch (_workshopItem.type)
		{
			case WorkshopItemType::NONE:
			break;
			case WorkshopItemType::MAP:
			{
				ImGui::Text("Map: %s", _workshopItem.authoredName.c_str());
			}
			break;
			case WorkshopItemType::RULESET:
			{
				ImGui::Text("Ruleset: %s", _workshopItem.authoredName.c_str());
			}
			break;
			case WorkshopItemType::PALETTE:
			{
				ImGui::Text("Palette: %s", _workshopItem.authoredName.c_str());
			}
			break;
		}

		ImGui::InputText("Title",
			_workshopItem.title.data(), _workshopItem.title.size(),
			ImGuiInputTextFlags_CallbackCharFilter,
			filterValidUserContentChar);
		ImGui::InputTextMultiline("Description",
			_workshopItem.description.data(), _workshopItem.description.size());

		if (_workshopItem.previewScreenshot)
		{
			int w = 300;
			ImGui::Image(
				(void*) (intptr_t) _workshopItem.previewScreenshot->textureID(),
				ImVec2(w,
					_workshopItem.previewScreenshot->height() * w
						/ _workshopItem.previewScreenshot->width()));
		}
		switch (_workshopItem.type)
		{
			case WorkshopItemType::NONE:
			break;
			case WorkshopItemType::MAP:
			{
				if (ImGui::Button("Take screenshot of entire map"))
				{
					_client.takeScreenshotOfMap();
				}
			}
			break;
			case WorkshopItemType::RULESET:
			break;
			case WorkshopItemType::PALETTE:
			{
				if (ImGui::Button("Generate preview image"))
				{
					takePreviewScreenshot = true;
				}
			}
			break;
		}

		if (_workshopItem.panelScreenshot)
		{
			int w = 300;
			ImGui::Image(
				(void*) (intptr_t) _workshopItem.panelScreenshot->textureID(),
				ImVec2(w,
					_workshopItem.panelScreenshot->height() * w
						/ _workshopItem.panelScreenshot->width()),
				// Zoom in because the rest is bleed.
				ImVec2(0.25, 0.25),
				ImVec2(0.75, 0.75));
		}
		switch (_workshopItem.type)
		{
			case WorkshopItemType::NONE:
			break;
			case WorkshopItemType::MAP:
			{
				if (ImGui::Button("Take screenshot for UI panels"))
				{
					_client.takeScreenshot(
						std::make_shared<Screenshot>(600, 400, "panel"));
				}
			}
			break;
			case WorkshopItemType::RULESET:
			case WorkshopItemType::PALETTE:
			break;
		}

		switch (_workshopItem.state)
		{
			case WorkshopItemState::NONE:
			{
				if (!_runningWorkshopQueries.empty())
				{
					ImGui::Text("Loading...");
				}
				else if (ImGui::Button("Publish"))
				{
					LOGD << "Creating item...";
					auto callback = SteamUGC()->CreateItem(APPLICATION_ID,
						k_EWorkshopFileTypeCommunity);
					awaitCreateItemResult(callback);
					_workshopItem.state = WorkshopItemState::CREATING;
				}
			}
			break;
			case WorkshopItemState::CREATING:
			case WorkshopItemState::UPLOADING:
			{
				ImGui::Text("Publishing...");
			}
			break;
			case WorkshopItemState::READY:
			{
				if (!_runningWorkshopQueries.empty())
				{
					ImGui::Text("Loading...");
				}
				else if (ImGui::Button("Publish"))
				{
					submitWorkshopItem();
				}
				ImGui::SameLine();
				if (ImGui::Button("View in Steam Workshop"))
				{
					std::string url = "steam://url/CommunityFilePage/"
						+ std::to_string(_workshopItem.fileId);
					SteamFriends()->ActivateGameOverlayToWebPage(url.c_str());
				}
			}
			break;
			case WorkshopItemState::CREATION_FAILED:
			case WorkshopItemState::UPLOAD_FAILED:
			{
				static ImVec4 errorcolor(1.0f, 0.4f, 0.2f, 1.0f);
				ImGui::TextColored(errorcolor, "Failed to publish.");
			}
			break;
		}
	}
	ImGui::End();

	if (!keepopen)
	{
		resetWorkshopItem(WorkshopItemType::NONE);
	}

	// We should not call screenshotTaken() while rendering ImGui windows,
	// so do it now.
	if (takePreviewScreenshot)
	{
		Palette::installNamed(_workshopItem.authoredName);
		screenshotTaken(createPalettePreviewScreenshot());
	}
}

void Steam::handleCreateItemResult(CreateItemResult_t* result, bool failure)
{
	if (failure) LOGE << "IO failed";
	else if (result == nullptr) LOGE << "Unexpected null";
	else if (result->m_eResult == k_EResultOK)
	{
		_workshopItem.fileId = result->m_nPublishedFileId;
		LOGI << "Created item " << (uint64_t) _workshopItem.fileId;

		if (result->m_bUserNeedsToAcceptWorkshopLegalAgreement)
		{
			const char* URL = "https://steamcommunity.com/sharedfiles/workshoplegalagreement";
			openUrl(URL);
		}

		submitWorkshopItem();
	}
	// TODO handle some specific error cases?
	else
	{
		LOGE << "EResult = " << (int) result->m_eResult;
		_workshopItem.state = WorkshopItemState::CREATION_FAILED;
	}
}

void Steam::submitWorkshopItem()
{
	LOGD << "Uploading item " << _workshopItem.fileId << "...";
	_workshopItem.state = WorkshopItemState::UPLOADING;

	// Prepare a brand new content folder, to avoid uploading old files.
	{
		auto timestampMs = SteadyClock::milliseconds();
		uint16_t key = rand() % (1 << 16);
		std::string path = Download::getDownloadsFolderWithSlash() + "workshop"
			"/uploaded_items"
			"/"	+ std::to_string(_workshopItem.fileId) + ""
			"/" + keycode(key, timestampMs);
		System::touchDirectory(path);
		_workshopItem.contentPath = System::absolutePath(path);
	}

	std::vector<const char*> tags;
	std::string metadata;
	switch (_workshopItem.type)
	{
		case WorkshopItemType::NONE:
		break;
		case WorkshopItemType::MAP:
		{
			Json::Value json = Map::loadMetadata(_workshopItem.authoredName);
			PoolType pooltype = PoolType::NONE;
			bool isChallenge = false;
			std::string rulesetname;
			try
			{
				if (json["pool"].isString())
				{
					pooltype = parsePoolType(json["pool"].asString());
				}
				if (json["challenge"].isObject())
				{
					isChallenge = true;
				}
				if (pooltype == PoolType::CUSTOM)
				{
					if (json["ruleset"].isString())
					{
						rulesetname = json["ruleset"].asString();
					}
				}
			}
			catch (const ParseError& error)
			{
				LOGW << "Failed to parse loaded metadata: " << error.what();
			}
			catch (const Json::Exception& error)
			{
				LOGW << "Failed to parse loaded metadata: " << error.what();
			}
			System::copyFile(Map::authoredFilename(_workshopItem.authoredName),
				_workshopItem.contentPath + "/content.map");
			if (System::isFile(_workshopItem.panelScreenshotPath))
			{
				System::copyFile(_workshopItem.panelScreenshotPath,
					_workshopItem.contentPath + "/panel.png");
			}
			tags.emplace_back("Map");
			if (isChallenge)
			{
				tags.emplace_back("Challenge");
			}
			if (System::isFile(Locator::rulesetAuthoredFilename(rulesetname)))
			{
				System::copyFile(Locator::rulesetAuthoredFilename(rulesetname),
					_workshopItem.contentPath + "/ruleset.json");
				tags.emplace_back("Custom Rules");
				// Overwrite the 'ruleset' specified in the metadata to refer
				// to this Workshop item, instead of to the author's local file.
				json["ruleset"] = buildUniqueTag(
					_workshopItem.title.data(), _workshopItem.fileId);
			}
			metadata = Writer::write(json);
		}
		break;
		case WorkshopItemType::RULESET:
		{
			System::copyFile(
				Locator::rulesetAuthoredFilename(_workshopItem.authoredName),
				_workshopItem.contentPath + "/ruleset.json");
			tags.emplace_back("Ruleset");
		}
		break;
		case WorkshopItemType::PALETTE:
		{
			System::copyFile(
				Palette::authoredFilename(_workshopItem.authoredName),
				_workshopItem.contentPath + "/palette.json");
			tags.emplace_back("Color Palette");
		}
		break;
	}

	auto handle = SteamUGC()->StartItemUpdate(APPLICATION_ID,
		_workshopItem.fileId);
	SteamUGC()->SetItemTitle(handle, _workshopItem.title.data());
	SteamUGC()->SetItemDescription(handle, _workshopItem.description.data());
	{
		SteamParamStringArray_t param { tags.data(), (int) tags.size() };
		SteamUGC()->SetItemTags(handle, &param);
	}
	SteamUGC()->RemoveItemKeyValueTags(handle, "authoredName");
	SteamUGC()->AddItemKeyValueTag(handle, "authoredName",
		_workshopItem.authoredName.c_str());
	if (!_workshopItem.contentPath.empty()
			&& System::isDirectory(_workshopItem.contentPath))
	{
		SteamUGC()->SetItemContent(handle, _workshopItem.contentPath.c_str());
	}
	if (!_workshopItem.previewScreenshotPath.empty()
			&& System::isFile(_workshopItem.previewScreenshotPath))
	{
		SteamUGC()->SetItemPreview(handle,
			_workshopItem.previewScreenshotPath.c_str());
	}
	if (metadata.size() + 1 >= k_cchDeveloperMetadataMax)
	{
		LOGE << "Discarding too large metadata: " << metadata;
		metadata = "";
	}
	SteamUGC()->SetItemMetadata(handle, metadata.c_str());

	auto callback = SteamUGC()->SubmitItemUpdate(handle, nullptr);
	if (callback == k_uAPICallInvalid)
	{
		LOGE << "Invalid handle";
	}
	else awaitSubmitItemUpdateResult(callback);
}

void Steam::handleSubmitItemUpdateResult(SubmitItemUpdateResult_t* result,
	bool failure)
{
	if (failure) LOGE << "IO failed";
	else if (result == nullptr) LOGE << "Unexpected null";
	else if (result->m_eResult == k_EResultOK)
	{
		LOGD << "Upload complete.";
		_workshopItem.state = WorkshopItemState::READY;

		if (result->m_bUserNeedsToAcceptWorkshopLegalAgreement)
		{
			std::string url = "steam://url/CommunityFilePage/"
				+ std::to_string(_workshopItem.fileId);
			SteamFriends()->ActivateGameOverlayToWebPage(url.c_str());
		}

		retrievePublishedWorkshopItems();
	}
	// TODO handle some specific error cases?
	else
	{
		LOGE << "EResult = " << (int) result->m_eResult;
		_workshopItem.state = WorkshopItemState::UPLOAD_FAILED;
	}
}

void Steam::retrievePublishedWorkshopItems(int pageNumber)
{
	CSteamID steamid = SteamUser()->GetSteamID();
	auto handle = SteamUGC()->CreateQueryUserUGCRequest(steamid.GetAccountID(),
		k_EUserUGCList_Published, k_EUGCMatchingUGCType_Items,
		k_EUserUGCListSortOrder_CreationOrderAsc,
		APPLICATION_ID, APPLICATION_ID,
		pageNumber);
	if (handle == k_UGCQueryHandleInvalid)
	{
		LOGE << "Failed to generate query.";
		return;
	}
	SteamUGC()->SetReturnKeyValueTags(handle, true);
	if (pageNumber == 1)
	{
		_publishedWorkshopItems.clear();
	}
	WorkshopQuery query;
	query.type = WorkshopQueryType::PUBLISHED;
	query.handle = handle;
	query.pageNumber = pageNumber;
	addWorkshopQuery(std::move(query));
}

void Steam::retrieveSubscribedMaps(int pageNumber)
{
	CSteamID steamid = SteamUser()->GetSteamID();
	auto handle = SteamUGC()->CreateQueryUserUGCRequest(steamid.GetAccountID(),
		k_EUserUGCList_Subscribed, k_EUGCMatchingUGCType_Items,
		k_EUserUGCListSortOrder_CreationOrderAsc,
		APPLICATION_ID, APPLICATION_ID,
		pageNumber);
	if (handle == k_UGCQueryHandleInvalid)
	{
		LOGE << "Failed to generate query.";
		return;
	}
	SteamUGC()->AddRequiredTag(handle, "Map");
	SteamUGC()->SetReturnKeyValueTags(handle, true);
	SteamUGC()->SetReturnMetadata(handle, true);
	WorkshopQuery query;
	query.type = WorkshopQueryType::SUBSCRIBED_MAPS;
	query.handle = handle;
	query.pageNumber = pageNumber;
	addWorkshopQuery(std::move(query));
}

void Steam::retrieveSubscribedRulesets(int pageNumber)
{
	CSteamID steamid = SteamUser()->GetSteamID();
	auto handle = SteamUGC()->CreateQueryUserUGCRequest(steamid.GetAccountID(),
		k_EUserUGCList_Subscribed, k_EUGCMatchingUGCType_Items,
		k_EUserUGCListSortOrder_CreationOrderAsc,
		APPLICATION_ID, APPLICATION_ID,
		pageNumber);
	if (handle == k_UGCQueryHandleInvalid)
	{
		LOGE << "Failed to generate query.";
		return;
	}
	SteamUGC()->AddRequiredTag(handle, "Ruleset");
	SteamUGC()->SetReturnKeyValueTags(handle, true);
	WorkshopQuery query;
	query.type = WorkshopQueryType::SUBSCRIBED_RULESETS;
	query.handle = handle;
	query.pageNumber = pageNumber;
	addWorkshopQuery(std::move(query));
}

void Steam::retrieveSubscribedPalettes(int pageNumber)
{
	CSteamID steamid = SteamUser()->GetSteamID();
	auto handle = SteamUGC()->CreateQueryUserUGCRequest(steamid.GetAccountID(),
		k_EUserUGCList_Subscribed, k_EUGCMatchingUGCType_Items,
		k_EUserUGCListSortOrder_CreationOrderAsc,
		APPLICATION_ID, APPLICATION_ID,
		pageNumber);
	if (handle == k_UGCQueryHandleInvalid)
	{
		LOGE << "Failed to generate query.";
		return;
	}
	SteamUGC()->AddRequiredTag(handle, "Color Palette");
	SteamUGC()->SetReturnKeyValueTags(handle, true);
	WorkshopQuery query;
	query.type = WorkshopQueryType::SUBSCRIBED_PALETTES;
	query.handle = handle;
	query.pageNumber = pageNumber;
	addWorkshopQuery(std::move(query));
}

void Steam::addWorkshopQuery(WorkshopQuery&& query)
{
	// Cancel previous queries of this type.
	auto type = query.type;
	_runningWorkshopQueries.erase(
		std::remove_if(_runningWorkshopQueries.begin(),
			_runningWorkshopQueries.end(),
			[type](const WorkshopQuery& q) { return q.type == type; }),
		_runningWorkshopQueries.end());
	// Add a new query to the end of the queue.
	_runningWorkshopQueries.emplace_back(query);
	// Start it.
	if (_runningWorkshopQueries.size() == 1)
	{
		startNextWorkshopQuery();
	}
}

void Steam::startNextWorkshopQuery()
{
	for (/**/;
		!_runningWorkshopQueries.empty();
		_runningWorkshopQueries.erase(_runningWorkshopQueries.begin()))
	{
		const WorkshopQuery& query = _runningWorkshopQueries[0];

		auto callback = SteamUGC()->SendQueryUGCRequest(query.handle);
		if (callback == k_uAPICallInvalid)
		{
			LOGE << "Invalid handle";
			continue;
		}

		LOGD << "Retrieving query " << int(query.handle)
			<< " of type " << int(query.type)
			<< "; page " << query.pageNumber;
		return awaitSteamUGCQueryCompleted(callback);
	}
}

void Steam::screenshotTaken(std::weak_ptr<Screenshot> weakScreenshot)
{
	auto screenshot = weakScreenshot.lock();
	if (!screenshot)
	{
		LOGE << "Expected screenshot";
		return;
	}

	auto timestampMs = SteadyClock::milliseconds();
	uint16_t key = rand() % (1 << 16);
	std::string itemname = keycode(key, timestampMs);

	if (screenshot->tag() == "map" || screenshot->tag() == "palette")
	{
		_workshopItem.previewScreenshot = screenshot;
		_workshopItem.previewScreenshotPath = saveScreenshot(screenshot,
			"steam/workshop/screenshots/previews/" + itemname);
	}
	else if (screenshot->tag() == "panel")
	{
		_workshopItem.panelScreenshot = screenshot;
		_workshopItem.panelScreenshotPath = saveScreenshot(screenshot,
			"steam/workshop/screenshots/panels/" + itemname);
	}
	else
	{
		LOGW << "Unknown screenshot tag '" << screenshot->tag() << "'";
		saveScreenshot(screenshot,
			"steam/workshop/screenshots/lostandfound/" + itemname);
	}
}

std::string Steam::saveScreenshot(std::shared_ptr<Screenshot> screenshot,
		const std::string& picturename)
{
	std::string filename = Locator::pictureFilename(picturename);
	System::touchFile(filename);
	std::vector<uint8_t> buffer = screenshot->writeToBuffer();
	int bytes_per_pixel = 4;
	int stride_in_bytes = bytes_per_pixel * screenshot->width();
	bool success = stbi_write_png(filename.c_str(),
		screenshot->width(),
		screenshot->height(),
		bytes_per_pixel, buffer.data(),
		stride_in_bytes);
	if (success)
	{
		LOGI << "Wrote screenshot to file " << filename;
		return System::absolutePath(filename);
	}
	else
	{
		LOGE << "Failed to write screenshot to " << filename;
		return "";
	}
}

void Steam::handleSteamUGCQueryCompleted(SteamUGCQueryCompleted_t* result,
		bool failure)
{
	WorkshopQueryType type = WorkshopQueryType::NONE;
	int pageNumber = 0;
	bool shouldContinue = false;

	if (failure) LOGE << "IO failed";
	else if (result == nullptr) LOGE << "Unexpected null";
	else if (result->m_eResult == k_EResultOK)
	{
		Json::Reader reader;
		// Look for the query with this handle.
		{
			auto handle = result->m_handle;
			if (!_runningWorkshopQueries.empty()
				&& _runningWorkshopQueries[0].handle == handle)
			{
				type = _runningWorkshopQueries[0].type;
				pageNumber = _runningWorkshopQueries[0].pageNumber;
				LOGD << "Completed query " << int(handle)
					<< " of type " << int(type)
					<< "; page " << pageNumber;
				_runningWorkshopQueries.erase(_runningWorkshopQueries.begin());
			}
			else
			{
				LOGD << "Completed discarded query " << int(handle);
			}
		}
		// Use the results.
		for (size_t i = 0; i < result->m_unNumResultsReturned; i++)
		{
			SteamUGCDetails_t details;
			if (!SteamUGC()->GetQueryUGCResult(result->m_handle, i,
					&details))
			{
				LOGE << "Failed to get details for " << i << "/"
					<< result->m_unNumResultsReturned;
				break;
			}
			if (details.m_eResult != k_EResultOK)
			{
				// TODO handle some specific error cases?
				LOGE << "EResult = " << (int) details.m_eResult;
				break;
			}
			PublishedWorkshopItem item;
			item.fileId = details.m_nPublishedFileId;
			item.title = std::string(details.m_rgchTitle);
			item.description = std::string(details.m_rgchDescription);
			auto tagstrm = std::stringstream(details.m_rgchTags);
			std::string tag;
			while (std::getline(tagstrm, tag, ','))
			{
				if (::tolower(tag) == "map")
				{
					item.type = WorkshopItemType::MAP;
				}
				else if (::tolower(tag) == "ruleset")
				{
					item.type = WorkshopItemType::RULESET;
				}
				else if (::tolower(tag) == "color palette")
				{
					item.type = WorkshopItemType::PALETTE;
				}
			}
			int n = SteamUGC()->GetQueryUGCNumKeyValueTags(result->m_handle, i);
			for (int t = 0; t < n; t++)
			{
				std::array<char, 256> keybuffer = {0};
				std::array<char, 256> valuebuffer = {0};
				SteamUGC()->GetQueryUGCKeyValueTag(result->m_handle, i, t,
					keybuffer.data(), keybuffer.size() - 1,
					valuebuffer.data(), valuebuffer.size() - 1);
				if (strcmp(keybuffer.data(), "authoredName") == 0)
				{
					item.authoredName = std::string(valuebuffer.data());
				}
			}
			Json::Value metadata = Json::nullValue;
			{
				std::array<char, 1024> metadatabuffer = {0};
				if (!SteamUGC()->GetQueryUGCMetadata(result->m_handle, i,
					metadatabuffer.data(), metadatabuffer.size() - 1))
				{
					LOGE << "Failed to get metadata for " << i << "/"
						<< result->m_unNumResultsReturned;
				}
				else if (metadatabuffer[0] != '\0')
				{
					if (!reader.parse(metadatabuffer.data(), metadata))
					{
						LOGE << "Failed to parse metadata"
							" '" << metadatabuffer.data() << "'"
							": " << reader.getFormattedErrorMessages();
					}
				}
			}
			LOGV << "Retrieved " << item.fileId << ""
				", a type " << ((int) item.type) << ""
				" " << details.m_rgchTags << ""
				" named " << item.title << ""
				", originally authored as '" << item.authoredName << "'"
				"";
			switch (type)
			{
				case WorkshopQueryType::PUBLISHED:
				{
					LOGV << "Retrieve published item.";
					_publishedWorkshopItems.push_back(item);
				}
				break;
				case WorkshopQueryType::SUBSCRIBED_MAPS:
				{
					std::string dirname;
					{
						std::array<char, 5000> pathbuffer;
						if (SteamUGC()->GetItemInstallInfo(item.fileId, nullptr,
							pathbuffer.data(), pathbuffer.size() - 1, nullptr))
						{
							dirname = std::string(pathbuffer.data());
							if (!dirname.empty() && dirname.back() != '/'
								&& dirname.back() != '\\')
							{
								dirname += "/";
							}
						}
					}
					LOGV << "Retrieved map '" << item.title << "'"
						" with metadata " << Writer::write(metadata) << ""
						" (dirname=" << dirname << ")";
					if (!dirname.empty() && metadata.isObject()
						&& System::isFile(dirname + "content.map")
						&& isValidUserContentName(item.title))
					{
						std::string name = buildUniqueTag(
							item.title.data(), item.fileId);
						Map::ExternalItem external;
						external.uniqueTag = name;
						external.quotedName = "\"" + item.title + "\"";
						external.sourceFilename = dirname + "content.map";
						external.metadata = metadata;
						Map::listExternalItem(std::move(external));
						Locator::useExternalFolder({ name, dirname });
					}
					else
					{
						LOGW << "Not using incomplete or invalid map"
							" (fileId=" << item.fileId << ")";
					}
				}
				break;
				case WorkshopQueryType::SUBSCRIBED_RULESETS:
				{
					std::string dirname;
					{
						std::array<char, 5000> pathbuffer;
						if (SteamUGC()->GetItemInstallInfo(item.fileId, nullptr,
							pathbuffer.data(), pathbuffer.size() - 1, nullptr))
						{
							dirname = std::string(pathbuffer.data());
							if (!dirname.empty() && dirname.back() != '/'
								&& dirname.back() != '\\')
							{
								dirname += "/";
							}
						}
					}
					LOGV << "Retrieved ruleset '" << item.title << "'"
						" (dirname=" << dirname << ")";
					if (!dirname.empty()
						&& System::isFile(dirname + "ruleset.json")
						&& isValidUserContentName(item.title))
					{
						std::string name = buildUniqueTag(
							item.title.data(), item.fileId);
						Locator::useExternalFolder({ name, dirname });
					}
					else
					{
						LOGW << "Not using incomplete or invalid ruleset"
							" (fileId=" << item.fileId << ")";
					}
				}
				break;
				case WorkshopQueryType::SUBSCRIBED_PALETTES:
				{
					std::string fname;
					{
						std::array<char, 5000> pathbuffer;
						if (SteamUGC()->GetItemInstallInfo(item.fileId, nullptr,
							pathbuffer.data(), pathbuffer.size() - 1, nullptr))
						{
							fname = std::string(pathbuffer.data());
							if (!fname.empty() && fname.back() != '/'
								&& fname.back() != '\\')
							{
								fname += "/";
							}
							fname += "palette.json";
						}
					}
					LOGV << "Retrieved palette (fname=" << fname << ")";
					if (!fname.empty() && System::isFile(fname)
						&& isValidUserContentName(item.title))
					{
						Palette::ExternalItem external;
						external.uniqueTag = buildUniqueTag(
							item.title.data(), item.fileId);
						external.quotedName = "\"" + item.title + "\"";
						external.sourceFilename = fname;
						Palette::listExternalItem(std::move(external));
					}
					else
					{
						LOGW << "Not using incomplete or invalid palette"
							" (fileId=" << item.fileId << ")";
					}
				}
				break;
				case WorkshopQueryType::NONE:
				{
					LOGV << "Discarded.";
				}
				break;
			}
		}
		if (result->m_unTotalMatchingResults >
			50 * (pageNumber - 1) + result->m_unNumResultsReturned)
		{
			shouldContinue = true;
		}
		SteamUGC()->ReleaseQueryUGCRequest(result->m_handle);
	}
	// TODO handle some specific error cases?
	else
	{
		LOGE << "EResult = " << (int) result->m_eResult;
	}

	if (shouldContinue)
	{
		switch (type)
		{
			case WorkshopQueryType::PUBLISHED:
			{
				retrievePublishedWorkshopItems(pageNumber + 1);
			}
			break;
			case WorkshopQueryType::SUBSCRIBED_MAPS:
			{
				retrieveSubscribedMaps(pageNumber + 1);
			}
			break;
			case WorkshopQueryType::SUBSCRIBED_RULESETS:
			{
				retrieveSubscribedRulesets(pageNumber + 1);
			}
			break;
			case WorkshopQueryType::SUBSCRIBED_PALETTES:
			{
				retrieveSubscribedPalettes(pageNumber + 1);
			}
			break;
			case WorkshopQueryType::NONE:
			break;
		}
	}

	startNextWorkshopQuery();
}

void Steam::openUrl(const std::string& url)
{
	LOGD << "Opening " << url;
#ifdef CANDIDATE
	if (SteamFriends())
#else
#ifdef DEVELOPMENT
	if (false)
#else
	if (SteamFriends())
#endif
#endif
	{
		SteamFriends()->ActivateGameOverlayToWebPage(url.c_str());
	}
	else
	{
		System::openURL(url.c_str());
	}
}

void Steam::restorePublishedItem(const PublishedWorkshopItem& published)
{
	if (published.fileId <= 0)
	{
		LOGE << "Invalid published workshop item.";
		return;
	}
	_workshopItem.fileId = published.fileId;
	_workshopItem.state = WorkshopItemState::READY;
	if (published.title.size() + 1 < _workshopItem.title.size())
	{
		std::copy(published.title.begin(), published.title.end(),
			_workshopItem.title.data());
		_workshopItem.title[published.title.size()] = '\0';
	}
	if (published.description.size() + 1 < _workshopItem.description.size())
	{
		std::copy(published.description.begin(), published.description.end(),
			_workshopItem.description.data());
		_workshopItem.description[published.description.size()] = '\0';
	}
}

void Steam::openWorkshopForMap(const std::string& mapname)
{
	if (_workshopItem.type == WorkshopItemType::MAP
		&& _workshopItem.authoredName == mapname)
	{
		return;
	}
	else if (!_runningWorkshopQueries.empty())
	{
		resetWorkshopItem(WorkshopItemType::NONE);
		return;
	}

	resetWorkshopItem(WorkshopItemType::MAP);
	_workshopItem.authoredName = mapname;
	for (const auto& item : _publishedWorkshopItems)
	{
		if (item.type == _workshopItem.type
			&& item.authoredName == _workshopItem.authoredName)
		{
			restorePublishedItem(item);
		}
	}
}

void Steam::openWorkshopForRuleset(const std::string& rulesetname)
{
	if (_workshopItem.type == WorkshopItemType::RULESET
		&& _workshopItem.authoredName == rulesetname)
	{
		return;
	}
	else if (!_runningWorkshopQueries.empty())
	{
		resetWorkshopItem(WorkshopItemType::NONE);
		return;
	}

	resetWorkshopItem(WorkshopItemType::RULESET);
	_workshopItem.authoredName = rulesetname;
	for (const auto& item : _publishedWorkshopItems)
	{
		if (item.type == _workshopItem.type
			&& item.authoredName == _workshopItem.authoredName)
		{
			restorePublishedItem(item);
		}
	}

	_workshopItem.previewScreenshotPath = System::absolutePath(
		Locator::picture("steam/ruleset_preview"));
}

void Steam::openWorkshopForPalette(const std::string& palettename)
{
	if (_workshopItem.type == WorkshopItemType::PALETTE
		&& _workshopItem.authoredName == palettename)
	{
		return;
	}
	else if (!_runningWorkshopQueries.empty())
	{
		resetWorkshopItem(WorkshopItemType::NONE);
		return;
	}

	resetWorkshopItem(WorkshopItemType::PALETTE);
	_workshopItem.authoredName = palettename;
	for (const auto& item : _publishedWorkshopItems)
	{
		if (item.type == _workshopItem.type
			&& item.authoredName == _workshopItem.authoredName)
		{
			restorePublishedItem(item);
		}
	}
}

void Steam::closeAllWorkshops()
{
	resetWorkshopItem(WorkshopItemType::NONE);
}

void Steam::refreshSubscribedWorkshopItems()
{
	int n = SteamUGC()->GetNumSubscribedItems();
	if (n <= 0) return;
	std::vector<PublishedFileId_t> fileIds(n, 0);
	n = SteamUGC()->GetSubscribedItems(fileIds.data(), fileIds.size());
	LOGD << "Got " << n << " subscribed items";

	// Remove unsubscribed palettes.
	{
		std::vector<std::string> removals;
		for (const auto& item : Palette::externalItems())
		{
			size_t pos = item.uniqueTag.find("@WORKSHOP/");
			if (pos == std::string::npos) continue;
			std::string idstr = item.uniqueTag.substr(
				pos + strlen("@WORKSHOP/"));
			bool keep = false;
			for (const auto& fileId : fileIds)
			{
				if (std::to_string(fileId) == idstr)
				{
					keep = true;
					break;
				}
			}
			if (!keep)
			{
				removals.push_back(item.uniqueTag);
			}
		}
		for (const auto& removal : removals)
		{
			Palette::unlistExternalItem(removal);
		}
	}

	// Remove unsubscribed maps.
	{
		std::vector<std::string> removals;
		for (const auto& item : Map::externalItems())
		{
			size_t pos = item.uniqueTag.find("@WORKSHOP/");
			if (pos == std::string::npos) continue;
			std::string idstr = item.uniqueTag.substr(
				pos + strlen("@WORKSHOP/"));
			bool keep = false;
			for (const auto& fileId : fileIds)
			{
				if (std::to_string(fileId) == idstr)
				{
					keep = true;
					break;
				}
			}
			if (!keep)
			{
				removals.push_back(item.uniqueTag);
			}
		}
		for (const auto& removal : removals)
		{
			Map::unlistExternalItem(removal);
		}
	}

	// Remove unsubscribed external folders.
	{
		std::vector<std::string> removals;
		for (const auto& item : Locator::externalFolders())
		{
			size_t pos = item.uniqueTag.find("@WORKSHOP/");
			if (pos == std::string::npos) continue;
			std::string idstr = item.uniqueTag.substr(
				pos + strlen("@WORKSHOP/"));
			bool keep = false;
			for (const auto& fileId : fileIds)
			{
				if (std::to_string(fileId) == idstr)
				{
					keep = true;
					break;
				}
			}
			if (!keep)
			{
				removals.push_back(item.uniqueTag);
			}
		}
		for (const auto& removal : removals)
		{
			Locator::forgetExternalFolder(removal);
		}
	}

	retrieveSubscribedPalettes();
	retrieveSubscribedMaps();
	retrieveSubscribedRulesets();
}

void Steam::handleItemInstalled(ItemInstalled_t* result, bool failure)
{
	if (failure) LOGE << "IO failed";
	else if (result == nullptr) LOGE << "Unexpected null";
	else if (result->m_unAppID == APPLICATION_ID)
	{
		auto fileId = result->m_nPublishedFileId;
		LOGD << "Item " << fileId << " was installed.";
		refreshSubscribedWorkshopItems();
	}
}

extern "C" void __cdecl steam_log_callback(int severity, const char* text)
{
	switch (severity)
	{
		case 0: LOGI << text; break;
		case 1: LOGW << text; break;
		default: LOGE << text; break;
	}
}

/* ############################# STEAM_ENABLED ############################# */
#endif
