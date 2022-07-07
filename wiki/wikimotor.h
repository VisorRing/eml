#ifndef WIKIMOTOR_H
#define WIKIMOTOR_H

#include "wikitable.h"
//#include "wikienv.h"
#include "motor.h"
#include "motoroutput.h"
#include "ml.h"
#include "util_splitter.h"
#include "util_string.h"
#include "ustring.h"
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/shared_ptr.hpp>
#include <vector>

class  WikiFormat;
class  WikiMotorObj;
typedef  boost::shared_ptr<WikiMotorObj>  WikiMotorObjPtr;

class  WikiMotorObjVec: public std::vector<WikiMotorObjPtr> {
 public:
    WikiMotorObjVec () {};
    virtual  ~WikiMotorObjVec () {};

    virtual void  eval (WikiMotorObjVec& ans, WikiFormat* wiki);
    virtual void  out (MotorOutputString& o, WikiFormat* wiki);
    virtual ustring  textOut (WikiFormat* wiki);
    virtual MNode*  toMNode (WikiFormat* wiki);
    virtual void  out_HTML (MotorOutputString& o, WikiFormat* wiki);
    virtual ustring  htmlOut (WikiFormat* wiki);
    virtual ustring  dump ();
    virtual void  trimSpc ();
    virtual bool  match (const char* t, size_t s);
    virtual bool  match (const char* t1, size_t s1, const char* t2, size_t s2);
    virtual bool  match (const ustring& t);
    virtual bool  match (const ustring& t1, const ustring& t2);
    virtual bool  matchHead (const char* t, size_t s);
    virtual bool  regmatch (const uregex& re);
    virtual void  splitCharA (ustring::value_type ch, WikiMotorObjVecVec& out);
    virtual bool  splitChar_keyword (ustring::value_type ch, ustring& name, WikiMotorObjVec& vvalue);
    virtual ustring::value_type  splitChar_keyword (ustring::value_type ch1, ustring::value_type ch2, ustring& name, WikiMotorObjVec& vvalue);
    virtual bool  splitChar_keyword (WikiFormat* wiki, ustring::value_type ch, ustring& name, ustring& value);
    virtual bool  splitChar (WikiFormat* wiki, ustring::value_type ch, ustring& name, ustring& value);
    virtual bool  splitChar (ustring::value_type ch, WikiMotorObjVec& out1, WikiMotorObjVec& out2);
    virtual bool  splitURL (WikiFormat* wiki, ustring& proto, ustring& host, ustring& path, ustring& params, ustring& anchor);
    virtual bool  splitURL (WikiFormat* wiki, ustring& url);
    virtual bool  splitURL_2 (WikiFormat* wiki, ustring& host, ustring& path, ustring& params, ustring& anchor);
    virtual bool  splitURL_2 (WikiFormat* wiki, const ustring& proto, ustring& url);
    virtual bool  splitURL_3 (WikiFormat* wiki, ustring& port, ustring& path, ustring& params, ustring& anchor);
    virtual bool  splitURL_3 (WikiFormat* wiki, const ustring& protohost, ustring& url);
    virtual bool  splitURLPath (WikiFormat* wiki, ustring& path, ustring& params, ustring& anchor);
    virtual bool  splitURLPath (WikiFormat* wiki, ustring& url);
};
typedef  boost::shared_ptr<WikiMotorObjVec>  WikiMotorObjVecPtr;

class  WikiMotorObjVecVec: public std::vector<WikiMotorObjVecPtr> {
 public:
    WikiMotorObjVecVec () {};
    virtual  ~WikiMotorObjVecVec () {};

    virtual void  eval (WikiMotorObjVec& ans, WikiFormat* wiki);
    virtual void  out (MotorOutputString& o, WikiFormat* wiki);
    virtual ustring  textOut (WikiFormat* wiki);
    virtual void  out_HTML (MotorOutputString& o, WikiFormat* wiki);
    virtual ustring  htmlOut (WikiFormat* wiki);
    virtual ustring  dump ();
    virtual void  join (size_t start, const ustring& ch, WikiMotorObjVec& ans);
};

class  MotorOutputWiki: public MotorOutput {
 public:
    WikiMotorObjVec*  ans;
    
    MotorOutputWiki (WikiMotorObjVec& _ans) {
	ans = &_ans;
    };
    virtual  ~MotorOutputWiki () {};
    virtual MotorOutput*  outamp (const ustring& t);
    virtual MotorOutput*  outamp_br (const ustring& t);
//    virtual MotorOutput*  outamp_wbr (const ustring& t);
    virtual MotorOutput*  outamp_nw (const ustring& t);
    virtual MotorOutput*  outamp_c3 (const ustring& t);
 protected:
    virtual MotorOutput*  outamp (uiterator b, uiterator e, uregex* re);
    virtual MotorOutput*  out (const ustring::value_type* s, size_t len);
 public:
    virtual bool  isResponse () {
	return false;
    };
};

class  WikiMotorObj {
 public:
    typedef enum {
	opt_normal,
	opt_br,
	opt_nbsp,
	opt_c3,
//	opt_wbr,
    }  vopt_t;

    enum  objType {
	wiki_text,
	wiki_char,
	wiki_emph,
	wiki_br,
	wiki_zwsp,
	wiki_var,
	wiki_table,
	wiki_func1,
	wiki_funcM,
	wiki_funcM2,
	wiki_funcLink,
	wiki_cond,
	wiki_html,
	wiki_mnode,
	wiki_deleted,
    }  type;

    WikiMotorObj (objType t): type (t) {};
    virtual  ~WikiMotorObj () {
	type = wiki_deleted;
    };
    virtual void  eval (WikiMotorObjVec& ans, const WikiMotorObjPtr* self, WikiFormat* wiki) {
//	ans.push_back (WikiMotorObjPtr (this));
	ans.push_back (*self);
    };
    virtual ustring  textOut (WikiFormat* wiki) = 0;
    virtual ustring  htmlOut (WikiFormat* wiki);
    virtual MNode*  toMNode (WikiFormat* wiki) {
	return newMNode_str (new ustring (textOut (wiki)));
    };
    virtual ustring  dump () = 0;
    virtual bool  operator == (const ustring& b) {
	return false;
    };
    virtual bool  match (const char* t, size_t s) {
	return false;
    };
    virtual bool  matchHead (const char* t, size_t s) {
	return false;
    };
    virtual bool  regmatch (const uregex& re) {
	return false;
    };
    virtual ustring  typeName () = 0;
};

class  WikiMotorObjText: public WikiMotorObj {
 public:
    ustring  text;

    WikiMotorObjText (uiterator b, uiterator e): WikiMotorObj (wiki_text) {
	text.assign (b, e);
    };
    WikiMotorObjText (ustring t): WikiMotorObj (wiki_text) {
	text.assign (t);
    };
    virtual  ~WikiMotorObjText () {};
    virtual ustring  textOut (WikiFormat* wiki);
    virtual ustring  dump ();
    virtual void  trimLeadingSpc ();
    virtual void  trimTrailingSpc ();
    virtual bool  operator == (const ustring& b) {
	return text == b;
    };
    virtual bool  match (const char* t, size_t s) {
	return ::match (text, t, s);
    };
    virtual bool  matchHead (const char* t, size_t s) {
	return ::matchHead (text, t, s);
    };
    virtual bool  regmatch (const uregex& re) {
	umatch  m;
	return usearch (text, m, re);
    };
    virtual ustring  typeName () {
	return ustring (CharConst ("text"));
    };
};
inline WikiMotorObjText*  WikiMotorObjText_type (WikiMotorObj* obj) {
    assert (obj->type == WikiMotorObj::wiki_text);
    return (WikiMotorObjText*)obj;
}

class  WikiMotorObjChar: public WikiMotorObj {
 public:
    ustring  text;

    WikiMotorObjChar (uiterator b, uiterator e): WikiMotorObj (wiki_char) {
	text.assign (b, e);
    };
    virtual  ~WikiMotorObjChar () {};
    virtual ustring  textOut (WikiFormat* wiki);
    virtual ustring  htmlOut (WikiFormat* wiki);
    virtual ustring  dump ();
    virtual ustring  typeName () {
	return ustring (CharConst ("char"));
    };
};

class  WikiMotorObjEmph: public WikiMotorObj {
 public:
    WikiMotorObjVec  text;
    int  level;

    WikiMotorObjEmph (): WikiMotorObj (wiki_emph) {
	level = 0;
    };
    virtual  ~WikiMotorObjEmph () {};
    virtual ustring  textOut (WikiFormat* wiki);
    virtual ustring  htmlOut (WikiFormat* wiki);
    virtual ustring  dump ();
    virtual ustring  typeName () {
	return ustring (CharConst ("emph"));
    };
};

class  WikiMotorObjBR: public WikiMotorObj {
 public:
    WikiMotorObjBR (): WikiMotorObj (wiki_br) {};
    virtual  ~WikiMotorObjBR () {};
    virtual ustring  textOut (WikiFormat* wiki);
    virtual ustring  htmlOut (WikiFormat* wiki);
    virtual ustring  dump ();
    virtual ustring  typeName () {
	return ustring (CharConst ("br"));
    };
};

class  WikiMotorObjZWSP: public WikiMotorObj {
 public:
    WikiMotorObjZWSP (): WikiMotorObj (wiki_br) {};
    virtual  ~WikiMotorObjZWSP () {};
    virtual ustring  textOut (WikiFormat* wiki);
    virtual ustring  htmlOut (WikiFormat* wiki);
    virtual ustring  dump ();
    virtual ustring  typeName () {
	return ustring (CharConst ("zwsp"));
    };
};

class  WikiMotorObjVar: public WikiMotorObj {
 public:
    ustring  name;
    vopt_t  vopt;

    WikiMotorObjVar (uiterator b, uiterator e, vopt_t op): WikiMotorObj (wiki_var) {
	name.assign (b, e);
	vopt = op;
    };
    virtual  ~WikiMotorObjVar () {};
    virtual void  eval (WikiMotorObjVec& ans, const WikiMotorObjPtr* self, WikiFormat* wiki);
    virtual ustring  textOut (WikiFormat* wiki);
    virtual ustring  htmlOut (WikiFormat* wiki);
    virtual MNode*  toMNode (WikiFormat* wiki);
    virtual ustring  dump ();
    virtual ustring  typeName () {
	return ustring (CharConst ("var"));
    };
};

class  WikiMotorObjTab: public WikiMotorObj {
 public:
    ustring  name;
    WikiMotorObjVecVec  arg;
    vopt_t  vopt;

    WikiMotorObjTab (const ustring& _name): WikiMotorObj (wiki_table) {
	name = _name;
	vopt = opt_normal;
    };
    virtual  ~WikiMotorObjTab () {};
    virtual void  setVopt (vopt_t _vopt) {
	vopt = _vopt;
    };
    virtual void  eval (WikiMotorObjVec& ans, const WikiMotorObjPtr* self, WikiFormat* wiki);
    virtual ustring  textOut (WikiFormat* wiki);
    virtual ustring  htmlOut (WikiFormat* wiki);
    virtual MNode*  toMNode (WikiFormat* wiki);
    virtual ustring  dump ();
    virtual ustring  typeName () {
	return ustring (CharConst ("table"));
    };
};

class  WikiMotorObjFunc: public WikiMotorObj {
 public:
    WikiCmdTableSupport::wikifunc_t*  func;
    MNodePtr  mfunc;
    ustring  name;

    WikiMotorObjFunc (objType t, const ustring& fn, WikiCmdTableSupport::wikifunc_t* fp): WikiMotorObj (t) {
	func = fp;
	mfunc = NULL;
	name = fn;
    };
    WikiMotorObjFunc (objType t, const ustring& fn, MNode* fp): WikiMotorObj (t) {
	func = NULL;
	mfunc = fp;
	name = fn;
    };
    virtual  ~WikiMotorObjFunc () {};
    virtual ustring  execDefun (MNode* vargs, WikiFormat* wiki);
    virtual void  eval (WikiMotorObjVec& ans, const WikiMotorObjPtr* self, WikiFormat* wiki) = 0;
    virtual void  evalFVal (const ustring& text, WikiMotorObjVec& ans, WikiFormat* wiki);
    virtual ustring  textOut (WikiFormat* wiki);
    virtual ustring  htmlOut (WikiFormat* wiki);
    virtual MNode*  toMNode (WikiFormat* wiki);
};

class  WikiMotorObjFunc1: public WikiMotorObjFunc {
 public:
    WikiMotorObjVec  arg;

    WikiMotorObjFunc1 (const ustring& fn, WikiCmdTableSupport::wikifunc_t* fp): WikiMotorObjFunc (wiki_func1, fn, fp) {};
    WikiMotorObjFunc1 (const ustring& fn, MNode* fp): WikiMotorObjFunc (wiki_func1, fn, fp) {};
    virtual  ~WikiMotorObjFunc1 () {};
    virtual ustring  execDefunArgs (WikiFormat* wiki);
    virtual void  eval (WikiMotorObjVec& ans, const WikiMotorObjPtr* self, WikiFormat* wiki);
    virtual ustring  dump ();
    virtual ustring  typeName () {
	return ustring (CharConst ("func1"));
    };
};

class  WikiMotorObjFuncM: public WikiMotorObjFunc {
 public:
    WikiMotorObjVecVec  arg;

    WikiMotorObjFuncM (const ustring& fn, WikiCmdTableSupport::wikifunc_t* fp): WikiMotorObjFunc (wiki_funcM, fn, fp) {};
    WikiMotorObjFuncM (const ustring& fn, MNode* fp): WikiMotorObjFunc (wiki_funcM, fn, fp) {};
    virtual  ~WikiMotorObjFuncM () {};
    virtual ustring  execDefunArgs (WikiFormat* wiki);
    virtual void  eval (WikiMotorObjVec& ans, const WikiMotorObjPtr* self, WikiFormat* wiki);
    virtual ustring  dump ();
    virtual ustring  typeName () {
	return ustring (CharConst ("funcM"));
    };
};

class  WikiMotorObjFuncM2: public WikiMotorObjFunc {
 public:
    WikiMotorObjVecVec  arg;
    WikiMotorObjVec  arg2;

    WikiMotorObjFuncM2 (const ustring& fn, WikiCmdTableSupport::wikifunc_t* fp): WikiMotorObjFunc (wiki_funcM2, fn, fp) {};
    WikiMotorObjFuncM2 (const ustring& fn, MNode* fp): WikiMotorObjFunc (wiki_funcM2, fn, fp) {};
    virtual  ~WikiMotorObjFuncM2 () {};
    virtual ustring  execDefunArgs (WikiFormat* wiki);
    virtual void  eval (WikiMotorObjVec& ans, const WikiMotorObjPtr* self, WikiFormat* wiki);
    virtual ustring  dump ();
    virtual ustring  typeName () {
	return ustring (CharConst ("funcM2"));
    };
};

class  WikiMotorObjFuncLink: public WikiMotorObjFunc {
 public:
    WikiMotorObjVecVec  arg;

    WikiMotorObjFuncLink (const ustring& fn, WikiCmdTableSupport::wikifunc_t* fp): WikiMotorObjFunc (wiki_funcLink, fn, fp) {};
    WikiMotorObjFuncLink (const ustring& fn, MNode* fp): WikiMotorObjFunc (wiki_funcLink, fn, fp) {};
    virtual  ~WikiMotorObjFuncLink () {};
    virtual ustring  execDefunArgs (WikiFormat* wiki);
    virtual void  eval (WikiMotorObjVec& ans, const WikiMotorObjPtr* self, WikiFormat* wiki);
    virtual ustring  dump ();
    virtual ustring  typeName () {
	return ustring (CharConst ("funcLink"));
    };
};
inline WikiMotorObjFuncLink*  WikiMotorObjFuncLink_type (WikiMotorObj* obj) {
    assert (obj->type == WikiMotorObj::wiki_funcLink);
    return (WikiMotorObjFuncLink*)obj;
}

class  WikiMotorObjCond: public WikiMotorObj {
 public:
    WikiMotorObjPtr  var;
    WikiMotorObjVec  value;
    bool  fneg;
    WikiMotorObjVec  trueobjs;
    WikiMotorObjVec  falseobjs;

    WikiMotorObjCond (bool _neg): WikiMotorObj (wiki_cond) {
	fneg = _neg;
    };
    virtual  ~WikiMotorObjCond () {};
    virtual void  eval (WikiMotorObjVec& ans, const WikiMotorObjPtr* self, WikiFormat* wiki);
    virtual ustring  textOut (WikiFormat* wiki);
    virtual ustring  htmlOut (WikiFormat* wiki);
    virtual ustring  dump ();
    virtual ustring  typeName () {
	return ustring (CharConst ("cond"));
    };
};

class  WikiMotorObjHtml: public WikiMotorObj {
 public:
    ustring  html;

    WikiMotorObjHtml (uiterator b, uiterator e): WikiMotorObj (wiki_html) {
	html.assign (b, e);
    };
    WikiMotorObjHtml (ustring t): WikiMotorObj (wiki_html) {
	html.assign (t);
    };
    virtual  ~WikiMotorObjHtml () {};
    virtual ustring  textOut (WikiFormat* wiki) {
	return html;
    };
    virtual ustring  htmlOut (WikiFormat* wiki) {
	return html;
    };
    virtual ustring  dump () {
	return html;
    };
    virtual bool  operator == (const ustring& b) {
	return html == b;
    };
    virtual bool  match (const char* t, size_t s) {
	return ::match (html, t, s);
    };
    virtual bool  matchHead (const char* t, size_t s) {
	return ::matchHead (html, t, s);
    };
    virtual bool  regmatch (const uregex& re) {
	umatch  m;
	return usearch (html, m, re);
    };
    virtual ustring  typeName () {
	return ustring (CharConst ("html"));
    };
};
inline WikiMotorObjHtml*  WikiMotorObjHtml_type (WikiMotorObj* obj) {
    assert (obj->type == WikiMotorObj::wiki_html);
    return (WikiMotorObjHtml*)obj;
}

class  WikiMotorObjMNode: public WikiMotorObj {
public:
    MNodePtr  node;

    WikiMotorObjMNode (MNode* _node): WikiMotorObj (wiki_mnode) {
	node = _node;
    };
    virtual  ~WikiMotorObjMNode () {};
    virtual ustring  textOut (WikiFormat* wiki);
    virtual MNode*  toMNode (WikiFormat* wiki) {
	return node ();
    };
    virtual ustring  dump ();
    virtual ustring  typeName () {
	return ustring (CharConst ("mnode"));
    };

};
inline WikiMotorObjMNode*  WikiMotorObjMNode_type (WikiMotorObj* obj) {
    assert (obj->type == WikiMotorObj::wiki_mnode);
    return (WikiMotorObjMNode*)obj;
}

extern uregex  re_wiki1;
class  WikiMotor {
 public:
    static const int TMATCH_NONE = 0;
    static const int TMATCH_CKET = 0x01;
    static const int TMATCH_COLN = 0x02;
//    static const int TMATCH_SPC = 0x04;
    static const int TMATCH_QUOT = 0x08;
    static const int TMATCH_BAR2 = 0x10;
    static const int TMATCH_BAR2ELSE = 0x20;
    static const int TMATCH_VAROPT = 0x40;
    static const int TMATCH_SELECT = 0x80;

    WikiFormat*  wiki;
    SplitterRe  sp;
    boost::ptr_vector<WikiMotorObj>  z;

    WikiMotor (uiterator b, uiterator e, WikiFormat* w): sp (b, e, re_wiki1) {
	wiki = w;
    };
    virtual  ~WikiMotor () {};
    virtual void  compile (WikiMotorObjVec& out, int tmatch = TMATCH_NONE);
 private:
    virtual void  compile_text (WikiMotorObjVec& out);
    virtual void  compile_2 (WikiMotorObjVec& out);
    virtual void  compile_3 (WikiMotorObjVec& out, const ustring& name);
    virtual bool  compile_4 (WikiMotorObjVec& out, const ustring& name);
    virtual void  compile_5 (WikiMotorObjVec& out, int nchar);
    virtual void  compile_6 (WikiMotorObjVec& out, const uiterator& b, const uiterator& e, bool fneg);
    virtual void  compile_6 (WikiMotorObjVec& out, WikiMotorObjPtr varptr, bool fneg);
    virtual void  compile_7 (WikiMotorObjVec& out, const ustring& name);
    virtual void  compile_8 (WikiMotorObjVec& out, const ustring& name);
};

void  join (WikiMotorObjVecVec::const_iterator b, WikiMotorObjVecVec::const_iterator e, const ustring& ch, WikiMotorObjVec& ans);

#endif /* WIKIMOTOR_H */
