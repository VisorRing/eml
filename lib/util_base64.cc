#include "util_base64.h"
#include "ustring.h"
#include <string.h>

static char  Base64Char[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

ustring  base64Encode (uiterator b, uiterator e) {
    ustring  ans;
    size_t  size;
    int  c0, c1, c2;

    while (b != e) {
	size = e - b;
	if (size >= 3) {
	    c0 = *b ++;
	    c1 = *b ++;
	    c2 = *b ++;
	    ans.append (1, Base64Char[(c0 >> 2) & 0x3f]);
	    ans.append (1, Base64Char[((c0 & 0x03) << 4) | ((c1 >> 4) & 0x0f)]);
	    ans.append (1, Base64Char[((c1 & 0x0f) << 2) | ((c2 >> 6) & 0x03)]);
	    ans.append (1, Base64Char[c2 & 0x3f]);
	} else if (size == 2) {
	    c0 = *b ++;
	    c1 = *b ++;
	    ans.append (1, Base64Char[(c0 >> 2) & 0x3f]);
	    ans.append (1, Base64Char[((c0 & 0x03) << 4) | ((c1 >> 4) & 0x0f)]);
	    ans.append (1, Base64Char[((c1 & 0x0f) << 2)]);
	    ans.append (1, '=');
	} else if (size == 1) {
	    c0 = *b ++;
	    ans.append (1, Base64Char[(c0 >> 2) & 0x3f]);
	    ans.append (1, Base64Char[((c0 & 0x03) << 4)]);
	    ans.append (1, '=');
	    ans.append (1, '=');
	} else {
	    break;
	}
    }
    return ans;
}

ustring  base64Decode (uiterator b, uiterator e) {
    ustring  ans;
    size_t  size;
    u_int  c0, c1, c2, c3;
    char*  p;
    u_int  c, x;

    size = e - b;
//    while (size >= 4) {
    while (size > 0) {
	c0 = *b ++;
	size --;
	if (isspace (c0)) {
	} else {
	    if (size > 0) {
		c1 = *b ++;
		size --;
	    } else {
		c1 = '=';
	    }
	    if (size > 0) {
		c2 = *b ++;
		size --;
	    } else {
		c2 = '=';
	    }
	    if (size > 0) {
		c3 = *b ++;
		size --;
	    } else {
		c3 = '=';
	    }
	    x = 0;

	    p = (char*)memchr (Base64Char, c0, sizeof (Base64Char) - 1);
	    if (p == NULL) 
		break;
	    x = (p - Base64Char) << 2;

	    p = (char*)memchr (Base64Char, c1, sizeof (Base64Char) - 1);
	    if (p == NULL)
		break;
	    c = p - Base64Char;
	    ans.append (1, (x | (c >> 4)));
	    x = c << 4;

	    p = (char*)memchr (Base64Char, c2, sizeof (Base64Char) - 1);
	    if (p == NULL)
		break;
	    c = p - Base64Char;
	    ans.append (1, (x | (c >> 2)));
	    x = c << 6;

	    p = (char*)memchr (Base64Char, c3, sizeof (Base64Char) - 1);
	    if (p == NULL)
		break;
	    c = p - Base64Char;
	    ans.append (1, (x | c));
	}
    }
    return ans;
}

static char  Base64URLChar[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

ustring  base64URLEncode (uiterator b, uiterator e) {
    ustring  ans;
    size_t  size;
    int  c0, c1, c2;

    while (b != e) {
	size = e - b;
	if (size >= 3) {
	    c0 = *b ++;
	    c1 = *b ++;
	    c2 = *b ++;
	    ans.append (1, Base64URLChar[(c0 >> 2) & 0x3f]);
	    ans.append (1, Base64URLChar[((c0 & 0x03) << 4) | ((c1 >> 4) & 0x0f)]);
	    ans.append (1, Base64URLChar[((c1 & 0x0f) << 2) | ((c2 >> 6) & 0x03)]);
	    ans.append (1, Base64URLChar[c2 & 0x3f]);
	} else if (size == 2) {
	    c0 = *b ++;
	    c1 = *b ++;
	    ans.append (1, Base64URLChar[(c0 >> 2) & 0x3f]);
	    ans.append (1, Base64URLChar[((c0 & 0x03) << 4) | ((c1 >> 4) & 0x0f)]);
	    ans.append (1, Base64URLChar[((c1 & 0x0f) << 2)]);
//	    ans.append (1, '=');
	} else if (size == 1) {
	    c0 = *b ++;
	    ans.append (1, Base64URLChar[(c0 >> 2) & 0x3f]);
	    ans.append (1, Base64URLChar[((c0 & 0x03) << 4)]);
//	    ans.append (1, '=');
//	    ans.append (1, '=');
	} else {
	    break;
	}
    }
    return ans;
}

ustring  base64URLDecode (uiterator b, uiterator e) {
    ustring  ans;
    size_t  size;
    u_int  c0, c1, c2, c3;
    char*  p;
    u_int  c, x;

    size = e - b;
//    while (size >= 4) {
    while (size >= 1) {
	c0 = *b ++;
	size --;
//	if (isspace (c0)) {
//	} else {
	if (size > 0) {
	    c1 = *b ++;
	    size --;
	} else {
	    c1 = 0;
	}
	if (size > 0) {
	    c2 = *b ++;
	    size --;
	} else {
	    c2 = 0;
	}
	if (size > 0) {
	    c3 = *b ++;
	    size --;
	} else {
	    c3 = 0;
	}
	x = 0;

	    p = (char*)memchr (Base64URLChar, c0, sizeof (Base64URLChar) - 1);
	    if (p == NULL) 
		break;
	    x = (p - Base64URLChar) << 2;

	    p = (char*)memchr (Base64URLChar, c1, sizeof (Base64URLChar) - 1);
	    if (p == NULL)
		break;
	    c = p - Base64URLChar;
	    ans.append (1, (x | (c >> 4)));
	    x = c << 4;

	    p = (char*)memchr (Base64URLChar, c2, sizeof (Base64URLChar) - 1);
	    if (p == NULL)
		break;
	    c = p - Base64URLChar;
	    ans.append (1, (x | (c >> 2)));
	    x = c << 6;

	    p = (char*)memchr (Base64URLChar, c3, sizeof (Base64URLChar) - 1);
	    if (p == NULL)
		break;
	    c = p - Base64URLChar;
	    ans.append (1, (x | c));
//	}
    }
    return ans;
}
