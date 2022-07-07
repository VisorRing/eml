#include "wikimotor.h"
#include "motorconst.h"
#include "wikienv.h"
#include "wikiformat.h"
#include "ml.h"
#include "expr.h"
#include "motor.h"
#include "utf8.h"
#include "util_check.h"
#include "util_string.h"
#include "ustring.h"
#include <boost/ptr_container/ptr_vector.hpp>
#include <vector>
#include <strings.h>

/* ============================================================ */
uregex  re_wiki1 (
	"("					// 1	-- {NAME
		"\\{"
		"(\\+?" rWIKIVAR ")"		// 2
		"("				// 3
			"([/~,])?"		// 4
			"\\}"
		"|"
			"(:|!?\\?|\\|\\||->)"	// 5
		")"
	")|"
		"([/~,]?\\})"			// 6    -- }  /}  ~}  ,}  -- TMATCH_CKET, TMATCH_VAROPT
	"|"
		"(\\|\\|\\?)"			// 7	-- ||?  -- TMATCH_BAR2ELSE
	"|"
		"(\\|\\|)"			// 8	-- ||  -- TMATCH_BAR2
	"|"
		"&"				//  	-- &
		"("				// 9
			";"
		"|"
			"([^;]);"		// 10
		"|"
			"([a-zA-Z][a-zA-Z0-9_]*);"	// 11
		"|"
			"#([0-9]{1,5});"		// 12
		"|"
			"#[xX]([0-9a-h]{1,4});"		// 13
		")"
	"|"
		"(:)"				// 14  -- :  -- TMATCH_COLN
	"|"
		"(<br>|<zwsp>)"			// 15  -- <br>  <zwsp>
	"|"
		"(''+)"				// 16  -- ''  -- TMATCH_QUOT
	"|"
		"\\\\(.)"			// 17  -- \*
	"|"
		"(!?\\?)"			// 18 -- ?  !?  -- TMATCH_SELECT
	);
static const int  re_wiki1_open2 = 1;
static const int  re_wiki1_wikivar = 2;
static const int  re_wiki1_wikivar_opt = 4;
static const int  re_wiki1_varsep = 5;
static const int  re_wiki1_close2 = 6;
static const int  re_wiki1_bar2else = 7;
static const int  re_wiki1_bar2 = 8;
static const int  re_wiki1_amp = 9;
static const int  re_wiki1_amp_char = 10;
static const int  re_wiki1_amp_word = 11;
static const int  re_wiki1_amp_num = 12;
static const int  re_wiki1_amp_hex = 13;
static const int  re_wiki1_colon = 14;
//static const int  re_wiki1_spc = 15;
static const int  re_wiki1_br = 15;
static const int  re_wiki1_quote = 16;
static const int  re_wiki1_bsl = 17;
static const int  re_wiki1_wikivar_sel = 18;
/* ============================================================ */
void  WikiMotorObjVec::eval (WikiMotorObjVec& ans, WikiFormat* wiki) {
    const_iterator  b = begin ();
    const_iterator  e = end ();
    for (; b < e; b ++) {
	(*b)->eval (ans, &*b, wiki);
    }
}

void  WikiMotorObjVec::out (MotorOutputString& o, WikiFormat* wiki) {
    const_iterator  b = begin ();
    const_iterator  e = end ();

    for (; b < e; b ++) {
	o.out_raw ((*b)->textOut (wiki));
    }
}

ustring  WikiMotorObjVec::textOut (WikiFormat* wiki) {
    MotorOutputString  o;
    out (o, wiki);
    return o.ans;
}

MNode*  WikiMotorObjVec::toMNode (WikiFormat* wiki) {
    MNodePtr  e;
    MNodePtr  t;
    int  i = 0;
    
    while (1) {
	if (i >= size ())
	    return NULL;
	e = operator [] (i)->toMNode (wiki);
	i ++;
	if (! isNil (e ()))
	    break;
    }
    while (1) {
	if (i >= size ())
	    return wiki->mlenv->retval = e.release ();
	t = operator [] (i)->toMNode (wiki);
	i ++;
	if (! isNil (t ())) {
	    e = newMNode_str (new ustring (to_string (e ())));
	    e ()->value_str ()->append (to_string (t ()));
	    break;
	}
    }
    for (; i < size (); i ++) {
	t = operator [] (i)->toMNode (wiki);
	e ()->value_str ()->append (to_string (t ()));
    }
    return wiki->mlenv->retval = e.release ();
}

void  WikiMotorObjVec::out_HTML (MotorOutputString& o, WikiFormat* wiki) {
    const_iterator  b = begin ();
    const_iterator  e = end ();
    for (; b < e; b ++) {
	o.out_raw ((*b)->htmlOut (wiki));
    }
}

ustring  WikiMotorObjVec::htmlOut (WikiFormat* wiki) {	// output HTML text with an internal charcter code.
    MotorOutputString  o;

    out_HTML (o, wiki);
    return o.ans;
}

ustring  WikiMotorObjVec::dump () {
    const_iterator  b = begin ();
    const_iterator  e = end ();
    ustring  ans;

    for (; b < e; b ++) {
	ans.append ((*b)->dump ());
    }
    return ans;
}

void  WikiMotorObjVec::trimSpc () {
    if (size () > 0) {
	WikiMotorObj*  a = operator [] (0).get ();
	if (a->type == WikiMotorObj::wiki_text) {
	    WikiMotorObjText*  t = WikiMotorObjText_type (a);
	    t->trimLeadingSpc ();
	}
	a = back ().get ();
	if (a->type == WikiMotorObj::wiki_text) {
	    WikiMotorObjText*  t = WikiMotorObjText_type (a);
	    t->trimTrailingSpc ();
	}
    }
}

bool  WikiMotorObjVec::match (const char* t, size_t s) {
    if (size () == 1) {
	WikiMotorObj*  a = operator [] (0).get ();
	return a->match (t, s);
    }
    return false;
}

bool  WikiMotorObjVec::match (const char* t1, size_t s1, const char* t2, size_t s2) {
    if (size () == 1) {
	WikiMotorObj*  a = operator [] (0).get ();
	return a->match (t1, s1) || a->match (t2, s2);
    }
    return false;
}

bool  WikiMotorObjVec::match (const ustring& t) {
    if (size () == 1) {
	WikiMotorObj*  a = operator [] (0).get ();
	return *a == t;
    }
    return false;
}

bool  WikiMotorObjVec::match (const ustring& t1, const ustring& t2) {
    if (size () == 1) {
	WikiMotorObj*  a = operator [] (0).get ();
	return (*a == t1 || *a == t2);
    }
    return false;
}

bool  WikiMotorObjVec::matchHead (const char* t, size_t s) {
    if (size() > 0) {
	return operator [] (0).get ()->matchHead (t, s);
    }
    return false;
}

bool  WikiMotorObjVec::regmatch (const uregex& re) {
    if (size () == 1) {
	WikiMotorObj*  a = operator [] (0).get ();
	return a->regmatch (re);
    }
    return false;
}

void  WikiMotorObjVec::splitCharA (ustring::value_type ch, WikiMotorObjVecVec& out) {
    int  i;
    WikiMotorObjVecPtr  v;
    WikiMotorObjPtr  obj;
    WikiMotorObjText*  t;
    uiterator  b, e, m;

    if (size () == 0)
	return;
    v = WikiMotorObjVecPtr (new WikiMotorObjVec);
    out.push_back (v);
    for (i = 0; i < size (); i ++) {
	obj = operator [] (i);
	if (obj->type == WikiMotorObj::wiki_text) {
	    t = WikiMotorObjText_type (obj.get ());
	    b = t->text.begin ();
	    e = t->text.end ();
	    if ((m = find (b, e, ch)) != e) {
		if (b < m)
		    v->push_back (WikiMotorObjPtr (new WikiMotorObjText (b, m)));
		v = WikiMotorObjVecPtr (new WikiMotorObjVec);
		out.push_back (v);
		b = m + 1;
		while (b < e && (m = find (b, e, ch)) != e) {
		    if (b < m)
			v->push_back (WikiMotorObjPtr (new WikiMotorObjText (b, m)));
		    v = WikiMotorObjVecPtr (new WikiMotorObjVec);
		    out.push_back (v);
		    b = m + 1;
		}
		if (b < m)
		    v->push_back (WikiMotorObjPtr (new WikiMotorObjText (b, e)));
	    } else {
		v->push_back (obj);
	    }
	} else {
	    v->push_back (obj);
	}
    }
}

/*
  splitChar_keyword doesn't evaluate the wiki functions in the vector. 
*/
bool  WikiMotorObjVec::splitChar_keyword (ustring::value_type ch, ustring& name, WikiMotorObjVec& vvalue) {
    int  i;
    WikiMotorObjPtr  obj;
    WikiMotorObjText*  t;
    uiterator  b, e, m;

    for (i = 0; i < size (); i ++) {
	obj = operator [] (i);
	if (obj->type == WikiMotorObj::wiki_text) {
	    t = WikiMotorObjText_type (obj.get ());
	    b = t->text.begin ();
	    e = t->text.end ();
	    if ((m = find (b, e, ch)) != e) {
		if (b < m)
		    name.append (b, m);
		b = m + 1;
		if (b < e)
		    vvalue.push_back (WikiMotorObjPtr (t = new WikiMotorObjText (b, e)));
		for (i ++; i < size (); i ++) {
		    vvalue.push_back (operator [] (i));
		}
		return true;
	    } else {
		name.append (t->text);
	    }
	} else {
	    name.append (obj->dump ());
	}
    }

    return false;
}

ustring::value_type  WikiMotorObjVec::splitChar_keyword (ustring::value_type ch1, ustring::value_type ch2, ustring& name, WikiMotorObjVec& vvalue) {
    int  i;
    WikiMotorObjPtr  obj;
    WikiMotorObjText*  t;
    uiterator  b, e, m;
    ustring::value_type  ans;

    for (i = 0; i < size (); i ++) {
	obj = operator [] (i);
	if (obj->type == WikiMotorObj::wiki_text) {
	    t = WikiMotorObjText_type (obj.get ());
	    b = t->text.begin ();
	    e = t->text.end ();
	    for (m = b; b < e; b ++) {
		ans = *m;
		if (ans == ch1 || ans == ch2)
		    goto Bp1;
	    }
	    name.append (t->text);
	} else {
	    name.append (obj->dump ());
	}
    }
    return 0;

 Bp1:;
    if (b < m)
	name.append (b, m);
    b = m + 1;
    if (b < e)
	vvalue.push_back (WikiMotorObjPtr (t = new WikiMotorObjText (b, e)));
    for (i ++; i < size (); i ++) {
	vvalue.push_back (operator [] (i));
    }
    return ans;
}

bool  WikiMotorObjVec::splitChar_keyword (WikiFormat* wiki, ustring::value_type ch, ustring& name, ustring& value) {
    WikiMotorObjVec  v;
    bool  ans = splitChar_keyword (ch, name, v);
    value = v.textOut (wiki);
    return ans;
}

bool  WikiMotorObjVec::splitChar (WikiFormat* wiki, ustring::value_type ch, ustring& name, ustring& value) {
    WikiMotorObjVec  out1, out2;
    bool  ans;

    ans = splitChar (ch, out1, out2);
    name = out1.textOut (wiki);
    value = out2.textOut (wiki);
    return ans;
}

bool  WikiMotorObjVec::splitChar (char ch, WikiMotorObjVec& out1, WikiMotorObjVec& out2) {
    int  i;
    WikiMotorObjPtr  obj;
    WikiMotorObjText*  t;
    uiterator  b, e, m;

    for (i = 0; i < size (); i ++) {
	obj = operator [] (i);
	if (obj->type == WikiMotorObj::wiki_text) {
	    t = WikiMotorObjText_type (obj.get ());
	    b = t->text.begin ();
	    e = t->text.end ();
	    if ((m = find (b, e, ch)) != e) {
		if (b < m)
		    out1.push_back (WikiMotorObjPtr (new WikiMotorObjText (b, m)));
		b = m + 1;
		if (b < e)
		    out2.push_back (WikiMotorObjPtr (new WikiMotorObjText (b, e)));
		for (i ++; i < size (); i ++) {
		    out2.push_back (operator [] (i));
		}
		return true;
	    } else {
		out1.push_back (obj);
	    }
	} else {
	    out1.push_back (obj);
	}
    }
    return false;
}

bool  WikiMotorObjVec::splitURL (WikiFormat* wiki, ustring& proto, ustring& host, ustring& path, ustring& params, ustring& anchor) {
    if (size () > 0) {
	WikiMotorObj*  o = operator [] (0).get ();
	if (o->type == WikiMotorObj::wiki_text) {
	    WikiMotorObjText*  t = WikiMotorObjText_type (o);
	    uiterator  b = t->text.begin ();
	    uiterator  e = t->text.end ();
	    if (::matchSkip (b, e, CharConst ("http://"))) {
		proto.assign (CharConst ("http"));
	    } else if (::matchSkip (b, e, CharConst ("https://"))) {
		proto.assign (CharConst ("https"));
	    } else {
		return false;
	    }
	    {
		WikiMotorObjVec  vec;
		WikiMotorObjVec  vhost, vpath;
		if (b < e)
		    vec.push_back (WikiMotorObjPtr (new WikiMotorObjText (b, e)));
		for (int i = 1; i < size (); i ++) {
		    vec.push_back (operator [] (i));
		}
		vpath.push_back (WikiMotorObjPtr (new WikiMotorObjText (uSlash)));
		vec.splitChar ('/', vhost, vpath);
		host = vhost.textOut (wiki);
		if (! matchHostname (host)) {
		    wiki->errorMsg.append (host).append (CharConst (": bad hostname.\n"));
		    host = uEmpty;
		}
		return (vpath.splitURLPath (wiki, path, params, anchor));
	    }
	} else {
	    return false;
	}
    }
    return false;
}

bool  WikiMotorObjVec::splitURL (WikiFormat* wiki, ustring& url) {
    ustring  proto, host, path, params, anchor;

    if (splitURL (wiki, proto, host, path, params, anchor)) {
	if (matchHostname (host)) {
	    url.assign (proto).append (CharConst ("://")).append (host).append (path);
	    if (params.length () > 0)
		url.append (CharConst ("?")).append (params);
	    if (anchor.length () > 0)
		url.append (anchor);
	    return true;
	} else {
	    return false;
	}
    } else {
	return false;
    }
}

// "[proto]:" part is excluded.
bool  WikiMotorObjVec::splitURL_2 (WikiFormat* wiki, ustring& host, ustring& path, ustring& params, ustring& anchor) {
    if (size () > 0) {
	WikiMotorObj*  o;
	o = operator [] (0).get ();
	if (o->matchHead (CharConst ("//"))) {
	    WikiMotorObjText*  t;
	    WikiMotorObjVec  vhost, vpath;
	    WikiMotorObjVec  vec;
	    bool  fsp;

	    t = WikiMotorObjText_type (o);
	    if (t->text.begin () + 2 < t->text.end ())
		vec.push_back (WikiMotorObjPtr (new WikiMotorObjText (t->text.begin () + 2, t->text.end ())));
	    for (int i = 1; i < size (); i ++) {
		vec.push_back (operator [] (i));
	    }
	    vpath.push_back (WikiMotorObjPtr (new WikiMotorObjText (uSlash)));
	    fsp = vec.splitChar ('/', vhost, vpath);
	    host = vhost.textOut (wiki);
	    if (! matchHostname (host)) {
		wiki->errorMsg.append (host).append (CharConst (": bad hostname.\n"));
		host = uEmpty;
	    }
	    if (fsp) {
		return (vpath.splitURLPath (wiki, path, params, anchor));
	    } else {
		return true;
	    }
	}
    }
    return false;
}

bool  WikiMotorObjVec::splitURL_2 (WikiFormat* wiki, const ustring& proto, ustring& url) {
    ustring  host, path, params, anchor;

    if (splitURL_2 (wiki, host, path, params, anchor)) {
	if (matchHostname (host)) {
	    url.assign (proto).append (CharConst ("://")).append (host).append (path);
	    if (params.length () > 0)
		url.append (CharConst ("?")).append (params);
	    if (anchor.length () > 0)
		url.append (anchor);
	    return true;
	} else {
	    return false;
	}
    } else {
	return false;
    }
}

bool  WikiMotorObjVec::splitURL_3 (WikiFormat* wiki, ustring& port, ustring& path, ustring& params, ustring& anchor) {
    WikiMotorObjVec  vport;
    WikiMotorObjVec  vpath;

    vpath.push_back (WikiMotorObjPtr (new WikiMotorObjText (uSlash)));
    if (splitChar ('/', vport, vpath)) {
	ustring  v = vport.textOut (wiki);
	if (matchNum (v)) {
	    int n = strtoul (v);
	    if (1 <= n && n < 65536) {
		port = v;
		return vpath.splitURLPath (wiki, path, params, anchor);
	    }
	}
    }
    return false;
}

bool  WikiMotorObjVec::splitURL_3 (WikiFormat* wiki, const ustring& protohost, ustring& url) {
    ustring  port;
    ustring  path;
    ustring  params;
    ustring  anchor;

    if (splitURL_3 (wiki, port, path, params, anchor)) {
	url = protohost;
	url.append (uColon).append (port).append (path);
	if (params.length () > 0)
	    url.append (CharConst ("?")).append (params);
	if (anchor.length () > 0)
	    url.append (anchor);
	return true;
    }
    return false;
}

class  AutoIpMode {
public:
    WikiFormat::ipMode_t  back;
    WikiFormat*  wiki;

    AutoIpMode (WikiFormat* _wiki, WikiFormat::ipMode_t _new) {
	wiki = _wiki;
	back = wiki->ipMode;
	wiki->ipMode = _new;
    };
    virtual  ~AutoIpMode () {
	wiki->ipMode = back;
    };
};

bool  WikiMotorObjVec::splitURLPath (WikiFormat* wiki, ustring& path, ustring& params, ustring& anchor) {
    WikiMotorObjVec  vpath, vparams, vanchor;
    WikiMotorObjVec  vpath2, vparams2;
    WikiMotorObjVecVec  names;
    bool  fparams, fanchor;
    int  i;
    int  n;
    ustring  u;
    AutoIpMode  autoObj (wiki, WikiFormat::M_URL);

    fparams = splitChar ('?', vpath, vparams);
    if (fparams) {
	fanchor = vparams.splitChar ('#', vparams2, vanchor);
	vparams = vparams2;
    } else {
	fanchor = vpath.splitChar ('#', vpath2, vanchor);
	vpath = vpath2;
    }
    vpath.splitCharA ('/', names);
    n = names.size () - 1;
    for (i = 0; i <= n; i ++) {
	u = percentEncode (names[i]->textOut (wiki));
	if (i > 0)
	    path.append (uSlash);
	path.append (u);
    }

    if (fparams) {
	WikiMotorObjVecVec  namevals;
	ustring  name, value;
	vparams.splitCharA ('&', namevals);
	for (i = 0; i < namevals.size (); i ++) {
	    if (i > 0)
		params.append (uAmp);
	    if (namevals[i].get()->splitChar (wiki, '=', name, value)) {
		params.append (percentEncode (name)).append (uEq).append (percentEncode (value));
	    } else {
		params.append (percentEncode (name));
	    }
	}
    }

    if (fanchor) {
	anchor = ustring (CharConst ("#")).append (percentEncode (vanchor.textOut (wiki)));
    }

    return true;
}

bool  WikiMotorObjVec::splitURLPath (WikiFormat* wiki, ustring& url) {
    ustring  path, params, anchor;

    if (splitURLPath (wiki, path, params, anchor)) {
	url = path;
	if (params.length () > 0)
	    url.append (CharConst ("?")).append (params);
	if (anchor.length () > 0)
	    url.append (anchor);
	return true;
    } else {
	return false;
    }
}

/* ============================================================ */
void  WikiMotorObjVecVec::eval (WikiMotorObjVec& ans, WikiFormat* wiki) {
    for (int i = 0; i < size ();i ++) {
	operator [] (i)->eval (ans, wiki);
    }
}

void  WikiMotorObjVecVec::out (MotorOutputString& o, WikiFormat* wiki) {
    for (int i = 0; i < size ();i ++) {
	operator [] (i)->out (o, wiki);
    }
}

ustring  WikiMotorObjVecVec::textOut (WikiFormat* wiki) {
    MotorOutputString  o;

    out (o, wiki);
    return o.ans;
}

#if 0
MNode*  WikiMotorObjVecVec::toMNode (WikiFormat* wiki) {
}
#endif

void  WikiMotorObjVecVec::out_HTML (MotorOutputString& o, WikiFormat* wiki) {
    for (int i = 0; i < size (); i ++) {
	operator [] (i)->out_HTML (o, wiki);
    }
}

ustring  WikiMotorObjVecVec::htmlOut (WikiFormat* wiki) {
    MotorOutputString  o;

    out (o, wiki);
    return o.ans;
}

ustring  WikiMotorObjVecVec::dump () {
    ustring  ans;
    for (int i = 0; i < size (); i ++) {
	ans.append (operator [] (i)->dump ());
    }
    return ans;
}

void  WikiMotorObjVecVec::join (size_t start, const ustring& ch, WikiMotorObjVec& ans) {
    ::join (begin (), end (), ch, ans);
}

/* ============================================================ */
MotorOutput*  MotorOutputWiki::outamp (const ustring& t) {
    ans->push_back (WikiMotorObjPtr (new WikiMotorObjText (t)));
    return this;
}

MotorOutput*  MotorOutputWiki::outamp_br (const ustring& t) {
    uiterator  b = t.begin ();
    uiterator  e = t.end ();
    uiterator  m;

    while (splitChar (b, e, '\n', m)) {
	if (b != m)
	    ans->push_back (WikiMotorObjPtr (new WikiMotorObjText (ustring (b, m))));
	ans->push_back (WikiMotorObjPtr (new WikiMotorObjBR ()));
	b = m + 1;
    }
    if (b != m)
	ans->push_back (WikiMotorObjPtr (new WikiMotorObjText (ustring (b, m))));
    return this;
}

#if 0
MotorOutput*  MotorOutputWiki::outamp_wbr (const ustring& t) {
//    ans->push_back (WikiMotorObjPtr (new WikiMotorObjHtml (o.ans)));
    MotorOutput::outamp_wbr (t);
    return this;
}
#endif

MotorOutput*  MotorOutputWiki::outamp_nw (const ustring& t) {
    if (t.size () > 0) {
	ans->push_back (WikiMotorObjPtr (new WikiMotorObjText (t)));
    } else {
	ans->push_back (WikiMotorObjPtr (new WikiMotorObjHtml (uNbsp)));
    }
    return this;
}

MotorOutput*  MotorOutputWiki::outamp_c3 (const ustring& t) {
    ans->push_back (WikiMotorObjPtr (new WikiMotorObjText (c3 (t))));
    return this;
}

MotorOutput*  MotorOutputWiki::outamp (uiterator b, uiterator e, uregex* re) {
    assert (0);
    return this;
}

MotorOutput*  MotorOutputWiki::out (const ustring::value_type* s, size_t len) {
    ans->push_back (WikiMotorObjPtr (new WikiMotorObjText (ustring (s, len))));
    return this;
}

/* ============================================================ */
ustring  WikiMotorObj::htmlOut (WikiFormat* wiki) {
    MotorOutputString  o;
    o.outamp (textOut (wiki));
    return o.ans;
}

/* ============================================================ */
ustring  WikiMotorObjText::textOut (WikiFormat* wiki) {
    return text;
}

ustring  WikiMotorObjText::dump () {
    return text;
}

void  WikiMotorObjText::trimLeadingSpc () {
    uiterator  b = text.begin ();
    uiterator  e = text.end ();
    if (*b == ' ') {
	++ b;
	for (; b < e; ++ b) {
	    if (*b != ' ')
		break;
	}
	text = ustring (b, e);
    }
}

void  WikiMotorObjText::trimTrailingSpc () {
    uiterator  b = text.begin ();
    uiterator  e = text.end () - 1;
    if (*e == ' ') {
	-- e;
	for (; b <= e; -- e) {
	    if (*e != ' ')
		break;
	}
	text = ustring (b, e + 1);
    }
}

/* ============================================================ */
ustring  WikiMotorObjChar::textOut (WikiFormat* wiki) {
    return text;
}

ustring  WikiMotorObjChar::htmlOut (WikiFormat* wiki) {
    if (text.length () > 1) {
	return text;
    } else {
	return WikiMotorObj::htmlOut (wiki);
    }
}

ustring  WikiMotorObjChar::dump () {
    return text;
}

/* ============================================================ */
ustring  WikiMotorObjEmph::textOut (WikiFormat* wiki) {
    ustring  ans;

    ans.assign (level, '\'').append (text.textOut (wiki)).append (level, '\'');
    return ans;
}

ustring  WikiMotorObjEmph::htmlOut (WikiFormat* wiki) {
    ustring  ans;

    switch (level) {
    case 2:
	ans.assign (CharConst ("<i>")).append (text.htmlOut (wiki)).append (CharConst ("</i>"));
	break;
    case 3:
	ans.assign (CharConst ("<b>")).append (text.htmlOut (wiki)).append (CharConst ("</b>"));
	break;
    case 5:
	ans.assign (CharConst ("<b><i>")).append (text.htmlOut (wiki)).append (CharConst ("</i></b>"));
	break;
    default:;
	assert (0);
    }
    return ans;
}

ustring  WikiMotorObjEmph::dump () {
    ustring  ans;

    ans.assign (level, '\'').append (text.dump ()).assign (level, '\'');
    return ans;
}

/* ============================================================ */
ustring  WikiMotorObjBR::textOut (WikiFormat* wiki) {
    return ustring (CharConst ("<br>"));
}

ustring  WikiMotorObjBR::htmlOut (WikiFormat* wiki) {
    return ustring (CharConst ("<br />"));
}

ustring  WikiMotorObjBR::dump () {
    return ustring (CharConst ("<br>"));
}

/* ============================================================ */
ustring  WikiMotorObjZWSP::textOut (WikiFormat* wiki) {
    return ustring (CharConst ("<zwsp>"));
}

ustring  WikiMotorObjZWSP::htmlOut (WikiFormat* wiki) {
    return ustring (CharConst ("&#8203;"));
}

ustring  WikiMotorObjZWSP::dump () {
    return ustring (CharConst ("<zwsp>"));
}

/* ============================================================ */
void  WikiMotorObjVar::eval (WikiMotorObjVec& ans, const WikiMotorObjPtr* self, WikiFormat* wiki) {
    ustring  val = fixUTF8 (wiki->getVar_string (name));
    MotorOutputWiki  o (ans);

    switch (vopt) {
    case opt_normal:
	o.outamp (val);
	break;
    case opt_br:
	o.outamp_br (val);
	break;
    case opt_nbsp:
	o.outamp_nw (val);
	break;
    case opt_c3:
	o.outamp_c3 (val);
	break;
#if 0
    case opt_wbr:
	o.outamp_wbr (val);
	break;
#endif
    default:
	assert (0);
    }
}

ustring  WikiMotorObjVar::textOut (WikiFormat* wiki) {
    return fixUTF8 (wiki->getVar_string (name));
}

ustring  WikiMotorObjVar::htmlOut (WikiFormat* wiki) {
    ustring  val = fixUTF8 (wiki->getVar_string (name));
    MotorOutputString  o;

    switch (vopt) {
    case opt_normal:
	o.outamp (val);
	return o.ans;
    case opt_br:
	o.outamp_br (val);
	return o.ans;
    case opt_nbsp:
	o.outamp_nw (val);
	return o.ans;
    case opt_c3:
	o.outamp_c3 (val);
	return o.ans;
#if 0
    case opt_wbr:
	o.outamp_wbr (val);
	return o.ans;
#endif
    default:
	assert (0);
    }
}

MNode*  WikiMotorObjVar::toMNode (WikiFormat* wiki) {
    return wiki->mlenv->getVar (name);
}

ustring  WikiMotorObjVar::dump () {
    ustring  ans;

    ans.append (CharConst ("{")).append (name);
    switch (vopt) {
    case opt_normal:
	ans.append (CharConst ("}"));
	break;
    case opt_br:
	ans.append (CharConst ("/}"));
	break;
    case opt_nbsp:
	ans.append (CharConst ("~}"));
	break;
    case opt_c3:
	ans.append (CharConst (",}"));
	break;
#if 0
    case opt_wbr:
	ans.append (CharConst ("^}"));
	break;
#endif
    default:
	assert (0);
    }
    return ans;
}

/* ============================================================ */
void  WikiMotorObjTab::eval (WikiMotorObjVec& ans, const WikiMotorObjPtr* self, WikiFormat* wiki) {
    MotorOutputWiki  o (ans);
    switch (vopt) {
    case opt_normal:
	o.outamp (textOut (wiki));
	break;
    case opt_br:
	o.outamp_br (textOut (wiki));
	break;
    case opt_nbsp:
	o.outamp_nw (textOut (wiki));
	break;
    case opt_c3:
	o.outamp_c3 (textOut (wiki));
	break;
    default:
	assert (0);
    }
}

ustring  WikiMotorObjTab::textOut (WikiFormat* wiki) {
    return fixUTF8 (to_string (toMNode (wiki)));
}

ustring  WikiMotorObjTab::htmlOut (WikiFormat* wiki) {
    MotorOutputString  o;
    switch (vopt) {
    case opt_normal:
	o.outamp (textOut (wiki));
	return o.ans;
    case opt_br:
	o.outamp_br (textOut (wiki));
	return o.ans;
    case opt_nbsp:
	o.outamp_nw (textOut (wiki));
	return o.ans;
    case opt_c3:
	o.outamp_c3 (textOut (wiki));
	return o.ans;
    default:
	assert (0);
    }
}

MNode*  WikiMotorObjTab::toMNode (WikiFormat* wiki) {
    MNode*  obj = wiki->mlenv->getVar (name);
    for (int i = 0; i < arg.size (); i ++) {
	ustring  key = arg[i]->textOut (wiki);
	if (isTable (obj)) {
	    obj = obj->tableGet (key);
	} else if (isVector (obj)) {
	    obj = obj->vectorGet (::to_int64 (key));
	} else {
	    return NULL;
	}
    }
    return obj;
}

ustring  WikiMotorObjTab::dump () {
    ustring  ans;
    ans.append (CharConst ("{")).append (name);
    for (int i = 0; i < arg.size (); i ++) {
	ans.append (uColon).append (arg[i]->dump ());
    }
    ans.append (CharConst ("}"));
    return ans;
}

/* ============================================================ */
ustring  WikiMotorObjFunc::execDefun (MNode* vargs, WikiFormat* wiki) {
    MNodePtr  node;
    try {
	// XXX: パラメータのないファンクションでも空のパラメータを書かなければならない -- {function:}
	if (isNil (mfunc ()->lambdaParams ())		// 引数がnil
	    && isNil (vargs->cdr ())
	    && isNil (vargs->car ())) {
	    node = ::execDefun (wiki->env->mlenv, mfunc ()->lambdaParams (), mfunc ()->lambdaBody (), NULL, name);
	} else {
	    if (checkDefunArgs (mfunc (), vargs)) {
		node = ::execDefun (wiki->env->mlenv, mfunc ()->lambdaParams (), mfunc ()->lambdaBody (), vargs, name);
	    } else {
		wiki->errorMsg.append (name).append (CharConst (": ")).append (uErrorWrongNumber).append (uLF);
	    }
	}
    } catch (ustring& msg) {
	wiki->logLispFunctionError (msg, name);
    }
    if (isStr (node ())) {
	return to_string (node ());
    } else {
	return uEmpty;
    }
}

void  WikiMotorObjFunc::evalFVal (const ustring& text, WikiMotorObjVec& ans, WikiFormat* wiki) {
    if (mfunc ()->lambdaModeBit (MNode::MODE_WIKIRAW)) {
	ans.push_back (WikiMotorObjPtr (new WikiMotorObjHtml (text)));
    } else if (mfunc ()-> lambdaModeBit (MNode::MODE_WIKIOUT)) {
	wiki->wikiMotor (text.begin (), text.end (), ans);
    } else if (mfunc ()-> lambdaModeBit (MNode::MODE_TEXTOUT)) {
	ans.push_back (WikiMotorObjPtr (new WikiMotorObjText (text)));
    }
}

ustring  WikiMotorObjFunc::textOut (WikiFormat* wiki) {
    WikiFormat::DepthCount  dc (wiki);
    WikiMotorObjVec  o;

    if (dc.test ()) {
	wiki->errorMsg.append (uErrorInclNest);
	return uEmpty;
    }
    eval (o, NULL, wiki);
    return o.textOut (wiki);
}

ustring  WikiMotorObjFunc::htmlOut (WikiFormat* wiki) {
    WikiFormat::DepthCount  dc (wiki);
    WikiMotorObjVec  o;

    if (dc.test ()) {
	wiki->errorMsg.append (uErrorInclNest);
	return uEmpty;
    }
    eval (o, NULL, wiki);
    return o.htmlOut (wiki);
}

MNode*  WikiMotorObjFunc::toMNode (WikiFormat* wiki) {
    WikiFormat::DepthCount  dc (wiki);
    WikiMotorObjVec  o;

    if (dc.test ()) {
	wiki->errorMsg.append (uErrorInclNest);
	return NULL;
    }
    eval (o, NULL, wiki);
    return o.toMNode (wiki);
}

/* ============================================================ */
ustring  WikiMotorObjFunc1::execDefunArgs (WikiFormat* wiki) {
    MNodePtr  vargs;
    AutoInclCount  autoIncl (wiki->env->mlenv);

    autoIncl.inc ();
    vargs = buildArgs (arg.textOut (wiki));
    return execDefun (vargs (), wiki);
}

void  WikiMotorObjFunc1::eval (WikiMotorObjVec& ans, const WikiMotorObjPtr* self, WikiFormat* wiki) {
    if (func) {
	if (func->fn1 (&arg, ans, wiki)) {
	} else {
	    ans.push_back (WikiMotorObjPtr (new WikiMotorObjText (dump ())));
	}
    } else {
	ustring  text = execDefunArgs (wiki);
	evalFVal (text, ans, wiki);
    }
}

ustring  WikiMotorObjFunc1::dump () {
    ustring  ans;
    ans.append (CharConst ("{")).append (name).append (uColon).append (arg.dump ()).append (CharConst ("}"));
    return ans;
}

/* ============================================================ */
ustring  WikiMotorObjFuncM::execDefunArgs (WikiFormat* wiki) {
    MNodePtr  vargs;
    AutoInclCount  autoIncl (wiki->env->mlenv);

    autoIncl.inc ();
    vargs = wiki->buildArgs (mfunc ()->lambdaModeBit (MNode::MODE_WIKINODEC), arg.begin (), arg.end ());
    return execDefun (vargs (), wiki);
}

void  WikiMotorObjFuncM::eval (WikiMotorObjVec& ans, const WikiMotorObjPtr* self, WikiFormat* wiki) {
    if (func) {
	if (func->fnM (&arg, ans, wiki)) {
	} else {
	    // error
	    ans.push_back (WikiMotorObjPtr (new WikiMotorObjText (dump ())));
	}
    } else {
	ustring  text = execDefunArgs (wiki);
	evalFVal (text, ans, wiki);
    }
}

ustring  WikiMotorObjFuncM::dump () {
    ustring  ans;
    ans.append (CharConst ("{")).append (name);
    for (int i = 0; i < arg.size (); i ++) {
	ans.append (uColon).append (arg[i]->dump ());
    }
    ans.append (CharConst ("}"));
    return ans;
}

/* ============================================================ */
ustring  WikiMotorObjFuncM2::execDefunArgs (WikiFormat* wiki) {
    MNodePtr  vargs;
    AutoInclCount  autoIncl (wiki->env->mlenv);

    autoIncl.inc ();
//    vargs = new MNode;
//    vargs ()->set_car (newMNode_str (new ustring (arg2.dump ())));
//    vargs ()->set_car (newMNode_str (new ustring (arg2.textOut (wiki))));
//    vargs ()->set_cdr (wiki->buildArgs (arg.begin (), arg.end ()));
    MNode*  margs = wiki->buildArgs (mfunc ()->lambdaModeBit (MNode::MODE_WIKINODEC), arg.begin (), arg.end ());
    MNode*  marg2;
    if (mfunc ()->lambdaModeBit (MNode::MODE_WIKINODEC2)) {
	marg2 = newMNode_str (new ustring (arg2.dump ()));
    } else {
	marg2 = newMNode_str (new ustring (arg2.textOut (wiki)));
    }
    vargs = newMNode_cons (marg2, margs);
    return execDefun (vargs (), wiki);
}

void  WikiMotorObjFuncM2::eval (WikiMotorObjVec& ans, const WikiMotorObjPtr* self, WikiFormat* wiki) {
    if (func) {
	if (func->fnM2 (&arg, &arg2, ans, wiki)) {
	} else {
	    // error
	    ans.push_back (WikiMotorObjPtr (new WikiMotorObjText (dump ())));
	}
    } else {
	ustring  text = execDefunArgs (wiki);
	evalFVal (text, ans, wiki);
    }
}

ustring  WikiMotorObjFuncM2::dump () {
    ustring  ans;
    ans.append (CharConst ("{")).append (name);
    for (int i = 0; i < arg.size (); i ++) {
	ans.append (uColon).append (arg[i]->dump ());
    }
    if (arg2.size () > 0) {
	ans.append (CharConst ("||")).append (arg2.dump ());
    }
    ans.append (CharConst ("}"));
    return ans;
}

/* ============================================================ */
ustring  WikiMotorObjFuncLink::execDefunArgs (WikiFormat* wiki) {
    MNodePtr  vargs;
    AutoInclCount  autoIncl (wiki->env->mlenv);

    autoIncl.inc ();
#ifdef DEBUG
//    std::cerr << "bit:" << mfunc ()->lambdaModeBit (MNode::MODE_WIKINODEC) << "\n";
#endif /* DEBUG */
    vargs = wiki->buildArgs (mfunc ()->lambdaModeBit (MNode::MODE_WIKINODEC), arg.begin (), arg.end ());
    return execDefun (vargs (), wiki);
}

void  WikiMotorObjFuncLink::eval (WikiMotorObjVec& ans, const WikiMotorObjPtr* self, WikiFormat* wiki) {
    ans.push_back (WikiMotorObjPtr (new WikiMotorObjText (dump ())));
}

ustring  WikiMotorObjFuncLink::dump () {
    ustring  ans;
    ans.append (CharConst ("{")).append (name);
    for (int i = 0; i < arg.size (); i ++) {
	ans.append (uColon).append (arg[i]->dump ());
    }
    ans.append (CharConst ("}"));
    return ans;
}

/* ============================================================ */
void  WikiMotorObjCond::eval (WikiMotorObjVec& ans, const WikiMotorObjPtr* self, WikiFormat* wiki) {
    ustring  v = var->textOut (wiki);
    ustring  val = value.textOut (wiki);
    bool  c = (v == val);
    WikiMotorObjVec*  o;

    if (c ^ fneg) {
	o = &trueobjs;
    } else {
	o = &falseobjs;
    }
    WikiMotorObjVec::iterator  b = o->begin ();
    WikiMotorObjVec::iterator  e = o->end ();
    for (; b < e; b ++) {
	ans.push_back ((*b));
    }
}

ustring  WikiMotorObjCond::textOut (WikiFormat* wiki) {
    ustring  v = var->textOut (wiki);
    ustring  val = value.textOut (wiki);
    bool  c = (v == val);

    if (c ^ fneg) {
	return trueobjs.textOut (wiki);
    } else {
	return falseobjs.textOut (wiki);
    }
}

ustring  WikiMotorObjCond::htmlOut (WikiFormat* wiki) {
    ustring  v = var->textOut (wiki);
    ustring  val = value.textOut (wiki);
    bool  c = (v == val);

    if (c ^ fneg) {
	return trueobjs.htmlOut (wiki);
    } else {
	return falseobjs.htmlOut (wiki);
    }
}

ustring  WikiMotorObjCond::dump () {
    ustring  ans;
    // ***XXX
    return ans;
}

/* ============================================================ */
ustring  WikiMotorObjMNode::textOut (WikiFormat* wiki) {
    return fixUTF8 (to_string (node ()));
}

ustring  WikiMotorObjMNode::dump () {
    return fixUTF8 (to_string (node ()));
}

/* ============================================================ */
void  WikiMotor::compile (WikiMotorObjVec& out, int tmatch) {
    /*
      tmatch:
	bit 1: terminate with '}'
	bit 2: terminate with ':'
	bit 3: terminate with SPC
	bit 4: terminate with ''
	bit 5: terminate with '|'
    */

    while (sp.nextSearch ()) {
//	std::cerr << ustring (sp.t, sp.u) << "\n";
	if (sp.match (re_wiki1_open2)) {	// {
	    compile_text (out);
	    compile_2 (out);
	} else if (sp.match (re_wiki1_close2)) { // }
	    uiterator  b = sp.matchBegin (re_wiki1_close2);
	    switch (*b) {
	    case '/':
	    case '~':
	    case ',':
		if (tmatch & TMATCH_VAROPT) {
		    compile_text (out);
		    return;
		} else if (tmatch & TMATCH_CKET) {
		    sp.t ++;
		    compile_text (out);
		    return;
		}
	    default:
		if (tmatch & TMATCH_CKET) {
		    compile_text (out);
		    return;
		}
	    }
	} else if (sp.match (re_wiki1_bar2else)) { // ||?
	    if (tmatch & TMATCH_BAR2ELSE) {
		compile_text (out);
		return;
	    }
	} else if (sp.match (re_wiki1_bar2)) { // ||
	    if (tmatch & TMATCH_BAR2) {
		compile_text (out);
		return;
	    }
	} else if (sp.match (re_wiki1_amp)) { // &
	    compile_text (out);
	    if (sp.match (re_wiki1_amp_char)) {	// &.;
		out.push_back (WikiMotorObjPtr (new WikiMotorObjChar (sp.matchBegin (re_wiki1_amp_char), sp.matchEnd (re_wiki1_amp_char))));
		sp.shiftCursor ();
	    } else if (sp.match (re_wiki1_amp_word)) { // &.+;
		out.push_back (WikiMotorObjPtr (new WikiMotorObjChar (sp.matchBegin (0), sp.matchEnd (0))));
		sp.shiftCursor ();
	    } else if (sp.match (re_wiki1_amp_num)) { // &#0000;
		out.push_back (WikiMotorObjPtr (new WikiMotorObjChar (sp.matchBegin (0), sp.matchEnd (0))));
		sp.shiftCursor ();
	    } else if (sp.match (re_wiki1_amp_hex)) { // &#x0000;
		out.push_back (WikiMotorObjPtr (new WikiMotorObjChar (sp.matchBegin (0), sp.matchEnd (0))));
		sp.shiftCursor ();
	    } else {		// &;
	    }
	} else if (sp.match (re_wiki1_colon)) { // :
	    if (tmatch & TMATCH_COLN) {
		compile_text (out);
		return;
	    }
	} else if (sp.match (re_wiki1_br)) { // <br>|<zwsp>
	    compile_text (out);
	    if (*(sp.matchBegin (re_wiki1_br) +1) == 'b')
		out.push_back (WikiMotorObjPtr (new WikiMotorObjBR ()));
	    else
		out.push_back (WikiMotorObjPtr (new WikiMotorObjZWSP ()));
	} else if (sp.match (re_wiki1_quote)) { // ''
	    compile_text (out);
	    if (tmatch & TMATCH_QUOT) {
		return;
	    } else {
		compile_5 (out, sp.matchEnd () - sp.matchBegin ());
	    }
	} else if (sp.match (re_wiki1_bsl)) {	// Backslash
	    compile_text (out);
	    out.push_back (WikiMotorObjPtr (new WikiMotorObjChar (sp.matchBegin (re_wiki1_bsl), sp.matchEnd (re_wiki1_bsl))));
	    sp.shiftCursor ();
	} else if (sp.match (re_wiki1_wikivar_sel)) {		//  ?  !?
	    if (tmatch & TMATCH_SELECT) {
		compile_text (out);
		return;
	    }
	} else {
	    assert (0);
	}
    }
    if (sp.begin () < sp.end ()) {
	out.push_back (WikiMotorObjPtr (new WikiMotorObjText (sp.begin (), sp.end ())));
	sp.shiftCursor ();
    }
}

void  WikiMotor::compile_text (WikiMotorObjVec& out) {
    if (sp.begin () < sp.end ()) {
	out.push_back (WikiMotorObjPtr (new WikiMotorObjText (sp.begin (), sp.end ())));
    }
    sp.shiftCursor ();
}

void  WikiMotor::compile_2 (WikiMotorObjVec& out) { // {
    if (sp.match (re_wiki1_varsep)) {		// : | !? | ?
	uiterator  b = sp.matchBegin (re_wiki1_varsep);
	switch (*b) {
	case ':':		// NAME:
	    compile_3 (out, ustring (sp.matchBegin (re_wiki1_wikivar), sp.matchEnd (re_wiki1_wikivar)));
	    break;
	case '?':		// NAME?
	    compile_6 (out, sp.matchBegin (re_wiki1_wikivar), sp.matchEnd (re_wiki1_wikivar), false);
	    break;
	case '!':		// NAME!?
	    compile_6 (out, sp.matchBegin (re_wiki1_wikivar), sp.matchEnd (re_wiki1_wikivar), true);
	    break;
	case '|':
	    compile_7 (out, ustring (sp.matchBegin (re_wiki1_wikivar), sp.matchEnd (re_wiki1_wikivar)));
	    break;
	case '-':		// NAME->
	    compile_8 (out, ustring (sp.matchBegin (re_wiki1_wikivar), sp.matchEnd (re_wiki1_wikivar)));
	    break;
	default:
	    assert (0);
	}
    } else if (sp.match (re_wiki1_wikivar_opt)) {	// / | ~ | , | ^
	uiterator  b = sp.matchBegin (re_wiki1_wikivar_opt);
	switch (*b) {
	case '/':
	    out.push_back (WikiMotorObjPtr (new WikiMotorObjVar (sp.matchBegin (re_wiki1_wikivar), sp.matchEnd (re_wiki1_wikivar), WikiMotorObjVar::opt_br)));
	    break;
	case '~':
	    out.push_back (WikiMotorObjPtr (new WikiMotorObjVar (sp.matchBegin (re_wiki1_wikivar), sp.matchEnd (re_wiki1_wikivar), WikiMotorObjVar::opt_nbsp)));
	    break;
	case ',':
	    out.push_back (WikiMotorObjPtr (new WikiMotorObjVar (sp.matchBegin (re_wiki1_wikivar), sp.matchEnd (re_wiki1_wikivar), WikiMotorObjVar::opt_c3)));
	    break;
#if 0
	case '^':
	    out.push_back (WikiMotorObjPtr (new WikiMotorObjVar (sp.matchBegin (re_wiki1_wikivar), sp.matchEnd (re_wiki1_wikivar), WikiMotorObjVar::opt_wbr)));
	    break;
#endif
	default:
	    assert (0);
	}
    } else {
	out.push_back (WikiMotorObjPtr (new WikiMotorObjVar (sp.matchBegin (re_wiki1_wikivar), sp.matchEnd (re_wiki1_wikivar), WikiMotorObjVar::opt_normal)));
    }
}

void  WikiMotor::compile_3 (WikiMotorObjVec& out, const ustring& name) {	// {NAME:
    WikiFuncTable::const_iterator  it;
    WikiCmdTableSupport::wikifunc_t*  wf;

    if ((it = GWikiFuncTable.find (name)) != GWikiFuncTable.end ()) {
	wf = it->second;
	switch (wf->atype) {
	case WikiCmdTableSupport::WikiArg1:
	    {
		WikiMotorObjFunc1*  obj = new WikiMotorObjFunc1 (name, wf);
		out.push_back (WikiMotorObjPtr (obj));
		compile (obj->arg, TMATCH_CKET);
		sp.shiftCursor ();
	    }
	    return;
	case WikiCmdTableSupport::WikiArgM:
	    {
		WikiMotorObjFuncM*  obj = new WikiMotorObjFuncM (name, wf);
		out.push_back (WikiMotorObjPtr (obj));
		while (1) {
		    WikiMotorObjVecPtr  vec (new WikiMotorObjVec);
		    obj->arg.push_back (vec);
		    compile (*vec, TMATCH_CKET | TMATCH_COLN);
		    if (sp.match (re_wiki1_colon)) {	// :
		    } else {
			break;
		    }
		}
		sp.shiftCursor ();
	    }
	    return;
	case WikiCmdTableSupport::WikiArgM2:
	    {
		WikiMotorObjFuncM2*  obj = new WikiMotorObjFuncM2 (name, wf);
		out.push_back (WikiMotorObjPtr (obj));
		while (1) {
		    WikiMotorObjVecPtr  vec (new WikiMotorObjVec);
		    obj->arg.push_back (vec);
		    compile (*vec, TMATCH_CKET | TMATCH_COLN | TMATCH_BAR2);
		    if (sp.match (re_wiki1_colon)) {	// :
		    } else {
			break;
		    }
		}
		if (sp.match (re_wiki1_bar2)) {	// ||
		    compile (obj->arg2, TMATCH_CKET);
		}
		sp.shiftCursor ();
	    }
	    return;
	default:
	    assert (0);
	}
    } else if (compile_4 (out, name)) {
	return;
    } else {
	if (sp.matchBegin () < sp.matchEnd ())
	    out.push_back (WikiMotorObjPtr (new WikiMotorObjText (sp.matchBegin (), sp.matchEnd ())));
	sp.shiftCursor ();
	compile (out, TMATCH_CKET);
	if (sp.matchBegin () < sp.matchEnd ())
	    out.push_back (WikiMotorObjPtr (new WikiMotorObjText (sp.matchBegin (), sp.matchEnd ())));
	sp.shiftCursor ();
	return;
    }
}

bool  WikiMotor::compile_4 (WikiMotorObjVec& out, const ustring& name) {
    MNode*  wd;

    if ((wd = wiki->env->wikienv->wikiFunc.getVar (name))) {
	WikiMotorObjFuncM*  obj = new WikiMotorObjFuncM (name, wd);
	out.push_back (WikiMotorObjPtr (obj));
	while (1) {
	    WikiMotorObjVecPtr  vec (new WikiMotorObjVec);
	    obj->arg.push_back (vec);
	    compile (*vec, TMATCH_CKET | TMATCH_COLN);
	    if (sp.match (re_wiki1_colon)) {	// :
	    } else {
		break;
	    }
	}
	sp.shiftCursor ();
	return true;
    } else if ((wd = wiki->env->wikienv->wikiFunc2.getVar (name))) {
	WikiMotorObjFuncM2*  obj = new WikiMotorObjFuncM2 (name, wd);
	out.push_back (WikiMotorObjPtr (obj));
	while (1) {
	    WikiMotorObjVecPtr  vec (new WikiMotorObjVec);
	    obj->arg.push_back (vec);
	    compile (*vec, TMATCH_CKET | TMATCH_COLN | TMATCH_BAR2);
	    if (sp.match (re_wiki1_colon)) {	// :
	    } else {
		break;
	    }
	}
	if (sp.match (re_wiki1_bar2)) { // ||
	    compile (obj->arg2, TMATCH_CKET);
	}
	sp.shiftCursor ();
	return true;
    } else if ((wd = wiki->env->wikienv->wikiLink.getVar (name))) {
	WikiMotorObjFuncLink*  obj = new WikiMotorObjFuncLink (name, wd);
	out.push_back (WikiMotorObjPtr (obj));
	while (1) {
	    WikiMotorObjVecPtr  vec (new WikiMotorObjVec);
	    obj->arg.push_back (vec);
	    compile (*vec, TMATCH_CKET | TMATCH_COLN);
	    if (sp.match (re_wiki1_colon)) {	// :
	    } else {
		break;
	    }
	}
	sp.shiftCursor ();
	return true;
    } else {
	return false;
    }
}

void  WikiMotor::compile_5 (WikiMotorObjVec& out, int nchar) { // ''
    WikiMotorObjEmph*  obj = new WikiMotorObjEmph;
    int  en;

#if 1
    if (nchar > 5) {
	out.push_back (WikiMotorObjPtr (new WikiMotorObjText (sp.matchBegin (), sp.matchBegin () + (nchar - 5))));
	nchar = 5;
    } else if (nchar == 4) {
	out.push_back (WikiMotorObjPtr (new WikiMotorObjText (sp.matchBegin (), sp.matchBegin () + 1)));
	nchar = 3;
    }
    out.push_back (WikiMotorObjPtr (obj));
#else
    out.push_back (WikiMotorObjPtr (obj));
    if (nchar > 5) {
	obj->text.push_back (WikiMotorObjPtr (new WikiMotorObjText (sp.matchBegin (), sp.matchBegin () + (nchar - 5))));
	nchar = 5;
    } else if (nchar == 4) {
	obj->text.push_back (WikiMotorObjPtr (new WikiMotorObjText (sp.matchBegin (), sp.matchBegin () + 1)));
	nchar = 3;
    }
#endif
    obj->level = nchar;
    while (! sp.isEnd ()) {
	compile (obj->text, TMATCH_QUOT);
	en = sp.matchEnd () - sp.matchBegin ();
	if (en >= nchar) {
	    if (en > nchar) {
#if 1
		sp.u -= en - nchar;
#else
		obj->text.push_back (WikiMotorObjPtr (new WikiMotorObjText (sp.matchBegin (), sp.matchBegin () + (en - nchar))));
#endif
	    }
	    sp.shiftCursor ();
	    return;
	} else {
	    obj->text.push_back (WikiMotorObjPtr (new WikiMotorObjText (sp.matchBegin (), sp.matchEnd ())));
	    sp.shiftCursor ();
	}
    }
}

void  WikiMotor::compile_6 (WikiMotorObjVec& out, const uiterator& b, const uiterator& e, bool fneg) {		// {NAME?
    compile_6 (out, WikiMotorObjPtr (new WikiMotorObjVar (b, e, WikiMotorObjVar::opt_normal)), fneg);
}

void  WikiMotor::compile_6 (WikiMotorObjVec& out, WikiMotorObjPtr varptr, bool fneg) {		// {NAME?
    WikiMotorObjCond*  obj = new WikiMotorObjCond (fneg);
    obj->var = varptr;
    out.push_back (WikiMotorObjPtr (obj));
    compile (obj->value, TMATCH_COLN | TMATCH_CKET);
    if (sp.match (re_wiki1_colon)) {	// :
	compile (obj->trueobjs, TMATCH_BAR2 | TMATCH_BAR2ELSE | TMATCH_CKET);
	if (sp.match (re_wiki1_bar2)) {	// ||
	    compile (obj->falseobjs, TMATCH_CKET);
	} else if (sp.match (re_wiki1_bar2else)) {	// ||!?\?
	    compile_6 (obj->falseobjs, varptr, false);
	}
    } else {
	// syntax error
    }
}

void  WikiMotor::compile_7 (WikiMotorObjVec& out, const ustring& name) {	// {NAME||
    WikiFuncTable::const_iterator  it;
    WikiCmdTableSupport::wikifunc_t*  wf;
    MNode*  wd;

    if ((it = GWikiFuncTable.find (name)) != GWikiFuncTable.end ()) {
	wf = it->second;
	switch (wf->atype) {
	case WikiCmdTableSupport::WikiArg1:
	    {
		WikiMotorObjFunc1*  obj = new WikiMotorObjFunc1 (name, wf);
		out.push_back (WikiMotorObjPtr (obj));
		compile (obj->arg, TMATCH_CKET);
		sp.shiftCursor ();
	    }
	    return;
	case WikiCmdTableSupport::WikiArgM2:
	    {
		WikiMotorObjFuncM2*  obj = new WikiMotorObjFuncM2 (name, wf);
		out.push_back (WikiMotorObjPtr (obj));
		compile (obj->arg2, TMATCH_CKET);
		sp.shiftCursor ();
	    }
	    return;
	default:;
	}
    } else if ((wd = wiki->env->wikienv->wikiFunc2.getVar (name))) {
	WikiMotorObjFuncM2*  obj = new WikiMotorObjFuncM2 (name, wd);
	out.push_back (WikiMotorObjPtr (obj));
	compile (obj->arg2, TMATCH_CKET);
	sp.shiftCursor ();
	return;
    }
    if (sp.matchBegin () < sp.matchEnd ())
	out.push_back (WikiMotorObjPtr (new WikiMotorObjText (sp.matchBegin (), sp.matchEnd ())));
    sp.shiftCursor ();
    compile (out, TMATCH_CKET);
    if (sp.matchBegin () < sp.matchEnd ())
	out.push_back (WikiMotorObjPtr (new WikiMotorObjText (sp.matchBegin (), sp.matchEnd ())));
    sp.shiftCursor ();
}

void  WikiMotor::compile_8 (WikiMotorObjVec& out, const ustring& name) {	// {NAME->
    WikiMotorObjTab*  obj = new WikiMotorObjTab (name);
    while (1) {
	WikiMotorObjVecPtr  vec (new WikiMotorObjVec);
	obj->arg.push_back (vec);
	compile (*vec, TMATCH_CKET | TMATCH_VAROPT | TMATCH_COLN | TMATCH_SELECT);
	if (sp.match (re_wiki1_colon)) {			// :
	} else if (sp.match (re_wiki1_close2)) {	// } /} ~} ,}
	    uiterator  b = sp.matchBegin (re_wiki1_close2);
	    if (*b == '}') {
		out.push_back (WikiMotorObjPtr (obj));
		break;
	    } else {
		switch (*b) {
		case '/':					// /}
		    obj->setVopt (WikiMotorObjTab::opt_br);
		    break;
		case '~':					// ~}
		    obj->setVopt (WikiMotorObjTab::opt_nbsp);
		    break;
		case ',':					// ,}
		    obj->setVopt (WikiMotorObjTab::opt_c3);
		    break;
		default:
		    assert (0);
		}
		out.push_back (WikiMotorObjPtr (obj));
		break;
	    }
	} else if (sp.match (re_wiki1_wikivar_sel)) {		// ? !?
	    uiterator  b = sp.matchBegin (re_wiki1_wikivar_sel);
	    switch (*b) {
	    case '?':
		compile_6 (out, WikiMotorObjPtr (obj), false);
		break;
	    case '!':
		compile_6 (out, WikiMotorObjPtr (obj), true);
		break;
	    default:
		assert (0);
	    }
	    break;
	} else {	// }
	    delete obj;
	    return;
	}
    }
    sp.shiftCursor ();
}

/* ============================================================ */
void  join (WikiMotorObjVecVec::const_iterator b, WikiMotorObjVecVec::const_iterator e, const ustring& ch, WikiMotorObjVec& ans) {
    WikiMotorObjVec*  v;
    WikiMotorObjText*  t = NULL;
    WikiMotorObjText*  t2 = NULL;
    WikiMotorObj*  o;

    for (; b < e; b ++) {
	if (ans.size () > 0) {
	    if (t) {
		if (t2) {
		    t->text.append (ch);
		} else {
		    t2 = new WikiMotorObjText (t->text + ch);
		    ans.pop_back ();
		    ans.push_back (WikiMotorObjPtr (t2));
		    t = t2;
		}
	    } else {
		t = t2 = new WikiMotorObjText (ch);
		ans.push_back (WikiMotorObjPtr (t2));
	    }
	}
	v = b->get ();
	for (int j = 0; j < v->size (); j ++) {
	    o = (*v)[j].get ();
	    if (t && o->type == WikiMotorObj::wiki_text) {
		if (t2) {
		    t2->text.append (WikiMotorObjText_type (o)->text);
		} else {
		    t = t2 = new WikiMotorObjText (WikiMotorObjText_type (t)->text + ch);
		    ans.pop_back ();
		    ans.push_back (WikiMotorObjPtr (t2));
		}
	    } else {
		ans.push_back ((*v)[j]);
		if (o->type == WikiMotorObj::wiki_text)
		    t = WikiMotorObjText_type (o);
		else
		    t = NULL;
		t2 = NULL;
	    }
	}
    }
}

/* ============================================================ */
