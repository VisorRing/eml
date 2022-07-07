#ifndef ML_TEXP_H
#define ML_TEXP_H

#include "ml.h"
#include "motorvar.h"
class  MlEnv;

class  ListMaker {
 public:
    ListMaker () {};
    virtual  ~ListMaker () {};

    virtual void  append (MNode* e) = 0;
    virtual MNode*  release () = 0;
};

class  ListMakerList: public ListMaker {
 public:
    MNodeList  ans;

    ListMakerList () {};
    virtual  ~ListMakerList () {};
    
    virtual void  append (MNode* e) {
	ans.append (e);
    };
    virtual MNode*  release () {
	return ans.release ();
    };
};

class  ListMakerVector: public ListMaker {
 public:
    MotorVector*  vec;

    ListMakerVector () {
	vec = new MotorVector;
    };
    virtual  ~ListMakerVector () {
	delete vec;
    };
    
    virtual void  append (MNode* e) {
	vec->push (e);
    };
    virtual MNode*  release () {
	MNode*  ans = newMNode_vector (vec);
	vec = NULL;
	return ans;
    };
};

class  ListMakerPtr {
 public:
    ListMaker*  val;

    ListMakerPtr () {
	val = NULL;
    };
    virtual  ~ListMakerPtr () {
	delete val;
	val = NULL;
    };

    virtual bool  assigned () {
	return val != NULL;
    };
    virtual ListMaker*  operator = (ListMaker* v) {
	delete val;
	val = v;
	return val;
    };
    virtual void  append (MNode* e) {
	assert (val);
	val->append (e);
    };
    virtual MNode*  release () {
	assert (val);
	MNode*  ans = val->release ();
	delete val;
	val = NULL;
	return ans;
    };
};

MNode*  ml_vector (bool fev, MNode* cell, MlEnv* mlenv);
//MNode*  ml_vector_get (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_vector_head (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_vector_back (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_vector_size (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_vector_put (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_vector_del (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_vector_unshift (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_vector_shift (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_vector_push (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_vector_pop (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_vector_append (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_vector_resize (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_vector_index (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_vector_slice (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_list_to_vector (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_vector_to_list (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_vector_to_set (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_vector_eval (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_vector_each (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_vector_map (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_vector_reduce (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_vector_reduce_while (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_vector_sort_string (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_table (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_table_get (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_table_put (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_table_del (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_table_append (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_table_keys (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_table_to_list (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_table_eval (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_table_each (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_table_reduce (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_table_reduce_while (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_enum_get (bool fev, MNode* cell, MlEnv* mlenv);

#endif /* ML_TEXP_H */
