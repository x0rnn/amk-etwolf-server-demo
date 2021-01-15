#ifndef ETWOLF_SERVER_DEMO_HOOK_H
#define ETWOLF_SERVER_DEMO_HOOK_H

#include "etwolf.h"
#include "record.h"
#include "../subhook/subhook.h"

typedef struct {
	unsigned long address;
	void          *hook;
	void          *orig;
	subhook_t     handle;
} Hook;

#endif