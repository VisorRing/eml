#include "ml-data.h"
#include "ml.h"
#include "mlenv.h"
#include "util_const.h"
#include "ustring.h"
#include "expr.h"

#define TO_NUMBEROBJ(var)	if (! isIntReal (var ())) var = to_number (var ())

/*DOC:
===min===
 (min NUMBER ...) -> NUMBER

smallest number

===max===
 (max NUMBER ...) -> NUMBER

largest number

*/
//#XAFUNC	min	ml_min
//#XWIKIFUNC	min
MNode*  ml_min (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    MNodePtr  rest;
    evalParams (fev, mlenv, cell, posList, posParams, NULL, NULL, EV_LIST, &rest);
    MNodePtr  ans, a;
    ans = posParams[0]();
    TO_NUMBEROBJ (ans);
    MNode*  args = rest ();
    while (args) {
	a = args->car ();
	TO_NUMBEROBJ (a);
	nextNode (args);
	if (ngt (ans (), a ())) {	// a < ans
	    ans = a ();
	}
    }
    return mlenv->retval = ans.release ();
}

//#XAFUNC	max	ml_max
//#XWIKIFUNC	max
MNode*  ml_max (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    MNodePtr  rest;
    evalParams (fev, mlenv, cell, posList, posParams, NULL, NULL, EV_LIST, &rest);
    MNodePtr  ans, a;
    ans = posParams[0]();
    TO_NUMBEROBJ (ans);
    MNode*  args = rest ();
    while (args) {
	a = args->car ();
	TO_NUMBEROBJ (a);
	nextNode (args);
	if (ngt (a (), ans ()))
	    ans = a ();
    }
    return mlenv->retval = ans.release ();
}
