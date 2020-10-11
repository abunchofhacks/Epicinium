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
#include "curl.hpp"
#include "source.hpp"

#include <regex>

#include "writer.hpp"
#include "system.hpp"

std::string Curl::_certbundlefilename = "keys/curl-ca-bundle.crt";

void Curl::setRoot(const std::string& root)
{
	if (root.empty())
	{
		_certbundlefilename = "keys/curl-ca-bundle.crt";
	}
	else if (root.back() == '/')
	{
		_certbundlefilename = root + "keys/curl-ca-bundle.crt";
	}
	else
	{
		_certbundlefilename = root + "/keys/curl-ca-bundle.crt";
	}
}

struct Curl::ReadData
{
	const char* text;
	size_t size;
};

class Curl::ParsedResponseHeaders
{
private:
	std::vector<std::string> upperkeys; // (married)
	std::vector<std::string> values; // (married)

	bool parsing = false;

public:
	bool parseLine(const std::string& line);

	std::string etag() const;
};

struct Curl::Transfer
{
	CURL* easy = nullptr;
	curl_slist* headerlist = nullptr;
	curl_mime* mime = nullptr;
	FILE* file = nullptr;
	FILE* etagfile = nullptr;
	std::vector<char> readbuffer;
	std::unique_ptr<ReadData> readdata = nullptr;
	std::shared_ptr<std::atomic<float>> progressmeter = nullptr;
	std::unique_ptr<ParsedResponseHeaders> responseheaders = nullptr;
	std::unique_ptr<Response> response = nullptr;
	std::promise<Response> promise;

	explicit Transfer(nullptr_t /**/)
	{}

	Transfer(CURL* easy,
			curl_slist* headerlist,
			std::unique_ptr<Response> response,
			std::promise<Response> promise) :
		easy(easy),
		headerlist(headerlist),
		response(std::move(response)),
		promise(std::move(promise))
	{}

	Transfer(CURL* easy,
			curl_slist* headerlist,
			curl_mime* mime,
			std::unique_ptr<Response> response,
			std::promise<Response> promise) :
		easy(easy),
		headerlist(headerlist),
		mime(mime),
		response(std::move(response)),
		promise(std::move(promise))
	{}

	Transfer(CURL* easy,
			curl_slist* headerlist,
			std::vector<char> readbuffer,
			std::unique_ptr<ReadData> readdata,
			std::unique_ptr<Response> response,
			std::promise<Response> promise) :
		easy(easy),
		headerlist(headerlist),
		readbuffer(std::move(readbuffer)),
		readdata(std::move(readdata)),
		response(std::move(response)),
		promise(std::move(promise))
	{}

	Transfer(CURL* easy,
			FILE* file,
			std::shared_ptr<std::atomic<float>> progressmeter,
			std::unique_ptr<Response> response,
			std::promise<Response> promise) :
		easy(easy),
		file(file),
		progressmeter(std::move(progressmeter)),
		response(std::move(response)),
		promise(std::move(promise))
	{}

	Transfer(CURL* easy,
			curl_slist* headerlist,
			FILE* file,
			FILE* etagfile,
			std::unique_ptr<ParsedResponseHeaders> responseheaders,
			std::unique_ptr<Response> response,
			std::promise<Response> promise) :
		easy(easy),
		headerlist(headerlist),
		file(file),
		etagfile(etagfile),
		responseheaders(std::move(responseheaders)),
		response(std::move(response)),
		promise(std::move(promise))
	{}
};

size_t Curl::read(char* buffer, size_t size, size_t nitems, void* dataptr)
{
	if (dataptr == nullptr) return 0;

	ReadData* data = (ReadData*) dataptr;

	if (data->size)
	{
		int written = (int) (std::min)(size * nitems, data->size);
		memcpy(buffer, data->text, written);
		data->text += written;
		data->size -= written;
		return written;
	}
	else
	{
		return 0;
	}
}

size_t Curl::write(char* buffer, size_t size, size_t nitems, void* dataptr)
{
	if (dataptr == nullptr) return 0;
	size_t length = size * nitems;

	std::string* response = (std::string*)(dataptr);
	response->append(buffer, length);

	return length;
}

size_t Curl::write_to_file(char* buffer, size_t size, size_t nitems, void* file)
{
	size_t written = fwrite(buffer, size, nitems, (FILE*) file);
	return written;
}

int Curl::update_progress(void* ptr, size_t dltotal, size_t dlnow,
	size_t /**/, size_t /**/)
{
	if (dltotal > 0 && dlnow > 0)
	{
		float percentage = 100.0f * dlnow / dltotal;

		std::atomic<float>& progressmeter = *((std::atomic<float>*) ptr);
		progressmeter.store(percentage);
	}

	return 0;
}

size_t Curl::read_header(char* buffer, size_t size, size_t nitems,
	void* dataptr)
{
	if (dataptr == nullptr) return 0;

	ParsedResponseHeaders* data = (ParsedResponseHeaders*) dataptr;

	size_t length = size * nitems;
	std::string line(buffer, length);
	if (data->parseLine(line))
	{
		return length;
	}
	else
	{
		return 0;
	}
}

Curl::Curl(std::string useragent) :
	_useragent(std::move(useragent))
{
	_multi = curl_multi_init();
	if (!_multi)
	{
		LOGF << "Curl multi failed";
	}
	DEBUG_ASSERT(_multi);
	_multirunning = -1;
}

Curl::~Curl()
{
	if (!_multi) return;

	CURLMcode code = curl_multi_cleanup(_multi);
	if (code != CURLM_OK)
	{
		LOGF << "Error during cleanup: " << curl_multi_strerror(code);
		DEBUG_ASSERT(false);
	}
}

void Curl::update()
{
	if (!_multi) return;

	// If it is positive, something is running.
	// If it is negative, something was added and therefore should be running.
	if (_multirunning == 0)
	{
		// Wait until something is added to the multi.
		return;
	}

	CURLMcode code = CURLM_OK;
	std::vector<std::pair<CURL*, CURLcode>> completed;

	{
		std::lock_guard<std::mutex> lock(_multimutex);

		int running = -1;
		code = curl_multi_perform(_multi, &running);

		if (running != _multirunning)
		{
			_multirunning = running;

			int _ignored = 0;
			struct CURLMsg* m = curl_multi_info_read(_multi, &_ignored);
			while (m && code == CURLM_OK)
			{
				if (m->msg == CURLMSG_DONE)
				{
					code = curl_multi_remove_handle(_multi, m->easy_handle);
					CURLcode errorcode = m->data.result;
					completed.emplace_back(m->easy_handle, errorcode);
				}
				m = curl_multi_info_read(_multi, &_ignored);
			}
		}
	}

	if (code != CURLM_OK)
	{
		LOGF << "Multi error: " << curl_multi_strerror(code);
		DEBUG_ASSERT(false);
	}

	for (auto pair : completed)
	{
		CURL* easy;
		CURLcode errorcode;
		std::tie(easy, errorcode) = pair;
		finish(extract(easy, errorcode));
	}
}

void Curl::begin(Transfer transfer)
{
	{
		std::lock_guard<std::mutex> lock(_transferstoragemutex);
		size_t i = 0;
		for (; i < _transfers.size() && _transfers[i].easy != nullptr; i++)
		{}
		if (i < _transfers.size())
		{
			_transfers[i] = std::move(transfer);
		}
		else
		{
			_transfers.emplace_back(std::move(transfer));
		}
	}

	CURLMcode code = CURLM_OK;

	{
		std::lock_guard<std::mutex> lock(_multimutex);
		code = curl_multi_add_handle(_multi, transfer.easy);
		// Mark that something was added to the multi.
		_multirunning = -1;
	}

	if (code != CURLM_OK)
	{
		LOGF << "Multi error: " << curl_multi_strerror(code);
		DEBUG_ASSERT(false);
	}
}

Curl::Transfer Curl::extract(CURL* easy, CURLcode errorcode)
{
	Transfer transfer = Transfer(nullptr);

	{
		std::lock_guard<std::mutex> lock(_transferstoragemutex);
		for (size_t i = 0; i < _transfers.size(); i++)
		{
			if (_transfers[i].easy == easy)
			{
				std::swap(transfer, _transfers[i]);
				break;
			}
		}
	}

	transfer.response->errorcode = errorcode;

	return transfer;
}

void Curl::finish(Transfer transfer)
{
	CURL* curl = transfer.easy;
	if (!curl)
	{
		LOGF << "Finishing empty transfer";
		DEBUG_ASSERT(false);
		return;
	}

	CURLcode errorcode = transfer.response->errorcode;

	if (errorcode == CURLE_OK)
	{
		int statuscode = 0;
		errorcode = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE,
			&statuscode);
		transfer.response->statuscode = statuscode;
	}

	if (errorcode != CURLE_OK)
	{
		LOGE << "Curl (" << ((void*) curl) << ")"
			<< " error code " << errorcode << ": "
			<< curl_easy_strerror(errorcode);
	}

	if (transfer.etagfile)
	{
		if (transfer.responseheaders)
		{
			std::string etag = transfer.responseheaders->etag();
			if (!etag.empty())
			{
				fwrite(etag.c_str(), 1, etag.size(), transfer.etagfile);
			}
		}

		fclose(transfer.etagfile);
	}

	if (transfer.file)
	{
		fclose(transfer.file);
	}

	// TODO this seems to segfault on OSX64 if Response is not explicitly
	// copied. Why!?
	Response response;
	response.errorcode = transfer.response->errorcode;
	response.statuscode = transfer.response->statuscode;
	response.body = std::move(transfer.response->body);
	transfer.promise.set_value(std::move(response));

	curl_slist_free_all(transfer.headerlist);
	curl_mime_free(transfer.mime);
	curl_easy_cleanup(curl);
}

std::future<Response> Curl::get(const std::string& url,
	const std::vector<std::string>& headers)
{
	std::promise<Response> promise;

	CURL* curl = curl_easy_init();
	if (!curl)
	{
		LOGE << "Curl failed";
		promise.set_value(Response{CURLE_FAILED_INIT, -1, ""});
		return promise.get_future();
	}

	curl_slist* headerlist = nullptr;
	for (const std::string& header : headers)
	{
		headerlist = curl_slist_append(headerlist, header.c_str());
	}

	std::unique_ptr<Response> response(new Response{CURLE_OK, -1, ""});

	CURLcode code = CURLE_OK;
	code = curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	if (code != CURLE_OK)
	{
		LOGE << "Curl setup failed (" << ((void*) curl) << ")"
			<< " error code " << code << ": "
			<< curl_easy_strerror(code);
		promise.set_value(Response{code, -1, ""});
		return promise.get_future();
	}
#ifndef DEVELOPMENT
	code = curl_easy_setopt(curl, CURLOPT_PROTOCOLS, CURLPROTO_HTTPS);
	if (code != CURLE_OK)
	{
		LOGE << "Curl setup failed (" << ((void*) curl) << ")"
			<< " error code " << code << ": "
			<< curl_easy_strerror(code);
		promise.set_value(Response{code, -1, ""});
		return promise.get_future();
	}
#endif
	code = curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, true);
	if (code != CURLE_OK)
	{
		LOGE << "Curl setup failed (" << ((void*) curl) << ")"
			<< " error code " << code << ": "
			<< curl_easy_strerror(code);
		promise.set_value(Response{code, -1, ""});
		return promise.get_future();
	}
	code = curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 5);
	if (code != CURLE_OK)
	{
		LOGE << "Curl setup failed (" << ((void*) curl) << ")"
			<< " error code " << code << ": "
			<< curl_easy_strerror(code);
		promise.set_value(Response{code, -1, ""});
		return promise.get_future();
	}
	code = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write);
	if (code != CURLE_OK)
	{
		LOGE << "Curl setup failed (" << ((void*) curl) << ")"
			<< " error code " << code << ": "
			<< curl_easy_strerror(code);
		promise.set_value(Response{code, -1, ""});
		return promise.get_future();
	}
	code = curl_easy_setopt(curl, CURLOPT_WRITEDATA, &(response->body));
	if (code != CURLE_OK)
	{
		LOGE << "Curl setup failed (" << ((void*) curl) << ")"
			<< " error code " << code << ": "
			<< curl_easy_strerror(code);
		promise.set_value(Response{code, -1, ""});
		return promise.get_future();
	}
	code = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);
	if (code != CURLE_OK)
	{
		LOGE << "Curl setup failed (" << ((void*) curl) << ")"
			<< " error code " << code << ": "
			<< curl_easy_strerror(code);
		promise.set_value(Response{code, -1, ""});
		return promise.get_future();
	}
	code = curl_easy_setopt(curl, CURLOPT_USERAGENT, _useragent.c_str());
	if (code != CURLE_OK)
	{
		LOGE << "Curl setup failed (" << ((void*) curl) << ")"
			<< " error code " << code << ": "
			<< curl_easy_strerror(code);
		promise.set_value(Response{code, -1, ""});
		return promise.get_future();
	}
	code = curl_easy_setopt(curl, CURLOPT_CAINFO, _certbundlefilename.c_str());
	if (code != CURLE_OK)
	{
		LOGE << "Curl setup failed (" << ((void*) curl) << ")"
			<< " error code " << code << ": "
			<< curl_easy_strerror(code);
		promise.set_value(Response{code, -1, ""});
		return promise.get_future();
	}

	std::future<Response> future = promise.get_future();
	begin(Transfer(curl, headerlist,
		std::move(response),
		std::move(promise)));
	LOGI << "Curl (" << ((void*) curl) << ") sending GET to " << url;
	return future;
}

std::future<Response> Curl::post(const std::string& url,
	const Json::Value& json,
	const std::vector<std::string>& headers)
{
	std::promise<Response> promise;

	CURL* curl = curl_easy_init();
	if (!curl)
	{
		LOGE << "Curl failed";
		promise.set_value({CURLE_FAILED_INIT, -1, ""});
		return promise.get_future();
	}

	std::string jsonstr;
	{
		std::stringstream strm;
		strm << Writer::write(json);
		jsonstr = strm.str();
	}
	std::vector<char> jsonbuffer = {jsonstr.begin(), jsonstr.end()};
	std::unique_ptr<ReadData> readdata(
		new ReadData{jsonbuffer.data(), jsonbuffer.size()});

	curl_slist* headerlist = nullptr;
	headerlist = curl_slist_append(headerlist,
		"Content-Type: application/json");
	for (const std::string& header : headers)
	{
		headerlist = curl_slist_append(headerlist, header.c_str());
	}

	std::unique_ptr<Response> response(new Response{CURLE_OK, -1, ""});

	CURLcode code = CURLE_OK;
	code = curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	if (code != CURLE_OK)
	{
		LOGE << "Curl setup failed (" << ((void*) curl) << ")"
			<< " error code " << code << ": "
			<< curl_easy_strerror(code);
		promise.set_value(Response{code, -1, ""});
		return promise.get_future();
	}
#ifndef DEVELOPMENT
	code = curl_easy_setopt(curl, CURLOPT_PROTOCOLS, CURLPROTO_HTTPS);
	if (code != CURLE_OK)
	{
		LOGE << "Curl setup failed (" << ((void*) curl) << ")"
			<< " error code " << code << ": "
			<< curl_easy_strerror(code);
		promise.set_value(Response{code, -1, ""});
		return promise.get_future();
	}
#endif
	code = curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, true);
	if (code != CURLE_OK)
	{
		LOGE << "Curl setup failed (" << ((void*) curl) << ")"
			<< " error code " << code << ": "
			<< curl_easy_strerror(code);
		promise.set_value(Response{code, -1, ""});
		return promise.get_future();
	}
	code = curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 5);
	if (code != CURLE_OK)
	{
		LOGE << "Curl setup failed (" << ((void*) curl) << ")"
			<< " error code " << code << ": "
			<< curl_easy_strerror(code);
		promise.set_value(Response{code, -1, ""});
		return promise.get_future();
	}
	code = curl_easy_setopt(curl, CURLOPT_POST, true);
	if (code != CURLE_OK)
	{
		LOGE << "Curl setup failed (" << ((void*) curl) << ")"
			<< " error code " << code << ": "
			<< curl_easy_strerror(code);
		promise.set_value(Response{code, -1, ""});
		return promise.get_future();
	}
	code = curl_easy_setopt(curl, CURLOPT_READFUNCTION, read);
	if (code != CURLE_OK)
	{
		LOGE << "Curl setup failed (" << ((void*) curl) << ")"
			<< " error code " << code << ": "
			<< curl_easy_strerror(code);
		promise.set_value(Response{code, -1, ""});
		return promise.get_future();
	}
	code = curl_easy_setopt(curl, CURLOPT_READDATA, readdata.get());
	if (code != CURLE_OK)
	{
		LOGE << "Curl setup failed (" << ((void*) curl) << ")"
			<< " error code " << code << ": "
			<< curl_easy_strerror(code);
		promise.set_value(Response{code, -1, ""});
		return promise.get_future();
	}
	code = curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, readdata->size);
	if (code != CURLE_OK)
	{
		LOGE << "Curl setup failed (" << ((void*) curl) << ")"
			<< " error code " << code << ": "
			<< curl_easy_strerror(code);
		promise.set_value(Response{code, -1, ""});
		return promise.get_future();
	}
	code = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write);
	if (code != CURLE_OK)
	{
		LOGE << "Curl setup failed (" << ((void*) curl) << ")"
			<< " error code " << code << ": "
			<< curl_easy_strerror(code);
		promise.set_value(Response{code, -1, ""});
		return promise.get_future();
	}
	code = curl_easy_setopt(curl, CURLOPT_WRITEDATA, &(response->body));
	if (code != CURLE_OK)
	{
		LOGE << "Curl setup failed (" << ((void*) curl) << ")"
			<< " error code " << code << ": "
			<< curl_easy_strerror(code);
		promise.set_value(Response{code, -1, ""});
		return promise.get_future();
	}
	code = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);
	if (code != CURLE_OK)
	{
		LOGE << "Curl setup failed (" << ((void*) curl) << ")"
			<< " error code " << code << ": "
			<< curl_easy_strerror(code);
		promise.set_value(Response{code, -1, ""});
		return promise.get_future();
	}
	code = curl_easy_setopt(curl, CURLOPT_USERAGENT, _useragent.c_str());
	if (code != CURLE_OK)
	{
		LOGE << "Curl setup failed (" << ((void*) curl) << ")"
			<< " error code " << code << ": "
			<< curl_easy_strerror(code);
		promise.set_value(Response{code, -1, ""});
		return promise.get_future();
	}
	code = curl_easy_setopt(curl, CURLOPT_CAINFO, _certbundlefilename.c_str());
	if (code != CURLE_OK)
	{
		LOGE << "Curl setup failed (" << ((void*) curl) << ")"
			<< " error code " << code << ": "
			<< curl_easy_strerror(code);
		promise.set_value(Response{code, -1, ""});
		return promise.get_future();
	}

	std::future<Response> future = promise.get_future();
	begin(Transfer(curl, headerlist,
		std::move(jsonbuffer),
		std::move(readdata),
		std::move(response),
		std::move(promise)));
	LOGI << "Curl (" << ((void*) curl) << ") sending POST to " << url
		<< ": " << jsonstr;
	return future;
}

std::future<Response> Curl::post(const std::string& url,
	const std::vector<std::string>& files,
	const Json::Value& json,
	const std::vector<std::string>& headers)
{
	std::promise<Response> promise;

	CURL* curl = curl_easy_init();
	if (!curl)
	{
		LOGE << "Curl failed";
		promise.set_value({CURLE_FAILED_INIT, -1, ""});
		return promise.get_future();
	}

	curl_mime* mime = curl_mime_init(curl);
	if (!mime)
	{
		LOGE << "Curl setup failed (" << ((void*) curl) << ")"
			<< " when initializing mime";
		promise.set_value(Response{CURLE_FAILED_INIT, -1, ""});
		return promise.get_future();
	}

	if (json.isObject())
	{
		for (auto& idx : json.getMemberNames())
		{
			curl_mimepart* part = curl_mime_addpart(mime);
			if (!part)
			{
				LOGE << "Curl setup failed (" << ((void*) curl) << ")"
					<< " when add part to mime";
				promise.set_value(Response{CURLE_FAILED_INIT, -1, ""});
				return promise.get_future();
			}
			CURLcode code = CURLE_OK;
			code = curl_mime_name(part, idx.c_str());
			if (code != CURLE_OK)
			{
				LOGE << "Curl setup failed (" << ((void*) curl) << ")"
					<< " error code " << code << ": "
					<< curl_easy_strerror(code);
				promise.set_value(Response{code, -1, ""});
				return promise.get_future();
			}
			code = curl_mime_type(part, "text/plain");
			if (code != CURLE_OK)
			{
				LOGE << "Curl setup failed (" << ((void*) curl) << ")"
					<< " error code " << code << ": "
					<< curl_easy_strerror(code);
				promise.set_value(Response{code, -1, ""});
				return promise.get_future();
			}
			std::string value = json[idx].asString();
			code = curl_mime_data(part, value.c_str(), CURL_ZERO_TERMINATED);
			if (code != CURLE_OK)
			{
				LOGE << "Curl setup failed (" << ((void*) curl) << ")"
					<< " error code " << code << ": "
					<< curl_easy_strerror(code);
				promise.set_value(Response{code, -1, ""});
				return promise.get_future();
			}
		}
	}

	for (auto& file : files)
	{
		curl_mimepart* part = curl_mime_addpart(mime);
		if (!part)
		{
			LOGE << "Curl setup failed (" << ((void*) curl) << ")"
				<< " when add part to mime";
			promise.set_value(Response{CURLE_FAILED_INIT, -1, ""});
			return promise.get_future();
		}
		CURLcode code = CURLE_OK;
		code = curl_mime_name(part, file.c_str());
		if (code != CURLE_OK)
		{
			LOGE << "Curl setup failed (" << ((void*) curl) << ")"
				<< " error code " << code << ": "
				<< curl_easy_strerror(code);
			promise.set_value(Response{code, -1, ""});
			return promise.get_future();
		}
		code = curl_mime_filename(part, file.c_str());
		if (code != CURLE_OK)
		{
			LOGE << "Curl setup failed (" << ((void*) curl) << ")"
				<< " error code " << code << ": "
				<< curl_easy_strerror(code);
			promise.set_value(Response{code, -1, ""});
			return promise.get_future();
		}
		code = curl_mime_type(part, "application/octet-stream");
		if (code != CURLE_OK)
		{
			LOGE << "Curl setup failed (" << ((void*) curl) << ")"
				<< " error code " << code << ": "
				<< curl_easy_strerror(code);
			promise.set_value(Response{code, -1, ""});
			return promise.get_future();
		}
		code = curl_mime_filedata(part, file.c_str());
		if (code != CURLE_OK)
		{
			LOGE << "Curl setup failed (" << ((void*) curl) << ")"
				<< " error code " << code << ": "
				<< curl_easy_strerror(code);
			promise.set_value(Response{code, -1, ""});
			return promise.get_future();
		}
	}

	curl_slist* headerlist = nullptr;
	headerlist = curl_slist_append(headerlist,
		"Content-Type: multipart/form-data");
	if (!headerlist)
	{
		LOGE << "Curl setup failed (" << ((void*) curl) << ")"
			<< " while initializing headerlist";
		promise.set_value(Response{CURLE_FAILED_INIT, -1, ""});
		return promise.get_future();
	}
	for (const std::string& header : headers)
	{
		headerlist = curl_slist_append(headerlist, header.c_str());
		if (!headerlist)
		{
			LOGE << "Curl setup failed (" << ((void*) curl) << ")"
				<< " while appending headerlist";
			promise.set_value(Response{CURLE_FAILED_INIT, -1, ""});
			return promise.get_future();
		}
	}

	std::unique_ptr<Response> response(new Response{CURLE_OK, -1, ""});

	CURLcode code = CURLE_OK;
	code = curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	if (code != CURLE_OK)
	{
		LOGE << "Curl setup failed (" << ((void*) curl) << ")"
			<< " error code " << code << ": "
			<< curl_easy_strerror(code);
		promise.set_value(Response{code, -1, ""});
		return promise.get_future();
	}
#ifndef DEVELOPMENT
	code = curl_easy_setopt(curl, CURLOPT_PROTOCOLS, CURLPROTO_HTTPS);
	if (code != CURLE_OK)
	{
		LOGE << "Curl setup failed (" << ((void*) curl) << ")"
			<< " error code " << code << ": "
			<< curl_easy_strerror(code);
		promise.set_value(Response{code, -1, ""});
		return promise.get_future();
	}
#endif
	code = curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, true);
	if (code != CURLE_OK)
	{
		LOGE << "Curl setup failed (" << ((void*) curl) << ")"
			<< " error code " << code << ": "
			<< curl_easy_strerror(code);
		promise.set_value(Response{code, -1, ""});
		return promise.get_future();
	}
	code = curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 5);
	if (code != CURLE_OK)
	{
		LOGE << "Curl setup failed (" << ((void*) curl) << ")"
			<< " error code " << code << ": "
			<< curl_easy_strerror(code);
		promise.set_value(Response{code, -1, ""});
		return promise.get_future();
	}
	code = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write);
	if (code != CURLE_OK)
	{
		LOGE << "Curl setup failed (" << ((void*) curl) << ")"
			<< " error code " << code << ": "
			<< curl_easy_strerror(code);
		promise.set_value(Response{code, -1, ""});
		return promise.get_future();
	}
	code = curl_easy_setopt(curl, CURLOPT_WRITEDATA, &(response->body));
	if (code != CURLE_OK)
	{
		LOGE << "Curl setup failed (" << ((void*) curl) << ")"
			<< " error code " << code << ": "
			<< curl_easy_strerror(code);
		promise.set_value(Response{code, -1, ""});
		return promise.get_future();
	}
	code = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);
	if (code != CURLE_OK)
	{
		LOGE << "Curl setup failed (" << ((void*) curl) << ")"
			<< " error code " << code << ": "
			<< curl_easy_strerror(code);
		promise.set_value(Response{code, -1, ""});
		return promise.get_future();
	}
	code = curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);
	if (code != CURLE_OK)
	{
		LOGE << "Curl setup failed (" << ((void*) curl) << ")"
			<< " error code " << code << ": "
			<< curl_easy_strerror(code);
		promise.set_value(Response{code, -1, ""});
		return promise.get_future();
	}
	code = curl_easy_setopt(curl, CURLOPT_USERAGENT, _useragent.c_str());
	if (code != CURLE_OK)
	{
		LOGE << "Curl setup failed (" << ((void*) curl) << ")"
			<< " error code " << code << ": "
			<< curl_easy_strerror(code);
		promise.set_value(Response{code, -1, ""});
		return promise.get_future();
	}
	code = curl_easy_setopt(curl, CURLOPT_CAINFO, _certbundlefilename.c_str());
	if (code != CURLE_OK)
	{
		LOGE << "Curl setup failed (" << ((void*) curl) << ")"
			<< " error code " << code << ": "
			<< curl_easy_strerror(code);
		promise.set_value(Response{code, -1, ""});
		return promise.get_future();
	}

	std::future<Response> future = promise.get_future();
	begin(Transfer(curl, headerlist, mime,
		std::move(response),
		std::move(promise)));
	LOGI << "Curl (" << ((void*) curl) << ") sending upload POST to " << url
		<< ": " << Writer::write(json);
	return future;
}

std::future<Response> Curl::download(const std::string& url,
	const std::string& filename,
	std::shared_ptr<std::atomic<float>> progressmeter)
{
	std::promise<Response> promise;

	CURL* curl = curl_easy_init();
	if (!curl)
	{
		LOGE << "Curl failed";
		promise.set_value({CURLE_FAILED_INIT, -1, ""});
		return promise.get_future();
	}

	std::unique_ptr<Response> response(new Response{CURLE_OK, -1,
		"(written to " + filename + ")"});

	System::touchFile(filename);
	FILE* file = fopen(filename.c_str(), "wb");

	CURLcode code = CURLE_OK;
	code = curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	if (code != CURLE_OK)
	{
		LOGE << "Curl setup failed (" << ((void*) curl) << ")"
			<< " error code " << code << ": "
			<< curl_easy_strerror(code);
		promise.set_value(Response{code, -1, ""});
		return promise.get_future();
	}
#ifndef DEVELOPMENT
	code = curl_easy_setopt(curl, CURLOPT_PROTOCOLS, CURLPROTO_HTTPS);
	if (code != CURLE_OK)
	{
		LOGE << "Curl setup failed (" << ((void*) curl) << ")"
			<< " error code " << code << ": "
			<< curl_easy_strerror(code);
		promise.set_value(Response{code, -1, ""});
		return promise.get_future();
	}
#endif
	code = curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, true);
	if (code != CURLE_OK)
	{
		LOGE << "Curl setup failed (" << ((void*) curl) << ")"
			<< " error code " << code << ": "
			<< curl_easy_strerror(code);
		promise.set_value(Response{code, -1, ""});
		return promise.get_future();
	}
	code = curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 5);
	if (code != CURLE_OK)
	{
		LOGE << "Curl setup failed (" << ((void*) curl) << ")"
			<< " error code " << code << ": "
			<< curl_easy_strerror(code);
		promise.set_value(Response{code, -1, ""});
		return promise.get_future();
	}
	code = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_to_file);
	if (code != CURLE_OK)
	{
		LOGE << "Curl setup failed (" << ((void*) curl) << ")"
			<< " error code " << code << ": "
			<< curl_easy_strerror(code);
		promise.set_value(Response{code, -1, ""});
		return promise.get_future();
	}
	code = curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
	if (code != CURLE_OK)
	{
		LOGE << "Curl setup failed (" << ((void*) curl) << ")"
			<< " error code " << code << ": "
			<< curl_easy_strerror(code);
		promise.set_value(Response{code, -1, ""});
		return promise.get_future();
	}
	code = curl_easy_setopt(curl, CURLOPT_USERAGENT, _useragent.c_str());
	if (code != CURLE_OK)
	{
		LOGE << "Curl setup failed (" << ((void*) curl) << ")"
			<< " error code " << code << ": "
			<< curl_easy_strerror(code);
		promise.set_value(Response{code, -1, ""});
		return promise.get_future();
	}
	code = curl_easy_setopt(curl, CURLOPT_CAINFO, _certbundlefilename.c_str());
	if (code != CURLE_OK)
	{
		LOGE << "Curl setup failed (" << ((void*) curl) << ")"
			<< " error code " << code << ": "
			<< curl_easy_strerror(code);
		promise.set_value(Response{code, -1, ""});
		return promise.get_future();
	}
	code = curl_easy_setopt(curl, CURLOPT_NOPROGRESS, false);
	if (code != CURLE_OK)
	{
		LOGE << "Curl setup failed (" << ((void*) curl) << ")"
			<< " error code " << code << ": "
			<< curl_easy_strerror(code);
		promise.set_value(Response{code, -1, ""});
		return promise.get_future();
	}
	code = curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, update_progress);
	if (code != CURLE_OK)
	{
		LOGE << "Curl setup failed (" << ((void*) curl) << ")"
			<< " error code " << code << ": "
			<< curl_easy_strerror(code);
		promise.set_value(Response{code, -1, ""});
		return promise.get_future();
	}
	code = curl_easy_setopt(curl, CURLOPT_XFERINFODATA, progressmeter.get());
	if (code != CURLE_OK)
	{
		LOGE << "Curl setup failed (" << ((void*) curl) << ")"
			<< " error code " << code << ": "
			<< curl_easy_strerror(code);
		promise.set_value(Response{code, -1, ""});
		return promise.get_future();
	}

	std::future<Response> future = promise.get_future();
	begin(Transfer(curl,
		file,
		progressmeter,
		std::move(response),
		std::move(promise)));
	LOGI << "Curl (" << ((void*) curl) << ") sending GET to " << url;
	return future;
}

std::future<Response> Curl::download(const std::string& url,
	const std::string& filename,
	const std::string& etagfilename)
{
	std::promise<Response> promise;

	CURL* curl = curl_easy_init();
	if (!curl)
	{
		LOGE << "Curl failed";
		promise.set_value({CURLE_FAILED_INIT, -1, ""});
		return promise.get_future();
	}

	curl_slist* headerlist = nullptr;

	if (System::isFile(etagfilename))
	{
		std::ifstream etagfile(etagfilename);
		std::string etag;
		if (etagfile && (std::getline(etagfile, etag) || etagfile.eof()))
		{
			if (!etag.empty())
			{
				std::string ifnonematch = "If-None-Match: \"" + etag + "\"";
				headerlist = curl_slist_append(headerlist, ifnonematch.c_str());
				if (!headerlist)
				{
					LOGE << "Curl setup failed (" << ((void*) curl) << ")"
						<< " while appending headerlist";
					promise.set_value(Response{CURLE_FAILED_INIT, -1, ""});
					return promise.get_future();
				}
			}
		}
		else
		{
			LOGW << "Failed to read etag from file '" << etagfilename << "'";
		}
	}

	std::unique_ptr<ParsedResponseHeaders> parsed(new ParsedResponseHeaders());

	std::unique_ptr<Response> response(new Response{CURLE_OK, -1,
		"(written to " + filename + ")"});

	System::touchFile(filename);
	FILE* file = fopen(filename.c_str(), "wb");

	System::touchFile(etagfilename);
	FILE* etagfile = fopen(etagfilename.c_str(), "wb");

	CURLcode code = CURLE_OK;
	code = curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	if (code != CURLE_OK)
	{
		LOGE << "Curl setup failed (" << ((void*) curl) << ")"
			<< " error code " << code << ": "
			<< curl_easy_strerror(code);
		promise.set_value(Response{code, -1, ""});
		return promise.get_future();
	}
#ifndef DEVELOPMENT
	code = curl_easy_setopt(curl, CURLOPT_PROTOCOLS, CURLPROTO_HTTPS);
	if (code != CURLE_OK)
	{
		LOGE << "Curl setup failed (" << ((void*) curl) << ")"
			<< " error code " << code << ": "
			<< curl_easy_strerror(code);
		promise.set_value(Response{code, -1, ""});
		return promise.get_future();
	}
#endif
	code = curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, true);
	if (code != CURLE_OK)
	{
		LOGE << "Curl setup failed (" << ((void*) curl) << ")"
			<< " error code " << code << ": "
			<< curl_easy_strerror(code);
		promise.set_value(Response{code, -1, ""});
		return promise.get_future();
	}
	code = curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 5);
	if (code != CURLE_OK)
	{
		LOGE << "Curl setup failed (" << ((void*) curl) << ")"
			<< " error code " << code << ": "
			<< curl_easy_strerror(code);
		promise.set_value(Response{code, -1, ""});
		return promise.get_future();
	}
	code = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_to_file);
	if (code != CURLE_OK)
	{
		LOGE << "Curl setup failed (" << ((void*) curl) << ")"
			<< " error code " << code << ": "
			<< curl_easy_strerror(code);
		promise.set_value(Response{code, -1, ""});
		return promise.get_future();
	}
	code = curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
	if (code != CURLE_OK)
	{
		LOGE << "Curl setup failed (" << ((void*) curl) << ")"
			<< " error code " << code << ": "
			<< curl_easy_strerror(code);
		promise.set_value(Response{code, -1, ""});
		return promise.get_future();
	}
	code = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);
	if (code != CURLE_OK)
	{
		LOGE << "Curl setup failed (" << ((void*) curl) << ")"
			<< " error code " << code << ": "
			<< curl_easy_strerror(code);
		promise.set_value(Response{code, -1, ""});
		return promise.get_future();
	}
	code = curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, read_header);
	if (code != CURLE_OK)
	{
		LOGE << "Curl setup failed (" << ((void*) curl) << ")"
			<< " error code " << code << ": "
			<< curl_easy_strerror(code);
		promise.set_value(Response{code, -1, ""});
		return promise.get_future();
	}
	code = curl_easy_setopt(curl, CURLOPT_HEADERDATA, parsed.get());
	if (code != CURLE_OK)
	{
		LOGE << "Curl setup failed (" << ((void*) curl) << ")"
			<< " error code " << code << ": "
			<< curl_easy_strerror(code);
		promise.set_value(Response{code, -1, ""});
		return promise.get_future();
	}
	code = curl_easy_setopt(curl, CURLOPT_USERAGENT, _useragent.c_str());
	if (code != CURLE_OK)
	{
		LOGE << "Curl setup failed (" << ((void*) curl) << ")"
			<< " error code " << code << ": "
			<< curl_easy_strerror(code);
		promise.set_value(Response{code, -1, ""});
		return promise.get_future();
	}
	code = curl_easy_setopt(curl, CURLOPT_CAINFO, _certbundlefilename.c_str());
	if (code != CURLE_OK)
	{
		LOGE << "Curl setup failed (" << ((void*) curl) << ")"
			<< " error code " << code << ": "
			<< curl_easy_strerror(code);
		promise.set_value(Response{code, -1, ""});
		return promise.get_future();
	}

	std::future<Response> future = promise.get_future();
	begin(Transfer(curl, headerlist,
		file,
		etagfile,
		std::move(parsed),
		std::move(response),
		std::move(promise)));
	LOGI << "Curl (" << ((void*) curl) << ") sending GET to " << url;
	return future;
}


// Source: RFC 7230

// The key consists of:
//                             ALPHA DIGIT !#$%&'*+-.^_`|~
#define HTTP_HEADER_KEY_RGX "([a-zA-Z0-9!#\\$%&'\\*\\+\\-\\.\\^_`\\|~]+)"

// The value consists of visible lowerhalf ASCII characters,
// but MAY contain upperhalf ASCII characters.
// There can also be horizontal whitespace within the value.
//                             VCHAR obs-text
#define HTTP_HEADER_VAL_RGX "([!-~\\x80-\\xFF]+(?:[ \t]+[!-~\\x80-\\xFF]+)*)"

bool Curl::ParsedResponseHeaders::parseLine(const std::string& line)
{
	std::regex headerregex = std::regex("^"
		// No whitespace before key.
		HTTP_HEADER_KEY_RGX
		// No whitespace between key and colon.
		":"
		// Optional horizontal whitespace between the colon and the value.
		"[ \t]*"
		// See above.
		HTTP_HEADER_VAL_RGX
		// Optional horizontal whitespace after the value.
		"[ \t]*"
		// Lines SHOULD end on CRLF but MAY end on LF.
		"\r?\n$");

	std::regex obsfoldregex = std::regex("^"
		// Line folding is indicated by horizontal whitespace before the
		// continuation of a value. This is obsolete but RFC7230 says we
		// MUST support it.
		"[ \t]+"
		// See above.
		HTTP_HEADER_VAL_RGX
		// Optional horizontal whitespace after the value.
		"[ \t]*"
		// Lines SHOULD end on CRLF but MAY end on LF.
		"\r?\n$");

	std::regex emptylineregex = std::regex("^"
		// Lines SHOULD end on CRLF but MAY end on LF.
		"\r?\n$");

	std::smatch match;
	if (!parsing)
	{
		upperkeys.clear();
		values.clear();
		parsing = true;
	}
	else if (std::regex_match(line, match, emptylineregex))
	{
		parsing = false;
	}
	else if (std::regex_match(line, match, headerregex))
	{
		if (match.size() != 3)
		{
			LOGE << "Invalid HTTP header regex match: " << line;
			return false;
		}

		// Store the key and value; match[0] is the entire line.
		upperkeys.emplace_back(::toupper(match[1]));
		values.emplace_back(match[2]);
	}
	else if (!values.empty()
		&& std::regex_match(line, match, obsfoldregex))
	{
		if (match.size() != 2)
		{
			LOGE << "Invalid HTTP header regex match: " << line;
			return false;
		}

		// Append the value to the last header; match[0] is the entire line.
		values.back().append(match[1]);
	}
	else
	{
		LOGE << "Invalid HTTP header line: @@@" << line << "@@@";
		return false;
	}

	return true;
}


// Source: RFC 7232

// The value consists of visible lowerhalf ASCII characters,
// except double quotes,
// but MAY contain upperhalf ASCII characters.
//                              VCHAR except double quotes, plus obs-text
#define HTTP_HEADER_ETAGC_RGX "([!#-~\\x80-\\xFF]+)"

std::string Curl::ParsedResponseHeaders::etag() const
{
	for (size_t i = 0; i < upperkeys.size(); i++)
	{
		if (upperkeys[i] != "ETAG") continue;

		const std::string& value = values[i];

		std::regex etagregex = std::regex("^"
			// Strong etags start with a double quote.
			"\""
			// See above.
			HTTP_HEADER_ETAGC_RGX
			// Entity tags end with a double quote.
			"\"$");
		std::regex weaketagregex = std::regex("^"
			// Weak etags start with a capital W, a slash and a double quote.
			"W/\""
			// See above.
			HTTP_HEADER_ETAGC_RGX
			// Entity tags end with a double quote.
			"\"$");

		std::smatch match;
		if (std::regex_match(value, match, etagregex))
		{
			return match[1];
		}
		else if (std::regex_match(value, match, weaketagregex))
		{
			LOGI << "Ignoring weak etag: " << value;
			return "";
		}
		else
		{
			LOGE << "Invalid etag header value: @@@" << value << "@@@";
			return "";
		}
	}

	return "";
}
