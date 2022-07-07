#ifndef MAIL_H
#define MAIL_H

#include "ml.h"
#include "ustring.h"

class  MimeBody {
 public:
    MNodePtr  file_store;
    MNodePtr  file_storage;
    MNodePtr  file_static;
    ustring  separator;

    MimeBody () {};
    virtual  ~MimeBody () {};

    
};

#endif /* MAIL_H */
