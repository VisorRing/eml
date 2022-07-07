#include "mlenv.h"
#include "ml.h"
#include "util_check.h"
#include "util_time.h"
#include "util_string.h"
#include "config.h"
#include "motorconst.h"
#include "motorvar.h"
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/unordered_map.hpp>

void  MlPool::setStartTime () {
    starttime = now ();
    limittime = starttime + cDEFAULTTIMELIMIT;
}

bool  MlPool::qtimeup (std::ostream* log) {
    if (now () > limittime) {
	*log << "timeup\n";
	return true;
    } else {
	return false;
    }
}

void  MlPool::inclIncCount () {
    if (includeCount == kIncludeMax)
	throw (uErrorInclNest);
    includeCount ++;
}

bool  MlPool::inclIncCount_nothrow () {
    if (includeCount == kIncludeMax)
	return false;
    includeCount ++;
    return true;
}

void  MlPool::declIncCount () {
    assert (includeCount > 0);
    includeCount --;
}

bool  MlEnv::searchMTable (const ustring& name, XFTableVal*& ans) {
    XFTable::iterator  it;
    int  i;

    if (mlFTable->mtable && (it = mlFTable->mtable->find (name)) != mlFTable->mtable->end ()) {
	ans = it->second;
	if (name[0] == '$') {
	    for (i = mlFTable->mstack.size () - 1; i >= 0; i --) {
		if (mlFTable->mstack[i].mfunc->mlid == ans->mlid) {
		    // nested error.
		    throw (name + ": forbidden.");
		    return false;
		}
	    }
	}
	return true;
    }
    return false;
}

bool  MlEnv::searchSTable (const ustring& name, XFTableVal*& ans, MLFunc*& mobj) {
    int  i;
    MlFTable::MStackVal*  t;
    XFTable::iterator  it;

    for (i = mlFTable->mstack.size () - 1; i >= 0; i --) {
	t = &mlFTable->mstack[i];
	if (t->mobj && t->mfunc->stable && (it = t->mfunc->stable->find (name)) != t->mfunc->stable->end ()) {
	    ans = it->second;
	    mobj = t->mobj;
	    return true;
	}
    }

    return false;
}

bool  MlEnv::searchFTable (const ustring& name, XFTableVal*& ans) {
    XFTable::iterator  it;

    if (mlFTable->ftable && (it = mlFTable->ftable->find (name)) != mlFTable->ftable->end ()) {
	ans = it->second;
	return true;
    } else {
	return false;
    }
}

bool  MlEnv::validName (const ustring& name) {
    umatch  m;
    static uregex  re_var (rMOTORVAR);

    return (regex_match (name, m, re_var));
}

void  MlEnv::setGlobalAry (const ustring& name, size_t i, MNode* val) {
    ustring  a (name);
    a.append (uUScore);
    a.append (to_ustring (i));
    mlPool->globalVar.setVar (a, val);
#ifdef DEBUG
    logSetVar (a, val);
#endif /* DEBUG */
}

void  MlEnv::setGlobalArySize (const ustring& name, size_t n) {
    ustring  a (name);
    MNode*  val;
    a.append (CharConst ("_n"));
    val = newMNode_num (n);
    mlPool->globalVar.setVar (a, val);
#ifdef DEBUG
    logSetVar (a, val);
#endif /* DEBUG */
}

void  MlEnv::setLocalAry (MotorVar* pool, const ustring& name, size_t i, MNode* val) {
    ustring  a (name);
    a.append (uUScore);
    a.append (to_ustring (i));
    pool->setVar (a, val);
#ifdef DEBUG
    logSetVar (a, val, true);
#endif /* DEBUG */
}

void  MlEnv::setLocalArySize (MotorVar* pool, const ustring& name, size_t n) {
    ustring  a (name);
    MNode*  val;
    a.append (CharConst ("_n"));
    val = newMNode_num (n);
    pool->setVar (a, val);
#ifdef DEBUG
    logSetVar (a, val, true);
#endif /* DEBUG */
}

MNode*  MlEnv::getGlobalAry (const ustring& name, size_t i) {
    ustring  a (name);
    a.append (uUScore);
    a.append (to_ustring (i));
    return mlPool->globalVar.getVar (a);
}

size_t  MlEnv::getGlobalArySize (const ustring& name) {
    MNode*  v;
    ustring  a (name);
    a.append (CharConst ("_n"));
    v = mlPool->globalVar.getVar (a);
    if (v && v->isReal ()) {
	return (size_t)v->value_real ();
    } else {
	return 0;
    }
}

MNode*  MlEnv::getLocalAry (MotorVar* pool, const ustring& name, size_t i) {
    ustring  a (name);
    a.append (uUScore);
    a.append (to_ustring (i));
    return pool->getVar (a);
}

size_t  MlEnv::getLocalArySize (MotorVar* pool, const ustring& name) {
    MNode*  v;
    ustring  a (name);

    a.append (CharConst ("_n"));
    v = pool->getVar (a);
    if (v && v->isReal ()) {
	return (size_t)v->value_real ();
    } else {
	return 0;
    }
}

void  MlEnv::setVar (const ustring& name, MNode* val) {
    MotorVar*  pool;

    if (validName (name)) {
	pool = findLocal (name);
	if (pool) {
	    pool->setVar (name, val);
#ifdef DEBUG
	    logSetVar (name, val, true);
#endif /* DEBUG */
	} else {
	    mlPool->globalVar.setVar (name, val);
#ifdef DEBUG
	    logSetVar (name, val);
#endif /* DEBUG */
	}
    } else {
	MNodePtr  p;
	p = val;
#ifdef DEBUG
	logSetVarError (name, val);
#endif /* DEBUG */
    }
}

void  MlEnv::setVar_nolog (const ustring& name, MNode* val) {
    MotorVar*  pool;

    if (validName (name)) {
	pool = findLocal (name);
	if (pool) {
	    pool->setVar (name, val);
	} else {
	    mlPool->globalVar.setVar (name, val);
	}
    } else {
	MNodePtr  p;
	p = val;
#ifdef DEBUG
	logSetVarError (name, val);
#endif /* DEBUG */
    }
}

void  MlEnv::setVar2 (const ustring& name, MNode* val) {
    ustring  sym;

    if (checkAry (name, sym)) {
	setAry (sym, val);
    } else {
	setVar (name, val);
    }
}

void  MlEnv::setAry (const ustring& name, size_t i, MNode* val) {
    MotorVar*  pool;

    if (validName (name)) {
	pool = findLocal (name);
	if (pool) {
	    setLocalAry (pool, name, i, val);
	} else {
	    setGlobalAry (name, i, val);
	}
    } else {
	MNodePtr  p;
	p = val;
#ifdef DEBUG
	logSetAryError (name, i, val);
#endif /* DEBUG */
    }
}

void  MlEnv::setArySize (const ustring& name, size_t n) {
    MotorVar*  pool;

    if (validName (name)) {
	pool = findLocal (name);
	if (pool) {
	    setLocalArySize (pool, name, n);
	} else {
	    setGlobalArySize (name, n);
	}
    } else {
	throw (padEmpty (name) + uErrorBadName);
#ifdef DEBUG
//	logSetArySizeError (name, n);
#endif /* DEBUG */
    }
}

void  MlEnv::setAry (const ustring& name, MNode* list) {
    MotorVar*  pool;
    size_t  n = 0;

    if (validName (name)) {
	pool = findLocal (name);
	if (pool) {
	    if (isNil (list)) {
	    } else if (list->isCons ()) {
		while (list && list->isCons ()) {
		    n ++;
		    setLocalAry (pool, name, n, list->car ());
		    nextNode (list);
		}
	    } else if (list->isVector ()) {
		MotorVector::iterator  b = list->value_vector ()->begin ();
		MotorVector::iterator  e = list->value_vector ()->end ();
		for (; b < e; ++ b) {
		    ++ n;
		    setLocalAry (pool, name, n, (*b) ());
		}
	    } else {
		throw (ustring (CharConst ("setting a scalar value to an array.")));
	    }
	    setLocalArySize (pool, name, n);
	} else {
	    if (isNil (list)) {
	    } else if (list->isCons ()) {
		while (list && list->isCons ()) {
		    n ++;
		    setGlobalAry (name, n, list->car ());
		    nextNode (list);
		}
	    } else if (list->isVector ()) {
		MotorVector::iterator  b = list->value_vector ()->begin ();
		MotorVector::iterator  e = list->value_vector ()->end ();
		for (; b < e; ++ b) {
		    ++ n;
		    setGlobalAry (name, n, (*b) ());
		}
	    } else {
		throw (ustring (CharConst ("setting a scalar value to an array.")));
	    }
	    setGlobalArySize (name, n);
	}
    } else {
	throw (padEmpty (name) + uErrorBadName);
#ifdef DEBUG
//	logSetVarError (name, NULL);
#endif /* DEBUG */
    }
}

MNode*  MlEnv::getVar (const ustring& name) {
    MotorVar*  pool;

    if (validName (name)) {
	pool = findLocal (name);
	if (pool) {
	    return (pool->getVar (name));
	} else {
	    return (mlPool->globalVar.getVar (name));
	}
    } else {
	throw (padEmpty (name) + uErrorBadName);
    }
    return NULL;
}

ustring  MlEnv::getVar_string (const ustring& name) {
    MNode*  v = getVar (name);
    if (v)
	return v->to_string ();
    else
	return uEmpty;
}

MNode*  MlEnv::getAry (const ustring& name, size_t i) {
    MotorVar*  pool;

    if (validName (name)) {
	pool = findLocal (name);
	if (pool) {
	    return getLocalAry (pool, name, i);
	} else {
	    return getGlobalAry (name, i);
	}
    } else {
	throw (padEmpty (name) + uErrorBadName);
    }
    return NULL;
}

ustring  MlEnv::getAry_string (const ustring& name, size_t i) {
    MNode*  v = getAry (name, i);
    if (v)
	return v->to_string ();
    else
	return uEmpty;
}

size_t  MlEnv::getArySize (const ustring& name) {
    MotorVar*  pool;

    if (validName (name)) {
	pool = findLocal (name);
	if (pool) {
	    return getLocalArySize (pool, name);
	} else {
	    return getGlobalArySize (name);
	}
    } else {
	throw (padEmpty (name) + uErrorBadName);
    }
    return 0;
}

void  MlEnv::beginLocal () {
    MotorVar*  v = new MotorVar;
    mlPool->localVar.push_back (v);
}

void  MlEnv::setLocalVar (const ustring& name, MNode* val) {
    MotorVar*  pool = &mlPool->localVar.back ();
    if (validName (name)) {
	pool->setVar (name, val);
#ifdef DEBUG
	logSetVar (name, val, true);
#endif /* DEBUG */
    } else {
	MNodePtr  p;
	p = val;
#ifdef DEBUG
	logSetVarError (name, val);
#endif /* DEBUG */
    }
}

void  MlEnv::defineLocalVar (const ustring& name) {
    MotorVar*  pool = &mlPool->localVar.back ();
    if (validName (name)) {
	pool->setVar (name, NULL);
    }
}

void  MlEnv::endLocal () {
    mlPool->localVar.pop_back ();
}

MotorVar*  MlEnv::findLocal (const ustring& name) {
    boost::ptr_vector<MotorVar>::reverse_iterator  t;
    MotorVar::iterator  it;

    for (t = mlPool->localVar.rbegin (); t != mlPool->localVar.rend (); t ++) {
	it = t->find (name);
	if (it != t->end ()) {
	    return &*t;
	}
    }
    return NULL;
}

#ifdef DEBUG
void  MlEnv::logSetVar (const ustring& name, MNode* val, bool flocal) {
    if (! log || mlPool->nolog)
	return;
    *log << "        ";
    for (int i = 0; i < nestCount; i ++)
	*log << "| ";
//    *log << "[";
    *log << "| >> ";
    if (flocal)
	*log << "*";
    *log <<  name << " <= ";
    if (val)
	*log << val->dump_string_short ();
    else
	*log << uNil;
//    *log << "]\n";
    *log << "\n";
}

void  MlEnv::logSetVarError (const ustring& name, MNode* val) {
    if (! log)
	return;
    for (int i = 0; i < nestCount; i ++)
	*log << "   ";
    *log << "  error: [";
    *log <<  name << " <= ";
    if (val)
	*log << val->dump_string_short ();
    *log << "]\n";
}

void  MlEnv::logSetAryError (const ustring& name, size_t i, MNode* val) {
    if (! log)
	return;
    for (int i = 0; i < nestCount; i ++)
	*log << "   ";
    *log << "  error: [" <<  name << uUScore << i << " <= ";
    if (val)
	*log << val->dump_string_short ();
    *log << "]\n";
}

void  MlEnv::logSetArySizeError (const ustring& name, size_t n) {
    if (! log)
	return;
    *log << "        ";
    for (int i = 0; i < nestCount; i ++)
	*log << "| ";
    *log << "[" <<  name << "_n" << " <= " << n << "]\n";
}
#endif

void  MlEnv::push_linenum (MNode* c, int ln) {
    mlPool->linenum.insert (std::pair<MNode*, int> (c, ln));
}

void  MlEnv::logLinenum (MNode* c) {
    boost::unordered_map<MNode*, int>::iterator  i;

    i = mlPool->linenum.find (c);
    if (i == mlPool->linenum.end ()) {
	*log << "<none>: ";
    } else {
	if (i->second < 10)
	    *log << " ";
	if (i->second < 100)
	    *log << " ";
	if (i->second < 1000)
	    *log << " ";
	if (i->second < 10000)
	    *log << " ";
	if (i->second < 100000)
	    *log << " ";
	*log << i->second << ": ";
    }
}

void  MlEnv::logSexpIndent (MNode* c) {
    int  i;
//    for (i = 0; i < mlPool->includeCount; i ++)
//	*log << ":";
    logLinenum (c);
    for (i = 0; i < nestCount; i ++)
	*log << "| ";
}

void  MlEnv::logSexp (MNode* c) {
    if (!mlPool->nolog && log && c && c->isCons ()) {
	logSexpIndent (c);
	*log << c->dump_string_short () << "\n";
#if 0
	if (c->car () && c->car ()->isSym ()
	    && (match (*ptr_symbol (c->car ()), CharConst ("defun")) || matchHead (*ptr_symbol (c->car ()), CharConst ("defun-")))
	    && c->cdr () && c->cdr ()->isCons ()
	    && c->cdr ()->cdr () && c->cdr ()->cdr ()->isCons ()) {
	    *log << "(" << c->car ()->dump_string ();
	    if (c->cdr ()->car ()) {
		*log << " " << c->cdr ()->car ()->dump_string ();
	    } else {
		*log << " ()";
	    }
	    if (c->cdr ()->cdr ()->car ()) {
		*log << " " << c->cdr ()->cdr ()->car ()->dump_string () << " ...\n";
	    } else {
		*log << " () ...\n";
	    }
	} else {
	    *log << c->dump_string_short () << "\n";
	}
#endif
    }
}

void  MlEnv::logSexpRet (MNode* c) {
//    if (!mlPool->nolog && log && c && c->isCons ()) {
    if (!mlPool->nolog && log) {
	logSexpIndent (c);
	*log << "-> ";
	if (c)
	    *log << c->dump_string_short () << "\n";
	else
	    *log << "()\n";
#if 0
	if (c->car () && c->car ()->isSym ()
	    && (match (*ptr_symbol (c->car ()), CharConst ("defun")) || matchHead (*ptr_symbol (c->car ()), CharConst ("defun-")))
	    && c->cdr () && c->cdr ()->isCons ()
	    && c->cdr ()->cdr () && c->cdr ()->cdr ()->isCons ()) {
	    *log << "(" << c->car ()->dump_string ();
	    if (c->cdr ()->car ()) {
		*log << " " << c->cdr ()->car ()->dump_string ();
	    } else {
		*log << " ()";
	    }
	    if (c->cdr ()->cdr ()->car ()) {
		*log << " " << c->cdr ()->cdr ()->car ()->dump_string () << " ...\n";
	    } else {
		*log << " () ...\n";
	    }
	} else {
	    *log << c->dump_string_short () << "\n";
	}
#endif
    }
}

void  MlEnv::setMStack (MLFunc* mobj) {
    MlFTable::MStackVal*  t = &mlFTable->mstack.back ();

    assert (t->mobj == NULL);
    t->mobj = mobj;
}

void  MlEnv::execDatastoreFunc () {
    int  i;

    for (i = 0; i < datastoreFuncStack.size (); i ++) {
	datastoreFuncStack[i].second (datastoreFuncStack[i].first);
    }
}

