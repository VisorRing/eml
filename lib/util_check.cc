#include "util_check.h"
#include "util_const.h"
#include "util_string.h"
#include "httpconst.h"
#include "motorconst.h"
#include "ustring.h"
#include <ctype.h>

bool  checkRe (const ustring& name, const uregex& re) {
    umatch  m;

    if (usearch (name, m, re)) {
	return true;		// OK
    } else {
	return false;		// NG
    }
}

bool  checkRe (uiterator b, uiterator e, const uregex& re) {
    umatch  m;

    if (usearch (b, e, m, re)) {
	return true;		// OK
    } else {
	return false;		// NG
    }
}

bool  matchName (const ustring& name) {
    static uregex  re ("^" kWNAME "{0,31}$");

    return (checkRe (name, re));
}

bool  matchFilename (const ustring& name) {
    static uregex  re ("^" kWNAME "{1,127}(\\." kWORD "{1,16})?$");

    return (checkRe (name, re));
}

bool  matchResourceName (const ustring& name) {
//    static uregex  re ("^(" kWNAME "{0,127}/)*" kWNAME "{0,127}(\\." kWORD "{1,16})?$");
    static uregex  re ("^([a-zA-Z0-9_][a-zA-Z0-9_.-]{0,127}/)*[a-zA-Z0-9_][a-zA-Z0-9_.-]{0,127}(\\." kWORD "{1,16})?$");

    return (checkRe (name, re));
}

bool  matchAbsoluteResourceName (const ustring& name) {
    static uregex  re ("^/(" kFName "/)*" kFName "$");

    return (checkRe (name, re));
}

static bool  isPrintableAscii (int c) {
    return 0x20 <= c && c <= 0x7e;
}

bool  matchASCII (uiterator b, uiterator e) {	// [ -\x7e]
    return matchWordFn (b, e, isPrintableAscii);
}

bool  matchIP (const ustring& name) {
    static uregex  re ("^[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}$");

    return (checkRe (name, re));
}

bool  matchDomain_dot (const ustring& name) {
    static uregex  re ("^\\.?([a-zA-Z0-9-]+\\.)*([a-zA-Z0-9-]+)$");

    return (checkRe (name, re));
}

bool  matchHostname (const ustring& name) {
    static uregex  re ("^[a-zA-Z0-9][a-zA-Z0-9\\-]*(\\.[a-zA-Z0-9][a-zA-Z0-9\\-]*)*$");

    return (checkRe (name, re));
}

bool  matchMimeType (const ustring& name) {
    static  uregex re ("^[a-z_0-9-]+/[a-z_0-9.+*-]+$");

    return (checkRe (name, re));
}

bool  checkMailAddr (const ustring& name) {
    static  uregex re ("^[^\\x00- @\\x7f-\\xff]+@[a-zA-Z0-9-]+(\\.[a-zA-Z0-9-]+)*$");

    return (checkRe (name, re));
}

bool  matchAlNum (uiterator b, uiterator e) {
    static char  table_alnum[] = {	// [a-zA-Z_0-9]
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 
	0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 
	0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 
    };
    return matchWordTbl (b, e, table_alnum);
}

bool  matchNum (uiterator b, uiterator e) {    
    for (; b < e; ++ b) {
	if (! isdigit (*b))
	    return false;
    }
    return true;
}

bool  matchWidth (uiterator b, uiterator e) {
    static uregex  re_width ("^[0-9]+(\\.[0-9]+)?(%|px|pt|in|mm|cm|em|ex)?$");
    return (checkRe (b, e, re_width));
}

bool  checkColor (uiterator b, uiterator e) {
    static uregex  re_color ("^#([0-9a-fA-F]{3}){1,2}$");
    return (checkRe (b, e, re_color));
}

bool  matchWikiID (uiterator b, uiterator e) {
    static uregex  re_wikiid ("^" rWikiID "$");
    return (checkRe (b, e, re_wikiid));
}

bool  checkAry (const ustring& name) {
    return (name.length () > 0 && name[0] == '@');
}

bool  checkAry (const ustring& name, ustring& sym) {
    if (checkAry (name)) {
	sym.assign (name.begin () + 1, name.end ());
	return true;
    } else {
	return false;
    }
}

bool  isHTTPS () {
    char*  e = getenv (kHTTPS);

    if (e && *e == 'o') {
	return true;
    } else {
	return false;
    }
}

int  checkAgent () {
    umatch  m;
    int  ans = 0;
    static uregex  re_os ("Macintosh|Mac_|Windows|iPhone|iPod");
    static uregex  re_ua ("Safari|MSIE|Gecko|Opera|NetFront");

    ustring  agent = getenvString (kHTTP_USER_AGENT);
    if (usearch (agent, m, re_os)) {
	switch (*m[0].first) {
	case 'M':
	    ans |= UA_Mac;
	    break;
	case 'W':
	    ans |= UA_Windows;
	    break;
	case 'i':
	    ans |= UA_iPhone;
	    break;
	}
    }
    if (usearch (agent, m, re_ua)) {
	switch (*m[0].first) {
	case 'S':
	    ans |= UA_Safari;
	    break;
	case 'M':
	    ans |= UA_IE;
	    break;
	case 'G':
	    ans |= UA_Mozilla;
	    break;
	case 'O':
	    ans |= UA_Opera;
	    break;
	case 'N':
	    ans |= UA_NetFront;
	    break;
	}
    }
    return ans;
}
