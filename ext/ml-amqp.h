#ifndef ML_AMQP_H
#define ML_AMQP_H

#include "ml.h"
#include "ml-id.h"
#include <amqp.h>
#include <amqp_tcp_socket.h>
#include <amqp_framing.h>

class  MNode;
class  MlEnv;

class  MLAmqp: public MLFunc {
 public:
    amqp_connection_state_t  amqp;
    amqp_socket_t*  amqp_sock;
    amqp_channel_t  channel;

    MLAmqp (MlEnv* _mlenv): MLFunc (cMLAmqpID, _mlenv) {
	amqp = amqp_new_connection ();
	amqp_sock = amqp_tcp_socket_new (amqp);
	channel = 1;
    };
    virtual  ~MLAmqp () {
	amqp_rpc_reply_t  rc_chclose = amqp_channel_close (amqp, channel, AMQP_REPLY_SUCCESS);
	amqp_rpc_reply_t  rc_conclose = amqp_connection_close (amqp, AMQP_REPLY_SUCCESS);
  	amqp_destroy_connection (amqp);
    };
    virtual bool  connect (const ustring& host, unsigned int port, const ustring& id, const ustring& pw);
};

MNode*  ml_amqp_connect (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_amqp_exchange_declare (bool fev, MNode* cell, MlEnv* mlenv, MLFunc* mobj);
MNode*  ml_amqp_queue_declare (bool fev, MNode* cell, MlEnv* mlenv, MLFunc* mobj);
MNode*  ml_amqp_queue_bind (bool fev, MNode* cell, MlEnv* mlenv, MLFunc* mobj);
MNode*  ml_amqp_publish (bool fev, MNode* cell, MlEnv* mlenv, MLFunc* mobj);
MNode*  ml_amqp_basic_consume (bool fev, MNode* cell, MlEnv* mlenv, MLFunc* mobj);
MNode*  ml_amqp_basic_ack (bool fev, MNode* cell, MlEnv* mlenv, MLFunc* mobj);
MNode*  ml_amqp_consume_message (bool fev, MNode* cell, MlEnv* mlenv, MLFunc* mobj);

#endif /* ML_AMQP_H */
