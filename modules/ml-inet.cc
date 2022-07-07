#include "ml-inet.h"
#include "ml.h"
#include "mlenv.h"
#include "motorenv.h"
#include "expr.h"
#include "util_const.h"
#include "util_inet.h"
#include "ustring.h"
#include <exception>

/*DOC:
==INET function==

*/
/*DOC:
===to-hostname===
 (to-hostname IPADDR) -> HOSTNAME

===gethostname===
 (gethostname IPADDR) -> HOSTNAME

*/
//#XAFUNC	to-hostname	ml_to_hostname
//#XAFUNC	gethostname	ml_to_hostname
MNode*  ml_to_hostname (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    evalParams (fev, mlenv, cell, posList, posParams);
    ustring  ip = to_string (posParams[0]());
    return newMNode_str (new ustring (getnameinfo (ip)));
}

/*DOC:
===to-ip-addr===
 (to-ip-addr HOSTNAME) -> (IP...)

*/
//#XAFUNC	to-ip-addr	ml_to_ip_addr
MNode*  ml_to_ip_addr (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    evalParams (fev, mlenv, cell, posList, posParams);
    ustring  hostname = to_string (posParams[0]());
    std::vector<ustring>  ip;
    getAddrInfo (hostname, ip);
    MNodeList  ans;
    for (int i = 0; i < ip.size (); i ++) {
	ans.append (newMNode_str (new ustring (ip[i])));
    }
    return ans.release ();
}
