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
#include <locale.h>
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

inline const char* getEnvLanguage()
{
	return getenv("LANGUAGE");
}

inline void setEnvLanguage(const std::string& newvalue)
{
#ifdef PLATFORMUNIX
	if (setenv("LANGUAGE", newvalue.c_str(), true) != 0)
	{
		LOGE << "Failed to set LANGUAGE to '" << newvalue << "':"
			" " << errno << ": " << strerror(errno);
	}
#else
	std::string newenv = "LANGUAGE=" + newvalue;
	if (_putenv(newenv.c_str()) != 0)
	{
		LOGE << "Failed to put '" << newenv << "'";
	}
#endif
}

Language::ScopedOverride::ScopedOverride(const std::string& override)
{
	if (getEnvLanguage() == nullptr) LOGV << "Current LANGUAGE is null.";
	else LOGV << "Current LANGUAGE: " << getEnvLanguage();

	std::stringstream strm;
	strm << override;
	if (getEnvLanguage() != nullptr)
	{
		strm << ":" << getEnvLanguage();
		_oldvalue = getEnvLanguage();
		_oldenvset = true;
	}
	std::string newenv = strm.str();
	setEnvLanguage(newenv);

	if (getEnvLanguage() == nullptr) LOGV << "Current LANGUAGE is null.";
	else LOGV << "Current LANGUAGE: " << getEnvLanguage();

	bind();
}

Language::ScopedOverride::~ScopedOverride()
{
	if (_oldenvset)
	{
		setEnvLanguage(_oldvalue);
	}

	if (getEnvLanguage() == nullptr) LOGV << "Current LANGUAGE is null.";
	else LOGV << "Current LANGUAGE: " << getEnvLanguage();

	bind();
}

void Language::use(const Settings& settings)
{
	// Change LC_MESSAGES from "C" to the system default, e.g. "en_US",
	// because otherwise localization is disabled and LANGUAGE is ignored.
	if (setlocale(LC_ALL, NULL) == nullptr) LOGV << "Current locale is null.";
	else LOGV << "Current locale: " << setlocale(LC_ALL, NULL);

	if (setlocale(LC_MESSAGES, "") == nullptr)
	{
		LOGE << "Failed to set locale to '':"
			" " << errno << ": " << strerror(errno);
	}

	if (setlocale(LC_ALL, NULL) == nullptr) LOGV << "Current locale is null.";
	else LOGI << "Current locale: " << setlocale(LC_ALL, NULL);

	{
		if (getEnvLanguage() == nullptr) LOGV << "Current LANGUAGE is null.";
		else LOGV << "Current LANGUAGE: " << getEnvLanguage();

		std::stringstream strm;
		strm << settings.language.value("en_US");
		if (getEnvLanguage() != nullptr)
		{
			strm << ":" << getEnvLanguage();
		}
		std::string language = strm.str();
		setEnvLanguage(language);
	}

	if (getEnvLanguage() == nullptr) LOGV << "Current LANGUAGE is null.";
	else LOGI << "Current LANGUAGE: " << getEnvLanguage();

	bind();
}

void Language::bind()
{
	constexpr const char* TEXTDOMAIN = "epicinium";
	constexpr const char* CODESET = "UTF-8";
	if (bindtextdomain(TEXTDOMAIN, _locdir.c_str()) == nullptr)
	{
		LOGE << "Failed to bind textdomain:"
			" " << errno << ": " << strerror(errno);
	}
	if (bind_textdomain_codeset(TEXTDOMAIN, CODESET) == nullptr)
	{
		LOGE << "Failed to bind textdomain:"
			" " << errno << ": " << strerror(errno);
	}
	if (textdomain(TEXTDOMAIN) == nullptr)
	{
		LOGE << "Failed to bind textdomain:"
			" " << errno << ": " << strerror(errno);
	}
}

bool Language::isCurrentlyEnglish()
{
	if (getEnvLanguage() != nullptr)
	{
		return (strncmp(getEnvLanguage(), "en_", 3) == 0);
	}
	return false;
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
		"pl_PL",
	};
}

std::vector<std::string> Language::incompleteTags()
{
	return {
		"cs_CZ",
		"it_IT",
		"nl_NL",
		"pt_BR",
		"ru_RU",
		"uk_UA",
	};
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
	else if (tag == "uk_UA")
	{
		return _("Ukrainian");
	}
	else
	{
		LOGW << "Missing language name for '" << tag << "'";
		return tag;
	}
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
}

void Language::bind()
{
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

std::string Language::getNameInOwnLanguage(const std::string& tag)
{
	Language::ScopedOverride override(tag);
	return getNameInActiveLanguage(tag);
}

std::string Language::getNameInActiveLanguage(const std::string&)
{
	return "English";
}
/* ######################################################################### */
#endif
