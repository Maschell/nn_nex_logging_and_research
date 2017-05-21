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
    if(!logRC4) return real_nn_nex_RC4Encryption_EncryptDecrypt(unkwn1,input_output,length,length2);

    char *in_result;

    bin_to_strhex((unsigned char *)input_output, length, &in_result);
    if(in_result != NULL){
        if(encryptionDirection == RC4ENCRYPTION_DIRECTION_ENCRYPT){
            log_printf("[RC4] decrypted input : %s\n",in_result);
        }else{
            if(fullRC4) log_printf("[RC4] encrypted input: %s\n",in_result);
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
            if(fullRC4) log_printf("[RC4] encrypted output : %s\n",out_result);
        }
    }
    free(out_result);

    return result;
}


DECL(void *,nn_nex_RC4Encryption_Encrypt,void * a,void * b){
    //log_printf("nn_nex_RC4Encryption_Encrypt %08X\n",b);
    encryptionDirection = RC4ENCRYPTION_DIRECTION_ENCRYPT; //Setting the encryption direction and use the EncryptDecrypt Hook
    //Data should be ((u32*)b)[4]  length in ((u32*)b)[5] Not working... data is slightly off.
    return real_nn_nex_RC4Encryption_Encrypt(a,b); //is calling nn_nex_RC4Encryption_EncryptDecrypt
}

DECL(void * ,nn_nex_RC4Encryption_Decrypt,void * a,void * b){
    //log_printf("nn_nex_RC4Encryption_Decrypt %08X\n",b);
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


typedef struct _pia_crypto_settings{
    u32 unkwn; //Always 0x01
    u8* key;
    u32 keylength;
} pia_crypto_settings;

/*
Encrypted with AES ECB
*/
//static nn::pia::common::Crypto::Encrypt(void *, void const *, unsigned int, nn::pia::common::Crypto::Setting const &)
DECL(void *,nn_pia_common_Crypto_Encrypt,void * data_out,void * data_in,u32 length,pia_crypto_settings settings){
    //log_printf("nn_pia_common_Crypto_Encrypt %08X %08X %08X %08X\n",data_out,data_in,length,settings);

    char *in_result;

    bin_to_strhex((unsigned char *)data_in, length, &in_result);
    if(in_result != NULL){
        log_printf("[PIA] decrypted input : %s\n",in_result);
    }
    free(in_result);

    void * result = real_nn_pia_common_Crypto_Encrypt(data_out,data_in,length,settings);

    char *out_result;

    bin_to_strhex((unsigned char *)data_out, length, &out_result);
    if(out_result != NULL){
        log_printf("[PIA] encrypted output : %s\n",out_result);
    }
    free(out_result);
    return result;
}

/*
Decrypted with AES ECB
*/
//static nn::pia::common::Crypto::Decrypt(void *, void const *, unsigned int, nn::pia::common::Crypto::Setting const &)
DECL(void *,nn_pia_common_Crypto_Decrypt,void * data_out,void * data_in,u32 length,pia_crypto_settings settings){
    //log_printf("nn_pia_common_Crypto_Decrypt %08X %08X %08X %08X\n",data_out,data_in,length,settings);

    char *in_result;

    bin_to_strhex((unsigned char *)data_in, length, &in_result);
    if(in_result != NULL){
        log_printf("[PIA] encrypted input : %s\n",in_result);
    }
    free(in_result);

    void * result = real_nn_pia_common_Crypto_Decrypt(data_out,data_in,length,settings);

    char *out_result;

    bin_to_strhex((unsigned char *)data_out, length, &out_result);
    if(out_result != NULL){
        log_printf("[PIA] decrypted output : %s\n",out_result);
    }
    free(out_result);
    return result;
}

//nn::pia::transport::UnreliableProtocol::sendImpl((nn::pia::StationIndex, unsigned char const *, unsigned int))
DECL(void *,nn_pia_transport_UnreliableProtocol_sendImpl,void * unkwn,u32 stationIndex,u8 * data,u32 length){
    if(!log_UnreliableProtocol_sendImpl) return real_nn_pia_transport_UnreliableProtocol_sendImpl(unkwn,stationIndex,data,length);
    //log_printf("nn_pia_transport_UnreliableProtocol_sendImpl %08X %08X %08X %08X\n",unkwn,stationIndex,data,length);

    char *in_result;

    bin_to_strhex((unsigned char *)data, length, &in_result);
    if(in_result != NULL){
         log_printf("[PIA][%08X] sendImpl send payload: %s\n",stationIndex,in_result);
    }
    free(in_result);

    void * result = real_nn_pia_transport_UnreliableProtocol_sendImpl(unkwn,stationIndex,data,length);
    return result;
}

// nn::pia::transport::UnreliableProtocol::Receive((nn::pia::StationIndex *, unsigned char *, unsigned int *, unsigned int))
DECL(void *,nn_pia_transport_UnreliableProtocol_Receive,void * unkwn,u32 * stationIndex,u8 * recv_buf,u32 * recv_buf_read_size,u32 recv_buf_size){
    void * result = real_nn_pia_transport_UnreliableProtocol_Receive(unkwn,stationIndex,recv_buf,recv_buf_read_size,recv_buf_size);
    if(!log_UnreliableProtocol_Receive) return result;
    if(result != 0) return result;
    //log_printf("nn_pia_transport_UnreliableProtocol_Receive res(%08X) :%08X %08X %08X %08X %08X\n",result,unkwn,*stationIndex,recv_buf,*recv_buf_read_size,recv_buf_size);

    if(recv_buf != NULL && recv_buf_read_size != NULL){
        char *in_result;

        bin_to_strhex((unsigned char *)recv_buf, *recv_buf_read_size, &in_result);
        if(in_result != NULL){
            log_printf("[PIA][%08X]  Receive recv payload: %s\n",*stationIndex,in_result);
        }
        free(in_result);
    }

    return result;
}

// nn::nex::PacketEncDec::Decode((nn::nex::Packet *))
DECL(void *,nn_nex_PacketEncDec_Decode,void* unkown,void * packet){
    void * result = real_nn_nex_PacketEncDec_Decode(unkown,packet);
    if(!log_PacketEncDec_Decode) return result;
    log_printf("real_nn_nex_PacketEncDec_Decode res: %08X. packet: %08X\n",result,packet);
    return result;
}

// nn::nex::PacketEncDec::Encode((nn::nex::Packet *, nn::nex::Stream::Type, bool))
DECL(void *,nn_nex_PacketEncDec_Encode,void* unkown,void * packet,void * type,bool unkwn){
    void * result = real_nn_nex_PacketEncDec_Encode(unkown,packet,type,unkwn);
    if(!log_PacketEncDec_Encode) return result;
    log_printf("real_nn_nex_PacketEncDec_Encode res: %08X. packet: %08X type: %08X bool: %08X\n",result,packet,type,unkwn);
    return result;
}

DECL(void *,nn_nex_Packet_SetPayload,void* unkown,void * buffer){
    void * result = real_nn_nex_Packet_SetPayload(unkown,buffer);
    //log_printf("nn_nex_Packet_SetPayload res: %08X. %08X\n",result,buffer);
    return result;
}

//# nn::nex::PRUDPEndPoint::SendPacket((nn::nex::PacketOut *))
DECL(void *,nn_nex_PRUDPEndPoint_SendPacket,void* packetOut){
    void * result = real_nn_nex_PRUDPEndPoint_SendPacket(packetOut);
    //log_printf("nn_nex_PRUDPEndPoint_SendPacket %08X\n",packetOut);
    return result;
}

hooks_magic_t method_hooks_game[] __attribute__((section(".data"))) = {
    MAKE_MAGIC_REAL(nn_nex_RC4Encryption_Encrypt),
    MAKE_MAGIC_REAL(nn_nex_RC4Encryption_Decrypt),
    MAKE_MAGIC_REAL(nn_nex_RC4Encryption_EncryptDecrypt),
    MAKE_MAGIC_REAL(nn_pia_common_Crypto_Encrypt),
    MAKE_MAGIC_REAL(nn_pia_common_Crypto_Decrypt),
    MAKE_MAGIC_REAL(nn_pia_transport_UnreliableProtocol_sendImpl),
    MAKE_MAGIC_REAL(nn_pia_transport_UnreliableProtocol_Receive),
    MAKE_MAGIC_REAL(nn_nex_PacketEncDec_Encode),
    MAKE_MAGIC_REAL(nn_nex_PacketEncDec_Decode),
    MAKE_MAGIC_REAL(nn_nex_Packet_SetPayload),
    MAKE_MAGIC_REAL(nn_nex_PRUDPEndPoint_SendPacket),
    //MAKE_MAGIC_REAL(nn_nex_EncryptionAlgorithm_Encrypt),
    //MAKE_MAGIC_REAL(nn_nex_EncryptionAlgorithm_Decrypt),
};

u32 method_hooks_size_game __attribute__((section(".data"))) = sizeof(method_hooks_game) / sizeof(hooks_magic_t);

//! buffer to store our instructions needed for our replacements
volatile unsigned int method_calls_game[sizeof(method_hooks_game) / sizeof(hooks_magic_t) * FUNCTION_PATCHER_METHOD_STORE_SIZE] __attribute__((section(".data")));
