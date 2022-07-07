#include "config.h"
#include "util_string.h"
#include "util_const.h"
#include "util_random.h"
#include "util_splitter.h"
#include "ustring.h"
#include "utf8.h"
#include "utf16.h"
#include <boost/regex.hpp>
#include <boost/regex/pattern_except.hpp>
#include <boost/algorithm/string.hpp>
#include <vector>
#include <algorithm>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <float.h>
#include <ctype.h>

UIConv::UIConv (const char* in, const char* out) {
    cd = iconv_open (in, out);
    if (cd == ICONV_ERR) {
	throw (ustring (in).append (CharConst (", ")).append (ustring (out)).append (CharConst (": unknown encoding.")));
    }
}

ustring  UIConv::cv (const ustring& text, bool flush) {
    ustring  ans;

    if (cd != ICONV_ERR) {
	char*  buf = new char[4096];
	const char*  ibuf;
	char*  obuf;
	size_t  isize, osize, rsize;

	ibuf = text.begin ().base ();
	isize = text.size ();
	while (isize > 0) {
	    obuf = buf;
	    osize = 4096;
	    rsize = ::iconv (cd, (char**)&ibuf, &isize, &obuf, &osize);
	    if (rsize == -1) {
		if (errno == EILSEQ) {
		    ibuf ++;	
		    isize --;
		    ans.append (CharConst ("_"));
		} else if (errno == EINVAL) {
		} else if (errno == E2BIG) {
		} else {
		    break;
		}
	    }
	    if (obuf > buf)
		ans.append (buf, obuf - buf);
	}
	if (flush) {
	    obuf = buf;
	    osize = 4096;
	    rsize = ::iconv (cd, NULL, NULL, &obuf, &osize);
	    if (obuf > buf)
		ans.append (buf, obuf - buf);
	}
	delete[] buf;
    }
    return ans;
}

///////////////////////////////////////////////////////////////////////
static bool  isDigit (int c) {
    return '0' <= c && c <= '9';
}

ustring  c3 (const ustring& str) {
    bool  qsign = false;
    uiterator  b, e, t;

    b = str.begin ();
    e = str.end ();
    if (str[0] == '-' || str[0] == '+') {
	qsign = true;
	b = b + 1;
    }
    t = b;
    if (matchHeadFn (t, e, isDigit)) {
	int  n = t - b;
	int  l = str.size () + n / 3;
	ustring  ans;
	ans.reserve (l);
	if (qsign) {
	    ans.append (1, str[0]);
	}
	for (; b < t; ++ b) {
	    ans.append (1, *b);
	    if (n > 1 && n % 3 == 1) {
		ans.append (CharConst (","));
	    }
	    n --;
	}
	for (; b != e; b ++) {
	    ans.append (1, *b);
	}
	return ans;
    } else {
	return str;
    }
}

ustring  to_ustring (int32_t v) {
    return boost::lexical_cast<ustring> (v);
}

ustring  to_ustring (uint32_t v) {
    return boost::lexical_cast<ustring> (v);
}

ustring  to_ustring (int64_t v) {
    return boost::lexical_cast<ustring> (v);
}

ustring  to_ustring (uint64_t v) {
    return boost::lexical_cast<ustring> (v);
}

ustring  to_ustring (long long int v) {
    return boost::lexical_cast<ustring> (v);
}

ustring  to_ustring (unsigned long long int v) {
    return boost::lexical_cast<ustring> (v);
}

ustring  to_ustring (double v) {
    char  b[32];
//    return ustring (b, snprintf (b, 32, "%g", v));
//    return ustring (b, snprintf (b, 32, "%.*g", DBL_DIG, v));
    int  s = snprintf(b, 32, "%.*g", DBL_DIG, v);
    int  i;
    for (i = 0; i < s; ++ i) {
	switch (b[i]) {
	case '.':
	case 'e':
	case 'E':
	    return ustring (b, s);
	default:;
	}
    }
    b[s++] = '.';
    b[s] = 0;
    return ustring (b, s);
}

int32_t  to_int32 (const ustring& v) {
    return boost::lexical_cast<int32_t> (v);
}

int64_t  to_int64 (const ustring& v) {
    return boost::lexical_cast<int64_t> (v);
}

uint64_t  to_uint64 (const ustring& v) {
    return boost::lexical_cast<uint64_t> (v);
}

double  to_double (const ustring& v) {
    try {
	return boost::lexical_cast<double> (v);
    } catch (boost::bad_lexical_cast msg) {
	return 0.0;
    }
}

static int  shex (char c) {
    if ('0' <= c && c <= '9') {
	return (c - '0');
    } else if ('a' <= c && c <= 'f') {
	return (c -  'a' + 10);
    } else if ('A' <= c && c <= 'F') {
	return (c - 'A' + 10);
    } else {
	return -1;
    }
}

static int  hex (char c) {
    if ('0' <= c && c <= '9') {
	return (c - '0');
    } else if ('a' <= c && c <= 'f') {
	return (c -  'a' + 10);
    } else if ('A' <= c && c <= 'F') {
	return (c - 'A' + 10);
    } else {
	return 0;
    }
}

static int  hex (char c1, char c2) {
    return (hex (c1) * 16 + hex (c2));
}

static char  hexchar (int c) {
    if (0 <= c && c <= 9)
	return '0' + c;
    else if (10 <= c && c <= 15)
	return 'a' - 10 + c;
    else
	return '0';
}

static char  hexchar_c (int c) {
    if (0 <= c && c <= 9)
	return '0' + c;
    else if (10 <= c && c <= 15)
	return 'A' - 10 + c;
    else
	return '0';
}

static ustring  percentHex (int c) {
    ustring  ans (3, '%');

    ans[1] = hexchar ((c >> 4) & 0x0f);
    ans[2] = hexchar (c & 0x0f);
    return ans;
}

ustring  percentHEX (int c) {
    ustring  ans (3, '%');

    ans[1] = hexchar_c ((c >> 4) & 0x0f);
    ans[2] = hexchar_c (c & 0x0f);
    return ans;
}

ustring  urldecode_nonul (const ustring& str) {
    ustring  ans;
    static uregex  re ("(\\+)|%([0-9a-fA-F][0-9a-fA-F])|\\x00");
    umatch  m;
    uiterator  b, e;

    ans.reserve (str.size ());
    b = str.begin ();
    e = str.end ();
    while (usearch (b, e, m, re)) {
	if (b != m[0].first) {
	    ans.append (b, m[0].first);
	}
	if (m[1].matched) {
	    ans.append (1, ' ');
	} else if (m[2].matched) {
	    int  v = hex (*(m[2].first), *(m[2].first + 1));
	    if (v != 0) 
		ans.append (1, v);
	} else {
	}
	b = m[0].second;
    }
    if (b != e) {
	ans.append (b, e);
    }

    return ans;
}

static ustring  omitPattern (const ustring& text, int (*fn)(int)) {
    uiterator  b = text.begin ();
    uiterator  e = text.end ();
    uiterator  p = b;
    for (; p < e; ++ p) {
	if (fn (*p))
	    break;
    }
    if (p == e) {
	return text;
    } else {
	ustring  ans;
	ans.reserve (text.length ());
	ans.assign (b, p);
	++ p;
	for (; p < e; ++ p) {
	    if (! fn (*p))
		ans.append (1, *p);
	}
	return ans;
    }
}

ustring  omitCtrl (const ustring& str) {
    return omitPattern (str, iscntrl);
}

static int  iscntrlx (int c) {
    static char  table_ctrlx[] = {
	1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 
    };
    if (0 <= c && c < 128)
	return table_ctrlx[c];
    return 0;
}

ustring  omitCtrlX (const ustring& str) {
    return omitPattern (str, iscntrlx);
}

static int  isNUL (int c) {
    return c == 0;
}

ustring  omitNul (const ustring& str) {
    return omitPattern (str, isNUL);
}

static int  iscrlfchar (int c) {
    return c == 0x0a || c == 0x0d;
}

ustring  omitNL (const ustring& str) {
    return omitPattern (str, iscrlfchar);
}

static int  isnonasciichar (int c) {
    return c < 0x20 || c > 0x7e;
}

ustring  omitNonAscii (const ustring& str) {
    return omitPattern (str, isnonasciichar);
}

static int  isnonasciiword (int c) {
    return c < 0x21 || c > 0x7e;
}

ustring  omitNonAsciiWord (const ustring& str) {
    return omitPattern (str, isnonasciiword);
}

static ustring  percentEncode (Splitter& sp) {
    ustring  ans;
    int  c;
    while (sp.nextSep ()) {
	if (sp.preSize () > 0)
	    ans.append (sp.pre ());
	c = *sp.matchBegin ();
	if (c == '\0') {
	    ans.append (uUScore);
	} else {
	    ans.append (percentHEX (c));
	}
    }
    if (sp.preSize () > 0)
	ans.append (sp.pre ());
    return ans;
}

static bool  findPercentChar (uiterator& b, uiterator e, uiterator& u) {
    static char  table_percentchar[] = {		// (\x00)|([^A-Za-z0-9_.~\-])
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 
	1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 
	1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 
    };
    int  c;
    for (; b < e; ++ b) {
	c = *b;
	if (c < 0 || c >= 128 || table_percentchar[c]) {
	    u = b + 1;
	    return true;
	}
    }
    u = e;
    return false;
}

ustring  percentEncode (uiterator b, uiterator e) {
//    static uregex  re ("(\\x00)|([^A-Za-z0-9_.~-])");
    SplitterFn  sp (b, e, findPercentChar);
    return percentEncode (sp);
}

static bool  findPercentPathChar (uiterator& b, uiterator e, uiterator& u) {
    static char  table_percentpathchar[] = {		// (\x00)|([^A-Za-z0-9_\/.~\-])
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 
	1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 
	1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 
    };
    int  c;
    for (; b < e; ++ b) {
	c = *b;
	if (c < 0 || c >= 128 || table_percentpathchar[c]) {
	    u = b + 1;
	    return true;
	}
    }
    u = e;
    return false;
}

ustring  percentEncode_path (uiterator b, uiterator e) {
//    static uregex  re ("(\\x00)|([^A-Za-z0-9_/.~-])");
    SplitterFn  sp (b, e, findPercentPathChar);
    return percentEncode (sp);
}

ustring  percentDecode (const ustring& str) {
    ustring  ans;
    static uregex  re ("%([0-9a-fA-F][0-9a-fA-F])|\\x00");
    umatch  m;
    uiterator  b, e;

    b = str.begin ();
    e = str.end ();
    while (usearch (b, e, m, re)) {
	if (b != m[0].first) {
	    ans.append (b, m[0].first);
	}
	if (m[1].matched) {
	    int  v = hex (*(m[1].first), *(m[1].first + 1));
	    if (v != 0) 
		ans.append (1, v);
	} else {
	}
	b = m[0].second;
    }
    if (b != e) {
	ans.append (b, e);
    }

    return fixUTF8 (ans);
}

static bool  findCookieEncChar (uiterator& b, uiterator e, uiterator& u) {
    static char  table_cookieencode[] = {		// ([\\x00-\\x1f\\x7f])|([ ,;%\\x80-\\xff])
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
	1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 
    };
    int  c;
    for (; b < e; ++ b) {
	c = *b;
	if (c < 0 || c >= 128 || table_cookieencode[c]) {
	    u = b + 1;
	    return true;
	}
    }
    u = e;
    return false;
}

ustring  cookieencode (const ustring& text) {
//    static uregex  re ("([\\x00-\\x1f\\x7f])|([ ,;%\\x80-\\xff])");
    SplitterFn  sp (text.begin (), text.end (), findCookieEncChar);
    return percentEncode (sp);
}

ustring  cookiedecode (const ustring& text) {
    umatch  m;
    uiterator  b, e;
    ustring  ans;
    int  a;
    static uregex  re ("%([0-9a-fA-F])([0-9a-fA-F])");

    b = text.begin ();
    e = text.end ();
    while (usearch (b, e, m, re)) {
	if (b != m[0].first)
	    ans.append (ustring (b, m[0].first));
	a = hex (*m[1].first, *m[2].first);
	ans.append (1, a);
	b = m[0].second;
    }
    if (b != e)
	ans.append (ustring (b, e));

    return ans;
}

ustring  clipColon (const ustring& text) {
    int  i;
    ustring  ans (text);

    for (i = 0; i < ans.size (); i ++) {
	if (ans[i] == ':')
	    ans[i] = '_';
    }
    return ans;
}

ustring  dirPart (const ustring& path) {
    ustring::size_type  s = path.rfind ('/', path.size ());

    if (s == ustring::npos) {
//	return uSlash;
	return uDot;
    } else {
	return ustring (path.begin (), path.begin () + s);
    }
}

ustring  filePart_osSafe (const ustring& path) {
    umatch  m;
    static uregex  re ("[^\\\\/]+$");

    if (usearch (path, m, re)) {
	return ustring (m[0].first, m[0].second);
    } else {
	return uEmpty;
    }
}

void  split (uiterator b, uiterator e, uregex& re, std::vector<ustring>& ans) {
    SplitterRe  sp (b, e, re);

    while (sp.next ()) {
	ans.push_back (sp.pre ());
    }
}

void  split (uiterator b, uiterator e, int ch, std::vector<ustring>& ans) {
    SplitterCh  sp (b, e, ch);

    while (sp.next ()) {
	ans.push_back (sp.pre ());
    }
}

void  splitE (uiterator b, uiterator e, uregex& re, std::vector<ustring>& ans) {
    SplitterRe  sp (b, e, re);

    if (b < e) {
	while (sp.nextSep ()) {
	    ans.push_back (sp.pre ());
	}
	ans.push_back (sp.pre ());
    }
}

void  splitE (uiterator b, uiterator e, int ch, std::vector<ustring>& ans) {
    SplitterCh  sp (b, e, ch);

    if (b < e) {
	while (sp.nextSep ()) {
	    ans.push_back (sp.pre ());
	}
	ans.push_back (sp.pre ());
    }
}

bool  splitChar (uiterator b, uiterator e, uiterator::value_type ch, uiterator& m1) {
    for (; b < e; b ++) {
	if (*b == ch) {
	    m1 = b;
	    return true;
	}
    }
    m1 = e;
    return false;
}

ustring  escape_re (const ustring& text) {
    ustring::const_iterator  b, e;
    umatch  m;
    ustring  ans;
    static uregex  re ("\\.|\\[|\\]|\\{|\\}|\\(|\\)|\\\\|\\*|\\+|\\?|\\||\\^|\\$");	// .[]{}()\*+?|^$
    b = text.begin ();
    e = text.end ();
    while (b != e && usearch (b, e, m, re)) {
	if (b != m[0].first)
	    ans.append (b, m[0].first);
	ans.append (uBSlash);
	ans.append (m[0].first, m[0].second);
	b = m[0].second;
    }
    if (b != e)
	ans.append (b, e);
    return ans;
}

ustring  slashEncode (const ustring& text) {
    ustring::const_iterator  b, e;
    umatch  m;
    ustring  ans;
    int  c;
    char  buf[4];
    static uregex  re ("([\\x00-\\x1f\\x7f])|(\\\\)|(\")");

    buf[0] = '\\';
    buf[1] = 'x';
    b = text.begin ();
    e = text.end ();
    while (b != e && usearch (b, e, m, re)) {
	if (b != m[0].first)
	    ans.append (b, m[0].first);
	if (m[1].matched) {
	    c = *m[0].first;
	    switch (c) {
	    case '\t':
		ans.append (CharConst ("\\t"));
		break;
	    case '\r':
		ans.append (CharConst ("\\r"));
		break;
	    case '\n':
		ans.append (CharConst ("\\n"));
		break;
	    default:
		buf[2] = hexchar ((c >> 4) & 0x0f);
		buf[3] = hexchar (c & 0x0f);
		ans.append (buf, 4);
	    }
	} else if (m[2].matched) {
	    ans.append (CharConst ("\\\\"));
	} else if (m[3].matched) {
	    ans.append (CharConst ("\\\""));
	} else {
	    assert (0);
	}
	b = m[0].second;
    }
    if (b != e)
	ans.append (b, e);
    return ans;
}

ustring  slashDecode (const ustring& text) {
    ustring::const_iterator  b, e;
    umatch  m;
    ustring  ans;
    int  c;
    static uregex  re ("\\\\([0-7][0-7][0-7]|[\\x00-\\x7f])");

    b = text.begin ();
    e = text.end ();
    while (b != e && usearch (b, e, m, re)) {
	if (b != m[0].first)
	    ans.append (b, m[0].first);
	b = m[0].first + 1;
	c = *b;
	switch (c) {
	case 't':
	    ans.append (CharConst ("\t"));
	    break;
	case 'r':
	    ans.append (CharConst ("\r"));
	    break;
	case 'n':
	    ans.append (CharConst ("\n"));
	    break;
	default:
	    if (m[0].second - m[0].first == 4) {
		c = (c - '0') * 64;
		b ++;
		c += (*b - '0') * 8;
		b ++;
		c += *b - '0';
		if (0 < c && c < 0x20)
		    ans.append (1, c);
	    } else {
		ans.append (1, c);
	    }
	}
	b = m[0].second;
    }
    if (b != e)
	ans.append (b, e);
    return ans;
}

unsigned long  strtoul (const ustring& str) {
    return strtoul (str.c_str (), NULL, 10);
}

unsigned long  strtoul (const uiterator& b) {
    return strtoul (&*b, NULL, 10);
}

long  strtol (const ustring& str) {
    return strtol (str.c_str (), NULL, 10);
}

double  strtod (const ustring& str) {
    return strtod (str.c_str (), NULL);
}

bool  passMatch (const ustring& pass, const ustring& cpass) {
    if (pass.length () == 0 || cpass.length () == 0)
	return false;
    return (strcmp (crypt (pass.c_str (), cpass.c_str ()), cpass.c_str ()) == 0);
}

ustring  passCrypt (const ustring& pass, passCryptFormat format) {
    // XXX not thread safe.
    ustring  salt;
    switch (format) {
    case FORMAT_MD5:
	salt = makeSalt ('1', 8);
	break;
    case FORMAT_BF:
	salt = bcryptGenSalt (12);
	break;
    case FORMAT_SHA256:
	salt = makeSalt ('5', 16);
	break;
    case FORMAT_SHA512:
	salt = makeSalt ('6', 16);
	break;
    default:
	assert (0);
    }
    return ustring (crypt (pass.c_str (), salt.c_str ()));
}

size_t  strLength (const ustring& src) {
    uiterator  b, e;
    size_t  n = 0;
    b = src.begin ();
    e = src.end ();
    while (b < e) {
	n ++;
	nextChar (b, e);
    }
    return n;
}

void  substring (const ustring& src, size_t idx, size_t len, int flen, ustring& ans) {
    uiterator  b, e, t;
    size_t  i;

    b = src.begin ();
    e = src.end ();
    for (i = 0; i < idx && b < e; i ++)
	nextChar (b, e);
    if (flen) {
	t = b;
	for (i = 0; i < len && t < e; i ++)
	    nextChar (t, e);
	ans.assign (b, t);
    } else {
	ans.assign (b, e);
    }
}

static bool  jssafe[] = {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,		// 0--15
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,		// 16--31
    1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,		// 32--47
    1,1,1,1,1,1,1,1,1,1,1,1,0,1,0,0,		// 48--63
    0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,		// 64--79
    1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,1,		// 80--95
    0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,		// 96--111
    1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,		// 112--127
};

ustring  jsEncode (const ustring& str) {
    int  i;
    ustring  u, ans;
    int  c, d;
    char  b[8];

    u = utf8to16 (str);
    ans.reserve (u.size () * 3);
    b[0] = '\\';
    b[1] = 'u';
    for (i = 0; i < u.size (); i += 2) {
	c = u[i];
	d = u[i + 1];
	if (c == 0 && 0 < d && d < 127 && jssafe[d]) {
	    ans.append (1, d);
	} else {
	    b[2] = hexchar ((c >> 4) & 0x0f);
	    b[3] = hexchar (c & 0x0f);
	    b[4] = hexchar ((d >> 4) & 0x0f);
	    b[5] = hexchar (d & 0x0f);
	    ans.append (b, 6);
	}
    }
    return ans;
}

ustring  filenameEncode (const ustring& text) {
    static uregex  re ("([\\x00-\\x1f\\x7f])|([^a-zA-Z0-9._-])|(^\\.+)");
    SplitterRe  sp (text, re);
    ustring  ans;
    int  c;

    if (text.length () == 0) {
	throw (ustring (text).append (uErrorBadName));
    }
    ans.reserve (text.length () + 16);
    while (sp.next ()) {
	if (sp.begin () < sp.end ())
	    ans.append (sp.begin (), sp.end ());
	if (sp.match (1)) {
	} else if (sp.match (2)) {
	    c = *sp.matchBegin (2);
	    ans.append (1, ':');
	    ans.append (1, hexchar ((c >> 4) & 0x0f));
	    ans.append (1, hexchar (c & 0x0f));
	} else if (sp.match (3)) {
	    for (c = sp.matchEnd (3) - sp.matchBegin (3); c > 0; c --) {
		ans.append (CharConst (":2e"));
	    }
	}
    }
    if (ans.length () > 250)
	ans.resize (250);
    return ans;
}

ustring  filenameEncodePath (const ustring& text) {
    // '/'をディレクトリパスの区切りとして許可する
    ustring  ans;
    static uregex  re ("/+");
    SplitterRe  sp (text, re);
    while (sp.next ()) {
	if (sp.begin () < sp.end ())
	    ans.append (filenameEncode (ustring (sp.begin (), sp.end ())));
	ans.append (CharConst ("/"));
    }
    if (sp.begin () < sp.end ())
	ans.append (filenameEncode (ustring (sp.begin (), sp.end ())));
    return ans;
}

ustring  filenameDecode (const ustring& text) {
    static uregex  re (":([0-9a-fA-F][0-9a-fA-F])");
    SplitterRe  sp (text, re);
    ustring  ans;
    int  c;

    ans.reserve (text.length ());
    while (sp.next ()) {
	if (sp.begin () < sp.end ())
	    ans.append (sp.begin (), sp.end ());
	if (sp.match (1)) {
	    c = hex (*(sp.matchBegin (1))) * 16 + hex (*(sp.matchBegin (1) + 1));
	    if (32 <= c && c < 256)
		ans.append (1, c);
	}
    }
    return ans;
}

bool  matchSkip (uiterator& b, uiterator e, const char* t, size_t s) {
    if (e - b >= s && memcmp (t, &b[0], s) == 0) {
	b += s;
	return true;
    } else {
	return false;
    }
}

bool  matchHead (uiterator& b, uiterator e, const char* t, size_t s) {
    if (e - b >= s && memcmp (t, &b[0], s) == 0) {
	return true;
    } else {
	return false;
    }
}

bool  matchHead (const ustring& str, const char* t, size_t s) {
    if (str.length () >= s && memcmp (t, &*str.begin (), s) == 0) {
	return true;
    } else {
	return false;
    }
}

bool  matchHead (const ustring& str, const ustring& head) {
    if (str.length () >= head.length () && memcmp (&*str.begin (), &*head.begin (), head.length ()) == 0) {
	return true;
    } else {
	return false;
    }
}

bool  match (uiterator b, uiterator e, const char* t, size_t s) {
    if (e - b == s && memcmp (t, &b[0], s) == 0) {
	return true;
    } else {
	return false;
    }
}

bool  match (const ustring& str, const char* t, size_t s) {
    if (str.length () == s && memcmp (t, str.data (), s) == 0) {
	return true;
    } else {
	return false;
    }
}

bool  match (uiterator b, uiterator e, const ustring& str) {
    if (e - b == str.length () && memcmp (str.data (), &b[0], str.length ()) == 0) {
	return true;
    } else {
	return false;
    }
}

bool  match (const ustring& str, const char* t, size_t s, const char* t2, size_t s2) {
    if (match (str, t, s) || match (str, t2, s2)) {
	return true;
    } else {
	return false;
    }
}

ustring  clipWhite (uiterator b, uiterator e) {
    while (b < e)
	if (isblank (*b)) {
	    b ++;
	} else {
	    break;
	}
    while (b < e)
	if (isblank (*(e - 1))) {
	    e --;
	} else {
	    break;
	}
    return ustring (b, e);
}
ustring  clipWhite (const ustring& str) {
    return clipWhite (str.begin (), str.end ());
}

ustring  getenvString (const char* key) {
    char*  e = getenv (key);
    if (e) {
	return ustring (e);
    } else {
	return uEmpty;
    }
}

ustring  zeroPad (int n, const ustring& src) {
    int  m;

    n = std::min (32, n);
    m = n - src.length ();
    if (m > 0) {
	ustring  ans;
	ans.reserve (m);
	ans.append (m, '0');
	ans.append (src);
	return ans;
    } else {
	return src;
    }
}

ustring  padEmpty (const ustring& name) {
    if (name.empty ())
	return ustring (CharConst ("(null)"));
    else
	return name;
}

uint64_t  hextoul (uiterator b, uiterator e) {
    uint64_t  ans = 0;
    int  n;

    for (n = 0; n < 8 && b != e; n ++, b ++) {
	ans = (ans << 4) + hex (*b);
    }
    return ans;
}

double  hextod (uiterator b, uiterator e, int base) {
    double  ans = 0.0;
    int  n;
    int  c;

    for (n = 0; b < e; n ++, b ++) {
	c = shex (*b);
	if (c < 0 || c >= base)
	    return ans;
	ans = ans * 16. + c;
    }
    return ans;
}

ustring  tohex (uint64_t e, int pad, int base, bool upcase) {
    uint64_t  a, b;
    int  r;
    ustring  ans;
    char  d[128];
    int  pos;
    const char*  digs;
    static const char  xdigsLower[] = "0123456789abcdef";
    static const char  xdigsUpper[] = "0123456789ABCDEF";

    pos = 128;
    b = base;
    if (upcase)
	digs = xdigsUpper;
    else
	digs = xdigsLower;
    if (e >= 0) {
	while (pos > 0 && e > 0) {
	    a = e / b;
	    r = e - a * b;
	    e = a;
	    if (r < 0) {
		r = 0;
	    } else if (r >= base) {
		r = base - 1;
	    }
	    d[--pos] = digs[r];
	}
	if (pad > 0) {
	    for (int i = 128 - pos; i < pad && i < 128; i ++) {
		d[--pos] = '0';
	    }
	}
	ans.assign (d + pos, 128 - pos);
    } else {
	/* *** */
    }
    return ans;
}

ustring  toCRLF (const ustring& str) {
    uiterator  b = str.begin ();
    uiterator  e = str.end ();
    uiterator  p;
    ustring  ans;

    p = b;
    while (findChar (b, e, '\n')) {
	ans.append (p, b).append (uCRLF);
	p = ++ b;
    }
    if (p < e)
	ans.append (p, e);
    return ans;
}

void  skipChar (uiterator& b, uiterator e, int ch) {
    while (b < e && *b == ch)
	++ b;
}

void  skipNextToChar (uiterator& b, uiterator e, int ch) {
    while (b < e) {
	if (*(b ++) == ch)
	    return;
    }
}

static ustring::value_type  toLower_ustring_value (ustring::value_type v) {
    if ('A' <= v && v <= 'Z') {
	return v - 'A' + 'a';
    } else {
	return v;
    }
}

ustring  toLower (uiterator b, uiterator e) {
    ustring::iterator  i;
    ustring  ans;
    ans.resize (e - b);
    i = ans.begin ();
    for (; b < e; b ++, i++) {
	*i = toLower_ustring_value (*b);
    }
    return ans;
}

static ustring  colpad0 (int n, const ustring& src) {
    int  m;

    if (n > 0) {
	n = std::min (32, n);
	m = n - src.length ();
	if (m > 0) {
	    ustring  ans;
	    ans.reserve (n);
	    ans.append (m, '0');
	    ans.append (src);
	    return ans;
	} else if (m == 0) {
	    return src;
	} else {
	    return ustring (src.end () - n, src.end ());
	}
    } else {
	return src;
    }
}

/*
 ${Y:4}, ${Y:2}
 ${M:2}, ${M}, ${M:name}, ${M:ab}
 ${D:2}, ${D}
 ${h:2}, ${h}
 ${m:2}, ${m}
 ${s:2}, ${s}
 ${W}, ${w}
 ${o}
*/
#if 0
ustring  formatDateString (const ustring& format, struct tm& v) {
    ustring  ans;
    uiterator  b, e;
    umatch  m;
    int  pc;
    static uregex  re ("\\$\\{(([YMDhmsWwo])(:([0-9]))?|M:((name)|(ab)|(abname)))\\}");
    std::vector<ustring>  fpar;

    b = format.begin ();
    e = format.end ();
    while (usearch (b, e, m, re)) {
	ans.append (b, m[0].first);
	b = m[0].second;
	if (m[5].matched) {
	    if (m[6].matched) {	// name
		ans.append (MStr[v.tm_mon]);
	    } else if (m[7].matched || m[8].matched) { // abname
		ans.append (MStr_a[v.tm_mon]);
	    }
	} else {
	    if (m[3].matched) {
		pc = strtol (ustring (m[4].first, m[4].second));
	    } else {
		pc = 0;
	    }
	    switch (*m[2].first) {
	    case 'Y':
		ans.append (colpad0 (pc, to_ustring (v.tm_year + 1900)));
		break;
	    case 'M':
		ans.append (colpad0 (pc, to_ustring (v.tm_mon + 1)));
		break;
	    case 'D':
		ans.append (colpad0 (pc, to_ustring (v.tm_mday)));
		break;
	    case 'h':
		ans.append (colpad0 (pc, to_ustring (v.tm_hour)));
		break;
	    case 'm':
		ans.append (colpad0 (pc, to_ustring (v.tm_min)));
		break;
	    case 's':
		ans.append (colpad0 (pc, to_ustring (v.tm_sec)));
		break;
	    case 'W':
		ans.append (WStr [v.tm_wday]);
		break;
	    case 'w':
		ans.append (WStr_a [v.tm_wday]);
		break;
	    case 'o':
		{
		    int  h, m;
		    if (v.tm_gmtoff < 0) {
			h = - v.tm_gmtoff / 60;
			m = h % 60;
			h = h / 60;
			ans.append (CharConst ("-")).append (colpad0 (4, to_ustring (h * 100 + m)));
		    } else {
			h = v.tm_gmtoff / 60;
			m = h % 60;
			h = h / 60;
			ans.append (CharConst ("+")).append (colpad0 (4, to_ustring (h * 100 + m)));
		    }
		}
		break;
	    }
	}
    }
    ans.append (b, e);

    return ans;
}
#endif
ustring  formatDateString (const ustring& format, struct tm& tmv) {
    size_t  n = format.length () * 2;
    if (n > 65535)
	n = 65535;
    char*  buf = new char[n];
    size_t  s = strftime (buf, n, format.c_str (), &tmv);
    ustring  ans (buf, s);
    delete[] buf;
    return ans;
}

ustring  toLower (const ustring& str) {
    return boost::to_lower_copy (str);
}

ustring  toUpper (const ustring& str) {
    return boost::to_upper_copy (str);
}

ustring  hexEncode (const ustring& data, bool upcase) {
    ustring  ans;
    uiterator  b, e;
    char  (*fn) (int);

    if (upcase)
	fn = hexchar_c;
    else
	fn = hexchar;
    ans.reserve (data.length () * 2);
    b = data.begin ();
    e = data.end ();
    for (; b < e; b ++) {
	ans.append (1, fn ((*b >> 4) & 0x0f));
	ans.append (1, fn (*b & 0x0f));
    }
    return ans;
}

ustring  hexDecode (const ustring& data) {
    ustring  ans;
    uiterator  b, e;
    int  c;

    ans.reserve (data.length () / 2);
    b = data.begin ();
    e = data.end ();
    for (; b < e; b ++) {
	c = *b ++;
	if (b < e) {
	    ans.append (1, hex (c, *b));
	}
    }
    return ans;
}

int  octchar (uiterator b) {	// 3bytes
    int  ans = 0;
    ans = *b - '0';
    ++ b;
    ans = ans * 8 + *b - '0';
    ++ b;
    ans = ans * 8 + *b - '0';
    return ans;
}

ustring  octchar (int c) {
    ustring  ans (3, 0);
    ans[2] = (c & 0x7) + '0';
    c >>= 3;
    ans[1] = (c & 0x7) + '0';
    c >>= 3;
    ans[0] = (c & 0x3) + '0';
    return ans;
}

bool  findNL (uiterator& b, uiterator e, uiterator& u) {
    for (; b < e; ++ b) {
	if (*b == '\n') {
	    u = b + 1;
	    return true;
	} else if (*b == '\r') {
	    u = b + 1;
	    if (u < e && *u == '\n')
		++ u;
	    return true;
	}
    }
    u = e;
    return false;
}

bool  findNLb (uiterator& b, uiterator e) {
    for (; b < e; ++ b) {
	if (*b == '\n') {
	    ++ b;
	    return true;
	} else if (*b == '\r') {
	    ++ b;
	    if (b < e && *b == '\n')
		++ b;
	    return true;
	}
    }
    return false;
}

bool  findChar (uiterator& b, uiterator e, int ch) {
    for (; b < e; ++ b) {
	if (*b == ch) {
	    return true;
	}
    }
    return false;
}

bool  findChars (uiterator& b, uiterator e, const ustring& pattern) {
    for (; b < e; ++ b) {
	if (pattern.find (*b) != ustring::npos) {
	    return true;
	}
    }
    return false;
}

bool  findCharFn (uiterator& b, uiterator e, bool (*fn)(int)) {
    for (; b < e; ++ b) {
	if (fn (*b))
	    return true;
    }
    return false;
}

bool  findSepColon (uiterator& b, uiterator e, uiterator& u) {
    // " *; *"を探索する。bは進む
    uiterator  p = b;
    if (findChar (b, e, ';')) {
	u = b + 1;
	while (p < b && *(b - 1) == ' ')
	    -- b;
	while (u < e && *u == ' ')
	    ++ u;
	return true;
    }
    u = e;
    return false;
}

bool  matchHeadFn (uiterator& b, uiterator e, bool (*fn)(int)) {
    if (b < e && fn (*b)) {
	do {
	    ++ b;
	} while (b < e && fn (*b));
	return true;
    }
    return false;
}

bool  matchWordTbl (uiterator b, uiterator e, char* tbl) {
    int  c;
    if (b < e) {
	do {
	    c = *b;
	    if (0 <= c && c < 128 && tbl[c]) {	// 128〜はfalse
	    } else {
		return false;
	    }
	    ++ b;
	} while (b < e);
	return true;
    } else {
	return false;
    }
}

bool  matchWordFn (uiterator b, uiterator e, bool (*fn)(int)) {
    int  c;
    if (b < e) {
	do {
	    c = *b;
	    if (0 <= c && c < 128 && fn (c)) {
	    } else {
		return false;
	    }
	    ++ b;
	} while (b < e);
	return true;
    } else {
	return false;
    }
}
