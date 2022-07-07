#ifndef MOTOR_FUNCTION_H
#define MOTOR_FUNCTION_H

#include "motor.h"
#include "ustring.h"
#include <vector>

class  MlEnv;

void  mf_eval (const std::vector<ustring>& args, MlEnv* mlenv);
void  mf_raw (const std::vector<ustring>& args, MlEnv* mlenv);
//void  mf_js (const std::vector<ustring>& args, MlEnv* mlenv);
void  mf_js (const std::vector<ustring>& args, const ustring& arg2, MlEnv* mlenv);
void  mf_url (const std::vector<ustring>& args, MlEnv* mlenv);
void  mf_pad0 (const std::vector<ustring>& args, MlEnv* mlenv);
void  mf_wiki (const std::vector<ustring>& args, MlEnv* mlenv);
//void  mf_date (const std::vector<ustring>& args, MlEnv* mlenv);
void  mf_date (const std::vector<ustring>& args, const ustring& arg2, MlEnv* mlenv);

#endif /* MOTOR_FUNCTION_H */
