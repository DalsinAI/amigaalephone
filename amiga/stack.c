/* A Shell gives 4 KB of stack; libnix's startup swaps to this much instead
 * (Aleph One's renderer, Lua and the MML/XML parser are deep). */
unsigned long __stack = 1024 * 1024;

/* For the Version command: the port's release (package/VERSION). */
static const char amiga_version[] __attribute__((used)) = "$VER: AlephOne 1.6.2-amiga2 (07.11.2026)";
