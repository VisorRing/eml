#include "motor.h"
#include "motoroutput.h"
#include "motorenv.h"
#include "mlenv.h"
#include "expr.h"
#include "util_const.h"
#include "util_check.h"
#include "util_file.h"
#include "util_splitter.h"
#include "util_string.h"
#include "ustring.h"
#include "mftable.h"
#include "motorconst.h"
#include "utf8.h"
#include <iostream>
#include <assert.h>

/*DOC:
==HTMLMotor==

*/

using namespace  std;

#define cSCOM	"(?:<!--)?"
#define cECOM	"(?: *-->)?"
static uregex  re_OpenBra ("\\\\|%{");
static uregex  re_OpenCloseBra ("\\\\|%{|}%");
static uregex  re_Name ("^[a-zA-Z_][a-zA-Z_0-9]*(\\.[a-zA-Z_][a-zA-Z_0-9]*)*");
static uregex  re_NameOpt ("^(}%|/}%|~}%|,}%|:|\\|\\||\\?|!\\?)");
static uregex  re_NextCloseColonBar ("\\\\|%{|}%|:|\\|\\|");
static uregex  re_NextColon ("\\\\|%{|:");
static uregex  re_NextCloseSelect ("\\\\|%{|}%|\\|\\|(!?\\?)?");
static uregex  re_NextClose ("\\\\|%{|}%");
static uregex  re_Capture ("([0-9]+)(\\|\\||}%)");

// ============================================================
ustring  MotorObj::dumpVec (MotorObjVec& v) {
    ustring  ans;
    int  i;

    for (i = 0; i < v.size (); i ++) {
	ans.append (v[i].dump ());
    }
    return ans;
}

// ============================================================
void  MotorObjText::out (MotorOutput* o, MotorEnv* env) {
    o->out_templateText (text);
}

ustring  MotorObjText::dump () {
    return ustring (uBra2 + text + uCket2);
}

// ============================================================
void  MotorObjVar::out (MotorOutput* o, MotorEnv* env) {
    ustring  val;

    val = env->mlenv->getVar_string (variable);
    switch (vopt) {
    case opt_normal:
	o->out_toHTML (val);
	break;
    case opt_br:
	o->out_toHTML_br (val);
	break;
    case opt_nbsp:
	if (val.size () > 0)
	    o->out_toHTML (val);
	else
	    o->outNbsp ();
	break;
    case opt_c3:
	val = c3 (val);
	o->out_toHTML (val);
	break;
    case opt_wbr:
	o->out_toHTML_wbr (val);
	break;
    default:;
	assert (0);
    }
}

ustring  MotorObjVar::dump () {
    ustring  ans = uBra2 + variable;

    switch (vopt) {
    case opt_normal:
	break;
    case opt_br:
	ans += uSlash;
	break;
    case opt_nbsp:
	ans += ustring (CharConst ("~"));
	break;
    case opt_c3:
	ans += ustring (CharConst (","));
	break;
    case opt_wbr:
	ans += ustring (CharConst ("^"));
	break;
//    case opt_raw:
//	break;
//    case opt_br_nbsp:
//	break;
//    case opt_br_a:
//	break;
//    case opt_br_a_nbsp:
//	break;
//    case opt_wiki:
//	break;
//    case opt_url:
//	break;
    default:
	assert (0);
    }
    return ans + uCket2;
}

// ============================================================
void  MotorObjCapture::out (MotorOutput* o, MotorEnv* env) {
    motor->setTemplate (num, &objs);
}

ustring  MotorObjCapture::dump () {
    return uBra2 + num + uColon + dumpVec (objs) + uCket2;
}

// ============================================================
void  MotorObjExpand::out (MotorOutput* o, MotorEnv* env) {
    MotorObj::MotorObjVec*  t = motor->getTemplate (num);
    if (t) {
	motor->output (*t, o, env);
    }
}

ustring  MotorObjExpand::dump () {
    return uBra2 + num + uCket2;
}

// ============================================================
void  MotorObjSelect::out (MotorOutput* o, MotorEnv* env) {
    ustring  val;
    MotorOutputString  m;

    val = env->mlenv->getVar_string (variable);
    motor->output (value, &m, env); // XXX: must apply rconv().
    if (val == m.ans) {
	if (fbang) {
	    motor->output (falseobjs, o, env);
	} else {
	    motor->output (trueobjs, o, env);
	}
    } else {
	if (fbang) {
	    motor->output (trueobjs, o, env);
	} else {
	    motor->output (falseobjs, o, env);
	}
    }
}

ustring  MotorObjSelect::dump () {
    ustring  ans;

    ans = uBra2 + variable;
    if (fbang)
	ans.append (CharConst ("!?"));
    else
	ans.append (CharConst ("?"));
    ans.append (dumpVec (value));
    ans.append (uColon);
    ans.append (dumpVec (trueobjs));
    if (falseobjs.size () > 0)
	ans.append (CharConst ("||")).append (dumpVec (falseobjs));
    return ans + uCket2;
}

// ============================================================
void  MotorObjFunc::out (MotorOutput* o, MotorEnv* env) {
    AutoInclCount  autoIncl (env->mlenv);

    autoIncl.inc ();
    try {
	std::vector<ustring>  sargs;
	for (int i = 0; i < args.size (); ++ i) {
	    sargs.push_back (to_string (&args[i], env));
	}
	fn (sargs, env->mlenv);
    } catch (ustring& msg) {
	ustring amsg = dump ();
	amsg = ellipsis (logText (amsg), cLOGSHORTSIZE);
	amsg.append (CharConst (": ")).append (msg);
	throw (amsg);
    }
}

ustring  MotorObjFunc::dump () {
    ustring  ans;
    int  i, j;

    ans = uBra2 + name;
    for (i = 0; i < args.size (); i ++) {
	ans.append (uColon).append (dumpVec (args[i]));
    }
    return ans + uCket2;
}

// ============================================================
void  MotorObjFunc2::out (MotorOutput* o, MotorEnv* env) {
    AutoInclCount  autoIncl (env->mlenv);

    autoIncl.inc ();
    try {
	std::vector<ustring>  sargs;
	for (int i = 0; i < args.size (); ++ i) {
	    sargs.push_back (to_string (&args[i], env));
	}
	ustring  sarg2 = to_string (&arg2, env);
	fn (sargs, sarg2, env->mlenv);
    } catch (ustring& msg) {
	ustring amsg = dump ();
	amsg = ellipsis (logText (amsg), cLOGSHORTSIZE);
	amsg.append (CharConst (": ")).append (msg);
	throw (amsg);
    }
}

ustring  MotorObjFunc2::dump () {
    ustring  ans;
    int  i, j;

    ans = uBra2 + name;
    for (i = 0; i < args.size (); i ++) {
	ans.append (uColon).append (dumpVec (args[i]));
    }
    if (arg2.size () > 0)
	ans.append (CharConst ("||")).append (dumpVec (arg2));
    return ans + uCket2;
}

// ============================================================
void  MotorObjDefun::out (MotorOutput* o, MotorEnv* env) {
    MNodePtr  ans;
    MNode*  sexp = env->motorCall.getVar (name);
#ifdef DEBUG2
    std::cerr << "defun:" << dump_to_texp (sexp) << "\n";
#endif /* DEBUG */
    if (isLambda (sexp)) {
	MNodeList  vargs;
	for (int i = 0; i < args.size (); ++ i) {
	    vargs.append (newMNode_str (new ustring (to_string (&args[i], env))));
	}
#ifdef DEBUG2
	std::cerr << "parameter:" << dump_to_texp (sexp->lambda.sexp->car ()) << "\n";
	std::cerr << "body:     " << dump_to_texp (sexp->lambda.sexp->cdr ()) << "\n";
	std::cerr << "argument: " << dump_to_texp (vargs ()) << "\n";
#endif /* DEBUG */
	ans = execDefun (env->mlenv, sexp->lambda.sexp->car (), sexp->lambda.sexp->cdr (), vargs (), name);
    }
}

ustring  MotorObjDefun::dump () {
    ustring  ans;
    int  i;

    ans = uBra2 + name;
    for (i = 0; i < args.size (); i ++) {
	ans.append (uColon).append (dumpVec (args[i]));
    }
    return ans + uCket2;
}

// ============================================================
// ============================================================
// ============================================================
/*DOC:
===variable interpolation===
|h:Tag|h:Note|
| %{''VAR''}% | variable expansion |
| %{''VAR''~}% | if empty, padding with \&nbsp; |
| %{''VAR'',}% | comma placement |
| %{''VAR''/}% | replacing NL with \<br> |
| %{''FUNCTION'':''PARAM1'':...||''PARAM''}% | function call. |
//| %{raw:''VAR''}% | no '&' encoding |
//| %{url:''VAR''}% | url encoding |
//| %{js:''VAR''}% | encoding for javascript string literal |
//| %{pad0:''NUM'':''VAR''}% | zero-padding |
//| %{text_nbsp:''VAR''}% | \<br> and empty padding |
//| %{text_url:''VAR''}% | replacing NL with \<br> and url link |
//| %{wiki:''VAR''}% | wiki output |

===conditional component===
|h:Tag|h:Note|
| %{''VAR''?''VALUE'':''TEXT1''}% | output ''TEXT1'' if ''VAR'' is ''VALUE'' |
| %{''VAR''?''VALUE'':''TEXT1''\|\|''TEXT2''}% | output ''TEXT1'' if ''VAR'' is ''VALUE'', otherwise ''TEXT2'' |
| %{''VAR''!?''VALUE'':''TEXT1''}% | output ''TEXT1'' unless ''VAR'' is ''VALUE'' |
| %{''VAR''!?''VALUE'':''TEXT1''\|\|''TEXT2''}% | output ''TEXT1'' unless ''VAR'' is ''VALUE'', otherwise ''TEXT2'' |

===template fraction and function===
|h:Tag|h:Note|
| %{''NUM'':''TEXT1''}% | fraction definition |
| %{eval:''NAME''}% | motor function execution |
| %{eval:''NAME'':''PARAM1'':''PARAM2''...}% | motor function execution with parameters |

*/
void  HTMLMotor::compile (const ustring& text, bool skipHead) {
    begin = text.begin ();
    end = text.end ();
    if (skipHead) {
	while (begin != end && *begin != '<') {
	    findNLb (begin, end);
	}
    }
    shiftOne (&objs);
}

void  HTMLMotor::compile_file (const ustring& path, bool skipHead) {
    ustring  b;

    if (readFile (path, b)) {
	compile (b, skipHead);
    }
}

void  HTMLMotor::output (MotorObj::MotorObjVec& v, MotorOutput* o, MotorEnv* env) {
    int  i, n;

    n = v.size ();
    for (i = 0; i < n; i ++) {
	v[i].out (o, env);
    }
}

void  HTMLMotor::output (MotorOutput* o, MotorEnv* env) {
    output (objs, o, env);
}

void  HTMLMotor::outputTemp (const ustring& text, MotorOutput* o, MotorEnv* env) {
    int  i, n;

    n = objs.size ();
#ifdef DEBUG2
    std::cerr << "objs.size ():" << objs.size () << "\n";
#endif /* DEBUG */
    compile (text);
    for (i = n; i < objs.size (); i ++) {
	objs[i].out (o, env);
    }
    while (objs.size () > n) {
#ifdef DEBUG2
	std::cerr << "objs.size ():" << objs.size () << "\n";
#endif /* DEBUG */
	objs.pop_back ();
    }
}

void  HTMLMotor::setTemplate (const ustring& name, MotorObj::MotorObjVec* value) {
    templates.erase (name);
    templates.insert (boost::unordered_map<ustring, MotorObj::MotorObjVec*>::value_type (name, value));
}

MotorObj::MotorObjVec*  HTMLMotor::getTemplate (const ustring& name) {
    boost::unordered_map<ustring, MotorObj::MotorObjVec*>::iterator  it = templates.find (name);
    if (it == templates.end ()) {
	return NULL;
    } else {
	return it->second;
    }
}

#ifdef DEBUG2
void  HTMLMotor::dump (std::ostream& o) {
    int  i;

    for (i = 0; i < objs.size (); i ++) {
//	objs[i].dump (o, 0);
	objs[i].dump ();
    }
}
#endif /* DEBUG */

void  HTMLMotor::shiftOne (MotorObj::MotorObjVec* sobjs) {
    umatch  m;
    uiterator  start = begin;
    while (start < end) {
	if (usearch (start, end, m, re_OpenBra)) {
	    switch (*m[0].first) {
	    case '\\':
		shiftPrec (sobjs, m);
		start = begin;
		nextChar (start, end);
		break;
	    case '%':		// %{
		shiftPrec (sobjs, m);
		start = begin - 2; // %{
		if (shiftOpen (sobjs)) {
		    start = begin;
		} else {
		    begin = start;
		    nextChar (start, end);
		}
		break;
	    default:
		assert (0);
	    }
	} else {
	    sobjs->push_back (new MotorObjText (this, begin, end));
	    start = end;
	}
    }
}

bool  HTMLMotor::shiftOpen (MotorObj::MotorObjVec* sobjs) {
    // %{ の後
    umatch  m;
    if (usearch (begin, end, m, re_Name)) {
	begin = m[0].second;
	return shiftOpenName (sobjs, ustring (m[0].first, m[0].second));
    } else if (usearch (begin, end, m, re_Capture)) { // %{888
	// static uregex  re_Capture ("([0-9]+)(\\|\\||}%)");
	ustring  num (m[1].first, m[1].second);
	begin = m[0].second;
	switch (*m[2].first) {
	case '|':		// %{888||
#ifdef DEBUG
	    std::cerr << "%{888||\n";
#endif /* DEBUG */
	    return shiftOpenNumBar (sobjs, num);
	case '}':		// %{888}%
#ifdef DEBUG
	    std::cerr << "%{888}%\n";
#endif /* DEBUG */
	    return shiftOpenNum (sobjs, num);
	default:
	    assert (0);
	}
    } else {
	return false;
    }
}

bool  HTMLMotor::shiftOpenName (MotorObj::MotorObjVec* sobjs, const ustring& name) {
    //static uregex  re_NameOpt ("^(}%|/}%|~}%|,}%|:|\\|\\||\\?|!\\?)");
    umatch  m;
    if (usearch (begin, end, m, re_NameOpt)) {
	begin = m[0].second;
	switch (*m[0].first) {
	case '}':		// %{NAME}%
	    sobjs->push_back (new MotorObjVar (this, name, MotorObjVar::opt_normal));
	    return true;
	case '/':		// %{NAME/}%
	    sobjs->push_back (new MotorObjVar (this, name, MotorObjVar::opt_br));
	    return true;
	case '~':		// %{NAME~}%
	    sobjs->push_back (new MotorObjVar (this, name, MotorObjVar::opt_nbsp));
	    return true;
	case ',':		// %{NAME,}%
	    sobjs->push_back (new MotorObjVar (this, name, MotorObjVar::opt_c3));
	    return true;
	case ':':		// %{NAME:
	    return shiftOpenNameColon (sobjs, name);
	case '|':		// %{NAME||
	    return shiftOpenNameBar (sobjs, name);
	case '?':		// %{NAME?
	    return shiftCond (sobjs, name, false);
	case '!':		// %{NAME!?
	    return shiftCond (sobjs, name, true);
	default:
	    assert (0);
	}
    }
    return false;
}

bool  HTMLMotor::shiftOpenNum (MotorObj::MotorObjVec* sobjs, const ustring& num) {
    // %{NUM}%
    MotorObjExpand*  o = new MotorObjExpand (this, num);
    sobjs->push_back (o);
    return true;
}

bool  HTMLMotor::shiftOpenNumBar (MotorObj::MotorObjVec* sobjs, const ustring& num) {
    // %{NUM||
    AutoDelete<MotorObjCapture>  o;
    o = new MotorObjCapture (this, num);
    if (shiftCaptureElements (&o ()->objs)) {
	sobjs->push_back (o.release ());
	return true;
    } else {
	return false;
    }
}

bool  HTMLMotor::shiftOpenNameColon (MotorObj::MotorObjVec* sobjs, const ustring& name) {
    // %{NAME:
    MFTable::iterator  it;
    MFTable2::iterator  it2;
    if ((it = IMFTable.find (name)) != IMFTable.end ()) {
	AutoDelete<MotorObjFunc>  o;
	o = new MotorObjFunc (this, name, it->second->fn);
	o ()->args.push_back (new MotorObj::MotorObjVec);
	if (shiftFParams (&o ()->args, NULL)) {
	    sobjs->push_back (o.release ());
	    return true;
	} else {
	    return false;
	}
    } else if ((it2 = IMFTable2.find (name)) != IMFTable2.end ()) {
	AutoDelete<MotorObjFunc2>  o;
	o = new MotorObjFunc2 (this, name, it2->second->fn);
	o ()->args.push_back (new MotorObj::MotorObjVec);
	if (shiftFParams (&o ()->args, &o ()->arg2)) {
	    sobjs->push_back (o.release ());
	    return true;
	} else {
	    return false;
	}
    } else {
	AutoDelete<MotorObjDefun>  o;
	o = new MotorObjDefun (this, name);
	o ()->args.push_back (new MotorObj::MotorObjVec);
	if (shiftFParams (&o ()->args, NULL)) {
	    sobjs->push_back (o.release ());
	    return true;
	} else {
	    return false;
	}
    }
}

bool  HTMLMotor::shiftFParams (MotorObj::MotorObjVecVec* args, MotorObj::MotorObjVec* arg2) {
    // static uregex  re_NextCloseColonBar ("\\\\|%{|}%|:|\\|\\|");
    umatch  m;
    uiterator  start = begin;
    while (usearch (start, end, m, re_NextCloseColonBar)) {
	switch (*m[0].first) {
	case '\\':		// \x
	    shiftPrec (&args->back (), m);
	    start = begin;
	    nextChar (start, end);
	    break;
	case '%':		// %{
	    shiftPrec (&args->back (), m);
	    start = begin - 2;	// %{
	    if (shiftOpen (&args->back ())) {
		start = begin;
	    } else {
		begin = start;
		nextChar (start, end);
	    }
	    break;
	case '}':		// }%
	    shiftPrec (&args->back (), m);
	    return true;
	case ':':		// :
	    shiftPrec (&args->back (), m);
	    args->push_back (new MotorObj::MotorObjVec);
	    start = begin;
	    break;
	case '|':		// ||
	    if (arg2 == NULL) {
		start = m[0].second;
	    } else {
		shiftPrec (&args->back (), m);
		return shiftFParams2 (arg2);
	    }
	    break;
	default:
	    assert (0);
	}
    }
    return false;
}

bool  HTMLMotor::shiftFParams2 (MotorObj::MotorObjVec* arg2) {
    // static uregex  re_NextClose ("\\\\|%{|}%");
    umatch  m;
    uiterator  start = begin;
    while (usearch (start, end, m, re_NextClose)) {
	switch (*m[0].first) {
	case '\\':		// \x
	    shiftPrec (arg2, m);
	    start = begin;
	    nextChar (start, end);
	    break;
	case '%':		// %{
	    shiftPrec (arg2, m);
	    start = begin - 2;	// %{
	    if (shiftOpen (arg2)) {
		start = begin;
	    } else {
		begin = start;
		nextChar (start, end);
	    }
	    break;
	case '}':		// }%
	    shiftPrec (arg2, m);
	    return true;
	default:
	    assert (0);
	}
    }
    return false;
}

bool  HTMLMotor::shiftOpenNameBar (MotorObj::MotorObjVec* sobjs, const ustring& name) {
    // %{NAME||
    MFTable2::iterator  it2;
    if ((it2 = IMFTable2.find (name)) != IMFTable2.end ()) {
	AutoDelete<MotorObjFunc2>  o;
	o = new MotorObjFunc2 (this, name, it2->second->fn);
//	o ()->args.push_back (new MotorObj::MotorObjVec);
	if (shiftFParams2 (&o ()->arg2)) {
	    sobjs->push_back (o.release ());
	    return true;
	} else {
	    return false;
	}
    } else {
	return false;
    }
}

bool  HTMLMotor::shiftCond (MotorObj::MotorObjVec* sobjs, const ustring& name, bool neg) {
    // %{NAME?
    // %{NAME!?
    AutoDelete<MotorObjSelect>  o;
    o = new MotorObjSelect (this, name, neg);
    if (shiftCondValue (o ())) {
	sobjs->push_back (o.release ());
	return true;
    } else {
	return false;
    }
}

bool  HTMLMotor::shiftCondValue (MotorObjSelect* sel) {
    // static uregex  re_NextColon ("\\\\|%{|:");
    umatch  m;
    uiterator  start = begin;
    while (usearch (start, end, m, re_NextColon)) {
	switch (*m[0].first) {
	case '\\':		// \x
	    shiftPrec (&sel->value, m);
	    start = begin;
	    nextChar (start, end);
	    break;
	case '%':		// %{
	    shiftPrec (&sel->value, m);
	    start = begin - 2;	// %{
	    if (shiftOpen (&sel->value)) {
		start = begin;
	    } else {
		begin = start;
		nextChar (start, end);
	    }
	    break;
	case ':':		// :
	    shiftPrec (&sel->value, m);
	    return shiftCondBlock (sel);
	default:
	    assert (0);
	}
    }
    return false;
}

bool  HTMLMotor::shiftCondBlock (MotorObjSelect* sel) {
    // static uregex  re_NextCloseSelect ("\\\\|%{|}%|\\|\\|(!?\\?)?");
    umatch  m;
    uiterator  start = begin;
    while (usearch (start, end, m, re_NextCloseSelect)) {
	switch (*m[0].first) {
	case '\\':		// \x
	    shiftPrec (&sel->trueobjs, m);
	    start = begin;
	    nextChar (start, end);
	    break;
	case '%':		// %{
	    shiftPrec (&sel->trueobjs, m);
	    start = begin - 2;	// %{
	    if (shiftOpen (&sel->trueobjs)) {
		start = begin;
	    } else {
		begin = start;
		nextChar (start, end);
	    }
	    break;
	case '}':		// }%
	    shiftPrec (&sel->trueobjs, m);
	    return true;
	case '|':		// ||, ||?, ||!?
	    shiftPrec (&sel->trueobjs, m);
	    if (m[1].matched) {
		AutoDelete<MotorObjSelect>  o;
		switch (*m[1].first) {
		case '?':	// ||?
		    o = new MotorObjSelect (this, sel->variable, false);
		    break;
		case '!':	// ||!?
		    o = new MotorObjSelect (this, sel->variable, true);
		    break;
		default:
		    assert (0);
		}
		if (shiftCondValue (o ())) {
		    sel->falseobjs.push_back (o.release ());
		    return true;
		} else {
		    return false;
		}
	    } else {		// ||
		return shiftCondOtherwise (sel);
	    }
	default:
	    assert (0);
	}
    }
    return false;
}

bool  HTMLMotor::shiftCondOtherwise (MotorObjSelect* sel) {
    // static uregex  re_NextClose ("\\\\|%{|}%");
    umatch  m;
    uiterator  start = begin;
    while (usearch (start, end, m, re_NextCloseSelect)) {
	switch (*m[0].first) {
	case '\\':		// \x
	    shiftPrec (&sel->falseobjs, m);
	    start = begin;
	    nextChar (start, end);
	    break;
	case '%':		// %{
	    shiftPrec (&sel->falseobjs, m);
	    start = begin - 2;	// %{
	    if (shiftOpen (&sel->falseobjs)) {
		start = begin;
	    } else {
		begin = start;
		nextChar (start, end);
	    }
	    break;
	case '}':		// }%
	    shiftPrec (&sel->falseobjs, m);
	    return true;
	default:
	    assert (0);
	}
    }
    return false;
}

bool  HTMLMotor::shiftCaptureElements (MotorObj::MotorObjVec* sobjs) {
    // static uregex  re_OpenCloseBra ("\\\\|%{|}%");
    umatch  m;
    uiterator  start = begin;
    while (usearch (start, end, m, re_OpenCloseBra)) {
	switch (*m[0].first) {
	case '\\':
	    shiftPrec (sobjs, m);
	    start = begin;
	    nextChar (start, end);
	    break;
	case '%':		// %{
	    shiftPrec (sobjs, m);
	    start = begin - 2;	// %{
	    if (shiftOpen (sobjs)) {
		start = begin;
	    } else {
		begin = start;
		nextChar (start, end);
	    }
	    break;
	case '}':		// }%
	    shiftPrec (sobjs, m);
	    return true;
	default:
	    assert (0);
	}
    }
    return false;
}

void  HTMLMotor::shiftPrec (MotorObj::MotorObjVec* sobjs, const umatch& m) {
    if (begin != m[0].first) {
	sobjs->push_back (new MotorObjText (this, begin, m[0].first));
    }
    begin = m[0].second;
}

// ============================================================

ustring  to_string (MotorObj::MotorObjVec* v, MotorEnv* env) {
    MotorOutputString  out;
    for (int i = 0; i < v->size (); ++ i) {
	(*v)[i].out (&out, env);
    }
    return out.ans;
}

