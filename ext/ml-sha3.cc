#include "config.h"
#include "ml-security.h"
#include "ml.h"
#include "mlenv.h"
#include "expr.h"
#include "util_base64.h"
#include "util_string.h"
#include "ustring.h"
#include "sha3.h"

static ustring  fn_sha3 (const ustring& text);

static ustring  fn_sha3 (const ustring& text) {
    u_char  buf[32];

    sha3_256 (buf, 32, uchar_type (text.c_str ()), text.length ());
    return ustring (char_type (buf), 32);
}

/*--DOC:
===sha3-string===
 (sha3-string TEXT [#src-hex | #src-base64] [#hex | #base64]) -> STRING

*/
//#XAFUNC	sha3-string	ml_sha3_string
//#XWIKIFUNC	sha3-string
MNode*  ml_sha3_string (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    kwParam  kwList[] = {{CharConst ("src-hex"), EV_LIST},    // 0
			 {CharConst ("src-base64"), EV_LIST}, // 1
			 {CharConst ("hex"), EV_LIST},	      // 2
			 {CharConst ("base64"), EV_LIST},     // 3
			 {NULL, 0, EV_END}};
    MNodePtr  kwParams[4];
    evalParams (fev, mlenv, cell, posList, posParams, kwList, kwParams);
    ustring  src = to_string (posParams[0]());
    if (to_bool (kwParams[0]()))
	src = hexDecode (src);
    else if (to_bool (kwParams[1]()))
	src = base64Decode (src.begin (), src.end ());
    enum {
	ENC_HEX,
	ENC_BASE64,
    }  encode = ENC_HEX;
    if (to_bool (kwParams[2]()))
	encode = ENC_HEX;
    else if (to_bool (kwParams[3]()))
	encode = ENC_BASE64;
    ustring  ans = fn_sha3 (src);
    switch (encode) {
    case ENC_HEX:
	ans = hexEncode (ans);
	break;
    case ENC_BASE64:
	ans = base64Encode (ans.begin (), ans.end ());
	break;
    default:
	assert (0);
    }
    return newMNode_str (new ustring (ans));
}

