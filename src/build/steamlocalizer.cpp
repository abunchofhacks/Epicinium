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

#include "settings.hpp"
#include "language.hpp"
#include "aichallenge.hpp"
#include "loginstaller.hpp"
#include "version.hpp"


struct SteamLanguage
{
	std::string localization_tag;
	std::string steam_name;
};

int main(int argc, char* argv[])
{
	Settings settings("settings-steamlocalizer.json", argc, argv);

	if (!settings.logname.defined())
	{
		settings.logname.override("steamlocalizer");
	}

	if (!settings.logrollback.defined()) settings.logrollback.override(5);
	LogInstaller(settings).install();

	LOGI << "Start v" << Version::current();

	Language::use(settings);

	std::vector<SteamLanguage> languages = {
		{"en_US", "english"},
	};

	std::cout << "\"lang\"" << std::endl;
	std::cout << "{" << std::endl;
	for (const SteamLanguage& language : languages)
	{
		Language::ScopedOverride override(language.localization_tag);

		std::cout << "\t\"" << language.steam_name << "\"" << std::endl;
		std::cout << "\t{" << std::endl;
		std::cout << "\t\t\"tokens\"" << std::endl;
		std::cout << "\t\t{" << std::endl;

		std::cout << "\t\t\t\"#in_menu\" \""
			<< _("In Menu")
			<< "\"" << std::endl;
		std::cout << "\t\t\t\"#looking_to_play\" \""
			<< _("Looking to Play")
			<< "\"" << std::endl;
		std::cout << "\t\t\t\"#playing_tutorial\" \""
			<< _("Playing Tutorial")
			<< "\"" << std::endl;
		std::cout << "\t\t\t\"#playing_challenge\" \""
			<< _("Playing Challenge")
			<< "\"" << std::endl;
		std::cout << "\t\t\t\"#playing_challenge_keyed\" \""
			<< ::format(
				// TRANSLATORS: The argument is a translated challenge name.
				_("Playing %s Challenge"),
				"{#challenge_name_" "%" "challenge_key" "%" "}")
			<< "\"" << std::endl;
		std::cout << "\t\t\t\"#watching_replay\" \""
			<< _("Watching Replay")
			<< "\"" << std::endl;

		std::cout << std::endl;

		std::cout << "\t\t\t\"#playing\" \""
			<< "{#playing_" "%" "lobby_type_key" "%" "}"
			<< "\"" << std::endl;
		std::cout << "\t\t\t\"#spectating\" \""
			<< "{#spectating_" "%" "lobby_type_key" "%" "}"
			<< "\"" << std::endl;
		std::cout << "\t\t\t\"#in_lobby\" \""
			<< "{#in_lobby_" "%" "lobby_type_key" "%" "}"
			<< "\"" << std::endl;

		std::cout << std::endl;

		std::cout << "\t\t\t\"#playing_one_vs_one\" \""
			<< _("Playing One vs One")
			<< "\"" << std::endl;
		std::cout << "\t\t\t\"#playing_free_for_all\" \""
			<< _("Playing Free For All")
			<< "\"" << std::endl;
		std::cout << "\t\t\t\"#playing_versus_ai\" \""
			<< _("Playing Versus AI")
			<< "\"" << std::endl;
		std::cout << "\t\t\t\"#playing_ai_vs_ai\" \""
			<< _("Playing AI vs AI")
			<< "\"" << std::endl;
		std::cout << "\t\t\t\"#spectating_one_vs_one\" \""
			<< _("Spectating One vs One")
			<< "\"" << std::endl;
		std::cout << "\t\t\t\"#spectating_free_for_all\" \""
			<< _("Spectating Free For All")
			<< "\"" << std::endl;
		std::cout << "\t\t\t\"#spectating_versus_ai\" \""
			<< _("Spectating Versus AI")
			<< "\"" << std::endl;
		std::cout << "\t\t\t\"#spectating_ai_vs_ai\" \""
			<< _("Spectating AI vs AI")
			<< "\"" << std::endl;
		std::cout << "\t\t\t\"#in_lobby_one_vs_one\" \""
			<< ::format(
				// TRANSLATORS: The two arguments are the current and maximum
				// number of players in the lobby, respectively.
				_("In One vs One Lobby (%s/%s)"),
				"%" "lobby_cur_size" "%",
				"%" "lobby_max_size" "%")
			<< "\"" << std::endl;
		std::cout << "\t\t\t\"#in_lobby_free_for_all\" \""
			<< ::format(
				// TRANSLATORS: The two arguments are the current and maximum
				// number of players in the lobby, respectively.
				_("In Free For All Lobby (%s/%s)"),
				"%" "lobby_cur_size" "%",
				"%" "lobby_max_size" "%")
			<< "\"" << std::endl;
		std::cout << "\t\t\t\"#in_lobby_versus_ai\" \""
			<< ::format(
				// TRANSLATORS: The two arguments are the current and maximum
				// number of players in the lobby, respectively.
				_("In Versus AI Lobby (%s/%s)"),
				"%" "lobby_cur_size" "%",
				"%" "lobby_max_size" "%")
			<< "\"" << std::endl;
		std::cout << "\t\t\t\"#in_lobby_ai_vs_ai\" \""
			<< ::format(
				// TRANSLATORS: The two arguments are the current and maximum
				// number of players in the lobby, respectively.
				_("In AI vs AI Lobby (%s/%s)"),
				"%" "lobby_cur_size" "%",
				"%" "lobby_max_size" "%")
			<< "\"" << std::endl;

		std::cout << std::endl;

		for (size_t i = 0; i < Challenge::ID_SIZE; i++)
		{
			AIChallenge challenge = (Challenge::Id) i;
			std::string key = challenge.getSteamShortKey();
			std::cout << "\t\t\t\"#challenge_name_" << key << "\" \""
				<< challenge.getDisplayName()
				<< "\"" << std::endl;
		}

		std::cout << "\t\t}" << std::endl;
		std::cout << "\t}" << std::endl;
	}
	std::cout << "}" << std::endl;
	return 0;
}
