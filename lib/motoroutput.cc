#include "motoroutput.h"
#include "ml.h"
#include "util_base64.h"
#include "util_string.h"
#include "filemacro.h"
#include "ustring.h"
#include "utf8.h"
#include "motorconst.h"
#include <iostream>

/* ============================================================ */
static uregex  re_encode ("(<)|(>)|(&)|(\")|(')");
static uregex  re_encode_br ("(<)|(>)|(&)|(\")|(')|(\n)");
static uregex  re_encode_br_a ("(<)|(>)|(&)|(\")|(')|(\n)|(" rHTML ")");

MotorOutput*  MotorOutput::outamp (uiterator b, uiterator e, uregex* re) {
    umatch  m;

    while (b != e && usearch (b, e, m, *re)) {
	if (b != m[0].first) {
	    ustring  t (b, m[0].first);
	    out_toText (t);
	}
	b = m[0].second;

	if (m[1].matched) {		// <
	    out_raw (CharConst ("&lt;"));
	} else if (m[2].matched) {	// >
	    out_raw (CharConst ("&gt;"));
	} else if (m[3].matched) {	// &
	    out_raw (CharConst ("&amp;"));
	} else if (m[4].matched) {	// "
	    out_raw (CharConst ("&quot;"));
	} else if (m[5].matched) {	// '
	    out_raw (CharConst ("&#39;"));
	} else if (m[6].matched) {	// \n
	    out_raw (CharConst ("<br />\n"));
	} else if (m[7].matched) {	// http...
	    out_raw (CharConst ("<a href=\""));
	    outamp (m[7].first, m[7].second, &re_encode);
	    out_raw (CharConst ("\">"));
	    outamp (m[7].first, m[7].second, &re_encode);
	    out_raw (CharConst ("</a>"));
	} else {
	    assert (0);
	}
    }
    if (b != e) {
	ustring  t (b, e);
	out_toText (t);
    }
    return this;
}

MotorOutput*  MotorOutput::out_templateText (const ustring& str) {
    return out_raw (fixUTF8 (str));
}

MotorOutput*  MotorOutput::out_toHTML (const ustring& str) {
    ustring  t = fixUTF8 (str);
    outamp (t.begin (), t.end (), &re_encode);
    return this;
}

MotorOutput*  MotorOutput::out_toHTML_br (const ustring& str) {
    ustring  t = fixUTF8 (str);
    outamp (t.begin (), t.end (), &re_encode_br);
    return this;
}

MotorOutput*  MotorOutput::out_toHTML_br_a (const ustring& str) {
    ustring  t = fixUTF8 (str);
    outamp (t.begin (), t.end (), &re_encode_br_a);
    return this;
}

MotorOutput*  MotorOutput::out_toHTML_wbr (const ustring& str) {
    ustring  t = fixUTF8 (str);
    outamp (t.begin (), t.end (), &re_encode);
    return this;
    /* *** */
    assert (0);
}

MotorOutput*  MotorOutput::out_toHTML_noCtrl (const ustring& str) {
    out_toHTML (omitCtrl (str));
    return this;
}

MotorOutput*  MotorOutput::out_toJS (const ustring& str) {
    out_raw (jsEncode (str));
    return this;
}

MotorOutput*  MotorOutput::out_noCtrl (const ustring& str) {
    out_raw (omitCtrl (str));
    return this;
}

MotorOutput*  MotorOutput::outNbsp () {
    out_raw (uNbsp);
    return this;
}

MotorOutput*  MotorOutput::out_file (const ustring& src) {
    FileMacro  f;
    char*  b;
    ssize_t  s;

    if (f.openRead (src.c_str ())) {
	b = new char[65536];
	while ((s = f.read (b, 65536)) > 0) {
	    out (b, s);
	}
	delete b;
    }
    return this;
}

MotorOutput*  MotorOutput::out_file_base64 (const ustring& src) {
    FileMacro  f;
    ustring  b (64, '\0');
    ustring::iterator  p = b.begin ();
    ssize_t  s;

    if (f.openRead (src.c_str ())) {
	while ((s = f.read (&(*p), 57)) > 0) {
	    out_raw (base64Encode (p, p + s));
	    out (CharConst ("\n"));
	}
    }
    return this;
}

MotorOutput*  MotorOutput::outamp (const ustring& t) {
    outamp (t.begin (), t.end (), &re_encode);
    return this;
}

MotorOutput*  MotorOutput::outamp_br (const ustring& t) {
    outamp (t.begin (), t.end (), &re_encode_br);
    return this;
}

MotorOutput*  MotorOutput::outamp_wbr (const ustring& t) {
    outamp (t.begin (), t.end (), &re_encode);
    return this;
    /* *** */
    assert (0);
}

MotorOutput*  MotorOutput::outamp_nw (const ustring& t) {
    if (t.size () > 0) {
	outamp (t);
    } else {
	outNbsp ();
    }
    return this;
}

MotorOutput*  MotorOutput::outamp_c3 (const ustring& t) {
    outamp (c3 (t));
    return this;
}

/* ============================================================ */
MotorOutput*  MotorOutputCOut::out (const ustring::value_type* s, size_t len) {
    std::cout.write (s, len);
    return this;
}

/* ============================================================ */
MotorOutput*  MotorOutputString::out (const ustring::value_type* s, size_t len) {
    ans.append (s, len);
    return this;
}

/* ============================================================ */
MotorOutput*  MotorOutputOStream::out (const ustring::value_type* s, size_t len) {
    ostream->write (s, len);
    return this;
}

/* ============================================================ */
