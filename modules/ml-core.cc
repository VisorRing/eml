#include "ml-core.h"
#include "ml.h"
#include "mlenv.h"
#include "util_const.h"
#include "ustring.h"
#include "expr.h"

/*DOC:
==boolean operations==

値をブール値として評価するとき，空文字列，文字列の「0」，数値の0，NILはFALSE，それ以外をTRUEとする。

*/

/*DOC:
===null===
 (null VALUE...) -> true or false
 (is-nil VALUE...) -> true or false

*/
//#XAFUNC	null	ml_null
//#XAFUNC	is-nil	ml_null
//#XWIKIFUNC	null
//#XWIKIFUNC	is-nil
MNode*  ml_null (bool fev, MNode* cell, MlEnv* mlenv) {
    MNodePtr  rest;
    evalParams (fev, mlenv, cell, EV_ASIS, &rest);
    MNode*  args = rest ();
    MNodePtr  a;
    bool  ans = true;
    while (args && ans) {
	a = eval (args->car (), mlenv);
	ans &= isNil (a ());
	nextNode (args);
    }
    return newMNode_bool (ans);
}

/*DOC:
===not-null===
 (not-null VALUE...) -> true or false

*/
//#XAFUNC	not-null	ml_not_null
//#XAFUNC	not-nil	ml_not_null
//#XWIKIFUNC	not-null
//#XWIKIFUNC	not-nil
MNode*  ml_not_null (bool fev, MNode* cell, MlEnv* mlenv) {
    MNodePtr  rest;
    evalParams (fev, mlenv, cell, EV_ASIS, &rest);
    MNode*  args = rest ();
    MNodePtr  a;
    bool  ans = true;
    while (args && ans) {
	a = eval (args->car (), mlenv);
	ans &= ! isNil (a ());
	nextNode (args);
    }
    return newMNode_bool (ans);
}

/*DOC:
===consp===
 (consp VALUE...) -> true or false
 (is-list VALUE...) -> true or false

*/
//#XAFUNC	consp	ml_consp
//#XWIKIFUNC	consp
//#XAFUNC	is-list	ml_consp
//#XWIKIFUNC	is-list
MNode*  ml_consp (bool fev, MNode* cell, MlEnv* mlenv) {
    MNodePtr  rest;
    evalParams (fev, mlenv, cell, EV_ASIS, &rest);
    MNode*  args = rest ();
    MNodePtr  a;
    bool  ans = true;
    while (args && ans) {
	a = eval (args->car (), mlenv);
	ans &= isCons (a ());
	nextNode (args);
    }
    return newMNode_bool (ans);
}

/*DOC:
===is-string===
 (is-string VALUE...) -> true or false

*/
//#XAFUNC	is-string	ml_stringp
//#XWIKIFUNC	is-string
//#XAFUNC	stringp	ml_stringp
//#XWIKIFUNC	stringp
MNode*  ml_stringp (bool fev, MNode* cell, MlEnv* mlenv) {
    MNodePtr  rest;
    evalParams (fev, mlenv, cell, EV_ASIS, &rest);
    MNode*  args = rest ();
    MNodePtr  a;
    bool  ans = true;
    while (args && ans) {
	a = eval (args->car (), mlenv);
	ans &= isStr (a ());
	nextNode (args);
    }
    return newMNode_bool (ans);
}

/*DOC:
===is-symbol===
 (is-symbol VALUE...) -> true or false

*/
//#XAFUNC	is-symbol	ml_symbolp
//#XWIKIFUNC	is-symbol
//#XAFUNC	symbolp	ml_symbolp
//#XWIKIFUNC	symbolp
MNode*  ml_symbolp (bool fev, MNode* cell, MlEnv* mlenv) {
    MNodePtr  rest;
    evalParams (fev, mlenv, cell, EV_ASIS, &rest);
    MNode*  args = rest ();
    MNodePtr  a;
    bool  ans = true;
    while (args && ans) {
	a = eval (args->car (), mlenv);
	ans &= isSym (a ());
	nextNode (args);
    }
    return newMNode_bool (ans);
}

/*DOC:
===is-integer===
 (is-integer VALUE...) -> true or false

*/
//#XAFUNC	is-integer	ml_is_integer
//#XWIKIFUNC	is-integer
MNode*  ml_is_integer (bool fev, MNode* cell, MlEnv* mlenv) {
    MNodePtr  rest;
    evalParams (fev, mlenv, cell, EV_ASIS, &rest);
    MNode*  args = rest ();
    MNodePtr  a;
    bool  ans = true;
    while (args && ans) {
	a = eval (args->car (), mlenv);
	ans &= isInt (a ());
	nextNode (args);
    }
    return newMNode_bool (ans);
}

/*DOC:
===is-float===
 (is-float VALUE...) -> true or false

*/
//#XAFUNC	is-float	ml_is_float
//#XWIKIFUNC	is-float
MNode*  ml_is_float (bool fev, MNode* cell, MlEnv* mlenv) {
    MNodePtr  rest;
    evalParams (fev, mlenv, cell, EV_ASIS, &rest);
    MNode*  args = rest ();
    MNodePtr  a;
    bool  ans = true;
    while (args && ans) {
	a = eval (args->car (), mlenv);
	ans &= isReal (a ());
	nextNode (args);
    }
    return newMNode_bool (ans);
}

/*DOC:
===is-number===
 (is-number VALUE...) -> true or false

*/
//#XAFUNC	is-number	ml_numberp
//#XWIKIFUNC	is-number
//#XAFUNC	numberp	ml_numberp
//#XWIKIFUNC	numberp
MNode*  ml_numberp (bool fev, MNode* cell, MlEnv* mlenv) {
    MNodePtr  rest;
    evalParams (fev, mlenv, cell, EV_ASIS, &rest);
    MNode*  args = rest ();
    MNodePtr  a;
    bool  ans = true;
    while (args && ans) {
	a = eval (args->car (), mlenv);
	ans &= isIntReal (a ());
	nextNode (args);
    }
    return newMNode_bool (ans);
}

/*DOC:
===is-vector===
 (is-vector VALUE...) -> true or false

*/
//#XAFUNC	is-vector	ml_vectorp
//#XWIKIFUNC	is-vector
//#XAFUNC	vectorp	ml_vectorp
//#XWIKIFUNC	vectorp
MNode*  ml_vectorp (bool fev, MNode* cell, MlEnv* mlenv) {
    MNodePtr  rest;
    evalParams (fev, mlenv, cell, EV_ASIS, &rest);
    MNode*  args = rest ();
    MNodePtr  a;
    bool  ans = true;
    while (args && ans) {
	a = eval (args->car (), mlenv);
	ans &= isVector (a ());
	nextNode (args);
    }
    return newMNode_bool (ans);
}

/*DOC:
===is-table===
 (is-table VALUE...) -> true or false

*/
//#XAFUNC	is-table	ml_tablep
//#XWIKIFUNC	is-table
//#XAFUNC	tablep	ml_tablep
//#XWIKIFUNC	tablep
MNode*  ml_tablep (bool fev, MNode* cell, MlEnv* mlenv) {
    MNodePtr  rest;
    evalParams (fev, mlenv, cell, EV_ASIS, &rest);
    MNode*  args = rest ();
    MNodePtr  a;
    bool  ans = true;
    while (args && ans) {
	a = eval (args->car (), mlenv);
	ans &= isTable (a ());
	nextNode (args);
    }
    return newMNode_bool (ans);
}

/*DOC:
===is-bool===
 (is-bool VALUE...) -> true or false

*/
//#XAFUNC	is-bool	ml_is_bool
//#XWIKIFUNC	is-bool
MNode*  ml_is_bool (bool fev, MNode* cell, MlEnv* mlenv) {
    MNodePtr  rest;
    evalParams (fev, mlenv, cell, EV_ASIS, &rest);
    MNode*  args = rest ();
    MNodePtr  a;
    bool  ans = true;
    while (args && ans) {
	a = eval (args->car (), mlenv);
	ans &= isBool (a ());
	nextNode (args);
    }
    return newMNode_bool (ans);
}

/*DOC:
===is-lambda===
 (is-lambda VALUE...) -> true or false

*/
//#XAFUNC	is-lambda	ml_is_lambda
//#XWIKIFUNC	is-lambda
MNode*  ml_is_lambda (bool fev, MNode* cell, MlEnv* mlenv) {
    MNodePtr  rest;
    evalParams (fev, mlenv, cell, EV_ASIS, &rest);
    MNode*  args = rest ();
    MNodePtr  a;
    bool  ans = true;
    while (args && ans) {
	a = eval (args->car (), mlenv);
	ans &= isLambda (a ());
	nextNode (args);
    }
    return newMNode_bool (ans);
}

/*DOC:
===is-builtin===
 (is-builtin VALUE...) -> true or false

*/
//#XAFUNC	is-builtin	ml_is_builtin
//#XWIKIFUNC	is-builtin
MNode*  ml_is_builtin (bool fev, MNode* cell, MlEnv* mlenv) {
    MNodePtr  rest;
    evalParams (fev, mlenv, cell, EV_ASIS, &rest);
    MNode*  args = rest ();
    MNodePtr  a;
    bool  ans = true;
    while (args && ans) {
	a = eval (args->car (), mlenv);
	ans &= isBuiltin (a ());
	nextNode (args);
    }
    return newMNode_bool (ans);
}

/*DOC:
===apply===
 (apply FUNCTION-NAME VALUE... LIST) -> ANY
 (apply LAMBDA VALUE... LIST) -> ANY

*/
//#XAFUNC	apply	ml_apply
MNode*  ml_apply (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    MNodePtr  rest;
    evalParams (fev, mlenv, cell, posList, posParams, NULL, NULL, EV_LIST, &rest);
    MNodeList  list;
    MNodePtr  fn;
    fn = posParams[0] ();
    MNode*  args = rest ();
    MNode*  a;
    if (isSym (fn ())) {
	fn = eval (fn (), mlenv);
	list.append (fn ());
    } else {
	list.append (fn ());
    }
    while (args) {
	a = args->car ();
	nextNode (args);
	if (args) {
	    list.append (a);
	} else {
	    if (isNil (a)) {
	    } else if (isCons (a)) {
		while (isCons (a)) {
		    list.append (a->car ());
		    nextNode (a);
		}
	    } else {
		throw (uErrorSyntax);
	    }
	}
    }
    return eval_fn (false, fn (), list (), mlenv);
}

#if 0
/*--DOC:
===funcall===
 (funcall LAMBDA PARAMS...)

*/
////#AFUNC	funcall	ml_funcall
MNode*  ml_funcall (bool fev, MNode* cell, MlEnv* mlenv) {
    MNode*  arg = cell->cdr ();
    MNodePtr  fn;

    if (! arg)
	throw (uErrorWrongNumber);
    fn = eval (arg->car (), mlenv);
    nextNode (arg);

    if (! fn ()) {
	throw (uErrorSyntax);
    } else if (isLambda (fn ())) {
	return mlenv->retval = execDefun (mlenv, fn (), arg, uEmpty);
    } else if (isSym (fn ())) {
	MNodePtr  f;
	f = new MNode ();
	f ()->set_car (fn ());
	f ()->set_cdr (arg);
	return mlenv->retval = eval (f (), mlenv);
    } else {
	throw (uErrorSyntax);
    }
    return NULL;
}
#endif

/*DOC:
===eval===
 (eval #trace ANY) -> ANY

*/
//#XAFUNC	eval	ml_eval
MNode*  ml_eval (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    kwParam  kwList[] = {{CharConst ("trace"), EV_LIST},	// 0
			 {NULL, 0, EV_END}};
    MNodePtr  kwParams[1];
    evalParams (fev, mlenv, cell, posList, posParams, kwList, kwParams);
    if (to_bool (kwParams[0]())) {
	// #trace
	AutoBackupBool  autoBackup (&mlenv->mlPool->nolog);
	mlenv->mlPool->nolog = false;
	return eval (posParams[0](), mlenv);
    } else {
	return eval (posParams[0](), mlenv);
    }
}

