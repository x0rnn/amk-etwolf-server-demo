#include "record.h"

/**
 * Returns client number from record.
 */
inline static int GetClientNumber(record_t *record) {
	return record - records;
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
 * Automatically stops the recording when writing fails.
 */
static void SVR_Write(record_t *record, void *buffer, size_t size) {

	if (!record->recording) {
		return;
	}

	int written = fwrite(buffer, 1, size, record->handle);

	if (written < size) {
		Com_Printf("Failed to write %d bytes into %s, stopping.\n", size - written, record->filename);
		SVR_StopRecord(GetClient(record));
	}

}

/**
 * CL_WriteDemoMessage counterpart.
 */
static void SVR_WriteDemoMessage(record_t *record, msg_t *msg) {

	int swLen;

	client_t *client = GetClient(record);

	swLen = LittleLong(client->netchan.outgoingSequence);
	SVR_Write(record, &swLen, 4);

	swLen = LittleLong(msg->cursize);
	SVR_Write(record, &swLen, 4);

	SVR_Write(record, msg->data, msg->cursize);

}

/**
 * Copied SV_UpdateServerCommandsToClient.
 * This was inlined in the vanilla binary.
 */
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
 * Generates an unique demo name.
 */
static qboolean SVR_DemoName(record_t *record) {

	client_t *client = GetClient(record);

	memset(record->filename, 0, sizeof(record->filename));
	sprintf(record->filename, "%s_%04d.dm_%d", svr.gameTime, svr.demoCounter++, PROTOCOL_VERSION);

	for (int i = 0; i < MAX_CLIENTS; i++) {

		// Pattern isn't unique enough? We can't let it write into the same file twice.
		if (GetClientNumber(&record[i]) != i && strcmp(records[i].filename, record->filename) == 0) {
			Com_Printf("Demo file %s is already in use.\n");
			return qfalse;
		}

	}

	return qtrue;

}

/**
 * Starts recording.
 */
void SVR_Record(client_t *client) {

	int           i;
	msg_t         msg;
	char          bufData[MAX_MSGLEN];
	char          demoPath[MAX_OSPATH];
	entityState_t *ent;
	entityState_t nullstate;
	int           len;

	record_t *record = GetRecord(client);

	if (!SVR_DemoName(record)) {
		return;
	}

	if (FS_CreatePath(record->filename)) {
		return;
	}

	strcpy(demoPath, svr.demoPath);
	strcat(demoPath, record->filename);

	record->handle = fopen(demoPath, "wb");

	if (record->handle == NULL) {
		Com_Printf("Could not open demo file: %s\n", record->filename);
		return;
	}

	record->recording = qtrue;
	record->waiting   = qtrue; // Await first non-delta snapshot.

	Com_Printf("Recording to %s\n", record->filename);

	MSG_Init(&msg, bufData, sizeof(bufData));

	MSG_WriteLong(&msg, client->lastClientCommand);

	MSG_WriteByte(&msg, svc_gamestate);
	MSG_WriteLong(&msg, client->reliableSequence);

	for (i = 0 ; i < MAX_CONFIGSTRINGS ; i++) {

		if (sv->configstrings[i][0]) {
			MSG_WriteByte(&msg, svc_configstring);
			MSG_WriteShort(&msg, i);
			MSG_WriteBigString(&msg, sv->configstrings[i]);
		}

	}

	memset(&nullstate, 0, sizeof(nullstate));

	for (i = 0 ; i < MAX_GENTITIES; i++) {

		ent = &sv->svEntities[i].baseline;

		if (!ent->number) {
			continue;
		}

		MSG_WriteByte(&msg, svc_baseline);
		MSG_WriteDeltaEntity(&msg, &nullstate, ent, qtrue);

	}

	MSG_WriteByte(&msg, svc_EOF);
	MSG_WriteLong(&msg, GetClientNumber(record));
	MSG_WriteLong(&msg, sv->checksumFeed);

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

	Com_Printf("Stopped demo %s\n", record->filename);

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
 * Called once on server startup.
 */
void SVR_Init(void) {

	SV_Init();

	Cmd_AddCommand("record", (xcommand_t) SVR_Record_f);
	Cmd_AddCommand("stoprecord", (xcommand_t) SVR_StopRecord_f);

	sv_maxclients = Cvar_Get("sv_maxclients", "20", CVAR_SERVERINFO | CVAR_LATCH);
	fs_basegame   = Cvar_Get("fs_basegame", "", CVAR_INIT);
	fs_basepath   = Cvar_Get("fs_basepath", "", CVAR_INIT);
	fs_homepath   = Cvar_Get("fs_homepath", "", CVAR_INIT);
	fs_gamedirvar = Cvar_Get("fs_game", "", CVAR_INIT | CVAR_SYSTEMINFO);

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
 * This is the best place to capture the message - it's complete
 * by now and it wasn't yet encoded for transmission.
 */
void SVR_Netchan_Transmit(client_t *client, msg_t *msg) {

	record_t *record = GetRecord(client);

	if (record->recording && client->state == CS_ACTIVE) {

		// We have a non-delta compressed frame, recording can begin.
		if (client->deltaMessage <= 0 || client->netchan.outgoingSequence - client->deltaMessage >= (PACKET_BACKUP - 3)) {
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

/**
 * SV_ExecuteClientMessage, but it makes the server send
 * a non-delta compressed snapshot when the recording starts.
 */
void SVR_ExecuteClientMessage(client_t *cl, msg_t *msg) {

	SV_ExecuteClientMessage(cl, msg);

	record_t *record = GetRecord(cl);

	// Enforce non-delta message while waiting for recording to start.
	if (record->recording && record->waiting) {
		cl->deltaMessage = -1;
	}

}

/**
 * Called on game initialization.
 * We use that to obtain the map time.
 */
static void SV_InitGame(int levelTime, int randomSeed, int restart) {

	char mod[32];

	time_t x = time(NULL);
	strftime(svr.gameTime, sizeof(svr.gameTime), "%Y-%m-%d-%H%M%S", localtime(&x));

	if (fs_gamedirvar->string[0]) {
		strcpy(mod, fs_gamedirvar->string);
	} else {
		strcpy(mod, "etmain");
	}

	sprintf(svr.demoPath, "%s/%s/demos/", fs_homepath->string, mod);

}

/**
 * Called on game shutdown.
 * We'll close all the demos here.
 */
static void SV_GameShutdown(qboolean restart) {

	if (restart) {
		return;
	}

	for (int i = 0; i < MAX_CLIENTS; i++) {

		if (records[i].recording) {
			SVR_StopRecord(GetClient(&records[i]));
		}

	}

}

/**
 * Called whenever a client disconnects.
 */
static void SV_ClientDisconnect(int clientNum) {

	record_t *record = &records[clientNum];

	if (record->recording) {
		SVR_StopRecord(GetClient(record));
	}

}

/**
 * Called whenever a client enters game world.
 */
static void SV_ClientBegin(int clientNum) {
}

/**
 * VM_Call hook.
 */
int QDECL SVR_VM_Call(vm_t *vm, int callnum, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8, int a9, int a10, int a11, int a12) {

	switch (callnum) {

		case GAME_INIT:
			SV_InitGame(a1, a2, a3);
			break;

		case GAME_SHUTDOWN:
			SV_GameShutdown(a1);
			break;

		case GAME_CLIENT_BEGIN:
			SV_ClientBegin(a1);
			break;

		case GAME_CLIENT_DISCONNECT:
			SV_ClientDisconnect(a1);
			break;

	}

	return VM_Call(vm, callnum, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12);

}