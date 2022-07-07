#ifndef ML_CRYPTO_H
#define ML_CRYPTO_H

class  MNode;
class  MlEnv;

MNode*  ml_bf_cbc_encode (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_bf_cbc_decode (bool fev, MNode* cell, MlEnv* mlenv);

#endif /* ML_CRYPTO_H */
