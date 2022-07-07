#include "ml-config.h"
#include "config.h"
#include "motorconst.h"
#include "ml.h"
#include "mlenv.h"
#include "motorenv.h"
#include "motoroutput.h"
#include "util_const.h"
#include "util_check.h"
#include "util_file.h"
#include "util_string.h"
#include "expr.h"
#include "ustring.h"
#include <vector>
#include <exception>
#include <stdlib.h>
#include <dirent.h>

/*DOC:
==config function==

*/
/*DOC:
===datastore-list===
 (datastore-list) -> LIST

*/
//#XAFUNC	datastore-list	ml_datastore_list
MNode*  ml_datastore_list (bool fev, MNode* cell, MlEnv* mlenv) {
    evalParams (fev, mlenv, cell);
    MNodeList  ans;
    DIR*  d;
    struct dirent  dirdata;
    struct dirent*  de;
    ustring  name;
    ustring  t;

    d = opendir (cDataTop);
    if (d) {
	while (readdir_r (d, &dirdata, &de) == 0 && de != NULL) {
#ifndef NOT_HAVE_D_NAMLEN
	    name.assign (de->d_name, de->d_namlen);
#else
	    name.assign (de->d_name);
#endif
	    t.assign (CharConst (cDataTop kDS)).append (name).append (CharConst (kSubStore));
	    if (matchName (name) && isDirectory (t)) {
		ans.append (newMNode_str (new ustring (name)));
	    }
	}
	closedir (d);
	d = NULL;
    }

    return mlenv->retval = ans.release ();
}

