#include "config.h"
#include "ml-http.h"
#include "ml.h"
#include "mlenv.h"
#include "motorenv.h"
#include "motoroutput.h"
#include "util_check.h"
#include "util_string.h"
#include "util_url.h"
#include "http-iconv.h"
#include "expr.h"
#include "ustring.h"
#include <iostream>
#include <fstream>
#include <time.h>

static void  url_sub (const ustring& url, HTTPSend* http, bool noencode) {
    ustring  str;
    uiterator  b, e;
    umatch  m;
    static uregex  re ("^([a-z]{1,6})://([a-zA-Z0-9-]+(\\.[a-zA-Z0-9-]+)*)(:([0-9]{1,5}))?/");

    str = omitCtrl (url);
    b = str.begin ();
    e = str.end ();
    if (usearch (b, e, m, re)) {
	http->proto = ustring (m[1].first, m[1].second);
	http->host.host = ustring (m[2].first, m[2].second);
	if (m[4].matched)
	    http->host.port = to_uint64 (ustring (m[5].first, m[5].second));
	else
	    if (http->proto == uHttps)
		http->host.port = 443;
	    else
		http->host.port = 80;
	if (http->useproxy) {
	    if (http->proto == uHttps) {
//		throw (ustring (CharConst ("proxy connection of SSL protocol not implemented.")));
		// XXX experimental https proxy connection.
		if (noencode)
		    http->path = ustring (m[0].second - 1, e);
		else
		    http->path = percentEncode_path (m[0].second - 1, e);
	    } else {
		if (noencode) {
		    http->path = str;
		} else {
		    http->path = ustring (b, m[0].second - 1);
		    http->path.append (percentEncode_path (m[0].second - 1, e));
		}
	    }
	} else {
	    http->conhost = http->host;
	    if (noencode)
		http->path = ustring (m[0].second - 1, e);
	    else
		http->path = percentEncode_path (m[0].second - 1, e);
	}
    } else {
//	throw (str + ": bad URL.");
	http->proto.resize (0);
	http->host.host.resize (0);
	http->host.port = 0;
	if (noencode)
	    http->path = str;
	else
	    http->path = percentEncode_path (str);
    }
}

static void  request_cookie (MNode* cookie, HTTPSend* http) {
    MNode*  a;

    if (isCons (cookie)) {
	while (cookie) {
	    if ((a = cookie->car ())) {
		if (isCons (a)) {
		    http->cookie.push_back (HTTPSend::mapelem (to_string (a->car ()), to_string (a->cdr ())));
		} else {
		    ustring  t = to_string (a);
		    ustring  u;
		    nextNode (cookie);
		    if (cookie)
			u = to_string (cookie->car ());
		    http->cookie.push_back (HTTPSend::mapelem (t, u));
		}
	    }
	    nextNode (cookie);
	}
    }
}

static void  request_headerquery (MNode* headerquery, HTTPSend* http) {
    MNode*  a;

    if (isCons (headerquery)) {
	while (headerquery) {
	    if ((a = headerquery->car ())) {
		if (isCons (a)) {
		    http->header_req.push_back (HTTPSend::mapelem (to_string (a->car ()), to_string (a->cdr ())));
		} else {
		    nextNode (headerquery);
		    if (headerquery)
			http->header_req.push_back (HTTPSend::mapelem (to_string (a), to_string (headerquery->car ())));
		}
	    }
	    nextNode (headerquery);
	}
    }
    return;
}

/*DOC:
==http client==

*/
/*DOC:
===build-url===
 (build-url SCHEME HOST PATH [:user NAME] [:password TEXT | :pw TEXT] [:port NUMBER] [:query '((NAME . VALUE) ...)]) -> STRING

*/
//#XAFUNC	build-url	ml_build_url
MNode*  ml_build_url (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_LIST, EV_LIST, EV_END};
    MNodePtr  posParams[3];
    kwParam  kwList[] = {{CharConst ("user"), EV_LIST},	    // 0
			 {CharConst ("password"), EV_LIST}, // 1
			 {CharConst ("pw"), EV_LIST},	    // 2
			 {CharConst ("port"), EV_LIST},	    // 3
			 {CharConst ("query"), EV_LIST},    // 4
			 {NULL, 0, EV_END}};
    MNodePtr  kwParams[5];
    evalParams (fev, mlenv, cell, posList, posParams, kwList, kwParams);
    ustring  scheme = to_text1 (posParams[0]());
    ustring  host = to_text1 (posParams[1]());
    ustring  path = to_text1 (posParams[2]());
    ustring  user = to_text1 (kwParams[0]());			// user
    ustring  pwd = to_text1 (kwParams[1]());			// password
    if (isNil (kwParams[1]())) pwd = to_text1 (kwParams[2]());	// pw
    int  port = to_int64 (kwParams[3]());				// port
    MNode*  query = kwParams[4]();
    AutoDelete<ustring>  ans;
    ans = new ustring;
    ans ()->append (scheme).append (CharConst ("://"));
    if (! isNil (kwParams[0]())) {
	omitCtrl (user);
	omitCtrl (pwd);
#ifdef QUERYENCODEALT
	ans ()->append (urlencode (user.begin (), user.end ()));
#else
	ans ()->append (percentEncode (user.begin (), user.end ()));
#endif
	if (pwd.size () > 0) {
	    ans ()->append (uColon);
#ifdef QUERYENCODEALT
	    ans ()->append (urlencode (pwd.begin (), pwd.end ()));
#else
	    ans ()->append (percentEncode (pwd.begin (), pwd.end ()));
#endif
	}
	ans ()->append (CharConst ("@"));
    }
    ans ()->append (host);
    if (port > 0) {
	ans ()->append (uColon).append (to_ustring (port));
    }
    if (path[0] != '/') {
	ans ()->append (uSlash);
    }
#ifdef QUERYENCODEALT
    ans ()->append (urlencode_path (path.begin (), path.end ()));
#else
    ans ()->append (percentEncode_path (path.begin (), path.end ()));
#endif
    if (! isNil (query)) {
	ans ()->append (buildQuery (query));
    }
    return newMNode_str (ans.release ());
}

/*DOC:
===build-url-path===
 (build-url-path ELEMENT...) -> STRING

*/
//#XAFUNC	build-url-path	ml_build_url_path
MNode*  ml_build_url_path (bool fev, MNode* cell, MlEnv* mlenv) {
    MNodePtr  rest;
    evalParams (fev, mlenv, cell, EV_LIST, &rest);
    MNode*  args = rest ();
    AutoDelete<ustring>  ans;
    MNode*  a;
    ustring  u;
    ans = new ustring;
    int  c = 0;
    while (args) {
	a = args->car ();
	if (! isNil (a)) {
	    u = to_string (a);
	    if (c == 0) {
#ifdef QUERYENCODEALT
		ans ()->append (urlencode (u.begin (), u.end ()));
#else
		ans ()->append (percentEncode (u.begin (), u.end ()));
#endif
		c ++;
	    } else {
#ifdef QUERYENCODEALT
		ans ()->append (uSlash).append (urlencode (u.begin (), u.end ()));
#else
		ans ()->append (uSlash).append (percentEncode (u.begin (), u.end ()));
#endif
	    }
	}
	nextNode (args);
    }

    return newMNode_str (ans.release ());
}

/*DOC:
===build-query===
 (build-query '((NAME . VALUE)...)) -> STRING

*/
//#XAFUNC	build-query	ml_build_query
MNode*  ml_build_query (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    evalParams (fev, mlenv, cell, posList, posParams);
    MNode*  query = posParams[0]();
    ustring  ans;
    MNode*  e;
    MNode*  a;
    int  c = 0;
    e = query;
    if (isCons (e)) {
	while (e) {
	    if ((a = e->car ())) {
		if (c > 0)
		    ans.append (uAmp);
		ans.append (percentEncode (to_string (a->car ())));
		if (! isNil (a->cdr ())) {
		    ans.append (uEq);
		    ans.append (percentEncode (to_string (a->cdr ())));
		}
		c ++;
	    }
	    nextNode (e);
	}
    }
    return newMNode_str (new ustring (ans));
}

/*DOC:
===decode-percent===
 (decode-percent STRING...) -> STRING

*/
//#XAFUNC	decode-percent	ml_decode_percent
//#XWIKIFUNC	decode-percent
MNode*  ml_decode_percent (bool fev, MNode* cell, MlEnv* mlenv) {
    MNodePtr  rest;
    evalParams (fev, mlenv, cell, EV_LIST, &rest);
    MNode*  args = rest ();
    ustring  str;
    while (args) {
	str.append (to_string (args->car ()));
	nextNode (args);
    }
    return newMNode_str (new ustring (percentDecode (str)));
}

/*DOC:
===http-date===
 (http-date INTEGER) -> STRING

*/
//#XAFUNC	http-date	ml_http_date
MNode*  ml_http_date (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    evalParams (fev, mlenv, cell, posList, posParams);
    time_t  tm = to_int64 (posParams[0]());
    struct tm  tms;
    gmtime_r (&tm, &tms);
    char  b[64];
    size_t  s = strftime (b, 64, "%a, %d %b %Y %H:%M:%S GMT", &tms);
    return newMNode_str (new ustring (b, s));
}

/*DOC:
===hostnamep===
 (hostnamep STRING) -> BOOL

*/
//#XAFUNC	hostnamep	ml_hostnamep
//#XWIKIFUNC	hostnamep
MNode*  ml_hostnamep (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    evalParams (fev, mlenv, cell, posList, posParams);
    ustring  hostname = to_string (posParams[0]());
    return newMNode_bool (matchHostname (hostname));
}

/*DOC:
===response-no-cache===
 (response-no-cache) -> NIL

*/
//#XAFUNC	response-no-cache	ml_response_no_cache
MNode*  ml_response_no_cache (bool fev, MNode* cell, MlEnv* mlenv) {
    evalParams (fev, mlenv, cell);
    mlenv->env->http.fNoCache = true;
    return NULL;
}

