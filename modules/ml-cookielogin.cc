#include "ml-cookielogin.h"
#include "ml.h"
#include "mlenv.h"
#include "motorconst.h"
#include "motorenv.h"
#include "expr.h"
#include "http.h"
#include "util_const.h"
#include "util_check.h"
#include "util_random.h"
#include "util_string.h"
#include "util_splitter.h"
#include "util_time.h"
#include "sigsafe.h"
#include "bdbmacro.h"
#include "filemacro.h"
#include <exception>

/*DOC:
==cookie authentication module==

*/

/*
  session record:
  key: <SessionKey>
  val: <ID>:<Limit>:<Avail>:<Group>:<IP>
*/

static void  splitRec (ustring& rec, ustring& id, ustring& limit, ustring& avail, ustring& group, ustring& ip) {
    SplitterCh  sp (rec, ':');

    sp.nextSep ();
    id = sp.pre ();
    sp.nextSep ();
    limit = sp.pre ();
    sp.nextSep ();
    avail = sp.pre ();
    sp.nextSep ();
    group = sp.pre ();
    sp.nextSep ();
    ip = sp.pre ();
    return;
}

static int  login_check (MLCookieLogin* obj, ustring& key, ustring& rid) {
    ustring  rec;
    ustring  id, limit, avail, group, ip;
    time_t  t;

    if (key.length () == 0)
	return 0;

    if (obj->db.get (key, rec)) {
	splitRec (rec, id, limit, avail, group, ip);
	if (limit.size () > 0) { // type 1 record
	    t = strtol (limit);
	    if (id.size () > 0 && t >= now ()) {
		rec = id;
		rec.append (uColon).append (to_ustring (now () + strtol (avail))).append (uColon).append (avail).append (uColon).append (group).append (uColon).append (ip);
		obj->db.put (key, rec);
		rid = id;
		return 1;	// type 1 record
	    }
	} else {
	    rid = id;
	    return 2;		// type 2 record
	}
    }
    return 0;
}

void  MLCookieLogin::opendb () {
    ustring  dbfile, lockfile;
    
    dbfile = ustring (dbpath) + kEXT_HASH;
    lockfile = ustring (dbpath) + kEXT_LOCK;

    lock.openAppendLock (lockfile.c_str ());
    db.open (dbfile.c_str ());
}

void  MLCookieLogin::closedb () {
    if (lock.isOpen ()) {
	db.close ();
	lock.close ();
    }
}


/*DOC:
===$login-cookie===
 ($login-cookie DB SESSIONKEY [:on-error FUNCTION]
 	SUBFUNCTION...) -> VALUE of last SUBFUNCTION

SESSIONKEY, SESSIONKEY+"N"
*/

//#XMFUNC	$login-cookie	ml_cookielogin	cMLCookieLoginID
MNode*  ml_cookielogin (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_LIST, EV_END};
    MNodePtr  posParams[2];
    kwParam  kwList[] = {{CharConst ("on-error"), EV_LIST},
			 {NULL, 0, EV_END}};
    MNodePtr  kwParams[1];
    MNodePtr  rest;
    evalParams (fev, mlenv, cell, posList, posParams, kwList, kwParams, EV_ASIS, &rest);
    MLCookieLogin  obj (mlenv);
    MNodePtr  ans;
    ustring  name = to_string (posParams[0]());
    obj.sessionkey = to_string (posParams[1]());
    if (name.size () == 0)
	throw (uErrorFilenameEmpty);
    if (! matchName (name))
	throw (name + uErrorBadName);
    if (mlenv->env) {
	SigSafe  sig;
	obj.dbpath = mlenv->env->path_to_auth (name);
	mlenv->setMStack (&obj);
	try {
	    ans = progn (rest (), mlenv);
	} catch (ustring& msg) {
	    MNode*  errfn = kwParams[0]();
	    if (errfn) {
		onErrorFn (errfn, mlenv);
	    } else {
		throw (msg);
	    }
	}
	mlenv->stopBreak (cell->car ());
    }

    return mlenv->retval = ans.release ();
}

/*DOC:
===subfunctions of $login-cookie===

*/
/*DOC:
====login====
 (login UID AVAIL_SEC [:group GROUP] [:ip REMOTE] [:path PATH] [:domain DOMAIN] [:span SECOND] [:limit TIME] [#secure | :secure BOOL]) -> NIL

secureオプションをつけると、cookieをセットする際、secureオプション付きのcookieとオプションなしのcookieをセットする。
それぞれ$login-cookie関数のパラメータSESSIONKEYの後ろに"N"を付加したものと、SESSIONKEYの値にセットする。

*/
//#XSFUNC	login	ml_cookielogin_login
MNode*  ml_cookielogin_login (bool fev, MNode* cell, MlEnv* mlenv, MLFunc* mobj) {
    paramType  posList[] = {EV_LIST, EV_LIST, EV_END};
    MNodePtr  posParams[2];
    kwParam  kwList[] = {{CharConst ("group"), EV_LIST},  // 0
			 {CharConst ("ip"), EV_LIST},	  // 1
			 {CharConst ("path"), EV_LIST},	  // 2
			 {CharConst ("domain"), EV_LIST}, // 3
			 {CharConst ("span"), EV_LIST},	  // 4
			 {CharConst ("limit"), EV_LIST},  // 5
			 {CharConst ("secure"), EV_LIST}, // 6
			 {NULL, 0, EV_END}};
    MNodePtr  kwParams[7];
    evalParams (fev, mlenv, cell, posList, posParams, kwList, kwParams);
    MLCookieLogin*  obj = MObjRef<MLCookieLogin> (mobj, cMLCookieLoginID);
    ustring  id = to_text1 (posParams[0]());
    time_t  avail = to_int64 (posParams[1]());
    if (avail < 0)
	avail = 0;
    ustring  group = to_text1 (kwParams[0]());
    ustring  ip = to_asciiword (kwParams[1]());
    ustring  path = to_asciiword (kwParams[2]());
    ustring  domain = to_asciiword (kwParams[3]());
    time_t  span = to_int64 (kwParams[4]());
    if (span < 0)
	span = 0;
    time_t  cookielimit = to_int64 (kwParams[5]());
    if (cookielimit < 0)
	cookielimit = 0;
    bool  fsecure = to_bool (kwParams[6]());
    time_t  limit;
    ustring  r;
    if (id.size () > 0) {
	// write to the db
	obj->opendb ();
	r.append (clipColon (id)).append (uColon);
	limit = now () + avail;
	r.append (to_ustring (limit)).append (uColon).append (to_ustring (avail)).append (uColon).append (clipColon (group));
	if (ip.size () > 0 && matchIP (ip)) {
	    r.append (uColon).append (ip);	// IP
	} else {
	    r.append (uColon);
	}
	obj->sessionval = randomKey ();
	obj->db.put (obj->sessionval, r);
	// write a cookie
	if (obj->sessionkey.size () > 0) {
	    ustring  key2 (obj->sessionkey);
	    key2.append (CharConst ("N"));
	    if (fsecure) {
		if (isHTTPS ()) {
		    obj->sessionval_ssl = randomKey ();
		    obj->db.put (obj->sessionval_ssl, obj->sessionval);
		    mlenv->env->http.setCookie (obj->sessionkey, obj->sessionval, path, span, cookielimit, domain, true, mlenv->env);
		    mlenv->env->http.setCookie (key2, obj->sessionval_ssl, path, span, cookielimit, domain, false, mlenv->env);
		} else {
		    mlenv->env->http.setCookie (obj->sessionkey, obj->sessionval, path, span, cookielimit, domain, false, mlenv->env);
		}
	    } else {
		mlenv->env->http.setCookie (key2, obj->sessionval, path, span, cookielimit, domain, false, mlenv->env);
	    }
	}
	obj->closedb ();
    }
    return NULL;
}

/*DOC:
====logout====
 (logout) -> NIL

*/
//#XSFUNC	logout	ml_cookielogin_logout
MNode*  ml_cookielogin_logout (bool fev, MNode* cell, MlEnv* mlenv, MLFunc* mobj) {
    evalParams (fev, mlenv, cell);
    MLCookieLogin*  obj = MObjRef<MLCookieLogin> (mobj, cMLCookieLoginID);
    ustring  key = mlenv->env->http.readCookie (obj->sessionkey);
    obj->opendb ();
    obj->db.del (key);
    ustring  id;
    int  rc = login_check (obj, key, id);
    if (rc == 2) {
	obj->db.del (id);
    }
    if (! isHTTPS ()) {
	ustring  key2 (obj->sessionkey);
	key2.append (CharConst ("N"));
	key = mlenv->env->http.readCookie (key2);
	if (obj->db.get (key, key2)) {
	    obj->db.del (key2);
	}
	obj->db.del (key);
    }
    obj->closedb ();
    return NULL;
}

/*DOC:
====check====
 (check [#secure | :secure BOOL]) -> ID or NIL

*/
//#XSFUNC	check	ml_cookielogin_check
MNode*  ml_cookielogin_check (bool fev, MNode* cell, MlEnv* mlenv, MLFunc* mobj) {
    kwParam  kwList[] = {{CharConst ("secure"), EV_LIST},
			 {NULL, 0, EV_END}};
    MNodePtr  kwParams[1];
    MLCookieLogin*  obj = MObjRef<MLCookieLogin> (mobj, cMLCookieLoginID);
    bool  fsecure = to_bool (kwParams[0]());
    ustring  key = mlenv->env->http.readCookie (obj->sessionkey);
    obj->opendb ();
    ustring  id;
    int  rc = login_check (obj, key, id);
    switch (rc) {
    case 1:
	obj->closedb ();
	return newMNode_str (new ustring (id));
	break;
    case 2:
	rc = login_check (obj, id, id);
	if (rc == 1) {
	    obj->closedb ();
	    return newMNode_str (new ustring (id));
	} else {
	    obj->closedb ();
	    return NULL;
	}
	break;
    }
    if (fsecure && ! isHTTPS ()) {
	ustring  key2 (obj->sessionkey);
	key2.append (CharConst ("N"));
	key = mlenv->env->http.readCookie (key2);
	rc = login_check (obj, key, id);
	if (rc == 2) {
	    rc = login_check (obj, id, id);
	    if (rc == 1) {
		obj->closedb ();
		return newMNode_str (new ustring (id));
	    } else {
		obj->closedb ();
		return NULL;
	    }
	}
    }
    obj->closedb ();

    return NULL;
}

/*DOC:
====delete====
 (delete ID) -> NIL

*/
//#XSFUNC	delete	ml_cookielogin_delete
MNode*  ml_cookielogin_delete (bool fev, MNode* cell, MlEnv* mlenv, MLFunc* mobj) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    evalParams (fev, mlenv, cell, posList, posParams);
    MLCookieLogin*  obj = MObjRef<MLCookieLogin> (mobj, cMLCookieLoginID);
    ustring  id = to_string (posParams[0]());
    obj->opendb ();
    obj->db.initeach ();
    ustring  key, val;
    std::vector<ustring>  keys;
    std::vector<ustring>::iterator  it;
    while (obj->db.each (key, val)) {
	SplitterCh  sp (val, ':');
	if (sp.nextSep ()) {	// セパレータが存在するとき、idを含むレコード
	    if (sp.pre () == id) {
		keys.push_back (key);
	    }
	}
    }
    for (it = keys.begin (); it != keys.end (); it ++) {
	obj->db.del (*it);
    }
    obj->closedb ();
    return NULL;
}

/*DOC:
====clear====
 (clear) -> NIL

*/
//#XSFUNC	clear	ml_cookielogin_clear
MNode*  ml_cookielogin_clear (bool fev, MNode* cell, MlEnv* mlenv, MLFunc* mobj) {
    evalParams (fev, mlenv, cell);
    MLCookieLogin*  obj = MObjRef<MLCookieLogin> (mobj, cMLCookieLoginID);
    ustring  id, limit, avail, group, ip;
    obj->opendb ();
    obj->db.initeach ();
    ustring  key, val;
    time_t  t;
    time_t  tn = now ();
    std::vector<ustring>  keys;
    std::vector<ustring>::iterator  it;
    while (obj->db.each (key, val)) {
	splitRec (val, id, limit, avail, group, ip);
	if (limit.length () > 0) {
	    t = strtol (limit);
	    if (t > 0 && t < tn) { // 期限切れ
		keys.push_back (key);
	    }
	} else {		// secure cookie
	    if (obj->db.get (id, val)) {
		splitRec (val, val, limit, avail, group, ip);
		if (limit.length () > 0) {
		    t = strtol (limit);
		    if (t > 0 && t < tn) {
			keys.push_back (key);
		    }
		}
	    } else {		// nonsecure cookieが消去されている
		keys.push_back (key);
	    }
	}
    }
    for (it = keys.begin (); it != keys.end (); it ++) {
	obj->db.del (*it);
    }
    obj->closedb ();
    return NULL;
}

/*DOC:
====session-key====
 (session-key) -> STRING

*/
//#XSFUNC	session-key	ml_cookielogin_session_key
MNode*  ml_cookielogin_session_key (bool fev, MNode* cell, MlEnv* mlenv, MLFunc* mobj) {
    evalParams (fev, mlenv, cell);
    MNode*  arg = cell->cdr ();
    MLCookieLogin*  obj = MObjRef<MLCookieLogin> (mobj, cMLCookieLoginID);

    if (arg)
	throw (uErrorWrongNumber);

    return newMNode_str (new ustring (obj->sessionval));
}

/*DOC:
====session-key-ssl====
 (session-key-ssl) -> STRING

*/
//#XSFUNC	session-key-ssl	ml_cookielogin_session_key_ssl
MNode*  ml_cookielogin_session_key_ssl (bool fev, MNode* cell, MlEnv* mlenv, MLFunc* mobj) {
    evalParams (fev, mlenv, cell);
    MLCookieLogin*  obj = MObjRef<MLCookieLogin> (mobj, cMLCookieLoginID);
    return newMNode_str (new ustring (obj->sessionval_ssl));
}
