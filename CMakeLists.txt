cmake_minimum_required(VERSION 3.13 FATAL_ERROR)
if(WIN32)
	cmake_policy(SET CMP0091 NEW)
endif()
project(epicinium)

option(DEVELOPMENT "Create a dev build instead of a release build")
option(CANDIDATE "Create a candidate build instead of a release build")
option(STEAM "Create a Steam build")

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

find_package(OpenGL REQUIRED)

set(CXX_STANDARD 11)
if(WIN32)
	set(CMAKE_MSVC_RUNTIME_LIBRARY MultiThreaded)
endif()

if(MSVC)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

if(WIN32)
	set(ARCHIVE_BIN_PATH ${CMAKE_SOURCE_DIR}/archive/windows64/bin)
elseif(APPLE)
	set(ARCHIVE_BIN_PATH ${CMAKE_SOURCE_DIR}/archive/osx64/bin)
else()
	set(ARCHIVE_BIN_PATH ${CMAKE_SOURCE_DIR}/archive/debian64/bin)
endif()

if(WIN32)
	set(CMAKE_LIBRARY_PATH ${ARCHIVE_BIN_PATH};${CMAKE_SOURCE_DIR}/archive/windows64_lib/)
else()
	set(CMAKE_LIBRARY_PATH ${ARCHIVE_BIN_PATH})
endif()

find_library(CRYPTO_LIBS crypto REQUIRED)
find_library(DISCORD_LIBS discord-rpc REQUIRED)
find_library(GLEW_LIBS glew32 REQUIRED)
find_library(CURL_LIBS curl REQUIRED)
find_library(SSL_LIBS ssl REQUIRED)
find_library(SDL2_LIBS SDL2 REQUIRED)
find_library(SDL2_IMAGE_LIBS SDL2_image REQUIRED)
find_library(SDL2_NET_LIBS SDL2_net REQUIRED)
find_library(SDL2_TTF_LIBS SDL2_ttf REQUIRED)
find_library(Z_LIBS z REQUIRED)

if(STEAM)
	if(WIN32)
		set(STEAM_BIN_PATH ${CMAKE_SOURCE_DIR}/archive/steam/redistributable_bin/win64/)
	elseif(APPLE)
		set(STEAM_BIN_PATH ${CMAKE_SOURCE_DIR}/archive/steam/redistributable_bin/osx/)
	else()
		set(STEAM_BIN_PATH ${CMAKE_SOURCE_DIR}/archive/steam/redistributable_bin/linux64/)
	endif()
	find_library(STEAM_LIBS
		NAMES steam_api steam_api64 REQUIRED
		PATHS ${STEAM_BIN_PATH})
endif()

include_directories(${CMAKE_SOURCE_DIR})
include_directories(src/action src/ai src/audio src/common src/engine)
include_directories(src/graphics src/interface src/logic src/main src/message)
include_directories(src/network src/shaders src/user)
include_directories(libs/ libs/SDL2)

add_executable(game src/main/main.cpp
                    src/main/.cu-menu.cpp
                    src/main/.cu-online.cpp
                    src/main/.cu-discord.cpp
                    src/main/mainmenu.cpp
                    src/main/multiplayermenu.cpp
                    src/main/settingsmenu.cpp
                    src/main/editormenu.cpp
                    src/main/steam.cpp
                    src/main/messageinjector.cpp
                    src/main/hostedgame.cpp
                    src/main/main.cpp
                    src/common/.cu-common.cpp
                    src/common/version.cpp
                    src/common/validation.cpp
                    src/common/dictator.cpp
                    src/logic/.cu-logic.cpp
                    src/logic/.cu-automaton.cpp
                    src/logic/bible.cpp
                    src/ai/.cu-ai.cpp
                    src/network/.cu-network.cpp
                    src/message/.cu-message.cpp
                    src/user/.cu-client.cpp
                    src/engine/.cu-diegetic.cpp
                    src/engine/.cu-engine.cpp
                    src/engine/observer.cpp
                    src/engine/selectorcontext.cpp
                    src/engine/ordercontext.cpp
                    src/engine/mapeditor.cpp
                    src/engine/commander.cpp
                    src/graphics/.cu-graphics.cpp
                    src/graphics/screenshot.cpp
                    src/audio/.cu-audio.cpp
                    src/action/.cu-action.cpp
                    src/action/figure.cpp
                    src/action/surface.cpp
                    src/interface/.cu-interface.cpp
                    libs/SDL2/SDL2_rotozoom.c
                    libs/SDL2/SDL2_gfxPrimitives.c
                    libs/imgui/imgui_draw.cpp
                    libs/imgui/imgui_widgets.cpp
                    libs/imgui/imgui_impl_opengl2.cpp
                    libs/imgui/imgui_sdl.cpp
                    libs/imgui/imgui_impl_sdl.cpp
                    libs/imgui/imgui_stdlib.cpp
                    libs/imgui/imgui.cpp
                    libs/imgui/imgui_demo.cpp
                    libs/tinygettext/iconv.cpp
                    libs/tinygettext/po_parser.cpp
                    libs/tinygettext/unix_file_system.cpp
                    libs/tinygettext/dictionary_manager.cpp
                    libs/tinygettext/plural_forms.cpp
                    libs/tinygettext/language.cpp
                    libs/tinygettext/log.cpp
                    libs/tinygettext/tinygettext.cpp
                    libs/tinygettext/dictionary.cpp
                    libs/stb/stb_impl.c
                    libs/jsoncpp/jsoncpp.cpp)
target_compile_definitions(game PRIVATE RELEASE)
if(WIN32)
	target_compile_definitions(game PRIVATE PLATFORMWINDOWS64)
	target_compile_definitions(game PRIVATE WIN32_LEAN_AND_MEAN)
	target_compile_definitions(game PRIVATE TINYGETTEXT_WITH_SDL)
elseif(APPLE)
	target_compile_definitions(game PRIVATE PLATFORMOSX64)
else()
	target_compile_definitions(game PRIVATE PLATFORMDEBIAN64)
endif()
if(DEVELOPMENT)
	target_compile_definitions(game PRIVATE DEVELOPMENT)
endif()
if(CANDIDATE)
	target_compile_definitions(game PRIVATE CANDIDATE)
endif()
if(STEAM)
	target_compile_definitions(game PRIVATE STEAM_ENABLED=true)
	target_link_libraries(game ${STEAM_LIBS})
endif()
target_link_libraries(game ${CRYPTO_LIBS} ${SSL_LIBS}
                           ${DISCORD_LIBS}
                           ${GLEW_LIBS}
                           ${CURL_LIBS}
                           ${SDL2_LIBS} ${SDL2_IMAGE_LIBS}
                           ${SDL2_NET_LIBS} ${SDL2_TTF_LIBS}
                           ${Z_LIBS})
target_link_libraries(game ${OPENGL_LIBRARIES})
target_link_libraries(game Threads::Threads ${CMAKE_DL_LIBS})
set_target_properties(game
	PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
add_custom_command(TARGET game POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory
		"${ARCHIVE_BIN_PATH}"
		"${CMAKE_BINARY_DIR}/bin")
if(STEAM)
	if(WIN32)
		add_custom_command(TARGET game POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy
				"${STEAM_BIN_PATH}/steam_api64.dll"
				"${CMAKE_BINARY_DIR}/bin")
	elseif(APPLE)
		add_custom_command(TARGET game POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy
				"${STEAM_BIN_PATH}/libsteam_api.dylib"
				"${CMAKE_BINARY_DIR}/bin")
	else()
		add_custom_command(TARGET game POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy
				"${STEAM_BIN_PATH}/libsteam_api.so"
				"${CMAKE_BINARY_DIR}/bin")
	endif()
	add_custom_command(TARGET game POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy
			"${CMAKE_SOURCE_DIR}/steam_appid.txt"
			"${CMAKE_BINARY_DIR}/")
endif()

add_executable(launcher src/build/launcher.c)
target_link_libraries(launcher Shlwapi)
set_target_properties(launcher
	PROPERTIES
	OUTPUT_NAME "epicinium"
)
add_dependencies(launcher game)
