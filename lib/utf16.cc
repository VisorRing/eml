#include "utf16.h"
#include "ustring.h"

ustring  utf8to16 (const ustring& src) {
    ustring  ans;
    int  i;
    u_char  c, d, e;
    u_int  x;

    ans.reserve (src.size ());
    for (i = 0; i < src.size ();) {
	c = src[i ++];
	if (c < 0x80) {
	    ans.append (1, 0);
	    ans.append (1, c);
	} else if (0xc2 <= c && c <= 0xdf) {
	    d = src[i ++];
	    if (0x80 <= d && d <= 0xbf) {
		x = ((c & 0x1f) << 6) + (d & 0x3f);
		ans.append (1, x >> 8);
		ans.append (1, x);
	    }
	} else if (c == 0xe0) {
	    d = src[i ++];
	    e = src[i ++];
	    if (0xa0 <= d && d <= 0xbf && 0x80 <= e && e <= 0xbf) {
		x = ((c & 0x1f) << 12) + ((d & 0x3f) << 6) + (e & 0x3f);
		ans.append (1, x >> 8);
		ans.append (1, x);
	    }
	} else if (0xe1 <= c && c <= 0xef) {
	    d = src[i ++];
	    e = src[i ++];
	    if (0x80 <= d && d <= 0xbf && 0x80 <= e && e <= 0xbf) {
		x = ((c & 0x1f) << 12) + ((d & 0x3f) << 6) + (e & 0x3f);
		ans.append (1, x >> 8);
		ans.append (1, x);
	    }
	} else if (0xf0 <= c && c <= 0xf4) {
	    // skip surrogate pair
	    i ++;
	    i ++;
	    i ++;
	} else {
	    // ignore illegal byte
	}
    }
    return ans;
}

std::wstring  utow (const ustring& src) {
    std::wstring  ans;
    int  i;
    u_char  c, d, e, f;
    u_int  x;

    ans.reserve (src.size ());
    for (i = 0; i < src.size ();) {
	c = src[i ++];
	if (c < 0x80) {
	    ans.append (1, c);
	} else if (0xc2 <= c && c <= 0xdf) {
	    d = src[i ++];
	    if (0x80 <= d && d <= 0xbf) {
		x = ((c & 0x1f) << 6) + (d & 0x3f);
		ans.append (1, x);
	    }
	} else if (c == 0xe0) {
	    d = src[i ++];
	    e = src[i ++];
	    if (0xa0 <= d && d <= 0xbf && 0x80 <= e && e <= 0xbf) {
		x = ((c & 0x1f) << 12) + ((d & 0x3f) << 6) + (e & 0x3f);
		ans.append (1, x);
	    }
	} else if (0xe1 <= c && c <= 0xef) {
	    d = src[i ++];
	    e = src[i ++];
	    if (0x80 <= d && d <= 0xbf && 0x80 <= e && e <= 0xbf) {
		x = ((c & 0x1f) << 12) + ((d & 0x3f) << 6) + (e & 0x3f);
		ans.append (1, x);
	    }
	} else if (0xf0 <= c && c <= 0xf4) {
	    d = src[i ++];
	    e = src[i ++];
	    f = src[i ++];
	    if (0x80 <= d && d <= 0xbf && 0x80 <= e && e <= 0xbf && 0x80 <= f && f <= 0xbf) {
		x = ((c & 0x07) << 18) + ((d & 0x3f) << 12) + ((e & 0x3f) << 6) + (f & 0x3f);
		ans.append (1, x);
	    }
	} else {
	    // ignore
	}
    }
    return ans;
}

ustring  wtou (const std::wstring& src) {
    ustring  ans;
    u_int  x;
    int  i;

    ans.reserve (src.size () * 3);
    for (i = 0; i < src.size (); i ++) {
	x = src[i];
	if (x < 0x0080) {
	    ans.append (1, x);
	} else if (x < 0x0800) {
	    ans.append (1, 0xc0 | ((x >> 6) & 0x3f));
	    ans.append (1, 0x80 | (x & 0x3f));
	} else if (x < 0x10000) {
	    ans.append (1, 0xe0 | ((x >> 12) & 0x0f));
	    ans.append (1, 0x80 | ((x >> 6) & 0x3f));
	    ans.append (1, 0x80 | (x & 0x3f));
	} else {
	    ans.append (1, 0xf0 | ((x >> 18) & 0x07));
	    ans.append (1, 0x80 | ((x >> 12) & 0x3f));
	    ans.append (1, 0x80 | ((x >> 6) & 0x3f));
	    ans.append (1, 0x80 | (x & 0x3f));
	}
    }
    return ans;
}
