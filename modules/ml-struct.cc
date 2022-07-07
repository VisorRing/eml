#include "ml-struct.h"
#include "ml.h"
#include "mlenv.h"
#include "motorenv.h"
#include "expr.h"
#include "util_const.h"
#include "util_string.h"
#include "ustring.h"
#include <exception>

/*DOC:
==lisp structure==

*/
/*DOC:
===quote===
 (quote LIST...) -> LIST

*/
//#XAFUNC	quote	ml_quote	nofn	norval
//#XWIKIFUNC	quote
MNode*  ml_quote (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_ASIS, EV_END};
    MNodePtr  posParams[1];

    evalParams (fev, mlenv, cell, posList, posParams);
    MNode*  a = posParams[0]();
    if (a == NULL) {
	return NULL;
    } else {
	switch (posParams[0]()->type) {
	case MNode::MC_NIL:
	    return NULL;
	default:
	    return posParams[0].release ();
	}
    }
}

/*DOC:
===if===
 (if EXPR THEN_FUNCTION ELSE_FUNCTION_BODY...) -> LAST VALUE

*/
//#XAFUNC	if	ml_if
//#XWIKIFUNC	if
MNode*  ml_if (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_ASIS, EV_END};
    MNodePtr  posParams[2];
    MNodePtr  rest;
    evalParams (fev, mlenv, cell, posList, posParams, NULL, NULL, EV_ASIS, &rest);
    if (mlenv->breaksym ())
	return mlenv->breakval ();
    if (to_bool (posParams[0] ())) {
	return eval (posParams[1] (), mlenv);
    } else {
	return progn (rest (), mlenv);
// ifはbreakの対象でない		mlenv->stopBreak (cell->car ());
    }
}

/*DOC:
===cond===
 (cond (EXPR BODY...)...) -> LAST VALUE

*/
//#XAFUNC	cond	ml_cond
//#XWIKIFUNC	cond
MNode*  ml_cond (bool fev, MNode* cell, MlEnv* mlenv) {
    MNode*  arg = cell->cdr ();
    MNodePtr  ans;
    MNode*  a;
    bool  f;

    while (arg && ! mlenv->breaksym ()) {
	a = arg->car ();
	if (a == NULL)
	    throw (uNil + uErrorBadType);
	if (! isCons (a))
	    throw (a->dump_string () + uErrorBadType);
	f = eval_bool (a->car (), mlenv);
	if (mlenv->breaksym ()) {
	    ans = mlenv->breakval ();
// condはbreakの対象ではない	    mlenv->stopBreak (cell->car ());
	    break;
	}
	if (f) {
	    ans = progn (a->cdr (), mlenv);
// condはbreakの対象ではない	    mlenv->stopBreak (cell->car ());
	    break;
	}
	nextNode (arg);
    }

    return mlenv->retval = ans.release ();
}

/*DOC:
===progn===
 (progn [:on-error FUNCTION] BODY...) -> LAST VALUE

*/
//#XAFUNC	progn	ml_progn
//#XWIKIFUNC	progn
MNode*  ml_progn (bool fev, MNode* cell, MlEnv* mlenv) {
    kwParam  kwList[] = {{CharConst ("on-error"), EV_LIST},
			 {NULL, 0, EV_END}};
    MNodePtr  kwParams[1];
    MNodePtr  rest;
    MNodePtr  ans;

    evalParams (fev, mlenv, cell, NULL, NULL, kwList, kwParams, EV_ASIS, &rest);
    try {
	ans = progn (rest (), mlenv);
    } catch (ustring& msg) {
	if (kwParams[0]()) {
	    onErrorFn (kwParams[0](), mlenv);
	} else {
	    throw (msg);
	}
    }
    mlenv->stopBreak (cell->car ());
    return mlenv->retval = ans.release ();
}

/*DOC:
===while===
 (while EXPR :limit INTEGER BODY...) -> LAST VALUE

*/
//#XAFUNC	while	ml_while
MNode*  ml_while (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_ASIS, EV_END};
    MNodePtr  posParams[1];
    kwParam  kwList[] = {{CharConst ("limit"), EV_LIST},		// 0
			 {NULL, 0, EV_END}};
    MNodePtr  kwParams[1];
    MNodePtr  rest;
    evalParams (fev, mlenv, cell, posList, posParams, kwList, kwParams, EV_ASIS, &rest);
    int64_t  limit = to_int64 (kwParams[0]());
    MNodePtr  ans;
    while (eval_bool (posParams[0](), mlenv)) {
	ans = progn (rest (), mlenv);
	if (mlenv->breaksym ()) {
	    mlenv->stopBreak (cell->car ());
	    break;
	}
	if (limit > 0) {
	    -- limit;
	    if (limit == 0)
		return mlenv->retval = ans.release ();
	}
    }
    return mlenv->retval = ans.release ();
}

/*DOC:
===break===
 (break FUNCTION-NAME VALUE) -> NULL

*/
//#XAFUNC	break	ml_break
//#XWIKIFUNC	break
MNode*  ml_break (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_LIST, EV_END};
    MNodePtr  posParams[2];
    evalParams (fev, mlenv, cell, posList, posParams);
    mlenv->setBreak (posParams[0].release (), posParams[1] ());
    return mlenv->retval = posParams[1].release ();
}

/*DOC:
===exit===
 (exit) -> NULL

*/
//#XAFUNC	exit	ml_exit
MNode*  ml_exit (bool fev, MNode* cell, MlEnv* mlenv) {
    evalParams (fev, mlenv, cell);
    mlenv->breakProg ();
    return NULL;
}

/*DOC:
===pipe===
 (pipe BODY1 BODY ... ) -> VALUE

*/
//#XAFUNC	pipe	ml_pipe
//#XWIKIFUNC	pipe
MNode*  ml_pipe (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_ASIS, EV_END};
    MNodePtr  posParams[1];
    MNodePtr  rest;
    evalParams (fev, mlenv, cell, posList, posParams, NULL, NULL, EV_ASIS, &rest);
    MNode*  body = rest ();
    MNodePtr  val;
    MNodePtr  op;
    val = posParams[0] ();
    while (body) {
	MNode*  op0 = body->car ();
	if (isCons (op0)) {
	    val = newMNode_cons (op0->car (),
				 newMNode_cons (val (), op0->cdr ()));
	} else if (isSym (op0)
		   && op0->sym->length () > 0
		   && ((*op0->sym)[0] == '#' || (*op0->sym)[0] == ':')) {
	    throw (to_string (op0) + uErrorBadParam);
	} else {
	    val = newMNode_cons (newMNode_sym (new ustring ("progn")),
				 newMNode_cons (val (),
						(newMNode_cons (op0))));
	}
	nextNode (body);
    }
    return mlenv->retval = eval (val (), mlenv);
}
