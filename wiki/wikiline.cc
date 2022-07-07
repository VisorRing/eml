#include "wikiline.h"
#include "wikienv.h"
#include "wikitable.h"
#include "wikiformat.h"
#include "wikimotor.h"
#include "motorconst.h"
#include "motoroutput.h"
#include "motorenv.h"
#include "ml.h"
#include "expr.h"
#include "util_const.h"
#include "util_check.h"
#include "util_string.h"
#include "utf8.h"
#include "ustring.h"
#include <boost/ptr_container/ptr_vector.hpp>
#include <vector>
#include <assert.h>

/*DOC:
==インライン要素==

*/
/* ============================================================ */
static void  errorBadParam (WikiMotorObjVec* param, WikiFormat* wiki) {
    wiki->errorMsg.append (param->dump()).append (CharConst (": bad parameter.\n"));
}

static bool  outamp_opt (MotorOutputWiki& o, const ustring& opt, const ustring& val) {
    if (opt.length () == 0) {
	o.outamp (val);
    } else if (opt.length () == 1) {
	switch (opt[0]) {
	case '/':
	    o.outamp_br (val);
	    break;
	case '~':
	    o.outamp_nw (val);
	    break;
	case ',':
	    o.outamp_c3 (val);
	    break;
//	case '^':
//	    o.outamp_wbr (val);
//	    break;
	default:
	    return false;
	}
    } else {
	return false;
    }
    return true;
}

/* ============================================================ */
/*DOC:
===強制改行===
 \<br>

//===幅なしスペース===
// <zwsp>

*/
/*DOC:
===変数展開===
 \{VARIABLE}
 \{VARIABLE/}
 \{VARIABLE~}
 \{VARIABLE,}
 \{VARIABLE->KEY:...}

*/
/*DOC:
===選択出力===
 \{VARIABLE?VALUE:true_text}
 \{VARIABLE!?VALUE:false_text}
 \{VARIABLE?VALUE:true_text||false_text}
 \{VARIABLE!?VALUE:false_text||true_text}
 \{VARIABLE?VALUE1:text1||?VALUE2:text2||?VALUE3:text3||text4}

*/
/*DOC:
===変数展開===
 \{getvar:NAME}
 \{getvar:NAME:/}
 \{getvar:NAME:~}
 \{getvar:NAME:,}

*/
//#WIKILINE	getvar	wl_getvar
bool  wl_getvar (WikiMotorObjVecVec* args, WikiMotorObjVec& out, WikiFormat* wiki) {
    ustring  var;
    ustring  opt;
    ustring  val;

    if (args->size () < 1 || args->size () > 2)
	return false;
    var = (*args)[0]->textOut (wiki);
    if (args->size () >= 2) {
	opt = (*args)[1]->textOut (wiki);
	val = fixUTF8 (wiki->getVar_string (var));
	MotorOutputWiki  o (out);
	return outamp_opt (o, opt, val);
    } else {
	out.push_back (WikiMotorObjPtr (new WikiMotorObjMNode (wiki->getVar (var))));
	return true;
    }
}

/*DOC:
===consセル展開
// \{car:NAME}
// \{cdr:NAME}
 \{nth:NAME:i}
 \{nth:NAME:i:/}
 \{nth:NAME:i:~}
 \{nth:NAME:i:,}
 \{nth:NAME:#}

*/
//#WIKILINE	car	wl_car
bool  wl_car (WikiMotorObjVecVec* args, WikiMotorObjVec& out, WikiFormat* wiki) {
    MotorOutputWiki  o (out);
    ustring  var;
    ustring  opt;
    MNodePtr  val;
    ustring  str;

    if (args->size () < 1 || args->size () > 2)
	return false;
    var = (*args)[0]->textOut (wiki);
    if (args->size () >= 2)
	opt = (*args)[1]->textOut (wiki);
    val = wiki->getVar (var);
    if (isCons (val ())) {
	val = val ()->car ();
	str = fixUTF8 (to_string (val ()));
    }
    return outamp_opt (o, opt, str);
}

//#WIKILINE	cdr	wl_cdr
bool  wl_cdr (WikiMotorObjVecVec* args, WikiMotorObjVec& out, WikiFormat* wiki) {
    MotorOutputWiki  o (out);
    ustring  var;
    ustring  opt;
    MNodePtr  val;
    ustring  str;

    if (args->size () < 1 || args->size () > 2)
	return false;
    var = (*args)[0]->textOut (wiki);
    if (args->size () >= 2)
	opt = (*args)[1]->textOut (wiki);
    val = wiki->getVar (var);
    if (isCons (val ())) {
	val = val ()->cdr ();
	str = fixUTF8 (to_string (val ()));
    }
    return outamp_opt (o, opt, str);
}

//#WIKILINE	nth	wl_nth
bool  wl_nth (WikiMotorObjVecVec* args, WikiMotorObjVec& out, WikiFormat* wiki) {
    ustring  var;
    size_t  idx;
    ustring  opt;
    MNodePtr  val;
    MNodePtr  ans;
    MNode*  e;

    if (args->size () < 2 || args->size () > 3)
	return false;
    var = (*args)[0]->textOut (wiki);
    if (args->size () >= 3)
	opt = (*args)[2]->textOut (wiki);
    val = wiki->getVar (var);
    e = val ();
    WikiMotorObjVec*  ppos = (*args)[1].get ();
    bool  fsize = false;
    if (ppos->match (CharConst ("#")))
	fsize = true;
    if (isCons (e)) {
	if (fsize) {
	    for (idx = 0; isCons (e); ) {
		++ idx;
		nextNode (e);
	    }
	    ans = newMNode_num (idx);
	} else {
	    idx = strtol (ppos->textOut (wiki));
	    while (idx > 0 && isCons (e)) {
		-- idx;
		nextNode (e);
	    }
	    if (isCons (e))
		ans = e->car ();
	}
    } else if (isNil (e)) {
	if (fsize) {
	    ans = newMNode_num (0);
	}
    }
    if (opt.length () == 0) {
	out.push_back (WikiMotorObjPtr (new WikiMotorObjMNode (ans ())));
	return true;
    } else {
	MotorOutputWiki  o (out);
	return outamp_opt (o, opt, fixUTF8 (ans.to_string ()));
    }
}

/*DOC:
===ベクタ展開===
 \{vector:NAME:i}
 \{vector:NAME:#}	-- ベクタの要素数

*/
//#WIKILINE	vector	wl_vector
bool  wl_vector (WikiMotorObjVecVec* args, WikiMotorObjVec& out, WikiFormat* wiki) {
    ustring  var;
    MNodePtr  val;
    ustring  str;

    if (args->size () != 2)
	return false;
    var = (*args)[0]->textOut (wiki);
    val = wiki->getVar (var);
    bool  fsize = false;
    WikiMotorObjVec*  ppos = (*args)[1].get ();
    size_t  idx;
    if (ppos->match (CharConst ("#"))) {
	fsize = true;
    } else {
	idx = to_int64 (ppos->textOut (wiki));
    }
    if (isVector (val ())) {
	if (fsize) {
	    out.push_back (WikiMotorObjPtr (new WikiMotorObjMNode (newMNode_num (val ()->vectorSize ()))));
	} else {
	    val = val ()->vectorGet (idx);
	    out.push_back (WikiMotorObjPtr (new WikiMotorObjMNode (val ())));
	}
    } else if (isNil (val ())) {
	if (fsize) {
	    out.push_back (WikiMotorObjPtr (new WikiMotorObjMNode (newMNode_num (0))));
	}
    }
    return true;
}

/*DOC:
===連結===
 \{join:SEPARATOR:NAME}
Wiki変数''NAME''に格納されたベクタ、または、リストを連結して出力する。

*/
//#WIKILINE	join	wl_join
bool  wl_join (WikiMotorObjVecVec* args, WikiMotorObjVec& out, WikiFormat* wiki) {
    MotorOutputWiki  o (out);
    ustring  sep;
    ustring  var;
    MNodePtr  val;
    size_t  i, n;
    ustring  ans;

    if (args->size () != 2)
	return false;
    sep = (*args)[0]->textOut (wiki);
    var = (*args)[1]->textOut (wiki);
    val = wiki->getVar (var);
    if (isCons (val ())) {
	MNode*  a = val ();
	i = 0;
	while (isCons (a)) {
	    if (i > 0)
		ans.append (sep);
	    ans.append (to_string (a->car ()));
	    nextNode (a);
	    ++ i;
	}
	o.outamp (fixUTF8 (ans));
    } else if (isVector (val ())) {
	n = val ()->vectorSize ();
	for (i = 0; i < n; ++ i) {
	    if (i > 0)
		ans.append (sep);
	    ans.append (to_string (val ()->vectorGet (i)));
	}
	o.outamp (fixUTF8 (ans));
    }
    return true;
}

/*DOC:
===テーブル展開===
 \{table:NAME:i}
// \{table:NAME::#}

*/
//#WIKILINE	table	wl_table
bool  wl_table (WikiMotorObjVecVec* args, WikiMotorObjVec& out, WikiFormat* wiki) {
    ustring  var;
    ustring  key;
    bool  fsize = false;
    MNodePtr  val;

    if (args->size () < 2) {
	return false;
    } else if (args->size () == 3) {
	fsize = (*args)[2].get ()->match (CharConst ("#"));
    } else if (args->size () > 3) {
	return false;
    }
    var = (*args)[0]->textOut (wiki);
    key = (*args)[1]->textOut (wiki);
    val = wiki->getVar (var);
    if (fsize) {
	// XXX
    } else if (isTable (val ())) {
	val = val ()->tableGet (key);
	out.push_back (WikiMotorObjPtr (new WikiMotorObjMNode (val ())));
    }
    return true;
}

/*DOC:
===Lispファンクション実行===
 \{eval:FUNC}

*/
//#WIKILINE	eval	wl_eval
bool  wl_eval (WikiMotorObjVec* arg2, WikiMotorObjVec& out, WikiFormat* wiki) {
#ifdef WIKIEVALPROTECT
    if (wiki->protectMode && ! wl->fsuper) {
	return false;
    }
#endif

    MotorTexp  ml (wiki->mlenv);
    ustring  sexp = arg2->textOut (wiki);
    MNodePtr  ans;

    if (! sexp.empty ()) {
	ml.scan (sexp);
	if (ml.top.isCons ()) {
	    MNode*  arg = ml.top.cdr ();
	    if (arg && arg->isCons ()) {
		try {
		    ans = progn (arg, wiki->mlenv);
		    out.push_back (WikiMotorObjPtr (new WikiMotorObjMNode (ans ())));
		} catch (ustring& msg) {
		    if (wiki->mlenv->currentCell ()) {
			wiki->errorMsg.append (wiki->mlenv->currentCell ()->dump_string_short ()).append (CharConst (": "));
		    }
		    wiki->errorMsg.append (msg).append (uLF);
		}
	    }
	}
    }
    return true;
}

/* ============================================================ */
/*DOC:
===強調===
 ''これはイタリック''
 '''これはボールド'''
 '''''イタリック＋ボールド'''''
 \{i||wiki function形式のイタリック}
 \{b||wiki function形式のボールド}
 \{bi||ボールド＋イタリック}
 \{ib||ボールド＋イタリック}
 \{sup||上付き}
 \{sub||下付き}

*/
static bool  wl_em_sub (const char* op, size_t oplen, const char* cl, size_t cllen,
			WikiMotorObjVec* arg2, WikiMotorObjVec& out, WikiFormat* wiki) {
    MotorOutputString  html;
    html.out_raw (op, oplen)
	->out_toText (arg2->htmlOut (wiki))
	->out_raw (cl, cllen);
    out.push_back (WikiMotorObjPtr (new WikiMotorObjHtml (html.ans)));
    return true;
}

//#WIKILINE	i	wl_italic
bool  wl_italic (WikiMotorObjVec* arg2, WikiMotorObjVec& out, WikiFormat* wiki) {
    return wl_em_sub (CharConst ("<i>"), CharConst ("</i>"), arg2, out, wiki);
}

//#WIKILINE	b	wl_bold
bool  wl_bold (WikiMotorObjVec* arg2, WikiMotorObjVec& out, WikiFormat* wiki) {
    return wl_em_sub (CharConst ("<b>"), CharConst ("</b>"), arg2, out, wiki);
}

//#WIKILINE	bi	wl_bolditalic
//#WIKILINE	ib	wl_bolditalic
bool  wl_bolditalic (WikiMotorObjVec* arg2, WikiMotorObjVec& out, WikiFormat* wiki) {
    return wl_em_sub (CharConst ("<b><i>"), CharConst ("</i></b>"), arg2, out, wiki);
}

//#WIKILINE	sup	wl_sup
bool  wl_sup (WikiMotorObjVec* arg2, WikiMotorObjVec& out, WikiFormat* wiki) {
    return wl_em_sub (CharConst ("<sup>"), CharConst ("</sup>"), arg2, out, wiki);
}

//#WIKILINE	sub	wl_sub
bool  wl_sub (WikiMotorObjVec* arg2, WikiMotorObjVec& out, WikiFormat* wiki) {
    return wl_em_sub (CharConst ("<sub>"), CharConst ("</sub>"), arg2, out, wiki);
}

/* ============================================================ */
/*DOC:
===リンク===
 {http://www.yahoo.co.jp}
 {+http://www.google.co.jp||Googleトップ}
 {https://ab.com/||HTTPSリンク}
 {+https://ab.com/||HTTPSリンク|
// [[link:new.hml?Name=n2&Value=[[v1]] 次]]
// [[link:new.hml?Name=n2&Value=[[v1]]:MainFrame 次]]
 {link:new.html:target=MainFrame:Name:n2:Value:{v1}||次}
 {+link:profile.hml?ID=[[id]]||プロフィール}
 {link:edit({id})||編集}

オプションパラメータ
|table:w=100%|t:w=20%|c:w=10%|t:|
|h:オプション|h:省略形|h:説明|
|'''id='''''ID''||id属性。|
|'''class='''''ClassList''||class属性。複数のクラスを指定する場合はコンマ区切りで連ねる。|
//|'''data-'''''name'''''='''''ID''||Bootstrap用属性。|
|'''onclick='''''Link''||onclick属性。Javascriptリンク。|
|'''onfocus='''''Link''||onfocus属性。Javascriptリンク。|
|'''onblur='''''Link''||onblur属性。Javascriptリンク。|
|'''onchange='''''Link''||onchange属性。Javascriptリンク。|
|'''target='''''Window''||target属性。|
//|''Window''||taget=は省略可。|

*/
static bool  wl_http_sub (bool newwin, WikiMotorObjVecVec* args, WikiMotorObjVec* arg2, WikiMotorObjVec& out, WikiFormat* wiki) {
    ustring  url;
    bool  fscript = false;
//    WikiAttrib1  attrib (wiki, WikiAttrib1::SEL_TARGET | WikiAttrib1::SEL_TARGET2, true, WikiAttrib1::M_ATTRIB);
    WikiAttribLink  attrib (wiki);
    WikiMotorObjVecVec::const_iterator  b = args->begin ();
    WikiMotorObjVecVec::const_iterator  e = args->end ();
    MotorOutputString  html;

    if (b < e) {
	// XXX*** ここではurlにパラメータ「?」を含んではならない。
	attrib.readLink (b, e, url, fscript);
	attrib.readAttrib (b, e);
	html.out_raw (CharConst ("<a"));
	if (fscript) {
	    attrib.onclick = url + attrib.onclick;
	    html.out_raw (CharConst (" href=\"#\""));
	} else {
	    if (attrib.arglist.size () > 0) {
		url.append (CharConst ("?"));
		for (int i = 0; i < attrib.arglist.size (); ++ i) {
		    if (i > 0)
			url.append (uAmp);
		    url.append (percentEncode (attrib.arglist[i])).append (uEq);
		    ++ i;
		    if (i < attrib.arglist.size ()) {
			url.append (percentEncode (attrib.arglist[i]));
		    }
		}
	    }
	    wiki->outputName (&html, CharConst ("href"), url, false);
	}
	if (newwin) {
	    attrib.target = ustring (CharConst ("_blank"));
	}
	attrib.output (&html);
	html.out_raw (CharConst (">"));
	if (arg2->size () > 0) {
	    html.out_toText (arg2->htmlOut (wiki));
	} else if (! fscript) {
	    html.out_toHTML_noCtrl (url);
	} else {
	}
	html.out_raw (CharConst ("</a>"));
    }
    out.push_back (WikiMotorObjPtr (new WikiMotorObjHtml (html.ans)));
    return true;
}

static void  copynargs (WikiMotorObjVecVec& nargs, WikiMotorObjVecVec* args, const ustring& proto) {
    WikiMotorObjVec*  v = new WikiMotorObjVec;
    v->push_back (WikiMotorObjPtr (new WikiMotorObjText (proto)));
    nargs.push_back (WikiMotorObjVecPtr (v));
    for (int i = 0; i < args->size (); i ++) {
	nargs.push_back ((*args)[i]);
    }
}

//#WIKILINE2	http	wl_http
bool  wl_http (WikiMotorObjVecVec* args, WikiMotorObjVec* arg2, WikiMotorObjVec& out, WikiFormat* wiki) {
    WikiMotorObjVecVec  nargs;
    copynargs (nargs, args, uHttp);
    return wl_http_sub (false, &nargs, arg2, out, wiki);
}

//#WIKILINE2	+http	wl_http_new
bool  wl_http_new (WikiMotorObjVecVec* args, WikiMotorObjVec* arg2, WikiMotorObjVec& out, WikiFormat* wiki) {
    WikiMotorObjVecVec  nargs;
    copynargs (nargs, args, uHttp);
    return wl_http_sub (true, &nargs, arg2, out, wiki);
}

//#WIKILINE2	https	wl_https
bool  wl_https (WikiMotorObjVecVec* args, WikiMotorObjVec* arg2, WikiMotorObjVec& out, WikiFormat* wiki) {
    WikiMotorObjVecVec  nargs;
    copynargs (nargs, args, uHttps);
    return wl_http_sub (false, &nargs, arg2, out, wiki);
}

//#WIKILINE2	+https	wl_https_new
bool  wl_https_new (WikiMotorObjVecVec* args, WikiMotorObjVec* arg2, WikiMotorObjVec& out, WikiFormat* wiki) {
    WikiMotorObjVecVec  nargs;
    copynargs (nargs, args, uHttps);
    return wl_http_sub (true, &nargs, arg2, out, wiki);
}

//#WIKILINE2	link	wl_link
bool  wl_link (WikiMotorObjVecVec* args, WikiMotorObjVec* arg2, WikiMotorObjVec& out, WikiFormat* wiki) {
    return wl_http_sub (false, args, arg2, out, wiki);
}

//#WIKILINE2	+link	wl_link_new
bool  wl_link_new (WikiMotorObjVecVec* args, WikiMotorObjVec* arg2, WikiMotorObjVec& out, WikiFormat* wiki) {
    return wl_http_sub (true, args, arg2, out, wiki);
}

/* ============================================================ */
/*DOC:
===IMGタグ===
 \{image:images/logo.gif}
 \{image:images/logo.gif||Logo}
 \{image:images/logo.gif:width=120:height=48||Logo}

オプションパラメータ
|table:w=100%|t:w=20%|c:w=10%|t:|
|h:オプション|h:省略形|h:説明|
|'''id='''''ID''||id属性。|
|'''class='''''ClassList''||class属性。複数のクラスを指定する場合はコンマ区切りで連ねる。|
//|'''data-'''''name'''''='''''ID''||Bootstrap用属性。|
|'''onclick='''''Link''||onclick属性。Javascriptリンク。|
|'''onfocus='''''Link''||onfocus属性。Javascriptリンク。|
|'''onblur='''''Link''||onblur属性。Javascriptリンク。|
|'''onchange='''''Link''||onchange属性。Javascriptリンク。|
|'''width='''''Size''|'''w='''|styleのwidth属性。単位はpx, pt, in, mm, cm, em, ex。または%。|
|'''height='''''Size''|'''h='''|styleのheight属性。単位はpx, pt, in, mm, cm, em, ex。または%。|
|'''alt='''''Text''||alt属性とlongdesc属性の指定。altオプションを指定すると，第二パラメータがlongdesc属性になる。|

*/
//#WIKILINE2	image	wl_image
bool  wl_image (WikiMotorObjVecVec* args, WikiMotorObjVec* arg2, WikiMotorObjVec& out, WikiFormat* wiki) {
    WikiMotorObjVecVec::const_iterator  b = args->begin ();
    WikiMotorObjVecVec::const_iterator  e = args->end ();
    ustring  url;
    WikiAttribImg  attrib (wiki);
    bool  fscript;
    MotorOutputString  html;

    if (! attrib.readLink (b, e, url, fscript, true))
	return false;
    if (! attrib.readAttrib (b, e))
	return false;
    if (b != e) {
	errorBadParam (b->get (), wiki);
	return false;
    }

    html.out_raw (CharConst ("<img"));
    wiki->outputName (&html, CharConst ("src"), url);
    attrib.output (&html);
    if (attrib.alt.length () > 0) {
	wiki->outputName (&html, CharConst ("alt"), attrib.alt);
	wiki->outputName (&html, CharConst ("longdesc"), arg2->textOut (wiki));
    } else {
	wiki->outputName (&html, CharConst ("alt"), arg2->textOut (wiki));
    }
    html.out_raw (CharConst (" />"));
    out.push_back (WikiMotorObjPtr (new WikiMotorObjHtml (html.ans)));

    return true;
}

/*DOC:
===テキストカラー===
 \{color:#ff0000||赤色}

オプションパラメータ
|table:w=100%|t:w=20%|c:w=10%|t:|
|h:オプション|h:省略形|h:説明|
|'''#'''''RRGGBB''||color属性。|

*/
//#WIKILINE	color	wl_color
bool  wl_color (WikiMotorObjVecVec* args, WikiMotorObjVec* arg2, WikiMotorObjVec& out, WikiFormat* wiki) {
    ustring  color, v;
    MotorOutputString  html;

    if (args->size () == 1) {
	color = (*args)[0]->textOut (wiki);
	if (! checkColor (color))
	    return false;
    } else {
	return false;
    }

    html.out_raw (CharConst ("<span style=\"color:"))
	->out_toHTML_noCtrl (color)
	->out_raw (CharConst (";\">"))
	->out_toText (arg2->htmlOut (wiki))
	->out_raw (CharConst ("</span>"));
    out.push_back (WikiMotorObjPtr (new WikiMotorObjHtml (html.ans)));

    return true;
}

static bool  matchAnchor (uiterator b, uiterator e) {
    int  c;
    static char  table_anchor[] = {	// [a-zA-Z0-9_\-]
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 
	0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 
	0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 
    };
    for (; b < e; ++ b) {
	c = *b;
	if (0 <= c && c < 128 && table_anchor[c]) {
	} else {
	    return false;
	}
    }
    return true;
}

/*DOC:
===アンカー===
 \{anchor:Fig1}

*/
//#WIKILINE	anchor	wl_anchor
bool  wl_anchor (WikiMotorObjVec* arg, WikiMotorObjVec& out, WikiFormat* wiki) {
    ustring  name (arg->textOut (wiki));
    MotorOutputString  html;

    if (! matchAnchor (name.begin (), name.end ()))
	return false;
    html.out_raw (CharConst ("<a"));
    wiki->outputName (&html, CharConst ("name"), name, false);
    html.out_raw (CharConst ("></a>"));
    out.push_back (WikiMotorObjPtr (new WikiMotorObjHtml (html.ans)));

    return true;
}

/*DOC:
===SPANタグ===
 \{span:red,top||クラス指定}
 \{span:id=Field1||ID指定}

オプションパラメータ
|table:w=100%|t:w=20%|c:w=10%|t:|
|h:オプション|h:省略形|h:説明|
|'''id='''''ID''||id属性。|
|'''class='''''ClassList''||class属性。複数のクラスを指定する場合はコンマ区切りで連ねる。|
//|'''data-'''''name'''''='''''ID''||Bootstrap用属性。|
|'''onclick='''''Link''||onclick属性。Javascriptリンク。|
|'''onfocus='''''Link''||onfocus属性。Javascriptリンク。|
|'''onblur='''''Link''||onblur属性。Javascriptリンク。|
|'''onchange='''''Link''||onchange属性。Javascriptリンク。|
|''ClassList''||class=は省略可。|

*/
//#WIKILINE2	span	wl_span
bool  wl_span (WikiMotorObjVecVec* args, WikiMotorObjVec* arg2, WikiMotorObjVec& out, WikiFormat* wiki) {
    WikiMotorObjVecVec::const_iterator  b = args->begin ();
    WikiMotorObjVecVec::const_iterator  e = args->end ();
    WikiAttrib1  attrib (wiki, WikiAttrib1::SEL_CLASS2, false, WikiAttrib1::M_ATTRIB);
    MotorOutputString  html;

    if (! attrib.readAttrib (b, e))
	return false;
    if (b != e) {
	errorBadParam (b->get (), wiki);
	return false;
    }

    html.out_raw (CharConst ("<span"));
    attrib.output (&html);
    html.out_raw (CharConst (">"))
	->out_toText (arg2->htmlOut (wiki))
	->out_raw (CharConst ("</span>"));
    out.push_back (WikiMotorObjPtr (new WikiMotorObjHtml (html.ans)));

    return true;
}

/* ============================================================ */
/*DOC:
===フォームエレメント===
 \{input:Name:size=24 初期値}
 \{input:Name:default}
 \{password:PW}
 \{hidden:Name 値}
 \{file:Name}
 \{submit 送信}
 \{button ボタン}
 \{radio:Name:Value:id=Radio1 ラベル}
 \{checkbox:Name:Value ラベル}
 \{textarea:Text:cols=40:rows=6:wrap=soft テキストを入力してください。}
 \{input-number:Name}

オプションパラメータ
|table:w=100%|t:w=15%|t:w=20%|t:c:w=10%|t:|
|h:フォームタグ|h:オプション|h:省略形|h:説明|
|共通|'''id='''''ID''||id属性。|
|^|'''class='''''ClassList''||class属性。複数のクラスを指定する場合はコンマ区切りで連ねる。|
//|^|'''data-'''''name'''''='''''ID''||Bootstrap用属性。|
|^|'''onclick='''''Link''||onclick属性。Javascriptリンク。|
|^|'''onfocus='''''Link''||onfocus属性。Javascriptリンク。|
|^|'''onblur='''''Link''||onblur属性。Javascriptリンク。|
|^|'''onchange='''''Link''||onchange属性。Javascriptリンク。|
|^|'''size='''''Integer''||size属性。|
|^|'''width='''''Size''|'''w='''|width属性。単位はpx, pt, in, mm, cm, em, ex。または%。|
|^|'''accept='''''Type''||accept属性。値は「camera」または，Mime Type。|
|^|'''default'''||FORM変数の値をプリセットする。|
|^radio, check|'''checked'''||checked属性。|
|^radio, check|''WikiLink''||Javascriptリンクを直接指定できる。|
|^textarea|'''cols='''''Integer''||cols属性。|
|^textarea|'''rows='''''Integer''||rows属性。|
|^textarea|'''wrap='''''Value''||wrap属性。値は，off, soft, hard。|
|^textarea|'''tab'''||tab入力させるJavascript出力。|

*/
static bool  wl_input_sub (const char* type, size_t typelen, WikiMotorObjVecVec* args, WikiMotorObjVec* arg2, WikiMotorObjVec& out, WikiFormat* wiki, bool novalue = false) {
    WikiMotorObjVecVec::const_iterator  b = args->begin ();
    WikiMotorObjVecVec::const_iterator  e = args->end ();
    WikiAttribInput  attrib (wiki, WikiAttribInput::SEL_INPUT);
    ustring  name;
    bool  ferr;
    MotorOutputString  html;

    if (b < e) {
	name = (*b)->textOut (wiki);
	b ++;
    }
    if (! attrib.readAttrib (b, e))
	return false;
    if (b != e) {
	errorBadParam (b->get (), wiki);
	return false;
    }

    html.out_raw (CharConst ("<input"));
    wiki->outputName (&html, CharConst ("type"), ustring (type, typelen), false);
    wiki->outputName (&html, CharConst ("name"), name, false);
    attrib.output (&html);
    if (! novalue) {
	if (attrib.pdefault) {
	    wiki->outputName (&html, CharConst ("value"), wiki->getVar_string (name), false);
	} else {
	    wiki->outputName (&html, CharConst ("value"), arg2->textOut (wiki), false);
	}
    }
    html.out_raw (CharConst (" />"));
    out.push_back (WikiMotorObjPtr (new WikiMotorObjHtml (html.ans)));

    return true;
}

bool  wl_input_radiocheck (const char* type, size_t typelen, WikiMotorObjVecVec* args, WikiMotorObjVec* arg2, WikiMotorObjVec& out, WikiFormat* wiki) {
    WikiMotorObjVecVec::const_iterator  b = args->begin ();
    WikiMotorObjVecVec::const_iterator  e = args->end ();
    WikiAttribInput  attrib (wiki, WikiAttribInput::SEL_CHECK);
    ustring  name;
    ustring  value;
    ustring  text;
    MotorOutputString  html;

    if (b < e) {
	name = (*b)->textOut (wiki);
	b ++;
    }
    if (b < e) {
	value = (*b)->textOut (wiki);
	b ++;
    }
    attrib.selector |= WikiAttrib1::SEL_SCRIPT2; // XXX
    if (! attrib.readAttrib (b, e))
	return false;
    attrib.onclick.append (attrib.script);
    text = arg2->htmlOut (wiki);

    if (attrib.pdefault && wiki->getVar_string (name) == value)
	attrib.pchecked = true;
    if (text.length () > 0)
	html.out_raw (CharConst ("<label>"));
    html.out_raw (CharConst ("<input"));
    wiki->outputName (&html, CharConst ("type"), ustring (type, typelen), false);
    wiki->outputName (&html, CharConst ("name"), name, false);
    wiki->outputName (&html, CharConst ("value"), value, false);
    attrib.output (&html);
    html.out_raw (CharConst (" />"));
    if (text.length () > 0)
	html.out_toText (text)
	    ->out_raw (CharConst ("</label>"));
    out.push_back (WikiMotorObjPtr (new WikiMotorObjHtml (html.ans)));

    return true;
}

//#WIKILINE2	input	wl_input
bool  wl_input (WikiMotorObjVecVec* args, WikiMotorObjVec* arg2, WikiMotorObjVec& out, WikiFormat* wiki) {
    return wl_input_sub (CharConst ("text"), args, arg2, out, wiki);
}

//#WIKILINE2	input-number	wl_input_number
bool  wl_input_number (WikiMotorObjVecVec* args, WikiMotorObjVec* arg2, WikiMotorObjVec& out, WikiFormat* wiki) {
    return wl_input_sub (CharConst ("number"), args, arg2, out, wiki);
}

//#WIKILINE2	password	wl_password
bool  wl_password (WikiMotorObjVecVec* args, WikiMotorObjVec* arg2, WikiMotorObjVec& out, WikiFormat* wiki) {
    return wl_input_sub (CharConst ("password"), args, arg2, out, wiki);
}

//#WIKILINE2	hidden	wl_hidden
bool  wl_hidden (WikiMotorObjVecVec* args, WikiMotorObjVec* arg2, WikiMotorObjVec& out, WikiFormat* wiki) {
    return wl_input_sub (CharConst ("hidden"), args, arg2, out, wiki);
}

//#WIKILINE2	file	wl_file
bool  wl_file (WikiMotorObjVecVec* args, WikiMotorObjVec& out, WikiFormat* wiki) {
    WikiMotorObjVec  arg2;

    if (wl_input_sub (CharConst ("file"), args, &arg2, out, wiki, true)) {
	if (wiki->curform)
	    wiki->curform->qfileform = true;
	return true;
    } else {
	return false;
    }
}

bool  wl_button_sub (const char* name, size_t len, WikiMotorObjVecVec* args, WikiMotorObjVec* arg2, WikiMotorObjVec& out, WikiFormat* wiki) {
    WikiMotorObjVecVec::const_iterator  b = args->begin ();
    WikiMotorObjVecVec::const_iterator  e = args->end ();
    WikiAttribButton  attrib (wiki, true);
    MotorOutputString  html;

    if (! attrib.readAttrib (b, e))
	return false;
    attrib.onclick.append (attrib.script);
    html.out_raw (CharConst ("<input type=\""))
	->out_raw (name, len)
	->out_raw ("\"");
    wiki->outputName (&html, CharConst ("name"), attrib.name);
    attrib.output (&html);
    wiki->outputName (&html, CharConst ("value"), arg2->textOut (wiki), false);
    html.out_raw (CharConst (" />"));
    out.push_back (WikiMotorObjPtr (new WikiMotorObjHtml (html.ans)));

    return true;
}

//#WIKILINE2	submit	wl_submit
bool  wl_submit (WikiMotorObjVecVec* args, WikiMotorObjVec* arg2, WikiMotorObjVec& out, WikiFormat* wiki) {
    return wl_button_sub (CharConst ("submit"), args, arg2, out, wiki);
}

//#WIKILINE2	button	wl_button
bool  wl_button (WikiMotorObjVecVec* args, WikiMotorObjVec* arg2, WikiMotorObjVec& out, WikiFormat* wiki) {
    return wl_button_sub (CharConst ("button"), args, arg2, out, wiki);
}

//#WIKILINE2	radio	wl_radio
bool  wl_radio (WikiMotorObjVecVec* args, WikiMotorObjVec* arg2, WikiMotorObjVec& out, WikiFormat* wiki) {
//	<input name="radio" type="radio" value="radio" checked="checked" />
    return wl_input_radiocheck (CharConst ("radio"), args, arg2, out, wiki);
}

//#WIKILINE2	checkbox	wl_checkbox
bool  wl_checkbox (WikiMotorObjVecVec* args, WikiMotorObjVec* arg2, WikiMotorObjVec& out, WikiFormat* wiki) {
//	<input name="checkbox" type="checkbox" value="1" checked="checked" />
    return wl_input_radiocheck (CharConst ("checkbox"), args, arg2, out, wiki);
}

//#WIKILINE2	textarea	wl_textarea
bool  wl_textarea (WikiMotorObjVecVec* args, WikiMotorObjVec* arg2, WikiMotorObjVec& out, WikiFormat* wiki) {
    WikiMotorObjVecVec::const_iterator  b = args->begin ();
    WikiMotorObjVecVec::const_iterator  e = args->end ();
    ustring  name;
    WikiAttribInput  attrib (wiki, WikiAttribInput::SEL_TEXTAREA);
    MotorOutputString  html;

    if (b < e) {
	name = (*b)->textOut (wiki);
	b ++;
    }
    if (! attrib.readAttrib (b, e))
	return false;
    
    if (attrib.pcols.size () == 0)
	attrib.pcols = ustring (CharConst ("80"));

    html.out_raw (CharConst ("<textarea"));
    wiki->outputName (&html, CharConst ("name"), name);
    attrib.output (&html);
    html.out_raw (CharConst (">"));
    if (attrib.pdefault) {
	html.out_toHTML (wiki->getVar_string (name));
    } else {
	html.out_toText (arg2->htmlOut (wiki));
    }
    html.out_raw (CharConst ("</textarea>"));
    out.push_back (WikiMotorObjPtr (new WikiMotorObjHtml (html.ans)));
    return true;
}

/* ============================================================ */
/*DOC:
===数値フォーマット===
 \{pad0:NUMBER:VALUE}

*/
//#WIKILINE	pad0	wl_pad0
bool  wl_pad0 (WikiMotorObjVecVec* args, WikiMotorObjVec& out, WikiFormat* wiki) {
    WikiMotorObjVecVec::const_iterator  b = args->begin ();
    WikiMotorObjVecVec::const_iterator  e = args->end ();
    int  n;
    ustring  t;

    if (b < e) {
	n = strtol ((*b)->textOut (wiki));
	b ++;
	if (b < e) {
	    t = (*b)->textOut (wiki);
	    out.push_back (WikiMotorObjPtr (new WikiMotorObjText (omitCtrl (zeroPad (n, wiki->getVar_string (t))))));
	    return true;
	}
    }
    return false;
}

/*DOC:
 \{c3:VALUE}

*/
//#WIKILINE	c3	wl_c3
bool  wl_c3 (WikiMotorObjVecVec* args, WikiMotorObjVec& out, WikiFormat* wiki) {
    WikiMotorObjVecVec::const_iterator  b = args->begin ();
    WikiMotorObjVecVec::const_iterator  e = args->end ();
    ustring  t;

    if (b < e) {
	t = (*b)->textOut (wiki);
	out.push_back (WikiMotorObjPtr (new WikiMotorObjText (omitCtrl (c3 (t)))));
	return true;
    }
    return false;
}

/*DOC:
===時刻出力===
 \{date:var_timeval}
 \{date:var_timeval||format}
 \{date:integer}
 \{date:integer||format}

 format: %<flag><width><directive>
 flag: -, 0, _, :, ::
 directive: %Y, %y, %C, %G, %g
 	%m, %b, %B
 	%d, %e, %j
 	%u, %w, %a, %A
 	%V, %W, %U
 	%H, %k // %I, %l // %M // %S //%f, %L // %s // %P, %p
 	%Z, %z, %:z, %::z
*/
//#WIKILINE2	date	wl_date
bool  wl_date (WikiMotorObjVecVec* args, WikiMotorObjVec* arg2, WikiMotorObjVec& out, WikiFormat* wiki) {
    WikiMotorObjVecVec::const_iterator  b = args->begin ();
    WikiMotorObjVecVec::const_iterator  e = args->end ();
    ustring  val;
    time_t  tm;
    struct tm  tmv;
    ustring  format;

    if (b < e) {
	val = (*b)->textOut (wiki);
	if (matchNum (val)) {
	} else {
	    val = wiki->getVar_string (val);
	}
	if (val.length () > 0) {
	    tm = strtol (val);
	    if (arg2->size () > 0)
		format = arg2->textOut (wiki);
	    else
		format = uTimeFormat;
	    localtime_r (&tm, &tmv);
	    out.push_back (WikiMotorObjPtr (new WikiMotorObjText (formatDateString (format, tmv))));
	}
	return true;
    }

    return false;
}

/*DOC:
===文字列出力===
 {q:TEXT}
 {q:TEXT:/}
 {q:TEXT:~}
 {q:TEXT:,}

*/
//#WIKILINE	q	wl_q
//bool  wl_q (WikiMotorObjVec* arg2, WikiMotorObjVec& out, WikiFormat* wiki) {
bool  wl_q (WikiMotorObjVecVec* args, WikiMotorObjVec& out, WikiFormat *wiki) {
    MotorOutputWiki  o (out);
    ustring  text;
    ustring  opt;
    if (args->size () >= 1) {
	text = (*args)[0]->textOut (wiki);
	if (args->size () >= 2)
	    opt = (*args)[1]->textOut (wiki);
	return outamp_opt (o, opt, text);
    } else {
	return false;
    }
}

/* ============================================================ */
#if 0
bool  wl_M2 (WikiMotorObjVecVec* args, WikiMotorObjVec* arg2, WikiMotorObjVec& out, WikiFormat* wiki) {
    out->out_raw (CharConst ("{{"));
    for (int i = 0; i < args.size (); i ++) {
	if (i > 0)
	    out->out_raw (uColon);
	out->out_text (args[i]);
    }
    out->out_raw (uSPC)->out_text (arg2)->out_raw (CharConst ("}}"));
    return true;
}
#endif
/* ============================================================ */
