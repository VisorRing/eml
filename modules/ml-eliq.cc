#include "ml-eliq.h"
#include "ml.h"
#include "mlenv.h"
#include "config.h"
#include "ustring.h"
#include "expr.h"
#include "motoroutput.h"
#include "util_base64.h"
#include "util_check.h"
#include "util_random.h"
#include "util_splitter.h"
#include "util_time.h"
#include "utf8.h"
#include <neon/ne_session.h>
#include <neon/ne_request.h>
#include <neon/ne_utils.h>
#include <neon/ne_uri.h>
#include <openssl/hmac.h>
#include <openssl/sha.h>
#include <exception>
#include <time.h>

/*DOC:
==Eliq library==

*/

static ustring  methodText (NeonQuery::method_t method) {
    switch (method) {
    case NeonQuery::METHOD_GET:
	return ustring (CharConst ("GET"));
    case NeonQuery::METHOD_POST:
	return ustring (CharConst ("POST"));
    case NeonQuery::METHOD_PUT:
	return ustring (CharConst ("PUT"));
    case NeonQuery::METHOD_DELETE:
	return ustring (CharConst ("DELETE"));
    default:
	assert (0);
    }
}

static ustring  base64encode (const char* p, size_t s) {
    ustring  str (p, s);
    return base64Encode (str.begin (), str.end ());
}

static ustring  base64encode (const ustring& str) {
    return base64Encode (str.begin (), str.end ());
}

static ustring  base64decode (const ustring& str) {
    return base64Decode (str.begin (), str.end ());
}

static ustring  signature (MLEliq* obj, const ustring& method, const ustring& path, const ustring& date, const ustring& nonce) {
    ustring  text;
    ustring  host = toLower (obj->hostconf.host);
    text.append (method).append (uLF);
    text.append (host).append (uColon).append (to_ustring (obj->hostconf.port)).append (uLF);
    text.append (path).append (uLF);
    text.append (obj->hostconf.token).append (uLF);
    text.append (date).append (uLF);
    text.append (nonce);
    unsigned char  md[EVP_MAX_MD_SIZE];
    unsigned int  md_len;
    HMAC (EVP_sha256 (), obj->hostconf.secret.c_str (), obj->hostconf.secret.length (), uchar_type (text.c_str ()), text.length (), md, &md_len);
    ustring  data (char_type (md), md_len);
    data = base64Encode (data.begin (), data.end ());
    return data;
}

static void  signatureHeader (MLEliq* obj, time_t tm, const ustring& nonce, const ustring& path, NeonQuery::method_t method) {
    struct tm  tms;
    gmtime_r (&tm, &tms);
    char  b[64];
    size_t  s = strftime (b, 64, "%a, %d %b %Y %H:%M:%S GMT", &tms);
    ustring  date (b, s);
    ustring  sig = signature (obj, methodText (method), path, date, nonce);
    ne_add_request_header (obj->query->req, "Date", date.c_str ());
    ne_add_request_header (obj->query->req, "X-Nonce", nonce.c_str ());
    ne_add_request_header (obj->query->req, "AccessKey", obj->hostconf.token.c_str ());
    ne_add_request_header (obj->query->req, "Signature", sig.c_str ());
}

static bool  eliqSubmit (MLEliq* obj, MlEnv* mlenv, const ustring& path, const ustring& sexps, const MLEliq::EliqOpts& eliqOpts) {
    time_t  tm = now ();
    ustring  nonce = randomKey () + randomKey ();
    // XXX DBが異常終了したとき、NeonQueryのdeleteでセグメントエラーが発生する。
    obj->query.reset (new NeonQuery (obj->session->get (), mlenv));
    obj->query->path = path;
    obj->query->method = NeonQuery::METHOD_POST;
    obj->query->req = ne_request_create (obj->session->get (), obj->query->methodStr (), path.c_str ());
    signatureHeader (obj, tm, nonce, path, NeonQuery::METHOD_POST);
    ne_add_request_header (obj->query->req, "Content-Type", "text/tab-separated-values; colenc=B");
    ustring  q;
    q.append (base64encode (CharConst ("env"))).append (uTAB);
    if (isNil (eliqOpts.env)) {
	q.append (base64encode (CharConst ("{}")));
    } else {
	q.append (base64encode (dump_to_texp (eliqOpts.env)));
    }
    q.append (uLF).append (base64encode (CharConst ("eval"))).append (uTAB).append (base64encode (sexps));
    ne_set_request_body_buffer (obj->query->req, q.c_str (), q.size ());
    int  rc = ne_begin_request (obj->query->req);
    if (rc == NE_OK) {
	obj->query->mode = NeonQuery::MODE_DISPATCHED;
	return true;
    } else {
	ustring  errorMsg;
	errorMsg.assign (ne_get_error (obj->query->session));
#ifdef DEBUG
	std::cerr << "error: " << rc << ": " << errorMsg << "\n";
#endif /* DEBUG */
	return false;
    }
}

static void  decodeLine (MLEliq* obj, MlEnv* mlenv, ustring (*fn)(const ustring&), const uiterator b, const uiterator e, MNode* vec, MNode* tbl, bool& vflag) {
    SplitterCh  sp (b, e, '\t');
    if (sp.next ()) {
	ustring  name;
	ustring  val;
	if (fn) {
	    name = fn (ustring (sp.begin (), sp.end ()));
	    val = fn (ustring (sp.matchEnd (), sp.eol ()));
	} else {
	    name = ustring (sp.begin (), sp.end ());
	    val = ustring (sp.matchEnd (), sp.eol ());
	}
	MotorTexp  ml (NULL);
	ml.scan (val);
	MNode*  x = NULL;
	if (ml.top.isCons () && isCons (ml.top.cdr ()))
	    x = ml.top.cdr ()->car ();
	if (name.length () > 0) {
	    // XXX vecとtblを分離する
	    if (vflag && matchNum (name.begin (), name.end ())) {
		int64_t  idx = to_int64 (name);
		if (vec->vectorSize () == idx) {
		    vec->vectorPut (idx, x);
		} else {
		    tbl->tablePut (name, x);
		    vflag = false;
		}
	    } else {
		tbl->tablePut (name, x);
		vflag = false;
	    }
	}
    }
}

static MNode*  decodeBody (MLEliq* obj, MlEnv* mlenv, ustring (*fn)(const ustring&)) {
    std::stringstream  ostr;
    MotorOutputOStream  out (&ostr);
    obj->query->readBody (&out);
    ustring  data = fixUTF8 (ostr.str ());
    SplitterCh  sp (data.begin (), data.end (), '\n');
    MNode*  vec = newMNode_vector ();
    MNode*  tbl = newMNode_table ();
    bool  vflag = true;
    while (sp.next ()) {
	decodeLine (obj, mlenv, fn, sp.begin (), sp.end (), vec, tbl, vflag);
    }
    return newMNode_cons (vec, newMNode_cons (tbl));
}

static MNode*  eliqDecode (MLEliq* obj, MlEnv* mlenv) {
    int  status = obj->query->getStatus ();
    if (status == 200) {
	return decodeBody (obj, mlenv, base64decode);
    } else {
	MNode*  tbl = newMNode_table ();
	tbl->tablePut (ustring (CharConst ("ERROR")), newMNode_num (status));
	MNode*  ans = newMNode_cons (newMNode_vector (),
				     newMNode_cons (tbl));
	return ans;
    }
}

static MNode*  eliqr (MLEliq* obj, MlEnv* mlenv, const ustring& sexps, const MLEliq::EliqOpts& eliqOpts) {
    ustring  path = ustring (CharConst ("/eliq"));
    if (eliqSubmit (obj, mlenv, path, sexps, eliqOpts)) {
	return eliqDecode (obj, mlenv);
    } else {
	throw (ustring ("connection failed"));
    }
}

static bool  eliq_connect (MlEnv* mlenv, MLEliq& obj, MNode* hostconf, MNode* rest, MNodePtr& ans) {
    obj.hostconf.host = to_text1 (hostconf->car ());
    nextNode (hostconf);
    if (! isCons (hostconf))
	throw (dump_to_texp (hostconf) + uErrorBadParam);
    obj.hostconf.port = to_int64 (hostconf->car ());
    nextNode (hostconf);
    if (! isCons (hostconf))
	throw (dump_to_texp (hostconf) + uErrorBadParam);
    obj.hostconf.token = to_text1 (hostconf->car ());
    nextNode (hostconf);
    if (! isCons (hostconf))
	throw (dump_to_texp (hostconf) + uErrorBadParam);
    obj.hostconf.secret = to_text1 (hostconf->car ());

    if (! matchHostname (obj.hostconf.host))
	throw (obj.hostconf.host + ": bad hostname.");
    if (obj.hostconf.port <= 0 || obj.hostconf.port >= 65536)
	throw (to_ustring (obj.hostconf.port) + ": bad port number.");

    obj.session.reset (new NeonSession (obj.proto, obj.hostconf.host, obj.hostconf.port));
    ans = progn (rest, mlenv);
    return true;
}

/*DOC
===eliq-connect===
 (eliq-connect (HOST PORT TOKEN SECRET) #https :on-error LAMBDA BLOCK ...) -> LAST_VALUE

*/
//#XMFUNC	eliq-connect	ml_eliq_connect	cMLEliqID
MNode*  ml_eliq_connect (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    kwParam  kwList[] = {{CharConst ("on-error"), EV_LIST},
			 {CharConst ("https"), EV_LIST},
			 {NULL, 0, EV_END}};
    MNodePtr  kwParams[2];
    MNodePtr  rest;
    evalParams (fev, mlenv, cell, posList, posParams, kwList, kwParams, EV_ASIS, &rest);
    NeonInitProc ();
    if (! isCons (posParams[0]()))
	throw (dump_to_texp (posParams[0]()) + uErrorBadParam);
    MNode*  hostconf = posParams[0]();
    MNode*  errfn = kwParams[0]();
    bool  ftls = to_bool (kwParams[1]());
    MNodePtr  ans;
    MLEliq  obj (mlenv);
    if (ftls) obj.setHttps ();
    mlenv->setMStack (&obj);
    try {
	eliq_connect (mlenv, obj, hostconf, rest (), ans);
    } catch (ustring& msg) {
	if (errfn) {
	    ans = onErrorFn (errfn, mlenv);
	} else {
	    throw (msg);
	}
    }
    mlenv->stopBreak (cell->car ());
    return mlenv->retval = ans.release ();
}

/*DOC:
=== subfunctions of eliq-connect===

*/

/*DOC:
====eliq-progn====
 (eliq-progn :env TABLE #debug #trace EXPR ...) -> (VECTOR TABLE)

*/
//#XSFUNC	eliq-progn	ml_eliq_progn	eliq-connect
MNode*  ml_eliq_progn (bool fev, MNode* cell, MlEnv* mlenv, MLFunc* mobj) {
    kwParam  kwList[] = {{CharConst ("env"), EV_LIST},	     // 0
			 {NULL, 0, EV_END}};
    MNodePtr  kwParams[1];
    MNodePtr  rest;
    evalParams (fev, mlenv, cell, NULL, NULL, kwList, kwParams, EV_LIST, &rest);
    MLEliq*  obj = (MLEliq*)mobj;
    MLEliq::EliqOpts  eliqOpts;
    eliqOpts.env = kwParams[0]();
    if (! isNil (eliqOpts.env) && ! isTable (eliqOpts.env))
	throw "bad :env parameter";
    MNode*  args = rest ();
    ustring  sexps;
    while (args) {
	if (sexps.length () > 0)
	    sexps.append (uSPC);
	sexps.append (to_string (args->car ()));
	nextNode (args);
    }
    return eliqr (obj, mlenv, sexps, eliqOpts);
}

