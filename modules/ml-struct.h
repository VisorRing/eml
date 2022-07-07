#ifndef ML_STRUCT_H
#define ML_STRUCT_H

class  MNode;
class  MlEnv;

MNode*  ml_quote (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_if (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_cond (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_select (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_case (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_otherwise (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_progn (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_while (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_break (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_exit (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_pipe (bool fev, MNode* cell, MlEnv* mlenv);

#endif /* ML_STRUCT_H */
