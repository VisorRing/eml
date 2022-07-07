#ifndef MOTORFUNC_H
#define MOTORFUNC_H

#include "ustring.h"
#include <vector>

class  MlEnv;

void  execMotorFunc (const ustring& name, std::vector<ustring>& args, MlEnv* mlenv);

#endif /* MOTORFUNC_H */
