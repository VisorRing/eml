#ifndef ML_DATA_H
#define ML_DATA_H

class MNode;
class MlEnv;

MNode*  ml_min (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_max (bool fev, MNode* cell, MlEnv* mlenv);

#endif /* ML_DATA_H */
