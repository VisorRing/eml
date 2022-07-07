#include "ml-debug.h"
#include "ml.h"
#include "mlenv.h"
#include "motorenv.h"
#include "expr.h"
#include "ustring.h"

/*DOC:
==デバッグモジュール
デバッグ出力に関する制御。

*/
#if 0
/*DOC:
===sleep===
 (sleep SECONDS) -> NIL

*/
////#XAFUNC	sleep	ml_sleep
MNode*  ml_sleep (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    evalParams (fev, mlenv, cell, posList, posParams);
    time_t  sec = to_int (posParams[0]());
    sleep (sec);
    return NULL;
}
#endif

/*DOC:
===stderr===
 (stderr TEXT...) -> LAST_VALUE

*/
//#XAFUNC	stderr	ml_stderr
MNode*  ml_stderr (bool fev, MNode* cell, MlEnv* mlenv) {
    MNodePtr  rest;
    evalParams (fev, mlenv, cell, EV_LIST, &rest);
    MNode*  args = rest ();
    MNode*  ans = NULL;
    ustring  text;
    while (args) {
	ans = args->car ();
	text.append (to_string (ans));
	nextNode (args);
    }
    if (text.length () > 0 && text[text.length () - 1] != '\n')
	text.append (uLF);
    std::cerr << text;
    return mlenv->retval = ans;
}
