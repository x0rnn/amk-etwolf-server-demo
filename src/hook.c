#include "hook.h"

subhook_t subhook_sv_init;
subhook_t subhook_sv_netchan_transmit;
subhook_t subhook_sv_executeclientmessage;
subhook_t subhook_sv_sendclientgamestate;
subhook_t subhook_vm_call;

/**
 * Installs function trampoline.
 */
static void *hook_function(subhook_t *hook, unsigned long address, void *func) {
	*hook = subhook_new((void *) address, func, 0);
	subhook_install(*hook);
	return subhook_get_trampoline(*hook);
}

/**
 * Entry point.
 */
void __attribute__((constructor)) Construct() {

	// Hooks.
	SV_Init                 = hook_function(&subhook_sv_init, 0x8055F90, SVR_Init);
	SV_Netchan_Transmit     = hook_function(&subhook_sv_netchan_transmit, 0x8059F70, SVR_Netchan_Transmit);
	SV_ExecuteClientMessage = hook_function(&subhook_sv_executeclientmessage, 0x8051B80, SVR_ExecuteClientMessage);
	SV_SendClientGameState  = hook_function(&subhook_sv_sendclientgamestate, 0x804EB60, SVR_SendClientGameState);
	VM_Call                 = hook_function(&subhook_vm_call, 0x8087680, SVR_VM_Call);

	// Common functions.
	Cmd_AddCommand = (void *) 0x8069C30;
	Com_Printf     = (void *) 0x806C450;
	Cvar_Get       = (void *) 0x8071E40;
	Cmd_Argc       = (void *) 0x8069780;
	Cmd_Argv       = (void *) 0x8069790;

	// MSG functions.
	MSG_Init             = (void *) 0x807F7D0;
	MSG_Bitstream        = (void *) 0x807CA40;
	MSG_WriteLong        = (void *) 0x807CFE0;
	MSG_WriteByte        = (void *) 0x807CF80;
	MSG_WriteShort       = (void *) 0x807CFB0;
	MSG_WriteString      = (void *) 0x807E790;
	MSG_WriteBigString   = (void *) 0x807E8A0;
	MSG_WriteDeltaEntity = (void *) 0x807D860;

	// Server functions.
	SV_SendMessageToClient = (void *) 0x805A580;

	// FS functions.
	FS_CreatePath = (void *) 0x8072AE0;

	// Server state mapping.
	svs = (serverStatic_t *) 0x8874BA0;
	sv  = (server_t *)       0x880A420;

}