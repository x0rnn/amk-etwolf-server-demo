#ifndef ETWOLF_SERVER_DEMO_HOOK_H
#define ETWOLF_SERVER_DEMO_HOOK_H

#include "etwolf.h"
#include "record.h"
#include "../subhook/subhook.h"

#define HOOK(a, b, c, d) subhook_t b; a = hook_function(&b, c, d); if (!a) return

#endif