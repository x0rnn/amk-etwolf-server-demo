#ifndef ETWOLF_SERVER_DEMO_RECORD_H
#define ETWOLF_SERVER_DEMO_RECORD_H

#define __attribute_pure__
#define __THROW
#define __nonnull(x)
#define __wur

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "etwolf.h"

typedef struct {
	qboolean recording;
	qboolean waiting;
	FILE     *handle;
	int      messageSequence;
} record_t;

void SVR_Init(void);
void SVR_Record(client_t *client);
void SVR_StopRecord(client_t *client);
void SVR_Record_f(void);
void SVR_StopRecord_f(void);
void SVR_Netchan_Transmit(client_t *client, msg_t *msg);
void SVR_ExecuteClientMessage(client_t *cl, msg_t *msg);
void SVR_SendClientGameState(client_t *client);
void SVR_SendMessageToClient(msg_t *msg, client_t *client);

record_t records[MAX_CLIENTS];

#endif