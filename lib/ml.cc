#include "ml.h"
#include "config.h"
#include "mlenv.h"
#include "expr.h"
#include "utf8.h"
#include "utf16.h"
#include "ustring.h"
#include "util_string.h"
#include "util_const.h"
#include <iostream>
#include <exception>
#include <assert.h>
#include <stdlib.h>
#include <ctype.h>

static bool  findSymSp (uiterator& b, uiterator e) {
    int  c;
    for (; b < e; ++ b) {
	c = *b;
	if (c == '\\' || c < ' ')	// 空白を含めない
	    return true;
    }
    return false;
}

static bool  matchSymOct (uiterator& b, uiterator e) {
    int  c;
    int  n = 0;
    uiterator  p = b;
    for (; p < e && n < 3; ++ p, ++ n) {
	c = *p;
	if ('0' <= c && c <= '7') {
	} else {
	    return false;
	}
    }
    b = p;
    return true;
}

static MotorTexp::word_type  matchRealNum (uiterator&b, uiterator e) {
    MotorTexp::word_type  type = MotorTexp::YYINT;
    uiterator  p = b;
    int  c;
    bool  f = false;
    if (p < e) {
	c = *p;
	if (c == '+' || c == '-')
	    ++ p;
	while (1) {
	    if (p == e) {
		goto Ep1;
	    } else if (isdigit ((c = *p))) {
		++ p;
		f = true;
	    } else if (c == '.') {
		++ p;
		type = MotorTexp::YYNUM;
		break;
	    } else if (c == 'e' || c == 'E') {
		break;
	    } else {
		goto Ep1;
	    }
	}
	while (1) {
	    if (p == e) {
		goto Ep1;
	    } else if (isdigit ((c = *p))) {
		++ p;
		f = true;
	    } else if (c == 'e' || c == 'E') {
		++ p;
		type = MotorTexp::YYNUM;
		break;
	    } else {
		goto Ep1;
	    }
	}
	if (p == e)
	    return MotorTexp::YYNONE;
	c = *p;
	if (c == '+' || c == '-')
	    ++ p;
	if (p == e)
	    return MotorTexp::YYNONE;
	while (1) {
	    if (p == e) {
		goto Ep1;
	    } else if (isdigit ((c = *p))) {
		++ p;
	    } else {
		break;
	    }
	}
    }
 Ep1:
    if (f) {
	b = p;
	return type;
    } else {
	return MotorTexp::YYNONE;
    }
}

static bool  matchSymbol_c (int c) {
    static char  table_symbol[] = {	// x00-x20"'();[]{}
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 1, 0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0, 
    };
    return c < 0 || 128 <= c || table_symbol[c];
}
inline bool  matchSymbol (uiterator&b, uiterator e) {
    return matchHeadFn (b, e, matchSymbol_c);
}

static bool  findNonSymbol_c (int c) { // '\'を含める
    static char  table_nonsymbol[] = {		// x00-x20"'();[]{}\\   ;
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
	1, 0, 1, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 
    };
    return 0 <= c && c < 128 && table_nonsymbol[c];
}
inline bool  findNonSymbol (uiterator&b, uiterator e) { // '\'を含める
    return findCharFn (b, e, findNonSymbol_c);
}

void  MNode::fdelete () {
#ifdef HEAPDEBUG_VERBOSE
    std::cerr << "fdelete:" << std::hex << this << std::dec << ":" << type << ":<";
    this->dump (std::cerr);
    std::cerr << ">\n";
#endif /* DEBUG */
    assert (type != MC_DELETED);
    switch (type) {
    case MC_NIL:
	break;
    case MC_CONS:
	if (cons.car) {
	    if (cons.car->refcount <= 1) {
		delete cons.car;
	    } else {
		cons.car->refcount --;
	    }
	    cons.car = NULL;
	}
	if (cons.cdr) {
	    if (cons.cdr->refcount <= 1) {
		delete cons.cdr;
	    } else {
		cons.cdr->refcount --;
	    }
	}
	break;
    case MC_STR:
	delete str;
	str = NULL;
	break;
    case MC_SYM:
	delete sym;
	sym = NULL;
	break;
    case MC_VECTOR:
	delete vector;
	vector = NULL;
	break;
    case MC_TABLE:
	delete table;
	table = NULL;
	break;
    case MC_LIBOBJ:
	libobj = NULL;		// deleteは行わない
	break;
    case MC_BUILTIN_FN:
	bfn = NULL;
	break;
    case MC_BUILTIN_MFN:
	bfn = NULL;
	break;
    case MC_BUILTIN_SFN:
	sfn = NULL;
	senv = NULL;
	break;
    case MC_LAMBDA:
	if (lambda.sexp) {
	    if (lambda.sexp->refcount <= 1) {
		delete lambda.sexp;
	    } else {
		lambda.sexp->refcount --;
	    }
	    lambda.sexp = NULL;
	}
	lambda.env = NULL;
	break;
    default:;
    }
    type = MC_DELETED;
}

void  MNode::swap (MNode& b) {
    char  u[sizeof (MNode)];
    memcpy (u, &b, sizeof (MNode));
    memcpy (&b, this, sizeof (MNode));
    memcpy (this, u, sizeof (MNode));
}

void  MNode::set_car (MNode* v) {
    assert (type == MC_NIL || (type == MC_CONS && cons.car == NULL));
    type = MC_CONS;
    cons.car = v;
    if (cons.car)
	cons.car->refcount ++;
}

void  MNode::unset_car () {
    assert (type == MC_CONS);
    if (cons.car) {
	if (cons.car->refcount <= 1) {
	    delete cons.car;
	} else {
	    cons.car->refcount --;
	}
	cons.car = NULL;
    }
}

void  MNode::unset_cdr () {
    assert (type == MC_CONS);
    if (cons.cdr) {
	if (cons.cdr->refcount <= 1) {
	    delete cons.cdr;
	} else {
	    cons.cdr->refcount --;
	}
	cons.cdr = NULL;
    }
}

void  MNode::set_cdr (MNode* v) {
    assert (type == MC_NIL || (type == MC_CONS && cons.cdr == NULL));
    type = MC_CONS;
    cons.cdr = v;
    if (cons.cdr)
	cons.cdr->refcount ++;
}

MNode*  MNode::release_cdr () {
    MNode*  ans = cdr ();
    set_cdr (NULL);
    if (ans)
	ans->refcount --;
    return ans;
}

void  MNode::set_str (ustring* v) {
    assert (type == MC_NIL);
    type = MC_STR;
    str = v;
}

void  MNode::set_sym (ustring* v) {
    assert (type == MC_NIL);
    type = MC_SYM;
    sym = v;
}

void  MNode::set_int64 (int64_t v) {
    assert (type == MC_NIL);
    type = MC_INT64;
    num = v;
}

void  MNode::set_num (int64_t v) {
    assert (type == MC_NIL);
    type = MC_INT64;
    num = v;
}

void  MNode::set_num (double v) {
    assert (type == MC_NIL);
    type = MC_DOUBLE;
    real = v;
}

void  MNode::set_nil () {
    assert (type == MC_NIL);
}

void  MNode::set_libobj (MLFunc* obj) {
    assert (type == MC_NIL);
    type = MC_LIBOBJ;
    libobj = obj;
}

void  MNode::set_builtin_fn (XFTableVal* fn) {
    assert (type == MC_NIL);
    type = MC_BUILTIN_FN;
    bfn = fn;
}

void  MNode::set_builtin_mfn (XFTableVal* fn) {
    assert (type == MC_NIL);
    type = MC_BUILTIN_MFN;
    bfn = fn;
}

void  MNode::set_builtin_sfn (XFTableVal* _fn, MLFunc* _senv) {
    assert (type == MC_NIL);
    type = MC_BUILTIN_SFN;
    sfn = _fn;
    senv = _senv;
}

void  MNode::set_lambda (MNode* _sexp, MlEnv* _env, uint32_t _mode) {
    assert (type == MC_NIL);
    type = MC_LAMBDA;
    lambda.sexp = _sexp;
    lambda.env = _env;
    lambda.mode = _mode;
    if (lambda.sexp)
	lambda.sexp->refcount ++;
}

void  MNode::set_bool (bool f) {
    assert (type == MC_NIL);
    type = MC_BOOL;
    q = f;
}


MNode*  MNode::append_cdr (MNode* v) {
    MNode*  c = this;
    MNode*  d;
    while (1) {
	d = c->cdr ();
	if (d) {
	    c = d;
	} else {
	    c->set_cdr (v);
	    return v;
	}
    }
    return NULL;
}

MNode*  MNode::vectorGet (size_t pos) {
    if (type != MC_VECTOR)
	throw (uErrorWrongType);
    return vector->get (pos);
}

size_t  MNode::vectorSize () {
    if (type != MC_VECTOR)
	throw (uErrorWrongType );
    return vector->size ();
}

void  MNode::vectorPut (size_t pos, MNode* e) {
    if (type != MC_VECTOR)
	throw (uErrorWrongType );
    vector->put (pos, e);
}

MNode*  MNode::vectorDel (size_t pos) {
    if (type != MC_VECTOR)
	throw (uErrorWrongType );
    MNodePtr  ans;
    size_t  n = vector->size ();
    size_t  to = pos + 1;
    ans = vector->get (pos);
    if (pos < n) {
	for (; to < n; ++ pos, ++ to) {
	    vector->put (pos, vector->get (to));
	}
	vector->resize (pos);
    }
    return ans.release ();
}

MNode*  MNode::vectorDel (size_t from, size_t to) {
    if (type != MC_VECTOR)
	throw (uErrorWrongType );
    MNodePtr  ans;
    size_t  n = vector->size ();
    size_t  pos;
    ++ to;
    ans = newMNode_vector ();
    for (pos = from; pos < to; ++ pos) {
	ans ()->vectorPush (vector->get (pos));
    }
    if (from < n && from < to) {
	for (; to < n; ++ from, ++ to) {
	    vector->put (from, vector->get (to));
	}
	vector->resize (from);
    }
    return ans.release ();
}

void  MNode::vectorUnshift (MNode* e) {
    if (type != MC_VECTOR)
	throw (uErrorWrongType );
    vector->unshift (e);
}

MNode*  MNode::vectorShift () {
    MNodePtr  ans;
    if (type != MC_VECTOR)
	throw (uErrorWrongType );
    ans = vector->shift ();
    return ans.release ();
}

void  MNode::vectorPush (MNode* e) {
    if (type != MC_VECTOR)
	throw (uErrorWrongType );
    vector->push (e);
}

MNode*  MNode::vectorPop () {
    MNodePtr  ans;
    if (type != MC_VECTOR)
	throw (uErrorWrongType );
    ans = vector->pop ();
    return ans.release ();
}

void  MNode::vectorResize (size_t pos) {
    if (type != MC_VECTOR)
	throw (uErrorWrongType );
    vector->resize (pos);
}

MNode*  MNode::tableGet (const ustring& name) {
    MNodePtr  ans;
    if (type != MC_TABLE)
	throw (uErrorWrongType );
    ans = table->getVar (name);
    return ans.release ();
}

void  MNode::tablePut (const ustring& name, MNode* e) {
    if (type != MC_TABLE)
	throw (uErrorWrongType );
    table->setVar (name, e);
}

MNode*  MNode::tableDel (const ustring& name) {
    MNodePtr  ans;
    if (type != MC_TABLE)
	throw (uErrorWrongType );
    ans = table->getVar (name);
    table->eraseVar (name);
    return ans.release ();
}

int64_t  MNode::to_int64 () {
    switch (type) {
    case MNode::MC_NIL:
	return 0;
    case MNode::MC_CONS:
    case MNode::MC_VECTOR:
    case MNode::MC_TABLE:
	return 0;
    case MNode::MC_STR:
	return ::to_int64 (*str);
    case MNode::MC_SYM:
	return ::to_int64 (*sym);
    case MNode::MC_INT64:
	return num;
    case MNode::MC_DOUBLE:
	return real;
    case MNode::MC_LIBOBJ:
    case MNode::MC_BUILTIN_FN:
    case MNode::MC_BUILTIN_MFN:
    case MNode::MC_BUILTIN_SFN:
    case MNode::MC_LAMBDA:
	return 0;
    default:
	assert (0);
    }
}

double  MNode::to_double () {
    switch (type) {
    case MNode::MC_NIL:
	return 0.;
    case MNode::MC_CONS:
    case MNode::MC_VECTOR:
    case MNode::MC_TABLE:
	return 0.;
    case MNode::MC_STR:
	return ::to_double (*str);
    case MNode::MC_SYM:
	return ::to_double (*sym);
    case MNode::MC_INT64:
	return num;
    case MNode::MC_DOUBLE:
	return real;
    case MNode::MC_LIBOBJ:
    case MNode::MC_BUILTIN_FN:
    case MNode::MC_BUILTIN_MFN:
    case MNode::MC_BUILTIN_SFN:
    case MNode::MC_LAMBDA:
	return 0.;
    default:
	assert (0);
    }
}

static MNode*  read_number(const ustring& str) {
    uiterator b = str.begin ();
    uiterator e = str.end ();
    uiterator p = b;
    MotorTexp::word_type  type = matchRealNum (p, e);
    if (p != e) {
	return newMNode_int64 (0);
    } else {
	switch (type) {
	case MotorTexp::YYINT:
	    return newMNode_int64 (to_int64 (str));
	case MotorTexp::YYNUM:
	    return newMNode_num (to_double (str));
	default:
	    return newMNode_int64 (0);
	}
    }
}

MNode*  MNode::to_number() {
    switch (type) {
    case MNode::MC_STR:
	return read_number(*str);
    case MNode::MC_SYM:
	return read_number(*sym);
    case MNode::MC_INT64:
	return this;
    case MNode::MC_DOUBLE:
	return this;
    case MNode::MC_NIL:
    case MNode::MC_CONS:
    case MNode::MC_VECTOR:
    case MNode::MC_TABLE:
    case MNode::MC_LIBOBJ:
    case MNode::MC_BUILTIN_FN:
    case MNode::MC_BUILTIN_MFN:
    case MNode::MC_BUILTIN_SFN:
    case MNode::MC_LAMBDA:
	return newMNode_int64 (0);
    default:
	assert (0);
    }
}

bool  MNode::to_bool () {
    // 空文字列，文字列の"0"，数値の0，nilはfalse
    switch (type) {
    case MNode::MC_NIL:
	return false;
    case MNode::MC_CONS:
    case MNode::MC_VECTOR:
    case MNode::MC_TABLE:
	return true;
    case MNode::MC_STR:
	return ::to_bool (*str);
    case MNode::MC_SYM:
	if (sym->length () > 0) { // nilというシンボルはnilではない
	    return true;
	} else {
	    return false;
	}
    case MNode::MC_INT64:
	if (num == 0) {
	    return false;
	} else {
	    return true;
	}
    case MNode::MC_DOUBLE:
	if (real == 0.) {
	    return false;
	} else {
	    return true;
	}
    case MNode::MC_LIBOBJ:
    case MNode::MC_BUILTIN_FN:
    case MNode::MC_BUILTIN_MFN:
    case MNode::MC_BUILTIN_SFN:
    case MNode::MC_LAMBDA:
	return true;
    case MNode::MC_BOOL:
	return q;
    default:
	assert (0);
    }
}

ustring  MNode::sym_to_string () {
    assert (type == MC_SYM);
    uiterator  b = sym->begin ();
    uiterator  e = sym->end ();
    uiterator  p;
    p = b;
    if (findNonSymbol (b, e)) {
	ustring  ans;
	do {
	    if (p < b)
		ans.append (p, b);
	    ans.append (1, '\\').append (octchar (*b));
	    ++ b;
	    p = b;
	} while (b < e && findNonSymbol (b, e));
	if (p < e)
	    ans.append (p, e);
	return ans;
    } else {
	return *sym;
    }
}

ustring  MNode::cons_to_texp (bool fcdr) {
    // fcdr: cdr部の出力
    ustring  ans;

    assert (type == MC_CONS);
    if (! fcdr
	&& cons.car && cons.car->type == MC_SYM && *cons.car->str == uQuote
	&& cons.cdr && cons.cdr->type == MC_CONS
	&& ::isNil (cons.cdr->cons.cdr)) {
	ans.append (CharConst ("'"));
	if (::isNil (cons.cdr->cons.car))
	    ans.append (uNil2);
	else
	    ans.append (cons.cdr->cons.car->to_texp ());
    } else {
	if (! fcdr)
	    ans.append (CharConst ("("));
	if (::isNil (cons.car))
	    ans.append (uNil2);
	else
	    ans.append (cons.car->to_texp ());
	if (::isNil (cons.cdr)) {
	    ans.append (CharConst (")"));
	} else {
	    switch (cons.cdr->type) {
	    case MC_CONS:
		ans.append (uSPC);
		ans.append (cons.cdr->cons_to_texp (true));
		break;
	    default:
		ans.append (CharConst (" . "));
		ans.append (cons.cdr->to_texp ());
		ans.append (CharConst (")"));
	    }		
	}
    }
    return ans;
}

ustring  MNode::to_texp () {
    switch (type) {
    case MNode::MC_NIL:
	return uNil2;
    case MNode::MC_CONS:
	return cons_to_texp ();
    case MNode::MC_STR:
	return uQ2 + slashEncode (*str) + uQ2;
    case MNode::MC_SYM:
	return sym_to_string ();
    case MNode::MC_INT64:
	return to_ustring (num);
    case MNode::MC_DOUBLE:
	return to_ustring (real);
    case MNode::MC_VECTOR:
	return vector_to_string ();
    case MNode::MC_TABLE:
	return table_to_string ();
    case MNode::MC_LIBOBJ:
	return ustring (CharConst ("OBJECT"));
    case MNode::MC_BUILTIN_FN:
	return ustring (CharConst ("<built-in:")) + ustring (bfn->name, bfn->namelen) + ustring (CharConst (">"));
    case MNode::MC_BUILTIN_MFN:
	return ustring (CharConst ("<built-in:")) + ustring (bfn->name, bfn->namelen) + ustring (CharConst (">"));
    case MNode::MC_BUILTIN_SFN:
	return ustring (CharConst ("<built-in:")) + ustring (sfn->name, sfn->namelen) + ustring (CharConst (">"));
    case MNode::MC_LAMBDA:
	{
	    ustring  ans (CharConst ("(lambda"));
	    MNode*  a = lambda.sexp;
	    while (a) {
		if (a->car ())
		    ans = ans + uSPC + a->car ()->to_texp ();
		else
		    ans = ans + uSPC + uNil2;
		nextNode (a);
	    }
	    ans += ustring (CharConst (")"));
	    return ans;
	}
    case MNode::MC_BOOL:
	if (q)
	    return uTrue;
	else
	    return uFalse;
    default:
	fprintf (stderr, "deleted object\n");
	assert (0);
    }
}

ustring  MNode::to_string () {
    switch (type) {
    case MNode::MC_NIL:
	return uEmpty;
    case MNode::MC_CONS:
	return cons_to_texp ();
    case MNode::MC_STR:
	return *str;
    case MNode::MC_SYM:
	return *sym;		// to-stringでは、エンコードしない
    case MNode::MC_INT64:
	return to_ustring (num);
    case MNode::MC_DOUBLE:
	return to_ustring (real);
    case MNode::MC_VECTOR:
	return vector_to_string ();
    case MNode::MC_TABLE:
	return table_to_string ();
    case MNode::MC_LIBOBJ:
    case MNode::MC_BUILTIN_FN:
    case MNode::MC_BUILTIN_MFN:
    case MNode::MC_BUILTIN_SFN:
    case MNode::MC_LAMBDA:
    case MNode::MC_BOOL:
	return to_texp ();
    default:
	assert (0);
    }
}

ustring  MNode::vector_to_string () {
    ustring  ans;

    if (type != MC_VECTOR)
	throw (uErrorWrongType );

    ans.append (CharConst ("["));
    for (MotorVector::size_type i = 0; i < vector->size (); ++ i) {
	if (i > 0)
	    ans.append (uSPC);
	ans.append (dump_to_texp (vector->get (i)));
    }
    ans.append (CharConst ("]"));
    return ans;
}

ustring  MNode::table_to_string () {
    ustring  ans;

    if (type != MC_TABLE)
	throw (uErrorWrongType );

    ans.append (CharConst ("{"));
    MotorVar::iterator  b, e;
    size_t  c = 0;
    b = table->begin ();
    e = table->end ();
    for (; b != e; ++ b, ++c) {
	if (c > 0)
	    ans.append (uSPC);
	ans.append (uQ2).append ((*b).first).append (uQ2);
	ans.append (CharConst (" => "));
	ans.append (dump_to_texp ((*b).second ()));
    }
    ans.append (CharConst ("}"));
    return ans;
}

void  MNode::dump (std::ostream& o, bool c) {
    o << logText (to_texp ());
}

ustring  MNode::dump_string (bool c) {
    return logText (to_texp ());
}

ustring  MNode::dump_string_short () {
    ustring  u = dump_string ();

    return ellipsis (u, cLOGSHORTSIZE);
}

MNode*  newMNode_sym (ustring* v) {
    MNode*  ans = new MNode;
    uiterator  b = v->begin ();
    uiterator  e = v->end ();
    uiterator  p;
    int  c;

    p = b;
    if (findSymSp (b, e)) {
	ustring*  w = new ustring;
	do {
	    if (p < b)
		w->append (p, b);
	    c = *b;
	    ++ b;
	    if (c == '\\') {
		p = b;
		if (matchSymOct (b, e)) {
		    c = octchar (p);
		    if (32 <= c && c < 127)
			w->append (1, c);
		} else {
		    w->append (1, '\\');
		}
	    } else {
		// skip;
	    }
	    p = b;
	} while (findSymSp (b, e));
	if (p < e)
	    w->append (p, e);
	ans->set_sym (w);
	delete v;
    } else {
	ans->set_sym (v);
    }
    return ans;
}

MNode*  newMNode_str (ustring* v) {
    MNode*  ans = new MNode;
    ans->set_str (v);
    return ans;
}

MNode*  newMNode_int64 (int64_t v) {
    MNode*  ans = new MNode;
    ans->set_int64 (v);
    return ans;
}

MNode*  newMNode_num (int64_t v) {
    MNode*  ans = new MNode;
    ans->set_int64 (v);
    return ans;
}

MNode*  newMNode_num (uint64_t v) {
    MNode*  ans = new MNode;
    ans->set_int64 (v);
    return ans;
}

MNode*  newMNode_num (double v) {
    MNode*  ans = new MNode;
    ans->set_num (v);
    return ans;
}

MNode*  newMNode_bool (bool v) {
    MNode*  ans = new MNode;
    ans->set_bool (v);
    return ans;
}

MNode*  newMNode_cons (MNode* a) {
    MNode*  ans = new MNode;
    ans->set_car (a);
    return ans;
}

MNode*  newMNode_cons (MNode* a, MNode* d) {
    MNode*  ans = new MNode;
    ans->set_car (a);
    ans->set_cdr (d);
    return ans;
}

MNode*  newMNode_symQuote () {
    return newMNode_sym (new ustring (uQuote));
}

MNode*  newMNode_quote (MNode* v) {
    return newMNode_cons (newMNode_symQuote (), newMNode_cons (v));
}

void  newMNodeCdr (MNode*& cell) {
    MNode*  x = new MNode;
    cell->set_cdr (x);
    cell = x;
}

MNode*  newMNode_vector (MotorVector* vec) {
    MNode*  ans = new MNode;
    ans->type = MNode::MC_VECTOR;
    ans->vector = vec;
    return ans;
}

MNode*  newMNode_vector () {
    return newMNode_vector (new MotorVector);
}

MNode*  newMNode_table (MotorVar* tbl) {
    MNode*  ans = new MNode;
    ans->type = MNode::MC_TABLE;
    ans->table = tbl;
    return ans;
}

MNode*  newMNode_table () {
    return newMNode_table (new MotorVar);
}

MNode*  newMNode_libobj (MLFunc* fn) {
    MNode*  ans = new MNode;
    ans->set_libobj (fn);
    return ans;
}

MNode*  newMNode_builtin_fn (XFTableVal* fn) {
    MNode*  ans = new MNode;
    ans->set_builtin_fn (fn);
    return ans;
}

MNode*  newMNode_builtin_mfn (XFTableVal* fn) {
    MNode*  ans = new MNode;
    ans->set_builtin_mfn (fn);
    return ans;
}

MNode*  newMNode_builtin_sfn (XFTableVal* fn, MLFunc* senv) {
    MNode*  ans = new MNode;
    ans->set_builtin_sfn (fn, senv);
    return ans;
}

MNode*  list_to_vector (MNode* e) {
    MNodePtr  ans;
    ans = newMNode_vector ();
    if (isNil (e)) {
    } else {
	assert (isCons (e));
	while (e) {
	    ans ()->vectorPush (e->car ());
	    nextNode (e);
	}
    }
    return ans.release ();
}

MNode*  quoted (MNode* v) {
    if (v) {
	switch (v->type) {
	case MNode::MC_SYM:
	    if (v->sym
		&& v->sym->length () > 0
		&& ((*v->sym)[0] == '#' || (*v->sym)[0] == ':')) {
	    } else {
		v = newMNode_quote (v);
	    }
	    break;
	case MNode::MC_CONS:
	    v = newMNode_quote (v);
	    break;
	default:;
	}
    }
    return v;
}

static void  format_hex (ustring& ans, MNode* a, std::vector<ustring>& par, bool fcap) {
    uint64_t  v = 0;
    char  buf[32];

    if (a)
	v = to_int64 (a);

    if (par.size () > 0) {
	int  p = strtol (par[0]);
	if (p < 0)
	    p = 1;
	if (p > 20)
	    p = 20;
	if (fcap)
	    ans.append (buf, snprintf (buf, 32, "%.*lX", p, v));
	else
	    ans.append (buf, snprintf (buf, 32, "%.*lx", p, v));
    } else {
	if (fcap)
	    ans.append (buf, snprintf (buf, 32, "%lX", v));
	else
	    ans.append (buf, snprintf (buf, 32, "%lx", v));
    }
}

static void  format_hex (ustring& ans, MNode* a, std::vector<ustring>& par) {
    format_hex (ans, a, par, false);
}

static void  format_HEX (ustring& ans, MNode* a, std::vector<ustring>& par) {
    format_hex (ans, a, par, true);
}

static void  format_int_sub (ustring& ans, MNode* a, std::vector<ustring>& par, bool pad0 = false) {
    int64_t  v = 0;
    char  buf[32];
    size_t  s;

    if (a)
	v = to_int64 (a);

    if (par.size () > 0) {
	bool  fclip = false;
	bool  fzero = pad0;
	bool  fc3 = false;
	if (match (par[0], CharConst ("comma")) || match (par[0], CharConst ("c"))) {
	    ans.append (c3 (to_ustring (v)));
	} else {
	    int  p = strtol (par[0]);
	    if (p < 0)
		p = 1;
	    if (p > 20)
		p = 20;
	    for (int i = 1; i < par.size (); i ++) {	
		if (match (par[i], CharConst ("clip"))) {
		    fclip = true;
		} else if (match (par[i], CharConst ("0"))) {
		    fzero = true;
		} else if (match (par[i], CharConst ("comma")) || match (par[i], CharConst ("c"))) {
		    fc3 = true;
		} else {
		    throw (par[i] + uErrorBadParam);
		}
	    }
	    if (fzero)
		s = snprintf (buf, 32, "%.*ld", p, v);
	    else
		s = snprintf (buf, 32, "%*ld", p, v);
	    if (fclip && s > p)
		ans.append (buf + s - p, p);
	    else if (! fclip && fc3) 
		ans.append (c3 (ustring (buf, s)));
	    else
		ans.append (buf, s);
	}
    } else {
	ans.append (to_ustring (v));
    }
}

static void  format_int (ustring& ans, MNode* a, std::vector<ustring>& par) {
    format_int_sub (ans, a, par);
}

static void  format_int0 (ustring& ans, MNode* a, std::vector<ustring>& par) {
    format_int_sub (ans, a, par, true);
}

static void  format_int (ustring& ans, MNode* a, int c, bool pad0 = false) {
    int64_t  v = 0;
    char  buf[32];
    size_t  s;

    if (a)
	v = to_int64 (a);

    if (c > 0) {
	if (c > 20)
	    c = 20;
	if (pad0)
	    s = snprintf (buf, 32, "%.*ld", c, v);
	else
	    s = snprintf (buf, 32, "%*ld", c, v);
	if (s > c)
	    ans.append (buf + s - c, c);
	else
	    ans.append (buf, s);
    } else {
	ans.append (to_ustring (v));
    }
}

static void  format_float (ustring& ans, MNode* a, std::vector<ustring>& par) {
    int  p1 = 0;
    int  p2 = 0;
    char  buf[32];

    if (par.size () > 0)
	p1 = strtol (par[0]);
    if (par.size () > 1)
	p2 = strtol (par[1]);
    if (p1 < 0)
	p1 = 0;
    if (p2 < 0)
	p2 = 0;
    if (p1 > 20)
	p1 = 20;
    if (p2 > 20)
	p2 = 20;
    ans.append (buf, snprintf (buf, 32, "%*.*lf", p1, p2, to_double (a)));
}

static void  format_string (ustring& ans, MNode* a, std::vector<ustring>& par) {
    int  p = 0;
    bool  fright = false;
    ustring  u = to_string (a);

    if (par.size () > 0)
	p = strtol (par[0]);
    if (p > 65536)
	p = 65536;
    if (par.size () > 1) {
	if (match (par[1], CharConst ("right")) || match (par[1], CharConst ("r")))
	    fright = true;
	else
	    throw (par[1] + uErrorBadParam);
    }
    if (fright) {
	if (u.size () < p)
	    ans.append (p - u.size (), ' ').append (u);
	else
	    ans.append (u);
    } else {
	if (u.size () < p)
	    ans.append (u).append (p - u.size (), ' ');
	else
	    ans.append (u);
    }
}

static void  format_literal (ustring& ans, MNode* a, const char* list[], int offset, size_t size) {
    int64_t  v;

    if (a) {
	v = to_int64 (a) - offset;
	if (0 <= v && v < size)
	    ans.append (list[v]);
    }
}

static void  format_month (ustring& ans, MNode* a, std::vector<ustring>& par) {
    format_literal (ans, a, MStr_a, 1, 12);
}

static void  format_Month (ustring& ans, MNode* a, std::vector<ustring>& par) {
    format_literal (ans, a, MStr, 1, 12);
}

static void  format_week (ustring& ans, MNode* a, std::vector<ustring>& par) {
    format_literal (ans, a, WStr_a, 0, 7);
}

static void  format_Week (ustring& ans, MNode* a, std::vector<ustring>& par) {
    format_literal (ans, a, WStr, 0, 7);
}

ustring  formatString (const ustring& format, boost::ptr_vector<MNodePtr>& par) {
    ustring  ans;
    uiterator  b, e;
    umatch  m;
    u_int  i;
    MNode*  a;
    uregex  re ("\\$\\{([1-9][0-9]*)(:([a-zA-Z][a-zA-Z0-9]*)(:([0-9a-z.:]+))?)?\\}");
    static struct {
	const char* name;
	size_t  namelen;
	void  (*fn)(ustring& ans, MNode* a, std::vector<ustring>& par);
    }  formatFunc[] = {
	{CharConst ("hex"), format_hex},
	{CharConst ("HEX"), format_HEX},
	{CharConst ("int"), format_int},
	{CharConst ("int0"), format_int0},
	{CharConst ("float"), format_float},
	{CharConst ("string"), format_string},
	{CharConst ("month"), format_month},
	{CharConst ("Month"), format_Month},
	{CharConst ("week"), format_week},
	{CharConst ("Week"), format_Week},
	{NULL, 0, NULL}
    };

    b = format.begin ();
    e = format.end ();
    while (usearch (b, e, m, re)) {
	ans.append (b, m[0].first);
	b = m[0].second;
	i = strtoul (ustring (m[1].first, m[1].second)) - 1;
	if (i < par.size ()) {
	    a = par[i] ();
	} else {
	    a = NULL;
	}
	if (! m[2].matched) {
	    if (a)
		ans.append (to_string (a));
	} else {
	    std::vector<ustring>  fpar;
	    int  i;
	    if (m[4].matched)
		split (m[5].first, m[5].second, ':', fpar);
	    for (i = 0; formatFunc[i].name; i ++) {
		if (match (m[3].first, m[3].second, formatFunc[i].name, formatFunc[i].namelen)) {
		    (*formatFunc[i].fn) (ans, a, fpar);
		    goto Bp1;
		}
	    }
	    ans.append (m[0].first, m[0].second);
	Bp1:;
	}
    }
    ans.append (b, e);

    return ans;
}

bool  eq (MNode* a, MNode* b) {
    if (a && b) {
	if (a->type == b->type) {
	    switch (a->type) {
	    case MNode::MC_NIL:
		return true;
	    case MNode::MC_CONS:
		if (a == b)
		    return true;
		else
		    return false;
	    case MNode::MC_STR:
		if (*a->str == *b->str)
		    return true;
		else
		    return false;
	    case MNode::MC_SYM:
		if (*a->sym == *b->sym)
		    return true;
		else
		    return false;
	    case MNode::MC_INT64:
		if (a->num == b->num)
		    return true;
		else
		    return false;
	    case MNode::MC_DOUBLE:
		if (a->real == b->real)
		    return true;
		else
		    return false;
	    case MNode::MC_VECTOR:
		if (a->vector->size () == b->vector->size ()) {
		    for (MotorVector::size_type i = 0; i < a->vector->size (); ++ i) {
			if (! eq (a->vector->get (i), b->vector->get (i)))
			    return false;
		    }
		    return true;
		} else {
		    return false;
		}
	    case MNode::MC_TABLE:
		if (a->table->size () == b->table->size ()) {
		    MotorVar::iterator  p, e;
		    p = a->table->begin ();
		    e = a->table->end ();
		    for (; p != e; ++ p) {
			if (! eq (p->second (), b->table->getVar (p->first)))
			    return false;
		    }
		    return true;
		} else {
		    return false;
		}
	    case MNode::MC_LIBOBJ:
		if (a->libobj == b->libobj)
		    return true;
		else
		    return false;
	    case MNode::MC_BUILTIN_FN:
		return a->bfn == b->bfn;
	    case MNode::MC_BUILTIN_MFN:
		return a->bfn == b->bfn;
	    case MNode::MC_BUILTIN_SFN:
		return a->sfn == b->sfn;
	    case MNode::MC_LAMBDA:
		return eq (a->lambda.sexp, b->lambda.sexp) && a->lambda.env == b->lambda.env;
	    case MNode::MC_BOOL:
		return a->q == b->q;
	    default:
		assert (0);
	    }
	} else {
	    return false;
	}
    } else if (a == NULL) {
	if (b == NULL) {
	    return true;
	} else {
	    return b->isNil ();
	}
    } else {			/* b == NULL */
	return a->isNil ();
    }
}

bool  equal (MNode* a, MNode* b) {
    if (a && b) {
	if (a->type == b->type) {
	    switch (a->type) {
	    case MNode::MC_NIL:
		return true;
	    case MNode::MC_CONS:
		if (equal (a->car (), b->car ())
		    && equal (a->cdr (), b->cdr ()))
		    return true;
		else
		    return false;
	    case MNode::MC_STR:
		if (*a->str == *b->str)
		    return true;
		else
		    return false;
	    case MNode::MC_SYM:
		if (*a->sym == *b->sym)
		    return true;
		else
		    return false;
	    case MNode::MC_INT64:
		if (a->num == b->num)
		    return true;
		else
		    return false;
	    case MNode::MC_DOUBLE:
		if (a->real == b->real)
		    return true;
		else
		    return false;
	    case MNode::MC_VECTOR:
		if (a->vector->size () == b->vector->size ()) {
		    for (MotorVector::size_type i = 0; i < a->vector->size (); ++ i) {
			if (! equal (a->vector->get (i), b->vector->get (i)))
			    return false;
		    }
		    return true;
		} else {
		    return false;
		}
	    case MNode::MC_TABLE:
		if (a->table->size () == b->table->size ()) {
		    MotorVar::iterator  p, e;
		    p = a->table->begin ();
		    e = a->table->end ();
		    for (; p != e; ++ p) {
			if (! equal (p->second (), b->table->getVar (p->first)))
			    return false;
		    }
		    return true;
		} else {
		    return false;
		}
	    case MNode::MC_LIBOBJ:
		if (a->libobj == b->libobj)
		    return true;
		else
		    return false;
	    case MNode::MC_BUILTIN_FN:
		return a->bfn == b->bfn;
	    case MNode::MC_BUILTIN_MFN:
		return a->bfn == b->bfn;
	    case MNode::MC_BUILTIN_SFN:
		return a->sfn == b->sfn;
	    case MNode::MC_LAMBDA:
		return eq (a->lambda.sexp, b->lambda.sexp);
	    case MNode::MC_BOOL:
		return a->q == b->q;
	    default:
		assert (0);
	    }
	} else {
	    return false;
	}
    } else if (a == NULL) {
	if (b == NULL) {
	    return true;
	} else {
	    return b->isNil ();
	}
    } else {			/* b == NULL */
	return a->isNil ();
    }
}

bool  neq (MNode* a, MNode* b) {
    if (a && b) {
	if (a->isInt ()) {
	    if (b->isInt ()) {
		return a->value_int () == b->value_int ();
	    } else if (b->isReal ()) {
		return (double)a->value_int () == b->value_real ();
	    } else {
		return false;
	    }
	} else if (a->isReal ()) {
	    if (b->isInt ()) {
		return a->value_real () == (double)b->value_int ();
	    } else if (b->isReal ()) {
		return a->value_real () == b->value_real ();
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

bool  ngt (MNode* a, MNode* b) {
    if (a && b) {
	if (a->isInt ()) {
	    if (b->isInt ()) {
		return a->value_int () > b->value_int ();
	    } else if (b->isReal ()) {
		return (double)a->value_int () > b->value_real ();
	    } else {
		return false;
	    }
	} else if (a->isReal ()) {
	    if (b->isInt ()) {
		return a->value_real () > (double)b->value_int ();
	    } else if (b->isReal ()) {
		return a->value_real () > b->value_real ();
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

bool  nge (MNode* a, MNode* b) {
    if (a && b) {
	if (a->isInt ()) {
	    if (b->isInt ()) {
		return a->value_int () >= b->value_int ();
	    } else if (b->isReal ()) {
		return (double)a->value_int () >= b->value_real ();
	    } else {
		return false;
	    }
	} else if (a->isReal ()) {
	    if (b->isInt ()) {
		return a->value_real () >= (double)b->value_int ();
	    } else if (b->isReal ()) {
		return a->value_real () >= b->value_real ();
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

void  MotorTexp::scan (const ustring& text, bool skip) {
    uiterator  b = text.begin ();
    uiterator  e = text.end ();
    word_type  type;
    ustring*  s;
    MNode*  cell = &top;
    int  linenum = 1;

    if (skip) {
	skipHead (b, e, linenum);
    }
    while (b != e) {
	if (skip) {
	    skipBlank (linenum, b, e);
	    if (*b == '<') {
#ifdef DEBUG
//		std::cerr << "---:" << ustring (b, b + 20) << "\n";
		if (mlenv && mlenv->log)
		    *mlenv->log << "---:" << ustring (b, b + 20) << "\n";
#endif /* DEBUG */
		return;
	    }
	}
	if (scanTexp (linenum, b, e, cell, true, YYNONE)) {
	    status = S_SURPLUS_PAREN;
	    return;
	}
    }
}

void  MotorTexp::skipHead (uiterator& b, uiterator& e, int& linenum) {
    while (b < e && (*b != '(' && *b != ';')) {
	if (findNLb (b, e)) {
	    linenum ++;
	}
    }
}

void  MotorTexp::scanWord (int& linenum, uiterator& b, uiterator& e, word_type& type, ustring*& ans) {
    ustring::value_type  c;
    uiterator  p;
 Ep1:;
    skipBlank (linenum, b, e);
    if (b != e) {
	c = *b;
	switch (c) {
	case '\"':
	    b ++;
	    type = YYTEXT;
	    ans = scanText (linenum, b, e);
	    break;
	case '\'':
	    b ++;
	    type = YYQUOTE;
	    ans = NULL;
	    break;
	case '(':
	    b ++;
	    type = YYPAR;
	    break;
	case ')':
	    b ++;
	    type = YYREN;
	    break;
	case ';':
	    b ++;
	    if (findNLb (b, e)) {
		linenum ++;
		goto Ep1;
	    } else {
		// end of comment but without nl.
		type = YYNONE;
	    }
	    break;
	case '[':
	    b ++;
	    type = YYPAR2;
	    break;
	case ']':
	    b ++;
	    type = YYREN2;
	    break;
	case '{':
	    b ++;
	    type = YYPAR3;
	    break;
	case '}':
	    b ++;
	    type = YYREN3;
	    break;
	default:
	    p = b;
	    if ((type = matchRealNum (b, e)) != YYNONE) {
		ans = new ustring (p, b);
	    } else if (matchSymbol (b, e)) {
		if (*p == '.' && b - p == 1) {
		    type = YYPERIOD;
		} else {
		    type = YYSIM;
		    ans = new ustring (p, b);
		}
		skipBlank (linenum, b, e);
	    } else {
		type = YYNONE;
//		std::cerr << "error\n";
		assert (0);
	    }
	}
    } else {
	type = YYNONE;
    }
}

ustring*  MotorTexp::scanText (int& linenum, uiterator& b, uiterator& e) {
    ustring::value_type  c;
    uiterator  p;
    ustring*  ans = new ustring;
    uint64_t  v;

    ans->reserve (128);
    while (b != e) {
	c = *b;
	if (c == '\"') {
	    b ++;
	    return ans;
	} else if (c == '\\') {
	    b ++;
	    if (b != e) {
		c = *b;
		switch (c) {
		case 't':
		    (*ans) += '\t';
		    b ++;
		    break;
		case 'n':
		    (*ans) += '\n';
		    b ++;
		    break;
		case 'r':
		    (*ans) += '\r';
		    b ++;
		    break;
		case '\n':
		    b ++;
		    linenum ++;
		    break;
		default:
		    p = b;
		    if (e - b >= 4 && matchHex4 (b, e)) {
			v = hextoul (p, b);
			if (v == '\t' || v == '\n' || v == '\r' || v >= 32) {
			    std::wstring  w (1, v);
			    ans->append (wtou (w));
			}
			break;
		    }
		bp1:
		    (*ans) += c;
		    b ++;
		}
	    }
	} else if (c == '\n') {
	    linenum ++;
	    nextChar (b, e, *ans);
	} else {
	    nextChar (b, e, *ans);
	}
    }

    // XXX no matching quote
    return ans;
}

void  MotorTexp::skipBlank (int& linenum, uiterator& b, uiterator& e) {
    ustring::value_type  c;

    while (b < e) {	// hack
	c = *b;
	if (0 <= c && c <= ' ') {
	    if (c == '\r') {
		++ linenum;
		++ b;
		if (b < e && *b == '\n')
		    ++ b;
	    } else if (c == '\n') {
		++ linenum;
		++ b;
	    } else {
		++ b;
	    }
	} else {
	    break;
	}
    }
}

bool  MotorTexp::scanCar (int& linenum, uiterator& b, uiterator& e, MNode* cell) {
    if (b != e) {
	if (scanTexp (linenum, b, e, cell, false, YYREN))
	    return true;
    }
    while (b != e) {
	if (scanTexp (linenum, b, e, cell, true, YYREN))
	    return true;
    }
    // reached at the end of the text.
    return false;
}

void  MotorTexp::scanQuote (int& linenum, uiterator& b, uiterator& e, MNode* cell) {
    cell->set_car (newMNode_symQuote ());
    newMNodeCdr (cell);

    if (b != e) {
	if (scanTexp (linenum, b, e, cell, false, YYNONE))
	    throw (uErrorSyntax);	// ')
    } else {
	throw (uErrorSyntax);
    }
}

void  MotorTexp::scanVector (int& linenum, uiterator& b, uiterator& e, MNode* cell) {
    MNodePtr  p;
    MNode*  x;
    while (true) {
	x = new MNode;
	p = x;
#ifdef DEBUG2
	std::cerr << "--" << ustring (b, e) << "\n";
#endif /* DEBUG */
	if (scanTexp (linenum, b, e, x, false, YYREN2)) {
#ifdef DEBUG2
	    std::cerr << "scanTexp\n";
	    std::cerr << ustring (b, e) << "\n";
#endif /* DEBUG */
	    break;
	}
	if (p ()->isCons ()) {
	    cell->vectorPush (p ()->car ());
	} else {
	    break;
	}
    }
}

void  MotorTexp::scanTable (int& linenum, uiterator& b, uiterator& e, MNode* cell) {
    MNodePtr  p;
    MNode*  x;
    while (true) {
	x = new MNode;
	p = x;
	if (scanTexp (linenum, b, e, x, false, YYREN3))
	    break;
	if (p ()->isCons ()) {
	    ustring  name = to_string (p ()->car ());
	    x = new MNode;
	    p = x;
	    if (scanTexp (linenum, b, e, x, false, YYREN3))
		break;
	    if (p ()->isCons ()) {
		MNode*  t = p ()->car ();
		if (t->isSym () && *(t->sym) == uAssoc) {
		    x = new MNode;
		    p = x;
		    if (scanTexp (linenum, b, e, x, false, YYREN3))
			break;
		    if (p ()->isCons ()) {
			cell->tablePut (name, p ()->car ());
		    } else {
			break;
		    }
		} else {
		    // error;
		    break;
		}
	    } else {
		break;		// terminate
	    }
	} else {
	    break;
	}
    }
}

bool  MotorTexp::scanTexp (int& linenum, uiterator& b, uiterator& e, MNode*& cell, bool qcdr, word_type closing) {
    word_type  type;
    ustring*  s;
    MNode*  x;

    scanWord (linenum, b, e, type, s);
    switch (type) {
    case YYNONE:
	break;
    case YYPAR:
	if (qcdr)
	    newMNodeCdr (cell);
	x = new MNode;
	if (mlenv)
	    mlenv->push_linenum (x, linenum);
	if (! scanCar (linenum, b, e, x)) {
	    status = S_MISSING_PAREN;
	}
	if (x->isNil ()) {
	    delete x;
	    cell->set_car (NULL);
	} else {
	    cell->set_car (x);
	}
	break;
    case YYPAR2:		// [
	if (qcdr)
	    newMNodeCdr (cell);
	x = newMNode_vector ();
	cell->set_car (x);
	if (mlenv)
	    mlenv->push_linenum (x, linenum);
	scanVector (linenum, b, e, x);
	break;
    case YYPAR3:		// {
	if (qcdr)
	    newMNodeCdr (cell);
	x = newMNode_table ();
	cell->set_car (x);
	if (mlenv)
	    mlenv->push_linenum (x, linenum);
	scanTable (linenum, b, e, x);
	break;
    case YYQUOTE:
	if (qcdr)
	    newMNodeCdr (cell);
	x = new MNode;
	cell->set_car (x);
	if (mlenv)
	    mlenv->push_linenum (x, linenum);
	scanQuote (linenum, b, e, x);
	break;
    case YYREN:
    case YYREN2:
    case YYREN3:
	if (type == closing) {
#ifdef DEBUG2
	    std::cerr << "close " << type << "\n";
#endif /* DEBUG */
	    return true;		// end with )
	} else {
	    return false;
	}
    case YYTEXT:
	if (qcdr)
	    newMNodeCdr (cell);
	cell->set_car (newMNode_str (s));
	break;
    case YYPERIOD:
	x = new MNode;
	if (scanCar (linenum, b, e, x)) {
	    if (x->isNil ()) {
		delete x;
		cell->set_cdr (NULL);
	    } else {
		cell->set_cdr (x->car ());
		delete x;
	    }
	} else {
	    delete x;
	}
	return true;
	break;
    case YYSIM:
	if (qcdr)
	    newMNodeCdr (cell);
	if (*s == uNil) {
	    delete s;
	    cell->set_car (new MNode);
	} else if (*s == uTrue) {
	    delete s;
	    cell->set_car (newMNode_bool (true));
	} else if (*s == uFalse) {
	    delete s;
	    cell->set_car (newMNode_bool (false));
	} else {
	    cell->set_car (newMNode_sym (s));
	}
	break;
    case YYINT:
	if (qcdr)
	    newMNodeCdr (cell);
	cell->set_car (newMNode_int64 (atoll (s->c_str ())));
	delete s;
	break;
    case YYNUM:
	if (qcdr)
	    newMNodeCdr (cell);
	cell->set_car (newMNode_num (atof (s->c_str ())));
	delete s;
	break;
    default:;
	assert (0);
    }
    return false;
}

bool  MotorTexp::matchHex4 (uiterator& b, uiterator e) {
    uiterator  p;
    int  n = 0;
    int  c;
    for (p = b; p < e && n < 4; ++ p, ++ n) {
	c = *b;
	if (('0' <= c && c <= '9')
	    || ('a' <= c && c <= 'f')
	    || ('A' <= c && c <= 'F')) {
	} else {
	    return false;
	}
    }
    b = p;
    return true;
}

void  nextNode (MNode*& arg) {
    if (arg) {
	switch (arg->type) {
	case MNode::MC_NIL:
	    arg = NULL;
	    break;
	case MNode::MC_CONS:
	    arg = arg->cdr ();
	    if (arg && ! arg->isCons ()) {
		throw (uErrorWrongType);
	    }
	    break;
	default:
	    throw (uErrorWrongType);
	}
    }
}

void  nextNodeNonNil (MNode*& arg) {
    nextNode (arg);
    if (! arg)
	throw (uErrorWrongNumber);
}
