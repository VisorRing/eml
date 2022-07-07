#include "ml-formvar.h"
#include "ml-store.h"
#include "ml.h"
#include "mlenv.h"
#include "formfile.h"
#include "motorenv.h"
#include "util_const.h"
#include "util_check.h"
#include "util_string.h"
#include "expr.h"
#include "utf8.h"
#include "utf16.h"
#include <exception>
#include <iostream>
#include <assert.h>

/*DOC:
==form variable manipulation==

*/
/*DOC:
===input-text===
 (input-text VARIABLE [#query] [#multi] [#trim] [:maxlen NUM] [:filter REGEX]) -> Text | List_of_Text

指定のFORM変数から値を読み込み，Motor変数に格納するか，または，関数の戻り値として値を返す。FORM変数名の先頭に@をつけると，同じ名前で指定した複数のFORM変数を全て読み込み，@をつけないと，最初のひとつのみ読み込む。

^#valueパラメータを指定すると，FORM変数の値は，LISP関数の戻り値として返され，指定しないと，同名のMotor変数に格納される。

===input-textarea===
 (input-textarea VARIABLE [#query] [#multi] [#trim] [:maxlen NUM] [:filter REGEX]) -> TEXT | LIST_of_TEXT

===input-int===
 (input-int VARIABLE [#query] [#multi]) -> NUMBER | LIST_of_NUMBER

===input-real===
 (input-real VARIABLE [#query] [#multi]) -> NUMBER | LIST_of_NUMBER

===input-int-or-blank===
 (input-int-or-blank VARIABLE [#query] [#multi]) -> NIL | NUMBER | LIST_of_NUMBER

===input-real-or-blank===
 (input-real-or-blank VARIABLE [#query] [#multi]) -> NIL | NUMBER | LIST_of_NUMBER

===input-ascii===
 (input-ascii VARIABLE [#query] [#multi] [#trim] [:maxlen NUM] [:filter REGEX]) -> Text | List_of_Text

===input-bool===
 (input-bool VARIABLE [#query] [#multi]) -> BOOL | LIST_of_BOOL

*/

MNode*  FormVarOp::stringNode (const ustring& val) {
    return newMNode_str (new ustring (val));
}

MNode*  FormVarOp::intNode (const ustring& val) {
    long  v = atol (val.c_str ());
    return newMNode_num (v);
}

MNode*  FormVarOp::intBlankNode (const ustring& val) {
    if (val.length () > 0)
	return intNode (val);
    else
	return NULL;
}

MNode*  FormVarOp::doubleNode (const ustring& val) {
    double  v = atof (val.c_str ());
    return newMNode_num (v);
}

MNode*  FormVarOp::doubleBlankNode (const ustring& val) {
    if (val.length () > 0)
	return doubleNode (val);
    else
	return NULL;
}

MNode*  FormVarOp::boolNode (const ustring& val) {
    if (val.length () == 0 || val == uFalse)
	return newMNode_bool (false);
    else
	return newMNode_bool (true);
}

bool  FormVarOp::optFilter (const ustring& name, ustring& val, MlEnv* mlenv) {
    bool  ans = true;

    if (filter.size () > 0) {
	if (wsearch_env (mlenv->regenv, val, filter)) {
	    val = wtou (std::wstring (mlenv->regenv.regmatch[0].first, mlenv->regenv.regmatch[0].second));
	} else {
	    val.resize (0);
	    ans = false;
	}
    }
    if (ftrim) {
	if (ftextarea) {
	    clipNLEnd (val);
	} else {
	    clipWhiteEnd (val);
	}
    }
    if (max > 0) {
	substring (val, 0, max, true, val);
    }
    return ans;
}

static void  formvar_input_readopt (bool fev, MNode*& cell, MlEnv* mlenv, ustring& name, FormVarOp& opt) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    kwParam  kwList[] = {{CharConst ("query"), EV_LIST},	// 0
			 {CharConst ("multi"), EV_LIST},	// 1
			 {CharConst ("trim"), EV_LIST},		// 2
			 {CharConst ("maxlen"), EV_LIST},	// 3
			 {CharConst ("filter"), EV_LIST},	// 4
			 {NULL, 0, EV_END}};
    MNodePtr  kwParams[5];
    evalParams (fev, mlenv, cell, posList, posParams, kwList, kwParams);
    name = to_string (posParams[0]());
    opt.fquery = to_bool (kwParams[0]());
    opt.fmulti = to_bool (kwParams[1]());
    opt.ftrim = to_bool (kwParams[2]());
    opt.max = to_int64 (kwParams[3]());
    if (opt.max < 0) opt.max = 0;
    opt.filter.assign (to_string (kwParams[4]()));
    if (name.size () == 0)
	throw (uErrorVarNameEmpty);
}

MNode*  FormVarOp::input_elem (const ustring& val, MlEnv* mlenv, ustring& name, ustring (*prefn)(const ustring&), MNode* (FormVarOp::*postfn)(const ustring&)) {
    ustring  val2;
    if (prefn)
	val2 = (*prefn) (val);
    else
	val2 = val;
    if (optFilter (name, val2, mlenv)) {
	MNode*  ans = (this->*postfn) (val2);
	return ans;
    } else {
	return NULL;
    }
}

MNode*  FormVarOp::input (MlEnv* mlenv, ustring& name, ustring (*prefn)(const ustring&), MNode* (FormVarOp::*postfn)(const ustring&)) {
    MNodePtr  h;
    ustring  val;

    if (fmulti) {
	size_t  n = mlenv->env->form->atSize (name);
	MNodeList  ans;
	for (size_t i = 0; i < n; ++ i) {
	    mlenv->env->form->at (name, i, val);
	    ans.append (input_elem (val, mlenv, name, prefn, postfn));
	}
	return mlenv->retval = ans.release ();
    } else {
	mlenv->env->form->at (name, val);
	return input_elem (val, mlenv, name, prefn, postfn);
    }
    return NULL;
}

//#XAFUNC	input-text	ml_formvar_input_text
//#XWIKIFUNC	input-text
MNode*  ml_formvar_input_text (bool fev, MNode* cell, MlEnv* mlenv) {
    MNode*  arg = cell->cdr ();
    ustring  name;
    FormVarOp  opt;

    formvar_input_readopt (fev, cell, mlenv, name, opt);
    return opt.input (mlenv, name, omitCtrl, &FormVarOp::stringNode);
}

//#XAFUNC	input-textarea	ml_formvar_input_textarea
//#XWIKIFUNC	input-textarea
MNode*  ml_formvar_input_textarea (bool fev, MNode* cell, MlEnv* mlenv) {
    MNode*  arg = cell->cdr ();
    ustring  name;
    FormVarOp  opt;

    formvar_input_readopt (fev, cell, mlenv, name, opt);
    opt.ftextarea = true;
    return opt.input (mlenv, name, omitCtrlX, &FormVarOp::stringNode);
}

//#XAFUNC	input-int	ml_formvar_input_int
//#XWIKIFUNC	input-int
MNode*  ml_formvar_input_int (bool fev, MNode* cell, MlEnv* mlenv) {
    MNode*  arg = cell->cdr ();
    ustring  name;
    FormVarOp  opt;

    formvar_input_readopt (fev, cell, mlenv, name, opt);
    return opt.input (mlenv, name, NULL, &FormVarOp::intNode);
}

//#XAFUNC	input-real	ml_formvar_input_real
//#XWIKIFUNC	input-real
MNode*  ml_formvar_input_real (bool fev, MNode* cell, MlEnv* mlenv) {
    MNode*  arg = cell->cdr ();
    ustring  name;
    FormVarOp  opt;

    formvar_input_readopt (fev, cell, mlenv, name, opt);
    return opt.input (mlenv, name, NULL, &FormVarOp::doubleNode);
}

//#XAFUNC	input-int-or-blank	ml_formvar_input_int_blank
//#XWIKIFUNC	input-int-or-blank
MNode*  ml_formvar_input_int_blank (bool fev, MNode* cell, MlEnv* mlenv) {
    MNode*  arg = cell->cdr ();
    ustring  name;
    FormVarOp  opt;

    formvar_input_readopt (fev, cell, mlenv, name, opt);
    return opt.input (mlenv, name, NULL, &FormVarOp::intBlankNode);
}

//#XAFUNC	input-real-or-blank	ml_formvar_input_real_blank
//#XWIKIFUNC	input-real-or-blank
MNode*  ml_formvar_input_real_blank (bool fev, MNode* cell, MlEnv* mlenv) {
    MNode*  arg = cell->cdr ();
    ustring  name;
    FormVarOp  opt;

    formvar_input_readopt (fev, cell, mlenv, name, opt);
    return opt.input (mlenv, name, NULL, &FormVarOp::doubleBlankNode);
}

//#XAFUNC	input-ascii	ml_formvar_input_ascii
//#XWIKIFUNC	input-ascii
MNode*  ml_formvar_input_ascii (bool fev, MNode* cell, MlEnv* mlenv) {
    MNode*  arg = cell->cdr ();
    ustring  name;
    FormVarOp  opt;

    formvar_input_readopt (fev, cell, mlenv, name, opt);
    return opt.input (mlenv, name, omitNonAscii, &FormVarOp::stringNode); // XXX jp
}

//#XAFUNC	input-bool	ml_formvar_input_bool
//#XWIKIFUNC	input-bool
MNode*  ml_formvar_input_bool (bool fev, MNode* cell, MlEnv* mlenv) {
    MNode*  arg = cell->cdr ();
    ustring  name;
    FormVarOp  opt;

    formvar_input_readopt (fev, cell, mlenv, name, opt);
    return opt.input (mlenv, name, NULL, &FormVarOp::boolNode);
}

static MNode*  getFile (MlEnv* mlenv, const FormVarOp& opt, const ustring& var, int i, const ustring& savename, bool fnumbering) {
    ustring  filename;
    int  idx = mlenv->env->form->at (var, i, filename);
    idx = mlenv->env->form->partAt (idx);
    if (idx >= 0 && filename.length () > 0) {
	ustring  dst = savename;
	if (fnumbering) {
	    dst.append (uUScore);
	    dst.append (to_ustring (i + 1));
	}
	ustring  dstpath = mlenv->env->path_store_file (dst);
	MNodeList  el;
	mlenv->env->form->saveFile (idx, dstpath, opt.max);
	el.append (newMNode_str (new ustring (dst)));
	el.append (newMNode_str (new ustring (filename)));
	el.append (newMNode_str (new ustring (mlenv->env->form->typeAt (idx))));
#ifdef DEBUG
	if (mlenv->log) {
	    *mlenv->log << "	[file: \"" << savename << "\"]\n";
	}
#endif /* DEBUG */
	return el.release ();
    } else {
	return NULL;
    }
}

/*DOC:
===input-file===
 (input-file VARIABLE STORE-FILENAME [#multi] [:maxsize NUM]) -> LIST_of_SavedName_OrigName_Type | LIST_of_LIST_of_SavedName_OrigName_type

*/
//#XAFUNC	input-file	ml_formvar_input_file
//#XWIKIFUNC	input-file
MNode*  ml_formvar_input_file (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_LIST, EV_END};
    MNodePtr  posParams[2];
    kwParam  kwList[] = {{CharConst ("multi"), EV_LIST},   // 0
			 {CharConst ("maxsize"), EV_LIST}, // 1
			 {NULL, 0, EV_END}};
    MNodePtr  kwParams[2];
    evalParams (fev, mlenv, cell, posList, posParams, kwList, kwParams);
    ustring  var = to_string (posParams[0]());
    ustring  fname = to_string (posParams[1]());
    FormVarOp  opt;
    opt.fmulti = to_bool (kwParams[0]());
    opt.max = to_int64 (kwParams[1]());
    if (opt.max < 0)
	opt.max = 0;
    if (mlenv->env->storedir.empty ()) {
	mlenv->setVar (uXSerial, newStoreSerial (mlenv));
    }
    if (opt.fmulti) {
	MNodeList  ans;
	int  n = mlenv->env->form->atSize (var);
	for (int i = 0; i < n; ++ i) {
	    MNode*  e = getFile (mlenv, opt, var, i, fname, true);
	    if (e)
		ans.append (e);
	}
	return ans.release ();
    } else {
	return getFile (mlenv, opt, var, 0, fname, false);
    }
}

/*DOC:
===input-file@===
 (input-file@ ARRAY FILE_PREFIX) -> LIST_of_a_List_of_SavedFileName_OriginalFileName_and_Type

*/
//#XAFUNC	input-file@	ml_formvar_input_file_a
//#XWIKIFUNC	input-file@
MNode*  ml_formvar_input_file_a (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_LIST, EV_END};
    MNodePtr  posParams[2];
    evalParams (fev, mlenv, cell, posList, posParams);
    ustring  name = to_string (posParams[0]());
    ustring  prefix = to_string (posParams[1]());
    if (mlenv->env->storedir.empty ()) {
	mlenv->setVar (uXSerial, newStoreSerial (mlenv));
    }
    ustring  target0 = mlenv->env->path_store_file (prefix);
    size_t  i, n;
    int  idx;
    MNodeList  ans;
    ustring  tgt;
    ustring  si;
    ustring  filename;
    ustring  tgtname;
    n = mlenv->env->form->atSize (name);
    for (i = 0; i < n; i ++) {
	idx = mlenv->env->form->at (name, i, filename);
#ifdef DEBUG2
	std::cerr << "i:" << idx << " ";
#endif /* DEBUG */
	idx = mlenv->env->form->partAt (idx);
#ifdef DEBUG2
	std::cerr << i << "/" << n << ":" << idx << "\n";
#endif /* DEBUG */
	if (idx >= 0) {
	    si.assign (to_ustring (i + 1));
	    tgt.assign (target0).append (si);
	    tgtname.assign (prefix).append (si);
	    // ++ filter
	    if (filename.length () > 0) {
		MNodeList  l;
		mlenv->env->form->saveFile (idx, tgt, 0);	// ++ opt.max
		l.append (newMNode_str (new ustring (tgtname)));
		l.append (newMNode_str (new ustring (filename)));
		l.append (newMNode_str (new ustring (mlenv->env->form->typeAt (idx))));
		ans.append (l.release ());
#ifdef DEBUG
		if (mlenv->log) {
		    *mlenv->log << "	[file: \"" << tgtname << "\"]\n";
		}
#endif /* DEBUG */
	    }
	}
    }
    return mlenv->retval = ans.release ();
}

/*DOC:
===query-elements===
 (query-elements VARIABLE) -> NUM

*/
//#XAFUNC	query-elements	ml_formvar_query_elements
//#XWIKIFUNC	query-elements
MNode*  ml_formvar_query_elements (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    evalParams (fev, mlenv, cell, posList, posParams);
    ustring  var = to_string (posParams[0]());
    return newMNode_num (mlenv->env->form->atSize (var));
}

/*DOC:
===read-query-string===
 (read-query-string) -> NIL

*/
//#XAFUNC	read-query-string	ml_read_query_string
MNode*  ml_read_query_string (bool fev, MNode* cell, MlEnv* mlenv) {
    evalParams (fev, mlenv, cell);
    if (mlenv->env && mlenv->env->form) {
	if (mlenv->env->form->method != CGIForm::M_GET) {
	    mlenv->env->form->read_get ();
	}
    }
    return NULL;
}

/*DOC:
===query-string===
 (query-string) -> TEXT

*/
//#XAFUNC	query-string	ml_query_string
MNode*  ml_query_string (bool fev, MNode* cell, MlEnv* mlenv) {
    evalParams (fev, mlenv, cell);
    if (mlenv->env && mlenv->env->form) {
	return newMNode_str (new ustring (mlenv->env->form->queryString));
    }
    return NULL;
}
