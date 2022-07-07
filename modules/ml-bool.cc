#include "ml-bool.h"
#include "ml.h"
#include "mlenv.h"
#include "motorenv.h"
#include "util_const.h"
#include "expr.h"
#include <exception>

/*DOC:
==boolean operations==

値をブール値として評価するとき，空文字列，文字列の「0」，数値の0，NILはFALSE，それ以外をTRUEとする。

*/

/*DOC:
===to-bool===
 (to-bool OBJECT) -> true | false

*/
//#XAFUNC	to-bool	ml_to_bool
//#XWIKIFUNC	to-bool
MNode*  ml_to_bool (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    evalParams (fev, mlenv, cell, posList, posParams);
    return newMNode_bool (to_bool (posParams[0]()));
}

/*DOC:
===not===
 (not BOOL) -> true | false

*/
//#XAFUNC	not	ml_not
//#XWIKIFUNC	not
MNode*  ml_not (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    evalParams (fev, mlenv, cell, posList, posParams);
    return newMNode_bool (! to_bool (posParams[0]()));
}

/*DOC:
===and===
 (and OBJECT OBJECT...) -> OBJECT

パラメータを左から順に評価し，FALSEになると，以降は評価しない。最後の値を返す。

*/
//#XAFUNC	and	ml_and
//#XWIKIFUNC	and
MNode*  ml_and (bool fev, MNode* cell, MlEnv* mlenv) {
    MNodePtr  rest;
    evalParams (fev, mlenv, cell, EV_ASIS, &rest);
    MNode*  args = rest ();
    MNodePtr  v;
    bool  a1 = true;
    while (args && a1) {
	v = eval (args->car (), mlenv);
	a1 &= to_bool (v ());
	nextNode (args);
    }
    return mlenv->retval = v.release ();
}

/*DOC:
===or===
 (or OBJECT OBJECT...) -> OBJECT

パラメータを左から順に評価し，TRUEになると，以降は評価しない。最後の値を返す。

*/
//#XAFUNC	or	ml_or
//#XWIKIFUNC	or
MNode*  ml_or (bool fev, MNode* cell, MlEnv* mlenv) {
    MNodePtr  rest;
    evalParams (fev, mlenv, cell, EV_ASIS, &rest);
    MNode*  args = rest ();
    MNodePtr  v;
    bool  a1 = false;
    while (args && ! a1) {
	v = eval (args->car (), mlenv);
	a1 |= to_bool (v ());
	nextNode (args);
    }
    return mlenv->retval = v.release ();
}

/*DOC:
===xor===
 (xor BOOL BOOL...) -> true | false

*/
//#XAFUNC	xor	ml_xor
//#XWIKIFUNC	xor
MNode*  ml_xor (bool fev, MNode* cell, MlEnv* mlenv) {
    MNodePtr  rest;
    evalParams (fev, mlenv, cell, EV_LIST, &rest);
    MNode*  args = rest ();
    bool  a1 = false;
    while (args) {
	a1 ^= to_bool (args->car ());
	nextNode (args);
    }
    return newMNode_bool (a1);
}

#define TO_NUMBEROBJ(var)	if (! isIntReal (var ())) var = to_number (var ())
/*DOC:
=== = ===
 (= NUMBER NUMBER...) -> true | false

パラメータを数値として比較し，すべて一致すれば，1を返す。左から順に評価し，一致しないものがあれば，以降は評価しない。

*/
//#XAFUNC	=	ml_neq
//#XWIKIFUNC	=
MNode*  ml_neq (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    MNodePtr  rest;
    evalParams (fev, mlenv, cell, posList, posParams, NULL, NULL, EV_ASIS, &rest);
    MNodePtr  v1, v2;
    v1 = posParams[0]();
    TO_NUMBEROBJ (v1);
    MNode*  args = rest ();
    MNode*  a;
    while (args) {
	a = args->car ();
	v2 = eval (a, mlenv);
	TO_NUMBEROBJ (v2);
	if (neq (v1 (), v2 ())) {
	    v1 = v2 ();
	} else {
	    return newMNode_bool (false);
	}
	nextNode (args);
    }
    return newMNode_bool (true);
}

/*DOC:
=== != ===
 (!= NUMBER NUMBER) -> true | false

numerical not equal

*/
//#XAFUNC	!=	ml_nne
//#XWIKIFUNC	!=
MNode*  ml_nne (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_LIST, EV_END};
    MNodePtr  posParams[2];
    evalParams (fev, mlenv, cell, posList, posParams);
    MNodePtr  v1, v2;
    v1 = posParams[0]();
    TO_NUMBEROBJ (v1);
    v2 = posParams[1]();
    TO_NUMBEROBJ (v2);
    return newMNode_bool (! neq (v1 (), v2 ()));
}

/*DOC:
=== > ===
 (> NUMBER NUMBER...) -> true | false

パラメータを数値として比較し，すべて右側の値が小さいとき，1を返す。左から順に評価し，右側が小さくないとき，以降は評価しない。

*/
//#XAFUNC	>	ml_gt
//#XWIKIFUNC	>
MNode*  ml_gt (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    MNodePtr  rest;
    evalParams (fev, mlenv, cell, posList, posParams, NULL, NULL, EV_ASIS, &rest);
    MNodePtr  v1, v2;
    v1 = posParams[0]();
    TO_NUMBEROBJ (v1);
    MNode*  args = rest ();
    MNode*  a;
    while (args) {
	a = args->car ();
	v2 = eval (a, mlenv);
	TO_NUMBEROBJ (v2);
	if (ngt (v1 (), v2 ())) {
	    v1 = v2 ();
	} else {
	    return newMNode_bool (false);
	}
	nextNode (args);
    }
    return newMNode_bool (true);
}

/*DOC:
=== < ===
 (< NUMBER NUMBER...) -> true | false

*/
//#XAFUNC	<	ml_lt
//#XWIKIFUNC	<
MNode*  ml_lt (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    MNodePtr  rest;
    evalParams (fev, mlenv, cell, posList, posParams, NULL, NULL, EV_ASIS, &rest);
    MNodePtr  v1, v2;
    v1 = posParams[0]();
    TO_NUMBEROBJ (v1);
    MNode*  args = rest ();
    MNode*  a;
    while (args) {
	a = args->car ();
	v2 = eval (a, mlenv);
	TO_NUMBEROBJ (v2);
	if (ngt (v2 (), v1 ())) {	// v1 < v2
	    v1 = v2 ();
	} else {
	    return newMNode_bool (false);
	}
	nextNode (args);
    }
    return newMNode_bool (true);
}

/*DOC:
=== >= ===
 (>= NUMBER NUMBER...) -> true | false

*/
//#XAFUNC	>=	ml_ge
//#XWIKIFUNC	>=
MNode*  ml_ge (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    MNodePtr  rest;
    evalParams (fev, mlenv, cell, posList, posParams, NULL, NULL, EV_ASIS, &rest);
    MNodePtr  v1, v2;
    v1 = posParams[0]();
    TO_NUMBEROBJ (v1);
    MNode*  args = rest ();
    MNode*  a;
    while (args) {
	a = args->car ();
	v2 = eval (a, mlenv);
	TO_NUMBEROBJ (v2);
	if (nge (v1 (), v2 ())) {
	    v1 = v2 ();
	} else {
	    return newMNode_bool (false);
	}
	nextNode (args);
    }
    return newMNode_bool (true);
}

/*DOC:
=== <= ===
 (<= NUMBER NUMBER...) -> true | false

*/
//#XAFUNC	<=	ml_le
//#XWIKIFUNC	<=
MNode*  ml_le (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    MNodePtr  rest;
    evalParams (fev, mlenv, cell, posList, posParams, NULL, NULL, EV_ASIS, &rest);
    MNodePtr  v1, v2;
    v1 = posParams[0]();
    MNode*  args = rest ();
    MNode*  a;
    while (args) {
	a = args->car ();
	v2 = eval (a, mlenv);
	if (nge (v2 (), v1 ())) {	// v1 <= v2
	    v1 = v2 ();
	} else {
	    return newMNode_bool (false);
	}
	nextNode (args);
    }
    return newMNode_bool (true);
}

/*DOC:
=== \=\=\= ===
 (=== OBJECT OBJECT...) -> true | false

Check if all args are the same lisp object.

*/
//#XAFUNC	===	ml_object_eq
//#XWIKIFUNC	===
MNode*  ml_object_eq (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    MNodePtr  rest;
    evalParams (fev, mlenv, cell, posList, posParams, NULL, NULL, EV_ASIS, &rest);
    MNode*  v1 = posParams[0]();
    MNode*  args = rest ();
    MNode*  a;
    MNodePtr  v2;
    while (args) {
	a = args->car ();
	v2 = eval (a, mlenv);
	if (eq (v1, v2 ())) {
	} else {
	    return newMNode_bool (false);
	}
	nextNode (args);
    }
    return newMNode_bool (true);
}

/*DOC:
=== \=\= ===
 (== OBJECT OBJECT...) -> true | false

Check if all args are the matching lisp object.

*/
//#XAFUNC	==	ml_object_equal
//#XWIKIFUNC	==
MNode*  ml_object_equal (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    MNodePtr  rest;
    evalParams (fev, mlenv, cell, posList, posParams, NULL, NULL, EV_ASIS, &rest);
    MNode*  v1 = posParams[0]();
    MNode*  args = rest ();
    MNode*  a;
    MNodePtr  v2;
    while (args) {
	a = args->car ();
	v2 = eval (a, mlenv);
	if (equal (v1, v2 ())) {
	} else {
	    return newMNode_bool (false);
	}
	nextNode (args);
    }
    return newMNode_bool (true);
}

/*DOC:
=== !\=\=\= ===
 (!=== OBJECT OBJECT) -> true | false

Check if all args are the same lisp object.

*/
//#XAFUNC	!===	ml_object_not_eq
//#XWIKIFUNC	!===
MNode*  ml_object_not_eq (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_LIST, EV_END};
    MNodePtr  posParams[2];
    evalParams (fev, mlenv, cell, posList, posParams);
    MNode*  v1 = posParams[0]();
    MNode*  v2 = posParams[1]();
    return newMNode_bool (! eq (v1, v2));
}

/*DOC:
=== !\=\= ===
 (!== OBJECT OBJECT) -> true | false

Check if all args are the matching lisp object.

*/
//#XAFUNC	!==	ml_object_not_equal
//#XWIKIFUNC	!==
MNode*  ml_object_not_equal (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_LIST, EV_END};
    MNodePtr  posParams[2];
    evalParams (fev, mlenv, cell, posList, posParams);
    MNode*  v1 = posParams[0]();
    MNode*  v2 = posParams[1]();
    return newMNode_bool (! equal (v1, v2));
}

