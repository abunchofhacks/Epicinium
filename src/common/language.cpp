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
#include "language.hpp"
#include "source.hpp"

#if INTL_ENABLED
#include "libs/tinygettext/tinygettext.hpp"
#include "libs/tinygettext/log.hpp"
#endif

#include "settings.hpp"


std::string Language::_locdir = "data/loc";

void Language::setRoot(const std::string& root)
{
	if (root.empty())
	{
		_locdir = "data/loc";
	}
	else if (root.back() == '/')
	{
		_locdir = root + "data/loc";
	}
	else
	{
		_locdir = root + "/data/loc";
	}
}


#if INTL_ENABLED
/* ######################################################################### */

static std::string trim_from_tinygettext(const std::string& message)
{
	size_t pos = message.find_last_not_of(" \t\r\n");
	if (pos == std::string::npos)
	{
		return message;
	}
	else
	{
		return message.substr(0, pos + 1);
	}
}

static void logi_for_tinygettext(const std::string& message)
{
	LOGI << trim_from_tinygettext(message);
}

static void logw_for_tinygettext(const std::string& message)
{
	LOGW << trim_from_tinygettext(message);
}

static void loge_for_tinygettext(const std::string& message)
{
	LOGE << trim_from_tinygettext(message);
}

static tinygettext::DictionaryManager _language_dictionary_manager;

inline void setLanguage(const std::string& newvalue)
{
	auto language = tinygettext::Language::from_env(newvalue);
	if (language)
	{
		LOGI << "Setting language to '" << newvalue << "'"
			" (" << language.str() << ")";
		_language_dictionary_manager.set_language(language);
	}
	else
	{
		LOGE << "Cannot set language to invalid '" << newvalue << "'";
	}
}

Language::ScopedOverride::ScopedOverride(const std::string& override)
{
	_oldvalue = _language_dictionary_manager.get_language().str();

	setLanguage(override);
}

Language::ScopedOverride::~ScopedOverride()
{
	if (!_oldvalue.empty())
	{
		setLanguage(_oldvalue);
	}
}

void Language::use(const Settings& settings)
{
	tinygettext::Log::set_log_info_callback(logi_for_tinygettext);
	tinygettext::Log::set_log_warning_callback(logw_for_tinygettext);
	tinygettext::Log::set_log_error_callback(loge_for_tinygettext);

	LOGI << "Enabling language support...";
	_language_dictionary_manager.add_directory(_locdir);
	LOGI << "Found " << _language_dictionary_manager.get_languages().size()
		<< " languages";

	setLanguage(settings.language.value("en_US"));
}

bool Language::isCurrentlyEnglish()
{
	auto lang = _language_dictionary_manager.get_language().get_language();
	return (lang.empty() || lang == "en");
}

std::vector<std::string> Language::supportedTags()
{
	return {
		"en_US",
	};
}

std::vector<std::string> Language::experimentalTags()
{
	return {
		"es_ES",
		"fr_FR",
		"pl_PL",
		"tr_TR",
	};
}

std::vector<std::string> Language::incompleteTags()
{
	return {
		"cs_CZ",
		"de_DE",
		"fi_FI",
		"it_IT",
		"nl_NL",
		"pt_BR",
		"ru_RU",
		"uk_UA",
	};
}

std::vector<std::string> Language::allDetectedTags()
{
	std::vector<std::string> tags;
	for (const auto& language : _language_dictionary_manager.get_languages())
	{
		tags.push_back(language.str());
	}
	return tags;
}

std::string Language::getNameInOwnLanguage(const std::string& tag)
{
	Language::ScopedOverride override(tag);
	return getNameInActiveLanguage(tag);
}

std::string Language::getNameInActiveLanguage(const std::string& tag)
{
	if (tag == "en_US")
	{
		return _("English");
	}
	else if (tag == "cs_CZ")
	{
		return _("Czech");
	}
	else if (tag == "de_DE")
	{
		return _("German");
	}
	else if (tag == "es_ES")
	{
		return _("Spanish");
	}
	else if (tag == "fi_FI")
	{
		return _("Finnish");
	}
	else if (tag == "fr_FR")
	{
		return _("French");
	}
	else if (tag == "it_IT")
	{
		return _("Italian");
	}
	else if (tag == "nl_NL")
	{
		return _("Dutch");
	}
	else if (tag == "pl_PL")
	{
		return _("Polish");
	}
	else if (tag == "pt_BR")
	{
		return _("Portuguese (BR)");
	}
	else if (tag == "ru_RU")
	{
		return _("Russian");
	}
	else if (tag == "tr_TR")
	{
		return _("Turkish");
	}
	else if (tag == "uk_UA")
	{
		return _("Ukrainian");
	}

	auto language = tinygettext::Language::from_env(tag);
	if (language)
	{
		return language.get_name();
	}

	LOGW << "Missing language name for '" << tag << "'";
	return tag;
}

std::string Language::gettext(const char* message)
{
	return _language_dictionary_manager.get_dictionary().translate(message);
}

/* ######################################################################### */
#else
/* ######################################################################### */

Language::ScopedOverride::ScopedOverride(const std::string&)
{
	LOGW << "Failed to switch languages because INTL_ENABLED is not true";
}

Language::ScopedOverride::~ScopedOverride()
{}

void Language::use(const Settings&)
{
	LOGW << "Cannot support other languages because INTL_ENABLED is not true";

	// Nothing to do.
	(void) _locdir;
}

bool Language::isCurrentlyEnglish()
{
	return true;
}

std::vector<std::string> Language::supportedTags()
{
	return {"en_US"};
}

std::vector<std::string> Language::incompleteTags()
{
	return {};
}

std::vector<std::string> Language::experimentalTags()
{
	return {};
}

std::vector<std::string> Language::allDetectedTags()
{
	return {};
}

std::string Language::getNameInOwnLanguage(const std::string&)
{
	return "English";
}

std::string Language::getNameInActiveLanguage(const std::string&)
{
	return "English";
}

std::string Language::gettext(const char* message)
{
	return message;
}

/* ######################################################################### */
#endif
