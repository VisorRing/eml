#ifndef ML_SENDMAIL_H
#define ML_SENDMAIL_H

class  MNode;
class  MlEnv;

MNode*  ml_sendmail (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_mail_address_p (bool fev, MNode* cell, MlEnv* mlenv);

#endif /* ML_SENDMAIL_H */
