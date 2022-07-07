#include "ml.h"
#include "mlenv.h"
#include "ustring.h"
#include "expr.h"
#include "ml-totp.h"
#include "util_time.h"
#include "util_random.h"
#include <time.h>
#include <arpa/inet.h>
#include <openssl/hmac.h>
#include <openssl/sha.h>

/*DOC:
== TOTP Authentication==

*/

static u_char  base32_char (u_char ch) {
    if (ch < '2') {
	return 255;
    } else if (ch <= '7') {
	return ch - '2' + 26;
    } else if (ch < 'A') {
	return 255;
    } else if (ch <= 'Z') {
	return ch - 'A';
    } else if (ch < 'a') {
	return 255;
    } else if (ch < 'z') {
	return ch - 'a';
    } else {
	return 255;
    }
}

static ustring  base32_decode(const ustring* token) {
    ustring  ans;
    u_int  n = 0;
    int  s = 0;
    u_int  ch;
    int  i;
    for (i = 0; i < token->size (); ++ i) {
	ch = base32_char ((*token)[i]);
	if (ch == 255)
	    return uEmpty;
	n = (n << 5) + ch;
	s += 5;
	if (s >= 8) {
	    s -= 8;
	    ans.append (1, (n >> s) & 0xff);
	}
    }
    return ans;
}

static u_char  char_base32 (u_char ch) {
    if (ch < 26) {
	return ch + 'A';
    } else {
	return ch - 26 + '2';
    }
}

static ustring  base32_encode (const ustring* bstr) {
    ustring  ans;
    int  i;
    u_int  n = 0;
    u_int  ch = 0;
    for (i = 0; n >= 5 || i < bstr->size ();) {
	if (n < 5) {
	    ch = (ch << 8) | (*bstr)[i ++];
//	    fprintf (stderr, "i:%d, n:%d\n", i, n);
	    n += 8;
//	    fprintf (stderr, "i:%d, n:%d\n", i, n);
	}
	ans.append (1, char_base32 ((ch >> (n - 5)) & 0x1f));
	n -= 5;
//	fprintf (stderr, "n:%d, %s\n", n, ans.c_str ());
    }
    return ans;
}

/*DOC:
===totp-auth===
 (totp-calc TOKEN) -> NUMBER

*/
//#XAFUNC	totp-calc	ml_totp_calc
MNode*  ml_totp_calc (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    evalParams (fev, mlenv, cell, posList, posParams);
    ustring  token = to_string (posParams[0]());
    token = base32_decode (&token);
    time_t  tm = now () / 30;
    uint32_t  ts[2];
    ts[0] = 0;
    ts[1] = htonl (tm);
    unsigned char  md[EVP_MAX_MD_SIZE];
    unsigned int  md_len = 0;
    HMAC (EVP_sha1 (), (void*)token.c_str (), token.length (), uchar_type (&ts), 8, md, &md_len);
    int  offset = md[19] & 0x0f;
    uint32_t  num = ((md[offset] & 0x7f) << 24) + (md[offset + 1] << 16) + (md[offset + 2] << 8) + md[offset + 3];
    char  buf[16];
    ustring*  ans = new ustring ();
    int  len = snprintf (buf, 16, "%06d", num);
    ans->append (buf + (len - 6), 6);
    return newMNode_str (ans);;
}

/*DOC:
===totp-create-token===
 (totp-create-token) -> STRING

*/
//#XAFUNC	totp-create-token	ml_totp_create_token
MNode*  ml_totp_create_token (bool fev, MNode* cell, MlEnv* mlenv) {
    evalParams (fev, mlenv, cell, NULL, NULL);
    ustring  bstr = smallRandomKey ();
    assert (bstr.size () == 10);
    ustring  ans = base32_encode (&bstr);
    return newMNode_str (new ustring (ans));
}
