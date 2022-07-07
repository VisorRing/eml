#include "ml-apache.h"
#include "ml.h"
#include "mlenv.h"
#include "motorenv.h"
#include "formfile.h"
#include "utf8.h"
#include "util_apache.h"
#include "util_check.h"
#include "util_const.h"
#include "util_string.h"
#include "httpconst.h"
#include "expr.h"
#include "ustring.h"
#include <assert.h>
#include <stdlib.h>

/*DOC:
==apache library==

*/

static MNode*  env_sub (MNode* cell, const char* name) {
    if (cell->cdr ()) {
	throw (uErrorWrongNumber);
    } else {
	ustring  e = getenvString (name);
	if (e.length () > 0) {
	    return newMNode_str (new ustring (fixUTF8 (omitCtrl (e))));
	} else {
	    return NULL;
	}
    }
}

/*DOC:
===server-name===
 (server-name) -> STRING

===server-port===
 (server-port) -> NUM

===remote-user===
 (remote-user) -> STRING

===referer===
 (referer) -> STRING

===user-agent===
 (user-agent) -> STRING

===apache-path-info===
 (apache-path-info) -> STRING

===apache-query-string===
 (apache-query-string) -> STRING

===apache-script-name===
 (apache-script-name) -> STRING

===apache-redirect-url===
 (apache-redirect-url) -> STRING

===remote-ip===
 (remote-ip) -> STRING

===request-method===
 (request-method) -> STRING

===request-type===
 (request-type) -> STRING

===apache-https===
 (apache-https) -> STRING

===ssl-client-m-serial===
 (ssl-client-m-serial) -> STRING

===ssl-client-s-dn===
 (ssl-client-s-dn) -> STRING

===ssl-client-i-dn===
 (ssl-client-i-dn) -> STRING

*/
//#XAFUNC	server-name	ml_server_name
//#XWIKIFUNC	server-name
//#XAFUNC	server-port	ml_server_port
//#XWIKIFUNC	server-port
//#XAFUNC	remote-user	ml_remote_user
//#XWIKIFUNC	remote-user
//#XAFUNC	remote-referer		ml_remote_referer
//#XWIKIFUNC	remote-referer
//#XAFUNC	remote-user-agent	ml_remote_user_agent
//#XWIKIFUNC	remote-user-agent
////#XAFUNC	apache-path-info	ml_apache_path_info
////#XWIKIFUNC	apache-path-info
//#XAFUNC	extra-path		ml_extra_path
//#XWIKIFUNC	extra-path
//#XAFUNC	apache-query-string	ml_apache_query_string
//#XWIKIFUNC	apache-query-string
////#XAFUNC	apache-script-name	ml_apache_script_name
////#XWIKIFUNC	apache-script-name
//#XAFUNC	script-path		ml_script_path
//#XWIKIFUNC	script-path
//#XAFUNC	apache-redirect-url	ml_apache_redirect_url
//#XWIKIFUNC	apache-redirect-url
//#XAFUNC	remote-ip	ml_remote_ip
//#XWIKIFUNC	remote-ip
//#XAFUNC	request-method	ml_request_method
//#XWIKIFUNC	request-method
//#XAFUNC	request-type	ml_request_type
//#XWIKIFUNC	request-type
//#XAFUNC	apache-https	ml_apache_https
//#XWIKIFUNC	apache-https
//#XAFUNC	ssl-client-m-serial	ml_ssl_client_m_serial
//#XWIKIFUNC	ssl-client-m-serial
//#XAFUNC	ssl-client-s-dn		ml_ssl_client_s_dn
//#XWIKIFUNC	ssl-client-s-dn
//#XAFUNC	ssl-client-i-dn		ml_ssl_client_i_dn
//#XWIKIFUNC	ssl-client-i-dn
MNode*  ml_server_name (bool fev, MNode* cell, MlEnv* mlenv) {
    return env_sub (cell, kSERVER_NAME);
}
MNode*  ml_server_port (bool fev, MNode* cell, MlEnv* mlenv) {
    return env_sub (cell, kSERVER_PORT);
}
MNode*  ml_remote_user (bool fev, MNode* cell, MlEnv* mlenv) {
    return env_sub (cell, kREMOTE_USER);
}
MNode*  ml_remote_referer (bool fev, MNode* cell, MlEnv* mlenv) {
    return env_sub (cell, kHTTP_REFERER);
}
MNode*  ml_remote_user_agent (bool fev, MNode* cell, MlEnv* mlenv) {
    return env_sub (cell, kHTTP_USER_AGENT);
}
MNode*  ml_extra_path (bool fev, MNode* cell, MlEnv* mlenv) {
    return env_sub (cell, kPATH_INFO);
}
MNode*  ml_apache_query_string (bool fev, MNode* cell, MlEnv* mlenv) {
    return env_sub (cell, kQUERY_STRING);
}
MNode*  ml_script_path (bool fev, MNode* cell, MlEnv* mlenv) {
    return env_sub (cell, kSCRIPT_NAME);
}
MNode*  ml_apache_redirect_url (bool fev, MNode* cell, MlEnv* mlenv) {
    return env_sub (cell, kREDIRECT_URL);
}
MNode*  ml_remote_ip (bool fev, MNode* cell, MlEnv* mlenv) {
    return env_sub (cell, kREMOTE_ADDR);
}
MNode*  ml_request_method (bool fev, MNode* cell, MlEnv* mlenv) {
    return env_sub (cell, kREQUEST_METHOD);
}
MNode*  ml_request_type (bool fev, MNode* cell, MlEnv* mlenv) {
    return env_sub (cell, kCONTENT_TYPE);
}
MNode*  ml_apache_https (bool fev, MNode* cell, MlEnv* mlenv) {
    return env_sub (cell, kHTTPS);
}
MNode*  ml_ssl_client_m_serial (bool fev, MNode* cell, MlEnv* mlenv) {
    return env_sub (cell, kSSL_CLIENT_M_SERIAL);
}
MNode*  ml_ssl_client_s_dn (bool fev, MNode* cell, MlEnv* mlenv) {
    return env_sub (cell, kSSL_CLIENT_S_DN);
}
MNode*  ml_ssl_client_i_dn (bool fev, MNode* cell, MlEnv* mlenv) {
    return env_sub (cell, kSSL_CLIENT_I_DN);
}

/*DOC:
===is-https===
 (is-https) -> BOOL

*/
//#XAFUNC	is-https	ml_is_https
MNode*  ml_is_https (bool fev, MNode* cell, MlEnv* mlenv) {
    evalParams (fev, mlenv, cell);
    ustring  e = getenvString (kHTTPS);
    if (match (e, CharConst ("on")))
	return mlTrue;
    else
	return NULL;
}

/*DOC:
===request-header===
 (request-header NAME) -> STRING

*/
////#AFUNC	get-http-header	ml_get_http_header
//#XAFUNC	request-header	ml_request_header
MNode*  ml_request_header (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    evalParams (fev, mlenv, cell, posList, posParams);
    ustring  name = toUpper (to_text1 (posParams[0]()));
    ustring::iterator  b = name.begin ();
    ustring::iterator  e = name.end ();
    for (; b != e; b ++) {
	if (*b == '-') {
	    *b = '_';
	}
    }
    if (matchAlNum (name.begin (), e) && name.length () < 128) {
	name = ustring (CharConst ("HTTP_")).append (name);
	ustring  e = getenvString (name.c_str ());
	if (e.length () > 0) {
	    return newMNode_str (new ustring (fixUTF8 (omitCtrl (e))));
	} else {
	    return NULL;
	}
    } else {
	return NULL;
    }
}

/*DOC:
===get-ssl-env===
 (get-ssl-env NAME) -> STRING

*/
//#XAFUNC	get-ssl-env	ml_get_ssl_env
MNode*  ml_get_ssl_env (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    evalParams (fev, mlenv, cell, posList, posParams);
    ustring  name = toUpper (to_text1 (posParams[0]()));
    ustring::iterator  b = name.begin ();
    ustring::iterator  e = name.end ();
    for (; b != e; b ++) {
	if (*b == '-') {
	    *b = '_';
	}
    }
    if (matchAlNum (name.begin (), e) && name.length () < 128) {
	name = ustring (CharConst ("SSL_")).append (name);
	ustring  e = getenvString (name.c_str ());
	if (e.length () > 0) {
	    return newMNode_str (new ustring (fixUTF8 (omitCtrl (e))));
	} else {
	    return NULL;
	}
    } else {
	return NULL;
    }
}

/*DOC:
===is-get-method===
 (is-get-method) -> BOOL

===is-post-method===
 (is-post-method) -> BOOL

*/
//#XAFUNC	is-get-method	ml_is_get_method
//#XWIKIFUNC	is-get-method
//#XAFUNC	is-post-method	ml_is_post_method
//#XWIKIFUNC	is-post-method
MNode*  ml_is_get_method (bool fev, MNode* cell, MlEnv* mlenv) {
    MNode*  arg = cell->cdr ();

    if (arg)
	throw (uErrorWrongNumber);

    if (match (mlenv->env->form->requestMethod, CharConst (kMETHOD_GET))) {
	return newMNode_bool (true);
    } else {
	return newMNode_bool (false);
    }
}

MNode*  ml_is_post_method (bool fev, MNode* cell, MlEnv* mlenv) {
    MNode*  arg = cell->cdr ();

    if (arg)
	throw (uErrorWrongNumber);

    if (match (mlenv->env->form->requestMethod, CharConst (kMETHOD_POST))) {
	return newMNode_bool (true);
    } else {
	return newMNode_bool (false);
    }
}

/*DOC:
===absolute-url===
 (absolute-url URL [#http] [#https] [#no-proto] [:port NUM]) -> STRING

*/
//#XAFUNC	absolute-url	ml_absolute_url
//#XWIKIFUNC	absolute-url
MNode*  ml_absolute_url (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    kwParam  kwList[] = {{CharConst ("http"), EV_LIST},
			 {CharConst ("https"), EV_LIST},
			 {CharConst ("no-proto"), EV_LIST},
			 {CharConst ("port"), EV_LIST},
			 {NULL, 0, EV_END}};
    MNodePtr  kwParams[4];
    evalParams (fev, mlenv, cell, posList, posParams, kwList, kwParams);
    enum {
	PROTO_HTTP,
	PROTO_HTTPS,
	PROTO_NONE
    }  proto;
    ustring  url = to_text1 (posParams[0]());
    if (isHTTPS ())
	proto = PROTO_HTTPS;
    else
	proto = PROTO_HTTP;
    if (to_bool (kwParams[0]())) {
	proto = PROTO_HTTP;
    } else if (to_bool (kwParams[1]())) {
	proto = PROTO_HTTPS;
    } else if (to_bool (kwParams[2]())) {
	proto = PROTO_NONE;
    }
    int  port = to_int64 (kwParams[3]());
    if (port < 1 || 65535 < port)
	port = 0;
    ustring*  ans;
    switch (proto) {
    case PROTO_HTTP:
	ans = new ustring (CharConst ("http://"));
	break;
    case PROTO_HTTPS:
	ans = new ustring (CharConst ("https://"));
	break;
    case PROTO_NONE:
	ans = new ustring (CharConst ("//"));
	break;
    default:
	assert (0);
    }
    ans->append (getenvString (kSERVER_NAME));
    if (port > 0) {
	ans->append (uColon).append (to_ustring (port));
    }
    ans->append (apacheAbsolutePath (url));

    return newMNode_str (ans);
}

/*DOC:
//===read-cookie===
===request-cookie===
 (request-cookie NAME) -> STRING

*/
//#XAFUNC	request-cookie	ml_request_cookie
MNode*  ml_request_cookie (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    evalParams (fev, mlenv, cell, posList, posParams);
    ustring  name = to_text1 (posParams[0]());
    ustring  val;
    if (mlenv->env && name.length () > 0) {
	val = mlenv->env->http.readCookie (name);
    }
    return newMNode_str (new ustring (val));
}

