#ifndef ETWOLF_SERVER_DEMO_RECORD_H
#define ETWOLF_SERVER_DEMO_RECORD_H

#define __attribute_pure__
#define __THROW
#define __THROWNL
#define __nonnull(x)
#define __wur

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <unistd.h>
#include "etwolf.h"

typedef struct {
	qboolean recording;
	qboolean waiting;
	FILE     *handle;
	int      messageSequence;
	char     filename[MAX_OSPATH];
} record_t;

struct {
	char gameTime[18];
	int  demoCounter;
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

#endif