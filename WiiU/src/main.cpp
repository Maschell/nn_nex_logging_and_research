#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include "main.h"
#include "common/common.h"
#include "common/os_defs.h"

#include "dynamic_libs/os_functions.h"
#include "dynamic_libs/gx2_functions.h"
#include "dynamic_libs/syshid_functions.h"
#include "dynamic_libs/vpad_functions.h"
#include "dynamic_libs/socket_functions.h"
#include "dynamic_libs/sys_functions.h"
#include "patcher/nsysnet_function_patcher.h"
#include "patcher/game_function_patcher.h"
#include "utils/function_patcher.h"
#include "kernel/kernel_functions.h"
#include "utils/logger.h"
#include "utils/logger.h"
#include "common/retain_vars.h"

u8 isFirstBoot __attribute__((section(".data"))) = 1;

/* Entry point */
extern "C" int Menu_Main(void){
    //!*******************************************************************
    //!                   Initialize function pointers                   *
    //!*******************************************************************
    //! aquire every rpl we want to patch

    InitOSFunctionPointers();
    InitSocketFunctionPointers(); //For logging

    InitSysFunctionPointers(); // For SYSLaunchMenu()

    //For patching
    InitVPadFunctionPointers();
    InitPadScoreFunctionPointers();

    SetupKernelCallback();

    log_init("192.168.0.181");

    //Disable/Enable logging of decrypted NSSL functions.
    logNSSLRead = 0;
    logNSSLWrite = 0;

    fullRC4 = 0;
    logRC4 = 1;

    log_recv = 1;
    log_recvfrom = 1;
    log_sendto = 1;

    log_UnreliableProtocol_sendImpl = 1;
    log_UnreliableProtocol_Receive = 1;

    log_PacketEncDec_Encode = 0;
    log_PacketEncDec_Decode = 0;

    u32 * main_entry_addr = (u32*)*((u32*)OS_SPECIFICS->addr_OSTitle_main_entry);

    if(OSGetTitleID()== 0x000500001010ED00){ //Mario Kart 8, values from update v64
        log_print("Mario Kart 8 EUR: Patching functions\n");
        u32 startAddressInRPX = 0x026774B8;

        setRealAddressByName(method_hooks_game,        method_hooks_size_game, "nn_nex_RC4Encryption_Encrypt",                  (u32)(u32*)((u32)main_entry_addr + (0x0291A44C-startAddressInRPX)));
        setRealAddressByName(method_hooks_game,        method_hooks_size_game, "nn_nex_RC4Encryption_Decrypt",                  (u32)(u32*)((u32)main_entry_addr + (0x0291A4DC-startAddressInRPX)));
        setRealAddressByName(method_hooks_game,        method_hooks_size_game, "nn_nex_RC4Encryption_EncryptDecrypt",           (u32)(u32*)((u32)main_entry_addr + (0x0291A2B8-startAddressInRPX)));

        setRealAddressByName(method_hooks_game,        method_hooks_size_game, "nn_pia_common_Crypto_Encrypt",                  (u32)(u32*)((u32)main_entry_addr + (0x029F423C-startAddressInRPX)));
        setRealAddressByName(method_hooks_game,        method_hooks_size_game, "nn_pia_common_Crypto_Decrypt",                  (u32)(u32*)((u32)main_entry_addr + (0x029F436C-startAddressInRPX)));

        setRealAddressByName(method_hooks_game,        method_hooks_size_game, "nn_nex_PacketEncDec_Encode",                    (u32)(u32*)((u32)main_entry_addr + (0x0293F014-startAddressInRPX)));
        setRealAddressByName(method_hooks_game,        method_hooks_size_game, "nn_nex_PacketEncDec_Decode",                    (u32)(u32*)((u32)main_entry_addr + (0x0293EDA0-startAddressInRPX)));

        setRealAddressByName(method_hooks_game,        method_hooks_size_game, "nn_pia_transport_UnreliableProtocol_sendImpl",  (u32)(u32*)((u32)main_entry_addr + (0x02A38644-startAddressInRPX)));
        setRealAddressByName(method_hooks_game,        method_hooks_size_game, "nn_pia_transport_UnreliableProtocol_Receive",   (u32)(u32*)((u32)main_entry_addr + (0x02A3833C-startAddressInRPX)));

        setRealAddressByName(method_hooks_game,        method_hooks_size_game, "nn_nex_Packet_SetPayload",                      (u32)(u32*)((u32)main_entry_addr + (0x02900898-startAddressInRPX)));
        setRealAddressByName(method_hooks_game,        method_hooks_size_game, "nn_nex_PRUDPEndPoint_SendPacket",               (u32)(u32*)((u32)main_entry_addr + (0x0294D000-startAddressInRPX)));
    }else if(OSGetTitleID()== 0x0005000010138300){ //Donkey Kong EUR, values from update v17
        log_print("Donkey Kong EUR: Patching functions\n");

        u32 startAddressInRPX = 0x2EEA934;              //Address of DK:TP start function in rs10_production.rpx v17

        setRealAddressByName(method_hooks_game,        method_hooks_size_game, "nn_nex_RC4Encryption_Encrypt",                  (u32)(u32*)((u32)main_entry_addr + (0x02E78678-startAddressInRPX)));
        setRealAddressByName(method_hooks_game,        method_hooks_size_game, "nn_nex_RC4Encryption_Decrypt",                  (u32)(u32*)((u32)main_entry_addr + (0x02E78708-startAddressInRPX)));
        setRealAddressByName(method_hooks_game,        method_hooks_size_game, "nn_nex_RC4Encryption_EncryptDecrypt",           (u32)(u32*)((u32)main_entry_addr + (0x02E784F8-startAddressInRPX)));
    }

    ApplyPatches();

    //Reset everything when were going back to the Mii Maker
    if(!isFirstBoot && isInMiiMakerHBL()){
        log_print("Returing to the Homebrew Launcher!\n");
        isFirstBoot = 0;
        deInit();
        return EXIT_SUCCESS;
    }

    //!*******************************************************************
    //!                        Patching functions                        *
    //!*******************************************************************

    if(!isInMiiMakerHBL()){ //Starting the application
        return EXIT_RELAUNCH_ON_LOAD;
    }

    if(isFirstBoot){ // First boot back to SysMenu
        isFirstBoot = 0;
        SYSLaunchMenu();
        return EXIT_RELAUNCH_ON_LOAD;
    }

    deInit();
    return EXIT_SUCCESS;
}

/*
    Patching all the functions!!!
*/
void ApplyPatches(){
    PatchInvidualMethodHooks(method_hooks_nsysnet,     method_hooks_size_nsysnet,      method_calls_nsysnet);
    PatchInvidualMethodHooks(method_hooks_game,        method_hooks_size_game,         method_calls_game);
}

/*
    Restoring everything!!
*/

void RestorePatches(){
    RestoreInvidualInstructions(method_hooks_nsysnet,  method_hooks_size_nsysnet);
    RestoreInvidualInstructions(method_hooks_game,     method_hooks_size_game);
    KernelRestoreInstructions();
}

void deInit(){
    RestorePatches();
    log_deinit();
}

s32 isInMiiMakerHBL(){
    if (OSGetTitleID != 0 && (
            OSGetTitleID() == 0x000500101004A200 || // mii maker eur
            OSGetTitleID() == 0x000500101004A100 || // mii maker usa
            OSGetTitleID() == 0x000500101004A000 ||// mii maker jpn
            OSGetTitleID() == 0x0005000013374842))
        {
            return 1;
    }
    return 0;
}
