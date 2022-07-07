#ifndef ML_DEFUN_H
#define ML_DEFUN_H

class  MlEnv;
class  MNode;

MNode*  ml_defun (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_defun_motor (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_lambda (bool fev, MNode* cell, MlEnv* mlenv);

#endif /* ML_DEFUN_H */
