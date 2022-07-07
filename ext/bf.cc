#include "bf.h"
#include <openssl/blowfish.h>
#include <string.h>
#include <assert.h>

bool  bf_cbc_decode (ustring& ans, const ustring& text, const ustring& key, const ustring& iv, crypto_padding_t padding) {
    int  r;
    BF_KEY  bfkey;
    unsigned char  ivb[8];

    BF_set_key (&bfkey, key.length (), uchar_type (key.data ()));
    if (iv.length () >= 8) {
	memcpy (ivb, iv.data (), 8);
    } else {
	memset (ivb, 0, 8);
	memcpy (ivb, iv.data (), iv.length ());
    }

    ans.resize (text.length ());
    BF_cbc_encrypt ((u_char*)text.data (), (u_char*)ans.data (), text.length (), &bfkey, ivb, BF_DECRYPT);
    switch (padding) {
    case PAD_ZERO:
	break;
    case PAD_PKCS5:
	if (text.length () == 0)
	    return false;
	r = *(ans.end () - 1);
	if (0 < r && r <= 8) {
	    ans.resize (ans.length () - r);
	} else {
	    return false;	// padding error
	}
	break;
    default:
	assert (0);
    }
    return true;
}
