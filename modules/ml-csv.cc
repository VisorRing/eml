#include "ml-csv.h"
#include "ml.h"
#include "ml-texp.h"
#include "mlenv.h"
#include "util_string.h"
#include "expr.h"
#include "ustring.h"

/*DOC:
==CSV function==

*/
/*DOC:
===csv-read===
 (csv-read STRING) -> VECTOR_OF_VECTOR

*/
//#XAFUNC	csv-read	ml_csv_read
//#XWIKIFUNC	csv-read
MNode*  ml_csv_read (bool fev, MNode* cell, MlEnv* mlenv) {
    static uregex  re ("(,)|(\")|(\r?\n)");
    static uregex  re_q ("(\"\")|\",|\"(\r?\n)|\"$");
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    evalParams (fev, mlenv, cell, posList, posParams);
    ustring  text = to_string (posParams[0]());
    ListMakerPtr  row;
    ListMakerPtr  ans;
    ans = new ListMakerVector;
    uiterator  b = text.begin ();
    uiterator  e = text.end ();
    umatch  m;
    ustring  cel;
    enum {
	BOL,
	COL,
	EOL
    }  mode = BOL;
    while (b < e) {
	if (usearch (b, e, m, re)) {
	    if (m[1].matched) {				// ,
		if (! row.assigned ()) row = new ListMakerVector;
		if (b == m[0].first) {
		    row.append (NULL);
		} else {
		    row.append (newMNode_str (new ustring (b, m[0].first)));
		}
		b = m[0].second;
		mode = COL;
	    } else if (m[2].matched) {			// "
		b = m[0].second;
		cel.resize (0);
		while (b < e) {
		    if (usearch (b, e, m, re_q)) {
			if (m[1].matched) {		// ""
			    cel.append (b, m[0].first + 1);
			    b = m[0].second;
			} else {
			    cel.append (b, m[0].first);
			    b = m[0].second;
			    if (m[2].matched) {		// "CRLF
				mode = EOL;
			    } else {
				mode = COL;
			    }
			    break;
			}
		    } else {
			cel.append (b, e);
			b = e;
			mode = EOL;
			break;
		    }
		}
		if (! row.assigned ()) row = new ListMakerVector;
		row.append (newMNode_str (new ustring (cel)));
	    } else if (m[3].matched) {			// CRLF
		if (! row.assigned ()) row = new ListMakerVector;
		if (mode == COL) {
		    if (b == m[0].first) {
			row.append (NULL);
		    } else {
			row.append (newMNode_str (new ustring (b, m[0].first)));
		    }
		} else if (b < m[0].first) {
		    row.append (newMNode_str (new ustring (b, m[0].first)));
		}
		b = m[0].second;
		mode = EOL;
	    } else {
		assert (0);
	    }
	} else {
	    if (! row.assigned ()) row = new ListMakerVector;
	    if (b == e) {
		row.append (NULL);
	    } else {
		row.append (newMNode_str (new ustring (b, e)));
	    }
	    mode = EOL;
	    b = e;
	}
	if (mode == EOL) {
	    ans.append (row.release ());
	    mode = BOL;
	}
    }
    if (mode == COL) {
	ans.append (row.release ());
	mode = BOL;
    }
    return mlenv->retval = ans.release ();
}

static void  getIndex (MNode* header, size_t& ikey, size_t& ival) {
    ikey = 0;
    ival = 0;
    for (size_t i = 0; i < header->vectorSize (); ++ i) {
	MNode*  c = header->vectorGet (i);
	if (isStr (c)) {
	    if (match (c->to_string (), CharConst ("_field"))) {
		ikey = i;
	    } else if (match (c->to_string (), CharConst ("_value"))) {
		ival = i;
	    }
	}
    }
}

static void  makeRecord (MNodePtr& doc, MNode* vec, MNode* header, MNodePtr& header2, size_t ikey, size_t ival) {
    static const ustring  kN (CharConst ("_n"));
    MNode*  k = vec->vectorGet (ikey);
    if (doc ()) {
	MNode*  n = doc ()->tableGet (kN);
	if (n && n->isInt ()) {
	    ++ n->num;
	}
	if (k && k->isStr ()) {
	    MNode*  v = vec->vectorGet (ival);
	    MNode*  o = doc ()->tableGet (to_string (k));
	    doc ()->tablePut (to_string (k), v);
	    if (! header2 ()->tableGet (to_string (k)))
		header2 ()->tablePut (to_string (k), mlTrue);
	}
    } else {
	doc = newMNode_table ();
	doc ()->tablePut (kN, newMNode_int64 (1));
	for (size_t i = 3; i < header->vectorSize (); ++ i) {
	    if (i == ikey) {
		if (k && k->isStr ()) {
		    MNode*  v = vec->vectorGet (ival);
		    doc ()->tablePut (to_string (k), v);
		    if (! header2 ()->tableGet (to_string (k)))
			header2 ()->tablePut (to_string (k), mlTrue);
		}
	    } else if (i == ival) {
		// skip
	    } else {
		doc ()->tablePut (to_string (header->vectorGet (i)), vec->vectorGet (i));
	    }
	}
    }
}

static MNode*  vslice (size_t offset, MNode* vec) {
    MNode*  ans = newMNode_vector ();
    for (size_t i = offset; i < vec->vectorSize (); ++ i) {
	ans->vectorPush (vec->vectorGet (i));
    }
    return ans;
}

static MNode*  decode_sub (size_t offset, MNode* an_group, MNode* an_type, MNode* header, MNode* recname, MNodePtr& body) {
    MNodePtr  ml;
    ml = newMNode_vector ();
    // body
    ml ()->vectorPush (body.release ());
    // keys
    if (an_group && header) {
	MNodePtr  keys;
	keys = newMNode_vector ();
	for (size_t i = 3; i < an_group->vectorSize (); ++ i) {
	    MNode*  a = an_group->vectorGet (i);
	    MNode*  b = header->vectorGet (i);
	    if (isStr (a) && match (a->to_string (), CharConst ("true"))) {
		keys()->vectorPush (b);
	    }
	}
	ml ()->vectorPush (keys.release ());
    } else {
	ml ()->vectorPush (NULL);
    }
    // header
    if (offset == 0 || header == NULL) {
	ml ()->vectorPush (header);
    } else {
	ml ()->vectorPush (vslice (offset, header));
    }
    // type
    if (offset == 0 || an_type == NULL) {
	ml ()->vectorPush (an_type);
    } else {
	ml ()->vectorPush (vslice (offset, an_type));
    }
    // record_name
    ml ()->vectorPush (recname);
    //
    return ml.release ();
}

/*DOC:
===a-csv-to-record===
 (a-csv-to-record #merge VECTOR_OF_VECTOR) -> [(VECTOR_OF_TABLE KEY_VECTOR HEADER_VECTOR TYPE_VECTOR TABLE_NAME) ... ]

*/
//#XAFUNC	a-csv-to-record	ml_a_csv_to_record
//#XWIKIFUNC	a-csv-to-record
MNode*  ml_a_csv_to_record (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    kwParam  kwList[] = {{CharConst ("merge"), EV_LIST},		// 0
			 {NULL, 0, EV_END}};
    MNodePtr  kwParams[1];
    evalParams (fev, mlenv, cell, posList, posParams, kwList, kwParams);
    MNode*  mat = posParams[0]();
    bool  fmerge = to_bool (kwParams[0]());
    enum {
	M_START,
//	M_ANNOTATE,
//	M_HEAD,
	M_BODY,
    }  mode = M_START;
    if (isNil (mat)) {
	return newMNode_vector ();
    } else if (mat->isVector ()) {
    } else {
	throw (mat->dump_string_short () + ": bad type.");
    }
    MNodePtr  ans;
    ans = newMNode_vector ();
    MNode*  an_group = NULL;
    MNode*  an_group_1 = NULL;
    MNode*  an_type = NULL;
    MNode*  an_type_1 = NULL;
    MNode*  header = NULL;
    MNode*  header_1 = NULL;
    MNode*  tabname = NULL;
    MNode*  tabname_0 = NULL;
    MNodePtr  body;
    for (size_t it = 0; it < mat->vectorSize (); ++ it) {
	MNode*  row = mat->vectorGet (it);
	if (row && row->isVector ()) {
	    if (row->vectorSize () > 0) {
		if (mode == M_START) {
		    MNode*  c = row->vectorGet (0);
		    if (isStr (c) && matchHead (c->to_string (), CharConst ("#"))) {
			// annotation
			if (match (c->to_string (), CharConst ("#datatype"))) {
			    an_type = row;
			} else if (match (c->to_string (), CharConst ("#group"))) {
			    an_group = row;
			} else if (match (c->to_string (), CharConst ("#default"))) {
			    // skip
			} else {
			    // skip
			}
		    } else {
			// header
			header = row;
			mode = M_BODY;
			if (body() == NULL)
			    body = newMNode_vector ();
			tabname = NULL;
			if (! an_group_1 && ! an_type_1 && ! header_1) {
			    an_group_1 = an_group;
			    an_type_1 = an_type;
			    header_1 = header;
			}
		    }
		} else if (header) {
		    // body
		    if (! tabname) {
			tabname = row->vectorGet (1);
			if (fmerge && tabname_0 && ! equal (tabname_0, tabname)) {
			    ans()->vectorPush (decode_sub (3, an_group_1, an_type_1, header_1, tabname_0, body));
			    an_group_1 = an_group;
			    an_type_1 = an_type;
			    header_1 = header;
			    body = newMNode_vector ();
			}
		    }
		    MNodePtr  doc;
		    doc = newMNode_table ();
		    for (size_t i = 3; i < header->vectorSize (); ++ i) {
			MNode*  k = header->vectorGet (i);
			MNode*  v = row->vectorGet (i);
			if (k && k->isStr ()) {
			    doc ()->tablePut (k->to_string (), v);
			}
		    }
		    body ()->vectorPush (doc.release ());
		}
	    } else {
		// 空行
		if (mode == M_BODY) {
		    if (! fmerge) {
			ans()->vectorPush (decode_sub (3, an_group, an_type, header, tabname, body));
			an_group_1 = NULL;
			an_type_1 = NULL;
			header_1 = NULL;
			body = NULL;
		    }
		    an_group = NULL;
		    an_type = NULL;
		    header = NULL;
		    tabname_0 = tabname;
		    mode = M_START;
		}
	    }
	} else {
	    // vectorではない。
	    throw (mat->dump_string_short () + ": bad type.");
	}
    }
    if (mode == M_BODY || (fmerge && mode == M_START && body ())) {
	if (fmerge) {
	    ans()->vectorPush (decode_sub (3, an_group_1, an_type_1, header_1, tabname, body));
	} else {
	    ans()->vectorPush (decode_sub (3, an_group, an_type, header, tabname, body));
	}
	an_group = NULL;
	an_type = NULL;
	an_group_1 = NULL;
	an_type_1 = NULL;
	header_1 = NULL;
	tabname_0 = tabname;
	mode = M_START;
    }

    return mlenv->retval = ans.release ();
}

/*DOC:
===a-csv-decode===
 (a-csv-decode VECTOR_OF_VECTOR) -> [(VECTOR_OF_VECTOR KEY_VECTOR HEADER_VECTOR TYPE_VECTOR TABLE_NAME) ... ]

*/
//#XAFUNC	a-csv-decode	ml_a_csv_decode
//#XWIKIFUNC	a-csv-decode
MNode*  ml_a_csv_decode (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    evalParams (fev, mlenv, cell, posList, posParams);
    MNode*  mat = posParams[0]();
    enum {
	M_START,
//	M_ANNOTATE,
//	M_HEAD,
	M_BODY,
    }  mode = M_START;
    if (isNil (mat)) {
	return newMNode_vector ();
    } else if (mat->isVector ()) {
    } else {
	throw (mat->dump_string_short () + ": bad type.");
    }
    MNodePtr  ans;
    MNode*  an_group = NULL;
    MNode*  an_type = NULL;
    MNode*  header = NULL;
    MNode*  tabname = NULL;
    MNodePtr  body;
    ans = newMNode_vector ();
    for (size_t it = 0; it < mat->vectorSize (); ++ it) {
	MNode*  row = mat->vectorGet (it);
	if (row && row->isVector ()) {
	    if (row->vectorSize () > 0) {
		if (mode == M_START) {
		    MNode*  c = row->vectorGet (0);
		    if (isStr (c) && matchHead (c->to_string (), CharConst ("#"))) {
			// annotation
			if (match (c->to_string (), CharConst ("#datatype"))) {
			    an_type = row;
			} else if (match (c->to_string (), CharConst ("#group"))) {
			    an_group = row;
			} else if (match (c->to_string (), CharConst ("#default"))) {
			} else {
			}
		    } else {
			// header
			header = row;
			mode = M_BODY;
			body = newMNode_vector ();
			tabname = NULL;
		    }
		} else {
		    if (! tabname)
			tabname = row->vectorGet (1);
		    body()->vectorPush (row);
		}
	    } else {
		// 空行
		if (mode == M_BODY) {
		    ans()->vectorPush (decode_sub (0, an_group, an_type, header, tabname, body));
		    an_group = NULL;
		    an_type = NULL;
		    header = NULL;
		    mode = M_START;
		}
	    }
	} else {
	    // vectorではない。
	    throw (mat->dump_string_short () + ": bad type.");
	}
    }
    if (mode == M_BODY) {
	ans()->vectorPush (decode_sub (0, an_group, an_type, header, tabname, body));
	an_group = NULL;
	an_type = NULL;
	header = NULL;
	mode = M_START;
    }

    return mlenv->retval = ans.release ();
}

