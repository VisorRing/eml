#ifndef ML_LIST_H
#define ML_LIST_H

class MNode;
class MlEnv;

MNode*  ml_list (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_car (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_cdr (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_cons (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_carcdr (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_append (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_reverse (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_nth (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_list_get (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_list_size (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_member_of (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_map (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_list_reduce (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_list_reduce_while (bool fev, MNode* cell, MlEnv* mlenv);

#endif /* ML_LIST_H */
