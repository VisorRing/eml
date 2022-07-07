#ifndef ML_WIKI_H
#define ML_WIKI_H

class  MNode;
class  MlEnv;

MNode*  ml_wiki (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_wiki_string (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_wiki_eval (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_wikivar (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_wikivar_ary (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_set_wikivar (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_defun_wiki_inline (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_defun_wiki_inline2 (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_defun_wiki_link (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_defun_wiki_command (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_defun_wiki_command2 (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_get_wiki_command (bool fev, MNode* cell, MlEnv* mlenv);

#endif /* ML_WIKI_H */
