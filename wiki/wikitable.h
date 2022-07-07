#ifndef WIKITABLE_H
#define WIKITABLE_H

#include "ustring.h"
#include <boost/unordered_map.hpp>
#include <vector>

class  MotorOutput;
class  WikiLine;
class  WikiFormat;
class  WikiMotorObjVec;
class  WikiMotorObjVecVec;

namespace WikiCmdTableSupport {
    typedef struct {
	const char*  name;
	size_t  namelen;
	const char*  elsename;
	size_t  elsenamelen;
	const char*  endname;
	size_t  endnamelen;
	void  (*fn) (WikiLine* wl, WikiFormat* wiki);
	void  (*fn_pass1) (WikiLine* wl, WikiFormat* wiki);
    }  wikicmd_t;
    typedef enum {
	WikiArg1,
	WikiArgM,
	WikiArgM2,
    }  wikifuncarg_t;
    typedef struct {
	const char*  name;
	size_t  namelen;
	wikifuncarg_t  atype;
	union {
	    bool  (*fn) ();
	    bool  (*fn1) (WikiMotorObjVec* arg, WikiMotorObjVec& out, WikiFormat* wiki);
	    bool  (*fnM) (WikiMotorObjVecVec* args, WikiMotorObjVec& out, WikiFormat* wiki);
	    bool  (*fnM2) (WikiMotorObjVecVec* args, WikiMotorObjVec* arg2, WikiMotorObjVec& out, WikiFormat* wiki);
	};
    }  wikifunc_t;
}

class  WikiCmdTable: public boost::unordered_map<ustring, WikiCmdTableSupport::wikicmd_t*> {
 public:
    WikiCmdTable (WikiCmdTableSupport::wikicmd_t* t) {
	int  i;
	for (i = 0; t[i].name; i ++) {
	    insert (value_type (ustring (t[i].name, t[i].namelen), &t[i]));
	}
    };
    virtual  ~WikiCmdTable () {};
};

class  WikiFuncTable: public boost::unordered_map<ustring, WikiCmdTableSupport::wikifunc_t*> {
public:
    WikiFuncTable (WikiCmdTableSupport::wikifunc_t* t) {
	int  i;
	for (i = 0; t[i].name; i ++) {
	    insert (value_type (ustring (t[i].name, t[i].namelen), &t[i]));
	}
    };
    virtual  ~WikiFuncTable () {};
};

extern WikiCmdTable  GWikiCmdTable;
extern WikiFuncTable  GWikiFuncTable;

#endif /* WIKITABLE_H */
