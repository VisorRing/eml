#ifndef ML_LOCALDB_H
#define ML_LOCALDB_H

#include "ml.h"
#include "ml-id.h"
#include "ustring.h"
#include "bdbmacro.h"
#include "filemacro.h"

class  MNode;
class  MlEnv;

class  MLLocalDb: public MLFunc {
 public:
    BDB*  db;
    FileMacro  lock;
    int64_t  limit;

    MLLocalDb (MlEnv* _mlenv): MLFunc (cMLLocalDbID, _mlenv) {
	db = NULL;
	limit = 10000;
    };
    virtual  ~MLLocalDb () {
	closedb ();
    };
    virtual void  dbPath (const ustring& name, bool fxserial, ustring& dbpath, ustring& lockpath) = 0;
    virtual void  openRead (const ustring& name, bool fxserial) = 0;
    virtual void  openRW (const ustring& name, bool fxserial) = 0;
    virtual void  closedb ();
};

class  MLLocalDbBtree: public MLLocalDb {
 public:
    MLLocalDbBtree (MlEnv* _mlenv): MLLocalDb (_mlenv) {};
    virtual  ~MLLocalDbBtree () {};

    virtual void  dbPath (const ustring& name, bool fxserial, ustring& dbpath, ustring& lockpath);
    virtual void  openRead (const ustring& name, bool fxserial);
    virtual void  openRW (const ustring& name, bool fxserial);
};

MNode*  ml_localdb_rw (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_localdb_x_read (bool fev, MNode* cell, MlEnv* mlenv, MLFunc* mobj);
MNode*  ml_localdb_x_write (bool fev, MNode* cell, MlEnv* mlenv, MLFunc* mobj);
MNode*  ml_localdb_x_delete (bool fev, MNode* cell, MlEnv* mlenv, MLFunc* mobj);
MNode*  ml_localdb_x_keys (bool fev, MNode* cell, MlEnv* mlenv, MLFunc* mobj);

#endif /* ML_LOCALDB_H */
