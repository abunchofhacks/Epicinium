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
#include "slackapi.hpp"
#include "source.hpp"

#include "curl.hpp"


SlackAPI::SlackAPI(std::shared_ptr<Curl> curl,
		const Setting<std::string>& url,
		const std::string& channelname,
		const Setting<std::string>& botname,
		const std::string& suffix) :
	_curl(std::move(curl)),
	_url(url.value("")),
	_channelname(channelname),
	_botname(botname.value("unknown") + "-" + suffix),
	_enabled(url.defined())
{
	if (_enabled && !botname.defined())
	{
		LOGW << "Slack enabled but no name provided;"
			<< " using '" << _botname << "' instead.";
	}
	DEBUG_ASSERT(!_enabled || botname.defined());

	if (!_enabled)
	{
		_curl = nullptr;
	}
}

SlackAPI::~SlackAPI()
{
	_enabled = false;
}

void SlackAPI::post(const std::string& text, const std::string& name,
	const char* emoji)
{
	if (!_enabled) return;

	Json::Value json(Json::objectValue);
	json["channel"] = _channelname;
	json["username"] = name;
	json["icon_emoji"] = emoji;
	json["text"] = text;

	_curl->post(_url, json);
}

void SlackAPI::post(const std::string& text)
{
	return post(text, _botname, ":signal_strength:");
}

void SlackAPI::error(const std::string& text, const std::string& name)
{
	return post(text, name, ":skull:");
}
