#include "ml-sendmail.h"
#include "ml-store.h"
#include "ml.h"
#include "mlenv.h"
#include "config.h"
#include "motorconst.h"
#include "motorenv.h"
#include "motor.h"
//#include "motoroutput-iconv.h"
//#include "iso2022jp.h"
#include "motoroutput.h"
#include "util_const.h"
#include "util_base64.h"
#include "util_check.h"
#include "util_file.h"
#include "util_string.h"
#include "util_splitter.h"
#include "util_proc.h"
#include "util_time.h"
#include "ustring.h"
#include "expr.h"
#include <vector>
#include <exception>
#include <stdio.h>

#define  TO_LIST_MAX	100

/*DOC:
==mail module==

*/
class  SendmailHdr {
public:
    ustring  fsubj;
    ustring  ffrom;
    ustring  fto;
    ustring  fcc;

    SendmailHdr () {};
    ~SendmailHdr () {};
    virtual void  line (const ustring& t);
};

void  SendmailHdr::line (const ustring& t) {
    uiterator  b, e;
    umatch  m;
    static uregex  re_head ("^((subject:\\s*)|(from:\\s*)|(to:\\s*)|(cc:\\s*))", boost::regex_constants::normal | boost::regex_constants::icase);

    b = t.begin ();
    e = t.end ();
    if (usearch (b, e, m, re_head)) {
	if (m[2].matched) {	// subject
	    fsubj = ustring (m[0].second, e);
	} else if (m[3].matched) { // from
	    ffrom = ustring (m[0].second, e);
	} else if (m[4].matched) { // to
	    fto = ustring (m[0].second, e);
	} else if (m[5].matched) { // cc
	    fcc = ustring (m[0].second, e);
	}
    }
}

static inline int  imin (int a, int b) {
    if (a < b) return a;
    return b;
}

static ustring  mimeEncodeLine(const ustring& text) {
    ustring  ans;
    uiterator  b, e;
    umatch  m;
    ustring  t;
    static uregex  re ("[\x80-\xff]+");
    b = text.begin ();
    e = text.end ();
    while (usearch (b, e, m, re)) {
	if (b != m[0].first)
	    ans.append (b, m[0].first);
	ans.append (CharConst ("=?utf-8?B?"));
	t = base64Encode (m[0].first, m[0].second);
	ans.append (t);
	ans.append (CharConst ("?="));
	b = m[0].second;
    }
    if (b != e)
	ans.append (b, e);
    return ans;
}

static void  sendmail (const ustring& text, const ustring& faddr, std::vector<ustring>& taddr, MlEnv* mlenv) {
    MotorOutputString  out;
    HTMLMotor  motor;
    SendmailHdr  hdr;
    ustring  line;
    ustring  t;
    ProcRW  proc;
    char**  argv;
    int  i;
    FILE*  fd;

    if (text.size () == 0) 
	return;
    {
	AutoSwapMotor  swap (mlenv->env->output, out);
	motor.compile (text);
	motor.output (&out, mlenv->env);
    }
    SplitterNL  sp (out.ans);
    line.resize (0);
    while (sp.next ()) {
	if (sp.b == sp.t) {
	    break;
	}
	if (*sp.b == ' ' || *sp.b == '\t') {
	    line.append (uLF);
	    line.append (sp.pre ());
	} else {
	    if (line.size () > 0)
		hdr.line (line);
	    line = ustring (sp.pre ());
	}
    }
    if (line.size () > 0)
	hdr.line (line);

    argv = (char**)new char* [taddr.size () + 4];
    argv[0] = (char*)cmd_qmailinject;
    argv[1] = (char*)"-f";
    argv[2] = noconst_char (faddr.c_str ());
    for (i = 0; i < taddr.size (); i ++) {
	argv[3 + i] = noconst_char (taddr[i].c_str ());
    }
    argv[3 + i] = NULL;
    proc.open (argv);
    delete argv;
    fd = fdopen (proc.wfd, "w");

    if (hdr.ffrom.size () > 0)
	t = ustring (CharConst ("From: ")) + mimeEncodeLine (hdr.ffrom);
    else
	t = ustring (CharConst ("From: ")) + faddr;
    fwrite (t.data (), sizeof (ustring::value_type), t.size (), fd);
    fwrite ("\n", 1, 1, fd);
				*mlenv->log << hdr.ffrom << "\n";	//***
				    *mlenv->log << t << "\n";	//***
    if (hdr.fto.size () > 0) {
	t = ustring (CharConst ("To: ")) + mimeEncodeLine (hdr.fto);
	fwrite (t.data (), sizeof (ustring::value_type), t.size (), fd);
	fwrite ("\n", 1, 1, fd);
				    *mlenv->log << t << "\n";	//***
    }
    if (hdr.fcc.size () > 0) {
	t = ustring (CharConst ("Cc: ")) + mimeEncodeLine (hdr.fcc);
	fwrite (t.data (), sizeof (ustring::value_type), t.size (), fd);
	fwrite ("\n", 1, 1, fd);
				    *mlenv->log << t << "\n";	//***
    }
    if (hdr.fsubj.size () > 0) {
	t = ustring (CharConst ("Subject: ")) + mimeEncodeLine (hdr.fsubj);
	fwrite (t.data (), sizeof (ustring::value_type), t.size (), fd);
	fwrite ("\n", 1, 1, fd);
				    *mlenv->log << t << "\n";	//***
    }
    fwrite (CharConst ("MIME-Version: 1.0\n"), 1, fd);
    fwrite (CharConst ("Content-Type: text/plain; charset=utf-8\n"), 1, fd);
    fwrite (CharConst ("Content-Transfer-Encoding: base64\n"), 1, fd);
// Date: Sun, 04 Jan 2009 17:32:11 +0900
    t = mailDate ();
    fwrite (t.data (), sizeof (ustring::value_type), t.size (), fd);

    fwrite ("\n", 1, 1, fd);

    t.resize (0);
    while (sp.next ()) {
	if (sp.b < sp.t)
	    t.append (sp.b, sp.t);
	t.append (CharConst ("\r\n"));
    }
    t = base64Encode (t.begin (), t.end ());
    for (int pos = 0; pos < t.size (); pos += 76) {
	int  e = imin (t.size (), pos + 76);
	fwrite (t.data () + pos, sizeof (ustring::value_type), e - pos, fd);
	fwrite ("\n", 1, 1, fd);
    }

    fclose (fd);
}

static void  sendmail_file (const ustring& file, const ustring& faddr, std::vector<ustring>& taddr, MlEnv* mlenv) {
    if (file.size () == 0) 
	return;

    ustring  text;
    if (readFile (file, text))
	sendmail (text, faddr, taddr, mlenv);
}

/*DOC:
===sendmail===
 (sendmail FROM_ADDR TO_ADDR_OR_LIST FILENAME_OR_TEXT #serial #named #static #text) -> NIL
// (sendmail FROM_ADDR TO_ADDR_OR_LIST [:source-serial FILENAME | :source-named FILENAME | :source-static FILENAME | :source-text TEXT]) -> NIL

送り先の最大数は，100。

*/
//#XAFUNC	sendmail	ml_sendmail
MNode*  ml_sendmail (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_LIST, EV_LIST, EV_END};
    MNodePtr  posParams[3];
//    kwParam  kwList[] = {{CharConst ("source-serial"), EV_LIST}, // 0
//			 {CharConst ("source-named"), EV_LIST},	 // 1
//			 {CharConst ("source-static"), EV_LIST}, // 2
//			 {CharConst ("source-text"), EV_LIST},	 // 3
//			 {NULL, 0, EV_END}};
    kwParam  kwList[] = {{CharConst ("serial"), EV_LIST}, // 0
			 {CharConst ("named"), EV_LIST},  // 1
			 {CharConst ("static"), EV_LIST}, // 2
			 {CharConst ("text"), EV_LIST},	  // 3
			 {NULL, 0, EV_END}};
    MNodePtr  kwParams[4];
    evalParams (fev, mlenv, cell, posList, posParams, kwList, kwParams);
    ustring  faddr = to_string (posParams[0]());
    std::vector<ustring>  taddr;
    ustring  msg = to_string (posParams[2]());
    StoreType  storetype (mlenv); 	// static
    storetype.setStatic ();
    if (to_bool (kwParams[0]()))
	storetype.srcSerial (msg); 	// :source-serial
    if (to_bool (kwParams[1]()))
	storetype.srcNamed (msg);	// :source-named
    if (to_bool (kwParams[2]()))
	storetype.srcStatic (msg);	// :source-static
    if (to_bool (kwParams[3]()))
	storetype.srcText (msg);	// :source-text
    if (! checkMailAddr (faddr)) {
	if (mlenv->log)
	    *mlenv->log << faddr << uErrorBadMailAddr << "\n";
	faddr.resize (0);
    }
    if (isCons (posParams[1]())) {
	MNode*  a = posParams[1]();
	while (a) {
	    ustring  u = to_string (a->car ());
	    if (checkMailAddr (u)) {
		taddr.push_back (u);
	    }
	    nextNode (a);		    
	}
    } else {
	ustring  u = to_string (posParams[1]());
	if (! checkMailAddr (u)) {
	    if (mlenv->log)
		*mlenv->log << u << uErrorBadMailAddr << "\n";
	} else {
	    taddr.push_back (u);
	}
    }
    if (taddr.size () > TO_LIST_MAX)
	throw (ustring ("too many addresses"));
    if (faddr.size () == 0) {
	if (mlenv->log)
	    *mlenv->log << "From address is undefined or illegal.\n";
    } else if (taddr.size () == 0) {
	if (mlenv->log)
	    *mlenv->log << "To address is undefined or illegal.\n";
    } else {
	ustring  text;
	text = storetype.read ();
	sendmail (text, faddr, taddr, mlenv);
    }

    return NULL;
}

/*DOC:
===mail-address-p===
 (mail-address-p STRING) -> BOOL

*/
//#XAFUNC	mail-address-p	ml_mail_address_p
//#XWIKIFUNC	mail-address-p
MNode*  ml_mail_address_p (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    evalParams (fev, mlenv, cell, posList, posParams);
    ustring  str = to_string (posParams[0]());
    return newMNode_bool (checkMailAddr (str));
}
