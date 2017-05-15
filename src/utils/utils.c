#include <malloc.h>
#include "utils.h"

/**
    http://stackoverflow.com/a/17147874
**/
void bin_to_strhex(unsigned char *bin, unsigned int binsz, char **result){
  char          hex_str[]= "0123456789ABCDEF";
  unsigned int  i;

  *result = (char *)malloc(binsz * 2 + 1);
  if(*result == NULL) return;
  (*result)[binsz * 2] = 0;

  if (!binsz)
    return;

  for (i = 0; i < binsz; i++)    {
      (*result)[i * 2 + 0] = hex_str[(bin[i] >> 4) & 0x0F];
      (*result)[i * 2 + 1] = hex_str[(bin[i]     ) & 0x0F];
    }
}
