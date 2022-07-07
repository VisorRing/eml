#include "wikiattrib.h"
#include "wikiformat.h"
#include "wikimotor.h"
#include "util_check.h"
#include "ustring.h"

bool  WikiAttrib1::readAttrib1 (WikiMotorObjVec* cell, bool& rc) {
    ustring  key;
    WikiMotorObjVec  vval;
    bool  ferr = false;

    if (cell->size () == 0) {
	if (mode == M_ATTRIB || mode == M_ATTRIB_TEXT)
	    return true;
	else
	    return false;
    }
    if (cell->splitChar_keyword ('=', key, vval)) {
	if (paramID (key, vval, ferr)) {
	} else if (paramClass (key, vval, ferr)) {
#ifdef BOOTSTRAPHACK
	} else if (paramDataPrefix (key, vval, ferr)) {
#endif
	} else if (paramOnClickCheck (key)) {
	    if (! checkScript (vval, onclick, ferr)) {
		wiki->errorMsg.append (cell->dump ()).append (CharConst (": link script error.\n"));
		ferr = true;
	    }
	} else if (paramOnFocusCheck (key)) {
	    if (! checkScript (vval, onfocus, ferr)) {
		wiki->errorMsg.append (cell->dump ()).append (CharConst (": link script error.\n"));
		ferr = true;
	    }
	} else if (paramOnBlurCheck (key)) {
	    if (! checkScript (vval, onblur, ferr)) {
		wiki->errorMsg.append (cell->dump ()).append (CharConst (": link script error.\n"));
		ferr = true;
	    }
	} else if (paramOnChangeCheck (key)) {
	    if (! checkScript (vval, onchange, ferr)) {
		wiki->errorMsg.append (cell->dump ()).append (CharConst (": link script error.\n"));
		ferr = true;
	    }
	} else if ((selector & SEL_TARGET) && paramTargetCheck (key)) {
	    paramTargetBody (key, vval.textOut (wiki), target, ferr);
	} else if (readAttribMore (key, vval, ferr)) {
	} else {
	    if (mode == M_ATTRIB) {
		wiki->errorMsg.append (cell->dump ()).append (CharConst (": bad attribute.\n"));
		rc = false;
	    }
	    return false;
	}
    } else {
	if (selector & SEL_CLASS2) {
	    paramClassValue (*cell, classlist, ferr);
	} else if (selector & SEL_TARGET2) {
	    paramTargetBody (uEmpty, cell->textOut (wiki), target, ferr);
	} else if ((selector & SEL_ONCLICK2) && checkScript (*cell, onclick, ferr)) {
	} else if ((selector & SEL_MULTIPLE2) && match (key, CharConst ("multiple"))) {
	    fmultiple = true;
	} else if ((selector & SEL_DEFAULT2) && match (key, CharConst ("default"))) {
	    fdefault = true;
	} else if ((selector & SEL_SCRIPT2) && checkScript (*cell, script, ferr)) {
	} else if (readAttribMore2 (key, *cell, ferr)) {
	} else {
	    if (mode == M_ATTRIB) {
		wiki->errorMsg.append (cell->dump ()).append (CharConst (": bad attribute.\n"));
		rc = false;
	    }
	    return false;
	}
    }
    if (ferr) {
	rc = false;
	return false;
    } else {
	return true;
    }
}

bool  WikiAttrib1::shiftAttrib (WikiMotorObjVecPtr& vec) {
    bool  rc = true;
    WikiMotorObjVecPtr  v2;

    while (vec->size () > 0) {
	WikiMotorObjVec  cell;

	v2 = WikiMotorObjVecPtr (new WikiMotorObjVec);
	if (vec->splitChar (':', cell, *v2) || mode == M_ATTRIB || mode == M_ATTRIB_TEXT) {
	    if (! readAttrib1 (&cell, rc))
		return rc;
	    vec = v2;
	} else {
	    return true;
	}
    }
    return true;
}

void  WikiAttrib1::skipAttrib (WikiMotorObjVecPtr& vec) {
    WikiMotorObjVecPtr  v2;

    if (vec->size () > 0) {
	WikiMotorObjVec  cell;

	v2 = WikiMotorObjVecPtr (new WikiMotorObjVec);
	if (vec->splitChar (':', cell, *v2)) {
	    vec = v2;
	}
    }
}

bool  WikiAttrib1::readAttrib (WikiMotorObjVecVec::const_iterator& b, const WikiMotorObjVecVec::const_iterator& e) {
    bool  rc = true;

    while (b < e) {
	WikiMotorObjVec*  cell = b->get ();
	if (! readAttrib1 (cell, rc))
	    return rc;
	b ++;
    }
    return true;
}

bool  WikiAttrib1::checkScript (WikiMotorObjVec& vec, ustring& scr, bool& ferr) {
    bool  f = false;
    int  i;
    ustring  u;
    for (i = 0; i < vec.size () && vec[i]->type == WikiMotorObj::wiki_funcLink; i ++) {
	WikiMotorObjFuncLink*  fn = WikiMotorObjFuncLink_type (&*vec[i]);
	u = fn->execDefunArgs (wiki);
	// XXX: 末尾の空白を除去し，セミコロンを確認する。
	scr.append (u);
	f = true;
    }
    if (f)
	return true;

    return false;
}

bool  WikiAttrib1::shiftLink (WikiMotorObjVecPtr& vec, ustring& url, bool& fscript) {
    bool  ferr = false;
    bool  rc;
    WikiMotorObjVec  cell;
    WikiMotorObjVecPtr  v2 (new WikiMotorObjVec);

    rc = vec->splitChar (':', cell, *v2);
    if (checkScript (cell, url, ferr)) {
	fscript = true;
	vec = v2;
	return true;
    } else if (rc && cell.match (CharConst ("http"), CharConst ("https"))) {
	ustring  proto = cell.dump ();
	vec = v2;
	cell.clear ();
	v2 = WikiMotorObjVecPtr (new WikiMotorObjVec);
	rc = vec->splitChar (':', cell, *v2);
	if (! cell.splitURL_2 (wiki, proto, url)) {
	    vec = v2;
	    return false;
	}
	vec = v2;
	if (vec->size () > 0) {
	    cell.clear ();
	    v2 = WikiMotorObjVecPtr (new WikiMotorObjVec);
	    rc = vec->splitChar (':', cell, *v2);
	    if (cell.splitURL_3 (wiki, url, url))
		vec = v2;
	}
	return true;
    } else if (cell.splitURLPath (wiki, url)) {
	vec = v2;
	return true;
    } else {
	return false;
    }
    return false;
}

bool  WikiAttrib1::readLink (WikiMotorObjVecVec::const_iterator& b, const WikiMotorObjVecVec::const_iterator& e, ustring& url, bool& fscript, bool noscript) {
    bool  ferr = false;
    bool  rc;
    WikiMotorObjVec*  cell;
    WikiMotorObjVec*  cell2;
    static uregex  re (CharConst ("^[a-z]{3,6}$"));

    if (b == e)
	return false;
    cell = b->get ();
    rc = (b + 1 < e);
    if (! noscript && checkScript (*cell, url, ferr)) {
	fscript = true;
	b ++;
	return true;
    } else if (rc && cell->regmatch (re) && (cell2 = (b + 1)->get ()) && cell2->size () > 0 && (*cell2)[0].get ()->matchHead (CharConst ("//"))) {
	ustring  proto = cell->dump ();
	b ++;
	cell = b->get ();
	if (! cell->splitURL_2 (wiki, proto, url)) {
	    b ++;
	    return false;
	}
	b ++;
	if (b < e) {
	    cell = b->get ();
	    if (cell->splitURL_3 (wiki, url, url))
		b ++;
	}
	return true;
    } else if (cell->splitURLPath (wiki, url)) {
	b ++;
	return true;
    } else {
	return false;
    }
    return false;
}

bool  WikiAttrib1::shiftName (WikiMotorObjVecPtr& vec, ustring& name) {
    if (vec->size () > 0) {
	WikiMotorObjVec  cell;
	WikiMotorObjVecPtr  v2 (new WikiMotorObjVec);

	vec->splitChar (':', cell, *v2);
	name = cell.textOut (wiki);
	vec = v2;
	return true;
    } else {
	return false;
    }
}

void  WikiAttrib1::output (MotorOutput* out) {
    wiki->outputID (out, id);
    wiki->outputClass (out, classlist);
#ifdef BOOTSTRAPHACK
//    std::pair<ustring,ustring>::const_iterator  b, e;
    std::vector<datapre_t>::const_iterator  b, e;
    b = datapre.begin ();
    e = datapre.end ();
    for (; b < e; b ++) {
	wiki->outputName (out, (*b).first.c_str (), (*b).first.size (), (*b).second);
    }
#endif
    if (start >= 0)
	wiki->outputNum (out, CharConst ("start"), start);
    wiki->outputSubmitScript (out, CharConst ("onclick"), onclick, scriptcut);
    wiki->outputSubmitScript (out, CharConst ("onfocus"), onfocus, scriptcut);
    wiki->outputSubmitScript (out, CharConst ("onblur"), onblur, scriptcut);
    wiki->outputSubmitScript (out, CharConst ("onchange"), onchange, scriptcut);
    wiki->outputName (out, CharConst ("target"), target);
    wiki->outputFlag (out, CharConst ("multiple"), fmultiple);
    // no fdefault
    outputMore (out);
}

bool  WikiAttrib1::paramID (const ustring& key, WikiMotorObjVec& vval, bool& ferr) {
    if (match (key, CharConst ("id"))) {
	paramIDValue (key, vval, id, ferr);
	return true;
    } else {
	return false;
    }
}

void  WikiAttrib1::paramIDValue (const ustring& key, WikiMotorObjVec& vval, ustring& var, bool& ferr) {
    ustring  value (vval.textOut (wiki));
    if (matchWikiID (value)) {
	var = value;
	ferr = false;
    } else {
	wiki->errorMsg.append (key).append (uEq).append (value).append (CharConst (": bad value\n"));
	ferr = true;
    }
}

bool  WikiAttrib1::paramClass (const ustring& key, WikiMotorObjVec& vval, bool& ferr) {
    if (match (key, CharConst ("class"))) {
	paramClassValue (vval, classlist, ferr);
	return true;
    } else {
	return false;
    }
}

void  WikiAttrib1::paramClassValue (WikiMotorObjVec& vval, std::vector<ustring>& var, bool& ferr) {
    WikiMotorObjVecVec  args;
    ferr = false;
    vval.splitCharA (',', args);
    for (int i = 0; i < args.size (); i ++) {
	ustring  value (args[i]->textOut (wiki));
	if (value.length () > 0) {
	    if ((selector & SEL_DIRECT) && value[0] == '#' && matchWikiID (ustring (value.begin () + 1, value.end ()))) {
		directlist.push_back (value);		// SEL_DIRECTが指定してある場合のみ有効
	    } else if (matchWikiID (value)) {
		var.push_back (value);
	    } else {
		wiki->errorMsg.append (value).append (CharConst (": bad class name\n"));
		ferr = true;
	    }
	}
    }
}

bool  WikiAttrib1::paramWidth (const ustring& key, WikiMotorObjVec& vval, ustring& var, bool& ferr) {
    if (match (key, CharConst ("width"), CharConst ("w"))) {
	paramWidthValue (key, vval, var, ferr);
	return true;
    } else {
	return false;
    }
}

bool  WikiAttrib1::paramHeight (const ustring& key, WikiMotorObjVec& vval, ustring& var, bool& ferr) {
    if (match (key, CharConst ("height"), CharConst ("h"))) {
	paramWidthValue (key, vval, var, ferr);
	return true;
    } else {
	return false;
    }
}

void  WikiAttrib1::paramWidthValue (const ustring& key, WikiMotorObjVec& vval, ustring& var, bool& ferr) {
    ustring  value (vval.textOut (wiki));
    if (matchWidth (value)) {
	var = value;
	ferr = false;
    } else {
	wiki->errorMsg.append (key).append (uEq).append (value).append (CharConst (": bad value\n"));
	ferr = true;
    }
}

bool  WikiAttrib1::paramSize (const char* name, size_t namelen, const ustring& key, WikiMotorObjVec& vval, ustring& var, bool& ferr) {
    if (match (key, name, namelen)) {
	ustring  value (vval.textOut (wiki));
	if (matchNum (value)) {
	    var = value;
	    ferr = false;
	} else {
	    ferr = true;
	}
	return true;
    }
    return false;
}

void  WikiAttrib1::paramUNum (const ustring& value, int& var, const ustring& name) {
    if (matchNum (value)) {
	var = strtoul (value);
    } else {
	wiki->errorMsg.append (name).append (uEq).append (value).append (uErrorBadValue).append (uLF);
    }
}

void  WikiAttrib1::paramColor (const ustring& value, ustring& var, const ustring& name) {
    if (checkColor (value)) {
	var = value;
    } else {
	wiki->errorMsg.append (name).append (uEq).append (value).append (uErrorBadValue).append (uLF);
    }
}

bool  WikiAttrib1::paramTargetCheck (const ustring& key) {
    return match (key, CharConst ("target"));
}

void  WikiAttrib1::paramTargetBody (const ustring& key, const ustring& value, ustring& var, bool& ferr) {
    if (value.length () == 0 || matchWikiID (value)) {
	var = value;
    } else {
	if (key.length () > 0)
	    wiki->errorMsg.append (key).append (uEq).append (value).append (CharConst (": bad target name.\n"));
	else
	    wiki->errorMsg.append (value).append (CharConst (": bad target name.\n"));
	ferr = true;
    }
}

bool  WikiAttrib1::paramOnClickCheck (const ustring& name) {
    return match (name, CharConst ("onclick"));
}

bool  WikiAttrib1::paramOnFocusCheck (const ustring& name) {
    return match (name, CharConst ("onfocus"));
}

bool  WikiAttrib1::paramOnBlurCheck (const ustring& name) {
    return match (name, CharConst ("onblur"));
}

bool  WikiAttrib1::paramOnChangeCheck (const ustring& name) {
    return match (name, CharConst ("onchange"));
}

#ifdef BOOTSTRAPHACK
bool  WikiAttrib1::paramDataPrefix (const ustring& key, WikiMotorObjVec& vval, bool& ferr) {
    if (matchHead (key, CharConst ("data-")) && matchWikiID (key)) {
	ustring  value (vval.textOut (wiki));
	if (matchWikiID (value)) {
	    datapre.push_back (std::pair<ustring,ustring> (key, value));
	    ferr = false;
	} else {
	    wiki->errorMsg.append (key).append (uEq).append (value).append (CharConst (": bad value\n"));
	    ferr = true;
	}
	return true;
    } else {
	return false;
    }
}
#endif
/* ============================================================ */
void  WikiAttribTable::init () {
    fheader = false;
    halign = HAlignNone;
    valign = VAlignNone;
    fnowrap = false;
#ifdef WIKITABLEATTRIB
    fnoborder = false;
    cellspacing = -1;
    cellpadding = -1;
#endif
    fpadding = false;
//	halign = HAlignNone;
    fnowhite = false;
    fturn = false;

    id.resize (0);
    while (! classlist.empty ())
	classlist.pop_back ();
    width.resize (0);
    height.resize (0);
    color.resize (0);
    bgcolor.resize (0);
    onclick.resize (0);
}

void  WikiAttribTable::copyFrom (WikiAttribTable& b) {
    int  i;

    id = b.id;
    for (i = 0; i < b.classlist.size (); i ++)
	classlist.push_back (b.classlist[i]);
    onclick = b.onclick;
    fheader = b.fheader;
    halign = b.halign;
    valign = b.valign;
    width = b.width;
    height = b.height;
    color = b.color;
    bgcolor = b.bgcolor;
    fnowrap = b.fnowrap;
}

bool  WikiAttribTable::readAttribMore (const ustring& key, WikiMotorObjVec& vval, bool& ferr) {
    if (paramWidth (key, vval, width, ferr)) {
    } else if (paramHeight (key, vval, height, ferr)) {
    } else if (match (key, CharConst ("color"))) {
	paramColor (vval.textOut (wiki), color, key);
    } else if (match (key, CharConst ("bgcolor"), CharConst ("bg"))) {
	paramColor (vval.textOut (wiki), bgcolor, key);
    } else {
	switch (selector2) {
	case SEL_TABLE:
#ifdef WIKITABLEATTRIB
	    if (match (key, CharConst ("cellspacing"), CharConst ("spc"))) {
		paramUNum (vval.textOut (wiki), cellspacing, key);
	    } else if (match (key, CharConst ("cellpadding"))) {
		paramUNum (vval.textOut (wiki), cellpadding, key);
	    } else {
		return false;
	    }
#else
	    return false;
#endif
	    break;
	case SEL_TR:
	case SEL_TD:
	    return false;
	default:
	    assert (0);
	}
    }
    return true;
}

bool  WikiAttribTable::readAttribMore2 (const ustring& key, WikiMotorObjVec& cell, bool& ferr) {
    if (match (key, CharConst ("left"), CharConst ("l"))) {
	halign = HAlignLeft;
    } else if (match (key, CharConst ("right"), CharConst ("r"))) {
	halign = HAlignRight;
    } else if (match (key, CharConst ("center"), CharConst ("c"))) {
	halign = HAlignCenter;
    } else {
	switch (selector2) {
	case SEL_TABLE:
#ifdef WIKITABLEATTRIB
	    if (match (key, CharConst ("noborder"), CharConst ("nb"))) {
		fnoborder = true;
	    } else
#endif
	    if (match (key, CharConst ("padding"), CharConst ("pad"))) {
		fpadding = true;
	    } else if (match (key, CharConst ("expanding"), CharConst ("expand"))) {
		fpadding = false;
	    } else if (match (key, CharConst ("nowhite"), CharConst ("nw"))) {
		fnowhite = true;
	    } else if (match (key, CharConst ("turn"))) {
		fturn = true;
	    } else {
		return false;
	    }
	    break;
	case SEL_TR:
	case SEL_TD:
	    if (match (key, CharConst ("header"), CharConst ("h"))) {
		fheader = true;
	    } else if (match (key, CharConst ("top"), CharConst ("t"))) {
		valign = VAlignTop;
	    } else if (match (key, CharConst ("middle"), CharConst ("m"))) {
		valign = VAlignMiddle;
	    } else if (match (key, CharConst ("bottom"), CharConst ("b"))) {
		valign = VAlignBottom;
	    } else if (match (key, CharConst ("nowrap"), CharConst ("nw"))) {
		fnowrap = true;
	    } else if (match (key, CharConst ("*"))) {
		init ();
	    } else {
		return false;
	    }
	    break;
	default:
	    assert (0);
	}
    }

    return true;
}

void  WikiAttribTable::outputMore (MotorOutput* out) {
    switch (halign) {
    case HAlignLeft:
	out->out_raw (CharConst (" align=\"left\""));
	break;
    case HAlignCenter:
	out->out_raw (CharConst (" align=\"center\""));
	break;
    case HAlignRight:
	out->out_raw (CharConst (" align=\"right\""));
	break;
    default:;
    }
    wiki->outputName (out, CharConst ("width"), width);
    wiki->outputName (out, CharConst ("height"), height);
    wiki->outputName (out, CharConst ("bgcolor"), bgcolor);
    if (color.length () > 0) {    
	ustring  style;
	style.append (CharConst ("color:")).append (color).append (CharConst (";"));
	wiki->outputStyle (out, style);
    }

    switch (selector2) {
    case SEL_TABLE:
#ifdef WIKITABLEATTRIB
	wiki->outputName (out, CharConst ("border"), fnoborder ? 0 : 1, false);
	wiki->outputName (out, CharConst ("cellspacing"), cellspacing, false);
	wiki->outputName (out, CharConst ("cellpadding"), cellpadding, false);
#endif
	break;
    case SEL_TR:
    case SEL_TD:
	switch (valign) {
	case VAlignTop:
	    out->out_raw (CharConst (" valign=\"top\""));
	    break;
	case VAlignMiddle:
	    out->out_raw (CharConst (" valign=\"middle\""));
	    break;
	case VAlignBottom:
	    out->out_raw (CharConst (" valign=\"bottom\""));
	    break;
	default:;
	}
	wiki->outputFlag (out, CharConst ("nowrap"), fnowrap);
	break;
    default:
	assert (0);
    }
}

/* ============================================================ */
bool  WikiAttribImg::readAttribMore (const ustring& key, WikiMotorObjVec& vval, bool& ferr) {
    if (paramWidth (key, vval, width, ferr)) {
	if (matchNum (width))
	    width.append (CharConst ("px"));
    } else if (paramHeight (key, vval, height, ferr)) {
	if (matchNum (height))
	    height.append (CharConst ("px"));
    } else if (match (key, CharConst ("alt"))) {
	alt = vval.textOut (wiki);
    } else {
	return false;
    }
    return true;
}

void  WikiAttribImg::outputMore (MotorOutput* out) {
//    wiki->outputName (out, CharConst ("width"), width);
//    wiki->outputName (out, CharConst ("height"), height);
    if (width.length () > 0 || height.length () > 0) {
	out->out_raw (CharConst (" style=\""));
	if (width.length () > 0)
	    out->out_raw (CharConst ("width:"))->out_toHTML_noCtrl (width)->out_raw (CharConst (";"));
	if (height.length () > 0)
	    out->out_raw (CharConst ("height:"))->out_toHTML_noCtrl (height)->out_raw (CharConst (";"));
	out->out_raw (CharConst ("\""));
    }
    // longdesc
}

/* ============================================================ */
bool  WikiAttribInput::readAttribMore (const ustring& key, WikiMotorObjVec& vval, bool& ferr) {
    if ((selector2 & SEL_INPUT) && paramSize (CharConst ("size"), key, vval, psize, ferr)) {
    } else if ((selector2 & SEL_ELSIZE) && match (key, CharConst ("size"))) {
	ustring  v = vval.textOut (wiki);
	if (match (v, CharConst ("*"))) {
	    elsize = 1;
	} else if (matchNum (v)) {
	    elsize = to_int64 (v);
	    if (elsize < 0 || elsize > 999) {
		elsize = 1;
	    }
	} else {
	    elsize = 1;
	}
    } else if (paramWidth (key, vval, pwidth, ferr)) {
    } else if ((selector2 & SEL_TEXTAREA) && paramSize (CharConst ("cols"), key, vval, pcols, ferr)) {
    } else if ((selector2 & SEL_TEXTAREA) && paramSize (CharConst ("rows"), key, vval, prows, ferr)) {
    } else if ((selector2 & SEL_TEXTAREA) && match (key, CharConst ("wrap"))) {
	ustring  value (vval.textOut (wiki));
	if (match (value, CharConst ("off"))) {
	    pwrap = W_OFF;
	} else if (match (value, CharConst ("soft"))) {
	    pwrap = W_SOFT;
	} else if (match (value, CharConst ("hard"))) {
	    pwrap = W_HARD;
	} else {
	    wiki->errorMsg.append (key).append (CharConst ("=")).append (vval.dump ()).append (CharConst (": link script error.\n"));
	    ferr = true;
	    return false;
	}
    } else if (match (key, CharConst ("accept"))) {
	ustring  value (vval.textOut (wiki));
	if (match (value, CharConst ("camera"))) {
	    paccept.assign (CharConst ("image/*;capture=camera"));
	} else {
	    static  uregex re ("^"
			       "(" "[a-z_0-9-]+/[a-z_0-9.+*-]+" ")"
			       "(" "," "(" "[a-z_0-9-]+/[a-z_0-9.+*-]+" ")" ")*"
			       "(" ";" "[a-z_0-9-]+=[a-zA-Z_0-9.+*-]+" ")*"
			       "$");
	    if (checkRe (value, re)) {
		paccept = value;
	    } else {
		return false;
	    }
	}
    } else {
	return false;
    }
    return true;
}

bool  WikiAttribInput::readAttribMore2 (const ustring& key, WikiMotorObjVec& cell, bool& ferr) {
    if (key == uDefault) {
	pdefault = true;
    } else if ((selector2 & SEL_CHECK) && match (key, CharConst ("checked"))) {
	pchecked = true;
#ifdef INSERTTABHACK
    } else if ((selector2 & SEL_TEXTAREA) && match (key, CharConst ("tab"))) {
	ftab = true;
#endif
    } else {
	return false;
    }
    return true;
}

void  WikiAttribInput::outputMore (MotorOutput* out) {
    wiki->outputName (out, CharConst ("size"), psize);
    wiki->outputName (out, CharConst ("size"), elsize);
    if (pwidth.size () > 0) {
	if (matchNum (pwidth)) {
	    out->out_raw (CharConst (" style=\"width:"))->out_toHTML_noCtrl (pwidth)->out_raw (CharConst ("px;\""));
	} else {
	    out->out_raw (CharConst (" style=\"width:"))->out_toHTML_noCtrl (pwidth)->out_raw (CharConst (";\""));
	}
    }
    if (selector2 & SEL_CHECK) {
	wiki->outputFlag (out, CharConst ("checked"), pchecked);
    }
    if (selector2 & SEL_TEXTAREA) {
	wiki->outputName (out, CharConst ("cols"), pcols, false);
	wiki->outputName (out, CharConst ("rows"), prows);
	switch (pwrap) {
	case W_OFF:
	    out->out_raw (CharConst (" wrap=\"off\""));
	    break;
	case W_SOFT:
	    out->out_raw (CharConst (" wrap=\"soft\""));
	    //	out->out_raw (CharConst (" wrap=\"virtual\""));
	    break;
	case W_HARD:
	    out->out_raw (CharConst (" wrap=\"hard\""));
	    //	out->out_raw (CharConst (" wrap=\"physical\""));
	    break;
	}
#ifdef INSERTTABHACK
	if (ftab) {
	    out->out_raw (uSPC)
		->out_raw (CharConst ("onkeypress"))
		->out_raw (CharConst ("=\""))
		->out_toHTML (ustring (CharConst ("return insertTab(event,this);")))
		->out_raw ("\"");
	    out->out_raw (uSPC)
		->out_raw (CharConst ("onkeydown"))
		->out_raw (CharConst ("=\""))
		->out_toHTML (ustring (CharConst ("return insertTab(event,this);")))
		->out_raw (uQ2);
	    out->out_raw (uSPC)
		->out_raw (CharConst ("onkeyup"))
		->out_raw (CharConst ("=\""))
		->out_toHTML (ustring (CharConst ("return insertTab(event,this);")))
		->out_raw (uQ2);
	}
#endif
    }
    if (paccept.length () > 0) {
	out->out_raw (CharConst (" accept=\""))
	    ->out_toHTML (paccept)
	    ->out_raw (uQ2);
    }
}

/* ============================================================ */
bool  WikiAttribButton::readAttrib (WikiMotorObjVecVec::const_iterator& b, const WikiMotorObjVecVec::const_iterator& e) {
    bool  rc = true;

    if (b < e) {
	WikiMotorObjVec*  cell = b->get ();
	mode_t  bm = mode;

	mode = M_NORMAL;
	if (! readAttrib1 (cell, rc)) {
	    name = cell->textOut (wiki);
	}
	mode = bm;
	b ++;
    }
    while (b < e) {
	WikiMotorObjVec*  cell = b->get ();
	if (! readAttrib1 (cell, rc))
	    return rc;
	b ++;
    }
    return true;
}

/* ============================================================ */
bool  WikiAttribLink::readAttribMore2 (const ustring& key, WikiMotorObjVec& cell, bool& ferr) {
    arglist.push_back (cell.textOut (wiki));
    return true;
}

/* ============================================================ */
bool  WikiAttribItem::readAttribMore (const ustring& key, WikiMotorObjVec& vval, bool& ferr) {
    if (match (key, CharConst ("ulclass"))) {
	if (owner && owner->parent && owner->parent->attrib.classlist.size () == 0) {
	    paramClassValue (vval, owner->parent->attrib.classlist, ferr);
	}
    } else if (match (key, CharConst ("ulid"))) {
	if (owner && owner->parent && owner->parent->attrib.id.size () == 0) {
	    paramIDValue (key, vval, owner->parent->attrib.id, ferr);
	}
    } else if (owner && owner->parent && (owner->parent->attrib.selector & SEL_START) && match (key, CharConst ("start"))) {
	paramUNum (vval.textOut (wiki), owner->parent->attrib.start, key);
    } else {
	return false;
    }
    return true;
}

/* ============================================================ */
