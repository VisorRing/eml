#ifndef ML_SQLITE3_H
#define ML_SQLITE3_H

#include "ml.h"
#include "ml-id.h"
#include "ustring.h"
#include "motorconst.h"
#include <boost/ptr_container/ptr_vector.hpp>
extern "C" {
#ifdef BDB5
#include <db5/dbsql.h>
#else
#include "sqlite3.h"
#endif
}

class  MNode;
class  MlEnv;

class  ustringptr_vector: public std::vector<ustring*> {
 public:
    ustringptr_vector () {};
    virtual  ~ustringptr_vector () {
	clear ();
    };
    virtual void  clear () {
	ustringptr_vector::const_iterator  it;
	for (it = begin (); it < end (); it ++) {
	    if (*it != NULL) {
		delete (*it);
	    }
	}
	std::vector<ustring*>::clear ();
    };
//    virtual ustring*  at (size_type pos);
//    virtual ustring*  back ();
//	...
};

class  MLSqlite3: public MLFunc {
 public:
    typedef ustringptr_vector  namearray;
    class  fsqlParam {
    public:
	ustring  sql;
	MLSqlite3::namearray  bindName;
	MLSqlite3::namearray  bindValue;
	enum {
	    COL_VECTOR,
	    COL_VECTOR_VECTOR,
	}  answertype;
	
	fsqlParam () {
	    answertype = COL_VECTOR;
	};
	virtual  ~fsqlParam () {};
    };

    sqlite3*  dbh;
    size_t  limit;
    ustring  voverflow;
    sqlite3_stmt*  dbst;
    int  ncols;
    bool  fcreate;
    MNodePtr  errfunc;
    MNodePtr  initfunc;
    bool  finit;
    bool  fbegin;
    int64_t  retry;
    MNodePtr  blockSym;

    MLSqlite3 (MlEnv* _mlenv): MLFunc (cMLSqlite3ID, _mlenv) {
	dbh = NULL;
	limit = kARRAYMAX;
	dbst = NULL;
	fcreate = false;
	finit = false;
	fbegin = false;
	retry = 0;
    };
    virtual  ~MLSqlite3 () {
	close ();
    };

    virtual int  open (ustring& name);
    virtual void  close ();
    virtual void  finalize ();
    virtual int  prepare (const ustring& sql);
    virtual void  bind (namearray& name, namearray& value);
    virtual void  bind (namearray& value);
    virtual void  exec ();
    virtual void  step ();
    virtual bool  isReady () {
	return (dbst != NULL);
    };
    virtual MNode*  answer_vector ();
    virtual MNode*  answer_vector_ary ();
    virtual sqlite3_int64  rowid ();
    virtual void  postError (ustring msg);
    virtual int  sql_s (const ustring& query);
    virtual void  setBreak ();
    virtual ustring  errmsg ();
};

MNode*  ml_sqlite3 (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_sqlite3_sqlv (bool fev, MNode* cell, MlEnv* mlenv, MLFunc* mobj);
MNode*  ml_sqlite3_sqlv_ary (bool fev, MNode* cell, MlEnv* mlenv, MLFunc* mobj);
MNode*  ml_sqlite3_rowid (bool fev, MNode* cell, MlEnv* mlenv, MLFunc* mobj);
MNode*  ml_sqlite3_escape_like (bool fev, MNode* cell, MlEnv* mlenv, MLFunc* mobj);
MNode*  ml_sqlite3_begin_transaction (bool fev, MNode* cell, MlEnv* mlenv, MLFunc* mobj);
MNode*  ml_sqlite3_rollback (bool fev, MNode* cell, MlEnv* mlenv, MLFunc* mobj);

#endif /* ML_SQLITE3_H */
