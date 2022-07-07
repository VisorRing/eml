#ifndef APP_H
#define APP_H

#include "config.h"
#include "httpconst.h"
#include "ustring.h"
#include <iostream>

class  MotorEnv;
class  AppEnv {
 public:
    ustring  arg0;
    ustring  datastore;
    ustring  getHtml;
    ustring  postHtml;
    ustring  postFileHtml;
    ustring  errorHtml;
    ustring  getML;
    ustring  postML;
    ustring  postFileML;
    size_t  postlimit;
    size_t  postfilelimit;
    ustring  mimetype;
    ustring  ocode;
    enum {
	CC_NONE,
	CC_COOKIE,
	CC_URL,
	CC_NOCACHE,
    }  cacheControl;
    enum {
	FOPT_NONE,
	FOPT_DENY,
	FOPT_SAMEORIGIN,
	FOPT_ALLOWFROM		/* ブラウザに実装されていない */
    }  frameOpt;
    ustring  foptUri;
    bool  debugDump;
    bool  noTrace;

    AppEnv () {
	cacheControl = CC_NONE;
	postlimit = cPOSTLIMITDEFAULT;
	postfilelimit = cPOSTFILELIMITDEFAULT;
	frameOpt = FOPT_NONE;
	debugDump = false;
	noTrace = false;
    };
    virtual  ~AppEnv () {};

    virtual void  readOption (int argc, char** argv, MotorEnv* env);
    virtual void  setDefault ();
//    virtual ustring  scriptName ();
    virtual void  dump (std::ostream& out);
    virtual void  setErrorLog (const ustring& path, bool fappend = false);
};

#endif /* APP_H */
