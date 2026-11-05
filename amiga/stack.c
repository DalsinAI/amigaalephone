/* A Shell gives 4 KB of stack; libnix's startup swaps to this much instead
 * (Aleph One's renderer, Lua and the MML/XML parser are deep). */
unsigned long __stack = 1024 * 1024;
