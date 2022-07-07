#ifndef ML_FORMVAR_H
#define ML_FORMVAR_H

#include "ml.h"
#include "ustring.h"
#include <unistd.h>

class  MNode;
class  MlEnv;

class  FormVarOp {
 public:
    bool  fquery;
    bool  fmulti;
    bool  ftextarea;
    bool  ftrim;
    size_t  max;
    ustring  filter;
	
    FormVarOp () {
	fquery = false;
	fmulti = false;
	ftextarea = false;
	ftrim = false;
	max = 0;
    };
    virtual  ~FormVarOp () {};

    virtual MNode*  stringNode (const ustring& val);
    virtual MNode*  intNode (const ustring& val);
    virtual MNode*  intBlankNode (const ustring& val);
    virtual MNode*  doubleNode (const ustring& val);
    virtual MNode*  doubleBlankNode (const ustring& val);
    virtual MNode*  boolNode (const ustring& val);
    virtual bool  optFilter (const ustring& name, ustring& val, MlEnv* mlenv);
    virtual MNode*  input (MlEnv* mlenv, ustring& name, ustring (*prefn)(const ustring&), MNode* (FormVarOp::*postfn)(const ustring&));
    virtual MNode*  input_elem (const ustring& val, MlEnv* mlenv, ustring& name, ustring (*prefn)(const ustring&), MNode* (FormVarOp::*postfn)(const ustring&));
};

MNode*  ml_formvar_input_text (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_formvar_input_textarea (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_formvar_input_int (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_formvar_input_real (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_formvar_input_int_blank (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_formvar_input_real_blank (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_formvar_input_ascii (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_formvar_input_bool (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_formvar_input_file (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_formvar_input_file_a (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_formvar_query_elements (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_read_query_string (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_query_string (bool fev, MNode* cell, MlEnv* mlenv);

#endif /* ML_FORMVAR_H */
