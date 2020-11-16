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

#include "libs/jsoncpp/json.h"

#include "player.hpp"
#include "role.hpp"
#include "visiontype.hpp"
#include "target.hpp"
#include "difficulty.hpp"
#include "version.hpp"
#include "responsestatus.hpp"

class TypeNamer;
struct Change;
struct Order;


class StreamedMessage;
class ParsedMessage;

class Message
{
public:
	enum class Type : uint8_t
	{
		INVALID = 0,
		INIT,
		CHAT,
		RATING_AND_STARS,
		RATING,
		STARS,
		RECENT_STARS,
		JOIN_SERVER,
		LEAVE_SERVER,
		JOIN_LOBBY,
		LEAVE_LOBBY,
		LIST_LOBBY,
		MAKE_LOBBY,
		DISBAND_LOBBY,
		EDIT_LOBBY,
		SAVE_LOBBY,
		LOCK_LOBBY,
		UNLOCK_LOBBY,
		NAME_LOBBY,
		MAX_PLAYERS,
		NUM_PLAYERS,
		CLAIM_ROLE,
		CLAIM_COLOR,
		CLAIM_VISIONTYPE,
		CLAIM_AI,
		CLAIM_DIFFICULTY,
		PICK_MAP,
		PICK_TIMER,
		PICK_REPLAY,
		PICK_CHALLENGE,
		PICK_RULESET,
		ADD_BOT,
		REMOVE_BOT,
		ENABLE_CUSTOM_MAPS,
		LIST_AI,
		LIST_MAP,
		LIST_REPLAY,
		LIST_RULESET,
		LIST_CHALLENGE,
		RULESET_REQUEST,
		RULESET_DATA,
		RULESET_UNKNOWN,
		SECRETS,
		SKINS,
		IN_GAME,
		START,
		GAME,
		RESTORE,
		REPLAY,
		TUTORIAL,
		CHALLENGE,
		BRIEFING,
		RESIGN,
		CHANGE,
		ORDER_OLD,
		ORDER_NEW,
		SYNC,
		PING,
		PONG,
		QUIT,
		CLOSING,
		CLOSED,
		VERSION,
		PATCH,
		DOWNLOAD,
		REQUEST,
		REQUEST_DENIED,
		REQUEST_FULFILLED,
		ENABLE_COMPRESSION,
		DISABLE_COMPRESSION,
		RANKINGS,
		STAMP,
		LINK_ACCOUNTS,
		PULSE
	};

	static StreamedMessage init();

	static StreamedMessage chat(
		const std::string& content, const std::string& sender);
	static StreamedMessage chat(
		const std::string& content, const std::string& sender,
		const Target& target);
	static StreamedMessage chat(
		const std::string& content, const Target& target);

	static StreamedMessage rating(
		const std::string& username, float rating);

	static StreamedMessage stars(
		const std::string& username, int stars);

	static StreamedMessage recent_stars(
		int stars);

	static StreamedMessage join_server(
		ResponseStatus status);
	static StreamedMessage join_server(
		const std::string& username);
	static StreamedMessage join_server(
		const std::string& username, const Json::Value& metadata);
	static StreamedMessage join_server(
		const std::string& token, const std::string& accountid,
		const Json::Value& metadata);
	static StreamedMessage join_server(
		const std::string& token, const std::string& accountid);

	static StreamedMessage leave_server();
	static StreamedMessage leave_server(
		const std::string& username);

	static StreamedMessage join_lobby();
	static StreamedMessage join_lobby(
		const std::string& lobbyid);
	static StreamedMessage join_lobby(
		const std::string& lobbyid, const std::string& username);
	static StreamedMessage join_lobby(
		const std::string& lobbyid, const Json::Value& metadata);

	static StreamedMessage leave_lobby();
	static StreamedMessage leave_lobby(
		const std::string& lobbyid, const std::string& username);

	static StreamedMessage make_lobby();
	static StreamedMessage make_lobby(
		const Json::Value& metadata);
	static StreamedMessage make_lobby(
		const std::string& lobbyid);

	static StreamedMessage disband_lobby(
		const std::string& lobbyid);

	static StreamedMessage edit_lobby(
		const std::string& lobbyid);

	static StreamedMessage save_lobby();
	static StreamedMessage save_lobby(
		const std::string& lobbyid);

	static StreamedMessage lock_lobby();
	static StreamedMessage lock_lobby(
		const std::string& lobbyid);

	static StreamedMessage unlock_lobby();
	static StreamedMessage unlock_lobby(
		const std::string& lobbyid);

	static StreamedMessage name_lobby(
		const std::string& content);
	static StreamedMessage name_lobby(
		const std::string& content, const std::string& lobbyid);

	static StreamedMessage max_players(
		const std::string& lobbyid, int value);

	static StreamedMessage num_players(
		const std::string& lobbyid, int value);

	static StreamedMessage claim_role(
		const Role& role, const std::string& username);

	static StreamedMessage claim_color(
		const Player& color, const std::string& username);

	static StreamedMessage claim_visiontype(
		const VisionType& visiontype, const std::string& username);

	static StreamedMessage claim_ai(
		const std::string& ainame, const std::string& slotname);

	static StreamedMessage claim_difficulty(
		const Difficulty& difficulty, const std::string& slotname);

	static StreamedMessage pick_map(
		const std::string& name);

	static StreamedMessage pick_timer(
		uint32_t time);

	static StreamedMessage pick_replay(
		const std::string& name);

	static StreamedMessage pick_challenge(
		const std::string& key);

	static StreamedMessage pick_ruleset(
		const std::string& name);

	static StreamedMessage add_bot();
	static StreamedMessage add_bot(
		const std::string& slotname);

	static StreamedMessage remove_bot(
		const std::string& slotname);

	static StreamedMessage enable_custom_maps();

	static StreamedMessage list_map(
		const std::string& name, const Json::Value& metadata);

	static StreamedMessage list_replay();
	static StreamedMessage list_replay(
		const std::string& name, const Json::Value& metadata);

	static StreamedMessage list_ruleset(
		const std::string& name);

	static StreamedMessage list_challenge(
		const std::string& key, const Json::Value& metadata);

	static StreamedMessage list_ai(
		const std::string& ainame);

	static StreamedMessage ruleset_request(
		const std::string& rulesetname);

	static StreamedMessage ruleset_data(
		const std::string& rulesetname, const Json::Value& data);

	static StreamedMessage ruleset_unknown(
		const std::string& rulesetname);

	static StreamedMessage secrets(
		const Json::Value& metadata);

	static StreamedMessage skins(
		const Json::Value& metadata);

	static StreamedMessage in_game(
		const std::string& lobbyid, const std::string& username,
		const Role& role);

	static StreamedMessage start();

	static StreamedMessage game();
	static StreamedMessage game(
		const Role& role, const Player& player,
		const std::string& rulesetname, uint32_t time);

	static StreamedMessage tutorial();
	static StreamedMessage tutorial(
		const Role& role, const Player& player,
		const std::string& rulesetname, uint32_t time);

	static StreamedMessage challenge();

	static StreamedMessage replay(
		bool enabled);

	static StreamedMessage briefing(
		const Json::Value& metadata);

	static StreamedMessage resign();
	static StreamedMessage resign(
		const std::string& username);

	static StreamedMessage change(
		const TypeNamer& typenamer,
		const std::vector<Change>& changes);

	static StreamedMessage order_old(
		const TypeNamer& typenamer,
		const std::vector<Order>& orders);

	static StreamedMessage order_new(
		const TypeNamer& typenamer,
		const std::vector<Order>& orders);

	static StreamedMessage sync();
	static StreamedMessage sync(
		uint32_t time);

	static StreamedMessage ping();

	static StreamedMessage pong();

	static StreamedMessage quit();

	static StreamedMessage closing();

	static StreamedMessage closed();

	static StreamedMessage version(
		const Version& vers);
	static StreamedMessage version(
		const Version& vers, const Json::Value& metadata);

	static StreamedMessage patch();
	static StreamedMessage patch(
		const Version& vers);
	static StreamedMessage patch(
		const Version& vers, const Json::Value& metadata);

	static StreamedMessage download(
		const std::string& name, const Json::Value& metadata);

//	static StreamedMessage request(
//		const std::string& name);

	static StreamedMessage request_fulfilled(
		const std::string& name, const Json::Value& metadata);

	static StreamedMessage request_denied(
		const std::string& name, const Json::Value& metadata);

	static StreamedMessage enable_compression(
		const std::string& modelname);

	static StreamedMessage disable_compression();

	static StreamedMessage rankings(
		const Json::Value& metadata);

	static StreamedMessage stamp(
		const Json::Value& metadata);

	static StreamedMessage link_accounts(
		const Json::Value& metadata);

	static StreamedMessage pulse();

	static ParsedMessage parse(const char* buffer, uint32_t length);

	static bool compressible(const Type& type);

	static Type parseMessageType(const std::string& str);
};

class StreamedMessage : private Message
{
private:
	friend Message;
	explicit StreamedMessage();

	Type _type;
	std::string _str;

public:
	Type type() const { return _type; }
	bool compressible() const { return Message::compressible(_type); }

	std::string str() const;

	friend std::ostream& operator<<(std::ostream& os,
		const StreamedMessage& message);
};

class ParsedMessage : private Message
{
public:
	explicit ParsedMessage();
	explicit ParsedMessage(Json::Value&& json);

private:
	Json::Value _json;

	Type _type;

	std::string _content;
	std::string _sender;
	uint32_t _time = 0;
	float _rating = 0;
	Player _player = Player::NONE;
	Role _role = Role::NONE;
	VisionType _visiontype = VisionType::NONE;
	Difficulty _difficulty = Difficulty::NONE;
	Target _target = Target::NONE;
	ResponseStatus _status = ResponseStatus::SUCCESS;
	Version _version;

public:
	Type type() const                          { return _type;       }
	const std::string& content() const         { return _content;    }
	const std::string& sender() const          { return _sender;     }
	uint32_t time() const                      { return _time;       }
	float rating() const                       { return _rating;     }
	const Player& player() const               { return _player;     }
	const Role& role() const                   { return _role;       }
	const VisionType& visiontype() const       { return _visiontype; }
	const Difficulty& difficulty() const       { return _difficulty; }
	const Target& target() const               { return _target;     }
	const Version& version() const             { return _version;    }
	const ResponseStatus& status() const       { return _status;     }

	const Json::Value& metadata() const;
	const Json::Value& changes() const;
	const Json::Value& orders() const;
	const Json::Value& data() const;

	friend std::ostream& operator<<(std::ostream& os,
		const ParsedMessage& message);
};
