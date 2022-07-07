#ifndef ML_INET_H
#define ML_INET_H

class  MNode;
class  MlEnv;

MNode*  ml_to_hostname (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_to_ip_addr (bool fev, MNode* cell, MlEnv* mlenv);

#endif /* ML_INET_H */
