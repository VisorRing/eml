#include "ml-amqp.h"
#include "ml.h"
#include "mlenv.h"
#include "config.h"
#include "ustring.h"
#include "expr.h"
#include "utf8.h"
#include "util_check.h"
#include "util_const.h"
#include <amqp.h>
#include <amqp_tcp_socket.h>
#include <amqp_framing.h>

/*DOC:
==AMQP library==

*/

bool  MLAmqp::connect (const ustring& host, unsigned int port, const ustring& id, const ustring& pw) {
    // コネクションを張る
    int  rc = amqp_socket_open (amqp_sock, host.c_str (), port);
    if (rc == AMQP_STATUS_OK) {
	// ログインする
	amqp_rpc_reply_t  rc_login = amqp_login (amqp, "/", AMQP_DEFAULT_MAX_CHANNELS, AMQP_DEFAULT_FRAME_SIZE, AMQP_DEFAULT_HEARTBEAT, AMQP_SASL_METHOD_PLAIN, id.c_str (), pw.c_str ());
	if (rc_login.reply_type != AMQP_RESPONSE_NORMAL) {
	    return false;
	}
	// チャンネルを開く
	amqp_channel_open_ok_t*  rc_channel = amqp_channel_open (amqp, channel);
	return true;
    } else {
	return false;
    }
}
// amqp_queue_declare(amqp, channel, queue, passive, durable, exclusive, autodelete, arguments)
// amqp_basic_publish(amqp, channel, exchange, routing, mandatory, immediate, properties, body)
// amqp_basic_consume(amqp, channel, queue, tag, no_local, no_ack, exclusive, , arguments)
// amqp_consume_message(amqp, envelope, timeout, flags)

/*DOC
===amqp-connect===
 (amqp-connect (HOST PORT TOKEN SECRET) BLOCK) -> LAST_VALUE

*/
//#XMFUNC		amqp-connect	ml_amqp_connect	cMLAmqpID
MNode*  ml_amqp_connect (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    MNodePtr  rest;
    evalParams (fev, mlenv, cell, posList, posParams, NULL, NULL, EV_ASIS, &rest);
    MNode*  hostconf = posParams[0]();
    if (! isCons (hostconf))
	throw (dump_to_texp (hostconf) + uErrorBadParam);
    MNodePtr  ans;
    MLAmqp  obj (mlenv);
    mlenv->setMStack (&obj);
    
    MNode*  a = hostconf;
    if (! isCons (a))
	throw (dump_to_texp (hostconf) + uErrorBadParam);
    ustring  host = to_string (a->car ());
    nextNode (a);
    if (! isCons (a))
	throw (dump_to_texp (hostconf) + uErrorBadParam);
    unsigned int  port = to_int64 (a->car ());
    nextNode (a);
    if (! isCons (a))
	throw (dump_to_texp (hostconf) + uErrorBadParam);
    ustring  key = to_string (a->car ());
    nextNode (a);
    if (! isCons (a))
	throw (dump_to_texp (hostconf) + uErrorBadParam);
    ustring  sec = to_string (a->car ());
    if (! isNil (a->cdr ()))
	throw (dump_to_texp (hostconf) + uErrorBadParam);
    if (! matchHostname (host))
	throw (host + ": bad hostname.");
    if (port <= 0 || port >= 65536)
	throw (to_ustring (port) + ": bad port number.");

    obj.connect (host, port, key, sec);

    ans = progn (rest (), mlenv);

    mlenv->stopBreak (cell->car ());
    return mlenv->retval = ans.release ();
}

/*DOC:
=== subfunctions of amqp-connect===

*/

#define byte_const(s)	{sizeof(s) - 1, noconst_char(s)}

/*DOC:
====amqp-exchange-declare
 (amqp-exchange-declare NAME [#direct | #topic | #headers | #fanout] #passive #durable #auto-delete #internal) -> true | nil

*/
//#XSFUNC	amqp-exchange-declare	ml_amqp_exchange_declare
MNode*  ml_amqp_exchange_declare (bool fev, MNode* cell, MlEnv* mlenv, MLFunc* mobj) {
    MLAmqp*  obj = MObjRef<MLAmqp> (mobj, cMLAmqpID);
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    kwParam  kwList[] = {{CharConst ("direct"), EV_LIST},
			 {CharConst ("topic"), EV_LIST},
			 {CharConst ("headers"), EV_LIST},
			 {CharConst ("fanout"), EV_LIST},
			 {CharConst ("passive"), EV_LIST},
			 {CharConst ("durable"), EV_LIST},
			 {CharConst ("auto-delete"), EV_LIST},
			 {CharConst ("internal"), EV_LIST},
			 {NULL, 0, EV_END}};
    MNodePtr  kwParams[8];
    evalParams (fev, mlenv, cell, posList, posParams, kwList, kwParams);
    ustring  exchange = to_string (posParams[0]());
    const amqp_bytes_t  aexchange = {exchange.size (), noconst_char (exchange.data ())};
    static const amqp_bytes_t  kDirect = byte_const ("direct");
    static const amqp_bytes_t  kTopic = byte_const ("topic");
    static const amqp_bytes_t  kHeaders = byte_const ("headers");
    static const amqp_bytes_t  kFanout = byte_const ("fanout");
    amqp_exchange_declare_ok_t*
	rc = amqp_exchange_declare (obj->amqp, obj->channel, aexchange,
				    to_bool (kwParams[0]()) ? kDirect : to_bool (kwParams[1]()) ? kTopic : to_bool (kwParams[2]()) ? kHeaders : kFanout,
				    to_bool (kwParams[4]()), to_bool (kwParams[5]()),
				    to_bool (kwParams[6]()), to_bool (kwParams[7]()),
				    amqp_empty_table);
    if (rc)
	return mlTrue;
    else
	return NULL;
}

/*DOC:
====amqp-queue-declare
 (amqp-queue-declare QUEUE #passive #durable #exclusive #auto-delete) -> true | nil

*/
//#XSFUNC	amqp-queue-declare	ml_amqp_queue_declare
MNode*  ml_amqp_queue_declare (bool fev, MNode* cell, MlEnv* mlenv, MLFunc* mobj) {
    MLAmqp*  obj = MObjRef<MLAmqp> (mobj, cMLAmqpID);
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    kwParam  kwList[] = {{CharConst ("passive"), EV_LIST},
			 {CharConst ("durable"), EV_LIST},
			 {CharConst ("exclusive"), EV_LIST},
			 {CharConst ("auto-delete"), EV_LIST},
			 {NULL, 0, EV_END}};
    MNodePtr  kwParams[4];
    evalParams (fev, mlenv, cell, posList, posParams, kwList, kwParams);
    ustring  queue = to_string (posParams[0]());
    const amqp_bytes_t  aqueue = {queue.size (), noconst_char (queue.data ())};
    amqp_queue_declare_ok_t*
	rc = amqp_queue_declare (obj->amqp, obj->channel, aqueue,
				 to_bool (kwParams[0]()), to_bool (kwParams[1]()), to_bool (kwParams[2]()),
				 to_bool (kwParams[3]()), amqp_empty_table);
    if (rc)
	return mlTrue;
    else
	return NULL;
}

/*DOC:
====amqp-queue-bind====
 (amqp-queue-bind QUEUE EXCHANGE ROUTING) -> true | nil

*/
//#XSFUNC	amqp-queue-bind	ml_amqp_queue_bind
MNode*  ml_amqp_queue_bind (bool fev, MNode* cell, MlEnv* mlenv, MLFunc* mobj) {
    MLAmqp*  obj = MObjRef<MLAmqp> (mobj, cMLAmqpID);
    paramType  posList[] = {EV_LIST, EV_LIST, EV_LIST, EV_END};
    MNodePtr  posParams[3];
    evalParams (fev, mlenv, cell, posList, posParams);
    ustring  queue = to_string (posParams[0]());
    const amqp_bytes_t  aqueue = {queue.size (), noconst_char (queue.data ())};
    ustring  exchange = to_string (posParams[1]());
    const amqp_bytes_t  aexchange = {exchange.size (), noconst_char (exchange.data ())};
    ustring  routing = to_string (posParams[2]());
    const amqp_bytes_t  arouting = {routing.size (), noconst_char (routing.data ())};
    amqp_queue_bind_ok_t*
	rc = amqp_queue_bind (obj->amqp, obj->channel, aqueue,
			      aexchange, arouting, amqp_empty_table);
    if (rc)
	return mlTrue;
    else
	return NULL;
}

/*DOC:
====amqp-publish====
 (amqp-publish EXCHANGE ROUTING BODY #mandatory #immediate) -> true | nil

*/
//#XSFUNC	amqp-publish	ml_amqp_publish
MNode*  ml_amqp_publish (bool fev, MNode* cell, MlEnv* mlenv, MLFunc* mobj) {
    MLAmqp*  obj = MObjRef<MLAmqp> (mobj, cMLAmqpID);
    paramType  posList[] = {EV_LIST, EV_LIST, EV_LIST, EV_END};
    MNodePtr  posParams[3];
    kwParam  kwList[] = {{CharConst ("mandatory"), EV_LIST},
			 {CharConst ("immediate"), EV_LIST},
			 {NULL, 0, EV_END}};
    MNodePtr  kwParams[2];
    evalParams (fev, mlenv, cell, posList, posParams, kwList, kwParams);
    ustring  exchange = to_string (posParams[0]());
    const amqp_bytes_t  aexchange = {exchange.size (), noconst_char (exchange.data ())};
    ustring  routing = to_string (posParams[1]());
    const amqp_bytes_t  arouting = {routing.size (), noconst_char (routing.data ())};
    ustring  body = to_string (posParams[2]());
    const amqp_bytes_t  abody = {body.size (), noconst_char (body.data ())};
    int  rc = amqp_basic_publish (obj->amqp, obj->channel,
				  aexchange, arouting, to_bool (kwParams[0]()),
				  to_bool (kwParams[1]()), NULL,
				  abody);
    if (rc == AMQP_STATUS_OK)
	return mlTrue;
    else
	return NULL;
}

/*DOC:
====amqp-basic-consume====
 (amqp-basic-consume QUEUE #no-local #no-ack #exclusive) -> true | nil
受信方法の指定。
* TAG –– コンシューマの識別のためのラベル

*/
//#XSFUNC	amqp-basic-consume	ml_amqp_basic_consume
MNode*  ml_amqp_basic_consume (bool fev, MNode* cell, MlEnv* mlenv, MLFunc* mobj) {
    MLAmqp*  obj = MObjRef<MLAmqp> (mobj, cMLAmqpID);
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    kwParam  kwList[] = {{CharConst ("no-local"), EV_LIST},
			 {CharConst ("no-ack"), EV_LIST},
			 {CharConst ("exclusive"), EV_LIST},
			 {NULL, 0, EV_END}};
    MNodePtr  kwParams[3];
    evalParams (fev, mlenv, cell, posList, posParams, kwList, kwParams);
    ustring  queue = to_string (posParams[0]());
    const amqp_bytes_t  aqueue = {queue.size (), noconst_char (queue.data ())};
    amqp_basic_consume_ok_t*
	rc = amqp_basic_consume (obj->amqp, obj->channel, aqueue, amqp_empty_bytes,
				 to_bool (kwParams[0]()),
				 ! to_bool (kwParams[1]()), // XXX no-ackのはずがauto-ackになっている
				 to_bool (kwParams[2]()),
				 amqp_empty_table);
    if (rc)
	return mlTrue;
    else
	return NULL;
}

/*DOC:
====amqp-basic-ack====
 (amqp-basic-ack DELIVERY_TAG #multiple) -> true | nil

*/
//#XSFUNC	amqp-basic-ack	ml_amqp_basic_ack
MNode*  ml_amqp_basic_ack (bool fev, MNode* cell, MlEnv* mlenv, MLFunc* mobj) {
    MLAmqp*  obj = MObjRef<MLAmqp> (mobj, cMLAmqpID);
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    kwParam  kwList[] = {{CharConst ("multiple"), EV_LIST},
			 {NULL, 0, EV_END}};
    MNodePtr  kwParams[1];
    evalParams (fev, mlenv, cell, posList, posParams, kwList, kwParams);
    uint64_t  tag = to_int64 (posParams[0]());
    int  rc = amqp_basic_ack (obj->amqp, obj->channel, tag, to_bool (kwParams[0]()));
    if (rc == 0)
	return mlTrue;
    else
	return NULL;
}

/*DOC:
====amqp-consume-message====
 (amqp-consume-message TIMEOUT_SEC) -> (MESSAGE DELIVERY_TAG EXCHANGE ROUTINGKEY) | nil

*/
//#XSFUNC	amqp-consume-message	ml_amqp_consume_message
MNode*  ml_amqp_consume_message (bool fev, MNode* cell, MlEnv* mlenv, MLFunc* mobj) {
    MLAmqp*  obj = MObjRef<MLAmqp> (mobj, cMLAmqpID);
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    evalParams (fev, mlenv, cell, posList, posParams);
    struct timeval  tm = {to_int64 (posParams[0]()), 0};
    amqp_envelope_t  envelope;
    amqp_rpc_reply_t  rc = amqp_consume_message (obj->amqp, &envelope, &tm, 0);
    switch (rc.reply_type) {
    case AMQP_RESPONSE_NORMAL:
	{
	    MNodeList  ans;
	    ustring*  str;
	    // message
	    str = new ustring (char_type (envelope.message.body.bytes), envelope.message.body.len);
	    fixUTF8 (*str);
	    ans.append (newMNode_str (str));
	    // delivery_tag
//	    str = new ustring (to_ustring (envelope.delivery_tag));
//	    ans.append (newMNode_str (str));
	    ans.append (newMNode_int64 (envelope.delivery_tag));
//	    // consumer_tag
//	    str = new ustring (char_type (envelope.consumer_tag.bytes), envelope.consumer_tag.len);
//	    fixUTF8 (*str);
//	    ans.append (newMNode_str (str));
	    // exchange
	    str = new ustring (char_type (envelope.exchange.bytes), envelope.exchange.len);
	    fixUTF8 (*str);
	    ans.append (newMNode_str (str));
	    // routing_key
	    str = new ustring (char_type (envelope.routing_key.bytes), envelope.routing_key.len);
	    fixUTF8 (*str);
	    ans.append (newMNode_str (str));
	    return mlenv->retval = ans.release ();
	}
    case AMQP_RESPONSE_LIBRARY_EXCEPTION:
	{
	    if (rc.library_error == AMQP_STATUS_UNEXPECTED_STATE) {
		amqp_frame_t  frame;
		if (amqp_simple_wait_frame (obj->amqp, &frame) != AMQP_STATUS_OK) {
		    return NULL;
		}
		if (frame.frame_type == AMQP_FRAME_METHOD) {
		    switch (frame.payload.method.id) {
		    case AMQP_BASIC_ACK_METHOD:
			break;
		    case AMQP_BASIC_RETURN_METHOD: {
			amqp_message_t  message;
			amqp_rpc_reply_t  rc_read = amqp_read_message (obj->amqp, frame.channel, &message, 0);
			if (rc_read.reply_type != AMQP_RESPONSE_NORMAL) {
			    return NULL;
			}
			amqp_destroy_message (&message);
			break;
		    }
		    case AMQP_CHANNEL_CLOSE_METHOD:
		    case AMQP_CONNECTION_CLOSE_METHOD:
		    default:
			return NULL;
		    }
		}
	    } else {
		return NULL;
	    }
	}
	break;
    default:;
    }
    return NULL;
}
