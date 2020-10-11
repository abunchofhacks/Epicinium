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

#include <dlfcn.h>


template <class T>
class LibLoader
{
private:
	std::string _pathToLib;
	void* _handle;
	void* _alloc;

public:
	LibLoader(LibLoader&& that) :
		_pathToLib(that._pathToLib),
		_handle(that._handle),
		_alloc(that._alloc)
	{
		that._handle = nullptr;
		that._alloc = nullptr;
	}

	LibLoader(const std::string& path) :
		_pathToLib(path),
		_handle(dlopen(_pathToLib.c_str(), RTLD_LAZY | RTLD_DEEPBIND)),
		_alloc(_handle ? dlsym(_handle, "allocate") : nullptr)
	{
		if (_handle == nullptr)
		{
			LOGE << "Failed to load library '" << _pathToLib << "': "
				<< dlerror();
		}
		// Check dlsym errors in opposite order; the errors overwrite eachother.
		else if (_alloc == nullptr)
		{
			LOGE << "Failed to load 'allocate'"
				<< " from library '" << _pathToLib << "': "
				<< dlerror();
			if (!dlclose(_handle))
			{
				LOGE << "Failed to unload library '" << _pathToLib << "': "
					<< dlerror();
			}
		}
		else
		{
			LOGI << "Library '" << _pathToLib << "' loaded";

			void* setup = dlsym(_handle, "setup");
			if (setup)
			{
				auto setupFunc = (void(*)(int, const char* const [])) setup;
				std::string arg_logname = "--logname=" + ::sanitize(_pathToLib);
				const char* const args[] = {
					_pathToLib.c_str(), // like main(), first arg is path
					arg_logname.c_str(),
					"--logrollback=2",
				};
				setupFunc(array_size(args), args);

				LOGI << "Library '" << _pathToLib << "' set up";
			}
		}
	}

	LibLoader(const LibLoader&) = delete;
	LibLoader& operator=(const LibLoader&) = delete;
	LibLoader& operator=(LibLoader&&) = delete;

	~LibLoader()
	{
		if (_handle != nullptr)
		{
			if (dlclose(_handle))
			{
				LOGI << "Library '" << _pathToLib << "' unloaded";
			}
			else
			{
				LOGE << "Failed to unload library '" << _pathToLib << "': "
					<< dlerror();
			}
		}
	}

public:
	explicit operator bool() const
	{
		return (_handle != nullptr);
	}

	const std::string& pathToLib() const
	{
		return _pathToLib;
	}

	template <typename ...As>
	T* instantiate(As... args)
	{
		if (!_alloc) return nullptr;

		auto allocFunc = (T*(*)(As...)) _alloc;
		return allocFunc(args...);
	}
};
