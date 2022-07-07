#ifndef ML_SECURITY_H
#define ML_SECURITY_H

#include "ml.h"
#include "ml-id.h"

class  MNode;
class  MlEnv;

MNode*  ml_hmac_sha1 (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_hmac_sha256 (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_md5 (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_sha1 (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_sha256 (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_md5_string (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_md5_file (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_sha1_string (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_sha1_file (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_sha256_string (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_sha256_file (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_hmac_sha256_n_string (bool fev, MNode* cell, MlEnv* mlenv);

#endif /* ML_SECURITY_H */
