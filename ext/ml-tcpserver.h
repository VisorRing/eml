#ifndef ML_TCPSERVER_H
#define ML_TCPSERVER_H

#include "ml.h"
#include "ml-id.h"
#include "ustring.h"
#include "bdbmacro.h"

class  MlEnv;

class  MLDbTcpserver: public MLFunc {
 public:
    BDBHash  db;
    ustring  dbpath;

    MLDbTcpserver (MlEnv* _mlenv): MLFunc (cMLDbTcpserverID, _mlenv) {};
    virtual  ~MLDbTcpserver () {
	closedb ();
    };
    virtual void  setPath (const ustring& name);
    virtual void  opendb ();
    virtual void  closedb ();
    virtual void  addAllow (const ustring& key, time_t span, MNode* rest, MlEnv* mlenv);
    virtual void  addDeny (const ustring& key);
    virtual void  del (const ustring& key);
    virtual bool  splitRec (const ustring& rec, bool& allow, time_t& limit, MNodeList& estr);
    virtual bool  splitRec (const ustring& rec, bool& allow, time_t& limit);
};

MNode*  ml_dbtcpserver (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_dbtcpserver_add_allow_ip (bool fev, MNode* cell, MlEnv* mlenv, MLFunc* mobj);
MNode*  ml_dbtcpserver_add_allow_host (bool fev, MNode* cell, MlEnv* mlenv, MLFunc* mobj);
MNode*  ml_dbtcpserver_add_deny_ip (bool fev, MNode* cell, MlEnv* mlenv, MLFunc* mobj);
MNode*  ml_dbtcpserver_add_deny_host (bool fev, MNode* cell, MlEnv* mlenv, MLFunc* mobj);
MNode*  ml_dbtcpserver_delete_ip (bool fev, MNode* cell, MlEnv* mlenv, MLFunc* mobj);
MNode*  ml_dbtcpserver_delete_host (bool fev, MNode* cell, MlEnv* mlenv, MLFunc* mobj);
MNode*  ml_dbtcpserver_read_ip (bool fev, MNode* cell, MlEnv* mlenv, MLFunc* mobj);
MNode*  ml_dbtcpserver_read_host (bool fev, MNode* cell, MlEnv* mlenv, MLFunc* mobj);
MNode*  ml_dbtcpserver_dump_ip (bool fev, MNode* cell, MlEnv* mlenv, MLFunc* mobj);
MNode*  ml_dbtcpserver_dump_host (bool fev, MNode* cell, MlEnv* mlenv, MLFunc* mobj);
MNode*  ml_dbtcpserver_cleanup (bool fev, MNode* cell, MlEnv* mlenv, MLFunc* mobj);

#endif /* ML_TCPSERVER_H */
