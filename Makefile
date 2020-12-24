#
# Part of Epicinium
# developed by A Bunch of Hacks.
#
# Copyright (c) 2017-2020 A Bunch of Hacks
#
# Epicinium is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Epicinium is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.
#
# [authors:]
# Sander in 't Veld (sander@abunchofhacks.coop)
# Daan Mulder (daan@abunchofhacks.coop)
#

# Make flag
MAKEFLAGS = --warn-undefined-variable

# Tricks
EMPTY :=
SPACE := $(EMPTY) #
COMMA := ,

# Per-repository config. Variables defined with ?= can be configured.
CUSTOMFILE = ./.custom.make
ifeq ($(wildcard $(CUSTOMFILE)),) # does not exist
# Nothing to do.
else
include $(CUSTOMFILE)
endif

# Per-device config. Variables defined with ?= can be configured.
CONFIGFILE = ./.config.make
ifeq ($(wildcard $(CONFIGFILE)),) # does not exist
# Nothing to do.
else
include $(CONFIGFILE)
endif

# Detect OS
ifeq ($(OS),Windows_NT)
detected_OS := Windows
else
detected_OS := $(shell sh -c 'uname -s 2>/dev/null || echo Unknown')
endif

# Detect shell
ifeq ($(findstring cmd.exe,$(SHELL)),cmd.exe)
detected_shell = cmd
else
detected_shell = sh
endif

# override me from CONFIGFILE or from the command line (cmd/sh)
USED_SHELL ?= $(detected_shell)
#

# Detect architecture (32-bit or 64-bit)
ifeq ($(detected_OS),Windows)
ifeq ($(PROCESSOR_ARCHITEW6432),AMD64)
ARCH       := 64
else
ifeq ($(PROCESSOR_ARCHITECTURE),AMD64)
ARCH       := 64
else
ARCH       := 32
endif
endif
else
ARCH       := $(shell getconf LONG_BIT)
endif

# Platform (os + arch)
ifeq ($(detected_OS),Windows)
PLATFORM   := windows$(ARCH)
else
ifeq ($(detected_OS),Darwin) # Mac OS X
PLATFORM   := osx$(ARCH)
else
PLATFORM   := debian$(ARCH)
endif
endif

# Distribution name
GAMENAME    = epicinium

# Version number
VERSIONFILE = ./bin/version

# Release, candidate or development?
VARIANTFILE = ./bin/variant
ifeq ($(wildcard $(VARIANTFILE)),) # does not exist
VARIANT    := release
else
VARIANT    := $(strip $(shell cat $(VARIANTFILE)))
endif

# Optimization level
PERFANTFILE = ./bin/perfant
ifeq ($(wildcard $(PERFANTFILE)),) # does not exist
ifeq ($(VARIANT),server)
PERFANT    := native
else
ifeq ($(VARIANT),dev)
PERFANT    := dev
else
PERFANT    := perf
endif
endif
else
PERFANT   := $(strip $(shell cat $(PERFANTFILE)))
endif

# Target platform
TARGETFILE  = ./bin/target
ifeq ($(wildcard $(TARGETFILE)),) # does not exist
TARGET     := $(PLATFORM)
else
TARGET     := $(strip $(shell cat $(TARGETFILE)))
endif

# Object files should be recompiled when these files change
RECOMPILERS = $(VARIANTFILE) $(PERFANTFILE) $(TARGETFILE)

# Compiler & Flags
CXX        ?= g++
CC         ?= gcc
AR         ?= ar

ifeq ($(detected_OS),Windows)
CXXVGE7     =
else
CXXVGE7    := $(shell expr `$(CXX) -dumpversion | cut -f1 -d.` \>= 7)
endif

CFLAGS      = $(CFLAG_OPT) $(CFLAG_SYM) $(WFLAGS) -pthread $(DEFINES)
CFLAGS_CC   = $(CFLAG_OPT) $(CFLAG_SYM) $(WFLAGS_C) $(DEFINES)
CFLAGS_LIB  = $(CFLAG_OPT) $(CFLAG_SYM) $(DEFINES_LIB)

CFLAG_ONAT ?= -march=native -O3
CFLAG_ODEV ?= -Og
CFLAG_OREL ?= -O3

ifeq ($(PERFANT),native)
CFLAG_OPT   = $(CFLAG_ONAT)
else
ifeq ($(PERFANT),dev)
CFLAG_OPT   = $(CFLAG_ODEV)
else
CFLAG_OPT   = $(CFLAG_OREL)
endif
endif

ifeq ($(VARIANT),release)
ifeq ($(detected_OS),Darwin) # Mac OS X
ifeq ($(ENABLE_STEAM),yes)
CFLAG_SYM  ?= -mmacosx-version-min=10.9
else
CFLAG_SYM  ?=
endif
else
CFLAG_SYM  ?= -s
endif
else
CFLAG_SYM  ?= -g
endif

WFLAGS      = $(WFLAGS_W) $(WFLAGS_CONFIG) $(WFLAGS_ADDED) $(WFLAGS_E)
WFLAGS_C    = $(WFLAGS_W) $(WFLAGS_E)

WFLAGS_W   ?= -pedantic -pedantic-errors -Wall -Wextra

# configure me from CONFIGFILE
WFLAGS_CONFIG ?=
#

# override me from the command line
WFLAGS_ADDED =
#

#              -Wnon-virtual-dtor -Wshadow=local\
#              -Wuninitialized -Wlogical-op\
#              -Wmissing-declarations -Wredundant-decls\
#              -Wduplicated-cond\
#              -Wdisabled-optimization\
#              -Wframe-larger-than=1048576\
#              -Wcast-qual -Wcast-align -Wuseless-cast -Wconversion\
#              -Wdouble-promotion -Wfloat-equal\
#              -Wpadded\
#              -Wsuggest-override\
#              -Wsuggest-final-types -Wsuggest-final-methods\
#              -Wduplicated-branches\
#              -Walloc-zero -Walloc-size-larger-than=1048576\
#              -Wlarger-than=1048576 -Wvla-larger-than=1048576\

WFLAGS_E   ?=

# override me from CONFIGFILE or from the command line (yes/no)
ENABLE_STEAM ?= no
ENABLE_FEMTOZIP ?= no
#

DEFINES     = $(DEFINES_PFM) $(DEFINES_DEV)\
              $(DEFINES_CUSTOM) $(DEFINES_CONFIG)\
              $(DEFINES_STM) $(DEFINES_FZIP)\
              $(DEFINES_PIC) $(DEFINES_LIC)\
              $(DEFINES_ADDED)

ifeq ($(TARGET),debian32)
DEFINES_PFM = -DPLATFORMDEBIAN32
else
ifeq ($(TARGET),debian64)
DEFINES_PFM = -DPLATFORMDEBIAN64
else
ifeq ($(TARGET),windows32)
DEFINES_PFM = -DPLATFORMWINDOWS32 -DWIN32_LEAN_AND_MEAN
else
ifeq ($(TARGET),windows64)
DEFINES_PFM = -DPLATFORMWINDOWS64 -DWIN32_LEAN_AND_MEAN
else
ifeq ($(TARGET),osx32)
DEFINES_PFM = -DPLATFORMOSX32
else
ifeq ($(TARGET),osx64)
DEFINES_PFM = -DPLATFORMOSX64
endif
endif
endif
endif
endif
endif

ifeq ($(VARIANT),server)
DEFINES_DEV = -DRELEASESERVER
else
ifeq ($(VARIANT),release)
DEFINES_DEV =
else
ifeq ($(VARIANT),candidate)
DEFINES_DEV = -DDEVELOPMENT -DCANDIDATE
else
DEFINES_DEV = -DDEVELOPMENT
endif
endif
endif

# configure me from CUSTOMFILE
DEFINES_CUSTOM ?=
#

# configure me from CONFIGFILE
DEFINES_CONFIG ?=
#

ifeq ($(ENABLE_STEAM),yes)
DEFINES_STM = -DSTEAM_ENABLED=true
else
DEFINES_STM =
endif

ifeq ($(ENABLE_FEMTOZIP),yes)
DEFINES_FZIP = -DFEMTOZIP_ENABLED=true
else
DEFINES_FZIP =
endif

DEFINES_PIC              =
.obj/.pic/%: DEFINES_PIC = -DINTL_ENABLED=false
.obj/.lic/%: DEFINES_PIC = -DINTL_ENABLED=false

DEFINES_LIC              =
.obj/.lic/%: DEFINES_LIC = -DSTATIC_WRITER_ENABLED=false\
                           -DLOG_REPLACE_WITH_CALLBACK_ENABLED=true

# override me from the command line
DEFINES_ADDED =
#

DEFINES_LIB =
ifeq ($(detected_OS),Windows)
.obj/libs/tinygettext/%: DEFINES_LIB = -DTINYGETTEXT_WITH_SDL
endif

DFLAGS      = -MT .obj/$*.o -MMD -MP -MF .dep/$*.Td

STANDARD   ?= -std=c++11
STANDARD_C ?= -std=c99

.obj/libs/tinygettext/%: STANDARD = -std=c++17

ifeq ($(TARGET),$(PLATFORM))
MFLAGS      =
else
MFLAGS      = -m32
endif

# configure me from CONFIGFILE
IFLAGS_CONFIG ?=
#

IFLAGS_FULL = $(IFLAGS_ROOT) $(IFLAGS_CORE) $(IFLAGS_CONFIG) $(IFLAGS_LIB)
IFLAGS      = $(IFLAGS_ROOT) $(IFLAGS_CONFIG) $(IFLAGS_LIB)

ifeq ($(detected_OS),Windows)
IFLAGS_ROOT = -I .
else
IFLAGS_ROOT = -I ./
endif

IFLAGS_CORE = $(foreach d,$(wildcard src/*/),-I $d)

IFLAGS_LIB =
.obj/libs/tinygettext/%: IFLAGS_LIB = -I libs -I libs/SDL2

# configure me from CONFIGFILE
LFLAGS_CONFIG ?=
#

# override me from the command line
LFLAGS_ADDED =
#

LFLAGS_LNCH = $(LFLAGS_CONS)
LFLAGS_LAST = $(CFLAG_OPT) $(LFLAGS_CONFIG) $(LFLAGS_ADDED) -lz -lm -pthread

%:     LPATH = $(LFLAGS_LBIN)
bin/%: LPATH = $(LFLAGS_LDOT)

ifeq ($(detected_OS),Windows)
LFLAGS_LDOT =
LFLAGS_LBIN =
LFLAGS_CPP  = -lstdc++
LFLAGS_CONS = -lpsapi -lshlwapi
LFLAGS_AINT =
LFLAGS_SDL  = -lSDL2main -lSDL2 -lSDL2_net -lSDL2_image -lSDL2_ttf
LFLAGS_SDLN = -lSDL2main -lSDL2 -lSDL2_net
LFLAGS_GL   = -lglew32 -lopengl32
LFLAGS_SSL  = -lssl -lcrypto
LFLAGS_CURL = -lcurl
LFLAGS_DISC = -ldiscord-rpc
LFLAGS_INTL = -lSDL2
else
ifeq ($(detected_OS),Darwin) # Mac OS X
LFLAGS_LDOT = -Wl,-rpath,@executable_path/ -Lbin
LFLAGS_LBIN = -Wl,-rpath,@executable_path/bin -Lbin
LFLAGS_CPP  = -lstdc++
LFLAGS_CONS =
LFLAGS_AINT = -ldl
LFLAGS_SDL  = -lfreetype -lpng\
              -lSDL2_image -lSDL2_net -lSDL2_ttf -lSDL2
LFLAGS_SDLN = -lSDL2_net -lSDL2
LFLAGS_GL   = -lGLEW -framework OpenGL
LFLAGS_SSL  = -lssl -lcrypto
LFLAGS_CURL = -lcurl
LFLAGS_DISC = -ldiscord-rpc
LFLAGS_INTL = -lstdc++fs
else
LFLAGS_LDOT = -Wl,-rpath,'$$ORIGIN'
LFLAGS_LBIN = -Wl,-rpath,'$$ORIGIN/bin'
LFLAGS_CPP  = -lstdc++
LFLAGS_CONS =
LFLAGS_AINT = -ldl
LFLAGS_SDL  = -Wl,--no-as-needed\
              bin/libfreetype.so bin/libpng16.so bin/libz.so\
              -Wl,--as-needed\
              bin/libSDL2_image.so bin/libSDL2_net.so bin/libSDL2_ttf.so\
              bin/libSDL2.so
LFLAGS_SDLN = -Wl,--no-as-needed\
              bin/libz.so\
              -Wl,--as-needed\
              bin/libSDL2_net.so bin/libSDL2.so
LFLAGS_GL   = bin/libGLEW.so -lGL
LFLAGS_SSL  = bin/libssl.so bin/libcrypto.so
LFLAGS_CURL = bin/libcurl.so
LFLAGS_DISC = bin/libdiscord-rpc.so
LFLAGS_INTL = -lstdc++fs
endif
endif

ifeq ($(ENABLE_FEMTOZIP),yes)
LFLAGS_FZIP = $(LFLAGS_FEMTOZIP_REAL)
else
LFLAGS_FZIP ?=
endif

ifeq ($(detected_OS),Windows)
LFLAGS_FEMTOZIP_REAL = -lfzip
else
ifeq ($(detected_OS),Darwin) # Mac OS X
LFLAGS_FEMTOZIP_REAL = -lfzip
else
LFLAGS_FEMTOZIP_REAL = bin/libfzip.so
endif
endif

ifeq ($(ENABLE_STEAM),yes)
LFLAGS_STM = $(LFLAGS_STEAM_REAL)
STM_DEP = $(STEAM_REAL_DEPENDENCIES)
else
LFLAGS_STM ?=
STM_DEP ?=
endif

ifeq ($(TARGET),windows64)
LFLAGS_STEAM_REAL = -lsteam_api64
STEAM_REAL_DEPENDENCIES = bin/steam_api64.lib bin/steam_api64.dll
STEAM_ARCHIVE_REDIS_DIR = archive/steam/redistributable_bin/win64
endif
ifeq ($(TARGET),windows32)
LFLAGS_STEAM_REAL = -lsteam_api
STEAM_REAL_DEPENDENCIES = bin/steam_api.lib bin/steam_api.dll
STEAM_ARCHIVE_REDIS_DIR = archive/steam/redistributable_bin
endif
ifeq ($(TARGET),osx64)
LFLAGS_STEAM_REAL = -lsteam_api -mmacosx-version-min=10.9
STEAM_REAL_DEPENDENCIES = bin/libsteam_api.dylib
STEAM_ARCHIVE_REDIS_DIR = archive/steam/redistributable_bin/osx
endif
ifeq ($(TARGET),debian64)
LFLAGS_STEAM_REAL = bin/libsteam_api.so
STEAM_REAL_DEPENDENCIES = bin/libsteam_api.so
STEAM_ARCHIVE_REDIS_DIR = archive/steam/redistributable_bin/linux64
endif
ifeq ($(TARGET),debian32)
LFLAGS_STEAM_REAL = bin/libsteam_api.so
STEAM_REAL_DEPENDENCIES = bin/libsteam_api.so
STEAM_ARCHIVE_REDIS_DIR = archive/steam/redistributable_bin/linux32
endif

ifeq ($(detected_OS),Darwin) # Mac OS X
FIX_LIBSTD_DYLIB = install_name_tool -change\
                   "/usr/local/opt/gcc@4.8/lib/gcc/4.8/libstdc++.6.dylib"\
                   "@rpath/libstdc++.dylib"
FIX_LIBGCC_DYLIB = install_name_tool -change\
                   "/usr/local/lib/gcc/4.8/libgcc_s.1.dylib"\
                   "@rpath/libgcc_s.dylib"
endif

# Bin folder
SUBDIRS     = $(wildcard src/*) $(wildcard libs/*)
DEPDIRFIXES = $(patsubst %,.dep/%/.dir_fix,$(SUBDIRS))
OBJDIRFIXES = $(patsubst %,.obj/%/.dir_fix,$(SUBDIRS))
PICDIRFIXES = .pic/.dir_fix\
              $(patsubst %,.obj/.pic/%/.dir_fix,$(SUBDIRS))\
              $(patsubst %,.dep/.pic/%/.dir_fix,$(SUBDIRS))
LICDIRFIXES = .lic/.dir_fix\
              $(patsubst %,.obj/.lic/%/.dir_fix,$(wildcard src/*))\
              $(patsubst %,.dep/.lic/%/.dir_fix,$(wildcard src/*))
DIRFIXES    = $(DEPDIRFIXES) $(OBJDIRFIXES) $(PICDIRFIXES) $(LICDIRFIXES)

# Compiling
C_CXX       = $(CXX) $(MFLAGS) $(STANDARD) $(DFLAGS)
C_CC        = $(CC) $(MFLAGS) $(STANDARD_C) $(DFLAGS)
.obj/%:           COMPILE_OBJ = $(C_CXX) $(CFLAGS) $(IFLAGS_FULL)
.obj/libs/%:      COMPILE_OBJ = $(C_CXX) $(CFLAGS_LIB) $(IFLAGS)
.obj/%:           COMPILE_COB = $(C_CC) $(CFLAGS_CC) $(IFLAGS)
.obj/libs/%:      COMPILE_COB = $(C_CC) $(CFLAGS_LIB) $(IFLAGS)
.obj/.pic/%:      COMPILE_OBJ = $(C_CXX) $(FPIC) $(CFLAGS) $(IFLAGS_FULL)
.obj/.pic/libs/%: COMPILE_OBJ = $(C_CXX) $(FPIC) $(CFLAGS_LIB) $(IFLAGS)
.obj/.pic/%:      COMPILE_COB = $(C_CC) $(FPIC) $(CFLAGS_CC) $(IFLAGS)
.obj/.pic/libs/%: COMPILE_COB = $(C_CC) $(FPIC) $(CFLAGS_LIB) $(IFLAGS)
.obj/.lic/%:      COMPILE_OBJ = $(C_CXX) $(FPIC) $(CFLAGS) $(IFLAGS_FULL)
.obj/.lic/%:      COMPILE_COB = $(C_CC) $(FPIC) $(CFLAGS_CC) $(IFLAGS)
COMPILE_BIN = $(C_CXX)
COMPILE_CBN = $(C_CC)
COMPILE_ALB = $(AR) rvs
COMPILE_AIL = $(C_CXX) -shared -nodefaultlibs $(FPIC) -Wl,--gc-sections
FPIC        = $(REAL_FPIC) -fvisibility=hidden -fvisibility-inlines-hidden\
              -ffunction-sections -fdata-sections
ifeq ($(detected_OS),Windows)
REAL_FPIC   =
else
REAL_FPIC   = -fPIC
endif
POSTCOMPILE = mv -f .dep/$*.Td .dep/$*.d

# Headers and source files
OUR_HEADERS = $(wildcard src/*/*.hpp)
OUR_SOURCES = $(wildcard src/*/*.cpp) $(wildcard src/*/.*.cpp)

# Determine Compilation Unit Elements
CU_ELEMENTS       := $(shell grep -HoE "^\#include \"[a-z.]+\""\
                     $(wildcard src/*/.cu-*.cpp)\
                     | sed -e "s/.cu-.*:\#include \"//" -e "s/\"//")


# override me from the command line (yes/no)
ENABLE_CU  ?= yes
#

# Determine source files
ifeq ($(ENABLE_CU),yes)
listsrc     = $(wildcard src/$(1)/.cu-*.cpp)\
              $(filter-out $(CU_ELEMENTS),$(wildcard src/$(1)/*.cpp))
else
listsrc     = $(wildcard src/$(1)/*.cpp)
endif

# Source
CMON_SRC    = $(call listsrc,common)
LGIC_SRC    = $(call listsrc,logic)
AINT_SRC    = $(call listsrc,ai)
NETW_SRC    = $(call listsrc,network)
MESG_SRC    = $(call listsrc,message)
USER_SRC    = $(call listsrc,user)
GRFX_SRC    = $(call listsrc,graphics)
ACTN_SRC    = $(call listsrc,action)
AUDI_SRC    = $(call listsrc,audio)
INFC_SRC    = $(call listsrc,interface)
ENGI_SRC    = $(call listsrc,engine)
JSON_SRC    = $(wildcard libs/jsoncpp/*.cpp)
IMGU_SRC    = $(wildcard libs/imgui/*.cpp)
SDL_SRC     = $(wildcard libs/SDL2/*.c)
INTL_SRC    = $(wildcard libs/tinygettext/*.cpp)
MAIN_SRC    = $(call listsrc,main)
SERV_SRC    = $(call listsrc,server)
EDIT_SRC    = $(call listsrc,edit)
ESSA_SRC    = $(call listsrc,essai)
TOOL_SRC    = $(wildcard src/build/*.cpp)
LNCH_SRC    = src/build/launcher.c

# Build targets are composed of parts
PARTS       = LNCH MAIN SERV EDIT ESSA\
              CMON LGIC AINT NETW MESG USER GRFX ACTN AUDI INFC ENGI\
              JSON SDL SLDN IMGU GL SSL CURL FZIP DISC INTL\
              CORE UNET ENGN LAST

CPP_PARTS   = MAIN SERV EDIT ESSA\
              CMON LGIC AINT NETW MESG USER GRFX ACTN AUDI INFC ENGI\
              JSON IMGU INTL\
              TOOL
C_PARTS     = LNCH SDL
COMP_PARTS  = CORE UNET ENGN

CORE_ELEMS  = CMON LGIC JSON
UNET_ELEMS  = NETW MESG USER
ENGN_ELEMS  = ENGI GRFX AUDI ACTN INFC

SRC_PARTS   = $(CPP_PARTS) $(C_PARTS)\
              $(CPP_PARTS:%=%_PIC) $(C_PARTS:%=%_PIC)\
              $(CPP_PARTS:%=%_LIC) $(C_PARTS:%=%_LIC)

# Flags
$(foreach part,$(PARTS),\
	$(eval LFLAGS_$(part)_PIC = $(LFLAGS_$(part))))
$(foreach part,$(PARTS),\
	$(eval LFLAGS_$(part)_LIC = $(LFLAGS_$(part))))

# Objects
$(foreach part,$(CPP_PARTS),\
	$(eval $(part)_OBJ = $(patsubst %.cpp,.obj/%.o,$($(part)_SRC))))
$(foreach part,$(C_PARTS),\
	$(eval $(part)_OBJ = $(patsubst %.c,.obj/%.o,$($(part)_SRC))))
$(foreach part,$(CPP_PARTS),\
	$(eval $(part)_PIC_OBJ = $($(part)_OBJ:.obj/%.o=.obj/.pic/%.o)))
$(foreach part,$(C_PARTS),\
	$(eval $(part)_PIC_OBJ = $($(part)_OBJ:.obj/%.o=.obj/.pic/%.o)))
$(foreach part,$(CPP_PARTS),\
	$(eval $(part)_LIC_OBJ = $($(part)_OBJ:.obj/%.o=.obj/.lic/%.o)))
$(foreach part,$(C_PARTS),\
	$(eval $(part)_LIC_OBJ = $($(part)_OBJ:.obj/%.o=.obj/.lic/%.o)))
$(foreach part,$(COMP_PARTS),\
	$(eval $(part)_OBJ = $(foreach elem,$($(part)_ELEMS),$($(elem)_OBJ))))
ALL_OBJ      = $(foreach elem,$(SRC_PARTS),$($(elem)_OBJ))

# Dependencies
$(foreach part,$(CPP_PARTS),\
	$(eval $(part)_DEP = $(patsubst %.cpp,.dep/%.d,$($(part)_SRC))))
$(foreach part,$(C_PARTS),\
	$(eval $(part)_DEP = $(patsubst %.c,.dep/%.d,$($(part)_SRC))))
$(foreach part,$(CPP_PARTS),\
	$(eval $(part)_PIC_DEP = $($(part)_DEP:.dep/%.d=.dep/.pic/%.d)))
$(foreach part,$(C_PARTS),\
	$(eval $(part)_PIC_DEP = $($(part)_DEP:.dep/%.d=.dep/.pic/%.d)))
$(foreach part,$(CPP_PARTS),\
	$(eval $(part)_LIC_DEP = $($(part)_DEP:.dep/%.d=.dep/.lic/%.d)))
$(foreach part,$(C_PARTS),\
	$(eval $(part)_LIC_DEP = $($(part)_DEP:.dep/%.d=.dep/.lic/%.d)))
$(foreach part,$(COMP_PARTS),\
	$(eval $(part)_DEP = $(foreach elem,$($(part)_ELEMS),$($(elem)_DEP))))
ALL_DEP      = $(foreach elem,$(SRC_PARTS),$($(elem)_DEP))
ALL_INCL_DEP = $(wildcard .dep/*.d) $(wildcard .dep/.*.d)\
               $(wildcard .dep/*/*.d) $(wildcard .dep/*/.*.d)\
               $(wildcard .dep/*/*/*.d) $(wildcard .dep/*/*/.*.d)\
               $(wildcard .dep/*/*/*/*.d) $(wildcard .dep/*/*/*/.*.d)\
               $(wildcard .dep/.pic/*.d) $(wildcard .dep/.pic/.*.d)\
               $(wildcard .dep/.pic/*/*.d) $(wildcard .dep/.pic/*/.*.d)\
               $(wildcard .dep/.pic/*/*/*.d) $(wildcard .dep/.pic/*/*/.*.d)\
               $(wildcard .dep/.pic/*/*/*/*.d) $(wildcard .dep/.pic/*/*/*/.*.d)\
               $(wildcard .dep/.lic/*.d) $(wildcard .dep/.lic/.*.d)\
               $(wildcard .dep/.lic/*/*.d) $(wildcard .dep/.lic/*/.*.d)\
               $(wildcard .dep/.lic/*/*/*.d) $(wildcard .dep/.lic/*/*/.*.d)\
               $(wildcard .dep/.lic/*/*/*/*.d) $(wildcard .dep/.lic/*/*/*/.*.d)

# Names of build targets
MAINNAMES = launcher game server editor essai\
            automatonlib libepicinium quicktest
TOOLNAMES = $(TOOL_SRC:src/build/%.cpp=%)
TBOTNAMES = $(filter bot%,$(TOOLNAMES))
TAILNAMES = $(filter-out $(MAINNAMES),$(filter lib%,$(TOOLNAMES)))
TBINNAMES = $(filter-out $(TAILNAMES),$(filter-out $(TBOTNAMES),\
            $(filter-out $(MAINNAMES),$(TOOLNAMES))))
NAMES     = $(MAINNAMES) $(TOOLNAMES)

# Binaries
ifeq ($(detected_OS),Windows)
launcher_OUT = $(GAMENAME).exe
game_OUT = bin/game.exe
server_OUT = bin/server.exe
editor_OUT = bin/editor.exe
essai_OUT = bin/essai.exe
automatonlib_OUT = bin/$(GAMENAME)-automaton.lib
$(foreach tool,$(TBINNAMES),$(eval $(tool)_OUT = bin/$(tool).exe))
libepicinium_OUT = bin/libepicinium.lib
$(foreach tool,$(TAILNAMES),$(eval $(tool)_OUT = $(tool:lib%=bin/ai%.dll)))
$(foreach tool,$(TBOTNAMES),$(eval $(tool)_OUT = $(tool:%=bin/%.exe)))
else
launcher_OUT = $(GAMENAME)
game_OUT = bin/game
server_OUT = server
editor_OUT = editor
essai_OUT = essai
automatonlib_OUT = bin/$(GAMENAME)-automaton.a
$(foreach tool,$(TBINNAMES),$(eval $(tool)_OUT = bin/$(tool)))
libepicinium_OUT = bin/libepicinium.a
$(foreach tool,$(TAILNAMES),$(eval $(tool)_OUT = $(tool:lib%=bin/ai%.so)))
$(foreach tool,$(TBOTNAMES),$(eval $(tool)_OUT = $(tool:%=bin/%)))
endif
quicktest_OUT =
ALL_OUT = $(foreach name,$(NAMES),$($(name)_OUT))

# Build parts
launcher = LNCH
game = CPP MAIN CORE AINT UNET ENGN SDL IMGU GL SSL CURL FZIP DISC INTL STM LAST
server = CPP SERV CORE AINT NETW MESG SDLN SSL CURL FZIP INTL LAST
editor = CPP EDIT CORE AINT ENGN SDL IMGU GL INTL LAST
essai = CPP ESSA CORE AINT INTL LAST
automatonlib = CPP CMON_PIC LGIC_PIC AINT_PIC LAST
pulse = CPP CORE NETW MESG SDLN SSL CURL INTL LAST
biblesaver = CPP CMON_PIC LGIC_PIC JSON_PIC LAST
sanitychecker = CPP CMON_PIC LGIC_PIC JSON_PIC LAST
mapchecker = CPP CMON_PIC LGIC_PIC JSON_PIC AINT_PIC LAST
replaytest = CPP CMON_PIC LGIC_PIC JSON_PIC LAST
benchmarktest = CPP CMON_PIC JSON_PIC LAST
perfalizer = CPP CMON_PIC JSON_PIC LAST
printversion = CPP CMON_PIC JSON_PIC LAST
printprimaries = CPP CMON_PIC JSON_PIC LAST
printsizes = CPP CMON_PIC LGIC_PIC JSON_PIC LAST
steamlocalizer = CPP CORE AINT INTL LAST
termlocalizer = CPP CORE INTL LAST
libepicinium = CPP CMON_LIC LGIC_LIC JSON_PIC AINT_LIC LAST
$(foreach tool,$(TAILNAMES),\
	$(eval $(tool) = CMON_PIC LGIC_PIC JSON_PIC AINT_PIC))
$(foreach tool,$(TBOTNAMES),\
	$(eval $(tool) = CPP CMON_PIC LGIC_PIC JSON_PIC AINT_PIC \
		NETW MESG SDLN SSL CURL LAST))
quicktest =

$(foreach name,$(NAMES),\
	$(eval $(name)_LFLAGS = $(foreach part,$($(name)),$(LFLAGS_$(part))))\
	$(eval $(name)_OBJ    = $(foreach part,$($(name)),$($(part)_OBJ)))\
	$(eval $(name)_DEP    = $(foreach part,$($(name)),$($(part)_DEP)))\
	)

$(foreach tool,$(TBINNAMES),\
	$(eval $(tool)_OBJ    = .obj/src/build/$(tool).o $($(tool)_OBJ))\
	$(eval $(tool)_DEP    = .dep/src/build/$(tool).d $($(tool)_DEP))\
	)

$(foreach tool,libepicinium,\
	$(eval $(tool)_OBJ    = .obj/.lic/src/build/$(tool).o $($(tool)_OBJ))\
	$(eval $(tool)_DEP    = .dep/.lic/src/build/$(tool).d $($(tool)_DEP))\
	)

$(foreach tool,$(TAILNAMES),\
	$(eval $(tool)_OBJ    = .obj/.pic/src/build/$(tool).o $($(tool)_OBJ))\
	$(eval $(tool)_DEP    = .dep/.pic/src/build/$(tool).d $($(tool)_DEP))\
	)

$(foreach tool,$(TBOTNAMES),\
	$(eval $(name)_LFLAGS = $(foreach part,$($(name)),$(LFLAGS_$(part))))\
	$(eval $(tool)_OBJ    = .obj/.pic/src/build/$(tool).o $($(tool)_OBJ))\
	$(eval $(tool)_DEP    = .dep/.pic/src/build/$(tool).d $($(tool)_DEP))\
	)

# Libraries
ifeq ($(detected_OS),Windows)
LIBRARIES   = $(filter-out bin/ai%,$(wildcard bin/*.dll))
else
ifeq ($(detected_OS),Darwin) # Mac OS X
LIBRARIES   = $(filter-out bin/ai%,$(wildcard bin/*.dylib))
else
LIBRARIES   = $(filter-out bin/ai%,$(wildcard bin/*.so*))
endif
endif

# configure me from CONFIGFILE or override me from the command line (yes/no)
BUILD_LOC  ?= no
CHECK_ALL  ?= no
#

# Languages
ALL_LOCALES = $(patsubst translations/%.po,%,$(wildcard translations/*.po))
LOCALES     = $(filter-out xx_%,$(ALL_LOCALES))
XX_LOCALES  = $(filter xx_%,$(ALL_LOCALES))
LOC_FILES   = $(patsubst %,data/loc/%.po,$(LOCALES))
XXLOC_FILES = $(patsubst %,data/loc/%.po,$(XX_LOCALES))

# Each top-level folder needs to have a dummy rule somewhere in the Makefile,
# so we don't accidentally forget to update the Makefile when adding a new one.
SPECIFIES   = $(patsubst %/,.dep/specify-%,\
              $(filter-out package/,$(wildcard */)))
SPCFD_FILES = $(patsubst %,.dep/specify-%,$(SPECIFIEDS))
SPECIFIEDS  = accounts ai archive audio bin brainstorm data docs downloads\
              essailogs\
              fonts keys libs logs maps patches packages pictures recordings\
              resources rulesets sessions sprites src tools translations\
              $(GAMENAME).app

# Targets
game: $(game_OUT) $(launcher_OUT)
.PHONY: game

launcher: $(launcher_OUT)
.PHONY: launcher

ifeq ($(detected_OS),Windows)
server: $(server_OUT)
editor: $(editor_OUT)
essai: $(essai_OUT)
.PHONY: server editor essai
endif

automatonlib: $(automatonlib_OUT)
.PHONY: automatonlib

libepicinium: $(libepicinium_OUT)
.PHONY: libepicinium

ifeq ($(detected_OS),Windows)
$(TBINNAMES): %: bin/%.exe
$(TBOTNAMES): %: bin/%.exe
$(TAILNAMES): lib%: bin/ai%.dll
else
$(TBINNAMES): %: bin/%
$(TBOTNAMES): %: bin/%
$(TAILNAMES): lib%: bin/ai%.so
endif
.PHONY: $(TOOLNAMES)

$(game_OUT): $(game_OBJ) $(game_DEP) $(RECOMPILERS) bin/checksome
	$(COMPILE_BIN) -o $@ $(filter %.o,$^) $(LPATH) $(game_LFLAGS)
ifeq ($(detected_OS),Darwin) # Mac OS X
#	$(FIX_LIBSTD_DYLIB) $(game_OUT)
#	$(FIX_LIBGCC_DYLIB) $(game_OUT)
ifeq ($(VARIANT),release)
	strip $(game_OUT)
endif
endif

$(server_OUT): $(server_OBJ) $(server_DEP) $(RECOMPILERS) bin/check
	$(COMPILE_BIN) -o $@ $(filter %.o,$^) $(LPATH) $(server_LFLAGS)
ifeq ($(detected_OS),Darwin) # Mac OS X
	$(FIX_LIBSTD_DYLIB) $(server_OUT)
	$(FIX_LIBGCC_DYLIB) $(server_OUT)
endif

$(editor_OUT): $(editor_OBJ) $(editor_DEP) $(RECOMPILERS)
	$(COMPILE_BIN) -o $@ $(filter %.o,$^) $(LPATH) $(editor_LFLAGS)
ifeq ($(detected_OS),Darwin) # Mac OS X
	$(FIX_LIBSTD_DYLIB) $(editor_OUT)
	$(FIX_LIBGCC_DYLIB) $(editor_OUT)
endif

$(essai_OUT): $(essai_OBJ) $(essai_DEP) $(RECOMPILERS)
	$(COMPILE_BIN) -o $@ $(filter %.o,$^) $(LPATH) $(essai_LFLAGS)
ifeq ($(detected_OS),Darwin) # Mac OS X
	$(FIX_LIBSTD_DYLIB) $(essai_OUT)
	$(FIX_LIBGCC_DYLIB) $(essai_OUT)
endif

$(automatonlib_OUT): $(automatonlib_OBJ) $(automatonlib_DEP) $(RECOMPILERS)
	$(COMPILE_ALB) $@ $(filter %.o,$^)
ifeq ($(detected_OS),Darwin) # Mac OS X
	$(FIX_LIBSTD_DYLIB) $(automatonlib_OUT)
	$(FIX_LIBGCC_DYLIB) $(automatonlib_OUT)
endif

$(launcher_OUT): $(launcher_OBJ) $(launcher_DEP) $(RECOMPILERS)
	$(COMPILE_CBN) -o $@ $(filter %.o,$^) $(LPATH) $(launcher_LFLAGS)
ifeq ($(detected_OS),Darwin) # Mac OS X
	$(FIX_LIBGCC_DYLIB) $(launcher_OUT)
endif

$(pulse_OUT): $(pulse_OBJ) $(pulse_DEP)
	$(COMPILE_BIN) -o $@ $(filter %.o,$^) $(LPATH) $(pulse_LFLAGS)

$(biblesaver_OUT): $(biblesaver_OBJ) $(biblesaver_DEP)
	$(COMPILE_BIN) -o $@ $(filter %.o,$^) $(LPATH) $(biblesaver_LFLAGS)

$(sanitychecker_OUT): $(sanitychecker_OBJ) $(sanitychecker_DEP)
	$(COMPILE_BIN) -o $@ $(filter %.o,$^) $(LPATH) $(sanitychecker_LFLAGS)

$(mapchecker_OUT): $(mapchecker_OBJ) $(mapchecker_DEP)
	$(COMPILE_BIN) -o $@ $(filter %.o,$^) $(LPATH) $(mapchecker_LFLAGS)

$(replaytest_OUT): $(replaytest_OBJ) $(replaytest_DEP)
	$(COMPILE_BIN) -o $@ $(filter %.o,$^) $(LPATH) $(replaytest_LFLAGS)

$(benchmarktest_OUT): $(benchmarktest_OBJ) $(benchmarktest_DEP)
	$(COMPILE_BIN) -o $@ $(filter %.o,$^) $(LPATH) $(benchmarktest_LFLAGS)

$(perfalizer_OUT): $(perfalizer_OBJ) $(perfalizer_DEP)
	$(COMPILE_BIN) -o $@ $(filter %.o,$^) $(LPATH) $(perfalizer_LFLAGS)

$(printversion_OUT): $(printversion_OBJ) $(printversion_DEP)
	$(COMPILE_BIN) -o $@ $(filter %.o,$^) $(LPATH) $(printversion_LFLAGS)

$(printprimaries_OUT): $(printprimaries_OBJ) $(printprimaries_DEP)
	$(COMPILE_BIN) -o $@ $(filter %.o,$^) $(LPATH) $(printprimaries_LFLAGS)

$(printsizes_OUT): $(printsizes_OBJ) $(printsizes_DEP)
	$(COMPILE_BIN) -o $@ $(filter %.o,$^) $(LPATH) $(printsizes_LFLAGS)

$(steamlocalizer_OUT): $(steamlocalizer_OBJ) $(steamlocalizer_DEP)
	$(COMPILE_BIN) -o $@ $(filter %.o,$^) $(LPATH) $(steamlocalizer_LFLAGS)

$(termlocalizer_OUT): $(termlocalizer_OBJ) $(termlocalizer_DEP)
	$(COMPILE_BIN) -o $@ $(filter %.o,$^) $(LPATH) $(termlocalizer_LFLAGS)

$(libepicinium_OUT): $(libepicinium_OBJ) $(libepicinium_DEP)
	$(COMPILE_ALB) -o $@ $(filter %.o,$^)

$(libquickquack_OUT): $(libquickquack_OBJ) $(libquickquack_DEP)
	$(COMPILE_AIL) -o $@ $(filter %.o,$^)

$(libhungryhippo_OUT): $(libhungryhippo_OBJ) $(libhungryhippo_DEP)
	$(COMPILE_AIL) -o $@ $(filter %.o,$^)

$(libchargingcheetah_OUT): $(libchargingcheetah_OBJ) $(libchargingcheetah_DEP)
	$(COMPILE_AIL) -o $@ $(filter %.o,$^)

$(librampantrhino_OUT): $(librampantrhino_OBJ) $(librampantrhino_DEP)
	$(COMPILE_AIL) -o $@ $(filter %.o,$^)

$(botquickquack_OUT): $(botquickquack_OBJ) $(botquickquack_DEP)
	$(COMPILE_BIN) -o $@ $(filter %.o,$^) $(LPATH) $(botquickquack_LFLAGS)

$(botchargingcheetah_OUT): $(botchargingcheetah_OBJ) $(botchargingcheetah_DEP)
	$(COMPILE_BIN) -o $@ $(filter %.o,$^) $(LPATH) $(botchargingcheetah_LFLAGS)

all: $(ALL_OUT)
.PHONY: all

ifeq ($(CHECK_ALL),yes)
bin/checksome: bin/check
	touch bin/checksome
else
bin/checksome: bin/checksanity
	touch bin/checksome
endif

bin/check: bin/checkbible bin/checksanity bin/checkmaps
bin/check: | $(SPECIFIES)
	touch bin/check

bin/checkbible: src/logic/bible.cpp src/logic/library.cpp
bin/checkbible: src/common/version.cpp src/common/version.hpp
bin/checkbible: $(VARIANTFILE) | $(biblesaver_OUT)
ifeq (USED_SHELL, cmd)
	"./$(biblesaver_OUT)"
else
	./$(biblesaver_OUT)
endif
	touch bin/checkbible

bin/checksanity: src/logic/player.hpp src/logic/cycle.hpp
bin/checksanity: src/logic/unittype.hpp src/logic/tiletype.hpp
bin/checksanity: src/logic/unittoken.hpp src/logic/tiletoken.hpp
bin/checksanity: src/logic/change.hpp src/logic/order.hpp
bin/checksanity: $(VARIANTFILE) | $(sanitychecker_OUT)
ifeq (USED_SHELL, cmd)
	"./$(sanitychecker_OUT)"
else
	./$(sanitychecker_OUT)
endif
	touch bin/checksanity

bin/checkmaps: src/logic/map.cpp src/ai/aichallenge.cpp
bin/checkmaps: $(VARIANTFILE) | $(mapchecker_OUT)
ifeq (USED_SHELL, cmd)
	"./$(mapchecker_OUT)"
else
	./$(mapchecker_OUT)
endif
	touch bin/checkmaps

.obj/%.s: %.cpp .dep/%.d $(RECOMPILERS) | .dep/.dir_fixes
	$(COMPILE_OBJ) -S -fverbose-asm -o $@ -c $<
	$(POSTCOMPILE)

.obj/%.i: %.cpp .dep/%.d $(RECOMPILERS) | .dep/.dir_fixes
	$(COMPILE_OBJ) -E -H -o $@ -c $<
	$(POSTCOMPILE)

.obj/%.o: %.cpp .dep/%.d $(RECOMPILERS) | .dep/.dir_fixes
	$(COMPILE_OBJ) -o $@ -c $<
	$(POSTCOMPILE)

.obj/%.o: %.c .dep/%.d $(RECOMPILERS) | .dep/.dir_fixes
	$(COMPILE_COB) -o $@ -c $<
	$(POSTCOMPILE)

.dep/%.d: ;
.PRECIOUS: .dep/%.d

$(STEAM_REAL_DEPENDENCIES): bin/%: $(STEAM_ARCHIVE_REDIS_DIR)/%
	cp $< $@

$(VERSIONFILE): src/common/version.cpp src/common/version.hpp $(VARIANTFILE)
$(VERSIONFILE): | $(printversion_OUT)
$(VERSIONFILE): | $(dir $(VERSIONFILE)).dir_fix
	"./$(printversion_OUT)" > $(VERSIONFILE)

$(VARIANTFILE): | $(dir $(VARIANTFILE)).dir_fix
	echo $(VARIANT) > $(VARIANTFILE)

dev: | $(dir $(VARIANTFILE)).dir_fix $(dir $(PERFANTFILE)).dir_fix
	echo dev > $(VARIANTFILE)
	echo dev > $(PERFANTFILE)
.PHONY: dev

candidate: | $(dir $(VARIANTFILE)).dir_fix $(dir $(PERFANTFILE)).dir_fix
	echo candidate > $(VARIANTFILE)
	echo perf > $(PERFANTFILE)
.PHONY: candidate

release: | $(dir $(VARIANTFILE)).dir_fix $(dir $(PERFANTFILE)).dir_fix
	echo release > $(VARIANTFILE)
	echo perf > $(PERFANTFILE)
.PHONY: release

releaseserver: | $(dir $(VARIANTFILE)).dir_fix $(dir $(PERFANTFILE)).dir_fix
	echo server > $(VARIANTFILE)
	echo native > $(PERFANTFILE)
.PHONY: releaseserver

$(PERFANTFILE): | $(dir $(PERFANTFILE)).dir_fix
	echo $(PERFANT) > $(PERFANTFILE)

perf: | $(dir $(PERFANTFILE)).dir_fix
	echo perf > $(PERFANTFILE)
.PHONY: perf

devperf: | $(dir $(PERFANTFILE)).dir_fix
	echo dev > $(PERFANTFILE)
.PHONY: devperf

nativeperf: | $(dir $(PERFANTFILE)).dir_fix
	echo native > $(PERFANTFILE)
.PHONY: nativeperf

$(TARGETFILE): | $(dir $(TARGETFILE)).dir_fix
	echo $(TARGET) > $(TARGETFILE)
	cp -RP archive/$(TARGET)/. ./

debian64: | $(dir $(TARGETFILE)).dir_fix
	echo debian64 > $(TARGETFILE)
	cp -RP archive/debian64/. ./
.PHONY: debian64

debian32: | $(dir $(TARGETFILE)).dir_fix
	echo debian32 > $(TARGETFILE)
	cp -RP archive/debian32/. ./
.PHONY: debian32

windows64: | $(dir $(TARGETFILE)).dir_fix
	echo windows64 > $(TARGETFILE)
	cp -RP archive/windows64/. ./
#	set PATH=D:\stack\3\mingw-w64\mingw64\bin;%PATH%
#	$Env:path="D:\MinGWs\mingw-w64\mingw64\bin;$Env:path"
.PHONY: windows64

windows32: | $(dir $(TARGETFILE)).dir_fix
	echo windows32 > $(TARGETFILE)
	cp -RP archive/windows32/. ./
#	set PATH=D:\stack\3\mingw-w64-32\mingw32\bin;%PATH%
#	$Env:path="D:\MinGWs\mingw-w64-32\mingw32\bin;$Env:path"
.PHONY: windows32

osx32: | $(dir $(TARGETFILE)).dir_fix
	echo osx32 > $(TARGETFILE)
	cp -RP archive/osx32/. ./
.PHONY: osx32

osx64: | $(dir $(TARGETFILE)).dir_fix
	echo osx64 > $(TARGETFILE)
	cp -RP archive/osx64/. ./
.PHONY: osx64

translations/$(GAMENAME).pot: $(OUR_SOURCES)
	xgettext --keyword=_ --language=C++ --sort-by-file \
		--add-comments=TRANSLATOR \
		--copyright-holder="A Bunch of Hacks" \
		--package-name="Epicinium" --package-version="0.35.0" \
		--msgid-bugs-address="sander@abunchofhacks.coop" \
		-o $@ $(OUR_SOURCES)

translations/en_US.po: translations/$(GAMENAME).pot
	msginit --locale=en_US.utf-8 --no-translator -o $@ -i $<

translations/xx_AA.po: translations/en_US.po
	msgfilter --keep-header -o $@ -i $< sed -e 's/\([aeiuoyAEIUOY]\)/\1\1/g'

translations/xx_XX.po: translations/en_US.po
	msgfilter --keep-header -o $@ -i $< \
		sed -e 's/\([aeiuoyAEIUOY]\+\)/測/g' \
			-e 's/\([^%a-zA-Z]\)\([a-zA-Z]\+\)/\1試/g' \
			-e 's/^\([a-zA-Z]\+\)/試/g'

LCLZR = translations/localizor
$(LCLZR)/.cs_CZ.po: $(LCLZR)/Czech.po $(LCLZR)/.exported
$(LCLZR)/.de_DE.po: $(LCLZR)/German.po $(LCLZR)/.exported
$(LCLZR)/.es_ES.po: $(LCLZR)/Spanish\ (Spain).po $(LCLZR)/.exported
$(LCLZR)/.fi_FI.po: $(LCLZR)/Finnish.po $(LCLZR)/.exported
$(LCLZR)/.fr_FR.po: $(LCLZR)/French.po $(LCLZR)/.exported
$(LCLZR)/.it_IT.po: $(LCLZR)/Italian.po $(LCLZR)/.exported
$(LCLZR)/.nl_NL.po: $(LCLZR)/Dutch.po $(LCLZR)/.exported
$(LCLZR)/.pl_PL.po: $(LCLZR)/Polish.po $(LCLZR)/.exported
$(LCLZR)/.pt_BR.po: $(LCLZR)/Portuguese\ (BR).po $(LCLZR)/.exported
$(LCLZR)/.ru_RU.po: $(LCLZR)/Russian.po $(LCLZR)/.exported
$(LCLZR)/.tr_TR.po: $(LCLZR)/Turkish.po $(LCLZR)/.exported
$(LCLZR)/.uk_UA.po: $(LCLZR)/Ukrainian.po $(LCLZR)/.exported

LCLZR_LOC_FILES = $(patsubst %,$(LCLZR)/.%.po,$(LOCALES))
LCLZR_CT = \"Content-Type: text/plain; charset=UTF-8\\\\n\"
LCLZR_CTE = \"Content-Transfer-Encoding: 8bit\\\\n\"
$(LCLZR_LOC_FILES):
	echo "msgid \"\"\nmsgstr \"\"\n$(LCLZR_CT)\n$(LCLZR_CTE)" > $@
	tail -n +3 "$<" >> $@

translations/%.po: translations/$(GAMENAME).pot $(LCLZR)/.%.po
	msginit --locale=$*.utf-8 --no-translator -o $@ -i $<
	msgmerge --update --backup=off \
		--no-fuzzy-matching \
		--compendium $(LCLZR)/.$*.po \
		$@ $<
	touch $@

ifeq ($(BUILD_LOC),yes)
data/loc/%.po: translations/%.po data/loc
	sed '/^#:/d' < $< > $@
endif

data/loc:
ifeq ($(USED_SHELL),cmd)
	"mkdir.exe" -p "$@"
else
	mkdir -p $@
endif
.PRECIOUS: data/loc

update-loc: $(LOC_FILES) $(XXLOC_FILES)
	@echo "updated"
.PHONY: update-loc

$(LCLZR)/terms_%.csv: $(LOC_FILES) $(termlocalizer_OUT)
	"./$(termlocalizer_OUT)" --language=$* > $@

translations/steam_rich_presence.vdf: $(LOC_FILES) $(steamlocalizer_OUT)
	"./$(steamlocalizer_OUT)" > $@

# override me from CONFIGFILE or from the command line (yes/no)
ifeq ($(USED_SHELL),cmd)
MKDIR ?= "mkdir.exe"
else
MKDIR ?= mkdir
endif
#

RULESETS         = $(filter-out $(wildcard rulesets/*test*),\
                   $(filter-out $(wildcard rulesets/*internal*),\
                   $(filter-out $(wildcard rulesets/*-rc*.json),\
                   $(filter-out $(wildcard rulesets/custom.json),\
                   $(wildcard rulesets/*.json)))))
MAPS             = $(filter-out $(wildcard maps/*test*),\
                   $(wildcard maps/*.map))

package: game $(LOC_FILES) bin/check
package: | $(SPECIFIES)
	rm -rf ./package/
	$(MKDIR) -p ./package
	$(MKDIR) -p ./package/pictures
	$(MKDIR) -p ./package/pictures/discord
	$(MKDIR) -p ./package/accounts
	$(MKDIR) -p ./package/sessions
	$(MKDIR) -p ./package/logs
	$(MKDIR) -p ./package/recordings
	$(MKDIR) -p ./package/downloads
	$(MKDIR) -p ./package/keys
	$(MKDIR) -p ./package/rulesets
	$(MKDIR) -p ./package/maps
	$(MKDIR) -p ./package/bin
	$(MKDIR) -p ./package/data/loc
	cp -RP docs ./package/
	cp -RP resources ./package/
	cp -RP sprites ./package/
	cp -RP pictures/ABOUT.txt ./package/pictures/
	cp -RP pictures/unknown.png ./package/pictures/
	cp -RP pictures/art ./package/pictures/
	cp -RP pictures/panels ./package/pictures/
	cp -RP pictures/discord/default.png ./package/pictures/discord/
	cp -RP audio ./package/
	cp -RP fonts ./package/
	cp -RP INSTALL.txt ./package/
	cp -RP accounts/ABOUT.txt ./package/accounts/
	cp -RP sessions/ABOUT.txt ./package/sessions/
	cp -RP logs/ABOUT.txt ./package/logs/
	cp -RP recordings/ABOUT.txt ./package/recordings/
	cp -RP downloads/ABOUT.txt ./package/downloads/
	cp -RP keys/ABOUT.txt ./package/keys/
	cp -RP keys/curl-ca-bundle.crt ./package/keys/
	cp -RP rulesets/ABOUT.txt ./package/rulesets/
	cp -RP rulesets/index.list ./package/rulesets/
	cp -RP $(RULESETS) ./package/rulesets/
	cp -RP maps/ABOUT.txt ./package/maps/
	cp -RP $(MAPS) ./package/maps/
	cp -RP $(launcher_OUT) ./package/$(launcher_OUT)
	cp -RP $(game_OUT) ./package/$(game_OUT)
	cp -RP bin/ABOUT.txt ./package/bin/
	cp -RP $(LIBRARIES) ./package/bin/
	cp -RP data/ABOUT.txt ./package/data/
	cp -RP $(LOC_FILES) ./package/data/loc/
.PHONY: package

# Evaluate PVERSION only when it is called because VERSIONFILE might not exist;
# note that VERSIONFILE is a prerequisite of zip and tar.
PVERSION = $(eval PVERSION := $$(strip $(shell cat $(VERSIONFILE))))$(PVERSION)

zip: package $(VERSIONFILE)
	cd package && zip ../$(GAMENAME)-$(TARGET)-$(PVERSION).zip -r *
.PHONY: zip

tar: package $(VERSIONFILE)
	cd package && tar -czvf ../$(GAMENAME)-$(TARGET)-$(PVERSION).tar.gz *
.PHONY: tar

app: package $(VERSIONFILE)
	rm -rf $(GAMENAME).app
	$(MKDIR) -p $(GAMENAME).app
	$(MKDIR) -p $(GAMENAME).app/Contents
	$(MKDIR) -p $(GAMENAME).app/Contents/MacOS
	$(MKDIR) -p $(GAMENAME).app/Contents/Resources
	cp -RP Info.plist $(GAMENAME).app/Contents/
	cp -RP package/data $(GAMENAME).app/Contents/Resources/
	cp -RP package/docs $(GAMENAME).app/Contents/Resources/
	cp -RP package/resources $(GAMENAME).app/Contents/Resources/
	cp -RP package/sprites $(GAMENAME).app/Contents/Resources/
	cp -RP package/pictures $(GAMENAME).app/Contents/Resources/
	cp -RP package/audio $(GAMENAME).app/Contents/Resources/
	cp -RP package/fonts $(GAMENAME).app/Contents/Resources/
	cp -RP package/accounts $(GAMENAME).app/Contents/Resources/
	cp -RP package/logs $(GAMENAME).app/Contents/Resources/
	cp -RP package/recordings $(GAMENAME).app/Contents/Resources/
	cp -RP package/sessions $(GAMENAME).app/Contents/Resources/
	cp -RP package/downloads $(GAMENAME).app/Contents/Resources/
	cp -RP package/keys $(GAMENAME).app/Contents/Resources/
	cp -RP package/rulesets $(GAMENAME).app/Contents/Resources/
	cp -RP package/maps $(GAMENAME).app/Contents/Resources/
	cp -RP package/$(launcher_OUT) $(GAMENAME).app/Contents/MacOS/
	cp -RP package/bin $(GAMENAME).app/Contents/MacOS/
.PHONY: app

# Dependency graphs
GRAPH_SUBDIRS  = $(subst src/,graph-,$(wildcard src/*))
GRAPH_SUBUSES  = $(patsubst src/%,graph-%-use,$(wildcard src/*))
GRAPH_SUBGITS  = $(patsubst src/%,graph-%-git,$(wildcard src/*))
GRAPH_COMMANDS = graph-source graph-subdirs $(GRAPH_SUBDIRS) $(GRAPH_SUBUSES)\
                 graph-source-git graph-subdirs-git $(GRAPH_SUBGITS)
GRAPH_SUBDOTS  = $(patsubst graph-%,.dep/%.dot,$(GRAPH_SUBDIRS))
GRAPH_USEDOTS  = $(patsubst graph-%,.dep/%.dot,$(GRAPH_SUBUSES))
GRAPH_SBGTDOTS = $(patsubst graph-%,.dep/%.dot,$(GRAPH_SUBGITS))
CINCLUDE2DOT   = tools/cinclude2dot\
                 --quotetypes quote\
                 --include ./,$(subst $(SPACE),$(COMMA),$(wildcard src/*/))\
                 --exclude ".*(\.cu-|common/|printsizes|sanitychecker).*"
GITCOUPLER     = tools/gitcoupler.sh

.dep/source.dot: $(OUR_SOURCES) $(OUR_HEADERS) | .dep/.dir_fix
	$(CINCLUDE2DOT) --src src --merge module --groups > $@

.dep/subdirs.dot: $(OUR_SOURCES) $(OUR_HEADERS) | .dep/.dir_fix
	$(CINCLUDE2DOT) --src src --merge directory > $@

$(GRAPH_SUBDOTS): .dep/%.dot: $(OUR_SOURCES) $(OUR_HEADERS) | .dep/.dir_fix
	$(CINCLUDE2DOT) --src src/$* --merge module --groups > $@

$(GRAPH_USEDOTS): .dep/%-use.dot: $(OUR_SOURCES) $(OUR_HEADERS) | .dep/.dir_fix
	$(CINCLUDE2DOT) --src src --reverse src/$* --merge module --groups > $@

.dep/source-git.dot: $(GITCOUPLER)
	$(GITCOUPLER) > $@

.dep/subdirs-git.dot: $(GITCOUPLER)
	$(GITCOUPLER) subdirs > $@

$(GRAPH_SBGTDOTS): .dep/%-git.dot: $(GITCOUPLER)
	$(GITCOUPLER) $* > $@

.dep/%.ps: .dep/%.dot
	dot -Tps $< -o $@

$(GRAPH_COMMANDS): graph-%: .dep/%.ps
	xreader $< 2> /dev/null &
.PHONY: $(GRAPH_COMMANDS)

# Fixing and cleaning
$(SPCFD_FILES): %: | .dep/.dir_fix
	touch $@

.dep/.dir_fixes: | $(DIRFIXES)
	touch $@
.PRECIOUS: .dep/.dir_fixes

.pic/src/%: | .pic/src src/% ;
.PRECIOUS: .pic/src/%

.pic/libs/%: | .pic/libs libs/% ;
.PRECIOUS: .pic/libs/%

.pic/src: src | .pic/.dir_fix
	ln -sn ../src .pic/src

.pic/libs: libs | .pic/.dir_fix
	ln -sn ../libs .pic/libs

.lic/src/%: | .lic/src src/% ;
.PRECIOUS: .lic/src/%

.lic/libs/%: | .lic/libs libs/% ;
.PRECIOUS: .lic/libs/%

.lic/src: src | .lic/.dir_fix
	ln -sn ../src .lic/src

.lic/libs: libs | .lic/.dir_fix
	ln -sn ../libs .lic/libs

%_fix:
ifeq ($(USED_SHELL),cmd)
	"mkdir.exe" -p "$@"
else
	mkdir -p $@
endif
.PRECIOUS: %_fix

clean:
	rm -f $(ALL_DEP)
	rm -f $(ALL_OBJ)
	rm -f $(ALL_OUT)
	rm -f ./bin/check ./bin/checkbible ./bin/checksanity ./bin/checkmaps
.PHONY: clean

reset:
	rm -rf .dep
	rm -rf .obj
	rm -rf .pic
	rm -rf .lic
	rm -rf ./bin
	rm -rf ./package
	rm -f ./$(GAMENAME).bat ./server.bat ./editor.bat
	rm -f $(ALL_OUT)
	rm -f ./biblesaver ./sanitychecker ./mapchecker ./printversion
	rm -f ./check ./checkbible ./checksanity ./checkmaps
	rm -f ./version ./variant ./perfant ./target
.PHONY: reset

# Print variables
print-%  : ; @echo $($*)

debugprint:
	@echo "detected_OS = $(detected_OS)"
	@echo "detected_shell = $(detected_shell)"
	@echo "USED_SHELL = $(USED_SHELL)"
	@echo "MKDIR = $(MKDIR)"
	@echo "ARCH = $(ARCH)"
	@echo "PLATFORM = $(PLATFORM)"
	@echo "TARGET = $(TARGET)"
	@echo "VARIANT = $(VARIANT)"
	@echo "PERFANT = $(PERFANT)"
	@echo "CXX = $(CXX)"
	@echo "CC = $(CC)"
	@echo "AR = $(AR)"
	@echo "CFLAGS = $(CFLAGS)"
	@echo "IFLAGS = $(IFLAGS)"
	@echo "IFLAGS_FULL = $(IFLAGS_FULL)"
	@echo "LFLAGS_LAST = $(LFLAGS_LAST)"
	@echo "STANDARD = $(STANDARD)"
	@echo "STANDARD_C = $(STANDARD_C)"
	@echo "ENABLE_STEAM = $(ENABLE_STEAM)"
	@echo "ENABLE_FEMTOZIP = $(ENABLE_FEMTOZIP)"
	@echo "ENABLE_CU = $(ENABLE_CU)"
	@echo "BUILD_LOC = $(BUILD_LOC)"
.PHONY: debugprint

# Include generated dependencies
-include $(ALL_INCL_DEP)
