#include "util_mimetype.h"
#include "config.h"
#include "util_const.h"
#include "util_string.h"
#include "motorconst.h"
#include "httpconst.h"
#include "bdbmacro.h"
#include "ustring.h"
#include <sys/stat.h>

static struct {
    const char*  ext;
    size_t  extlen;
    const char*  type;
    size_t  typelen;
}  typelist [] = {
    {CharConst ("js"), CharConst ("application/javascript")},
    {CharConst ("doc"), CharConst ("application/msword")},
    {CharConst ("pdf"), CharConst ("application/pdf")},
    {CharConst ("xls"), CharConst ("application/vnd.ms-excel")},
    {CharConst ("ppt"), CharConst ("application/vnd.ms-powerpoint")},
    {CharConst ("swf"), CharConst ("application/x-shockwave-flash")},
    {CharConst ("gif"), CharConst ("image/gif")},
    {CharConst ("jpeg"), CharConst ("image/jpeg")},
    {CharConst ("jpg"), CharConst ("image/jpeg")},
    {CharConst ("png"), CharConst ("image/png")},
    {CharConst ("tiff"), CharConst ("image/tiff")},
    {CharConst ("tif"), CharConst ("image/tiff")},
    {CharConst ("css"), CharConst ("text/css")},
    {CharConst ("csv"), CharConst ("text/csv")},
    {CharConst ("html"), CharConst ("text/html")},
    {CharConst ("htm"), CharConst ("text/html")},
    {CharConst ("txt"), CharConst ("text/plain")},
    {CharConst ("text"), CharConst ("text/plain")},
    {CharConst ("3gp"), CharConst ("video/3gpp")},
    {CharConst ("mp4"), CharConst ("video/mp4")},
    {CharConst ("mpg4"), CharConst ("video/mp4")},
    {CharConst ("mpeg"), CharConst ("video/mpeg")},
    {CharConst ("mpg"), CharConst ("video/mpeg")},
    {CharConst ("mov"), CharConst ("video/quicktime")},
    {CharConst ("flv"), CharConst ("video/x-flv")},
    {CharConst ("m4v"), CharConst ("video/x-m4v")},
    {CharConst ("wm"), CharConst ("video/x-ms-wm")},
    {CharConst ("avi"), CharConst ("video/x-msvideo")},
    {NULL, 0, NULL, 0}
};

static ustring  mimetype_db (const ustring& ext, bool f) {
    BDBHashRDOnly  bdb;
    ustring  ans;

    bdb.open (cDataTop kMimeTypeDB);
    if (! bdb.get (ext, ans)) {
	if (f)
	    ans.assign (CharConst (kMIME_OCTET));
	else
	    ans.resize (0);
    }
    bdb.close ();
    return ans;
}

static ustring  mimetype_static (const ustring& ext, bool f) {
    int  i;

    for (i = 0; typelist[i].ext; i ++) {
	if (match (ext, typelist[i].ext, typelist[i].extlen))
	    return ustring (typelist[i].type, typelist[i].typelen);
    }
    if (f)
	return ustring (CharConst (kMIME_OCTET));
    else
	return uEmpty;
}

ustring  getExt (const ustring& name) {
    static uregex  re ("\\.([a-zA-Z0-9]{1,16})$");
    umatch  m;

    if (usearch (name, m, re)) {
	ustring  ans (m[1].second - m[1].first, ' ');
	uiterator  b;
	ustring::iterator  it;
	for (it = ans.begin (), b = m[1].first; b != m[1].second; it ++, b ++) {
	    if ('A' <= *b && *b <= 'Z') {
		*it = *b + ('a' - 'A');
	    } else {
		*it = *b;
	    }
	}
	return ans;
    } else {
	return uEmpty;
    }
}

ustring  mimetype (const ustring& ext, bool f) {
    struct stat  sb;

    if (ext.length () == 0) {
	if (f)
	    return ustring (CharConst (kMIME_OCTET));
	else
	    return uEmpty;
    } else {
	if (stat (cDataTop kMimeTypeDB, &sb) == 0) {
	    return mimetype_db (ext, f);
	} else {
	    return mimetype_static (ext, f);
	}
    }
}
