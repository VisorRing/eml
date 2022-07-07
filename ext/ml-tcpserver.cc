#include "ml-tcpserver.h"
#include "motorconst.h"
#include "ml.h"
#include "mlenv.h"
#include "motorenv.h"
#include "ustring.h"
#include "expr.h"
#include "util_const.h"
#include "util_check.h"
#include "util_string.h"
#include "bdbmacro.h"
#include "sigsafe.h"
#include <sys/file.h>
#include <string.h>
#include <time.h>
#include <exception>

/*DOC:
==dbtcpserver module==

*/
static uregex  re_iprange ("^[0-9]{1,3}\\.([0-9]{1,3}\\.([0-9]{1,3}\\.([0-9]{1,3})?)?)?$");
static uregex  re_domainrange ("^\\.?[a-zA-Z0-9][a-zA-Z0-9\\-]*(\\.[a-zA-Z0-9][a-zA-Z0-9\\-]*)*$");

void  MLDbTcpserver::setPath (const ustring& name) {
    dbpath = mlenv->env->path_to_auth (name);
    dbpath.append (CharConst (kEXT_HASH));
}

void  MLDbTcpserver::opendb () {
    db.open (dbpath.c_str ());
    flock (db.db->fd (db.db), LOCK_EX);
}

void  MLDbTcpserver::closedb () {
    if (db.db) {
	flock (db.db->fd (db.db), LOCK_UN);
	db.close ();
    }
}

void  MLDbTcpserver::addAllow (const ustring& key, time_t span, MNode* rest, MlEnv* mlenv) {
    ustring  val;
    ustring  estr;

    if (span < 0)
	span = 1;		// timeout
    if (span > 0) {
	span += time (NULL);
	val.append (CharConst ("\xff\xff\xff\xff")).append ((const char*)(&span), sizeof (time_t));
    }
    if (rest) {
	while (rest) {
	    estr = to_string (rest->car ());
	    nextNode (rest);
	    if (! matchASCII (estr.begin (), estr.end ()) || estr.length () > 1024)
		throw (estr + uErrorBadValue);
	    val.append (CharConst ("+")).append (estr).append (1, 0);
	}
    }
    db.put (key, val);
}

void  MLDbTcpserver::addDeny (const ustring& key) {
    ustring  val;

    val.assign (CharConst ("D\x00"));
    db.put (key, val);
}

void  MLDbTcpserver::del (const ustring& key) {
    db.del (key);
}

bool  MLDbTcpserver::splitRec (const ustring& rec, bool& allow, time_t& limit, MNodeList& estr) {
    uiterator  b, e, m;

    b = rec.begin ();
    e = rec.end ();
    allow = true;
    limit = 0;
    if (b == e) {
	return true;
    } else if (matchHead (b, e, CharConst ("D\x00"))) {
	allow = false;
	limit = 0;
	return true;
    } else if (matchSkip (b, e, CharConst ("\xff\xff\xff\xff"))) {
	if (e - b >= sizeof (time_t)) {
	    allow = true;
	    memcpy ((void*)&limit, &*b, sizeof (time_t));
	    b += sizeof (time_t);
	} else {
	    return false;
	}
    }
    while (b <= e) {
	if (b == e) {
	    return true;
	} else {
	    if (*b == '+') {
		b ++;
		if (splitChar (b, e, '\x00', m)) {
		    estr.append (newMNode_str (new ustring (b, m)));
		    b = m + 1;
		}
	    }
	}
    }
    return false;
}

bool  MLDbTcpserver::splitRec (const ustring& rec, bool& allow, time_t& limit) {
    uiterator  b, e, m;

    b = rec.begin ();
    e = rec.end ();
    allow = true;
    limit = 0;
    if (b == e) {
	return true;
    } else if (matchHead (b, e, CharConst ("D\x00"))) {
	allow = false;
	limit = 0;
	return true;
    } else if (matchSkip (b, e, CharConst ("\xff\xff\xff\xff"))) {
	if (e - b >= sizeof (time_t)) {
	    allow = true;
	    memcpy ((void*)&limit, &*b, sizeof (time_t));
	    b += sizeof (time_t);
	} else {
	    return false;
	}
    }
    return true;
}

/*DOC:
===$dbtcpserver===
 ($dbtcpserver NAME [SUBFUNCTION...])
DJBのucspi-tcp拡張版のBerkeleyDBアクセス制御データベースのハンドリング。

*/
//#XMFUNC	$dbtcpserver	ml_dbtcpserver	cMLDbTcpserverID
MNode*  ml_dbtcpserver (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    MNodePtr  rest;
    evalParams (fev, mlenv, cell, posList, posParams, NULL, NULL, EV_ASIS, &rest);
    MLDbTcpserver  obj (mlenv);
    ustring  name = to_string (posParams[0]());
    if (name.size () == 0)
	throw (uErrorFilenameEmpty);
    if (! matchName (name))
	throw (name + uErrorBadName);
    MNodePtr  ans;
    if (mlenv->env) {
	SigSafe  sig;
	obj.setPath (name);

	mlenv->setMStack (&obj);
	obj.opendb ();
	ans = progn (rest (), mlenv);
	mlenv->stopBreak (cell->car ());
	obj.closedb ();
    }

    return mlenv->retval = ans ();
}

/*DOC:
===subfunctions of $dbtcpserver===

*/
/*DOC:
====add-allow-ip====
 (add-allow-ip IP SPAN ENV...) -> NIL
IPアドレス，アドレスレンジをキーにしたallow行を追加する。
ドットで終わる４オクテット未満のアドレスを指定すると，アドレスレンジ指定となる。
SPANが0以上の時，登録時からSPAN秒後までの間，レコードが有効になる。
環境変数指定を「名前=値」の形式で指定する。環境変数指定は複数指定できる。

*/
//#XSFUNC	add-allow-ip	ml_dbtcpserver_add_allow_ip
MNode*  ml_dbtcpserver_add_allow_ip (bool fev, MNode* cell, MlEnv* mlenv, MLFunc* mobj) {
    paramType  posList[] = {EV_LIST, EV_LIST, EV_END};
    MNodePtr  posParams[2];
    MNodePtr  rest;
    evalParams (fev, mlenv, cell, posList, posParams, NULL, NULL, EV_LIST, &rest);
    MLDbTcpserver*  obj = MObjRef<MLDbTcpserver> (mobj, cMLDbTcpserverID);
    ustring  ip = to_string (posParams[0]());
    time_t  span = to_int64 (posParams[1]());
    if (ip.length () > 0 && ! checkRe (ip, re_iprange))
	throw (ip + uErrorBadValue);
    obj->addAllow (ip, span, rest (), mlenv);
    return NULL;
}

/*DOC:
====add-allow-host====
 (add-allow-host DOMAIN SPAN ENV...) -> NIL
ホスト名，ドメイン名をキーにしたallow行を追加する。
ドットで始まるドメイン名を指定すると，ドメイン全体の指定となる。
SPANが0以上の時，登録時からSPAN病後までの間，レコードが有効になる。
環境変数指定を「名前=値」の形式で指定する。環境変数指定は複数指定できる。

*/
//#XSFUNC	add-allow-host	ml_dbtcpserver_add_allow_host
MNode*  ml_dbtcpserver_add_allow_host (bool fev, MNode* cell, MlEnv* mlenv, MLFunc* mobj) {
    paramType  posList[] = {EV_LIST, EV_LIST, EV_END};
    MNodePtr  posParams[2];
    MNodePtr  rest;
    evalParams (fev, mlenv, cell, posList, posParams, NULL, NULL, EV_LIST, &rest);
    MLDbTcpserver*  obj = MObjRef<MLDbTcpserver> (mobj, cMLDbTcpserverID);
    ustring  host = to_string (posParams[0]());
    time_t  span = to_int64 (posParams[1]());
    if (host.length () > 0 && ! checkRe (host, re_domainrange))
	throw (host + uErrorBadValue);
    if (host.length () > 0)
	host = ustring (CharConst ("=")).append (host);
    obj->addAllow (host, span, rest (), mlenv);
    return NULL;
}

/*DOC:
====add-deny-ip====
 (add-deny-ip IP) -> NIL
IPアドレス，アドレスレンジをキーにしたdeny行を追加する。
ドットで終わる４オクテット未満のアドレスを指定すると，アドレスレンジ指定となる。

*/
//#XSFUNC	add-deny-ip	ml_dbtcpserver_add_deny_ip
MNode*  ml_dbtcpserver_add_deny_ip (bool fev, MNode* cell, MlEnv* mlenv, MLFunc* mobj) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    evalParams (fev, mlenv, cell, posList, posParams);
    MLDbTcpserver*  obj = MObjRef<MLDbTcpserver> (mobj, cMLDbTcpserverID);
    ustring  ip = to_string (posParams[0]());
    if (ip.length () > 0 && ! checkRe (ip, re_iprange))
	throw (ip + uErrorBadValue);
    obj->addDeny (ip);
    return NULL;
}

/*DOC:
====add-deny-host====
 (add-deny-host DOMAIN) -> NIL
ホスト名，ドメイン名をキーにしたdeny行を追加する。
ドットで始まるドメイン名を指定すると，ドメイン全体の指定となる。

*/
//#XSFUNC	add-deny-host	ml_dbtcpserver_add_deny_host
MNode*  ml_dbtcpserver_add_deny_host (bool fev, MNode* cell, MlEnv* mlenv, MLFunc* mobj) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    evalParams (fev, mlenv, cell, posList, posParams);
    MLDbTcpserver*  obj = MObjRef<MLDbTcpserver> (mobj, cMLDbTcpserverID);
    ustring  host = to_string (posParams[0]());
    if (host.length () > 0 && ! checkRe (host, re_domainrange))
	throw (host + uErrorBadValue);
    if (host.length () > 0)
	host = ustring (CharConst ("=")).append (host);
    obj->addDeny (host);
    return NULL;
}

/*DOC:
====delete-ip====
 (delete-ip IP) -> NIL
IPアドレスをキーとして登録したレコードを削除する。

*/
//#XSFUNC	delete-ip	ml_dbtcpserver_delete_ip
MNode*  ml_dbtcpserver_delete_ip (bool fev, MNode* cell, MlEnv* mlenv, MLFunc* mobj) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    evalParams (fev, mlenv, cell, posList, posParams);
    MLDbTcpserver*  obj = MObjRef<MLDbTcpserver> (mobj, cMLDbTcpserverID);
    ustring  ip = to_string (posParams[0]());
    if (ip.length () > 0 && ! checkRe (ip, re_iprange))
	throw (ip + uErrorBadValue);
    obj->del (ip);
    return NULL;
}

/*DOC:
====delete-host====
 (delete-host DOMAIN)
ドメイン名をキーとして登録したレコードを削除する。

*/
//#XSFUNC	delete-host	ml_dbtcpserver_delete_host
MNode*  ml_dbtcpserver_delete_host (bool fev, MNode* cell, MlEnv* mlenv, MLFunc* mobj) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    evalParams (fev, mlenv, cell, posList, posParams);
    MLDbTcpserver*  obj = MObjRef<MLDbTcpserver> (mobj, cMLDbTcpserverID);
    ustring  host = to_string (posParams[0]());
    if (host.length () > 0 && ! checkRe (host, re_domainrange))
	throw (host + uErrorBadValue);
    if (host.length () > 0)
	host = ustring (CharConst ("=")).append (host);
    obj->del (host);
    return NULL;
}

static MNode*  accessRec (MLDbTcpserver* obj, const ustring& key) {
    ustring  val;
    bool  allow;
    time_t  limit;
    MNodeList  estr;

    if (obj->db.get (key, val)) {
	if (obj->splitRec (val, allow, limit, estr)) {
	    MNodeList  ans;
	    if (allow) {
		ans.append (newMNode_str (new ustring (CharConst ("allow"))));
	    } else {
		ans.append (newMNode_str (new ustring (CharConst ("deny"))));
	    }
	    ans.append (newMNode_num (limit));
	    ans.append (estr.release ());
	    return ans.release ();
	}
    }
    return NULL;
}

//static void  accessRec (MLDbTcpserver* obj, const ustring& key, const ustring& val, MNodeList& lkey, MNodeList& lallow, MNodeList& llimit, MNodeList& lestr) {
static void  accessRec (MLDbTcpserver* obj, const ustring& key, const ustring& val, MNodeList& ans) {
    bool  allow;
    time_t  limit;
    MNodeList  estr;
    MNodeList  rec;

    if (obj->splitRec (val, allow, limit, estr)) {
	rec.append (newMNode_str (new ustring (key)));
	if (allow) {
	    rec.append (newMNode_str (new ustring (CharConst ("allow"))));
	} else {
	    rec.append (newMNode_str (new ustring (CharConst ("deny"))));
	}
	rec.append (newMNode_num (limit));
	rec.append (estr.release ());
	ans.append (rec.release ());
    }
}

/*DOC:
====read-ip====
 (read-ip IP) -> (["allow" | "deny"] TIME ENV_LIST)
IPアドレスをキーとしてレコードを読み取る。
レコードの期限が指定されているものは，期限の時刻のUNIX timeが返される。
環境変数指定はリストで返される。

*/
//#XSFUNC	read-ip	ml_dbtcpserver_read_ip
MNode*  ml_dbtcpserver_read_ip (bool fev, MNode* cell, MlEnv* mlenv, MLFunc* mobj) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    evalParams (fev, mlenv, cell, posList, posParams);
    MLDbTcpserver*  obj = MObjRef<MLDbTcpserver> (mobj, cMLDbTcpserverID);
    ustring  ip = to_string (posParams[0]());
    if (ip.length () > 0 && ! checkRe (ip, re_iprange))
	throw (ip + uErrorBadValue);
    return accessRec (obj, ip);
}

/*DOC:
====read-host====
 (read-host DOMAIN) -> (["allow" | "deny"] TIME ENV_LIST)
ドメイン名をキーとしてレコードを読み取る。
レコードの期限が指定されているものは，期限の時刻のUNIX timeが返される。
環境変数指定はリストで返される。

*/
//#XSFUNC	read-host	ml_dbtcpserver_read_host
MNode*  ml_dbtcpserver_read_host (bool fev, MNode* cell, MlEnv* mlenv, MLFunc* mobj) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    evalParams (fev, mlenv, cell, posList, posParams);
    MLDbTcpserver*  obj = MObjRef<MLDbTcpserver> (mobj, cMLDbTcpserverID);
    ustring  host = to_string (posParams[0]());
    if (host.length () > 0 && ! checkRe (host, re_domainrange))
	throw (host + uErrorBadValue);
    if (host.length () > 0)
	host = ustring (CharConst ("=")).append (host);
    return accessRec (obj, host);
}

/*DOC:
====dump-ip====
 (dump-ip) -> ((IP ACTION TIME ENV_LIST) ...)
IPアドレスをキーとした全レコードをリストで返す。
空文字列のキーのレコードは，dump-ip関数の戻り値に含まれる。

*/
//#XSFUNC	dump-ip	ml_dbtcpserver_dump_ip
MNode*  ml_dbtcpserver_dump_ip (bool fev, MNode* cell, MlEnv* mlenv, MLFunc* mobj) {
    evalParams (fev, mlenv, cell);
    MLDbTcpserver*  obj = MObjRef<MLDbTcpserver> (mobj, cMLDbTcpserverID);
    MNodeList  ans, lkey, lallow, llimit, lestr;
    ustring  key, val;
    obj->db.initeach ();
    while (obj->db.each (key, val)) {
	if (key.length () > 0 && key[0] == '=') {
	} else {
	    accessRec (obj, key, val, ans);
	}
    }
    return mlenv->retval = ans ();
}

/*DOC:
====dump-host====
 (dump-host) -> ((DOMAIN ACTION TIME ENV_LIST) ...)
ドメイン名をキーとしたレコードをリストで返す。

*/
//#XSFUNC	dump-host	ml_dbtcpserver_dump_host
MNode*  ml_dbtcpserver_dump_host (bool fev, MNode* cell, MlEnv* mlenv, MLFunc* mobj) {
    evalParams (fev, mlenv, cell);
    MLDbTcpserver*  obj = MObjRef<MLDbTcpserver> (mobj, cMLDbTcpserverID);
    MNodeList  ans, lkey, lallow, llimit, lestr;
    ustring  key, val;
    obj->db.initeach ();
    while (obj->db.each (key, val)) {
	if (key.length () > 0 && key[0] == '=') {
	    key.assign (key.begin () + 1, key.end ());
	    accessRec (obj, key, val, ans);
	} else {
	}
    }
    return mlenv->retval = ans ();
}

/*DOC:
====cleanup====
 (cleanup) -> NIL
有効期限指定のあるレコードで，期限を過ぎたレコードを削除する。

*/
//#XSFUNC	cleanup	ml_dbtcpserver_cleanup
MNode*  ml_dbtcpserver_cleanup (bool fev, MNode* cell, MlEnv* mlenv, MLFunc* mobj) {
    evalParams (fev, mlenv, cell);
    MLDbTcpserver*  obj = MObjRef<MLDbTcpserver> (mobj, cMLDbTcpserverID);
    ustring  key, val;
    std::vector<ustring>  ary;
    time_t  now = time (NULL);
    bool  allow;
    time_t  limit;
    std::vector<ustring>::iterator  b, e;
    obj->db.initeach ();
    while (obj->db.each (key, val)) {
	if (obj->splitRec (val, allow, limit)) {
	    if (allow && limit > 0 && limit < now) {
		ary.push_back (key);
	    }
	} else {
	    ary.push_back (key);
	}
    }
    b = ary.begin ();
    e = ary.end ();
    for (; b < e; b ++) {
	obj->db.del (*b);
    }

    return NULL;
}

