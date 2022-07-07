#ifndef ML_DB_H
#define ML_DB_H

#include "ml.h"
#include "ml-id.h"
#include "ustring.h"
#include "bdbmacro.h"
#include "filemacro.h"

class  MNode;
class  MlEnv;

class  MLDb: public MLFunc {
 public:
    BDB*  db;
    FileMacro  lock;
    size_t  limit;

    MLDb (MlEnv* _mlenv): MLFunc (cMLDbID, _mlenv) {
	db = NULL;
	limit = 10000;
    };
    virtual  ~MLDb () {
	closedb ();
    };
    virtual void  dbPath (const ustring& name, bool fxserial, ustring& dbpath, ustring& lockpath) = 0;
    virtual void  openRead (const ustring& name, bool fxserial) = 0;
    virtual void  openRW (const ustring& name, bool fxserial) = 0;
    virtual void  closedb ();
};

class  MLDbBtree: public MLDb {
 public:
    MLDbBtree (MlEnv* _mlenv): MLDb (_mlenv) {};
    virtual  ~MLDbBtree () {};

    virtual void  dbPath (const ustring& name, bool fxserial, ustring& dbpath, ustring& lockpath);
    virtual void  openRead (const ustring& name, bool fxserial);
    virtual void  openRW (const ustring& name, bool fxserial);
};

class  MLDbHash: public MLDb {
 public:
    MLDbHash (MlEnv* _mlenv): MLDb (_mlenv) {};
    virtual  ~MLDbHash () {};

    virtual void  dbPath (const ustring& name, bool fxserial, ustring& dbpath, ustring& lockpath);
    virtual void  openRead (const ustring& name, bool fxserial);
    virtual void  openRW (const ustring& name, bool fxserial);
};

MNode*  ml_db_read (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_db_rw (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_hash_read (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_hash_rw (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_db_x_read (bool fev, MNode* cell, MlEnv* mlenv, MLFunc* mobj);
MNode*  ml_db_x_write (bool fev, MNode* cell, MlEnv* mlenv, MLFunc* mobj);
MNode*  ml_db_x_delete (bool fev, MNode* cell, MlEnv* mlenv, MLFunc* mobj);
MNode*  ml_db_x_db_select (bool fev, MNode* cell, MlEnv* mlenv, MLFunc* mobj);
MNode*  ml_temp_read (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_temp_write (bool fev, MNode* cell, MlEnv* mlenv);

#endif /* ML_DB_H */
