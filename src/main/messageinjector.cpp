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
#include "messageinjector.hpp"
#include "source.hpp"


#if DICTATOR_ENABLED
/* ############################ DICTATOR_ENABLED ############################ */

#include "dictator.hpp"
#include "loop.hpp"
#include "input.hpp"
#include "clock.hpp"
#include "keycode.hpp"
#include "message.hpp"
#include "parseerror.hpp"


void MessageInjector::update()
{
	static bool show = false;
	bool wasshown = show;

	if (Input::get()->isDebugKeyHeld())
	{
		if (ImGui::Begin("Windows", nullptr,
			ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse))
		{
			ImGui::Checkbox("Message Injection", &show);
		}
		ImGui::End();
	}
	else if (Input::get()->isKeyHeld(SDL_SCANCODE_INSERT))
	{
		show = true;
	}

	if (show)
	{
		if (!wasshown) ImGui::SetNextWindowCollapsed(false);

		if (ImGui::Begin("Message Injection", nullptr,
				ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::PushItemWidth(400.0f);
			static const std::string TEMPLATES[] = {
				Message::quit().str(),
				Message::ping().str(),
				Message::patch().str(),
				Message::chat("BLANK", Target::GENERAL).str(),
				Message::ruleset_request("BLANK").str(),
				Message::pick_map("BLANK").str(),
				Message::pick_timer(60).str(),
				Message::pick_replay("BLANK").str(),
				Message::pick_challenge("BLANK").str(),
				Message::pick_ruleset("BLANK").str(),

			};
			static std::array<char, 512> input = {{ 0 }};
			static bool up;
			up = false;
			ImGui::ListBoxHeader("Templates", array_size(TEMPLATES), 4);
			for (size_t i = 0; i < array_size(TEMPLATES); i++)
			{
				const std::string& str = TEMPLATES[i];
				if (ImGui::Selectable(str.c_str()))
				{
					assert(str.size() < input.size());
					strncpy(input.data(), str.c_str(), input.size() - 1);
					up = true;
				}
			}
			ImGui::ListBoxFooter();
			ImGui::Separator();
			up |= ImGui::InputTextMultiline("Json", input.data(), input.size());
			static bool verifyJson = true;
			static bool verifyMessage = true;
			static bool enforceVerify = true;
			ImGui::Columns(3, NULL, false);
			up |= ImGui::Checkbox("Verify JSON", &verifyJson);
			if (!verifyJson) verifyMessage = false;
			if (!verifyJson) enforceVerify = false;
			ImGui::NextColumn();
			up |= ImGui::Checkbox("Verify Message", &verifyMessage);
			if (verifyMessage) verifyJson = true;
			ImGui::NextColumn();
			up |= ImGui::Checkbox("Enforced", &enforceVerify);
			if (enforceVerify) verifyJson = true;
			ImGui::NextColumn();
			ImGui::Columns(1);
			static bool valid = false;
			static std::string errormessage;
			static std::unique_ptr<ParsedMessage> parsed;
			if (verifyJson)
			{
				if (up)
				{
					std::string str = input.data();
					Json::Reader reader;
					Json::Value json;
					valid = reader.parse(str, json);
					if (!valid)
					{
						errormessage = reader.getFormattedErrorMessages();
					}

					if (valid && verifyMessage)
					{
						try
						{
							parsed.reset(new ParsedMessage(std::move(json)));
						}
						catch (const ParseError& error)
						{
							valid = false;
							errormessage = error.what();
						}
						catch (const Json::Exception& error)
						{
							valid = false;
							errormessage = error.what();
						}
					}
				}

				if (valid)
				{
					static ImVec4 validcolor(0.1f, 0.8f, 0.4f, 1.0f);
					ImGui::TextColored(validcolor, "Valid");
					ImGui::TextDisabled("No errors.");
				}
				else
				{
					static ImVec4 invalidcolor(1.0f, 0.4f, 0.2f, 1.0f);
					ImGui::TextColored(invalidcolor, "Error");
					ImGui::Text("%s", errormessage.c_str());
				}
			}
			else
			{
				ImGui::TextDisabled("Unverified");
			}
			static std::string lastkeycode;
			static float recency = 0.0f;
			if (ImGui::Button("Send")
				&& (valid || !enforceVerify))
			{
				_dictator.sendbuffer.emplace_back(input.data());
				{
					auto timestampMs = SteadyClock::milliseconds();
					uint16_t key = rand() % (1 << 16);
					lastkeycode = keycode(key, timestampMs);
				}
				recency = 1.0f;
			}
			else
			{
				recency = std::max(0.0f, recency - Loop::delta());
			}
			ImGui::SameLine();
			ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, recency),
				"Message sent (%s)", lastkeycode.c_str());
			ImGui::PopItemWidth();
		}
		ImGui::End();
	}
}

/* ############################ DICTATOR_ENABLED ############################ */
#endif
