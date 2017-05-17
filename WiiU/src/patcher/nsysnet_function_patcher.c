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
#include "nsysnet_function_patcher.h"

#include "utils/logger.h"

/*
DECL(s32, send, s32 s, const void *buffer, s32 size, s32 flags){
    char *result;
    bin_to_strhex((unsigned char *)buffer, size, &result);
    if(result != NULL){
        log_printf("[send]     socket: %08X, flags %08X size: %08X data: %s\n",s,flags,size,result);
    }else{
        log_printf("[send]     socket: %08X, flags %08X size: %08X data: malloc failed\n",s,flags,size);
    }
    return real_send(s,buffer,size,flags);

}

DECL(s32, recv, s32 s, void *buffer, s32 size, s32 flags){
    s32 result_size = real_recv(s,buffer,size,flags);
    char *result;
    bin_to_strhex((unsigned char *)buffer, result_size, &result);
    if(result != NULL){
        log_printf("[recv]     socket: %08X, flags %08X size: %08X data: %s\n",s,flags,result_size,result);
    }else{
        log_printf("[recv]     socket: %08X, flags %08X size: %08X data: malloc failed\n",s,flags,result_size);
    }
    return result_size;
}*/

DECL(s32, recvfrom,s32 s, void *buffer, s32 size, s32 flags,struct sockaddr *src_addr, s32 *addrlen){
    s32 result_size = real_recvfrom(s,buffer,size,flags,src_addr,addrlen);
    char *result;
    bin_to_strhex((unsigned char *)buffer, result_size, &result);
    char *ip = NULL;
    u16 port = 0;
    if(*addrlen > 0 && src_addr != NULL ){
        ip = inet_ntoa(((struct sockaddr_in *)src_addr)->sin_addr);
        port = ((struct sockaddr_in *)src_addr)->sin_port;
    }
    if(result != NULL){
        if(ip == NULL){
            log_printf("[recvfrom] socket: %08X, flags %08X size: %08X data: %s\n",s,flags,result_size,result);
        }else{
            log_printf("[recvfrom] socket: %08X, addr: %s:%d , flags %08X size: %08X data: %s\n",s,ip,port,flags,result_size,result);
        }
    }else{
        log_printf("[recvfrom] socket: %08X, flags %08X size: %08X data: malloc failed\n",s,flags,result_size,result);
    }
    return result_size;
}

DECL(s32, sendto, s32 s, const void *buffer, s32 size, s32 flags, const struct sockaddr *dest, s32 dest_len){
    char *result;
    bin_to_strhex((unsigned char *)buffer, size, &result);
    if(result != NULL){
        char *ip = NULL;
        u16 port = 0;
        if(dest_len > 0 && dest != NULL){
            ip = inet_ntoa(((struct sockaddr_in *)dest)->sin_addr);
            port = ((struct sockaddr_in *)dest)->sin_port;
        }
        if(ip == NULL){
            log_printf("[sendto]   socket: %08X, flags %08X size: %08X data: %s\n",s,flags,size,result);
        }else{
            log_printf("[sendto]   socket: %08X, addr: %s:%d, flags %08X size: %08X data: %s\n",s,ip,port,flags,size,result);
        }
    }else{
        log_printf("[sendto]   socket: %08X, flags %08X size: %08X data: malloc failed\n",s,flags,size,result);
    }
    return real_sendto(s,buffer,size,flags,dest,dest_len);
}

DECL(s32, NSSLWrite, s32 connection, const void* buf, s32 len,s32 * written){
    if(!logNSSLWrite) return real_NSSLWrite(connection,buf,len,written);
    char *decrypted_input_data;
    bin_to_strhex((unsigned char *)buf, len, &decrypted_input_data);

    s32 result = real_NSSLWrite(connection,buf,len,written);

    if(*written > 0){
        if(decrypted_input_data != NULL){
            log_printf("[NSSLWrite] connection %08X len: %08X written %08X data: %s\n",connection,len,*written,decrypted_input_data);
        }else{
            log_printf("[NSSLWrite] connection %08X len: %08X written %08X data: malloc failed\n",connection,len,*written);
        }
    }else{
        log_printf("[NSSLWrite] connection %08X len: %08X written %08X\n",connection,len,*written);
    }

    free(decrypted_input_data);

    return result;
}

DECL(s32, NSSLRead, s32 connection, const void* buf, s32 len,s32 * read){
    if(!logNSSLRead) return real_NSSLRead(connection,buf,len,read);
    s32 result = real_NSSLRead(connection,buf,len,read);

    if(*read > 0){
        char *decrypted_output_data;
        bin_to_strhex((unsigned char *)buf, *read, &decrypted_output_data);

        if(decrypted_output_data != NULL ){
            log_printf("[NSSLRead] connection %08X len: %08X read %08X data: %s\n",connection,len,*read,decrypted_output_data);
        }else{
            log_printf("[NSSLRead] connection %08X len: %08X read %08X data: malloc failed\n",connection,len,*read);
        }

        free(decrypted_output_data);
    }else{
        log_printf("[NSSLRead] connection %08X len: %08X read %08X\n",connection,len,*read);
    }

    return result;
}

hooks_magic_t method_hooks_nsysnet[] __attribute__((section(".data"))) = {
    //MAKE_MAGIC(send, LIB_NSYSNET, STATIC_FUNCTION),
    //MAKE_MAGIC(recv, LIB_NSYSNET, STATIC_FUNCTION),
    MAKE_MAGIC(recvfrom, LIB_NSYSNET, STATIC_FUNCTION),
    MAKE_MAGIC(sendto, LIB_NSYSNET, STATIC_FUNCTION),
    MAKE_MAGIC(NSSLWrite, LIB_NSYSNET, STATIC_FUNCTION),
    MAKE_MAGIC(NSSLRead, LIB_NSYSNET, STATIC_FUNCTION),
};

u32 method_hooks_size_nsysnet __attribute__((section(".data"))) = sizeof(method_hooks_nsysnet) / sizeof(hooks_magic_t);

//! buffer to store our instructions needed for our replacements
volatile unsigned int method_calls_nsysnet[sizeof(method_hooks_nsysnet) / sizeof(hooks_magic_t) * FUNCTION_PATCHER_METHOD_STORE_SIZE] __attribute__((section(".data")));
