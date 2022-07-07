#include "ml.h"
#include "util_const.h"
#include "motorconst.h"
#include "ustring.h"

ustring  uErrorWrongNumber (CharConst ("wrong number of argument."));
ustring  uErrorWrongType (CharConst ("wrong type of argument."));
ustring  uErrorDiv0 (CharConst ("divided by 0."));
ustring  uErrorBadArg (CharConst ("bad argument."));
ustring  uErrorSyntax (CharConst ("syntax error."));
ustring  uErrorNotImpl (CharConst ("not implemented."));
ustring  uErrorFilenameEmpty (CharConst ("file name is empty."));
ustring  uErrorCmdNameEmpty (CharConst ("command name is empty."));
ustring  uErrorVarNameEmpty (CharConst ("variable name is empty."));
ustring  uErrorNoStore (CharConst ("no serial store directory."));
ustring  uErrorNoStorage (CharConst ("no named store directory."));
ustring  uErrorInclNest (CharConst ("deeply nested."));
ustring  uErrorBadName (CharConst (": bad name."));
ustring  uErrorBadParam (CharConst (": bad parameter."));
ustring  uErrorBadValueN (CharConst ("bad value."));
ustring  uErrorBadValue (CharConst (": bad value."));
ustring  uErrorNotFound (CharConst (": not found."));
ustring  uErrorMissingDatastore (CharConst ("missing datastore parameter."));
ustring  uErrorBadParamDef (CharConst (": bad parameter definition."));
ustring  uErrorRegexp (CharConst ("regular expression error."));
const char*  uErrorNil = "nil data.";
const char*  uErrorBadType = ": bad type.";
const char*  uErrorBadCmd = ": bad command.";
const char*  uErrorCantOpen = ": can't open.";
const char*  uErrorBadFile = ": bad filename.";
const char*  uErrorBadMailAddr = ": bad mail address.";
const char*  uErrorBadDatastore = ": bad datastore name.";
const char*  uErrorFileSize = ": file size too large.";

ustring  uOne (CharConst ("1"));
ustring  uNil (CharConst ("nil"));
ustring  uNil2 (CharConst ("()"));
ustring  uLF (CharConst ("\n"));
ustring  uCRLF (CharConst (kCRLF));
ustring  uSPC (CharConst (" "));
ustring  uTAB (CharConst ("\t"));
ustring  uR (CharConst ("_R"));
ustring  uDash (CharConst ("-"));
ustring  uSlash (CharConst ("/"));
//ustring  uSlash2 (CharConst ("//"));
ustring  uBSlash (CharConst ("\\"));
ustring  uColon (CharConst (":"));
ustring  uDot (CharConst ("."));
ustring  uComma (CharConst (","));
ustring  uUScore (CharConst ("_"));
ustring  uEq (CharConst ("="));
ustring  uAmp (CharConst ("&"));
ustring  uQ2 (CharConst ("\""));
ustring  uBra2 (CharConst ("%{"));
ustring  uCket2 (CharConst ("}%"));
ustring  uEmpty;
ustring  uXSerial (CharConst ("XSerial"));
ustring  uLambda (CharConst ("lambda"));
ustring  uQuote (CharConst ("quote"));
ustring  uTrue (CharConst ("true"));
ustring  uFalse (CharConst ("false"));
ustring  uUTF8 (CharConst ("UTF-8"));
ustring  uNbsp (CharConst ("&nbsp;"));
//ustring  uTimeFormat (CharConst ("${Y}.${M}.${D}"));
ustring  uTimeFormat (CharConst ("%Y.%-m.%-d"));
ustring  uHttp (CharConst ("http"));
ustring  uHttps (CharConst ("https"));
ustring  uDefault (CharConst ("default"));
ustring  uAssoc (CharConst ("=>"));

const char*  MStr_a[] = {
    "Jan", "Feb", "Mar", "Apr",
    "May", "Jun", "Jul", "Aug",
    "Sep", "Oct", "Nov", "Dec"
};

const char*  MStr[] = {
    "January", "February", "March", "April",
    "May", "June", "July", "August",
    "September", "October", "November", "December"
};

const char*  WStr_a[] = {
    "Sun", "Mon", "Tue", "Wed",
    "Thu", "Fri", "Sat"
};

const char*  WStr[] = {
    "Sunday", "Monday", "Tuesday", "Wednesday",
    "Thursday", "Friday", "Saturday"
};

MNode*  mlTrue = NULL;
static MNodePtr  trueHolder;

class  ConstInit {
public:
    ConstInit () {
	trueHolder = mlTrue = newMNode_bool (true);
    };
    ~ConstInit () {};
};
static ConstInit  init;
