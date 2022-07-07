#include "wikiformat.h"
#include "config.h"
#include "wikiline.h"
#include "wikitable.h"
#include "wikicmd.h"
#include "wikienv.h"
#include "ml.h"
#include "expr.h"
#include "motorconst.h"
#include "motorenv.h"
#include "motoroutput.h"
#include "util_const.h"
#include "util_check.h"
#include "util_string.h"
#include "utf8.h"
#include "ustring.h"
#include <boost/ptr_container/ptr_vector.hpp>

/*
  WIKIMOTORPRE: セットすると，フォーマット済みテキストブロックの中でインライン要素の解釈を行う。
  UTF8JP: セットすると，条件によりパラグラフ内の改行を空白に置き換えない。
  WIKITABLECOMPATFLAG: セットすると，旧形式の表の継続記号もゆるす。
*/

#define  kComment	"//"
#define  kWikiP		'^'
#define  kWikiH		'='
#define  kWikiPRE	' '
#define  kWikiPRE2	'\t'
#define  kWikiUL	'*'
#define  kWikiOL	'#'
//#define  kWikiNL	']'
#define  kWikiNL	'+'
#define  kWikiDL	';'
#define  kWikiTABLE	'|'
#define  kWikiQUOTE	'>'
#define  kWikiQUOTE_e	'<'
#define  kWikiDIV	'{'
#define  uWikiDIV	"{div:"
#define  uWikiFORM	"{form:"
#define  uWikiELEMENT	"{element:"
#define  kWikiDIV_e	'}'
#define  kWikiHR	'-'
#define  kWikiCmd	'$'
#define  uWikiCmd	"$"

#define  uP	"<p>"
#define  uPe	"</p>\n"
#define  uH1	"<h1>"
#define  uH2	"<h2>"
#define  uH3	"<h3>"
#define  uH4	"<h4>"
#define  uH5	"<h5>"
#define  uH6	"<h6>"
#define  uH1e	"</h1>\n"
#define  uH2e	"</h2>\n"
#define  uH3e	"</h3>\n"
#define  uH4e	"</h4>\n"
#define  uH5e	"</h5>\n"
#define  uH6e	"</h6>\n"
#define  uHR	"<hr />\n"
//#define  unbsp	"&nbsp;"

ustring  uWiki (CharConst ("wiki_"));
ustring  uAWiki (CharConst ("@wiki_"));

/*DOC:
$pre-mode:true
==テキストの整形ルール==

===ディレクティブ行
最初の行が、$$で始まるとき、ディレクティブ行。

 $$normal-mode

*/
/* ============================================================ */
void  WikiMlEnv::setVar (const ustring& name, MNode* val) {
    MlEnv::setVar (wikiVarName (name), val);
}

void  WikiMlEnv::setAry (const ustring& name, size_t i, MNode* val) {
    MlEnv::setAry (wikiVarName (name), i, val);
}

void  WikiMlEnv::setArySize (const ustring& name, size_t n) {
    MlEnv::setArySize (wikiVarName (name), n);
}

void  WikiMlEnv::setAry (const ustring& name, MNode* list) {
    MlEnv::setAry (wikiVarName (name), list);
}

MNode*  WikiMlEnv::getVar (const ustring& name) {
    return MlEnv::getVar (wikiVarName (name));
}

MNode*  WikiMlEnv::getAry (const ustring& name, size_t i) {
    return MlEnv::getAry (wikiVarName (name), i);
}

size_t  WikiMlEnv::getArySize (const ustring& name) {
    return MlEnv::getArySize (wikiVarName (name));
}

void  WikiMlEnv::setLocalVar (const ustring& name, MNode* val) {
    MlEnv::setLocalVar (wikiVarName (name), val);
}

void  WikiMlEnv::defineLocalVar (const ustring& name) {
    MlEnv::defineLocalVar (wikiVarName (name));
}

ustring  WikiMlEnv::wikiVarName (const ustring& name) {
    ustring  ans (uWiki);
    ans.append (name);
    return ans;
}

/* ============================================================ */
void  WikiMotorOutputString::flush (bool fsuper) {
    wikiOutput (ans, fsuper, wiki);
    ans.resize (0);
}

/* ============================================================ */
void  WikiBlockComplex::outputBlock (MotorOutput* out) {
    for (int i = 0; i < block.size (); i ++) {
	block[i].output (out);
    }
}

/* ============================================================ */
/*DOC:
===段落===
 マークアップ文字以外の文字で書き出すと，段落になる。
 ^行頭が他のマークアップ文字になるときは，行頭に^を書く。
 
 空行は，段落を分ける。
 
 
 段落と段落の間に空行を２行以上入れると、「<div class="pggap"></div>」を出力する。

{div:greenbox
マークアップ文字以外の文字で書き出すと，段落になる。
^行頭が他のマークアップ文字になるときは，行頭に^を書く。

空行は，段落を分ける。


段落と段落の間に空行を２行以上入れると、「<div class="pggap"></div>」を出力する。
}

*/
bool  WikiBlockParagraph::nextLine (uiterator b, uiterator e) {
    if (*b == kWikiP) {
	addLine (b, e);
	return true;
    } else {
	return false;
    }
}

void  WikiBlockParagraph::addLine (uiterator b, uiterator e) {
    if (b[0] == kWikiP) {
	b ++;
	pflag = true;
    }
    if (b == e)
	return;
    {
	WikiMotorObjVec  o;
	WikiMotorObjVec::iterator  ob;
	WikiMotorObjVec::iterator  oe;
	wiki->wikiMotorInline (b, e, o);
	if (objv.size () > 0 && o.size () > 0) {
#ifdef UTF8JP
	    WikiMotorObj*  v;
	    WikiMotorObjText*  t1;
	    WikiMotorObjText*  t2;
	    uiterator  i1, i2;
	    int  n;
	    v = &*objv.back ();
	    if (v->type == WikiMotorObj::wiki_text) {
		t1 = WikiMotorObjText_type (v);
		v = &*o.front ();
		if (v->type == WikiMotorObj::wiki_text) {
		    t2 = WikiMotorObjText_type (v);
		    if (t1->text.length () > 0 && t2->text.length () > 0) {
			lastChar (t1->text, i1);
			n = t1->text.end () - i1;
			i2 = t2->text.begin ();
			nextChar (i2, t2->text.end ());
			if (n == 3 && i2 - t2->text.begin () == 3) {
			} else {
			    objv.push_back (WikiMotorObjPtr (new WikiMotorObjText (uSPC)));
			}
		    } else {
			objv.push_back (WikiMotorObjPtr (new WikiMotorObjText (uSPC)));
		    }
		} else {
		    objv.push_back (WikiMotorObjPtr (new WikiMotorObjText (uSPC)));
		}
	    } else {
		objv.push_back (WikiMotorObjPtr (new WikiMotorObjText (uSPC)));
	    }
#else
	    objv.push_back (WikiMotorObjPtr (new WikiMotorObjText (uSPC)));
#endif
	}
	ob = o.begin ();
	oe = o.end ();
	for (; ob < oe; ob ++) {
	    objv.push_back (*ob);
	}
    }
}

void  WikiBlockParagraph::addHtml (const ustring& ht) {
    objv.push_back (WikiMotorObjPtr (new WikiMotorObjHtml (ht)));
}

void  WikiBlockParagraph::output (MotorOutput* out) {
    if (singleTag) {
	out->out_toText (objv.htmlOut (wiki));
    } else {
	out->out_raw (CharConst (uP));
	out->out_toText (objv.htmlOut (wiki));
	out->out_raw (CharConst (uPe));
    }
}

/* ============================================================ */
/*DOC:
===タイトル===
行頭に=を書くと，タイトルになる。行末の=は，省略可。

 =タイトル１=
 ==タイトル２==
 ===タイトル３===#anchor1
{div:greenbox
=タイトル１=
==タイトル２==
===タイトル３===#anchor1
}

*/

bool  WikiBlockH::nextLine (uiterator b, uiterator e) {
    return false;
}

void  WikiBlockH::addLine (uiterator b, uiterator e) {
    uiterator  u;
    umatch  m;
    int  n, cn;
    static uregex  re ("(=+)#([a-zA-Z0-9_-]+)$");

    cn = wiki->countWikiH (b, e);
    level = cn + wiki->headbase;
    if (level > 6)
	level = 6;
    level0 = wiki->hlevel;

    for (; b < e && *b == ' '; b ++) {}		// 先頭の空白をスキップ
    if (usearch (b, e, m, re)) {
	u = m[0].first;
	n = m[1].second - m[1].first - cn;
	if (n > 0) {
	    u += n;
	}
	for (; b < u && u[-1] == ' '; u --) {}
	title = wiki->wikiMotor (b, u);
	anchor = ustring (m[2].first, m[2].second);
    } else {
	u = e;
	for (n = cn; n > 0 && b < u && u[-1] == '='; u --, n --) {}
	for (; b < u && u[-1] == ' '; u --) {}
	title = wiki->wikiMotor (b, u);
    }
    wiki->hlevel = level;
}

WikiBlock::closeType  WikiBlockH::closeLine (uiterator b, uiterator e) {
    if (e - b == 1 && *b == kWikiDIV_e) {
	// closing point of the div
	int  i;
	for (i = wiki->bstack.size () - 1; i >= 0; i --) {
	    switch (wiki->bstack[i]->type) {
	    case BlockDiv:
	    case BlockForm:
		// found
		for (i = wiki->bstack.size () - 1; i >= 0; i --) {
		    wiki->pop_block ();
		    switch (wiki->bstack[i]->type) {
		    case BlockDiv:
		    case BlockForm:
			if (wiki->cur)
			    wiki->cur->close ();
			wiki->cur = NULL;
			return CloseTrue;
		    default:;
		    }
		}
	    default:;
	    }
	}
	// no opening point.
	return CloseFalse;
    } else if (b != e && *b == kWikiH) {
	int  l = wiki->countWikiH (b, e) + wiki->headbase;
	int  i;
	if (l > 6)
	    l = 6;
	WikiBlockH*  obj;
	for (i = wiki->bstack.size () - 1; i >= 0; i --) {
	    if (wiki->bstack[i]->type == BlockH) {
		obj = (WikiBlockH*)wiki->bstack[i];
		if (obj->level >= l) {
		    wiki->pop_block ();
		} else {
		    break;
		}
	    } else {
		break;
	    }
	}
	if (wiki->cur)
	    wiki->cur->close ();
	wiki->cur = NULL;
	return CloseFalse;
    } else {
	return CloseFalse;
    }
}

void  WikiBlockH::close () {
    wiki->hlevel = level0;
}

void  WikiBlockH::output (MotorOutput* out) {
    int  i;

    assert (0 < level && level <= 6);
    if (checkEmpty ())
	return;

    outputBeginDiv (level, out);
    switch (level) {
    case 1: out->out_raw (CharConst (uH1)); break;
    case 2: out->out_raw (CharConst (uH2)); break;
    case 3: out->out_raw (CharConst (uH3)); break;
    case 4: out->out_raw (CharConst (uH4)); break;
    case 5: out->out_raw (CharConst (uH5)); break;
    case 6: out->out_raw (CharConst (uH6)); break;
    }
    if (anchor.size () > 0) {
	out->out_raw (CharConst ("<a name=\""))->out_toHTML_noCtrl (anchor)->out_raw (CharConst ("\">"))->out_toText (title)->out_raw (CharConst ("</a>"));
    } else {
	out->out_noCtrl (title);
    }
    switch (level) {
    case 1: out->out_raw (CharConst (uH1e)); break;
    case 2: out->out_raw (CharConst (uH2e)); break;
    case 3: out->out_raw (CharConst (uH3e)); break;
    case 4: out->out_raw (CharConst (uH4e)); break;
    case 5: out->out_raw (CharConst (uH5e)); break;
    case 6: out->out_raw (CharConst (uH6e)); break;
    }
    outputBlock (out);
    outputEndDiv (level, out);
}

void  WikiBlockH::outputBeginDiv (int lv, MotorOutput* out) {
    switch (lv) {
    case 1: out->out_raw (CharConst ("<div class=\"hh1\">\n")); break;
    case 2: out->out_raw (CharConst ("<div class=\"hh2\">\n")); break;
    case 3: out->out_raw (CharConst ("<div class=\"hh3\">\n")); break;
    case 4: out->out_raw (CharConst ("<div class=\"hh4\">\n")); break;
    case 5: out->out_raw (CharConst ("<div class=\"hh5\">\n")); break;
    case 6: out->out_raw (CharConst ("<div class=\"hh6\">\n")); break;
    }
}

void  WikiBlockH::outputEndDiv (int lv, MotorOutput* out) {
#ifdef DEBUG
    out->out_raw (CharConst ("</div><!-- hh"))->out_raw (to_ustring (lv))->out_raw (CharConst (" -->\n"));
#else
    out->out_raw (CharConst ("</div>\n"));
#endif /* DEBUG */
}

bool  WikiBlockH::checkEmpty () {
    return title.empty ();
}

/* ============================================================ */
/*DOC:
===フォーマット済みテキストブロック===
  行頭に空白文字を書くと，<pre>〜</pre>タグで囲まれます。
インライン要素の解釈は行われない。
{div:greenbox
 行頭に空白文字を書くと，<pre>〜</pre>タグで囲まれます。
}

*/
bool  WikiBlockPreformatRaw::nextLine (uiterator b, uiterator e) {
    if (*b == markup) {
	addLine (b, e);
	return true;
    } else {
	return false;
    }
}

void  WikiBlockPreformatRaw::addLine (uiterator b, uiterator e) {
    markup = *b;

    if (markup == ' ')
	b ++;
    if (wiki->motorPre) {
	text.append (wiki->wikiMotor (b, e));
    } else {
	MotorOutputString  o;
	o.out_toHTML (ustring (b, e));
	text.append (o.ans);
    }
    text.append (uLF);
}

void  WikiBlockPreformatRaw::output (MotorOutput* out) {
    out->out_raw (CharConst ("<pre>"))->out_toText (text)->out_raw (CharConst ("</pre>\n"));
}

/* ============================================================ */
bool  WikiBlockItemText::nextLine (uiterator b, uiterator e) {
    return false;
}

void  WikiBlockItemText::addLine (uiterator b, uiterator e) {
    WikiMotor  motor (b, e, wiki);
    WikiMotorObjVecPtr  v1 (new WikiMotorObjVec);

    motor.compile (*v1);
    if (! attrib.shiftAttrib (v1))
	return;
    v1->trimSpc ();		// 前後の空白のトリミング
    html.append (v1->htmlOut (wiki));
}

bool  WikiBlockItemText::checkAddLine (uiterator b, uiterator e) {
    int  ch;
    WikiBlock*  obj;
    WikiBlockItem*  wbi;

    if (b == e) {
    } else {
	ch = *b;
	if (block.size () > 0) {
	    obj = &block.back ();
	    switch (obj->type) {
	    case BlockItemUL:
	    case BlockItemOL:
	    case BlockItemNL:
		wbi = (WikiBlockItem*)obj;
		if (wbi->ch == ch) {
		    wbi->addLine (b, e);
		    return true;
		}
		break;
	    default:;
	    }
	}
    }
    return false;
}

void  WikiBlockItemText::output (MotorOutput* out) {
    if (indentHack) {
	outputBlock (out);
    } else {
//	if (html.length () > 0) {
	    out->out_raw (CharConst ("<li"));
	    attrib.output (out);
	    out->out_raw (CharConst (">"));
	    out->out_toText (html);
	    outputBlock (out);
	    out->out_raw (CharConst ("</li>\n"));
//	} else {
//	    outputBlock (out);
//	}
    }
}

/* ============================================================ */
/*DOC:
===箇条書き===
 *項目1
 **項目1-1
 #ナンバリング1
 ##ナンバリング1-2
 +インデント1
 ++インデント1-1
 *''[OPTION1:OPTION2:...:]''TEXT
 *id=item1:class=style1:onclick=\{ignore:}:TEXT

行頭に「*」を書くと，<UL>タグによるリスト，「#」を書くと<NL>タグによる番号付きリスト，「+」を書くとnobullクラス指定の<UL>タグによるリスト。

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
|'''start='''''Number''||start属性。OLのみ。|

{div:greenbox
*項目1
**項目1-1
#ナンバリング1
##ナンバリング1-2
+インデント1
++インデント1-1
*id=item1:class=style1:onclick={ignore:}:TEXT
}

*/
bool  WikiBlockItem::nextLine (uiterator b, uiterator e) {
    if (*b == ch) {
	addLine (b, e);
	return true;
    } else {
	return false;
    }
}

void  WikiBlockItem::addLine (uiterator b, uiterator e) {
    WikiBlock*  wb;
    WikiBlockItem*  wbi;
    WikiBlockItemText*  wbt;
    int  c;

    if (ch == 0)
	ch = b[0];
    assert (b != e && b[0] == ch);
    b ++;
    if (b == e) {
	wbt = new WikiBlockItemText (wiki, this);
	block.push_back (wbt);
	wbt->addLine (b, e);
    } else {
	c = *b;
	if (block.size () > 0 && block.back ().checkAddLine (b, e)) {
	} else {
	    switch (c) {
	    case kWikiUL:	// *
		wbi = new WikiBlockItem (WikiBlock::BlockItemUL, wiki);
		break;
	    case kWikiOL:	// #
		wbi = new WikiBlockItem (WikiBlock::BlockItemOL, wiki);
		break;
	    case kWikiNL:	// +
		wbi = new WikiBlockItem (WikiBlock::BlockItemNL, wiki);
		break;
	    default:
		wbi = NULL;
	    }
	    if (wbi) {
		if (block.size () > 0) {
		    wbt = &block.back ();
		    wbi->addLine (b, e);
		    wbt->block.push_back (wbi);
		} else {
		    wbt = new WikiBlockItemText (wiki, this);
		    block.push_back (wbt);
		    wbi->addLine (b, e);
		    wbt->block.push_back (wbi);
		    wbt->indentHack = true;
		}
	    } else {
		wbt = new WikiBlockItemText (wiki, this);
		block.push_back (wbt);
		wbt->addLine (b, e);
	    }
	}
    }
}

void  WikiBlockItem::output (MotorOutput* out) {
    int  i;

    switch (type) {
    case BlockItemUL:
	out->out_raw (CharConst ("<ul"));
	attrib.output (out);
	out->out_raw (CharConst (">\n"));
	outputBlock (out);
	out->out_raw (CharConst ("</ul>\n"));
	break;
    case BlockItemOL:
	out->out_raw (CharConst ("<ol"));
	attrib.output (out);
	out->out_raw (CharConst (">\n"));
	outputBlock (out);
	out->out_raw (CharConst ("</ol>\n"));
	break;
    case BlockItemNL:
	out->out_raw (CharConst ("<ul"));
	attrib.classlist.push_back (ustring (CharConst ("nobull")));
	attrib.output (out);
	attrib.classlist.pop_back ();
	out->out_raw (CharConst (">\n"));
	outputBlock (out);
	out->out_raw (CharConst ("</ul>\n"));
	break;
    default:
	std::cerr << "type:" << type << uLF;
	assert (0);
    }
}

void  WikiBlockItem::outputBlock (MotorOutput* out) {
    for (int i = 0; i < block.size (); i ++) {
	block[i].output (out);
    }
}

/* ============================================================ */
/*xxxDOC:
===定義リスト===
 ;見出し:説明文

{div:greenbox
;見出し:説明文
}

*/
bool  WikiBlockItemDL::nextLine (uiterator b, uiterator e) {
    if (*b == kWikiDL) {
	addLine (b, e);
	return true;
    } else {
	return false;
    }
}

void  WikiBlockItemDL::addLine (uiterator b, uiterator e) {
    WikiMotorObjVec  objs;
    WikiMotorObjVec*  def;
    WikiMotorObjVec*  desc;

    wiki->wikiMotorInline (b + 1, e, objs);
    def = new WikiMotorObjVec;
    desc = new WikiMotorObjVec;
    objv1.push_back (def);
    objv2.push_back (desc);
    objs.splitChar (':', *def, *desc);
}

void  WikiBlockItemDL::output (MotorOutput* out) {
    int  i;

    out->out_raw (CharConst ("<dl>\n"));
    for (i = 0; i < objv1.size (); i ++) {
	out->out_raw (CharConst ("<dt>"))->out_toText (objv1[i].htmlOut (wiki))->out_raw (CharConst ("</dt>\n"));
	out->out_raw (CharConst ("<dd>"))->out_toText (objv2[i].htmlOut (wiki))->out_raw (CharConst ("</dd>\n"));
    }
    out->out_raw (CharConst ("</dl>\n"));
}

/* ============================================================ */
/*DOC:
===表===
 |table:w=100%:数量リスト|head:right:||right:|
 |*:|head:品名|head:数量|
 |1|テレビ|2|
 |2|空気清浄機|3|

+・テーブルセルオプション
表定義の最初の行の最初のカラムに「table:」を書くと，この行はテーブルオプション指定行になる。属性指定の後にテキストを書くと，表のキャプションになる。
２番目以降のカラムは，デフォルトのカラムオプション指定になる。

|table:w=100%:テーブルオプション|t:w=20%|c:w=10%|t:|
|h:オプション|h:省略形|h:説明|
|'''id='''''ID''||id属性。|
|'''class='''''ClassList''||class属性。複数のクラスを指定する場合はコンマ区切りで連ねる。|
//|'''data-'''''name'''''='''''ID''||Bootstrap用属性。|
|'''onclick='''''Link''||onclick属性。Javascriptリンク。|
|'''onfocus='''''Link''||onfocus属性。Javascriptリンク。|
|'''onblur='''''Link''||onblur属性。Javascriptリンク。|
|'''onchange='''''Link''||onchange属性。Javascriptリンク。|
|'''width='''''Size''|'''w='''|width属性。単位はpx, pt, in, mm, cm, em, ex。または%。|
//|'''height='''''Size''|'''h='''|height属性。単位はpx, pt, in, mm, cm, em, ex。または%。|
|'''color='''''Color''||color style属性。|
|'''bgcolor='''''Color''|'''bg='''|bgcolor属性。|
|'''cellspacing='''''Integer''|'''spc='''|cellspacing属性。＊コンパイル時設定による。|
|'''cellpadding='''''Integer''||cellpadding属性。＊コンパイル時設定による。|
|'''left'''|'''l'''|align="left"属性。|
|'''right'''|'''r'''|align="right"属性。|
|'''center'''|'''c'''|align="center"属性。|
|'''noborder'''|''nb''|border="0"属性。|
|'''padding'''|'''pad'''|行のカラム定義が最大カラム数に満たない時，空のカラムを追加する。|
|'''expanding'''|'''expand'''|行のカラム定義が最大カラム数に満たない時，最終カラムを伸ばす。|
|'''nowhite'''|'''nw'''|空セルにnbspを出力する。|
|'''turn'''||表の行と列を入れ替える。|

+・行オプション
 |row:bgcolor=#ff0|\{Name}|\{Value}|
表の行の最初のカラムに「row:」を書くと，このカラムは行オプション指定になる。行オプション指定のカラムは，表には出力されず，この後のカラムが表のカラムとなる。
行オプションで指定できる属性はセルオプションと同じものである。

+・セルオプション
// |header:名前|class=big:東武|
// |header:

|table:w=100%:行オプション，セルオプション|t:w=20%|c:w=10%|t:|
|h:オプション|h:省略形|h:説明|
|'''id='''''ID''||id属性。|
|'''class='''''ClassList''||class属性。複数のクラスを指定する場合はコンマ区切りで連ねる。|
//|'''data-'''''name'''''='''''ID''||Bootstrap用属性。|
|'''onclick='''''Link''||onclick属性。Javascriptリンク。|
|'''onfocus='''''Link''||onfocus属性。Javascriptリンク。|
|'''onblur='''''Link''||onblur属性。Javascriptリンク。|
|'''onchange='''''Link''||onchange属性。Javascriptリンク。|
|'''width='''''Size''|'''w='''|width属性。単位はpx, pt, in, mm, cm, em, ex。または%。|
|'''height='''''Size''|'''h='''|height属性。単位はpx, pt, in, mm, cm, em, ex。または%。|
|'''color='''''Color''||color style属性。|
|'''bgcolor='''''Color''|'''bg='''|bgcolor属性。|
|'''header'''|'''h'''|thタグを出力する。|
|'''left'''|'''l'''|align="left"属性。|
|'''right'''|'''r'''|align="right"属性。|
|'''center'''|'''c'''|align="center"属性。|
|'''top'''|'''t'''|valign="top"属性。|
|'''middle'''|'''m'''|valign="middle"属性。|
|'''bottom'''|'''b'''|valign="bottom"属性。|
|'''nowrap'''|'''nw'''|nowrap属性。|
|'''*'''||オプションのクリア。|

|h:結合記号|h:意味|
|\<|左のセルと結合する。|
|\<''text''|左のセルと内容が一致するとき，結合する。|
|\<^''text''|左のセルと内容が一致し，左のセルが上のセルと結合しているとき，結合する。|
|\^|上のセルと結合する。|
|\^''text''|上のセルと内容が一致するとき，結合する。|
|\^<''text''|上のセルと内容が一致し，上のセルが左のセルと結合しているとき，結合する。|

+・継続オプション
行末に「&」を指定すると，次のテーブル記述を１行として継続する。数値指定を付加し「!''NUM''&」と指定するとカラム数が''NUM''に達したところで継続をやめる。

*/
void  WikiBlockTable::TableCell::cellBody (WikiMotorObjVec* vtext, WikiBlockTable* table, int idx) {
    int  i;
    CellList_t*  col;

    if (vtext->size () > 0 && (*vtext)[0].get ()->type == WikiMotorObj::wiki_text) {
	WikiMotorObjText*  w = WikiMotorObjText_type ((*vtext)[0].get ());
	uiterator  b = w->text.begin ();
	uiterator  e = w->text.end ();
	
	if (b < e && *b == '<') {
	    fcolspan = true;
	    b ++;
	    if (b < e && *b == '^') {
		b ++;
		spanmatch = true;
	    }
	} else if (b < e && *b == '^') {
	    frowspan = true;
	    b ++;
	    if (b < e && *b == '<') {
		b ++;
		spanmatch = true;
	    }
	}
	(*vtext)[0].reset (new WikiMotorObjText (b, e));
    }

    html = vtext->htmlOut (table->wiki);
    if (fcolspan) {
	if (! spanmatch || table->ary.size () <= 1 || table->ary[table->ary.size () - 2][idx].colspan == 0) {
	    col = &table->ary.back ();
	    for (i = idx - 1; i >= 0; i --) {
		if ((*col)[i].colspan > 0) {
		    if (html.length () == 0 || (*col)[i].html == html) {
			colspan = 0;
			(*col)[i].colspan ++;
		    }
		    break;
		}
	    }
	}
    }
    if (frowspan) {
	if (! spanmatch || idx == 0 || table->ary.back ()[idx - 1].rowspan == 0) {
	    for (i = table->ary.size () - 2; i >= 0; i --) {
		col = &table->ary[i];
		if (idx < col->size () && (*col)[idx].rowspan > 0) {
		    if (html.length () == 0 || (*col)[idx].html == html) {
			rowspan = 0;
			(*col)[idx].rowspan ++;
		    }
		    break;
		}
	    }
	}
    }
}

void  WikiBlockTable::TableCell::outputTD (WikiFormat* wiki, MotorOutput* out, bool fturn) {
    int  i;

    if (cellattrib.fheader)
	out->out_raw (CharConst ("<th"));
    else
	out->out_raw (CharConst ("<td"));
    cellattrib.output (out);
    if (fturn) {
	if (colspan > 1)
	    wiki->outputName (out, CharConst ("rowspan"), colspan, false);
	if (rowspan > 1)
	    wiki->outputName (out, CharConst ("colspan"), rowspan, false);
    } else {
	if (colspan > 1)
	    wiki->outputName (out, CharConst ("colspan"), colspan, false);
	if (rowspan > 1)
	    wiki->outputName (out, CharConst ("rowspan"), rowspan, false);
    }
    out->out_raw (CharConst (">"));
}

void  WikiBlockTable::TableCell::outputTDe (MotorOutput* out) {
    if (cellattrib.fheader)
	out->out_raw (CharConst ("</th>\n"));
    else
	out->out_raw (CharConst ("</td>\n"));
}

/* ============================================================ */
bool  WikiBlockTable::nextLine (uiterator b, uiterator e) {
    if (*b == kWikiTABLE) {
	addLine (b, e);
	return true;
    } else {
	return false;
    }
}

void  WikiBlockTable::addLine (uiterator b, uiterator e) {
    assert (b[0] == '|');
    b ++;
    if (n == 0 && matchSkip (b, e, CharConst ("table:"))) {
	addLine_head (b, e);
    } else {
	addLine_body (b, e);
    }
    n ++;
}

WikiBlock::closeType  WikiBlockTable::closeLine (uiterator b, uiterator e) {
#ifdef WIKITABLECOMPATFLAG
    static uregex  re ("^\\}\\}(($)|(\\|)|((!([1-9][0-9]*))?(\\\\|&)$))");
#else
    static uregex  re ("^\\}\\}(($)|(\\|)|((!([1-9][0-9]*))?(&)$))");
#endif
    umatch  m;

    if (usearch (b, e, m, re)) {
	if (m[2].matched) {	// }}
	    wiki->pop_block ();
	    return CloseTrue;
	} else if (m[3].matched) { // }}|...
	    wiki->pop_block ();
	    if (wiki->cur && wiki->cur->type == BlockTable) {
		WikiBlockTable*  obj = (WikiBlockTable*)wiki->cur;
		obj->cont = true;
		addLine_body (m[3].second, e);
	    } else {
		assert (0);
	    }
	    return CloseTrue;
	} else if (m[4].matched) {		// }}\     .
	    wiki->pop_block ();
	    if (wiki->cur && wiki->cur->type == BlockTable) {
		WikiBlockTable*  obj = (WikiBlockTable*)wiki->cur;
		obj->cont = true;
		addLine_body (m[4].first, e);
	    } else {
		assert (0);
	    }
	    return CloseTrue;
	}
    }
    return CloseFalse;
}

void  WikiBlockTable::output (MotorOutput* out) {
    normalize ();
    outputTableTag (out);
    outputTBody (out);
    out->out_raw (CharConst ("</table>\n"));
}

void  WikiBlockTable::addLine_head (uiterator b, uiterator e) {
    WikiMotor  motor (b, e, wiki);
    WikiMotorObjVec  objv;
    WikiMotorObjVecVec  objvv;
    int  i;
    TableCell*  cell;

    motor.compile (objv);
    objv.splitCharA ('|', objvv);
    if (objvv.size () > 0 && objvv.back ()->size () == 0)
	objvv.pop_back ();
    if (objvv.size () > 0)
	attrib.shiftAttrib (objvv[0]);

    cell = new TableCell (wiki, WikiAttrib1::M_NORMAL);
    if (objvv.size () > 0)
	cell->cellBody (objvv[0].get (), this, 0);
    captionHtml = cell->html;
    delete cell;

    for (i = 1; i < objvv.size (); i ++) {
	cell = new TableCell (wiki, WikiAttrib1::M_ATTRIB);
	cell->cellattrib.shiftAttrib (objvv[i]);
	defaultList.push_back (cell);
    }
}

void  WikiBlockTable::addLine_body (uiterator b, uiterator e) {
    CellList_t*  cols;
    bool  fmorecell = false;
    umatch  m;
#ifdef WIKITABLECOMPATFLAG
    static uregex  re ("(!([1-9][0-9]*))?(\\\\|&)$");
#else
    static uregex  re ("(!([1-9][0-9]*))?(&)$");
#endif

    if (cont) {
	cont = false;
	cols = &ary.back ();
    } else {
	cols = new CellList_t (wiki);
	ary.push_back (cols);
    }

    if (usearch (b, e, m, re)) { // 
	e = m[0].first;
	if (m[2].matched) {	// ...!NUM\         .
	    int  v = strtoul (m[2].first);
	    ccont ++;
	    if (ccont >= v) {
		cont = false;
		ccont = 0;
	    } else {
		cont = true;
	    }
	} else {		// ...\             .
	    cont = true;
	}
	if (b < e && e[-1] == '|') { // ...|\       .
	    fmorecell = true;
	}
    } else {
	ccont = 0;
    }

    {
	WikiMotor  motor (b, e, wiki);
	WikiMotorObjVec  objv;
	WikiMotorObjVecVec  objvv;
	TableCell*  cell;
	int  i, n;
	bool  fop;
	static const ustring  brabra (CharConst ("{{"));

	motor.compile (objv);
	objv.splitCharA ('|', objvv);
	if (objvv.size () > 0 && objvv.back ()->size () == 0) {
	    objvv.pop_back ();
	    fop = false;
	} else {
	    fop = true;
	}
	n = objvv.size () - 1;
	i = 0;
	if (i <= n && cols->size () == 0
	    && objvv[i]->matchHead (CharConst ("row:"))) {
	    cols->rowattrib.skipAttrib (objvv[i]);
	    cols->rowattrib.shiftAttrib (objvv[i]);
	    i = 1;
	}
	for (; i <= n; i ++) {
	    if (attrib.fturn)
		cell = newCell (cols->rowattrib);
	    else
		cell = newCell (cols->size ());
	    if (! cell->cellattrib.shiftAttrib (objvv[i])) {
		delete cell;
		return;
	    }
	    if (i == n && fop && objvv[i]->match (brabra)) {
		cols->push_back (cell);
		wiki->push_block (&cell->block);
		return;
	    } else {
		objvv[i]->trimSpc ();
		cell->cellBody (objvv[i].get (), this, cols->size ());
		cols->push_back (cell);
	    }
	}
	if (fmorecell) {
	    if (attrib.fturn)
		cell = newCell (cols->rowattrib);
	    else
		cell = newCell (cols->size ());
	    cols->push_back (cell);
	}
    }
}

WikiBlockTable::TableCell*  WikiBlockTable::newCell (int idx) {
    TableCell*  ans = new TableCell (wiki, WikiAttrib1::M_NORMAL);

    if (idx < defaultList.size ())
	ans->cellattrib.copyFrom (defaultList[idx].cellattrib);
    return ans;
}

WikiBlockTable::TableCell*  WikiBlockTable::newCell (WikiAttribTable& rowattrib) {
    TableCell*  ans = new TableCell (wiki, WikiAttrib1::M_NORMAL);

    ans->cellattrib.copyFrom (rowattrib);
    return ans;
}

void  WikiBlockTable::normalize () {
    int  maxcol = 0;
    int  i, j;
    CellList_t*  col;
    TableCell*  cell;

    for (i = 0; i < ary.size (); i ++) {
	col = &ary[i];
	if (col->size () > maxcol)
	    maxcol = col->size ();
    }
    if (attrib.fpadding) {
	for (i = 0; i < ary.size (); i ++) {
	    col = &ary[i];
	    if (col->size () < maxcol) {
		if (attrib.fturn)
		    cell = newCell (col->rowattrib);
		else
		    cell = newCell (col->size ());
		cell->colspan = maxcol - col->size ();
		col->push_back (cell);
		while (col->size () < maxcol) {
		    if (attrib.fturn)
			cell = newCell (col->rowattrib);
		    else
			cell = newCell (col->size ());
		    cell->colspan = 0;
		    col->push_back (cell);
		}
	    }
	    for (j = 0; j < col->size (); ++ j) {
		cell = &(*col)[j];
		if (cell->colspan >= 2) {
		    cell->cellattrib.width = uEmpty;
		}
	    }
	}
    } else {			// expand
	for (i = 0; i < ary.size (); i ++) {
	    col = &ary[i];
	    for (j = col->size () - 1; j >= 0; j --) {
		cell = &(*col)[j];
		if (cell->colspan > 0) {
		    cell->colspan += maxcol - col->size ();
		    while (col->size () < maxcol) {
			if (attrib.fturn)
			    cell = newCell (col->rowattrib);
			else
			    cell = newCell (col->size ());
			cell->colspan = 0;
			col->push_back (cell);
		    }
		    break;
		}
	    }
	    for (j = 0; j < col->size (); ++ j) {
		cell = &(*col)[j];
		if (cell->colspan >= 2) {
		    cell->cellattrib.width = uEmpty;
		}
	    }
	}
    }
}

void  WikiBlockTable::outputTableTag (MotorOutput* out) {
    int  i;

#ifdef WIKITABLEATTRIB
    if (attrib.cellspacing < 0)
	attrib.cellspacing = 0;
    if (attrib.cellpadding < 0)
	attrib.cellpadding = 0;
#endif
    out->out_raw (CharConst ("<table"));
    attrib.output (out);
    out->out_raw (CharConst (">\n"));
    if (captionHtml.length () > 0) {
	out->out_raw (CharConst ("<caption>"))->out_toText (captionHtml)->out_raw (CharConst ("</caption>\n"));
    }
}

void  WikiBlockTable::outputTBody (MotorOutput* out) {
    int  i, j, n;
    CellList_t*  col;
    TableCell*  cell;

    if (attrib.fturn) {
	if (ary.size () > 0) {
	    n = ary[0].size ();
	    for (j = 0; j < n; j ++) {
		out->out_raw (CharConst ("<tr"));
		if (defaultList.size () > j) {
		    defaultList[j].cellattrib.output (out);
		}
		out->out_raw (CharConst (">\n"));
		for (i = 0; i < ary.size (); i ++) {
		    col = &ary[i];
		    cell = &(*col)[j];
		    outputTBodyCell (wiki, out, cell);
		}
		out->out_raw (CharConst ("</tr>\n"));
	    }
	}
    } else {
	for (i = 0; i < ary.size (); i ++) {
	    col = &ary[i];
	    out->out_raw (CharConst ("<tr"));
	    col->rowattrib.output (out);
	    out->out_raw (CharConst (">\n"));
	    for (j = 0; j < col->size (); j ++) {
		cell = &(*col)[j];
		outputTBodyCell (wiki, out, cell);
	    }
	    out->out_raw (CharConst ("</tr>\n"));
	}
    }
}

void  WikiBlockTable::outputTBodyCell (WikiFormat* wiki, MotorOutput* out, TableCell* cell) {
    if (cell->colspan > 0 && cell->rowspan > 0) {
	cell->outputTD (wiki, out, attrib.fturn);
	if (cell->block.size () > 0) {
	    if (cell->block.size () == 1 && cell->block[0].type == BlockParagraph) {
		WikiBlockParagraph*  b = WikiBlockParagraph_type (&cell->block[0]);
		if (! b->pflag) {
		    b->singleTag = true;
		}
	    }
	    wiki->output (cell->block);
	} else {
	    if (cell->html.size () > 0) {
		out->out_toText (cell->html);
	    } else {
		if (! attrib.fnowhite)
		    out->out_raw (uNbsp);
	    }
	}
	cell->outputTDe (out);
    }
}

/* ============================================================ */
/*DOC:
===selectタグ===
 |select:NAME:function:default:size=NUM:multiple|DEFAULTVALUE|
 |VALUE|
 |LABEL|VALUE|selected|

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
|'''size='''''Integer''||size属性。|
|'''multiple'''||multiple属性。|

*/
bool  WikiBlockSelect::nextLine (uiterator b, uiterator e) {
    if (*b == kWikiTABLE) {
	addLine (b, e);
	return true;
    } else {
	close ();
	return false;
    }
}

void  WikiBlockSelect::addLine (uiterator b, uiterator e) {
    assert (b[0] == '|');
    b ++;
    if (n == 0 && matchSkip (b, e, CharConst ("select:"))) {
	addLine_head (b, e);
    } else {
	addLine_body (b, e);
    }
    n ++;
}

void  WikiBlockSelect::addLine_head (uiterator b, uiterator e) {
    WikiMotor  motor (b, e, wiki);
    WikiMotorObjVec  objv;
//    WikiMotorObjVecVec  objvv; //-> head
    WikiMotorObjVec*  vcell;
    bool  ferr;

    motor.compile (objv);
    objv.splitCharA ('|', head);
    if (head.size () > 0 && head.back ()->size () == 0)
	head.pop_back ();
    if (head.size () > 0) {
	attrib.shiftName (head[0], name);
	attrib.shiftAttrib (head[0]);
    }
    if (attrib.fmultiple && attrib.elsize == 0) {
	attrib.elsize = 1;
    }
}

void  WikiBlockSelect::addLine_body (uiterator b, uiterator e) {
    WikiMotor  motor (b, e, wiki);
    WikiMotorObjVec  objv;
    WikiMotorObjVecVec  objvv;
    SelectItem  v;

    motor.compile (objv);
    objv.splitCharA ('|', objvv);
    if (objvv.size () > 0 && objvv.back ()->size () == 0)
	objvv.pop_back ();
    if (objvv.size () > 0) {
	v.label = omitCtrl (objvv[0].get ()->textOut (wiki));
    } else {
	v.label.resize (0);
    }
    if (objvv.size () > 1) {
	v.value = omitCtrl (objvv[1].get ()->textOut (wiki));
	v.fvalue = true;
    } else {
	v.value.resize (0);
	v.fvalue = false;
    }
    v.fselect = false;
    if (objvv.size () > 2) {
//	if (objvv[2].get ()->match (CharConst ("selected"), CharConst ("sel"))) {
	ustring  kw = objvv[2].get ()->textOut (wiki);
	if (match (kw, CharConst ("selected")) || match (kw, CharConst ("sel"))) {
	    v.fselect = true;
	}
    }
    item.push_back (v);
}

void  WikiBlockSelect::close () {
    wiki->cur = NULL;
    wiki->pop_block ();
    assert (wiki->cur->type == BlockParagraph);

    MotorOutputString  out;
    output (&out);
    wiki->cur->addHtml (out.ans);
    delete this;
}

void  WikiBlockSelect::output (MotorOutput* out) {
    int  i;
    ustring  u;

    if (attrib.elsize == 1)
	attrib.elsize = item.size ();
    out->out_raw (CharConst ("<select"));
    wiki->outputName (out, CharConst ("name"), name, false);
    attrib.output (out);
    if (attrib.script.length () > 0)
	wiki->outputName (out, CharConst ("onChange"), attrib.script);
    out->out_raw (CharConst (">\n"));

    if (attrib.fdefault) {
	u = wiki->getVar_string (name);
    } else if (head.size () > 1) {
	u = head[1]->textOut (wiki);
    }
#ifdef DEBUG2
    std::cerr << "u:" << u << uLF;
#endif /* DEBUG */

    for (i = 0; i < item.size (); i ++) {
	SelectItem*  v = &item[i];
	out->out_raw (CharConst ("<option"));
	if (v->fvalue) {
	    wiki->outputName (out, CharConst ("value"), v->value, false);
	    if (v->fselect || (attrib.fdefault && u == v->value) || (head.size () > 1 && u == v->value))
		out->out_raw (CharConst (" selected=\"selected\""));
	} else {
	    if (v->fselect || (attrib.fdefault && u == v->label) || (head.size () > 1 && u == v->label))
		out->out_raw (CharConst (" selected=\"selected\""));
	}
	out->out_raw (CharConst (">"));
	out->out_toHTML (v->label)->out_raw (CharConst ("</option>\n"));
    }
    out->out_raw (CharConst ("</select>\n"));
}

/* ============================================================ */
/*DOC:
===引用===
 >
 この部分は，引用です。
 <

*/
bool  WikiBlockQuote::nextLine (uiterator b, uiterator e) {
    return false;
}

WikiBlock::closeType  WikiBlockQuote::closeLine (uiterator b, uiterator e) {
    if (e - b == 1 && *b == kWikiQUOTE_e) {
	wiki->pop_block ();
	wiki->cur = NULL;
	return CloseTrue;
    } else {
	return CloseFalse;
    }
}

void  WikiBlockQuote::addLine (uiterator b, uiterator e) {
    // do nothing
}

void  WikiBlockQuote::output (MotorOutput* out) {
    out->out_raw (CharConst ("<blockquote>\n"));
    outputBlock (out);
    out->out_raw (CharConst ("</blockquote>\n"));
}

/* ============================================================ */
/*DOC:
===DIVタグ===
 {div:''ClassName''
 ...
 }
 {div:id=''ID'':''ClassName'':onclick=''LinkFunction''
 ...
 }

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
|''ClassList''||class=は省略できる。|
|'''#nop'''||divタグを出力しない。pタグを抑制する。|

*/
bool  WikiBlockDiv::nextLine (uiterator b, uiterator e) {
    return false;
}

void  WikiBlockDiv::addLine (uiterator b, uiterator e) {
    bool  rc = matchSkip (b, e, CharConst (uWikiDIV));	// {div:
    assert (rc);
    WikiMotor  motor (b, e, wiki);
    WikiMotorObjVecPtr  vec (new WikiMotorObjVec);

    motor.compile (*vec);
    attrib.shiftAttrib (vec);
    {
	std::vector<ustring>::const_iterator  b = attrib.directlist.begin ();
	std::vector<ustring>::const_iterator  e = attrib.directlist.end ();
	for (; b < e; ++ b) {
	    if (match (*b, CharConst ("#nop"))) {
		nopflag = true;
	    } else {
		wiki->errorMsg.append (*b).append (CharConst (": bad attribute.\n"));
	    }
	}
    }
}

WikiBlock::closeType  WikiBlockDiv::closeLine (uiterator b, uiterator e) {
    if (e - b == 1 && *b == kWikiDIV_e) {
	wiki->pop_block ();
	wiki->cur = NULL;
	return CloseTrue;
    } else {
	return CloseFalse;
    }
}

void  WikiBlockDiv::output (MotorOutput* out) {
    bool  nonl = false;

    if (nopflag) {
	boost::ptr_vector<WikiBlock>::iterator  b = block.begin ();
	boost::ptr_vector<WikiBlock>::iterator  e = block.end ();
	for (; b < e; ++ b) {
	    if (b->type == BlockParagraph) {
		WikiBlockParagraph*  p = WikiBlockParagraph_type (&(*b));
		if (! p->pflag) {
		    p->singleTag = true;
		    nonl = true;
		}
	    }
	}
    } else {
	if (block.size () == 1 && block[0].type == BlockParagraph) {
	    WikiBlockParagraph*  b = WikiBlockParagraph_type (&block[0]);
	    if (! b->pflag) {
		b->singleTag = true;
		nonl = true;
	    }
	}
	out->out_raw (CharConst ("<div"));
	attrib.output (out);
	if (nonl)
	    out->out_raw (CharConst (">"));
	else
	    out->out_raw (CharConst (">\n"));
    }
    outputBlock (out);
    if (! nopflag) {
	out->out_raw (CharConst ("</div>\n"));
    } else {
	out->out_raw (uLF);
    }
}

/* ============================================================ */
/*DOC:
===FORMタグ===
 {form:POST:''URL''
 ...
 }
 {form:POST:''URL'':target=''TargetWindow'':id=''ID'':class=''ClassName''
 ...
 }

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

*/
bool  WikiBlockForm::nextLine (uiterator b, uiterator e) {
    return false;
}

void  WikiBlockForm::addLine (uiterator b, uiterator e) {
//    bool  rc = matchSkip (b, e, CharConst ("{form:"));
    bool  rc = matchSkip (b, e, CharConst (uWikiFORM));
    assert (rc);
    WikiMotor  motor (b, e, wiki);
    WikiMotorObjVecPtr  vec (new WikiMotorObjVec);
    ustring  key;
    WikiMotorObjVecPtr  v2 (new WikiMotorObjVec);

    motor.compile (*vec);
    vec->splitChar_keyword (':', key, *v2);
    if (match (key, CharConst ("get"), CharConst ("GET"))) {
	method = M_GET;
	vec = v2;
    } else if (match (key, CharConst ("post"), CharConst ("POST"))) {
	method = M_POST;
	vec = v2;
    }
    if (vec->size () > 0) {
	attrib.shiftLink (vec, url, fscript);
    }
    attrib.shiftAttrib (vec);
}

WikiBlock::closeType  WikiBlockForm::closeLine (uiterator b, uiterator e) {
    if (e - b == 1 && *b == kWikiDIV_e) {
	wiki->pop_block ();
	wiki->cur = NULL;
	wiki->curform = NULL;
	return CloseTrue;
    } else {
	return CloseFalse;
    }
}

void  WikiBlockForm::output (MotorOutput* out) {
    out->out_raw (CharConst ("<form"));
    switch (method) {
    case M_GET:
	out->out_raw (CharConst (" method=\"get\""));
	break;
    case M_NONE:
    case M_POST:
	out->out_raw (CharConst (" method=\"post\""));
	break;
    default:
	assert (0);
    }
    if (fscript) {
	wiki->outputName (out, CharConst ("action"), ustring (CharConst ("#")), false);
	wiki->outputSubmitScript (out, CharConst ("onsubmit"), url, true);
    } else {
	wiki->outputName (out, CharConst ("action"), url, false);
    }
    if (qfileform)
	out->out_raw (CharConst (" enctype=\"multipart/form-data\""));
    attrib.output (out);
    out->out_raw (CharConst (">\n"));
    outputBlock (out);
    out->out_raw (CharConst ("</form>\n"));
}

/* ============================================================ */
/*DOC:
===Elementブロック===
 {element:''Name''
 ...
 }

Wikiテキストの名前付きブロック。

*/
bool  WikiBlockElement::nextLine (uiterator b, uiterator e) {
    return false;
}

void  WikiBlockElement::addLine (uiterator b, uiterator e) {
    bool  rc = matchSkip (b, e, CharConst (uWikiELEMENT));
    assert (rc);
    WikiMotor  motor (b, e, wiki);
    WikiMotorObjVecPtr  vec (new WikiMotorObjVec);
    WikiMotorObjVecPtr  v2 (new WikiMotorObjVec);

    motor.compile (*vec);
    vec->splitChar_keyword (':', name, *v2);
    if (name.size () == 0) {
	wiki->errorMsg.append (CharConst ("missing name in the element block."));
    }
    if (v2->size () > 0) {
	wiki->errorMsg.append (v2->dump ()).append (CharConst (": bad attribute.\n"));
    }
    if (name.size () > 0) {
	wiki->elementMap.put (name, this);
    }
}

WikiBlock::closeType  WikiBlockElement::closeLine (uiterator b, uiterator e) {
    if (e - b == 1 && *b == kWikiDIV_e) {
	wiki->pop_block ();
	wiki->cur = NULL;
	return CloseTrue;
    } else {
	return CloseFalse;
    }
}

void  WikiBlockElement::output (MotorOutput* out) {
//***    outputBlock (out);
}

/* ============================================================ */
/*DOC:
===水平線===
 ----

*/
bool  WikiBlockHR::nextLine (uiterator b, uiterator e) {
    return false;
}

void  WikiBlockHR::addLine (uiterator b, uiterator e) {
    // do nothing
}

void  WikiBlockHR::output (MotorOutput* out) {
    out->out_raw (CharConst (uHR));
}

/* ============================================================ */
bool  WikiBlockRaw::nextLine (uiterator b, uiterator e) {
    return false;
}

void  WikiBlockRaw::addLine (uiterator b, uiterator e) {
    text.assign (b, e);
}

void  WikiBlockRaw::output (MotorOutput* out) {
    out->out_toText (text);
}

/* ============================================================ */
bool  WikiBlockBlank::nextLine (uiterator b, uiterator e) {
    if (b == e) {
	count ++;
	return true;
    } else {
	switch (*b) {		// paragraph以外の全て
	case kWikiH:
	case kWikiPRE:
	case kWikiPRE2:
	case kWikiUL:
	case kWikiOL:
	case kWikiNL:
	case kWikiDL:
	case kWikiTABLE:
	    count = 0;
	    break;
	default:
	    if ((*b == kWikiQUOTE && e - b == 1) ||
		matchHead (b, e, CharConst (uWikiDIV)) ||
		(wiki->curform == NULL && matchHead (b, e, CharConst (uWikiFORM))) ||
		(wiki->curform == NULL && matchHead (b, e, CharConst (uWikiELEMENT))) ||
		matchHead (b, e, CharConst ("----"))) {
		count = 0;
	    } else {
		if (count >= 2)
		    enable = true;
	    }
	}
	return false;
    }
}

void  WikiBlockBlank::addLine (uiterator b, uiterator e) {
    count ++;
}

void  WikiBlockBlank::output (MotorOutput* out) {
    if (enable) {
	out->out_raw (CharConst ("<div class=\"pggap\"></div>\n"));
    }
}

/* ============================================================ */
void  WikiFormat::pass1 (const ustring& text, WikiLine::linevec* block, bool fsuper) {
    SplitterNL  sp (text);

    if (matchHead (text, CharConst ("$$"))) {
	if (sp.next ()) {
	    uiterator  b = sp.begin () + 2; // ""$$""をスキップ
	    uiterator  e = sp.end ();
	    // 複数キーワードに対応するには、コンマ区切りにする
	    if (match (b, e, CharConst ("normal-mode")))
		fsuper = false;
	}
    }

    pass1_1 (sp, NULL, NULL, block, NULL, NULL, NULL, fsuper);
}

static bool  findCmdSep (uiterator& b, uiterator e, uiterator& u) {
    int  c;
    uiterator  p = b;
    for (; p < e; ++ p) {
	c = *p;
	if (c == ':') {
	    b = p;
	    u = b + 1;
	    return true;
	} else if (c == ' ' || c == '\t') {
	    b = p;
	    u = b;
	    do {
		++ u;
	    } while (u < e && ((c = *u) == ' ' || c == '\t'));
	    return true;
	}
    }
    b = p;
    u = e;
    return true;
}

int  WikiFormat::pass1_1 (Splitter& sp, ustring* elseword, ustring* endword, WikiLine::linevec* block, uiterator* elsebegin0, uiterator* elsebegin, uiterator* elseend, bool fsuper) {
    uiterator  b, e, t, u, v;

    while (sp.next ()) {
	b = sp.begin ();
	e = sp.end ();
	while (b < e && b[0] == '\t')	// TABを無視
	    b ++;
	if (matchSkip (b, e, CharConst (kComment))) {
	    // comment
	} else if (b != e && b[0] == kWikiCmd) {
	    t = b;
	    u = b;
	    if (findCmdSep (u, e, v)) {
	    } else {
		u = e;
		v = e;
	    }
	    if (endword && match (t, u, *endword)) { 
		return 2;	// endword
	    } else if (elseword && match (t, u, *elseword)) {
		if (elsebegin0)
		    *elsebegin0 = t;
		if (elsebegin)
		    *elsebegin = v;
		if (elseend)
		    *elseend = e;
		return 1;	// elseword
	    } else if (pass1_2 (sp, b, e, t, u, v, block, fsuper)) {
	    } else {
		WikiLine*  wl;
		block->push_back (wl = new WikiLine (b, e, fsuper));
		wl->fn = wc_call_defun;
	    }
	} else {
	    block->push_back (new WikiLine (b, e, fsuper));
	}
    }
    return 0;			// end of line
}

bool  WikiFormat::pass1_2 (Splitter& sp, uiterator& b, uiterator& e, uiterator& t, uiterator& u, uiterator& v, WikiLine::linevec* block, bool fsuper) {
    WikiCmdTable::iterator  it;
    ustring  elseword, endword;
    WikiLine*  wl;
    WikiLine*  wl2;
    int  rc;

    if ((it = GWikiCmdTable.find (ustring (t, u))) != GWikiCmdTable.end ()) {
	block->push_back (wl = new WikiLine (b, v, e, fsuper));
	wl->fn = it->second->fn;
	if (it->second->endname) {
	    endword.assign (it->second->endname, it->second->endnamelen);
	    if (it->second->elsename) {
		elseword.assign (it->second->elsename, it->second->elsenamelen);
		// if-then-else block
		wl->block = new WikiLine::linevec;
		rc = pass1_1 (sp, &elseword, &endword, wl->block, &b, &v, &e, fsuper);
		switch (rc) {
		case 0:	// eot
			// no end line
		    if (endword.length () > 0) {
			errorMsg.append (CharConst ("no matcing \"")).append (endword).append (CharConst ("\".\n"));
		    }
		    break;
		case 1:	// else
		    do {
			wl2 = new WikiLine (b, v, e, fsuper);
			wl2->fn = wl->fn;
			wl->block2 = wl2;
			wl = wl2;
			wl->block = new WikiLine::linevec;
			rc = pass1_1 (sp, &elseword, &endword, wl->block, &b, &v, &e, fsuper);
			if (rc == 0) {
			    // no end line
			    if (endword.length () > 0) {
				errorMsg.append (CharConst ("no matcing \"")).append (endword).append (CharConst ("\".\n"));
			    }
			} else if (rc == 2) {
			    assert (wl->block2 == NULL);
			    wl->block2 = new WikiLine (sp.begin (), sp.end (), false);
			}
		    } while (rc == 1);
		    break;
		case 2:	// end
		    assert (wl->block2 == NULL);
		    wl->block2 = new WikiLine (sp.begin (), sp.end (), false);
		    break;
		default:
		    assert (0);
		}
	    } else {
		// block
		wl->block = new WikiLine::linevec;
		rc = pass1_1 (sp, NULL, &endword, wl->block, NULL, NULL, NULL, fsuper);
		if (rc == 0) {
		    // no end line error
		    if (endword.length () > 0) {
			errorMsg.append (CharConst ("no matcing \"")).append (endword).append (CharConst ("\".\n"));
		    }
		} else if (rc == 2) {
		    assert (wl->block2 == NULL);
		    wl->block2 = new WikiLine (sp.begin (), sp.end (), false);
		}
	    }
	} else {
	    // nonblock
	}
	if (it->second->fn_pass1)
	    it->second->fn_pass1 (wl, this);

	return true;
    } else if (t < u && *t == kWikiCmd) {
	bool  fx = true;
	MNode*  wf;
	ustring  name (t + 1, u);			// '$'を取り除く
//	if (protectMode && ! fsuper && ! env->wikienv->wikiGuestFunc.get (name))
//	    fx = false;
//	if (fx && (wf = env->wikienv->wikiCmd2.getVar (name))) { // Wiki Command
	if ((wf = env->wikienv->wikiCmd2.getVar (name)) && (fsuper || ! isLambda (wf) || wf->lambdaModeBit (MNode::MODE_WIKIPUBLIC))) { // Wiki Command
	    ustring  endword;
	    endword.assign (CharConst (uWikiCmd "end"));		// $end
	    block->push_back (wl = new WikiLine (b, e, fsuper));
	    wl->fn = wc_call_defun;
	    wl->block = new WikiLine::linevec;
	    rc = pass1_1 (sp, NULL, &endword, wl->block, NULL, NULL, NULL, fsuper);
	    if (rc == 0) {
		// no end line error
		if (endword.length () > 0) {
		    errorMsg.append (CharConst ("no matching \"")).append (endword).append (CharConst ("\".\n"));
		}
	    } else if (rc == 2) {
		assert (wl->block2 == NULL);
		wl->block2 = new WikiLine (sp.begin (), sp.end (), false);
	    }
	    return true;
	}
    }

    return false;
}

#ifdef DEBUG2
static void  dump (WikiLine::linevec* block, int indent = 0) {
    int  i, j;
    if (! block) {
	for (j = 0; j < indent; j ++) std::cerr << "	";
	std::cerr << "(NULL)\n";
	return;
    }
    for (i = 0; i < block->size (); i ++) {
	WikiLine*  wl = &(*block)[i];
	for (j = 0; j < indent; j ++) std::cerr << "	";
	if (wl->fn)
	    std::cerr << (void*)wl->fn << ":";
	std::cerr << ustring (wl->begin, wl->end) << uLF;
	while (1) {
	    if (wl->block) {
		dump (wl->block, indent + 1);
	    }
	    if (wl->block2) {
		wl = wl->block2;
		for (j = 0; j < indent; j ++) std::cerr << "	";
		std::cerr << "else:" << (void*)wl->fn << ":" << ustring (wl->begin, wl->end) << uLF;
		//	    dump (wl->block, indent + 1);
	    } else {
		break;
	    }
	}
    }
}
#endif /* DEBUG */

void  WikiFormat::compile (const ustring& text, bool fsuper) {
    try {
	WikiLine::linevec  block;
	pass1 (text, &block, fsuper);
#ifdef DEBUG2
	dump (&block);
#endif /* DEBUG */

	{
	    WikiLineScanner  scanner (&block);
	    compileLines (scanner);
	    while (1) {
		if (cur)
		    cur->close ();
		cur = NULL;
		if (bstack.size () > 0)
		    pop_block ();
		else
		    break;
	    }
	    cur = NULL;
	}
    } catch (ustring& msg) {
	logLispFunctionError (msg, uEmpty);
    }
}

void  WikiFormat::compileElement (const ustring& text, bool fsuper, const ustring& elementName) {
    try {
	WikiLine::linevec  block;
	pass1 (text, &block, fsuper);
#ifdef DEBUG2
	dump (&block);
#endif /* DEBUG */

	{
	    WikiLine*  wl;
	    wl = env->wikienv->wikiElement.getVar (elementName);
	    if (wl && wl->block) {
#ifdef DEBUG
		std::cerr << "(wiki):$element:" << elementName << "\n";
#endif /* DEBUG */
		WikiLineScanner  scanner (wl->block);
		compileLines (scanner);
		while (1) {
		    if (cur)
			cur->close ();
		    cur = NULL;
		    if (bstack.size () > 0)
			pop_block ();
		    else
			break;
		}
		cur = NULL;
	    }
	}
    } catch (ustring& msg) {
	logLispFunctionError (msg, uEmpty);
    }
}

void  WikiFormat::outputElement (const ustring& elementName) {
    WikiBlockElement*  b;
    
    b = elementMap.get (elementName);
    if (b) {
	output (b->block);
    }
}

void  WikiFormat::output (boost::ptr_vector<WikiBlock>& ary) {
    int  i;

    for (i = 0; i < ary.size (); i ++) {
	ary[i].output (env->output);
    }
    errorOutput ();
}

void  WikiFormat::errorOutput () {
    if (errorMsg.size () > 0) {
	env->output->out_raw (CharConst (uP));
	env->output->out_toHTML_br (errorMsg);
	env->output->out_raw (CharConst (uPe));
	errorMsg.resize (0);
    }
}

bool  WikiFormat::checkClose (uiterator b, uiterator e) {
    int  n = bstack.size ();
    WikiBlock::closeType  rc;

    while (n > 0) {
	n --;
	rc = bstack[n]->closeLine (b, e);
	switch (rc) {
	case WikiBlock::CloseTrue:
	    return true;
	case WikiBlock::CloseFalse:
	    return false;
	default:;
	}
    }
    return false;
}

void  WikiFormat::compileLine (WikiLineScanner& scanner) {
    WikiLine*  wl = scanner.cur ();
    if (! wl)
	return;
    if (wl->fn) {
	wl->fn (wl, this);
    } else {
	uiterator  b = wl->begin;
	uiterator  e = wl->end;

	if (b == e) {		// empty line
	    if (cur) {
		switch (cur->type) {
		case WikiBlock::BlockParagraph:
		    cur->close ();
		    cur = new WikiBlockBlank (this);
		    blockp->push_back (cur);
		    cur->addLine (b, e);
		    break;
		case WikiBlock::BlockBlank:
		    cur->addLine (b, e);
		    break;
		default:
		    cur->close ();
		    cur = NULL;
		}
	    }
	} else if (matchSkip (b, e, CharConst ("//"))) {	// comment
	} else if (checkClose (b, e)) {
	} else if (cur && cur->nextLine (b, e)) {
	} else if (b[0] == kWikiP) { // ^
	    if (cur)
		cur->close ();
	    cur = new WikiBlockParagraph (this);
	    blockp->push_back (cur);
	    cur->addLine (b, e);
	} else if (b[0] == kWikiH) {	// =
	    WikiBlockH*  obj;
	    if (cur)
		cur->close ();
	    cur = obj = new WikiBlockH (this);
	    blockp->push_back (cur);
	    cur->addLine (b, e);
	    if (obj->checkEmpty ()) {
		cur->close ();
		cur = NULL;
	    } else {
		push_block (&obj->block);
	    }
	} else if (b[0] == kWikiPRE	// SPC
		   || b[0] == kWikiPRE2) {	// TAB
	    if (cur)
		cur->close ();
	    cur = new WikiBlockPreformatRaw (this);
	    blockp->push_back (cur);
	    cur->addLine (b, e);
	} else if (b[0] == kWikiUL) {	// *
	    WikiBlockItem*  obj;
	    if (cur)
		cur->close ();
	    cur = obj = new WikiBlockItem (WikiBlock::BlockItemUL, this);
	    blockp->push_back (cur);
	    obj->addLine (b, e);
	} else if (b[0] == kWikiOL) {	// #
	    WikiBlockItem*  obj;
	    if (cur)
		cur->close ();
	    cur = obj = new WikiBlockItem (WikiBlock::BlockItemOL, this);
	    blockp->push_back (cur);
	    obj->addLine (b, e);
	} else if (b[0] == kWikiNL) {	// +
	    WikiBlockItem*  obj;
	    if (cur)
		cur->close ();
	    cur = obj = new WikiBlockItem (WikiBlock::BlockItemNL, this);
	    blockp->push_back (cur);
	    obj->addLine (b, e);
	} else if (b[0] == kWikiDL) {	// ;
	    if (cur)
		cur->close ();
	    cur = new WikiBlockItemDL (this);
	    blockp->push_back (cur);
	    cur->addLine (b, e);
	} else if (matchHead (b, e, CharConst ("|select:"))) {	// |select:
	    if (cur && cur->type == WikiBlock::BlockParagraph) {
	    } else {
		if (cur)
		    cur->close ();
		cur = new WikiBlockParagraph (this);
		blockp->push_back (cur);
	    }
	    push_block (blockp);
	    cur = new WikiBlockSelect (this);
	    cur->addLine (b, e);
	} else if (b[0] == kWikiTABLE) {	// |
	    WikiBlockTable*  obj;
	    if (cur)
		cur->close ();
	    cur = obj = new WikiBlockTable (this);
	    blockp->push_back (cur);
	    cur->addLine (b, e);
	} else if (b[0] == kWikiQUOTE && e - b == 1) {	// >
	    WikiBlockQuote*  obj;
	    if (cur)
		cur->close ();
	    cur = obj = new WikiBlockQuote (this);
	    blockp->push_back (cur);
	    push_block (&obj->block);
//	} else if (matchHead (b, e, CharConst ("{div:"))) {
	} else if (matchHead (b, e, CharConst (uWikiDIV))) {
	    WikiBlockComplex*  obj;
	    if (cur)
		cur->close ();
	    cur = obj = new WikiBlockDiv (this);
	    blockp->push_back (cur);
	    cur->addLine (b, e);
	    push_block (&obj->block);
	} else if (curform == NULL && matchHead (b, e, CharConst (uWikiFORM))) {
	    WikiBlockComplex*  obj;
	    if (cur)
		cur->close ();
	    cur = obj = curform = new WikiBlockForm (this);
	    blockp->push_back (cur);
	    cur->addLine (b, e);
	    push_block (&obj->block);
	} else if (curform == NULL && matchHead (b, e, CharConst (uWikiELEMENT))) {
	    WikiBlockComplex*  obj;
	    if (cur)
		cur->close ();
	    cur = obj = new WikiBlockElement (this);
	    blockp->push_back (cur);
	    cur->addLine (b, e);
	    push_block (&obj->block);
	} else if (matchHead (b, e, CharConst ("----"))) { // ----
	    if (cur)
		cur->close ();
	    cur = new WikiBlockHR (this);
	    blockp->push_back (cur);
	    cur->addLine (b, e);
	} else {
	Bp1:
	    if (cur && cur->type != WikiBlock::BlockParagraph) {
		cur->close ();
		cur = NULL;
	    }
	    if (! cur) {
		cur = new WikiBlockParagraph (this);
		blockp->push_back (cur);
	    }
	    cur->addLine (b, e);
	}
    }
}

void  WikiFormat::compileLines (WikiLineScanner& scanner, bool (*fn)(WikiLine& spp, WikiLineScanner& scanner, WikiFormat* wiki, void* par), void* par) {
    WikiLine*  wl;

    while ((wl = scanner.next ())) {
	if (fn && fn (*scanner.cur (), scanner, this, par)) {
	    break;
	} else {
	    compileLine (scanner);
	}
    }
}

void  WikiFormat::pushBlockRaw (uiterator b, uiterator e) {
    WikiBlockRaw*  obj;

    obj = new WikiBlockRaw (this);
    blockp->push_back (obj);
    obj->addLine (b, e);
}

int  WikiFormat::countWikiH (uiterator& b, uiterator e) {
    int  ans = 0;

    while (b != e && *b == kWikiH) {
	ans ++;
	b ++;
    }
    if (ans > 6)
	ans = 6;
    return ans;
}

void  WikiFormat::outputName (MotorOutput* out, const char* name, size_t len, const ustring& val, bool cond) {
    if (! cond || val.length () > 0)
	out->out_raw (uSPC)
	    ->out_raw (name, len)
	    ->out_raw (CharConst ("=\""))
	    ->out_toHTML_noCtrl (val)
	    ->out_raw (uQ2);
}

void  WikiFormat::outputName (MotorOutput* out, const char* name, size_t len, long val, bool cond) {
    if (! cond || val > 0)
	out->out_raw (uSPC)
	    ->out_raw (name, len)
	    ->out_raw (CharConst ("=\""))
	    ->out_raw (to_ustring (val))
	    ->out_raw (uQ2);
}

void  WikiFormat::outputStyle (MotorOutput* out, const ustring& style) {
    if (style.length () > 0)
	out->out_raw (CharConst (" style=\""))
	    ->out_raw (style)
	    ->out_raw (uQ2);
}

void  WikiFormat::outputFlag (MotorOutput* out, const char* name, size_t len, bool flag) {
    if (flag)
	out->out_raw (uSPC)
	    ->out_raw (name, len)
	    ->out_raw (CharConst ("=\""))
	    ->out_raw (name, len)
	    ->out_raw (uQ2);
}

void  WikiFormat::outputID (MotorOutput* out, const ustring& id) {
    if (id.length () > 0)
	out->out_raw (CharConst (" id=\""))
	    ->out_toHTML_noCtrl (id)
	    ->out_raw (uQ2);
}

void  WikiFormat::outputClass (MotorOutput* out, std::vector<ustring>& classes) {
    if (classes.size () > 0) {
	out->out_raw (CharConst (" class=\""));
	for (int i = 0; i < classes.size (); i ++) {
	    if (i > 0)
		out->out_raw (uSPC);
	    out->out_toHTML_noCtrl (classes[i]);
	}
	out->out_raw (uQ2);
    }
}

void  WikiFormat::outputSubmitScript (MotorOutput* out, const char* name, size_t len, const ustring& onclick, bool scriptcut) {
    if (onclick.length () > 0) {
	out->out_raw (uSPC)
	    ->out_raw (name, len)
	    ->out_raw (CharConst ("=\""))
	    ->out_toHTML_noCtrl (onclick);
	if (scriptcut)
	    out->out_raw (CharConst ("return false;\""));
	else
	    out->out_raw (CharConst ("\""));
    }
}

void  WikiFormat::outputNum (MotorOutput* out, const char* name, size_t len, int val) {
    out->out_raw (uSPC)
	->out_raw (name, len)
	->out_raw (CharConst ("="))
	->out_raw (to_ustring (val));
}

MNode*  WikiFormat::arrayToTexp (const ustring& name) {
    size_t  i, n;
    MNodeList  e;

    n = mlenv->getArySize (name);
    for (i = 1; i <= n; i ++) {
	e.append (mlenv->getAry (name, i));
    }
    return mlenv->retval = e ();
}

#if 0
MNode*  WikiFormat::evalVar (const ustring& name) {
    // @NAME → getarray
    // NAME → getvar
    ustring  sym;
    if (checkAry (name, sym)) {
	return arrayToTexp (sym);
    } else {
	return mlenv->getVar (name);
    }
}
#endif

void  WikiFormat::wikiMotor (uiterator b, uiterator e, WikiMotorObjVec& ans) {
    WikiMotor  motor (b, e, this);
    AutoInclCount  autoIncl (env->mlenv);

    if (! autoIncl.inc_test ())
	throw (uErrorInclNest);
    motor.compile (ans, WikiMotor::TMATCH_NONE);
}

ustring  WikiFormat::wikiMotor (uiterator b, uiterator e) {
    WikiMotorObjVec  objv;

    wikiMotor (b, e, objv);
    return objv.htmlOut (this);
}

void  WikiFormat::wikiMotorInline (uiterator b, uiterator e, WikiMotorObjVec& ans) {
    WikiMotorObjVec  objv;
    WikiMotor  motor (b, e, this);

    motor.compile (objv, WikiMotor::TMATCH_NONE);
    objv.eval (ans, this);
}

MNode*  WikiFormat::buildArgs (bool fnodec, WikiMotorObjVecVec::const_iterator b, WikiMotorObjVecVec::const_iterator e) {
    MNodeList  ans;
    if (fnodec) {
	for (; b < e; b ++)
	    ans.append (newMNode_str (new ustring ((*b)->dump ())));
    } else {
	for (; b < e; b ++)
	    ans.append ((*b)->toMNode (this));
    }
    return ans.release ();
}

void  WikiFormat::logLispFunctionError (const ustring& msg, const ustring& cmd) {
    if (cmd.length () > 0)
	errorMsg.append (cmd).append (CharConst (": "));
    errorMsg.append (CharConst ("lisp function error.\n"));
    if (mlenv->env->mlenv->currentCell ()) {
	if (mlenv->env->log)
	    *mlenv->env->log << mlenv->env->mlenv->currentCell ()->dump_string_short () << ": ";
	*mlenv->env->log << msg << uLF;
    }
}

/*DOC:
$pre-mode:
*/
/* ============================================================ */
