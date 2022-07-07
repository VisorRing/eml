#include "iso2022jp.h"
#include "util_base64.h"
#include "util_string.h"
#include "ustring.h"

ustring  mimeEncodeJP (const ustring& text) {
    ustring  ans;
    uiterator  b, e;
    umatch  m;
    ustring  t;
    static uregex  re ("(\\033\\$[@B][^\\033]*\\033\\([BJ])");
    
    b = text.begin ();
    e = text.end ();
    while (usearch (b, e, m, re)) {
	if (b != m[0].first)
	    ans.append (b, m[0].first);
	ans.append (CharConst ("=?ISO-2022-JP?B?"));
	t = base64Encode (m[0].first, m[0].second);
	ans.append (t);
	ans.append (CharConst ("?="));
	b = m[0].second;
    }
    if (b != e)
	ans.append (b, e);
    return ans;
}
