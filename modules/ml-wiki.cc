#include "ml-wiki.h"
#include "mlenv.h"
#include "wikiformat.h"
#include "wikienv.h"
#include "motorenv.h"
#include "expr.h"
#include "util_const.h"
#include "util_check.h"
#include "ustring.h"
#include <exception>

/*DOC:
==wiki processing==

*/

static void  wiki_sub (bool fev, MNode* cell, MlEnv* mlenv, ustring& text, bool& super, ustring& element) {
    kwParam  kwList[] = {{CharConst ("super"), EV_LIST},	// 0
			 {CharConst ("element"), EV_LIST},	// 1
			 {NULL, 0, EV_END}};
    MNodePtr  kwParams[2];
    MNodePtr  rest;
    evalParams (fev, mlenv, cell, NULL, NULL, kwList, kwParams, EV_LIST, &rest);
    super = to_bool (kwParams[0]());
    element = to_string (kwParams[1]());
    MNode*  args = rest ();
    while (args) {
	text.append (to_string (args->car ()));
	nextNode (args);
    }
}

/*DOC:
===wiki===
 (wiki #super :element NAME TEXT ...) -> NIL

*/
//#XAFUNC	wiki	ml_wiki
MNode*  ml_wiki (bool fev, MNode* cell, MlEnv* mlenv) {
    ustring  text;
    bool  super = false;
    ustring  element;

    wiki_sub (fev, cell, mlenv, text, super, element);

    if (! mlenv->env->responseDone)
	mlenv->env->standardResponse_html ();

    WikiFormat  w (mlenv->env, true);
    if (element.size () == 0) {
	w.compile (text, super);
    } else {
	w.compileElement (text, super, element);
    }
    w.output ();

    return NULL;
}

/*DOC:
===wiki-string===
 (wiki-string #super :element NAME TEXT ...) -> STRING

*/
//#XAFUNC	wiki-string	ml_wiki_string
MNode*  ml_wiki_string (bool fev, MNode* cell, MlEnv* mlenv) {
    ustring  text;
    bool  super = false;
    ustring  element;
    MotorOutputString  out;
    AutoSwapMotor  swap (mlenv->env->output, out);

    wiki_sub (fev, cell, mlenv, text, super, element);

    WikiFormat  w (mlenv->env, true);
    if (element.size () == 0) {
	w.compile (text, super);
    } else {
	w.compileElement (text, super, element);
    }
    w.output ();

    return newMNode_str (new ustring (out.ans));
}

/*DOC:
===wiki-eval===
 (wiki-eval EXPR) -> ANY

WikiコンテクストでEXPRを評価する。

*/
//#XAFUNC	wiki-eval	ml_wiki_eval
MNode*  ml_wiki_eval (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    evalParams (fev, mlenv, cell, posList, posParams, NULL, NULL);
    WikiFormat  wiki (mlenv->env, true);
    return mlenv->retval = eval (posParams[0](), wiki.mlenv);
}

static ustring*  wikivar (const ustring& name) {
    ustring*  ans = new ustring;
    ans->reserve (uWiki.length () + name.length ());
    ans->append (uWiki);
    ans->append (name);
    return ans;
}

/*DOC:
===wikivar===
 (wikivar NAME) -> STRING

Return string prefixed with 'wiki_'.

*/
//#XAFUNC	wikivar	ml_wikivar
MNode*  ml_wikivar (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    evalParams (fev, mlenv, cell, posList, posParams);
    ustring  name = to_string (posParams[0]());
    return newMNode_str (wikivar (name));
}

static void  setWikivar (const ustring& name, MlEnv* mlenv) {
    AutoDelete<ustring>  var;
    var = wikivar (name);
    mlenv->setVar (*var (), mlenv->getVar (name));
}

/*DOC:
===set-wikivar===
 (set-wikivar [VARIABLE | LIST] ...) -> NIL

*/
//#XAFUNC	set-wikivar	ml_set_wikivar
MNode*  ml_set_wikivar (bool fev, MNode* cell, MlEnv* mlenv) {
    MNodePtr  rest;
    evalParams (fev, mlenv, cell, EV_LIST, &rest);
    MNode*  args = rest ();
    while (args) {
	setWikivar (to_string (args->car ()), mlenv);
	nextNode (args);
    }
    return NULL;
}

static void defaultLambdaBit (MNode* node, uint32_t fieldbit, uint32_t defaultbit) {
    assert (node->type == MNode::MC_LAMBDA);
    if (node->lambda.mode & fieldbit) {
    } else {
	node->lambda.mode |= defaultbit;
    }
}

/*DOC:
===defun-wiki-inline===
 (defun-wiki-inline FNAME (ARGS...) BLOCK...) -> NIL
 &[;[FNAME:ARGS:...]]

===defun-wiki-inline2===
 (defun-wiki-inline2 FNAME (ARG2 ARGS...) BLOCK...) -> NIL
 &[;[FNAME:ARGS:... ARGS2]]

*/
//#XAFUNC	defun-wiki-inline	ml_defun_wiki_inline	norval
MNode*  ml_defun_wiki_inline (bool fev, MNode* cell, MlEnv* mlenv) {
    ustring  name;
    MNode*  sexp = NULL;
    MNode*  ans;

    checkDefun (cell->cdr (), name, sexp);
    ans = newLambda (sexp);
    defaultLambdaBit (ans, MNode::MODE_WIKIRAW | MNode::MODE_WIKIOUT | MNode::MODE_TEXTOUT, MNode::MODE_TEXTOUT);
    mlenv->env->wikienv->wikiFunc.setVar (name, ans);

    return ans;
}

//#XAFUNC	defun-wiki-inline2	ml_defun_wiki_inline2	norval
MNode*  ml_defun_wiki_inline2 (bool fev, MNode* cell, MlEnv* mlenv) {
    ustring  name;
    MNode*  sexp = NULL;
    MNode*  ans;

    checkDefun (cell->cdr (), name, sexp);
    ans = newLambda (sexp);
    defaultLambdaBit (ans, MNode::MODE_WIKIRAW | MNode::MODE_WIKIOUT | MNode::MODE_TEXTOUT, MNode::MODE_TEXTOUT);
    mlenv->env->wikienv->wikiFunc2.setVar (name, ans);

    return ans;
}

/*DOC:
===defun-wiki-link===
 (defun-wiki-link FNAME (ARGS...) BLOCK...) -> NIL
 |select:NAME:FNAME:ARGS:...|
 {form:POST:FNAME:ARGS:...

*/
//#XAFUNC	defun-wiki-link	ml_defun_wiki_link	norval
MNode*  ml_defun_wiki_link (bool fev, MNode* cell, MlEnv* mlenv) {
    ustring  name;
    MNode*  sexp = NULL;
    MNode*  ans;

    checkDefun (cell->cdr (), name, sexp);
    ans = newLambda (sexp);
//    defaultLambdaBit (ans, MNode::MODE_WIKIRAW | MNode::MODE_WIKIOUT, MNode::MODE_WIKIRAW);
    mlenv->env->wikienv->wikiLink.setVar (name, ans);

    return ans;
}

/*DOC:
===defun-wiki-command===
 (defun-wiki-command FNAME (ARGS...) BLOCK...) -> NIL
 $FNAME:ARGS:...

===defun-wiki-command2===
 (defun-wiki-command2 FNAME (ARGS...) BLOCK...) -> NIL
 $FNAME:ARGS:...
 	:
 $end
*/
//#XAFUNC	defun-wiki-command	ml_defun_wiki_command	norval
//#XAFUNC	defun-wiki-command2	ml_defun_wiki_command2	norval
MNode*  ml_defun_wiki_command (bool fev, MNode* cell, MlEnv* mlenv) {
    ustring  name;
    MNode*  sexp = NULL;
    MNode*  ans;

    checkDefun (cell->cdr (), name, sexp);
    ans = newLambda (sexp);
    mlenv->env->wikienv->wikiCmd.setVar (name, ans);

    return ans;
}

MNode*  ml_defun_wiki_command2 (bool fev, MNode* cell, MlEnv* mlenv) {
    ustring  name;
    MNode*  sexp = NULL;
    MNode*  ans;

    checkDefun (cell->cdr (), name, sexp);
    ans = newLambda (sexp);
    mlenv->env->wikienv->wikiCmd2.setVar (name, ans);

    return ans;
}

/*DOC:
===get-wiki-command===
 (get-wiki-command NAME) -> LAMBDA

*/
//#XAFUNC	get-wiki-command	ml_get_wiki_command
MNode*  ml_get_wiki_command (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    evalParams (fev, mlenv, cell, posList, posParams);
    ustring  name = to_string (posParams[0]());
    return mlenv->env->wikienv->wikiCmd.getVar (name);
}
