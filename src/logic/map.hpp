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


enum class PoolType : uint8_t
{
	NONE = 0,
	MULTIPLAYER,
	CUSTOM,
	DIORAMA,
};

constexpr size_t POOLTYPE_SIZE = ((size_t) PoolType::DIORAMA) + 1;

const char* stringify(const PoolType& type);
PoolType parsePoolType(const std::string& str);
std::ostream& operator<<(std::ostream& os, const PoolType& type);

class Map
{
public:
	struct ExternalItem
	{
		std::string uniqueTag;
		std::string quotedName;
		std::string sourceFilename;
		Json::Value metadata;
	};

private:
	static std::string _resourcemapsfolder;
	static std::string _authoredmapsfolder;
	static std::vector<ExternalItem> _cachedexternalitems;

public:
	static void setResourceRoot(const std::string& root);
	static void setAuthoredRoot(const std::string& root);

	static std::string readOnlyFilename(const std::string& name);
	static std::string authoredFilename(const std::string& name);

	static Json::Value loadMetadata(const std::string& name);

	static bool exists(const std::string& name);

	static const std::vector<std::string>& pool();
	static const std::vector<std::string>& customPool();
	static const std::vector<std::string>& userPool();
	static const std::vector<std::string>& hiddenTutorialPool();
	static const std::vector<std::string>& hiddenChallengePool();
	static const std::vector<std::string>& hiddenDioramaPool();

	static std::string DIORAMA_MAPNAME;

	static std::vector<std::string> listAuthored();

	static void listExternalItem(ExternalItem&& item);
	static void unlistExternalItem(const std::string& uniqueTag);

	static const std::vector<ExternalItem>& externalItems();
};
