#include "record.h"

/**
 * Returns client number from record.
 */
inline static int GetClientNumber(record_t *record) {
	return (record - records) / sizeof(record);
}

/**
 * Returns client from record.
 */
inline static client_t *GetClient(record_t *record) {
	return svs->clients + GetClientNumber(record);
}

/**
 * Returns record from client.
 */
inline static record_t *GetRecord(client_t *client) {
	return &records[client - svs->clients];
}

/**
 * Writes data to the demo.
 */
static void SVR_Write(record_t *record, void *buffer, size_t size) {

	if (!record->recording) {
		return;
	}

	int written = fwrite(buffer, 1, size, record->handle);

	if (written < size) {
		Com_Printf("%d should get written, actual: %d\n", size, written);
		SVR_StopRecord(GetClient(record));
	}

}

static void SVR_WriteDemoMessage(record_t *record, msg_t *msg) {

	int swLen;

	client_t *client = GetClient(record);

	swLen = LittleLong(client->netchan.outgoingSequence);
	SVR_Write(record, &swLen, 4);

	swLen = LittleLong(msg->cursize);
	SVR_Write(record, &swLen, 4);

	SVR_Write(record, msg->data, msg->cursize);

}

// This got inlined.
static void SVR_UpdateServerCommandsToClient( client_t *client, msg_t *msg ) {

	int i;

	for (i = client->reliableAcknowledge + 1 ; i <= client->reliableSequence ; i++) {
		MSG_WriteByte(msg, svc_serverCommand);
		MSG_WriteLong(msg, i);
		MSG_WriteString(msg, client->reliableCommands[i & (MAX_RELIABLE_COMMANDS - 1)]);
	}

	client->reliableSent = client->reliableSequence;

}

/**
 * Creates game state change message.
 */
static void SVR_CreateGameStateMessage(record_t *record, msg_t *msg, qboolean updateServerCommands) {

	int           i;
	entityState_t *ent;
	entityState_t nullstate;
	client_t      *client;

	client = GetClient(record);

	MSG_WriteLong(msg, client->lastClientCommand);

	if (updateServerCommands) {
		SVR_UpdateServerCommandsToClient(client, msg);
	}

	MSG_WriteByte(msg, svc_gamestate);
	MSG_WriteLong(msg, client->reliableSequence);

	for (i = 0 ; i < MAX_CONFIGSTRINGS ; i++) {

		if (sv->configstrings[i][0]) {
			MSG_WriteByte(msg, svc_configstring);
			MSG_WriteShort(msg, i);
			MSG_WriteBigString(msg, sv->configstrings[i]);
		}

	}

	memset(&nullstate, 0, sizeof(nullstate));

	for (i = 0 ; i < MAX_GENTITIES; i++) {

		ent = &sv->svEntities[i].baseline;

		if (!ent->number) {
			continue;
		}

		MSG_WriteByte(msg, svc_baseline);
		MSG_WriteDeltaEntity(msg, &nullstate, ent, qtrue);

	}

	MSG_WriteByte(msg, svc_EOF);
	MSG_WriteLong(msg, GetClientNumber(record));
	MSG_WriteLong(msg, sv->checksumFeed);

}

/**
 * Starts recording.
 */
void SVR_Record(client_t *client) {

	msg_t msg;
	char  bufData[MAX_MSGLEN];
	int   len;

	record_t *record = GetRecord(client);

	record->handle = fopen("demo.dm_84", "wb");

	record->recording = qtrue;
	record->waiting   = qtrue; // Await first non-delta snapshot.

	MSG_Init(&msg, bufData, sizeof(bufData));

	SVR_CreateGameStateMessage(record, &msg, qfalse);

	MSG_WriteByte(&msg, svc_EOF);

	len = LittleLong(client->reliableSequence - 1);
	SVR_Write(record, &len, 4);

	len = LittleLong(msg.cursize);
	SVR_Write(record, &len, 4);

	SVR_Write(record, msg.data, msg.cursize);

}

/**
 * Stops recording.
 */
void SVR_StopRecord(client_t *client) {

	int end          = -1;
	record_t *record = GetRecord(client);

	SVR_Write(record, &end, 4);
	SVR_Write(record, &end, 4);

	record->recording = qfalse;

	fclose(record->handle);

}

/**
 * Parses client number from argument list.
 */
static qboolean GetArgClientNum(char *command, int *clientNum) {

	if (Cmd_Argc() < 2) {
		Com_Printf("Usage: %s <slot>\n", command);
		return qfalse;
	}

	*clientNum = atoi(Cmd_Argv(1));

	if (*clientNum >= sv_maxclients->integer || svs->clients[*clientNum].state != CS_ACTIVE) {
		Com_Printf("Client %d is not active.\n", *clientNum);
		return qfalse;
	}

	return qtrue;

}

/**
 * Called once on startup.
 */
void SVR_Init(void) {

	SV_Init();

	Cmd_AddCommand("record", (xcommand_t) SVR_Record_f);
	Cmd_AddCommand("stoprecord", (xcommand_t) SVR_StopRecord_f);

	sv_maxclients = Cvar_Get("sv_maxclients", "20", CVAR_SERVERINFO | CVAR_LATCH);

}

/**
 * Record command handler.
 */
void SVR_Record_f(void) {

	int clientNum;

	if (!GetArgClientNum("record", &clientNum)) {
		return;
	}

	if (records[clientNum].recording) {
		Com_Printf("Already recording.\n");
		return;
	}

	SVR_Record(GetClient(&records[clientNum]));

}

/**
 * Stop record command handler.
 */
void SVR_StopRecord_f(void) {

	int clientNum;

	if (!GetArgClientNum("stoprecord", &clientNum)) {
		return;
	}

	if (!records[clientNum].recording) {
		Com_Printf("Not recording a demo.\n");
		return;
	}

	SVR_StopRecord(GetClient(&records[clientNum]));

}

/**
 * Captures the message to a client.
 */
void SVR_Netchan_Transmit(client_t *client, msg_t *msg) {

	record_t *record = GetRecord(client);

	if (record->recording && client->state == CS_ACTIVE) {

		if (record->waiting && (client->deltaMessage <= 0 || client->netchan.outgoingSequence - client->deltaMessage >= (PACKET_BACKUP - 3))) {
			record->waiting = qfalse;
		}

		if (!record->waiting) {

			msg_t msg_c = *msg;
			MSG_WriteByte(&msg_c, svc_EOF);
			SVR_WriteDemoMessage(record, &msg_c);

		}

	}

	SV_Netchan_Transmit(client, msg);

}

// TODO: Remove.
void SVR_ExecuteClientMessage(client_t *cl, msg_t *msg) {

	SV_ExecuteClientMessage(cl, msg);

	record_t *record = GetRecord(cl);

	// Enforce non-delta message while waiting for recording to start.
	if (record->recording && record->waiting) {
		cl->deltaMessage = -1;
	}

}

void SVR_SendClientGameState(client_t *client) {

	msg_t msg;
	char  bufData[MAX_MSGLEN];

	MSG_Init(&msg, bufData, sizeof(bufData));

	client->state         = CS_PRIMED;
	client->pureAuthentic = 0;
	client->gotCP         = qfalse;

	client->gamestateMessageNum = client->netchan.outgoingSequence;

	SVR_CreateGameStateMessage(GetRecord(client), &msg, qtrue);
	SV_SendMessageToClient(&msg, client);

}