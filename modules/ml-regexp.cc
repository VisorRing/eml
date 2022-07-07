#include "ml-regexp.h"
#include "ml.h"
#include "ml-texp.h"
#include "mlenv.h"
#include "motorenv.h"
#include "motoroutput.h"
#include "util_const.h"
#include "util_check.h"
#include "util_string.h"
#include "util_wsplitter.h"
#include "expr.h"
#include "utf8.h"
#include "utf16.h"
#include "ustring.h"
#include <exception>

/*DOC:
==regular expression functions==

*/

/*DOC:
===regexp-match===
 (regexp-match REGEX TEXT [#i]) -> BOOL

*/
//#XAFUNC	regexp-match	ml_regexp_match
//#XWIKIFUNC	regexp-match
MNode*  ml_regexp_match (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_LIST, EV_END};
    MNodePtr  posParams[2];
    kwParam  kwList[] = {{CharConst ("i"), EV_LIST},
			 {NULL, 0, EV_END}};
    MNodePtr  kwParams[1];
    evalParams (fev, mlenv, cell, posList, posParams, kwList, kwParams);
    ustring  reg = to_string (posParams[0]());
    ustring  text = to_string (posParams[1]());
    boost::wregex::flag_type  f = boost::regex_constants::normal;
    bool  ans;
    if (to_bool (kwParams[0]()))
	f |= boost::regex_constants::icase;
    ans = wsearch_env (mlenv->regenv, text, reg, f);
    return newMNode_bool (ans);
}

/*DOC:
===match-string===
 (match-string NUM) -> STRING

*/
//#XAFUNC	match-string	ml_match_string
//#XWIKIFUNC	match-string
MNode*  ml_match_string (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    evalParams (fev, mlenv, cell, posList, posParams);
    int  n = to_int64 (posParams[0]());
    MNode*  ans = NULL;
    if (0 <= n && n < mlenv->regenv.regmatch.size ()) {
	ans = newMNode_str (new ustring (wtou (std::wstring (mlenv->regenv.regmatch[n].first, mlenv->regenv.regmatch[n].second))));
    }
    return ans;
}

/*DOC:
===prematch===
 (prematch) -> STRING

*/
//#XAFUNC	prematch	ml_prematch
//#XWIKIFUNC	prematch
MNode*  ml_prematch (bool fev, MNode* cell, MlEnv* mlenv) {
    evalParams (fev, mlenv, cell);
    MNode*  ans = NULL;
    std::wstring::const_iterator  b = mlenv->regenv.regtext.begin ();
    ans = newMNode_str (new ustring (wtou (std::wstring (b, mlenv->regenv.regmatch[0].first))));
    return ans;
}

/*DOC:
===postmatch===
 (postmatch) -> STRING

*/
//#XAFUNC	postmatch	ml_postmatch
//#XWIKIFUNC	postmatch
MNode*  ml_postmatch (bool fev, MNode* cell, MlEnv* mlenv) {
    evalParams (fev, mlenv, cell);
    MNode*  ans = NULL;
    std::wstring::const_iterator  e = mlenv->regenv.regtext.end ();
    ans = newMNode_str (new ustring (wtou (std::wstring (mlenv->regenv.regmatch[0].second, e))));
    return ans;
}

/*DOC:
===string-filter===
 (string-filter REGEX STRING [#i] [:max NUM]) -> STRING

*/
//#XAFUNC	string-filter	ml_string_filter
//#XWIKIFUNC	string-filter
MNode*  ml_string_filter (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_LIST, EV_END};
    MNodePtr  posParams[2];
    kwParam  kwList[] = {{CharConst ("i"), EV_LIST},
			 {CharConst ("max"), EV_LIST},
			 {NULL, 0, EV_END}};
    MNodePtr  kwParams[2];
    evalParams (fev, mlenv, cell, posList, posParams, kwList, kwParams);
    ustring  reg = to_string (posParams[0]());
    ustring  text = to_string (posParams[1]());
    boost::wregex::flag_type  f = boost::regex_constants::normal;
    if (to_bool (kwParams[0]()))
	f |= boost::regex_constants::icase;
    size_t  max = to_int64 (kwParams[1]());
    if (max < 0) {
	max = 0;
    }
    if (wsearch_env (mlenv->regenv, text, reg, f)) {
	ustring  ans = wtou (std::wstring (mlenv->regenv.regmatch[0].first, mlenv->regenv.regmatch[0].second));
	if (max > 0) {
	    substring (ans, 0, max, true, ans);
	}
	return newMNode_str (new ustring (ans));
    } else {
	return NULL;		// unmatched
    }
}

/*DOC:
===regexp-replace===
 (regexp-replace REGEX TO_TEXT TEXT [#i] [#g | #global]) -> TEXT

*/
//#XAFUNC	regexp-replace	ml_regexp_replace
//#XWIKIFUNC	regexp-replace
MNode*  ml_regexp_replace (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_LIST, EV_LIST, EV_END};
    MNodePtr  posParams[3];
    kwParam  kwList[] = {{CharConst ("i"), EV_LIST},
			 {CharConst ("g"), EV_LIST},
			 {CharConst ("global"), EV_LIST},
			 {NULL, 0, EV_END}};
    MNodePtr  kwParams[3];
    evalParams (fev, mlenv, cell, posList, posParams, kwList, kwParams);
    ustring  reg = to_string (posParams[0]());
    ustring  to = to_string (posParams[1]());
    ustring  text = to_string (posParams[2]());
    boost::wregex::flag_type  f = boost::regex_constants::normal;
    if (to_bool (kwParams[0]()))
	f |= boost::regex_constants::icase;
    boost::match_flag_type  mf = boost::regex_constants::match_default;
    bool  fglobal = to_bool (kwParams[1]()) || to_bool (kwParams[2]());
    ustring  ans;
    if (! fglobal)
	mf |= boost::regex_constants::format_first_only;
    ans = wreplace (text, reg, to, f, mf);
    return newMNode_str (new ustring (ans));
}

/*DOC:
===regexp-split-2===
 (regexp-split-2 REGEX STRING [#i]) -> (PREMATCH_STRING POSTMATCH_STRING)

*/
//#XAFUNC	regexp-split-2	ml_regexp_split_2
//#XWIKIFUNC	regexp-split-2
MNode*  ml_regexp_split_2 (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_LIST, EV_END};
    MNodePtr  posParams[2];
    kwParam  kwList[] = {{CharConst ("i"), EV_LIST},
			 {NULL, 0, EV_END}};
    MNodePtr  kwParams[1];
    evalParams (fev, mlenv, cell, posList, posParams, kwList, kwParams);
    ustring  reg = to_string (posParams[0]());
    ustring  text = to_string (posParams[1]());
    boost::wregex::flag_type  f = boost::regex_constants::normal;
    f |= to_bool (kwParams[0]()) ? boost::regex_constants::icase : 0;
    MNodeList  ans;
    if (wsearch_env (mlenv->regenv, text, reg, f)) {
	std::wstring::const_iterator  b = mlenv->regenv.regtext.begin ();
	std::wstring::const_iterator  e = mlenv->regenv.regtext.end ();
	ans.append (newMNode_str (new ustring (wtou (std::wstring (b, mlenv->regenv.regmatch[0].first)))));
	ans.append (newMNode_str (new ustring (wtou (std::wstring (mlenv->regenv.regmatch[0].second, e)))));
    } else {
	ans.append (newMNode_str (new ustring (text)));
	ans.append (NULL);
    }

    return ans.release ();
}

/*DOC:
===regexp-split===
// (regexp-split REGEX STRING [#keep] [#i]) -> STRING_LIST
// (regexp-split REGEX STRING #vector [#keep] [#i]) -> STRING_VECTOR
 (regexp-split REGEX STRING [#i] [#trim] [:parts NUM]) -> (STRING ...)
 (regexp-split REGEX STRING #vector [#i] [#trim] [:parts NUM]) -> [STRING ...]

*/
//#XAFUNC	regexp-split	ml_regexp_split
//#XWIKIFUNC	regexp-split
MNode*  ml_regexp_split (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_LIST, EV_END};
    MNodePtr  posParams[2];
    kwParam  kwList[] = {{CharConst ("i"), EV_LIST},	  // 0
			 {CharConst ("trim"), EV_LIST},	  // 1
			 {CharConst ("parts"), EV_LIST},  // 2
			 {CharConst ("vector"), EV_LIST}, // 3
			 {NULL, 0, EV_END}};
    MNodePtr  kwParams[4];
    evalParams (fev, mlenv, cell, posList, posParams, kwList, kwParams);
    ustring  reg = to_string (posParams[0]());
    ustring  text = to_string (posParams[1]());
    bool  flagI = to_bool (kwParams[0]());
    bool  flagTrim = to_bool (kwParams[1]());
    int64_t  pnum = to_int64 (kwParams[2]());
    bool  flagVec = to_bool (kwParams[3]());
    ListMakerPtr  ans;
    boost::wregex::flag_type  reg_flags = boost::regex_constants::normal;
    reg_flags |= to_bool (kwParams[2]()) ? boost::regex_constants::icase : 0;
    if (flagVec)
	ans = new ListMakerVector;
    else
	ans = new ListMakerList;
    if (reg.length () == 0) {
	uiterator  b = text.begin ();
	uiterator  e = text.end ();
	uiterator  s;
	while (b < e) {
	    if (pnum > 0) {
		if (pnum == 1) {
		    ans.append (newMNode_str (new ustring (b, e)));
		    break;
		}
		-- pnum;
	    }
	    s = b;
	    nextChar (b, e);
	    ans.append (newMNode_str (new ustring (s, b)));
	}
    } else {
	try {
	    std::wstring  wt = utow (text);
	    std::wstring  wreg = utow (reg);
	    boost::wregex  wre (wreg, reg_flags);
	    WSplitter  sp (wt, wre);
	    size_t  m = wt.length () + 1;
	    
	    bool  (WSplitter::*nfn)();
	    if (flagTrim)
		nfn = &WSplitter::next;
	    else
		nfn = &WSplitter::nextSep;
	    while ((sp.*nfn) ()) {
		if (pnum > 0) {
		    if (pnum == 1) {
			ans.append (newMNode_str (new ustring (sp.b, sp.e)));
			return ans.release ();
		    }
		    -- pnum;
		}
		ans.append (newMNode_str (new ustring (sp.cur ())));
		m --;
		if (m == 0)
		    throw (uErrorRegexp);
	    }
	    if (! flagTrim)
		ans.append (newMNode_str (new ustring (sp.cur ())));
	} catch (boost::regex_error& err) {
	    throw (uErrorRegexp);
	}
    }
    return ans.release ();
}

/*DOC:
===escape-regexp===
 (escape-regexp STRING...) -> STRING

*/
//#XAFUNC	escape-regexp	ml_escape_regexp
//#XWIKIFUNC	escape-regexp
MNode*  ml_escape_regexp (bool fev, MNode* cell, MlEnv* mlenv) {
    MNodePtr  rest;
    evalParams (fev, mlenv, cell, EV_LIST, &rest);
    MNode*  args = rest ();
    ustring  str;
    while (args) {
	str.append (to_string (args->car ()));
	nextNode (args);
    }
    return newMNode_str (new ustring (escape_re (str)));
}

