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
#include "discordapi.hpp"
#include "source.hpp"

#include "writer.hpp"
#include "curl.hpp"


DiscordAPI* DiscordAPI::_installed = nullptr;

DiscordAPI::DiscordAPI(std::shared_ptr<Curl> curl,
		const Setting<std::string>& url) :
	_curl(std::move(curl)),
	_url(url.value("")),
	_enabled(url.defined())
{
	if (_enabled)
	{
		_sendthread = std::thread(&DiscordAPI::runSendThread, this);
	}
	else
	{
		_curl = nullptr;
	}

	_installed = this;
}

DiscordAPI::~DiscordAPI()
{
	_enabled = false;

	// Now that we have disabled ourselves, notify the sendthread that
	// we are shutting down. (It might already be awake, but soit.)
	_buffernotifier.notify_one();

	// We need to wait for the thread to finish.
	if (_sendthread.joinable()) _sendthread.join();
}

void DiscordAPI::runSendThread()
{
	Writer writer;
	writer.install();

	int criticalErrors = 0;

	while (true)
	{
	try
	{
		Post post;

		{
			// We need a lock around the buffer to prevent it from being
			// written to as we are reading from it.
			std::unique_lock<std::mutex> lock(_buffermutex);

			// Is there anything to send?
			if (_buffer.empty())
			{
				// If we have been disabled, we know no more message are coming.
				if (!_enabled) return;

				// Wait for a thread to notify us, either when there is
				// something to send or when we are shutting down.
				// Note that this unlocks the _buffermutex lock exactly
				// while we are waiting.
				_buffernotifier.wait(lock, [this](){

					return (!_enabled || !_buffer.empty());
				});
			}

			// We might have been woken up because we have been disabled.
			if (_buffer.empty()) return;

			// Either there was something to send, or we waited until there was.
			post = _buffer.front();
		}

		// Send the post.
		bool success = send(post.text, post.name);

		// Pop from buffer only if post was successful
		if (success)
		{
			// We need a lock around the buffer to prevent it from being
			// written to as we are reading from it.
			std::unique_lock<std::mutex> lock(_buffermutex);

			_buffer.pop();
		}
		else
		{
			LOGW << "Post unsuccesful, try again";
			if (_retryAfter > 0)
				std::this_thread::sleep_for(
				std::chrono::milliseconds(_retryAfter));
		}
	}
	catch (const std::exception& e)
	{
		LOGE << "Error while running discord thread: " << e.what();
		LOGE << "Error count: " << ++criticalErrors;
		RETHROW_IF_DEV();
		if (criticalErrors >= 5)
		{
			LOGF << "This server is gravely ill and needs to be cured!";
			LOGE << "Disabling discord";
			_enabled = false;
		}
	}
	catch (...)
	{
		LOGE << "Unknown error while running discord thread";
		LOGE << "Error count: " << ++criticalErrors;
		RETHROW_IF_DEV();
		if (criticalErrors >= 5)
		{
			LOGF << "This server is gravely ill and needs to be cured!";
			LOGE << "Disabling discord";
			_enabled = false;
		}
	}
	}
}

bool DiscordAPI::send(const std::string& text, const std::string& name)
{
	_retryAfter = 0;
	Json::Value payload(Json::objectValue);
	payload["content"] = text;
	if (!name.empty()) payload["username"] = name;

	std::future<Response> future = _curl->post(_url + "?wait=true", payload);
	future.wait();
	Response response = future.get();

	if (response.errorcode)
	{
		LOGE << "Curl error while sending " << text;
		return false;
	}

	if (response.statuscode == 429)
	{
		Json::Reader reader;
		Json::Value json;
		if (reader.parse(response.body, json) && json.isObject()
			&& json["retry_after"].isInt())
		{
			_retryAfter = json["retry_after"].asInt();
			LOGW << "We are being rate limited, retry after " << _retryAfter;
			return false;
		}
		// else (statuscode != 200)
	}

	if (response.statuscode != 200)
	{
		LOGE << "Response code " << response.statuscode
			<< " while sending " << text;
		LOGE << "Response: " << response.body;
		return false;
	}

	return true;
}

void DiscordAPI::post(const std::string& text, const std::string& name)
{
	if (!_enabled) return;

	{
		// We need a lock around the buffer to prevent it from being
		// read from as we are writing from it.
		std::lock_guard<std::mutex> lock(_buffermutex);

		// Buffer the post.
		_buffer.emplace(text, name);
	}

	// Now that we have unlocked the buffer, notify the send thread that
	// there is work to do. (It might already be awake, but soit.)
	_buffernotifier.notify_one();
}

void DiscordAPI::post(const std::string& text)
{
	return post(text, "");
}
