// tinygettext - A gettext replacement that works directly on .po files
// Copyright (c) 2009 Ingo Ruhnke <grumbel@gmail.com>
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgement in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

/*
 *   This file was modified in October 2020 by Sander in 't Veld
 *   (sander@abunchofhacks.coop):
 *   1. added support for compilers where filesystem is still experimental;
 *   2. replaced stem() with filename() because the file extension is needed.
 */

#include "tinygettext/unix_file_system.hpp"

#if defined __has_include
#   if __has_include(<filesystem>) && ((__cplusplus >= 201703L) || (defined(_MSVC_LANG) && (_MSVC_LANG >= 201703L) && (_MSC_VER >= 1913)))
#     include <filesystem>
namespace fs = std::filesystem;
#   else
#     define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#     include <experimental/filesystem>
      namespace fs = std::experimental::filesystem;
#   endif
#else
#   include <filesystem>
    namespace fs = std::filesystem;
#endif

#include <fstream>
#include <stdlib.h>

namespace tinygettext {

UnixFileSystem::UnixFileSystem()
{
}

std::vector<std::string>
UnixFileSystem::open_directory(const std::string& pathname)
{
  std::vector<std::string> files;
  for(auto const& p : fs::directory_iterator(pathname))
  {
    files.push_back(p.path().filename().string());
  }
  return files;
}

std::unique_ptr<std::istream>
UnixFileSystem::open_file(const std::string& filename)
{
  return std::unique_ptr<std::istream>(new std::ifstream(filename));
}

} // namespace tinygettext

/* EOF */
