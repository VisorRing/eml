#ifndef CDBOBJ_H
#define CDBOBJ_H

#include <string.h>
#include <sys/types.h>
extern "C" {
#include "cdb.h"
}

class CDBMem {
 public:
    struct cdb  db;

    CDBMem (unsigned char* data, uint32_t size) {
	memset (&db, 0, sizeof (db));
	cdb_init_mem (&db, data, size);
    };
    virtual  ~CDBMem () {
	cdb_free_mem (&db);
    };
//    struct cdb*  operator () () {
//	return &db;
//    };
    virtual int  find (const char* p, unsigned int len) {
	return cdb_find (&db, (char*)p, len);
    };
    virtual uint32_t  datapos () {
	return db.dpos;
    };
    virtual uint32_t  datalen () {
	return db.dlen;
    };
    virtual int  read (char* bufp) {
	return cdb_read (&db, bufp, datalen (), datapos ());
    };
};

#endif /* CDBOBJ_H */
