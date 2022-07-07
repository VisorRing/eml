#include "motor-function.h"
#include "expr.h"
#include "ml.h"
#include "mlenv.h"
#include "motoroutput.h"
#include "motorenv.h"
#include "motor.h"
#include "wikiformat.h"
#include "util_const.h"
#include "util_string.h"
#include "util_time.h"
#include "ustring.h"
#include <boost/ptr_container/ptr_vector.hpp>
#include <vector>

/*DOC:
===eval===
 %{eval:NAME}%
 %{eval:NAME:ARGS:...}%

*/
//#MTFUNC	eval	mf_eval
void  mf_eval (const std::vector<ustring>& args, MlEnv* mlenv) {
    MNodePtr  vargs;
    MNodePtr  ans;

    if (args.size () > 0) {
	vargs = buildArgs (1, args);
	MNode*  sexp = mlenv->env->motorCall.getVar (args[0]);
	if (isLambda (sexp)) {
	    ans = execDefun (mlenv, sexp, vargs (), args[0]);
	}
    }
}

/*DOC:
===raw===
 %{raw:VAR}%

*/
//#MTFUNC	raw	mf_raw
void  mf_raw (const std::vector<ustring>& args, MlEnv* mlenv) {
    ustring  u;
    MNode*  v;

    if (args.size () == 1) {
	v = mlenv->getVar (args[0]);
	if (v) {
	    u = v->to_string ();
	    mlenv->env->output->out_toText (u);
	}
    } else {
	throw (uErrorWrongNumber);		// ***
    }
}

/*DOC:
===url===
 %{url:VAR}%

*/
//#MTFUNC	url	mf_url
void  mf_url (const std::vector<ustring>& args, MlEnv* mlenv) {
    MNode*  v;

    if (args.size () == 1) {
	v = mlenv->getVar (args[0]);
	if (v) {
	    mlenv->env->output->out_raw (percentEncode_path (v->to_string ()));
	}
    } else {
	throw (uErrorWrongNumber);		// ***
    }
}

/*DOC:
===js===
 %{js||TEXT}%

*/
//#MTFUNC2	js	mf_js
void  mf_js (const std::vector<ustring>& args, const ustring& arg2, MlEnv* mlenv) {
    MNode*  v;

    if (args.size () == 0) {
	mlenv->env->output->out_toJS (arg2);
    } else {
	throw (uErrorWrongNumber);		// ***
    }
}

/*DOC:
===pad0===
 %{pad0:NUM:VAR}%

*/
//#MTFUNC	pad0	mf_pad0
void  mf_pad0 (const std::vector<ustring>& args, MlEnv* mlenv) {
    int  n;
    ustring  t;

    if (args.size () > 1) {
	n = strtol (args[0]);
	t = to_string (mlenv->getVar (args[1]));
	mlenv->env->output->out_toHTML (zeroPad (n, t));
    }
}

/*DOC:
===wiki===
 %{wiki:VAR}%
 %{wiki:VAR:super}%

*/
//#MTFUNC	wiki	mf_wiki
void  mf_wiki (const std::vector<ustring>& args, MlEnv* mlenv) {
    bool  super = false;
    MNode*  v;

    if (args.size () < 1)
	return;
    
    v = mlenv->getVar (args[0]);
    if (args.size () == 2) {
	if (match (args[1], CharConst ("super"))) {
	    super = true;
	}
    }
    if (v) {
	WikiFormat  w (mlenv->env, true);
	ustring t = v->to_string ();
	w.compile (t, super);
	w.output ();
    }
}

/*DOC:
===date===
 %{date:VAR||FORMAT}%

 format: %<flag><width><directive>
 flag: -, 0, _, :, ::
 directive: %Y, %y, %C, %G, %g
 	%m, %b, %B
 	%d, %e, %j
 	%u, %w, %a, %A
 	%V, %W, %U
 	%H, %k // %I, %l // %M // %S //%f, %L // %s // %P, %p
 	%Z, %z, %:z, %::z

*/
//#MTFUNC2	date	mf_date
void  mf_date (const std::vector<ustring>& args, const ustring& arg2, MlEnv* mlenv) {
    time_t  tm;
    struct tm  tmv;
    ustring  format;
    int  i;
    MNode*  v;

    if (args.size () > 0) {
	v = mlenv->getVar (args[0]);
	if (! isNil (v)) {
	    tm = to_int64 (v);
	    format = arg2;
	    if (format.length () == 0)
		format = uTimeFormat;
	    localtime_r (&tm, &tmv);
	    mlenv->env->output->out_toHTML (formatDateString (format, tmv));
	}
    }
}
