#ifndef EXPR_H
#define EXPR_H

#include "ml.h"
#include "ustring.h"
#include <vector>

class  MlEnv;
class  MotorVar;

typedef struct {
    const char*  name;
    size_t  namelen;
    bool  fbool;
}  paramList;

typedef enum {
    EV_END,
    EV_LIST,
    EV_ASIS,
}  paramType;

typedef struct {
    const char*  name;
    size_t  namelen;
    paramType  type;
}  kwParam;

MNode*  eval (MNode* ptr, MlEnv* mlenv);
MNode*  vectorDup (MNode* c);
MNode*  vectorEval (MNode* c, MlEnv* mlenv);
MNode*  tableDup (MNode* c);
MNode*  tableEval (MNode* c, MlEnv* mlenv);
int64_t  eval_int64 (MNode* ptr, MlEnv* mlenv);
double  eval_double (MNode* ptr, MlEnv* mlenv);
ustring  eval_str (MNode* ptr, MlEnv* mlenv);
ustring  eval_text1 (MNode* cell, MlEnv* mlenv);
ustring  eval_asciiword (MNode* cell, MlEnv* mlenv);
bool  eval_bool (MNode* ptr, MlEnv* mlenv);
MNode*  eval_fn (bool fev, MNode* fn, MNode* cell, MlEnv* mlenv);
MNode*  eval_builtin_fn (bool fev, XFTableVal* fn, MNode* cell, MlEnv* mlenv);
MNode*  eval_builtin_mfn (bool fev, XFTableVal* fn, MNode* cell, MlEnv* mlenv);
MNode*  eval_builtin_sfn (bool fev, XFTableVal* fn, MLFunc* senv, MNode* cell, MlEnv* mlenv);
MNode*  eval_lambda (bool fev, MNode* fn, MNode* cell, MlEnv* mlenv);
ustring  eval_file (MNode* cell, MlEnv* mlenv);
MNode*  progn (MNode* arg, MlEnv* mlenv);
void  progn_ex (MNode* arg, MlEnv* mlenv);
void  checkDefun (MNode* cell, ustring& rname, MNode*& sexp);
MNode*  newLambda (MNode* cell);
MNode*  buildArgs (int start, const std::vector<ustring>& args);
MNode*  buildArgs (int start, const std::vector<ustring>& args, const ustring& arg2);
MNode*  buildArgs (const ustring& arg1);
bool  checkDefunArgs (MNode* lambda, MNode* values);
//MNode*  execDefun (MlEnv* mlenv, MotorVar* pool, const ustring& name, MNode* values);
//MNode*  execDefun (MlEnv* mlenv, MNode* lambda, MNode* values, MNode* name);
MNode*  execDefun (MlEnv* mlenv, MNode* params, MNode* body, MNode* args, const ustring& name);
inline MNode*  execDefun (MlEnv* mlenv, MNode* lambda, MNode* args, const ustring& name) { assert (lambda->isLambda ()); return execDefun (mlenv, lambda->lambda.sexp->car (), lambda->lambda.sexp->cdr (), args, name); }	// *** DUMMY
MNode*  onErrorFn (MNode* fn, MlEnv* mlenv);
void  setParams (MNode* list, int nparam, std::vector<MNode*>* params, paramList *kwlist, std::vector<MNode*>* keywords, MNode** rest, bool padding = false);
void  evalParams (bool fev, MlEnv* mlenv, MNode* cell, const paramType posList[] = NULL, MNodePtr posParams[] = NULL, const kwParam kwList[] = NULL, MNodePtr kwParams[] = NULL, paramType restType = EV_END, MNodePtr* restParams = NULL);
inline void  evalParams (bool fev, MlEnv* mlenv, MNode* cell, paramType restType, MNodePtr* restParams) { evalParams (fev, mlenv, cell, NULL, NULL, NULL, NULL, restType, restParams);}

#endif /* EXPR_H */
