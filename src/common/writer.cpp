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
#include "writer.hpp"


#if STATIC_WRITER_ENABLED
/* ######################### STATIC_WRITER_ENABLED  ######################### */

#include <map>
#include <thread>
#include <mutex>


static std::map<std::thread::id, Writer*> _installed;
static std::mutex _writerMutex;

std::string Writer::write(const Json::Value& json)
{
	std::ostringstream sstream;
	Writer* writer;
	try
	{
		writer = _installed.at(std::this_thread::get_id());
	}
	catch (const std::out_of_range&)
	{
		LOGW << "No writer installed in thread " << std::this_thread::get_id();
		return "";
	}
	writer->_writer->write(json, &sstream);
	return sstream.str();
}

Writer::Writer()
{
	Json::StreamWriterBuilder builder;
	builder["indentation"] = "";
	_writer.reset(builder.newStreamWriter());
}

void Writer::install()
{
	std::lock_guard<std::mutex> lock(_writerMutex);
	_installed[std::this_thread::get_id()] = this;
}

/* ######################### STATIC_WRITER_ENABLED  ######################### */
#endif
