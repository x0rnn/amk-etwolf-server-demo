#ifndef ETWOLF_SERVER_DEMO_RECORD_H
#define ETWOLF_SERVER_DEMO_RECORD_H

// CLion is sometimes retarded.
#if 0
#define __attribute_pure__
#define __THROW
#define __THROWNL
#define __nonnull(x)
#define __wur
#define __attribute_const__
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include "../zlib/zlib.h"
#include "etwolf.h"

#define INTERMISSION_STOP_DELAY 5000

#define BUFFER_SIZE (1 << 15)

#define Q_COLOR_ESCAPE  '^'
#define Q_IsColorString(p)  (p && *(p) == Q_COLOR_ESCAPE && *((p) + 1) && *((p) + 1) != Q_COLOR_ESCAPE)

typedef struct {
	qboolean recording;
	qboolean waiting;
	qboolean compressed;
	FILE     *handle;
	gzFile   gzHandle;
	int      messageSequence;
	char     filename[MAX_OSPATH];
	qboolean connected;
} record_t;

struct {
	char        gameTime[18];
	int         demoCounter;
	char        demoPath[MAX_OSPATH];
	gamestate_t gameState;
	int         intermissionTime;
	qboolean    gameInitialized;
} svr;

void      SVR_Init(void);
void      SVR_Record(client_t *client);
void      SVR_StopRecord(client_t *client);
void      SVR_Record_f(void);
void      SVR_StopRecord_f(void);
void      SVR_Netchan_Transmit(client_t *client, msg_t *msg);
void      SVR_ExecuteClientMessage(client_t *cl, msg_t *msg);
int QDECL SVR_VM_Call(vm_t *vm, int callnum, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8, int a9, int a10, int a11, int a12);

record_t  records[MAX_CLIENTS];

cvar_t *svr_autoRecord;
cvar_t *svr_demoName;
cvar_t *svr_compress;

#endif