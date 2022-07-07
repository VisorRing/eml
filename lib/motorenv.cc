#include "config.h"
#include "motorconst.h"
#include "motor.h"
#include "motorenv.h"
#include "wikienv.h"
#include "mlenv.h"
#include "formfile.h"
#include "ml-defun.h"
#include "expr.h"
#include "utf8.h"
#include "util_check.h"
#include "util_const.h"
#include "util_file.h"
#include "util_string.h"
#include "filemacro.h"
#include "ustring.h"
#include <stdlib.h>
#include <unistd.h>

//MotorVar  GDeTable;

MotorEnv::MotorEnv (AppEnv* ae, CGIFormFile* fm, HTMLMotor* m, MotorOutput* out) {
    appenv = ae;
    form = fm;
    motor = m;
    output = out;
    wikioutput = NULL;
    log = &std::cerr;
//    setRequestEnv ();
    mlPool = new MlPool;
    mlFTable = new MlFTable;
    mlenv = new MlEnv (mlPool, mlFTable);
    mlenv->env = this;
    mlenv->log = log;
    mlenv->setStartTime ();
    mlenv->resetProg ();
    mlFTable->setFTable (&GFTable, &GMTable);
    wikienv = new WikiEnv;
    errflag = false;
    responseDone = false;
    currentPath = NULL;
    scriptName = getenvString (kSCRIPT_NAME);
    documentRoot = getenvString (kDOCUMENT_ROOT);
    scriptFilename = getenvString (kSCRIPT_FILENAME);
}

MotorEnv::~MotorEnv () {
    delete mlenv;
    delete wikienv;
    delete mlPool;
    delete mlFTable;
}

#if 0
void  MotorEnv::setRequestEnv () {
    scriptName = getenvString (kSCRIPT_NAME);
}
#endif

void  MotorEnv::setErrorFlag () {
//    mlenv->breakProg ();
    errflag = true;
}

void  MotorEnv::setErrorVar (const ustring& name) {
    errflag = true;
    if (mlenv->validName (name)) {
	errorvar.setVar (name);
#ifdef DEBUG
	if (log) {
	    *log << "	[!" << name << "]\n";
	}
#endif /* DEBUG */
    }
}

bool  MotorEnv::getErrorVar (const ustring& name) {
    if (mlenv->validName (name))
	return errorvar.getVar (name);
    return false;
}

void  MotorEnv::motorItem (const ustring& name) {
    MotorObj::MotorObjVec*  itemobj;

    if (motor) {
	itemobj = motor->getTemplate (name);
	if (itemobj)
	    motor->output (*itemobj, output, this);
    }
}

void  MotorEnv::motorText (const ustring& text) {
    motor->outputTemp (text, output, this);
}

ustring  MotorEnv::search_resource_file (const ustring& name) {
    ustring  ans;
    ustring  top;

#ifdef cDocTop
    if (isHTTPS ()) {
	top.assign (CharConst (cSDocTop));
    } else {
	top.assign (CharConst (cDocTop));
    }
#else
    top = documentRoot;
#endif
    
    if (isAbsolutePath (name)) {
	/* absolute path */
	if (top.length () > 0) {
	    ans = top + name;
	    shapePath (ans);
	    if (matchHead (ans, top) && matchAbsoluteResourceName (ans) && isPlainFile (ans)) {
		return ans;
	    } else {
		return uEmpty;
	    }
	} else {
#ifdef STANDALONE
	    // 環境変数 DOCUMENT_ROOTなし
	    return name;
#else
	    return uEmpty;
#endif
	}
    } else {
	/* name is a relative path. */
	if (currentPath == NULL) {
	    ans = scriptFilename;
	} else {
	    ans = *currentPath;
	}
#ifdef DEBUG
//	std::cerr << "currentPath:" << ans << "\n";
#endif /* DEBUG */
	if (ans.length () > 0) {
	    ans = dirPart (ans) + uSlash + name;
	    shapePath (ans);
#ifdef DEBUG
//	    std::cerr << "ans:" << ans << "\n";
#endif /* DEBUG */
	    if (matchHead (ans, top) && matchAbsoluteResourceName (ans) && isPlainFile (ans)) {
		return ans;
#ifdef STANDALONE
	    } else if (isPlainFile (ans)) {
		return ans;
#endif
	    }
	} else {			// offline execution.
#ifdef STANDALONE
	    ans = name;
	    shapePath (ans);
	    if (matchResourceName (ans) && isPlainFile (ans)) {
		return ans;
	    }
#else
	    return uEmpty;
#endif
	}

	// リソースディレクトリ指定があればサーチする
	ans = getenvString (kSearchName);
#ifdef DEBUG
//	std::cerr << "searchName:" << ans << "\n";
#endif /* DEBUG */
	if (documentRoot.length () > 0 && ans.length () > 0) {
	    ans = documentRoot + uSlash + ans + uSlash + name;
	    shapePath (ans);
	    if (matchHead (ans, top) && matchAbsoluteResourceName (ans) && isPlainFile (ans)) {
		return ans;
	    }
	}
    }

    return uEmpty;
}

ustring  MotorEnv::path_to_auth (const ustring& name) {
    ustring  ans;

#ifdef SHARE_AUTHDB
    ans.reserve (128);
    ans.append (CharConst (cDataTop kSubAuth));
    ans.append (name);
#else
    ans.reserve (128);
    if (datastore.length () > 0) {
	ans.append (CharConst (cDataTop kDS));
	ans.append (datastore);
	ans.append (CharConst (kSubAuth));
    } else {
	throw (uErrorMissingDatastore);
    }
    ans.append (name);
#endif

    return ans;
}

ustring  MotorEnv::path_to_etc (const ustring& name) {
    ustring  ans;

    ans.append (CharConst (cDataTop kEtc));
    ans.append (name);
    return ans;
}

ustring  MotorEnv::path_to_db () {
    if (datastore.length () == 0)
	throw (uErrorMissingDatastore);
    return ustring (CharConst (cDataTop kDS)) + datastore + ustring (CharConst (kSubDB));
}

ustring  MotorEnv::path_db (const ustring& name, const char* suffix) {
    ustring  ans;
    static uregex  re ("^[a-zA-Z][a-zA-Z0-9_\\-]*$");
    umatch  m;

    assert (suffix);
    ans.reserve (128);
    if (datastore.length () > 0) {
	if (usearch (name, m, re)) {
	    ans.append (CharConst (cDataTop kDS));
	    ans.append (datastore);
	    ans.append (CharConst (kSubDB));
	    ans.append (name);
	    ans.append (suffix);
	} else {
	    throw (padEmpty (name) + ustring (CharConst (": bad datafile name.")));
	}
    } else {
	throw (uErrorMissingDatastore);
    }
    return ans;
}

ustring  MotorEnv::path_store_file (const ustring& name, const char* suffix) {
    if (storedir.empty ())
	throw (uErrorNoStore);
    if (suffix)
	return storedir + filenameEncode (name) + suffix;
    else
	return storedir + filenameEncode (name);
}

ustring  MotorEnv::path_storage_file (const ustring& name, const char* suffix) {
    if (storagedir.empty ())
	throw (uErrorNoStorage);
    if (suffix)
	return storagedir + filenameEncode (name) + suffix;
    else
	return storagedir + filenameEncode (name);
}

ustring  MotorEnv::path_static_file (const ustring& name) {
    ustring  ans;
    ustring  top;
#ifdef cDocTop
    if (isHTTPS ()) {
	top.assign (CharConst (cSDocTop));
    } else {
	top.assign (CharConst (cDocTop));
    }
#else
    top = documentRoot;
#endif
    if (isAbsolutePath (name)) {
	/* absolute path */
	if (top.length () > 0) {
	    ans = top + name;
	    shapePath (ans);
	    if (matchHead (ans, top) && matchAbsoluteResourceName (ans)) {
		return ans;
	    } else {
		return uEmpty;
	    }
	} else {
#ifdef STANDALONE
	    // 環境変数 DOCUMENT_ROOTなし
	    return name;
#else
	    return uEmpty;
#endif
	}
    } else {
	/* name is a relative path. */
	if (currentPath == NULL) {
	    ans = scriptFilename;
	} else {
	    ans = *currentPath;
	}
	if (ans.length () > 0) {
	    ans = dirPart (ans) + uSlash + name;
	    shapePath (ans);
#ifdef DEBUG
//	    std::cerr << "ans:" << ans << "\n";
#endif /* DEBUG */
	    if (matchHead (ans, top) && matchAbsoluteResourceName (ans)) {
		return ans;
#ifdef STANDALONE
	    } else {
		return ans;
#endif
	    }
	} else {			// offline execution.
#ifdef STANDALONE
	    ans = name;
	    shapePath (ans);
	    if (matchResourceName (ans)) {
		return ans;
	    }
#else
	    return uEmpty;
#endif
	}
    }
    return uEmpty;
}

ustring  MotorEnv::path_to_posttemp () {
    ustring  ans;

    ans.reserve (64);
    ans.append (CharConst (cDataTop kSubTemp "post-"));
    ans.append (to_ustring (getpid ()));
    return ans;
}

ustring  MotorEnv::path_to_store () {
    if (datastore.length () == 0)
	throw (uErrorMissingDatastore);
    return ustring (CharConst (cDataTop kDS)) + datastore + ustring (CharConst (kSubStore));
}

ustring  MotorEnv::path_to_store_index () {
    if (datastore.length () == 0)
	throw (uErrorMissingDatastore);
    return ustring (CharConst (cDataTop kDS)) + datastore + ustring (CharConst (kSubStore kStoreSerial));
}

ustring  MotorEnv::path_to_storage () {
    if (datastore.length () == 0)
	throw (uErrorMissingDatastore);
    return ustring (CharConst (cDataTop kDS)) + datastore + ustring (CharConst (kSubStorage));
}

ustring  MotorEnv::path_to_storage_index () {
    if (datastore.length () == 0)
	throw (uErrorMissingDatastore);
    return ustring (CharConst (cDataTop kDS)) + datastore + ustring (CharConst (kSubStorage kStoreSerial));
}

void  MotorEnv::setDefault () {
    datastore = appenv->datastore;
    switch (form->method) {
    case CGIForm::M_GET:
	htmlFile = appenv->getHtml;
	break;
    case CGIForm::M_POST:
	if (form->contentType == CGIForm::T_MULTIPART) {
	    htmlFile = appenv->postFileHtml;
	} else {
	    htmlFile = appenv->postHtml;
	}
	break;
    case CGIForm::M_NONE:
	htmlFile = appenv->getHtml;
	break;
    default:;
    }
    errorHtmlFile = appenv->errorHtml;
    mimetype = appenv->mimetype;
    setFrameOpt ();
}

void  MotorEnv::setDefaultDatastore () {
    datastore = appenv->datastore;
}

void  MotorEnv::setDatastore (const ustring& name) {
    if (name.size () == 0) {
	datastore = appenv->datastore;
    } else {
	if (! matchName (name))
	    throw (name + uErrorBadDatastore);
	datastore = name;
    }
}

void  MotorEnv::setFrameOpt () {
    switch (appenv->frameOpt) {
    case AppEnv::FOPT_NONE:
	http.frameOpt = HTTPResponse::FOPT_NONE;
	break;
    case AppEnv::FOPT_DENY:
	http.frameOpt = HTTPResponse::FOPT_DENY;
	break;
    case AppEnv::FOPT_SAMEORIGIN:
	http.frameOpt = HTTPResponse::FOPT_SAMEORIGIN;
	break;
    case AppEnv::FOPT_ALLOWFROM:
	http.frameOpt = HTTPResponse::FOPT_ALLOWFROM;
	http.foptUri = appenv->foptUri;
	break;
    default:;
    }
}

void  MotorEnv::readFormVar () {
    switch (form->contentType) {
    case CGIForm::T_XML:
	form->read_raw (appenv->postlimit);
	break;
    case CGIForm::T_JSON:
	form->read_raw (appenv->postlimit);
	break;
    default:
	switch (form->method) {
	case CGIForm::M_GET:
	case CGIForm::M_DELETE:
	    form->read_get ();
	    break;
	case CGIForm::M_POST:
	    if (form->contentType == CGIForm::T_MULTIPART) {
		if (appenv->postFileML.size () > 0 || appenv->postFileHtml.size () > 0) {
		    form->read_multipart (this);
		} else {
		    ::close (0);
		}
	    } else {
		form->read_post (appenv->postlimit);
	    }
	    break;
	default:;
	}
    }
}

void  MotorEnv::cacheControl () {
    if (form->method == CGIForm::M_GET) {
	switch (appenv->cacheControl) {
	case AppEnv::CC_COOKIE:
	    http.setRandomCookie (this);
	    break;
	case AppEnv::CC_URL:
	    throw (uErrorNotImpl);
	    break;
	case AppEnv::CC_NOCACHE:
	    http.fNoCache = true;
	    break;
	case AppEnv::CC_NONE:
	    break;
	}
    }
}

void  MotorEnv::doML () {
    switch (form->method) {
    case CGIForm::M_GET:
	doML (appenv->getML);
	break;
    case CGIForm::M_POST:
	if (form->contentType == CGIForm::T_MULTIPART) {
	    doML (appenv->postFileML);
	} else {
	    doML (appenv->postML);
	}
	break;
    default:
//	*log << "REQUEST_METHOD not defined.\n";
	doML (appenv->getML);
    }
}

void  MotorEnv::doML (const ustring& file) {
    ustring  b;
    ustring  f;
    bool  rc;
    bool  dash;

    if (file.size () == 0)
	return;

    if (file == uDash) {
	f = scriptFilename;
	if (f.size () == 0)
	    return;
	rc = readFile (f, b);
	dash = true;
    } else {
	f = search_resource_file (file);
	if (f.size () == 0)
	    return;
	rc = readFile (f, b);
	dash = false;
    }
    if (rc) {
	MotorTexp  ml (mlenv);
	AutoBackupPtr<ustring>  autoStr (&currentPath, &f);

	ml.scan (b, dash);
	if (ml.top.isCons ()) {
	    try {
		progn_ex (&ml.top, mlenv);
	    } catch (ustring& msg) {
		logMessage (msg);
		mlenv->currentCell = NULL;
	    } catch (boost::bad_lexical_cast tmsg) {
		logMessage (uErrorBadValueN);
		mlenv->currentCell = NULL;
	    }
	}
    } else {
	*log << file << uErrorCantOpen << "\n";
    }
}

void  MotorEnv::doMotor () {
    if (responseDone)
	return;

    if (errflag) {
	if (errorHtmlFile.size () > 0)
	    doMotor (errorHtmlFile);
	else if (htmlFile.size () > 0)
	    doMotor (htmlFile);
    } else if (htmlFile.size () > 0) {
	doMotor (htmlFile);
    }
    if (!responseDone) {
	noContentResponse ();
    }
}

void  MotorEnv::doMotor (const ustring& file, const ustring& type, MotorOutput* out) {
    ustring  f;

    if (file == uDash) {
	f = scriptFilename;
	if (f.size () == 0)
	    return;
	doMotorFile (f, true, type, out);
    } else {
	f = search_resource_file (file);
	if (f.size () == 0)
	    return;
	doMotorFile (f, false, type, out);
    }
}

void  MotorEnv::doMotorText (const ustring& text, const ustring& type, MotorOutput* out) {
    if (out == NULL)
	out = output;

    motor->compile (text);

    try {
	standardResponse (type, out->charset (), uEmpty, false);
	motor->output (out, this);
    } catch (ustring& msg) {
	logMessage (msg);
	mlenv->currentCell = NULL;
    } catch (boost::bad_lexical_cast tmsg) {
	logMessage (uErrorBadValueN);
	mlenv->currentCell = NULL;
    }
}

void  MotorEnv::doMotorFile (const ustring& file, bool skipHead, const ustring& type, MotorOutput* out) {
    if (out == NULL)
	out = output;

    motor->compile_file (file, skipHead);

    try {
	standardResponse (type, out->charset (), uEmpty, false);
	motor->output (out, this);
    } catch (ustring& msg) {
	logMessage (msg);
	mlenv->currentCell = NULL;
    } catch (boost::bad_lexical_cast tmsg) {
	logMessage (uErrorBadValueN);
	mlenv->currentCell = NULL;
    }
}

void  MotorEnv::outputFile (const ustring& src, const ustring& type, bool base64) {
    if (isPlainFile (src)) {
	if (! responseDone)
	    standardResponse (type);
	if (base64)
	    output->out_file_base64 (src);
	else
	    output->out_file (src);
    } else {
	if (! responseDone)
	    noContentResponse ();
    }
}

void  MotorEnv::outputFile (const ustring& src, const ustring& type, bool finline, const ustring& dispname, bool base64) {
    if (isPlainFile (src)) {
	if (! responseDone) {
	    off_t  filesize;
	    fileSize (src, filesize);
	    standardResponse (type, uEmpty, dispname, finline, filesize);
	}
	if (base64)
	    output->out_file_base64 (src);
	else
	    output->out_file (src);
    } else {
	if (! responseDone)
	    noContentResponse ();
    }
}

void  MotorEnv::standardResponse (const ustring& type) {
    if (responseDone)
	return;
    if (output->isResponse ()) {
	http.standardResponse (output, type, output->charset (), this);
	responseDone = true;
    }
}

void  MotorEnv::standardResponse (const ustring& type, const ustring& charset, const ustring& dispname, bool finline, off_t filesize) {
    if (responseDone)
	return;
    if (output->isResponse ()) {
	if (dispname.length () > 0) {
	    http.disposition (output, finline, dispname, filesize);
	    http.standardResponse (output, type, output->charset (), this);
	} else if (matchHead (type, CharConst ("text/")) && charset.length () > 0) {
	    http.standardResponse (output, type, charset, this);
	} else {
	    http.standardResponse (output, type, output->charset (), this);
	}
	responseDone = true;
    }
}

void  MotorEnv::standardResponse_html () {
    if (responseDone)
	return;
    if (output->isResponse ()) {
	http.standardResponse_html (output, this);
	responseDone = true;
    }
}

void  MotorEnv::noContentResponse () {
    if (responseDone)
	return;
    if (output->isResponse ()) {
	http.noContentResponse (output, this);
	responseDone = true;
    }
}

void  MotorEnv::forbiddenResponse () {
    if (responseDone)
	return;
    if (output->isResponse ()) {
	http.forbiddenResponse (output, this);
	responseDone = true;
    }
}

void  MotorEnv::location (const ustring& url) {
    if (responseDone)
	return;
    if (output->isResponse ()) {
	http.location (output, url, this);
	responseDone = true;
    }
}

void  MotorEnv::location_html (const ustring& url) {
    if (responseDone)
	return;
    if (output->isResponse ()) {
	http.location_html (output, url, this);
	responseDone = true;
    }
}

void  MotorEnv::logMessage (const ustring& msg) {
    boost::ptr_vector<MNodePtr>  p;

    if (log) {
	if (mlenv->currentCell ()) {
	    datetime_list (p, now ());
	    *log << formatString (ustring (CharConst ("[${3:int:2:0}/${2:month}/${1:int:4:0}:${4:int:2:0}:${5:int:2:0}:${6:int:2:0}] ")), p);
	    mlenv->logLinenum (mlenv->currentCell ());
	    *log << mlenv->currentCell ()->dump_string_short () << ": ";
	}
	*log << logText (msg) << "\n";
    }
}
