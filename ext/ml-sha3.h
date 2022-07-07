#ifndef ML_SHA3_H
#define ML_SHA3_H

#include "ml.h"
#include "ml-id.h"

class  MNode;
class  MlEnv;

MNode*  ml_sha3 (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_sha3_string (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_sha3_file (bool fev, MNode* cell, MlEnv* mlenv);

#endif /* ML_SHA3_H */
