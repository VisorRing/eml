#include "expr.h"
#include "ml.h"
#include "mlenv.h"
#include "ftable.h"
#include "motorvar.h"
#include "util_const.h"
#include "util_string.h"
#include "util_check.h"
#include "ustring.h"
#include <boost/unordered_map.hpp>
#include <iostream>
#include <stdlib.h>
#include <assert.h>

MNode*  eval (MNode* cell, MlEnv* mlenv) {
    if (! cell)
	return NULL;
    switch (cell->type) {
    case MNode::MC_NIL:
	return NULL;
    case MNode::MC_CONS:
	{
	    MNodePtr  f;
	    f = eval (cell->car (), mlenv);
	    return eval_fn (true, f (), cell, mlenv);
	}
	break;
    case MNode::MC_STR:
	return cell;
    case MNode::MC_SYM: {
	const ustring*  u = ptr_symbol (cell);
	if (u->length () > 0) {
	    switch ((*u)[0]) {
	    case '#':
	    case ':':
		return cell;
	    }
	}
	if (*u == uTrue) {
	    return mlTrue;
	} else if (*u == uFalse) {
	    return NULL;
	} else if (*u == uNil) {
	    return NULL;
	}
	XFTableVal*  t;
	MLFunc*  senv;
	if (mlenv->searchFTable (*u, t)) {
	    // normal function
	    return newMNode_builtin_fn (t);
	} else if (mlenv->searchMTable (*u, t)) {
	    return newMNode_builtin_mfn (t);
	} else if (mlenv->searchSTable (*u, t, senv)) {
	    return newMNode_builtin_sfn (t, senv);
	} else {
	    return mlenv->getVar (*u);
	}
    }
    case MNode::MC_INT64:
	return cell;
    case MNode::MC_DOUBLE:
	return cell;
    case MNode::MC_VECTOR:
//	return vectorDup (cell);
	return vectorEval (cell, mlenv);
    case MNode::MC_TABLE:
//	return tableDup (cell);
	return tableEval (cell, mlenv);
    case MNode::MC_LIBOBJ:
	return cell;
    case MNode::MC_BOOL:
	return cell;
    default:
	assert (0);
    }
    return NULL;			// not reached
}

MNode*  vectorDup (MNode* c) {
    MNodePtr  ans;
    MNode*  x;
    MotorVector::iterator  b, e;

    assert (c && c->isVector ());
    x = newMNode_vector ();
    ans = x;
    b = c->value_vector ()->begin ();
    e = c->value_vector ()->end ();
    for (; b < e; ++ b) {
	x->vectorPush ((*b) ());
    }
    return ans.release ();
}

MNode*  vectorEval (MNode* c, MlEnv* mlenv) {
    MNodePtr  ans;
    MNode*  x;
    MotorVector::iterator  b, e;

    assert (c && c->isVector ());
    x = newMNode_vector ();
    ans = x;
    b = c->value_vector ()->begin ();
    e = c->value_vector ()->end ();
    for (; b < e; ++ b) {
#ifdef DEBUG2
	std::cerr << "eval:" << to_string ((*b) ()) << "\n";
#endif /* DEBUG */
	x->vectorPush (eval ((*b) (), mlenv));
    }
    return ans.release ();
}

MNode*  tableDup (MNode* c) {
    MNodePtr  ans;
    MNode*  x;
    MotorVar::iterator  b, e;

    assert (c && c->isTable ());
    x = newMNode_table ();
    ans = x;
    b = c->value_table ()->begin ();
    e = c->value_table ()->end ();
    for (; b != e; ++ b) {
	x->tablePut ((*b).first, (*b).second ());
    }

    return ans.release ();
}

MNode*  tableEval (MNode* c, MlEnv* mlenv) {
    MNodePtr  ans;
    MNode*  x;
    MotorVar::iterator  b, e;

    assert (c && c->isTable ());
    x = newMNode_table ();
    ans = x;
    b = c->value_table ()->begin ();
    e = c->value_table ()->end ();
    for (; b != e; ++ b) {
#ifdef DEBUG2
//	std::cerr << "eval:" << to_string ((*b) ()) << "\n";
#endif /* DEBUG */
	x->tablePut ((*b).first, eval ((*b).second (), mlenv)); // テーブルのキーは、文字列。
    }
#ifdef DEBUG
    mlenv->logSexp (ans ());
#endif /* DEBUG */

    return ans.release ();
}

int64_t  eval_int64 (MNode* cell, MlEnv* mlenv) {
    MNodePtr  p;
    p = eval (cell, mlenv);
    return to_int64 (p ());
}

double  eval_double (MNode* cell, MlEnv* mlenv) {
    MNodePtr  p;
    p = eval (cell, mlenv);
    return to_double (p ());
}

ustring  eval_str (MNode* cell, MlEnv* mlenv) {
    MNodePtr  p;
    p = eval (cell, mlenv);
    return to_string (p ());
}

ustring  eval_text1 (MNode* cell, MlEnv* mlenv) {
    MNodePtr  p;
    p = eval (cell, mlenv);
    return to_text1 (p ());
}

ustring  eval_asciiword (MNode* cell, MlEnv* mlenv) {
    MNodePtr  p;
    p = eval (cell, mlenv);
    return to_asciiword (p ());
}

bool  eval_bool (MNode* cell, MlEnv* mlenv) {
    MNodePtr  p;
    p = eval (cell, mlenv);
    return to_bool (p ());
}

MNode*  eval_fn (bool fev, MNode* fn, MNode* cell, MlEnv* mlenv) {
    if (fn) {
	switch (fn->type) {
	case MNode::MC_BUILTIN_FN:
	    return eval_builtin_fn (fev, fn->bfn, cell, mlenv);
	case MNode::MC_BUILTIN_MFN:
	    return eval_builtin_mfn (fev, fn->bfn, cell, mlenv);
	case MNode::MC_BUILTIN_SFN:
	    return eval_builtin_sfn (fev, fn->sfn, fn->senv, cell, mlenv);
	case MNode::MC_LAMBDA:
	    return eval_lambda (fev, fn, cell, mlenv);
	default:;
	}
    }
    throw (cell->dump_string_short () + ustring (CharConst (": undefined function")));
}

MNode*  eval_builtin_fn (bool fev, XFTableVal* fn, MNode* cell, MlEnv* mlenv) {
#ifdef DEBUG
    AutoCallCount  ac (mlenv);
//    if (! (cell && isSym (cell-> car()) && *(ptr_symbol (cell->car ())) == uQuote))
    if (! fn->notrace_fn)
	mlenv->logSexp (cell);
#endif /* DEBUG */
    MNode*  ans = NULL;
    MNodePtr  bcell;
    bcell = mlenv->currentCell;
    mlenv->currentCell = cell;
    ans = fn->fn (fev, cell, mlenv);
    mlenv->currentCell = bcell;
#ifdef DEBUG
//    if (! (cell && isSym (cell-> car()) && *(ptr_symbol (cell->car ())) == uQuote))
    if (! fn->notrace_rval)
	mlenv->logSexpRet (ans);
#endif /* DEBUG */
    return ans;
}

MNode*  eval_builtin_mfn (bool fev, XFTableVal* fn, MNode* cell, MlEnv* mlenv) {
#ifdef DEBUG
    AutoCallCount  ac (mlenv);
    if (! fn->notrace_fn)
	mlenv->logSexp (cell);
#endif /* DEBUG */
    MNode*  ans;
    MNodePtr  bcell;
    mlenv->pushMStack (fn);
    bcell = mlenv->currentCell;
    ans = fn->fn (fev, cell, mlenv);
    mlenv->currentCell = bcell;
    mlenv->popMStack ();
#ifdef DEBUG
    if (! fn->notrace_rval)
	mlenv->logSexpRet (ans);
#endif /* DEBUG */
    return ans;
}

MNode*  eval_builtin_sfn (bool fev, XFTableVal* fn, MLFunc* senv, MNode* cell, MlEnv* mlenv) {
#ifdef DEBUG
    AutoCallCount  ac (mlenv);
    if (! fn->notrace_fn)
	mlenv->logSexp (cell);
#endif /* DEBUG */
    MNode*  ans;
    MNodePtr  bcell;
    bcell = mlenv->currentCell;
    mlenv->currentCell = cell;
    ans = fn->sfn (fev, cell, mlenv, senv);
    mlenv->currentCell = bcell;
#ifdef DEBUG
    if (! fn->notrace_rval)
	mlenv->logSexpRet (ans);
#endif /* DEBUG */
    return ans;
}

//MNode*  eval_lambda (bool fev, MNode* fn_body, MNode* cell, MlEnv* lambda_env, MlEnv* mlenv) {
MNode*  eval_lambda (bool fev, MNode* lambda, MNode* cell, MlEnv* mlenv) {
    MNodeList  args;
    MNode*  a = cell->cdr ();
    MNode*  ans = NULL;

    assert (lambda->isLambda ());
#ifdef DEBUG
    AutoCallCount  ac (mlenv);
    mlenv->logSexp (cell);
#endif /* DEBUG */
    while (a) {
	if (fev)
	    args.append (eval (a->car (), mlenv));
	else
	    args.append (a->car ());
	nextNode (a);
    }
#ifdef DEBUG
//    if (mlenv->mlPool->nodebugFunc.get (*name)) {
//	AutoBackupBool  autoBackup (&mlenv->mlPool->nolog);
//	mlenv->mlPool->nolog = true;
//	ans = execDefun (mlenv, fn->sexp, list (), cell->car ());
//    } else {
//	ans = execDefun (mlenv, fn_body->car (), fn_body->cdr (), list (), to_string (cell->car ()));
//	ans = execDefun (mlenv, fn_body->car (), fn_body->cdr (), list (), to_string (cell->car ()));
//	ans = execDefun (mlenv, lambda->lambda.sexp->car (), lambda->lambda.sexp->cdr (), args (), to_string (cell->car ()));
    if (lambda->lambdaModeBit (MNode::MODE_NOTRACE)) {
	AutoBackupBool  autoBackup (&mlenv->mlPool->nolog);
	mlenv->mlPool->nolog = true;
	ans = execDefun (mlenv, lambda->lambdaParams (), lambda->lambdaBody (), args (), to_string (cell->car ()));
    } else if (lambda->lambdaModeBit (MNode::MODE_TRACE)) {
	AutoBackupBool  autoBackup (&mlenv->mlPool->nolog);
	mlenv->mlPool->nolog = false;
	ans = execDefun (mlenv, lambda->lambdaParams (), lambda->lambdaBody (), args (), to_string (cell->car ()));
    } else {
	ans = execDefun (mlenv, lambda->lambdaParams (), lambda->lambdaBody (), args (), to_string (cell->car ()));
    }
    mlenv->logSexpRet (ans);
#else
//    ans = execDefun (mlenv, lambda->lambda.sexp->car (), lambda->lambda.sexp->cdr (), args (), to_string (cell->car ()));
    ans = execDefun (mlenv, lambda->lambdaParams (), lambda->lambdaBody (), args (), to_string (cell->car ()));
#endif /* DEBUG */
    return ans;
}

ustring  eval_file (MNode* cell, MlEnv* mlenv) {
    ustring  ans = eval_str (cell, mlenv);
    if (! matchFilename (ans))	// XXX dummy
	ans.resize (0);
    return ans;
}

MNode*  progn (MNode* arg, MlEnv* mlenv) {
    MNodePtr  ans;

    if (! arg)
	return NULL;

    assert (arg->isCons ());
    while (arg && ! mlenv->breaksym ()) {
	if (arg->car ()) {
	    ans = eval (arg->car (), mlenv);
	    if (mlenv->breaksym ())
		return mlenv->breakval ();
	} else {
	    ans = NULL;
	}
	nextNode (arg);
    }
    if (mlenv->breaksym ())
	return mlenv->breakval ();
    return ans.release ();
}

void  progn_ex (MNode* arg, MlEnv* mlenv) {
    MNodePtr  ans;

    assert (arg && arg->isCons ());
    if (arg->cdr () && arg->cdr ()->isCons ()) {
	arg = arg->cdr ();
	mlenv->mlPool->resetProg ();
	while (arg && ! mlenv->breaksym ()) {
	    if (arg->car () && arg->car ()->isCons ()) {
		ans = eval (arg->car (), mlenv);
		if (mlenv->breaksym ()) {
		    mlenv->setBreak (NULL, NULL);
		    return;
		}
	    }
	    nextNode (arg);
	}
    }
}

void  checkDefun (MNode* arg, ustring& name, MNode*& sexp) {
    MNode*  param;
    MNode*  body;
    MNode*  a;
    
    if (! arg)
	throw (uErrorWrongNumber);
    if (! arg->car ())
	throw (ustring (CharConst ("bad name.")));

    name = arg->car ()->to_string ();
    nextNodeNonNil (arg);
    sexp = arg;

    param = arg->car ();
    body = arg->cdr ();
    if ((param && ! param->isNil () && ! param->isCons ())
	|| (body && ! body->isNil () && ! body->isCons ())) {
	throw (uErrorWrongType);
    }
    for (a = param; a && a->isCons (); nextNode (a)) {
	if (! a->car () || ! a->car ()->isSym ())
	    throw (param->dump_string () + uErrorBadType);
    }
}

MNode*  newLambda (MNode* cell) {	// cellは ((PARAMS ...) BODY ...)
    if (! cell || ! cell->isCons ())
	throw (uErrorSyntax);

    MNode*  ans = new MNode;
//    ans->set_car (newMNode_sym (new ustring (uLambda)));
//    ans->set_cdr (cell);
    uint32_t  mode = 0x0;
    MNode*  p = cell->cdr ();
    while (isCons (p)) {
	MNode*  a = p->car ();
	if (isSym (a)) {
	    if (match (*a->value_sym (), CharConst ("no-trace"))) {
		mode |= MNode::MODE_NOTRACE;
	    } else if (match (*a->value_sym (), CharConst ("trace"))) {
		mode |= MNode::MODE_TRACE;
	    } else if (match (*a->value_sym (), CharConst ("public-function"))) {
		mode |= MNode::MODE_WIKIPUBLIC;
	    } else if (match (*a->value_sym (), CharConst ("no-decode"))) {
		mode |= MNode::MODE_WIKINODEC;
	    } else if (match (*a->value_sym (), CharConst ("no-decode2"))) {
		mode |= MNode::MODE_WIKINODEC2;
	    } else if (match (*a->value_sym (), CharConst ("raw-output"))) {
		mode |= MNode::MODE_WIKIRAW;
	    } else if (match (*a->value_sym (), CharConst ("wiki-output"))) {
		mode |= MNode::MODE_WIKIOUT;
	    } else if (match (*a->value_sym (), CharConst ("text-output"))) {
		mode |= MNode::MODE_TEXTOUT;
	    } else {
		break;
	    }
	} else {
	    break;
	}
	nextNode (p);
    }
    ans->set_lambda (cell, NULL, mode);	// ((PARAMS ...) BODY ...)
    return ans;
}

MNode*  buildArgs (int start, const std::vector<ustring>& args) {
    MNodeList  ans;

    for (; start < args.size (); start ++) {
	ans.append (newMNode_str (new ustring (args[start])));
    }
    return ans.release ();
}

MNode*  buildArgs (int start, const std::vector<ustring>& args, const ustring& arg2) {
    MNodeList  ans;

    ans.append (newMNode_str (new ustring (arg2)));
    for (; start < args.size (); start ++) {
	ans.append (newMNode_str (new ustring (args[start])));
    }
    return ans.release ();
}

MNode*  buildArgs (const ustring& arg1) {
    MNodeList  ans;

    ans.append (newMNode_str (new ustring (arg1)));
    return ans.release ();
}

class  KwList: public boost::unordered_map<ustring, std::pair<bool,MNode*> > {
public:
    KwList () {};
    ~KwList () {};
    void  insertVar (const ustring& name, bool f) {
	erase (name);
	insert (KwList::value_type (name, std::pair<bool,MNode*> (f, NULL)));
    };
    void  setVar (const ustring& name, MNode* val) {
	KwList::iterator  it = find (name);
	if (it == end ()) {
	} else {
	    it->second.second = val;
	}
    };
    MNode*  getVar (const ustring& name) {
	KwList::iterator  it = find (name);
	if (it == end ()) {
	    return NULL;
	} else {
	    return it->second.second;
	}
    };
    bool  defined (const ustring& name) {
	KwList::iterator  it = find (name);
	return (it != end ());
    };
    bool  definedBoolType (const ustring& name) {
	KwList::iterator  it = find (name);
	return (it != end () && it->second.first);
    };
};

bool  checkDefunArgs (MNode* lambda, MNode* values) {
//    MNode*  sexp = lambda->cdr ();
    MNode*  body = lambda->lambdaBody ();
    MNode*  param = lambda->lambdaParams ();
    AutoDelete<KwList>  kwlist;
    const ustring*  u;
    ustring  k;
    bool  skip;

    for (; param; nextNode (param)) {
	u = ptr_symbol (param->car ());
	if (match (*u, CharConst ("&rest"))) {
	    break;
	} else if (match (*u, CharConst ("&key"))) {
	    if (kwlist ()) {
		return false;		// &key appeared.
	    } else {
		kwlist = new KwList;
	    }
	} else {
	    if (kwlist ()) {
		kwlist ()->insertVar (*u, true);
	    } else {
	    }
	}
    }
    skip = false;
    for (param = lambda->lambdaParams (); param;) {
	u = NULL;
	if (kwlist () && values && values->car ()->isSym ()
	    && (u = ptr_symbol (values->car ()))->length () > 0
	    && (*u)[0] == '#'
	    && kwlist ()->defined (k = ustring (u->begin () + 1, u->end ()))) {
	    nextNode (values);
	} else if (u && u->length () > 0
		   && (*u)[0] == ':'
		   && kwlist ()->defined (k = ustring (u->begin () + 1, u->end ()))) {
	    nextNode (values);
	    nextNode (values);
	} else if (match (*(u = ptr_symbol (param->car ())), CharConst ("&rest"))) {
	    nextNode (param);
	    if (param) {
		values = NULL;
	    } else {
//		throw (sexp->car ()->dump_string_short () + uErrorBadParamDef);
		return false;
	    }
	    break;
	} else if (match (*u, CharConst ("&key"))) {
	    skip = true;
	    nextNode (param);
	} else {
	    if (skip) {
		nextNode (param);
	    } else {
		nextNode (param);
		nextNode (values);
	    }
	}
    }
    for (; values;) {
	u = NULL;
	if (kwlist () && values && values->car ()->isSym ()
	    && (u = ptr_symbol (values->car ()))->length () > 0
	    && (*u)[0] == '#'
	    && kwlist ()->defined (k = ustring (u->begin () + 1, u->end ()))) {
	    nextNode (values);
	} else if (u && u->length () > 0
		   && (*u)[0] == ':'
		   && kwlist ()->defined (ustring (u->begin () + 1, u->end ()))) {
	    nextNode (values);
	    nextNode (values);
	} else {
	    return false;
	}
    }
    return true;
}

MNode*  execDefun (MlEnv* mlenv, MNode* params, MNode* body, MNode* args, const ustring& name) {
    MNode*  par;
    MNodePtr  ans;
    AutoDelete<KwList>  kwlist;
    const ustring*  u;
    ustring  k;
    bool  skip;
    MNode*  args0 = args;

    mlenv->beginLocal ();
    // it is assumed that param is a list of symbols.
    for (par = params; par; nextNode (par)) {
	u = ptr_symbol (par->car ());
	if (match (*u, CharConst ("&rest"))) {
	    break;
	} else if (match (*u, CharConst ("&key"))) {
	    if (kwlist ()) {
		throw (params->dump_string_short () + uErrorBadParamDef);
	    } else {
		kwlist = new KwList;
	    }
	} else {
	    if (kwlist ()) {
		kwlist ()->insertVar (*u, true);
		mlenv->setLocalVar (*u, NULL);
	    } else {
	    }
	}
    }

    skip = false;
    for (par = params; par;) {
	u = NULL;
	if (kwlist () && args && args->car () && args->car ()->isSym ()
	    && (u = ptr_symbol (args->car ()))->length () > 0
	    && (*u)[0] == '#'
	    && kwlist ()->defined (k = ustring (u->begin () + 1, u->end ()))) {
	    mlenv->setLocalVar (k, mlTrue);
	    nextNode (args);
	} else if (u && u->length () > 0
		   && (*u)[0] == ':'
		   && kwlist ()->defined (k = ustring (u->begin () + 1, u->end ()))) {
	    nextNode (args);
	    if (args)
		mlenv->setLocalVar (k, args->car ());
	    else
		mlenv->setLocalVar (k, NULL);
	    nextNode (args);
	} else if (match (*(u = ptr_symbol (par->car ())), CharConst ("&rest"))) {
	    nextNode (par);
	    if (par) {
		mlenv->setLocalVar (*ptr_symbol (par->car ()), args);
		args = NULL;
	    } else {
		throw (params->dump_string_short () + uErrorBadParamDef);
	    }
	    break;
	} else if (match (*u, CharConst ("&key"))) {
	    skip = true;
	    nextNode (par);
	} else {
	    if (skip) {
		nextNode (par);
	    } else {
		if (args)
		    mlenv->setLocalVar (*u, args->car ());
		else
		    mlenv->setLocalVar (*u, NULL);
		nextNode (par);
		nextNode (args);
	    }
	}
    }
    for (; args;) {
	u = NULL;
	if (kwlist () && args && args->car ()->isSym ()
	    && (u = ptr_symbol (args->car ()))->length () > 0
	    && (*u)[0] == '#'
	    && kwlist ()->defined (k = ustring (u->begin () + 1, u->end ()))) {
	    mlenv->setLocalVar (k, mlTrue);
	    nextNode (args);
	} else if (u && u->length () > 0
		   && (*u)[0] == ':'
		   && kwlist ()->defined (ustring (u->begin () + 1, u->end ()))) {
	    nextNode (args);
	    if (args)
		mlenv->setLocalVar (k, args->car ());
	    else
		mlenv->setLocalVar (k, NULL);
	    nextNode (args);
	} else {
//	    throw (uErrorWrongNumber);
//	    throw (lambda->dump_string_short () + args0->dump_string_short () + ": " + uErrorWrongNumber);
	    MNodePtr  a;
	    a = new MNode;
	    a ()->set_car (newMNode_sym (new ustring (name)));
	    a ()->set_cdr (args0);
	    throw (a ()->dump_string_short () + ": " + uErrorWrongNumber);
	}
    }

    ans = progn (body, mlenv);
    mlenv->stopBreak (mlenv->constLambda ());
    mlenv->endLocal ();

    return mlenv->retval = ans.release ();
}

MNode*  onErrorFn (MNode* fn, MlEnv* mlenv) {
    MNodePtr  ag;

    ag = new MNode;
    ag ()->set_car (mlenv->currentCell ());
    return execDefun (mlenv, fn, ag (), uEmpty); // mlenv->retvalに保存している
}

void  setParams (MNode* list, int nparam, std::vector<MNode*>* params, paramList *kwlist, std::vector<MNode*>* keywords, MNode** rest, bool padding) {
    KwList*  kw = NULL;
    const ustring*  u;
    MNode*  a;
    int  i;
    ustring  name;

    if (kwlist) {
	kw = new KwList;
	for (i = 0; kwlist[i].name; i ++) {
	    kw->insertVar (ustring (kwlist[i].name, kwlist[i].namelen), kwlist[i].fbool);
	}
    }

    while (list) {
	a = list->car ();
	if (a && kw && a->isSym () && (u = ptr_symbol (a)) && u->size () > 0) {
	    switch ((*u)[0]) {
	    case ':':
		name = ustring (u->begin () + 1, u->end ());
		if (kw->defined (name)) {
		    nextNode (list);
		    if (list) {
			kw->setVar (name, list->car ());
			nextNode (list);
		    } else {
			delete kw;
			throw (uErrorWrongNumber);
		    }
		} else {
		    delete kw;
		    throw (uQ2 + *u + uQ2 + uErrorBadParam);
		}
		break;
	    case '#':
		name = ustring (u->begin () + 1, u->end ());
		if (kw->definedBoolType (name)) {
		    nextNode (list);
		    kw->setVar (name, mlTrue);
		} else {
		    delete kw;
		    throw (uQ2 + *u + uQ2 + uErrorBadParam);
		}
		break;
	    default:
		goto Bp1;
	    }
	} else {
	Bp1:;
	    if (params && (params->size () < nparam || (nparam == 0 && rest == NULL))) {
		nextNode (list);
		params->push_back (a);
	    } else {
		break;
	    }
	}
    }

    if (rest) {
	*rest = list;
    } else if (list) {
	delete kw;
	throw (uErrorWrongNumber);
    }

    if (params && params->size () < nparam) {
	if (padding) {
	    while (params->size () < nparam) {
		params->push_back (NULL);
	    }
	} else {
	    delete kw;
	    throw (uErrorWrongNumber);
	}
    }

    if (kwlist && keywords) {
	for (i = 0; kwlist[i].name; i ++) {
	    keywords->push_back (kw->getVar (ustring (kwlist[i].name, kwlist[i].namelen)));
	}
    }

    delete kw;
}

static int  listMatch (const kwParam* kwList, ustring& name) {
    for (int i = 0; kwList[i].type != EV_END; ++ i) {
	if (match (name, kwList[i].name, kwList[i].namelen)) {
	    return i;
	}
    }
    return -1;
}

void  evalParams (bool fev, MlEnv* mlenv, MNode* cell, const paramType posList[], MNodePtr posParams[], const kwParam kwList[], MNodePtr kwParams[], paramType restType, MNodePtr* restParams) {
    MNode*  args = cell->cdr ();
    MNode*  a;
    const ustring*  u;
    ustring  name;
    int  p;
    int  pos = 0;
    MNodeList  rest;

    while (isCons (args)) {
	a = args->car ();
	args = args->cdr ();
	if (a && kwList && a->isSym () && (u = ptr_symbol (a)) && u-> size () > 0) {
	    switch ((*u)[0]) {
	    case ':':
		if (! kwList)
		    throw (a->dump_string_short () + uErrorBadParam);
		name = ustring (u->begin () + 1, u->end ());
		if ((p = listMatch (kwList, name)) >= 0) {
		    if (isCons (args)) {
			a = args->car ();
			args = args->cdr ();
			if (fev) {
			    kwParams[p] = eval (a, mlenv);
			} else {
			    kwParams[p] = a;
			}
		    } else {
			throw (args->dump_string_short () + uErrorBadParam);
		    }
		} else {
		    throw (a->dump_string_short () + uErrorBadParam);
		}
		break;
	    case '#':
		if (! kwList)
		    throw (a->dump_string_short () + uErrorBadParam);
		name = ustring (u->begin () + 1, u->end ());
		if ((p = listMatch (kwList, name)) >= 0) {
		    kwParams[p] = newMNode_bool (true);
		} else {
		    throw (a->dump_string_short () + uErrorBadParam);
		}
		break;
	    default:;
		goto Bp1;
	    }
	} else {
	Bp1:;
	    if (posList && posList[pos] != EV_END && posParams) {
		switch (posList[pos]) {
		case EV_LIST:
		    if (fev) {
			posParams[pos] = eval (a, mlenv);
		    } else {
			posParams[pos] = a;
		    }
		    ++ pos;
		    break;
		case EV_ASIS:
		    posParams[pos] = a;
		    ++ pos;
		    break;
		case EV_END:
		    break;
		}
	    } else {
		switch (restType) {
		case EV_LIST:
		    if (fev) {
			rest.append (eval (a, mlenv));
		    } else {
			rest.append (a);
		    }
		    break;
		case EV_ASIS:
		    rest.append (a);
		    break;
		default:;
		    throw (a->dump_string_short () + ": " + uErrorWrongNumber);
		}
	    }
	}
    }
    if (restParams) {
	*restParams = rest.release ();
    }
}
