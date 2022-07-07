#include "ml-include.h"
#include "ml.h"
#include "mlenv.h"
#include "motorenv.h"
#include "expr.h"
#include "util_const.h"
#include "util_check.h"
#include "util_file.h"
#include "ustring.h"
#include <exception>

/*DOC:
==MiniLisp file reading function==

*/
/*DOC:
===include===
 (include [#force | :force BOOL] FILENAME...)

*/
//#XAFUNC	include	ml_include
MNode*  ml_include (bool fev, MNode* cell, MlEnv* mlenv) {
    kwParam  kwList[] = {{CharConst ("force"), EV_LIST},
			 {NULL, 0, EV_END}};
    MNodePtr  kwParams[1];
    MNodePtr  rest;
    evalParams (fev, mlenv, cell, NULL, NULL, kwList, kwParams, EV_LIST, &rest);
    bool  fforce = to_bool (kwParams[0] ());
    ustring  file, f;
    MNodePtr  ans;
    MNode*  args = rest ();
    while (args) {
	file = eval_str (args->car (), mlenv);
	nextNode (args);
	f = mlenv->env->search_resource_file (file);
	if (f.size () > 0) {
	    ustring  b;
	    bool  rc;
	    if (fforce || ! mlenv->includedFile (f)) {
		mlenv->setIncludedFile (f);
		rc = readFile (f, b);
		if (rc) {
		    MotorTexp  ml (mlenv);
		    AutoInclCount  autoIncl (mlenv);
		    AutoBackupPtr<ustring>  autoStr (&mlenv->env->currentPath, &f);
		    ml.scan (b);
#ifdef DEBUG2
		    ml.top.dump (std::cerr, false);
		    std::cerr << "\n";
#endif /* DEBUG */
		    autoIncl.inc ();
		    if (ml.top.isCons ()) {
			ans = progn (&ml.top, mlenv);
			if (mlenv->breaksym ()) {
			    mlenv->stopBreak (cell->car ());
			    rest = NULL;
			}
		    }
		} else {
		    throw (file + uErrorNotFound);
		}
	    }
	} else {
	    throw (file + uErrorNotFound);
	}
    }
    return mlenv->retval = ans.release ();
}
