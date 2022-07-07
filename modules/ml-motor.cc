#include "config.h"
#include "ml-motor.h"
#include "ml.h"
#include "ml-store.h"
#include "mlenv.h"
#include "motorenv.h"
#include "motoroutput.h"
#include "motoroutput-iconv.h"
#include "expr.h"
#include "util_const.h"
#include "util_check.h"
#include "util_mimetype.h"
#include "util_string.h"
#include "util_url.h"
#include "ustring.h"

/*DOC:
==HTMLMotor function==

*/

/*DOC:
===response-motor-file===
 (response-motor-file HTMLFILE [#serial | #named | #static] [:type MIMETYPE] [#error] [:code ENCODING] [#continue]) -> NIL
// (motor-file [:source-serial FILENAME | :source-named FILENAME | :source-static FILENAME] [:type MIMETYPE] [#error] [:code ENCODING] [#continue]) -> NIL

*/
//#XAFUNC	response-motor-file	ml_response_motor_file
MNode*  ml_response_motor_file (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    kwParam  kwList[] = {{CharConst ("serial"), EV_LIST}, // 0
			 {CharConst ("named"), EV_LIST},	// 1
			 {CharConst ("static"), EV_LIST}, // 2
			 {CharConst ("type"), EV_LIST},	// 3
			 {CharConst ("error"), EV_LIST},	// 4
			 {CharConst ("code"), EV_LIST},	// 5
			 {CharConst ("continue"), EV_LIST}, // 6
			 {NULL, 0, EV_END}};
    MNodePtr  kwParams[7];
    evalParams (fev, mlenv, cell, posList, posParams, kwList, kwParams);
    StoreType  storetype (mlenv);
    storetype.setStatic ();
    if (! isNil (posParams[0]()))
	storetype.setParam (to_string (posParams[0]()));
    if (to_bool (kwParams[0]())) { // serial
	storetype.setSerial ();
    } else if (to_bool (kwParams[1]())) { // named
	storetype.setNamed ();
    } else if (to_bool (kwParams[2]())) { // static
	storetype.setStatic ();
    } else {
	storetype.setStatic ();
    }
    ustring  type = to_asciiword (kwParams[3]()); // type
    bool  ferr = to_bool (kwParams[4]());	  // error
    ustring  encoding = to_asciiword (kwParams[5]()); // code
    bool  cflag = to_bool (kwParams[6]());	      // continue
    ustring  src = storetype.src ();
    if (src.size () == 0)
	throw (uErrorFilenameEmpty);
    if (ferr)
	mlenv->env->setErrorFlag ();
    if (encoding.empty ()) {
	if (type.size () == 0) {
	    mlenv->env->doMotorFile (src, false, mlenv->env->mimetype);
	} else {
	    mlenv->env->doMotorFile (src, false, type);
	}
    } else {
	MotorOutputIConvOStream  out (encoding.c_str ());
	if (type.size () == 0) {
	    mlenv->env->doMotorFile (src, false, mlenv->env->mimetype, &out);
	} else {
	    mlenv->env->doMotorFile (src, false, type, &out);
	}
    }
    if (! cflag)
	mlenv->breakProg ();
    return NULL;
}

/*DOC:
===response-file===
 (response-file FILENAME [#serial | #named | #static] [:type MIME_TYPE] [#inline] [:name NAME] [#base64] [#continue]) -> NIL

*/
//#XAFUNC	response-file	ml_response_file
MNode*  ml_response_file (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    kwParam  kwList[] = {{CharConst ("serial"), EV_LIST},   // 0
			 {CharConst ("named"), EV_LIST},    // 1
			 {CharConst ("static"), EV_LIST},   // 2
			 {CharConst ("type"), EV_LIST},	    // 3
			 {CharConst ("inline"), EV_LIST},   // 4
			 {CharConst ("name"), EV_LIST},	    // 5
			 {CharConst ("base64"), EV_LIST},   // 6
			 {CharConst ("continue"), EV_LIST}, // 7
			 {NULL, 0, EV_END}};
    MNodePtr  kwParams[8];
    evalParams (fev, mlenv, cell, posList, posParams, kwList, kwParams);
    StoreType  storetype (mlenv);
    storetype.setStatic ();
    if (! isNil (posParams[0]()))
	storetype.setParam (to_string (posParams[0]()));
    if (to_bool (kwParams[0]())) { // serial
	storetype.setSerial ();
    } else if (to_bool (kwParams[1]())) { // named
	storetype.setNamed ();
    } else if (to_bool (kwParams[2]())) { // static
	storetype.setStatic ();
    } else {
	storetype.setStatic ();
    }
    ustring  type = to_asciiword (kwParams[3]());  // type
    bool  finline = to_bool (kwParams[4]());	   // inline
    ustring  dispname = to_string (kwParams[5]()); // name
    bool  fbase64 = to_bool (kwParams[6]());	   // base64
    bool  cflag = to_bool (kwParams[7]());	   // continue
    ustring  src = storetype.src ();
    if (type.empty ()) {
	if (dispname.length () > 0) {
	    type = mimetype (getExt (dispname));
	} else {
	    type = mimetype (getExt (src));
	}
    } else if (! matchMimeType (type)) {
	type = mimetype (type);
    }
#ifdef DEBUG2
//    fprintf (stderr, "src:%s, type:%s, dispname:%s\n", src.c_str (), type.c_str (), dispname.c_str ());
#endif /* DEBUG */
    if (finline || dispname.length () > 0) {
	mlenv->env->outputFile (src, type, finline, dispname, fbase64);
    } else {
	mlenv->env->outputFile (src, type, fbase64);
    }
    if (! cflag)
	mlenv->breakProg ();
    return NULL;
}

/*DOC:
===response-header===
 (response-header TYPE [#inline] [#attachment] [:filename NAME] [:charset NAME]) -> NIL
:dispositionと:charsetは，排他。

*/
//#XAFUNC	response-header	ml_response_header
MNode*  ml_response_header (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    kwParam  kwList[] = {{CharConst ("inline"), EV_LIST},     // 0
			 {CharConst ("attachment"), EV_LIST}, // 1
			 {CharConst ("filename"), EV_LIST},   // 2
			 {CharConst ("charset"), EV_LIST},    // 3
			 {NULL, 0, EV_END}};
    MNodePtr  kwParams[4];
    evalParams (fev, mlenv, cell, posList, posParams, kwList, kwParams);
    ustring  type = to_asciiword (posParams[0]());
    bool  finline = to_bool (kwParams[0]());	  // inline
    bool  fattach = to_bool (kwParams[1]());	  // attachment
    ustring  name = to_string (kwParams[2]());	  // filename
    ustring  charset = to_string (kwParams[3]()); // charset
    if (! name.empty ())
	fattach = true;
    if (fattach)
	finline = false;
    if (type.empty ())
	throw (ustring (CharConst ("missing type.")));
    else if (! matchMimeType (type))
	type = mimetype (type);
    if (! mlenv->env->responseDone) {
	mlenv->env->standardResponse (type, charset, name, finline);
    }
    return NULL;
}

/*DOC:
===output-raw===
 (output-raw TEXT...) -> NIL
Text output asis.

*/
//#XAFUNC	output-raw	ml_output_raw
MNode*  ml_output_raw (bool fev, MNode* cell, MlEnv* mlenv) {
    MNodePtr  rest;
    evalParams (fev, mlenv, cell, EV_LIST, &rest);
    ustring  text;
    MNode*  args = rest ();
    while (args) {
	text.append (to_string (args->car ()));
	nextNode (args);
    }
    mlenv->env->output->out_toText (text);
    return NULL;
}

/*DOC:
===motor-item===
 (motor-item NUM...) -> NIL

*/
//#XAFUNC	motor-item	ml_motor_item
MNode*  ml_motor_item (bool fev, MNode* cell, MlEnv* mlenv) {
    MNodePtr  rest;
    evalParams (fev, mlenv, cell, EV_LIST, &rest);
    MNode*  args = rest ();
    if (! mlenv->env->responseDone)
	mlenv->env->standardResponse ();
    ustring  num;
    while (args) {
	num = to_string (args->car ());
	mlenv->env->motorItem (num);
	nextNode (args);
    }
    return NULL;
}

/*DOC:
===motor-output===
 (motor-output TEXT...) -> NIL
Text output via HTMLMotor as HTML template.

*/
//#XAFUNC	motor-output	ml_motor_output
MNode*  ml_motor_output (bool fev, MNode* cell, MlEnv* mlenv) {
    MNodePtr  rest;
    evalParams (fev, mlenv, cell, EV_LIST, &rest);
    MNode*  args = rest ();
    if (! mlenv->env->responseDone)
	mlenv->env->standardResponse ();
    while (args) {
	mlenv->env->motorText (to_string (args->car ()));
	nextNode (args);
    }
    return NULL;
}

/*DOC:
===motor-output-html===
 (motor-output-html TEXT...) -> NIL

*/
//#XAFUNC	motor-output-html	ml_motor_output_html
MNode*  ml_motor_output_html (bool fev, MNode* cell, MlEnv* mlenv) {
    MNodePtr  rest;
    evalParams (fev, mlenv, cell, EV_LIST, &rest);
    MNode*  args = rest ();
    if (! mlenv->env->responseDone)
	mlenv->env->standardResponse ();
    {
	MotorOutputString  out2;
	{
	    AutoSwapMotor  swap (mlenv->env->output, out2);
	    while (args) {
		mlenv->env->motorText (to_string (args->car ()));
		nextNode (args);
	    }
	}
	mlenv->env->output->out_toHTML (out2.ans);
    }
    return NULL;
}

/*DOC:
===motor-output-string===
 (motor-output-string TEXT...) -> STRING
Text output via HTMLMotor as STRING.

*/
//#XAFUNC	motor-output-string	ml_motor_output_string
MNode*  ml_motor_output_string (bool fev, MNode* cell, MlEnv* mlenv) {
    MNodePtr  rest;
    evalParams (fev, mlenv, cell, EV_LIST, &rest);
    MNode*  args = rest ();
    MotorOutputString  out;
    {
	AutoSwapMotor  swap (mlenv->env->output, out);
	while (args) {
	    mlenv->env->motorText (to_string (args->car ()));
	    nextNode (args);
	}
    }
    return newMNode_str (new ustring (out.ans));
}

/*DOC:
===forbidden===
 (forbidden [#continue | :continue BOOL]) -> NIL
403 Forbiddenレスポンスを出力する。
continueオプションを付けない場合，以降のファンクションの実行を中断する。

*/
//#XAFUNC	forbidden	ml_forbidden
MNode*  ml_forbidden (bool fev, MNode* cell, MlEnv* mlenv) {
    kwParam  kwList[] = {{CharConst ("continue"), EV_LIST},
			 {NULL, 0, EV_END}};
    MNodePtr  kwParams[1];
    evalParams (fev, mlenv, cell, NULL, NULL, kwList, kwParams);
    bool  cflag = to_bool (kwParams[0]());
    if (! mlenv->env->responseDone)
	mlenv->env->forbiddenResponse ();
    mlenv->env->setErrorFlag ();
    if (! cflag)
	mlenv->breakProg ();
    return NULL;
}

/*DOC:
===no-content===
 (no-content [#continue | :continue BOOL]) -> NIL
200 No Contentレスポンスを出力する。
continueオプションを付けない場合，以降のファンクションの実行を中断する。

*/
//#XAFUNC	no-content	ml_no_content
MNode*  ml_no_content (bool fev, MNode* cell, MlEnv* mlenv) {
    kwParam  kwList[] = {{CharConst ("continue"), EV_LIST},
			 {NULL, 0, EV_END}};
    MNodePtr  kwParams[1];
    evalParams (fev, mlenv, cell, NULL, NULL, kwList, kwParams);
    bool  cflag = to_bool (kwParams[0]());
    if (! mlenv->env->responseDone)
	mlenv->env->noContentResponse ();
    mlenv->env->setErrorFlag ();
    if (! cflag)
	mlenv->breakProg ();
    return NULL;
}

static void  location_sub (ustring& url, MNode* query, MlEnv* mlenv) {
    upair  scheme, delim, user, pass, host, port, rest;
    ustring  ans;

    if (checkURL (url, &scheme, &delim, &user, &pass, &host, &port, &rest)) {
	ans.assign (scheme.first, scheme.second).append (delim.first, delim.second);
	if (user.first != user.second) {
	    ans.append (user.first, user.second);
	    if (pass.first != pass.second) {
		ans.append (uColon).append (pass.first, pass.second);
	    }
	    ans.append (CharConst ("@"));
	}
	ans.append (host.first, host.second);
	if (port.first != port.second) {
	    ans.append (uColon).append (port.first, port.second);
	}
#ifdef QUERYENCODEALT
	ans.append (urlencode_path (rest.first, rest.second));
#else
	ans.append (percentEncode_path (rest.first, rest.second));
#endif
    } else {
#ifdef QUERYENCODEALT
	ans.assign (urlencode_path (url.begin (), url.end ()));
#else
	ans.assign (percentEncode_path (url.begin (), url.end ()));
#endif
    }
    if (query)
	ans.append (buildQuery (query));

    url = ans;
}

/*DOC:
===location===
 (location URL [:query '((NAME . VALUE)...) | #raw] [#continue]) -> NIL

Locationヘッダを含むレスポンスを出力する。

*/
//#XAFUNC	location	ml_location
MNode*  ml_location (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    kwParam  kwList[] = {{CharConst ("query"), EV_LIST},    // 0
			 {CharConst ("raw"), EV_LIST},	    // 1
			 {CharConst ("continue"), EV_LIST}, // 2
			 {NULL, 0, EV_END}};
    MNodePtr  kwParams[3];
    evalParams (fev, mlenv, cell, posList, posParams, kwList, kwParams);
    ustring  url = to_text1 (posParams[0]());
    MNode*  query = kwParams[0]();	   // query
    bool  fraw = to_bool (kwParams[1]());  // raw
    bool  cflag = to_bool (kwParams[2]()); // continue
    if (fraw) {
	if (! checkURLSafe (url))
	    throw (url + ": bad URL.");
    } else {
	location_sub (url, query, mlenv);
    }
    mlenv->env->location (url);
    if (! cflag)
	mlenv->breakProg ();
    return NULL;
}

/*DOC:
===location-html===
 (location-html URL [:query '((NAME . VALUE)...) | #raw] [#continue]) -> NIL

*/
//#XAFUNC	location-html	ml_location_html
MNode*  ml_location_html (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    kwParam  kwList[] = {{CharConst ("query"), EV_LIST},    // 0
			 {CharConst ("raw"), EV_LIST},	    // 1
			 {CharConst ("continue"), EV_LIST}, // 2
			 {NULL, 0, EV_END}};
    MNodePtr  kwParams[3];
    evalParams (fev, mlenv, cell, posList, posParams, kwList, kwParams);
    ustring  url = to_text1 (posParams[0]());
    MNode*  query = kwParams[0]();	   // query
    bool  fraw = to_bool (kwParams[1]());  // raw
    bool  cflag = to_bool (kwParams[2]()); // continue
    if (fraw) {
	if (! checkURLSafe (url))
	    throw (url + ": bad URL.");
    } else {
	location_sub (url, query, mlenv);
    }
    mlenv->env->location_html (url);
    if (! cflag)
	mlenv->breakProg ();
    return NULL;
}

/*DOC:
===set-cookie===
 (set-cookie NAME VALUE [:path PATH] [:domain DOMAIN] [:span TIME] [:limit TIME]) -> NIL

*/
//#XAFUNC	set-cookie	ml_set_cookie
MNode*  ml_set_cookie (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_LIST, EV_END};
    MNodePtr  posParams[2];
    kwParam  kwList[] = {{CharConst ("path"), EV_LIST},	  // 0
			 {CharConst ("domain"), EV_LIST}, // 1
			 {CharConst ("span"), EV_LIST},	  // 2
			 {CharConst ("limit"), EV_LIST},  // 3
			 {CharConst ("secure"), EV_LIST}, // 4
			 {NULL, 0, EV_END}};
    MNodePtr  kwParams[5];
    evalParams (fev, mlenv, cell, posList, posParams, kwList, kwParams);
    ustring  name = to_string (posParams[0]());
    ustring  value = to_string (posParams[1]());
    ustring  path = to_asciiword (kwParams[0]());
    ustring  domain = to_asciiword (kwParams[1]());
    time_t  span = to_int64 (kwParams[2]());
    time_t  limit = to_int64 (kwParams[3]());
    bool  fsecure = to_bool (kwParams[4]());
    if (span < 0)
	span = 0;
    if (limit < 0)
	limit = 0;
    if (name.length () > 128)
	throw (ustring (CharConst ("too long name.")));
    if (value.length () > 512)
	throw (ustring (CharConst ("too long value.")));
    mlenv->env->http.setCookie (name, value, path, span, limit, domain, fsecure, mlenv->env);
    return NULL;
}

/*DOC:
===set-header===
 (set-header NAME VALUE ...) -> NIL

*/
//#XAFUNC	set-header	ml_set_header
MNode*  ml_set_header (bool fev, MNode* cell, MlEnv* mlenv) {
    MNodePtr  rest;
    evalParams (fev, mlenv, cell, EV_LIST, &rest);
    MNode*  args = rest ();
    ustring  name;
    ustring  value;
    while (args) {
	name = to_string (args->car ());
	nextNode (args);
	if (args)
	    value = to_string (args->car ());
	else
	    value.resize (0);
	nextNode (args);
	mlenv->env->http.setHeader (name, value);
    }
    return NULL;
}
