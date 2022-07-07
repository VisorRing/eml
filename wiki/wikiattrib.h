#ifndef WIKIATTRIB_H
#define WIKIATTRIB_H

#include "wikimotor.h"
#include "ustring.h"
#include <vector>

class  WikiFormat;
class  MotorOutput;

class  WikiAttrib1 {
 public:
    static const uint32_t  SEL_ID        = 0x0000;
    static const uint32_t  SEL_TARGET    = 0x0004;
//    static const uint32_t  SEL_ELSIZE    = 0x0008;
    static const uint32_t  SEL_CLASS2    = 0x0010;
    static const uint32_t  SEL_TARGET2   = 0x0020;
    static const uint32_t  SEL_ONCLICK2  = 0x0040;
    static const uint32_t  SEL_DEFAULT2  = 0x0080;
    static const uint32_t  SEL_MULTIPLE2 = 0x0100;
    static const uint32_t  SEL_SCRIPT2   = 0x0200;
    static const uint32_t  SEL_START     = 0x0400;
    static const uint32_t  SEL_DIRECT    = 0x0800;	// #指定
    typedef enum {
	M_NORMAL,
	M_ATTRIB,
	M_ATTRIB_TEXT,
    }  mode_t;

    uint32_t  selector;
    mode_t  mode;
    WikiFormat*  wiki;
    bool  ferr;
    ustring  id;
    std::vector<ustring>  classlist;
    std::vector<ustring>  directlist;	// #指定
    ustring  onclick;
    ustring  onfocus;
    ustring  onblur;
    ustring  onchange;
    bool  scriptcut;
    ustring  target;
    bool  fmultiple;
    bool  fdefault;		/* no output */
    ustring  script;		/* no output */
    int  start;
#ifdef BOOTSTRAPHACK
    typedef  std::pair<ustring,ustring>  datapre_t;
    std::vector<datapre_t>  datapre;
#endif
    
    WikiAttrib1 (WikiFormat* _wiki, uint32_t _selector, bool _scriptcut, mode_t _mode /*= M_NORMAL*/) {
	wiki = _wiki;
	selector = _selector;
	mode = _mode;
	scriptcut = _scriptcut;
	fdefault = false;
	fmultiple = false;
	start = -1;
    };
    virtual  ~WikiAttrib1 () {};
    virtual bool  readAttrib1 (WikiMotorObjVec* cell, bool& rc);
    virtual bool  shiftAttrib (WikiMotorObjVecPtr& vec);
    virtual void  skipAttrib (WikiMotorObjVecPtr& vec);
    virtual bool  readAttrib (WikiMotorObjVecVec::const_iterator& b, const WikiMotorObjVecVec::const_iterator& e);
    virtual bool  checkScript (WikiMotorObjVec& vec, ustring& script, bool& ferr);
    virtual bool  shiftLink (WikiMotorObjVecPtr& vec, ustring& url, bool& fscript);
    virtual bool  readLink (WikiMotorObjVecVec::const_iterator& b, const WikiMotorObjVecVec::const_iterator& e, ustring& url, bool& fscript, bool noscript = false);
    virtual bool  shiftName (WikiMotorObjVecPtr& vec, ustring& name);
    virtual void  output (MotorOutput* out);
 private:
    virtual bool  readAttribMore (const ustring& key, WikiMotorObjVec& vval, bool& ferr) {
	return false;
    };
    virtual bool  readAttribMore2 (const ustring& key, WikiMotorObjVec& cell, bool& ferr) {
	return false;
    };
    virtual void  outputMore (MotorOutput* out) {};

 public:
    virtual bool  paramID (const ustring& key, WikiMotorObjVec& vval, bool& ferr);
    virtual void  paramIDValue (const ustring& key, WikiMotorObjVec& vval, ustring& var, bool& ferr);
    virtual bool  paramClass (const ustring& key, WikiMotorObjVec& vval, bool& ferr);
    virtual void  paramClassValue (WikiMotorObjVec& vval, std::vector<ustring>& var, bool& ferr);
    virtual bool  paramWidth (const ustring& key, WikiMotorObjVec& vval, ustring& var, bool& ferr);
    virtual bool  paramHeight (const ustring& key, WikiMotorObjVec& vval, ustring& var, bool& ferr);
    virtual void  paramWidthValue (const ustring& key, WikiMotorObjVec& vval, ustring& var, bool& ferr);
    virtual bool  paramSize (const char* name, size_t namelen, const ustring& key, WikiMotorObjVec& vval, ustring& var, bool& ferr);
    virtual void  paramUNum (const ustring& value, int& var, const ustring& name);
    virtual void  paramColor (const ustring& value, ustring& var, const ustring& name);
    virtual bool  paramTargetCheck (const ustring& key);
    virtual void  paramTargetBody (const ustring& key, const ustring& value, ustring& var, bool& ferr);
    virtual bool  paramOnClickCheck (const ustring& name);
    virtual bool  paramOnFocusCheck (const ustring& name);
    virtual bool  paramOnBlurCheck (const ustring& name);
    virtual bool  paramOnChangeCheck (const ustring& name);
#ifdef BOOTSTRAPHACK
    virtual bool  paramDataPrefix (const ustring& key, WikiMotorObjVec& vval, bool& ferr);
#endif
};

class  WikiAttribTable: public WikiAttrib1 {
 public:
    typedef enum {
	HAlignNone,
	HAlignLeft,
	HAlignRight,
	HAlignCenter,
    }  halign_t;
    typedef enum {
	VAlignNone,
	VAlignTop,
	VAlignBottom,
	VAlignMiddle,
    }  valign_t;

    enum selector_t {
	SEL_TABLE,
	SEL_TR,
	SEL_TD,
    }  selector2;

    bool  fheader;
    halign_t  halign;
    valign_t  valign;
    ustring  width;
    ustring  height;
    ustring  color;
    ustring  bgcolor;
    bool  fnowrap;
#ifdef WIKITABLEATTRIB
    bool  fnoborder;		/* table */
    int  cellspacing;
    int  cellpadding;
#endif
    bool  fpadding;
    bool  fnowhite;
    bool  fturn;

    WikiAttribTable (WikiFormat* _wiki, selector_t _selector2, mode_t _mode): WikiAttrib1 (_wiki, SEL_ID, false, _mode) {
	selector2 = _selector2;
	init ();
    };
    virtual  ~WikiAttribTable () {};
    virtual void  init ();
    virtual void  copyFrom (WikiAttribTable& b);
 private:
    virtual bool  readAttribMore (const ustring& key, WikiMotorObjVec& vval, bool& ferr);
    virtual bool  readAttribMore2 (const ustring& key, WikiMotorObjVec& cell, bool& ferr);
    virtual void  outputMore (MotorOutput* out);
};

class  WikiAttribImg: public WikiAttrib1 {
 public:
    ustring  width;
    ustring  height;
    ustring  alt;

    WikiAttribImg (WikiFormat* _wiki): WikiAttrib1 (_wiki, SEL_ID, false, M_ATTRIB) {};
    virtual  ~WikiAttribImg () {};
 private:
    virtual bool  readAttribMore (const ustring& key, WikiMotorObjVec& vval, bool& ferr);
    virtual void  outputMore (MotorOutput* out);
};

class  WikiAttribInput: public WikiAttrib1 {
 public:
    static const uint32_t  SEL_INPUT    = 0x0001;
    static const uint32_t  SEL_CHECK    = 0x0002;
    static const uint32_t  SEL_TEXTAREA = 0x0004;
    static const uint32_t  SEL_ELSIZE    = 0x0008;

    uint32_t  selector2;
    int64_t  elsize;
    ustring  pwidth;
    ustring  psize;
    bool  pdefault;
    bool  pchecked;
    ustring  pcols;
    ustring  prows;
    ustring  paccept;
    enum {
	W_OFF, W_SOFT, W_HARD,
    }  pwrap;
#ifdef INSERTTABHACK
    bool  ftab;
#endif

    WikiAttribInput (WikiFormat* _wiki, uint32_t _sel2): WikiAttrib1 (_wiki, SEL_MULTIPLE2, false, M_ATTRIB) {
	selector2 = _sel2;
	elsize = 0;
	pdefault = false;
	pchecked = false;
	pwrap = W_SOFT;
#ifdef INSERTTABHACK
	ftab = false;
#endif
    };
    virtual  ~WikiAttribInput () {};
 private:
    virtual bool  readAttribMore (const ustring& key, WikiMotorObjVec& vval, bool& ferr);
    virtual bool  readAttribMore2 (const ustring& key, WikiMotorObjVec& cell, bool& ferr);
    virtual void  outputMore (MotorOutput* out);
};

class  WikiAttribButton: public WikiAttrib1 {
 public:
    ustring  name;

    WikiAttribButton (WikiFormat* _wiki, bool _scriptcut): WikiAttrib1 (_wiki, SEL_SCRIPT2, _scriptcut, M_ATTRIB) {};
    virtual  ~WikiAttribButton () {};
    virtual bool  readAttrib (WikiMotorObjVecVec::const_iterator& b, const WikiMotorObjVecVec::const_iterator& e);
};

class  WikiAttribLink: public WikiAttrib1 {
public:
    std::vector<ustring>  arglist;

    WikiAttribLink (WikiFormat* _wiki): WikiAttrib1 (_wiki, WikiAttrib1::SEL_TARGET, true, WikiAttrib1::M_ATTRIB) {};
    virtual  ~WikiAttribLink () {};
    virtual bool  readAttribMore2 (const ustring& key, WikiMotorObjVec& cell, bool& ferr);
};

class  WikiBlockItemText;
class  WikiAttribItem: public WikiAttrib1 {
 public:
    WikiBlockItemText*  owner;

    WikiAttribItem (WikiFormat* _wiki, WikiBlockItemText* _b): WikiAttrib1 (_wiki, SEL_ID, false, M_NORMAL) {
	owner = _b;
    };
    virtual  ~WikiAttribItem () {};
    virtual bool  readAttribMore (const ustring& key, WikiMotorObjVec& vval, bool& ferr);
};

#endif /* WIKIATTRIB_H */
