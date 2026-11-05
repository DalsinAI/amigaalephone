Aleph One for AmigaOS 3.x
=========================

Aleph One 1.6.2 (upstream https://github.com/Aleph-One-Marathon/alephone,
tag release-20230529, commit 1f4c973aba1017c814e8c3f0db001fe030b1c555,
GPL-3.0-or-later), the open-source Marathon engine, for AmigaOS 3.2 on a
68040 or 68060 with an FPU, with its software renderer.

Status: builds and links; not yet run on an Amiga.


What you need
-------------

- AmigaOS 3.2 (or OS 3.2.3 with GFOpenUp), a 68040/68060 with an FPU, and
  plenty of Fast RAM: the program alone is about 9 MB, and a scenario's
  shapes and sounds come on top (32 MB or more).
- An RTG screen that SDL 2 can open (640x480 is used, full screen).
- OpenGPU's runtime, which OpenUp installs: opengpu.library and
  LIBS:OpenGPU/SDL2.module and SDL2_image.module.
- For a network game only: a TCP/IP stack (bsdsocket.library).
- Marathon data: none comes with the engine. See Data-ReadMe in the drawer.


Using it
--------

Copy one scenario's data into the Scenario drawer and open AlephOne's icon,
or name a scenario's drawer from a Shell:

    cd Work:Games/AlephOne
    AlephOne Work:Games/Marathon2

From Workbench, a DATA tool type on AlephOne's icon (DATA=Work:Marathon2)
names the scenario; icons given with AlephOne's (shift-click) are passed as
a Shell's arguments would be (a scenario drawer, a film, a saved game).

Preferences, saved games, quick saves, films, screenshots and the log go in
the User drawer beside the program, made on the first run.

The first run starts at 640x480 with the software renderer drawing in 8-bit
colour at low resolution (pixels doubled), sound mixed at 22050 Hz, and
ephemera off: what a 68040 can keep up with. Graphics in Preferences has
the rest (16/32-bit colour, high resolution), as on other systems.


How it is built: the route
--------------------------

Aleph One's own SDL 2 code, unchanged, linked with OpenGPU's SDL 2
developer kit, as the Crispy Doom and DSDA-Doom ports are: libSDL2.a and
libSDL2_image.a are the program's halves of SDL2.module and
SDL2_image.module; libSDL2_ttf.a (with FreeType) and libSDL2_net.a are
inside the program. The other route, our SDL 1.2 shim (acsdl12), was not
taken: Aleph One has needed SDL 2 since 2018, its SDL 2 code would need a
compatibility layer, and SDL_ttf/SDL_image would be missing.

Why release-20230529: it is the last release that mixes its own sound
through SDL's audio callback. From release-20231001 on, Aleph One needs
OpenAL and libsndfile, which have no AmigaOS 3 build; every release since
2015 needs Boost.Filesystem (below). Releases before 2018 are SDL 1.2.

Dependencies:

- SDL2, SDL2_image, SDL2_ttf, SDL2_net: the OpenGPU kit.
- zlib: zlib 1.3.1's sources, built in.
- Boost 1.83.0: headers only (property_tree, iostreams, algorithm,
  tokenizer, unordered, system, range). Boost.Filesystem, the one compiled
  Boost library Aleph One asks for, is replaced by amiga/amiga_fs.cpp on
  dos.library (Lock/Examine/ExNext, CreateDir, DeleteFile, Rename) behind
  amiga/include/boost/filesystem.hpp, the part of its interface that
  Files/FileHandler.cpp and XML/Plugins.cpp use.
- Lua 5.2: Aleph One's own copy (Source_Files/Lua).
- Left out: OpenGL (OpenGPU's GL is there, but no GLU, and a 68040 is not
  the machine for it), FFmpeg (movie export, music other than AIFF/WAV),
  libsndfile, Vorbis, MAD, SMPEG, Speex (network voice), libpng (image
  cache files are saved through SDL2_image), zziplib (zipped plugins),
  curl (update check and the metaserver's HTTP), miniupnpc, NFD.
- libstdc++ asks for POSIX threads: amiga/pthread_stubs.c (one thread).
  Aleph One runs the game in one thread; SDL threads (mytm_sdl.cpp) are only
  made for a network game's packet timing.

Build (GFOpenAmigaGCC 16.2.0-open4 or later; the defaults are CradleLab's
paths):

    make -f amiga/Makefile.amigaos -j2 \
         STOVE=$HOME/CradleLab/toolchain/openamigagcc-16.2.0-open4-x86_64-linux \
         NDK=$HOME/CradleLab/gfopensdk/OpenNDK/Include_H \
         SDL2KIT=/mnt/amigachrome/team2/deps/opengpu-sdk \
         ZLIB=$HOME/CradleLab/zlib-1.3.1 \
         BOOST=/path/to/boost_1_83_0

BOOST defaults to .deps/boost_1_83_0 (not in the repository: unpack
boost_1_83_0.tar.bz2's boost/ there). Output: build/amigaos/AlephOne/
(AlephOne, ReadMe, Data-ReadMe, COPYING.txt, MML/, Plugins/Default_Theme/,
an empty Scenario/, and their icons) and build/amigaos/AlephOne.info.
Flags: -noixemul -m68040 -m68881 -O2 -std=gnu++17 and GCC 16's
-fno-tree-loop-distribute-patterns. Upstream's files build with the
compiler's default warnings, as upstream's configure builds them, and
give none; amiga/ builds with -Wall -Wextra and gives none. The linker
says "duplicate section ... interop_error_category::message ... has
different contents" for an inline Boost.System function: each object's
copy points at its own copy of a string, the linker keeps one copy and its
own relocations, so this is expected and harmless.

Icons: amiga/icons/icons.json, from Aleph One's program icon
(flatpak/alephone.png) and scenario file icon:

    python3 AMICRADLE/scripts/port_icons.py amiga/icons/icons.json --out amiga/icons


Changes to upstream
-------------------

Under #ifdef __amigaos__:

- CSeries/cspaths_sdl.cpp: PROGDIR: paths: the engine's data beside the
  program, the scenario in PROGDIR:Scenario (or the drawer named), the
  rest in PROGDIR:User; ';' separates ALEPHONE_DATA's directories.
- Files/FileHandler.cpp: AmigaDOS names: "Vol:" + "x" is joined without a
  '/' ("PROGDIR:/x" would be the parent), names split at ':' as well as
  '/', and "//" is not folded into "/" (it is the parent's parent).
- shell.cpp, amiga/amiga_main.cpp: a Workbench start's arguments (the
  DATA tool type and the icons given).
- CSeries/csmisc_sdl.cpp: SDL_GetTicks/SDL_Delay for the game's clock in
  place of std::chrono and std::this_thread (libnix's clock steps in 1/50 s,
  and std::this_thread brings in std::thread).
- Files/WadImageCache.cpp: cache file names from std::mt19937 in place of
  Boost's uuid random_generator, which reads /dev/urandom.
- CSeries/csalerts_sdl.cpp: a URL is opened with SDL_OpenURL (no fork).
- CSeries/cstypes.h: GLfloat as a typedef, the same as OpenGPU's
  SDL_opengl.h has (upstream's #define breaks that header).
- RenderMain/OGL_Headers.h: no <GL/glu.h> (the kit has none).
- Misc/preferences.cpp: the 68040 defaults above; software rendering is
  kept when a preferences file asks for OpenGL.
- Sound/SoundManager.h: 22050 Hz by default.
- Network/SDL_netx.cpp: SDL_net.h's INADDR_* are undefined before the
  NDK's <netinet/in.h> defines them again (the same values).

Under #ifdef HAVE_OPENGL (upstream's build without OpenGL had rotted):
RenderMain/OGL_Setup.cpp (the texture, model and shader MML calls),
RenderOther/screen.cpp (the OpenGL context check; MainScreenSwap),
Misc/interface.cpp (OGL_Blitter), FFmpeg/Movie.h (the FBO member).

DISABLE_NETWORKING no longer builds upstream (AStream and the HUD's network
calls are behind it), so the network game is in, on SDL2_net.


Byte order and alignment
------------------------

Marathon's files come from the big-endian Mac, like the 68k. Aleph One
reads them field by field (Files/Packing.h, big-endian by default;
SDL_ReadBE16/32 for resource forks, AppleSingle/MacBinary, physics, PICT,
AIFF) and swaps only when ALEPHONE_LITTLE_ENDIAN is set, from SDL_BYTEORDER
in CSeries/cseries.h; the kit's SDL headers give SDL_BIG_ENDIAN on m68k, so
byte_swap_memory and the other swaps compile away. amiga/endian_check.cpp
stops the build if that ever changes. Little-endian data is read as such:
WAV (BasicIFFDecoder, SDL_ReadLE*), the mixer's per-channel endian flag
(Sound/Mixer.cpp) and DDS texture headers (ImageLoader_Shared.cpp,
SDL_SwapLE*). Saved games, films,
network packets and wad files go through the same Packing/AStream code
(network_data_formats.cpp copies with memcpy on a big-endian machine, as
the format is big-endian). No struct is read or written whole from a file
(searched for fread/SDL_RWread of a struct), so the 68k's 2-byte alignment
of int and pointers does not change any file format. Not run yet: the
first test on an Amiga is the real check.


What to check first
-------------------

1. It opens a 640x480 screen and the main menu appears with a scenario in
   Scenario (Marathon 2 is the usual first test), and the User drawer and
   its subdrawers are made.
2. A level starts, the world draws (8-bit, low resolution) and the frame
   rate is usable on a 68040/68060; sound and the AIFF music play.
3. Saving and loading a game in User/Saved Games; a film recorded and
   replayed; preferences kept between runs.
4. Plugins (Default_Theme from PROGDIR:Plugins) and the scenario's own MML
   and Scripts load (Preferences, Plugins lists them).
5. Without a TCP/IP stack, stay out of the network game's dialogs: SDL_net
   calls bsdsocket.library directly.
