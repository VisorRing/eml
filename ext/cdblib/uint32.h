#ifndef UINT32_H
#define UINT32_H

#include <sys/types.h>

//typedef uint32_t  uint32;
#define uint32 uint32_t

extern void uint32_pack(char *,uint32);
extern void uint32_pack_big(char *,uint32);
extern void uint32_unpack(char *,uint32 *);
extern void uint32_unpack_big(char *,uint32 *);

#endif
