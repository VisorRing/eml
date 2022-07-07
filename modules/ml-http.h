#ifndef ML_HTTP_H
#define ML_HTTP_H

#include "ml.h"
#include "http-iconv.h"
#include "util_tcp.h"

class  MNode;
class  MlEnv;

MNode*  ml_build_url (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_build_url_path (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_build_query (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_decode_percent (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_http_date (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_hostnamep (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_response_no_cache (bool fev, MNode* cell, MlEnv* mlenv);

#endif /* ML_HTTP_H */
