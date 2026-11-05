/* Aleph One keys its byte swapping off ALEPHONE_LITTLE_ENDIAN (cseries.h,
 * from SDL_BYTEORDER). Marathon's data is big-endian like the 68k, so the
 * swaps must compile away here: stop the build if SDL's headers ever say
 * otherwise. */
#include "cseries.h"
#include "byte_swapping.h"

static_assert(SDL_BYTEORDER == SDL_BIG_ENDIAN, "SDL thinks the 68k is little-endian");
static_assert(!PlatformIsLittleEndian(), "ALEPHONE_LITTLE_ENDIAN is set on a big-endian 68k");
