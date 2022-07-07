#ifndef MOTOROUTPUT_H
#define MOTOROUTPUT_H

#include "util_const.h"
#include "util_regex.h"
#include "ustring.h"

class  MotorOutput {
 public:
    MotorOutput () {};
    virtual  ~MotorOutput () {};

 protected:
    virtual MotorOutput*  outamp (uiterator b, uiterator e, uregex* re);
    virtual void  init () {};
    virtual MotorOutput*  out (const ustring::value_type* s, size_t len) = 0; /* raw output. */

 public:    
    virtual MotorOutput*  out_raw (const ustring::value_type* s, size_t len) {
	return out (s, len);
    }
    virtual MotorOutput*  out_raw (uiterator s, uiterator e) {
	return out (s.base (), e - s);
    };
    virtual MotorOutput*  out_raw (const ustring& text) {
	return out_raw (text.begin ().base (), text.size ());
    };
    virtual MotorOutput*  out_toText (const ustring& str) {	/* output with a text code convertion. */
	out_raw (str);
	return this;
    };
    virtual MotorOutput*  out_templateText (const ustring& str);	/* fix the charactor set. */
    virtual MotorOutput*  out_toHTML (const ustring& str);	/* amp encode */
    virtual MotorOutput*  out_toHTML_br (const ustring& str);
    virtual MotorOutput*  out_toHTML_br_a (const ustring& str);
    virtual MotorOutput*  out_toHTML_wbr (const ustring& str);
    virtual MotorOutput*  out_toHTML_noCtrl (const ustring& str);
    virtual MotorOutput*  out_toJS (const ustring& str);
    virtual MotorOutput*  out_noCtrl (const ustring& str);
    virtual MotorOutput*  outNbsp ();
    virtual MotorOutput*  out_file (const ustring& srcFile);
    virtual MotorOutput*  out_file_base64 (const ustring& srcFile);
    virtual MotorOutput*  outamp (const ustring& t);
    virtual MotorOutput*  outamp_br (const ustring& t);
    virtual MotorOutput*  outamp_wbr (const ustring& t);
    virtual MotorOutput*  outamp_nw (const ustring& t);
    virtual MotorOutput*  outamp_c3 (const ustring& t);
    virtual const ustring  charset () {
	return uUTF8;
    };
    virtual bool  isResponse () = 0;
};

class  MotorOutputCOut: public MotorOutput {
 public:
    MotorOutputCOut () {};
    virtual  ~MotorOutputCOut () {};

 protected:
    virtual MotorOutput*  out (const ustring::value_type* s, size_t len);
 public:
    virtual bool  isResponse () {
	return true;
    };
};

class  MotorOutputString: public MotorOutput {
 public:
    ustring  ans;		/* XXX: customize the allocator. */
    
    MotorOutputString () {};
    virtual  ~MotorOutputString () {};
    virtual void  init () {
	ans.resize (0);
    };
 protected:
    virtual MotorOutput*  out (const ustring::value_type* s, size_t len);
 public:
    virtual bool  isResponse () {
	return false;
    };
};

class  MotorOutputOStream: public MotorOutput {
 public:
    std::ostream*  ostream;

    MotorOutputOStream (std::ostream* _ostream): ostream (_ostream) {};
    virtual  ~MotorOutputOStream () {};

 protected:
    virtual MotorOutput*  out (const ustring::value_type* s, size_t len);
 public:
    virtual bool  isResponse () {
	return true;
    };
};

class  AutoSwapMotor {
 public:
    MotorOutput**  target;
    MotorOutput*  back;

    AutoSwapMotor (MotorOutput*& _target, MotorOutput& swap) {
	target = &_target;
	back = *target;
	*target = &swap;
    };
    virtual  ~AutoSwapMotor () {
	*target = back;
    };
};

#endif /* MOTOROUTPUT_H */
