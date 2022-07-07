#ifndef HTTP_H
#define HTTP_H

#include "ml.h"
#include "util_tcp.h"
#include "ustring.h"
#include <boost/unordered_map.hpp>
#include <iostream>
#include <vector>
#include <ostream>
#include <time.h>

class  MotorOutput;
class  MotorEnv;
class  HTTPResponse {
 public:
    std::vector<ustring>  setcookie;
    bool  cookieDone;
    ustring  cookie;
    boost::unordered_map<ustring, ustring>  cookiemap;
    bool  fNoCache;
    std::vector<std::pair<ustring, ustring> >  moreheader;
    enum {
	FOPT_NONE,
	FOPT_DENY,
	FOPT_SAMEORIGIN,
	FOPT_ALLOWFROM		/* ブラウザに実装されていない */
    }  frameOpt;
    ustring  foptUri;		/*  */

    HTTPResponse (): cookieDone (false), fNoCache (false), frameOpt (FOPT_NONE) {};
    virtual  ~HTTPResponse () {};
    virtual void  printCacheControl (MotorOutput* out);
    virtual void  printCookie (MotorOutput* out, MotorEnv* env);
    virtual void  setCookie (const ustring& key, const ustring& val, const ustring& path, time_t span, time_t limit, const ustring& domain, bool fsecure, MotorEnv* env);
    virtual void  setCookiePair (uiterator b, const uiterator e);
    virtual void  parseCookie ();
    virtual ustring  readCookie (const ustring& key);
    virtual void  setRandomCookie (MotorEnv* env);
    virtual void  standardResponse (MotorOutput* out, const ustring& type, const ustring& charset, MotorEnv* env);
    virtual void  standardResponse_html (MotorOutput* out, MotorEnv* env);
    virtual void  disposition (MotorOutput* out, bool finline, const ustring& name, off_t filesize);
    virtual void  location (MotorOutput* out, const ustring& url, MotorEnv* env);
    virtual void  location_html (MotorOutput* out, const ustring& url, MotorEnv* env);
    virtual void  noContentResponse (MotorOutput* out, MotorEnv* env);
    virtual void  forbiddenResponse (MotorOutput* out, MotorEnv* env);
    virtual void  setHeader (const ustring& key, const ustring& val);
    virtual void  printMoreHeader (MotorOutput* out, MotorEnv* env);
};

class  CookieInfo {
 public:
    ustring  key;
    ustring  value;
    ustring  path;
    time_t  span;
    ustring  domain;
    bool  fhttps;

    CookieInfo (): span (0), fhttps (false) {};
    CookieInfo (const ustring& _key, const ustring& _value): key (_key), value (_value), span (0), fhttps (false) {};
    CookieInfo (const ustring& _key, const ustring& _value, const ustring& _path, time_t _span, const ustring& _domain, bool _fhttps): key (_key), value (_value), path (_path), span (_span), domain (_domain), fhttps (_fhttps) {};
    virtual  ~CookieInfo () {};
};

#if 0
class  HTTPSend;
class  HTTPQuery {
 public:
    MNodePtr  params;

    HTTPQuery () {};
    virtual  ~HTTPQuery () {};

    virtual void  assign (MNode* p) {
	params = p;
    };
    virtual ustring  formString (HTTPSend* o = NULL);
    virtual void  writeForm (TcpClient& out, HTTPSend* o = NULL);
    virtual void  formSize (HTTPSend* o = NULL);
};
#endif

class  HTTPSend {
 public:
    typedef  std::pair<ustring,ustring>  mapelem;

    enum {
	HTTP_INIT,
	HTTP_OPEN,
	HTTP_QUERY,
	HTTP_HEAD,
	HTTP_DONE,
    }  status;
//    bool  fpost;
//    ustring  method;
    enum method_t {
	M_GET, M_HEAD, M_POST, M_OPTIONS, M_PUT, M_DELETE,
	M_TRACE, M_PATCH, M_LINK, M_UNLINK
    }  method;
    ustring  proto;
    HostSpec  host;
//    ustring  reqhost;
    HostSpec  conhost;
    ustring  path;
    MNodePtr  params;
    MNodePtr  getparams;
//    HTTPQuery  params;
//    HTTPQuery  getparams;
    ustring  rawquery;
    ustring  rawqueryfile;
    ustring  querytype;
    MNodePtr  fileparams_store;
    MNodePtr  fileparams_storage;
    MNodePtr  fileparams_static;
    size_t  paramsLen;
    ustring  separator;
    bool  formData;
    ustring  id;
    ustring  pw;
    std::vector<mapelem>  cookie;
    bool  useproxy;
    ustring  proxyid;
    ustring  proxypw;
    std::vector<mapelem>  header_req;
    std::vector<mapelem>  header_reply;
    std::vector<CookieInfo>  cookie_reply;
    int  responseCode;

    HTTPSend () {
	method = M_GET;
	status = HTTP_INIT;
//	fpost = false;
	paramsLen = 0;
	responseCode = 0;
	useproxy = false;
	formData = false;
    };
    virtual  ~HTTPSend () {};
//    virtual void  setMethod (const ustring& _method, bool fpost);
    virtual void  setMethod (method_t _method) {
	method = _method;
//	if (method == M_POST)
//	    fpost = true;
//	else
//	    fpost = false;
    };
    virtual void  setPostFileMethod () {
	method = M_POST;
//	fpost = true;
	formData = true;
    };
    virtual bool  submit (TcpClient& client, TcpBuf& buf, MlEnv* mlenv);
    virtual int  post (TcpClient& client, TcpBuf& buf, ustring& ans, MlEnv* mlenv);

    virtual void  makeHeader (ustring& q, const ustring& key, const ustring& val);
    virtual void  makeCookieHeader (ustring& q);
    virtual void  writeQueryForm (MNode* e, TcpClient& out);
    virtual void  appendQueryForm (MNode* e, ustring& out);
    virtual size_t  queryFormSize (MNode* e);
    virtual void  makeSeparator ();
    virtual size_t  writeFileForm (TcpClient* out, MlEnv* mlenv);
    virtual size_t  writeFileFormPart_text (TcpClient* out, const ustring& name, const ustring& val);
    virtual size_t  writeFileFormPart (TcpClient* out, const ustring& name, const ustring& filename, const ustring& path);
    virtual size_t  writeFile (TcpClient* out, const ustring& path);
    virtual ustring  query (MlEnv* mlenv);
    virtual int  readReplyHead (TcpClient& client, TcpBuf& buf);
    virtual void  readReplyBody (TcpClient& client, TcpBuf& buf, MotorOutput* out);
    virtual void  readReplyBody (TcpClient& client, TcpBuf& buf, ustring& ans);
    virtual const ustring*  findHeader (const ustring& name);
 private:
    virtual void  readCookie (uiterator b, uiterator e);
    virtual ustring  cv (const ustring& text);
    virtual ustring  rcv (const ustring& text);
};

#endif /* HTTP_H */
