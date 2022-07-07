#ifndef MOTORENV_H
#define MOTORENV_H

#include "app.h"
#include "motorvar.h"
#include "ml-defun.h"
#include "http.h"
#include "ustring.h"
#include "util_time.h"
#include <iostream>

class  FTable;
class  CGIFormFile;
class  HTMLMotor;
class  MotorOutput;
class  MLFunc;
class  MNode;
class  MlPool;
class  MlFTable;
class  MlEnv;
class  WikiEnv;
class  WikiMotorOutputString;
class  MotorEnv {
 public:
    AppEnv*  appenv;
    CGIFormFile*  form;
    HTMLMotor*  motor;
    MotorOutput*  output;
    WikiMotorOutputString*  wikioutput;
    HTTPResponse  http;

    ustring  datastore;
    ustring  htmlFile;
    ustring  errorHtmlFile;
    ustring  mimetype;
    MlEnv*  mlenv;
    WikiEnv*  wikienv;
    MlPool*  mlPool;
    MlFTable*  mlFTable;
    MotorErrorVar  errorvar;
    ustring  storedir;
    ustring  storagedir;
    MotorVar  motorCall;
    bool  errflag;
    bool  responseDone;
    std::ostream*  log;
    ustring*  currentPath;
    ustring  scriptName;
    ustring  documentRoot;
    ustring  scriptFilename;

    MotorEnv (AppEnv* ae, CGIFormFile* fm, HTMLMotor* m, MotorOutput* out);
    virtual  ~MotorEnv ();

    virtual void  setErrorFlag ();
    virtual void  setErrorVar (const ustring& name);
    virtual bool  getErrorVar (const ustring& name);
    virtual void  motorItem (const ustring& name);
    virtual void  motorText (const ustring& text);

    virtual ustring  search_resource_file (const ustring& name);
    virtual ustring  path_to_auth (const ustring& name);
    virtual ustring  path_to_etc (const ustring& name);
    virtual ustring  path_to_db ();
    virtual ustring  path_db (const ustring& name, const char* suffix);
    virtual ustring  path_store_file (const ustring& name, const char* suffix = NULL);
    virtual ustring  path_storage_file (const ustring& name, const char* suffix = NULL);
    virtual ustring  path_static_file (const ustring& name);
    virtual ustring  path_to_posttemp ();
    virtual ustring  path_to_store ();
    virtual ustring  path_to_store_index ();
    virtual ustring  path_to_storage ();
    virtual ustring  path_to_storage_index ();

    virtual void  setDefault ();
    virtual void  setDefaultDatastore ();
    virtual void  setDatastore (const ustring& name);
    virtual void  setFrameOpt ();
    virtual void  readFormVar ();
    virtual void  cacheControl ();
    virtual void  doML ();
    virtual void  doML (const ustring& file);
    virtual void  doMotor ();
    virtual void  doMotor (const ustring& file) {
	doMotor (file, mimetype);
    };
    virtual void  doMotor (const ustring& file, const ustring& type, MotorOutput* out = NULL);
    virtual void  doMotorText (const ustring& text) {
	doMotor (text, mimetype);
    }
    virtual void  doMotorText (const ustring& text, const ustring& type, MotorOutput* out = NULL);
    virtual void  doMotorFile (const ustring& file, bool skipHead, const ustring& type, MotorOutput* out = NULL);
    virtual void  outputFile (const ustring& src, const ustring& type, bool base64 = false);
    virtual void  outputFile (const ustring& src, const ustring& type, bool finline, const ustring& dispname, bool base64 = false);
    virtual void  standardResponse () {
	standardResponse (mimetype);
    };
    virtual void  standardResponse (const ustring& type);
    virtual void  standardResponse (const ustring& type, const ustring& charset, const ustring& dispname, bool finline, off_t filesize = 0);
    virtual void  standardResponse_html ();
    virtual void  noContentResponse ();
    virtual void  forbiddenResponse ();
    virtual void  location (const ustring& url);
    virtual void  location_html (const ustring& url);
    virtual void  logMessage (const ustring& msg);
};

#endif /* MOTORENV_H */
