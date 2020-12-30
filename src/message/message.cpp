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
#include "message.hpp"
#include "source.hpp"

#include "writer.hpp"
#include "parseerror.hpp"
#include "typenamer.hpp"
#include "change.hpp"
#include "order.hpp"


StreamedMessage Message::init()
{
	StreamedMessage message;
	message._type = Message::Type::INIT;
	std::stringstream strm;
	strm << "{\"type\":\"init\"}";
	message._str = strm.str();
	return message;
}

StreamedMessage Message::chat(
		const std::string& content, const std::string& sender)
{
	StreamedMessage message;
	message._type = Message::Type::CHAT;
	std::stringstream strm;
	strm << "{\"type\":\"chat\"";
	strm << ",\"content\":"
		<< Json::valueToQuotedString(content.c_str());
	strm << ",\"sender\":"
		<< Json::valueToQuotedString(sender.c_str());
	strm << "}";
	message._str = strm.str();
	return message;
}

StreamedMessage Message::chat(
		const std::string& content, const std::string& sender,
		const Target& target)
{
	StreamedMessage message;
	message._type = Message::Type::CHAT;
	std::stringstream strm;
	strm << "{\"type\":\"chat\"";
	strm << ",\"content\":"
		<< Json::valueToQuotedString(content.c_str());
	strm << ",\"sender\":"
		<< Json::valueToQuotedString(sender.c_str());
	strm << ",\"target\":\"" << target << "\"";
	strm << "}";
	message._str = strm.str();
	return message;
}

StreamedMessage Message::chat(
		const std::string& content, const Target& target)
{
	StreamedMessage message;
	message._type = Message::Type::CHAT;
	std::stringstream strm;
	strm << "{\"type\":\"chat\"";
	strm << ",\"content\":"
		<< Json::valueToQuotedString(content.c_str());
	strm << ",\"target\":\"" << target << "\"";
	strm << "}";
	message._str = strm.str();
	return message;
}

StreamedMessage Message::rating(
		const std::string& username, float rating)
{
	StreamedMessage message;
	message._type = Message::Type::RATING;
	std::stringstream strm;
	strm << "{\"type\":\"rating\"";
	strm << ",\"content\":"
		<< Json::valueToQuotedString(username.c_str());
	strm << ",\"rating\":" << rating;
	strm << "}";
	message._str = strm.str();
	return message;
}

StreamedMessage Message::stars(
		const std::string& username, int stars)
{
	StreamedMessage message;
	message._type = Message::Type::STARS;
	std::stringstream strm;
	strm << "{\"type\":\"stars\"";
	strm << ",\"content\":"
		<< Json::valueToQuotedString(username.c_str());
	strm << ",\"time\":" << stars;
	strm << "}";
	message._str = strm.str();
	return message;
}

StreamedMessage Message::recent_stars(
		int stars)
{
	StreamedMessage message;
	message._type = Message::Type::RECENT_STARS;
	std::stringstream strm;
	strm << "{\"type\":\"recent_stars\"";
	strm << ",\"time\":" << stars;
	strm << "}";
	message._str = strm.str();
	return message;
}

StreamedMessage Message::join_server(
		ResponseStatus status)
{
	StreamedMessage message;
	message._type = Message::Type::JOIN_SERVER;
	std::stringstream strm;
	strm << "{\"type\":\"join_server\"";
	strm << ",\"status\":" << std::to_string(int(status));
	strm << "}";
	message._str = strm.str();
	return message;
}

StreamedMessage Message::join_server(
		const std::string& username)
{
	StreamedMessage message;
	message._type = Message::Type::JOIN_SERVER;
	std::stringstream strm;
	strm << "{\"type\":\"join_server\"";
	strm << ",\"content\":"
		<< Json::valueToQuotedString(username.c_str());
	strm << "}";
	message._str = strm.str();
	return message;
}

StreamedMessage Message::join_server(
		const std::string& username, const Json::Value& metadata)
{
	StreamedMessage message;
	message._type = Message::Type::JOIN_SERVER;
	std::stringstream strm;
	strm << "{\"type\":\"join_server\"";
	strm << ",\"content\":"
		<< Json::valueToQuotedString(username.c_str());
	strm << ",\"metadata\":" << Writer::write(metadata);
	strm << "}";
	message._str = strm.str();
	return message;
}

StreamedMessage Message::join_server(
		const std::string& token, const std::string& accountid,
		const Json::Value& metadata)
{
	StreamedMessage message;
	message._type = Message::Type::JOIN_SERVER;
	std::stringstream strm;
	strm << "{\"type\":\"join_server\"";
	strm << ",\"content\":"
		<< Json::valueToQuotedString(token.c_str());
	strm << ",\"sender\":"
		<< Json::valueToQuotedString(accountid.c_str());
	strm << ",\"metadata\":" << Writer::write(metadata);
	strm << "}";
	message._str = strm.str();
	return message;
}

StreamedMessage Message::join_server(
		const std::string& token, const std::string& accountid)
{
	StreamedMessage message;
	message._type = Message::Type::JOIN_SERVER;
	std::stringstream strm;
	strm << "{\"type\":\"join_server\"";
	strm << ",\"content\":"
		<< Json::valueToQuotedString(token.c_str());
	strm << ",\"sender\":"
		<< Json::valueToQuotedString(accountid.c_str());
	strm << "}";
	message._str = strm.str();
	return message;
}

StreamedMessage Message::leave_server()
{
	StreamedMessage message;
	message._type = Message::Type::LEAVE_SERVER;
	std::stringstream strm;
	strm << "{\"type\":\"leave_server\"";
	strm << "}";
	message._str = strm.str();
	return message;
}

StreamedMessage Message::leave_server(
		const std::string& username)
{
	StreamedMessage message;
	message._type = Message::Type::LEAVE_SERVER;
	std::stringstream strm;
	strm << "{\"type\":\"leave_server\"";
	strm << ",\"content\":"
		<< Json::valueToQuotedString(username.c_str());
	strm << "}";
	message._str = strm.str();
	return message;
}

StreamedMessage Message::join_lobby()
{
	StreamedMessage message;
	message._type = Message::Type::JOIN_LOBBY;
	std::stringstream strm;
	strm << "{\"type\":\"join_lobby\"";
	strm << "}";
	message._str = strm.str();
	return message;
}

StreamedMessage Message::join_lobby(
		const std::string& lobbyid)
{
	StreamedMessage message;
	message._type = Message::Type::JOIN_LOBBY;
	std::stringstream strm;
	strm << "{\"type\":\"join_lobby\"";
	strm << ",\"content\":"
		<< Json::valueToQuotedString(lobbyid.c_str());
	strm << "}";
	message._str = strm.str();
	return message;
}

StreamedMessage Message::join_lobby(
		const std::string& lobbyid, const std::string& username)
{
	StreamedMessage message;
	message._type = Message::Type::JOIN_LOBBY;
	std::stringstream strm;
	strm << "{\"type\":\"join_lobby\"";
	strm << ",\"content\":"
		<< Json::valueToQuotedString(lobbyid.c_str());
	strm << ",\"sender\":"
		<< Json::valueToQuotedString(username.c_str());
	strm << "}";
	message._str = strm.str();
	return message;
}

StreamedMessage Message::join_lobby(
		const std::string& lobbyid, const Json::Value& metadata)
{
	StreamedMessage message;
	message._type = Message::Type::JOIN_LOBBY;
	std::stringstream strm;
	strm << "{\"type\":\"join_lobby\"";
	strm << ",\"content\":"
		<< Json::valueToQuotedString(lobbyid.c_str());
	strm << ",\"metadata\":" << Writer::write(metadata);
	strm << "}";
	message._str = strm.str();
	return message;
}

StreamedMessage Message::leave_lobby()
{
	StreamedMessage message;
	message._type = Message::Type::LEAVE_LOBBY;
	std::stringstream strm;
	strm << "{\"type\":\"leave_lobby\"";
	strm << "}";
	message._str = strm.str();
	return message;
}

StreamedMessage Message::leave_lobby(
		const std::string& lobbyid, const std::string& username)
{
	StreamedMessage message;
	message._type = Message::Type::LEAVE_LOBBY;
	std::stringstream strm;
	strm << "{\"type\":\"leave_lobby\"";
	strm << ",\"content\":"
		<< Json::valueToQuotedString(lobbyid.c_str());
	strm << ",\"sender\":"
		<< Json::valueToQuotedString(username.c_str());
	strm << "}";
	message._str = strm.str();
	return message;
}

StreamedMessage Message::make_lobby()
{
	StreamedMessage message;
	message._type = Message::Type::MAKE_LOBBY;
	std::stringstream strm;
	strm << "{\"type\":\"make_lobby\"";
	strm << "}";
	message._str = strm.str();
	return message;
}

StreamedMessage Message::make_lobby(
		const Json::Value& metadata)
{
	StreamedMessage message;
	message._type = Message::Type::MAKE_LOBBY;
	std::stringstream strm;
	strm << "{\"type\":\"make_lobby\"";
	strm << ",\"metadata\":" << Writer::write(metadata);
	strm << "}";
	message._str = strm.str();
	return message;
}

StreamedMessage Message::make_lobby(
		const std::string& lobbyid)
{
	StreamedMessage message;
	message._type = Message::Type::MAKE_LOBBY;
	std::stringstream strm;
	strm << "{\"type\":\"make_lobby\"";
	strm << ",\"content\":"
		<< Json::valueToQuotedString(lobbyid.c_str());
	strm << "}";
	message._str = strm.str();
	return message;
}

StreamedMessage Message::disband_lobby(
		const std::string& lobbyid)
{
	StreamedMessage message;
	message._type = Message::Type::DISBAND_LOBBY;
	std::stringstream strm;
	strm << "{\"type\":\"disband_lobby\"";
	strm << ",\"content\":"
		<< Json::valueToQuotedString(lobbyid.c_str());
	strm << "}";
	message._str = strm.str();
	return message;
}

StreamedMessage Message::edit_lobby(
		const std::string& lobbyid)
{
	StreamedMessage message;
	message._type = Message::Type::EDIT_LOBBY;
	std::stringstream strm;
	strm << "{\"type\":\"edit_lobby\"";
	strm << ",\"content\":"
		<< Json::valueToQuotedString(lobbyid.c_str());
	strm << "}";
	message._str = strm.str();
	return message;
}

StreamedMessage Message::save_lobby()
{
	StreamedMessage message;
	message._type = Message::Type::SAVE_LOBBY;
	std::stringstream strm;
	strm << "{\"type\":\"save_lobby\"";
	strm << "}";
	message._str = strm.str();
	return message;
}

StreamedMessage Message::save_lobby(
		const std::string& lobbyid)
{
	StreamedMessage message;
	message._type = Message::Type::SAVE_LOBBY;
	std::stringstream strm;
	strm << "{\"type\":\"save_lobby\"";
	strm << ",\"content\":"
		<< Json::valueToQuotedString(lobbyid.c_str());
	strm << "}";
	message._str = strm.str();
	return message;
}

StreamedMessage Message::lock_lobby()
{
	StreamedMessage message;
	message._type = Message::Type::LOCK_LOBBY;
	std::stringstream strm;
	strm << "{\"type\":\"lock_lobby\"";
	strm << "}";
	message._str = strm.str();
	return message;
}

StreamedMessage Message::lock_lobby(
		const std::string& lobbyid)
{
	StreamedMessage message;
	message._type = Message::Type::LOCK_LOBBY;
	std::stringstream strm;
	strm << "{\"type\":\"lock_lobby\"";
	strm << ",\"content\":"
		<< Json::valueToQuotedString(lobbyid.c_str());
	strm << "}";
	message._str = strm.str();
	return message;
}

StreamedMessage Message::unlock_lobby()
{
	StreamedMessage message;
	message._type = Message::Type::UNLOCK_LOBBY;
	std::stringstream strm;
	strm << "{\"type\":\"unlock_lobby\"";
	strm << "}";
	message._str = strm.str();
	return message;
}

StreamedMessage Message::unlock_lobby(
		const std::string& lobbyid)
{
	StreamedMessage message;
	message._type = Message::Type::UNLOCK_LOBBY;
	std::stringstream strm;
	strm << "{\"type\":\"unlock_lobby\"";
	strm << ",\"content\":"
		<< Json::valueToQuotedString(lobbyid.c_str());
	strm << "}";
	message._str = strm.str();
	return message;
}

StreamedMessage Message::name_lobby(
		const std::string& content)
{
	StreamedMessage message;
	message._type = Message::Type::NAME_LOBBY;
	std::stringstream strm;
	strm << "{\"type\":\"name_lobby\"";
	strm << ",\"content\":"
		<< Json::valueToQuotedString(content.c_str());
	strm << "}";
	message._str = strm.str();
	return message;
}

StreamedMessage Message::name_lobby(
		const std::string& content, const std::string& lobbyid)
{
	StreamedMessage message;
	message._type = Message::Type::NAME_LOBBY;
	std::stringstream strm;
	strm << "{\"type\":\"name_lobby\"";
	strm << ",\"content\":"
		<< Json::valueToQuotedString(content.c_str());
	strm << ",\"sender\":"
		<< Json::valueToQuotedString(lobbyid.c_str());
	strm << "}";
	message._str = strm.str();
	return message;
}

StreamedMessage Message::max_players(
		const std::string& lobbyid, int value)
{
	StreamedMessage message;
	message._type = Message::Type::MAX_PLAYERS;
	std::stringstream strm;
	strm << "{\"type\":\"max_players\"";
	strm << ",\"content\":"
		<< Json::valueToQuotedString(lobbyid.c_str());
	strm << ",\"time\":" << value;
	strm << "}";
	message._str = strm.str();
	return message;
}

StreamedMessage Message::num_players(
		const std::string& lobbyid, int value)
{
	StreamedMessage message;
	message._type = Message::Type::NUM_PLAYERS;
	std::stringstream strm;
	strm << "{\"type\":\"num_players\"";
	strm << ",\"content\":"
		<< Json::valueToQuotedString(lobbyid.c_str());
	strm << ",\"time\":" << value;
	strm << "}";
	message._str = strm.str();
	return message;
}

StreamedMessage Message::claim_role(
		const Role& role, const std::string& username)
{
	StreamedMessage message;
	message._type = Message::Type::CLAIM_ROLE;
	std::stringstream strm;
	strm << "{\"type\":\"claim_role\"";
	strm << ",\"sender\":"
		<< Json::valueToQuotedString(username.c_str());
	strm << ",\"role\":\"" << role << "\"";
	strm << "}";
	message._str = strm.str();
	return message;
}

StreamedMessage Message::claim_color(
		const Player& color, const std::string& username)
{
	StreamedMessage message;
	message._type = Message::Type::CLAIM_COLOR;
	std::stringstream strm;
	strm << "{\"type\":\"claim_color\"";
	strm << ",\"sender\":"
		<< Json::valueToQuotedString(username.c_str());
	strm << ",\"player\":\"" << color << "\"";
	strm << "}";
	message._str = strm.str();
	return message;
}

StreamedMessage Message::claim_visiontype(
		const VisionType& visiontype, const std::string& username)
{
	StreamedMessage message;
	message._type = Message::Type::CLAIM_VISIONTYPE;
	std::stringstream strm;
	strm << "{\"type\":\"claim_visiontype\"";
	strm << ",\"sender\":"
		<< Json::valueToQuotedString(username.c_str());
	strm << ",\"visiontype\":\"" << visiontype << "\"";
	strm << "}";
	message._str = strm.str();
	return message;
}

StreamedMessage Message::claim_ai(
		const std::string& ainame, const std::string& slotname)
{
	StreamedMessage message;
	message._type = Message::Type::CLAIM_AI;
	std::stringstream strm;
	strm << "{\"type\":\"claim_ai\"";
	strm << ",\"content\":"
		<< Json::valueToQuotedString(ainame.c_str());
	strm << ",\"sender\":"
		<< Json::valueToQuotedString(slotname.c_str());
	strm << "}";
	message._str = strm.str();
	return message;
}

StreamedMessage Message::claim_difficulty(
		const Difficulty& difficulty, const std::string& slotname)
{
	StreamedMessage message;
	message._type = Message::Type::CLAIM_DIFFICULTY;
	std::stringstream strm;
	strm << "{\"type\":\"claim_difficulty\"";
	strm << ",\"sender\":"
		<< Json::valueToQuotedString(slotname.c_str());
	strm << ",\"difficulty\":\"" << difficulty << "\"";
	strm << "}";
	message._str = strm.str();
	return message;
}

StreamedMessage Message::pick_map(
		const std::string& name)
{
	StreamedMessage message;
	message._type = Message::Type::PICK_MAP;
	std::stringstream strm;
	strm << "{\"type\":\"pick_map\"";
	strm << ",\"content\":"
		<< Json::valueToQuotedString(name.c_str());
	strm << "}";
	message._str = strm.str();
	return message;
}

StreamedMessage Message::pick_timer(
		uint32_t time)
{
	StreamedMessage message;
	message._type = Message::Type::PICK_TIMER;
	std::stringstream strm;
	strm << "{\"type\":\"pick_timer\"";
	strm << ",\"time\":" << std::to_string(time);
	strm << "}";
	message._str = strm.str();
	return message;
}

StreamedMessage Message::pick_replay(
		const std::string& name)
{
	StreamedMessage message;
	message._type = Message::Type::PICK_REPLAY;
	std::stringstream strm;
	strm << "{\"type\":\"pick_replay\"";
	strm << ",\"content\":"
		<< Json::valueToQuotedString(name.c_str());
	strm << "}";
	message._str = strm.str();
	return message;
}

StreamedMessage Message::pick_challenge(
		const std::string& key)
{
	StreamedMessage message;
	message._type = Message::Type::PICK_CHALLENGE;
	std::stringstream strm;
	strm << "{\"type\":\"pick_challenge\"";
	strm << ",\"content\":"
		<< Json::valueToQuotedString(key.c_str());
	strm << "}";
	message._str = strm.str();
	return message;
}

StreamedMessage Message::pick_ruleset(
		const std::string& name)
{
	StreamedMessage message;
	message._type = Message::Type::PICK_RULESET;
	std::stringstream strm;
	strm << "{\"type\":\"pick_ruleset\"";
	strm << ",\"content\":"
		<< Json::valueToQuotedString(name.c_str());
	strm << "}";
	message._str = strm.str();
	return message;
}

StreamedMessage Message::add_bot()
{
	StreamedMessage message;
	message._type = Message::Type::ADD_BOT;
	std::stringstream strm;
	strm << "{\"type\":\"add_bot\"";
	strm << "}";
	message._str = strm.str();
	return message;
}

StreamedMessage Message::add_bot(
		const std::string& slotname)
{
	StreamedMessage message;
	message._type = Message::Type::ADD_BOT;
	std::stringstream strm;
	strm << "{\"type\":\"add_bot\"";
	strm << ",\"content\":"
		<< Json::valueToQuotedString(slotname.c_str());
	strm << "}";
	message._str = strm.str();
	return message;
}

StreamedMessage Message::remove_bot(
		const std::string& slotname)
{
	StreamedMessage message;
	message._type = Message::Type::REMOVE_BOT;
	std::stringstream strm;
	strm << "{\"type\":\"remove_bot\"";
	strm << ",\"content\":"
		<< Json::valueToQuotedString(slotname.c_str());
	strm << "}";
	message._str = strm.str();
	return message;
}

StreamedMessage Message::enable_custom_maps()
{
	StreamedMessage message;
	message._type = Message::Type::ENABLE_CUSTOM_MAPS;
	std::stringstream strm;
	strm << "{\"type\":\"enable_custom_maps\"";
	strm << "}";
	message._str = strm.str();
	return message;
}

StreamedMessage Message::list_map(
		const std::string& name, const Json::Value& metadata)
{
	StreamedMessage message;
	message._type = Message::Type::LIST_MAP;
	std::stringstream strm;
	strm << "{\"type\":\"list_map\"";
	strm << ",\"content\":"
		<< Json::valueToQuotedString(name.c_str());
	strm << ",\"metadata\":" << Writer::write(metadata);
	strm << "}";
	message._str = strm.str();
	return message;
}

StreamedMessage Message::list_replay()
{
	StreamedMessage message;
	message._type = Message::Type::LIST_REPLAY;
	std::stringstream strm;
	strm << "{\"type\":\"list_replay\"";
	strm << "}";
	message._str = strm.str();
	return message;
}

StreamedMessage Message::list_replay(
		const std::string& name, const Json::Value& metadata)
{
	StreamedMessage message;
	message._type = Message::Type::LIST_REPLAY;
	std::stringstream strm;
	strm << "{\"type\":\"list_replay\"";
	strm << ",\"content\":"
		<< Json::valueToQuotedString(name.c_str());
	strm << ",\"metadata\":" << Writer::write(metadata);
	strm << "}";
	message._str = strm.str();
	return message;
}

StreamedMessage Message::list_ruleset(
		const std::string& name)
{
	StreamedMessage message;
	message._type = Message::Type::LIST_RULESET;
	std::stringstream strm;
	strm << "{\"type\":\"list_ruleset\"";
	strm << ",\"content\":"
		<< Json::valueToQuotedString(name.c_str());
	strm << "}";
	message._str = strm.str();
	return message;
}

StreamedMessage Message::list_ruleset(
		const std::string& name, const Json::Value& metadata)
{
	StreamedMessage message;
	message._type = Message::Type::LIST_RULESET;
	std::stringstream strm;
	strm << "{\"type\":\"list_ruleset\"";
	strm << ",\"content\":"
		<< Json::valueToQuotedString(name.c_str());
	strm << ",\"metadata\":" << Writer::write(metadata);
	strm << "}";
	message._str = strm.str();
	return message;
}

StreamedMessage Message::list_challenge(
		const std::string& key, const Json::Value& metadata)
{
	StreamedMessage message;
	message._type = Message::Type::LIST_CHALLENGE;
	std::stringstream strm;
	strm << "{\"type\":\"list_challenge\"";
	strm << ",\"content\":"
		<< Json::valueToQuotedString(key.c_str());
	strm << ",\"metadata\":" << Writer::write(metadata);
	strm << "}";
	message._str = strm.str();
	return message;
}

StreamedMessage Message::list_ai(
		const std::string& ainame)
{
	StreamedMessage message;
	message._type = Message::Type::LIST_AI;
	std::stringstream strm;
	strm << "{\"type\":\"list_ai\"";
	strm << ",\"content\":"
		<< Json::valueToQuotedString(ainame.c_str());
	strm << "}";
	message._str = strm.str();
	return message;
}

StreamedMessage Message::list_ai(
		const std::string& ainame, const Json::Value& metadata)
{
	StreamedMessage message;
	message._type = Message::Type::LIST_AI;
	std::stringstream strm;
	strm << "{\"type\":\"list_ai\"";
	strm << ",\"content\":"
		<< Json::valueToQuotedString(ainame.c_str());
	strm << ",\"metadata\":" << Writer::write(metadata);
	strm << "}";
	message._str = strm.str();
	return message;
}

StreamedMessage Message::ruleset_request(
		const std::string& rulesetname)
{
	StreamedMessage message;
	message._type = Message::Type::RULESET_REQUEST;
	std::stringstream strm;
	strm << "{\"type\":\"ruleset_request\"";
	strm << ",\"content\":"
		<< Json::valueToQuotedString(rulesetname.c_str());
	strm << "}";
	message._str = strm.str();
	return message;
}

StreamedMessage Message::ruleset_data(
		const std::string& rulesetname, const Json::Value& data)
{
	StreamedMessage message;
	message._type = Message::Type::RULESET_DATA;
	std::stringstream strm;
	strm << "{\"type\":\"ruleset_data\"";
	strm << ",\"content\":"
		<< Json::valueToQuotedString(rulesetname.c_str());
	strm << ",\"data\":" << Writer::write(data);
	strm << "}";
	message._str = strm.str();
	return message;
}

StreamedMessage Message::ruleset_unknown(
		const std::string& rulesetname)
{
	StreamedMessage message;
	message._type = Message::Type::RULESET_UNKNOWN;
	std::stringstream strm;
	strm << "{\"type\":\"ruleset_unknown\"";
	strm << ",\"content\":"
		<< Json::valueToQuotedString(rulesetname.c_str());
	strm << "}";
	message._str = strm.str();
	return message;
}

StreamedMessage Message::secrets(
		const Json::Value& metadata)
{
	StreamedMessage message;
	message._type = Message::Type::SECRETS;
	std::stringstream strm;
	strm << "{\"type\":\"secrets\"";
	strm << ",\"metadata\":" << Writer::write(metadata);
	strm << "}";
	message._str = strm.str();
	return message;
}

StreamedMessage Message::skins(
		const Json::Value& metadata)
{
	StreamedMessage message;
	message._type = Message::Type::SKINS;
	std::stringstream strm;
	strm << "{\"type\":\"skins\"";
	strm << ",\"metadata\":" << Writer::write(metadata);
	strm << "}";
	message._str = strm.str();
	return message;
}

StreamedMessage Message::in_game(
		const std::string& lobbyid, const std::string& username,
		const Role& role)
{
	StreamedMessage message;
	message._type = Message::Type::IN_GAME;
	std::stringstream strm;
	strm << "{\"type\":\"in_game\"";
	strm << ",\"content\":"
		<< Json::valueToQuotedString(lobbyid.c_str());
	strm << ",\"sender\":"
		<< Json::valueToQuotedString(username.c_str());
	strm << ",\"role\":\"" << role << "\"";
	strm << "}";
	message._str = strm.str();
	return message;
}

StreamedMessage Message::start()
{
	StreamedMessage message;
	message._type = Message::Type::START;
	std::stringstream strm;
	strm << "{\"type\":\"start\"";
	strm << "}";
	message._str = strm.str();
	return message;
}

StreamedMessage Message::game()
{
	StreamedMessage message;
	message._type = Message::Type::GAME;
	std::stringstream strm;
	strm << "{\"type\":\"game\"";
	strm << "}";
	message._str = strm.str();
	return message;
}

StreamedMessage Message::game(
		const Role& role, const Player& player,
		const std::string& rulesetname, uint32_t time)
{
	StreamedMessage message;
	message._type = Message::Type::GAME;
	std::stringstream strm;
	strm << "{\"type\":\"game\"";
	strm << ",\"role\":\"" << role << "\"";
	strm << ",\"player\":\"" << player << "\"";
	strm << ",\"content\":"
		<< Json::valueToQuotedString(rulesetname.c_str());
	strm << ",\"time\":" << std::to_string(time);
	strm << "}";
	message._str = strm.str();
	return message;
}

StreamedMessage Message::tutorial()
{
	StreamedMessage message;
	message._type = Message::Type::TUTORIAL;
	std::stringstream strm;
	strm << "{\"type\":\"tutorial\"";
	strm << "}";
	message._str = strm.str();
	return message;
}
StreamedMessage Message::tutorial(
		const Role& role, const Player& player,
		const std::string& rulesetname, uint32_t time)
{
	StreamedMessage message;
	message._type = Message::Type::TUTORIAL;
	std::stringstream strm;
	strm << "{\"type\":\"tutorial\"";
	strm << ",\"role\":\"" << role << "\"";
	strm << ",\"player\":\"" << player << "\"";
	strm << ",\"content\":"
		<< Json::valueToQuotedString(rulesetname.c_str());
	strm << ",\"time\":" << std::to_string(time);
	strm << "}";
	message._str = strm.str();
	return message;
}

StreamedMessage Message::challenge()
{
	StreamedMessage message;
	message._type = Message::Type::CHALLENGE;
	std::stringstream strm;
	strm << "{\"type\":\"challenge\"";
	strm << "}";
	message._str = strm.str();
	return message;
}

StreamedMessage Message::replay(
		bool enabled)
{
	StreamedMessage message;
	message._type = Message::Type::REPLAY;
	std::stringstream strm;
	strm << "{\"type\":\"replay\"";
	strm << ",\"time\":" << std::to_string(int(enabled));
	strm << "}";
	message._str = strm.str();
	return message;
}

StreamedMessage Message::briefing(
		const Json::Value& metadata)
{
	StreamedMessage message;
	message._type = Message::Type::BRIEFING;
	std::stringstream strm;
	strm << "{\"type\":\"briefing\"";
	strm << ",\"metadata\":" << Writer::write(metadata);
	strm << "}";
	message._str = strm.str();
	return message;
}

StreamedMessage Message::resign()
{
	StreamedMessage message;
	message._type = Message::Type::RESIGN;
	std::stringstream strm;
	strm << "{\"type\":\"resign\"";
	strm << "}";
	message._str = strm.str();
	return message;
}

StreamedMessage Message::resign(
		const std::string& username)
{
	StreamedMessage message;
	message._type = Message::Type::RESIGN;
	std::stringstream strm;
	strm << "{\"type\":\"resign\"";
	strm << ",\"content\":"
		<< Json::valueToQuotedString(username.c_str());
	strm << "}";
	message._str = strm.str();
	return message;
}

StreamedMessage Message::change(
		const TypeNamer& typenamer,
		const std::vector<Change>& changes)
{
	StreamedMessage message;
	message._type = Message::Type::CHANGE;
	std::stringstream strm;
	strm << "{\"type\":\"change\"";
	strm << ",\"changes\":[";
	strm << TypeEncoder(&typenamer);
	bool empty = true;
	for (auto& change : changes)
	{
		if (empty) empty = false;
		else strm << ",";
		strm << change;
	}
	strm << "]";
	strm << "}";
	message._str = strm.str();
	return message;
}

StreamedMessage Message::order_old(
		const TypeNamer& typenamer,
		const std::vector<Order>& orders)
{
	StreamedMessage message;
	message._type = Message::Type::ORDER_OLD;
	std::stringstream strm;
	strm << "{\"type\":\"order_old\"";
	strm << ",\"orders\":[";
	strm << TypeEncoder(&typenamer);
	bool empty = true;
	for (auto& order : orders)
	{
		if (empty) empty = false;
		else strm << ",";
		strm << order;
	}
	strm << "]";
	strm << "}";
	message._str = strm.str();
	return message;
}

StreamedMessage Message::order_new(
		const TypeNamer& typenamer,
		const std::vector<Order>& orders)
{
	StreamedMessage message;
	message._type = Message::Type::ORDER_NEW;
	std::stringstream strm;
	strm << "{\"type\":\"order_new\"";
	strm << ",\"orders\":[";
	strm << TypeEncoder(&typenamer);
	bool empty = true;
	for (auto& order : orders)
	{
		if (empty) empty = false;
		else strm << ",";
		strm << order;
	}
	strm << "]";
	strm << "}";
	message._str = strm.str();
	return message;
}

StreamedMessage Message::order_new(
		const TypeNamer& typenamer,
		const std::vector<Order>& orders,
		const Json::Value& metadata)
{
	StreamedMessage message;
	message._type = Message::Type::ORDER_NEW;
	std::stringstream strm;
	strm << "{\"type\":\"order_new\"";
	strm << ",\"metadata\":" << Writer::write(metadata);
	strm << ",\"orders\":[";
	strm << TypeEncoder(&typenamer);
	bool empty = true;
	for (auto& order : orders)
	{
		if (empty) empty = false;
		else strm << ",";
		strm << order;
	}
	strm << "]";
	strm << "}";
	message._str = strm.str();
	return message;
}

StreamedMessage Message::sync()
{
	StreamedMessage message;
	message._type = Message::Type::SYNC;
	std::stringstream strm;
	strm << "{\"type\":\"sync\"";
	strm << "}";
	message._str = strm.str();
	return message;
}

StreamedMessage Message::sync(
		uint32_t time)
{
	StreamedMessage message;
	message._type = Message::Type::SYNC;
	std::stringstream strm;
	strm << "{\"type\":\"sync\"";
	strm << ",\"time\":" << std::to_string(time);
	strm << "}";
	message._str = strm.str();
	return message;
}

StreamedMessage Message::ping()
{
	StreamedMessage message;
	message._type = Message::Type::PING;
	std::stringstream strm;
	strm << "{\"type\":\"ping\"";
	strm << "}";
	message._str = strm.str();
	return message;
}

StreamedMessage Message::pong()
{
	StreamedMessage message;
	message._type = Message::Type::PONG;
	std::stringstream strm;
	strm << "{\"type\":\"pong\"";
	strm << "}";
	message._str = strm.str();
	return message;
}

StreamedMessage Message::quit()
{
	StreamedMessage message;
	message._type = Message::Type::QUIT;
	std::stringstream strm;
	strm << "{\"type\":\"quit\"";
	strm << "}";
	message._str = strm.str();
	return message;
}

StreamedMessage Message::closing()
{
	StreamedMessage message;
	message._type = Message::Type::CLOSING;
	std::stringstream strm;
	strm << "{\"type\":\"closing\"";
	strm << "}";
	message._str = strm.str();
	return message;
}

StreamedMessage Message::closed()
{
	StreamedMessage message;
	message._type = Message::Type::CLOSED;
	std::stringstream strm;
	strm << "{\"type\":\"closed\"";
	strm << "}";
	message._str = strm.str();
	return message;
}

StreamedMessage Message::version(
		const Version& vers)
{
	StreamedMessage message;
	message._type = Message::Type::VERSION;
	std::stringstream strm;
	strm << "{\"type\":\"version\"";
	strm << ",\"version\":\"" << vers << "\"";
	strm << "}";
	message._str = strm.str();
	return message;
}

StreamedMessage Message::version(
		const Version& vers, const Json::Value& metadata)
{
	StreamedMessage message;
	message._type = Message::Type::VERSION;
	std::stringstream strm;
	strm << "{\"type\":\"version\"";
	strm << ",\"version\":\"" << vers << "\"";
	strm << ",\"metadata\":" << Writer::write(metadata);
	strm << "}";
	message._str = strm.str();
	return message;
}

StreamedMessage Message::patch()
{
	StreamedMessage message;
	message._type = Message::Type::PATCH;
	std::stringstream strm;
	strm << "{\"type\":\"patch\"";
	strm << "}";
	message._str = strm.str();
	return message;
}

StreamedMessage Message::patch(
		const Version& vers)
{
	StreamedMessage message;
	message._type = Message::Type::PATCH;
	std::stringstream strm;
	strm << "{\"type\":\"patch\"";
	strm << ",\"version\":\"" << vers << "\"";
	strm << "}";
	message._str = strm.str();
	return message;
}

StreamedMessage Message::patch(
		const Version& vers, const Json::Value& metadata)
{
	StreamedMessage message;
	message._type = Message::Type::PATCH;
	std::stringstream strm;
	strm << "{\"type\":\"patch\"";
	strm << ",\"version\":\"" << vers << "\"";
	strm << ",\"metadata\":" << Writer::write(metadata);
	strm << "}";
	message._str = strm.str();
	return message;
}

StreamedMessage Message::download(
		const std::string& name, const Json::Value& metadata)
{
	StreamedMessage message;
	message._type = Message::Type::DOWNLOAD;
	std::stringstream strm;
	strm << "{\"type\":\"download\"";
	strm << ",\"content\":"
		<< Json::valueToQuotedString(name.c_str());
	strm << ",\"metadata\":" << Writer::write(metadata);
	strm << "}";
	message._str = strm.str();
	return message;
}

StreamedMessage Message::request_fulfilled(
		const std::string& name, const Json::Value& metadata)
{
	StreamedMessage message;
	message._type = Message::Type::REQUEST_FULFILLED;
	std::stringstream strm;
	strm << "{\"type\":\"request_fulfilled\"";
	strm << ",\"content\":"
		<< Json::valueToQuotedString(name.c_str());
	strm << ",\"metadata\":" << Writer::write(metadata);
	strm << "}";
	message._str = strm.str();
	return message;
}

StreamedMessage Message::request_denied(
		const std::string& name, const Json::Value& metadata)
{
	StreamedMessage message;
	message._type = Message::Type::REQUEST_DENIED;
	std::stringstream strm;
	strm << "{\"type\":\"request_denied\"";
	strm << ",\"content\":"
		<< Json::valueToQuotedString(name.c_str());
	strm << ",\"metadata\":" << Writer::write(metadata);
	strm << "}";
	message._str = strm.str();
	return message;
}

StreamedMessage Message::enable_compression(
		const std::string& modelname)
{
	StreamedMessage message;
	message._type = Message::Type::ENABLE_COMPRESSION;
	std::stringstream strm;
	strm << "{\"type\":\"enable_compression\"";
	strm << ",\"content\":"
		<< Json::valueToQuotedString(modelname.c_str());
	strm << "}";
	message._str = strm.str();
	return message;
}

StreamedMessage Message::disable_compression()
{
	StreamedMessage message;
	message._type = Message::Type::DISABLE_COMPRESSION;
	std::stringstream strm;
	strm << "{\"type\":\"disable_compression\"";
	strm << "}";
	message._str = strm.str();
	return message;
}

StreamedMessage Message::rankings(
		const Json::Value& metadata)
{
	StreamedMessage message;
	message._type = Message::Type::RANKINGS;
	std::stringstream strm;
	strm << "{\"type\":\"rankings\"";
	strm << ",\"metadata\":" << Writer::write(metadata);
	strm << "}";
	message._str = strm.str();
	return message;
}

StreamedMessage Message::stamp(
		const Json::Value& metadata)
{
	StreamedMessage message;
	message._type = Message::Type::STAMP;
	std::stringstream strm;
	strm << "{\"type\":\"stamp\"";
	strm << ",\"metadata\":" << Writer::write(metadata);
	strm << "}";
	message._str = strm.str();
	return message;
}

StreamedMessage Message::link_accounts(
		const Json::Value& metadata)
{
	StreamedMessage message;
	message._type = Message::Type::LINK_ACCOUNTS;
	std::stringstream strm;
	strm << "{\"type\":\"link_accounts\"";
	strm << ",\"metadata\":" << Writer::write(metadata);
	strm << "}";
	message._str = strm.str();
	return message;
}

StreamedMessage Message::pulse()
{
	StreamedMessage message;
	message._type = Message::Type::PULSE;
	std::stringstream strm;
	strm << "{\"type\":\"pulse\"";
	strm << "}";
	message._str = strm.str();
	return message;
}

StreamedMessage::StreamedMessage() :
	_type(Type::INVALID)
{}

std::string StreamedMessage::str() const
{
	return _str;
}

std::ostream& operator<<(std::ostream& os, const StreamedMessage& message)
{
	return os << message._str;
}

ParsedMessage Message::parse(const char* buffer, uint32_t length)
{
	Json::Reader reader;
	Json::Value json;
	if (!reader.parse(std::string(buffer, length), json))
	{
		throw ParseError("Invalid json");
	}
	return ParsedMessage(std::move(json));
}

ParsedMessage::ParsedMessage() :
	_json(Json::nullValue),
	_type(Type::INVALID)
{}

ParsedMessage::ParsedMessage(Json::Value&& json) :
	_json(json),
	_type(parseMessageType(json["type"].asString()))
{
	switch (_type)
	{
		case Type::INIT:
		break;
		case Type::CHAT:
		{
			_content = _json["content"].asString();
			if (_json["sender"].isString())
			{
				_sender = _json["sender"].asString();
			}
			if (_json["target"].isString())
			{
				_target = parseTarget(_json["target"].asString());
			}
		}
		break;
		case Type::RATING_AND_STARS:
		{
			if (_json["content"].isString())
			{
				_content = _json["content"].asString();
			}
			if (_json["rating"].isDouble())
			{
				_rating = _json["rating"].asFloat();
			}
			if (_json["time"].isUInt())
			{
				_time = _json["time"].asUInt();
			}
		}
		break;
		case Type::RATING:
		{
			if (_json["content"].isString())
			{
				_content = _json["content"].asString();
			}
			if (_json["rating"].isDouble())
			{
				_rating = _json["rating"].asFloat();
			}
		}
		break;
		case Type::STARS:
		{
			if (_json["content"].isString())
			{
				_content = _json["content"].asString();
			}
			if (_json["time"].isUInt())
			{
				_time = _json["time"].asUInt();
			}
		}
		break;
		case Type::RECENT_STARS:
		{
			if (_json["time"].isUInt())
			{
				_time = _json["time"].asUInt();
			}
		}
		break;
		case Type::JOIN_SERVER:
		{
			if (json["status"].isUInt())
			{
				_status = ResponseStatus(json["status"].asUInt());
			}
			else
			{
				if (_json["content"].isString())
				{
					_content = _json["content"].asString();
				}
				if (_json["sender"].isString())
				{
					_sender = _json["sender"].asString();
				}
			}
		}
		break;
		case Type::LEAVE_SERVER:
		{
			if (_json["content"].isString())
			{
				_content = _json["content"].asString();
			}
		}
		break;
		case Type::JOIN_LOBBY:
		{
			if (_json["content"].isString())
			{
				_content = _json["content"].asString();
			}
			if (_json["sender"].isString())
			{
				_sender = _json["sender"].asString();
			}
		}
		break;
		case Type::LEAVE_LOBBY:
		{
			if (_json["content"].isString())
			{
				_content = _json["content"].asString();
			}
			if (_json["sender"].isString())
			{
				_sender = _json["sender"].asString();
			}
		}
		break;
		case Type::LIST_LOBBY:
		{
			if (_json["content"].isString())
			{
				_content = _json["content"].asString();
			}
			if (_json["sender"].isString())
			{
				_sender = _json["sender"].asString();
			}
		}
		break;
		case Type::MAKE_LOBBY:
		{
			if (_json["content"].isString())
			{
				_content = _json["content"].asString();
			}
		}
		break;
		case Type::DISBAND_LOBBY:
		case Type::EDIT_LOBBY:
		case Type::SAVE_LOBBY:
		case Type::LOCK_LOBBY:
		case Type::UNLOCK_LOBBY:
		{
			if (_json["content"].isString())
			{
				_content = _json["content"].asString();
			}
		}
		break;
		case Type::NAME_LOBBY:
		{
			if (_json["content"].isString())
			{
				_content = _json["content"].asString();
			}
			if (_json["sender"].isString())
			{
				_sender = _json["sender"].asString();
			}
		}
		break;
		case Type::MAX_PLAYERS:
		case Type::NUM_PLAYERS:
		{
			if (_json["time"].isUInt())
			{
				_time = _json["time"].asUInt();
			}
			if (_json["content"].isString())
			{
				_content = _json["content"].asString();
			}
		}
		break;
		case Type::CLAIM_ROLE:
		{
			if (_json["sender"].isString())
			{
				_sender = _json["sender"].asString();
			}
			if (_json["role"].isString())
			{
				_role = parseRole(_json["role"].asString());
			}
		}
		break;
		case Type::CLAIM_COLOR:
		{
			if (_json["sender"].isString())
			{
				_sender = _json["sender"].asString();
			}
			if (_json["player"].isString())
			{
				_player = parsePlayer(_json["player"].asString());
			}
		}
		break;
		case Type::CLAIM_VISIONTYPE:
		{
			if (_json["sender"].isString())
			{
				_sender = _json["sender"].asString();
			}
			if (_json["visiontype"].isString())
			{
				_visiontype = parseVisionType(json["visiontype"].asString());
			}
		}
		break;
		case Type::CLAIM_AI:
		{
			if (_json["content"].isString())
			{
				_content = _json["content"].asString();
			}
			if (_json["sender"].isString())
			{
				_sender = _json["sender"].asString();
			}
		}
		break;
		case Type::CLAIM_DIFFICULTY:
		{
			if (_json["sender"].isString())
			{
				_sender = _json["sender"].asString();
			}
			if (_json["difficulty"].isString())
			{
				_difficulty = parseDifficulty(json["difficulty"].asString());
			}
		}
		break;
		case Type::PICK_MAP:
		{
			if (_json["content"].isString())
			{
				_content = _json["content"].asString();
			}
		}
		break;
		case Type::PICK_TIMER:
		{
			if (_json["time"].isUInt())
			{
				_time = _json["time"].asUInt();
			}
		}
		break;
		case Type::PICK_REPLAY:
		case Type::PICK_CHALLENGE:
		case Type::PICK_RULESET:
		case Type::ADD_BOT:
		case Type::REMOVE_BOT:
		case Type::LIST_MAP:
		case Type::LIST_REPLAY:
		case Type::LIST_RULESET:
		case Type::LIST_CHALLENGE:
		case Type::LIST_AI:
		case Type::RULESET_REQUEST:
		case Type::RULESET_DATA:
		case Type::RULESET_UNKNOWN:
		{
			if (_json["content"].isString())
			{
				_content = _json["content"].asString();
			}
		}
		break;
		case Type::ENABLE_CUSTOM_MAPS:
		case Type::SECRETS:
		case Type::SKINS:
		break;
		case Type::IN_GAME:
		{
			if (_json["content"].isString())
			{
				_content = _json["content"].asString();
			}
			if (_json["sender"].isString())
			{
				_sender = _json["sender"].asString();
			}
			if (_json["role"].isString())
			{
				_role = parseRole(_json["role"].asString());
			}
		}
		break;
		case Type::START:
		break;
		case Type::GAME:
		case Type::TUTORIAL:
		case Type::CHALLENGE:
		{
			if (_json["role"].isString() || _json["player"].isString())
			{
				if (_json["role"].isString())
				{
					_role = parseRole(_json["role"].asString());
				}
				if (_json["player"].isString())
				{
					_player = parsePlayer(_json["player"].asString());
				}
				if (_json["content"].isString())
				{
					_content = _json["content"].asString();
				}
				else
				{
					_content = Version(_json["version"]).name();
				}
				if (_json["time"].isUInt())
				{
					_time = _json["time"].asUInt();
				}
			}
			if (_json["difficulty"].isString())
			{
				_difficulty = parseDifficulty(json["difficulty"].asString());
			}
		}
		break;
		case Type::RESTORE:
		{
			if (_json["content"].isString())
			{
				_content = _json["content"].asString();
			}
			if (_json["time"].isUInt())
			{
				_time = _json["time"].asUInt();
			}
		}
		break;
		case Type::REPLAY:
		{
			if (_json["time"].isUInt())
			{
				_time = _json["time"].asUInt();
			}
		}
		break;
		case Type::RESIGN:
		{
			if (_json["content"].isString())
			{
				_content = _json["content"].asString();
			}
		}
		break;
		case Type::BRIEFING:
		break;
		case Type::CHANGE:
		{
			if (!_json["changes"].isArray())
			{
				throw ParseError("No changes array");
			}
		}
		break;
		case Type::ORDER_OLD:
		case Type::ORDER_NEW:
		{
			if (!_json["orders"].isArray())
			{
				throw ParseError("No orders array");
			}
		}
		break;
		case Type::SYNC:
		{
			if (_json["time"].isUInt())
			{
				_time = _json["time"].asUInt();
			}
		}
		break;
		case Type::PING:
		case Type::PONG:
		case Type::QUIT:
		case Type::CLOSING:
		case Type::CLOSED:
		break;
		case Type::VERSION:
		case Type::PATCH:
		{
			if (!_json["version"].isNull())
			{
				_version = Version(_json["version"]);
			}
		}
		break;
		case Type::DOWNLOAD:
		case Type::REQUEST:
		case Type::REQUEST_DENIED:
		case Type::REQUEST_FULFILLED:
		case Type::ENABLE_COMPRESSION:
		{
			_content = _json["content"].asString();
		}
		break;
		case Type::DISABLE_COMPRESSION:
		case Type::RANKINGS:
		case Type::STAMP:
		case Type::LINK_ACCOUNTS:
		case Type::PULSE:
		break;

		case Type::INVALID:
		break;
	}
}

Message::Type Message::parseMessageType(const std::string& type)
{
	if      (type == "init") return Type::INIT;
	else if (type == "chat") return Type::CHAT;
	else if (type == "rating_and_stars") return Type::RATING_AND_STARS;
	else if (type == "rating") return Type::RATING;
	else if (type == "stars") return Type::STARS;
	else if (type == "recent_stars") return Type::RECENT_STARS;
	else if (type == "join_server") return Type::JOIN_SERVER;
	else if (type == "leave_server") return Type::LEAVE_SERVER;
	else if (type == "join_lobby") return Type::JOIN_LOBBY;
	else if (type == "leave_lobby") return Type::LEAVE_LOBBY;
	else if (type == "list_lobby") return Type::LIST_LOBBY;
	else if (type == "make_lobby") return Type::MAKE_LOBBY;
	else if (type == "disband_lobby") return Type::DISBAND_LOBBY;
	else if (type == "edit_lobby") return Type::EDIT_LOBBY;
	else if (type == "save_lobby") return Type::SAVE_LOBBY;
	else if (type == "lock_lobby") return Type::LOCK_LOBBY;
	else if (type == "unlock_lobby") return Type::UNLOCK_LOBBY;
	else if (type == "name_lobby") return Type::NAME_LOBBY;
	else if (type == "max_players") return Type::MAX_PLAYERS;
	else if (type == "num_players") return Type::NUM_PLAYERS;
	else if (type == "claim_role") return Type::CLAIM_ROLE;
	else if (type == "claim_color") return Type::CLAIM_COLOR;
	else if (type == "assign_color") return Type::CLAIM_COLOR;
	else if (type == "claim_visiontype") return Type::CLAIM_VISIONTYPE;
	else if (type == "claim_ai") return Type::CLAIM_AI;
	else if (type == "claim_difficulty") return Type::CLAIM_DIFFICULTY;
	else if (type == "pick_map") return Type::PICK_MAP;
	else if (type == "pick_timer") return Type::PICK_TIMER;
	else if (type == "pick_replay") return Type::PICK_REPLAY;
	else if (type == "pick_challenge") return Type::PICK_CHALLENGE;
	else if (type == "pick_ruleset") return Type::PICK_RULESET;
	else if (type == "add_bot") return Type::ADD_BOT;
	else if (type == "remove_bot") return Type::REMOVE_BOT;
	else if (type == "enable_custom_maps") return Type::ENABLE_CUSTOM_MAPS;
	else if (type == "list_ai") return Type::LIST_AI;
	else if (type == "list_map") return Type::LIST_MAP;
	else if (type == "list_replay") return Type::LIST_REPLAY;
	else if (type == "list_ruleset") return Type::LIST_RULESET;
	else if (type == "list_challenge") return Type::LIST_CHALLENGE;
	else if (type == "ruleset_request") return Type::RULESET_REQUEST;
	else if (type == "ruleset_data") return Type::RULESET_DATA;
	else if (type == "ruleset_unknown") return Type::RULESET_UNKNOWN;
	else if (type == "secrets") return Type::SECRETS;
	else if (type == "skins") return Type::SKINS;
	else if (type == "in_game") return Type::IN_GAME;
	else if (type == "start") return Type::START;
	else if (type == "game") return Type::GAME;
	else if (type == "restore") return Type::RESTORE;
	else if (type == "replay") return Type::REPLAY;
	else if (type == "tutorial") return Type::TUTORIAL;
	else if (type == "challenge") return Type::CHALLENGE;
	else if (type == "briefing") return Type::BRIEFING;
	else if (type == "resign") return Type::RESIGN;
	else if (type == "change") return Type::CHANGE;
	else if (type == "order_old") return Type::ORDER_OLD;
	else if (type == "order_new") return Type::ORDER_NEW;
	else if (type == "sync") return Type::SYNC;
	else if (type == "ping") return Type::PING;
	else if (type == "pong") return Type::PONG;
	else if (type == "quit") return Type::QUIT;
	else if (type == "closing") return Type::CLOSING;
	else if (type == "closed") return Type::CLOSED;
	else if (type == "version") return Type::VERSION;
	else if (type == "patch") return Type::PATCH;
	else if (type == "download") return Type::DOWNLOAD;
	else if (type == "request") return Type::REQUEST;
	else if (type == "request_denied") return Type::REQUEST_DENIED;
	else if (type == "request_fulfilled") return Type::REQUEST_FULFILLED;
	else if (type == "enable_compression") return Type::ENABLE_COMPRESSION;
	else if (type == "disable_compression") return Type::DISABLE_COMPRESSION;
	else if (type == "rankings") return Type::RANKINGS;
	else if (type == "stamp") return Type::STAMP;
	else if (type == "link_accounts") return Type::LINK_ACCOUNTS;
	else if (type == "pulse") return Type::PULSE;
	else if (type == "invalid") return Type::INVALID;
	else throw ParseError("Unknown message type '" + type + "'");
}

const Json::Value& ParsedMessage::metadata() const
{
	return _json["metadata"];
}

const Json::Value& ParsedMessage::changes() const
{
	return _json["changes"];
}

const Json::Value& ParsedMessage::orders() const
{
	return _json["orders"];
}

const Json::Value& ParsedMessage::data() const
{
	return _json["data"];
}

std::ostream& operator<<(std::ostream& os, const ParsedMessage& message)
{
	return os << Writer::write(message._json);
}

bool Message::compressible(const Type& type)
{
	switch (type)
	{
		case Type::INIT:
		case Type::RATING_AND_STARS:
		case Type::RATING:
		case Type::STARS:
		case Type::RECENT_STARS:
		case Type::JOIN_SERVER:
		case Type::LEAVE_SERVER:
		case Type::JOIN_LOBBY:
		case Type::LEAVE_LOBBY:
		case Type::LIST_LOBBY:
		case Type::MAKE_LOBBY:
		case Type::DISBAND_LOBBY:
		case Type::EDIT_LOBBY:
		case Type::SAVE_LOBBY:
		case Type::LOCK_LOBBY:
		case Type::UNLOCK_LOBBY:
		case Type::MAX_PLAYERS:
		case Type::NUM_PLAYERS:
		case Type::CLAIM_ROLE:
		case Type::CLAIM_COLOR:
		case Type::CLAIM_VISIONTYPE:
		case Type::CLAIM_AI:
		case Type::CLAIM_DIFFICULTY:
		case Type::PICK_MAP:
		case Type::PICK_TIMER:
		case Type::PICK_REPLAY:
		case Type::PICK_CHALLENGE:
		case Type::PICK_RULESET:
		case Type::ADD_BOT:
		case Type::REMOVE_BOT:
		case Type::ENABLE_CUSTOM_MAPS:
		case Type::LIST_AI:
		case Type::LIST_MAP:
		case Type::LIST_REPLAY:
		case Type::LIST_RULESET:
		case Type::LIST_CHALLENGE:
		case Type::RULESET_REQUEST:
		case Type::RULESET_DATA:
		case Type::RULESET_UNKNOWN:
		case Type::SECRETS:
		case Type::SKINS:
		case Type::IN_GAME:
		case Type::START:
		case Type::GAME:
		case Type::RESTORE:
		case Type::REPLAY:
		case Type::TUTORIAL:
		case Type::CHALLENGE:
		case Type::BRIEFING:
		case Type::RESIGN:
		case Type::CHANGE:
		case Type::ORDER_OLD:
		case Type::ORDER_NEW:
		case Type::SYNC:
		case Type::PING:
		case Type::PONG:
		case Type::QUIT:
		case Type::CLOSING:
		case Type::CLOSED:
		case Type::RANKINGS:
		case Type::STAMP:
		case Type::LINK_ACCOUNTS:
		case Type::PULSE:
		return true;

		// Version and filenames are too precious to compress.
		case Type::VERSION:
		case Type::PATCH:
		case Type::REQUEST:
		case Type::REQUEST_DENIED:
		case Type::REQUEST_FULFILLED:
		case Type::ENABLE_COMPRESSION:
		case Type::DISABLE_COMPRESSION:
		return false;

		// Downloads are raw bytes hence likely incompressible. If we want to
		// compress downloads, we should download actual ".zip" files.
		case Type::DOWNLOAD:
		return false;

		// Messages containing a lot of human input are unpredictable
		// (especially once we add Unicode support) and hence difficult to
		// compress with a premade dictionary.
		case Type::CHAT:
		case Type::NAME_LOBBY:
		return false;

		// We should never send invalid messages, let alone compress them.
		case Type::INVALID:
		return false;
	}

	return false;
}
