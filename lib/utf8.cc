#include "utf8.h"
#include "util_const.h"
#include "util_splitter.h"
#include "ustring.h"
#include "cdbobj.h"
#include <iostream>

ustring  translateChar (const ustring& str, CDBMem* cdb) {
    ustring  ans;
    uiterator  b, e;
    u_char  c, c2, c3, c4;
    char  cb[8];

    ans.reserve (str.length () + 32);
    b = str.begin ();
    e = str.end ();
    for (; b < e;) {
	c = *b;
	if (c == 0) {
	    b ++;
	} else if (c <= 0x7f) {
	    if (cdb->find (&*b, 1) > 0) {
		cdb->read (cb);
		ans.append (cb, cdb->datalen ());
	    } else {
		ans.append (&*b, 1);
	    }
	    b ++;
	} else if (0xc2 <= c && c <= 0xdf) { // 2byte
	    if (b + 1 < e) {
		if (cdb->find (&*b, 2) > 0) {
		    cdb->read (cb);
		    ans.append (cb, cdb->datalen ());
		} else {
		    ans.append (&*b, 2);
		}
		b += 2;
	    } else {
		ans.append (uUScore);
		b ++;
	    }
	} else if (0xe0 <= c && c <= 0xef) { // 3byte
	    if (b + 2 < e) {
		if (cdb->find (&*b, 3) > 0) {
		    cdb->read (cb);
		    ans.append (cb, cdb->datalen ());
		} else {
		    ans.append (&*b, 3);
		}
		b += 3;
	    }else {
		ans.append (uUScore);
		b ++;
	    }
	} else if (0xf0 <= c && c <= 0xf4) { // 4byte
	    if (b + 3 < e) {
		if (cdb->find (&*b, 4) > 0) {
		    cdb->read (cb);
		    ans.append (cb, cdb->datalen ());
		} else {
		    ans.append (&*b, 4);
		}
		b += 4;
	    }else {
		ans.append (uUScore);
		b ++;
	    }
	} else {
	    ans.append (uUScore);
	    b ++;
	}
    }
    return ans;
}

/*
  Clip char which is not a valid UTF-8 text.
*/
ustring  fixUTF8 (const ustring& str) {
    ustring  ans;
    size_t  i, n;
    u_char  c, d, e, f;

    n = str.length ();
    ans.reserve (n);
    for (i = 0; i < n;) {
	c = str[i ++];
	if (c == 0) {
	} else if (c <= 0x7f) {
	    if (c == '\r') {
		ans.append (1, '\n');
		if (i < n && str[i] == '\n') {
		    i ++;
		}
	    } else {
		ans.append (1, c);
	    }
	} else if (0xc2 <= c && c <= 0xdf && i < n) {
	    d = str[i];
	    if (0x80 <= d && d <= 0xbf) {
		i ++;
		ans.append (1, c);
		ans.append (1, d);
	    }
	} else if (c == 0xe0 && i + 1 < n) {
	    d = str[i];
	    e = str[i + 1];
	    if (0xa0 <= d && d <= 0xbf && 0x80 <= e && e <= 0xbf) {
		i += 2;
		ans.append (1, c);
		ans.append (1, d);
		ans.append (1, e);
	    }
	} else if (0xe1 <= c && c <= 0xef && i + 1 < n) {
	    d = str[i];
	    e = str[i + 1];
	    if (0x80 <= d && d <= 0xbf && 0x80 <= e && e <= 0xbf) {
		i += 2;
		ans.append (1, c);
		ans.append (1, d);
		ans.append (1, e);
	    }
	} else if (c == 0xf0 && i + 2 < n) {
	    d = str[i];
	    e = str[i + 1];
	    f = str[i + 2];
	    if (0x90 <= d && d <= 0xbf && 0x80 <= e && e <= 0xbf && 0x80 <= f && f <= 0xbf) {
		i += 3;
		ans.append (1, c);
		ans.append (1, d);
		ans.append (1, e);
		ans.append (1, f);
	    }
	} else if (0xf1 <= c && c <= 0xf3 && i + 2 < n) {
	    d = str[i];
	    e = str[i + 1];
	    f = str[i + 2];
	    if (0x80 <= d && d <= 0xbf && 0x80 <= e && e <= 0xbf && 0x80 <= f && f <= 0xbf) {
		i += 3;
		ans.append (1, c);
		ans.append (1, d);
		ans.append (1, e);
		ans.append (1, f);
	    }
	} else if (c == 0xf4 && i + 2 < n) {
	    d = str[i];
	    e = str[i + 1];
	    f = str[i + 2];
	    if (0x80 <= d && d <= 0x8f && 0x80 <= e && e <= 0xbf && 0x80 <= f && f <= 0xbf) {
		i += 3;
		ans.append (1, c);
		ans.append (1, d);
		ans.append (1, e);
		ans.append (1, f);
	    }
	} else {
	    ans.append (uUScore);
	}
    }
    return ans;
}

void  nextChar (uiterator& it, const uiterator& end) {
    ustring::value_type  c;

    c = *(it ++);
    if ((c & 0x80) == 0) {
    } else if ((c & 0xe0) == 0xc0) {
	if (it != end) it ++;
    } else if ((c & 0xf0) == 0xe0) {
	if (it != end) it ++;
	if (it != end) it ++;
    } else if ((c & 0xf8) == 0xf0) {
	if (it != end) it ++;
	if (it != end) it ++;
	if (it != end) it ++;
    } else if ((c & 0xfc) == 0xf8) {
	if (it != end) it ++;
	if (it != end) it ++;
	if (it != end) it ++;
	if (it != end) it ++;
    } else if ((c & 0xfe) == 0xfc) {
	if (it != end) it ++;
	if (it != end) it ++;
	if (it != end) it ++;
	if (it != end) it ++;
	if (it != end) it ++;
    } else {
	// error
    }
}

void  nextChar (uiterator& it, const uiterator& end, ustring& target) {
    ustring::value_type  c;

    c = *(it ++);
    if ((c & 0x80) == 0) {
	target += c;
    } else if ((c & 0xe0) == 0xc0) {
	if (it < end) {
	    target += c;
	    target += (*it ++);
	} else {
	    it = end;
	}
    } else if ((c & 0xf0) == 0xe0) {
	if (it + 1 < end) {
	    target += c;
	    target += (*it ++);
	    target += (*it ++);
	} else {
	    it = end;
	}
    } else if ((c & 0xf8) == 0xf0) {
	if (it + 2 < end) {
	    target += c;
	    target += (*it ++);
	    target += (*it ++);
	    target += (*it ++);
	} else {
	    it = end;
	}
    } else if ((c & 0xfc) == 0xf8) {
	if (it + 3 < end) {
	    target += c;
	    target += (*it ++);
	    target += (*it ++);
	    target += (*it ++);
	    target += (*it ++);
	} else {
	    it = end;
	}
    } else if ((c & 0xfe) == 0xfc) {
	if (it + 4 < end) {
	    target += c;
	    target += (*it ++);
	    target += (*it ++);
	    target += (*it ++);
	    target += (*it ++);
	    target += (*it ++);
	} else {
	    it = end;
	}
    } else {
	// error
    }
}

void  lastChar (const ustring& text, uiterator& ans) {
    uiterator  b = text.begin ();
    uiterator  e = text.end ();

    assert (b != e);
    e --;
    while ((*e & 0xc0) == 0x80) {
	e --;
    }
    ans = e;
}

ustring  ellipsis (const ustring& text, int limit) {
    uiterator  b, e;
    ustring  u;

    u.reserve (256);
    for (b = text.begin (), e = text.end (); limit > 0 && b != e; nextChar (b, e, u)) {
	limit --;
    }
    if (b != e) {
	u.append (CharConst ("..."));
    }
    return u;
}

static bool  findCtrlChar (uiterator& b, uiterator e, uiterator& u) {
    int  c;
    for (; b < e; ++ b) {
	c = *b;
	if ((0 <= c && c < 0x20) || c == 0x7f) {	// [\x00-\x1f\x7f]
	    u = b + 1;
	    return true;
	}
    }
    u = e;
    return false;
}

ustring  logText (const ustring& text) {
    SplitterFn  sp (text, findCtrlChar);
    if (sp.nextSep ()) {
	ustring  ans;
	do {
	    if (sp.preSize () > 0)
		ans.append (sp.pre ());
	    if (*sp.end () == '\n') {
		ans.append (CharConst ("//"));
	    } else {
		ans.append (uUScore);
	    }
	} while (sp.nextSep ());
	if (sp.preSize () > 0)
	    ans.append (sp.pre ());
	return ans;
    } else {
	return text;
    }
}

void  clipEnd (ustring& val, uregex& re1, uregex& re2) {
    uiterator  b, i1, i2, e;
    umatch  m;

    b = val.begin ();
    e = val.end ();
    if (usearch (b, e, m, re1)) {
	i1 = m[0].second;
    } else {
	i1 = b;
    }
    if (usearch (i1, e, m, re2)) {
	i2 = m[0].first;
    } else {
	i2 = e;
    }
    if (i1 != b || i2 != e) {
	val = ustring (i1, i2);
    }
}

void  clipWhiteEnd (ustring& val) {
    uregex  re1 ("^(" UTF8_SPACE "|" UTF8_ZWSPACE "|" UTF8_IDEOSPACE ")+");
    uregex  re2 ("(" UTF8_SPACE "|" UTF8_ZWSPACE "|" UTF8_IDEOSPACE ")+$");

    clipEnd (val, re1, re2);
}

void  clipNLEnd (ustring& val) {
    uregex  re1 ("^\\n+");
    uregex  re2 ("\\n+$");

    clipEnd (val, re1, re2);
}
