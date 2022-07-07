#include "ml-diff.h"
#include "diff.h"
#include "ml.h"
#include "mlenv.h"
#include "expr.h"
#include "ustring.h"
#include <exception>

/*DOC:
==diff function==

*/
/*DOC:
===diff===
// (diff TEXT1 TEXT2 VAR_LEFT1 VAR_LEFT2 VAR_RIGHT1 VAR_RIGHT2) -> NIL
 (diff LEFT_TEXT RIGHT_TEXT) -> (LEFT_OP LEFT_LINE RIGHT_OP RIGHT_LINE)

*/
//#XAFUNC	diff	ml_diff
MNode*  ml_diff (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_LIST, EV_END};
    MNodePtr  posParams[2];
    evalParams (fev, mlenv, cell, posList, posParams);
    ustring  t1 = to_string (posParams[0]());
    ustring  t2 = to_string (posParams[1]());
    MNodePtr  l1, l2, r1, r2;
    MNodeList  ans;
    diffreg (&t1, &t2, &l1, &l2, &r1, &r2, mlenv);
    ans.append (l1.release ());
    ans.append (l2.release ());
    ans.append (r1.release ());
    ans.append (r2.release ());
    return ans.release ();
}
