#include "motorfunc.h"
#include "motorenv.h"
#include "mlenv.h"
#include "ml-defun.h"
#include "mftable.h"
#include "expr.h"
#include "utf8.h"
#include "ustring.h"


void  execMotorFunc (const ustring& name, std::vector<ustring>& args, MlEnv* mlenv) {
    MFTable::iterator  it;
    AutoInclCount  autoIncl (mlenv);

    autoIncl.inc ();
//    mlenv->inclIncCount ();
    if ((it = IMFTable.find (name)) != IMFTable.end ()) {
	try {
	    it->second->fn (args, mlenv);
	} catch (ustring& msg) {
	    ustring amsg;
	    amsg.append (CharConst ("[[")).append (name);
	    for (int i = 0; i < args.size (); i ++) {
		amsg.append (uColon).append (args[i]);
	    }
	    amsg.append (CharConst ("]]: ")).append (msg);
	    amsg = ellipsis (logText (amsg), cLOGSHORTSIZE);
	    throw (amsg);
	}
    } else {
//#ifdef DEBUG2
//	std::cerr << name << ": not found.\n";
//#endif /* DEBUG */
	MNodePtr  vargs;
	MNodePtr  ans;

	vargs = buildArgs (0, args);
	ans = execDefun (mlenv, &mlenv->env->motorCall, name, vargs ());
    }
//    mlenv->declIncCount ();
}
