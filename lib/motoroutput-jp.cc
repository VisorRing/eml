#include "motoroutput-jp.h"
#include "utf8-jp.h"
#include <iostream>

/* ============================================================ */
MotorOutput*  MotorOutputCOutJPMS::out (const ustring::value_type* s, size_t len) {
    std::cout.write (s, len);
    return this;
}

MotorOutput*  MotorOutputCOutJPMS::out_toText (const ustring& str) {
    out_raw (fixToMS (str));
    return this;
}

/* ============================================================ */
