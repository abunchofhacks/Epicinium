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

/*
This pointer should be treated like a unique_ptr<X> (not shared_ptr<X>),
but the pointee can safely set this pointer to null without destroying itself.
*/
template <class X>
class imploding_ptr
{
private:
	std::shared_ptr<X> _pointer;

	template <class Y>
	friend class imploding_ptr;

public:
	imploding_ptr() : _pointer(nullptr) {}

	template <class Y>
	imploding_ptr(imploding_ptr<Y>&& iptr) : _pointer(iptr._pointer) {}

	imploding_ptr(std::unique_ptr<X>&& uptr) : _pointer(uptr) {}

	imploding_ptr(X* ptr) : _pointer(ptr) {}

	imploding_ptr(std::nullptr_t) : _pointer(nullptr) {}

	template <class Y>
	imploding_ptr& operator=(imploding_ptr<Y>&& iptr)
	{
		_pointer = std::move(iptr._pointer);
		return *this;
	}

	imploding_ptr& operator=(std::unique_ptr<X>&& uptr)
	{
		_pointer = std::move(uptr);
		return *this;
	}

	imploding_ptr& operator=(X* ptr)
	{
		_pointer.reset(ptr);
		return *this;
	}

	imploding_ptr& operator=(std::nullptr_t)
	{
		_pointer.reset();
		return *this;
	}

	bool operator==(std::nullptr_t)
	{
		return (_pointer == nullptr);
	}

	bool operator!=(std::nullptr_t)
	{
		return (_pointer != nullptr);
	}

	explicit operator bool() const
	{
		return (_pointer != nullptr);
	}

	std::shared_ptr<X> lock() const
	{
		return _pointer;
	}

	std::weak_ptr<X> remember() const
	{
		return _pointer;
	}
};
