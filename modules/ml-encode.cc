#include "ml-encode.h"
#include "ml.h"
#include "mlenv.h"
#include "motorenv.h"
#include "motoroutput.h"
#include "expr.h"
#include "util_const.h"
#include "util_base64.h"
#include "util_string.h"
#include "utf8.h"
#include "ustring.h"
//#include <boost/ptr_container/ptr_vector.hpp>
#include <vector>
#include <exception>

/*DOC:
==encoding functions==

*/

/*DOC:
===encode-control-char===
 (encode-control-char STRING...) -> STRING

制御文字，「\（バックスラッシュ）」，「"（ダブルクオート）」をバックスラッシュでエンコードする。

*/
//#XAFUNC	encode-control-char	ml_encode_control_char
//#XWIKIFUNC	encode-control-char
MNode*  ml_encode_control_char (bool fev, MNode* cell, MlEnv* mlenv) {
    MNodePtr  rest;
    evalParams (fev, mlenv, cell, EV_LIST, &rest);
    MNode*  args = rest ();
    ustring  str;
    while (args) {
	str.append (to_string (args->car ()));
	nextNode (args);
    }
    return newMNode_str (new ustring (slashEncode (str)));
}

/*DOC:
===decode-control-char===
 (decode-control-char STRING...) -> STRING

*/
//#XAFUNC	decode-control-char	ml_decode_control_char
//#XWIKIFUNC	decode-control-char
MNode*  ml_decode_control_char (bool fev, MNode* cell, MlEnv* mlenv) {
    MNodePtr  rest;
    evalParams (fev, mlenv, cell, EV_LIST, &rest);
    MNode*  args = rest ();
    ustring  str;
    while (args) {
	str.append (to_string (args->car ()));
	nextNode (args);
    }
    return newMNode_str (new ustring (fixUTF8 (slashDecode (str))));
}

/*DOC:
===escape-wiki===
 (escape-wiki STRING ...) -> STRING

*/
//#XAFUNC	escape-wiki	ml_escape_wiki
//#XWIKIFUNC	escape-wiki
MNode*  ml_escape_wiki (bool fev, MNode* cell, MlEnv* mlenv) {
    MNodePtr  rest;
    evalParams (fev, mlenv, cell, EV_LIST, &rest);
    MNode*  args = rest ();
    ustring  text;
    static uregex  re ("([{}:|&<>\'\\\\])|(\n)");
    while (args) {
	text.append (to_string (args->car ()));
	nextNode (args);
    }
    uiterator  b = text.begin ();
    uiterator  e = text.end ();
    umatch  m;
    ustring  ans;
    while (b < e && usearch (b, e, m, re)) {
	if (b < m[0].first)
	    ans.append (b, m[0].first);
	if (m[1].matched) {
//	    ans.append (CharConst ("\\")).append (m[1].first, m[1].second);
	    ans.append (uBSlash).append (m[1].first, m[1].second); // "\\"
	} else if (m[2].matched) {
	    ans.append (uSPC);
	} else {
	    assert (0);
	}
	b = m[0].second;
    }
    if (b < e)
	ans.append (b, e);
    return newMNode_str (new ustring (ans));
}

/*DOC:
===percent-encode===
 (percent-encode #path STRING...) -> STRING

*/
//#XAFUNC	urlencode	ml_percent_encode
//#XWIKIFUNC	urlencode
//#XAFUNC	percent-encode	ml_percent_encode
//#XWIKIFUNC	percent-encode
MNode*  ml_percent_encode (bool fev, MNode* cell, MlEnv* mlenv) {
    kwParam  kwList[] = {{CharConst ("path"), EV_LIST},		// 0
			 {NULL, 0, EV_END}};
    MNodePtr  kwParams[1];
    MNodePtr  rest;
    evalParams (fev, mlenv, cell, NULL, NULL, kwList, kwParams, EV_LIST, &rest);
    MNode*  args = rest ();
    ustring  str;
    while (args) {
	str.append (to_string (args->car ()));
	nextNode (args);
    }
    if (to_bool (kwParams[0] ())) {		// path
	return newMNode_str (new ustring (percentEncode_path (str)));
    } else {
	return newMNode_str (new ustring (percentEncode (str)));
    }
}

/*DOC:
===base64-encode===
 (base64-encode STRING...) -> STRING

*/
//#XAFUNC	base64-encode	ml_base64_encode
//#XWIKIFUNC	base64-encode
MNode*  ml_base64_encode (bool fev, MNode* cell, MlEnv* mlenv) {
    MNodePtr  rest;
    evalParams (fev, mlenv, cell, EV_LIST, &rest);
    MNode*  args = rest ();
    ustring  str;
    while (args) {
	str.append (to_string (args->car ()));
	nextNode (args);
    }
    str = base64Encode (str.begin (), str.end ());
    return newMNode_str (new ustring (str));
}

/*DOC:
===base64-decode===
 (base64-decode [#hex | #HEX] STRING...) -> STRING

*/
//#XAFUNC	base64-decode	ml_base64_decode
//#XWIKIFUNC	base64-decode
MNode*  ml_base64_decode (bool fev, MNode* cell, MlEnv* mlenv) {
    kwParam  kwList[] = {{CharConst ("hex"), EV_LIST},
			 {CharConst ("HEX"), EV_LIST},
			 {NULL, 0, EV_END}};
    MNodePtr  kwParams[2];
    MNodePtr  rest;
    evalParams (fev, mlenv, cell, NULL, NULL, kwList, kwParams, EV_LIST, &rest);
    MNode*  args = rest ();
    bool  fhex = to_bool (kwParams[0]());
    bool  fHex = to_bool (kwParams[1]());
    ustring  str;
    while (args) {
	str.append (to_string (args->car ()));
	nextNode (args);
    }
    str = base64Decode (str.begin (), str.end ());
    if (fhex) {
	str = hexEncode (str);
    } else if (fHex) {
	str = hexEncode (str, true);
    } else {
	str = fixUTF8 (str);
    }
    return newMNode_str (new ustring (str));
}

/*DOC:
===base64-url-encode===
 (base64-url-encode STRING...) -> STRING

*/
//#XAFUNC	base64-url-encode	ml_base64_url_encode
//#XWIKIFUNC	base64-url-encode
MNode*  ml_base64_url_encode (bool fev, MNode* cell, MlEnv* mlenv) {
    MNodePtr  rest;
    evalParams (fev, mlenv, cell, EV_LIST, &rest);
    MNode*  args = rest ();
    ustring  str;
    while (args) {
	str.append (to_string (args->car ()));
	nextNode (args);
    }
    str = base64URLEncode (str.begin (), str.end ());
    return newMNode_str (new ustring (str));
}

/*DOC:
===base64-url-decode===
 (base64-url-decode STRING...) -> STRING

*/
//#XAFUNC	base64-url-decode	ml_base64_url_decode
//#XWIKIFUNC	base64-url-decode
MNode*  ml_base64_url_decode (bool fev, MNode* cell, MlEnv* mlenv) {
    MNodePtr  rest;
    evalParams (fev, mlenv, cell, EV_LIST, &rest);
    MNode*  args = rest ();
    ustring  str;
    while (args) {
	str.append (to_string (args->car ()));
	nextNode (args);
    }
    str = base64URLDecode (str.begin (), str.end ());
    return newMNode_str (new ustring (fixUTF8 (str)));
}

/*DOC:
===hex-encode===
 (hex-encode [#HEX] STRING...) -> STRING
 (string-to-hex [#HEX] STRING ...) -> STRING

*/
//#XAFUNC	hex-encode	ml_hex_encode
//#XAFUNC	string-to-hex	ml_hex_encode
//#XWIKIFUNC	hex-encode
//#XWIKIFUNC	string-to-hex
MNode*  ml_hex_encode (bool fev, MNode* cell, MlEnv* mlenv) {
    kwParam  kwList[] = {{CharConst ("HEX"), EV_LIST},
			 {NULL, 0, EV_END}};
    MNodePtr  kwParams[1];
    MNodePtr  rest;
    evalParams (fev, mlenv, cell, EV_LIST, &rest);
    MNode*  args = rest ();
    bool  fHex = to_bool (kwParams[0]());
    ustring  text;
    ustring  ans;
    while (args) {
	text = to_string (args->car ());
	nextNode (args);
	ans.append (hexEncode (text, fHex));
    }
    return newMNode_str (new ustring (ans));
}

/*DOC:
===hex-decode===
 (hex-decode [#ctrl] STRING...) -> STRING
 (hex-to-string [#ctrl] STRING ...) -> STRING

*/
//#XAFUNC	hex-decode	ml_hex_decode
//#XAFUNC	hex-to-string	ml_hex_decode
//#XWIKIFUNC	hex-decode
//#XWIKIFUNC	hex-to-string
MNode*  ml_hex_decode (bool fev, MNode* cell, MlEnv* mlenv) {
    kwParam  kwList[] = {{CharConst ("ctrl"), EV_LIST},
			 {NULL, 0, EV_END}};
    MNodePtr  kwParams[1];
    MNodePtr  rest;
    evalParams (fev, mlenv, cell, EV_LIST, &rest);
    MNode*  args = rest ();
    bool  fctrl = to_bool (kwParams[0]());
    ustring  text;
    ustring  ans;
    while (args) {
	text = to_string (args->car ());
	nextNode (args);
	ans.append (hexDecode (text));
    }
    if (fctrl)
	return newMNode_str (new ustring (fixUTF8 (ans)));
    else
	return newMNode_str (new ustring (omitCtrlX (fixUTF8 (ans))));
}
