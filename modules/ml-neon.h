#ifndef ML_NEON_H
#define ML_NEON_H

#include "ml.h"
#include "ml-id.h"
#include "ml-store.h"
#include "http-iconv.h"
#include "util_string.h"
#include "filemacro.h"
#include <boost/scoped_ptr.hpp>
#include <neon/ne_session.h>
#include <neon/ne_request.h>
#include <vector>

class  MNode;
class  MlEnv;
class  NeonQuery;

class  NeonPostBodyProvider {
 public:
    NeonQuery*  query;
    std::vector<std::pair<ustring, ustring> >  postFile;
    size_t  paramsLen;
    ustring  separator;
    enum {
	S_NONE,
	S_TEXT,
	S_FILEHEAD,
	S_FILEBODY,
	S_FILETERM,
	S_TAIL,
	S_DONE,
    }  state;
    MNode*  tp;
    std::vector<std::pair<ustring, ustring> >::iterator  bp;
    std::vector<std::pair<ustring, ustring> >::iterator  ep;
    ustring  ubuf;
    off_t  offset;
    FileMacro  fd;

    NeonPostBodyProvider (NeonQuery* _query) {
	query = _query;
	paramsLen = 0;
	makeSeparator ();
	state = S_NONE;
	tp = NULL;
	offset = 0;
    };
    virtual  ~NeonPostBodyProvider () {};

    void  pushFile (const ustring& name, const ustring& path);
    void  makeSeparator ();
    ustring  separatorLine ();
    ustring  separatorHeader ();
    ustring  textSeparator (const ustring& name);
    ustring  fileSeparator (const ustring& name, const ustring& filename);
    ustring  tailSeparator ();
    ne_off_t  calcLength ();
    char*  bodyProvider (char* buffer, size_t buflen);
 private:
    ssize_t  bodyProviderText (char* buffer, size_t buflen);
};

class  NeonSession {
 public:
    typedef enum {
	PROTO_NONE,
	PROTO_HTTP,
	PROTO_HTTPS,
    }  proto_t;

    ne_session*  session;
    proto_t  proto;

    NeonSession (proto_t _proto, const ustring& host, unsigned int port);
    virtual  ~NeonSession ();
    
    virtual ne_session&  operator * () const {
	return *session;
    };
    virtual ne_session*  operator -> () const {
	return session;
    };
    virtual ne_session*  get () const {
	return session;
    };
    virtual void  setNoVerify ();
    virtual void  setProxy (const ustring& host, int port);
};

class  NeonQuery {
 public:
    typedef enum {
	METHOD_NONE,
	METHOD_GET,
	METHOD_POST,
	METHOD_PUT,
	METHOD_DELETE,
	METHOD_HEAD,
	METHOD_PATCH,
	METHOD_FILE,		/* pseudo method */
    }  method_t;

    ne_request*  req;
    ne_session*  session;
    method_t  method;
    ustring  path;
    bool  rawPath;
    ustring  basicID;
    ustring  basicPW;
    MNodePtr  queryParam;
    MNodePtr  getQueryParam;
    StoreType  rawQuery;
    StoreType  rawGetQuery;
    ustring  querytype;		/* リクエストヘッダのcontent-type */
    MNodePtr  cookie;
    MNodePtr  header;
    boost::scoped_ptr<NeonPostBodyProvider>  qbody;
    boost::scoped_ptr<UIConv>  cv_in;
    boost::scoped_ptr<UIConv>  cv_out;
    enum {
	MODE_SETUP,
	MODE_DISPATCHED,
	MODE_RECEIVED,
    }  mode;
    ustring  errorMsg;
    std::vector<CookieInfo>  replyCookie;
    bool  replyCookieDone;

    NeonQuery (ne_session* _session, MlEnv* _mlenv): rawQuery (_mlenv), rawGetQuery (_mlenv) {
	session = _session;
	req = NULL;
	method = METHOD_GET;
	rawPath = false;
	mode = MODE_SETUP;
	replyCookieDone = false;
	qbody.reset (new NeonPostBodyProvider (this));
    };
    virtual  ~NeonQuery () {
	closeReq ();
    };

    virtual void  closeReq ();
    virtual void  setIConv (const char* name);
    virtual void  submit ();
    virtual void  readBody (MotorOutput* out);
    virtual int  getStatus ();
    virtual ustring  cv (const ustring& src);
    virtual ustring  rcv (const ustring& src);
    virtual ustring  getResponseHeader (const char* name);
    virtual MNode*  getResponseHeaderAll ();
    virtual ustring  getResponseCookie (const ustring& name);
    virtual MNode*  getResponseCookieAll ();
    virtual const char*  methodStr ();
 private:
    virtual ustring  buildGetQuery ();
    virtual void  buildQuery (MNode* e, ustring& out);
    virtual ustring  buildMimeSeparator_text (const ustring& name);
    virtual ustring  buildMimeSeparator_file (const ustring& name, const ustring& filename);
    virtual ustring  filterPath ();
    virtual void  buildCookie ();
    virtual void  buildHeader ();
    virtual void  buildBasicAuthHeader ();
    virtual void  setFormType ();
    virtual void  setFormType_urlencoded ();
    virtual void  setFormType_formdata ();
    virtual void  parseCookie ();
    virtual void  setRawQuery (FileMacro& fd);
};

class  MLNeon: public MLFunc {
 public:
    ustring  host;
    int  port;
    NeonSession::proto_t  proto;
    ustring  proxyhost;
    int  proxyport;
    ustring  proxyid;
    ustring  proxypw;
    bool  fnoverify;
    boost::scoped_ptr<NeonSession>  session;
    boost::scoped_ptr<NeonQuery>  query;

    MLNeon (MlEnv* _mlenv): MLFunc (cMLNeonID, _mlenv) {
	proto = NeonSession::PROTO_HTTP;
	proxyport = 0;
	fnoverify = false;
    };
    virtual  ~MLNeon () {};

    virtual void  newSession (MlEnv* mlenv);
};

inline void  assignNLNeon (MLNeon*& var, MNode* e) {
    if (e && e->type == MNode::MC_LIBOBJ && e->value_libobj () && e->value_libobj ()->id == cMLNeonID)
	var = (MLNeon*)e->value_libobj ();
}

extern void  NeonInitProc ();
MNode*  ml_neon (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_neon_http_request (bool fev, MNode* cell, MlEnv* mlenv, MLFunc* mobj);
MNode*  ml_neon_http_status (bool fev, MNode* cell, MlEnv* mlenv, MLFunc* mobj);
MNode*  ml_neon_http_response (bool fev, MNode* cell, MlEnv* mlenv, MLFunc* mobj);
MNode*  ml_neon_http_response_file (bool fev, MNode* cell, MlEnv* mlenv, MLFunc* mobj);
MNode*  ml_neon_http_response_output (bool fev, MNode* cell, MlEnv* mlenv, MLFunc* mobj);
MNode*  ml_neon_http_get_cookie (bool fev, MNode* cell, MlEnv* mlenv, MLFunc* mobj);
MNode*  ml_neon_http_get_all_cookies (bool fev, MNode* cell, MlEnv* mlenv, MLFunc* mobj);
MNode*  ml_neon_http_content_type (bool fev, MNode* cell, MlEnv* mlenv, MLFunc* mobj);
MNode*  ml_neon_http_get_header (bool fev, MNode* cell, MlEnv* mlenv, MLFunc* mobj);
MNode*  ml_neon_http_get_all_headers (bool fev, MNode* cell, MlEnv* mlenv, MLFunc* mobj);

#endif /* ML_NEON_H */
