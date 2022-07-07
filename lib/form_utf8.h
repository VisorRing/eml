#ifndef FORM_UTF8_H
#define FORM_UTF8_H

#include "formfile.h"
#include "ustring.h"

class  CGIFormUTF8: public CGIFormFile {
 public:
    CGIFormUTF8 () {};
    virtual  ~CGIFormUTF8 () {};
    virtual void  decode (ustring& key);
    virtual void  fix (ustring& key);
};

#endif /* FORM_UTF8_H */
