#include "heapdebug.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

extern "C"{
#ifdef BDB5
#include <db5/db_185.h>
#else
#ifdef Linux
#include <db_185.h>
#else
#include <db.h>
//#include "db_185+.h"
#endif
#endif
}
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/uio.h>

#define DB1	DB
#define DBT1	DBT
#define DB1_BTREE	DB_BTREE
#define DB1_HASH	DB_HASH

static void  ex ();
static void  initDB ();
class  HdInit {
public:
    HdInit () {
	initDB ();
    };
    virtual  ~HdInit () {
	ex ();
    };
};

static HdInit init;
static int Inited = 0;
static int Exited = 0;
static DB1*  db;

static void  initDB () {
#ifdef BDB5
    db = dbopen (NULL, O_CREAT | O_RDWR, 0, DB1_BTREE, NULL);
#else
    db = dbopen (NULL, O_RDWR, 0, DB1_BTREE, NULL);
#endif
    Inited = 1;
}

static void  hexDumpS (uint8_t* data, size_t size) {
    int n = size;
    char  ch;
    if (n > 48) n = 48;
    fprintf (stderr, "    :: ");
    for (int i = 0; i < n; ++ i) {
	if (isprint (data[i])) {
	    ch = data[i];
	} else {
	    ch = '.';
	}
	fprintf (stderr, "[%c|%.2x]", ch, data[i]);
    }
    fprintf (stderr, "\n");
}

static void  ex () {
    std::cout.flush ();
    std::cerr.flush ();
    if (db) {
#ifdef DEBUG2
	fprintf (stderr, "atexit()\n");
	fprintf (stderr, "pid:%d\n", getpid ());
#endif /* DEBUG */
	DBT1  key, val;
	int  rc;
	u_int  seqflag;
	seqflag = R_FIRST;
	while ((rc = (*db->seq) (db, &key, &val, seqflag)), (seqflag = R_NEXT), (rc == 0)) {
	    fprintf (stderr, "lost::  %.8x(%.6x|%ld) ",
		     *(u_int*)key.data, *(u_int*)val.data, *(size_t*)val.data);
	    hexDumpS (*(uint8_t**)key.data, *(size_t*)val.data);
	}
	(*db->close) (db);
	db = NULL;
	Inited = 0;
	Exited = 1;
    }
}

inline void  mdbAdd (void* ptr, size_t sz) {
    DBT1  key, val;

    if (!Inited)
	initDB ();
    key.data = &ptr;
    key.size = sizeof (void*);
    val.data = &sz;
    val.size = sizeof (size_t);
    (*db->put) (db, &key, &val, 0);
#ifdef HEAPDEBUG_VERBOSE
    fprintf (stderr, "malloc: %.8x(%.6x|%d)\n", ptr, sz, sz);
    fflush (stderr);
#endif
}

void  mdbDelete (void* ptr) {
    DBT1  key, val;

    if (! db)
	return;

    key.data = &ptr;
    key.size = sizeof (void*);
    if ((*db->get) (db, &key, &val, 0) == 0) {
#ifdef    HEAPDEBUG_VERBOSE
	fprintf (stderr, "free  : %.8x(%.6x|%d)\n", ptr, *(size_t*)val.data, *(size_t*)val.data);
	fflush (stderr);
#endif /* HEAPDEBUG_VERBOSE */
	(*db->del) (db, &key, 0);
    }else{
	fprintf (stderr, "no matching ptr: %p\n", ptr);
	fflush (stderr);
    }
}

void  mdbClose () {
    (*db->close) (db);
    db = NULL;
    Inited = 0;
    Exited = 1;
#ifdef DEBUG2
    fprintf (stderr, "mdbClose()\n");
#endif /* DEBUG */
}

#ifdef GCC2
void*  __builtin_new (size_t sz) {
    void*  p;

    if (sz == 0) {
	sz = 1;
    }
    p = (void*)malloc (sz);
    mdbAdd (p, sz);
    return p;
}

void  __builtin_delete (void* ptr) {
    if (ptr) {
	free (ptr);
	mdbDelete (ptr);
    }
}
#else
void*  operator new (size_t sz) //throw (bad_alloc)
{
  void *p;

  /* malloc (0) is unpredictable; avoid it.  */
  if (sz == 0)
    sz = 1;
  p = (void *) malloc (sz);
  mdbAdd (p, sz);
  return p;
}

//void  operator delete (void *ptr) //throw ()
void  operator delete (void *ptr) noexcept
{
    if (ptr) {
	free (ptr);
	mdbDelete (ptr);
    }
}
#endif
