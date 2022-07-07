#ifndef ML_H
#define ML_H

#include "util_const.h"
#include "util_string.h"
#include "ustring.h"
#include "ftable.h"
#include <iostream>
#include <assert.h>

class  MlEnv;
class  MotorVar;
class  MotorVector;
class  MLFunc;
class  MNode {
 public:
    typedef enum {
	MC_NIL,
	MC_CONS,
	MC_STR,
	MC_SYM,
	MC_INT64,
	MC_DOUBLE,
	MC_VECTOR,
	MC_TABLE,
	MC_LIBOBJ,
	MC_BUILTIN_FN,
	MC_BUILTIN_MFN,
	MC_BUILTIN_SFN,
	MC_LAMBDA,
	MC_BOOL,
	MC_DELETED,
    }  ptr_type;
    static const uint32_t  MODE_NOTRACE =	0x00000001;
    static const uint32_t  MODE_TRACE =		0x00000002;
    static const uint32_t  MODE_WIKIPUBLIC =	0x00000010;
    static const uint32_t  MODE_WIKINODEC =	0x00000020;
    static const uint32_t  MODE_WIKINODEC2 =	0x00000040;
    static const uint32_t  MODE_WIKIRAW =	0x00000080;
    static const uint32_t  MODE_WIKIOUT =	0x00000100;
    static const uint32_t  MODE_TEXTOUT =	0x00000200;

    ptr_type  type;
    int  refcount;
    union {
	struct {
	    MNode*  car;
	    MNode*  cdr;
	}  cons;
	ustring*  str;
	ustring*  sym;
	int64_t  num;
	double  real;
	MotorVector*  vector;
	MotorVar*  table;
	MLFunc*  libobj;
	XFTableVal*  bfn;	// MC_BUILTIN_FN, MC_BUILTIN_MFN
	struct {		// MC_BUILTIN_SFN
	    XFTableVal*  sfn;
	    MLFunc*  senv;
	};
	struct {		// MC_LAMBDA
	    MNode*  sexp;
	    MlEnv*  env;
	    uint32_t  mode;
	}  lambda;
	bool  q;
    };
    MNode () {
	type = MC_NIL;
	cons.car = NULL;
	cons.cdr = NULL;
	refcount = 0;
    };
//    MNode (bool v) {};
    ~MNode () {
	fdelete ();
    };
    
    void  fdelete ();
    MNode*  refinc () {
	refcount ++;
	return this;
    };
    void  swap (MNode& b);
    void  set_car (MNode* v);
    void  unset_car ();
    void  unset_cdr ();
    void  set_cdr (MNode* v);
    MNode*  release_cdr ();
    void  set_str (ustring* v);
    void  set_sym (ustring* v);
    void  set_int64 (int64_t v);
    void  set_num (int64_t v);
    void  set_num (double v);
    void  set_nil ();
    void  set_libobj (MLFunc* obj);
    void  set_builtin_fn (XFTableVal* fn);
    void  set_builtin_mfn (XFTableVal* fn);
    void  set_builtin_sfn (XFTableVal* _fn, MLFunc* _senv);
    void  set_lambda (MNode* _sexp, MlEnv* _env, uint32_t _mode);
    void  set_bool (bool f);
    inline bool  isNil () {
	return type == MC_NIL;
    };
    inline bool  isCons () {
	return type == MC_CONS;
    };
    inline bool  isStr () {
	return type == MC_STR;
    };
    inline bool  isSym () {
	return type == MC_SYM;
    };
    inline bool  isInt () {
	return type == MC_INT64;
    };
    inline bool  isReal () {
	return type == MC_DOUBLE;
    };
    inline bool  isIntReal () {
	return type == MC_INT64 || type == MC_DOUBLE;
    };
    inline bool  isVector () {
	return type == MC_VECTOR;
    };
    inline bool  isTable () {
	return type == MC_TABLE;
    };
    inline bool  isLibObj () {
	return type == MC_LIBOBJ;
    };
    inline bool  isLambda () {
	return type == MC_LAMBDA;
    };
    inline bool  isBool () {
	return type == MC_BOOL;
    };
    inline MNode*  car () {
	if (type != MC_CONS)
	    throw (uErrorWrongType);
	return cons.car;
    };
    inline MNode*  cdr () {
	if (type != MC_CONS)
	    throw (uErrorWrongType);
	return cons.cdr;
    };
    inline ustring*  value_str () {
	assert (isStr ());
	return str;
    };
    inline ustring*  value_sym () {
	assert (isSym ());
	return sym;
    };
    inline int64_t  value_int () {
	assert (isInt ());
	return num;
    };
    inline double  value_real () {
	assert (isReal ());
	return real;
    };
    inline MotorVector*  value_vector () {
	assert (isVector ());
	return vector;
    };
    inline MotorVar*  value_table () {
	assert (isTable ());
	return table;
    };
    inline MLFunc*  value_libobj () {
	assert (isLibObj ());
	return libobj;
    };
    inline MNode*  lambdaParams () {
	assert (type == MC_LAMBDA);
	assert (lambda.sexp->type == MC_CONS);
	return lambda.sexp->car ();
    };
    inline MNode*  lambdaBody () {
	assert (type == MC_LAMBDA);
	assert (lambda.sexp->type == MC_CONS);
	return lambda.sexp->cdr ();
    };
    inline uint32_t  lambdaMode () {
	assert (type == MC_LAMBDA);
	return lambda.mode;
    };
    inline bool  lambdaModeBit (uint32_t bit) {
	assert (type == MC_LAMBDA);
	return lambda.mode & bit;
    };
    MNode*  append_cdr (MNode* v);
    MNode*  vectorGet (size_t pos);
    size_t  vectorSize ();
    void  vectorPut (size_t pos, MNode* e);
    MNode*  vectorDel (size_t pos);
    MNode*  vectorDel (size_t from, size_t to);
    void  vectorUnshift (MNode* e);
    MNode*  vectorShift ();
    void  vectorPush (MNode* e);
    MNode*  vectorPop ();
    void  vectorResize (size_t pos);
    MNode*  tableGet (const ustring& name);
    void  tablePut (const ustring& name, MNode* e);
    MNode*  tableDel (const ustring& name);
    int64_t  to_int64 ();
    double  to_double ();
    MNode*  to_number();
    bool  to_bool ();
    ustring  sym_to_string ();
    ustring  cons_to_texp (bool fcdr = false);
    ustring  to_texp ();
    ustring  to_string ();
    ustring  vector_to_string ();
    ustring  table_to_string ();
    void  dump (std::ostream& o, bool c = false);
    ustring  dump_string (bool c = false);
    ustring  dump_string_short ();
};

MNode*  newMNode_sym (ustring* v);
MNode*  newMNode_str (ustring* v);
MNode*  newMNode_int64 (int64_t v);
MNode*  newMNode_num (int64_t v);
MNode*  newMNode_num (uint64_t v);
inline MNode*  newMNode_num(int32_t v) {return newMNode_num((int64_t)v);}
inline MNode*  newMNode_num(uint32_t v) {return newMNode_num((int64_t)v);}
inline MNode*  newMNode_num(long long v) {return newMNode_num((int64_t)v);}
MNode*  newMNode_num (double v);
MNode*  newMNode_bool (bool v);
MNode*  newMNode_cons (MNode* a);
MNode*  newMNode_cons (MNode* a, MNode* d);
MNode*  newMNode_symQuote ();
MNode*  newMNode_quote (MNode* v);
void  newMNodeCdr (MNode*& cell);
MNode*  newMNode_vector (MotorVector* vec);
MNode*  newMNode_vector ();
MNode*  newMNode_table (MotorVar* tbl);
MNode*  newMNode_table ();
MNode*  newMNode_libobj (MLFunc* fn);
MNode*  newMNode_builtin_fn (XFTableVal* fn);
MNode*  newMNode_builtin_mfn (XFTableVal* fn);
MNode*  newMNode_builtin_sfn (XFTableVal* fn, MLFunc* senv);

inline bool  isNil (MNode* a) {
    return (a == NULL || a->isNil ());
}
inline bool  isCons (MNode* a) {
    return (a != NULL && a->isCons ());
}
inline bool  isStr (MNode* a) {
    return (a != NULL && a->isStr ());
}
inline bool  isSym (MNode* a) {
    return (a != NULL && a->isSym ());
}
inline bool  isInt (MNode* a) {
    return (a != NULL && a->isInt ());
}
inline bool  isReal (MNode* a) {
    return (a != NULL && a->isReal ());
}
inline bool  isIntReal (MNode* a) {
    return (a != NULL && a->isIntReal ());
}
inline bool  isVector (MNode* a) {
    return (a != NULL && a->isVector ());
}
inline bool  isTable (MNode* a) {
    return (a != NULL && a->isTable ());
}
inline bool  isLambda (MNode* sexp) {
    return sexp && sexp->isLambda ();
}
inline bool  isBool (MNode* a) {
    return a && a->isBool ();
}
inline bool  isBuiltin (MNode* sexp) {
    if (sexp) {
	switch (sexp->type) {
	case MNode::MC_BUILTIN_FN:
	case MNode::MC_BUILTIN_MFN:
	case MNode::MC_BUILTIN_SFN:
	    return true;
	default:;
	}
    }
    return false;
}
inline const ustring*  ptr_symbol (MNode* e) {
    assert (isSym (e));
    return e->sym;
}
inline MotorVar*  ptr_table (MNode* e) {
    assert (isTable (e));
    return e->table;
}
inline bool  to_bool (const ustring& v) {
    return ! (v.length () == 0 || (v.length () == 1 && v[0] == '0'));
}
inline bool  to_bool (MNode* c) {
    if (c)
	return c->to_bool ();
    else
	return false;
}
inline int64_t  to_int64 (MNode* c) {
    if (c) {
	return c->to_int64 ();
    } else {
	return 0;
    }
}
inline double  to_double (MNode* c) {
    if (c)
	return c->to_double ();
    else
	return 0.;
}
inline MNode*  to_number (MNode* c) {
    if (c) {
	return c->to_number ();
    } else {
	return newMNode_int64 (0);
    }
}
inline ustring  to_string (MNode* c) {
    if (c)
	return c->to_string ();
    else
	return uEmpty;
}
inline ustring  to_text1 (MNode* c) {
    if (c)
	return omitCtrl (c->to_string ());
    else
	return uEmpty;
}
inline ustring  to_asciiword (MNode* c) {
    if (c)
	return omitNonAsciiWord (c->to_string ());
    else
	return uEmpty;
}
MNode*  list_to_vector (MNode* e);
inline ustring  dump_to_texp (MNode* c) {
    if (c)
	return c->to_texp ();
    else
	return uNil2;
}
#define dump_to_sexp(x)		dump_to_texp(x)

MNode*  quoted (MNode* v);

class  MNodePtr;
ustring  formatString (const ustring& format, boost::ptr_vector<MNodePtr>& par);

bool  eq (MNode* a, MNode* b);
bool  equal (MNode* a, MNode* b);
bool  neq (MNode* a, MNode* b);
bool  ngt (MNode* a, MNode* b);
bool  nge (MNode* a, MNode* b);

class  MNodePtr {
 public:
    MNode*  p;

    MNodePtr (): p (NULL) {};
    ~MNodePtr () {
	fdelete (p);
	p = NULL;
    };
    void  fdelete (MNode* x) {
	if (x) {
	    if (x->refcount <= 1)
		delete x;
	    else
		x->refcount --;
	}
    };
    MNode*  operator () () {
	return p;
    };
    MNode*  operator = (MNode* b) {
	MNode*  x = p;
	p = b;
	if (p)
	    p->refcount ++;
	fdelete (x);
	return p;
    };
    MNode*  operator = (MNodePtr& b) {
	MNode*  x = p;
	p = b ();
	if (p)
	    p->refcount ++;
	fdelete (x);
	return p;
    };
    MNode*  release () {
	MNode*  ans = p;
	p = NULL;
	if (ans)
	    ans->refcount --;
	return ans;
    };
    inline ustring  to_string () {
	return ::to_string (p);
    };
    inline ustring  to_asciiword () {
	return ::to_asciiword (p);
    };
    inline bool  to_bool () {
	return ::to_bool (p);
    };
};

class  MNodeList {
 public:
    MNodePtr  ans;
    MNode*  a;

    MNodeList () {
	a = NULL;
    };
    virtual  ~MNodeList () {};

    virtual void  append (MNode* v) {
	if (a) {
	    newMNodeCdr (a);
	} else {
	    ans = a = new MNode;
	}
	a->set_car (v);
    };
    virtual void  set_cdr_cut (MNode* v) {
	if (a) {
	    a->set_cdr (v);
	} else {
	    ans = v;
	}
    };
#if 0 // not tested
    virtual void  set_cdr (MNode* v) {
	set_cdr_cur (v);
	for (a = v; a && a->isCons () && a->cdr (); a = a->cdr ()) {}
    };
#endif
    virtual MNode*  operator () () {
	return ans ();
    };
    virtual MNode*  release () {
	return ans.release ();
    };
};

class  MotorTexp {
 public:
    typedef enum {
	YYNONE, YYPAR, YYREN, YYPAR2, YYREN2, YYPAR3, YYREN3,
	YYTEXT, YYSIM, YYINT, YYNUM, YYQUOTE, YYPERIOD,
    }  word_type;

    MNode  top;
    MlEnv*  mlenv;
    enum {
	S_NORMAL,
	S_MISSING_PAREN,
	S_SURPLUS_PAREN,
    }  status;

    MotorTexp (MlEnv* e) {
	mlenv = e;
	status = S_NORMAL;
    };
    virtual  ~MotorTexp () {};

    virtual void  scan (const ustring& text, bool skip = false);
    virtual void  skipHead (uiterator& b, uiterator& e, int& linenum);
    virtual void  scanWord (int& linenum, uiterator& b, uiterator& e, word_type& type, ustring*& ans);
    virtual ustring*  scanText (int& linenum, uiterator& b, uiterator& e);
    virtual void  skipBlank (int& linenum, uiterator& b, uiterator& e);
    virtual bool  scanCar (int& linenum, uiterator& b, uiterator& e, MNode* cell);
    virtual void  scanQuote (int& linenum, uiterator& b, uiterator& e, MNode* cell);
    virtual void  scanVector (int& linenum, uiterator& b, uiterator& e, MNode* cell);
    virtual void  scanTable (int& linenum, uiterator& b, uiterator& e, MNode* cell);
    virtual bool  scanTexp (int& linenum, uiterator& b, uiterator& e, MNode*& cell, bool qcdr, word_type closing);
    virtual bool  matchHex4 (uiterator& b, uiterator e);
};

class  MLFunc {
 public:
    int  id;
    MlEnv*  mlenv;

    MLFunc (int v, MlEnv* _mlenv): id (v), mlenv (_mlenv) {};
    virtual  ~MLFunc () {};
};
template <class T> inline T*  MObjRef (MLFunc* mobj, int id) {
    assert (mobj && mobj->id == id);
    return (T*)mobj;
}

void  nextNode (MNode*& arg);
void  nextNodeNonNil (MNode*& arg);

//#define  evkw(n,v)	(keywords[n] && ! isNil (v = eval (keywords[n], mlenv)))
//#define  evkw_bool(n,v)	(keywords[n] && (v = eval_bool (keywords[n], mlenv)))
//#define  evkw_int(n,v)	(keywords[n] && (v = eval_int (keywords[n], mlenv)))
//#define  evkw_str(n,v)	if (keywords[n]) v = eval_str (keywords[n], mlenv)

#endif /* ML_H */
