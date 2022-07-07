#include "ml-json.h"
#include "picojson.h"
#include "ml.h"
#include "mlenv.h"
#include "formfile.h"
#include "motorenv.h"
#include "motoroutput.h"
#include "ustring.h"
#include "expr.h"
#include "utf8.h"
#include <exception>

/*DOC:
==JSON module==

*/

static MNode*  picoj2texp (const picojson::value& v) {
    if (v.is<picojson::null>()) {
	return NULL;
    } else if (v.is<bool>()) {
	return newMNode_bool (v.get<bool> ());
    } else if (v.is<int64_t>()) {
	return newMNode_int64 (v.get<int64_t> ());
    } else if (v.is<double>()) {
	return newMNode_num (v.get<double> ());
    } else if (v.is<std::string>()) {
	ustring  ans = fixUTF8 (v.get<std::string> ());
	return newMNode_str (new ustring (ans));
    } else if (v.is<picojson::array>()) {
	MNodePtr  ans;
	ans = newMNode_vector ();
	const picojson::array&  a = v.get<picojson::array> ();
	for (picojson::array::const_iterator i = a.begin (); i != a.end (); ++i) {
	    ans ()->vectorPush (picoj2texp (*i));
	}
	return ans.release ();
    } else if (v.is<picojson::object> ()) {
	MNodePtr  ans;
	ans = newMNode_table ();
	const picojson::object&  o = v.get<picojson::object> ();
	for (picojson::object::const_iterator i = o.begin (); i != o.end (); ++i) {
	    ustring  name = fixUTF8 (i->first);
	    ans ()->tablePut (name, picoj2texp (i->second));
	}
	return ans.release ();
    }
    return NULL;		// error
}

/*DOC:
===json-read===
 (json-read TEXT) -> LIST
 scalar_value → Atom
 array → VECTOR
 object → TABLE

*/
//#XAFUNC	json-read	ml_json_texp_read
//#XWIKIFUNC	json-read
MNode*  ml_json_texp_read (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    evalParams (fev, mlenv, cell, posList, posParams);
    ustring  text = to_string (posParams[0]());
    picojson::value  val;
    ustring  err;
    picojson::parse (val, text.begin (), text.end (), &err);
    return picoj2texp (val);
}

static bool  jsonWrite (MotorOutput* out, MNode* json) {
    bool  rc = true;
    if (json) {
	switch (json->type) {
	case MNode::MC_NIL:
	    out->out_raw (CharConst ("null"));
	    break;
	case MNode::MC_CONS:
	    out->out_raw (CharConst ("null"));
	    rc = false;
	    break;
	case MNode::MC_STR:
	    out->out_raw (uQ2)->out_toJS (to_string (json))->out_raw (uQ2);
	    break;
	case MNode::MC_SYM:
	    if (match (*ptr_symbol (json), CharConst ("true"))) {
		out->out_raw (CharConst ("true"));
	    } else if (match (*ptr_symbol (json), CharConst ("false"))) {
		out->out_raw (CharConst ("false"));
	    } else if (match (*ptr_symbol (json), CharConst ("nil"))) {
		out->out_raw (CharConst ("null"));
	    } else {
		out->out_raw (CharConst ("null"));
		rc = false;
	    }
	    break;
	case MNode::MC_INT64:
	    out->out_raw (json->to_string ());
	    break;
	case MNode::MC_DOUBLE:
	    out->out_raw (json->to_string ());
	    break;
	case MNode::MC_VECTOR:
	    {
		out->out_raw (CharConst ("["));
		size_t  i, n;
		n = json->vectorSize ();
		for (i = 0; i < n; ++ i) {
		    if (i > 0)
			out->out_raw (CharConst (","));
		    rc &= jsonWrite (out, json->vectorGet (i));
		}
		out->out_raw (CharConst ("]"));
	    }
	    break;
	case MNode::MC_TABLE:
	    {
		out->out_raw (CharConst ("{"));
		MotorVar::iterator  b = ptr_table (json)->begin ();
		MotorVar::iterator  e = ptr_table (json)->end ();
		int  n = 0;
		for (; b != e; ++ b, ++ n) {
		    if (n > 0)
			out->out_raw (CharConst (","));
		    out->out_raw (uQ2)->out_toJS ((*b).first)->out_raw (uQ2)->out_raw (uColon);
		    rc &= jsonWrite (out, (*b).second ());
		}
		out->out_raw (CharConst ("}"));
	    }
	    break;
	case MNode::MC_BOOL:
	    if (json->to_bool ()) {
		out->out_raw (CharConst ("true"));
	    } else {
		out->out_raw (CharConst ("false"));
	    }
	    break;
	case MNode::MC_LIBOBJ:
	    out->out_raw (CharConst ("OBJECT"));
	    rc = false;
	    break;
//	case MNode::MC_DELETED:
//	    break;
	default:
	    assert (0);
	}
    } else {
	out->out_raw (CharConst ("null"));
    }
    return rc;
}

/*DOC:
===json-output===
 (json-output [#continue] LIST) -> NIL

*/
//#XAFUNC	json-output	ml_json_texp_output
MNode*  ml_json_texp_output (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    kwParam  kwList[] = {{CharConst ("continue"), EV_LIST},	// 0
			 {NULL, 0, EV_END}};
    MNodePtr  kwParams[1];
    evalParams (fev, mlenv, cell, posList, posParams, kwList, kwParams);
    MNode*  json = posParams[0]();
    bool  cflag = to_bool (kwParams[0]());
    if (! mlenv->env->responseDone)
	mlenv->env->standardResponse (ustring (CharConst (kMIME_JSON)), ustring (uUTF8), uEmpty, false);
    MotorOutputCOut  out;
    jsonWrite (&out, json);
    if (! cflag)
	mlenv->breakProg ();
    return NULL;
}

/*DOC:
===json-output-string===
 (json-output-string LIST) -> STRING

*/
//#XAFUNC	json-output-string	ml_json_texp_output_string
//#XWIKIFUNC	json-output-string
MNode*  ml_json_texp_output_string (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    evalParams (fev, mlenv, cell, posList, posParams);
    MNode*  json = posParams[0]();
    MotorOutputString  out;
    jsonWrite (&out, json);
    return newMNode_str (new ustring (out.ans));
}

/*DOC:
===input-json===
 (input-json) -> LIST

*/
//#XAFUNC	input-json	ml_input_json
MNode*  ml_input_json (bool fev, MNode* cell, MlEnv* mlenv) {
    evalParams (fev, mlenv, cell, NULL, NULL);
    picojson::value  val;
    ustring  err;
    picojson::parse (val, mlenv->env->form->queryString.begin (), mlenv->env->form->queryString.end (), &err);
    return picoj2texp (val);
}
