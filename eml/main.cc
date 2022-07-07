#include "heapdebug.h"
#include "httpconst.h"
#include "app.h"
#include "motorenv.h"
#include "motor.h"
#include "mlenv.h"
#ifdef UTF8JP
#include "form_utf8-jp.h"
#include "motoroutput-jp.h"
#include "motoroutput-iconv.h"
#else
#include "form_utf8.h"
#include "motoroutput.h"
#endif
#include "util_check.h"
#include "ustring.h"
#include <iostream>
#include <stdlib.h>
#include <unistd.h>

using namespace  std;

int  main (int argc, char** argv) {
    AppEnv  aenv;
    CGIFormFile*  form = NULL;
    HTMLMotor  motor;
    MotorOutput*  out = NULL;
#ifdef UTF8JP
    form = new CGIFormUTF8JPMS;
    if (checkAgent () & UA_Windows) {
	out = new MotorOutputCOutJPMS;
    } else {
	out = new MotorOutputCOut;
    }
#else
    form = new CGIFormUTF8;
    out = new MotorOutputCOut;
#endif
    MotorEnv  env (&aenv, form, &motor, out);

#ifdef kErrorLog
    aenv.setErrorLog (ustring (CharConst (kErrorLog)));
#endif
    try {
	aenv.readOption (argc, argv, &env);

	if (aenv.ocode.size () > 0) {
	    delete out;
	    out = NULL;
	    out = new MotorOutputIConvOStream (aenv.ocode.c_str ());
	    env.output = out;
	}
	if (aenv.debugDump) {
	    close (2);
	    dup (1);
	    if (form->method != CGIForm::M_NONE) {
		env.standardResponse (ustring (kMIME_TEXT), uUTF8, uEmpty, false);
		cout.flush ();
		cerr.flush ();
	    } else {
		env.responseDone = true;
	    }
	}
	if (aenv.noTrace) {
	    env.mlenv->mlPool->nolog = true;
	}

#ifdef DEBUG
//	char*  em = getenv (kREQUEST_METHOD);
//	char*  en = getenv (kSCRIPT_NAME);
	std::cerr << "============================================================\n";
	std::cerr << mailDate ();
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
	std::cerr << msg << uLF;
	goto Ex1;
    }
    catch (boost::bad_lexical_cast tmsg) {
	std::cerr << uErrorBadValueN << uLF;
	goto Ex1;
    }

 Ex1:;
    delete out;
    delete form;

    return 0;
}
