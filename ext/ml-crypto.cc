//#include "ml-json.h"
#include "bf.h"
#include "ml.h"
#include "mlenv.h"
#include "motorenv.h"
#include "motoroutput.h"
#include "util_base64.h"
#include "ustring.h"
#include "expr.h"
#include "utf8.h"
#include <openssl/blowfish.h>
#include <exception>

/*DOC:
==encrypt with blowfish==

*/

static bool  hex (int c, int& ans) {
    if ('0' <= c && c <= '9') {
	ans = c - '0';
	return true;
    } else if ('a' <= c && c <= 'f') {
	ans = c -  'a' + 10;
	return true;
    } else if ('A' <= c && c <= 'F') {
	ans = c - 'A' + 10;
	return true;
    } else {
	return false;
    }
}

static ustring  hexToBin (const ustring& text) {
    uiterator  b, e;
    ustring  ans;		// binary
    int  c1, c2;

    b = text.begin ();
    e = text.end ();
    while (b < e) {
	if (hex (*b, c1)) {
	    b ++;
	    if (hex (*b, c2)) {
		b ++;
		ans.append (1, c1 * 16 + c2);
	    } else {
		break;
	    }
	} else if (*b == ' ') {
	    b ++;
	} else {
	    break;
	}
    }
    return ans;
}

/*DOC:
===bf-cbc-encode===
 (bf-cbc-encode [:key KEY | :key-hex HEX-STRING] [:iv STRING | :iv-hex HEX-STRING] [#zero-pad] [#pkcs5-pad] STRING) -> STRING
//PKCS5Padding

初期化ベクタ固定の場合、秘密保持目的の使用には弱い。

*/
//#XAFUNC	bf-cbc-encode	ml_bf_cbc_encode
MNode*  ml_bf_cbc_encode (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    kwParam  kwList[] = {{CharConst ("key"), EV_LIST},	     // 0
			 {CharConst ("iv"), EV_LIST},	     // 1
			 {CharConst ("zero-pad"), EV_LIST},  // 2
			 {CharConst ("pkcs5-pad"), EV_LIST}, // 3
			 {CharConst ("key-hex"), EV_LIST},   // 4
			 {CharConst ("iv-hex"), EV_LIST},    // 5
			 {NULL, 0, EV_END}};
    MNodePtr  kwParams[6];
    evalParams (fev, mlenv, cell, posList, posParams, kwList, kwParams);
    ustring  text = to_string (posParams[0]());
    ustring  key;
    if (! isNil (kwParams[0]()))
	key = to_string (kwParams[0]());
    else if (! isNil (kwParams[4]()))
	key = hexToBin (to_string (kwParams[4]()));
    else
	key.assign (CharConst (k_crypto_bf_key));
    ustring  iv;
    if (! isNil (kwParams[1]()))
	iv = to_string (kwParams[1]());
    else if (! isNil (kwParams[5]()))
	iv = hexToBin (to_string (kwParams[5]()));
    else
	iv.assign (CharConst (k_crypto_bf_iv));
    enum {
	PAD_ZERO,
	PAD_PKCS5,
    }  padding = PAD_PKCS5;
    if (to_bool (kwParams[2]()))
	padding = PAD_ZERO;
    if (to_bool (kwParams[3]()))
	padding = PAD_PKCS5;
    BF_KEY  bfkey;
    unsigned char  ivb[8];
    BF_set_key (&bfkey, key.length (), uchar_type (key.data ()));
    if (iv.length () >= 8) {
	memcpy (ivb, iv.data (), 8);
    } else {
	memset (ivb, 0, 8);
	memcpy (ivb, iv.data (), iv.length ());
    }
    int  r;
    r = text.length () % 8;
    switch (padding) {
    case PAD_ZERO:
	if (r > 0) {
	    text.append (8 - r, 0);
	}
	break;
    case PAD_PKCS5:
	if (r > 0) {
	    text.append (8 - r, (char)(8 - r));
	} else {
	    text.append (8, (char)8);
	}
	break;
    default:
	assert (0);
    }
    ustring  ans;
    BF_cbc_encrypt ((u_char*)text.data (), (u_char*)ans.data (), text.length (), &bfkey, ivb, BF_ENCRYPT);

    return newMNode_str (new ustring (base64Encode (ans.begin (), ans.end ())));
}

/*DOC:
===bf-cbc-decode===
 (bf-cbc-decode [:key KEY | :key-hex HEX-STRING] [:iv STRING | :iv-hex HEX-STRING] [#zero-pad] [#pkcs5-pad] BASE64-STRING) -> BASE64-STRING

*/
//#XAFUNC	bf-cbc-decode	ml_bf_cbc_decode
MNode*  ml_bf_cbc_decode (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    kwParam  kwList[] = {{CharConst ("key"), EV_LIST},	     // 0
			 {CharConst ("iv"), EV_LIST},	     // 1
			 {CharConst ("zero-pad"), EV_LIST},  // 2
			 {CharConst ("pkcs5-pad"), EV_LIST}, // 3
			 {CharConst ("key-hex"), EV_LIST},   // 4
			 {CharConst ("iv-hex"), EV_LIST},    // 5
			 {NULL, 0, EV_END}};
    MNodePtr  kwParams[6];
    evalParams (fev, mlenv, cell, posList, posParams, kwList, kwParams);
    ustring  text = to_string (posParams[0]());
    ustring  key;
    if (! isNil (kwParams[0]()))
	key = to_string (kwParams[0]());
    else if (! isNil (kwParams[4]()))
	key = hexToBin (to_string (kwParams[4]()));
    else
	key.assign (CharConst (k_crypto_bf_key));
    ustring  iv;		// 8byte
    if (! isNil (kwParams[1]()))
	iv = to_string (kwParams[1]());
    else if (! isNil (kwParams[5]()))
	iv = hexToBin (to_string (kwParams[5]()));
    else
	iv.assign (CharConst (k_crypto_bf_iv));
    crypto_padding_t  padding = PAD_PKCS5;
    if (to_bool (kwParams[2]()))
	padding = PAD_ZERO;
    else if (to_bool (kwParams[3]()))
	padding = PAD_PKCS5;
    int  r = text.length () % 8;
    if (r > 0)
	return NULL;		// error
    switch (padding) {
    case PAD_PKCS5:
	if (text.length () == 0)
	    return NULL;
	break;
    case PAD_ZERO:
	break;
    }
    ustring  ans;
    if (! bf_cbc_decode (ans, text, key, iv, padding))
	throw (ustring (CharConst ("PKCS5 padding error")));
    ans = fixUTF8 (ans);
    return newMNode_str (new ustring (ans));
}
