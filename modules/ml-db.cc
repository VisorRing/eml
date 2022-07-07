#include "ml-db.h"
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

#define DB_XSERIAL_COMPAT  1

/*DOC:
==db module==

*/

void  MLDb::closedb () {
    if (db) {
	db->close ();
	lock.close ();
	delete db;
	db = NULL;
    }
}

void  MLDbBtree::dbPath (const ustring& name, bool fxserial, ustring& dbpath, ustring& lockpath) {
    if (fxserial) {
	dbpath = mlenv->env->path_store_file (name, kEXT_BTREE);
	lockpath = mlenv->env->path_store_file (name, kEXT_LOCK);
    } else {
	dbpath = mlenv->env->path_db (name, kEXT_BTREE);
	lockpath = mlenv->env->path_db (name, kEXT_LOCK);
    }
}

void  MLDbBtree::openRead (const ustring& name, bool fxserial) {
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

void  MLDbBtree::openRW (const ustring& name, bool fxserial) {
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

void  MLDbHash::dbPath (const ustring& name, bool fxserial, ustring& dbpath, ustring& lockpath) {
    if (fxserial) {
	dbpath = mlenv->env->path_store_file (name, kEXT_HASH);
	lockpath = mlenv->env->path_store_file (name, kEXT_LOCK);
    } else {
	dbpath = mlenv->env->path_db (name, kEXT_HASH);
	lockpath = mlenv->env->path_db (name, kEXT_LOCK);
    }
}

void  MLDbHash::openRead (const ustring& name, bool fxserial) {
    if (! db) {
	ustring  dbpath;
	ustring  lockpath;
	BDBHashRDOnly*  o = new BDBHashRDOnly;

	db = o;
	dbPath (name, fxserial, dbpath, lockpath);
	lock.openReadLock (lockpath.c_str ());
	o->open (dbpath.c_str ());
    }
}

void  MLDbHash::openRW (const ustring& name, bool fxserial) {
    if (! db) {
	ustring  dbpath;
	ustring  lockpath;
	BDBHash*  o = new BDBHash;

	db = o;
	dbPath (name, fxserial, dbpath, lockpath);
	lock.openAppendLock (lockpath.c_str ());
	o->open (dbpath.c_str ());
    }
}

typedef enum {
    FN_READ,
    FN_RW,
}  DBFUNC;

static MNode*  ml_db_sub (bool fev, MNode* cell, MlEnv* mlenv, MLDb* obj, DBFUNC fn) {
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
===$db-read===
 ($db-read NAME [:limit NUM] [#serial | :serial BOOL] [:on-error FUNCTION]
 	SUBFUNCTION...)

*/
//#XMFUNC	$db-read	ml_db_read	cMLDbID
MNode*  ml_db_read (bool fev, MNode* cell, MlEnv* mlenv) {
    MLDbBtree  obj (mlenv);
    return ml_db_sub (fev, cell, mlenv, &obj, FN_READ);
}

/*DOC:
===$db-rw===
 ($db-rw NAME [:limit NUM] [#serial | :serial BOOL] [:on-error FUNCTION]
 	SUBFUNCTION...)

*/
//#XMFUNC	$db-rw	ml_db_rw	cMLDbID
MNode*  ml_db_rw (bool fev, MNode* cell, MlEnv* mlenv) {
    MLDbBtree  obj (mlenv);
    return ml_db_sub (fev, cell, mlenv, &obj, FN_RW);
}

/*DOC:
===$hash-read===
 ($hash-read NAME [:limit NUM] [#serial | :serial BOOL] [:on-error FUNCTION]
	[SUBFUNCTION...])

*/
//#XMFUNC	$hash-read	ml_hash_read	cMLDbID
MNode*  ml_hash_read (bool fev, MNode* cell, MlEnv* mlenv) {
    MLDbHash  obj (mlenv);
    return ml_db_sub (fev, cell, mlenv, &obj, FN_READ);
}

/*DOC:
===$hash-rw===
 ($hash-rw NAME [:limit NUM] [#serial | :serial BOOL] [:on-error FUNCTION]
	[SUBFUNCTION...])

*/
//#XMFUNC	$hash-rw	ml_hash_rw	cMLDbID
MNode*  ml_hash_rw (bool fev, MNode* cell, MlEnv* mlenv) {
    MLDbHash  obj (mlenv);
    return ml_db_sub (fev, cell, mlenv, &obj, FN_RW);
}

/*DOC:
===subfunctions of $db-read and $db-rw===

*/
/*DOC:
====read====
 (read KEY) -> STRING-LIST or NIL

*/
//#XSFUNC	read	ml_db_x_read	$db-read
//#XSFUNC	read	ml_db_x_read	$db-rw
//#XSFUNC	read	ml_db_x_read	$hash-read
//#XSFUNC	read	ml_db_x_read	$hash-rw
MNode*  ml_db_x_read (bool fev, MNode* cell, MlEnv* mlenv, MLFunc* mobj) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    evalParams (fev, mlenv, cell, posList, posParams);
    MLDb*  obj = MObjRef<MLDb> (mobj, cMLDbID);
    ustring  key = to_string (posParams[0]());
    ustring  val;
    if (obj->db && obj->db->get (key, val)) {
	uiterator  b = val.begin ();
	uiterator  e = val.end ();
	uiterator  it;
	MNodeList  ans;
	for (it = b; ; it ++) {
	    if (*it == 0 || it == e) {
		ans.append (newMNode_str (new ustring (b, it)));
		b = it + 1;
		if (it == e)
		    break;
	    }
	}
	return mlenv->retval = ans.release ();
    } else {
	return NULL;
    }
}

/*DOC:
====write====
 (write KEY VALUES...) -> NIL

*/
//#XSFUNC	write	ml_db_x_write	$db-rw
//#XSFUNC	write	ml_db_x_write	$hash-rw
MNode*  ml_db_x_write (bool fev, MNode* cell, MlEnv* mlenv, MLFunc* mobj) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    MNodePtr  rest;
    evalParams (fev, mlenv, cell, posList, posParams, NULL, NULL, EV_LIST, &rest);
    MLDb*  obj = MObjRef<MLDb> (mobj, cMLDbID);
    ustring  key = to_string (posParams[0]());
    std::vector<ustring>  values;
    MNode*  args = rest ();
    ustring  v;
    while (args) {
	if (v.size () > 0)
	    v.append (CharConst ("\0"));
	v.append (to_string (args->car ()));
	nextNode (args);
    }
    if (obj->db)
	obj->db->put (key, v);
    return NULL;
}

/*DOC:
====delete====
 (delete KEY) -> NIL

*/
//#XSFUNC	delete	ml_db_x_delete	$db-rw
//#XSFUNC	delete	ml_db_x_delete	$hash-rw
MNode*  ml_db_x_delete (bool fev, MNode* cell, MlEnv* mlenv, MLFunc* mobj) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    evalParams (fev, mlenv, cell, posList, posParams);
    MLDb*  obj = MObjRef<MLDb> (mobj, cMLDbID);
    ustring  key = to_string (posParams[0]());
    if (obj->db)
	obj->db->del (key);
    return NULL;
}

static void  set_read_vars (ustring& val, std::vector<ustring>& vars, std::vector<int>& type, MlEnv* mlenv) {
    if (vars.size () > 0) {
	uiterator  b = val.begin ();
	uiterator  e = val.end ();
	uiterator  it;
	int  i = 0;
	for (it = b; ; it ++) {
	    if (it == e || *it == 0) {
		mlenv->setVar (vars[i], newMNode_str (new ustring (b, it)));
		b = it + 1;
		i ++;
		if (i == vars.size ())
		    break;
		if (it == e) {
		    for (; i < vars.size (); i ++) {
			mlenv->setVar (vars[i], NULL);
		    }
		    break;
		}
	    }
	}
    }
}

static void  set_read_arys (ustring& val, std::vector<ustring>& vars, std::vector<int>& type, MlEnv* mlenv) {
    if (vars.size () > 0) {
	uiterator  b = val.begin ();
	uiterator  e = val.end ();
	uiterator  it;
	int  i = 0;
	int  n = 0;
	bool  af = false;
	for (it = b; ; it ++) {
	    if (it == e || *it == 0) {
		switch (type[i]) {
		case 1:	// variable
		    mlenv->setVar (vars[i], newMNode_str (new ustring (b, it)));
		    type[i] = 0;
		    break;
		case 2:	// array
		    if (! af) {
			n ++;
			af = true;
		    }
		    mlenv->setAry (vars[i], n, newMNode_str (new ustring (b, it)));
		    break;
		}
		b = it + 1;
		do {
		    i ++;
		    if (i == type.size ()) {
			i = 0;
			af = false;
		    }
		} while (type[i] == 0);
		if (it == e)
		    break;
	    }
	}
	if (af) {
	    for (; i < vars.size (); i ++) {
		switch (type[i]) {
		case 1:	// variable
		    mlenv->setVar (vars[i], NULL);
		    break;
		case 2:	// array
		    mlenv->setAry (vars[i], n, NULL);
		    break;
		}
	    }
	}
	for (i = 0; i < vars.size (); i ++) {
	    if (type[i] == 2) // array
		mlenv->setArySize (vars[i], n);
	}
    }
}

/*DOC:
====db-select====
 (db-select LAMBDA) -> LIST_OF_LIST

*/
//#XSFUNC	db-select	ml_db_x_db_select	$db-read
//#XSFUNC	db-select	ml_db_x_db_select	$db-rw
//#XSFUNC	db-select	ml_db_x_db_select	$hash-read
//#XSFUNC	db-select	ml_db_x_db_select	$hash-rw
MNode*  ml_db_x_db_select (bool fev, MNode* cell, MlEnv* mlenv, MLFunc* mobj) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    evalParams (fev, mlenv, cell, posList, posParams);
    MLDb*  obj = MObjRef<MLDb> (mobj, cMLDbID);
    MNode*  lambda = posParams[0]();
    if (obj->db)
	obj->db->initeach ();
    ustring  key, val;
    MNodePtr  h;
    MNodeList  ans;
    while (obj->db && obj->db->each (key, val)) {
	MNodeList  ag;
	uiterator  b = val.begin ();
	uiterator  e = val.end ();
	uiterator  it;
	ag.append (newMNode_str (new ustring (key)));
	for (it = b; ; it ++) {
	    if (*it == 0 || it == e) {
		ag.append (newMNode_str (new ustring (b, it)));
		b = it + 1;
		if (it == e)
		    break;
	    }
	}

	h = execDefun (mlenv, lambda, ag (), uEmpty);
	if (to_bool (h ())) {
	    ans.append (ag.release ());
	}
    }

    return mlenv->retval = ans ();
}

/*DOC:
===temp-read===
 (temp-read KEY) -> STRING-LIST or NIL

*/
static ustring  TempRWDB (CharConst ("temprwdb"));

//#XAFUNC	temp-read	ml_temp_read
MNode*  ml_temp_read (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    evalParams (fev, mlenv, cell, posList, posParams);
    MLDbBtree  obj (mlenv);
    ustring  key = to_string (posParams[0]());
    if (mlenv->env->storedir.empty ())
	throw (uErrorNoStore);
    obj.openRead (TempRWDB, true);
    ustring  val;
    if (obj.db && obj.db->get (key, val)) {
	uiterator  b = val.begin ();
	uiterator  e = val.end ();
	uiterator  it;
	MNodeList  ans;
	for (it = b; ; it ++) {
	    if (*it == 0 || it == e) {
		ans.append (newMNode_str (new ustring (b, it)));
		b = it + 1;
		if (it == e)
		    break;
	    }
	}
	return mlenv->retval = ans.release ();
    }
    return NULL;
}

/*DOC:
===temp-write===
 (temp-write KEY VALUES...) -> NIL

*/
//#XAFUNC	temp-write	ml_temp_write
MNode*  ml_temp_write (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    MNodePtr  rest;
    evalParams (fev, mlenv, cell, posList, posParams, NULL, NULL, EV_LIST, &rest);
    MLDbBtree  obj (mlenv);
    ustring  key = to_string (posParams[0]());
    std::vector<ustring>  values;
    ustring  v;
    MNode*  args = rest ();
    while (args) {
	if (v.size () > 0)
	    v.append (CharConst ("\0"));
	v.append (to_string (args->car ()));
    }
    if (mlenv->env->storedir.empty ())
	throw (uErrorNoStore);
    obj.openRW (TempRWDB, true);
    if (obj.db)
	obj.db->put (key, v);
    return NULL;
}
