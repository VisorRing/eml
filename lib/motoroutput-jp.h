#ifndef MOTOROUTPUT_JP_H
#define MOTOROUTPUT_JP_H

#include "motoroutput.h"
#include "utf8-jp.h"

class  MotorOutputCOutJPMS: public MotorOutput {
 public:
    MotorOutputCOutJPMS () {};
    virtual  ~MotorOutputCOutJPMS () {};    

 protected:
    virtual MotorOutput*  out (const ustring::value_type* s, size_t len);

 public:
    virtual MotorOutput*  out_toText (const ustring& str);
    virtual bool  isResponse () {
	return true;
    };
};

#endif /* MOTOROUTPUT_JP_H */
