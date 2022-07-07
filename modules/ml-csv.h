#ifndef ML_CSV_H
#define ML_CSV_H

class  MNode;
class  MlEnv;

MNode*  ml_csv_read (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_a_csv_to_record (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_a_csv_decode (bool fev, MNode* cell, MlEnv* mlenv);

#endif /* ML_CSV_H */
