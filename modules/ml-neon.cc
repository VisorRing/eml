#include "ml-neon.h"
#include "config.h"
#include "ml-http.h"
#include "ml.h"
#include "mlenv.h"
#include "motorenv.h"
#include "motoroutput.h"
#include "util_check.h"
#include "util_string.h"
#include "util_random.h"
#include "util_mimetype.h"
#include "util_file.h"
#include "util_base64.h"
#include "http-iconv.h"
#include "expr.h"
#include "utf8.h"
#include "ustring.h"
#include "filemacro.h"
#include <fstream>
#include <neon/ne_session.h>
#include <neon/ne_request.h>
#include <neon/ne_utils.h>
#include <neon/ne_uri.h>

class  NEONInitOnce {
public:
    int  count;

    NEONInitOnce () {
	count = 0;
    };
    virtual  ~NEONInitOnce () {
	if (count > 0) {
	    ne_sock_exit ();
	}
    };

    void  init () {
	if (count == 0) {
	    ne_sock_init ();
	    ++ count;
#ifdef DEBUG
	    std::cerr << "neon feature:"
		      << (ne_has_support (NE_FEATURE_SSL) ? " FEATURE_SSL" : "")
		      << (ne_has_support (NE_FEATURE_ZLIB) ? " FEATURE_ZLIB" : "")
		      << (ne_has_support (NE_FEATURE_IPV6) ? " FEATURE_IPV6" : "")
		      << (ne_has_support (NE_FEATURE_LFS) ? " FEATURE_LFS" : "")
		      << (ne_has_support (NE_FEATURE_SOCKS) ? " FEATURE_SOCKS" : "")
		      << (ne_has_support (NE_FEATURE_TS_SSL) ? " FEATURE_TS_SSL" : "")
		      << "\n";
#endif /* DEBUG */
	}
    }
};
NEONInitOnce  NeonInit;
void  NeonInitProc () {
    NeonInit.init ();
}

//============================================================
void  NeonPostBodyProvider::pushFile (const ustring& name, const ustring& path) {
    if (name.length () > 0 && isPlainFile (path))
	postFile.push_back (std::pair<ustring, ustring> (name, path));
}

void  NeonPostBodyProvider::makeSeparator () {
    separator.assign (CharConst ("--------")).append (randomKey ()).append (randomKey ());
}

ustring  NeonPostBodyProvider::separatorLine () {
    return ustring (CharConst ("--")).append (separator).append (uCRLF);
}

ustring  NeonPostBodyProvider::separatorHeader () {
    return ustring (CharConst (kMIME_FORMDATA "; boundary=")).append (separator);
}

ustring  NeonPostBodyProvider::textSeparator (const ustring& name) {
    ustring  ans;
    ans.assign (CharConst ("--")).append (separator).append (uCRLF);
    ans.append (CharConst (kRES_DISP ": form-data; name=" kQ2)).append (percentEncode (query->cv (name))).append (CharConst (kQ2 kCRLF kCRLF));
    return ans;
}

ustring  NeonPostBodyProvider::fileSeparator (const ustring& name, const ustring& filename) {
    ustring  ans;
    ans.assign (CharConst ("--")).append (separator).append (uCRLF);
    ans.append (CharConst (kRES_DISP ": form-data; name=" kQ2)).append (percentEncode (query->cv (name))).append (CharConst (kQ2 "; filename=" kQ2)).append (slashEncode (filename)).append (CharConst (kQ2 kCRLF));
    ans.append (CharConst (kRES_TYPE ": ")).append (mimetype (getExt (filename))).append (CharConst (kCRLF kCRLF));
    return ans;
}

ustring  NeonPostBodyProvider::tailSeparator () {
    ustring  ans;
    ans.assign (CharConst ("--")).append (separator).append (CharConst ("--" kCRLF));
    return ans;
}

ne_off_t  NeonPostBodyProvider::calcLength () {
    ne_off_t  ans = 0;
    MNode*  a;
    ustring  u;
    off_t  s;

    tp = query->queryParam ();
    bp = postFile.begin ();
    ep = postFile.end ();
    if (isCons (tp) || bp < ep) {
    } else {
        return ans;     	// = 0
    }
    if (isCons (tp)) {
	while (tp) {
	    a = tp->car ();
	    if (isCons (a)) {
		ans += textSeparator (to_string (a->car ())).length () + to_string (a->cdr ()).length () + 2;
	    }
	    nextNode (tp);
	}
    }
    for (; bp < ep; ++ bp) {
	if (! fileSize ((*bp).second, s))
	    s = 0;
	u = filePart_osSafe ((*bp).second);
	ans += fileSeparator ((*bp).first, u).length () + s + 2;
    }
    ans += tailSeparator ().length ();
#ifdef DEBUG2
    std::cerr << "calcLength(): " << ans << "\n";
#endif /* DEBUG */
    return ans;
}

char*  NeonPostBodyProvider::bodyProvider (char* buffer, size_t buflen) {
    ssize_t  ans = 0;
    if (buflen == 0) {
	tp = query->queryParam ();
	bp = postFile.begin ();
	ep = postFile.end ();
	if (isCons (tp) || bp < ep) {
	    state = S_TEXT;
	} else {
	    state = S_DONE;
	}
	offset = 0;
	// 正常なら0を返す
    } else {
#ifdef DEBUG2
	std::cerr << "state:" << state << ", offset:" << offset << ", buflen:" << buflen << "\n";
#endif /* DEBUG */
	switch (state) {
	case S_TEXT:
	    if (offset > 0) {
		ans = bodyProviderText (buffer, buflen);
	    } else if (isCons (tp)) {
		MNode*  a = tp->car ();
		nextNode (tp);
		ubuf = textSeparator (to_string (a->car ())) + to_string (a->cdr ()) + uCRLF;
#ifdef DEBUG2
		std::cerr << "ubuf:" << omitNonAscii (ubuf.substr (0, 8)) << "\n";
#endif /* DEBUG */
		ans = bodyProviderText (buffer, buflen);
	    } else {
		state = S_FILEHEAD;
		ans = bodyProvider (buffer, buflen) - buffer;
	    }
	    break;
	case S_FILEHEAD:
	    if (offset > 0) {
		ans = bodyProviderText (buffer, buflen);
		if (offset == 0)
		    state = S_FILEBODY;
	    } else if (bp < ep) {
		ustring  u = filePart_osSafe ((*bp).second);
		ubuf = fileSeparator ((*bp).first, u);
#ifdef DEBUG2
		std::cerr << "ubuf:" << omitNonAscii (ubuf.substr (0, 8)) << "\n";
#endif /* DEBUG */
		ans = bodyProviderText (buffer, buflen);
		if (offset == 0)
		    state = S_FILEBODY;
	    } else {
		state = S_TAIL;
		ans = bodyProvider (buffer, buflen) - buffer;
	    }
	    break;
	case S_FILEBODY:
	    if (offset > 0) {
		ans = fd.read (buffer, buflen);
#ifdef DEBUG2
		std::cerr << "read\n";
#endif /* DEBUG */
		if (ans <= 0) {
		    fd.close ();
#ifdef DEBUG2
		    std::cerr << "close\n";
#endif /* DEBUG */
		    offset = 0;
		    ++ bp;
		    state = S_FILETERM;
		    ans = bodyProvider (buffer, buflen) - buffer;
		} else {
		    offset += ans;
		}
	    } else {	// offset == 0
		ustring  u = filePart_osSafe ((*bp).second);
#ifdef DEBUG2
		std::cerr << "open:" << (*bp).second << "\n";
#endif /* DEBUG */
		if (fd.openRead ((*bp).second.c_str ())) {
		    ans = fd.read (buffer, buflen);
		    if (ans <= 0) {
			fd.close ();
#ifdef DEBUG2
			std::cerr << "close\n";
#endif /* DEBUG */
			++ bp;
			state = S_FILETERM;
			ans = bodyProvider (buffer, buflen) - buffer;
		    } else {
			offset = ans;
		    }
		} else {
#ifdef DEBUG2
		    std::cerr << "open failed\n";
#endif /* DEBUG */
		    ++ bp;
		    state = S_FILETERM;
		    ans = bodyProvider (buffer, buflen) - buffer;
		}
	    }
	    break;
	case S_FILETERM:
	    if (offset > 0) {
		ans = bodyProviderText (buffer, buflen);
	    } else {
		ubuf = uCRLF;
#ifdef DEBUG2
		std::cerr << "ubuf:CRLF\n";
#endif /* DEBUG */
		ans = bodyProviderText (buffer, buflen);
	    }
	    if (offset == 0)
		state = S_FILEHEAD;
	    break;
	case S_TAIL:
	    if (offset > 0) {
		ans = bodyProviderText (buffer, buflen);
		if (offset == 0)
		    state = S_DONE;
	    } else {
		ubuf = tailSeparator ();
		ans = bodyProviderText (buffer, buflen);
		if (offset == 0)
		    state = S_DONE;
	    }
	    break;
	case S_DONE:
	    // 0を返す
	    break;
	default:;
	}
    }

    if (ans > 0 && buflen > ans)
	return bodyProvider (buffer + ans, buflen - ans);
#ifdef DEBUG2
    std::cerr << "bodyProvider ():" << ans << "\n";
#endif /* DEBUG */
    return buffer + ans;
}

ssize_t  NeonPostBodyProvider::bodyProviderText (char* buffer, size_t buflen) {
    ssize_t  ans = ubuf.length () - offset;
#ifdef DEBUG2
    std::cerr << "bodyProviderText: buflen:" << buflen << ", offset:" << offset << ", text:" << omitNonAsciiWord (ubuf.substr (offset, offset + 6)) << "\n";
#endif /* DEBUG */
    if (ans <= buflen) {
	memcpy (buffer, ubuf.data () + offset, ans);
	offset = 0;
    } else {
	memcpy (buffer, ubuf.data () + offset, buflen);
	offset += buflen;
	ans = buflen;
    }
    return ans;
}

//============================================================
static int  ignoreVerifyFn (void* neon, int failures, const ne_ssl_certificate* cert) {
#ifdef DEBUG2
    std::cerr << "ignoreVerifyFn ()\n";
#endif /* DEBUG */
    return 0;
}

NeonSession::NeonSession (proto_t _proto, const ustring& host, unsigned int port) {
    const char*  protoName;

    proto = _proto;
    switch (proto) {
    case PROTO_HTTP:
	protoName = "http";
	break;
    case PROTO_HTTPS:
	protoName = "https";
	break;
    default:
	assert (0);
    }
    session = ne_session_create (protoName, host.c_str (), port);
    if (! session)
	return;
    if (proto == PROTO_HTTPS)
	ne_ssl_trust_default_ca (session);
#ifdef DEBUG
    std::cerr << "session flag:"
	      << (ne_get_session_flag (session, NE_SESSFLAG_PERSIST) ? " NE_SESSFLAG_PERSIST" : "")
	      << (ne_get_session_flag (session, NE_SESSFLAG_ICYPROTO) ? " NE_SESSFLAG_ICYPROTO" : "")
	      << (ne_get_session_flag (session, NE_SESSFLAG_SSLv2) ? " NE_SESSFLAG_SSLv2" : "")
	      << (ne_get_session_flag (session, NE_SESSFLAG_RFC4918) ? " NE_SESSFLAG_RFC4918" : "")
	      << (ne_get_session_flag (session, NE_SESSFLAG_CONNAUTH) ? " NE_SESSFLAG_CONNAUTH" : "")
	      << (ne_get_session_flag (session, NE_SESSFLAG_TLS_SNI) ? " NE_SESSFLAG_TLS_SNI" : "")
	      << (ne_get_session_flag (session, NE_SESSFLAG_EXPECT100) ? " NE_SESSFLAG_EXPECT100" : "")
	      << "\n";
#endif /* DEBUG */
}

NeonSession::~NeonSession () {
    ne_session_destroy (session);
    session = NULL;
}

void  NeonSession::setNoVerify () {
    if (proto == PROTO_HTTPS)
	ne_ssl_set_verify (session, ignoreVerifyFn, this);
}

void  NeonSession::setProxy (const ustring& host, int port) {
    if (matchHostname (host) && port > 0 && port < 65536) {
#ifdef DEBUG2
	std::cerr << "set proxy " << host << ":" << port << "\n";
#endif /* DEBUG */
	ne_session_proxy (session, host.c_str (), port);
    } else {
	throw (ustring (CharConst ("bad proxy host.")));
    }
}

//============================================================
void  NeonQuery::closeReq () {
    if (req) {
	switch (mode) {
	case MODE_DISPATCHED:
	    ne_discard_response (req);
	case MODE_RECEIVED:
	    ne_end_request (req);
	case MODE_SETUP:;
	}
	mode = MODE_SETUP;
	ne_request_destroy (req);
	req = NULL;
    }
}

void  NeonQuery::setIConv (const char* name) {
    cv_in.reset (new UIConv (kCODE_UTF8, name));
    cv_out.reset (new UIConv (name, kCODE_UTF8));
}

static ssize_t  qbodyProvider (void* userdata, char* buffer, size_t buflen) {
    NeonPostBodyProvider*  obj = (NeonPostBodyProvider*)userdata;
    return obj->bodyProvider (buffer, buflen) - buffer;
}

void  NeonQuery::submit () {
    FileMacro  fd;
    ustring  uri = filterPath ();
    ustring  getQuery = buildGetQuery ();

    errorMsg.resize (0);
    if (getQuery.length () > 0) {
	uri.append (CharConst ("?"));
	uri.append (getQuery);
    }
    closeReq ();
    req = ne_request_create (session, methodStr (), uri.c_str ());
    buildCookie ();
    buildHeader ();
    switch (method) {
    case METHOD_POST:
	if (! rawQuery.isEmpty ()) {
	    setRawQuery (fd);
	} else {
	    rawQuery.setText ();
	    buildQuery (queryParam (), rawQuery.param);
	    setRawQuery (fd);
	}
	break;
    case METHOD_FILE:
	ne_add_request_header (req, kRES_TYPE, qbody->separatorHeader ().c_str ());
	ne_set_request_body_provider (req, qbody->calcLength (), qbodyProvider, qbody.get ());
	break;
    case METHOD_GET:
    case METHOD_PUT:
    case METHOD_DELETE:
    case METHOD_PATCH:
	if (! rawQuery.isEmpty ()) {
	    setRawQuery (fd);
	}
	break;
    default:;
    }
    int  rc = ne_begin_request (req);
    if (rc == NE_OK) {
	mode = MODE_DISPATCHED;
    } else {
	errorMsg.assign (ne_get_error (session));
#ifdef DEBUG
	std::cerr << "error: " << errorMsg << "\n";
#endif /* DEBUG */
    }
}

void  NeonQuery::readBody (MotorOutput* out) {
    if (mode == MODE_DISPATCHED) {
	char  buf[65536];
	ssize_t  s;
	while ((s = ne_read_response_block (req, buf, 65536)) > 0) {
	    out->out_raw (buf, s);
	}
	if (s == 0) {		// success
	    mode = MODE_RECEIVED;
	} else if (s < 0) {	// error
	    mode = MODE_RECEIVED;
	}
    }
}

int  NeonQuery::getStatus () {
    if (req) {
	const ne_status*  st = ne_get_status (req);
	if (st)
	    return st->code;
    }
    return 0;			// error
}

const char*  NeonQuery::methodStr () {
    switch (method) {
    case METHOD_GET:
	return kMETHOD_GET;
    case METHOD_POST:
	return kMETHOD_POST;
    case METHOD_PUT:
	return kMETHOD_PUT;
    case METHOD_DELETE:
	return kMETHOD_DELETE;
    case METHOD_HEAD:
	return kMETHOD_HEAD;
    case METHOD_PATCH:
	return kMETHOD_PATCH;
    case METHOD_FILE:
	return kMETHOD_POST;
    default:
	assert (0);
    }
}

ustring  NeonQuery::buildGetQuery () {
    ustring  ans;

    if (! isNil (getQueryParam ())) {
	buildQuery (getQueryParam (), ans);
    } else if (! rawGetQuery.isEmpty () && rawGetQuery.isText ()) {
	ans = rawGetQuery.read ();
    } else if (method == METHOD_GET) {
	if (! rawQuery.isEmpty () && rawQuery.isText ()) {
	    ans = rawQuery.read ();
	} else if (! isNil (queryParam ())) {
	    buildQuery (queryParam (), ans);
	}
    }
    return ans;
}

void  NeonQuery::buildQuery (MNode* e, ustring& out) {
    int  c = 0;
    MNode*  a;

    if (isCons (e)) {
	while (e) {
	    a = e->car ();
	    if (isCons (a)) {
		if (c > 0)
		    out.append (uAmp);
		out.append (percentEncode (cv (to_string (a->car ()))));
		if (! isNil (a->cdr ())) {
		    out.append (uEq);
		    out.append (percentEncode (cv (to_string (a->cdr ()))));
		}
		++ c;
	    }
	    nextNode (e);
	}
    }
}

ustring  NeonQuery::buildMimeSeparator_text (const ustring& name) {
    ustring  ans;
    ans.assign (qbody->separatorLine ());
    ans.append (CharConst ("Content-Disposition: form-data; name=" kQ2)).append (percentEncode (cv (name))).append (CharConst (kQ2 kCRLF kCRLF));
    return ans;
}

ustring  NeonQuery::buildMimeSeparator_file (const ustring& name, const ustring& filename) {
    ustring  ans;
    ans.assign (qbody->separatorLine ());
    ans.append (CharConst ("Content-Disposition: form-data; name=" kQ2)).append (percentEncode (cv (name))).append (CharConst (kQ2 "; filename=" kQ2)).append (slashEncode (filename)).append (CharConst (kQ2 kCRLF));
    ans.append (CharConst ("Content-Type: ")).append (mimetype (getExt (filename))).append (CharConst (kCRLF kCRLF));
    return ans;
}

ustring  NeonQuery::cv (const ustring& src) {
    if (cv_out.get ()) {
	return cv_out->cv (src, true);
    } else {
	return src;
    }
}

ustring  NeonQuery::rcv (const ustring& src) {
    if (cv_in.get ()) {
	return cv_in->cv (src, true);
    } else {
	return src;
    }
}

ustring  NeonQuery::getResponseHeader (const char* name) {
    const char*  ans = ne_get_response_header (req, name);
    if (ans)
	return ustring (ans);
    else
	return uEmpty;
}

MNode*  NeonQuery::getResponseHeaderAll () {
    MNodeList  ans;
    void*  csr = NULL;
    const char*  name;
    const char*  val;
    do {
	csr = ne_response_header_iterate (req, csr, &name, &val);
	if (csr)
	    ans.append (newMNode_cons (newMNode_str (new ustring (name)), newMNode_str (new ustring (val))));
	else
	    break;
    } while (1);
    return ans.release ();
}

ustring  NeonQuery::getResponseCookie (const ustring& name) {
    parseCookie ();
    for (int i = 0; i < replyCookie.size (); ++ i) {
	if (replyCookie[i].key == name)
	    return replyCookie[i].value;
    }
    return uEmpty;
}

MNode*  NeonQuery::getResponseCookieAll () {
    MNodeList  ans;
    parseCookie ();
    for (int i = 0; i < replyCookie.size (); ++ i) {
	ans.append (newMNode_cons (newMNode_str (new ustring (replyCookie[i].key)), newMNode_str (new ustring (replyCookie[i].value))));
    }
    return ans.release ();
}

ustring  NeonQuery::filterPath () {
    if (rawPath) {
	return omitCtrl (path);
    } else {
	return percentEncode_path (omitCtrl (path));
    }
}

void  NeonQuery::buildCookie () {
    MNode*  e = cookie ();
    MNode*  a;
    ustring  data;
    size_t  off = 0;
    ustring  u;

    if (isCons (e)) {
	while (e) {
	    a = e->car ();
	    if (isNil (a)) {
	    } else if (isCons (a)) {
		u.assign (cookieencode (to_string (a->car ()))).append (uEq).append (cookieencode (to_string (a->cdr ())));
		if (off > 800) {
		    data.append (CharConst (";" kCRLF " ")).append (u);
		    off = u.length ();
		} else if (off > 0) {
		    data.append (CharConst ("; ")).append (u);
		    off += u.length () + 2;
		} else {
		    data.append (u);
		    off += u.length ();
		}
	    } else {
		throw (cookie ()->dump_string_short () + uErrorBadParam);
	    }
	    nextNode (e);
	}
    }
    if (data.length () > 0)
	ne_add_request_header (req, "Cookie", data.c_str ());
}

void  NeonQuery::buildHeader () {
    MNode*  e = header ();
    MNode*  a;
    ustring  key, val;
    static uregex  re1 ("[\\x00-\\x1f: ]");
    static uregex  re2 ("[\\x00-\\x1f]");

    if (isCons (e)) {
	while (e) {
	    a = e->car ();
	    if (isNil (a)) {
	    } else if (isCons (a)) {
		key = to_string (a->car ());
		val = to_string (a->cdr ());
		if (! checkRe (key, re1) && ! checkRe (val, re2) && key.length () + val.length () < 2048) {
		    ne_add_request_header (req, key.c_str (), val.c_str ());
		}
	    } else {
		throw (cookie ()->dump_string_short () + uErrorBadParam);
	    }
	    nextNode (e);
	}
    }
    if (basicID.length () > 0)
	buildBasicAuthHeader ();
}

void  NeonQuery::buildBasicAuthHeader () {
    ustring  idpw;
    ustring  auth;
    idpw.assign (basicID).append (uColon).append (basicPW);
    auth.assign (CharConst ("Basic ")).append (base64Encode (idpw.begin (), idpw.end ()));
    ne_add_request_header (req, "Authorization", auth.c_str ());
}

void  NeonQuery::setFormType () {
    if (querytype.length () > 0) {
	ne_add_request_header(req, "Content-type", querytype.c_str ());
    } else {
	setFormType_urlencoded ();
    }
}

void  NeonQuery::setFormType_urlencoded () {
    ne_add_request_header(req, "Content-type", kMIME_URLENCODED);
}

void  NeonQuery::setFormType_formdata () {
    ne_add_request_header(req, "Content-type", kMIME_FORMDATA);
}

void  NeonQuery::parseCookie () {
    if (! replyCookieDone) {
	replyCookieDone = true;
	ustring  u = getResponseHeader ("set-cookie");
	uiterator  b = u.begin ();
	uiterator  e = u.end ();
	uregex  re1 ("[=,;]");
	uregex  re2 ("[,;]");
	uregex  re3 ("^path=|expires=|domain=|secure");
	uregex  re4 ("^..., *[0-9]+[^,;]+");
	umatch  m;
	while (b < e && usearch (b, e, m, re1)) {	// = | , | ;
	    CookieInfo  info;
	    switch (*m[0].first) {
	    case '=':
		info.key.assign (b, m[0].first);
		b = m[0].second;
		if (usearch (b, e, m, re2)) {	// , | ;
		    info.value.assign (b, m[0].first);
		    if (*m[0].first == ';') {
			b = m[0].second;
			while (b < e) {
			    if (usearch (b, e, m, re3)) {	// path=,expires=,...
				if (*m[0].first == 'e') {
				    if (usearch (b, e, m, re4)) {
					b = m[0].second;
					if (*(b - 1) == ',') {
					    skipSpace (b, e);
					    break;
					} else {	// ;
					    skipSpace (b, e);
					}
				    } else if (usearch (b, e, m, re2)) {
					b = m[0].second;
					skipSpace (b, e);
					if (*m[0].first == ',')
					    break;
				    } else {
					b = e;
				    }
				} else if (usearch (b, e, m, re2)) {
				    b = m[0].second;
				    skipSpace (b, e);
				    if (*m[0].first == ',')
					break;
				} else {
				    b = e;
				}
			    } else if (usearch (b, e, m, re2)) {	// , | ;
				b = m[0].second;
				skipSpace (b, e);
				if (*m[0].first == ',')
				    break;	// while
			    } else {
				b = e;
			    }
			}
		    }
		} else {
		    info.value.assign (b, e);
		}
		replyCookie.push_back (info);
		break;
	    case ';':
		b = m[0].second;
		skipNextToChar (b, e, ',');
		skipSpace (b, e);
		break;
	    case ',':
	    default:
		b = m[0].second;
		skipSpace (b, e);
	    }
	}
    }
}

void  NeonQuery::setRawQuery (FileMacro& fd) {
    setFormType ();
    if (rawQuery.isText ()) {
	ne_set_request_body_buffer (req, rawQuery.param.c_str (), rawQuery.param.size ());
    } else {
	ustring  src = rawQuery.src ();
	if (src.length () > 0) {
	    if (fd.openRead (src.c_str ())) {
		ne_set_request_body_fd (req, fd.fd, 0, fd.size ());
	    }
	}
    }
}

//============================================================
void  MLNeon::newSession (MlEnv* mlenv) {
    session.reset (new NeonSession (proto, host, port));
    // *** User-Agent
//    query.reset (new NeonQuery (session->get (), mlenv));
}

//============================================================
/*DOC:
==neon library==

*/
/*DOC:
===$neon, %neon==
 (%neon [#http | #https] Host Port
 	:module-store VAR
 	:proxy '(HOSTNAME . PORT) :proxy-user '(ID . PASSWORD)
 	:on-error FUNCTION
 	#no-verify
 	[SUBFUNCTION...]) -> LAST_VALUE

*/
//#XMFUNC	$neon	ml_neon	cMLNeonID
//#XMFUNC	%neon	ml_neon	cMLNeonID
MNode*  ml_neon (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_LIST, EV_END};
    MNodePtr  posParams[2];
    kwParam  kwList[] = {{CharConst ("http"), EV_LIST},		// 0
			 {CharConst ("https"), EV_LIST},	// 1
			 {CharConst ("proxy"), EV_LIST},	// 2
			 {CharConst ("proxy-user"), EV_LIST},	// 3
			 {CharConst ("on-error"), EV_LIST},	// 4
			 {CharConst ("no-verify"), EV_LIST},	// 5
			 {CharConst ("module-store"), EV_LIST},	// 6
			 {NULL, 0, EV_END}};
    MNodePtr  kwParams[7];
    MNodePtr  rest;
    evalParams (fev, mlenv, cell, posList, posParams, kwList, kwParams, EV_ASIS, &rest);
    NeonInit.init ();
    MLNeon  obj (mlenv);
    obj.host = to_string (posParams[0]());
    obj.port = to_int64 (posParams[1]());
    if (to_bool (kwParams[0]())) {
	obj.proto = NeonSession::PROTO_HTTP;
    } else if (to_bool (kwParams[1]())) {
	obj.proto = NeonSession::PROTO_HTTPS;
    }
    if (! isNil (kwParams[2]())) {
	if (isCons (kwParams[2]())) {
	    obj.proxyhost = to_string (kwParams[2] ()->car ());
	    obj.proxyport = to_int64 (kwParams[2] ()->cdr ());
	} else {
	    throw (kwParams[2] ()->dump_string_short () + uErrorBadParam);
	}
    }
    if (! isNil (kwParams[3] ())) {			// 3:proxy-user
	if (isCons (kwParams[3]())) {
	    obj.proxyid = to_string (kwParams[3]()->car ());
	    obj.proxypw = to_string (kwParams[3]()->cdr ());
	} else {
	    throw (kwParams[3]()->dump_string_short () + uErrorBadParam);
	}
    }
    MNode*  errfn = kwParams[4]();
    bool  b = to_bool (kwParams[5]());
    MNodePtr  objStore;
    if (! isNil (kwParams[6]())) {			// 6:module-store
	if (isSym (kwParams[6]())) {
	    objStore = newMNode_libobj (&obj);
	    mlenv->setVar (to_string (kwParams[6]()), objStore ());
	} else {
	    throw (kwParams[6]()->dump_string_short () + uErrorBadParam);
	}
    }
    MNodePtr  ans;
    if (! matchHostname (obj.host))
	throw (obj.host + ": bad hostname.");
    if (obj.port <= 0 || obj.port >= 65536)
	throw (to_ustring (obj.port) + ": bad port number.");
    //****** proxy-userが未実装
    obj.newSession (mlenv);
    if (obj.proxyhost.length () > 0)
	obj.session->setProxy (obj.proxyhost, obj.proxyport);
    if (obj.fnoverify)
	obj.session->setNoVerify ();
    mlenv->setMStack (&obj);
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
    if (objStore () && objStore ()->isLibObj ())
	objStore ()->type = MNode::MC_NIL;		// 強制NIL化
    return mlenv->retval = ans.release ();
}

static void  buildFileList (NeonQuery* query, MNode* postFileSerial, MNode* postFileNamed, MNode* postFileStatic, MlEnv* mlenv) {
    MNode*  e;
    MNode*  a;
    ustring  name;
    ustring  val;
    ustring  path;

    e = postFileSerial;
    if (isCons (e)) {
	while (e) {
	    if ((a = e->car ()) && isCons (a)) {
		name = to_string (a->car ());
		val = to_string (a->cdr ());
		path = mlenv->env->path_store_file (val);
		query->qbody->pushFile (name, path);
	    }
	    nextNode (e);
	}
    }
    e = postFileNamed;
    if (isCons (e)) {
	while (e) {
	    if ((a = e->car ()) && isCons (a)) {
		name = to_string (a->car ());
		val = to_string (a->cdr ());
		path = mlenv->env->path_storage_file (val);
		query->qbody->pushFile (name, path);
	    }
	    nextNode (e);
	}
    }
    e = postFileStatic;
    if (isCons (e)) {
	while (e) {
	    if ((a = e->car ()) && isCons (a)) {
		name = to_string (a->car ());
		val = to_string (a->cdr ());
		path = mlenv->env->path_static_file (val);
		if (path.size () == 0)
		    throw (val + uErrorBadFile);
		query->qbody->pushFile (name, path);
	    }
	    nextNode (e);
	}
    }
}

/*DOC:
===subfunctions of $neon===

*/
static MNode*  checkConsList (MNode* e) {
    MNodeList  ans;
    MNode*  a;
    if (isCons (e)) {
	while (e) {
	    if (isCons ((a = e->car ())))
		ans.append (a);
	    nextNode (e);
	}
    }
    return ans.release ();
}

/*DOC:
====http-request====
 (http-request PATH [#get | #post | #put | #delete | #head | #patch | #file]
 	:basic-user '(ID . PASSWORD)
 	:query '((NAME . VALUE) ...)
 	:get-query '((NAME . VALUE) ...)
// 	#no-encode
 	:post-file-serial '((NAME . FILE) ...)
 	:post-file-named '((NAME . FILE) ...)
 	:post-file-static '((NAME . FILE) ...)
 	:raw-query TEXT :raw-file-serial FILE :raw-file-named FILE :raw-file-static FILE
	:raw-get-query TEXT
 	:query-type MIMETYPE
 	:cookie '((NAME . VALUE) ...) :header '((NAME . VALUE) ...)
 	[#sjis | #euc-jp | :iconv NAME]
	:module MODULE
*/
//#XSFUNC	http-request	ml_neon_http_request	$neon
//#XSFUNC	http-request	ml_neon_http_request	%neon
MNode*  ml_neon_http_request (bool fev, MNode* cell, MlEnv* mlenv, MLFunc* mobj) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    kwParam  kwList[] = {{CharConst ("get"), EV_LIST},			// 0
			 {CharConst ("post"), EV_LIST},			// 1
			 {CharConst ("put"), EV_LIST},			// 2
			 {CharConst ("delete"), EV_LIST},		// 3
			 {CharConst ("head"), EV_LIST},			// 4
			 {CharConst ("file"), EV_LIST},			// 5
			 {CharConst ("basic-user"), EV_LIST},		// 6
			 {CharConst ("query"), EV_LIST},		// 7
			 {CharConst ("get-query"), EV_LIST},		// 8
			 {CharConst ("post-file-serial"), EV_LIST},	// 9
			 {CharConst ("post-file-named"), EV_LIST},	// 10
			 {CharConst ("post-file-static"), EV_LIST},	// 11
			 {CharConst ("raw-query"), EV_LIST},		// 12
			 {CharConst ("raw-file-serial"), EV_LIST},	// 13
			 {CharConst ("raw-file-named"), EV_LIST},	// 14
			 {CharConst ("raw-file-static"), EV_LIST},	// 15
			 {CharConst ("query-type"), EV_LIST},		// 16
			 {CharConst ("cookie"), EV_LIST},		// 17
			 {CharConst ("header"), EV_LIST},		// 18
			 {CharConst ("sjis"), EV_LIST},			// 19
			 {CharConst ("euc-jp"), EV_LIST},		// 20
			 {CharConst ("iconv"), EV_LIST},		// 21
			 {CharConst ("module"), EV_LIST},		// 22
			 {CharConst ("raw-get-query"), EV_LIST},	// 23
			 {CharConst ("patch"), EV_LIST},		// 24
			 {CharConst ("raw-path"), EV_LIST},		// 25
			 {NULL, 0, EV_END}};
    MNodePtr  kwParams[26];
    evalParams (fev, mlenv, cell, posList, posParams, kwList, kwParams);
    MLNeon*  obj = (MLNeon*)mobj;
    MNodePtr  postFileSerial;
    MNodePtr  postFileNamed;
    MNodePtr  postFileStatic;
    if (! isNil (kwParams[22]()))			// 22:module
	assignNLNeon (obj, kwParams[22] ());
    obj->query.reset (new NeonQuery (obj->session->get (), mlenv));
    obj->query->path = to_string (posParams[0]());
    obj->query->rawPath = to_bool (kwParams[25] ());	// 25:raw-path
    if (to_bool (kwParams[0] ())) {			// 0:get
	obj->query->method = NeonQuery::METHOD_GET;
    } else if (to_bool (kwParams[1] ())) {		// 1:post
	obj->query->method = NeonQuery::METHOD_POST;
    } else if (to_bool (kwParams[2] ())) {		// 2:put
	obj->query->method = NeonQuery::METHOD_PUT;
    } else if (to_bool (kwParams[3] ())) {		// 3:delete
	obj->query->method = NeonQuery::METHOD_DELETE;
    } else if (to_bool (kwParams[4] ())) {		// 4:head
	obj->query->method = NeonQuery::METHOD_HEAD;
    } else if (to_bool (kwParams[5] ())) {		// 5:file / pseudo method
	obj->query->method = NeonQuery::METHOD_FILE;
    } else if (to_bool (kwParams[24] ())) {		// 24: patch
	obj->query->method = NeonQuery::METHOD_PATCH;
    }
    if (! isNil (kwParams[6] ())) {			// 6:basic-user
	if (isCons (kwParams[6] ())) {
	    obj->query->basicID = to_string (kwParams[6] ()->car ());
	    obj->query->basicPW = to_string (kwParams[6] ()->cdr ());
	} else {
	    throw (kwParams[6] ()->dump_string_short () + uErrorBadParam);
	}
    }
    if (! isNil (kwParams[7] ()))			// 7:query
	obj->query->queryParam = checkConsList (kwParams[7] ());
    if (! isNil (kwParams[8] ()))			// 8:get-query
	obj->query->getQueryParam = checkConsList (kwParams[8] ());
    if (! isNil (kwParams[23] ()))			// 23:raw-get-query
	obj->query->rawGetQuery.srcText (to_string (kwParams[23] ()));
    if (! isNil (kwParams[9] ()))			// 9:post-file-serial
	postFileSerial = checkConsList (kwParams[9] ());
    if (! isNil (kwParams[10] ()))			// 10:post-file-named
	postFileNamed = checkConsList (kwParams[10] ());
    if (! isNil (kwParams[11] ()))			// 11:post-file-static
	postFileStatic = checkConsList (kwParams[11] ());
    if (! isNil (kwParams[12] ()))			// 12:raw-query
	obj->query->rawQuery.srcText (to_string (kwParams[12] ()));
    if (! isNil (kwParams[13] ()))			// 13:raw-file-serial
	obj->query->rawQuery.srcSerial (to_string (kwParams[13] ()));
    if (! isNil (kwParams[14] ()))			// 14:raw-file-named
	obj->query->rawQuery.srcNamed (to_string (kwParams[14] ()));
    if (! isNil (kwParams[15] ()))			// 15:raw-file-static
	obj->query->rawQuery.srcStatic (to_string (kwParams[15] ()));
    if (! isNil (kwParams[16] ())) {			// 16:query-type
	obj->query->querytype = to_string (kwParams[16] ());
	if (! matchASCII (obj->query->querytype.begin (), obj->query->querytype.end ()))
	    throw (obj->query->querytype + ustring (CharConst (": bad type")));
    }
    obj->query->cookie = kwParams[17] ();		// 17:cookie
    obj->query->header = kwParams[18] ();		// 18:header
    if (to_bool (kwParams[19] ())) {			// 19:sjis
	obj->query->setIConv ("SHIFT_JIS");
    } else if (to_bool (kwParams[20] ())) {		// 20:euc-jp
	obj->query->setIConv ("EUC-JP");
    } else if (! isNil (kwParams[21] ())) {		// 21:iconv
	ustring  code = to_string (kwParams[21] ());
	uregex  re ("^[a-zA-Z0-9][a-zA-Z0-9_.:-]*$");
	umatch  m;
	if (usearch (code, m, re)) {
	    obj->query->setIConv (code.c_str ());
	} else {
	    throw (ustring (code).append (CharConst (": unknown encoding.")));
	}
    }
    if (obj->query->method == NeonQuery::METHOD_FILE)
	buildFileList (obj->query.get (), postFileSerial (), postFileNamed (), postFileStatic (), mlenv);
    obj->query->submit ();
    return NULL;
}

/*DOC:
====http-status====
 (http-status :module MODULE) -> NUMBER

*/
//#XSFUNC	http-status	ml_neon_http_status	$neon
//#XSFUNC	http-status	ml_neon_http_status	%neon
MNode*  ml_neon_http_status (bool fev, MNode* cell, MlEnv* mlenv, MLFunc* mobj) {
    kwParam  kwList[] = {{CharConst ("module"), EV_LIST},
			 {NULL, 0, EV_END}};
    MNodePtr  kwParams[1];
    evalParams (fev, mlenv, cell, NULL, NULL, kwList, kwParams);
    MLNeon*  obj = (MLNeon*)mobj;
    if (! isNil (kwParams[0] ()))			// 0:module
	assignNLNeon (obj, kwParams[0] ());
    if (! obj->query)
	return NULL;
    return newMNode_num (obj->query->getStatus ());
}

/*DOC:
====http-response====
 (http-response :module MODULE) -> STRING

*/
//#XSFUNC	http-response	ml_neon_http_response	$neon
//#XSFUNC	http-response	ml_neon_http_response	%neon
MNode*  ml_neon_http_response (bool fev, MNode* cell, MlEnv* mlenv, MLFunc* mobj) {
    kwParam  kwList[] = {{CharConst ("module"), EV_LIST},
			 {NULL, 0, EV_END}};
    MNodePtr  kwParams[1];
    evalParams (fev, mlenv, cell, NULL, NULL, kwList, kwParams);
    MLNeon*  obj = (MLNeon*)mobj;
    ustring  ans;
    if (! isNil (kwParams[0] ()))			// 0:module
	assignNLNeon (obj, kwParams[0] ());
    if (! obj->query)
	return NULL;
    std::stringstream  ostr;
    MotorOutputOStream  out (&ostr);
    obj->query->readBody (&out);
    return newMNode_str (new ustring (fixUTF8 (obj->query->rcv (ostr.str ()))));
}

/*DOC:
====http-response-file====
 (http-response-file FILENAME :module MODULE) -> NIL

*/
//#XSFUNC	http-response-file	ml_neon_http_response_file	$neon
//#XSFUNC	http-response-file	ml_neon_http_response_file	%neon
MNode*  ml_neon_http_response_file (bool fev, MNode* cell, MlEnv* mlenv, MLFunc* mobj) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    kwParam  kwList[] = {{CharConst ("module"), EV_LIST},
			 {NULL, 0, EV_END}};
    MNodePtr  kwParams[1];
    evalParams (fev, mlenv, cell, posList, posParams, kwList, kwParams);
    MLNeon*  obj = (MLNeon*)mobj;
    ustring  name = to_string (posParams[0] ());
    if (! isNil (kwParams[0] ()))					// 0:module
	assignNLNeon (obj, kwParams[0] ());
    if (! obj->query)
	return NULL;
    ustring  tgt = mlenv->env->path_store_file (name);
    ustring  tmp;
    tmp.assign (tgt).append (CharConst ("-tmp")).append (to_ustring (getpid ()));
    {
	std::ofstream  stream;
	MotorOutputOStream  out (&stream);
	stream.open (tmp.c_str (), std::ios_base::out | std::ios_base::binary);
	obj->query->readBody (&out);
	stream.close ();
	rename (tmp.c_str (), tgt.c_str ());
    }

    return NULL;
}

/*DOC:
====http-response-output====
 (http-response-output :module MODULE #continue) -> NIL

*/
//#XSFUNC	http-response-output	ml_neon_http_response_output	$neon
//#XSFUNC	http-response-output	ml_neon_http_response_output	%neon
MNode*  ml_neon_http_response_output (bool fev, MNode* cell, MlEnv* mlenv, MLFunc* mobj) {
    kwParam  kwList[] = {{CharConst ("continue"), EV_LIST},		// 0
			 {CharConst ("module"), EV_LIST},		// 1
			 {NULL, 0, EV_END}};
    MNodePtr  kwParams[2];
    evalParams (fev, mlenv, cell, NULL, NULL, kwList, kwParams);
    MLNeon*  obj = (MLNeon*)mobj;
    bool  cflag = to_bool (kwParams[0] ());				// 0:continue
    if (! isNil (kwParams[1] ()))					// 1:module
	assignNLNeon (obj, kwParams[1] ());
    if (! obj->query)
	return NULL;
    if (! mlenv->env->responseDone)
	mlenv->env->standardResponse_html ();
    obj->query->readBody (mlenv->env->output);
    if (! cflag)
	mlenv->breakProg ();
    return NULL;
}

/*DOC:
====http-get-cookie====
 (http-get-cookie NAME :module MODULE) -> STRING

*/
//#XSFUNC	http-get-cookie	ml_neon_http_get_cookie	$neon
//#XSFUNC	http-get-cookie	ml_neon_http_get_cookie	%neon
//#XSFUNC	get-cookie	ml_neon_http_get_cookie	$neon
//#XSFUNC	get-cookie	ml_neon_http_get_cookie	%neon
MNode*  ml_neon_http_get_cookie (bool fev, MNode* cell, MlEnv* mlenv, MLFunc* mobj) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    kwParam  kwList[] = {{CharConst ("module"), EV_LIST},
			 {NULL, 0, EV_END}};
    MNodePtr  kwParams[1];
    evalParams (fev, mlenv, cell, posList, posParams, kwList, kwParams);
    MLNeon*  obj = (MLNeon*)mobj;
    ustring  name = to_string (kwParams[0] ());
    if (! isNil (kwParams[0] ()))			// 0:module
	assignNLNeon (obj, kwParams[0] ());
    if (! obj->query)
	return NULL;
    ustring  ans = obj->query->getResponseCookie (name);
    if (ans.length () > 0) {
	return newMNode_str (new ustring (ans));
    } else {
	return NULL;
    }
}

/*DOC:
====http-get-all-cookies====
 (http-get-all-cookies :module MODULE) -> LIST of (NAME . VALUE)

*/
//#XSFUNC	http-get-all-cookies	ml_neon_http_get_all_cookies	$neon
//#XSFUNC	http-get-all-cookies	ml_neon_http_get_all_cookies	%neon
// --quirk
//#XSFUNC	get-cookie-all	ml_neon_http_get_all_cookies	$neon
//#XSFUNC	get-cookie-all	ml_neon_http_get_all_cookies	%neon
// quirk--
MNode*  ml_neon_http_get_all_cookies (bool fev, MNode* cell, MlEnv* mlenv, MLFunc* mobj) {
    kwParam  kwList[] = {{CharConst ("module"), EV_LIST},
			 {NULL, 0, EV_END}};
    MNodePtr  kwParams[1];
    evalParams (fev, mlenv, cell, NULL, NULL, kwList, kwParams);
    MLNeon*  obj = (MLNeon*)mobj;
    if (! isNil (kwParams[0] ()))			// 0:module
	assignNLNeon (obj, kwParams[0] ());
    if (! obj->query)
	return NULL;
    return obj->query->getResponseCookieAll ();
}

/*DOC:
====http-content-type====
 (http-content-type :module MODULE) -> STRING

*/
//#XSFUNC	http-content-type	ml_neon_http_content_type	$neon
//#XSFUNC	http-content-type	ml_neon_http_content_type	%neon
MNode*  ml_neon_http_content_type (bool fev, MNode* cell, MlEnv* mlenv, MLFunc* mobj) {
    kwParam  kwList[] = {{CharConst ("module"), EV_LIST},
			 {NULL, 0, EV_END}};
    MNodePtr  kwParams[1];
    evalParams (fev, mlenv, cell, NULL, NULL, kwList, kwParams);
    MLNeon*  obj = (MLNeon*)mobj;
    if (! isNil (kwParams[0] ()))				// 0:module
	assignNLNeon (obj, kwParams[0] ());
    if (! obj->query)
	return NULL;
    ustring  u = obj->query->getResponseHeader ("content-type");
    if (u.length () > 0) {
	uiterator  b, e, m;
	b = u.begin ();
	e = u.end ();
	if (splitChar (b, e, ';', m)) {
	    return newMNode_str (new ustring (toLower (b, m)));
	} else {
	    return newMNode_str (new ustring (toLower (b, e)));
	}
    } else {
	return NULL;
    }
}

/*DOC:
====http-get-header====
 (http-get-header NAME :module MODULE) -> STRING

*/
//#XSFUNC	http-get-header	ml_neon_http_get_header	$neon
//#XSFUNC	http-get-header	ml_neon_http_get_header	%neon
MNode*  ml_neon_http_get_header (bool fev, MNode* cell, MlEnv* mlenv, MLFunc* mobj) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    kwParam  kwList[] = {{CharConst ("module"), EV_LIST},
			 {NULL, 0, EV_END}};
    MNodePtr  kwParams[1];
    evalParams (fev, mlenv, cell, posList, posParams, kwList, kwParams);
    MLNeon*  obj = (MLNeon*)mobj;
    ustring  name = to_string (posParams[0] ());
    if (! isNil (kwParams[0] ()))					// 0:module
	assignNLNeon (obj, kwParams[0] ());
    if (! obj->query)
	return NULL;
    ustring  u = obj->query->getResponseHeader (name.c_str ());
    if (u.length () > 0) {
	return newMNode_str (new ustring (u));
    } else {
	return NULL;
    }
}

/*DOC:
====http-get-all-headers====
 (http-get-all-headers :module MODULE) -> LIST of (NAME . VALUE)

*/
//#XSFUNC	http-get-all-headers	ml_neon_http_get_all_headers	$neon
//#XSFUNC	http-get-all-headers	ml_neon_http_get_all_headers	%neon
// --quirk
//#XSFUNC	http-get-header-all	ml_neon_http_get_all_headers	$neon
//#XSFUNC	http-get-header-all	ml_neon_http_get_all_headers	%neon
// quirk--
MNode*  ml_neon_http_get_all_headers (bool fev, MNode* cell, MlEnv* mlenv, MLFunc* mobj) {
    kwParam  kwList[] = {{CharConst ("module"), EV_LIST},
			 {NULL, 0, EV_END}};
    MNodePtr  kwParams[1];
    evalParams (fev, mlenv, cell, NULL, NULL, kwList, kwParams);
    MLNeon*  obj = (MLNeon*)mobj;
    if (! isNil (kwParams[0] ()))			// 0:module
	assignNLNeon (obj, kwParams[0] ());
    if (! obj->query)
	return NULL;
    return obj->query->getResponseHeaderAll ();
}

