#include "ml-list.h"
#include "ml.h"
#include "mlenv.h"
#include "util_const.h"
#include "ustring.h"
#include "expr.h"

/*DOC:
==list==

*/

/*DOC:
===list===
 (list LIST...) -> LIST

*/
//#XAFUNC	list	ml_list
//#XWIKIFUNC	list
MNode*  ml_list (bool fev, MNode* cell, MlEnv* mlenv) {
    MNodePtr  rest;

    evalParams (fev, mlenv, cell, NULL, NULL, NULL, NULL, EV_LIST, &rest);
    return mlenv->retval = rest.release ();
}

/*DOC:
===car===
 (car LIST) -> LIST

*/
//#XAFUNC	car	ml_car
//#XWIKIFUNC	car
MNode*  ml_car (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    evalParams (fev, mlenv, cell, posList, posParams);
    if (isCons (posParams[0]())) {
	return mlenv->retval = posParams[0]()->car ();
    } else {
	return NULL;
    }
}

/*DOC:
===cdr===
 (cdr LIST) -> LIST

*/
//#XAFUNC	cdr	ml_cdr
//#XWIKIFUNC	cdr
MNode*  ml_cdr (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    evalParams (fev, mlenv, cell, posList, posParams);
    if (isCons (posParams[0]())) {
	return mlenv->retval = posParams[0]()->cdr ();
    } else {
	return NULL;
    }
}

/*DOC:
===cons===
 (cons CAR CDR) -> CONS

*/
//#XAFUNC	cons	ml_cons
//#XWIKIFUNC	cons
MNode*  ml_cons (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_LIST, EV_END};
    MNodePtr  posParams[2];
    evalParams (fev, mlenv, cell, posList, posParams);
    return newMNode_cons (posParams[0](), posParams[1]());
}

/*DOC:
===carcdr===
 (carcdr LIST) -> (CAR CDR)

*/
//#XAFUNC	carcdr	ml_carcdr
//#XWIKIFUNC	carcdr
MNode*  ml_carcdr (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    evalParams (fev, mlenv, cell, posList, posParams);
    MNode*  a = posParams[0]();
    if (isNil (a)) {
	return NULL;
    } else if (! isCons (a)) {
	throw (uErrorWrongType);
    } else {
	return newMNode_cons (a->car (), newMNode_cons (a->cdr ()));
    }
}

/*DOC:
===append===
 (append LIST...) -> LIST

*/
//#XAFUNC	append	ml_append
//#XWIKIFUNC	append
MNode*  ml_append (bool fev, MNode* cell, MlEnv* mlenv) {
    MNodePtr  rest;
    evalParams (fev, mlenv, cell, NULL, NULL, NULL, NULL, EV_LIST, &rest);
    MNodeList  list;
    MNode*  ag = rest ();
    MNode*  a;
    while (ag) {
	a = ag->car ();
	nextNode (ag);
	if (ag) {
	    while (isCons (a)) {
		list.append (a->car ());
		nextNode (a);
	    }
	} else {
	    list.set_cdr_cut (a);
	}
    }

    return mlenv->retval = list ();
}

/*DOC:
===reverse===
 (reverse LIST) -> LIST

*/
//#XAFUNC	reverse	ml_reverse
//#XWIKIFUNC	reverse
MNode*  ml_reverse (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    evalParams (fev, mlenv, cell, posList, posParams);
    MNodePtr  ans;
    MNode*  a = posParams[0]();
    MNode*  c;
    if (! isCons (a)) {
	ans = a;
    } else {
	while (isCons (a)) {
	    c = newMNode_cons (a->car (), ans ());
	    ans = c;
	    nextNode (a);
	}
    }
    return mlenv->retval = ans.release ();
}

/*DOC:
===nth===
 (nth N LIST) -> ELEMENT
(car LIST) is (nth 0 LIST)

*/
//#XAFUNC	nth	ml_nth
//#XWIKIFUNC	nth
MNode*  ml_nth (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_LIST, EV_END};
    MNodePtr  posParams[2];

    evalParams (fev, mlenv, cell, posList, posParams);
    int64_t  n = to_int64 (posParams[0]());
    MNode*  a = posParams[1]();
    while (n > 0 && isCons (a)) {
	-- n;
	nextNode (a);
    }
    if (isCons (a))
	return mlenv->retval = a->car ();
    else
	return NULL;
}

/*DOC:
===list-get===
 (list-get LIST N) -> ELEMENT
(car LIST) is (list-get LIST 0)

*/
//#XAFUNC	list-get	ml_list_get
//#XWIKIFUNC	list-get
MNode*  ml_list_get (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_LIST, EV_END};
    MNodePtr  posParams[2];

    evalParams (fev, mlenv, cell, posList, posParams);
    MNode*  a = posParams[0]();
    int64_t  n = to_int64 (posParams[1]());
    while (n > 0 && isCons (a)) {
	-- n;
	nextNode (a);
    }
    if (isCons (a))
	return mlenv->retval = a->car ();
    else
	return NULL;
}

/*DOC:
===list-size===
 (list-size LIST) -> INTEGER
 (list-count LIST) -> INTEGER

*/
//#XAFUNC	list-size	ml_list_size
//#XWIKIFUNC	list-size
//#XAFUNC	list-count	ml_list_size
//#XWIKIFUNC	list-count
MNode*  ml_list_size (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];

    evalParams (fev, mlenv, cell, posList, posParams);
    MNode*  a = posParams[0]();
    int  n = 0;
    while (isCons (a)) {
	++ n;
	nextNode (a);
    }
    return newMNode_num (n);
}

#if 0
/*--DOC:
===replca===
 (replca CELL NEWCAR) -> NEWCAR

*/
////#AFUNC	replca	ml_replca
MNode*  ml_replca (bool fev, MNode* cell, MlEnv* mlenv) {
    MNode*  arg = cell->cdr ();
    MNodePtr  c;
    MNodePtr  newcar;

    if (! arg)
	throw (uErrorWrongNumber);
    c = eval (arg->car (), mlenv);
    nextNodeNonNil (arg);
    newcar = eval (arg->car (), mlenv);
    nextNode (arg);
    if (arg)
	throw (uErrorWrongNumber);

    if (! c ())
	throw (ustring (CharConst ("nil data.")));
    if (! isCons (c ()))
	throw (c ()->dump_string () + uErrorBadType);
    c ()->unset_car ();
    c ()->set_car (newcar ());

    return mlenv->retval = newcar ();
}
#endif
#if 0
/*--DOC:
===replcd===
 (replcd CELL NEWCDR) -> NEWCDR

*/
////#AFUNC	replcd	ml_replcd
MNode*  ml_replcd (bool fev, MNode* cell, MlEnv* mlenv) {
    MNode*  arg = cell->cdr ();
    MNodePtr  c;
    MNodePtr  newcdr;

    if (! arg)
	throw (uErrorWrongNumber);
    c = eval (arg->car (), mlenv);
    nextNodeNonNil (arg);
    newcdr = eval (arg->car (), mlenv);
    nextNode (arg);
    if (arg)
	throw (uErrorWrongNumber);

    if (! c ())
	throw (ustring (CharConst ("nil data.")));
    if (! isCons (c ()))
	throw (c ()->dump_string () + uErrorBadType);
    c ()->unset_cdr ();
    c ()->set_cdr (newcdr ());

    return mlenv->retval = newcdr ();
}
#endif

/*DOC:
===member-of===
 (member-of ELT LIST) -> BOOL
 (member-of ELT VECTOR) -> BOOL

Return 1 if LIST contaions ELT. The comparison is done by ===.

*/
//#XAFUNC	member-of	ml_member_of
//#XWIKIFUNC	member-of
MNode*  ml_member_of (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_LIST, EV_END};
    MNodePtr  posParams[2];
    evalParams (fev, mlenv, cell, posList, posParams);
    MNode*  obj = posParams[0]();
    MNode*  list = posParams[1]();
    if (isCons (list)) {
	while (isCons (list)) {
	    if (eq (obj, list->car ())) {
		return newMNode_bool (true);
	    }
	    nextNode (list);
	}
    } else if (isVector (list)) {
	int  n = list->vectorSize ();
	for (int i = 0; i < n; ++ i) {
	    if (eq (obj, list->vectorGet (i))) {
		return newMNode_bool (true);
	    }
	}
    } else {
    }
    return NULL;
}

/*DOC:
===map===
 (map LIST LAMBDA) -> LIST

*/
//#XAFUNC	map	ml_map
//#XWIKIFUNC	map
MNode*  ml_map (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_LIST, EV_END};
    MNodePtr  posParams[2];
    evalParams (fev, mlenv, cell, posList, posParams);
    if (isNil (posParams[0]()))
	return NULL;
    if (! isCons (posParams[0]()))
	throw (cell->dump_string_short () + uErrorWrongType);
    MNode*  fn = posParams[1]();
    MNode*  list = posParams[0]();
    MNode*  a;
    MNodePtr  h;
    MNodeList  ans;
    while (isCons (list)) {
	a = list->car ();
	list = list->cdr ();
	MNodeList  arg;
	arg.append (fn);
	arg.append (a);
	ans.append (eval_fn (false, fn, arg (), mlenv));
    }
    return ans.release ();
}

/*DOC:
===list-reduce===
 (list-reduce LIST INIT LAMBDA) -> VALUE

*/
//#XAFUNC	list-reduce	ml_list_reduce
//#XWIKIFUNC	list-reduce
//#XAFUNC	map-reduce	ml_list_reduce
//#XWIKIFUNC	map-reduce
MNode*  ml_list_reduce (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_LIST, EV_LIST, EV_END};
    MNodePtr  posParams[3];
    evalParams (fev, mlenv, cell, posList, posParams);
    MNode*  list = posParams[0]();
    MNodePtr  ans;
    ans = posParams[1]();
    if (isNil (list))
	return mlenv->retval = ans.release ();
    if (! isCons (list))
	throw (cell->dump_string_short () + uErrorWrongType);
    MNode*  fn = posParams[2]();
    MNode*  a;
    MNodePtr  h;
    while (isCons (list)) {
	a = list->car ();
	nextNode (list);
	MNodeList  arg;
	arg.append (fn);
	arg.append (a);
	arg.append (ans ());
	ans = eval_fn (false, fn, arg (), mlenv);
    }
    return mlenv->retval = ans.release ();
}

/*DOC:
===list-reduce-while===
 (list-reduce-while LIST INIT LAMBDA) -> VALUE
 LAMBDA :: (lambda (elem acc) ...) -> (BOOL ACC)

*/
//#XAFUNC	list-reduce-while	ml_list_reduce_while
//#XWIKIFUNC	list-reduce-while
MNode*  ml_list_reduce_while (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_LIST, EV_LIST, EV_END};
    MNodePtr  posParams[3];
    evalParams (fev, mlenv, cell, posList, posParams);
    MNode*  list = posParams[0]();
    MNodePtr  ans;
    ans = posParams[1]();
    if (isNil (list))
	return mlenv->retval = ans.release ();
    if (! isCons (list))
	throw (cell->dump_string_short () + uErrorWrongType);
    MNode*  fn = posParams[2]();
    MNode*  a;
    MNodePtr  h;
    MNodePtr  b;
    while (isCons (list)) {
	a = list->car ();
	nextNode (list);
	MNodeList  arg;
	arg.append (fn);
	arg.append (a);
	arg.append (ans ());
	ans = eval_fn (false, fn, arg (), mlenv);
	if (isCons (ans ())) {
	    b = ans ()->car ();
	    if (isCons (ans ()->cdr ())) {
		ans = ans ()->cdr ()->car ();
		if (to_bool (b ())) {
		    // true はループ
		} else {
		    break;
		}
	    } else {
		throw (ans ()->dump_string () + ": bad return value of list-reduce-while function.");
	    }
	} else {
	    throw (ans ()->dump_string () + ": bad return value of list-reduce-while function.");
	}
    }
    return mlenv->retval = ans.release ();
}

