/****************************************************************************
 * Copyright (C) 2017 Maschell
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ****************************************************************************/

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "common/retain_vars.h"
#include "game_function_patcher.h"

#include "utils/logger.h"
#include "utils/utils.h"

#define RC4ENCRYPTION_DIRECTION_ENCRYPT 0
#define RC4ENCRYPTION_DIRECTION_DECRYPT 1

DECL(u32,nn_nex_RC4Encryption_EncryptDecrypt,void * unkwn1/* probably a nn::nex::Buffer */, void * input_output, u32 length, u32 length2){
    char *in_result;

    bin_to_strhex((unsigned char *)input_output, length, &in_result);
    if(in_result != NULL){
        if(encryptionDirection == RC4ENCRYPTION_DIRECTION_ENCRYPT){
            log_printf("[RC4] decrypted input : %s\n",in_result);
        }else{
            log_printf("[RC4] encrypted input: %s\n",in_result);
        }
    }
    free(in_result);

    u32 result = real_nn_nex_RC4Encryption_EncryptDecrypt(unkwn1,input_output,length,length2);

    char *out_result;
    bin_to_strhex((unsigned char *)input_output, length, &out_result);
    if(out_result != NULL){
        if(encryptionDirection == RC4ENCRYPTION_DIRECTION_DECRYPT){
            log_printf("[RC4] decrypted output: %s\n",out_result);
        }else{
            log_printf("[RC4] encrypted output : %s\n",out_result);
        }
    }
    free(out_result);

    return result;
}


DECL(void *,nn_nex_RC4Encryption_Encrypt,void * a,void * b){
    encryptionDirection = RC4ENCRYPTION_DIRECTION_ENCRYPT; //Setting the encryption direction and use the EncryptDecrypt Hook
    //Data should be ((u32*)b)[4]  length in ((u32*)b)[5] Not working... data is slightly off.
    return real_nn_nex_RC4Encryption_Encrypt(a,b); //is calling nn_nex_RC4Encryption_EncryptDecrypt
}

DECL(void * ,nn_nex_RC4Encryption_Decrypt,void * a,void * b){
    encryptionDirection = RC4ENCRYPTION_DIRECTION_DECRYPT; //Setting the encryption direction and use the EncryptDecrypt Hook
    void * res = real_nn_nex_RC4Encryption_Decrypt(a,b); //is calling nn_nex_RC4Encryption_EncryptDecrypt
    //Data should be ((u32*)b)[4]  length in ((u32*)b)[5] Not working... data is slightly off.
    return res;
}

/*
DECL(void * ,nn_nex_EncryptionAlgorithm_Encrypt,void * a,void * b){
    log_printf("nn_nex_EncryptionAlgorithm_Encrypt\n");
    return real_nn_nex_EncryptionAlgorithm_Encrypt(a,b);
}
DECL(void * ,nn_nex_EncryptionAlgorithm_Decrypt,void * a,void * b){
    log_printf("nn_nex_EncryptionAlgorithm_Decrypt\n");
    return real_nn_nex_EncryptionAlgorithm_Decrypt(a,b);
}*/


hooks_magic_t method_hooks_game[] __attribute__((section(".data"))) = {
    MAKE_MAGIC_REAL(nn_nex_RC4Encryption_Encrypt),
    MAKE_MAGIC_REAL(nn_nex_RC4Encryption_Decrypt),
    MAKE_MAGIC_REAL(nn_nex_RC4Encryption_EncryptDecrypt),
    //MAKE_MAGIC_REAL(nn_nex_EncryptionAlgorithm_Encrypt),
    //MAKE_MAGIC_REAL(nn_nex_EncryptionAlgorithm_Decrypt),
};

u32 method_hooks_size_game __attribute__((section(".data"))) = sizeof(method_hooks_game) / sizeof(hooks_magic_t);

//! buffer to store our instructions needed for our replacements
volatile unsigned int method_calls_game[sizeof(method_hooks_game) / sizeof(hooks_magic_t) * FUNCTION_PATCHER_METHOD_STORE_SIZE] __attribute__((section(".data")));
