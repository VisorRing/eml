#include "wikicmd.h"
#include "wikitable.h"
#include "wikiformat.h"
#include "wikienv.h"
#include "wikimotor.h"
#include "ml-formvar.h"
#include "ml-variable.h"
#include "ml.h"
#include "expr.h"
#include "motorenv.h"
#include "motoroutput.h"
#include "util_const.h"
#include "util_check.h"
#include "util_string.h"
#include "ustring.h"
#include <boost/ptr_container/ptr_vector.hpp>
#include <assert.h>
#include <stdlib.h>

//#define  WIKIREPEATPROTECT	1
//#define  WIKIEVALPROTECT	1

/*
WIKICOMPAT: 1をセットすると，defun-wiki-commandで定義した関数の中で，motor-output, motor-output-rawで出力したテキストをWikiテキストとして解釈する。
*/
/*DOC:
$pre-mode:1
==行コマンド==

*/
/* ============================================================ */
static void  do_linevec (WikiLine::linevec* block, WikiFormat* wiki) {
    if (block) {
	WikiLineScanner  scanner (block);
	wiki->compileLines (scanner);
    }
}

static MNode*  parse_arg (const ustring& text, WikiFormat* wiki) {
    MotorTexp  ml (wiki->mlenv);
    ml.scan (text);
    if (ml.top.isCons ()) {
	return wiki->mlenv->retval = ml.top.cdr ();
    }
    return NULL;
}

static MNode*  eval_car (MNode* cons, WikiFormat* wiki) {
    MNode*  ans = NULL;
    if (cons && cons->isCons ()) {
	try {
	    ans = eval (cons->car (), wiki->mlenv);
	} catch (ustring& msg) {
	    if (wiki->mlenv->currentCell ()) {
		wiki->errorMsg.append (wiki->mlenv->currentCell ()->dump_string_short ()).append (CharConst (": "));
	    }
	    wiki->errorMsg.append (msg).append (uLF);
	}
    }
    return ans;
}

/* ============================================================ */
/*DOC:
===$repeat===
 $repeat:''VAR'':''FROM'':''TO''[:''STEP'']
 ...
 $end
変数''VAR''に，数値''FROM''から''TO''までの値を１づつ加算しながら代入し，$endまでの間のブロックを繰り返し実行する。

*/
//#WIKICMD	$repeat	$end	wc_repeat
void  wc_repeat (WikiLine* wl, WikiFormat* wiki) {
    WikiMotor  motor (wl->begin, wl->end, wiki);
    WikiMotorObjVec  objv;
    WikiMotorObjVecVec  args;
    ustring  iv;
    int64_t  from = 0;
    int64_t  to = 0;
    int64_t  step = 1;
    int64_t  i;
    MNodePtr  h;

#ifdef WIKIREPEATPROTECT
    if (wiki->protectMode && ! wl->fsuper) {
#ifdef DEBUG
	std::cerr << "(wiki):(protected)\n";
#endif /* DEBUG */
	return;
    }
#endif

#ifdef DEBUG
    std::cerr << "(wiki):" << ustring (wl->begin0, wl->end) << "\n";
#endif /* DEBUG */
    motor.compile (objv);
    objv.splitCharA (':', args);
    if (args.size () == 3 || args.size () == 4) {
	iv = clipWhite (args[0]->textOut (wiki));
	from = to_int64 (args[1]->textOut (wiki));
	to = to_int64 (args[2]->textOut (wiki));
	if (args.size () == 4)
	    step = to_int64 (args[3]->textOut (wiki));
    } else {
	wiki->errorMsg.append (CharConst ("$repeat: wrong number of parameters.\n"));
	return;
    }

    if (step > 0) {
	for (i = from; i <= to; i += step) {
	    h = newMNode_num (i);
	    wiki->mlenv->setVar (iv, h ());
	    do_linevec (wl->block, wiki);
//	    if (mlenv->qtimeup ())
//		break;
	}
    } else if (step < 0) {
	for (i = from; i >= to; i += step) {
	    h = newMNode_num (i);
	    wiki->mlenv->setVar (iv, h ());
	    do_linevec (wl->block, wiki);
//	    if (wiki->mlenv->qtimeup ())
//		break;
	}
    }
}

/* ============================================================ */
/*DOC:
===$block===
 $block:''EXPR''
 ...
 $elseblock:''EXPR''
 ...
 $elseblock
 ...
 $end

LISPセル''EXPR''が真のとき，$elseblockまたは$endまでのブロックを実行する。
$elseblockに条件を書かないとき，それ以前のすべての条件が真でないときに次のブロックを実行する。

*/
//#WIKICMD	$block	$elseblock	$end	wc_block
void  wc_block (WikiLine* wl, WikiFormat* wiki) {
    ustring  sexp (wl->begin, wl->end);
    MNodePtr  v;
    bool  q;

#ifdef WIKIEVALPROTECT
    if (wiki->protectMode && ! wl->fsuper) {
#ifdef DEBUG
	std::cerr << "(wiki):(protected)\n";
#endif /* DEBUG */
	return;
    }
#endif

#ifdef DEBUG
    std::cerr << "(wiki):" << ustring (wl->begin0, wl->end) << "\n";
#endif /* DEBUG */
    if (sexp.empty ()) {
	q = true;
    } else {
	v = parse_arg (sexp, wiki);
	v = eval_car (v (), wiki);
	q = to_bool (v ());
    }
    if (q) {
	do_linevec (wl->block, wiki);
#ifdef DEBUG
	std::cerr << "(wiki):" << "$end" << "\n";
#endif /* DEBUG */
    } else {
	WikiLine*  w2 = wl->block2;
	if (w2 && w2->fn) {
	    wl->fn (w2, wiki);
	} else {
#ifdef DEBUG
	    std::cerr << "(wiki):" << "$end" << "\n";
#endif /* DEBUG */
	}
    }
}

/* ============================================================ */
static void  case_block (ustring& value, WikiLine* wl, WikiFormat* wiki);

/*DOC:
===$switch===
 $switch:''TEXT1''
 $case:''TEXT2''[:''TEXT3''...]
 ...
 $case
 ...
 $end


*/
//#WIKICMD	$switch	$case	$end	wc_switch
void  wc_switch (WikiLine* wl, WikiFormat* wiki) {
    WikiMotor  motor (wl->begin, wl->end, wiki);
    WikiMotorObjVec  objv;
    WikiMotorObjVecVec  args;
    motor.compile (objv);
    objv.splitCharA (':', args);
    ustring  value;
    if (args.size () == 1) {
	value = args[0]->textOut (wiki);
    } else {
	wiki->errorMsg.append (CharConst ("$switch: wrong number of parameters.\n"));
	return;
    }
    if (wl->block2) {
	case_block (value, wl->block2, wiki);
    }
}

static void  case_block (ustring& value, WikiLine* wl, WikiFormat* wiki) {
    WikiMotor  motor (wl->begin, wl->end, wiki);
    WikiMotorObjVec  objv;
    WikiMotorObjVecVec  args;
    motor.compile (objv);
    objv.splitCharA (':', args);
    if (args.size () == 0) {
	do_linevec (wl->block, wiki);
    } else {
	ustring  v;
	for (int i = 0; i < args.size (); ++ i) {
	    v = args[i]->textOut (wiki);
	    if (value == v) {
		do_linevec (wl->block, wiki);
		return;
	    }
	}
	if (wl->block2) {
	    case_block (value, wl->block2, wiki);
	}
    }
}

/* ============================================================ */
#if 0
////#WIKICMD	$setvar	wc_setvar
void  wc_setvar (WikiLine* wl, WikiFormat* wiki) {
    MotorTexp  ml (wiki->mlenv);
    ustring  sexp (wl->begin, wl->end);
    MNode*  arg;
    ustring  var;
    MNodePtr  v;

#if 0
    if (wiki->protectMode && ! wl->fsuper) {
#ifdef DEBUG
	std::cerr << "(wiki):(protected)\n";
#endif /* DEBUG */
	return;
    }
#endif

#ifdef DEBUG
    std::cerr << "(wiki):" << ustring (wl->begin0, wl->end) << "\n";
#endif /* DEBUG */
    if (! sexp.empty ()) {
	ml.scan (sexp);
	if (ml.top.isCons ()) {
	    arg = ml.top.cdr ();
	    if (arg && arg->isCons ()) {
		var = to_string (arg->car ());
	    }
	    nextNode (arg);
	    if (! arg) {
		wiki->errorMsg.append (CharConst ("$setvar: wrong number of parameters.\n"));
		return;
	    }
	    try {
		v = eval (arg->car (), wiki->mlenv);
	    } catch (ustring& msg) {
		if (wiki->mlenv->currentCell ()) {
		    wiki->errorMsg.append (wiki->mlenv->currentCell ()->dump_string_short ()).append (CharConst (": "));
		}
		wiki->errorMsg.append (msg).append (uLF);
		return;
	    }
	}
	if (var.size () > 0) {
	    if (checkAry (var, var)) {
		wiki->mlenv->setAry (var, v ());
	    } else {
		wiki->mlenv->setVar (var, v ());
	    }
	}
    }
}
#endif

/* ============================================================ */
/*DOC:
===$eval===
 $eval:''EXPR''

LISPセル''EXPR''を実行する。

*/
//#WIKICMD	$eval	wc_eval
void  wc_eval (WikiLine* wl, WikiFormat* wiki) {
    ustring  sexp (wl->begin, wl->end);
    MNodePtr  v;

#ifdef WIKIEVALPROTECT
    if (wiki->protectMode && ! wl->fsuper) {
#ifdef DEBUG
	std::cerr << "(wiki):(protected)\n";
#endif /* DEBUG */
	return;
    }
#endif

#ifdef DEBUG
    std::cerr << "(wiki):" << ustring (wl->begin0, wl->end) << "\n";
#endif /* DEBUG */
    if (! sexp.empty ()) {
	v = parse_arg (sexp, wiki);
	v = eval_car (v (), wiki);
    }
}

/* ============================================================ */
/*DOC:
===$evalblock===
 $evalblock
 ...
 $end

ブロック内部に記述されたLISPファンクションを実行する。
実行できる関数は制限されている。

*/
//#WIKICMD	$evalblock	$end	wc_evalblock
void  wc_evalblock (WikiLine* wl, WikiFormat* wiki) {
//    MotorTexp  ml (wiki->mlenv);

#ifdef WIKIEVALPROTECT
    if (wiki->protectMode && ! wl->fsuper) {
#ifdef DEBUG
	std::cerr << "(wiki):(protected)\n";
#endif /* DEBUG */
	return;
    }
#endif

    if (wl->block && wl->block->size () > 0) {
	ustring  sexp (wl->end + 1, wl->block->back ().end + 1);
#ifdef DEBUG
	std::cerr << "(wiki): $evalblock\n";
#endif /* DEBUG */
	if (! sexp.empty ()) {
	    MNode  top;
	    top.set_cdr (parse_arg (sexp, wiki));
	    try {
		progn_ex (&top, wiki->mlenv);
	    } catch (ustring& msg) {
		if (wiki->mlenv->currentCell ()) {
		    wiki->errorMsg.append (wiki->mlenv->currentCell ()->dump_string_short ()).append (CharConst (": "));
		}
		wiki->errorMsg.append (msg).append (uLF);
	    }
	}
    }
}

/* ============================================================ */
static bool  matchSkipEqs (uiterator& b, uiterator e) {
    if (b < e && *b == '=') {
	do {
	    ++ b;
	} while (b < e && *b == '=');
	return true;
    }
    return false;
}

/*DOC:
===$insert===
 $insert:VARIABLE
 $insert:VARIABLE:==
 $insert:VARIABLE:superuser
 $insert:VARIABLE:superuser:==

変数''VARIABLE''に格納されたWikiテキストを挿入する。
オプションの「=」を追加すると，「=」の数だけ挿入されるWikiテキストのタイトルの深さが下げられる。
superuserオプションをつけると，管理者機能が有効になる。

*/
//#WIKICMD	$insert	wc_insert
void  wc_insert (WikiLine* wl, WikiFormat* wiki) {
    WikiMotor  motor (wl->begin, wl->end, wiki);
    WikiMotorObjVec  objv;
    WikiMotorObjVecVec  argsv;
    std::vector<ustring>  args;
    int  hn;
    bool  super = false;
    bool  protect;
    int  i;
    
#ifdef DEBUG
    std::cerr << "(wiki):" << ustring (wl->begin0, wl->end) << "\n";
#endif /* DEBUG */
    motor.compile (objv);
    objv.splitCharA (':', argsv);
    for (i = 0; i < argsv.size (); i ++)
	args.push_back (argsv[i]->textOut (wiki));

    protect = wiki->protectMode && ! wl->fsuper;
    hn = 0;
    for (i = 1; i < args.size (); i ++) {
	if (! protect && match (args[i], CharConst ("superuser"))) {
	    super = true;
	} else {
	    uiterator  b = args[i].begin ();
	    uiterator  e = args[i].end ();
	    uiterator  p = b;
	    if (matchSkipEqs (b, e)) {
		hn = b - p;
		if (hn < 0)
		    hn = 0;
		if (hn > 5)
		    hn = 5;
	    } else {
		// bad parameter
	    }
	}
    }
    if (args.size () >= 1) {
	AutoInclCount  autoIncl (wiki->mlenv);
	ustring  text = wiki->getVar_string (args[0]);
	WikiLine::linevec  bl;
	wiki->headbase += hn;
	wiki->pass1 (text, &bl, super);
	autoIncl.inc ();
	do_linevec (&bl, wiki);
	wiki->headbase -= hn;
    }
}

/* ============================================================ */
/*DOC:
===$data===
 $data:VARIABLE
 ''TEXT...''
 $end

*/
static void  wldump (ustring& data, WikiLine::linevec* vwl);
static void  wldump (ustring& data, WikiLine* wl) {
    data.append (wl->begin0, wl->end).append (uLF);
    if (wl->block) {
	wldump (data, wl->block);
    }
    if (wl->block2) {
	wldump (data, wl->block2);
    }
}

static void  wldump (ustring& data, WikiLine::linevec* vwl) {
    int  i, n;

    n = vwl->size ();
    for (i = 0; i < n; i ++) {
	wldump (data, &(*vwl)[i]);
    }
}

//#WIKICMD	$data	$end	wc_data
void  wc_data (WikiLine* wl, WikiFormat* wiki) {
    WikiMotor  motor (wl->begin, wl->end, wiki);
    ustring  name;
    ustring  data;
    WikiMotorObjVec  objv;
    WikiMotorObjVecVec  args;

    if (wiki->protectMode && ! wl->fsuper) {
#ifdef DEBUG
	std::cerr << "(wiki):(protected)\n";
#endif /* DEBUG */
	return;
    }

    motor.compile (objv);
    objv.splitCharA (':', args);
    if (args.size () > 0) {
	name = clipWhite (args[0]->textOut (wiki));
    } else {
	wiki->errorMsg.append (CharConst ("$data: missing variable name.\n"));
	return;
    }

    if (wl->block) {
	wldump (data, wl->block);
	wiki->mlenv->setVar (name, newMNode_str (new ustring (data)));
    }
}

/* ============================================================ */
/*DOC:
===$local===
 $local:''VARIABLE'',...
 ...
 $end

*/
//#WIKICMD	$local	$end	wc_local
void  wc_local (WikiLine* wl, WikiFormat* wiki) {
    WikiMotor  motor (wl->begin, wl->end, wiki);
    WikiMotorObjVec  objv;
    WikiMotorObjVecVec  args;
    std::vector<ustring>  vars;
    int  i, n;

#ifdef WIKIEVALPROTECT
    if (wiki->protectMode && ! wl->fsuper) {
#ifdef DEBUG
	std::cerr << "(wiki):(protected)\n";
#endif /* DEBUG */
	return;
    }
#endif

#ifdef DEBUG
    std::cerr << "(wiki):" << ustring (wl->begin0, wl->end) << "\n";
#endif /* DEBUG */

    motor.compile (objv);
    objv.splitCharA (':', args);
    if (args.size () == 1) {
	WikiMotorObjVecVec  v;
	args[0]->splitCharA (',', v);
	for (i = 0; i < v.size (); i ++)
	    vars.push_back (clipWhite (v[i]->textOut (wiki)));
    } else {
	wiki->errorMsg.append (CharConst ("$local: wrong number of parameters.\n"));
	return;
    }
    n = vars.size ();
    if (n > 0) {
	AutoLocalVariable  autoLocal (wiki->mlenv);
	for (i = 0; i < n; i ++) {
	    wiki->mlenv->setLocalVar (vars[i], NULL);
	}
	do_linevec (wl->block, wiki);
#ifdef DEBUG
	std::cerr << "(endlocal)\n";
#endif /* DEBUG */
    } else {
	do_linevec (wl->block, wiki);
    }
#ifdef DEBUG
    std::cerr << "(wiki):$end\n";
#endif /* DEBUG */
}

/* ============================================================ */
/*DOC:
===$macro===
 $macro:''NAME'':''VARIABLE'',...
 ...
 $end

 $NAME:PARAM,...
マクロは組み込みコマンドをオーバーライドできない。

*/
//#WIKICMD	$macro	$end	wc_macro
void  wc_macro (WikiLine* wl, WikiFormat* wiki) {
    WikiMotor  motor (wl->begin, wl->end, wiki);
    WikiMotorObjVec  objv;
    WikiMotorObjVecVec  args;
    ustring  name;
    MNodeList  vars;
    int  i, n;

    assert (0); // メモリ管理問題

    if (wiki->protectMode && ! wl->fsuper) {
#ifdef DEBUG
	std::cerr << "(wiki):(protected)\n";
#endif /* DEBUG */
	return;
    }

#ifdef DEBUG
    std::cerr << "(wiki):" << ustring (wl->begin0, wl->end) << "\n";
#endif /* DEBUG */

    motor.compile (objv);
    objv.splitCharA (':', args);
    if (args.size () >= 1) {
	name = clipWhite (args[0]->textOut (wiki));
    } else {
	wiki->errorMsg.append (CharConst ("$macro: wrong number of parameters.\n"));
	return;
    }
    if (args.size () == 2) {
	WikiMotorObjVecVec  v;
	args[1]->splitCharA (',', v);
	for (i = 0; i < v.size (); i ++) {
	    vars.append (newMNode_str (new ustring (clipWhite (v[i]->textOut (wiki)))));
	}
    } else if (args.size () > 2) {
	wiki->errorMsg.append (CharConst ("$macro: wrong number of parameters.\n"));
	return;
    }

    if (wl->block) {
	wiki->env->wikienv->wikiMacro.setVar (name, vars.release (), wl->block);
	wl->block = NULL;
    }

#ifdef DEBUG
//    std::cerr << "(wiki):$end\n";
#endif /* DEBUG */
}

/* ============================================================ */
/*DOC:
===$element===
 $element:''NAME''
 ...
 $end

*/
//#WIKICMD	$element	$end	wc_element	wp_element
void  wc_element (WikiLine* wl, WikiFormat* wiki) {
#ifdef DEBUG
//    std::cerr << "(wiki):$element:" << ustring (wl->begin, wl->end) << "\n";
#endif /* DEBUG */
}

void  wp_element (WikiLine* wl, WikiFormat* wiki) {
    ustring  name (wl->begin, wl->end);

    if (name.size () == 0) {
	wiki->errorMsg.append (CharConst ("$element: wrong number of parameters.\n"));
	return;
    }
    wiki->env->wikienv->wikiElement.setVar (name, wl);
}

/* ============================================================ */
/*DOC:
===$input-text, $input-textarea, $input-int, $input-real, $input-int-or-blank, $input-real-or-blank, $input-ascii, $input-bool===
 $input-text:VARIABLE
 $input-textarea:VARIABLE
 $input-int:VARIABLE
 $input-real:VARIABLE
 $input-int-or-blank:VARIABLE
 $input-real-or-blank:VARIABLE
 $input-ascii:VARIABLE
 $input-bool:VARIABLE

*/
//#WIKICMD	$input-text	wc_input_text
//#WIKICMD	$input-textarea	wc_input_textarea
//#WIKICMD	$input-int	wc_input_int
//#WIKICMD	$input-real	wc_input_real
//#WIKICMD	$input-int-or-blank	wc_input_int_or_blank
//#WIKICMD	$input-real-or-blank	wc_input_real_or_blank
//#WIKICMD	$input-ascii	wc_input_ascii
//#WIKICMD	$input-bool	wc_input_bool
static void  wc_input_sub (WikiLine* wl, WikiFormat* wiki, ustring (*prefn)(const ustring&), MNode* (FormVarOp::*postfn)(const ustring&)) {
    WikiMotor  motor (wl->begin, wl->end, wiki);
    WikiMotorObjVec  objv;
    WikiMotorObjVecVec  args;

#ifdef DEBUG
    std::cerr << "(wiki):" << ustring (wl->begin0, wl->end) << "\n";
#endif /* DEBUG */
    motor.compile (objv);
    objv.splitCharA (':', args);
    ustring  name;
    int  i;
    FormVarOp  opt;

    if (args.size () > 0) {
	name = args[0]->textOut (wiki);
	for (i = 1; i < args.size (); i ++) {
	    if (args[i]->matchHead (CharConst ("maxlen="))) {
		ustring  x, y;
		args[i]->splitChar (wiki, '=', x, y);
		opt.max = to_uint64 (y);
	    } else if (args[i]->match (CharConst ("multi"))) {
		opt.fmulti = true;
	    } else if (args[i]->match (CharConst ("trim"))) {
		opt.ftrim = true;
	    } else if (args[i]->matchHead (CharConst ("filter="))) {
		ustring  x, y;
		args[i]->splitChar (wiki, '=', x, y);
		opt.filter = y;
	    } else {
		wiki->errorMsg.append (args[i]->textOut (wiki) + ustring (CharConst (": bad parameter.\n")));
		return;
	    }
	}
	MNode*  e = opt.input (wiki->mlenv, name, prefn, postfn);
	wiki->mlenv->setVar (name, e);
    }
}

void  wc_input_text (WikiLine* wl, WikiFormat* wiki) {
    wc_input_sub (wl, wiki, omitCtrl, &FormVarOp::stringNode);
}

void  wc_input_textarea (WikiLine* wl, WikiFormat* wiki) {
    wc_input_sub (wl, wiki, omitCtrlX, &FormVarOp::stringNode);
}

void  wc_input_int (WikiLine* wl, WikiFormat* wiki) {
    wc_input_sub (wl, wiki, NULL, &FormVarOp::intNode);
}

void  wc_input_real (WikiLine* wl, WikiFormat* wiki) {
    wc_input_sub (wl, wiki, NULL, &FormVarOp::doubleNode);
}

void  wc_input_int_or_blank (WikiLine* wl, WikiFormat* wiki) {
    wc_input_sub (wl, wiki, NULL, &FormVarOp::intBlankNode);
}

void  wc_input_real_or_blank (WikiLine* wl, WikiFormat* wiki) {
    wc_input_sub (wl, wiki, NULL, &FormVarOp::doubleBlankNode);
}

void  wc_input_ascii (WikiLine* wl, WikiFormat* wiki) {
    wc_input_sub (wl, wiki, omitNonAscii, &FormVarOp::stringNode);
}

void  wc_input_bool (WikiLine* wl, WikiFormat* wiki) {
    wc_input_sub (wl, wiki, NULL, &FormVarOp::boolNode);
}

/* ============================================================ */
/*DOC:
===$pre-mode===
 $pre-mode:''BOOL''

フォーマット済みブロック内で，インラインファンクションの展開を行うモードの設定。
trueで展開を行う。

*/
//#WIKICMD	$pre-mode	wc_premode
//#WIKICMD	$premode	wc_premode
void  wc_premode (WikiLine* wl, WikiFormat* wiki) {
    WikiMotor  motor (wl->begin, wl->end, wiki);
    WikiMotorObjVec  objv;
    WikiMotorObjVecVec  argsv;
    ustring  arg;

#ifdef DEBUG
    std::cerr << "(wiki):" << ustring (wl->begin0, wl->end) << "\n";
#endif /* DEBUG */
    motor.compile (objv);
    objv.splitCharA (':', argsv);
    if (argsv.size () > 0) {
	arg = argsv[0]->textOut (wiki);
    }
//    wiki->motorPre = to_bool (arg);
    MNodePtr  v;
    v = parse_arg (arg, wiki);
    v = eval_car (v (), wiki);
    wiki->motorPre = to_bool (v ());
}


/* ============================================================ */
static void  op_call_defun_wikicmd (ustring& name, MNode* wf, WikiMotorObjVec& objv2, WikiLine* wl, WikiFormat* wiki, int version) {
    WikiMotorObjVecVec  argsv;
    MNodePtr  vargs;

#ifdef DEBUG2
//    std::cerr << "call_defun:" << name << ":" << objv2.dump ().c_str () << "\n";
#endif /* DEBUG */
    objv2.splitCharA (':', argsv);
    vargs = wiki->buildArgs (wf->lambdaModeBit (MNode::MODE_WIKINODEC), argsv.begin (), argsv.end ());
    {
	MNodePtr  node;
	MNodePtr  bcell;

	bcell = wiki->env->mlenv->currentCell;
	wiki->env->mlenv->currentCell = NULL;
	if (version == 2) {
	    ustring  data;
	    if (wl->block)
		wldump (data, wl->block);
	    vargs = newMNode_cons (newMNode_str (new ustring (data)), vargs ());
	}
	try {
	    node = execDefun (wiki->env->mlenv, wf->lambdaParams (), wf->lambdaBody (), vargs (), name);
	} catch (ustring& msg) {
	    wiki->logLispFunctionError (msg, ustring (wl->begin, wl->end));
	}
	wiki->env->mlenv->currentCell = bcell;
	if (isStr (node ())) {
	    ustring  text = to_string (node ());
	    if (wf->lambdaModeBit (MNode::MODE_WIKIRAW)) {
		wiki->pushBlockRaw (text.begin (), text.end ());
	    } else if (wf->lambdaModeBit (MNode::MODE_WIKIOUT)) {
		WikiLine::linevec  bl;
		bool  fnormal = false;
		if (matchHead (text, CharConst ("$normal-mode:")))
		    fnormal = true;
		wiki->pass1 (text, &bl, wl->fsuper & !fnormal);
		do_linevec (&bl, wiki);
	    }
	}
    }
}

static void  call_defun_wikicmd (ustring& name, MNode* wf, WikiMotorObjVec& objv2, WikiLine* wl, WikiFormat* wiki) {
    op_call_defun_wikicmd (name, wf, objv2, wl, wiki, 1);
}

static void  call_defun_wikicmd2 (ustring& name, MNode* wf, WikiMotorObjVec& objv2, WikiLine* wl, WikiFormat* wiki) {
    op_call_defun_wikicmd (name, wf, objv2, wl, wiki, 2);
}

static void  call_defun_macro (ustring& name, WikiMacro* mf, WikiMotorObjVec& objv2, WikiLine* wl, WikiFormat* wiki) {
    WikiMotorObjVecVec  argsv;

    objv2.splitCharA (':', argsv);
    {
	AutoLocalVariable  autoLocal (wiki->mlenv);
	MNode*  p = mf->vars ();
	MNode*  t;
	MNode*  v;
	int  i, n;
	n = argsv.size ();
	i = 0;
	while (p && (t = p->car ())) {
	    if (i < n) {
		v = newMNode_str (new ustring (argsv[i]->textOut (wiki)));
		i ++;
	    } else {
		v = NULL;
	    }
	    wiki->mlenv->setLocalVar (to_string (t), v);
	    p = p->cdr ();
	}
	do_linevec (mf->wl, wiki);
    }
}

void  wc_call_defun (WikiLine* wl, WikiFormat* wiki) {
    WikiMotor  motor (wl->begin + 1, wl->end, wiki);
    WikiMotorObjVec  objv;
    WikiMotorObjVec  objv2;
    ustring  name;
    MNode*  wf;
    WikiMacro*  mf;

#ifdef DEBUG
    std::cerr << "(wiki):" << ustring (wl->begin0, wl->end) << "\n";
#endif /* DEBUG */
    motor.compile (objv);
#ifdef DEBUG2
//    std::cerr << "call_defun:" << objv.dump () << "\n";
//    std::cerr << ustring (wl->begin, wl->end).c_str () << "\n";
#endif /* DEBUG */
    objv.splitChar_keyword (':', name, objv2);
#ifdef DEBUG2
//    std::cerr << "command:" << name << "\n";
//    std::cerr << "objv2:" << objv2.dump ().c_str () << "\n";
#endif /* DEBUG */
    if ((wf = wiki->env->wikienv->wikiCmd.getVar (name)) && (wl->fsuper || ! isLambda (wf) || wf->lambdaModeBit (MNode::MODE_WIKIPUBLIC))) { // Wiki Command
	call_defun_wikicmd (name, wf, objv2, wl, wiki);
    } else if ((wf = wiki->env->wikienv->wikiCmd2.getVar (name)) && (wl->fsuper || ! isLambda (wf) || wf->lambdaModeBit (MNode::MODE_WIKIPUBLIC))) { // Wiki Command2
	call_defun_wikicmd2 (name, wf, objv2, wl, wiki);
    } else if ((mf = wiki->env->wikienv->wikiMacro.getVar (name))) { // Wiki Macro. macroは誰でも実行できる。
//	std::cerr << "macro:" << name << "\n";
	call_defun_macro (name, mf, objv2, wl, wiki);
    } else {			// 処理なし
	WikiLine::linevec  bl;
	WikiLine*  wl2;
	ustring  u (CharConst ("^"));

	u.append (wl->begin, wl->end);
	wl2 = new WikiLine (u.begin (), u.end (), wl->fsuper);
	bl.push_back (wl2);
	do_linevec (&bl, wiki);
    }
}

void  wikiOutput (const ustring& text, bool fsuper, WikiFormat* wiki) {
    WikiLine::linevec  bl;
    wiki->pass1 (text, &bl, fsuper);
    do_linevec (&bl, wiki);
}

/*DOC:
$pre-mode:true
*/
/* ============================================================ */
typedef  enum {V_VAR, V_VEC, V_TAB}  each_var_type;
static bool  each_var (const char* cmd, size_t cmd_len, WikiFormat* wiki, WikiMotorObjVec* arg, each_var_type& fvec, std::vector<ustring>& lv, std::vector<ustring>& lk) {
    fvec = V_VAR;
    if (arg->size () == 0) {
	wiki->errorMsg.append (cmd, cmd_len).append (CharConst (": bad parameter.\n"));
	return false;
    }
    WikiMotorObj*  obj = (*arg)[0].get ();
    if (obj->type != WikiMotorObj::wiki_text) {
	wiki->errorMsg.append (cmd, cmd_len).append (CharConst (": bad parameter.\n"));
	return false;
    }
    WikiMotorObjText*  rvar = WikiMotorObjText_type (obj);
    if (rvar->text.length () > 0) {
	switch (rvar->text[0]) {
	case '*':
	    fvec = V_VEC;
	    rvar->text = ustring (rvar->text.begin () + 1, rvar->text.end ());
	    break;
	case '%':
	    fvec = V_TAB;
	    rvar->text = ustring (rvar->text.begin () + 1, rvar->text.end ());
	    break;
	}
    }
    ustring  s;
    switch (fvec) {
    case V_VEC: {
	WikiMotorObjVecVec  var;
	arg->splitCharA (',', var);
	for (int i = 0; i < var.size (); i ++) {
	    s = clipWhite (var[i]->textOut (wiki));
	    lv.push_back (s);
	}
	break;
    }
    case V_TAB: {
	WikiMotorObjVecVec  var;
	arg->splitCharA (',', var);
	for (int i = 0; i < var.size (); i ++) {
	    WikiMotorObjVec  key;
	    WikiMotorObjVec  val;
	    if (var[i]->splitChar ('=', key, val)) {
		lk.push_back (clipWhite (key.textOut (wiki)));
		lv.push_back (clipWhite (val.textOut (wiki)));
	    } else {
		s = clipWhite (var[i]->textOut (wiki));
		lk.push_back (s);
		lv.push_back (s);
	    }
	}
	break;
    }
    default:
	lv.push_back (clipWhite (arg->textOut (wiki)));
    }
    return true;
}

static void  each_enum (WikiFormat* wiki, WikiMotorObjVec* arg, MNodePtr& lvv) {
    WikiMotorObj*  obj = (*arg)[0].get ();
    switch (obj->type) {
    case WikiMotorObj::wiki_var:
    case WikiMotorObj::wiki_table:
    case WikiMotorObj::wiki_func1:
    case WikiMotorObj::wiki_funcM:
    case WikiMotorObj::wiki_funcM2:
//    case WikiMotorObj::wiki_funcLink:
	lvv = obj->toMNode (wiki);
	break;
    default:
	lvv = wiki->getVar (clipWhite (arg->textOut (wiki)));
    }
}

/*DOC:
===$list-each===
 $list-each:[*]''VAR1'',''VAR2'',...:''VAR_VECTOR''[:''INDEX_VAR''][:''EXPR'']
 ...
 [$else]
 ...
 $end
// $list-each:%''VAR1'',''KEY2''=''VAR2'',...:''VAR_VECTOR''[:''INDEX_VAR'']

リスト''VAR_LIST''の各要素を変数''VAR1''に代入しながら，$endまでのブロックを繰り返し実行する。
''INDEX_VAR''を指定すると，インデックス番号（1始まり）を変数''INDEX_VAR''に代入して，ブロックを実行する。

*/
//#WIKICMD	$list-each	$else	$end	wc_list_each
void  wc_list_each (WikiLine* wl, WikiFormat* wiki) {
    WikiMotor  motor (wl->begin, wl->end, wiki);
    WikiMotorObjVec  objv;
    WikiMotorObjVecVec  args;
    std::vector<ustring>  lv;	// ***XXX
    std::vector<ustring>  lk;
    each_var_type  fvec;
    MNodePtr  lvv;
    ustring  iv;
    int  i, iu, it;
    MNodePtr  h;

#ifdef WIKIREPEATPROTECT
    if (wiki->protectMode && ! wl->fsuper) {
#ifdef DEBUG
	std::cerr << "(wiki):(protected)\n";
#endif /* DEBUG */
	return;
    }
#endif

#ifdef DEBUG
    std::cerr << "(wiki):" << ustring (wl->begin0, wl->end) << "\n";
#endif /* DEBUG */
    motor.compile (objv);
    objv.splitCharA (':', args);
    if (args.size () == 2 || args.size () == 3) {
	if (! each_var (CharConst ("$list-each"), wiki, args[0].get (), fvec, lv, lk))
	    return;
	each_enum (wiki, args[1].get (), lvv);
	if (isCons (lvv ())) {
	} else if (isNil (lvv ())) {
	} else {
	    wiki->errorMsg.append (CharConst ("$list-each: bad value type.\n"));
	    return;
	}
	if (args.size () == 3) {
	    iv = clipWhite (args[2]->textOut (wiki));
	}
    } else {
	wiki->errorMsg.append (CharConst ("$list-each: wrong number of parameters.\n"));
	return;
    }
    {
	iu = lv.size ();
	AutoLocalVariable  autoLocal (wiki->mlenv);
	for (i = 0; i < iu; i ++) {
	    wiki->mlenv->setLocalVar (lv[i], NULL);
	}
	if (iv.size () > 0)
	    wiki->mlenv->setLocalVar (iv, NULL);
	MNode*  e = lvv ();
	i = 0;
	if (! isNil (e)) {
	    while (isCons (e)) {
//		setvar_sel (vars (), e->car (), wiki->mlenv);
		MNode*  v = e->car ();
		switch (fvec) {
		case V_VEC:
		    if (isCons (v)) {
			for (it = 0; it < iu; ++ it) {
			    if (isCons (v)) {
				wiki->mlenv->setVar (lv[it], v->car ());
			    } else {
				wiki->mlenv->setVar (lv[it], NULL);
			    }
			    nextNode (v);
			}
		    } else if (isVector (v)) {
			for (it = 0; it < iu; ++ it) {
			    wiki->mlenv->setVar (lv[it], v->vectorGet (it));
			}
		    } else {
			wiki->errorMsg.append (CharConst ("$list-each: bad value type.\n"));
			return;
		    }
		    break;
		case V_TAB:
		    //
		    // ***V_TAB
		    break;
		default:;
		    wiki->mlenv->setVar (lv[0], v);
		}
		if (iv.size () > 0) {
		    h = newMNode_num (i + 1);
		    wiki->mlenv->setVar (iv, h ());
		}
		do_linevec (wl->block, wiki);
		nextNode (e);
		++ i;
	    }
	} else {
	    if (wl->block2 && wl->block2->block)
		do_linevec (wl->block2->block, wiki);
	}
    }
}

/* ============================================================ */
/*DOC:
===$vector-each===
 $vector-each:[*]''VAR1'',''VAR2'',...:''VAR_VECTOR''[:''INDEX_VAR'']
 ...
 [$else]
 ...
 $end
 $vector-each:%''VAR1'',''KEY2''=''VAR2'',...:''VAR_VECTOR''[:''INDEX_VAR'']

ベクタ''VAR_VECTOR''の各要素をそれぞれ変数''VAR1'', ''VAR2'', ...に代入しながら，$endまでのブロックを繰り返し実行する。
ブロックの中で変数''VAR1'', ...に代入した値は，ベクタに書き戻されない。
''INDEX_VAR''を指定すると，インデックス番号（1始まり）を変数''INDEX_VAR''に代入して，ブロックを実行する。

*/
//#WIKICMD	$vector-each	$else	$end	wc_vector_each
void  wc_vector_each (WikiLine* wl, WikiFormat* wiki) {
    WikiMotor  motor (wl->begin, wl->end, wiki);
    WikiMotorObjVec  objv;
    WikiMotorObjVecVec  args;
    std::vector<ustring>  lv;
    std::vector<ustring>  lk;
    each_var_type  fvec;
    MNodePtr  lvv;
    ustring  iv;
    int  i, n, iu, it;
    MNodePtr  h;

#ifdef WIKIREPEATPROTECT
    if (wiki->protectMode && ! wl->fsuper) {
#ifdef DEBUG
	std::cerr << "(wiki):(protected)\n";
#endif /* DEBUG */
	return;
    }
#endif

#ifdef DEBUG
    std::cerr << "(wiki):" << ustring (wl->begin0, wl->end) << "\n";
#endif /* DEBUG */
    motor.compile (objv);
    objv.splitCharA (':', args);
    if (args.size () == 2 || args.size () == 3) {
	if (! each_var (CharConst ("$vector-each"), wiki, args[0].get (), fvec, lv, lk))
	    return;
	each_enum (wiki, args[1].get (), lvv);
	if (isVector (lvv ())) {
	} else if (isNil (lvv ())) {
	} else {
	    wiki->errorMsg.append (CharConst ("$vector-each: bad value type.\n"));
	    return;
	}
	if (args.size () == 3) {
	    iv = clipWhite (args[2]->textOut (wiki));
	}
    } else {
	wiki->errorMsg.append (CharConst ("$vector-each: wrong number of parameters.\n"));
	return;
    }
    iu = lv.size ();
    if (isVector (lvv ())) {
	AutoLocalVariable  autoLocal (wiki->mlenv);
	for (i = 0; i < iu; i ++) {
	    wiki->mlenv->setLocalVar (lv[i], NULL);
	}
	if (iv.size () > 0)
	    wiki->mlenv->setLocalVar (iv, NULL);
	n = lvv ()->vectorSize ();
	if (n > 0) {
	    for (i = 0; i < n; ++ i) {
		MNode*  a = lvv ()->vectorGet (i);
		switch (fvec) {
		case V_VEC:
		    if (isVector (a)) {
			for (it = 0; it < iu; ++ it)
			    wiki->mlenv->setVar (lv[it], a->vectorGet (it));
		    } else if (isNil (a)) {
			for (it = 0; it < iu; ++ it)
			    wiki->mlenv->setVar (lv[it], NULL);
		    } else {
			wiki->errorMsg.append (CharConst ("$vector-each: bad value type.\n"));
			return;
		    }
		    break;
		case V_TAB:
		    if (isTable (a)) {
			for (it = 0; it < iu; ++ it)
			    wiki->mlenv->setVar (lv[it], a->tableGet (lk[it]));
		    } else if (isNil (a)) {
			for (it = 0; it < iu; ++ it)
			    wiki->mlenv->setVar (lv[it], NULL);
		    } else {
			wiki->errorMsg.append (CharConst ("$vector-each: bad value type.\n"));
			return;
		    }
		    break;
		default:
		    wiki->mlenv->setVar (lv[0], a);
		}
		if (iv.size () > 0) {
		    h = newMNode_num (i + 1);
		    wiki->mlenv->setVar (iv, h ());
		}
		do_linevec (wl->block, wiki);
	    }
	} else {
	    if (wl->block2 && wl->block2->block)
		do_linevec (wl->block2->block, wiki);
	}
    } else {
	if (wl->block2 && wl->block2->block)
	    do_linevec (wl->block2->block, wiki);
    }
}

/*DOC:
===$vector-iterate===
 $vector-iterate:''VAR1'',''VAR2'',...[:''INDEX_VAR'']
 ...
 [$else]
 ...
 $end

ベクタ''VAR1'', ''VAR2'', ...の各要素をそれぞれ変数''VAR1'', ''VAR2'', ...に代入しながら，$endまでのブロックを繰り返し実行する。
ブロックの中で変数''VAR1'', ...に代入した値は，配列に書き戻されない。
ベクタの長さが異なる場合は，ベクタ''VAR1''の長さに揃えられる。
''INDEX_VAR''を指定すると，配列のインデックス番号を変数''INDEX_VAR''に代入して，ブロックを実行する。1始まり。

*/
//#WIKICMD	$vector-iterate	$else	$end	wc_dovector
void  wc_dovector (WikiLine* wl, WikiFormat* wiki) {
    WikiMotor  motor (wl->begin, wl->end, wiki);
    WikiMotorObjVec  objv;
    WikiMotorObjVecVec  args;
    std::vector<ustring>  lv;
    boost::ptr_vector<MNodePtr>  lvv;
    ustring  index_var;
    int  i, n, iu, it;
    MNodePtr  h;
    ustring  s;
    MNode*  t;

#ifdef WIKIREPEATPROTECT
    if (wiki->protectMode && ! wl->fsuper) {
#ifdef DEBUG
	std::cerr << "(wiki):(protected)\n";
#endif /* DEBUG */
	return;
    }
#endif

#ifdef DEBUG
    std::cerr << "(wiki):" << ustring (wl->begin0, wl->end) << "\n";
#endif /* DEBUG */
    motor.compile (objv);
    objv.splitCharA (':', args);
    if (args.size () == 1 || args.size () == 2) {
	WikiMotorObjVecVec  v;
	args[0]->splitCharA (',', v);
	for (int i = 0; i < v.size (); i ++) {
	    s = clipWhite (v[i]->textOut (wiki));
	    t = wiki->getVar (s);
	    if (isVector (t)) {
		lv.push_back (s);
		lvv.push_back (new MNodePtr);
		lvv.back () = t;
	    } else if (isNil (t)) {
	    } else {
		wiki->errorMsg.append (CharConst ("$vector-iterate: bad value type.\n"));
	    }
	}
	if (args.size () == 2) {
	    index_var = clipWhite (args[1]->textOut (wiki));
	}
    } else {
	wiki->errorMsg.append (CharConst ("$vector-iterate: wrong number of parameters.\n"));
    }
    iu = lv.size ();
    if (iu > 0) {
	n = lvv[0] ()->vectorSize ();
	if (n > 0) {
	    for (i = 0; i < n; ++ i) {
		for (it = 0; it < iu; ++ it) {
		    wiki->mlenv->setVar (lv[it], lvv[it] ()->vectorGet (i));
		}
		if (index_var.size () > 0) {
		    h = newMNode_num (i + 1);
		    wiki->mlenv->setVar (index_var, h ());
		}
		do_linevec (wl->block, wiki);
	    }
	    for (it = 0; it < iu; ++ it) {			// 書き戻す
		wiki->mlenv->setVar (lv[it], lvv[it] ());
	    }
	} else {
	    if (wl->block2 && wl->block2->block)
		do_linevec (wl->block2->block, wiki);
	}
    } else {
	if (wl->block2 && wl->block2->block)
	    do_linevec (wl->block2->block, wiki);
    }
}

/* ============================================================ */
static void  vector_sort (std::vector<ustring>&  vec) {
    const bool  fdesc = false;
    ustring  a;
    int  s, i, j, k;
    int  n = vec.size ();
    for (i = 1; i < n; i ++) {
	j = i;
	while (j > 0) {
	    k = (j - 1) / 2;
	    if (fdesc ^ vec[k] >= vec[j])
		break;
//	    swap (v[k], v[j]);
	    a = vec[j]; vec[j] = vec[k]; vec[k] = a;
	    j = k;
	}
    }
    for (; n > 0; n --) {
//	swap (v[0], v[n - 1]);
	a = vec[n - 1]; vec[n - 1] = vec[0]; vec[0] = a;
	for (i = 1; i < n - 1; i ++) {
	    j = i;
	    while (j > 0) {
		k = (j - 1) / 2;
		if (fdesc ^ vec[k] >= vec[j])
		    break;
//		swap (v[k], v[j]);
		a = vec[j]; vec[j] = vec[k]; vec[k] = a;
		j = k;
	    }
	}
    }
}

/*DOC:
===$table-each===
 $table-each:''VARKEY'':''VARVAL'':''VAR_TABLE''[:''INDEX_VAR'']
 ...
 [$else]
 ...
 $end
 $table-each:''VARKEY'':*''VARVAL1'',''VARVAL2'',...:''VAR_TABLE''[:''INDEX_VAR'']
 $table-each:''VARKEY'':%''VARVAL1'',''KEY2''=''VARVAL2'',...:''VAR_TABLE''[:''INDEX_VAR'']

テーブル''VAR_TABLE''の各要素をそれぞれ変数''VAR1'', ''VAR2'', ...に代入しながら，$endまでのブロックを繰り返し実行する。
ブロックの中で変数''VAR1'', ...に代入した値は，テーブルに書き戻されない。
''INDEX_VAR''を指定すると，インデックス番号（1始まり）を変数''INDEX_VAR''に代入して，ブロックを実行する。

*/
//#WIKICMD	$table-each	$else	$end	wc_table_each
void  wc_table_each (WikiLine* wl, WikiFormat* wiki) {
    WikiMotor  motor (wl->begin, wl->end, wiki);
    WikiMotorObjVec  objv;
    WikiMotorObjVecVec  args;
    each_var_type  fvec;
    ustring  s;
    ustring  varkey;
    std::vector<ustring>  varvals;
    std::vector<ustring>  keys;
    ustring  iv;
    MNodePtr  lvv;

#ifdef WIKIREPEATPROTECT
    if (wiki->protectMode && ! wl->fsuper) {
#ifdef DEBUG
	std::cerr << "(wiki):(protected)\n";
#endif /* DEBUG */
	return;
    }
#endif

#ifdef DEBUG
    std::cerr << "(wiki):" << ustring (wl->begin0, wl->end) << "\n";
#endif /* DEBUG */
    motor.compile (objv);
    objv.splitCharA (':', args);
    if (args.size () == 3 || args.size () == 4) {
	// key
	varkey = clipWhite (args[0]->textOut (wiki));
	// val
	if (! each_var (CharConst ("$vector-each"), wiki, args[1].get (), fvec, varvals, keys))
	    return;
	// table variable
	each_enum (wiki, args[2].get (), lvv);
	if (isTable (lvv ())) {
	} else if (isNil (lvv ())) {
	} else {
	    wiki->errorMsg.append (CharConst ("$table-each: bad value type.\n"));
	    return;
	}
	if (args.size () == 4) {
	    iv = clipWhite (args[3]->textOut (wiki));
	}
    } else {
	wiki->errorMsg.append (CharConst ("$table-each: wrong number of parameters.\n"));
	return;
    }

    std::vector<ustring>  keyvec;
    if (isTable (lvv ())) {
	MotorVar::iterator  b;
	MotorVar::iterator  e;
	b = lvv ()->value_table ()->begin ();
	e = lvv ()->value_table ()->end ();
	while (b != e) {
	    keyvec.push_back ((*b).first);
	    vector_sort (keyvec);
	    ++ b;
	}
    }
    std::vector<ustring>::iterator  b = keyvec.begin ();
    std::vector<ustring>::iterator  e = keyvec.end ();

    if (isTable (lvv ()) && b != e) {
	AutoLocalVariable  autoLocal (wiki->mlenv);
	wiki->mlenv->setLocalVar (varkey, NULL);
	for (int i = 0; i < varvals.size (); ++ i) {
	    wiki->mlenv->setLocalVar (varvals[i], NULL);
	}
	if (iv.size () > 0)
	    wiki->mlenv->setLocalVar (iv, NULL);
	int  i = 0;
	while (b != e) {
	    wiki->mlenv->setVar (varkey, newMNode_str (new ustring (*b)));
	    MNode*  a = lvv ()->tableGet (*b);
	    switch (fvec) {
	    case V_VEC:
		if (isVector (a)) {
		    for (int it = 0; it < varvals.size(); it ++)
			wiki->mlenv->setVar (varvals[it], a->vectorGet (it));
		} else if (isNil (a)) {
		    for (int it = 0; it < varvals.size(); it ++)
			wiki->mlenv->setVar (varvals[it], NULL);
		} else {
		    wiki->errorMsg.append (CharConst ("$table-each: bad value type.\n"));
		    return;
		}
		break;
	    case V_TAB:
		if (isTable (a)) {
		    for (int it = 0; it < varvals.size(); it ++)
			wiki->mlenv->setVar (varvals[it], a->tableGet (keys[it]));
		} else if (isNil (a)) {
		    for (int it = 0; it < varvals.size(); it ++)
			wiki->mlenv->setVar (varvals[it], NULL);
		} else {
		    wiki->errorMsg.append (CharConst ("$table-each: bad value type.\n"));
		    return;
		}
		break;
	    default:;
		wiki->mlenv->setVar (varvals[0], a);
	    }
	    if (iv.size () > 0) {
		wiki->mlenv->setVar (iv, newMNode_num (i + 1));
	    }
	    do_linevec (wl->block, wiki);
	    ++ b;
	    ++ i;
	}
    } else {
	if (wl->block2 && wl->block2->block)
	    do_linevec (wl->block2->block, wiki);
    }
}

/* ============================================================ */
