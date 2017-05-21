#ifndef RETAINS_VARS_H_
#define RETAINS_VARS_H_
#include <gctypes.h>
extern volatile u8 encryptionDirection;
extern volatile u8 logNSSLRead;
extern volatile u8 logNSSLWrite;
extern volatile u8 logRC4;
extern volatile u8 fullRC4;
extern volatile u8 log_recv;
extern volatile u8 log_recvfrom;
extern volatile u8 log_sendto;
extern volatile u8 log_UnreliableProtocol_sendImpl;
extern volatile u8 log_UnreliableProtocol_Receive;
extern volatile u8 log_PacketEncDec_Encode;
extern volatile u8 log_PacketEncDec_Decode;
#endif // RETAINS_VARS_H_
