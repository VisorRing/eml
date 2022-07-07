#include "ml-string.h"
#include "ml.h"
#include "ml-texp.h"
#include "mlenv.h"
#include "motorenv.h"
#include "motoroutput.h"
#include "util_const.h"
#include "util_check.h"
#include "util_random.h"
#include "util_string.h"
#include "util_wsplitter.h"
#include "expr.h"
#include "utf8.h"
#include "utf16.h"
#include "ustring.h"
#include <vector>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <exception>
#include <stdlib.h>

/*DOC:
==string function==

*/
/*DOC:
===eq===
 (eq STRING STRING...) -> 1 or NIL
 (string-eq STRING STRING...) -> 1 or NIL

全てのSTRINGが同じ時、1を返す。

*/
//#XAFUNC	eq	ml_string_eq
//#XAFUNC	string-eq	ml_string_eq
//#XWIKIFUNC	eq
//#XWIKIFUNC	string-eq
MNode*  ml_string_eq (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    MNodePtr  rest;
    evalParams (fev, mlenv, cell, posList, posParams, NULL, NULL, EV_LIST, &rest);
    ustring  v1 = to_string (posParams[0]());
    ustring  v2;
    MNode*  args = rest ();
    while (args) {
	v2 = to_string (args->car ());
	if (v1 == v2) {
	} else {
	    return newMNode_bool (false);
	}
	nextNode (args);
    }
    return newMNode_bool (true);
}

/*DOC:
===ne===
 (ne STRING STRING) -> 1 or NIL
 (string-ne STRING STRING) -> 1 or NIL

STRINGが異なる時、1を返す。

*/
//#XAFUNC	ne	ml_string_ne
//#XAFUNC	string-ne	ml_string_ne
//#XWIKIFUNC	ne
//#XWIKIFUNC	string-ne
MNode*  ml_string_ne (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_LIST, EV_END};
    MNodePtr  posParams[2];
    evalParams (fev, mlenv, cell, posList, posParams);
    ustring  v1 = to_string (posParams[0]());
    ustring  v2 = to_string (posParams[1]());
    return newMNode_bool (v1 != v2);
}

//#XAFUNC	lt	ml_string_lt
//#XAFUNC	string-lt	ml_string_lt
//#XWIKIFUNC	lt
//#XWIKIFUNC	string-lt
MNode*  ml_string_lt (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_LIST, EV_END};
    MNodePtr  posParams[2];
    evalParams (fev, mlenv, cell, posList, posParams);
    ustring  v1 = to_string (posParams[0]());
    ustring  v2 = to_string (posParams[1]());
    return newMNode_bool (v1 < v2);
}

//#XAFUNC	le	ml_string_le
//#XAFUNC	string-le	ml_string_le
//#XWIKIFUNC	le
//#XWIKIFUNC	string-le
MNode*  ml_string_le (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_LIST, EV_END};
    MNodePtr  posParams[2];
    evalParams (fev, mlenv, cell, posList, posParams);
    ustring  v1 = to_string (posParams[0]());
    ustring  v2 = to_string (posParams[1]());
    return newMNode_bool (v1 <= v2);
}

//#XAFUNC	gt	ml_string_gt
//#XAFUNC	string-gt	ml_string_gt
//#XWIKIFUNC	gt
//#XWIKIFUNC	string-gt
MNode*  ml_string_gt (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_LIST, EV_END};
    MNodePtr  posParams[2];
    evalParams (fev, mlenv, cell, posList, posParams);
    ustring  v1 = to_string (posParams[0]());
    ustring  v2 = to_string (posParams[1]());
    return newMNode_bool (v1 > v2);
}

//#XAFUNC	ge	ml_string_ge
//#XAFUNC	string-ge	ml_string_ge
//#XWIKIFUNC	ge
//#XWIKIFUNC	string-ge
MNode*  ml_string_ge (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_LIST, EV_END};
    MNodePtr  posParams[2];
    evalParams (fev, mlenv, cell, posList, posParams);
    ustring  v1 = to_string (posParams[0]());
    ustring  v2 = to_string (posParams[1]());
    return newMNode_bool (v1 >= v2);
}

/*DOC:
===empty===
 (empty TEXT...) -> 1 or NIL

文字列TEXTの長さが0の時、1を返す。

*/
//#XAFUNC	empty	ml_emptyp
//#XWIKIFUNC	empty
MNode*  ml_emptyp (bool fev, MNode* cell, MlEnv* mlenv) {
    MNodePtr  rest;
    evalParams (fev, mlenv, cell, EV_LIST, &rest);
    MNode*  args = rest ();
    ustring  u;
    while (args) {
	u = to_string (args->car ());
	if (u.size () > 0)
	    return newMNode_bool (false);
	nextNode (args);
    }
    return newMNode_bool (true);
}

/*DOC:
===not-emptyp===
 (not-empty TEXT...) -> 1 or NIL

文字列TEXTの長さが0でない時、1を返す。

*/
//#XAFUNC	not-empty	ml_not_emptyp
//#XWIKIFUNC	not-empty
MNode*  ml_not_emptyp (bool fev, MNode* cell, MlEnv* mlenv) {
    MNodePtr  rest;
    evalParams (fev, mlenv, cell, EV_LIST, &rest);
    MNode*  args = rest ();
    ustring  u;
    while (args) {
	u = to_string (args->car ());
	if (u.size () == 0)
	    return newMNode_bool (false);
	nextNode (args);
    }
    return newMNode_bool (true);
}

/*DOC:
===concat===
 (concat STRING...) -> STRING
パラメータの文字列STRINGを連結して一つの文字列を返す。

*/
//#XAFUNC	concat	ml_concat
//#XWIKIFUNC	concat
MNode*  ml_concat (bool fev, MNode* cell, MlEnv* mlenv) {
    MNodePtr  rest;
    evalParams (fev, mlenv, cell, EV_LIST, &rest);
    AutoDelete<ustring>  a1;
    a1 = new ustring;
    MNode*  args = rest ();
    while (args) {
	a1 ()->append (to_string (args->car ()));
	nextNode (args);
    }
    return newMNode_str (a1.release ());
}

/*DOC:
===megabyte===
 (megabyte NUMBER) -> STRING

数値NUMBERをK、M、G、T、P単位（1024の倍数）の文字列に変換する。

*/
//#XAFUNC	megabyte	ml_megabyte
//#XWIKIFUNC	megabyte
MNode*  ml_megabyte (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    evalParams (fev, mlenv, cell, posList, posParams);
    double  val = to_double (posParams[0]());
    ustring  u;
    if (val < 900) {
	u = to_ustring (val);
	return newMNode_str (new ustring (u));
    }
    val = floor (val / 1024. * 10.) / 10.;
    if (val < 900) {
	u = to_ustring (val);
	u.append (CharConst ("K"));
	return newMNode_str (new ustring (u));
    }
    val = floor (val / 1024. * 10. ) / 10.;
    if (val < 900) {
	u = to_ustring (val);
	u.append (CharConst ("M"));
	return newMNode_str (new ustring (u));
    }
    val = floor (val / 1024. * 10.) / 10.;
    if (val < 900) {
	u = to_ustring (val);
	u.append (CharConst ("G"));
	return newMNode_str (new ustring (u));
    }
    val = floor (val / 1024. * 10.) / 10.;
    if (val < 900) {
	u = to_ustring (val);
	u.append (CharConst ("T"));
	return newMNode_str (new ustring (u));
    }
    val = floor (val / 1024. * 10.) / 10.;
    u = to_ustring (val);
    u.append (CharConst ("P"));
    return newMNode_str (new ustring (u));
}

/*DOC:
===c3===
 (c3 INTEGER) -> STRING

数値INTEGERを3桁ごとにカンマ区切りの文字列に変換する。

*/
//#XAFUNC	c3	ml_c3
//#XWIKIFUNC	c3
MNode*  ml_c3 (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    evalParams (fev, mlenv, cell, posList, posParams);
    ustring  u = to_string (posParams[0]());
    return newMNode_str (new ustring (c3 (u)));
}

/*DOC:
===string-join===
 (string-join TEXT [STRING | ARRAY | LIST | VECTOR]...) -> STRING

*/
//#XAFUNC	string-join	ml_string_join
//#XWIKIFUNC	string-join
MNode*  ml_string_join (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    MNodePtr  rest;
    evalParams (fev, mlenv, cell, posList, posParams, NULL, NULL, EV_LIST, &rest);
    ustring  sep = to_string (posParams[0]());
    MNode*  args = rest ();
    ustring  var;
    ustring  u;
    ustring  ans;
    int  i, n, c;
    MNode*  v;

    c = 0;
    while (args) {
	MNode*  val = args->car ();
	nextNode (args);
	if (val) {
	    if (isSym (val)) {
		var = val->to_string ();
		checkAry (var, var);
		n = mlenv->getArySize (var);
		for (i = 1; i <= n; i ++) {
		    if (c == 0)
			c ++;
		    else
			ans.append (sep);
		    v = mlenv->getAry (var, i);
		    if (v)
			ans.append (v->to_string ());
		}
	    } else if (isCons (val)) {
		MNode*  a = val;
		for (; isCons (a); a = a->cdr ()) {
		    if (c == 0)
			c ++;
		    else
			ans.append (sep);
		    if (! isNil (a->car ()))
			ans.append (a->car ()->to_string ());
		}
	    } else if (isVector (val)) {
		size_t  n = val->vectorSize ();
		size_t  i;
		for (i = 0; i < n; ++ i) {
		    if (i > 0)
			ans.append (sep);
		    MNode*  a = val->vectorGet (i);
		    if (! isNil (a))
			ans.append (a->to_string ());
		}
	    } else {
		var = val->to_string ();
		if (c == 0)
		    c ++;
		else
		    ans.append (sep);
		ans.append (var);
	    }
	}
    }
    return newMNode_str (new ustring (ans));
}

/*DOC:
===password-match===
 (password-match PASSWORD CRYPT) -> BOOL

*/
//#XAFUNC	password-match	ml_password_match
//#XWIKIFUNC	password-match
MNode*  ml_password_match (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_LIST, EV_END};
    MNodePtr  posParams[2];
    evalParams (fev, mlenv, cell, posList, posParams);
    ustring  pass = to_string (posParams[0]());
    ustring  cpass = to_string (posParams[1]());
    return newMNode_bool (passMatch (pass, cpass));
}

/*DOC:
===password-crypt===
 (password-crypt PASSWORD [#md5 | #sha256 | #sha512]) -> STRING

deprecated.

*/
//#XAFUNC	password-crypt	ml_password_crypt
//#XWIKIFUNC	password-crypt
MNode*  ml_password_crypt (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    kwParam  kwList[] = {{CharConst ("md5"), EV_LIST},
			 {CharConst ("sha256"), EV_LIST},
			 {CharConst ("sha512"), EV_LIST},
			// {CharConst ("bf"), EV_LIST},
			 {NULL, 0, EV_END}};
    MNodePtr  kwParams[3];
    evalParams (fev, mlenv, cell, posList, posParams, kwList, kwParams);
    ustring  pass = to_string (posParams[0]());
    passCryptFormat  format;
    format = FORMAT_MD5;
    if (to_bool (kwParams[0]()))
	format = FORMAT_MD5;
    if (to_bool (kwParams[1]()))
	format = FORMAT_SHA256;
    if (to_bool (kwParams[2]()))
	format = FORMAT_SHA512;
//    if (to_bool (kwParams[3]()))
//	format = FORMAT_BF;
    return newMNode_str (new ustring (passCrypt (pass, format)));
}

/*DOC:
===bcrypt-hash===
 (bcrypt-hash PASSWORD) -> STRING

*/
//#XAFUNC	bcrypt-hash	ml_bcrypt_hash
MNode*  ml_bcrypt_hash (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    evalParams (fev, mlenv, cell, posList, posParams);
    ustring  pass = to_string (posParams[0]());
    return newMNode_str (new ustring (passCrypt (pass, FORMAT_BF)));
}

/*DOC:
===substring===
 (substring STR INDEX LENGTH) -> STRING
 (substring STR INDEX) -> STRING

INDEX number of the first character of STR is 0.

*/
//#XAFUNC	substring	ml_substring
//#XWIKIFUNC	substring
MNode*  ml_substring (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_LIST, EV_LIST, EV_END};
    MNodePtr  posParams[3];
    evalParams (fev, mlenv, cell, posList, posParams);
    ustring  str = to_string (posParams[0]());
    size_t  index = to_int64 (posParams[1]());
    size_t  length = 0;
    int  mode;
    ustring  ans;

    if (isNil (posParams[2]())) {
	mode = 2;
    } else {
	mode = 3;
	length = to_int64 (posParams[2]());
    }
    substring (str, index, length, mode == 3, ans);
    return newMNode_str (new ustring (ans));
}
    
/*DOC:
===tail-substring===
 (tail-substring STR INDEX LENGTH) -> STRING
 (tail-substring STR INDEX) -> STRING

INDEX number of the last character of STR is 0.

*/
//#XAFUNC	tail-substring	ml_tail_substring
//#XWIKIFUNC	tail-substring
MNode*  ml_tail_substring (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_LIST, EV_LIST, EV_END};
    MNodePtr  posParams[3];
    evalParams (fev, mlenv, cell, posList, posParams);
    ustring  str = to_string (posParams[0]());
    size_t  index = to_int64 (posParams[1]());
    size_t  length = 0;
    int  mode;
    ustring  ans;
    if (isNil (posParams[2]())) {
	mode = 2;
    } else {
	mode = 3;
	length = to_int64 (posParams[2]());
    }
    size_t  s = strLength (str);
    if (mode == 3)
	substring (str, s - index - 1, length, 1, ans);
    else
	substring (str, s - index - 1, 0, 0, ans);
    return newMNode_str (new ustring (ans));
}
    
/*DOC:
===length===
 (length STRING) -> NUMBER

*/
//#XAFUNC	length	ml_length
//#XWIKIFUNC	length
MNode*  ml_length (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    evalParams (fev, mlenv, cell, posList, posParams);
    ustring  str = to_string (posParams[0]());
    size_t  ans;
    ans = strLength (str);
    return newMNode_num (ans);
}

/*DOC:
===byte-length===
 (byte-length STRING) -> NUMBER

*/
//#XAFUNC	byte-length	ml_byte_length
//#XWIKIFUNC	byte-length
MNode*  ml_byte_length (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    evalParams (fev, mlenv, cell, posList, posParams);
    ustring  str = to_string (posParams[0]());
    size_t  ans;
    ans = str.length ();
    return newMNode_num (ans);
}

/*DOC:
===pad0===
 (pad0 NUMBER STRING) -> STRING
 (pad0 NUMBER STRING_LIST) -> STRING_LIST
 (pad0 NUMBER_LIST STRING_LIST) -> STRING_LIST

*/
//#XAFUNC	pad0	ml_pad0
//#XWIKIFUNC	pad0
MNode*  ml_pad0 (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_LIST, EV_END};
    MNodePtr  posParams[2];
    evalParams (fev, mlenv, cell, posList, posParams);
    MNode*  num = posParams[0]();
    MNode*  np = num;
    MNode*  val = posParams[1]();
    MNode*  vp = val;
    int64_t  n = 0;
    MNodeList  ans;
    if (vp) {
	if (isCons (vp)) {
	    while (vp) {
		if (np) {
		    if (isCons (np)) {
			n = to_int64 (np->car ());
			np = np->cdr ();
		    } else {
			n = to_int64 (np);
		    }
		}
		ans.append (newMNode_str (new ustring (zeroPad (n, to_string (vp->car ())))));
		nextNode (vp);
	    }
	    return ans.release ();
	} else {
	    if (np) {
		if (isCons (np))
		    n = to_int64 (np->car ());
		else
		    n = to_int64 (np);
	    }
	    return newMNode_str (new ustring (zeroPad (n, to_string (vp))));
	}
    }

    return NULL;
}

/*DOC:
===ellipsis===
 (ellipsis NUM STRING) -> STRING

*/
//#XAFUNC	ellipsis	ml_ellipsis
//#XWIKIFUNC	ellipsis
MNode*  ml_ellipsis (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_LIST, EV_END};
    MNodePtr  posParams[2];
    evalParams (fev, mlenv, cell, posList, posParams);
    int64_t  num = to_int64 (posParams[0]());
    ustring  str = to_string (posParams[1]());
    str = ellipsis (str, num);
    return newMNode_str (new ustring (str));
}

/*DOC:
===string-format===
 (string-format FORMAT LIST-OF-ARGS) -> STRING
 (string-format FORMAT ARGS...) -> STRING

|h:format|h:sample|h:note|
|${''NUM''}|${1}||
|${''NUM'':hex:''NUM''}|${1:hex:4}||
|${''NUM'':HEX:''NUM''}|${1:HEX:4}||
|${''NUM'':int:''NUM''}|${1:int:5}||
|${''NUM'':int:''NUM'':c}|${1:int:5:c}||
|${''NUM'':int:''NUM'':comma}|${1:int:5:comma}||
|${''NUM'':int:''NUM'':clip}|${1:int:5:clip}||
|${''NUM'':int:''NUM'':0}|${1:int:5:0}||
|${''NUM'':float:''NUM'':''NUM''}|${1:float:4:3}||
|${''NUM'':string:''NUM''}|${1:string:20}||
|${''NUM'':string:''NUM'':right}|${1:string:20:right}||
|${''NUM'':month}|${1:month}|Jan, Feb,...|
|${''NUM'':Month}|${1:Month}|January, February,...|
|${''NUM'':week}|${1:week}|Sun, Mon,...|
|${''NUM'':Week}|${1:Week}|Sunday, Monday,...|

*/
//#XAFUNC	string-format	ml_string_format
//#XWIKIFUNC	string-format
MNode*  ml_string_format (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    MNodePtr  rest;
    evalParams (fev, mlenv, cell, posList, posParams, NULL, NULL, EV_LIST, &rest);
    ustring  format = to_string (posParams[0]());
    boost::ptr_vector<MNodePtr>  par;
    MNode*  args = rest ();
    while (args) {
	MNode*  a = args->car ();
	if (isCons (a)) {
	    MNodePtr  h;
	    h = a;
	    while (a) {
		par.push_back (new MNodePtr);
		par.back () = a->car ();
		nextNode (a);
	    }
	} else {
	    par.push_back (new MNodePtr);
	    par.back () = a;
	}
	nextNode (args);
    }
    return newMNode_str (new ustring (formatString (format, par)));
}

/*DOC:
===random-key===
 (random-key) -> STRING

*/
//#XAFUNC	random-key	ml_random_key
//#XWIKIFUNC	random-key
MNode*  ml_random_key (bool fev, MNode* cell, MlEnv* mlenv) {
    evalParams (fev, mlenv, cell);
    return newMNode_str (new ustring (randomKey ()));
}

/*DOC:
===generate-uuid===
 (generate-uuid)

*/
//#XAFUNC	generate-uuid	ml_generate_uuid
MNode*  ml_generate_uuid (bool fev, MNode* cell, MlEnv* mlenv) {
    evalParams (fev, mlenv, cell, NULL, NULL);
    const boost::uuids::uuid  id = boost::uuids::random_generator ()();
    return newMNode_str (new ustring (boost::lexical_cast<ustring> (id)));
}

/*DOC:
===to-string===
 (to-string [#bin | #oct | #hex | #HEX] [:pad NUMBER] OBJECT) -> STRING

*/
//#XAFUNC	to-string	ml_to_string
//#XWIKIFUNC	to-string
MNode*  ml_to_string (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    kwParam  kwList[] = {{CharConst ("bin"), EV_LIST}, // 0
			 {CharConst ("oct"), EV_LIST}, // 1
			 {CharConst ("hex"), EV_LIST}, // 2
			 {CharConst ("HEX"), EV_LIST}, // 3
			 {CharConst ("pad"), EV_LIST}, // 4
			 {NULL, 0, EV_END}};
    MNodePtr  kwParams[5];
    evalParams (fev, mlenv, cell, posList, posParams, kwList, kwParams);
    bool  fbin = to_bool (kwParams[0]());
    bool  foct = to_bool (kwParams[1]());
    bool  fhex = to_bool (kwParams[2]());
    bool  fHex = to_bool (kwParams[3]());
    int64_t  pad = to_int64 (kwParams[4]());
    MNode*  obj = posParams[0]();
    ustring  text;
    if (fhex || fHex || foct || fbin) {
	if (isReal (obj)) {
	    if (fhex) {
		text = tohex (to_int64 (obj), pad, 16, false);
	    } else if (fHex) {
		text = tohex (to_int64 (obj), pad, 16, true);
	    } else if (foct) {
		text = tohex (to_int64 (obj), pad, 8);
	    } else if (fbin) {
		text = tohex (to_int64 (obj), pad, 2);
	    }
	} else {
	}
    } else {
	text = to_string (obj);
    }
    return newMNode_str (new ustring (text));
}

/*DOC:
===to-symbol===
 (to-symbol STRING) -> SYMBOL

*/
//#XAFUNC	to-symbol	ml_to_symbol
//#XWIKIFUNC	to-symbol
MNode*  ml_to_symbol (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    evalParams (fev, mlenv, cell, posList, posParams);
    MNode*  text = posParams[0]();
    if (text) {
	if (isSym (text)) {
	    return mlenv->retval = text;
	} else {
	    return newMNode_sym (new ustring (text->to_string ()));
	}
    } else {
	return NULL;
    }
}

/*DOC:
===dump-to-texp, dump-to-sexp===
 (dump-to-texp OBJECT...) -> STRING
 (dump-to-sexp OBJECT...) -> STRING

*/
//#XAFUNC	dump-to-texp	ml_dump_to_texp
//#XAFUNC	dump-to-sexp	ml_dump_to_texp
//#XWIKIFUNC	dump-to-texp
//#XWIKIFUNC	dump-to-sexp
MNode*  ml_dump_to_texp (bool fev, MNode* cell, MlEnv* mlenv) {
    MNodePtr  rest;
    evalParams (fev, mlenv, cell, EV_LIST, &rest);
    ustring  text;
    MNode*  args = rest ();
    while (args) {
	MNode*  e = args->car ();
	nextNode (args);
	if (text.length () > 0)
	    text.append (CharConst (" "));
	text.append (dump_to_texp (e));
    }
    return newMNode_str (new ustring (text));
}

/*DOC:
===read-texp, read-sexp===
 (read-sexp STRING) -> OBJECT
 (read-texp STRING) -> OBJECT

*/
//#XAFUNC	read-texp	ml_read_texp
//#XAFUNC	read-sexp	ml_read_texp
//#XWIKIFUNC	read-texp
//#XWIKIFUNC	read-sexp
MNode*  ml_read_texp (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    evalParams (fev, mlenv, cell, posList, posParams);
    ustring  text = to_string (posParams[0]());
    MotorTexp  ml (NULL);
    ml.scan (text);
    if (ml.top.isCons () && isCons (ml.top.cdr ()))
	return mlenv->retval = ml.top.cdr ()->car ();
    else
	return NULL;
}

/*DOC:
===read-texps, read-sexps===
 (read-sexps STRING) -> (OBJECT ...)
 (read-texps STRING) -> (OBJECT ...)

*/
//#XAFUNC	read-texps	ml_read_texps
//#XAFUNC	read-sexps	ml_read_texps
//#XWIKIFUNC	read-texps
//#XWIKIFUNC	read-sexps
MNode*  ml_read_texps (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    evalParams (fev, mlenv, cell, posList, posParams);
    ustring  text = to_string (posParams[0]());
    MotorTexp  ml (NULL);
    ml.scan (text);
    if (ml.top.isCons () && isCons (ml.top.cdr ()))
	return mlenv->retval = ml.top.cdr ();
    else
	return NULL;
}

/*DOC:
===is-ascii63===
 (is-ascii63 STRING) -> BOOL

*/
//#XAFUNC	is-ascii63	ml_is_ascii63
//#XWIKIFUNC	is-ascii63
MNode*  ml_is_ascii63 (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    evalParams (fev, mlenv, cell, posList, posParams);
    ustring  text = to_string (posParams[0]());
    bool  ans = matchASCII (text.begin (), text.end ());
    return newMNode_bool (ans);
}

static MNode*  sort_string_list (MNode* a, bool fdesc, MlEnv* mlenv) {
    std::vector<MNode*>  list;
    MNodeList  ans;
    while (a) {
	if (isCons (a)) {
	    if (isStr (a->car ())) {
		list.push_back (a->car ());
	    } else {
		list.push_back (NULL);
	    }
	    nextNode (a);
	} else {
	    break;
	}
    }

    int  s, i, j, k;
    int  n = list.size ();
    for (i = 1; i < n; i ++) {
	j = i;
	while (j > 0) {
	    k = (j - 1) / 2;
	    if (! list[k])
		if (! list[j])
		    break;
		else
		    if (fdesc)
			break;
		    else ;
	    else if (! list[j])
		if (fdesc)
		    ;
		else
		    break;
	    else if (fdesc ^ (*list[k]->value_str () >= *list[j]->value_str ()))
		break;
//	    swap (v[k], v[j]);
	    a = list[j]; list[j] = list[k]; list[k] = a;
	    j = k;
	}
    }
    for (; n > 0; n --) {
//	swap (v[0], v[n - 1]);
	a = list[n - 1]; list[n - 1] = list[0]; list[0] = a;
	for (i = 1; i < n - 1; i ++) {
	    j = i;
	    while (j > 0) {
		k = (j - 1) / 2;
//		if (! list[k] || ! list[j])
//		    break;
		if (! list[k])
		    if (! list[j])
			break;
		    else
			if (fdesc)
			    break;
			else ;
		else if (! list[j])
		    if (fdesc)
			;
		    else
			break;
		else if (fdesc ^ (*list[k]->value_str () >= *list[j]->value_str ()))
		    break;
//		swap (v[k], v[j]);
		a = list[j]; list[j] = list[k]; list[k] = a;
		j = k;
	    }
	}
    }

    n = list.size ();
    for (i = 0; i < n; i ++) {
	ans.append (list[i]);
    }
    return ans.release ();
}

static MNode*  sort_string_vec (MNode* a, bool fdesc, MlEnv* mlenv) {
    MNodePtr  ans;
    ans = newMNode_vector ();
    size_t  it, iu;
    iu = a->vectorSize ();
    for (it = 0; it < iu; ++ it) {
	MNode*  x = a->vectorGet (it);
	if (isStr (x)) {
	    ans ()->vectorPush (x);
	} else {
	    ans ()->vectorPush (NULL);
	}
    }
    MotorVector*  vec = ans ()->value_vector ();

    int  s, i, j, k;
    int  n = vec->size ();
    for (i = 1; i < n; i ++) {
	j = i;
	while (j > 0) {
	    k = (j - 1) / 2;
	    if (! (*vec)[k]())
		if (! (*vec)[j]())
		    break;
		else
		    if (fdesc)
			break;
		    else ;
	    else if (! (*vec)[j]())
		if (fdesc)
		    ;
		else
		    break;
	    else if (fdesc ^ (*(*vec)[k]()->value_str () >= *(*vec)[j]()->value_str ()))
		break;
//	    swap (v[k], v[j]);
	    a = (*vec)[j](); (*vec)[j] = (*vec)[k](); (*vec)[k] = a;
	    j = k;
	}
    }
    for (; n > 0; n --) {
//	swap (v[0], v[n - 1]);
	a = (*vec)[n - 1](); (*vec)[n - 1] = (*vec)[0](); (*vec)[0] = a;
	for (i = 1; i < n - 1; i ++) {
	    j = i;
	    while (j > 0) {
		k = (j - 1) / 2;
//		if (! (*vec)[k]() || ! (*vec)[j]())
//		    break;
		if (! (*vec)[k]())
		    if (! (*vec)[j]())
			break;
		    else
			if (fdesc)
			    break;
			else ;
		else if (! (*vec)[j]())
		    if (fdesc)
			;
		    else
			break;
		else if (fdesc ^ (*(*vec)[k]()->value_str () >= *(*vec)[j]()->value_str ()))
		    break;
//		swap (v[k], v[j]);
		a = (*vec)[j](); (*vec)[j] = (*vec)[k](); (*vec)[k] = a;
		j = k;
	    }
	}
    }
    return mlenv->retval = ans.release ();
}

/*DOC:
===sort-string===
 (sort-string LIST [#asc] [#desc]) -> LIST

*/
//#XAFUNC	sort-string	ml_sort_string
//#XWIKIFUNC	sort-string
MNode*  ml_sort_string (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    kwParam  kwList[] = {{CharConst ("asc"), EV_LIST},
			 {CharConst ("desc"), EV_LIST},
			 {NULL, 0, EV_END}};
    MNodePtr  kwParams[2];
    evalParams (fev, mlenv, cell, posList, posParams, kwList, kwParams);
    bool  fdesc = false;
    if (to_bool (kwParams[0]()))
	fdesc = false;
    if (to_bool (kwParams[1]()))
	fdesc = true;
    MNode*  a = posParams[0]();
    if (isCons (a)) {
	return sort_string_list (a, fdesc, mlenv);
    } else if (isVector (a)) {
	return sort_string_vec (a, fdesc, mlenv);
    } else if (isNil (a)) {
	return NULL;
    } else {
	throw (a->dump_string_short () + ": bad type.");
    }
}

/*DOC:
===to-upper===
 (to-upper STRING) -> STRING

*/
//#XAFUNC	to-upper	ml_to_upper
//#XWIKIFUNC	to-upper
MNode*  ml_to_upper (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    evalParams (fev, mlenv, cell, posList, posParams);
    ustring  text = to_string (posParams[0]());
    return newMNode_str (new ustring (toUpper (text)));
}

/*DOC:
===to-lower===
 (to-lower STRING) -> STRING

*/
//#XAFUNC	to-lower	ml_to_lower
//#XWIKIFUNC	to-lower
MNode*  ml_to_lower (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    evalParams (fev, mlenv, cell, posList, posParams);
    ustring  text = to_string (posParams[0]());
    return newMNode_str (new ustring (toLower (text)));
}

/*DOC:
===dirname===
 (dirname PATH [FILENAME]) -> STRING

*/
//#XAFUNC	dirname	ml_dirname
//#XWIKIFUNC	dirname
MNode*  ml_dirname (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_LIST, EV_END};
    MNodePtr  posParams[2];
    evalParams (fev, mlenv, cell, posList, posParams);
    ustring  text = to_string (posParams[0]());
    if (text.length () == 0) {
    } else {
	size_t  p = text.rfind ('/');
	if (p != ustring::npos) {
	    text = text.substr (0, p + 1);
	} else if (text.back () != '/') {
	    text.append (CharConst ("/"));
	}
    }
    if (isNil (posParams[1] ())) {
	return newMNode_str (new ustring (text));
    } else {
	return newMNode_str (new ustring (text + to_string (posParams[1]())));
    }
}

/*DOC:
===string-duplicate===
 (string-duplicate STRING N) -> STRING
 N <= 1000
*/
//#XAFUNC	string-duplicate	ml_string_duplicate
//#XWIKIFUNC	string-duplicate
MNode*  ml_string_duplicate (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_LIST, EV_END};
    MNodePtr  posParams[2];
    evalParams (fev, mlenv, cell, posList, posParams);
    ustring  text = to_string (posParams[0]());
    int64_t  n = to_int64 (posParams[1]());
    if (n < 0) 
	return newMNode_str (new ustring (text));
    if (n > 1000)
	n = 1000;
    ustring*  ans = new ustring;
    for (int i = 0; i < n; ++ i) {
	ans->append (text);
    }
    return newMNode_str (ans);
}

static  void flatten(ustring& str, MNode* el) {
    if (isNil (el)) {
	// skip
    } else if (isCons (el)) {
	while (el) {
	    MNode*  a = el->car ();
	    flatten(str, a);
	    nextNode (el);
	}
    } else {
	str.append (to_string (el));
    }
}

/*DOC:
===string-flatten===
 (string-flatten LIST) -> STRING

*/
//#XAFUNC	string-flatten	ml_string_flatten
//#XWIKIFUNC	string-flatten
MNode*  ml_string_flatten (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    evalParams (fev, mlenv, cell, posList, posParams);
    MNode*  list = posParams[0]();
    ustring  ans;
    flatten (ans, list);
    return newMNode_str (new ustring (ans));
}

