#ifndef ML_JS_H
#define ML_JS_H

class  MNode;
class  MlEnv;

MNode*  ml_encode_js (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_to_json (bool fev, MNode* cell, MlEnv* mlenv);

#endif /* ML_JS_H */
