#ifndef FORM_UTF8_JP_H
#define FORM_UTF8_JP_H

#include "form_utf8.h"

class  CGIFormUTF8JPMS: public CGIFormUTF8 {
 public:
    CGIFormUTF8JPMS () {};
    virtual  ~CGIFormUTF8JPMS () {};
    virtual void  decode (ustring& key);
    virtual void  fix (ustring& key);
};

#endif /* FORM_UTF8_JP_H */
