#include "app.h"
#include "motorenv.h"
#include "util_const.h"
#include "util_check.h"
#include "util_string.h"
#include <boost/lexical_cast.hpp>
#include <string.h>
#include <iostream>
#include <stdlib.h>
#include <fcntl.h>

static bool  pcmp (char** a, const char* b, int n) {
    int  rc = strncmp (*a, b, n);
    if (rc == 0) {
	*a += n;
	return true;
    } else {
	return false;
    }
}
#define cmp(a,b)	pcmp (&(a), (b), sizeof (b) - 1)
#define zcmp(a,b)	(strcmp ((a), (b)) == 0)

/*DOC:
=ml=

|table:|
|datastore:NAME|
|get-html:FILENAME|
|post-html:FILENAME|
|post-file-html:FILENAME|
|html:FILENAME|
|error-html:FILENAME|
|get-ml:FILENAME|
|post-ml:FILENAME|
|post-file-ml:FILENAME|
|ml:FILENAME|
|type:MIME-TYPE|
|to-code:ENCODING|
|post-limit:DATASIZE|
|post-file-limit:DATASIZE|
|random-cookie|
|random-url|
|no-cache|
|frame-options:[DENY|SAMEORIGIN]|
|dump|

*/

void  AppEnv::readOption (int argc, char** argv, MotorEnv* env) {
    int  i;
    char*  p;

    arg0 = ustring (argv[0]);

#ifdef TARGET_MLDUMP
    debugDump = true;
#endif

    for (i = 1; i < argc; i ++) {
	p = argv[i];
	if (cmp (p, "datastore:")) {
	    datastore = ustring (p);
	    if (! matchName (datastore))
		throw (datastore + uErrorBadDatastore);
	} else if (cmp (p, "get-html:")) {
	    getHtml = ustring (p);
	    if (getHtml != uDash && ! matchResourceName (getHtml))
		throw (getHtml + uErrorBadFile);
	} else if (cmp (p, "post-html:")) {
	    postHtml = ustring (p);
	    if (postHtml != uDash && ! matchResourceName (postHtml))
		throw (postHtml + uErrorBadFile);
	} else if (cmp (p, "post-file-html:")) {
	    postFileHtml = ustring (p);
	    if (postFileHtml != uDash && ! matchResourceName (postFileHtml))
		throw (postFileHtml + uErrorBadFile);
	} else if (cmp (p, "html:")) {
	    postFileHtml = postHtml = getHtml = ustring (p);
	    if (getHtml != uDash && ! matchResourceName (getHtml))
		throw (getHtml + uErrorBadFile);
	} else if (cmp (p, "error-html:")) {
	    errorHtml = ustring (p);
	    if (errorHtml != uDash && ! matchResourceName (errorHtml))
		throw (errorHtml + uErrorBadFile);
	} else if (cmp (p, "get-ml:")) {
	    getML = ustring (p);
	    if (getML != uDash && ! matchResourceName (getML))
		throw (getML + uErrorBadFile);
	} else if (cmp (p, "post-ml:")) {
	    postML = ustring (p);
	    if (postML != uDash && ! matchResourceName (postML))
		throw (postML + uErrorBadFile);
	} else if (cmp (p, "post-file-ml:")) {
	    postFileML = ustring (p);
	    if (postFileML != uDash && ! matchResourceName (postFileML))
		throw (postFileML + uErrorBadFile);
	} else if (cmp (p, "ml:")) {
	    postML = getML = ustring (p);
	    if (getML != uDash && ! matchResourceName (getML))
		throw (getML + uErrorBadFile);
	} else if (cmp (p, "type:")) {
	    mimetype = ustring (p);
	    if (! matchMimeType (mimetype))
		throw (mimetype + ": bad mime type.");
	} else if (cmp (p, "to-code:")) {
	    ocode = ustring (p);
	    if (! matchName (ocode))
		throw (ocode + ": bad encoding name.");
	} else if (cmp (p, "post-limit:")) {
	    size_t  num = boost::lexical_cast <size_t> (p);
	    if (num > cPOSTLIMITHARD) {
		*env->log << "post-limit:" << num << ": limit exceeded.\n";
		postlimit = cPOSTLIMITHARD;
	    } else {
		postlimit = num;
	    }
	} else if (cmp (p, "postfile-limit:")
		   || cmp (p, "post-file-limit:")) {
	    size_t  num = boost::lexical_cast <size_t> (p);
	    if (num > cPOSTFILELIMITHARD) {
		*env->log << "post-file-limit:" << num << ": limit exceeded.\n";
		postfilelimit = cPOSTFILELIMITHARD;
	    } else {
		postfilelimit = num;
	    }
	} else if (zcmp (p, "random-cookie")) {
	    cacheControl = CC_COOKIE;
	} else if (zcmp (p, "random-url")) {
	    cacheControl = CC_URL;
	} else if (zcmp (p, "no-cache")) {
	    cacheControl = CC_NOCACHE;
	} else if (cmp (p, "frame-options:")) {
	    if (zcmp (p, "DENY")) {
		frameOpt = FOPT_DENY;
	    } else if (zcmp (p, "SAMEORIGIN")) {
		frameOpt = FOPT_SAMEORIGIN;
	    } else {
		throw (ustring (CharConst ("frame-options:")) + p + ustring (CharConst (" bad option.")));
	    }
	} else if (zcmp (p, "dump")) {
	    debugDump = true;
	} else if (zcmp (p, "no-trace")) {
	    noTrace = true;
	} else if (p[0] == ';' || p[0] == '(' || p[0] == '<') {
	    break;
	} else {
	    throw (ustring (p) + ustring (": unrecognized parameter"));
	}
    }
}

void  AppEnv::setDefault () {
    if (mimetype.size () == 0)
	mimetype = ustring (CharConst (kMIME_HTML));
    if (cacheControl == CC_NONE)
	cacheControl = CC_COOKIE;
}

#if 0
ustring  AppEnv::scriptName () {
    return getenvString (kSCRIPT_FILENAME);
}
#endif

void  AppEnv::dump (std::ostream& out) {
    out << arg0 << "\n";
    if (datastore.size () > 0)
	out << "	datastore:" << datastore << "\n";
    if (getML.size () > 0) {
	if (getML == postML) {
	    out << "	ml:" << getML << "\n";
	} else {
	    out << "	get-ml:" << getML << "\n";
	    if (postML.size () > 0)
		out << "	post-ml:" << postML << "\n";
	}
    } else if (postML.size () > 0) {
	out << "	post-ml:" << postML << "\n";
    }
    if (postFileML.size () > 0) {
	out << "	post-file-ml:" << postFileML << "\n";
    }
    if (getHtml.size () > 0) {
	if (getHtml == postHtml) {
	    out << "	html:" << getHtml << "\n";
	} else {
	    out << "	get-html:" << getHtml << "\n";
	    if (postHtml.size () > 0)
		out << "	post-html:" << postHtml << "\n";
	}
    } else if (postHtml.size () > 0) {
	out << "	post-html:" << postHtml << "\n";
    }
    if (postFileHtml.size () > 0) {
	out << "	post-file-html:" << postFileHtml << "\n";
    }
    if (errorHtml.size () > 0)
	out << "	error-html:" << errorHtml << "\n";
    if (mimetype.size () > 0)
	out << "	type:" << mimetype << "\n";
    if (postlimit != cPOSTLIMITDEFAULT)
	out << "	post-limit:" << postlimit << "\n";
    if (postfilelimit != cPOSTFILELIMITDEFAULT)
	out << "	post-file-limit:" << postfilelimit << "\n";
    switch (cacheControl) {
    case CC_COOKIE:
	out << "	random-cookie\n";
	break;
    case CC_URL:
	out << "	random-url\n";
	break;
    case CC_NOCACHE:
	out << "	no-cache\n";
	break;
    default:;
    }
    switch (frameOpt) {
    case FOPT_NONE:
	break;
    case FOPT_DENY:
	out << "	frame-options:DENY\n";
	break;
    case FOPT_SAMEORIGIN:
	out << "	frame-options:SAMEORIGIN\n";
	break;
    case FOPT_ALLOWFROM:
	out << "	frame-options:ALLOW-FROM " << foptUri << "\n";
	break;
    default:;
    }
    if (noTrace)
	out << "	no-trace\n";
#ifndef TARGET_MLDUMP
    if (debugDump)
	out << "	dump\n";
#endif
}

void  AppEnv::setErrorLog (const ustring& path, bool fappend) {
    int  flag = O_WRONLY | O_CREAT;

    if (fappend)
	flag |= O_APPEND;
    close (2);
    open (path.c_str (), flag, 0666);
}
