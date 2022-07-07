#ifndef MOTOROUTPUT_ICONV_H
#define MOTOROUTPUT_ICONV_H

#include "motoroutput.h"
#include "httpconst.h"
#include "util_string.h"

class  MotorOutputIConv: public MotorOutput {
 public:
    UIConv  cd;
    ustring  code;

    MotorOutputIConv (const char* _code): cd (_code, kCODE_UTF8) {
	code.assign (_code);
    };
    virtual  ~MotorOutputIConv () {};

    virtual MotorOutput*  out_toText (const ustring& str);
    virtual MotorOutput*  out_templateText (const ustring& str);
    virtual MotorOutput*  flush ();
    virtual const ustring  charset () {
	return code;
    };
};

class  MotorOutputIConvOStream: public MotorOutputIConv {
 public:
    MotorOutputIConvOStream (const char* v): MotorOutputIConv (v) {};
    virtual  ~MotorOutputIConvOStream () {};

 protected:
    virtual MotorOutput*  out (const ustring::value_type* s, size_t len);
 public:
    virtual bool  isResponse () {
	return true;
    };
};

class  MotorOutputIConvString: public MotorOutputIConv {
 public:
    ustring  ans;

    MotorOutputIConvString (const char* v): MotorOutputIConv (v) {};
    virtual  ~MotorOutputIConvString () {};

 protected:
    virtual MotorOutput*  out (const ustring::value_type* s, size_t len);
 public:
    virtual bool  isResponse () {
	return false;
    };
};

#endif /* MOTOROUTPUT_ICONV_H */
