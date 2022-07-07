#ifndef ML_ENCODE_H
#define ML_ENCODE_H

class  MNode;
class  MlEnv;

MNode*  ml_encode_control_char (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_decode_control_char (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_escape_wiki (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_percent_encode (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_percent_decode (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_base64_encode (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_base64_decode (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_base64_url_encode (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_base64_url_decode (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_hex_encode (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_hex_decode (bool fev, MNode* cell, MlEnv* mlenv);

#endif /* ML_ENCODE_H */
