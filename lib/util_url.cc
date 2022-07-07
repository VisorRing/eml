#include "config.h"
#include "util_url.h"
#include "util_const.h"
#include "util_string.h"
#include "ml.h"
#include "ustring.h"

/*
  genericurl     = scheme ":" schemepart
  scheme         = 1*[ lowalpha | digit | "+" | "-" | "." ]
  schemepart     = *xchar | ip-schemepart
  ip-schemepart  = "//" login [ "/" urlpath ]
  login          = [ user [ ":" password ] "@" ] hostport
  hostport       = host [ ":" port ]
  host           = hostname | hostnumber
  hostname       = *[ domainlabel "." ] toplabel
  domainlabel    = alphadigit | alphadigit *[ alphadigit | "-" ] alphadigit
  toplabel       = alpha | alpha *[ alphadigit | "-" ] alphadigit
  alphadigit     = alpha | digit
  hostnumber     = digits "." digits "." digits "." digits
  port           = digits
  user           = *[ uchar | ";" | "?" | "&" | "=" ]
  password       = *[ uchar | ";" | "?" | "&" | "=" ]
  urlpath        = *xchar    ; depends on protocol see section 3.1

  lowalpha       = "a" ... "z"
  hialpha        = "A" ... "Z"
  alpha          = lowalpha | hialpha
  digit          = "0" ... "9"
  safe           = "$" | "-" | "_" | "." | "+"
  extra          = "!" | "*" | "'" | "(" | ")" | ","
  national       = "{" | "}" | "|" | "\" | "^" | "~" | "[" | "]" | "`"
  punctuation    = "<" | ">" | "#" | "%" | <">
  reserved       = ";" | "/" | "?" | ":" | "@" | "&" | "="
  hex            = digit | "A" ... "F" | "a" ... "f"
  escape         = "%" hex hex

  unreserved     = alpha | digit | safe | extra
  uchar          = unreserved | escape
  xchar          = unreserved | reserved | escape
  digits         = 1*digit
*/

bool  checkURL (ustring& url, upair* scheme, upair* delim, upair* user, upair* pass, upair* host, upair* port, upair* rest) {
#define CDIGIT		"0-9"
#define CHEX		CDIGIT "a-fA-F"
#define CALPHA		"a-zA-Z"
#define CALPHADIGIT	CALPHA CDIGIT
#define MSB1CHAR	"\x80-\xff"
#define CSAFE		"$\\-_.+"
#define CEXTRA		"!*'(),"
#define CRESERVED	";/?:@&="
#define CESCAPE		"%[" CHEX "][" CHEX "]"
#define CUNRESERVED	CALPHADIGIT CSAFE CEXTRA
#define WUCHAR		"[" CUNRESERVED MSB1CHAR "]|" CESCAPE
#define WXCHAR		"[" CUNRESERVED CRESERVED MSB1CHAR "]|" CESCAPE
#define WUSER		"(" WUCHAR "|[;?&=])*"
#define WPASSWORD	WUSER
#define WHOST		"([" CALPHADIGIT "][" CALPHADIGIT "-]*\\.)*[" CALPHADIGIT "][" CALPHADIGIT "-]*"
#define WHOSTPART	"(" WHOST ")(:([" CDIGIT "]+))?"
#define WLOGIN		"((" WUSER ")(:(" WPASSWORD "))?@)?" WHOSTPART
    uiterator  b, e;
    umatch  m;
    static uregex  re_scheme ("^([a-z0-9][a-z0-9+.-]*):");
    static uregex  re_ippart ("^//" WLOGIN);
    static uregex  re_part ("^(" WXCHAR ")*");

    if (scheme)
	scheme->first = scheme->second = uEmpty.begin ();
    if (delim)
	delim->first = delim->second = uEmpty.begin ();
    if (user)
	user->first = user->second = uEmpty.begin ();
    if (pass)
	pass->first = pass->second = uEmpty.begin ();
    if (host)
	host->first = host->second = uEmpty.begin ();
    if (port)
	port->first = port->second = uEmpty.begin ();
    if (rest)
	rest->first = rest->second = uEmpty.begin ();

    b = url.begin ();
    e = url.end ();
    if (usearch (b, e, m, re_scheme)) {
	// 1:scheme
	if (scheme)
	    *scheme = m[1];
	b = m[0].second;
	if (usearch (b, e, m, re_ippart)) {
	    // 2:user, 5:password, 7:host, 10:port
	    if (delim) {
		delim->first = b - 1;
		delim->second = b + 2;
	    }
	    if (user)
		*user = m[2];
	    if (pass)
		*pass = m[5];
	    if (host)
		*host = m[7];
	    if (port)
		*port = m[10];
	    if (rest) {
		rest->first = m[0].second;
		rest->second = e;
	    }
	    return true;
	} else if (usearch (b, e, m, re_part)) {
	    // 0:*xchar
	    if (m[0].second == e) {
		if (delim) {
		    delim->first = b - 1;
		    delim->second = b;
		}
		if (rest)
		    *rest = m[0];
		return true;
	    } else {
		return false;
	    }
	} else {
	    return false;
	}
    } else {
	return false;
    }
}

bool  checkURLSafe (ustring& url) {
    umatch  m;
    static uregex  re ("^(" WXCHAR ")*$");

    return usearch (url, m, re);
}

bool  checkScheme (ustring& proto) {
    umatch  m;
    static uregex  re ("^([a-z0-9][a-z0-9+.-]*)$");

    return  usearch (proto, m, re);
}

#ifdef QUERYENCODEALT
static ustring  urlencode (uiterator b, uiterator e, const uregex& re) {
    umatch  m;
    ustring  ans;
    int  c;

    while (b < e && usearch (b, e, m, re)) {
	while (b < m[0].first) {
	    if (*b == 0)
		ans.append (uUScore);
	    else
		ans.append (percentHEX (*b++));
	}
	ans.append (m[0].first, m[0].second);
	b = m[0].second;
    }
    while (b < e) {
	c = *b ++;
	if (c == 0)
	    ans.append (uUScore);
	else
	    ans.append (percentHEX (c));
    }

    return ans;
}

ustring  urlencode (uiterator b, uiterator e) {
    static uregex  re ("[" CUNRESERVED MSB1CHAR "]+");
    return urlencode (b, e, re);
}

ustring  urlencode_path (uiterator b, uiterator e) {
    static uregex  re ("[" CUNRESERVED "/" MSB1CHAR "]+");
    return urlencode (b, e, re);
}
#endif

ustring  buildQuery (MNode* query) {
    int  c = 0;
    ustring  ans;
    ustring  u;

    while (query && query->isCons ()) {
	MNode*  a = query->car ();
	if (isCons (a)) {
	    if (c == 0)
		ans.append (CharConst ("?"));
	    else
		ans.append (CharConst ("&"));
	    c ++;
	    u = to_string (a->car ());
#ifdef QUERYENCODEALT
	    ans.append (urlencode (u.begin (), u.end ()));
#else
	    ans.append (percentEncode (u.begin (), u.end ()));
#endif
	    ans.append (uEq);
	    u = to_string (a->cdr ());
#ifdef QUERYENCODEALT
	    ans.append (urlencode (u.begin (), u.end ()));
#else
	    ans.append (percentEncode (u.begin (), u.end ()));
#endif
	} else if (isNil (a)) {
	} else {
	    throw (to_string (a) + uErrorBadValue);
	}
	nextNode (query);
    }
    return ans;
}
