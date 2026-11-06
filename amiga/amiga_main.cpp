/* A Workbench start of Aleph One: the arguments a Shell would give.
 *
 * From a Shell, "AlephOne Work:Marathon2" names the scenario drawer as on
 * any other system. From Workbench there is no command line, so this makes
 * one: the program's name, then a DATA tool type (DATA=Work:Marathon2) on
 * the program's icon, then any icons given with it (a scenario's drawer, a
 * film, a saved game or a plugin, by shift-clicking or as a project's
 * default tool), each as its full AmigaDOS name. */
#include <cstring>
#include <cstdlib>
#include <string>
#include <vector>

#include <exec/types.h>
#include <workbench/startup.h>
#include <workbench/workbench.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/icon.h>
#include <intuition/intuition.h>
#include <proto/intuition.h>

#include "amiga_main.h"

struct Library* IconBase;

static std::string lock_name(BPTR lock, const char* name)
{
	char buf[512];
	if (!lock || !NameFromLock(lock, (STRPTR)buf, sizeof(buf)))
		return name ? name : "";
	if (name && *name)
		AddPart((STRPTR)buf, (CONST_STRPTR)name, sizeof(buf));
	return buf;
}

void amiga_workbench_args(int* argc, char*** argv)
{
	if (*argc != 0)
		return;
	struct WBStartup* wb = (struct WBStartup*)*argv;
	static std::vector<std::string> words;
	static std::vector<char*> ptrs;
	words.push_back("AlephOne");

	if (wb && wb->sm_NumArgs > 0) {
		struct WBArg* self = &wb->sm_ArgList[0];
		IconBase = OpenLibrary((CONST_STRPTR)"icon.library", 36);
		if (IconBase) {
			BPTR old = CurrentDir(self->wa_Lock);
			struct DiskObject* dob = GetDiskObject((STRPTR)self->wa_Name);
			if (dob) {
				const char* data = (const char*)FindToolType((CONST_STRPTR*)dob->do_ToolTypes, (CONST_STRPTR)"DATA");
				if (data && *data)
					words.push_back(data);
				FreeDiskObject(dob);
			}
			CurrentDir(old);
			CloseLibrary(IconBase);
			IconBase = NULL;
		}
		if (self->wa_Name && *self->wa_Name)
			words[0] = (const char*)self->wa_Name;
		for (LONG i = 1; i < wb->sm_NumArgs; i++) {
			struct WBArg* a = &wb->sm_ArgList[i];
			/* a drawer comes as its lock with an empty name */
			words.push_back(lock_name(a->wa_Lock, (const char*)a->wa_Name));
		}
	}

	for (auto& w : words)
		ptrs.push_back(&w[0]);
	ptrs.push_back(NULL);
	*argc = (int)words.size();
	*argv = ptrs.data();
}

void amiga_alert_requester(const char* title, const char* message)
{
	struct IntuitionBase* IntuitionBase = (struct IntuitionBase*) OpenLibrary((CONST_STRPTR) "intuition.library", 37);
	if (!IntuitionBase)
		return;
	struct EasyStruct es;
	es.es_StructSize = sizeof(es);
	es.es_Flags = 0;
	es.es_Title = (CONST_STRPTR) title;
	es.es_TextFormat = (CONST_STRPTR) "%s";
	es.es_GadgetFormat = (CONST_STRPTR) "OK";
	// EasyRequest does not wrap: a long alert would run off the screen and
	// hide its button, so the text is broken into lines of up to 60.
	std::string text, word;
	size_t col = 0;
	for (const char* p = message;; p++) {
		if (*p && *p != ' ' && *p != '\n') {
			word += *p;
			continue;
		}
		if (!word.empty()) {
			if (col && col + 1 + word.size() > 60) {
				text += '\n';
				col = 0;
			} else if (col) {
				text += ' ';
				col++;
			}
			text += word;
			col += word.size();
			word.clear();
		}
		if (!*p)
			break;
		if (*p == '\n') {
			text += '\n';
			col = 0;
		}
	}
	APTR args[1] = { (APTR) text.c_str() };
	EasyRequestArgs(NULL, &es, NULL, args);
	CloseLibrary((struct Library*) IntuitionBase);
}
