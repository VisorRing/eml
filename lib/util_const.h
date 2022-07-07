#ifndef UTIL_CONST_H
#define UTIL_CONST_H

#include "ustring.h"

#define  kCRLF	"\r\n"
#define  kQ2	"\""

class  MNode;

extern ustring  uErrorWrongNumber;
extern ustring  uErrorWrongType;
extern ustring  uErrorDiv0;
extern ustring  uErrorBadArg;
extern ustring  uErrorSyntax;
extern ustring  uErrorNotImpl;
extern ustring  uErrorFilenameEmpty;
extern ustring  uErrorCmdNameEmpty;
extern ustring  uErrorVarNameEmpty;
extern ustring  uErrorNoStore;
extern ustring  uErrorNoStorage;
extern ustring  uErrorInclNest;
extern ustring  uErrorBadName;
extern ustring  uErrorBadParam;
extern ustring  uErrorBadValueN;
extern ustring  uErrorBadValue;
extern ustring  uErrorNotFound;
extern ustring  uErrorMissingDatastore;
extern ustring  uErrorBadParamDef;
extern ustring  uErrorRegexp;

extern const char*  uErrorNil;
extern const char*  uErrorBadType;
extern const char*  uErrorBadCmd;
extern const char*  uErrorCantOpen;
extern const char*  uErrorBadFile;
extern const char*  uErrorBadMailAddr;
extern const char*  uErrorBadDatastore;
extern const char*  uErrorFileSize;

extern const char*  MStr_a[];
extern const char*  MStr[];
extern const char*  WStr_a[];
extern const char*  WStr[];

extern ustring  uOne;
extern ustring  uNil;
extern ustring  uNil2;
extern ustring  uLF;
extern ustring  uCRLF;
extern ustring  uSPC;
extern ustring  uTAB;
extern ustring  uR;
extern ustring  uDash;
extern ustring  uSlash;
//extern ustring  uSlash2;
extern ustring  uBSlash;
extern ustring  uColon;
extern ustring  uDot;
extern ustring  uComma;
extern ustring  uUScore;
extern ustring  uEq;
extern ustring  uAmp;
extern ustring  uQ2;
extern ustring  uBra2;
extern ustring  uCket2;
extern ustring  uEmpty;
extern ustring  uXSerial;
extern ustring  uLambda;
extern ustring  uQuote;
extern ustring  uTrue;
extern ustring  uFalse;
extern ustring  uUTF8;
extern ustring  uNbsp;
extern ustring  uTimeFormat;
extern ustring  uHttp;
extern ustring  uHttps;
extern ustring  uDefault;
extern ustring  uAssoc;

extern MNode*  mlTrue;

#endif /* UTIL_CONST_H */
