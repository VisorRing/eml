#ifndef ML_VARIABLE_H
#define ML_VARIABLE_H

#include "ml.h"
class  MlEnv;

void  setvar_sel (MNode* name, MNode* val, MlEnv* mlenv);
MNode*  ml_setvar (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_storevar (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_let (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_getvar (bool fev, MNode* cell, MlEnv* mlenv);

#endif /* ML_VARIABLE_H */
