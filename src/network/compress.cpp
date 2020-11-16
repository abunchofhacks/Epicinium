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
#include "compress.hpp"
#include "source.hpp"

#include "libs/zlib/zlib.h"

#include "system.hpp"


bool Compress::iszippable(const std::string& filepath)
{
	size_t dotpos = filepath.find_last_of('.');

	// Binary files are zippable.
	if (dotpos == std::string::npos) return true;

	std::string extension = filepath.substr(dotpos + 1);
	if (extension.empty())
	{
		LOGW << "No extension in filepath '" << filepath << "'";
		DEBUG_ASSERT(false);
		return false;
	}

	// PNG files are already compressed.
	if (extension == "png") return false;

	// Uncompressed audio formats are zippable.
	if (extension == "wav") return true;

	// Text files are highly zippable.
	if (extension == "txt") return true;
	if (extension == "md") return true;

	// Json-based files are zippable.
	if (extension == "json") return true;
	if (extension == "log") return true;
	if (extension == "list") return true;
	if (extension == "map") return true;

	// Binary files are zippable.
	if (extension == "dll") return true;
	if (extension == "dylib") return true;
	if (extension == "exe") return true;
	if (extension == "so") return true;
	if (extension.find_first_not_of("0123456789") == std::string::npos)
	{
		return true;
	}

	// Certificates? Sure, why not.
	if (extension == "crt") return true;

	// Apparently yes, Femtozip models are zippable.
	if (extension == "fzm") return true;

	// Most files are zippable but lets err on the side of caution.
	LOGW << "Unknown file format '" << extension << "'"
		" in filepath '" << filepath << "'";
	DEBUG_ASSERT(false);
	return false;
}

bool Compress::isgzipped(const std::string& filepath)
{
	return (filepath.compare(filepath.size() - 3, 3, ".gz") == 0);
}

std::string Compress::gzipfilename(const std::string& filepath)
{
	return filepath + ".gz";
}

std::string Compress::gzip(const std::string& filepath)
{
	return gzip(filepath, gzipfilename(filepath));
}

std::string Compress::gzip(const std::string& filepath,
	const std::string& filepathOut)
{
	std::ifstream file = System::ifstream(filepath,
		std::ios::binary | std::ios::ate);
	size_t filesize = file.tellg();
	file.seekg(0, std::ios::beg);

#ifdef PLATFORMUNIX
	gzFile out = gzopen(filepathOut.c_str(), "wb9");
#else
	std::wstring wfilename = System::utf16FromUtf8(filepathOut);
	gzFile out = gzopen_w(wfilename.c_str(), "wb9");
#endif
	if (out == nullptr)
	{
		LOGE << "Failed to open output file " << filepathOut;
		int errornumber;
		LOGE << gzerror(out, &errornumber);
		LOGE << errornumber;
		DEBUG_ASSERT(false);
		return "";
	}

	constexpr size_t BUFFERSIZE = 65535;
	std::array<char, BUFFERSIZE> buffer;

	for (size_t offset = 0; offset < filesize; offset += BUFFERSIZE)
	{
		size_t size = std::min(filesize - offset, BUFFERSIZE);
		if (!file.read(buffer.data(), size))
		{
			LOGE << "Failed to open input file " << filepath;
			DEBUG_ASSERT(false);
			return "";
		}

		if (!gzwrite(out, buffer.data(), size))
		{
			LOGE << "Error writing output file " << filepathOut;
			int errornumber;
			LOGE << gzerror(out, &errornumber);
			LOGE << errornumber;
			DEBUG_ASSERT(false);
			return "";
		}
	}

	int errornumber = gzclose(out);
	if (errornumber != Z_OK)
	{
		LOGE << "Error writing output file " << filepathOut;
		LOGE << errornumber;
		DEBUG_ASSERT(false);
		return "";
	}

	return filepathOut;
}

std::string Compress::gunzip(const std::string& filepath)
{
	if (!isgzipped(filepath))
	{
		LOGE << "Input file " << filepath << " is not gzipped";
		DEBUG_ASSERT(false);
		return "";
	}

	return gunzip(filepath, filepath.substr(0, filepath.size() - 3));
}

std::string Compress::gunzip(const std::string& filepath,
	const std::string& filepathOut)
{
	std::ofstream file = System::ofstream(filepathOut,
		std::ios::binary | std::ios::trunc);
	if (!file)
	{
		LOGE << "Failed to open output file " << filepathOut;
		DEBUG_ASSERT(false);
		return "";
	}

	constexpr size_t BUFFERSIZE = 65535;
	std::array<char, BUFFERSIZE> buffer;

#ifdef PLATFORMUNIX
	gzFile in = gzopen(filepath.c_str(), "rb");
#else
	std::wstring wfilename = System::utf16FromUtf8(filepath);
	gzFile in = gzopen_w(wfilename.c_str(), "rb");
#endif
	if (in == nullptr)
	{
		LOGE << "Failed to open input file " << filepath;
		int errornumber;
		LOGE << gzerror(in, &errornumber);
		LOGE << errornumber;
		DEBUG_ASSERT(false);
		return "";
	}

	do
	{
		int size = gzread(in, buffer.data(), BUFFERSIZE);
		if (size < 0)
		{
			LOGE << "Error reading input file " << filepath;
			int errornumber;
			LOGE << gzerror(in, &errornumber);
			LOGE << errornumber;
			LOGE << "Read " << size;
			DEBUG_ASSERT(false);
			return "";
		}

		if (!file.write(buffer.data(), size))
		{
			LOGE << "Failed to write to output file " << filepathOut;
			DEBUG_ASSERT(false);
			return "";
		}
	}
	while (!gzeof(in));

	int errornumber = gzclose(in);
	if (errornumber != Z_OK)
	{
		LOGE << "Error closing input file " << filepath;
		LOGE << errornumber;
		DEBUG_ASSERT(false);
		return "";
	}

	return filepathOut;
}
