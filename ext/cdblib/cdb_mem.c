/* Public domain. */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include "readwrite.h"
#include "error.h"
#include "seek.h"
#include "byte.h"
#include "../cdb-0.75/cdb.h"
#include "uint32.h"

void  cdb_free_mem( struct cdb* c ){
    c->map = 0;
}

void  cdb_init_mem( struct cdb *c, unsigned char* data, uint32 size ){
    cdb_free(c);
    cdb_findstart(c);
    c->fd = -1;

    c->size = size;
    c->map = data;
}

