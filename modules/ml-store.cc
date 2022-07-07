#include "ml-store.h"
#include "config.h"
#include "motorconst.h"
#include "ml.h"
#include "mlenv.h"
#include "expr.h"
#include "motorenv.h"
#include "util_const.h"
#include "util_check.h"
#include "util_file.h"
#include "util_proc.h"
#include "util_time.h"
#include "util_random.h"
#include "util_string.h"
#include "util_mimetype.h"
#include "utf8.h"
#include "ustring.h"
#include "bdbmacro.h"
#include "filemacro.h"
#include <strings.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>

#define  StoreFileNameMax	48

static MNode*  listFiles (const ustring& dirpath) {
    MNodeList  ans;
    DIR*  dir;
    struct dirent  de;
    struct dirent*  dep;
    ustring  name;
    ustring  t;

    dir = opendir (dirpath.c_str ());
    if (dir) {
	while (readdir_r (dir, &de, &dep) == 0 && dep != NULL) {
#ifndef NOT_HAVE_D_NAMLEN
	    name.assign (dep->d_name, dep->d_namlen);
#else
	    name.assign (dep->d_name);
#endif
	    if (name.length () > 0 && name[0] != '.') {
		ans.append (newMNode_str (new ustring (filenameDecode (name))));
	    }
	}
	closedir (dir);
	dir = NULL;
    }
    return ans.release ();
}

//============================================================
StoreType::StoreType (MlEnv* _mlenv) {
    mlenv = _mlenv;
    if (mlenv->env->storagedir.length () > 0) {
	type = F_NAMED;
    } else if (mlenv->env->storedir.length () > 0) {
	type = F_SERIAL;
    } else {
	type = F_NONE;
    }
}

const ustring  StoreType::src (const ustring& name) {
    if (name.size () == 0)
	throw (uErrorFilenameEmpty);
    switch (type) {
    case F_SERIAL:
	return mlenv->env->path_store_file (name);
    case F_NAMED:
	return mlenv->env->path_storage_file (name);
    case F_STATIC:
	return mlenv->env->path_static_file (name);
    default:
	throw (uErrorNoStore);
    }
}

const ustring  StoreType::src () {
    return src (param);
}

ustring  StoreType::read () {
    if (type == F_TEXT) {
	return param;
    } else {
	ustring  ans;
	if (readFile (src (param), ans, cPOSTLIMITHARD)) {
	    if (encoding.size () > 0 && ans.size () > 0) {
		UIConv  cd (kCODE_UTF8, encoding.c_str ());
		ans = cd.cv (ans, true);
	    }
	    ans = fixUTF8 (ans);
	}
	return ans;
    }
}

//============================================================

/*DOC:
==data store module==

*/
/*
  *record of serial.db
  key:	<xserial>
  val:	<creation_time>:<last_access_time>:<serial>:<subdir>:<savedir>
*/
static void  openDB (BDBBtree& db, FileMacro& lock, MlEnv* mlenv) {
    ustring  t = mlenv->env->path_to_store_index ();
    ustring  dbfile = t + kEXT_BTREE;
    ustring  lockfile = t + kEXT_LOCK;

    lock.openAppendLock (lockfile.c_str ());
    db.openRW (dbfile.c_str ());
}

static bool  splitRec (ustring& rec, ustring& uctime, ustring& uatime, ustring& sr, ustring& subdir, ustring& savedir) {
    uiterator  b = rec.begin ();
    uiterator  e = rec.end ();
    umatch  m;
    static uregex  re (":");

    if (b == e || ! usearch (b, e, m, re))
	return false;
    uctime.assign (b, m[0].first);
    b = m[0].second;
    if (b == e || ! usearch (b, e, m, re))
	return false;
    uatime.assign (b, m[0].first);
    b = m[0].second;
    if (b == e || ! usearch (b, e, m, re))
	return false;
    sr.assign (b, m[0].first);
    b = m[0].second;
    if (b == e || ! usearch (b, e, m, re))
	return false;
    subdir.assign (b, m[0].first);
    b = m[0].second;
    savedir.assign (b, e);

    return true;
}

MNode*  newStoreSerial (MlEnv* mlenv) {
    BDBBtree  db;
    FileMacro  lock;
    uint64_t  srl;
    ustring  v, x, subdir, nw;
    time_t  tm;
    ustring  xs;

    tm = now ();
    nw = to_ustring (tm);
    subdir = strYMD (tm);
    subdir.append (uSlash);

    openDB (db, lock, mlenv);

    if (db.get (uDash, x)) {
	srl = to_uint64 (x) + 1;
    } else {
	srl = 1;
    }

    xs = randomKey (srl);

    x = to_ustring (srl);
    db.put (uDash, x);

    subdir.append (to_ustring (srl & 0x000003FFF000ULL)).append (uSlash);
    subdir.append (x);
    v = nw;
    v.append (uColon).append (nw).append (uColon).append (x).append (uColon).append (subdir).append (uColon);
    db.put (xs, v);

    db.close ();
    lock.close ();

    v = mlenv->env->path_to_store ();
    makeSubdir (v, subdir);
    mlenv->env->storedir = v + uSlash;

    MNode*  ans = newMNode_str (new ustring (xs));
    //    mlenv->setVar (uXSerial, ans);
#ifdef DEBUG2
    std::cerr << "storedir:" << mlenv->env->storedir << "\n";
#endif /* DEBUG */

    return mlenv->retval = ans;
}

static bool  setSerial (MlEnv* mlenv, ustring& xs, ustring& path) {
    BDBBtree  db;
    FileMacro  lock;
    ustring  nw, v, uctime, uatime, sr, subdir, savedir;
    time_t  tm;

    tm = now ();
    nw = to_ustring (tm);

    openDB (db, lock, mlenv);

    if (! db.get (xs, v) || ! splitRec (v, uctime, uatime, sr, subdir, savedir)) {
	db.close ();
	lock.close ();
	return false;
    }

    v = nw;
    v.append (uColon).append (nw).append (uColon).append (sr).append (uColon).append (subdir).append (uColon).append (savedir);
    db.put (xs, v);

    db.close ();
    lock.close ();

    if (uctime.size () == 0) {
	v = mlenv->env->path_to_store ();
	makeSubdir (v, subdir);
	path = v;
	path.append (uSlash);
    } else {
	path = mlenv->env->path_to_store ();
	path.append (subdir).append (uSlash);
    }

    return true;
}

static ustring*  nextRec (char*& b, char* e, bool fans) {
    char*  p = b;
    ustring*  ans = NULL;

    while (b < e) {
	if (*b == ':') {
	    if (fans)
		ans = new ustring (p, b);
	    b ++;
	    return ans;
	}
	b ++;
    }
    if (p < e && fans)
	ans = new ustring (p, e);
    return ans;
}

static void  pathRec (DBT1* val, ustring*& subdir, ustring*& savedir) {
    char*  b;
    char*  e;
    ustring*  u;

    b = (char*)val->data;
    e = b + val->size;
    nextRec (b, e, false);
    nextRec (b, e, false);
    nextRec (b, e, false);
    u = nextRec (b, e, true);
    if (u)
	subdir = u;
    else
	subdir = NULL;
    u = nextRec (b, e, true);
    if (u)
	savedir = u;
    else
	savedir = NULL;
}

static void  pdir (ustring& path) {
    int  i;

    for (i = path.size () - 1; i > sizeof (cDataTop); i --) {
	if (path[i] == '/') {
	    path.resize (i);
	    return;
	}
    }
}

static void  deleteStore (MlEnv* mlenv) {
    char*  argv1[4];
    ustring  u;

    if (! mlenv->env->storedir.empty ()) {
	argv1[0] = (char*)cmd_rm;
	argv1[1] = (char*)"-fr";
	if (*mlenv->env->storedir.rbegin () == '/') {
	    u.assign (mlenv->env->storedir.begin (), mlenv->env->storedir.end () - 1);
	    argv1[2] = noconst_char (u.c_str ());
	} else {
	    argv1[2] = noconst_char (mlenv->env->storedir.c_str ());
	}
	argv1[3] = NULL;
#ifdef DEBUG
	std::cerr << argv1[0] << " " << argv1[1] << " " << argv1[2] << "\n";
#endif /* DEBUG */
	exec_cmd (argv1);

	mlenv->env->storedir.resize (0);
	mlenv->setVar (uXSerial, NULL);
    }
}

static void  cleanStore (int span, MlEnv* mlenv) {
    BDBBtree  db;
    FileMacro  lock;
    time_t  limit, t;
    DBT1  key, val;
    boost::ptr_vector<ustring>  xs;
    boost::ptr_vector<ustring>  path;
    ustring*  subdir = NULL;
    ustring*  savedir = NULL;
    int  i;
    ustring  u;
    char*  argv1[4];

    limit = now () - span;
    openDB (db, lock, mlenv);

    db.initeach ();
    while (db.each (&key, &val)) {
	if (key.size > 1) {
	    t = strtol ((char*)val.data, NULL, 10);
	    if (t > 0 && t < limit) {
		xs.push_back (new ustring ((char*)key.data, key.size));
		pathRec (&val, subdir, savedir);
		if (subdir)
		    path.push_back (subdir);
		if (savedir)
		    path.push_back (savedir);
	    }
	}
    }
    for (i = 0; i < xs.size (); i ++) {
	db.del (xs[i]);
    }

    db.close ();
    lock.close ();

    argv1[0] = (char*)cmd_rm;
    argv1[1] = (char*)"-fr";
    argv1[3] = NULL;
    for (i = 0; i < path.size (); i ++) {
	assert (path[i].size () > 0);
	u = mlenv->env->path_to_store () + path[i];
	argv1[2] = noconst_char (u.c_str ());
	exec_cmd (argv1);
	pdir (u);			// レベル1
	rmdir (u.c_str ());
	pdir (u);			// レベル2
	rmdir (u.c_str ());
    }
}

class  StorageDB {
public:
    BDBBtree  db;
    FileMacro  lock;
    MlEnv*  mlenv;

    StorageDB (MlEnv* _mlenv) {
	ustring  t = _mlenv->env->path_to_storage_index ();
	ustring  dbfile = t + kEXT_BTREE;
	ustring  lockfile = t + kEXT_LOCK;
	mlenv = _mlenv;
	lock.openAppendLock (lockfile.c_str ());
	db.openRW (dbfile.c_str ());
    };
    virtual  ~StorageDB () {
	db.close ();
	lock.close ();
    };
    virtual ustring  toAbsPath (const ustring& subpath) {
	return mlenv->env->path_to_storage () + subpath;
    };
    virtual void  put (const ustring& name, const ustring& subpath) {
	time_t  tm = now ();
	ustring  v;

	v.assign (to_ustring (tm)).append (uColon).append (subpath);
	db.put (name, v);
    };
    virtual bool  get (const ustring& name, ustring& ans) {
	// ans: subpath to storage directory.
	ustring  v;
	umatch  m;
	static uregex  re ("^[0-9]+:([0-9]+/[^\\x00- /\\x7f]+)$");

	if (db.get (name, v) && usearch (v, m, re)) {
	    ans = ustring (m[1].first, m[1].second);
	    put (name, ans);
	    return true;
	} else {
	    return false;
	}
    };
    virtual bool  del (const ustring& name, ustring& ans) {
	ustring  subpath;

	mlenv->env->storagedir = uEmpty;
	if (get (name, subpath)) {
	    db.del (name);
//	    ans.assign (mlenv->env->path_to_storage ()).append (subpath);
	    ans = toAbsPath (subpath); // no ending slash(/).
	    return true;
	} else {
	    return false;
	}
    };
    virtual ustring  makePath (const ustring& name) {
	// sets mlenv->env->storagedir
	int  i;
	int  sum = 0;
	char  b[8];
	size_t  s;
	ustring  ename;
	ustring  ans;

	ename = filenameEncode (name);
	for (i = 0; i < name.length (); i ++) {
	    sum += name[i];
	}
	sum = (sum & 65535) % 1000;
	mlenv->env->storagedir = mlenv->env->path_to_storage ();
	s = snprintf (b, 8, "%.3d", sum);
	mlenv->env->storagedir.append (b, s);
	mkdir (mlenv->env->storagedir.c_str (), 0777);
	ename = filenameEncode (name);
	mlenv->env->storagedir.append (uSlash).append (ename);
	mkdir (mlenv->env->storagedir.c_str (), 0777);
	mlenv->env->storagedir.append (uSlash);
	ans.assign (b, s).append (uSlash).append (ename);
	return ans;
    };
    virtual void  setStorage (const ustring& name, bool fcreate = false) {
	ustring  subpath;
	if (name.length () == 0) {
	    mlenv->env->storagedir = uEmpty;
	} else if (get (name, subpath)) {
//	    mlenv->env->storagedir.assign (mlenv->env->path_to_storage ()).append (subpath).append (uSlash);
	    mlenv->env->storagedir = toAbsPath (subpath) + uSlash;
	} else if (fcreate) {
	    subpath = makePath (name); // mlenv->env->storagedir
	    put (name, subpath);
	} else {
	    // do not create dir.
	}
    };
    virtual bool  rename (const ustring& nameFrom, const ustring& nameTo) {
	ustring  subpathFrom, subpathTo;
	int  rc;

	if (nameFrom.length () == 0 || nameTo.length () == 0) {
	    return false;
	} else if (get (nameFrom, subpathFrom)) {
	    if (get (nameTo, subpathTo)) {
		// merge
		return false;
	    } else {
		ustring  f = toAbsPath (subpathFrom);
		ustring  t;

		subpathTo = makePath (nameTo); // mlenv->env->storagedir
		t = toAbsPath (subpathTo);
		rmdir (t.c_str ());
		rc = ::rename (f.c_str (), t.c_str ());
#if 0
		std::cerr << "nameFrom:" << nameFrom << " nameTo:" << nameTo << "\n";
		std::cerr << "storagedir:" << mlenv->env->storagedir << "\n";
		std::cerr << "rename (" << f << "," << t << "):" << rc << "\n";
#endif /* DEBUG */
		del (nameFrom, f);
		put (nameTo, subpathTo);
		return true;
	    }
	} else {
	    // no source directory
	    return false;
	}
    };
};

static void  deleteStorage (MlEnv* mlenv, const ustring& name) { // new
    StorageDB  sdb (mlenv);
    ustring  tgt;

    if (sdb.del (name, tgt)) {
	char*  argv1[4];
	argv1[0] = (char*)cmd_rm;
	argv1[1] = (char*)"-fr";
	argv1[2] = noconst_char (tgt.c_str ());
	argv1[3] = NULL;
	exec_cmd (argv1);
    }
}

/*DOC:
===new-serial===
 (new-serial) -> SERIAL

*/
//#XAFUNC	new-serial	ml_new_xserial
MNode*  ml_new_xserial (bool fev, MNode* cell, MlEnv* mlenv) {
    evalParams (fev, mlenv, cell);
    return newStoreSerial (mlenv);
}

/*DOC:
===set-serial===
 (set-serial XSERIAL) -> BOOL
アクセスキーが正しければ，motor変数「XSerial」にアクセスキーが保存され，trueを返す。

*/
//#XAFUNC	set-serial	ml_set_xserial
MNode*  ml_set_xserial (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    evalParams (fev, mlenv, cell, posList, posParams);
    ustring  val = to_string (posParams[0]());
    MNodePtr  h;
    if (setSerial (mlenv, val, mlenv->env->storedir)) {
	h = newMNode_str (new ustring (val));
	mlenv->setVar (uXSerial, h.p);
	return newMNode_bool (true);
    } else {
	mlenv->env->storedir.resize (0);
	mlenv->setVar (uXSerial, NULL);

	return newMNode_bool (false);
    }
}

/*DOC:
===read-file===
 (read-file FILENAME [#serial | #named | #static] [:code ENCODING]) -> STRING

*/
//#XAFUNC	read-file	ml_read_file
MNode*  ml_read_file (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    kwParam  kwList[] = {{CharConst ("code"), EV_LIST},	  // 0
			 {CharConst ("serial"), EV_LIST}, // 1
			 {CharConst ("named"), EV_LIST},  // 2
			 {CharConst ("static"), EV_LIST}, // 3
			 {NULL, 0, EV_END}};
    MNodePtr  kwParams[4];
    evalParams (fev, mlenv, cell, posList, posParams, kwList, kwParams);
    StoreType  storetype (mlenv);
    storetype.setParam (to_string (posParams[0]()));
    ustring  encoding = to_string (kwParams[0]());
    if (to_bool (kwParams[1]()))
	storetype.setSerial ();
    else if (to_bool (kwParams[2]()))
	storetype.setNamed ();
    else if (to_bool (kwParams[3]()))
	storetype.setStatic ();
    ustring  src = storetype.src ();
    ustring  data;
    if (readFile (src, data, cPOSTLIMITHARD)) {
	if (encoding.size () > 0 && data.size () > 0) {
	    UIConv  cd (kCODE_UTF8, encoding.c_str ());
	    data = cd.cv (data, true);
	}
	data = fixUTF8 (data);
	return newMNode_str (new ustring (data));
    } else {
	return NULL;
    }
}

/*DOC:
===write-file===
 (write-file FILENAME STRING [#serial | #named] [:code ENCODING]
 		[#crlf | :crlf BOOL]) -> NIL

*/
//#XAFUNC	write-file	ml_write_file
MNode*  ml_write_file (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_LIST, EV_END};
    MNodePtr  posParams[2];
    kwParam  kwList[] = {{CharConst ("code"), EV_LIST},	  // 0
			 {CharConst ("crlf"), EV_LIST},	  // 1
			 {CharConst ("serial"), EV_LIST}, // 2
			 {CharConst ("named"), EV_LIST},  // 3
			 {NULL, 0, EV_END}};
    MNodePtr  kwParams[4];
    evalParams (fev, mlenv, cell, posList, posParams, kwList, kwParams);
    StoreType  storetype (mlenv);
    storetype.setParam (to_string (posParams[0]()));
    ustring  data = to_string (posParams[1]());
    ustring  encoding = to_string (kwParams[0]());
    bool  fcrlf = to_bool (kwParams[1]());
    if (to_bool (kwParams[2]()))
	storetype.setSerial ();
    else if (to_bool (kwParams[3]()))
	storetype.setNamed ();
    ustring  tgt = storetype.src ();
    if (encoding.size () > 0 && data.size () > 0) {
	UIConv  cd (encoding.c_str (), kCODE_UTF8);
	data = cd.cv (data, true);
    }
    if (fcrlf)
	data = toCRLF (data);
    writeFile (tgt, data);
    return NULL;
}

/*DOC:
===delete-store===
 (delete-store) -> NIL
一時ディレクトリを削除する。

*/
//#XAFUNC	delete-store	ml_delete_store
MNode*  ml_delete_store (bool fev, MNode* cell, MlEnv* mlenv) {
    evalParams (fev, mlenv, cell);
    deleteStore (mlenv);
    return NULL;
}

/*DOC:
===clean-store===
 (clean-store) -> NIL
１日以上経過したデータストアを消去する。

*/
//#XAFUNC	clean-store	ml_clean_store
MNode*  ml_clean_store (bool fev, MNode* cell, MlEnv* mlenv) {
    evalParams (fev, mlenv, cell);
    int  span = 86400;
    cleanStore (span, mlenv);
    return NULL;
}

/*DOC:
===datastore===
 (datastore) -> NIL
 (datastore NAME) -> NIL

NAME must be less than 32 bytes.

*/
//#XAFUNC	datastore	ml_datastore
MNode*  ml_datastore (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    evalParams (fev, mlenv, cell, posList, posParams);
    ustring  name;
    if (isNil (posParams[0]())) {
	mlenv->env->setDefaultDatastore ();
    } else {
	name = to_string (posParams[0]());
	mlenv->env->setDatastore (name);
	mlenv->execDatastoreFunc ();
    }
    return NULL;
}

/*DOC:
===datastore-progn===
 (datastore-progn NAME [:on-error FUNCTION] BODY...) -> NIL

NAME must be less than 32 bytes.

*/
//#XAFUNC	datastore-progn	ml_datastore_progn
MNode*  ml_datastore_progn (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    kwParam  kwList[] = {{CharConst ("on-error"), EV_LIST},	  // 0
			 {NULL, 0, EV_END}};
    MNodePtr  kwParams[1];
    MNodePtr  rest;
    evalParams (fev, mlenv, cell, posList, posParams, kwList, kwParams, EV_ASIS, &rest);
    ustring  name = to_string (posParams[0]());
    MNode*  errfn = kwParams[0]();
    ustring  oname;
    MNodePtr  ans;
    try {
	oname = mlenv->env->datastore;
	mlenv->env->setDatastore (name);
	mlenv->execDatastoreFunc ();
	ans =  progn (rest (), mlenv);
	mlenv->env->setDatastore (oname);
	mlenv->execDatastoreFunc ();
    } catch (ustring& msg) {
	mlenv->env->setDatastore (oname);
	mlenv->execDatastoreFunc ();
	if (errfn) {
	    onErrorFn (errfn, mlenv);
	} else {
	    throw (msg);
	}
    }
    mlenv->stopBreak (cell->car ());
    return mlenv->retval = ans.release ();
}

/*DOC:
===new-storage===
 (new-storage NAME) -> NIL
(set-storage NAME #create)と等価。

*/
//#XAFUNC	new-storage	ml_new_storage
MNode*  ml_new_storage (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    evalParams (fev, mlenv, cell, posList, posParams);
    ustring  name = to_string (posParams[0]());
    {
	StorageDB  sdb (mlenv);
	sdb.setStorage (name, true);
    }
    return NULL;
}

/*DOC:
===set-storage===
 (set-storage NAME [#create]) -> NIL

*/
//#XAFUNC	set-storage	ml_set_storage
MNode*  ml_set_storage (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    kwParam  kwList[] = {{CharConst ("create"), EV_LIST},
			 {NULL, 0, EV_END}};
    MNodePtr  kwParams[1];
    evalParams (fev, mlenv, cell, posList, posParams, kwList, kwParams);
    ustring  name = to_string (posParams[0]());
    bool  fcreate = to_bool (kwParams[0]());
    {
	StorageDB  sdb (mlenv);
#ifdef COMPAT_STORAGE_CREATE
	sdb.setStorage (name, true);
#else
	sdb.setStorage (name, fcreate);
#endif
    }
    return NULL;
}

/*DOC:
===rename-storage===
 (rename-storage NAME_FROM NAME_TO) -> NIL

*/
//#XAFUNC	rename-storage	ml_rename_storage
MNode*  ml_rename_storage (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_LIST, EV_END};
    MNodePtr  posParams[2];
    evalParams (fev, mlenv, cell, posList, posParams);
    ustring  namefrom = to_string (posParams[0]());
    ustring  nameto = to_string (posParams[1]());
    bool  ans;
    {
	StorageDB  sdb (mlenv);
	ans = sdb.rename (namefrom, nameto);
    }
    return newMNode_bool (ans);
}

/*DOC:
===delete-storage===
 (delete-storage NAME) -> NIL

*/
//#XAFUNC	delete-storage	ml_delete_storage
MNode*  ml_delete_storage (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    evalParams (fev, mlenv, cell, posList, posParams);
    ustring  name = to_string (posParams[0]());
    deleteStorage (mlenv, name);
    return NULL;
}

/*DOC:
===save-file===
 (save-file NAME_STORE NAME_STORAGE) -> NIL

*/
//#XAFUNC	save-file	ml_save_file
MNode*  ml_save_file (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_LIST, EV_END};
    MNodePtr  posParams[2];
    evalParams (fev, mlenv, cell, posList, posParams);
    ustring  namestore = to_string (posParams[0]());
    ustring  namestorage = to_string (posParams[1]());
    if (mlenv->env->storedir.empty ())
	throw (uErrorNoStore);
    if (mlenv->env->storagedir.empty ())
	throw (uErrorNoStorage);
    ustring  src = mlenv->env->path_store_file (namestore);
    ustring  tgt = mlenv->env->path_storage_file (namestorage);
    if (src.size () == 0)
	throw (namestore + uErrorBadFile);
    if (tgt.size () == 0)
	throw (namestorage + uErrorBadFile);
    rename (src.c_str (), tgt.c_str ());
    symlink (tgt.c_str (), src.c_str ());
    return NULL;
}

/*DOC:
===restore-file===
 (restore-file NAME_STORAGE NAME_STORE) -> NIL

*/
//#XAFUNC	restore-file	ml_restore_file
MNode*  ml_restore_file (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_LIST, EV_END};
    MNodePtr  posParams[2];
    evalParams (fev, mlenv, cell, posList, posParams);
    ustring  namestore = to_string (posParams[0]());
    ustring  namestorage = to_string (posParams[1]());
    if (mlenv->env->storedir.empty ())
	throw (uErrorNoStore);
    if (mlenv->env->storagedir.empty ())
	throw (uErrorNoStorage);
    ustring  src = mlenv->env->path_storage_file (namestorage);
    ustring  tgt = mlenv->env->path_store_file (namestore);
    if (src.size () == 0)
	throw (namestorage+ uErrorBadFile);
    if (tgt.size () == 0)
	throw (namestore + uErrorBadFile);
    unlink (tgt.c_str ());
    symlink (src.c_str (), tgt.c_str ());
    return NULL;
}

/*DOC:
===delete-file===
 (delete-file NAME_STORAGE [#serial | #named]) -> NIL

*/
//#XAFUNC	delete-file	ml_delete_file
MNode*  ml_delete_file (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    kwParam  kwList[] = {{CharConst ("serial"), EV_LIST},	 // 0
			 {CharConst ("named"), EV_LIST},	 // 1
			 {NULL, 0, EV_END}};
    MNodePtr  kwParams[2];
    evalParams (fev, mlenv, cell, posList, posParams, kwList, kwParams);
    StoreType  storetype (mlenv);
    storetype.setParam (to_string (posParams[0]()));
    if (to_bool (kwParams[0]()))
	storetype.setSerial ();
    if (to_bool (kwParams[1]()))
	storetype.setNamed ();
    ustring  tgt = storetype.src ();
    if (tgt.length () > 0)
	unlink (tgt.c_str ());
    return NULL;
}

/*DOC:
===rename-file===
 (rename-file FROM_FILE TO_FILE [#serial | #named]) -> NIL

*/
//#XAFUNC	rename-file	ml_rename_file
MNode*  ml_rename_file (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_LIST, EV_END};
    MNodePtr  posParams[2];
    kwParam  kwList[] = {{CharConst ("serial"), EV_LIST},	 // 0
			 {CharConst ("named"), EV_LIST},	 // 1
			 {NULL, 0, EV_END}};
    MNodePtr  kwParams[2];
    evalParams (fev, mlenv, cell, posList, posParams, kwList, kwParams);
    ustring  name_from = to_string (posParams[0]());
    ustring  name_to = to_string (posParams[1]());
    StoreType  storetype (mlenv);
    if (to_bool (kwParams[0]()))
	storetype.setSerial ();
    else if (to_bool (kwParams[1]()))
	storetype.setNamed ();
    ustring  tgt_from = storetype.src (name_from);
    ustring  tgt_to = storetype.src (name_to);
    if (tgt_from.size () == 0)
	throw (name_from + uErrorBadFile);
    if (tgt_to.size () == 0)
	throw (name_to + uErrorBadFile);
    ::rename (tgt_from.c_str (), tgt_to.c_str ());
    return NULL;
}

/*DOC:
===filesize===
 (filesize FILENAME [#serial | #named | #static]) -> INTEGER or NIL
// (file-size [:source-serial FILENAME | :source-named FILENAME | :source-static FILENAME]) -> INTEGER or NIL

*/
//#XAFUNC	filesize	ml_filesize
MNode*  ml_filesize (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    kwParam  kwList[] = {{CharConst ("serial"), EV_LIST},	 // 0
			 {CharConst ("named"), EV_LIST},	 // 1
			 {CharConst ("static"), EV_LIST},	 // 2
			 {NULL, 0, EV_END}};
    MNodePtr  kwParams[3];
    evalParams (fev, mlenv, cell, posList, posParams, kwList, kwParams);
    StoreType  storetype (mlenv);
    storetype.setParam (to_string (posParams[0]()));
    if (to_bool (kwParams[0]()))
	storetype.setSerial ();
    else if (to_bool (kwParams[1]()))
	storetype.setNamed ();
    else if (to_bool (kwParams[2]()))
	storetype.setStatic ();
    ustring  src = storetype.src ();
    off_t  size;
    if (src.length () > 0 && fileSize (src, size)) {
	return newMNode_num (size);
    } else {
	return NULL;
    }
}

/*DOC:
===list-files===
 (list-files [#serial | #named]) -> LIST_of_FILENAMES
 (list-files #static PATH) -> LIST_of_FILENAMES

*/
//#XAFUNC	list-files	ml_list_files
MNode*  ml_list_files (bool fev, MNode* cell, MlEnv* mlenv) {
    kwParam  kwList[] = {{CharConst ("serial"), EV_LIST},	 // 0
			 {CharConst ("named"), EV_LIST},	 // 1
			 {CharConst ("static"), EV_LIST},	 // 2
			 {NULL, 0, EV_END}};
    MNodePtr  kwParams[3];
    MNodePtr  rest;
    evalParams (fev, mlenv, cell, NULL, NULL, kwList, kwParams, EV_LIST, &rest);
    StoreType  storetype (mlenv);
    if (to_bool (kwParams[0]()))
	storetype.setSerial ();
    if (to_bool (kwParams[1]()))
	storetype.setNamed ();
    if (to_bool (kwParams[2]()))
	storetype.setStatic ();
    ustring  src;
    switch (storetype.type) {
    case StoreType::F_SERIAL:
	if (! isNil (rest ()))
	    throw (uErrorWrongNumber);
	if (mlenv->env->storedir.empty ())
	    throw (uErrorNoStore);
	src = mlenv->env->storedir;
	break;
    case StoreType::F_NAMED:
	if (! isNil (rest ()))
	    throw (uErrorWrongNumber);
	if (mlenv->env->storagedir.empty ())
	    throw (uErrorNoStorage);
	src = mlenv->env->storagedir;
	break;
    case StoreType::F_STATIC:
	{
	    MNode*  a = rest ();
	    if (isNil (a))
		throw (uErrorWrongNumber);
	    MNode*  path = a->car ();
	    nextNode (a);
	    if (! isNil (a))
		throw (uErrorWrongNumber);
	    src = storetype.src (to_string (path));
	}
	break;
    default:
	throw (uErrorNoStore);
    }
    return listFiles (src);
}

/*DOC:
===open-for-write===
 (open-for-write FILENAME [#serial | #named] #bom BODY...) -> NIL

*/
//#XMFUNC	open-for-write	ml_open_for_write	cMLOpenFileID
MNode*  ml_open_for_write (bool fev, MNode* cell, MlEnv* mlenv) {
    MLOpenFile  obj (mlenv);
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    kwParam  kwList[] = {{CharConst ("serial"), EV_LIST}, // 0
			 {CharConst ("named"), EV_LIST},  // 1
			 {CharConst ("bom"), EV_LIST},  // 2
			 {NULL, 0, EV_END}};
    MNodePtr  kwParams[3];
    MNodePtr  rest;
    evalParams (fev, mlenv, cell, posList, posParams, kwList, kwParams, EV_ASIS, &rest);
    StoreType  storetype (mlenv);
    storetype.setParam (to_string (posParams[0]()));
    if (to_bool (kwParams[0]()))	// serial
	storetype.setSerial ();
    else if (to_bool (kwParams[1]()))	// named
	storetype.setNamed ();
    ustring  tgt = storetype.src ();
    bool  fbom = to_bool (kwParams[2]());
    obj.fp.openWrite (tgt.c_str ());
    obj.fp.write (CharConst ("\xef\xbb\xbf"));
    mlenv->setMStack (&obj);
    MNodePtr  ans;
    try {
	ans = progn (rest (), mlenv);
    } catch (ustring& msg) {
	throw (msg);
    }
    mlenv->stopBreak (cell->car ());

    return NULL;
}

/*DOC:
===subfunctions of open-for-write===

*/
/*DOC:
====file-write====
 (file-write TEXT) -> nil

*/
//#XSFUNC	file-write	ml_open_for_write_file_write	open-for-write
MNode*  ml_open_for_write_file_write (bool fev, MNode* cell, MlEnv* mlenv, MLFunc* mobj) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    evalParams (fev, mlenv, cell, posList, posParams);
    MLOpenFile*  obj = MObjRef<MLOpenFile> (mobj, cMLOpenFileID);
    ustring  data = to_string (posParams[0]());
    if (data.size () > 0)
	obj->fp.write (&data.at (0), data.size ());
    return NULL;
}
