#include "ml-defun.h"
#include "motorconst.h"
#include "expr.h"
#include "ml.h"
#include "mlenv.h"
#include "motorenv.h"
#include "util_const.h"
#include "ustring.h"
#include <vector>
#include <exception>

/*DOC:
==function definition==

*/
/*DOC:
===defun===
 (defun NAME LIST BLOCK...) -> NIL

*/
//#XAFUNC	defun	ml_defun	norval
MNode*  ml_defun (bool fev, MNode* cell, MlEnv* mlenv) {
    MNode*  sexp = NULL;
    ustring  name;
    MNode*  ans;

    checkDefun (cell->cdr (), name, sexp);
    ans = newLambda (sexp);
    mlenv->setVar_nolog (name, ans);
    return ans;
}

/*DOC:
===defun-motor===
 (defun-motor NAME LIST BLOCK...) -> NIL

*/
//#XAFUNC	defun-motor	ml_defun_motor	norval
MNode*  ml_defun_motor (bool fev, MNode* cell, MlEnv* mlenv) {
    MNode*  sexp = NULL;
    ustring  name;
    MNode*  ans;

    checkDefun (cell->cdr (), name, sexp);
    ans = newLambda (sexp);
    mlenv->env->motorCall.setVar (name, ans);
    return ans;
}

/*DOC:
===lambda===
 (lambda LIST BLOCK...) -> LAMBDA

*/
//#XAFUNC	lambda	ml_lambda	norval
//#XWIKIFUNC	lambda
MNode*  ml_lambda (bool fev, MNode* cell, MlEnv* mlenv) {
    MNode*  sexp = NULL;
    ustring  name;

    checkDefun (cell, name, sexp);
    return newLambda (sexp);
}

