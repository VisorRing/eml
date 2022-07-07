#include "ml-addon.h"
#include "ml.h"
#include "mlenv.h"
#include "config.h"
#include "motorconst.h"
#include "motorenv.h"
#include "motoroutput.h"
#include "utf8.h"
#include "util_const.h"
#include "util_check.h"
#include "util_file.h"
#include "util_string.h"
#include "util_splitter.h"
#include "util_proc.h"
#include "ustring.h"
#include "expr.h"
#include <vector>
#include <exception>
#include <arpa/inet.h>

/*DOC:
==add-on command==
外部プログラムを呼び出す。

呼び出されるプログラムのargv[1], argv[2]に，それぞれ一時ディレクトリ，保存ディレクトリのパスが引渡される。
add-onファンクションで与える引数は，プログラムの標準入力に，改行文字区切りで与えられる。
ファンクションの引数に改行文字が含まれている場合、空白文字に置き換えられる。

*/

class  AddonParams {
public:
    ustring  cmd;
    MNode*  rest;
    bool  fRawInput;
    ustring  par;
    ProcRW  proc;

    AddonParams (const ustring& _cmd, MNode* _rest, bool _rawInput) {
	cmd = _cmd;
	rest = _rest;
	fRawInput = _rawInput;
	setPars ();
	if (cmd.size () == 0)
	    throw (uErrorCmdNameEmpty);
	if (! matchFilename (cmd))
	    throw (cmd + uErrorBadCmd);
    };
    virtual  ~AddonParams () {};
    virtual void  setPars () {
	if (fRawInput) {
	    while (rest) {
		if (par.length () > 0)
		    par.append (uLF);
		par.append (to_string (rest->car ()));
		nextNode (rest);
	    }
	} else {
	    ustring  u;
	    while (rest) {
		u = to_string (rest->car ());
		{
		    ustring  ans;
		    SplitterNL  sp (u);
		    while (sp.nextSep ()) {
			ans.append (sp.pre ()).append (uSPC);
		    }
		    ans.append (sp.pre ());
		    u = ans;
		}
		if (par.length () > 0)
		    par.append (uLF);
		par.append (to_string (rest->car ()));
		nextNode (rest);
	    }
	}
    };
    virtual void  procOpen (MlEnv* mlenv) {
	char*  argv[4];
	ustring  cmd0 (CharConst (cDataTop kBin));
	cmd0.append (cmd);
	argv[0] = (char*)cmd0.c_str ();
	argv[1] = (char*)mlenv->env->storedir.c_str ();
	argv[2] = (char*)mlenv->env->storagedir.c_str ();
	argv[3] = NULL;
	if (isExecutableFile (cmd0)) {
	    procReqC (argv, mlenv);
	    return;
	} else {
	    cmd0 = ustring (CharConst (cDataTop kEBin));
	    cmd0.append (cmd);
	    argv[0] = (char*)cmd0.c_str ();
	    if (isExecutableFile (cmd0)) {
		procReqP (argv, mlenv);
		return;
	    }
	}
	throw (cmd + ustring (CharConst (": command not found")));
    };
    virtual void  procReqC (char** argv, MlEnv* mlenv) {
	proc.open (argv, &mlenv->env->storedir);
	proc.write (par);
	proc.closeWriter ();
    };
    virtual void  procReqP (char** argv, MlEnv* mlenv) {
	proc.open (argv, &mlenv->env->storedir);
	uint8_t  b[sizeof (uint32_t)];
	uint32_t  len = htonl (par.length ());
	memcpy (b, (uint8_t*)&len, sizeof (uint32_t));
	proc.write ((char*)b, sizeof (uint32_t));
	proc.write (par);
	proc.closeWriter ();
    };
};

/*DOC:
===add-on===
 (add-on COMMAND [#raw-input | :raw-input BOOL] [ARG1 ARG2...]) -> STRING

===add-on-tab===
 (add-on-tab COMMAND [#raw-input | :raw-input BOOL] [ARG1 ARG2...]) -> STRING-LIST

*/

//#XAFUNC	add-on	ml_addon
MNode*  ml_addon (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    kwParam  kwList[] = {{CharConst ("raw-input"), EV_LIST},
			 {NULL, 0, EV_END}};
    MNodePtr  kwParams[1];
    MNodePtr  rest;
    evalParams (fev, mlenv, cell, posList, posParams, kwList, kwParams, EV_LIST, &rest);
    AddonParams  o (to_string (posParams[0]()), rest (), to_bool (kwParams[0]()));
    ustring*  ans = NULL;
    o.procOpen (mlenv);
    ans = new ustring;
    o.proc.read (*ans);
    *ans = omitCtrlX (fixUTF8 (*ans));

    return newMNode_str (ans);
}

//#XAFUNC	add-on-tab	ml_addon_tab
MNode*  ml_addon_tab (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    kwParam  kwList[] = {{CharConst ("raw-input"), EV_LIST},
			 {NULL, 0, EV_END}};
    MNodePtr  kwParams[1];
    MNodePtr  rest;
    evalParams (fev, mlenv, cell, posList, posParams, kwList, kwParams, EV_LIST, &rest);
    AddonParams  o (to_string (posParams[0]()), rest (), to_bool (kwParams[0]()));
    o.procOpen (mlenv);
    {
	ustring  a;
	uiterator  b, e, p;
	MNodeList  ans;
	o.proc.read (a);
	a = omitCtrlX (fixUTF8 (a));
	b = a.begin ();
	e = a.end ();
	p = b;
	while (findChar (b, e, '\t')) {
	    ans.append (newMNode_str (new ustring (p, b)));
	    p = ++ b;
	}
	if (p < e) {
	    ans.append (newMNode_str (new ustring (p, e)));
	}
	return mlenv->retval = ans.release ();
    }
}

/*DOC:
===add-on-output===
 (add-on-output [#continue | :continue BOOL] [:type MIME_TYPE] [#raw-input | :raw-input BOOL] COMMAND [ARG1 ARG2...]) -> NIL

*/
//#XAFUNC	add-on-output	ml_addon_output
MNode*  ml_addon_output (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    kwParam  kwList[] = {{CharConst ("continue"), EV_LIST},  // 0
			 {CharConst ("type"), EV_LIST},	     // 1
			 {CharConst ("raw-input"), EV_LIST}, // 2
			 {NULL, 0, EV_END}};
    MNodePtr  kwParams[3];
    MNodePtr  rest;
    evalParams (fev, mlenv, cell, posList, posParams, kwList, kwParams, EV_LIST, &rest);
    AddonParams  o (to_string (posParams[0]()), rest (), to_bool (kwParams[2]()));
    bool  fContinue = to_bool (kwParams[0]());
    ustring  type = to_string (kwParams[1]());
    char*  b;
    ssize_t  s;
    o.procOpen (mlenv);
    if (type.length () > 0) {
	mlenv->env->standardResponse (type);
    } else {
	static ustring  k (CharConst (kMIME_OCTET));
	mlenv->env->standardResponse (k);
    }
#define BUFSIZE	65536
    b = (char*)malloc (BUFSIZE);
    while ((s = o.proc.read (b, BUFSIZE)) > 0) {
	mlenv->env->output->out_raw (b, s);
    }
    free (b);
#undef BUFSIZE
    if (! fContinue)
	mlenv->breakProg ();
    return NULL;
}
