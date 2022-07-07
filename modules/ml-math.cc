#include "ml-math.h"
#include "ml.h"
#include "mlenv.h"
#include "motorenv.h"
#include "expr.h"
#include "util_const.h"
#include "util_random.h"
#include <exception>
#include <math.h>

/*DOC:
==mathematical function==

*/

/*DOC:
===to-integer===
 (to-integer [#bin | #oct | #hex | #HEX] OBJECT) -> NUMBER

*/
//#XAFUNC	to-integer	ml_to_integer
//#XWIKIFUNC	to-integer
MNode*  ml_to_integer (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    kwParam  kwList[] = {{CharConst ("hex"), EV_LIST}, // 0
			 {CharConst ("HEX"), EV_LIST}, // 1
			 {CharConst ("oct"), EV_LIST}, // 2
			 {CharConst ("bin"), EV_LIST}, // 3
			 {NULL, 0, EV_END}};
    MNodePtr  kwParams[4];
    evalParams (fev, mlenv, cell, posList, posParams, kwList, kwParams);
    bool  fhex = to_bool (kwParams[0]()) || to_bool (kwParams[1]());
    bool  foct = to_bool (kwParams[2]());
    bool  fbin = to_bool (kwParams[3]());
    ustring  text;
    int64_t  ans;
    if (fhex) {
	text = to_string (posParams[0]());
	ans = hextod (text.begin (), text.end (), 16);
    } else if (foct) {
	text = to_string (posParams[0]());
	ans = hextod (text.begin (), text.end (), 8);
    } else if (fbin) {
	text = to_string (posParams[0]());
	ans = hextod (text.begin (), text.end (), 2);
    } else {
	try {
	    ans = to_int64 (posParams[0]());
	} catch (boost::bad_lexical_cast msg) {
	    // 暗黙の変換では、BadValueエラーになるが、to-numberでは、エラーにしない。
	    ans = 0;
	}
    }
    return newMNode_num (ans);
}

/*DOC:
===to-number===
 (to-number [#bin | #oct | #hex | #HEX] OBJECT) -> NUMBER

*/
//#XAFUNC	to-number	ml_to_number
//#XWIKIFUNC	to-number
MNode*  ml_to_number (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    kwParam  kwList[] = {{CharConst ("hex"), EV_LIST}, // 0
			 {CharConst ("HEX"), EV_LIST}, // 1
			 {CharConst ("oct"), EV_LIST}, // 2
			 {CharConst ("bin"), EV_LIST}, // 3
			 {NULL, 0, EV_END}};
    MNodePtr  kwParams[4];
    evalParams (fev, mlenv, cell, posList, posParams, kwList, kwParams);
    bool  fhex = to_bool (kwParams[0]()) || to_bool (kwParams[1]());
    bool  foct = to_bool (kwParams[2]());
    bool  fbin = to_bool (kwParams[3]());
    ustring  text;
    int64_t  ans;
    if (fhex) {
	text = to_string (posParams[0]());
	ans = hextod (text.begin (), text.end (), 16);
	return newMNode_num (ans);
    } else if (foct) {
	text = to_string (posParams[0]());
	ans = hextod (text.begin (), text.end (), 8);
	return newMNode_num (ans);
    } else if (fbin) {
	text = to_string (posParams[0]());
	ans = hextod (text.begin (), text.end (), 2);
	return newMNode_num (ans);
    } else {
	return to_number (posParams[0]());
    }
}

#define MKFN1(op, name)	\
static MNode*  mop_##name (MNode* a) { \
    if ((a)->isInt ()) { \
	return newMNode_int64 (op (a)->value_int ()); \
    } else if ((a)->isReal ()) { \
	return newMNode_num (op (a)->value_real ()); \
    } \
    return NULL; \
}
#define MKFN2(op, name)	\
static MNode*  mop_##name (MNode* a, MNode* b) { \
    if ((a)->isInt ()) { \
	if ((b)->isInt ()) { \
	    return newMNode_int64 ((a)->value_int () op (b)->value_int ()); \
	} else if ((b)->isReal ()) { \
	    return newMNode_num ((double)(a)->value_int () op (b)->value_real ()); \
	} \
    } else if ((a)->isReal ()) { \
	if ((b)->isInt ()) { \
	    return newMNode_num ((a)->value_real () op (double)(b)->value_int ()); \
	} else if ((b)->isReal ()) { \
	    return newMNode_num ((a)->value_real () op (b)->value_real ()); \
	} \
    } \
    return NULL; \
}
MKFN1(-, neg)
MKFN2(+, add)
MKFN2(-, sub)
MKFN2(*, mul)
MKFN2(/, div)
#undef MKFN1
#undef MKFN2
inline MNode*  TO_NUMBER (MNode* val) {
    return isIntReal (val) ? val : to_number (val);
}

/*DOC:
=== + ===
 (+ NUMBER NUMBER...) -> NUMBER

*/
//#XAFUNC	+	ml_add
//#XWIKIFUNC	+
MNode*  ml_add (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    MNodePtr  rest;
    evalParams (fev, mlenv, cell, posList, posParams, NULL, NULL, EV_LIST, &rest);
    MNodePtr  ans, a;
    ans = TO_NUMBER (posParams[0]());
    MNode*  ag = rest ();
    while (ag) {
	a = TO_NUMBER (ag->car ());
	ans = mop_add (ans (), a ());
	nextNode (ag);
    }
    return mlenv->retval = ans.release ();
}

/*DOC:
=== - ===
 (- NUMBER) -> NUMBER
 (- NUMBER NUMBER) -> NUMBER

*/
//#XAFUNC	-	ml_sub
//#XWIKIFUNC	-
MNode*  ml_sub (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_LIST, EV_END};
    MNodePtr  posParams[2];
    evalParams (fev, mlenv, cell, posList, posParams);
    if (! isNil (posParams[1]())) {		// (- NUM1 NUM2)
	MNodePtr  a1, a2;
	a1 = TO_NUMBER (posParams[0]());
	a2 = TO_NUMBER (posParams[1]());
	return mop_sub (a1 (), a2 ());
    } else {					// (- NUM1)
	MNodePtr  a1;
	a1 = TO_NUMBER (posParams[0]());
	return mop_neg (a1 ());
    }
}

/*DOC:
=== * ===
 (* NUMBER NUMBER...) -> NUMBER

*/
//#XAFUNC	*	ml_mult
//#XWIKIFUNC	*
MNode*  ml_mult (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    MNodePtr  rest;
    evalParams (fev, mlenv, cell, posList, posParams, NULL, NULL, EV_LIST, &rest);
    MNodePtr  ans, a;
    ans = TO_NUMBER (posParams[0]());
    MNode*  args = rest ();
    while (args) {
	a = TO_NUMBER (args->car ());
	ans = mop_mul (ans (), a ());
	nextNode (args);
    }
    return mlenv->retval = ans.release ();
}

/*DOC:
=== / ===
 (/ NUMBER NUMBER) -> NUMBER

*/
//#XAFUNC	/	ml_div
//#XWIKIFUNC	/
MNode*  ml_div (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_LIST, EV_END};
    MNodePtr  posParams[2];
    evalParams (fev, mlenv, cell, posList, posParams);
    MNodePtr  a1, a2;
    a1 = TO_NUMBER (posParams[0]());
    a2 = TO_NUMBER (posParams[1]());
    if ((a2 ()->isInt () && a2 ()->value_int () == 0)
	|| (a2 ()->isReal () && a2 ()->value_real () == 0.)) {
	throw (uErrorDiv0);
    }
    return mop_div (a1 (), a2 ());
}

/*DOC:
=== % ===
 (% NUMBER NUMBER) -> NUMBER
剰余算

*/
//#XAFUNC	%	ml_mod
//#XWIKIFUNC	%
MNode*  ml_mod (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_LIST, EV_END};
    MNodePtr  posParams[2];
    evalParams (fev, mlenv, cell, posList, posParams);
    MNodePtr  a1, a2;
    a1 = TO_NUMBER (posParams[0]());
    a2 = TO_NUMBER (posParams[1]());
    if ((a2 ()->isInt () && a2 ()->value_int () == 0)
	|| (a2 ()->isReal () && a2 ()->value_real () == 0.)) {
	throw (uErrorDiv0);
    }
    if (isInt (a1 ()) && isInt (a2 ())) {
	int64_t  b1 = to_int64 (a1 ());
	int64_t  b2 = to_int64 (a2 ());
	return newMNode_int64 (b1 - b1 / b2 * b2);
    } else {
	return newMNode_num (fmod (to_double (a1 ()), to_double (a2 ())));
    }
}

/*DOC:
=== idivmod ===
 (idivmod INTEGER INTEGER) -> (QUOTIENT REMAINDER)
剰余算

*/
//#XAFUNC	idivmod	ml_idivmod
//#XWIKIFUNC	idivmod
MNode*  ml_idivmod (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_LIST, EV_END};
    MNodePtr  posParams[2];
    evalParams (fev, mlenv, cell, posList, posParams);
    int64_t  a1, a2, a3;
    a1 = to_int64 (posParams[0]());
    a2 = to_int64 (posParams[1]());
    if (a2 == 0)
	throw (uErrorDiv0);
    a3 = a1 / a2;
    return newMNode_cons (newMNode_int64 (a3), newMNode_cons (newMNode_int64 (a1 - a3 * a2)));
}

/*DOC:
===ceil===
 (ceil NUMBER) -> NUMBER
引数以上の最小の整数。

*/
//#XAFUNC	ceil	ml_ceil
//#XWIKIFUNC	ceil
MNode*  ml_ceil (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    evalParams (fev, mlenv, cell, posList, posParams);
    double  v = to_double (posParams[0]());
    return newMNode_int64 ((int64_t)ceil (v));
}

/*DOC:
===floor===
 (floor NUMBER) -> NUMBER
引数以下の最大の整数。

*/
//#XAFUNC	floor	ml_floor
//#XWIKIFUNC	floor
MNode*  ml_floor (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    evalParams (fev, mlenv, cell, posList, posParams);
    double  v = to_double (posParams[0]());
    return newMNode_int64 ((int64_t)floor (v));
}

/*DOC:
===fraction===
 (fraction NUMBER) -> NUMBER
引数の小数部分。

*/
//#XAFUNC	fraction	ml_fraction
//#XWIKIFUNC	fraction
MNode*  ml_fraction (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    evalParams (fev, mlenv, cell, posList, posParams);
    double  v = to_double (posParams[0]());
    return newMNode_num (v - floor (v));
}

/*DOC:
===random===
 (random) -> NUMBER
0以上，1未満の乱数。

*/
//#XAFUNC	random	ml_random
//#XWIKIFUNC	random
MNode*  ml_random (bool fev, MNode* cell, MlEnv* mlenv) {
    evalParams (fev, mlenv, cell);
    return newMNode_num (randDouble ());
}

/*DOC:
===random-int===
 (random-int NUM) -> NUMBER
0以上，NUM未満の整数乱数。

*/
//#XAFUNC	random-int	ml_random_int
//#XWIKIFUNC	random-int
MNode*  ml_random_int (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    evalParams (fev, mlenv, cell, posList, posParams);
    int64_t  n = to_int64 (posParams[0]());
    return newMNode_num (random_int (n));
}
