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
#include "loginstaller.hpp"
#include "source.hpp"

#include "libs/plog/Init.h"

#if LOG_REPLACE_WITH_CALLBACK_ENABLED
#include "libs/plog/Appenders/IAppender.h"
#include "libs/plog/Converters/UTF8Converter.h"
#include "libs/plog/Record.h"
#include "libs/plog/Util.h"
#endif

#include "settings.hpp"
#include "version.hpp"
#include "system.hpp"


#if LOG_REPLACE_WITH_CALLBACK_ENABLED
class CallbackAppender : public plog::IAppender
{
public:
	virtual void write(const plog::Record& record)
	{
		writeToCallback(record);
	}

private:
	static void writeToCallback(const plog::Record& record);

public:
	static void installCallback(log_callback_fn callback);
};
#endif

#if LOG_REPLACE_WITH_CALLBACK_ENABLED
static CallbackAppender _installed_callback_appender;
#endif

std::string LogInstaller::_logsfolder = "logs/";

void LogInstaller::setRoot(const std::string& root)
{
	if (root.empty())
	{
		_logsfolder = "logs/";
	}
	else if (root.back() == '/')
	{
		_logsfolder = root + "logs/";
	}
	else
	{
		_logsfolder = root + "/logs/";
	}
}

LogInstaller::LogInstaller(const Settings& settings) :
	_name(settings.logname.value()),
	_level(settings.loglevel.value("")),
	_perf(settings.perflog.value("")),
	_rollback(std::max(1, settings.logrollback.value(20)))
{}

LogInstaller::LogInstaller(const std::string& name, int rollback,
		const std::string& level, const std::string& perf) :
	_name(name),
	_level(level),
	_perf(perf),
	_rollback(std::max(1, rollback))
{}

LogInstaller::LogInstaller(log_callback_fn callback, uint8_t severity) :
	_name(""),
	_level(plog::severityToString((plog::Severity) severity)),
	_perf(""),
	_rollback(1)
{
	assert(callback != nullptr);
#if LOG_REPLACE_WITH_CALLBACK_ENABLED
	CallbackAppender::installCallback(callback);
#else
	assert(false && LOG_REPLACE_WITH_CALLBACK_ENABLED);
#endif
}

void LogInstaller::install()
{
	// Parse the verbosity.
	plog::Severity verbosity = plog::verbose;
	if (!_level.empty())
	{
		char x = std::toupper(_level[0]);
		bool found = false;
		for (plog::Severity s = plog::none;
			s <= plog::verbose;
			s = static_cast<plog::Severity>(s + 1))
		{
			if (x == plog::severityToString(s)[0])
			{
				verbosity = s;
				found = true;
				break;
			}
		}
		if (!found)
		{
			std::cerr << "Unknown loglevel '" << _level << "'" << std::endl;
		}
	}

#if LOG_REPLACE_WITH_CALLBACK_ENABLED
	plog::init<0>(std::max(verbosity, plog::verbose),
		&_installed_callback_appender);
#else
	System::touchDirectory(_logsfolder);

	// Verbose log that includes everything.
	std::string verbosefilename = _logsfolder + _name + ".verbose.log";
	// Default info log.
	std::string infofilename = _logsfolder + _name + ".info.log";
	// Error log. Also includes warnings.
	std::string errorfilename = _logsfolder + _name + ".errors.log";

	// Each file is up to 1MB.
	constexpr int SIZE = 1024000;

	// If we want a separate verbose log, keep three separate logs.
	if (_verboserollback > 0)
	{
		plog::init<0>(std::max(verbosity, plog::verbose),
			verbosefilename.c_str(), SIZE, _verboserollback);
		plog::init<1>(std::min(verbosity, plog::debug),
			infofilename.c_str(), SIZE, _rollback);
		plog::init<2>(plog::warning, errorfilename.c_str(), SIZE, _rollback);
		plog::get<0>()->addAppender(plog::get<1>());
		plog::get<0>()->addAppender(plog::get<2>());
	}
	// If we are relatively verbose, keep a separate error log.
	else if (verbosity > plog::warning)
	{
		plog::init<0>(verbosity, infofilename.c_str(), SIZE, _rollback);
		plog::init<1>(plog::warning, errorfilename.c_str(), SIZE, _rollback);
		plog::get<0>()->addAppender(plog::get<1>());
	}
	// Otherwise, only keep an error log.
	else if (verbosity != plog::none)
	{
		plog::init<0>(verbosity, errorfilename.c_str(), SIZE, _rollback);
	}

	// If we have a PERFLOG_INSTANCE, install it.
#ifdef PERFLOG_INSTANCE
	if (!_perf.empty())
	{
		plog::Severity perfosity = std::max(plog::info, verbosity);
		std::string fname = perflogfilename();
		plog::init<PERFLOG_INSTANCE>(perfosity, fname.c_str(), SIZE, 1);
	}
#endif
#endif
}

std::string LogInstaller::perflogfilename() const
{
	std::string fname;
	if (_perf != ".")
	{
		fname = _logsfolder + _name + ".perf." + _perf + ".log";
	}
	else if (Version::current().isWellDefined())
	{
		std::stringstream strm;
		strm << Version::current();
		fname = _logsfolder + _name + ".perf." + strm.str() + ".log";
	}
	else fname = _logsfolder + _name + ".perf.log";
	return fname;
}




#if LOG_REPLACE_WITH_CALLBACK_ENABLED
static log_callback_fn _installed_appender_callback = nullptr;

class FuncFormatter
{
public:
	static plog::util::nstring format(const plog::Record& record);
};

void CallbackAppender::writeToCallback(const plog::Record& record)
{
	plog::Severity severity = record.getSeverity();
	uint8_t severity_as_u8 = (uint8_t) severity;
	std::string str = plog::UTF8Converter::convert(
		FuncFormatter::format(record));
	_installed_appender_callback(severity_as_u8, str.c_str());
}

void CallbackAppender::installCallback(log_callback_fn callback)
{
	_installed_appender_callback = callback;
}

plog::util::nstring FuncFormatter::format(const plog::Record& record)
{
	plog::util::nostringstream ss;
	ss << PLOG_NSTR("[") << record.getFunc() << PLOG_NSTR("@")
		<< record.getLine() << PLOG_NSTR("] ");
	ss << record.getMessage();

	return ss.str();
}
#endif
