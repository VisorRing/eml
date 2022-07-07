#ifndef ML_TOTP_H
#define ML_TOTP_H

class  MNode;
class  MlEnv;

MNode*  ml_totp_calc (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_totp_create_token (bool fev, MNode* cell, MlEnv* mlenv);

#endif /* ML_TOTP_H */
