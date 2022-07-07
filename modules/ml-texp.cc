#include "ml-texp.h"
#include "ml.h"
#include "mlenv.h"
#include "motorenv.h"
#include "util_const.h"
#include "util_check.h"
#include "ustring.h"
#include "expr.h"
#include <exception>

/*DOC:
==vector manupilation==

*/
/*DOC:
===vector===
 (vector OBJ ...) -> VECTOR

*/
//#XAFUNC	vector	ml_vector
//#XWIKIFUNC	vector
MNode*  ml_vector (bool fev, MNode* cell, MlEnv* mlenv) {
    MNodePtr  rest;
    evalParams (fev, mlenv, cell, EV_LIST, &rest);
    MNode*  args = rest ();
    MNodePtr  ans;
    ans = newMNode_vector ();
    while (args) {
	ans ()->vectorPush (args->car ());
	nextNode (args);
    }
    return mlenv->retval = ans.release ();
}

/*DOC:
===vector-size===
 (vector-size VECTOR) -> INTEGER

VECTORがnilのとき、0を返す。

*/
//#XAFUNC	vector-size	ml_vector_size
//#XWIKIFUNC	vector-size
MNode*  ml_vector_size (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    evalParams (fev, mlenv, cell, posList, posParams);
    MNode*  vec = posParams[0]();
    if (isNil (vec))
	return newMNode_num (0);
    else if (! isVector (vec))
	throw (uErrorWrongType);
    else
	return newMNode_num (vec->vectorSize ());
}

/*DOC:
===vector-get===
 (vector-get VECTOR N) -> VALUE

インデックスに要素がない場合、nilを返す。VECTORがnilのとき、nilを返す。

*/
//#XAFUNC	vector-get	ml_table_get
//#XWIKIFUNC	vector-get

/*DOC:
===vector-head===
 (vector-head VECTOR) -> VALUE

VECTORがnilのとき、nilを返す。

*/
//#XAFUNC	vector-head	ml_vector_head
//#XWIKIFUNC	vector-head
MNode*  ml_vector_head (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    evalParams (fev, mlenv, cell, posList, posParams);
    MNode*  vec = posParams[0]();
    if (isNil (vec))
	return NULL;
    else if (! isVector (vec))
	throw (uErrorWrongType);
    else
	return mlenv->retval = vec->vectorGet (0);
}

/*DOC:
===vector-back===
 (vector-back VECTOR) -> VALUE

VECTORがnilのとき、nilを返す。

*/
//#XAFUNC	vector-back	ml_vector_back
//#XWIKIFUNC	vector-back
MNode*  ml_vector_back (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    evalParams (fev, mlenv, cell, posList, posParams);
    MNode*  vec = posParams[0]();
    if (isNil (vec))
	return NULL;
    else if (! isVector (vec))
	throw (uErrorWrongType);
    else
	return mlenv->retval = vec->vectorGet (vec->vectorSize () - 1);
}

/*DOC:
===vector-put===
 (vector-put VECTOR N VALUE) -> VECTOR

VECTORがnilのとき、新たに作成したベクトルを返す。

*/
//#XAFUNC	vector-put	ml_vector_put
//#XWIKIFUNC	vector-put
MNode*  ml_vector_put (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_LIST, EV_LIST, EV_END};
    MNodePtr  posParams[3];
    evalParams (fev, mlenv, cell, posList, posParams);
    MNode*  vec = posParams[0]();
    int64_t  n = to_int64 (posParams[1]());
    MNode*  e = posParams[2]();
    if (isNil (vec))
	posParams[0] = vec = newMNode_vector ();
    else if (! isVector (vec))
	throw (uErrorWrongType);
    vec->vectorPut (n, e);
    return mlenv->retval = vec;
}

/*DOC:
===vector-del===
 (vector-del VECTOR FROM [TO]) -> VECTOR

VECTORがnilのとき、[]を返す。

*/
//#XAFUNC	vector-del	ml_vector_del
//#XWIKIFUNC	vector-del
MNode*  ml_vector_del (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_LIST, EV_LIST, EV_END};
    MNodePtr  posParams[3];
    evalParams (fev, mlenv, cell, posList, posParams);

    MNode*  vec = posParams[0]();
    MNode*  fromobj = posParams[1]();
    MNode*  toobj = posParams[2]();

    if (isNil (vec))
	return newMNode_vector ();
    else if (! isVector (vec))
	throw (uErrorWrongType);

    if (isNil (toobj)) {
	vec->vectorDel (to_int64 (fromobj));
    } else {
	vec->vectorDel (to_int64 (fromobj), to_int64 (toobj));
    }
    return mlenv->retval = vec;
}

/*DOC:
===vector-unshift===
 (vector-unshift VECTOR VALUE) -> VECTOR

VECTORがnilのとき、新たに作成したベクトルを返す。

*/
//#XAFUNC	vector-unshift	ml_vector_unshift
//#XWIKIFUNC	vector-unshift
MNode*  ml_vector_unshift (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_LIST, EV_END};
    MNodePtr  posParams[2];
    evalParams (fev, mlenv, cell, posList, posParams);
    MNode*  vec = posParams[0]();
    MNode*  e = posParams[1]();
    if (isNil (vec))
	posParams[0] = vec = newMNode_vector ();
    else if (! isVector (vec))
	throw (uErrorWrongType);
    vec->vectorUnshift (e);
    return mlenv->retval = vec;
}

/*DOC:
===vector-shift===
 (vector-shift VECTOR) -> (VECTOR VALUE)

VECTORがnilのとき、(nil nil)を返す。

*/
//#XAFUNC	vector-shift	ml_vector_shift
//#XWIKIFUNC	vector-shift
MNode*  ml_vector_shift (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    evalParams (fev, mlenv, cell, posList, posParams);
    MNode*  vec = posParams[0]();
    if (isNil (vec)) {
	MNodeList  ans;
	ans.append (NULL);
	ans.append (NULL);
	return mlenv->retval = ans.release ();
    } else if (! isVector (vec))
	throw (uErrorWrongType);
    MNode*  e = vec->vectorShift ();
    return newMNode_cons (vec, newMNode_cons (e));
}

/*DOC:
===vector-push===
 (vector-push VECTOR VALUE) -> VECTOR

VECTORがnilのとき、新たに作成したベクトルを返す。

*/
//#XAFUNC	vector-push	ml_vector_push
//#XWIKIFUNC	vector-push
MNode*  ml_vector_push (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_LIST, EV_END};
    MNodePtr  posParams[2];
    evalParams (fev, mlenv, cell, posList, posParams);
    MNode*  vec = posParams[0]();
    MNode*  e = posParams[1]();
    if (isNil (vec))
	posParams[0] = vec = newMNode_vector ();
    else if (! isVector (vec))
	throw (uErrorWrongType);
    vec->vectorPush (e);
    return mlenv->retval = vec;
}

/*DOC:
===vector-pop===
 (vector-pop VECTOR) -> (VECTOR VALUE)

VECTORがnilのとき、(nil nil)を返す。

*/
//#XAFUNC	vector-pop	ml_vector_pop
//#XWIKIFUNC	vector-pop
MNode*  ml_vector_pop (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    evalParams (fev, mlenv, cell, posList, posParams);
    MNode*  vec = posParams[0]();
    if (isNil (vec)) {
	MNodeList  ans;
	ans.append (NULL);
	ans.append (NULL);
	return mlenv->retval = ans.release ();
    } if (! isVector (vec))
	throw (uErrorWrongType);
    MNode*  e = vec->vectorPop ();
    return newMNode_cons (vec, newMNode_cons (e));
}

/*DOC:
===vector-append===
 (vector-append VECTOR1 VECTOR2...) -> VECTOR

VECTOR1がnilのとき、新たに作成したベクトルを返す。

*/
//#XAFUNC	vector-append	ml_vector_append
//#XWIKIFUNC	vector-append
MNode*  ml_vector_append (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    MNodePtr  rest;
    evalParams (fev, mlenv, cell, posList, posParams, NULL, NULL, EV_LIST, &rest);
    MNode*  vec = posParams[0]();
    if (isNil (vec))
	posParams[0] = vec = newMNode_vector ();
    else if (! isVector (vec))
	throw (uErrorWrongType);
    MNode*  args = rest ();
    while (args) {
	MNode*  e = args->car ();
	nextNode (args);
	if (isNil (e)) {
	} else if (isVector (e)) {
	    int  n = e->vectorSize ();
	    for (int i = 0; i < n; ++ i) {
		vec->vectorPush (e->vectorGet (i));
	    }
	} else {
	    throw (uErrorWrongType);
	}
    }
    return mlenv->retval = vec;
}

/*DOC:
===vector-resize===
 (vector-resize VECTOR N) -> VECTOR

VECTORがnilのとき、nilを返す。

*/
//#XAFUNC	vector-resize	ml_vector_resize
//#XWIKIFUNC	vector-resize
MNode*  ml_vector_resize (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_LIST, EV_END};
    MNodePtr  posParams[2];
    evalParams (fev, mlenv, cell, posList, posParams);
    MNode*  vec = posParams[0]();
    int64_t  n = to_int64 (posParams[1]());
    if (isNil (vec))
	return NULL;
    else if (! isVector (vec))
	throw (uErrorWrongType);
    vec->vectorResize (n);
    return mlenv->retval = vec;
}

/*DOC:
===vector-index===
 (vector-index VECTOR OBJECT) -> INTEGER \| nil

*/
//#XAFUNC	vector-index	ml_vector_index
//#XWIKIFUNC	vector-index
MNode*  ml_vector_index (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_LIST, EV_END};
    MNodePtr  posParams[2];
    evalParams (fev, mlenv, cell, posList, posParams);
    MNode*  vec = posParams[0]();
    MNode*  obj = posParams[1]();
    if (isNil (vec)) {
	return NULL;
    } else if (vec->isVector ()) {
	size_t  n = vec->vectorSize ();
	for (size_t i = 0; i < n; ++ i) {
	    MNode*  e = vec->vectorGet (i);
	    if (equal (obj, e)) {
		return newMNode_int64 (i);
	    }
	}
	return NULL;
    } else {
	throw (uErrorWrongType);
    }
}

/*DOC:
===vector-slice===
 (vector-slice VECTOR START END) -> VECTOR

*/
//#XAFUNC	vector-slice	ml_vector_slice
//#XWIKIFUNC	vector-slice
MNode*  ml_vector_slice (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_LIST, EV_LIST, EV_END};
    MNodePtr  posParams[3];
    evalParams (fev, mlenv, cell, posList, posParams);
    MNode*  vec = posParams[0]();
    size_t  istart = to_int64 (posParams[1]());
    size_t  iend;
    if (isNil (vec)) {
	return NULL;
    } else if (vec->isVector ()) {
	if (posParams[2]()) {
	    iend = to_int64 (posParams[2]()) + 1;
	    if (iend > vec->vectorSize ())
		iend = vec->vectorSize ();
	} else {
	    iend = vec->vectorSize ();
	}
	MNodePtr  ans;
	ans = newMNode_vector ();
	for (size_t i = istart; i < iend; ++ i) {
	    ans ()->vectorPush (vec->vectorGet (i));
	}
	return mlenv->retval = ans.release ();
    } else {
	throw (uErrorWrongType);
    }
}

/*DOC:
===list-to-vector===
 (list-to-vector LIST) -> VECTOR

*/
//#XAFUNC	list-to-vector	ml_list_to_vector
//#XWIKIFUNC	list-to-vector
MNode*  ml_list_to_vector (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    evalParams (fev, mlenv, cell, posList, posParams);
    MNode*  e = posParams[0]();
    if (isNil (e)) {
    } else if (! isCons (e)) {
	throw (uErrorWrongType);
    }
    return list_to_vector (e);
}

/*DOC:
===vector-to-list===
 (vector-to-list VECTOR) -> LIST

*/
//#XAFUNC	vector-to-list	ml_vector_to_list
//#XWIKIFUNC	vector-to-list
MNode*  ml_vector_to_list (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    evalParams (fev, mlenv, cell, posList, posParams);
    MNode*  e = posParams[0]();
    MNodeList  ans;
    if (isNil (e))
	return NULL;
    if (! isVector (e))
	throw (uErrorWrongType);
    MotorVector::iterator  b = e->value_vector ()->begin ();
    MotorVector::iterator  t = e->value_vector ()->end ();
    for (; b < t; ++ b) {
	ans.append ((*b) ());
    }
    return mlenv->retval = ans.release ();
}

/*DOC:
===vector-to-set===
 (vector-to-set VECTOR) -> TABLE

*/
//#XAFUNC	vector-to-set	ml_vector_to_set
//#XWIKIFUNC	vector-to-set
MNode*  ml_vector_to_set (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    evalParams (fev, mlenv, cell, posList, posParams);
    MNode*  e = posParams[0]();
    MNodePtr  ans;
    ans = newMNode_table ();
    if (isNil (e))
	return NULL;
    if (! isVector (e))
	throw (uErrorWrongType);
    MotorVector::iterator  b = e->value_vector ()->begin ();
    MotorVector::iterator  t = e->value_vector ()->end ();
    for (; b < t; ++ b) {
	ans ()->tablePut (to_string ((*b)()), newMNode_bool (true));
    }
    return mlenv->retval = ans.release ();
}

/*DOC:
===vector-eval===
 (vector-eval VECTOR) -> VECTOR

*/
//#XAFUNC	vector-eval	ml_vector_eval
MNode*  ml_vector_eval (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    evalParams (fev, mlenv, cell, posList, posParams);
    MNode*  vec = posParams[0]();
    if (isNil (vec))
	return newMNode_vector ();
    if (! isVector (vec)) {
	throw (uErrorWrongType);
    }
    return vectorEval (vec, mlenv);
}

/*DOC:
===vector-each===
 (vector-each VARIABLE VECTOR [:index VARIABLE2] BODY...) -> LAST VALUE
 (vector-each '(VARIABLE1...) '(VECTOR...) [:index VARIABLE2] BODY...) -> LAST VALUE

VECTORがnilのとき、なにもしない。

*/
//#XAFUNC	vector-each	ml_vector_each
//#XWIKIFUNC	vector-each
MNode*  ml_vector_each (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_LIST, EV_END};
    MNodePtr  posParams[2];
    kwParam  kwList[] = {{CharConst ("index"), EV_LIST}, // 0
			 {NULL, 0, EV_END}};
    MNodePtr  kwParams[1];
    MNodePtr  rest;
    evalParams (fev, mlenv, cell, posList, posParams, kwList, kwParams, EV_ASIS, &rest);
    std::vector<ustring>  lv;
    size_t  it, iu;
    MNode*  vlist = posParams[0]();
    MNode*  list = posParams[1]();
    ustring  iv;
    std::vector<MNode*>  llv;
    MNodePtr  ans;
    MNodePtr  h;
    size_t  i, n;
    if (isNil (list)) {
	return NULL;
    } else if (isSym (vlist)) {
	lv.push_back (vlist->to_string ());
	iu = lv.size ();
	if (isVector (list)) {
	    llv.push_back (list);
	} else if (isNil (list)) {
	    llv.push_back (NULL);
	} else {
	    throw (uErrorWrongType);
	}
    } else if (isCons (vlist)) {
	while (isCons (vlist)) {
	    lv.push_back (to_string (vlist->car ()));
	    nextNode (vlist);
	}
	iu = lv.size ();
	for (i = 0; i < iu; i ++) {
	    if (list) {
		MNode*  e = list->car ();
		if (isVector (e)) {
		    llv.push_back (e);
		} else if (isNil (e)) {
		    llv.push_back (NULL);
		} else {
		    throw (uErrorWrongType);
		}
	    } else {
		llv.push_back (NULL);
	    }
	    nextNode (list);
	}
    } else {
	throw (to_string (vlist) + ustring (": bad argument."));
    }
    if (! isNil (kwParams[0]()))
	iv = to_string (kwParams[0]());
    
    {
	AutoLocalVariable  autoLocal (mlenv);

	for (it = 0; it < iu; it ++)
	    mlenv->defineLocalVar (lv[it]);
	if (iv.length () > 0)
	    mlenv->defineLocalVar (iv);

	if (iu > 0 && llv.size () > 0) {
	    if (llv[0]) {
		n = llv[0]->vectorSize ();
	    } else {
		n = 0;
	    }
	    for (i = 0; i < n; ++ i) {
		if (iv.length () > 0) {
		    mlenv->setVar (iv, newMNode_num (i));
		}
		for (it = 0; it < iu; it ++) {
		    if (llv[it])
			mlenv->setVar (lv[it], llv[it]->vectorGet (i));
		    else
			mlenv->setVar (lv[it], NULL);
		}
		ans = progn (rest (), mlenv);
		if (mlenv->breaksym ()) {
		    mlenv->stopBreak (cell->car ());
		    break;
		}
	    }
	}
    }

    return mlenv->retval = ans.release ();
}

/*DOC:
===vector-map===
 (vector-map VECTOR LAMBDA) -> VECTOR
 	LAMBDA: (lambda (e) ...) -> VALUE

VECTORがnilのとき、[]を返す。

*/
//#XAFUNC	vector-map	ml_vector_map
//#XWIKIFUNC	vector-map
MNode*  ml_vector_map (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_LIST, EV_END};
    MNodePtr  posParams[2];
    evalParams (fev, mlenv, cell, posList, posParams);
    MNode*  vec = posParams[0]();
    MNode*  lambda = posParams[1]();
    if (isNil (vec))
	return newMNode_vector ();
    else if (! isVector (vec))
	throw (uErrorWrongType);
    MNodePtr  ans;
    ans = newMNode_vector ();
    size_t  it, iu;
    iu = vec->vectorSize ();
    for (it = 0; it < iu; ++ it) {
	MNodeList  o;
	o.append (lambda);
	o.append (vec->vectorGet (it));
	ans ()->vectorPush (eval_fn (false, lambda, o (), mlenv));
    }
    return mlenv->retval = ans.release ();
}

/*DOC:
===vector-reduce===
 (vector-reduce VECTOR INIT LAMBDA) -> ACC
 	LAMBDA: (lambda (e acc) ...) -> ACC

VECTORがnilのとき、INITを返す。

*/
//#XAFUNC	vector-reduce	ml_vector_reduce
//#XWIKIFUNC	vector-reduce
MNode*  ml_vector_reduce (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_LIST, EV_LIST, EV_END};
    MNodePtr  posParams[3];
    evalParams (fev, mlenv, cell, posList, posParams);
    MNode*  vec = posParams[0]();
    MNodePtr  acc;
    acc = posParams[1]();
    MNode*  lambda = posParams[2]();
    if (isNil (vec))
	return mlenv->retval = acc ();
    else if (! isVector (vec))
	throw (uErrorWrongType);
    size_t  it, iu;
    iu = vec->vectorSize ();
    for (it = 0; it < iu; ++ it) {
	MNodeList  o;
	o.append (lambda);
	o.append (vec->vectorGet (it));
	o.append (acc ());
	acc = eval_fn (false, lambda, o (), mlenv);
    }
    return mlenv->retval = acc.release ();
}

/*DOC:
===vector-reduce-while===
 (vector-reduce-while VECTOR INIT LAMBDA) -> ACC
 	LAMBDA: (lambda (e acc) ...) -> (BOOL ACC)

VECTORがnilのとき、INITを返す。

*/
//#XAFUNC	vector-reduce-while	ml_vector_reduce_while
//#XWIKIFUNC	vector-reduce-while
MNode*  ml_vector_reduce_while (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_LIST, EV_LIST, EV_END};
    MNodePtr  posParams[3];
    evalParams (fev, mlenv, cell, posList, posParams);
    MNode*  vec = posParams[0]();
    MNodePtr  acc;
    MNodePtr  q;
    acc = posParams[1]();
    MNode*  lambda = posParams[2]();
    if (isNil (vec))
	return mlenv->retval = acc ();
    else if (! isVector (vec))
	throw (uErrorWrongType);
    size_t  it, iu;
    iu = vec->vectorSize ();
    for (it = 0; it < iu; ++ it) {
	MNodeList  o;
	o.append (lambda);
	o.append (vec->vectorGet (it));
	o.append (acc ());
	acc = eval_fn (false, lambda, o (), mlenv);
	if (isCons (acc ())) {
	    q = acc ()->car ();
	    if (isCons (acc ()->cdr ())) {
		acc = acc ()->cdr ()->car ();
		if (to_bool (q ())) {
		    // true はループ
		} else {
		    break;
		}
	    } else {
		throw (acc ()->dump_string () + ": bad return value of vector-reduce-while function.");
	    }
	} else {
	    throw (acc ()->dump_string () + ": bad return value of vector-reduce-while function.");
	}
    }
    return mlenv->retval = acc.release ();
}

/*DOC:
===vector-sort-string===
 (vector-sort-string VECTOR [#asc] [#desc]) -> VECTOR

*/
//#XAFUNC	vector-sort-string	ml_vector_sort_string
//#XWIKIFUNC	vector-sort-string
MNode*  ml_vector_sort_string (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    kwParam  kwList[] = {{CharConst ("asc"), EV_LIST},
			 {CharConst ("desc"), EV_LIST},
			 {NULL, 0, EV_END}};
    MNodePtr  kwParams[2];
    evalParams (fev, mlenv, cell, posList, posParams, kwList, kwParams);
    bool  fdesc = false;
    if (to_bool (kwParams[0]()))
	fdesc = false;
    if (to_bool (kwParams[1]()))
	fdesc = true;
    MNodePtr  ans;
    ans = newMNode_vector ();
    MotorVector*  vec = ans ()->value_vector ();
    MNode*  a = posParams[0]();
    if (isNil (a))
	return newMNode_vector ();
    else if (! isVector (a))
	throw (uErrorWrongType);

    size_t  it, iu;
    iu = a->vectorSize ();
    for (it = 0; it < iu; ++ it) {
	MNode*  e = a->vectorGet (it);
	if (isStr (e)) {
	    vec->push (e);
	} else {
	    vec->push (NULL);
	}
    }

    int  s, i, j, k;
    int  n = iu;
    for (i = 1; i < n; i ++) {
	j = i;
	while (j > 0) {
	    k = (j - 1) / 2;
	    if (! vec->get (k))
		if (! vec->get (j))
		    break;
		else
		    if (fdesc)
			break;
		    else ;
	    else if (! vec->get (j))
		if (fdesc)
		    ;
		else
		    break;
	    else if (fdesc ^ (*vec->get (k)->value_str () >= *vec->get (j)->value_str ()))
		break;
//	    swap (v[k], v[j]);
	    a = vec->get (j); vec->put (j, vec->get (k)); vec->put (k, a);
	    j = k;
	}
    }
    for (; n > 0; n --) {
//	swap (v[0], v[n - 1]);
	a = vec->get (n - 1); vec->put (n - 1, vec->get (0)); vec->put (0, a);
	for (i = 1; i < n - 1; i ++) {
	    j = i;
	    while (j > 0) {
		k = (j - 1) / 2;
////		if (! list[k] || ! list[j])
////		    break;
		if (! vec->get (k))
		    if (! vec->get (j))
			break;
		    else
			if (fdesc)
			    break;
			else ;
		else if (! vec->get (j))
		    if (fdesc)
			;
		    else
			break;
		else if (fdesc ^ (*vec->get (k)->value_str () >= *vec->get (j)->value_str ()))
		    break;
//		swap (v[k], v[j]);
		a = vec->get (j); vec->put (j, vec->get (k)); vec->put (k, a);
		j = k;
	    }
	}
    }

    return mlenv->retval = ans.release ();
}

/*DOC:
==table manupilation==

*/
/*DOC:
===table===
 (table CONS ...) -> TABLE

*/
//#XAFUNC	table	ml_table
//#XWIKIFUNC	table
MNode*  ml_table (bool fev, MNode* cell, MlEnv* mlenv) {
    MNodePtr  rest;
    evalParams (fev, mlenv, cell, EV_LIST, &rest);
    MNodePtr  ans;
    MNode*  args = rest ();

    ans = newMNode_table ();
    while (args) {
	MNode*  v = args->car ();
	nextNode (args);
	if (isCons (v)) {
	    ans ()->tablePut (to_string (v->car ()), v->cdr ());
	} else if (isNil (v)) {
	} else {
	    throw (uErrorWrongType);
	}
    }
    return mlenv->retval = ans.release ();
}

/*DOC:
===table-get===
 (table-get TABLE NAME [NAME | N]...) -> VALUE

*/
//#XAFUNC	table-get	ml_table_get
//#XWIKIFUNC	table-get
MNode*  ml_table_get (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    MNodePtr  rest;
    evalParams (fev, mlenv, cell, posList, posParams, NULL, NULL, EV_LIST, &rest);
    MNode*  tbl = posParams[0]();
    MNode*  args = rest ();
    while (args) {
	MNode*  e = args->car ();
	nextNode (args);
	if (isNil (tbl)) {
	    return NULL;
	} else if (isTable (tbl)) {
	    tbl = tbl->tableGet (to_string (e));
	} else if (isVector (tbl)) {
	    tbl = tbl->vectorGet (to_int64 (e));
	} else {
	    throw (uErrorWrongType);
	}
    }

    return mlenv->retval = tbl;
}

/*DOC:
===table-put===
 (table-put TABLE NAME VALUE) -> TABLE

TABLEがnilのとき、テーブルを新規作成する。

*/
//#XAFUNC	table-put	ml_table_put
//#XWIKIFUNC	table-put
MNode*  ml_table_put (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_LIST, EV_LIST, EV_END};
    MNodePtr  posParams[3];
    evalParams (fev, mlenv, cell, posList, posParams);
    MNode*  tbl = posParams[0]();
    ustring  name = to_string (posParams[1]());
    MNode*  e = posParams[2]();
    if (isNil (tbl))
	posParams[0] = tbl = newMNode_table ();
    else if (! isTable (tbl))
	throw (uErrorWrongType);
    tbl->tablePut (name, e);
    return mlenv->retval = tbl;
}

/*DOC:
===table-del===
 (table-del TABLE NAME) -> TABLE

TABLEがnilのとき、{}を返す。

*/
//#XAFUNC	table-del	ml_table_del
//#XWIKIFUNC	table-del
MNode*  ml_table_del (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_LIST, EV_END};
    MNodePtr  posParams[2];
    evalParams (fev, mlenv, cell, posList, posParams);
    MNode*  tbl = posParams[0]();
    ustring  name = to_string (posParams[1]());
    if (isNil (tbl))
	return newMNode_table ();
    else if (! isTable (tbl))
	throw (uErrorWrongType);
    tbl->tableDel (name);
    return mlenv->retval = tbl;
}

/*DOC:
===table-append===
 (table-append TABLE1 TABLE2...) -> TABLE1

TABLE1がnilのとき、テーブルを新規作成する。

*/
//#XAFUNC	table-append	ml_table_append
//#XWIKIFUNC	table-append
MNode*  ml_table_append (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    MNodePtr  rest;
    evalParams (fev, mlenv, cell, posList, posParams, NULL, NULL, EV_LIST, &rest);
    MNode*  tbl = posParams[0]();
    if (isNil (tbl))
	posParams[0] = tbl = newMNode_table ();
    else if (! isTable (tbl))
	throw (uErrorWrongType);
    MNode*  args = rest ();
    while (args) {
	MNode*  e = args->car ();
	nextNode (args);
	if (isNil (e)) {
	} else if (isTable (e)) {
	    MotorVar::iterator  b = e->value_table ()->begin ();
	    MotorVar::iterator  t = e->value_table ()->end ();
	    for (; b != t; ++ b) {
		tbl->tablePut ((*b).first, (*b).second ());
	    }
	} else {
	    throw (uErrorWrongType);
	}
    }

    return mlenv->retval = tbl;
}

/*DOC:
===table-keys===
 (table-keys TABLE) -> VECTOR

TABLEがnilのとき、[]を返す。

*/
//#XAFUNC	table-keys	ml_table_keys
//#XWIKIFUNC	table-keys
MNode*  ml_table_keys (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    evalParams (fev, mlenv, cell, posList, posParams);
    MNode*  tbl = posParams[0]();
    if (isNil (tbl)) {
	return newMNode_vector ();
    } else if (! isTable (tbl)) {
	throw (uErrorWrongType);
    }
    MotorVar::iterator  b = tbl->value_table ()->begin ();
    MotorVar::iterator  t = tbl->value_table ()->end ();
    MNodePtr  ans;
    ans = newMNode_vector ();
    for (; b != t; ++ b) {
	ans ()->vectorPush (newMNode_str (new ustring ((*b).first)));
    }
    return mlenv->retval = ans.release ();
}

/*DOC:
===table-to-list===
 (table-to-list TABLE) -> ((KEY . VAL) ...)

*/
//#XAFUNC	table-to-list	ml_table_to_list
//#XWIKIFUNC	table-to-list
MNode*  ml_table_to_list (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    evalParams (fev, mlenv, cell, posList, posParams);
    MNode*  e = posParams[0]();
    MNodeList  ans;
    if (isNil (e))
	return NULL;
    if (! isTable (e))
	throw (uErrorWrongType);
    MotorVar::iterator  b = e->value_table ()->begin ();
    MotorVar::iterator  t = e->value_table ()->end ();
    for (; b != t; ++ b) {
	ans.append (newMNode_cons (newMNode_str (new ustring ((*b).first)), (*b).second ())); // テーブルのキーは、シンボルではなく文字列
    }
    return mlenv->retval = ans.release ();
}

/*DOC:
===table-eval===
 (table-eval TABLE) -> TABLE

*/
//#XAFUNC	table-eval	ml_table_eval
MNode*  ml_table_eval (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    evalParams (fev, mlenv, cell, posList, posParams);
    MNode*  tbl = posParams[0]();
    if (isNil (tbl))
	return newMNode_table ();
    if (! isTable (tbl))
	throw (uErrorWrongType);
    return tableEval (tbl, mlenv);
}

/*DOC:
===table-each===
 (table-each 'VARIABLE1 'VARIABLE2 TABLE BODY...) -> LAST VALUE

*/
//#XAFUNC	table-each	ml_table_each
//#XWIKIFUNC	table-each
MNode*  ml_table_each (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_LIST, EV_LIST, EV_END};
    MNodePtr  posParams[3];
    MNodePtr  rest;
    evalParams (fev, mlenv, cell, posList, posParams, NULL, NULL, EV_ASIS, &rest);
    ustring  vkey = to_string (posParams[0]());
    ustring  vval = to_string (posParams[1]());
    MNode*  tbl = posParams[2]();
    MNodePtr  ans;
    if (vkey.size () == 0 || vval.size () == 0) {
	throw (uErrorBadArg);
    }
    if (isNil (tbl))
	return NULL;
    if (! isTable (tbl))
	throw (uErrorWrongType);
    
    {
	AutoLocalVariable  autoLocal (mlenv);
	mlenv->defineLocalVar (vkey);
	mlenv->defineLocalVar (vval);
	MotorVar::iterator  b = tbl->value_table ()->begin ();
	MotorVar::iterator  t = tbl->value_table ()->end ();
	for (; b != t; ++ b) {
	    mlenv->setVar (vkey, newMNode_str (new ustring ((*b).first))); // テーブルのキーは、文字列
	    mlenv->setVar (vval, (*b).second ());
	    ans = progn (rest (), mlenv);
	    if (mlenv->breaksym ()) {
		mlenv->stopBreak (cell->car ());
		break;
	    }
	}
    }
    return mlenv->retval = ans.release ();
}

/*DOC:
===table-reduce===
 (table-reduce TABLE INIT LAMBDA) -> ACC
 	LAMBDA: (lambda (key val acc) ...) -> ACC

TABLEがnilのとき、INITを返す。

*/
//#XAFUNC	table-reduce	ml_table_reduce
//#XWIKIFUNC	table-reduce
MNode*  ml_table_reduce (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_LIST, EV_LIST, EV_END};
    MNodePtr  posParams[3];
    evalParams (fev, mlenv, cell, posList, posParams);
    MNode*  tbl = posParams[0]();
    MNodePtr  acc;
    acc = posParams[1]();
    MNode*  lambda = posParams[2]();
    if (isNil (tbl))
	return mlenv->retval = acc.release ();
    if (! isTable (tbl))
	throw (uErrorWrongType);
    MotorVar::iterator  b = tbl->value_table ()->begin ();
    MotorVar::iterator  t = tbl->value_table ()->end ();
    for (; b != t; ++ b) {
	MNodeList  o;
	o.append (lambda);
	o.append (newMNode_str (new ustring ((*b).first)));
	o.append ((*b).second ());
	o.append (acc ());
	acc = eval_fn (false, lambda, o (), mlenv);
    }
    return mlenv->retval = acc.release ();
}

/*DOC:
===table-reduce-while===
 (table-reduce-while TABLE INIT LAMBDA) -> ACC
 	LAMBDA: (lambda (key val acc) ...) -> (BOOL ACC)

TABLEがnilのとき、INITを返す。

*/
//#XAFUNC	table-reduce-while	ml_table_reduce_while
//#XWIKIFUNC	table-reduce-while
MNode*  ml_table_reduce_while (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_LIST, EV_LIST, EV_END};
    MNodePtr  posParams[3];
    evalParams (fev, mlenv, cell, posList, posParams);
    MNode*  tbl = posParams[0]();
    MNodePtr  acc;
    MNodePtr  q;
    acc = posParams[1]();
    MNode*  lambda = posParams[2]();
    if (isNil (tbl))
	return mlenv->retval = acc.release ();
    if (! isTable (tbl))
	throw (uErrorWrongType);
    MotorVar::iterator  b = tbl->value_table ()->begin ();
    MotorVar::iterator  t = tbl->value_table ()->end ();
    for (; b != t; ++ b) {
	MNodeList  o;
	o.append (lambda);
	o.append (newMNode_str (new ustring ((*b).first)));
	o.append ((*b).second ());
	o.append (acc ());
	acc = eval_fn (false, lambda, o (), mlenv);
	if (isCons (acc ())) {
	    q = acc ()->car ();
	    if (isCons (acc ()->cdr ())) {
		acc = acc ()->cdr ()->car ();
		if (to_bool (q ())) {
		    // true はループ
		} else {
		    break;
		}
	    } else {
		throw (acc ()->dump_string () + ": bad return value of table-reduce-while function.");
	    }
	} else {
	    throw (acc ()->dump_string () + ": bad return value of table-reduce-while function.");
	}
    }
    return mlenv->retval = acc.release ();
}

/*DOC:
=== @ ===
 (@ [TABLE | VECTOR] [NAME | N]...) -> VALUE

*/
//#XAFUNC	@	ml_enum_get
//#XWIKIFUNC	@
MNode*  ml_enum_get (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    MNodePtr  rest;
    evalParams (fev, mlenv, cell, posList, posParams, NULL, NULL, EV_LIST, &rest);
    MNode*  obj = posParams[0]();
    MNode*  args = rest ();
    while (args) {
	MNode*  e = args->car ();
	nextNode (args);
	if (isNil (obj)) {
	    return NULL;
	} else if (isTable (obj)) {
	    obj = obj->tableGet (to_string (e));
	} else if (isVector (obj)) {
	    obj = obj->vectorGet (to_int64 (e));
	} else {
	    throw (uErrorWrongType);
	}
    }

    return mlenv->retval = obj;
}
