#ifndef ETWOLF_SERVER_DEMO_ETWOLF_H
#define ETWOLF_SERVER_DEMO_ETWOLF_H

#define MAX_CHALLENGES                    1024
#define MAX_CLIENTS                       64
#define MAX_TEMPBAN_ADDRESSES             MAX_CLIENTS
#define SERVER_PERFORMANCECOUNTER_SAMPLES 6
#define MAX_EVENTS                        4
#define MAX_INFO_STRING                   1024
#define MAX_RELIABLE_COMMANDS             256
#define MAX_STRING_CHARS                  1024
#define MAX_BINARY_MESSAGE                32768
#define MAX_NAME_LENGTH                   36
#define MAX_QPATH                         64
#define MAX_DOWNLOAD_WINDOW               8
#define MAX_OSPATH                        256
#define MAX_STATS                         16
#define MAX_PERSISTANT                    16
#define MAX_POWERUPS                      16
#define MAX_WEAPONS                       64
#define MAX_MAP_AREA_BYTES                32
#define MAX_MSGLEN                        32768
#define PACKET_BACKUP                     32
#define PACKET_MASK                       (PACKET_BACKUP - 1)
#define MAX_CONFIGSTRINGS                 1024
#define GENTITYNUM_BITS                   10
#define MAX_GENTITIES                     (1 << GENTITYNUM_BITS)
#define MAX_MODELS                        256
#define MAX_ENT_CLUSTERS                  16
#define MAX_BPS_WINDOW                    20
#define MAX_SERVER_TAGS                   256
#define MAX_TAG_FILES                     64
#define PROTOCOL_VERSION                  84
#define MAX_NAME_LENGTH                   36

#define CVAR_ARCHIVE             1
#define CVAR_USERINFO            2
#define CVAR_SERVERINFO          4
#define CVAR_SYSTEMINFO          8
#define CVAR_INIT                16
#define CVAR_LATCH               32
#define CVAR_ROM                 64
#define CVAR_USER_CREATED        128
#define CVAR_TEMP                256
#define CVAR_CHEAT               512
#define CVAR_NORESTART           1024
#define CVAR_WOLFINFO            2048
#define CVAR_UNSAFE              4096
#define CVAR_SERVERINFO_NOUPDATE 8192

#define LittleLong(x) (x)
#define QDECL

typedef float vec_t;
typedef vec_t vec3_t[3];
typedef enum {qfalse, qtrue} qboolean;

enum svc_ops_e {
	svc_bad,
	svc_nop,
	svc_gamestate,
	svc_configstring,
	svc_baseline,
	svc_serverCommand,
	svc_download,
	svc_snapshot,
	svc_EOF
};

typedef struct {
	int    trType;
	int    trTime;
	int    trDuration;
	vec3_t trBase;
	vec3_t trDelta;
} trajectory_t;

typedef struct entityState_s {
	int          number;
	int          eType;
	int          eFlags;
	trajectory_t pos;
	trajectory_t apos;
	int          time;
	int          time2;
	vec3_t       origin;
	vec3_t       origin2;
	vec3_t       angles;
	vec3_t       angles2;
	int          otherEntityNum;
	int          otherEntityNum2;
	int          groundEntityNum;
	int          constantLight;
	int          dl_intensity;
	int          loopSound;
	int          modelindex;
	int          modelindex2;
	int          clientNum;
	int          frame;
	int          solid;
	int          event;
	int          eventParm;
	int          eventSequence;
	int          events[MAX_EVENTS];
	int          eventParms[MAX_EVENTS];
	int          powerups;
	int          weapon;
	int          legsAnim;
	int          torsoAnim;
	int          density;
	int          dmgFlags;
	int          onFireStart, onFireEnd;
	int          nextWeapon;
	int          teamNum;
	int          effect1Time, effect2Time, effect3Time;
	int          aiState;
	int          animMovetype;
} entityState_t;

typedef enum {
	CS_FREE,
	CS_ZOMBIE,
	CS_CONNECTED,
	CS_PRIMED,
	CS_ACTIVE
} clientState_t;

typedef struct usercmd_s {
	int         serverTime;
	char        buttons;
	char        wbuttons;
	char        weapon;
	char        flags;
	int         angles[3];
	signed char forwardmove, rightmove, upmove;
	char        doubleTap;
	char        identClient;
} usercmd_t;

typedef struct {
	qboolean linked;
	int      linkcount;
	int      svFlags;
	int      singleClient;
	qboolean bmodel;
	vec3_t   mins, maxs;
	int      contents;
	vec3_t   absmin, absmax;
	vec3_t   currentOrigin;
	vec3_t   currentAngles;
	int      ownerNum;
	int      eventTime;
	int      worldflags;
	qboolean snapshotCallback;
} entityShared_t;

typedef struct {
	entityState_t  s;
	entityShared_t r;
} sharedEntity_t;

typedef struct cplane_s {
	vec3_t normal;
	float  dist;
	char   type;
	char   signbits;
	char   pad[2];
} cplane_t;

typedef struct {
	qboolean allsolid;
	qboolean startsolid;
	float    fraction;
	vec3_t   endpos;
	cplane_t plane;
	int      surfaceFlags;
	int      contents;
	int      entityNum;
} trace_t;

typedef struct playerState_s {
	int      commandTime;
	int      pm_type;
	int      bobCycle;
	int      pm_flags;
	int      pm_time;
	vec3_t   origin;
	vec3_t   velocity;
	int      weaponTime;
	int      weaponDelay;
	int      grenadeTimeLeft;
	int      gravity;
	float    leanf;
	int      speed;
	int      delta_angles[3];
	int      groundEntityNum;
	int      legsTimer;
	int      legsAnim;
	int      torsoTimer;
	int      torsoAnim;
	int      movementDir;
	int      eFlags;
	int      eventSequence;
	int      events[MAX_EVENTS];
	int      eventParms[MAX_EVENTS];
	int      oldEventSequence;
	int      externalEvent;
	int      externalEventParm;
	int      externalEventTime;
	int      clientNum;
	int      weapon;
	int      weaponstate;
	int      item;
	vec3_t   viewangles;
	int      viewheight;
	int      damageEvent;
	int      damageYaw;
	int      damagePitch;
	int      damageCount;
	int      stats[MAX_STATS];
	int      persistant[MAX_PERSISTANT];
	int      powerups[MAX_POWERUPS];
	int      ammo[MAX_WEAPONS];
	int      ammoclip[MAX_WEAPONS];
	int      holdable[16];
	int      holding;
	int      weapons[MAX_WEAPONS / (sizeof(int) * 8)];
	vec3_t   mins, maxs;
	float    crouchMaxZ;
	float    crouchViewHeight, standViewHeight, deadViewHeight;
	float    runSpeedScale, sprintSpeedScale, crouchSpeedScale;
	int      viewlocked;
	int      viewlocked_entNum;
	float    friction;
	int      nextWeapon;
	int      teamNum;
	int      onFireStart;
	int      serverCursorHint;
	int      serverCursorHintVal;
	trace_t  serverCursorHintTrace;
	int      ping;
	int      pmove_framecount;
	int      entityEventSequence;
	int      sprintExertTime;
	int      classWeaponTime;
	int      jumpTime;
	int      weapAnim;
	qboolean releasedFire;
	float    aimSpreadScaleFloat;
	int      aimSpreadScale;
	int      lastFireTime;
	int      quickGrenTime;
	int      leanStopDebounceTime;
	int      weapHeat[MAX_WEAPONS];
	int      curWeapHeat;
	int      identifyClient;
	int      identifyClientHealth;
	int      aiState;
} playerState_t;

typedef struct {
	int           areabytes;
	char          areabits[MAX_MAP_AREA_BYTES];
	playerState_t ps;
	int           num_entities;
	int           first_entity;
	int           messageSent;
	int           messageAcked;
	int           messageSize;
} clientSnapshot_t;

typedef struct {
	int            type;
	unsigned char  ip[4];
	unsigned char  ipx[10];
	unsigned short port;
} netadr_t;

typedef struct {
	int      sock;
	int      dropped;
	netadr_t remoteAddress;
	int      qport;
	int      incomingSequence;
	int      outgoingSequence;
	int      fragmentSequence;
	int      fragmentLength;
	char     fragmentBuffer[MAX_MSGLEN];
	qboolean unsentFragments;
	int      unsentFragmentStart;
	int      unsentLength;
	char     unsentBuffer[MAX_MSGLEN];
} netchan_t;

typedef struct {
	qboolean allowoverflow;
	qboolean overflowed;
	qboolean oob;
	char     *data;
	int      maxsize;
	int      cursize;
	int      uncompsize;
	int      readcount;
	int      bit;
} msg_t;

typedef struct netchan_buffer_s {
	msg_t                   msg;
	char                    msgBuffer[MAX_MSGLEN];
	char                    lastClientCommandString[MAX_STRING_CHARS];
	struct netchan_buffer_s *next;
} netchan_buffer_t;

typedef struct client_s {
	clientState_t    state;
	char             userinfo[MAX_INFO_STRING];
	char             reliableCommands[MAX_RELIABLE_COMMANDS][MAX_STRING_CHARS];
	int              reliableSequence;
	int              reliableAcknowledge;
	int              reliableSent;
	int              messageAcknowledge;
	int              binaryMessageLength;
	char             binaryMessage[MAX_BINARY_MESSAGE];
	qboolean         binaryMessageOverflowed;
	int              gamestateMessageNum;
	int              challenge;
	usercmd_t        lastUsercmd;
	int              lastMessageNum;
	int              lastClientCommand;
	char             lastClientCommandString[MAX_STRING_CHARS];
	sharedEntity_t   *gentity;
	char             name[MAX_NAME_LENGTH];
	char             downloadName[MAX_QPATH];
	int              download;
	int              downloadSize;
	int              downloadCount;
	int              downloadClientBlock;
	int              downloadCurrentBlock;
	int              downloadXmitBlock;
	unsigned char    *downloadBlocks[MAX_DOWNLOAD_WINDOW];
	int              downloadBlockSize[MAX_DOWNLOAD_WINDOW];
	qboolean         downloadEOF;
	int              downloadSendTime;
	qboolean         bDlOK;
	char             downloadURL[MAX_OSPATH];
	qboolean         bWWWDl;
	qboolean         bWWWing;
	qboolean         bFallback;
	int              deltaMessage;
	int              nextReliableTime;
	int              lastPacketTime;
	int              lastConnectTime;
	int              nextSnapshotTime;
	qboolean         rateDelayed;
	int              timeoutCount;
	clientSnapshot_t frames[PACKET_BACKUP];
	int              ping;
	int              rate;
	int              snapshotMsec;
	int              pureAuthentic;
	qboolean         gotCP;
	netchan_t        netchan;
	netchan_buffer_t *netchan_start_queue;
	netchan_buffer_t *netchan_end_queue;
	int              downloadnotify;
} client_t;

typedef struct {
	netadr_t adr;
	int      challenge;
	int      time;
	int      pingTime;
	int      firstTime;
	int      firstPing;
	qboolean connected;
} challenge_t;

typedef struct tempBan_s {
	netadr_t adr;
	int      endtime;
} tempBan_t;

typedef struct {
	qboolean      initialized;
	int           time;
	int           snapFlagServerBit;
	client_t      *clients;
	int           numSnapshotEntities;
	int           nextSnapshotEntities;
	entityState_t *snapshotEntities;
	int           nextHeartbeatTime;
	challenge_t   challenges[MAX_CHALLENGES];
	netadr_t      redirectAddress;
	tempBan_t     tempBanAddresses[MAX_TEMPBAN_ADDRESSES];
	int           sampleTimes[SERVER_PERFORMANCECOUNTER_SAMPLES];
	int           currentSampleIndex;
	int           totalFrameTime;
	int           currentFrameIndex;
	int           serverLoad;
} serverStatic_t;

typedef enum {
	SS_DEAD,
	SS_LOADING,
	SS_GAME,
} serverState_t;

typedef struct svEntity_s {
	struct worldSector_s *worldSector;
	struct svEntity_s    *nextEntityInWorldSector;
	entityState_t        baseline;
	int                  numClusters;
	int                  clusternums[MAX_ENT_CLUSTERS];
	int                  lastCluster;
	int                  areanum, areanum2;
	int                  snapshotCounter;
	int                  originCluster;
} svEntity_t;

typedef struct worldSector_s {
	int                  axis;
	float                dist;
	struct worldSector_s *children[2];
	svEntity_t           *entities;
} worldSector_t;

typedef struct md3Tag_s {
	char   name[MAX_QPATH];
	vec3_t origin;
	vec3_t axis[3];
} md3Tag_t;

typedef struct {
	char filename[MAX_QPATH];
	int  start;
	int  count;
} tagHeaderExt_t;

typedef struct {
	serverState_t   state;
	qboolean        restarting;
	int             serverId;
	int             restartedServerId;
	int             checksumFeed;
	int             checksumFeedServerId;
	int             snapshotCounter;
	int             timeResidual;
	int             nextFrameTime;
	struct cmodel_s *models[MAX_MODELS];
	char            *configstrings[MAX_CONFIGSTRINGS];
	qboolean        configstringsmodified[MAX_CONFIGSTRINGS];
	svEntity_t      svEntities[MAX_GENTITIES];
	char            *entityParsePoint;
	sharedEntity_t  *gentities;
	int             gentitySize;
	int             num_entities;
	playerState_t   *gameClients;
	int             gameClientSize;
	int             restartTime;
	int             bpsWindow[MAX_BPS_WINDOW];
	int             bpsWindowSteps;
	int             bpsTotalBytes;
	int             bpsMaxBytes;
	int             ubpsWindow[MAX_BPS_WINDOW];
	int             ubpsTotalBytes;
	int             ubpsMaxBytes;
	float           ucompAve;
	int             ucompNum;
	md3Tag_t        tags[MAX_SERVER_TAGS];
	tagHeaderExt_t  tagHeadersExt[MAX_TAG_FILES];
	int             num_tagheaders;
	int             num_tags;
} server_t;

typedef struct cvar_s {
	char          *name;
	char          *string;
	char          *resetString;
	char          *latchedString;
	int           flags;
	qboolean      modified;
	int           modificationCount;
	float         value;
	int           integer;
	struct cvar_s *next;
	struct cvar_s *hashNext;
} cvar_t;

typedef struct vmSymbol_s {
	struct vmSymbol_s *next;
	int               symValue;
	int               profileCount;
	char              symName[1];
} vmSymbol_t;

typedef struct vm_s {
	int programStack;
	int (*systemCall)(int *parms);
	char name[MAX_QPATH];
	char fqpath[MAX_QPATH + 1];
	void *dllHandle;
	int (QDECL *entryPoint)(int callNum, ...);
	qboolean          currentlyInterpreting;
	qboolean          compiled;
	char              *codeBase;
	int               codeLength;
	int               *instructionPointers;
	int               instructionPointersLength;
	char              *dataBase;
	int               dataMask;
	int               stackBottom;
	int               numSymbols;
	struct vmSymbol_s *symbols;
	int               callLevel;
	int               breakFunction;
	int               breakCount;
} vm_t;

typedef enum {
	GAME_INIT,
	GAME_SHUTDOWN,
	GAME_CLIENT_CONNECT,
	GAME_CLIENT_BEGIN,
	GAME_CLIENT_USERINFO_CHANGED,
	GAME_CLIENT_DISCONNECT,
	GAME_CLIENT_COMMAND,
	GAME_CLIENT_THINK,
	GAME_RUN_FRAME,
	GAME_CONSOLE_COMMAND,
	GAME_SNAPSHOT_CALLBACK,
	BOTAI_START_FRAME,
	BOT_VISIBLEFROMPOS,
	BOT_CHECKATTACKATPOS,
	GAME_MESSAGERECEIVED,
} gameExport_t;

typedef enum {
	GS_INITIALIZE = -1,
	GS_PLAYING,
	GS_WARMUP_COUNTDOWN,
	GS_WARMUP,
	GS_INTERMISSION,
	GS_WAITING_FOR_PLAYERS,
	GS_RESET,
} gamestate_t;

typedef void (*xcommand_t)(void);

void      (*Cmd_AddCommand)(const char *cmd_name, xcommand_t function);
void      (*Com_Printf)(const char *msg, ...);
void      (*SV_Init)(void);
void      (*SV_Netchan_Transmit)(client_t *client, msg_t *msg);
void      (*SV_ExecuteClientMessage)(client_t *cl, msg_t *msg);
cvar_t*   (*Cvar_Get)(const char *var_name, const char *var_value, int flags);
int       (*Cmd_Argc)(void);
char*     (*Cmd_Argv)(int arg);
char*     (*Info_ValueForKey)(const char *s, const char *key);
int QDECL (*VM_Call)(vm_t *vm, int callnum, ...);

void (*MSG_Init)(msg_t *buf, char *data, int length);
void (*MSG_Bitstream)(msg_t *buf);
void (*MSG_WriteLong)(msg_t *sb, int c);
void (*MSG_WriteByte)(msg_t *sb, int c);
void (*MSG_WriteShort)(msg_t *sb, int c);
void (*MSG_WriteString)(msg_t *sb, const char *s);
void (*MSG_WriteBigString)(msg_t *sb, const char *s);
void (*MSG_WriteDeltaEntity)(msg_t *msg, struct entityState_s *from, struct entityState_s *to, qboolean force);

serverStatic_t *svs;
server_t       *sv;

cvar_t *sv_maxclients;
cvar_t *fs_basegame;
cvar_t *fs_basepath;
cvar_t *fs_homepath;
cvar_t *fs_gamedirvar;
cvar_t *gamestate;
cvar_t *mapname;

#endif