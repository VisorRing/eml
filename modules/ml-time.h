#ifndef ML_TIME_H
#define ML_TIME_H

class  MNode;
class  MlEnv;

MNode*  ml_now (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_now_microsec (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_datetime3 (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_date4 (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_time3 (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_datetotime (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_gmdatetime3 (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_gmdate4 (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_gmtime3 (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_gmdatetotime (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_date_delta (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_date_format (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_gmdate_format (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_read_rfc3339_date (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_to_rfc3339_date (bool fev, MNode* cell, MlEnv* mlenv);

#endif /* ML_TIME_H */
