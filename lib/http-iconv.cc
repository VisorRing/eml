#include "http-iconv.h"
#include <iostream>
#include <exception>

ustring  HTTPSendIConv::cv (const ustring& text) {
    // 出力側
    return cd_out.cv (text, true);
}

ustring  HTTPSendIConv::rcv (const ustring& text) {
    // 入力側
    return cd_in.cv (text, true);
}
