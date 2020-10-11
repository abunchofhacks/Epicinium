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
#include "stomt.hpp"
#include "source.hpp"

#include "writer.hpp"
#include "curl.hpp"
#include "clienthandler.hpp"
#include "settings.hpp"


#ifdef DEVELOPMENT
static std::string STOMTAPPID = "x0Pgekxwj6fV7LMLjctPXsAyS";
static std::string STOMTURL = "https://test.rest.stomt.com/stomts";
#else
static std::string STOMTAPPID = "REeUDShML7TmgKDgFPj8KqEjw";
static std::string STOMTURL = "https://rest.stomt.com/stomts";
#endif

Stomt::Stomt(ClientHandler& owner, Settings& settings,
		std::shared_ptr<Curl> curl) :
	_owner(owner),
	_settings(settings),
	_curl(std::move(curl))
{}

Stomt::~Stomt()
{}

void Stomt::sendStomt(bool positive, const std::string& text)
{
	std::string accesstoken = _settings.stomtToken.value("");

	std::vector<std::string> headers;
	headers.emplace_back("appid: " + STOMTAPPID);
	if (!accesstoken.empty())
	{
		headers.emplace_back("accesstoken: " + accesstoken);
	}

	Json::Value payload(Json::objectValue);
	payload["positive"] = positive;
	payload["text"] = text;

	_futureResponse = _curl->post(STOMTURL, payload,
		std::move(headers));
}

Stomt::StomtData Stomt::parseResponse(Response response)
{
	if (response.errorcode)
	{
		return {ResponseStatus::CONNECTION_FAILED, "", ""};
	}

	LOGI << "Got a response from STOMT server:"
		<< " [" << response.statuscode << "] "
		<< response.body;

	Json::Reader reader;
	Json::Value json;
	if (reader.parse(response.body, json) && json.isObject())
	{
		if (json["data"].isObject()
			&& json["data"]["shortlink"].isString())
		{
			std::string accesstoken;
			LOGI << "Successfully posted STOMT";
			if (json["meta"].isObject()
				&& json["meta"]["accesstoken"].isString())
			{
				accesstoken = json["meta"]["accesstoken"].asString();
			}
			return {ResponseStatus::SUCCESS,
				json["data"]["shortlink"].asString(),
				accesstoken};
		}
		else if (json["error"].isString())
		{
			LOGW << "Posting STOMT failed, error: "
				<< json["error"].asString();
			return {ResponseStatus::UNKNOWN_ERROR, "", ""};
		}
		else
		{
			LOGW << "Posting STOMT failed, unknown response contents";
			return {ResponseStatus::RESPONSE_MALFORMED, "", ""};
		}
	}
	else
	{
		LOGE << "Posting STOMT failed, malformed response";
		return {ResponseStatus::RESPONSE_MALFORMED, "", ""};
	}
}

void Stomt::update()
{
	_curl->update();

	if (!_futureResponse.valid()) return;
	if (_futureResponse.wait_for(std::chrono::seconds(0))
		!= std::future_status::ready) return;

	StomtData data = parseResponse(_futureResponse.get());

	if (!data.accesstoken.empty() && !_settings.stomtToken.defined())
	{
		_settings.stomtToken = data.accesstoken;
		_settings.save();
	}

	switch (data.status)
	{
		case ResponseStatus::SUCCESS:
		{
			LOGI << "Successfully submitted feedback";
			_owner.sentFeedback(data.link);
		}
		break;
		case ResponseStatus::CREDS_INVALID:
		case ResponseStatus::USERNAME_TAKEN:
		case ResponseStatus::EMAIL_TAKEN:
		case ResponseStatus::CONNECTION_FAILED:
		case ResponseStatus::ACCOUNT_LOCKED:
		case ResponseStatus::ACCOUNT_DISABLED:
		case ResponseStatus::KEY_TAKEN:
		case ResponseStatus::IP_BLOCKED:
		case ResponseStatus::KEY_REQUIRED:
		case ResponseStatus::EMAIL_UNVERIFIED:
		case ResponseStatus::USERNAME_REQUIRED_NOUSER:
		case ResponseStatus::USERNAME_REQUIRED_INVALID:
		case ResponseStatus::USERNAME_REQUIRED_TAKEN:
		case ResponseStatus::METHOD_INVALID:
		case ResponseStatus::REQUEST_MALFORMED:
		case ResponseStatus::RESPONSE_MALFORMED:
		case ResponseStatus::DATABASE_ERROR:
		{
			LOGE << "Submitting feedback failed with unexpected response status "
				<< std::to_string((int) data.status);
			_owner.feedbackFailed(data.status);
		}
		break;
		case ResponseStatus::UNKNOWN_ERROR:
		{
			LOGE << "Submitting feedback failed due to unknown error";
			_owner.feedbackFailed(data.status);
		}
		break;
	}
}
