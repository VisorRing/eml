#ifndef HTTP_ICONV_H
#define HTTP_ICONV_H

#include "http.h"
#include "httpconst.h"
#include "util_string.h"

class  HTTPSendIConv: public HTTPSend {
 public:
    UIConv  cd_in;
    UIConv  cd_out;
    ustring  code;

    HTTPSendIConv (const char* _code): cd_in (kCODE_UTF8, _code), cd_out (_code, kCODE_UTF8) {
	code.assign (_code);
    };
    virtual  ~HTTPSendIConv () {};
    virtual const ustring  charset () {
	return code;
    };
    virtual ustring  cv (const ustring& text);
    virtual ustring  rcv (const ustring& text);
};

#endif /* HTTP_ICONV_H */
