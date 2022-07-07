#include "util_apache.h"
#include "util_const.h"
#include "util_file.h"
#include "util_splitter.h"
#include "util_string.h"
#include "httpconst.h"
#include "ustring.h"
#include <vector>

ustring  apacheAbsolutePath (const ustring& url) {
    ustring  ans;
    std::vector<ustring>  ary;
    std::vector<ustring>::iterator  it;
    SplitterCh  sp (url, '/');
    uiterator  b, e;
    bool  fdirpath;
    size_t  len = url.length ();

    if (len == 0 || url[len - 1] == '/') {
	fdirpath = true;
    } else {
	fdirpath = false;
    }
    if (isAbsolutePath (url)) {
	sp.next ();
    } else {
	// mod_rewriteで書き換えた時、元のURLを返す
	ustring  e = getenvString (kREQUEST_URI);
	// REQUEST_URIはデコードされていない。
	ustring::size_type  p = e.find_last_of ('?');
	if (p != ustring::npos)
	    e.resize (p);
	e = percentDecode (e);	// ディレクトリパスをチェック前にデコードする。
	splitE (e.begin (), e.end (), '/', ary);
	if (ary.size () > 0 && ary.back ().length () > 0) {
	    ary.pop_back();
	}
    }
    while (sp.next ()) {
	b = sp.begin ();
	e = sp.end ();
	if (b == e) {
	} else if (match (b, e, CharConst ("."))) {
	} else if (match (b, e, CharConst (".."))) {
	    if (ary.size () > 0)
		ary.pop_back ();
	} else {
	    ary.push_back (ustring (b, e));
	}
    }

    for (it = ary.begin (); it != ary.end (); it ++) {
	if ((*it).length () > 0) {
	    ans.append (uSlash).append (*it);
	}
    }
    if (fdirpath) {
	ans.append (uSlash);
    }

    return ans;
}
