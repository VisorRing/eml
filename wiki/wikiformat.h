#ifndef WIKIFORMAT_H
#define WIKIFORMAT_H

#include "wikimotor.h"
#include "wikiattrib.h"
#include "wikitable.h"
#include "motoroutput.h"
#include "motorenv.h"
#include "mlenv.h"
#include "ftable.h"
#include "ustring.h"
#include "util_splitter.h"
#include "util_string.h"
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/unordered_map.hpp>
#include <vector>
#include <iostream>
#include <assert.h>

class  WikiBlock;
class  WikiFormat;

class  WikiMlEnv: public MlEnv {
 public:
    WikiMlEnv (MlPool* _mlpool, MlFTable* _mlftable): MlEnv (_mlpool, _mlftable) {};
    virtual  ~WikiMlEnv () {};

    virtual void  setVar (const ustring& name, MNode* val);
    virtual void  setAry (const ustring& name, size_t i, MNode* val);
    virtual void  setArySize (const ustring& name, size_t n);
    virtual void  setAry (const ustring& name, MNode* list);
    virtual MNode*  getVar (const ustring& name);
    virtual MNode*  getAry (const ustring& name, size_t i);
    virtual size_t  getArySize (const ustring& name);
    virtual void  setLocalVar (const ustring& name, MNode* val);
    virtual void  defineLocalVar (const ustring& name);
    virtual ustring  wikiVarName (const ustring& name);
};

class  WikiMotorOutputString: public MotorOutputString {
 public:
    WikiFormat*  wiki;

    WikiMotorOutputString (WikiFormat* _wiki): wiki (_wiki) {};
    virtual  ~WikiMotorOutputString () {};
    virtual void  flush (bool fsuper);
    virtual bool  isResponse () {
	return false;
    };
};

class  WikiLine {
 public:
    typedef boost::ptr_vector<WikiLine>  linevec;

    uiterator  begin0;
    uiterator  begin;
    uiterator  end;
    void  (*fn) (WikiLine* wl, WikiFormat* wiki);
    linevec*  block;
    WikiLine*  block2;
    bool  fsuper;

    WikiLine (uiterator b, uiterator e, bool _fsuper) {
	begin0 = begin = b;
	end = e;
	fn = NULL;
	block = NULL;
	block2 = NULL;
	fsuper = _fsuper;
    };
    WikiLine (uiterator b0, uiterator b, uiterator e, bool _fsuper) {
	begin0 = b0;
	begin = b;
	end = e;
	fn = NULL;
	block = NULL;
	block2 = NULL;
	fsuper = _fsuper;
    };
    virtual  ~WikiLine () {
	delete block;
	delete block2;
    };
    
};

class  WikiMacro {
 public:
    MNodePtr  vars;
    WikiLine::linevec*  wl;

#if 0
    WikiMacro (MNode* _vars, WikiLine::linevec* _wl) {
	vars = _vars;
	wl = _wl;
    };
#endif
    WikiMacro () {
	wl = NULL;
    };
    virtual  ~WikiMacro () {
//	delete vars;
//	vars = NULL;
	delete wl;
	wl = NULL;
    };
};

class  WikiLineScanner {
 public:
    WikiLine::linevec*  wlv;
    int  idx;

    WikiLineScanner (WikiLine::linevec* v) {
	assert (v);
	wlv = v;
	idx = -1;
    };
    virtual  ~WikiLineScanner () {};

    virtual WikiLine*  cur () {
	if (0 <= idx && idx < wlv->size ()) {
	    return &(*wlv)[idx];
	} else {
	    return NULL;
	}
    };
    virtual WikiLine*  next () {
	idx ++;
	return cur ();
    };
    virtual void  rollback () {
	if (idx >= 0)
	    idx --;
    };
    virtual bool  isEnd () {
	return idx >= wlv->size ();
    };
};

class  WikiBlock {
 public:
    typedef enum {
	BlockNone,
	BlockParagraph,
	BlockH,
	BlockPreformatRaw,
	BlockItemText,
	BlockItemUL,
	BlockItemOL,
	BlockItemNL,
	BlockItemDL,
	BlockTS,
	BlockTable,
	BlockSelect,
	BlockQuote,
	BlockDF,
	BlockDiv,
	BlockForm,
	BlockElement,
	BlockHR,
	BlockRaw,
	BlockBlank,
    }  blockType;
    typedef enum {
	CloseNA,
	CloseFalse,
	CloseTrue,
    }  closeType;

    blockType  type;
    WikiFormat*  wiki;

    WikiBlock (blockType t, WikiFormat* w): type (t), wiki (w) {};
    virtual  ~WikiBlock () {};

    virtual void  addLine (uiterator b, uiterator e) {
	assert (0);
    };
    virtual bool  nextLine (uiterator b, uiterator e) = 0;
    virtual closeType  closeLine (uiterator b, uiterator e) {
	return CloseNA;
    };
    virtual void  addLines (uiterator b, uiterator e) {
	assert (0);
    };
    virtual void  addHtml (const ustring& ht) {
	assert (0);
    };
    virtual void  close () {};
    virtual void  output (MotorOutput* out) = 0;
};

class  WikiBlockComplex: public WikiBlock {
 public:
    boost::ptr_vector<WikiBlock>  block;

    WikiBlockComplex (blockType t, WikiFormat* w): WikiBlock (t, w) {};
    virtual  ~WikiBlockComplex () {};
    virtual void  outputBlock (MotorOutput* out);
};

class  WikiBlockParagraph: public WikiBlock {
 public:
    int  count;
    bool  singleTag;
    bool  pflag;
    WikiMotorObjVec  objv;

    WikiBlockParagraph (WikiFormat* w): WikiBlock (BlockParagraph, w) {
	count = 0;
	singleTag = false;
	pflag = false;
    };
    virtual  ~WikiBlockParagraph () {};

    virtual bool  nextLine (uiterator b, uiterator e);
    virtual void  addLine (uiterator b, uiterator e);
    virtual void  addHtml (const ustring& ht);
    virtual void  output (MotorOutput* out);
};
inline WikiBlockParagraph*  WikiBlockParagraph_type (WikiBlock* b) {
    assert (b->type == WikiBlock::BlockParagraph);
    return (WikiBlockParagraph*)b;
}

class  WikiBlockH: public WikiBlockComplex {
 public:
    ustring  title;
    ustring  anchor;
    int  level0;
    int  level;

    WikiBlockH (WikiFormat* w): WikiBlockComplex (BlockH, w) {
	level0 = level = 0;
    };
    virtual  ~WikiBlockH () {};

    virtual bool  nextLine (uiterator b, uiterator e);
    virtual void  addLine (uiterator b, uiterator e);
    virtual closeType  closeLine (uiterator b, uiterator e);
    virtual void  close ();
    virtual void  output (MotorOutput* out);
    virtual void  outputBeginDiv (int lv, MotorOutput* out);
    virtual void  outputEndDiv (int lv, MotorOutput* out);
    virtual bool  checkEmpty ();
};

class  WikiBlockPreformatRaw: public WikiBlock {
 public:
    ustring  text;
    char  markup;

    WikiBlockPreformatRaw (WikiFormat* w): WikiBlock (BlockPreformatRaw, w) {
	markup = 0;
    };
    virtual  ~WikiBlockPreformatRaw () {};

    virtual bool  nextLine (uiterator b, uiterator e);
    virtual void  addLine (uiterator b, uiterator e);
    virtual void  output (MotorOutput* out);
};

class  WikiBlockItem;
class  WikiBlockItemText: public WikiBlockComplex {
 public:
    ustring  html;
    bool  indentHack;
    WikiAttribItem  attrib;
    WikiBlockItem*  parent;

    WikiBlockItemText (WikiFormat* w, WikiBlockItem* _p): WikiBlockComplex (BlockItemText, w), attrib (w, this) {
	indentHack = false;
	parent = _p;
    };
    virtual  ~WikiBlockItemText () {};
    virtual bool  nextLine (uiterator b, uiterator e);
    virtual void  addLine (uiterator b, uiterator e);
    virtual bool  checkAddLine (uiterator b, uiterator e);
    virtual void  output (MotorOutput* out);
};

class  WikiBlockItem: public WikiBlock {
 public:
    int  ch;
    boost::ptr_vector<WikiBlockItemText>  block;
    WikiAttrib1  attrib;

    WikiBlockItem (blockType t, WikiFormat* w): WikiBlock (t, w), attrib (w, WikiAttrib1::SEL_ID, false, WikiAttrib1::M_NORMAL) {
	assert (t == BlockItemUL || t == BlockItemOL || t == BlockItemNL);
	ch = 0;
	if (t == BlockItemOL)
	    attrib.selector |= WikiAttrib1::SEL_START;
    };
    virtual  ~WikiBlockItem () {};

    virtual bool  nextLine (uiterator b, uiterator e);
    virtual void  addLine (uiterator b, uiterator e);
    virtual void  output (MotorOutput* out);
    virtual void  outputBlock (MotorOutput* out);
};

class  WikiBlockItemDL: public WikiBlock {
 public:
    boost::ptr_vector<WikiMotorObjVec>  objv1;
    boost::ptr_vector<WikiMotorObjVec>  objv2;

    WikiBlockItemDL (WikiFormat* w): WikiBlock (BlockItemDL, w) {};
    virtual  ~WikiBlockItemDL () {};

    virtual bool  nextLine (uiterator b, uiterator e);
    virtual void  addLine (uiterator b, uiterator e);
    virtual void  output (MotorOutput* out);
};

class  WikiBlockTable: public WikiBlock {
 public:
    class  TableCell {
    public:
	ustring  html;
	boost::ptr_vector<WikiBlock>  block;
	WikiAttribTable  cellattrib;
	bool  fcolspan;
	bool  frowspan;
	int  rowspan;
	int  colspan;
	bool  spanmatch;

	TableCell (WikiFormat* _wiki, WikiAttrib1::mode_t _mode): cellattrib (_wiki, WikiAttribTable::SEL_TD, _mode) {
	    fcolspan = false;
	    frowspan = false;
	    rowspan = 1;
	    colspan = 1;
	    spanmatch = false;
	};
	virtual  ~TableCell () {};
	virtual void  cellBody (WikiMotorObjVec* vtext, WikiBlockTable* table, int idx);
	virtual void  outputTD (WikiFormat* wiki, MotorOutput* out, bool fturn);
	virtual void  outputTDe (MotorOutput* out);
    };

    class  CellList_t: public boost::ptr_vector<TableCell> {
    public:
	WikiAttribTable  rowattrib;

	CellList_t (WikiFormat* _wiki): rowattrib (_wiki, WikiAttribTable::SEL_TR, WikiAttrib1::M_ATTRIB) {};
	virtual  ~CellList_t () {};
    };
    typedef boost::ptr_vector<CellList_t>  TableAry_t;

    int  n;
    WikiAttribTable  attrib;
    bool  cont;
    int  ccont;
    CellList_t  defaultList;
    TableAry_t  ary;
    ustring  captionHtml;

    WikiBlockTable (WikiFormat* _wiki): WikiBlock (BlockTable, _wiki), attrib (_wiki, WikiAttribTable::SEL_TABLE, WikiAttrib1::M_ATTRIB_TEXT), defaultList (_wiki) {
	n = 0;
	cont = false;
	ccont = 0;
    };
    virtual  ~WikiBlockTable () {};

    virtual bool  nextLine (uiterator b, uiterator e);
    virtual void  addLine (uiterator b, uiterator e);
    virtual closeType  closeLine (uiterator b, uiterator e);
    virtual void  output (MotorOutput* out);
    virtual void  addLine_head (uiterator b, uiterator e);
    virtual void  addLine_body (uiterator b, uiterator e);
    virtual TableCell*  newCell (int idx);
    virtual TableCell*  newCell (WikiAttribTable& rowattrib);
    virtual void  normalize ();
    virtual void  outputTableTag (MotorOutput* out);
    virtual void  outputTBody (MotorOutput* out);
    virtual void  outputTBodyCell (WikiFormat* wiki, MotorOutput* out, TableCell* cell);
};

class  WikiBlockSelect: public WikiBlock {
 public:
    class  SelectItem {
    public:
	ustring  label;
	ustring  value;
	bool  fvalue;
	bool  fselect;

	SelectItem (): fvalue (false), fselect (false) {};
	virtual  ~SelectItem () {};
    };

    WikiMotorObjVecVec  head;
    int  n;
    ustring  name;
    WikiAttribInput  attrib;
    std::vector<SelectItem>  item;

    WikiBlockSelect (WikiFormat* w): WikiBlock (BlockSelect, w), attrib (w, WikiAttribInput::SEL_ELSIZE) {
	attrib.selector |= WikiAttrib1::SEL_DEFAULT2 | WikiAttrib1::SEL_MULTIPLE2 | WikiAttrib1::SEL_SCRIPT2;
	n = 0;
    };
    virtual  ~WikiBlockSelect () {};

    virtual bool  nextLine (uiterator b, uiterator e);
    virtual void  addLine (uiterator b, uiterator e);
    virtual void  addLine_head (uiterator b, uiterator e);
    virtual void  addLine_body (uiterator b, uiterator e);
    virtual void  close ();
    virtual void  output (MotorOutput* out);
};

class  WikiBlockQuote: public WikiBlockComplex {
 public:
    WikiBlockQuote (WikiFormat* w): WikiBlockComplex (BlockQuote, w) {};
    virtual  ~WikiBlockQuote () {};

    virtual bool  nextLine (uiterator b, uiterator e);
    virtual closeType  closeLine (uiterator b, uiterator e);
    virtual void  addLine (uiterator b, uiterator e);
    virtual void  output (MotorOutput* out);
};

class  WikiBlockDiv: public WikiBlockComplex {
 public:
    WikiAttrib1  attrib;
    bool  nopflag;

    WikiBlockDiv (WikiFormat* w): WikiBlockComplex (BlockDiv, w), attrib (w, WikiAttrib1::SEL_CLASS2 | WikiAttrib1::SEL_DIRECT, false, WikiAttrib1::M_ATTRIB) {
	nopflag = false;
    };
    virtual  ~WikiBlockDiv () {};

    virtual bool  nextLine (uiterator b, uiterator e);
    virtual void  addLine (uiterator b, uiterator e);
    virtual closeType  closeLine (uiterator b, uiterator e);
    virtual void  output (MotorOutput* out);
};

class  WikiBlockForm: public WikiBlockComplex {
 public:
    enum {
	M_NONE,
	M_GET,
	M_POST,
    }  method;
    WikiAttrib1  attrib;
    ustring  url;
    bool  fscript;
    bool  qfileform;

    WikiBlockForm (WikiFormat* w): WikiBlockComplex (BlockForm, w), attrib (w, WikiAttrib1::SEL_TARGET, true, WikiAttrib1::M_ATTRIB) {
	method = M_NONE;
	fscript = false;
	qfileform = false;
    };
    virtual  ~WikiBlockForm () {};

    virtual bool  nextLine (uiterator b, uiterator e);
    virtual void  addLine (uiterator b, uiterator e);
    virtual closeType  closeLine (uiterator b, uiterator e);
    virtual void  output (MotorOutput* out);
};

class  WikiBlockElement: public WikiBlockComplex {
 public:
    ustring  name;

    WikiBlockElement (WikiFormat* w): WikiBlockComplex (BlockElement, w) {};
    virtual  ~WikiBlockElement () {};

    virtual bool  nextLine (uiterator b, uiterator e);
    virtual void  addLine (uiterator b, uiterator e);
    virtual closeType  closeLine (uiterator b, uiterator e);
    virtual void  output (MotorOutput* out);
};

class  WikiBlockElementMap: public boost::unordered_map<ustring, WikiBlockElement*> {
 public:
    WikiBlockElementMap () {};
    virtual  ~WikiBlockElementMap () {};

    WikiBlockElement*  get (const ustring& name) {
	iterator  it = find (name);
	if (it == end ()) {
	    return NULL;
	} else {
	    return it->second;
	}
    };
    WikiBlockElement*  put (const ustring& name, WikiBlockElement* e) {
	std::pair<iterator, bool>  x;
	x = insert (value_type (name, e));
	x.first->second = e;
	return e;
    };
    WikiBlockElement*  del (const ustring& name) {
	assert (0);
    };
};

class  WikiBlockHR: public WikiBlock {
 public:
    WikiBlockHR (WikiFormat* w): WikiBlock (BlockHR, w) {};
    virtual  ~WikiBlockHR () {};

    virtual bool  nextLine (uiterator b, uiterator e);
    virtual void  addLine (uiterator b, uiterator e);
    virtual void  output (MotorOutput* out);
};

class  WikiBlockRaw: public WikiBlock {
 public:
    ustring  text;

    WikiBlockRaw(WikiFormat* w): WikiBlock (BlockRaw, w) {};
    virtual  ~WikiBlockRaw () {};

    virtual bool  nextLine (uiterator b, uiterator e);
    virtual void  addLine (uiterator b, uiterator e);
    virtual void  output (MotorOutput* out);
};

class  WikiBlockBlank: public WikiBlock {
 public:
    int  count;
    bool  enable;

    WikiBlockBlank (WikiFormat* w): WikiBlock (BlockBlank, w) {
	count = 0;
	enable = false;
    };
    virtual  ~WikiBlockBlank () {};

    virtual bool  nextLine (uiterator b, uiterator e);
    virtual void  addLine (uiterator b, uiterator e);
    virtual void  output (MotorOutput* out);
};
inline WikiBlockBlank*  WikiBlockBlank_type (WikiBlock* b) {
    assert (b->type == WikiBlock::BlockBlank);
    return (WikiBlockBlank*)b;
}

class  WikiFormat {
 public:
    typedef  enum {
	M_NONE,
	M_URL,
    }  ipMode_t;
    class  DepthCount {
    public:
	WikiFormat*  wiki;
	DepthCount (WikiFormat* _wiki) {
	    wiki = _wiki;
	    wiki->depth ++;
	};
	virtual  ~DepthCount () {
	    wiki->depth --;
	    assert (wiki->depth >= 0);
	};
	virtual bool  test () {
	    return wiki->depth >= 32;
	};
    };

    WikiBlock*  cur;
    WikiBlockForm*  curform;
    int  hlevel;
    int  headbase;
    MotorEnv*  env;
    MlEnv*  mlenv;
    MlFTable  mlFTable;
    ustring  errorMsg;
    boost::ptr_vector<WikiBlock>  block;
    boost::ptr_vector<WikiBlock>*  blockp;
    std::vector<WikiBlock*>  bstack;
    std::vector<boost::ptr_vector<WikiBlock>*>  pstack;
    WikiBlockElementMap  elementMap;
    bool  protectMode;
    ipMode_t  ipMode;
    int  depth;
    bool  motorPre;

    WikiFormat (MotorEnv* e, bool mode) {
	cur = NULL;
	curform = NULL;
	hlevel = 0;
	headbase = 0;
	env = e;
	mlenv = new WikiMlEnv (env->mlenv->mlPool, &mlFTable);
	mlenv->env = env;
	if (env->log)
	    mlenv->log = env->log;
	else
	    mlenv->log = &std::cerr;
	mlenv->setStartTime ();
	mlenv->resetProg ();
	mlFTable.setFTable (&GWikiFTable, NULL);
	blockp = &block;
	protectMode = mode;
	ipMode = M_NONE;
	depth = 0;
#ifdef WIKIMOTORPRE
	motorPre = true;
#else
	motorPre = false;
#endif
    };
    virtual  ~WikiFormat () {
	delete mlenv;
    };

    virtual void  compile (const ustring& text, bool fsuper);
    virtual void  compileElement (const ustring& text, bool fsuper, const ustring& elementName);
    virtual void  output (boost::ptr_vector<WikiBlock>& ary);
    virtual void  output () {
	output (block);
    };
    virtual void  outputElement (const ustring& elementName);
    virtual void  pass1 (const ustring& text, WikiLine::linevec* block, bool fsuper);
    virtual int  pass1_1 (Splitter& sp, ustring* elseword, ustring* endword, WikiLine::linevec* block, uiterator* elsebegin0, uiterator* elsebegin, uiterator* elseend, bool fsuper);
    virtual bool  pass1_2 (Splitter& sp, uiterator& b, uiterator& e, uiterator& t, uiterator& u, uiterator& v, WikiLine::linevec* block, bool fsuper);
    virtual void  errorOutput ();

    virtual bool  checkClose (uiterator b, uiterator e);
    virtual void  compileLine (WikiLineScanner& scanner);
    virtual void  compileLines (WikiLineScanner& scanner, bool (*fn)(WikiLine& spp, WikiLineScanner& scanner, WikiFormat* wiki, void* par) = NULL, void* par = NULL);
    virtual void  pushBlockRaw (uiterator b, uiterator e);
    virtual void  push_block (boost::ptr_vector<WikiBlock>* b) {
	bstack.push_back (cur);
	cur = NULL;
	pstack.push_back (blockp);
	blockp = b;
    };
    virtual void  pop_block () {
	if (cur)
	    cur->close ();
	cur = bstack.back ();
	bstack.pop_back ();
	blockp = pstack.back ();
	pstack.pop_back ();
    };
    virtual int  countWikiH (uiterator& b, uiterator e);
    virtual void  wikiMotor (uiterator b, uiterator e, WikiMotorObjVec& ans);
    virtual ustring  wikiMotor (uiterator b, uiterator e);
    virtual void  wikiMotorInline (uiterator b, uiterator e, WikiMotorObjVec& ans);
    virtual void  outputName (MotorOutput* out, const char* name, size_t len, const ustring& val, bool cond = true);
    virtual void  outputName (MotorOutput* out, const char* name, size_t len, long val, bool cond = true);
    virtual void  outputStyle (MotorOutput* out, const ustring& style);
    virtual void  outputFlag (MotorOutput* out, const char* name, size_t len, bool flag);
    virtual void  outputID (MotorOutput* out, const ustring& id);
    virtual void  outputClass (MotorOutput* out, std::vector<ustring>& classes);
    virtual void  outputSubmitScript (MotorOutput* out, const char* name, size_t len, const ustring& onclick, bool scriptcut);
    virtual void  outputNum (MotorOutput* out, const char* name, size_t len, int val);
    virtual MNode*  getVar (const ustring& name) {
	return mlenv->getVar (name);
    };
    virtual ustring  getVar_string (const ustring& name) {
	return mlenv->getVar_string (name);
    };
    virtual ustring  getAry_string (const ustring& name, size_t i) {
	return mlenv->getAry_string (name, i);
    };
    virtual size_t  getArySize (const ustring& name) {
	return mlenv->getArySize (name);
    };
    virtual MNode*  arrayToTexp (const ustring& name);
//    virtual MNode*  evalVar (const ustring& name);
    virtual MNode*  buildArgs (bool fnodec, WikiMotorObjVecVec::const_iterator b, WikiMotorObjVecVec::const_iterator e);
    virtual void  logLispFunctionError (const ustring& msg, const ustring& cmd);
};

extern ustring  uWiki;
extern ustring  uAWiki;

#endif /* WIKIFORMAT_H */
