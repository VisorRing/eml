#ifndef UTIL_SPLITTER_H
#define UTIL_SPLITTER_H

#include "ustring.h"
#include "util_string.h"

class  Splitter {
 public:
    uiterator  b;		// 先頭
    uiterator  t;		// 区切り文字列先頭
    uiterator  u;		// 区切り文字列末尾
    uiterator  e;		// 末尾

    Splitter (uiterator _begin, uiterator _end) {
	b = t = u = _begin;
	e = _end;
    };
    virtual  ~Splitter () {};

    virtual bool  isEnd () {
	return b == e;
    };
    virtual uiterator  begin () {
	return b;
    };
    virtual uiterator  end () {
	return t;
    };
    virtual ustring  pre () {
	return ustring (b, t);
    };
    virtual size_t  preSize () {
	return t - b;
    };
    virtual uiterator  matchBegin () {
	return t;
    };
    virtual uiterator  matchEnd () {
	return u;
    };
    virtual uiterator  eol () {
	return e;
    };
    virtual void  rewind (int i) {
	int  n = u - t;
	if (n > i) {
	    u -= i;
	} else {
	    u -= n;
	}
    };
    virtual void  shiftCursor () {
	b = u;
    };
    virtual bool  next () = 0;
    virtual bool  nextSep () = 0;
};

class  SplitterRe: public Splitter {
 public:
    uregex*  re;
    umatch  m;

    SplitterRe (const ustring& text, uregex& _re): Splitter (text.begin (), text.end ()) {
	re = &_re;
    };
    SplitterRe (uiterator _begin, uiterator _end, uregex& _re): Splitter (_begin, _end) {
	re = &_re;
    };
    virtual  ~SplitterRe () {};

    virtual bool  next () {
	b = u;
	if (b < e) {
	    if (usearch (b, e, m, *re)) {
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
	if (b < e) {
	    if (usearch (b, e, m, *re)) {
		t = m[0].first;
		u = m[0].second;
		return true;
	    } else {
		t = u = e;
		return false;
	    }
	} else {
	    t = u = e;
	    return false;
	}
    };
    virtual bool  match (int index) {
	return (t != u && m[index].matched);
    }
    virtual uiterator  matchBegin () {
	return t;
    };
    virtual uiterator  matchEnd () {
	return u;
    };
    virtual uiterator  matchBegin (int index) {
	return m[index].first;
    };
    virtual uiterator  matchEnd (int index) {
	return m[index].second;
    };
    virtual bool  nextSearch () {
	if (u != e) {
	    if (usearch (u, e, m, *re)) {
		t = m[0].first;
		u = m[0].second;
		return true;
	    } else {
		t = e;
		u = e;
		return false;
	    }
	} else {
	    t = e;
	    u = e;
	    return false;
	}
    };
};

class  SplitterCh: public Splitter {
 public:
    int  ch;

    SplitterCh (uiterator _begin, uiterator _end, int _ch): Splitter (_begin, _end) {
	ch = _ch;
    };
    SplitterCh (const ustring& text, int _ch): Splitter (text.begin (), text.end ()) {
	ch = _ch;
    };
    virtual  ~SplitterCh () {};

    virtual bool  next () {
	b = t = u;
	if (b < e) {
	    if (findChar (t, e, ch)) {
		u = t + 1;
	    } else {
		u = e;
	    }
	    return true;
	} else {
	    return false;
	}
    };
    virtual bool  nextSep () {
	b = t = u;
	if (b < e) {
	    if (findChar (t, e, ch)) {
		u = t + 1;
		return true;
	    } else {
		u = e;
		return false;
	    }
	} else {
	    t = u = e;
	    return false;
	}
    };
};

class  SplitterChars: public Splitter {
 public:
    ustring  pattern;

    SplitterChars (uiterator _begin, uiterator _end, const ustring& _pat): Splitter (_begin, _end) {
	pattern = _pat;
    };
    SplitterChars (const ustring& text, const ustring& _pat): Splitter (text.begin (), text.end ()) {
	pattern = _pat;
    };
    virtual  ~SplitterChars () {};

    virtual bool  next () {
	b = t = u;
	if (b < e) {
	    if (findChars (t, e, pattern)) {
		u = t + 1;
	    } else {
		u = e;
	    }
	    return true;
	} else {
	    return false;
	}
    };
    virtual bool  nextSep () {
	b = t = u;
	if (b < e) {
	    if (findChars (t, e, pattern)) {
		u = t + 1;
		return true;
	    } else {
		u = e;
		return false;
	    }
	} else {
	    t = u = e;
	    return false;
	}
    };
};

class  SplitterFn: public Splitter {
 public:
    bool  (*fn) (uiterator&, uiterator, uiterator&);

    SplitterFn (uiterator _begin, uiterator _end, bool (*_fn)(uiterator&, uiterator, uiterator&)): Splitter (_begin, _end) {
	fn = _fn;
    };
    SplitterFn (const ustring& text, bool (*_fn)(uiterator&, uiterator, uiterator&)): Splitter (text.begin (), text.end ()) {
	fn = _fn;
    };
    virtual  ~SplitterFn () {};

    virtual bool  next () {
	b = t = u;
	if (b < e) {
	    fn (t, e, u);
	    return true;
	} else {
	    return false;
	}
    };
    virtual bool  nextSep () {
	b = t = u;
	if (b < e) {
	    return fn (t, e, u);
	} else {
	    t = u = e;
	    return false;
	}
    };
};

class  SplitterNL: public Splitter {
 public:
    SplitterNL (uiterator _begin, uiterator _end): Splitter (_begin, _end) {};
    SplitterNL (const ustring& text): Splitter (text.begin (), text.end ()) {};
    virtual  ~SplitterNL () {};

    virtual bool  next () {
	b = t = u;
	if (b < e) {
	    findNL (t, e, u);
	    return true;
	} else {
	    return false;
	}
    };
    virtual bool  nextSep () {
	b = t = u;
	if (b < e) {
	    return findNL (t, e, u);
	} else {
	    t = u = e;
	    return false;
	}
    };
};

#endif /* UTIL_SPLITTER_H */
