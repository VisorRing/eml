#ifndef ML_ELIQ_H
#define ML_ELIQ_H

#include "ml-neon.h"

class  MNode;
class  MlEnv;

class  MLEliq: public MLFunc {
 public:
    class  EliqOpts {
    public:
	MNode*  env;
	bool  fdebug;
	bool  ftrace;
	ustring  signal;
	bool  fbroadcast;
	ustring  wait;
	int  waittime;

	EliqOpts () {
	    env = NULL;
	    fdebug = ftrace = fbroadcast = false;
	    waittime = 0;
	};
	~EliqOpts () {};
    };
    class EliqHostConf {
    public:
	ustring  host;
	int  port;
	ustring  token;
	ustring  secret;

	EliqHostConf () {
	    port = 0;
	};
	~EliqHostConf () {};
    };

    EliqHostConf  hostconf;
    NeonSession::proto_t  proto;
    boost::scoped_ptr<NeonSession>  session;
    boost::scoped_ptr<NeonQuery>  query;

    MLEliq (MlEnv* _mlenv): MLFunc (cMLEliqID, _mlenv) {
	proto = NeonSession::PROTO_HTTP;
    };
    virtual  ~MLEliq () {};

    inline void  setHttps () {
	proto = NeonSession::PROTO_HTTPS;
    };
};

MNode*  ml_eliq_connect (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_eliq_progn (bool fev, MNode* cell, MlEnv* mlenv, MLFunc* mobj);

#endif /* ML_ELIQ_H */
