#ifndef BDBMACRO_H
#define BDBMACRO_H

#include "ustring.h"
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
#include <fcntl.h>
#include <exception>

#define DB1	DB
#define DBT1	DBT
#define DB1_BTREE	DB_BTREE
#define DB1_HASH	DB_HASH

class  BDB {
 public:
    DB1*  db;
    DBT1  key1, val1;
    u_int  seqflag;

    BDB (): db (NULL) {};
    virtual  ~BDB () {
	close ();
    };
    virtual void  close () {
	if (db) {
	    (*db->close) (db);
	    db = NULL;
	}
    };
    virtual void  put (DBT1* key, DBT1* val) {
	if (!db || !key)
	    return;
	if (val) {
	    (*db->put) (db, key, val, 0);
	} else {
	    (*db->del) (db, key, 0);
	}
    };
    virtual void  put (const u_char* key, size_t skey, const u_char* val, size_t sval) {
	if (!db)
	    return;
	key1.data = (void*)key;
	key1.size = skey;
	if (val) {
	    val1.data = (void*)val;
	    val1.size = sval;
	    (*db->put) (db, &key1, &val1, 0);
	} else {
	    (*db->del) (db, &key1, 0);
	}
    };
    virtual void  put (const ustring& key, const ustring& val) {
	if (!db)
	    return;
	key1.data = (void*)key.data ();
	key1.size = key.size ();
	val1.data = (void*)val.data ();
	val1.size = val.size ();
	(*db->put) (db, &key1, &val1, 0);
    };
    virtual void  del (DBT1* key) {
	if (!db || !key)
	    return;
	(*db->del) (db, key, 0);
    };
    virtual void  del (const u_char* key, size_t skey) {
	if (!db)
	    return;
	key1.data = (void*)key;
	key1.size = skey;
	(*db->del) (db, &key1, 0);
    };
    virtual void  del (const ustring& key) {
	if (!db)
	    return;
	key1.data = (void*)key.data ();
	key1.size = key.size ();
	(*db->del) (db, &key1, 0);
    };
    virtual int  get (DBT1* key, DBT1* val) {
	int  rc;
	if (db && key) {
	    rc = (*db->get) (db, key, val, 0);
	    if (rc) {
		val->data = NULL;
		val->size = 0;
	    }
	    return (rc == 0);		// success
	} else {
	    val->data = NULL;
	    val->size = 0;
	    return 0;			// failure
	}
    };
    virtual int  get (const ustring& key, ustring& val) {
	int  rc;
	if (db) {
	    key1.data = (void*)key.data ();
	    key1.size = key.size ();
	    rc = (*db->get) (db, &key1, &val1, 0);
	    if (rc) {
		val.resize (0);
	    } else {
		val.assign ((char*)val1.data, val1.size);
	    }
	    return (rc == 0);		// success
	} else {
	    val.resize (0);
	    return 0;			// failure
	}
    };
    virtual void  flush () {
	if (db)
	    (*db->sync) (db, 0);
    };
    virtual int  isempty () {
	if (db && (*db->seq) (db, &key1, &val1, R_FIRST) == 0) {
	    return 0;
	}else{
	    return 1;			// empty
	}
    };
    virtual void  initeach () {
	seqflag = R_FIRST;
    };
    virtual int  each (DBT1* key, DBT1* val) {
	int  rc;
	if (!db)
	    return (0);
	rc = (*db->seq) (db, key, val, seqflag);
	seqflag = R_NEXT;
	if (rc == 0) {
	    return 1;
	}else{
	    return 0;
	}
    };
    virtual int  each (ustring& key, ustring& val) {
	int  rc;
	if (!db)
	    return (0);
	rc = (*db->seq) (db, &key1, &val1, seqflag);
	seqflag = R_NEXT;
	if (rc == 0) {
	    key.assign ((char*)key1.data, key1.size);
	    val.assign ((char*)val1.data, val1.size);
	    return 1;
	}else{
	    return 0;
	}
    };
};

class  BDBHash: public BDB {
 public:
    void  open (const char* path) {
#if 0
	HASHINFO  info;

	info.bsize = 1024;
	info.ffactor = 0;
	info.nelem = 1;
	info.cachesize = 0;
	info.hash = NULL;
	info.lorder = 0;
	db = dbopen (path, O_RDWR | O_CREAT, 0666, DB1_HASH, &info);
#endif
	db = dbopen (path, O_RDWR | O_CREAT, 0666, DB1_HASH, NULL);
	if (! db)
	    throw (ustring (CharConst ("can't open database.")));
    };
};

class  BDBHashRDOnly: public BDB {
 public:
    void  open (const char* path) {
	db = dbopen (path, O_RDONLY, 0666, DB1_HASH, NULL);
	if (! db)
	    throw (ustring (CharConst ("can't open database.")));
    };
};

class  BDBBtree: public BDB {
 public:
    void  openRead (const char* path) {
	db = dbopen (path, O_RDONLY, 0666, DB1_BTREE, NULL);
	if (! db)
	    throw (ustring (CharConst ("can't open database.")));
    };
    void  openRW (const char* path) {
	db = dbopen (path, O_RDWR | O_CREAT, 0666, DB1_BTREE, NULL);
	if (! db)
	    throw (ustring (CharConst ("can't open database.")));
    };
};

#endif /* BDBMACRO_H */
