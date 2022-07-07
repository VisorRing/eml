#ifndef ML_MOTOR_H
#define ML_MOTOR_H

class  MNode;
class  MlEnv;

MNode*  ml_response_motor_file (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_response_file (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_response_header (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_output_raw (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_motor_item (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_motor_output (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_motor_output_html (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_motor_output_string (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_forbidden (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_no_content (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_location (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_location_html (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_set_cookie (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_set_header (bool fev, MNode* cell, MlEnv* mlenv);

#endif /* ML_MOTOR_H */
