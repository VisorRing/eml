#ifndef ML_MATH_H
#define ML_MATH_H

class  MNode;
class  MlEnv;

MNode*  ml_to_integer (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_to_number (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_add (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_sub (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_mult (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_div (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_mod (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_idivmod (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_ceil (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_floor (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_fraction (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_random (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_random_int (bool fev, MNode* cell, MlEnv* mlenv);

#endif /* ML_MATH_H */
