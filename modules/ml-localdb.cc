#include "ml-localdb.h"
#include "motorconst.h"
#include "ml.h"
#include "mlenv.h"
#include "motorenv.h"
#include "ustring.h"
#include "expr.h"
#include "util_const.h"
#include "util_check.h"
#include "util_string.h"
#include <vector>
#include <exception>

/*DOC:
==local db module==

*/

void  MLLocalDb::closedb () {
    if (db) {
	db->close ();
	lock.close ();
	delete db;
	db = NULL;
    }
}

void  MLLocalDbBtree::dbPath (const ustring& name, bool fxserial, ustring& dbpath, ustring& lockpath) {
    if (fxserial) {
	dbpath = mlenv->env->path_store_file (name, kEXT_BTREE);
	lockpath = mlenv->env->path_store_file (name, kEXT_LOCK);
    } else {
	dbpath = mlenv->env->path_db (name, kEXT_BTREE);
	lockpath = mlenv->env->path_db (name, kEXT_LOCK);
    }
}

void  MLLocalDbBtree::openRead (const ustring& name, bool fxserial) {
    if (! db) {
	ustring  dbpath;
	ustring  lockpath;
	BDBBtree*  o = new BDBBtree;

	db = o;;
	dbPath (name, fxserial, dbpath, lockpath);
	if (lock.openReadLock (lockpath.c_str ())) {
	    o->openRead (dbpath.c_str ());
	} else {
	    delete db;
	    db = NULL;
	}
    }
}

void  MLLocalDbBtree::openRW (const ustring& name, bool fxserial) {
    if (! db) {
	ustring  dbpath;
	ustring  lockpath;
	BDBBtree*  o = new BDBBtree;

	db = o;
	dbPath (name, fxserial, dbpath, lockpath);
	lock.openAppendLock (lockpath.c_str ());
	o->openRW (dbpath.c_str ());
    }
}

typedef enum {
    FN_READ,
    FN_RW,
}  DBFUNC;

static MNode*  ml_localdb_sub (bool fev, MNode* cell, MlEnv* mlenv, MLLocalDb* obj, DBFUNC fn) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    kwParam  kwList[] = {{CharConst ("limit"), EV_LIST},    // 0
			 {CharConst ("serial"), EV_LIST},   // 1
			 {CharConst ("on-error"), EV_LIST}, // 2
#ifdef DB_XSERIAL_COMPAT
			 {CharConst ("xserial"), EV_LIST},  // 3
#endif
			 {NULL, 0, EV_END}};
    MNodePtr  kwParams[4];
    MNodePtr  rest;
    evalParams (fev, mlenv, cell, posList, posParams, kwList, kwParams, EV_ASIS, &rest);
    ustring  name = to_string (posParams[0]());
    obj->limit = to_int64 (kwParams[0]());
    if (obj->limit < 0)
	obj->limit = 0;
    if (obj->limit > kARRAYMAX)
	obj->limit = kARRAYMAX;
    bool  fxserial = to_bool (kwParams[1]());
#ifdef DB_XSERIAL_COMPAT
    if (to_bool (kwParams[3]()))
	fxserial = true;
#endif
    MNode*  errfn = kwParams[2]();
    if (name.size () == 0)
	throw (uErrorFilenameEmpty);
    if (fxserial && mlenv->env->storedir.empty ())
	throw (uErrorNoStore);
    switch (fn) {
    case FN_READ:
	obj->openRead (name, fxserial);
	break;
    case FN_RW:
	obj->openRW (name, fxserial);
	break;
    default:
	assert (0);
    }
    mlenv->setMStack (obj);
    MNodePtr  ans;
    try {
	ans = progn (rest (), mlenv);
    } catch (ustring& msg) {
	if (errfn) {
	    onErrorFn (errfn, mlenv);
	} else {
	    throw (msg);
	}
    }
    mlenv->stopBreak (cell->car ());
    return mlenv->retval = ans ();
}

/*DOC:
===local-db===
// ($db-rw NAME [:limit NUM] [#serial | :serial BOOL] [:on-error FUNCTION]
// 	SUBFUNCTION...)
 (local-db NAME #serial #named BODY ...) -> LAST_VALUE

*/
//#XMFUNC	local-db	ml_localdb_rw	cMLLocalDbID
MNode*  ml_localdb_rw (bool fev, MNode* cell, MlEnv* mlenv) {
    MLLocalDbBtree  obj (mlenv);
    return ml_localdb_sub (fev, cell, mlenv, &obj, FN_RW);
}


/*DOC:
===subfunctions of local-db===

*/
/*DOC:
====db-lookup====
// (read KEY) -> STRING-LIST or NIL
 (db-lookup KEY) -> VALUE | nil

*/
//#XSFUNC	db-lookup	ml_localdb_x_read	local-db
MNode*  ml_localdb_x_read (bool fev, MNode* cell, MlEnv* mlenv, MLFunc* mobj) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    evalParams (fev, mlenv, cell, posList, posParams);
    MLLocalDb*  obj = MObjRef<MLLocalDb> (mobj, cMLLocalDbID);
    ustring  key = to_string (posParams[0]());
    ustring  val;
    if (obj->db && obj->db->get (key, val)) {
	MotorTexp  ml (NULL);
	ml.scan (val);
	if (ml.top.isCons () && isCons (ml.top.cdr ()))
	    mlenv->retval = ml.top.cdr ()->car ();
	return mlenv->retval ();
    } else {
	return NULL;
    }
}

/*DOC:
====db-insert====
// (write KEY VALUES...) -> NIL
 (db-insert KEY VALUE) -> nil

*/
//#XSFUNC	db-insert	ml_localdb_x_write	local-db
MNode*  ml_localdb_x_write (bool fev, MNode* cell, MlEnv* mlenv, MLFunc* mobj) {
    paramType  posList[] = {EV_LIST, EV_LIST, EV_END};
    MNodePtr  posParams[2];
    evalParams (fev, mlenv, cell, posList, posParams);
    MLLocalDb*  obj = MObjRef<MLLocalDb> (mobj, cMLLocalDbID);
    ustring  key = to_string (posParams[0]());
    MNode*  val = posParams[1]();
    if (obj->db)
	obj->db->put (key, dump_to_texp (val));
    return NULL;
}

/*DOC:
====db-delete====
 (db-delete KEY) -> NIL

*/
//#XSFUNC	db-delete	ml_localdb_x_delete	local-db
MNode*  ml_localdb_x_delete (bool fev, MNode* cell, MlEnv* mlenv, MLFunc* mobj) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    evalParams (fev, mlenv, cell, posList, posParams);
    MLLocalDb*  obj = MObjRef<MLLocalDb> (mobj, cMLLocalDbID);
    ustring  key = to_string (posParams[0]());
    if (obj->db)
	obj->db->del (key);
    return NULL;
}

/*DOC:
====db-keys====
 (db-keys) -> (STRING ...)

*/
//#XSFUNC	db-keys	ml_localdb_x_keys	local-db
MNode*  ml_localdb_x_keys (bool fev, MNode* cell, MlEnv* mlenv, MLFunc* mobj) {
    evalParams (fev, mlenv, cell);
    MNodeList  ans;
    MLLocalDb*  obj = MObjRef<MLLocalDb> (mobj, cMLLocalDbID);
    ustring  key, val;
    if (obj->db) {
	obj->db->initeach ();
	while (obj->db->each (key, val)) {
	    ans.append (newMNode_str (new ustring (key)));
	}
    }
    return mlenv->retval = ans ();
}
