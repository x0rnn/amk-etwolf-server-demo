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
 * Removes color escape sequences from string.
 */
char *Q_CleanStr(char *string) {

	char* d;
	char* s;
	int   c;

	s = string;
	d = string;

	while ((c = *s) != 0) {

		if (Q_IsColorString(s)) {
			s++;
		} else if (c >= 0x20 && c <= 0x7E) {
			*d++ = c;
		}

		s++;

	}

	*d = '\0';

	return string;

}

/**
 * Closes handle.
 */
static void SVR_Close(record_t *record) {

	if (!record->recording) {
		return;
	}

	record->recording = qfalse;

	if (record->compressed) {
		gzclose(record->gzHandle);
	} else {
		fclose(record->handle);
	}

}

/**
 * Writes data to the demo.
 * Automatically stops the recording when writing fails.
 */
static void SVR_Write(record_t *record, void *buffer, size_t size) {

	int written;

	if (!record->recording) {
		return;
	}

	if (record->compressed) {
		written = gzwrite(record->gzHandle, buffer, size);
	} else {
		written = fwrite(buffer, 1, size, record->handle);
	}

	if (written < size) {
		Com_Printf("Failed to write %d bytes into %s, stopping.\n", size - written, record->filename);
		SVR_Close(record);
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
 * Generates a name.
 */
static void SVR_DemoName(record_t *record, int order) {

	char   filename[MAX_OSPATH] = {0};
	int    pLen, i, j, k;
	time_t x;
	char   name[MAX_NAME_LENGTH];
	char   *guid;

	client_t *client = GetClient(record);

	pLen = strlen(svr_demoName->string);

	for (i = 0; i < pLen; i++) {

		if (svr_demoName->string[i] == '%' && i + 1 < pLen) {

			switch (svr_demoName->string[i + 1]) {

				case 'T':
					strcpy(filename + strlen(filename), svr.gameTime);
					break;

				case 't':
					x = time(NULL);
					strftime(filename + strlen(filename), 18, "%Y-%m-%d-%H%M%S", localtime(&x));
					break;

				case 'G':
				case 'g':

					guid = Info_ValueForKey(client->userinfo, "cl_guid");
					j    = strlen(guid);

					for (k = svr_demoName->string[i + 1] == 'G' ? 0 : 32 - 8; k < 32; k++) {

						if (k < j && (guid[k] >= '0' && guid[k] <= '9' || guid[k] >= 'a' && guid[k] <= 'f' || guid[k] >= 'A' && guid[k] <= 'F')) {
							filename[strlen(filename)] = guid[k];
						} else {
							filename[strlen(filename)] = '0';
						}

					}

					break;

				case 'n':

					memset(name, 0, sizeof(name));
					strcpy(name, client->name);
					Q_CleanStr(name);

					j = strlen(name);

					for (k = 0; k < j; k++) {

						if (name[k] >= '0' && name[k] <= '9' || name[k] >= 'A' && name[k] <= 'Z' || name[k] >= 'a' && name[k] <= 'z') {
							filename[strlen(filename)] = name[k];
						} else {
							filename[strlen(filename)] = '_';
						}

					}

					break;

				case 'p':
					sprintf(filename + strlen(filename), "%02d", GetClientNumber(record));
					break;

				case 'c':
					sprintf(filename + strlen(filename), "%04d", svr.demoCounter++);
					break;

				case 'm':
					strcat(filename + strlen(filename), mapname->string);
					break;

				default:
					i--; // Print an unknown character as is.

			}

			i++;
			continue;

		}

		filename[strlen(filename)] = svr_demoName->string[i];

	}

	memset(record->filename, 0, sizeof(record->filename));

	if (order == 0) {
		sprintf(record->filename, "%s.dm_%d", filename, PROTOCOL_VERSION);
	} else {
		sprintf(record->filename, "%s(%03d).dm_%d", filename, order, PROTOCOL_VERSION);
	}

	if (svr_compress->integer) {
		strcat(record->filename, ".gz");
	}

}

/**
 * Starts recording.
 */
void SVR_Record(client_t *client) {

	int           i;
	msg_t         msg;
	char          bufData[MAX_MSGLEN];
	char          demoPath[MAX_OSPATH];
	char          dir[MAX_OSPATH];
	entityState_t *ent;
	entityState_t nullstate;
	int           len;

	if (client->state != CS_ACTIVE) {
		return;
	}

	if (gamestate->integer < GS_PLAYING || gamestate->integer > GS_WARMUP) {
		return;
	}

	record_t *record = GetRecord(client);

	// We must not start the recording before the client is connected.
	if (record->recording || !record->connected) {
		return;
	}

	i = 0;

	do {

		if (i == 1000) {
			Com_Printf("Could not find a non-existent demo file %s\n", record->filename);
			return;
		}

		memset(demoPath, 0, sizeof(demoPath));
		SVR_DemoName(record, i++);
		strcpy(demoPath, svr.demoPath);
		strcat(demoPath, record->filename);

	} while(access(demoPath, F_OK) == 0);

	strcpy(dir, demoPath);

	for (i = strlen(dir) - 1; i >= 0; i--) {

		if (dir[i] == '/') {
			dir[i] = 0;
			break;
		}

	}

	if (mkdir(dir, 0700) != 0 && errno != EEXIST) {
		Com_Printf("Failed to create %s: %s\n", demoPath, strerror(errno));
		return;
	}

	record->compressed = svr_compress->integer;

	if (record->compressed) {
		record->gzHandle = gzopen(demoPath, "wb");
	} else {
		record->handle = fopen(demoPath, "wb");
	}

	if (record->compressed && record->gzHandle == NULL || !record->compressed && record->handle == NULL) {
		Com_Printf("Could not open demo file: %s\n", record->filename);
		return;
	}

	if (record->compressed) {
		gzbuffer(record->gzHandle, BUFFER_SIZE);
	} else {
		setvbuf(record->handle, NULL, _IOFBF, BUFFER_SIZE);
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

	if (!record->recording) {
		return;
	}

	SVR_Write(record, &end, 4);
	SVR_Write(record, &end, 4);

	SVR_Close(record);

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

	if (*clientNum < 0 || *clientNum >= sv_maxclients->integer || svs->clients[*clientNum].state != CS_ACTIVE || !records[*clientNum].connected) {
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
	gamestate     = Cvar_Get("gamestate", "-1", CVAR_WOLFINFO | CVAR_ROM);
	mapname       = Cvar_Get("mapname", "nomap", CVAR_SERVERINFO | CVAR_ROM);

	svr_autoRecord = Cvar_Get("svr_autorecord", "0", CVAR_ARCHIVE);
	svr_demoName   = Cvar_Get("svr_demoname", "%T-%m-%g-%n", CVAR_ARCHIVE);
	svr_compress   = Cvar_Get("svr_compress", "0", CVAR_ARCHIVE);

}

/**
 * Record command handler.
 */
void SVR_Record_f(void) {

	int clientNum;

	if (gamestate->integer < GS_PLAYING || gamestate->integer > GS_WARMUP) {
		Com_Printf("Game is not running.\n");
		return;
	}

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

	if (!restart || !svr.gameInitialized) {

		svr.gameInitialized = qtrue;

		time_t x = time(NULL);
		strftime(svr.gameTime, sizeof(svr.gameTime), "%Y-%m-%d-%H%M%S", localtime(&x));

		if (fs_gamedirvar->string[0]) {
			strcpy(mod, fs_gamedirvar->string);
		} else {
			strcpy(mod, "etmain");
		}

		sprintf(svr.demoPath, "%s/%s/demos/", fs_homepath->string, mod);

		svr.demoCounter = 0;
		svr.gameState   = GS_INITIALIZE;

	}

}

/**
 * Starts recording everyone.
 */
static void SV_StartRecordAll() {

	client_t *client;

	for (int i = 0; i < sv_maxclients->integer; i++) {

		client = GetClient(&records[i]);

		if (gamestate->integer < GS_WARMUP) {
			SVR_Record(client);
		}

	}

}

/**
 * Stops recording everyone.
 */
static void SV_StopRecordAll() {

	// Use MAX_CLIENTS here, just to be safe.
	for (int i = 0; i < MAX_CLIENTS; i++) {

		if (records[i].recording) {
			SVR_StopRecord(GetClient(&records[i]));
		}

	}

}

/**
 * Called on game shutdown.
 * We'll close all the demos here.
 */
static void SV_GameShutdown(qboolean restart) {

	if (restart) {
		return;
	}

	SV_StopRecordAll();

	// Reset all records.
	memset(records, 0, sizeof(records));

}

/**
 * Called on every frame.
 */
static void SV_RunFrame(int serverTime) {

	if (gamestate->integer != svr.gameState) {

		switch (gamestate->integer) {

			// Autorecord when countdown or game starts.
			case GS_WARMUP_COUNTDOWN:
			case GS_PLAYING:

				if (svr_autoRecord->integer) {
					SV_StartRecordAll();
				}

				break;

			// Intermission stop recording countdown begins.
			case GS_INTERMISSION:
				svr.intermissionTime = serverTime;
				break;

		}

		svr.gameState = gamestate->integer;

	}

	// Intermission stop record countdown elapsed.
	if (svr.intermissionTime && svr.gameState == GS_INTERMISSION && svr.intermissionTime + INTERMISSION_STOP_DELAY < serverTime) {
		svr.intermissionTime = 0;
		SV_StopRecordAll();
	}

}

/**
 * Called whenever a client disconnects.
 */
static void SV_ClientDisconnect(int clientNum) {

	record_t *record = &records[clientNum];
	record->connected = qfalse;

	if (record->recording) {
		SVR_StopRecord(GetClient(record));
	}

}

/**
 * Called whenever a client enters game world.
 */
static void SV_ClientBegin(int clientNum) {

	record_t *record = &records[clientNum];
	client_t *client = GetClient(record);

	if (svr_autoRecord->integer && !record->recording && client->state == CS_ACTIVE && record->connected) {

		switch (gamestate->integer) {

			case GS_PLAYING:
			case GS_WARMUP_COUNTDOWN:
				SVR_Record(client);
				break;

		}

	}

}

/**
 * Called whenever a client connects.
 */
static void SV_ClientConnect(int clientNum, qboolean firstTime, qboolean isBot) {
	records[clientNum].connected = qtrue;
}

/**
 * VM_Call hook.
 */
int QDECL SVR_VM_Call(vm_t *vm, int callnum, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8, int a9, int a10, int a11, int a12) {

	switch (callnum) {

		case GAME_INIT:
			SV_InitGame(a1, a2, a3);
			break;

		case GAME_RUN_FRAME:
			SV_RunFrame(a1);
			break;

		case GAME_SHUTDOWN:
			SV_GameShutdown(a1);
			break;

		case GAME_CLIENT_CONNECT:
			SV_ClientConnect(a1, a2, a3);
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