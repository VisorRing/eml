#ifndef UTIL_WSPLITTER_H
#define UTIL_WSPLITTER_H

#include "ustring.h"
#include "utf16.h"

class  WSplitter {
 public:
    boost::wregex*  re;
    std::wstring::const_iterator  b, t, u, e;
    boost::wsmatch  m;

    WSplitter (const std::wstring& text, boost::wregex& r) {
	b = t = u = text.begin ();
	e = text.end ();
	re = &r;
    };
    virtual  ~WSplitter () {};
    virtual bool  next () {
	b = u;
	if (b != e) {
	    if (regex_search (b, e, m, *re, boost::regex_constants::match_single_line)) {
		t = m[0].first;
		u = m[0].second;
	    } else {
		t = e;
		u = e;
	    }
	    return true;
	} else {
	    return false;
	}
    };
    virtual bool  nextSep () {
	b = u;
	if (b != e) {
	    if (regex_search (b, e, m, *re, boost::regex_constants::match_single_line)) {
		t = m[0].first;
		u = m[0].second;
		return true;
	    } else {
		t = e;
		u = e;
		return false;
	    }
	} else {
	    t = u = e;
	    return false;
	}
    };
    virtual ustring  cur () {
	std::wstring  x (b, t);
	return wtou (x);
    };
};

#endif /* UTIL_WSPLITTER_H */
