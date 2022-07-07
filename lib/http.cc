#include "http.h"
#include "httpconst.h"
#include "motorenv.h"
#include "motoroutput.h"
#include "mlenv.h"
#include "util_apache.h"
#include "util_file.h"
#include "util_string.h"
#include "util_time.h"
#include "util_base64.h"
#include "util_check.h"
#include "util_const.h"
#include "util_random.h"
#include "util_splitter.h"
#include "util_tcp.h"
#include "util_mimetype.h"
#include "filemacro.h"
#include "ustring.h"
#include "utf8.h"
#include <vector>
#include <iostream>
#include <stdlib.h>

#define NO_KEEPALIVE
//#define HTTPS_NOCACHE

void  HTTPResponse::printCacheControl (MotorOutput* out) {
    out->out_raw (CharConst ("Cache-Control: private\n"));
    // privateは、プロキシ等のキャッシュ
    // no-storeは、ブラウザキャッシュ
}

void  HTTPResponse::printCookie (MotorOutput* out, MotorEnv* env) {
    std::vector<ustring>::iterator  it;
    
    for (it = setcookie.begin (); it != setcookie.end (); it ++) {
	out->out_raw (CharConst ("Set-Cookie: "))->out_noCtrl (*it)->out_raw (uLF);
#ifdef DEBUG
	if (env->log) {
	    *env->log << "Set-Cookie: " << *it << "\n";
	}
#endif /* DEBUG */
    }
}

void  HTTPResponse::setCookie (const ustring& key, const ustring& val, const ustring& path, time_t span, time_t limit, const ustring& domain, bool fsecure, MotorEnv* env) {
    ustring  ck;
    ustring  u;
    size_t  len;
    
    if (key.size () <= 128 && val.size () <= 512) {
	ck = cookieencode (key);
	ck.append (uEq);
	ck.append (cookieencode (val));
	ck.append (CharConst ("; path="));
	u = apacheAbsolutePath (path);
	len = u.length ();
	if (len > 1 && u[len - 1] == '/') { // '/'のみはOK。
	    u.resize (len - 1);
	}
	ck.append (u);
	if (span > 0) {
	    limit = now () + span;
	}
	if (limit > 0) {
	    ck.append (CharConst ("; expires="));
	    ck.append (dateCookie (limit));
	}
	if (domain.size () > 0 && matchDomain_dot (domain)) { // ???
	    ck.append (CharConst ("; domain="));
	    ck.append (domain);
	}
	if (fsecure) {
	    ck.append (CharConst ("; secure"));
	}

	setcookie.push_back (ck);
    }
}

void  HTTPResponse::setCookiePair (uiterator b, const uiterator e) {
    uiterator  m;
    ustring  key, val;

    if ((m = find (b, e, '=')) != e) {
	if (m != b) {
	    key = ustring (b, m);
	    key = omitNonAsciiWord (key);
	    val = ustring (m + 1, e);
	    val = omitNonAsciiWord (val);
	    val = cookiedecode (val);
	    val = fixUTF8 (val);
	    cookiemap.insert (boost::unordered_map<ustring, ustring>::value_type (key, val));
	}
    }
}

void  HTTPResponse::parseCookie () {
    cookieDone = true;
    cookie = getenvString (kHTTP_COOKIE);
    SplitterFn  sp (cookie, findSepColon);
    while (sp.next ()) {
	if (sp.preSize () > 0)
	    setCookiePair (sp.begin (), sp.end ());
    }
}

ustring  HTTPResponse::readCookie (const ustring& key) {
    boost::unordered_map<ustring, ustring>::iterator  it;

    if (! cookieDone)
	parseCookie ();

    it = cookiemap.find (key);
    if (it != cookiemap.end ()) {
	return it->second;
    } else {
	return uEmpty;
    }
}

void  HTTPResponse::setRandomCookie (MotorEnv* env) {
    ustring  u = smallRandomKey ();
    setCookie (uR, u, uSlash, 0, 0, uEmpty, false, env);
}

void  HTTPResponse::standardResponse (MotorOutput* out, const ustring& type, const ustring& charset, MotorEnv* env) {
#ifdef STANDALONE
    if (env && env->documentRoot.length () == 0)
	return;
#endif
    if (! charset.empty () && matchHead (type, CharConst ("text/"))) {
	out->out_raw (CharConst (kRES_TYPE ": "))->out_noCtrl (type)->out_raw (CharConst ("; " kCHARSET))->out_noCtrl (charset)->out_raw (uLF);
    } else {
	out->out_raw (CharConst (kRES_TYPE ": "))->out_noCtrl (type)->out_raw (uLF);
    }
    printMoreHeader (out, env);
#ifdef HTTPS_NOCACHE
    if (fNoCache || isHTTPS ())
	printCacheControl (out);
#else
    if (fNoCache)
	printCacheControl (out);
#endif
    printCookie (out, env);
#ifdef NO_KEEPALIVE
    out->out_raw (CharConst (kRES_CONNECTION_CLOSE))->out_raw (uLF);
#endif
    switch (frameOpt) {
    case FOPT_NONE:
	break;
    case FOPT_DENY:
	out->out_raw (CharConst (kRES_FRAMEOPT ": " "DENY"))->out_raw (uLF);
	break;
    case FOPT_SAMEORIGIN:
	out->out_raw (CharConst (kRES_FRAMEOPT ": " "SAMEORIGIN"))->out_raw (uLF);
	break;
    case FOPT_ALLOWFROM:
	assert (0);
	out->out_raw (CharConst (kRES_FRAMEOPT ": " "ALLOW-FROM "));
	out->out_toHTML (foptUri);
	out->out_raw (uLF);
	break;
    default:;
    }
    out->out_raw (uLF);
}

void  HTTPResponse::standardResponse_html (MotorOutput* out, MotorEnv* env) {
#ifdef STANDALONE
    if (env && env->documentRoot.length () == 0)
	return;
#endif
    out->out_raw (CharConst (kRES_TYPE ": " kMIME_HTML));
    if (env && env->output) {
	out->out_raw (CharConst ("; " kCHARSET))->out_noCtrl (env->output->charset ())->out_raw (uLF);
    }
    printMoreHeader (out, env);
#ifdef HTTPS_NOCACHE
    if (fNoCache || isHTTPS ())
	printCacheControl (out);
#else
    if (fNoCache)
	printCacheControl (out);
#endif
    printCookie (out, env);
#ifdef NO_KEEPALIVE
    out->out_raw (CharConst (kRES_CONNECTION_CLOSE))->out_raw (uLF);
#endif
    out->out_raw (uLF);
}

void  HTTPResponse::disposition (MotorOutput* out, bool finline, const ustring& name, off_t filesize) {
    ustring  n2;
    SplitterCh sp (name, '\"');

    if (sp.nextSep ()) {
	n2.reserve (name.length ());
	n2.append (sp.pre ());
	n2.append (uUScore);
	while (sp.nextSep ()) {
	    n2.append (sp.pre ());
	    n2.append (uUScore);
	}
	n2.append (sp.pre ());
    } else {
	n2 = name;
    }
    if (finline) {
	out->out_raw (CharConst (kRES_DISP ": " kINLINE));
    } else {
	out->out_raw (CharConst (kRES_DISP ": " kATTACHMENT));
    }
    if (n2.length () > 0) {
	out->out_raw (CharConst ("; filename=\""))->out_noCtrl (n2)->out_raw (uQ2);
    }
    out->out_raw (uLF);
    if (filesize > 0) {
	out->out_raw (CharConst (kHEAD_CONTENT_LENGTH ": "))->out_raw (to_ustring (filesize))->out_raw (uLF);
    }
}

void  HTTPResponse::location (MotorOutput* out, const ustring& url, MotorEnv* env) {
    out->out_raw (CharConst ("Status: 302 Moved Temporarily\n"));
    printCacheControl (out);
    out->out_raw (CharConst ("Location: "))
	->out_noCtrl (url)
	->out_raw (CharConst ("\n"
			      kRES_TYPE ": " kMIME_TEXT "; " kCHARSET kCODE_UTF8 "\n"));
    printCookie (out, env);
#ifdef NO_KEEPALIVE
    out->out_raw (CharConst (kRES_CONNECTION_CLOSE))->out_raw (uLF);
#endif
    out->out_raw (CharConst ("\n"
			     "Location: "))
	->out_noCtrl (url)
	->out_raw (CharConst ("\n"));
}

void  HTTPResponse::location_html (MotorOutput* out, const ustring& url, MotorEnv* env) {
    out->out_raw (CharConst (kRES_TYPE ": " kMIME_HTML "; " kCHARSET kCODE_UTF8 "\n"));
    printCacheControl (out);
    printCookie (out, env);
#ifdef NO_KEEPALIVE
    out->out_raw (CharConst (kRES_CONNECTION_CLOSE))->out_raw (uLF);
#endif
    out->out_raw (CharConst ("\n"
			     "<html>\n"
			     "<head>\n"
			     "<title></title>\n"
			     "<meta http-equiv=\"refresh\" content=\"0;URL="))
	->out_toHTML (url)
	->out_raw (CharConst ("\">\n"
			      "</head>\n"
			      "<body>\n"
			      "<script language=\"JavaScript\" type=\"text/javascript\">\n"
			      "location.href='"))
	->out_toJS (url)
	->out_raw (CharConst ("';\n"
			      "</script>\n"
			      "<noscript>go to <a href=\""))
	->out_toHTML (url)
	->out_raw (CharConst ("\">here</a>.</noscript>\n"
			      "</body>\n"
			      "</html>\n"));
}

void  HTTPResponse::noContentResponse (MotorOutput* out, MotorEnv* env) {
#ifdef STANDALONE
    if (env && env->documentRoot.length () == 0)
	return;
#endif
    out->out_raw (CharConst ("Status: 200 No Content\n"
			     kRES_TYPE ": " kMIME_TEXT "; " kCHARSET kCODE_UTF8 "\n"));
    printCacheControl (out);
    printCookie (out, env);
#ifdef NO_KEEPALIVE
    out->out_raw (CharConst (kRES_CONNECTION_CLOSE))->out_raw (uLF);
#endif
    out->out_raw (CharConst ("\n"
			     "No Content.\n"));
}

void  HTTPResponse::forbiddenResponse (MotorOutput* out, MotorEnv* env) {
#ifdef STANDALONE
//    if (env && env->documentRoot.length () == 0)
//	return;
#endif
    out->out_raw (CharConst ("Status: 403 Forbidden\n"
			     kRES_TYPE ": " kMIME_TEXT "; " kCHARSET kCODE_UTF8 "\n"));
    printCacheControl (out);
    printCookie (out, env);
#ifdef NO_KEEPALIVE
    out->out_raw (CharConst (kRES_CONNECTION_CLOSE))->out_raw (uLF);
#endif
    out->out_raw (CharConst ("\n"
			     "Forbidden.\n"));
}

void  HTTPResponse::setHeader (const ustring& key, const ustring& val) {
    static char  table_httpheader[] = {		// [a-zA-Z_0-9-]
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 
	0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 
	0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 
    };

    if (! matchWordTbl (key.begin (), key.end (), table_httpheader))
        throw (key + ": bad header name.");
    if (! matchASCII (val.begin (), val.end ()))
        throw (val + ": bad header value.");
    moreheader.push_back (std::pair<ustring,ustring> (key, val));
}

void  HTTPResponse::printMoreHeader (MotorOutput* out, MotorEnv* env) {
    std::vector<std::pair<ustring, ustring> >::iterator  it;
    for (it = moreheader.begin (); it != moreheader.end (); ++ it) {
        out->out_raw ((*it).first)->out_raw (CharConst (": "))->out_raw ((*it).second)->out_raw (uLF);
#ifdef DEBUG
        if (env->log) {
            *env->log << (*it).first << ": " << (*it).second << "\n";
        }
#endif /* DEBUG */
    }
}

bool  HTTPSend::submit (TcpClient& client, TcpBuf& buf, MlEnv* mlenv) {
    bool  rc;
    ustring  q;

    if (status != HTTP_INIT)
	return false;

    status = HTTP_OPEN;
    q = query (mlenv);
    client.write (&*q.begin (), q.length ());
#ifdef DEBUG2
    std::cerr << q;
#endif /* DEBUG */
    switch (method) {
    case M_POST:
	if (formData) {
	    writeFileForm (&client, mlenv);
	} else if (rawquery.length () > 0) {
	    client.write (&*rawquery.begin (), rawquery.length ());
	} else if (rawqueryfile.length () > 0) {
	    writeFile (&client, rawqueryfile);
	} else {
	    writeQueryForm (params (), client);
	}
	break;
    case M_PUT:
	if (rawquery.length () > 0) {
	    client.write (&*rawquery.begin (), rawquery.length ());
	} else if (rawqueryfile.length () > 0) {
	    writeFile (&client, rawqueryfile);
	}
	break;
    default:;
    }
    client.flush_write ();
    status = HTTP_QUERY;
    
    return true;
}

int  HTTPSend::post (TcpClient& client, TcpBuf& buf, ustring& ans, MlEnv* mlenv) {
    if (submit (client, buf, mlenv)) {
	readReplyHead (client, buf);
	readReplyBody (client, buf, ans);
    }

    return responseCode;
}

void  HTTPSend::makeHeader (ustring& q, const ustring& key, const ustring& val) {
    static uregex  re1 ("[\\x00-\\x1f: ]");
    static uregex  re2 ("[\\x00-\\x1f]");

    if (! checkRe (key, re1) && ! checkRe (val, re2) && key.length () + val.length () < 2048) {
	q.append (key).append (CharConst (": ")).append (val).append (uCRLF);
    }
}

void  HTTPSend::makeCookieHeader (ustring& q) {
    ustring  c;
    std::vector<mapelem>::iterator  b, e;
    ssize_t  s = 0;

    b = cookie.begin ();
    e = cookie.end ();
    if (b < e) {
	q.append (CharConst ("Cookie: "));
	for (; b < e; b ++) {
	    if (s > 0) {
		if (s > 800) {
		    q.append (CharConst (";" kCRLF " "));
		} else {
		    q.append (CharConst ("; "));
		}
		s = 0;
	    }
	    s -= q.length ();
	    q.append (cookieencode (b->first)).append (uEq).append (cookieencode (b->second));
	    s += q.length ();
	}
	q.append (uCRLF);
    }
}

ustring  HTTPSend::query (MlEnv* mlenv) {
    ustring  q;

    switch (method) {
    case M_GET:
	q.assign (CharConst ("GET"));
	break;
    case M_HEAD:
	q.assign (CharConst ("HEAD"));
	break;
    case M_POST:
	q.assign (CharConst ("POST"));
	break;
    case M_OPTIONS:
	q.assign (CharConst ("OPTIONS"));
	break;
    case M_PUT:
	q.assign (CharConst ("PUT"));
	break;
    case M_DELETE:
	q.assign (CharConst ("DELETE"));
	break;
    case M_TRACE:
	q.assign (CharConst ("TRACE"));
	break;
    case M_PATCH:
	q.assign (CharConst ("PATCH"));
	break;
    case M_LINK:
	q.assign (CharConst ("LINK"));
	break;
    case M_UNLINK:
	q.assign (CharConst ("UNLINK"));
	break;
    default:
	assert (0);
    }
    q.append (uSPC).append (path);
    switch (method) {
    case M_POST:
	if (formData)
	    makeSeparator ();
	if (getparams ()) {
	    q.append (CharConst ("?"));
	    appendQueryForm (getparams (), q);
	}
	break;
    case M_PUT:
	if (getparams ()) {
	    q.append (CharConst ("?"));
	    appendQueryForm (getparams (), q);
	}
	break;
    default:
	if (getparams ()) {
	    q.append (CharConst ("?"));
	    appendQueryForm (getparams (), q);
	} else if (params ()) {
	    q.append (CharConst ("?"));
	    appendQueryForm (params (), q);
	}
    }
    q.append (CharConst (" HTTP/1.0" kCRLF));
    makeHeader (q, ustring (CharConst ("Host")), host.host);
    q.append (CharConst ("Accept: */*" kCRLF));
    makeCookieHeader (q);
    if (id.length () > 0) {
	ustring  idpw;
	idpw.assign (id).append (uColon).append (pw);
	q.append (CharConst ("Authorization: Basic ")).append (base64Encode (idpw.begin (), idpw.end ())).append (uCRLF);
    }
    if (useproxy && proxyid.length () > 0) {
	ustring idpw;
	idpw.assign (proxyid).append (uColon).append (proxypw);
	q.append (CharConst ("Proxy-Authorization: Basic ")).append (base64Encode (idpw.begin (), idpw.end ())).append (uCRLF);
    }
    for (std::vector<mapelem>::iterator i = header_req.begin (); i < header_req.end (); i ++) {
	makeHeader (q, (*i).first, (*i).second);
    }
    switch (method) {
    case M_POST:
	if (formData) {
	    q.append (CharConst ("Content-Length: ")).append (to_ustring (writeFileForm (NULL, mlenv))).append (uCRLF);
	    querytype.assign (CharConst ("multipart/form-data; boundary=")).append (separator);
	} else if (rawquery.length () > 0) {
	    q.append (CharConst ("Content-Length: ")).append (to_ustring (rawquery.length ())).append (uCRLF);
	} else if (rawqueryfile.length () > 0) {
	    off_t  s;
	    fileSize (rawqueryfile, s);
	    q.append (CharConst ("Content-Length: ")).append (to_ustring (s)).append (uCRLF);
	} else {
	    q.append (CharConst ("Content-Length: ")).append (to_ustring (queryFormSize (params ()))).append (uCRLF);
	}
	break;
    case M_PUT:
	if (rawquery.length () > 0) {
	    q.append (CharConst ("Content-Length: ")).append (to_ustring (rawquery.length ())).append (uCRLF);
	} else if (rawqueryfile.length () > 0) {
	    off_t  s;
	    fileSize (rawqueryfile, s);
	    q.append (CharConst ("Content-Length: ")).append (to_ustring (s)).append (uCRLF);
	}
	break;
    default:;
    }
    if (querytype.length () > 0)
	q.append (CharConst ("Content-Type: ")).append (querytype).append (uCRLF);
    q.append (CharConst ("Connection: close" kCRLF kCRLF));

    return q;
}

void  HTTPSend::writeQueryForm (MNode* e, TcpClient& out) {
    MNode*  a;
    int  c = 0;

    if (e && e->isCons ()) {
	while (e) {
	    if ((a = e->car ())) {
		ustring  u;
		if (c > 0)
		    u.append (uAmp);
		u.append (percentEncode (cv (to_string (a->car ()))));
		if (! isNil (a->cdr ())) {
		    u.append (uEq);
		    u.append (percentEncode (cv (to_string (a->cdr ()))));
		}
		out.write (&*u.begin (), u.length ());
		c ++;
	    }
	    nextNode (e);
	}
    }
}

void  HTTPSend::appendQueryForm (MNode* e, ustring& out) {
    MNode*  a;
    int  c = 0;

    if (e && e->isCons ()) {
	while (e) {
	    if ((a = e->car ())) {
		if (c > 0)
		    out.append (uAmp);
		out.append (percentEncode (cv (to_string (a->car ()))));
		if (! isNil (a->cdr ())) {
		    out.append (uEq);
		    out.append (percentEncode (cv (to_string (a->cdr ()))));
		}
		c ++;
	    }
	    nextNode (e);
	}
    }
}

size_t  HTTPSend::queryFormSize (MNode* e) {
    MNode*  a;
    int  c = 0;
    size_t  ans = 0;

    if (e && e->isCons ()) {
	while (e) {
	    if ((a = e->car ())) {
		if (c > 0)
		    ans += uAmp.length ();
		ans += percentEncode (cv (to_string (a->car ()))).length ();
		if (! isNil (a->cdr ())) {
		    ans += uEq.length ();
		    ans += percentEncode (cv (to_string (a->cdr ()))).length ();
		}
		c ++;
	    }
	    nextNode (e);
	}
    }
    return ans;
}

void  HTTPSend::makeSeparator () {
    separator.assign (CharConst ("--------")).append (randomKey ()).append (randomKey ());
}

size_t  HTTPSend::writeFileForm (TcpClient* out, MlEnv* mlenv) {
    size_t  ans = 0;
    MNode*  e;
    MNode*  a;
    ustring  u;
    ustring  name;
    ustring  val;
    ustring  path;

    // text data
    e = params ();
    if (e && e->isCons ()) {
	while (e) {
	    if ((a = e->car ())) {
		name = to_string (a->car ());
		val = to_string (a->cdr ());
		ans += writeFileFormPart_text (out, name, val);
	    }
	    nextNode (e);
	}
    }

    // files in the temporal store
    e = fileparams_store ();
    if (e && e->isCons ()) {
	while (e) {
	    if ((a = e->car ())) {
		name = to_string (a->car ());
		val = to_string (a->cdr ());
		path = mlenv->env->path_store_file (val);
		ans += writeFileFormPart (out, name, val, path);
	    }
	    nextNode (e);
	}
    }

    // files in the permanent store
    e = fileparams_storage ();
    if (e && e->isCons ()) {
	while (e) {
	    if ((a = e->car ())) {
		name = to_string (a->car ());
		val = to_string (a->cdr ());
		path = mlenv->env->path_storage_file (val);
		ans += writeFileFormPart (out, name, val, path);
	    }
	    nextNode (e);
	}
    }

    // files in the content directory
    e = fileparams_static ();
    if (e && e->isCons ()) {
	while (e) {
	    if ((a = e->car ())) {
		name = to_string (a->car ());
		val = to_string (a->cdr ());
		path = mlenv->env->path_static_file (val);
		if (path.size () == 0)
		    throw (val + uErrorBadFile);
		ans += writeFileFormPart (out, name, val, path);
	    }
	    nextNode (e);
	}
    }

    u.assign (CharConst ("--")).append (separator).append (CharConst ("--" kCRLF));
    if (out)
	out->write (&*u.begin (), u.length ());
    ans += u.length ();
#ifdef DEBUG2
    if (out)
	std::cerr << u;
#endif /* DEBUG */
    return ans;
}

size_t  HTTPSend::writeFileFormPart_text (TcpClient* out, const ustring& name, const ustring& val) {
    size_t  ans = 0;
    ustring  u;

    u.assign (CharConst ("--")).append (separator).append (uCRLF);
    u.append (CharConst ("Content-Disposition: form-data; name=" kQ2)).append (percentEncode (cv (name))).append (CharConst (kQ2 kCRLF kCRLF));
    u.append (cv (val)).append (uCRLF);
    if (out)
	out->write (&*u.begin (), u.length ());
    ans += u.length ();
#ifdef DEBUG2
    if (out)
	std::cerr << u;
#endif /* DEBUG */
    return ans;
}

size_t  HTTPSend::writeFileFormPart (TcpClient* out, const ustring& name, const ustring& filename, const ustring& path) {
    size_t  ans = 0;
    ustring  u;
    off_t  s;

    u.assign (CharConst ("--")).append (separator).append (uCRLF);
    u.append (CharConst ("Content-Disposition: form-data; name=" kQ2)).append (percentEncode (cv (name))).append (CharConst (kQ2 "; filename=" kQ2)).append (slashEncode (filename)).append (CharConst (kQ2 kCRLF));
    u.append (CharConst ("Content-Type: ")).append (mimetype (getExt (filename))).append (CharConst (kCRLF kCRLF));
    if (out)
	out->write (&*u.begin (), u.length ());
    ans += u.length ();
#ifdef DEBUG2
    if (out)
	std::cerr << u;
#endif /* DEBUG */
    if (out) {
	if (path.length () > 0)
	    ans += writeFile (out, path);
	out->write (CharConst (kCRLF)); // XXX
	ans += 2;
    } else {
	if (path.length () > 0)
	    if (fileSize (path, s))
		ans += s;
	ans += 2;
    }
#ifdef DEBUG2
    if (out)
	std::cerr << uCRLF;
#endif /* DEBUG */
    return ans;
}

size_t  HTTPSend::writeFile (TcpClient* out, const ustring& path) {
    size_t  ans = 0;

    if (path.size () > 0) {
	FileMacro  f;
	off_t  s;
	ssize_t  n;
	char  buf [65536];

	if (f.openRead (path.c_str ())) {
	    s = f.size ();
	    ans = s;
	    while (s > 0) {
		n = s < 65536 ? s : 65536;
		n = f.read (buf, n);
		if (n < 0)
		    break;
		out->write (buf, n);
#ifdef DEBUG2
		std::cerr << "--data--\n";
#endif /* DEBUG */
		s -= n;
	    }
	    f.close ();
	}
    }
    return ans;
}

int  HTTPSend::readReplyHead (TcpClient& client, TcpBuf& buf) {
    ustring  line;
    uiterator  b, e, m1, m2;

    if (status != HTTP_QUERY)
	return 0;

    if (! buf.getln (client, line)) {
	return 0;
    }
    if (! match (line.substr (0, 5), CharConst ("HTTP/"))) {
	return false;		// bad protocol
    } else {
	ustring::size_type  p1 = line.find (' ', 0);
	ustring::size_type  p2 = line.find (' ', p1 + 1);
	responseCode = strtoul (line.substr (p1 + 1, p2));
    }
    while (buf.getln (client, line)) {
	if (line.empty ())
	    break;
	// ヘッダ内の制御文字は削除されるが，ヘッダ名の大小文字は保存される。
	line = omitCtrl (fixUTF8 (line));
	b = line.begin ();
	e = line.end ();
	if (splitChar (b, e, ':', m1)) {
	    ustring  s;
	    m2 = m1 + 1;
	    skipSpace (m2, e);
	    s = ustring (b, m1);
	    header_reply.push_back (mapelem (s, ustring (m2, e)));
	    s = toLower (s);
	    if (match (s, CharConst ("set-cookie"))) {
		readCookie (m2, e);
	    }
	}
    }
    status = HTTP_HEAD;

    return  responseCode;
}

void  HTTPSend::readReplyBody (TcpClient& client, TcpBuf& buf, MotorOutput* out) {
    if (status != HTTP_HEAD)
	return;

    out->out_raw (buf.start, buf.tail);
    buf.consume ();
    while (buf.fill (client)) {
	out->out_raw (buf.start, buf.tail);
	buf.consume ();
    }
    status = HTTP_DONE;
}

void  HTTPSend::readReplyBody (TcpClient& client, TcpBuf& buf, ustring& ans) {
    std::stringstream  ostr;
    MotorOutputOStream  out (&ostr);
    readReplyBody (client, buf, &out);
    ans = fixUTF8 (rcv (ostr.str ()));
}

const ustring*  HTTPSend::findHeader (const ustring& name) { // nameは小文字に変換すること
    std::vector<mapelem>::const_iterator  b = header_reply.begin ();
    std::vector<mapelem>::const_iterator  e = header_reply.end ();

    for (; b < e; b ++) {
	if (toLower (b->first) == name) {
	    return &b->second;
	}
    }
    return NULL;
}

void  HTTPSend::readCookie (uiterator b, uiterator e) {
    std::vector<ustring>  ary;
    uiterator  ib, ie, im;
    static uregex  re ("; *");

    split (b, e, re, ary);
    if (ary.size () > 0) {
	CookieInfo  ci;

	ib = ary[0].begin ();
	ie = ary[0].end ();
	if (splitChar (ib, ie, '=', im)) {
	    ci.key.assign (ib, im);
	    ci.value.assign (im + 1, ie);
	} else {
	    ci.key.assign (ib, ie);
	}
	cookie_reply.push_back (ci);
    }
}

ustring  HTTPSend::cv (const ustring& text) {
    return text;
}

ustring  HTTPSend::rcv (const ustring& text) {
    return text;
}

