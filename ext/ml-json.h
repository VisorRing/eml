#ifndef ML_JSON_H
#define ML_JSON_H

class  MNode;
class  MlEnv;

MNode*  ml_json_texp_read (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_json_texp_output (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_json_texp_output_string (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_input_json (bool fev, MNode* cell, MlEnv* mlenv);

#endif /* ML_JSON_H */
