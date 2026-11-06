#ifndef ALEPHONE_AMIGA_MAIN_H
#define ALEPHONE_AMIGA_MAIN_H
/* amiga/amiga_main.cpp: a Workbench start's arguments (argc 0) as a
 * Shell's: the program name, a DATA tool type, and the icons given. */
void amiga_workbench_args(int* argc, char*** argv);
/* An Intuition requester on the Workbench screen, for alerts raised before
 * the game's screen is up (OpenGPU's SDL 2 has no message box). */
void amiga_alert_requester(const char* title, const char* message);
#endif
