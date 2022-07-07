#include "utf8-jp.h"
#include "utf8.h"
#include "ustring.h"
#include "japanese-hankaku.h"
#include "cdbobj.h"

typedef struct {
    const char*  c;
    size_t  s;
}  CS;

static uregex  re_js ("(\xe2\x80\x94)|(\xe3\x80\x9c)|(\xe2\x80\x96)|(\xe2\x88\x92)|(\xc2\xa2)|(\xc2\xa3)|(\xc2\xac)");
static CS  t_ms[] = {
	CharConst ("\xe2\x80\x95"),
	CharConst ("\xef\xbd\x9e"),
	CharConst ("\xe2\x88\xa5"),
	CharConst ("\xef\xbc\x8d"),
	CharConst ("\xef\xbf\xa0"),
	CharConst ("\xef\xbf\xa1"),
	CharConst ("\xef\xbf\xa2"),
};
static uregex  re_ms ("(\xe2\x80\x95)|(\xef\xbd\x9e)|(\xe2\x88\xa5)|(\xef\xbc\x8d)|(\xef\xbf\xa0)|(\xef\xbf\xa1)|(\xef\xbf\xa2)");
static CS  t_js[] = {
	CharConst ("\xe2\x80\x94"),
	CharConst ("\xe3\x80\x9c"),
	CharConst ("\xe2\x80\x96"),
	CharConst ("\xe2\x88\x92"),
	CharConst ("\xc2\xa2"),
	CharConst ("\xc2\xa3"),
	CharConst ("\xc2\xac"),
};


static ustring  fix (const ustring& text, uregex& re, CS* t) {
    uiterator  b, e;
    umatch  m;
    ustring  ans;
    int  i;

    ans.reserve (text.size () + 32);
    b = text.begin ();
    e = text.end ();
    while (usearch (b, e, m, re)) {
	if (b != m[0].first)
	    ans.append (b, m[0].first);
	for (i = 0; i < 7; i ++) {
	    if (m[i + 1].matched) {
		ans.append (t[i].c, t[i].s);
		break;
	    }
	}
	b = m[0].second;
    }
    if (b != e)
	ans.append (b, e);

    return ans;
}

ustring  fixFromMS (const ustring& text) {
    return fix (text, re_ms, t_js);
}

ustring  fixToMS (const ustring& text) {
    return fix (text, re_js, t_ms);
}

ustring  fullWidthASCIItoASCII (const ustring& text) {
    CDBMem  cdb (JPHankakuTable, JPHankakuTableSize);

    return translateChar (text, &cdb);
}
