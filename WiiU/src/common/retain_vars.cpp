#include <gctypes.h>
#include "retain_vars.h"
volatile u8 encryptionDirection __attribute__((section(".data"))) = 0;
volatile u8 logNSSLRead __attribute__((section(".data"))) = 0;
volatile u8 logNSSLWrite __attribute__((section(".data"))) = 0;
volatile u8 fullRC4 __attribute__((section(".data"))) = 0;
volatile u8 logRC4 __attribute__((section(".data"))) = 0;
volatile u8 log_recv __attribute__((section(".data"))) = 0;
volatile u8 log_recvfrom __attribute__((section(".data"))) = 0;
volatile u8 log_sendto __attribute__((section(".data"))) = 0;
volatile u8 log_UnreliableProtocol_sendImpl __attribute__((section(".data"))) = 0;
volatile u8 log_UnreliableProtocol_Receive __attribute__((section(".data"))) = 0;

volatile u8 log_PacketEncDec_Encode __attribute__((section(".data"))) = 0;
volatile u8 log_PacketEncDec_Decode __attribute__((section(".data"))) = 0;

