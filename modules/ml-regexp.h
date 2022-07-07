#ifndef ML_REGEXP_H
#define ML_REGEXP_H

class MNode;
class MlEnv;

MNode*  ml_regexp_match (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_match_string (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_prematch (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_postmatch (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_string_filter (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_regexp_replace (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_regexp_split_2 (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_regexp_split (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_escape_regexp (bool fev, MNode* cell, MlEnv* mlenv);

#endif /* ML_REGEXP_H */
