#ifndef ML_COOKIELOGIN_H
#define ML_COOKIELOGIN_H

#include "ml.h"
#include "ml-id.h"
#include "ustring.h"
#include "bdbmacro.h"
#include "filemacro.h"

class  MlEnv;
class  MLCookieLogin: public MLFunc {
 public:
    BDBHash  db;
    FileMacro  lock;
    ustring  dbpath;
    ustring  sessionkey;
    ustring  sessionval;
    ustring  sessionval_ssl;

    MLCookieLogin (MlEnv* _mlenv): MLFunc (cMLCookieLoginID, _mlenv) {};
    virtual  ~MLCookieLogin () {
	closedb ();
    };

    virtual void  opendb ();
    virtual void  closedb ();
};

MNode*  ml_cookielogin (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_cookielogin_login (bool fev, MNode* cell, MlEnv* mlenv, MLFunc* mobj);
//MNode*  ml_cookielogin_login_dual (bool fev, MNode* cell, MlEnv* mlenv, MLFunc* mobj);
MNode*  ml_cookielogin_logout (bool fev, MNode* cell, MlEnv* mlenv, MLFunc* mobj);
MNode*  ml_cookielogin_check (bool fev, MNode* cell, MlEnv* mlenv, MLFunc* mobj);
MNode*  ml_cookielogin_delete (bool fev, MNode* cell, MlEnv* mlenv, MLFunc* mobj);
MNode*  ml_cookielogin_clear (bool fev, MNode* cell, MlEnv* mlenv, MLFunc* mobj);
MNode*  ml_cookielogin_session_key (bool fev, MNode* cell, MlEnv* mlenv, MLFunc* mobj);
MNode*  ml_cookielogin_session_key_ssl (bool fev, MNode* cell, MlEnv* mlenv, MLFunc* mobj);

#endif /* ML_COOKIELOGIN_H */
