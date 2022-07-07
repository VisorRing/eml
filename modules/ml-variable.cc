#include "ml-variable.h"
#include "ml.h"
#include "mlenv.h"
#include "motorenv.h"
#include "util_const.h"
#include "util_check.h"
#include "ustring.h"
#include "expr.h"
#include <vector>
#include <exception>

/*DOC:
==variable accessing==

*/

static void  setvar (ustring name, MNode* val, MlEnv* mlenv) {
    mlenv->setVar2 (name, val);
}

static void  setvar_list_list (MNode* nameList, MNode* valList, MlEnv* mlenv) {
    MNode*  name;
    MNode*  val;
    while (isCons (nameList)) {
	name = nameList->car ();
	nextNode (nameList);
	if (isCons (valList)) {
	    val = valList->car ();
	    nextNode (valList);
	} else {
	    val = NULL;
	}
	setvar_sel (name, val, mlenv);
    }
}

static void  setvar_list_vec (MNode* nameList, MNode* valList, MlEnv* mlenv) {
    MNode*  name;
    MNode*  val;
    int  pos = 0;
    while (isCons (nameList)) {
	name = nameList->car ();
	nextNode (nameList);
	val = valList->vectorGet (pos ++);
	setvar_sel (name, val, mlenv);
    }
}

static void  setvar_vec_list (MNode* nameList, MNode* valList, MlEnv* mlenv) {
    MNode*  name;
    MNode*  val;
    int  pos;
    int  n = nameList->vectorSize ();
    for (pos = 0; pos < n; ++ pos) {
	name = nameList->vectorGet (pos);
	if (isCons (valList)) {
	    val = valList->car ();
	    nextNode (valList);
	} else {
	    val = NULL;
	}
	setvar_sel (name, val, mlenv);
    }
}

static void  setvar_vec_vec (MNode* nameList, MNode* valList, MlEnv* mlenv) {
    MNode*  name;
    MNode*  val;
    int  pos;
    int  n = nameList->vectorSize ();
    for (pos = 0; pos < n; ++ pos) {
	name = nameList->vectorGet (pos);
	val = valList->vectorGet (pos);
	setvar_sel (name, val, mlenv);
    }
}

void  setvar_sel (MNode* name, MNode* val, MlEnv* mlenv) {
    if (isSym (name)) {
	setvar (name->to_string (), val, mlenv);
    } else if (isStr (name)) {
	setvar (name->to_string (), val, mlenv);
    } else if (isCons (name)) {
	if (isCons (val)) {
	    setvar_list_list (name, val, mlenv);
	} else if (isVector (val)) {
	    setvar_list_vec (name, val, mlenv);
	} else if (isNil (val)) {
	    setvar_list_list (name, val, mlenv);
	} else {
	    throw (uErrorSyntax);
	}
    } else if (isVector (name)) {
	if (isCons (val)) {
	    setvar_vec_list (name, val, mlenv);
	} else if (isVector (val)) {
	    setvar_vec_vec (name, val, mlenv);
	} else if (isNil (val)) {
	    setvar_list_list (name, val, mlenv);
	} else {
	    throw (uErrorSyntax);
	}
    } else {
	throw (uErrorSyntax);
    }
}

static void  setLocalVar (const ustring& name, MNode* val, MlEnv* mlenv) {
    ustring  aname;
    if (checkAry (name, aname)) {
	mlenv->defineLocalVar (aname);
	mlenv->setAry (aname, val);
    } else {
	mlenv->setLocalVar (name, val);
    }
}

static void  setLocalVarRecursive (MNode* cons, MNode* val, MlEnv* mlenv) {
    if (isNil (val)) {
	while (isCons (cons)) {
	    MNode*  a = cons->car ();
	    if (isNil (a)) {
		throw (cons->dump_string_short () + ": bad type.");
	    } else if (isSym (a)) {
		mlenv->setLocalVar (*ptr_symbol (a), NULL);
	    } else if (isCons (a)) {
		setLocalVarRecursive (a, val, mlenv);
	    } else {
		throw (cons->dump_string_short () + ": bad type.");
	    }
	    nextNode (cons);
	}
    } else if (isCons (val)) {
	while (isCons (cons)) {
	    MNode*  a = cons->car ();
	    MNode*  v = isCons (val) ? val->car () : NULL;
	    if (isNil (a)) {
		throw (cons->dump_string_short () + ": bad type.");
	    } else if (isSym (a)) {
		mlenv->setLocalVar (*ptr_symbol (a), v);
	    } else if (isCons (a)) {
		setLocalVarRecursive (a, v, mlenv);
	    } else {
		throw (cons->dump_string_short () + ": bad type.");
	    }
	    nextNode (cons);
	    nextNode (val);
	}
    } else if (isVector (val)) {
	size_t  i = 0;
	while (isCons (cons)) {
	    MNode*  a = cons->car ();
	    if (isNil (a)) {
		throw (cons->dump_string_short () + ": bad type.");
	    } else if (isSym (a)) {
		mlenv->setLocalVar (*ptr_symbol (a), val->vectorGet (i));
	    } else if (isCons (a)) {
		setLocalVarRecursive (a, val->vectorGet (i), mlenv);
	    } else {
		throw (cons->dump_string_short () + ": bad type.");
	    }
	    nextNode (cons);
	    ++ i;
	}
    } else {
	throw (val->dump_string_short () + ": bad type.");
    }
}

/*DOC:
===setvar===
 (setvar VARIABLE_or_ARRAY VALUE [VARIABLE_or_ARRAY VALUE]...) -> LAST_VALUE

*/
//#XAFUNC	setvar	ml_setvar
//#XWIKIFUNC	setvar
MNode*  ml_setvar (bool fev, MNode* cell, MlEnv* mlenv) {
    // パラメータは順次評価する。先行して評価すると、変数への代入が前後する。
    MNode*  args = cell->cdr ();
    MNodePtr  name;
    MNodePtr  val;
    while (isCons (args)) {
	if (fev)
	    name = eval (args->car (), mlenv);
	else
	    name = args->car ();
	nextNode (args);
	if (isNil (args)) {
	    val = NULL;
	} else if (isCons (args)) {
	    if (fev)
		val = eval (args->car (), mlenv);
	    else
		val = args->car ();
	    nextNode (args);
	} else {
	    throw (uErrorSyntax);
	}
	setvar_sel (name (), val (), mlenv);
    }
    return mlenv->retval = val.release ();
}

/*DOC:
===storevar===
 (storevar VALUE VARIABLE) -> VALUE

*/
//#XAFUNC	storevar	ml_storevar
//#XWIKIFUNC	storevar
MNode*  ml_storevar (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_LIST, EV_END};
    MNodePtr  posParams[2];
    evalParams (fev, mlenv, cell, posList, posParams);
    setvar_sel (posParams[1](), posParams[0](), mlenv);
    return mlenv->retval = posParams[0] ();
}

/*DOC:
===let===
 (let VARLIST BODY ...) -> LAST VALUE
 VARLIST ::= (VAR_OR_LIST)
 VAR_OR_LIST ::= VARIABLE | (VARDEF VALUE) 
 VARDEF ::= VARIABLE | LIST
*/
//#XAFUNC	let	ml_let
//#XWIKIFUNC	let
MNode*  ml_let (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_ASIS, EV_END};
    MNodePtr  posParams[1];
    MNodePtr  rest;
    evalParams (fev, mlenv, cell, posList, posParams, NULL, NULL, EV_ASIS, &rest);
    MNode*  varlist = posParams[0]();
    MNodePtr  ans;
    {
	AutoLocalVariable  autoLocal (mlenv);

	if (isCons (varlist)) {
	    MNode*  v;
	    MNodePtr  p;
	    const ustring*  name;
	    MNode*  a;

	    while (varlist) {
		v = varlist->car ();
		if (v) {
		    switch (v->type) {
		    case MNode::MC_CONS:
			a = v->car ();
			if (isNil (a)) {
			    throw (v->dump_string_short () + ": bad type.");
			} else if (isSym (a)) {
			    name = ptr_symbol (a);
			    nextNode (v);
			    p = eval (v->car (), mlenv);
			    setLocalVar (*name, p (), mlenv);
			} else if (isCons (a)) {
			    nextNode (v);
			    p = eval (v->car (), mlenv);
			    setLocalVarRecursive (a, p (), mlenv);
			} else {
			    throw (v->dump_string_short () + ": bad type.");
			}
			break;
		    case MNode::MC_SYM:
			name = ptr_symbol (v);
			mlenv->setLocalVar (*name, NULL);
			break;
		    default:
			throw (v->dump_string_short () + ": bad type.");
		    }
		}
		nextNode (varlist);
	    }
	}

	ans = progn (rest (), mlenv);
	mlenv->stopBreak (cell->car ());
    }
    return mlenv->retval = ans ();
}

/*DOC:
===getvar===
 (getvar VARIABLE) -> VALUE

*/
//#XAFUNC	getvar	ml_getvar
//#XWIKIFUNC	getvar
MNode*  ml_getvar (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    evalParams (fev, mlenv, cell, posList, posParams);
    ustring  name;
    name = to_string (posParams[0]());
    return mlenv->getVar (name);
}
