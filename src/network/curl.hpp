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
#pragma once
#include "header.hpp"

#include <future>
#include <mutex>

#include "libs/curl/curl.h"


struct Response
{
	CURLcode errorcode;
	int statuscode;
	std::string body;
};

class Curl
{
private:
	static std::string _certbundlefilename;

public:
	static void setRoot(const std::string& root);

private:
	struct ReadData;
	class ParsedResponseHeaders;
	struct Transfer;

	static size_t read(char* buffer, size_t size, size_t nitems,
		void* dataptr);
	static size_t write(char* buffer, size_t size, size_t nitems,
		void* dataptr);
	static size_t write_to_file(char* buffer, size_t size, size_t nitems,
		void* fileptr);
	static int update_progress(void* ptr, size_t dltotal, size_t dlnow,
		size_t ultotal, size_t ulnow);
	static size_t read_header(char* buffer, size_t size, size_t nitems,
		void* dataptr);

public:
	Curl(std::string useragent);
	~Curl();
	Curl(const Curl&) = delete;
	Curl(Curl&&) = delete;
	Curl& operator=(const Curl&) = delete;
	Curl& operator=(Curl&&) = delete;

private:
	std::string _useragent;

	std::mutex _multimutex;
	CURLM* _multi;
	int _multirunning;

	std::mutex _transferstoragemutex;
	std::vector<Transfer> _transfers;

	void begin(Transfer transfer);
	Transfer extract(CURL* easy, CURLcode errorcode);
	void finish(Transfer transfer);

public:
	void update();

	std::future<Response> get(const std::string& url,
		const std::vector<std::string>& headers = std::vector<std::string>());
	std::future<Response>  post(const std::string& url,
		const Json::Value& json,
		const std::vector<std::string>& headers = std::vector<std::string>());
	std::future<Response>  post(const std::string& url,
		const std::vector<std::string>& files,
		const Json::Value& json,
		const std::vector<std::string>& headers = std::vector<std::string>());
	std::future<Response> download(const std::string& url,
		const std::string& filename,
		std::shared_ptr<std::atomic<float>> progressmeter);
	std::future<Response> download(const std::string& url,
		const std::string& filename,
		const std::string& etagfilename);
};
