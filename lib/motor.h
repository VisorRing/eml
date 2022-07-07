#ifndef MOTOR_H
#define MOTOR_H

#include "motoroutput.h"
#include "motorenv.h"
#include "ustring.h"
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/unordered_map.hpp>
#include <vector>

class  HTMLMotor;
class  MotorObj {
 public:
    typedef  boost::ptr_vector<MotorObj>  MotorObjVec;
    typedef  boost::ptr_vector<MotorObjVec>  MotorObjVecVec;

    enum objType {
	motor_text,
	motor_var,
	motor_capture,
	motor_expand,
	motor_select,
	motor_func,
	motor_func2,
	motor_defun,
//	motor_wiki,
//	motor_ml,
    }  type;
    HTMLMotor*  motor;

    MotorObj (objType t, HTMLMotor* owner): type (t), motor (owner) {};
    virtual  ~MotorObj () {};
    virtual void  out (MotorOutput* o, MotorEnv* env) = 0;
    virtual ustring  dump () = 0;
    virtual ustring  dumpVec (MotorObjVec& v);
};

class  MotorObjText: public MotorObj {
 public:
    ustring  text;

    MotorObjText (HTMLMotor* owner, uiterator begin, uiterator end): MotorObj (motor_text, owner) {
	text.assign (begin, end);
    };
    virtual  ~MotorObjText () {};
    virtual void  out (MotorOutput* o, MotorEnv* env);
    virtual ustring  dump ();
};

class  MotorObjVar: public MotorObj {
 public:
    ustring  variable;
    enum  opt {
	opt_normal,
	opt_br,
	opt_nbsp,
	opt_c3,
	opt_wbr,
//	opt_raw,
//	opt_br_nbsp,
//	opt_br_a,
//	opt_br_a_nbsp,
//	opt_wiki,
//	opt_url,
    }  vopt;

    MotorObjVar (HTMLMotor* owner, const ustring& name, opt p2): MotorObj (motor_var, owner) {
	variable = name;
	vopt = p2;
    };
    virtual  ~MotorObjVar () {};
    virtual void  out (MotorOutput* o, MotorEnv* env);
    virtual ustring  dump ();
};

class  MotorObjCapture: public MotorObj {
 public:
    ustring  num;
    MotorObjVec  objs;

    MotorObjCapture (HTMLMotor* owner, const ustring& _num): MotorObj (motor_capture, owner) {
	num = _num;
    };
    virtual  ~MotorObjCapture () {};
    virtual void  out (MotorOutput* o, MotorEnv* env);
    virtual ustring  dump ();
};

class  MotorObjExpand: public MotorObj {
public:
    ustring  num;

    MotorObjExpand (HTMLMotor* owner, const ustring& _num): MotorObj (motor_expand, owner) {
	num = _num;
    };
    virtual  ~MotorObjExpand () {};
    virtual void  out (MotorOutput* o, MotorEnv* env);
    virtual ustring  dump ();
};

class  MotorObjSelect: public MotorObj {
 public:
    ustring  variable;
    MotorObjVec  value;
    bool  fbang;
    MotorObjVec  trueobjs;
    MotorObjVec  falseobjs;

    MotorObjSelect (HTMLMotor* owner, const ustring& var, bool fb): MotorObj (motor_select, owner) {
	variable = var;
	fbang = fb;
    };
    virtual  ~MotorObjSelect () {};
    virtual void  out (MotorOutput* o, MotorEnv* env);
    virtual ustring  dump ();
};

class  MlEnv;
class  MotorObjFunc: public MotorObj {
 public:
    ustring  name;
    void  (*fn) (const std::vector<ustring>&, MlEnv*);
    MotorObjVecVec  args;

    MotorObjFunc (HTMLMotor* owner, const ustring& _name, void (*_fn) (const std::vector<ustring>&, MlEnv*)): MotorObj (motor_func, owner) {
	name = _name;
	fn = _fn;
    };
    virtual  ~MotorObjFunc () {};
    virtual void  out (MotorOutput* o, MotorEnv* env);
    virtual ustring  dump ();
};

class  MotorObjFunc2: public MotorObj {
 public:
    ustring  name;
    void  (*fn) (const std::vector<ustring>&, const ustring&, MlEnv*);
    MotorObjVecVec  args;
    MotorObjVec  arg2;

    MotorObjFunc2 (HTMLMotor* owner, const ustring& _name, void (*_fn) (const std::vector<ustring>&, const ustring&, MlEnv*)): MotorObj (motor_func2, owner) {
	name = _name;
	fn = _fn;
    };
    virtual  ~MotorObjFunc2 () {};
    virtual void  out (MotorOutput* o, MotorEnv* env);
    virtual ustring  dump ();
};

class  MotorObjDefun: public MotorObj {
 public:
    ustring  name;
    MotorObjVecVec  args;

    MotorObjDefun (HTMLMotor* owner, const ustring& _name): MotorObj (motor_defun, owner) {
	name = _name;
    };
    virtual  ~MotorObjDefun () {};
    virtual void  out (MotorOutput* o, MotorEnv* env);
    virtual ustring  dump ();
};

class  HTMLMotor {
 public:
    MotorObj::MotorObjVec  objs;
    uiterator  begin;
    uiterator  end;
    boost::unordered_map<ustring, MotorObj::MotorObjVec*>  templates;

    HTMLMotor () {};
    virtual  ~HTMLMotor () {};
    virtual void  compile (const ustring& text, bool skipHead = false);
    virtual void  compile_file (const ustring& path, bool skipHead = false);
    virtual void  output (MotorObj::MotorObjVec& v, MotorOutput* o, MotorEnv* env);
    virtual void  output (MotorOutput* o, MotorEnv* env);
    virtual void  outputTemp (const ustring& text, MotorOutput* o, MotorEnv* env);
    virtual void  setTemplate (const ustring& name, MotorObj::MotorObjVec* value);
    virtual MotorObj::MotorObjVec*  getTemplate (const ustring& name);
#ifdef DEBUG2
    virtual void  dump (std::ostream& o);
#endif /* DEBUG */
    
 private:
    typedef enum {term_none, term_else_bra, term_bra, term_colon}  term_type;
    typedef enum {end_none, end_else, end_bra, end_colon}  end_type;
    virtual void  shiftOne (MotorObj::MotorObjVec* sobjs);
    virtual bool  shiftOpen (MotorObj::MotorObjVec* sobjs);
    virtual bool  shiftOpenName (MotorObj::MotorObjVec* sobjs, const ustring& name);
    virtual bool  shiftOpenNum (MotorObj::MotorObjVec* sobjs, const ustring& num);
    virtual bool  shiftOpenNumBar (MotorObj::MotorObjVec* sobjs, const ustring& num);
    virtual bool  shiftOpenNameColon (MotorObj::MotorObjVec* sobjs, const ustring& name);
    virtual bool  shiftOpenNameBar (MotorObj::MotorObjVec* sobjs, const ustring& name);
    virtual bool  shiftFParams (MotorObj::MotorObjVecVec* args, MotorObj::MotorObjVec* arg2);
    virtual bool  shiftFParams2 (MotorObj::MotorObjVec* arg2);
    virtual bool  shiftCond (MotorObj::MotorObjVec* sobjs, const ustring& name, bool neg);
    virtual bool  shiftCondValue (MotorObjSelect* sel);
    virtual bool  shiftCondBlock (MotorObjSelect* sel);
    virtual bool  shiftCondOtherwise (MotorObjSelect* sel);
    virtual bool  shiftCaptureElements (MotorObj::MotorObjVec* sobjs);
    virtual void  shiftPrec (MotorObj::MotorObjVec* sobjs, const umatch& m);
};

ustring  to_string (MotorObj::MotorObjVec* v, MotorEnv* env);

#endif /* MOTOR_H */
