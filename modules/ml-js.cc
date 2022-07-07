#include "ml-js.h"
#include "ml.h"
#include "mlenv.h"
#include "expr.h"
#include "util_const.h"
#include "util_string.h"
#include "ustring.h"
#include <vector>
#include <exception>

/*DOC:
==javascript functions==

*/
/*DOC:
===encode-js===
 (encode-js STRING...) -> STRING

*/
//#XAFUNC	encode-js	ml_encode_js
//#XWIKIFUNC	encode-js
MNode*  ml_encode_js (bool fev, MNode* cell, MlEnv* mlenv) {
    MNodePtr  rest;
    evalParams (fev, mlenv, cell, EV_LIST, &rest);
    ustring  str;
    MNode*  args = rest ();
    while (args) {
	str.append (to_string (args->car ()));
	nextNode (args);
    }
    str = jsEncode (str);
    return newMNode_str (new ustring (str));
}

static ustring  to_json (MNode* e) {
    if (e) {
	MNodePtr  h;
	switch (e->type) {
	case MNode::MC_NIL:;
	    return ustring (CharConst ("null"));	// RFC7159では、トップレベルがobject, array以外でも可
	case MNode::MC_CONS:
	    h = list_to_vector (e);
	    return to_json (h ());
	case MNode::MC_STR:
	    return dump_to_texp (e);
	case MNode::MC_SYM:
	    return ustring (CharConst ("Symbol(")) + e->sym_to_string () + ustring (CharConst (")"));
	case MNode::MC_BOOL:
	    if (e->to_bool ())
		return uTrue;
	    else
		return uFalse;
	case MNode::MC_DOUBLE:
	    return e->to_string ();
//	case MNode::MC_INT:
//	    return e->to_string ();
	case MNode::MC_VECTOR: {
	    ustring  ans;
	    ans.append (CharConst ("["));
	    size_t  n = e->vectorSize ();
	    for (int i = 0; i < n; ++ i) {
		if (i > 0)
		    ans.append (uComma);
		ans.append (to_json (e->vectorGet (i)));
	    }
	    ans.append (CharConst ("]"));
	    return ans;
	}
	case MNode::MC_TABLE: {
	    ustring  ans;
	    ans.append (CharConst ("{"));
	    MotorVar::iterator  b = e->value_table ()->begin ();
	    MotorVar::iterator  t = e->value_table ()->end ();
	    MotorVar::iterator  x;
	    for (x = b; x != t; ++ x) {
		if (x != b)
		    ans.append (uComma);
		ans.append (uQ2).append (slashEncode ((*x).first)).append (uQ2);
		ans.append (uColon);
		ans.append (to_json ((*x).second ()));
	    }
	    ans.append (CharConst ("}"));
	    return ans;
	}
	case MNode::MC_LIBOBJ:
	    return ustring (CharConst ("LibObj()"));
	case MNode::MC_BUILTIN_FN:
	    return ustring (CharConst ("Function()"));
	case MNode::MC_BUILTIN_MFN:
	    return ustring (CharConst ("Function()"));
	case MNode::MC_BUILTIN_SFN:
	    return ustring (CharConst ("Function()"));
	case MNode::MC_LAMBDA:
	    return ustring (CharConst ("Lambda()"));
	default:
	    assert (0);
	}
    } else {
	return ustring (CharConst ("null"));	// RFC7159では、トップレベルがobject, array以外でも可
    }
}

/*DOC:
===to-json===
 (to-json OBJECT) -> STRING

*/
//#XAFUNC	to-json	ml_to_json
//#XWIKIFUNC	to-json
MNode*  ml_to_json (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    evalParams (fev, mlenv, cell, posList, posParams);
    return newMNode_str (new ustring (to_json (posParams[0]())));
}

