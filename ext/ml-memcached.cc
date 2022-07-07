#include "ml-memcached.h"
#include "motorconst.h"
#include "config.h"
#include "ml.h"
#include "mlenv.h"
#include "motorenv.h"
#include "ustring.h"
#include "expr.h"
#include <libmemcached/memcached.h>
#include <time.h>
#include <vector>
#include <exception>

/*DOC:
==memcached module==

*/

/*DOC:
===$memcached===
 ($memcached [#unix | :unix BOOL] [#inet | :inet BOOL] [SUBFUNCTION...]) -> VALUE

*/
//#MFUNC	$memcached	ml_memcached	cMLMemcachedID
MNode*  ml_memcached (MNode* cell, MlEnv* mlenv) {
    MNode*  arg = cell->cdr ();
    MLMemcached  obj (mlenv);
    bool  finet = false;
    MNodePtr  ans;
    memcached_return_t  rc;
    std::vector<MNode*>  keywords;
    MNode*  rest;
    static paramList  kwlist[] = {
	{CharConst ("unix"), true},
	{CharConst ("inet"), true},
	{NULL, 0, 0}
    };

    if (obj.mem == NULL) {
	throw (ustring (CharConst ("can't connect to the memcached server")));
    }
    setParams (arg, 0, NULL, kwlist, &keywords, &rest);
    if (keywords[0])		// #unix
	finet = ! eval_bool (keywords[0], mlenv);
    if (keywords[1])		// #inet
	finet = eval_bool (keywords[1], mlenv);
    if (finet) {
	rc = memcached_server_add (obj.mem, memcached_ip, memcached_port);
    } else {
	rc = memcached_server_add_unix_socket (obj.mem, mlenv->env->path_to_etc (ustring (CharConst (memcached_socket))).c_str ());
    }
    rc = memcached_behavior_set (obj.mem, MEMCACHED_BEHAVIOR_BINARY_PROTOCOL, 1);

    mlenv->setMStack (&obj);
    ans = progn (rest, mlenv);
    mlenv->stopBreak (cell->car ());

    return mlenv->retval = ans ();
}

/*DOC:
===subfunctions of $memcached===

*/
typedef  enum {
    OP_SET,
    OP_ADD,
    OP_REP,
}  cacheop_t;

static MNode*  cache_set (MNode* cell, MlEnv* mlenv, MLFunc* mobj, cacheop_t op) {
    MNode*  arg = cell->cdr ();
    MLMemcached*  obj = MObjRef<MLMemcached> (mobj, cMLMemcachedID);
    ustring  key;
    MNode*  rest;
    int  n;
    ustring  val;
    time_t  expire = 0;
    memcached_return_t  rc;
    std::vector<MNode*>  params;
    std::vector<MNode*>  keywords;
    static paramList  kwlist[] = {
	{CharConst ("expire"), false},
	{NULL, 0, 0}
    };

    setParams (arg, 1, &params, kwlist, &keywords, &rest);
    key = eval_str (params[0], mlenv);
    if (keywords[0]) {
	expire = eval_int (keywords[0], mlenv);
    }
    n = 0;
    while (rest) {
	if (n > 0)
	    val.append (1, '\0');
	val.append (eval_str (rest->car (), mlenv));
	nextNode (rest);
	n ++;
    }

    switch (op) {
    case OP_SET:
	rc = memcached_set (obj->mem, key.data (), key.length (), val.data (), val.length (), expire, 0);
	break;
    case OP_ADD:
	rc = memcached_add (obj->mem, key.data (), key.length (), val.data (), val.length (), expire, 0);
	break;
    case OP_REP:
	rc = memcached_replace (obj->mem, key.data (), key.length (), val.data (), val.length (), expire, 0);
	break;
    default:;
    }

    return NULL;
}

/*DOC:
====cache-set====
 (cache-set KEY VALUES... [:expire TIMESEC]) -> NULL
====cache-add====
 (cache-add KEY VALUES... [:expire TIMESEC]) -> NULL
====cache-replace====
 (cache-replace KEY VALUES... [:expire TIMESEC]) -> NULL

*/
//#SFUNC	cache-set	ml_memcached_cache_set
//#SFUNC	cache-add	ml_memcached_cache_add
//#SFUNC	cache-replace	ml_memcached_cache_replace
MNode*  ml_memcached_cache_set (MNode* cell, MlEnv* mlenv, MLFunc* mobj) {
    return cache_set (cell, mlenv, mobj, OP_SET);
}

MNode*  ml_memcached_cache_add (MNode* cell, MlEnv* mlenv, MLFunc* mobj) {
    return cache_set (cell, mlenv, mobj, OP_ADD);
}

MNode*  ml_memcached_cache_replace (MNode* cell, MlEnv* mlenv, MLFunc* mobj) {
    return cache_set (cell, mlenv, mobj, OP_REP);
}

/*DOC:
====cache-get====
 (cache-get KEY) -> STRING-LIST or NIL

*/
//#SFUNC	cache-get	ml_memcached_cache_get
MNode*  ml_memcached_cache_get (MNode* cell, MlEnv* mlenv, MLFunc* mobj) {
    MNode*  arg = cell->cdr ();
    MLMemcached*  obj = MObjRef<MLMemcached> (mobj, cMLMemcachedID);
    ustring  key;
    char*  val;
    size_t  valLen = 0;
    uint32_t  flag;
    memcached_return_t  rc;
    MNodeList  ans;

    if (!arg) 
	throw (uErrorWrongNumber);
    key = eval_str (arg->car (), mlenv);
    nextNode (arg);
    if (arg) 
	throw (uErrorWrongNumber);

    val = memcached_get (obj->mem, key.data (), key.length (), &valLen, &flag, &rc);
    if (val) {
	ustring  u (val, valLen);
	uiterator  b = u.begin ();
	uiterator  e = u.end ();
	uiterator  it;

	for (it = b; ; it ++) {
	    if (*it == 0 || it == e) {
		ans.append (newMNode_str (new ustring (b, it)));
		b = it + 1;
		if (it == e)
		    break;
	    }
	}
    }

    return ans.release ();
}

/*DOC:
====cache-delete====
 (cache-delete KEY [:expire TIMESEC]) -> NULL

*/
//#SFUNC	cache-delete	ml_memcached_cache_delete
MNode*  ml_memcached_cache_delete (MNode* cell, MlEnv* mlenv, MLFunc* mobj) {
    MNode*  arg = cell->cdr ();
    MLMemcached*  obj = MObjRef<MLMemcached> (mobj, cMLMemcachedID);
    ustring  key;
    time_t  expire = 0;
    memcached_return_t  rc;
    std::vector<MNode*>  params;
    std::vector<MNode*>  keywords;
    static paramList  kwlist[] = {
	{CharConst ("expire"), false},
	{NULL, 0, 0}
    };

    setParams (arg, 1, &params, kwlist, &keywords, NULL);
    key = eval_str (params[0], mlenv);
    if (keywords[0]) {
	expire = eval_int (keywords[0], mlenv);
    }

    rc = memcached_delete (obj->mem, key.data (), key.length (), expire);

    return NULL;
}

/*DOC:
====cache-increment====
 (cache-increment KEY [:offset INTEGER] [:initial INTEGER] [:expire TIME]) -> NULL

*/
//#SFUNC	cache-increment	ml_memcached_cache_increment
MNode*  ml_memcached_cache_increment (MNode* cell, MlEnv* mlenv, MLFunc* mobj) {
    MNode*  arg = cell->cdr ();
    MLMemcached*  obj = MObjRef<MLMemcached> (mobj, cMLMemcachedID);
    ustring  key;
    memcached_return_t  rc;
    uint64_t  offset = 1;
    uint64_t  initial = 0;
    time_t  expire = 0;
    uint64_t  val;
    std::vector<MNode*>  params;
    std::vector<MNode*>  keywords;
    static paramList  kwlist[] = {
	{CharConst ("offset"), false},
	{CharConst ("initial"), false},
	{CharConst ("expire"), false},
	{NULL, 0, 0}
    };

    setParams (arg, 1, &params, kwlist, &keywords, NULL);
    key = eval_str (params[0], mlenv);
    if (keywords[0]) {		// offset
	offset = eval_int (keywords[0], mlenv);
    }
    if (keywords[1]) {		// initial
	initial = eval_int (keywords[1], mlenv); // XXX: not uint64_t
	if (keywords[2]) {	// expire
	    expire = eval_int (keywords[2], mlenv);
	}
	rc = memcached_increment_with_initial (obj->mem, key.data (), key.length (), offset, initial, expire, &val);
    } else {
	rc = memcached_increment (obj->mem, key.data (), key.length (), offset, &val);
    }
    
    if (rc == MEMCACHED_SUCCESS) {
	return newMNode_num (val);
    } else {
	return NULL;
    }
}

/*DOC:
====cache-decrement====
 (cache-decrement KEY [:offset INTEGER] [:initial INTEGER] [:expire TIME]) -> NULL

*/
//#SFUNC	cache-decrement	ml_memcached_cache_decrement
MNode*  ml_memcached_cache_decrement (MNode* cell, MlEnv* mlenv, MLFunc* mobj) {
    MNode*  arg = cell->cdr ();
    MLMemcached*  obj = MObjRef<MLMemcached> (mobj, cMLMemcachedID);
    ustring  key;
    memcached_return_t  rc;
    uint64_t  offset = 1;
    uint64_t  initial = 0;
    time_t  expire = 0;
    uint64_t  val;
    std::vector<MNode*>  params;
    std::vector<MNode*>  keywords;
    static paramList  kwlist[] = {
	{CharConst ("offset"), false},
	{CharConst ("initial"), false},
	{CharConst ("expire"), false},
	{NULL, 0, 0}
    };

    setParams (arg, 1, &params, kwlist, &keywords, NULL);
    key = eval_str (params[0], mlenv);
    if (keywords[0]) {		// offset
	offset = eval_int (keywords[0], mlenv);
    }
    if (keywords[1]) {		// initial
	initial = eval_int (keywords[1], mlenv); // XXX: not uint64_t
	if (keywords[2]) {	// expire
	    expire = eval_int (keywords[2], mlenv);
	}
	rc = memcached_decrement_with_initial (obj->mem, key.data (), key.length (), offset, initial, expire, &val);
    } else {
	rc = memcached_decrement (obj->mem, key.data (), key.length (), offset, &val);
    }
    
    if (rc == MEMCACHED_SUCCESS) {
	return newMNode_num (val);
    } else {
	return NULL;
    }
}
