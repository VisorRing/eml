#include "ml-sqlite3.h"
#include "motorconst.h"
#include "ml.h"
#include "mlenv.h"
#include "motorenv.h"
#include "ustring.h"
#include "expr.h"
#include "util_const.h"
#include "util_file.h"
#include "util_random.h"
#include "util_splitter.h"
#include "util_string.h"
#include "utf8.h"
#include <exception>
#include <unistd.h>

//#define  DB5_DBDIR_HACK	1
#define  DEFAULT_RETRY	10
#ifdef BDB5SQLITE
#define  kEXT_SQLITE3	".db5sql"
#else
#define  kEXT_SQLITE3	".sqlite3"
#endif

/*DOC:
==sqlite3 module==

*/

static void  randomSleep () {
    usleep ((useconds_t)(10000 + 100000 * randDouble ()));
}

int  MLSqlite3::open (ustring& name) {
    int  rc;
    int  flags = SQLITE_OPEN_READWRITE;

    if (initfunc () && ! isPlainFile (name)) {
	flags |= SQLITE_OPEN_CREATE;
	finit = true;
    } else if (fcreate) {
	flags |= SQLITE_OPEN_CREATE;
    } else {
    }

    rc = sqlite3_open_v2 (name.c_str (), &dbh, flags, NULL);
    if (! rc) {
	sqlite3_busy_timeout (dbh, 60000);
	dbst = NULL;
	return 1;		// 1: OK
    } else {
	return 0;		// 0: NG
    }
}

void  MLSqlite3::close () {
    if (dbh) {
	finalize ();
	sqlite3_close (dbh);
	dbh = NULL;
    }
}

void  MLSqlite3::finalize () {
    if (dbst) {
	sqlite3_finalize (dbst);
	dbst = NULL;
    }
}

int  MLSqlite3::prepare (const ustring& sql) {
    int  rc;

#ifdef DEBUG
    if (mlenv->log && ! mlenv->mlPool->nolog) {
	*mlenv->log << "\t" << sql << "\n";
    }
#endif /* DEBUG */
    rc = sqlite3_prepare_v2 (dbh, sql.c_str (), sql.length (), &dbst, NULL);
    if (rc != SQLITE_OK) {
	postError (ustring (CharConst ("SQL error: ")) + sqlite3_errmsg (dbh));
    }
    return rc;
}

void  MLSqlite3::bind (namearray& name, namearray& value) {
    namearray::iterator  i1, i2;
    int  n;

    for (i1 = name.begin (), i2 = value.begin ();
	 i1 != name.end ();
	 i1 ++, i2 ++) {
#ifdef DEBUG
	if (mlenv->log && ! mlenv->mlPool->nolog) {
	    *mlenv->log << "\tbind ";
	    if (*i1)
		*mlenv->log << *(*i1);
	    else
		*mlenv->log << "(NULL)";
	    *mlenv->log << " <= \"";
	    if (*i2)
		*mlenv->log << ellipsis (logText (*(*i2)), cLOGSHORTSIZE);
	    else
		*mlenv->log << "(NULL)";
	    *mlenv->log << "\"\n";
	}
#endif /* DEBUG */
	if (*i1) {
	    n = sqlite3_bind_parameter_index (dbst, (*i1)->c_str ());
	    if (n == 0) {
		throw (ustring ("\"") + *(*i1) + ustring ("\": named parameter not found."));
	    }
	} else {
	    throw (ustring ("\"(NULL)\": named parameter not found."));
	}
	if (*i2)
	    sqlite3_bind_text (dbst, n, (*i2)->c_str (), (*i2)->length (), NULL);
	else
	    sqlite3_bind_null (dbst, n);
    }
}

void  MLSqlite3::bind (namearray& value) {
    namearray::iterator  i2;
    int  n;

    for (n = 1, i2 = value.begin (); i2 != value.end (); n ++, i2 ++) {
#ifdef DEBUG
	if (mlenv->log && ! mlenv->mlPool->nolog) {
	    *mlenv->log << "\tbind[" << n << "] <= \"";
	    if (*i2)
		*mlenv->log << ellipsis (logText (*(*i2)), cLOGSHORTSIZE);
	    else
		*mlenv->log << "(NULL)";
	    *mlenv->log << "\"\n";
	}
#endif /* DEBUG */
	if (*i2)
	    sqlite3_bind_text (dbst, n, (*i2)->c_str (), (*i2)->length (), NULL);
	else
	    sqlite3_bind_null (dbst, n);
    }
}

void  MLSqlite3::exec () {
    int  rc;

    rc = sqlite3_step (dbst);
    switch (rc) {
    case SQLITE_DONE:
	finalize ();
	break;
    case SQLITE_ROW:
	ncols = sqlite3_column_count (dbst);
	break;
    case SQLITE_BUSY:
	if (mlenv->log)
	    mlenv->env->logMessage (ustring (CharConst ("SQL busy.")));
	goto Err;
    case SQLITE_LOCKED:
	if (mlenv->log)
	    mlenv->env->logMessage (ustring (CharConst ("SQL locked.")));
	goto Err;
    default:			// error
	if (mlenv->log)
	    mlenv->env->logMessage (ustring (CharConst ("SQL error: ")) + to_ustring (rc) + uLF);
	goto Err;
    }
    return;

 Err:;
    finalize ();
    setBreak ();
}

void  MLSqlite3::step () {
    int  rc;

    rc = sqlite3_step (dbst);
    switch (rc) {
    case SQLITE_DONE:
	finalize ();
	break;
    case SQLITE_ROW:
	break;
    case SQLITE_BUSY:
	if (mlenv->log)
	    *mlenv->log << "SQL busy.\n";
	goto Err;
    case SQLITE_LOCKED:
	if (mlenv->log)
	    *mlenv->log << "SQL deadlocked.\n";
	goto Err;
    default:
	if (mlenv->log)
	    *mlenv->log << "SQL error.\n";
	goto Err;
    }
    return;

 Err:;
    finalize ();
    mlenv->breakProg ();
}

MNode*  MLSqlite3::answer_vector () {
    MNodePtr  ans;
    int  i;

    ans = newMNode_vector ();
    if (isReady ()) {
	for (i = 0; i < ncols; i ++) {
	    if (sqlite3_column_type (dbst, i) == SQLITE_NULL) {
		ans ()->vectorPush (NULL);
	    } else {
		ans ()->vectorPush (newMNode_str (new ustring (char_type (sqlite3_column_text (dbst, i)), sqlite3_column_bytes (dbst, i))));
	    }
	}
    }
    finalize ();
    return ans.release ();
}

MNode*  MLSqlite3::answer_vector_ary () {
    MNodePtr  ans;
    int  i, n;
    MNodePtr  list;

    ans = newMNode_vector ();
    n = 0;
    while (isReady ()) {
	list = newMNode_vector ();
	n ++;
	for (i = 0; i < ncols; i ++) {
	    if (sqlite3_column_type (dbst, i) == SQLITE_NULL) {
		list ()->vectorPush (NULL);
	    } else {
		list ()->vectorPush (newMNode_str (new ustring (char_type (sqlite3_column_text (dbst, i)), sqlite3_column_bytes (dbst, i))));
	    }
	}
	ans ()->vectorPush (list.release ());
	if (n >= limit && limit > 0)
	    break;
	step ();
    }

    return ans.release ();
}

sqlite3_int64  MLSqlite3::rowid () {
    return sqlite3_last_insert_rowid (dbh);
}

void  MLSqlite3::postError (ustring msg) {
    if (errfunc ()) {
	MNodePtr  ag;
	MNodePtr  v;
	ag = new MNode;
	ag ()->set_car (newMNode_str (new ustring (msg)));
	v = execDefun (mlenv, errfunc (), ag (), uEmpty);
    } else {
	throw (msg);
    }
}

int  MLSqlite3::sql_s (const ustring& query) {
    int  rc;
    int  n = 0;

    rc = prepare (query);
    if (rc != SQLITE_OK)
	return rc;
    do {
	rc = sqlite3_step (dbst);
	if (rc == SQLITE_DONE || rc == SQLITE_ROW)
	    break;
	n ++;
	if (n > 10) {
	    mlenv->env->logMessage (ustring (CharConst ("begin busy.")));
	    mlenv->breakProg ();
	    break;
	} else {
	    randomSleep ();
	}
    } while (1);

    return rc;
}

void  MLSqlite3::setBreak () {
    mlenv->setBreak (blockSym (), NULL);
}

ustring  MLSqlite3::errmsg () {
    return ustring (sqlite3_errmsg (dbh));
}

static void  setDirFunc (MLFunc* mobj) {
    MLSqlite3*  obj = MObjRef<MLSqlite3> (mobj, cMLSqlite3ID);
    ustring  u = mobj->mlenv->env->path_to_db ();
#ifdef BDB5SQLITE
#ifdef DB5_DBDIR_HACK
    sqlite3_set_dbdir (obj->dbh, u.c_str ());
#endif
#else
    sqlite3_set_dbdir (obj->dbh, u.c_str ());
#endif
}

/*DOC:
===$sqlite3===
 ($sqlite3 DB [:limit NUM] [#create | :create BOOL] [:initialize FUNCTION] [SUBFUNCTION...]) -> LAST VALUE

*/
//#XMFUNC	$sqlite3	ml_sqlite3	cMLSqlite3ID
MNode*  ml_sqlite3 (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    kwParam  kwList[] = {{CharConst ("limit"), EV_LIST},
			 {CharConst ("create"), EV_LIST},
			 {CharConst ("onerror"), EV_LIST},
			 {CharConst ("initialize"), EV_LIST},
			 {NULL, 0, EV_END}};
    MNodePtr  kwParams[4];
    MNodePtr  rest;
    evalParams (fev, mlenv, cell, posList, posParams, kwList, kwParams, EV_ASIS, &rest);
    MLSqlite3  obj (mlenv);
    ustring  db = to_string (posParams[0]());
    if (! isNil (kwParams[0]())) {
	obj.limit = to_int64 (kwParams[0]());
	if (obj.limit <= 0)
	    obj.limit = 1;
	if (obj.limit > kARRAYMAX)
	    obj.limit = kARRAYMAX;
    }
    obj.fcreate = to_bool (kwParams[1]());
    if (! isNil (kwParams[2]()))
	obj.errfunc = kwParams[2]();
    if (! isNil (kwParams[3]()))
	obj.initfunc = kwParams[3]();
    if (db.size () == 0)
	throw (uErrorFilenameEmpty);
    ustring  dbfile = mlenv->env->path_db (db, kEXT_SQLITE3);
    obj.finit = false;
    if (! obj.open (dbfile))
	throw (obj.errmsg ());
    MNodePtr  ans;
    mlenv->setMStack (&obj);
    {
	MlEnv::autoDatastoreFunc  dsf (mlenv, &obj, setDirFunc);
	if (obj.finit) {
	    MNodeList  arg;
	    arg.append (obj.initfunc ());
	    MNodePtr  v;
	    v = eval_fn (false, obj.initfunc (), arg (), mlenv);
	    obj.finit = false;
	}
	ans = progn (rest (), mlenv);
	mlenv->stopBreak (cell->car ());
    }
    return mlenv->retval = ans ();
}

static void  bindParam (MNode* p, MLSqlite3::fsqlParam& par, MLSqlite3* obj) {
    AutoDelete<ustring>  key;
    AutoDelete<ustring>  val;
    umatch  m;
    static uregex  re ("^:[a-zA-Z0-9_]{1,32}$");

    if (! isCons (p)) {
	throw (p->dump_string_short () + ustring (": bad bind parameter."));
    } else {
	while (p) {
	    MNode*  el = p->car ();
	    if (isCons (p)) {
		key = new ustring (to_string (el->car ()));
		if (usearch (*key (), m, re)) {
		    if (el->cdr ()) {
			val = new ustring (to_string (el->cdr ()));
			par.bindName.push_back (key.release ());
			par.bindValue.push_back (val.release ());
		    } else {
			par.bindName.push_back (key.release ());
			par.bindValue.push_back (NULL);
		    }
		} else {
		    throw (uQ2 + *key () + ustring ("\": bad bind name."));
		}
	    } else {
		throw (p->dump_string_short () + ustring (": bad bind parameter."));
	    }
	    nextNode (p);
	}
    }
}

/*DOC:
===subfunctions of $sqlite3===

*/
static MNode*  ml_sqlite3_sql_main (bool fev, MNode* cell, MlEnv* mlenv, MLSqlite3::fsqlParam& par, MLFunc* mobj) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    kwParam  kwList[] = {{CharConst ("bind"), EV_LIST},
			 {NULL, 0, EV_END}};
    MNodePtr  kwParams[1];
    MNodePtr  rest;
    evalParams (fev, mlenv, cell, posList, posParams, kwList, kwParams, EV_LIST, &rest);
    MLSqlite3*  obj = MObjRef<MLSqlite3> (mobj, cMLSqlite3ID);
    par.sql = to_string (posParams[0]());
    int  mode = 0;
    if (! isNil (kwParams[0]())) {		// bind
	mode = 1;
	bindParam (kwParams[0](), par, obj);
    }
    if (mode == 0) {
	MNode*  arg = rest ();
	while (isCons (arg)) {
	    if (isNil (arg->car ())) {
		par.bindValue.push_back (NULL);
	    } else {
		par.bindValue.push_back (new ustring (to_string (arg->car ())));
	    }
	    nextNode (arg);
	}
	mode = 2;
    }
    MNodePtr  ans;
    int  rc = obj->prepare (par.sql);
    if (rc == SQLITE_OK) {
	if (mode == 2) {
	    obj->bind (par.bindValue);
	} else {
	    obj->bind (par.bindName, par.bindValue);
	}
	obj->exec ();
	if (obj->isReady ()) {
	    switch (par.answertype) {
	    case MLSqlite3::fsqlParam::COL_VECTOR:
		ans = obj->answer_vector ();
		break;
	    case MLSqlite3::fsqlParam::COL_VECTOR_VECTOR:
		ans = obj->answer_vector_ary ();
		break;
	    }
	}
    } else {			// ! SQLITE_OK
//	obj->postError (ustring (CharConst ("SQL error: ")) + sqlite3_errmsg (obj->dbh));
    }

    return ans.release ();
}

/*DOC:
====sql====
 (sql QUERY [:bind '((NAME . VALUE)...)]) -> VECTOR
 (sql@ QUERY [:bind '((NAME . VALUE)...)]) -> VECTOR_OF_VECTOR

*/
//#XSFUNC	sql	ml_sqlite3_sqlv
//#XSFUNC	sql@	ml_sqlite3_sqlv_ary
MNode*  ml_sqlite3_sqlv (bool fev, MNode* cell, MlEnv* mlenv, MLFunc* mobj) {
    MLSqlite3::fsqlParam  par;

    par.answertype = MLSqlite3::fsqlParam::COL_VECTOR;
    return ml_sqlite3_sql_main (fev, cell, mlenv, par, mobj);
}

MNode*  ml_sqlite3_sqlv_ary (bool fev, MNode* cell, MlEnv* mlenv, MLFunc* mobj) {
    MLSqlite3::fsqlParam  par;

    par.answertype = MLSqlite3::fsqlParam::COL_VECTOR_VECTOR;
    return ml_sqlite3_sql_main (fev, cell, mlenv, par, mobj);
}

/*DOC:
====rowid====
 (rowid) -> INTEGER

*/
//#XSFUNC	rowid	ml_sqlite3_rowid
MNode*  ml_sqlite3_rowid (bool fev, MNode* cell, MlEnv* mlenv, MLFunc* mobj) {
    evalParams (fev, mlenv, cell);
    MLSqlite3*  obj = MObjRef<MLSqlite3> (mobj, cMLSqlite3ID);
    return newMNode_int64 (obj->rowid ());
}

static ustring  escape_like (const ustring& str) {
    SplitterChars  sp (str, ustring (CharConst ("%_\\")));
    if (sp.nextSep ()) {
	ustring  ans;
	do {
	    ans.append (sp.pre ());
//	    ans.append (CharConst ("\\"));
	    ans.append (uBSlash);		// "\\"
	    ans.append (sp.matchBegin (), sp.matchEnd ());
	} while (sp.nextSep ());
	ans.append (sp.pre ());
	return ans;
    } else {
	return str;
    }
}

/*DOC:
====escape-like====
 (escape-like STRING) -> STRING

*/
//#XSFUNC	escape-like	ml_sqlite3_escape_like
MNode*  ml_sqlite3_escape_like (bool fev, MNode* cell, MlEnv* mlenv, MLFunc* mobj) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    evalParams (fev, mlenv, cell, posList, posParams);
    MLSqlite3*  obj = MObjRef<MLSqlite3> (mobj, cMLSqlite3ID);
    ustring  str = to_string (posParams[0]());
    ustring  ans = escape_like (str);
    return newMNode_str (new ustring (ans));
}

/*DOC:
====begin-transaction====
 (begin-transaction [#exclusive|#writer] [:retry NUM] [:on-abend FUNCTION] [SUBFUNCTION...]) -> LAST VALUE

*/
//#XSFUNC	begin-transaction	ml_sqlite3_begin_transaction
MNode*  ml_sqlite3_begin_transaction (bool fev, MNode* cell, MlEnv* mlenv, MLFunc* mobj) {
    kwParam  kwList[] = {{CharConst ("retry"), EV_LIST},
			 {CharConst ("on-abend"), EV_LIST},
			 {CharConst ("exclusive"), EV_LIST},
			 {CharConst ("writer"), EV_LIST},
			 {NULL, 0, EV_END}};
    MNodePtr  kwParams[4];
    MNodePtr  rest;
    evalParams (fev, mlenv, cell, NULL, NULL, kwList, kwParams, EV_ASIS, &rest);
    MLSqlite3*  obj = MObjRef<MLSqlite3> (mobj, cMLSqlite3ID);
    obj->retry = DEFAULT_RETRY;
    if (! isNil (kwParams[0]()))
	obj->retry = to_int64 (kwParams[0]());
    if (obj->retry < 0)
	obj->retry = 0;
    MNode*  abendfunc = kwParams[1]();
    bool  fexclusive = to_bool (kwParams[2]()) || to_bool (kwParams[3]());
    bool  fbeginquery = false;
    MNodePtr  ans;
    bool  fexit;
    obj->blockSym = cell->car ();
    do {
	fexit = true;
	if (! obj->fbegin) {
	    obj->fbegin = true;
	    fbeginquery = true;
	    if (fexclusive) {
		static const ustring  q (CharConst ("begin exclusive"));
		obj->sql_s (q);
	    } else {
		static const ustring  q (CharConst ("begin"));
		obj->sql_s (q);
	    }
	}

	ans = progn (rest (), mlenv);

	if (mlenv->breaksym ()) {
	    if (fbeginquery) {
		static const ustring  q (CharConst ("rollback"));
		obj->fbegin = false;
		obj->sql_s (q);
		if (mlenv->stopBreak (cell->car ())) {
		    obj->retry --;
		    if (obj->retry > 0) {
			fexit = false;
			// usleep (1000000 + 3000000 * randDouble ());
		    }
		}
		if (fexit == true) {
		    if (! isNil (abendfunc)) {
			MNodePtr  ag;
			MNodePtr  v;
			v = execDefun (mlenv, abendfunc, ag (), uEmpty);
		    } else {
			mlenv->breakProg ();
		    }
		}
	    }
	} else {
	    if (fbeginquery) {
		static const ustring  q (CharConst ("end"));
		obj->fbegin = false;
		obj->sql_s (q);
	    }
	}
    } while (! fexit);

    return ans.release ();
}

/*
//DOC:
====rollback====
 (rollback) -> NIL

*/
//#XSFUNC	rollback	ml_sqlite3_rollback
MNode*  ml_sqlite3_rollback (bool fev, MNode* cell, MlEnv* mlenv, MLFunc* mobj) {
    evalParams (fev, mlenv, cell);
    MLSqlite3*  obj = MObjRef<MLSqlite3> (mobj, cMLSqlite3ID);
    obj->retry = 0;
    obj->setBreak ();
    return NULL;
}
