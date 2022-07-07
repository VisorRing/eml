#ifndef ML_ADDON_H
#define ML_ADDON_H

class  MNode;
class  MlEnv;

MNode*  ml_addon (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_addon_tab (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_addon_output (bool fev, MNode* cell, MlEnv* mlenv);

#endif /* ML_ADDON_H */
