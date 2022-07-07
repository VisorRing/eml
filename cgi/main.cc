#include "heapdebug.h"
#include "httpconst.h"
#include "app.h"
#include "motorenv.h"
#include "motor.h"
#ifdef UTF8JP
#include "form_utf8-jp.h"
#include "motoroutput-jp.h"
#else
#include "form_utf8.h"
#include "motoroutput.h"
#endif
#include "util_check.h"
#include "util_string.h"
#include "util_file.h"
#include "util_mimetype.h"
#include "util_splitter.h"
#include "ustring.h"
#include <iostream>
#include <stdlib.h>
#include <unistd.h>

using namespace  std;

static ustring  glue1 (MotorEnv* env) {
    char*  cwd = getcwd (NULL, 0);
    ustring  p;
    std::vector<ustring>  a;
    ustring  pathinfo;
    ustring  scriptfilename;
//    ustring  scriptname;
    ustring  pwd;
    int  i;
    int  rc;

    p = getenvString (kPATH_INFO);
    {
//	uiterator  b = p.begin ();
//	uiterator  e = p.end ();
//	umatch  m;
//	while (usearch (b, e, m, re_slash)) {
//	    a.push_back (ustring (b, m[0].first));
//	    b = m[0].second;
//	}
//	a.push_back (ustring (b, e));
	SplitterCh  sp (p, '/');
	while (sp.next ()) {
	    a.push_back (sp.pre ());
	}
    }
    if (cwd) {
	scriptfilename = ustring (cwd);
	free (cwd);
	scriptfilename.append (CharConst ("/ml"));
//	scriptname = getenvString (kSCRIPT_NAME);
	pwd = scriptfilename;
	for (i = 1; i < a.size (); i ++) {
	    scriptfilename.append (uSlash).append (a[i]);
//	    scriptname.append (uSlash).append (a[i]);
	    env->scriptName.append (uSlash).append (a[i]);
	    if (isPlainFile (scriptfilename)) {
		break;
	    } else if (isDirectory (scriptfilename)) {
	    } else {
		return uEmpty;
	    }
	    pwd = scriptfilename;
	}
	for (i ++; i < a.size (); i ++) {
	    pathinfo.append (uSlash).append (a[i]);
	}
	setenv (kPATH_INFO, pathinfo.c_str (), 1);
	setenv (kSCRIPT_FILENAME, scriptfilename.c_str (), 1);
//	setenv (kSCRIPT_NAME, scriptname.c_str (), 1);
	setenv (kSCRIPT_NAME, env->scriptName.c_str (), 1);
	rc = chdir (pwd.c_str ());
    } else {
    }
    return scriptfilename;
}

void  glue2 (ustring scriptfilename, AppEnv* aenv, MotorEnv* env) {
    ustring  hmltext;
    int  argc = 0;
    char** argv = NULL;

    int  rc;
    FileMacro  f;
    off_t  s;
    ustring::iterator  b, b0, e;
    int  st;

#define  MAXARGS	32

    if (scriptfilename.length () == 0)
	return;

    if (f.openRead (scriptfilename.c_str ())) {
	s = f.size ();
	if (s > 2048) {
	    s = 2048;
	}
	if (s > 0) {
	    hmltext.resize (s + 1);
	    f.read (&hmltext.at (0), s);
	    hmltext.at (s) = '\0';
	}
	f.close ();

	b = hmltext.begin ();
	e = hmltext.end () - 1;
	if (s >= 3 && memcmp (&b[0], "\xef\xbb\xbf", 3) == 0) {
	    b += 3;
	}
	b0 = b;
	st = 0;
	argc = 0;
	for (; b < e; b ++) {
	    switch (st) {
	    case 0:		// init, space
		if (isspace (*b)) {
		} else if (' ' < *b && *b < 0x7f) {
		    st = 2;
		    argc ++;
		} else {	// non printable
		    *b = 0;
		    b = e;
		}
		break;
	    case 2:		// ascii
		if (isspace (*b)) {
		    st = 0;
		    if (argc == MAXARGS) {
			*b = 0;
			b = e;
		    }
		} else if (' ' < *b && *b < 0x7f) {
		} else {
		    *b = 0;
		    b = e;
		}
		break;
	    }
	}
	argv = new char* [argc + 1];
	b = b0;
	st = 0;
	argc = 0;
	for (; b < e; b ++) {
	    switch (st) {
	    case 0:
		if (isspace (*b)) {
		} else if (' ' < *b && *b < 0x7f) {
		    st = 2;
		    argv[argc] = &b[0];
		    argc ++;
		} else {
		    *b = 0;
		    b = e;
		}
		break;
	    case 2:		// ascii
		if (isspace (*b)) {
		    st = 0;
		    *b = 0;
		    if (argc == MAXARGS) {
			b = e;
		    }
		} else if (' ' < *b && *b < 0x7f) {
		} else {
		    *b = 0;
		    b = e;
		}
		break;
	    }
	}
	argv[argc] = NULL;
    } else {
	// open failed
    }

    if (argv)
	aenv->readOption (argc, argv, env);

    delete argv; 
}

int  main (int argc, char** argv) {
    AppEnv  aenv;
    CGIFormFile*  form = NULL;
    HTMLMotor  motor;
    MotorOutput*  out = NULL;
    ustring  scriptfilename;

#ifdef kErrorLog
//    close (2);
//    open (kErrorLog, O_WRONLY | O_APPEND | O_CREAT, 0666);
    aenv.setErrorLog (ustring (CharConst (kErrorLog)));
#endif

#ifdef UTF8JP
    if (checkAgent () & UA_Windows) {
	form = new CGIFormUTF8JPMS;
	out = new MotorOutputCOutJPMS;
    } else {
	form = new CGIFormUTF8JPMS;
	out = new MotorOutputCOut;
    }
#else
    form = new CGIFormUTF8;
    out = new MotorOutputCOut;
#endif
    MotorEnv  env (&aenv, form, &motor, out);

    try {
	ustring  ext;

	scriptfilename = glue1 (&env);
	if (! isPlainFile (scriptfilename) || isExecutableFile (scriptfilename)) {
//	if (! isPlainFile (scriptfilename)) {
	    env.forbiddenResponse ();
	    goto Ex1;
	}

	ext = getExt (scriptfilename);
	if (match (ext, CharConst ("hml"))) {
	} else if (match (ext, CharConst ("ml"))) {
	    env.noContentResponse ();
	    goto Ex1;
	} else {
	    ustring  type = mimetype (ext);
	    if (type.empty ()) {
		// not reached
		type.assign (CharConst (kMIME_TEXT));
	    }
	    env.outputFile (scriptfilename, type);
	    goto Ex1;
	}

	glue2 (scriptfilename, &aenv, &env);

#ifdef DEBUG
//	char*  em = getenv (kREQUEST_METHOD);
//	char*  en = getenv (kSCRIPT_NAME);
	std::cerr << "============================================================\n";
#if 0
	if (em) {
	    std::cerr << em << " ";
	    if (en)
		std::cerr << en;
	    std::cerr << "\n";
	}
#endif
	std::cerr << env.form->requestMethod << " " << env.scriptName << "\n";
	aenv.dump (std::cerr);
	std::cerr << "--\n";
#endif /* DEBUG */
	aenv.setDefault ();

	env.setDefault ();
	env.readFormVar ();
	env.cacheControl ();
	env.doML ();
	env.doMotor ();
    }
    catch (ustring& msg) {
	std::cerr << msg << "\n";
	goto Ex1;
    }

 Ex1:;
    delete out;
    delete form;

    return 0;
}
