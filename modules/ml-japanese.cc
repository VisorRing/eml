#include "ml-japanese.h"
#include "ml.h"
#include "mlenv.h"
#include "motorenv.h"
#include "motoroutput.h"
#include "util_const.h"
#include "util_string.h"
#include "expr.h"
#include "utf8-jp.h"
#include "ustring.h"

/*DOC:
==日本語処理==

*/
/*DOC:
===fullwidth-ascii-to-halfwidth===
 (fullwidth-ascii-to-halfwidth STRING...) -> STRING
*/
//#XAFUNC	fullwidth-ascii-to-halfwidth	ml_fullwidth_ascii_to_halfwidth
//#XWIKIFUNC	fullwidth-ascii-to-halfwidth
MNode*  ml_fullwidth_ascii_to_halfwidth (bool fev, MNode* cell, MlEnv* mlenv) {
    MNodePtr  rest;
    evalParams (fev, mlenv, cell, EV_LIST, &rest);
    ustring  ans;
    MNode*  args = rest ();
    while (args) {
	ans.append (fullWidthASCIItoASCII (to_string (args->car ())));
	nextNode (args);
    }
    return newMNode_str (new ustring (ans));
}
