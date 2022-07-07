#ifndef ML_CORE_H
#define ML_CORE_H

class MNode;
class MlEnv;

MNode*  ml_null (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_not_null (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_consp (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_stringp (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_symbolp (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_is_integer (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_is_float (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_numberp (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_vectorp (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_tablep (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_is_bool (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_is_lambda (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_is_builtin (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_apply (bool fev, MNode* cell, MlEnv* mlenv);
//MNode*  ml_funcall (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_eval (bool fev, MNode* cell, MlEnv* mlenv);

#endif /* ML_CORE_H */
