/* config.h for AmigaOS 3.x (68040/68060 + FPU) on OpenGPU's SDL 2: stands in
 * for the one configure makes. Software renderer only. */
#ifndef ALEPHONE_AMIGA_CONFIG_H
#define ALEPHONE_AMIGA_CONFIG_H

#define PACKAGE "AlephOne"
#define PACKAGE_NAME "AlephOne"
#define PACKAGE_TARNAME "AlephOne"
#define PACKAGE_VERSION "20230529"
#define VERSION "20230529"

#define HAVE_LUA 1          /* the bundled Lua 5.2 */
#define HAVE_SDL_IMAGE 1    /* SDL2_image.module: plugins' PNG/JPEG art */
/* The network game stays in, on the kit's SDL2_net: upstream's
 * DISABLE_NETWORKING no longer builds (AStream, used by the sound and
 * image loaders, and the HUD's network calls are behind it). */
#define HAVE_UNISTD_H 1

/* No OpenGL, FFmpeg, libsndfile, Vorbis, MAD, SMPEG, Speex, libpng,
 * zziplib, curl, miniupnpc or NFD. */

#endif
