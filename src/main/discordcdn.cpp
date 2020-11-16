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
#include "discordcdn.hpp"
#include "source.hpp"

#include "system.hpp"
#include "locator.hpp"
#include "curl.hpp"


static std::string pictureName(const std::string& data)
{
	return "discord/avatars/" + data;
}

static std::string url(const std::string& data)
{
	return "https://cdn.discordapp.com/avatars/" + data + ".png?size=64";
}

DiscordCDN::DiscordCDN(std::shared_ptr<Curl> curl) :
	_alive(true),
	_threadrunning(false),
	_curl(std::move(curl))
{
	// Start the thread.
	_threadrunning = true;
	_thread = std::thread(&DiscordCDN::runThread, this);
}

DiscordCDN::~DiscordCDN()
{
	// We are shutting down.
	_alive = false;

	// We need to wait for the thread to finish or risk memory crashes.
	if (_threadrunning)
	{
		LOGI << "Waiting until thread finishes...";

		// Notify the thread that we are shutting down.
		// (It might already be awake, but soit.)
		_requestnotifier.notify_one();
	}
	if (_thread.joinable()) _thread.join();

	LOGI << "Thread finished.";
}

std::string DiscordCDN::getAvatarPicture(const char* userid, const char* avatar)
{
	// If no avatar is set, use the Discord default avatar.
	// Only the fourth check should fail, but we check them all anyway.
	if (!userid || !userid[0]) return "discord/default";
	if (!avatar || !avatar[0]) return "discord/default";

	std::string data = std::string(userid) + "/" + std::string(avatar);
	std::string picturename = pictureName(data);

	// If the have not yet downloaded this avatar, do so now.
	if (!System::isFile(Locator::pictureFilename(picturename)))
	{
		// We need a lock around buffering the request to prevent them from
		// being read as we are writing them. We also need to make sure
		// the chunk header and the chunk are sent contiguously.
		{
			std::lock_guard<std::mutex> lock(_requestmutex);

			_requestbuffer.emplace(data);
		}

		// Now that we have unlocked the buffer, notify the thread that
		// there is work to do. (It might already be awake, but soit.)
		_requestnotifier.notify_one();
	}

	return picturename;
}

bool DiscordCDN::update(std::vector<std::string>& buffer)
{
	std::lock_guard<std::mutex> lock(_fulfilledmutex);

	if (_fulfilled.empty()) return false;

	buffer = std::move(_fulfilled);

	return true;
}

void DiscordCDN::runThread()
{
	try
	{
	while (_alive)
	{
		std::string data;

		{
			// We need a lock around the buffer to prevent it from being
			// written to as we are reading from it.
			std::unique_lock<std::mutex> lock(_requestmutex);

			// Is there anything to request?
			if (_requestbuffer.empty())
			{
				// Wait for the client to notify us, either when there is
				// something to send or when we are shutting down.
				// Note that this unlocks the _requestmutex lock exactly
				// while we are waiting.
				// It also protects us from spuriously waking up.
				_requestnotifier.wait(lock, [this](){

					return (!_alive || !_requestbuffer.empty());
				});
			}

			// We might have been woken up because we are shutting down.
			if (!_alive)
			{
				// Finish this thread.
				_threadrunning = false;
				return;
			}

			// Either there was something to send, or we waited until there was.
			data = std::move(_requestbuffer.front());
			_requestbuffer.pop();
		}

		std::string filename = Locator::pictureFilename(pictureName(data));

		LOGI << "Downloading '" << data << "' from Discord";

		{
			std::future<Response> future = _curl->get(url(data));
			bool ready = false;
			while (!ready)
			{
				if (!_alive)
				{
					// Finish this thread.
					_threadrunning = false;
					return;
				}

				_curl->update();
				ready = (future.wait_for(std::chrono::seconds(0))
					== std::future_status::ready);
			}
			Response response = future.get();

			if (response.errorcode)
			{
				LOGW << "Error while downloading from Discord: "
					<< "Curl error " << response.errorcode;

				// Finish this thread.
				_threadrunning = false;
				return;
			}

			if (response.statuscode != 200)
			{
				LOGW << "Got an unexpected response from Discord:"
					<< " [" << response.statuscode << "]"
					<< response.body;

				// Finish this thread.
				_threadrunning = false;
				return;
			}

			System::touchFile(filename);

			std::ofstream file = System::ofstream(filename,
				std::ofstream::binary | std::ofstream::trunc);
			if (!file.is_open())
			{
				LOGW << "Error while writing to file '" << filename << "'";

				// Finish this thread.
				_threadrunning = false;
				return;
			}

			file << response.body;

			LOGV << "Downloaded " << filename;
		}

		{
			// We need a lock around the buffer to prevent it from being
			// read from as we are writing from it.
			std::unique_lock<std::mutex> lock(_fulfilledmutex);

			// We have downloaded another file.
			_fulfilled.emplace_back(filename);
		}

		LOGI << "Downloaded '" << data << "' from Discord";
	}
	}
	catch (const std::exception& e)
	{
		LOGE << "Error while running DiscordCDN thread: " << e.what();
		RETHROW_IF_DEV();

		// Finish this thread.
		_threadrunning = false;
		return;
	}
	catch (...)
	{
		LOGE << "Unknown error while running DiscordCDN thread";
		RETHROW_IF_DEV();

		// Finish this thread.
		_threadrunning = false;
		return;
	}

	// Finish this thread.
	_threadrunning = false;
	return;
}
