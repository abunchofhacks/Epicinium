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
#include "source.hpp"

#include "automaton.hpp"
#include "change.hpp"
#include "order.hpp"
#include "map.hpp"
#include "ai.hpp"
#include "challenge.hpp"
#include "aichallenge.hpp"
#include "aicommander.hpp"
#include "ailibrary.hpp"
#include "difficulty.hpp"
#include "library.hpp"
#include "recording.hpp"
#include "loginstaller.hpp"
#include "language.hpp"


struct Buffer
{
	std::string str;
};

extern "C"
{
	Automaton* epicinium_automaton_allocate(size_t playercount,
		const char* ruleset_name);
	void epicinium_automaton_deallocate(Automaton* automaton);

	void epicinium_automaton_add_player(Automaton* automaton, uint8_t player);
	void epicinium_automaton_grant_global_vision(Automaton* automaton,
		uint8_t player);
	void epicinium_automaton_load_map(Automaton* automaton,
		const char* map_name, bool shuffleplayers);
	void epicinium_automaton_restore(Automaton* automaton,
		const char* recording_name);
	void epicinium_automaton_load_replay(Automaton* automaton,
		const char* recording_name);
	void epicinium_automaton_start_recording(Automaton* automaton,
		const char* metadata, const char* recording_name);
	void epicinium_automaton_set_challenge(Automaton* automaton,
		uint16_t challenge);
	bool epicinium_automaton_is_active(Automaton* automaton);
	bool epicinium_automaton_is_replay_active(Automaton* automaton);
	const char* epicinium_automaton_act(Automaton* automaton, Buffer* buffer);
	bool epicinium_automaton_is_gameover(Automaton* automaton);
	bool epicinium_automaton_is_defeated(Automaton* automaton, uint8_t player);
	uint32_t epicinium_automaton_current_round(Automaton* automaton);
	int32_t epicinium_automaton_global_score(Automaton* automaton);
	int32_t epicinium_automaton_score(Automaton* automaton, uint8_t player);
	int32_t epicinium_automaton_award(Automaton* automaton, uint8_t player);
	void epicinium_automaton_resign(Automaton* automaton, uint8_t player);
	const char* epicinium_automaton_hibernate(Automaton* automaton,
		Buffer* buffer);
	const char* epicinium_automaton_awake(Automaton* automaton, Buffer* buffer);
	void epicinium_automaton_receive(Automaton* automaton, uint8_t player,
		const char* orders);
	const char* epicinium_automaton_prepare(Automaton* automaton,
		Buffer* buffer);
	const char* epicinium_automaton_rejoin(Automaton* automaton,
		uint8_t player, Buffer* buffer);

	size_t epicinium_map_pool_size();
	const char* epicinium_map_pool_get(size_t i);
	size_t epicinium_map_custom_pool_size();
	const char* epicinium_map_custom_pool_get(size_t i);
	size_t epicinium_map_user_pool_size();
	const char* epicinium_map_user_pool_get(size_t i);

	bool epicinium_ruleset_initialize_collection();
	const char* epicinium_ruleset_current_name();
	bool epicinium_ruleset_exists(const char* name);

	size_t epicinium_ai_pool_size();
	const char* epicinium_ai_pool_get(size_t i);
	bool epicinium_ai_exists(const char* name);

	AILibrary* epicinium_ai_allocate(const char* name,
		uint8_t player, uint8_t difficulty,
		const char* ruleset_name, char character);
	void epicinium_ai_deallocate(AILibrary* ai);

	void epicinium_ai_receive(AILibrary* ai, const char* changes);
	void epicinium_ai_prepare_orders(AILibrary* ai);
	const char* epicinium_ai_retrieve_orders(AILibrary* ai,
		Buffer* buffer);
	const char* epicinium_ai_descriptive_name(AILibrary* ai,
		Buffer* buffer);
	const char* epicinium_ai_descriptive_metadata(AILibrary* ai,
		Buffer* buffer);

	uint16_t epicinium_current_challenge_id();
	const char* epicinium_challenge_key(uint16_t id);
	size_t epicinium_challenge_num_bots(uint16_t id);
	const char* epicinium_challenge_bot_name(uint16_t id);
	uint8_t epicinium_challenge_bot_difficulty(uint16_t id);
	const char* epicinium_challenge_map_name(uint16_t id);
	const char* epicinium_challenge_ruleset_name(uint16_t id);
	const char* epicinium_challenge_display_name(uint16_t id);
	const char* epicinium_challenge_panel_picture_name(uint16_t id);
	const char* epicinium_challenge_discord_image_key(uint16_t id);
	const char* epicinium_challenge_steam_short_key(uint16_t id);
	size_t epicinium_challenge_briefing_size(uint16_t id);
	const char* epicinium_challenge_briefing_key(uint16_t id, size_t i);
	const char* epicinium_challenge_briefing_value(uint16_t id, size_t i);

	Buffer* epicinium_buffer_allocate();
	void epicinium_buffer_deallocate(Buffer* buffer);

	void epicinium_log_initialize(log_callback_fn callback, uint8_t severity);
}

static std::unique_ptr<Library> _ruleset_collection;
static std::string _ruleset_current_name;

extern "C"
{
	Automaton* epicinium_automaton_allocate(size_t playercount,
		const char* ruleset)
	{
		try
		{
			return new Automaton(playercount, ruleset);
		}
		catch (const std::exception& e)
		{
			// If Automaton creation throws, we return null, in which case
			// the Automaton need not and must not be deallocated.
			LOGE << "Exception: " << e.what();
			return nullptr;
		}
	}
	void epicinium_automaton_deallocate(Automaton* automaton)
	{
		delete automaton;
	}

	void epicinium_automaton_add_player(Automaton* automaton,
		uint8_t player_as_u8)
	{
		Player player = (Player) player_as_u8;
		automaton->addPlayer(player);
	}
	void epicinium_automaton_grant_global_vision(Automaton* automaton,
		uint8_t player_as_u8)
	{
		Player player = (Player) player_as_u8;
		automaton->grantGlobalVision(player);
	}
	void epicinium_automaton_load_map(Automaton* automaton,
		const char* map_name, bool shuffleplayers)
	{
		try
		{
			automaton->load(map_name, shuffleplayers);
		}
		catch (const std::exception& e)
		{
			LOGE << "Exception: " << e.what();
			return;
		}
	}
	void epicinium_automaton_restore(Automaton* automaton,
		const char* recording_name)
	{
		// TODO implement
		(void) automaton;
		(void) recording_name;
		LOGE << "Unimplemented.";
	}
	void epicinium_automaton_load_replay(Automaton* automaton,
		const char* recording_name)
	{
		try
		{
			Recording recording(recording_name);
			automaton->replay(recording);
		}
		catch (const std::exception& e)
		{
			LOGE << "Exception: " << e.what();
			return;
		}
	}
	void epicinium_automaton_start_recording(Automaton* automaton,
		const char* metadata_as_cstr, const char* recording_name)
	{
		try
		{
			Json::Reader reader;
			Json::Value metadata;
			if (!reader.parse(metadata_as_cstr, metadata))
			{
				LOGE << "Failed to parse metadata.";
				return;
			}

			automaton->startRecording(metadata, recording_name);
		}
		catch (const std::exception& e)
		{
			LOGE << "Exception: " << e.what();
			return;
		}
	}
	void epicinium_automaton_set_challenge(Automaton* automaton,
		uint16_t challenge_id_as_u16)
	{
		Challenge::Id challenge_id = (Challenge::Id) challenge_id_as_u16;
		auto challenge = std::make_shared<Challenge>(challenge_id);
		automaton->setChallenge(challenge);
	}
	bool epicinium_automaton_is_active(Automaton* automaton)
	{
		return automaton->active();
	}
	bool epicinium_automaton_is_replay_active(Automaton* automaton)
	{
		return automaton->replaying();
	}
	const char* epicinium_automaton_act(Automaton* automaton, Buffer* buffer)
	{
		ChangeSet cset = automaton->act();
		std::stringstream strm;
		strm << TypeEncoder(&(automaton->bible())) << cset;
		buffer->str = strm.str();
		return buffer->str.c_str();
	}
	bool epicinium_automaton_is_gameover(Automaton* automaton)
	{
		return automaton->gameover();
	}
	bool epicinium_automaton_is_defeated(Automaton* automaton,
		uint8_t player_as_u8)
	{
		Player player = (Player) player_as_u8;
		return automaton->defeated(player);
	}
	uint32_t epicinium_automaton_current_round(Automaton* automaton)
	{
		return automaton->round();
	}
	int32_t epicinium_automaton_global_score(Automaton* automaton)
	{
		return automaton->globalScore();
	}
	int32_t epicinium_automaton_score(Automaton* automaton,
		uint8_t player_as_u8)
	{
		Player player = (Player) player_as_u8;
		return automaton->score(player);
	}
	int32_t epicinium_automaton_award(Automaton* automaton,
		uint8_t player_as_u8)
	{
		Player player = (Player) player_as_u8;
		return automaton->award(player);
	}
	void epicinium_automaton_resign(Automaton* automaton,
		uint8_t player_as_u8)
	{
		Player player = (Player) player_as_u8;
		automaton->resign(player);
	}
	const char* epicinium_automaton_hibernate(Automaton* automaton,
		Buffer* buffer)
	{
		ChangeSet cset = automaton->hibernate();
		std::stringstream strm;
		strm << TypeEncoder(&(automaton->bible())) << cset;
		buffer->str = strm.str();
		return buffer->str.c_str();
	}
	const char* epicinium_automaton_awake(Automaton* automaton, Buffer* buffer)
	{
		ChangeSet cset = automaton->awake();
		std::stringstream strm;
		strm << TypeEncoder(&(automaton->bible())) << cset;
		buffer->str = strm.str();
		return buffer->str.c_str();
	}
	void epicinium_automaton_receive(Automaton* automaton, uint8_t player_as_u8,
		const char* orders_as_cstr)
	{
		Player player = (Player) player_as_u8;
		std::string orders_as_str = std::string(orders_as_cstr);
		std::vector<Order> orders;
		try
		{
			orders = Order::parseOrders(automaton->bible(), orders_as_str);
		}
		catch (const std::exception& e)
		{
			LOGE << "Exception: " << e.what();
			return;
		}
		automaton->receive(player, orders);
	}
	const char* epicinium_automaton_prepare(Automaton* automaton,
		Buffer* buffer)
	{
		ChangeSet cset = automaton->prepare();
		std::stringstream strm;
		strm << TypeEncoder(&(automaton->bible())) << cset;
		buffer->str = strm.str();
		return buffer->str.c_str();
	}
	const char* epicinium_automaton_rejoin(Automaton* automaton,
		uint8_t player_as_u8, Buffer* buffer)
	{
		Player player = (Player) player_as_u8;
		ChangeSet cset = automaton->rejoin(player);
		std::stringstream strm;
		strm << TypeEncoder(&(automaton->bible())) << cset;
		buffer->str = strm.str();
		return buffer->str.c_str();
	}

	size_t epicinium_map_pool_size()
	{
		return Map::pool().size();
	}
	const char* epicinium_map_pool_get(size_t i)
	{
		return Map::pool()[i].c_str();
	}
	size_t epicinium_map_custom_pool_size()
	{
		return Map::customPool().size();
	}
	const char* epicinium_map_custom_pool_get(size_t i)
	{
		return Map::customPool()[i].c_str();
	}
	size_t epicinium_map_user_pool_size()
	{
		return Map::userPool().size();
	}
	const char* epicinium_map_user_pool_get(size_t i)
	{
		return Map::userPool()[i].c_str();
	}

	bool epicinium_ruleset_initialize_collection()
	{
		if (_ruleset_collection)
		{
			// This method should only be called once.
			return false;
		}

		try
		{
			_ruleset_collection.reset(new Library());
			_ruleset_collection->load();
			_ruleset_collection->install();
			_ruleset_current_name = _ruleset_collection->nameCurrentBible();
			return true;
		}
		catch (const std::exception& e)
		{
			LOGE << "Exception: " << e.what();
			return false;
		}
	}
	const char* epicinium_ruleset_current_name()
	{
		return _ruleset_current_name.c_str();
	}
	bool epicinium_ruleset_exists(const char* name)
	{
		return _ruleset_collection->existsBible(name);
	}

	size_t epicinium_ai_pool_size()
	{
		return AI::pool().size();
	}
	const char* epicinium_ai_pool_get(size_t i)
	{
		return AI::pool()[i].c_str();
	}
	bool epicinium_ai_exists(const char* name)
	{
		return AI::exists(name) || AILibrary::exists(name);
	}

	AILibrary* epicinium_ai_allocate(const char* name,
		uint8_t player_as_u8, uint8_t difficulty_as_u8,
		const char* ruleset, char character)
	{
		Player player = (Player) player_as_u8;
		Difficulty difficulty = (Difficulty) difficulty_as_u8;
		try
		{
			std::string libdefname = AI::libraryDefaultFilename(name);
			if (AILibrary::exists(libdefname))
			{
				// This might return null if AI creation fails, in which case
				// the AILibrary need not and must not be deallocated.
				return AILibrary::allocate(libdefname,
					player, difficulty, ruleset, character);
			}
			else if (AI::exists(name))
			{
				// This might return null if AI creation fails, in which case
				// the AICommander need not and must not be deallocated.
				return AI::allocate(name,
					player, difficulty, ruleset, character);
			}
			else if (AILibrary::exists(name))
			{
				// This might return null if AI creation fails, in which case
				// the AILibrary need not and must not be deallocated.
				return AILibrary::allocate(name,
					player, difficulty, ruleset, character);
			}
			else
			{
				LOGE << "Cannot allocate non-existing AI '" << name << "'.";
				return nullptr;
			}
		}
		catch (const std::exception& e)
		{
			// If AI creation throws, we also return null.
			LOGE << "Exception: " << e.what();
			return nullptr;
		}
	}
	void epicinium_ai_deallocate(AILibrary* ai)
	{
		delete ai;
	}

	void epicinium_ai_receive(AILibrary* ai, const char* changes)
	{
		try
		{
			ai->receiveChangesAsString(changes);
		}
		catch (const std::exception& e)
		{
			LOGE << "Exception: " << e.what();
		}
	}
	void epicinium_ai_prepare_orders(AILibrary* ai)
	{
		ai->prepareOrders();
	}
	const char* epicinium_ai_retrieve_orders(AILibrary* ai,
		Buffer* buffer)
	{
		buffer->str = ai->ordersAsString();
		return buffer->str.c_str();
	}
	const char* epicinium_ai_descriptive_name(AILibrary* ai,
		Buffer* buffer)
	{
		buffer->str = ai->descriptivename();
		return buffer->str.c_str();
	}
	const char* epicinium_ai_descriptive_metadata(AILibrary* ai,
		Buffer* buffer)
	{
		std::stringstream strm;
		strm << "{\"difficulty\":" << Json::valueToQuotedString(
			::stringify(ai->difficulty()));
		strm << ",\"character\":" << Json::valueToQuotedString(
			ai->characterstring().c_str());
		strm << ",\"displayname\":" << Json::valueToQuotedString(
			ai->displayname().c_str());
		strm << ",\"ainame\":" << Json::valueToQuotedString(
			ai->ainame().c_str());
		strm << ",\"authors\":" << Json::valueToQuotedString(
			ai->authors().c_str());
		strm << "}";
		buffer->str = strm.str();
		return buffer->str.c_str();
	}

	uint16_t epicinium_current_challenge_id()
	{
		return (uint16_t) Challenge::current();
	}
	const char* epicinium_challenge_key(uint16_t id_as_u16)
	{
		Challenge::Id id = (Challenge::Id) id_as_u16;
		return AIChallenge::getKey(id);
	}
	size_t epicinium_challenge_num_bots(uint16_t id_as_u16)
	{
		Challenge::Id id = (Challenge::Id) id_as_u16;
		return AIChallenge::getNumBots(id);
	}
	const char* epicinium_challenge_bot_name(uint16_t id_as_u16)
	{
		Challenge::Id id = (Challenge::Id) id_as_u16;
		return AIChallenge::getBotName(id);
	}
	uint8_t epicinium_challenge_bot_difficulty(uint16_t id_as_u16)
	{
		Challenge::Id id = (Challenge::Id) id_as_u16;
		Difficulty difficulty = AIChallenge::getBotDifficulty(id);
		return (uint8_t) difficulty;
	}
	const char* epicinium_challenge_map_name(uint16_t id_as_u16)
	{
		Challenge::Id id = (Challenge::Id) id_as_u16;
		return AIChallenge::getMapName(id);
	}
	const char* epicinium_challenge_ruleset_name(uint16_t id_as_u16)
	{
		Challenge::Id id = (Challenge::Id) id_as_u16;
		return AIChallenge::getRulesetName(id);
	}
	const char* epicinium_challenge_display_name(uint16_t id_as_u16)
	{
		Challenge::Id id = (Challenge::Id) id_as_u16;

		// The server needs to send the display name in English,
		// each client will retranslate the display name on its own.
		Language::ScopedOverride override("en_US");
		return AIChallenge::getDisplayName(id);
	}
	const char* epicinium_challenge_panel_picture_name(uint16_t id_as_u16)
	{
		Challenge::Id id = (Challenge::Id) id_as_u16;
		return AIChallenge::getPanelPictureName(id);
	}
	const char* epicinium_challenge_discord_image_key(uint16_t id_as_u16)
	{
		Challenge::Id id = (Challenge::Id) id_as_u16;
		return AIChallenge::getDiscordImageKey(id);
	}
	const char* epicinium_challenge_steam_short_key(uint16_t id_as_u16)
	{
		Challenge::Id id = (Challenge::Id) id_as_u16;
		return AIChallenge::getSteamShortKey(id);
	}
	size_t epicinium_challenge_briefing_size(uint16_t /**/)
	{
		return AIChallenge::BRIEF_SIZE;
	}
	const char* epicinium_challenge_briefing_key(uint16_t /**/, size_t i)
	{
		AIChallenge::Brief brief = (AIChallenge::Brief) i;
		return AIChallenge::stringify(brief);
	}
	const char* epicinium_challenge_briefing_value(uint16_t id_as_u16, size_t i)
	{
		Challenge::Id id = (Challenge::Id) id_as_u16;
		AIChallenge::Brief brief = (AIChallenge::Brief) i;

		// The server needs to send the display name in English,
		// each client will retranslate the display name on its own.
		Language::ScopedOverride override("en_US");
		return AIChallenge::getBrief(id, brief);
	}

	Buffer* epicinium_buffer_allocate()
	{
		return new Buffer();
	}
	void epicinium_buffer_deallocate(Buffer* buffer)
	{
		delete buffer;
	}

	void epicinium_log_initialize(log_callback_fn callback, uint8_t severity)
	{
		LogInstaller(callback, severity).install();
	}
}
