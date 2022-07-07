#include "ml-xml.h"
#include "ml.h"
#include "mlenv.h"
#include "formfile.h"
#include "motorenv.h"
#include "motoroutput.h"
#include "ustring.h"
#include "expr.h"
#include "utf8.h"
#include "filemacro.h"
#include <expat.h>
#include <exception>
#include <vector>
#include <boost/scoped_ptr.hpp>

/*DOC:
==xml module==

*/

class  expat {
public:
    XML_Parser  xp;
    void*  buff;
    static const size_t  buffsize = 65536;

    expat (bool mybuffer = true) {
	xp = XML_ParserCreate (NULL);
	if (mybuffer)
	    buff = XML_GetBuffer (xp, buffsize);
	else
	    buff = NULL;
    };
    virtual  ~expat () {
	XML_ParserFree (xp);
	xp = NULL;
	buff = NULL;
    };
    virtual int  parse (size_t len, int isfinal) {
	return XML_ParseBuffer (xp, len, isfinal);
    };
    virtual int  parse (const char* s, size_t len, int isfinal) {
	return XML_Parse (xp, s, len, isfinal);
    };
    virtual void  setUserData (void* userData) {
	XML_SetUserData(xp, userData);
    };
    virtual void  setElementHandler (XML_StartElementHandler start, XML_EndElementHandler end, XML_CharacterDataHandler text) {
	XML_SetElementHandler (xp, start, end);
	XML_SetCharacterDataHandler (xp, text);
    };
    virtual enum XML_Error  errorCode () {
	return XML_GetErrorCode (xp);
    };
};

class  XmlReaderData {
public:
    MNodePtr  ans;
    typedef  enum {
	M_START,
	M_TEXT,
	M_CHILD,
    }  mode_t;
    std::vector<MNode*>  ptr;
    std::vector<mode_t>  mode;
    ustring  text;

    XmlReaderData () {};
    virtual  ~XmlReaderData () {
	while (ptr.size () > 0) {
	    delete ptr.back ();
	    ptr.pop_back ();
	}
    };

    virtual void  newNode (const XML_Char* name, const XML_Char** atts) = 0;
    virtual void  addText (const ustring& s) = 0;
    virtual void  endNode () = 0;
};

class  XmlReaderData_rg: public XmlReaderData {
public:
    XmlReaderData_rg () {};
    virtual  ~XmlReaderData_rg () {};

    virtual void  newNode (const XML_Char* name, const XML_Char** atts) {
	MNodeList  a;
	a.append (newMNode_sym (new ustring (fixUTF8 (ustring (name)))));
	a.append (NULL);
	for (; *atts; ) {
	    MNode*  c = new MNode;
	    c->set_car (newMNode_sym (new ustring (fixUTF8 (ustring (*atts)))));
	    atts ++;
	    c->set_cdr (newMNode_str (new ustring (fixUTF8 (ustring (*atts)))));
	    atts ++;
	    a.append (c);
	}
	ptr.push_back (a.release ());
	mode.push_back (M_START);
	text.resize (0);
    };
    virtual void  addText (const ustring& s) {
	mode_t  m = mode.back ();

	if (m == M_START || m == M_TEXT) {
	    text.append (s);
	    mode.back () = M_TEXT;
	}
    };
    virtual void  endNode () {
	MNode*  a = ptr.back ();
	mode_t  m = mode.back ();

	ptr.pop_back ();
	mode.pop_back ();

	switch (m) {
	case M_START:
	    break;
	case M_TEXT:
	    a->cdr ()->set_car (newMNode_str (new ustring (text)));
	    break;
	case M_CHILD:
	    break;
	default:;
	}
	if (ptr.size () == 0) {
	    ans = a;
	} else {
	    if (! ptr.back ()->cdr ()->car ()) {
		MNode*  c = new MNode;
		c->set_car (a);
		ptr.back ()->cdr ()->set_car (c);
	    } else if (ptr.back ()->cdr ()->car ()->isCons ()) {
		MNode*  c = ptr.back ()->cdr ()->car ();
		MNode*  d = new MNode;

		while (c->cdr ()) {
		    c = c->cdr ();
		}
		c->set_cdr (d);
		d->set_car (a);
	    } else {
		assert (0);
	    }
	    mode.back () = M_CHILD;
	}
    };
};

class  XmlReaderData_sxml: public XmlReaderData {
public:
    XmlReaderData_sxml () {};
    virtual  ~XmlReaderData_sxml () {};

    virtual void  newNode (const XML_Char* name, const XML_Char** atts) {
	MNodeList  a;
	a.append (newMNode_sym (new ustring (fixUTF8 (ustring (name)))));
	if (*atts) {
	    MNodeList  b;
	    b.append (newMNode_sym (new ustring (CharConst ("@"))));
	    for (; *atts; ) {
		MNode*  k = newMNode_sym (new ustring (fixUTF8 (ustring (*atts))));
		atts ++;
		MNode*  v = newMNode_str (new ustring (fixUTF8 (ustring (*atts))));
		atts ++;
		b.append (newMNode_cons (k, newMNode_cons (v)));
	    }
	    a.append (b.release ());
	}
	ptr.push_back (a.release ());
	mode.push_back (M_START);
	text.resize (0);
    };
    virtual void  addText (const ustring& s) {
	mode_t  m = mode.back ();

	if (m == M_START || m == M_TEXT) {
	    text.append (s);
	    mode.back () = M_TEXT;
	}
    };
    virtual void  endNode () {
	MNode*  a = ptr.back ();
	mode_t  m = mode.back ();

	ptr.pop_back ();
	mode.pop_back ();

	switch (m) {
	case M_START:
	    break;
	case M_TEXT:
	    a->append_cdr (newMNode_cons (newMNode_str (new ustring (text))));
	    break;
	case M_CHILD:
	    break;
	default:;
	}
	if (ptr.size () == 0) {
	    ans = a;
	} else {
	    ptr.back ()->append_cdr (newMNode_cons (a));
	    mode.back () = M_CHILD;
	}
    };
};

class  XmlReaderData_txml: public XmlReaderData {
public:
    XmlReaderData_txml () {};
    virtual  ~XmlReaderData_txml () {};

    virtual void  newNode (const XML_Char* name, const XML_Char** atts) {
	MNodeList  a;
	a.append (newMNode_sym (new ustring (fixUTF8 (ustring (name)))));
	a.append (NULL);
	if (atts && *atts) {
	    MNode*  c = newMNode_table ();
	    a.append (c);
	    for (; *atts; ) {
		ustring  key = fixUTF8 (ustring (*atts ++));
		ustring  val = fixUTF8 (ustring (*atts ++));
		c->tablePut (key, newMNode_str (new ustring (val)));
	    }
	}
	ptr.push_back (a.release ());
	mode.push_back (M_START);
	text.resize (0);
    };
    virtual void  addText (const ustring& s) {
	mode_t  m = mode.back ();

	if (m == M_START || m == M_TEXT) {
	    text.append (s);
	    mode.back () = M_TEXT;
	}
    };
    virtual void  endNode () {
	MNode*  a = ptr.back ();
	mode_t  m = mode.back ();

	ptr.pop_back ();
	mode.pop_back ();

	switch (m) {
	case M_START:
	    break;
	case M_TEXT:
	    a->cdr ()->set_car (newMNode_str (new ustring (text)));
	    break;
	case M_CHILD:
	    break;
	default:;
	}
	if (ptr.size () == 0) {
	    ans = a;
	} else {
	    MNode*  b = ptr.back ()->cdr ();
	    MNode*  c = b->car ();
	    if (! c) {
		c = newMNode_vector ();
		b->set_car (c);
	    }
	    c->vectorPush (a);
	    mode.back () = M_CHILD;
	}
    };
};


static void  xmlstart (void* data, const XML_Char* name, const XML_Char** atts) {
    XmlReaderData*  reader = (XmlReaderData*)data;
    reader->newNode (name, atts);
}

static void  xmlend (void* data, const XML_Char* name) {
    XmlReaderData*  reader = (XmlReaderData*)data;
    reader->endNode ();
}

static void  xmltext (void *data, const XML_Char *s, int len) {
    XmlReaderData*  reader = (XmlReaderData*)data;
    reader->addText (ustring (s, len));
}

class  XMLType {
public:
    typedef enum {
	TYPE_RG,
	TYPE_SXML,
	TYPE_TXML,
    }  type_t;

    type_t  type;

    XMLType () {
	type = TYPE_RG;
    };
    virtual  ~XMLType () {};

    void  setParam (MlEnv* mlenv, MNode* p1, MNode* p2) {
	if (p1 && eval_bool (p1, mlenv))
	    type = TYPE_SXML;
	else if (p2 && eval_bool (p2, mlenv))
	    type = TYPE_TXML;
    };
    XmlReaderData*  createReader () {
	switch (type) {
	case TYPE_SXML:
	    return new XmlReaderData_sxml;
	case TYPE_TXML:
	    return new XmlReaderData_txml;
	default:
	    return new XmlReaderData_rg;
	}
    };
    void  xmlWrite (MotorOutput* out, MNode* list) {
	switch (type) {
	case TYPE_SXML:
	    xmlWrite_sxml (out, list);
	    break;
	case TYPE_TXML:
	    xmlWrite_txml (out, list);
	    break;
	default:
	    xmlWrite_rg (out, list);
	}
    };
    void  xmlWrite_rg (MotorOutput* out, MNode* list) {
	if (isCons (list) && isSym (list->car ())) {
	    MNode*  c;
	    MNode*  d;
	    const ustring*  name = ptr_symbol (list->car ());
	    list = list->cdr ();
	    if (isCons (list)) {
		out->out_raw (CharConst ("<"))->out_toHTML_noCtrl (*name);
		c = list->car ();
		d = list->cdr ();
		while (isCons (d)) { // attribute
		    if (isCons (d->car ())) {
			out->out_raw (uSPC)
			    ->out_toHTML_noCtrl (to_string (d->car ()->car()))
			    ->out_raw (CharConst ("=\""))
			    ->out_toHTML_noCtrl (to_string (d->car ()->cdr ()))
			    ->out_raw (uQ2);
		    } else {
			out->out_raw (uSPC)
			    ->out_toHTML_noCtrl (to_string (d->car ()))
			    ->out_raw (CharConst ("=\""))
			    ->out_toHTML_noCtrl (to_string (d->car ()))
			    ->out_raw (uQ2);
		    }
		    d = d->cdr ();
		}
		if (c) {
		    out->out_raw (CharConst (">"));
		    if (isCons (c)) {
			out->out_raw (uLF);
			while (isCons (c)) {
			    xmlWrite_rg (out, c->car ());
			    c = c->cdr ();
			}
		    } else if (isStr (c)) {
			out->out_toHTML (*c->value_str ());
		    } else {
			out->out_toHTML (to_string (c));
		    }
		    out->out_raw (CharConst ("</"))
			->out_toHTML_noCtrl (*name)
			->out_raw (CharConst (">\n"));
		} else {
		    out->out_raw (CharConst ("/>\n"));
		}
	    }
	}
    };
    void  xmlWrite_sxml (MotorOutput* out, MNode* list) {
	static ustring  uAt (CharConst ("@"));
	
	if (isCons (list) && isSym (list->car ())) {
	    MNode*  c;
	    MNode*  d;
	    MNode*  e;
	    const ustring*  name = ptr_symbol (list->car ());
	    list = list->cdr ();
	    out->out_raw (CharConst ("<"))->out_toHTML_noCtrl (*name);
	    if (isCons (list) && isCons (c = list->car ())
		&& isSym (d = c->car ()) && *ptr_symbol (d) == uAt) { // attribute
		c = c->cdr ();
		list = list->cdr ();
		while (isCons (c)) {
		    d = c->car ();
		    c = c->cdr ();
		    if (isCons (d)) {
			out->out_raw (uSPC)
			    ->out_toHTML_noCtrl (to_string (d->car ()))
			    ->out_raw (CharConst ("=\""));
			e = d->cdr ();
			if (isNil (e) || ! isCons (e)) {
			    out->out_raw (uQ2);
			} else {
			    out->out_toHTML_noCtrl (to_string (e->car ()))
				->out_raw (uQ2);
			}
		    } else {
			out->out_raw (uSPC)
			    ->out_toHTML_noCtrl (to_string (d))
			    ->out_raw (CharConst ("=\""))
			    ->out_toHTML_noCtrl (to_string (d))
			    ->out_raw (uQ2);
		    }
		}
	    }
	    if (isNil (list)) {
		out->out_raw (CharConst ("/>\n"));
	    } else {
		out->out_raw (CharConst (">"));
		c = list->car ();
		if (isCons (c)) {
		    out->out_raw (uLF);
		    while (isCons (list)) {
			xmlWrite_sxml (out, list->car ());
			list = list->cdr ();
		    }
		} else {
		    out->out_toHTML (to_string (c));
		}
		out->out_raw (CharConst ("</"))
		    ->out_toHTML_noCtrl (*name)
		    ->out_raw (CharConst (">\n"));
	    }
	}
    };
    void  xmlWrite_txml (MotorOutput* out, MNode* list) {
	MNode*  c;
	MNode*  d;
	if (isCons (list) && isSym (c = list->car ())) {
	    const ustring*  name = ptr_symbol (c);
	    list = list->cdr ();
	    out->out_raw (CharConst ("<"))->out_toHTML_noCtrl (*name);
	    if (isCons (list)) {
		if (isTable (c = list->car ()) || (isCons (c = list->cdr ()) && isTable (c = c->car ()))) {
		    MotorVar::iterator  b = c->value_table ()->begin ();
		    MotorVar::iterator  e = c->value_table ()->end ();
		    for (; b != e; ++ b) {
			out->out_raw (uSPC)
			    ->out_toHTML_noCtrl ((*b).first)
			    ->out_raw (CharConst ("=\""))
			    ->out_toHTML_noCtrl (to_string ((*b).second ()))
			    ->out_raw (uQ2);
		    }
		}

		c = list->car ();
		if (isVector (c)) {
		    int  n = c->vectorSize ();
		    if (n > 0) {
			out->out_raw (CharConst (">\n"));
			for (int ix = 0; ix < n; ++ ix) {
			    xmlWrite_txml (out, c->vectorGet (ix));
			}
			out->out_raw (CharConst ("</"))
			    ->out_toHTML_noCtrl (*name)
			    ->out_raw (CharConst (">\n"));
		    } else {
			out->out_raw (CharConst ("/>\n"));
		    }
		} else if (isTable (c) || isNil (c)) {
		    out->out_raw (CharConst ("/>\n"));
		} else {
		    out->out_raw (CharConst (">"))
			->out_toHTML (to_string (c))
			->out_raw (CharConst ("</"))
			->out_toHTML_noCtrl (*name)
			->out_raw (CharConst (">\n"));
		}
	    } else {
		out->out_raw (CharConst ("/>\n"));
	    }
	}
    };
};

/*DOC:
===xml-read===
 (xml-read TEXT [#sxml] [#txml]) -> LIST

*/
//#XAFUNC	xml-read	ml_xml_read
//#XWIKIFUNC	xml-read
MNode*  ml_xml_read (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    kwParam  kwList[] = {{CharConst ("sxml"), EV_LIST}, // 0
			 {CharConst ("txml"), EV_LIST}, // 1
			 {NULL, 0, EV_END}};
    MNodePtr  kwParams[2];
    evalParams (fev, mlenv, cell, posList, posParams, kwList, kwParams);
    ustring  text = to_string (posParams[0]());
    XMLType  fxml;
    if (to_bool (kwParams[0]())) {
	fxml.type = XMLType::TYPE_SXML;
    } else if (to_bool (kwParams[1]())) {
	fxml.type = XMLType::TYPE_TXML;
    }
    boost::scoped_ptr<XmlReaderData>  data;
    expat  exp (false);
    int  rc;
    data.reset (fxml.createReader ());
    exp.setElementHandler (xmlstart, xmlend, xmltext);
    exp.setUserData (data.get ());
    rc = exp.parse (text.data (), text.length (), true);
#ifdef DEBUG
    if (! rc) {
	std::cerr << XML_ErrorString (exp.errorCode ()) << "\n";
    }
#endif /* DEBUG */

#ifdef DEBUG2
    std::cerr << data->ans ()->dump_string () << "\n";
#endif /* DEBUG */

    return mlenv->retval = data->ans.release ();
}

/*DOC:
===xml-read-file===
 (xml-read-file FILENAME [#sxml] [#txml]) -> LIST

 Element ::= (list TagName ElementBody Attribute* )
 Attribute ::= (cons AttributeName AttributeValue )
 ElementBody ::= TextData
 		| (list Element* )
 		| nil
 TagName ::= Symbol
 AttributeName ::= String
 AttributeValue ::= String
 TextData ::= String

*/
//#XAFUNC	xml-read-file	ml_xml_read_file
MNode*  ml_xml_read_file (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    kwParam  kwList[] = {{CharConst ("sxml"), EV_LIST}, // 0
			 {CharConst ("txml"), EV_LIST}, // 1
			 {NULL, 0, EV_END}};
    MNodePtr  kwParams[2];
    evalParams (fev, mlenv, cell, posList, posParams, kwList, kwParams);
    ustring  filename = to_string (posParams[0]());
    XMLType  fxml;
    if (to_bool (kwParams[0]())) {
	fxml.type = XMLType::TYPE_SXML;
    } else if (to_bool (kwParams[1]())) {
	fxml.type = XMLType::TYPE_TXML;
    }
    boost::scoped_ptr<XmlReaderData>  data;
    expat  exp;
    FileMacro  fp;
    size_t  s;
    data.reset (fxml.createReader ());
    if (mlenv->env->storedir.empty ())
	throw (uErrorNoStore);
    filename = mlenv->env->path_store_file (filename);

    if (fp.openRead (filename.c_str ())) {
	exp.setElementHandler (xmlstart, xmlend, xmltext);
	exp.setUserData (data.get ());
	while (1) {
	    s = fp.read (exp.buff, exp.buffsize);
	    exp.parse (s, (s == 0));
	    if (s == 0)
		break;
	}
	fp.close ();
    }

#ifdef DEBUG2
    std::cerr << data->ans ()->dump_string () << "\n";
#endif /* DEBUG */

    return mlenv->retval = data->ans.release ();
}

/*DOC:
===xml-output===
 (xml-output [#continue] [#sxml] [#txml] LIST) -> NIL

*/
//#XAFUNC	xml-output	ml_xml_output
MNode*  ml_xml_output (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    kwParam  kwList[] = {{CharConst ("continue"), EV_LIST}, // 0
			 {CharConst ("sxml"), EV_LIST},	    // 1
			 {CharConst ("txml"), EV_LIST},	    // 2
			 {NULL, 0, EV_END}};
    MNodePtr  kwParams[3];
    evalParams (fev, mlenv, cell, posList, posParams, kwList, kwParams);
    MNode*  xml = posParams[0]();
    bool  cflag = to_bool (kwParams[0]());
    XMLType  fxml;
    if (to_bool (kwParams[1]())) {
	fxml.type = XMLType::TYPE_SXML;
    } else if (to_bool (kwParams[2]())) {
	fxml.type = XMLType::TYPE_TXML;
    }
    if (! mlenv->env->responseDone)
	mlenv->env->standardResponse (ustring (CharConst (kMIME_XML)), ustring (uUTF8), uEmpty, false);
    MotorOutputCOut  out;
    out.out_raw (CharConst ("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"));
    fxml.xmlWrite (&out, xml);
    if (! cflag)
	mlenv->breakProg ();
    return NULL;
}

/*DOC:
===xml-output-string===
 (xml-output LIST [#sxml] [#txml]) -> STRING

*/
//#XAFUNC	xml-output-string	ml_xml_output_string
//#XWIKIFUNC	xml-output-string
MNode*  ml_xml_output_string (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    kwParam  kwList[] = {{CharConst ("sxml"), EV_LIST}, // 0
			 {CharConst ("txml"), EV_LIST}, // 1
			 {NULL, 0, EV_END}};
    MNodePtr  kwParams[2];
    evalParams (fev, mlenv, cell, posList, posParams, kwList, kwParams);
    MNode*  xml = posParams[0]();
    XMLType  fxml;
    if (to_bool (kwParams[0]())) {
	fxml.type = XMLType::TYPE_SXML;
    } else if (to_bool (kwParams[1]())) {
	fxml.type = XMLType::TYPE_TXML;
    }
    MotorOutputString  out;
    out.out_raw (CharConst ("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"));
    fxml.xmlWrite (&out, xml);
    return newMNode_str (new ustring (out.ans));
}

/*DOC:
===input-xml===
 (input-xml [#sxml] [#txml]) -> LIST

*/
//#XAFUNC	input-xml	ml_input_xml
MNode*  ml_input_xml (bool fev, MNode* cell, MlEnv* mlenv) {
    kwParam  kwList[] = {{CharConst ("sxml"), EV_LIST}, // 0
			 {CharConst ("txml"), EV_LIST}, // 1
			 {NULL, 0, EV_END}};
    MNodePtr  kwParams[2];
    evalParams (fev, mlenv, cell, NULL, NULL, kwList, kwParams);
    XMLType  fxml;
    if (to_bool (kwParams[0]())) {
	fxml.type = XMLType::TYPE_SXML;
    } else if (to_bool (kwParams[1]())) {
	fxml.type = XMLType::TYPE_TXML;
    }
    boost::scoped_ptr<XmlReaderData>  data;
    expat  exp (false);
    data.reset (fxml.createReader ());
    exp.setElementHandler (xmlstart, xmlend, xmltext);
    exp.setUserData (data.get ());
    exp.parse (mlenv->env->form->queryString.data (), mlenv->env->form->queryString.length (), true);

#ifdef DEBUG2
    std::cerr << data->ans ()->dump_string () << "\n";
#endif /* DEBUG */

    return mlenv->retval = data->ans.release ();
}
