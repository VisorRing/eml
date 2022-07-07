#include "config.h"
#include "ml-security.h"
#include "ml-store.h"
#include "ml.h"
#include "mlenv.h"
#include "expr.h"
#include "util_base64.h"
#include "util_string.h"
#include "filemacro.h"
#include "ustring.h"
#include <md5.h>
#include <openssl/hmac.h>
#include <openssl/sha.h>
#include <sys/types.h>

#define  S4K	4096

/*DOC:
==cryptographic functions

*/
static MNode*  ml_hmac_sub (bool fev, const EVP_MD* fn, MNode* cell, MlEnv* mlenv);
static MNode*  ml_hash_sub (bool fev, ustring fn1 (const ustring& text), ustring fn2 (FileMacro& f), MNode* cell, MlEnv* mlenv);
static ustring  fn_md5 (const ustring& text);
static ustring  fn_md5_file (FileMacro& f);
static ustring  fn_sha1 (const ustring& text);
static ustring  fn_sha1_file (FileMacro& f);
static ustring  fn_sha256 (const ustring& text);
static ustring  fn_sha256_file (FileMacro& f);

/*DOC:
===hmac-sha1===
 (hmac-sha1 KEY_STRING TEXT [#hex] [#base64]) => STRING
 (hmac-sha1 [:key-serial FILENAME | :key-named FILENAME | :key-static FILENAME | :key-text TEXT] [:message-serial FILENAME | :message-named FILENAME | :message-static FILENAME | :message-text TEXT] [#hex] [#base64]) => STRING

*/
//#XAFUNC	hmac-sha1	ml_hmac_sha1
//#XWIKIFUNC	hmac-sha1
MNode*  ml_hmac_sha1 (bool fev, MNode* cell, MlEnv* mlenv) {
    return ml_hmac_sub (fev, EVP_sha1 (), cell, mlenv);
}

/*DOC:
===hmac-sha256===
 (hmac-sha256 KEY_STRING TEXT [#hex] [#base64] [#decode-key-hex | #decode-key-base64] [#decode-message-hex | #decode-message-base64]) => STRING
 (hmac-sha256 [:key-serial FILENAME | :key-named FILENAME | :key-static FILENAME | :key-text TEXT] [:message-serial FILENAME | :message-named FILENAME | :message-static FILENAME | :message-text TEXT] [#hex] [#base64] [#decode-key-hex | #decode-key-base64] [#decode-message-hex | #decode-message-base64]) => STRING

^#decode-message-hex, #decode-message-base64オプションは、ファイル入力には適用されない
。

*/
//#XAFUNC	hmac-sha256	ml_hmac_sha256
//#XWIKIFUNC	hmac-sha256
MNode*  ml_hmac_sha256 (bool fev, MNode* cell, MlEnv* mlenv) {
    return ml_hmac_sub (fev, EVP_sha256 (), cell, mlenv);
}

typedef enum {
    ENC_NONE,
    ENC_HEX,
    ENC_BASE64,
}  text_encode_t;

MNode*  ml_hmac_sub (bool fev, const EVP_MD* fn, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_LIST, EV_END};
    MNodePtr  posParams[2];
    kwParam  kwList[] = {{CharConst ("hex"), EV_LIST},		  // 0
			 {CharConst ("base64"), EV_LIST},	  // 1
			 {CharConst ("key-serial"), EV_LIST},	  // 2
			 {CharConst ("key-named"), EV_LIST},	  // 3
			 {CharConst ("key-static"), EV_LIST},	  // 4
			 {CharConst ("key-text"), EV_LIST},	  // 5
			 {CharConst ("message-serial"), EV_LIST}, // 6
			 {CharConst ("message-named"), EV_LIST},  // 7
			 {CharConst ("message-static"), EV_LIST}, // 8
			 {CharConst ("message-text"), EV_LIST},	  // 9
			 {CharConst ("decode-key-hex"), EV_LIST}, // 10
			 {CharConst ("decode-key-base64"), EV_LIST}, // 11
			 {CharConst ("decode-message-hex"), EV_LIST}, // 12
			 {CharConst ("decode-message-base64"), EV_LIST}, // 13
			 {NULL, 0, EV_END}};
    MNodePtr  kwParams[14];
    evalParams (fev, mlenv, cell, posList, posParams, kwList, kwParams);
    StoreType  keyType (mlenv);
    StoreType  msgType (mlenv);
    if (! isNil (posParams[0]()))
	keyType.srcText (to_string (posParams[0]()));
    if (! isNil (posParams[1]()))
	msgType.srcText (to_string (posParams[1]()));
    text_encode_t  encode = ENC_HEX;
    if (to_bool (kwParams[0]()))
	encode = ENC_HEX;
    if (to_bool (kwParams[1]()))
	encode = ENC_BASE64;
    if (! isNil (kwParams[2]()))
	keyType.srcSerial (to_string (kwParams[2]()));	// :key-serial
    if (! isNil (kwParams[3]()))
	keyType.srcNamed (to_string (kwParams[3]()));	// :key-named
    if (! isNil (kwParams[4]()))
	keyType.srcStatic (to_string (kwParams[4]()));	// :key-static
    if (! isNil (kwParams[5]()))
	keyType.srcText (to_string (kwParams[5]()));	// :key-text
    if (! isNil (kwParams[6]()))
	msgType.srcSerial (to_string (kwParams[6]()));	// :message-serial
    if (! isNil (kwParams[7]()))
	msgType.srcNamed (to_string (kwParams[7]()));	// :message-named
    if (! isNil (kwParams[8]()))
	msgType.srcStatic (to_string (kwParams[8]()));	// :message-static
    if (! isNil (kwParams[9]()))
	msgType.srcText (to_string (kwParams[9]()));	// :message-text
    text_encode_t  decodeKey = ENC_NONE;
    if (! isNil (kwParams[10]()))		// #decode-key-hex
	decodeKey = ENC_HEX;
    if (! isNil (kwParams[11]()))		// #decode-key-base64
	decodeKey = ENC_BASE64;
    text_encode_t  decodeMsg = ENC_NONE;
    if (! isNil (kwParams[12]()))		// #decode-message-hex
	decodeMsg = ENC_HEX;
    if (! isNil (kwParams[13]()))		// #decode-message-base64
	decodeMsg = ENC_BASE64;
    ustring  key;
    if (keyType ())
	key = keyType.read ();
    switch (decodeKey) {
    case ENC_HEX:
	key = hexDecode (key);
	break;
    case ENC_BASE64:
	key = base64Decode (key.begin (), key.end ());
	break;
    default:;
    }
    ustring  text;
    unsigned char  md[EVP_MAX_MD_SIZE];
    unsigned int  md_len = 0;
    if (msgType.isText ()) {
	text = msgType.read ();
	switch (decodeMsg) {
	case ENC_HEX:
	    text = hexDecode (text);
	    break;
	case ENC_BASE64:
	    text = base64Decode (text.begin (), text.end ());
	    break;
	default:;
	}
	HMAC (fn, key.c_str (), key.length (), uchar_type (text.c_str ()), text.length (), md, &md_len);
    } else {
#if OPENSSL_VERSION_NUMBER < 0x10100000L
	HMAC_CTX  hmac;
	FileMacro  f;
	ssize_t  len;
	u_char  buf[S4K];
	if (f.openRead (msgType.src ().c_str ())) {
	    HMAC_Init (&hmac, key.c_str (), key.length (), fn);
	    while ((len = f.read (buf, S4K)) > 0) {
		HMAC_Update (&hmac, buf, len);
	    }
	    HMAC_Final (&hmac, md, &md_len);
	}
#else
	HMAC_CTX*  hmac;
	FileMacro  f;
	ssize_t  len;
	u_char  buf[S4K];
	if (f.openRead (msgType.src ().c_str ())) {
	    hmac = HMAC_CTX_new ();
	    HMAC_Init_ex (hmac, key.c_str (), key.length (), fn, NULL);
	    while ((len = f.read (buf, S4K)) > 0) {
		HMAC_Update (hmac, buf, len);
	    }
	    HMAC_Final (hmac, md, &md_len);
	    HMAC_CTX_free (hmac);
	}
#endif
    }
    ustring  ans;
    ans.assign (char_type (md), md_len);
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

static ustring  fn_md5 (const ustring& text) {
    MD5_CTX  ctx;
    u_char  buf[16];

    MD5Init (&ctx);
    MD5Update (&ctx, text.c_str (), text.length ());
    MD5Final (buf, &ctx);
    return ustring (char_type (buf), 16);
}

static ustring  fn_md5_file (FileMacro& f) {
    MD5_CTX  ctx;
    ssize_t  len;
    u_char  buf[S4K];

    MD5Init (&ctx);
    while ((len = f.read (buf, S4K)) > 0) {
	MD5Update (&ctx, buf, len);
    }
    MD5Final (buf, &ctx);
    return ustring (char_type (buf), 16);
}

static ustring  fn_sha1 (const ustring& text) {
    SHA_CTX  ctx;
    u_char  buf[SHA_DIGEST_LENGTH];

    SHA1_Init (&ctx);
    SHA1_Update (&ctx, text.c_str (), text.length ());
    SHA1_Final (buf, &ctx);
    return ustring (char_type (buf), SHA_DIGEST_LENGTH);
}

static ustring  fn_sha1_file (FileMacro& f) {
    SHA_CTX  ctx;
    ssize_t  len;
    u_char  buf[S4K];

    SHA1_Init (&ctx);
    while ((len = f.read (buf, S4K)) > 0) {
	SHA1_Update (&ctx, buf, len);
    }
    SHA1_Final (buf, &ctx);
    return ustring (char_type (buf), SHA_DIGEST_LENGTH);
}

static ustring  fn_sha256 (const ustring& text) {
    SHA256_CTX  ctx;
    u_char  buf[32];

    SHA256_Init (&ctx);
    SHA256_Update (&ctx, text.c_str (), text.length ());
    SHA256_Final (buf, &ctx);
    return ustring (char_type (buf), 32);
}

static ustring  fn_sha256_file (FileMacro& f) {
    SHA256_CTX  ctx;
    ssize_t  len;
    u_char  buf[S4K];

    SHA256_Init (&ctx);
    while ((len = f.read (buf, S4K)) > 0) {
	SHA256_Update (&ctx, buf, len);
    }
    SHA256_Final (buf, &ctx);
    return ustring (char_type (buf), 32);
}

/*DOC:
===md5===
 (md5 TEXT [#hex | #base64 | :hex BOOL | :base64 BOOL] [#decode-message-hex | #decode-message-base64]) -> STRING
 (md5 [:message-serial FILENAME | :message-named FILENAME | :message-static FILENAME | :message-text TEXT] [#hex | #base64 | :hex BOOL | :base64 BOOL] [#decode-message-hex | #decode-message-base64]) -> STRING

*/
//#XAFUNC	md5	ml_md5
//#XWIKIFUNC	md5
MNode*  ml_md5 (bool fev, MNode* cell, MlEnv* mlenv) {
    return ml_hash_sub (fev, fn_md5, fn_md5_file, cell, mlenv);
}

/*DOC:
===sha1===
 (sha1 TEXT [#hex | #base64 | :hex BOOL | :base64 BOOL] [#decode-message-hex | #decode-message-base64]) -> STRING
 (sha1 [:message-serial FILENAME | :message-named FILENAME | :message-static FILENAME | :message-text TEXT] [#hex | #base64 | :hex BOOL | :base64 BOOL] [#decode-message-hex | #decode-message-base64]) -> STRING

*/
//#XAFUNC	sha1	ml_sha1
//#XWIKIFUNC	sha1
MNode*  ml_sha1 (bool fev, MNode* cell, MlEnv* mlenv) {
    return ml_hash_sub (fev, fn_sha1, fn_sha1_file, cell, mlenv);
}

/*DOC:
===sha256===
 (sha256 TEXT [#hex | #base64 | :hex BOOL | :base64 BOOL] [#decode-message-hex | #decode-message-base64]) -> STRING
 (sha256 [:message-serial FILENAME | :message-named FILENAME | :message-static FILENAME | :message-text TEXT] [#hex | #base64 | :hex BOOL | :base64 BOOL] [#decode-message-hex | #decode-message-base64]) -> STRING

*/
//#XAFUNC	sha256	ml_sha256
//#XWIKIFUNC	sha256
MNode*  ml_sha256 (bool fev, MNode* cell, MlEnv* mlenv) {
    return ml_hash_sub (fev, fn_sha256, fn_sha256_file, cell, mlenv);
}

static MNode*  ml_hash_sub (bool fev, ustring fn_text (const ustring& text), ustring fn_file (FileMacro& f), MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    kwParam  kwList[] = {{CharConst ("hex"), EV_LIST},		  // 0
			 {CharConst ("base64"), EV_LIST},	  // 1
			 {CharConst ("message-serial"), EV_LIST}, // 2
			 {CharConst ("message-named"), EV_LIST},  // 3
			 {CharConst ("message-static"), EV_LIST}, // 4
			 {CharConst ("message-text"), EV_LIST},	  // 5
			 {CharConst ("decode-message-hex"), EV_LIST}, // 6
			 {CharConst ("decode-message-base64"), EV_LIST}, // 7
			 {NULL, 0, EV_END}};
    MNodePtr  kwParams[8];
    evalParams (fev, mlenv, cell, posList, posParams, kwList, kwParams);
    StoreType  msgType (mlenv);
    if (! isNil (posParams[0]()))
	msgType.srcText (to_string (posParams[0]()));
    text_encode_t  encode = ENC_HEX;
    if (! isNil (kwParams[0]())) // 0:hex
	encode = ENC_HEX;
    if (! isNil (kwParams[1]())) // 1:base64
	encode = ENC_BASE64;
    if (! isNil (kwParams[2]()))
	msgType.srcSerial (to_string (kwParams[2]()));	// :message-serial
    if (! isNil (kwParams[3]()))
	msgType.srcNamed (to_string (kwParams[3]()));	// :message-named
    if (! isNil (kwParams[4]()))
	msgType.srcStatic (to_string (kwParams[4]()));	// :message-static
    if (! isNil (kwParams[5]()))
	msgType.srcText (to_string (kwParams[5]()));	// :message-text
    text_encode_t  decodeMsg = ENC_NONE;
    if (! isNil (kwParams[6]()))			// #decode-message-hex
	decodeMsg = ENC_HEX;
    if (! isNil (kwParams[7]()))			// #decode-message-base64
	decodeMsg = ENC_BASE64;
    ustring  text;
    ustring  ans;
    if (msgType.isText ()) {
	text = msgType.read ();
	switch (decodeMsg) {
	case ENC_HEX:
	    text = hexDecode (text);
	    break;
	case ENC_BASE64:
	    text = base64Decode (text.begin (), text.end ());
	    break;
	default:;
	}
	ans = fn_text (text);
    } else if (! msgType ()) {
	// error
    } else {
	FileMacro  f;
	if (f.openRead (msgType.src ().c_str ())) {
	    ans = fn_file (f);
	}
    }
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


static MNode*  ml_hash_string (bool fev, MNode* cell, MlEnv* mlenv, ustring fn (const ustring& text)) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    kwParam  kwList[] = {{CharConst ("hex"), EV_LIST},
			 {CharConst ("base64"), EV_LIST},
			 {NULL, 0, EV_END}};
    MNodePtr  kwParams[2];
    evalParams (fev, mlenv, cell, posList, posParams, kwList, kwParams);
    ustring  text = to_string (posParams[0]());
    enum {
	ENC_HEX,
	ENC_BASE64,
    }  encode = ENC_HEX;
    if (to_bool (kwParams[0]()))
	encode = ENC_HEX;
    if (to_bool (kwParams[1]()))
	encode = ENC_BASE64;
    ustring  ans = fn (text);
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

static MNode*  ml_hash_file (bool fev, MNode* cell, MlEnv* mlenv, ustring fn (FileMacro& f)) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    kwParam  kwList[] = {{CharConst ("serial"), EV_LIST},		// 0
			 {CharConst ("named"), EV_LIST},		// 1
			 {CharConst ("static"), EV_LIST},		// 2
			 {CharConst ("hex"), EV_LIST},		// 3
			 {CharConst ("base64"), EV_LIST},		// 4
			 {CharConst ("source-serial"), EV_LIST},	// 5
			 {CharConst ("source-named"), EV_LIST},	// 6
			 {CharConst ("source-static"), EV_LIST},	// 7
			 {NULL, 0, EV_END}};
    MNodePtr  kwParams[8];
    evalParams (fev, mlenv, cell, posList, posParams, kwList, kwParams);
    ustring  name = to_string (posParams[0]());
    StoreType  storetype (mlenv);
    if (to_bool (kwParams[0]()))
	storetype.setSerial ();
    if (to_bool (kwParams[1]()))
	storetype.setNamed ();
    if (to_bool (kwParams[2]()))
	storetype.setStatic ();
    enum {
	ENC_HEX,
	ENC_BASE64,
    }  encode = ENC_HEX;
    if (to_bool (kwParams[3]()))
	encode = ENC_HEX;
    if (to_bool (kwParams[4]()))
	encode = ENC_BASE64;
    if (! isNil (kwParams[5]()))
	storetype.srcSerial (to_string (kwParams[5]()));
    if (! isNil (kwParams[6]()))
	storetype.srcNamed (to_string (kwParams[6]()));
    if (! isNil (kwParams[7]()))
	storetype.srcStatic (to_string (kwParams[7]()));
    ustring  src = storetype.src (name);
    ustring  ans;
    if (src.length () > 0) {
	FileMacro  f;

	if (f.openRead (src.c_str ())) {
	    ans = fn (f);
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
    }
    return NULL;
}

/*--DOC:
===md5-string===
 (md5-string TEXT [#hex | #base64]) -> STRING

*/
//#XAFUNC	md5-string	ml_md5_string
//#XWIKIFUNC	md5-string
MNode*  ml_md5_string (bool fev, MNode* cell, MlEnv* mlenv) {
    return ml_hash_string (fev, cell, mlenv, fn_md5);
}

/*--DOC:
===md5-file===
 (md5-file FILENAME [#serial | #named | #static] [#hex | #base64]) -> STRING
 (md5-file [:source-serial FILENAME | :source-named FILENAME | :source-static FILENAME] [#hex | #base64]) -> STRING

*/
//#XAFUNC	md5-file	ml_md5_file
MNode*  ml_md5_file (bool fev, MNode* cell, MlEnv* mlenv) {
    return ml_hash_file (fev, cell, mlenv, fn_md5_file);
}

/*--DOC:
===sha1-string===
 (sha1-string TEXT [#hex | #base64]) -> STRING

*/
//#XAFUNC	sha1-string	ml_sha1_string
//#XWIKIFUNC	sha1-string
MNode*  ml_sha1_string (bool fev, MNode* cell, MlEnv* mlenv) {
    return ml_hash_string (fev, cell, mlenv, fn_sha1);
}

/*--DOC:
===sha1-file===
 (sha1-file FILENAME [#serial | #named | #static] [#hex | #base64]) -> STRING
 (sha1-file FILENAME [:source-serial FILENAME | :source-named FILENAME | :source-static FILENAME] [#hex | #base64]) -> STRING

*/
//#XAFUNC	sha1-file	ml_sha1_file
MNode*  ml_sha1_file (bool fev, MNode* cell, MlEnv* mlenv) {
    return ml_hash_file (fev, cell, mlenv, fn_sha1_file);
}

/*--DOC:
===sha256-string===
 (sha256-string TEXT [#hex | #base64]) -> STRING

*/
//#XAFUNC	sha256-string	ml_sha256_string
//#XWIKIFUNC	sha256-string
MNode*  ml_sha256_string (bool fev, MNode* cell, MlEnv* mlenv) {
    return ml_hash_string (fev, cell, mlenv, fn_sha256);
}

/*--DOC:
===sha256-file===
 (sha256-file FILENAME [#serial | #named | #static] [#hex | #base64]) -> STRING
 (sha256-file FILENAME [:source-serial FILENAME | :source-named FILENAME | :source-static FILENAME] [#hex | #base64]) -> STRING

*/
//#XAFUNC	sha256-file	ml_sha256_file
MNode*  ml_sha256_file (bool fev, MNode* cell, MlEnv* mlenv) {
    return ml_hash_file (fev, cell, mlenv, fn_sha256_file);
}

/*DOC:
===hmac-sha256-n-string===
 (hmac-sha256-n-string KEY TEXT1 ... [#hex | #base64]) -> STRING

*/
//#XAFUNC	hmac-sha256-n-string	ml_hmac_sha256_n_string
//#XWIKIFUNC	hmac-sha256-n-string
MNode*  ml_hmac_sha256_n_string (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    kwParam  kwList[] = {{CharConst ("hex"), EV_LIST},
			 {CharConst ("base64"), EV_LIST},
			 {NULL, 0, EV_END}};
    MNodePtr  kwParams[2];
    MNodePtr  rest;
    evalParams (fev, mlenv, cell, posList, posParams, kwList, kwParams, EV_LIST, &rest);
    ustring  key = to_string (posParams[0]());
    MNode*  args = rest ();
    enum {
	ENC_HEX,
	ENC_BASE64,
    }  encode = ENC_HEX;
    if (to_bool (kwParams[0]())) // 0:hex
	encode = ENC_HEX;
    if (to_bool (kwParams[1]())) // 1:base64
	encode = ENC_BASE64;
    ustring  text;
    unsigned char  md[EVP_MAX_MD_SIZE];
    unsigned int  md_len;
    while (args) {
	text = to_string (args->car ());
	HMAC (EVP_sha256 (), key.c_str (), key.length (), uchar_type (text.c_str ()), text.length (), md, &md_len);
	key.assign (char_type (md), md_len);
	nextNode (args);
    }
    switch (encode) {
    case ENC_HEX:
	key = hexEncode (key);
	break;
    case ENC_BASE64:
	key = base64Encode (key.begin (), key.end ());
	break;
    default:
	assert (0);
    }
    return newMNode_str (new ustring (key));
}

