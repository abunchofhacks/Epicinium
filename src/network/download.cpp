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
#include "download.hpp"
#include "source.hpp"

#include "compress.hpp"
#include "base32.hpp"


std::string Download::_downloadsfolder = "downloads/";

void Download::setRoot(const std::string& root)
{
	if (root.empty())
	{
		_downloadsfolder = "downloads/";
	}
	else if (root.back() == '/')
	{
		_downloadsfolder = root + "downloads/";
	}
	else
	{
		_downloadsfolder = root + "/downloads/";
	}
}

Download Download::parse(const Json::Value& metadata)
{
	// TODO better error handling?
	std::string filename = metadata["name"].asString();

	Download dl;
	dl.sourcefilename = _downloadsfolder + filename;
	dl.targetfilename = filename;

	if (metadata["compressed"].isBool() && metadata["compressed"].asBool())
	{
		dl.compressed = true;
		dl.sourcefilename = Compress::gzipfilename(dl.sourcefilename);
	}

	if (metadata["checksum"].isString())
	{
		dl.checksum = Base32::decode(metadata["checksum"].asString());
	}

	if (metadata["data"].isString())
	{
		dl.sourcedata = metadata["data"].asString();
	}

	if (metadata["executable"].isBool() && metadata["executable"].asBool())
	{
		dl.executable = true;
	}

	if (metadata["symbolic"].isBool() && metadata["symbolic"].asBool())
	{
		dl.symbolic = true;
	}

	return dl;
}
