#include "motoroutput-iconv.h"
#include "ustring.h"
#include <iostream>

MotorOutput*  MotorOutputIConv::out_toText (const ustring& str) {
    ustring  o;

    o = cd.cv (str);
    out (o.data (), o.length ());
    return this;
}

MotorOutput*  MotorOutputIConv::out_templateText (const ustring& str) {
    return out_toText (str);
}

MotorOutput*  MotorOutputIConv::flush () {
    ustring  o;

    o = cd.cv (uEmpty, true);	// flush
    out (o.data (), o.length ());
    return this;
}

MotorOutput*  MotorOutputIConvOStream::out (const ustring::value_type* s, size_t len) {
    std::cout.write (s, len);
    return this;
}

MotorOutput*  MotorOutputIConvString::out (const ustring::value_type* s, size_t len) {
    ans.append (ustring (s, len));
    return this;
}

