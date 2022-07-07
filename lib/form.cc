#include "form.h"
#include "app.h"
#include "motorenv.h"
#include "httpconst.h"
#include "util_const.h"
#include "util_check.h"
#include "util_string.h"
#include "ustring.h"
#include <iostream>
#include <stdlib.h>
#include <string.h>

//#define STRICT_FORMVAR	0

void  CGIForm::parseURLEncoded (const ustring& query) {
    ustring::size_type  s, e, t;
    uiterator  ib, ie;
    uiterator  is = query.begin ();

    s = 0;
    while (1) {
	ib = is + s;
	e = query.find ('&', s);
	if (e == ustring::npos) {
	    ie = query.end ();
	    insertNameValue (iarg, ib, ie);
	    break;
	} else {
	    ie = is + e;
	    insertNameValue (iarg, ib, ie);
	    s = e + 1;
	}
    }
}

void  CGIForm::read_get () {
    queryString = getenvString (kQUERY_STRING);
    if (queryString.length () > 0) {
	parseURLEncoded (queryString);
    }
}

void  CGIForm::read_post (size_t limit) {
    size_t  size;

    size = postSize ();
    if (size == 0 || size > limit)
	return;
    queryString.reserve (size);
    queryString.resize (size);
    assert (&queryString[0] == queryString.data ());
    std::cin.read (&queryString[0], size);
    queryString.resize (std::cin.gcount ());
    if (queryString.length () == size) {
	parseURLEncoded (queryString);
    }
}

void  CGIForm::read_raw (size_t limit) {
    size_t  size;

    size = postSize ();
    if (size == 0 || size > limit)
	return;
    queryString.reserve (size);
    queryString.resize (size);
    assert (&queryString[0] == queryString.data ());
    std::cin.read (&queryString[0], size);
    queryString.resize (std::cin.gcount ());
    fix (queryString);
}

CGIForm::method_type  CGIForm::methodType () {
    requestMethod = getenvString (kREQUEST_METHOD);

    if (requestMethod.size () == 0)
	return M_NONE;
    else if (match (requestMethod, CharConst (kMETHOD_GET)))
	return M_GET;
    else if (match (requestMethod, CharConst (kMETHOD_HEAD)))
	return M_HEAD;
    else if (match (requestMethod, CharConst (kMETHOD_POST)))
	return M_POST;
    else if (match (requestMethod, CharConst (kMETHOD_OPTIONS)))
	return M_OPTIONS;
    else if (match (requestMethod, CharConst (kMETHOD_PUT)))
	return M_PUT;
    else if (match (requestMethod, CharConst (kMETHOD_DELETE)))
	return M_DELETE;
    else
	return M_OTHER;
}

void  CGIForm::checkContentType () {
    ustring  env = getenvString (kCONTENT_TYPE);
    uiterator  b = env.begin ();
    uiterator  e = env.end ();

    if (matchHead (b, e, CharConst (kMIME_URLENCODED))) {
	contentType = T_URLENCODED;
    } else if (matchSkip (b, e, CharConst (kMIME_FORMDATA))) {
	umatch  m;
	static uregex  re ("^\\s*;\\s*boundary=\"?([ '()+,./0-9:=?A-Z_a-z-]*['()+,./0-9:=?A-Z_a-z-])");

	if (usearch (b, e, m, re)) {
	    boundary = m[1];
	    contentType = T_MULTIPART;
	} else {		// error: no boundary
	    contentType = T_NONE;
	}
    } else if (matchHead (b, e, CharConst (kMIME_XML))) {
	contentType = T_XML;
    } else if (matchHead (b, e, CharConst (kMIME_JSON))) {
	contentType = T_JSON;
    } else {
	contentType = T_NONE;
    }
}

size_t  CGIForm::postSize () {
    char*  e = getenv (kCONTENT_LENGTH);

    if (e == NULL)
	return 0;
    return boost::lexical_cast <size_t> (e);
}

int  CGIForm::at (const ustring& name, ustring& ans) {
    return at (name, 0, ans);
}

int  CGIForm::at (const ustring& name, size_t i, ustring& ans) {
    return at (iarg, name, i, ans);
}

size_t  CGIForm::atSize (const ustring& name) {
    return atSize (iarg, name);
}

int  CGIForm::at (map_t& mp, const ustring& name, size_t i, ustring& ans) {
    map_t::iterator  it;
    int  idx = -1;

    it = mp.find (name);
    if (it == mp.end ()) {
	ans = uEmpty;
    } else {
	if (it->second < index.size ()) {
	    std::vector<int>*  v = &index [it->second];
	    if (v->size () > i) {
		idx = (*v)[i];
		ans = values[idx];
	    } else {
		ans = uEmpty;
	    }
	} else {
	    ans = uEmpty;
	}
    }
    return idx;
}

size_t  CGIForm::atSize (map_t& mp, const ustring& name) {
    map_t::iterator  it;

    it = mp.find (name);
    if (it == mp.end ()) {
    } else {
	if (it->second < index.size ()) {
	    std::vector<int>*  v = &index [it->second];
	    return v->size ();
	}
    }
    return 0;
}

int  CGIForm::insert (map_t& mp, const ustring& name, const ustring& value) {
    int  ans = -1;

#ifdef STRICT_FORMVAR
    if (matchName (name))
#else
    if (name.length () > 0 && name.length () < 64)
#endif
	{
	    map_t::iterator  it = mp.find (name);
	    std::vector<int>*  v;
	    
	    if (it == mp.end ()) {
		v = insertName (mp, name);
	    } else {
		v = &index.at (it->second);
	    }
	    if (v) {
		ans = values.size ();
		values.push_back (value);
		v->push_back (ans);
	    }
	}
    return ans;
}

#ifdef DEBUG2
void  CGIForm::dump (std::ostream& o) {
    map_t::iterator  it;
    std::vector<int>*  t;
    std::vector<int>::iterator  iu;
    int  c;

    for (it = iarg.begin (); it != iarg.end (); it ++) {
	o << it->first << ":\t";
	t = &index.at (it->second);
	if (t->size () == 0) {
	    o << "(empty)\n";
	} else {
	    c = 0;
	    for (iu = t->begin (); iu != t->end (); iu ++) {
		if (c == 0) {
		    o << values.at (*iu) << "\n";
		} else {
		    o << std::string ((it->first.size () + 1) / 8 + 1 , '\t') << values.at (*iu) << "\n";
		}
		c ++;
	    }
	}
    }
}
#endif /* DEBUG */

void  CGIForm::insertNameValue (map_t& mp, uiterator& b, uiterator& e) {
    uiterator  it;

    if (b == e)
	return;

    for (it = b; it != e; it ++) {
	if (*it == '=') {
	    ustring  name (b, it);
	    ustring  value (it + 1, e);

	    decode (name);
	    decode (value);
	    insert (mp, name, value);
	    return;
	}
    }
    {
	ustring  name (b, e);
	ustring  value;

	decode (name);
	insert (mp, name, value);
    }
}

std::vector<int>*  CGIForm::insertName (map_t& mp, const ustring& name) {
    std::vector<int>*  ans = new std::vector<int>;
    index.push_back (ans);
    mp.insert (map_t::value_type (name, index.size () - 1));
    return ans;
}

