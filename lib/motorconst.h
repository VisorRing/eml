#ifndef MOTORCONST_H
#define MOTORCONST_H

#define  rMOTORVAR	"[a-zA-Z_0-9][a-zA-Z_0-9+*-]*"
#define  rWIKIVAR	"[a-zA-Z_][a-zA-Z_0-9-]*"
#define  rWikiID	"[a-zA-Z_][a-zA-Z_0-9-]*"
#define  kWORD		"[a-zA-Z0-9_]"
#define  kWNAME		"[a-zA-Z_][a-zA-Z0-9_-]"
#define  kFName		"[^\\x00-\\x20./\\x7f-\\xff][^\\x00-\\x20/\\x7f-\\xff]{0,127}"
#define  rHTML		"https?://[a-zA-Z\\-]+(\\.[a-zA-Z\\-]+)*(:[0-9]+)?(/[!#%&+,\\-.0-9:<=>?@A-Z_a-z~]*)*"
#define  kDS		"/"
#define  kSubDB		"/db/"
#define  kSubStore	"/store/"
#define  kSubStorage	"/save/"
#define  kSubTemp	"/tmp/"
#define  kSubAuth	"/auth/"
#define  kBin		"/bin/"
#define  kEBin		"/ebin/"
#define  kEtc		"/etc/"
#define  kStoreSerial	"serial"
#ifdef BDB5
#define  kEXT_HASH	".db5"
#define  kEXT_BTREE	".btree5"
#define  kMimeTypeDB	"/etc/mimetype.db5"
#else
#define  kEXT_HASH	".db"
#define  kEXT_BTREE	".btree"
#define  kMimeTypeDB	"/etc/mimetype.db"
#endif
#define  kEXT_LOCK	".lock"
#define  kARRAYMAX	10000
#define  kIncludeMax	8
#define  kSearchName	"HMLSEARCHPATH"

#endif /* MOTORCONST_H */
