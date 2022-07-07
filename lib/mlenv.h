#ifndef MLENV_H
#define MLENV_H

#include "util_wsearch.h"
#include "ftable.h"
#include "motorvar.h"
#include <boost/unordered_map.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <vector>
#include <iostream>

class  MNode;
class  MotorEnv;

class  MlPool {
 public:
    boost::unordered_map<MNode*, int>  linenum;
    MotorVar  globalVar;
    boost::ptr_vector<MotorVar>  localVar;
#ifdef DEBUG
    MotorSet  nodebugFunc;
#endif /* DEBUG */
    time_t  starttime;
    time_t  limittime;
    int  includeCount;
    MotorSet  includedFile;
    MNodePtr  breaksym;
    MNodePtr  breakval;
    bool  nolog;

    MlPool () {
	includeCount = 0;
	nolog = false;
	starttime = limittime = 0;
    };
    virtual  ~MlPool () {};

    virtual void  setStartTime ();
    virtual bool  qtimeup (std::ostream* log);
    virtual void  resetProg () {
	breaksym = NULL;
	breakval = NULL;
    };
    virtual void  breakProg () {
	breaksym = new MNode;
	breaksym ()->set_car (new MNode);
	breakval = NULL;
    };
    virtual void  inclIncCount ();
    virtual bool  inclIncCount_nothrow ();
    virtual void  declIncCount ();
};

class  MlFTable {
 public:
    XFTable*  ftable;
    XMTable*  mtable;
    class  MStackVal {
    public:
	XFTableVal*  mfunc;
	MLFunc*  mobj;

	MStackVal (XFTableVal* mf): mfunc (mf), mobj (NULL) {};
	virtual  ~MStackVal () {};
    };
    std::vector<MStackVal>  mstack;

    MlFTable () {
	ftable = NULL;
	mtable = NULL;
    };
    virtual  ~MlFTable () {};

    virtual void  setFTable (XFTable* ft, XMTable* mt) {
	ftable = ft;
	mtable = mt;
    };
};

class  MlEnv {
 public:
    typedef  void (*datastoreFunc_t)(MLFunc*);
    class  autoDatastoreFunc {
    public:
	MlEnv*  mlenv;
	
	autoDatastoreFunc (MlEnv* _mlenv, MLFunc* mobj, datastoreFunc_t fn) {
	    mlenv = _mlenv;
	    mlenv->datastoreFuncStack.push_back (std::pair<MLFunc*,datastoreFunc_t> (mobj, fn));
	};
	virtual  ~autoDatastoreFunc () {
	    mlenv->datastoreFuncStack.pop_back ();
	};
    };

    MlPool*  mlPool;
    MlFTable*  mlFTable;
    std::vector<std::pair<MLFunc*,datastoreFunc_t> >  datastoreFuncStack;
//    std::wstring  regtext;
//    boost::wsmatch  regmatch;
    WSearchEnv  regenv;
    MNodePtr  retval;
    MNodePtr  currentCell;
    MotorEnv*  env;
    std::ostream*  log;
    int  nestCount;
    MNodePtr  constLambda;

    MlEnv (MlPool* _mlpool, MlFTable* _mlftable) {
	mlPool = _mlpool;
	mlFTable = _mlftable;
	env = NULL;
	log = NULL;
	nestCount = -1;
	constLambda = newMNode_sym (new ustring (uLambda));
    };
    virtual  ~MlEnv () {};

    virtual bool  searchMTable (const ustring& name, XFTableVal*& ans);
    virtual bool  searchSTable (const ustring& name, XFTableVal*& ans, MLFunc*& mobj);
    virtual bool  searchFTable (const ustring& name, XFTableVal*& ans);
    virtual void  pushMStack (XFTableVal* obj) {
	mlFTable->mstack.push_back (MlFTable::MStackVal (obj));
    };
    virtual void  popMStack () {
	mlFTable->mstack.pop_back ();
    };
    virtual void  setStartTime () {
	mlPool->setStartTime ();
    };
    virtual bool  qtimeup () {
	return mlPool->qtimeup (log);
    };
    virtual void  resetProg () {
	mlPool->resetProg ();
    };
    virtual void  breakProg () {
	mlPool->breakProg ();
    };
    virtual void  inclIncCount () {
	mlPool->inclIncCount ();
    };
    virtual bool  inclIncCount_nothrow () {
	return mlPool->inclIncCount_nothrow ();
    };
    virtual void  declIncCount () {
	mlPool->declIncCount ();
    };
    virtual bool  validName (const ustring& name);
    virtual void  setGlobalAry (const ustring& name, size_t i, MNode* val);
    virtual void  setGlobalArySize (const ustring& name, size_t n);
    virtual void  setLocalAry (MotorVar* pool, const ustring& name, size_t i, MNode* val);
    virtual void  setLocalArySize (MotorVar* pool, const ustring& name, size_t n);
    virtual MNode*  getGlobalAry (const ustring& name, size_t i);
    virtual size_t  getGlobalArySize (const ustring& name);
    virtual MNode*  getLocalAry (MotorVar* pool, const ustring& name, size_t i);
    virtual size_t  getLocalArySize (MotorVar* pool, const ustring& name);
    virtual void  setVar (const ustring& name, MNode* val);
    virtual void  setVar_nolog (const ustring& name, MNode* val);
    virtual void  setVar2 (const ustring& name, MNode* val);
    virtual void  setAry (const ustring& name, size_t i, MNode* val);
    virtual void  setArySize (const ustring& name, size_t n);
    virtual void  setAry (const ustring& name, MNode* list);
    virtual MNode*  getVar (const ustring& name);
    virtual ustring  getVar_string (const ustring& name);
    virtual MNode*  getAry (const ustring& name, size_t i);
    virtual ustring  getAry_string (const ustring& name, size_t i);
    virtual size_t  getArySize (const ustring& name);
    virtual void  beginLocal ();
    virtual void  setLocalVar (const ustring& name, MNode* val);
    virtual void  defineLocalVar (const ustring& name);
    virtual void  endLocal ();
    virtual MotorVar*  findLocal (const ustring& name);
#ifdef DEBUG
    virtual void  logSetVar (const ustring& name, MNode* val, bool flocal = false);
    virtual void  logSetVarError (const ustring& name, MNode* val);
    virtual void  logSetAryError (const ustring& name, size_t i, MNode* val);
    virtual void  logSetArySizeError (const ustring& name, size_t n);
#endif /* DEBUG */

    virtual void  push_linenum (MNode* c, int ln);
    virtual void  logLinenum (MNode* c);
    virtual void  logSexpIndent (MNode* c);
    virtual void  logSexp (MNode* c);
    virtual void  logSexpRet (MNode* c);
    virtual void  setMStack (MLFunc* mobj);
    virtual void  execDatastoreFunc ();
    virtual MNode*  breaksym () {
	return mlPool->breaksym ();
    };
    virtual MNode*  breakval () {
	return mlPool->breakval ();
    };
    virtual void  setBreak (MNode* sym, MNode* val) {
	mlPool->breaksym = sym;
	mlPool->breakval = val;
    };
    virtual bool  stopBreak (MNode* fnname) {
	if (breaksym ()
	    && (breaksym ()->isNil ()
		|| equal (breaksym (), fnname))) {
	    setBreak (NULL, NULL);
	    return true;
	}
	return false;
    };
#ifdef DEBUG
    virtual void  setNodebugFunc (const ustring& name) {
	mlPool->nodebugFunc.set (name);
    };
#endif /* DEBUG */
    virtual bool  includedFile (const ustring& name) {
	return mlPool->includedFile.get (name);
    };
    virtual void  setIncludedFile (const ustring& name) {
	mlPool->includedFile.set (name);
    };
};

template<class T>  class  AutoDelete {
 public:
    T*  p;

    AutoDelete (): p (NULL) {};
    virtual  ~AutoDelete () {
	delete p;
	p = NULL;
    };
    T*  operator () () {
	return p;
    };
    T*  operator = (T* b) {
	delete p;
	p = b;
	return p;
    };
    T*  release () {
	T*  ans = p;
	p = NULL;
	return ans;
    };
};

class  AutoLocalVariable {
 public:
    MlEnv*  mlenv;

    AutoLocalVariable (MlEnv* e) {
	mlenv = e;
	mlenv->beginLocal ();
    };
    virtual  ~AutoLocalVariable () {
	mlenv->endLocal ();
    };
};

class  AutoInclCount {
 public:
    MlEnv*  mlenv;
    int  count;

    AutoInclCount (MlEnv* e) {
	mlenv = e;
	count = 0;
    };
    virtual  ~AutoInclCount () {
	assert (count <= 1);
	if (count == 1)
	    mlenv->declIncCount ();
    };
    void  inc () {
	mlenv->inclIncCount ();
	count ++;
    };
    bool  inc_test () {
	if (mlenv->inclIncCount_nothrow ()) {
	    count ++;
	    return true;
	} else {
	    return false;
	}
    };
};

class  AutoBackupBool {
public:
    bool*  ptr;
    bool  b;

    AutoBackupBool (bool* _ptr) {
	ptr = _ptr;
	b = *ptr;
    };
    virtual  ~AutoBackupBool () {
	*ptr = b;
    };
};

template<class T> class  AutoBackupPtr {
 public:
    T**  ptr;
    T*  p;

    AutoBackupPtr (T** _p, T* _v) {
	ptr = _p;
	p = *ptr;
	*ptr = _v;
    };
    virtual  ~AutoBackupPtr () {
	*ptr = p;
    };
};

class  AutoCallCount {
 public:
    MlEnv*  mlenv;
    AutoCallCount (MlEnv* _env): mlenv (_env) {
	if (! mlenv->mlPool->nolog)
	    mlenv->nestCount ++;
    };
    virtual  ~AutoCallCount () {
	if (! mlenv->mlPool->nolog)
	    mlenv->nestCount --;
    };
};

#endif /* MLENV_H */
