#ifndef ML_BOOL_H
#define ML_BOOL_H

class MNode;
class MlEnv;

MNode*  ml_to_bool (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_not (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_and (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_or (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_xor (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_neq (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_nne (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_gt (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_lt (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_ge (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_le (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_object_eq (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_object_equal (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_object_not_eq (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_object_not_equal (bool fev, MNode* cell, MlEnv* mlenv);

#endif /* ML_BOOL_H */
