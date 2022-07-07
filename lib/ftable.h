#ifndef FTABLE_H
#define FTABLE_H

#include "ustring.h"
#include <boost/unordered_map.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

class  MNode;
class  MlEnv;
class  MLFunc;
class  FTable;
class  XFTable;

typedef struct {
    const char*  name;
    size_t  namelen;
    const char*  module;
    size_t  modlen;
    union {
	MNode*  (*fn) (MNode* arg, MlEnv* mlenv);
	MNode*  (*sfn) (MNode* arg, MlEnv* mlenv, MLFunc* mobj);
    };
    int  mlid;
    FTable*  stable;
}  FTableVal;
typedef struct {
    const char*  name;
    size_t  namelen;
    const char*  module;
    size_t  modlen;
    union {
	MNode*  (*fn) (bool fev, MNode* arg, MlEnv* mlenv);
	MNode*  (*sfn) (bool fev, MNode* arg, MlEnv* mlenv, MLFunc* mobj);
    };
    uint8_t  notrace_fn;
    uint8_t  notrace_rval;
    int  mlid;
    XFTable*  stable;
}  XFTableVal;

class  FTable: public boost::unordered_map<ustring, FTableVal*> {
public:
    FTable () {};
    FTable (FTableVal* t) {
	int  i;
	for (i = 0; t[i].name; i ++) {
	    insert (FTable::value_type (ustring (t[i].name, t[i].namelen), &t[i]));
	}
    };
    virtual  ~FTable () {};
};
class  XFTable: public boost::unordered_map<ustring, XFTableVal*> {
public:
    XFTable () {};
    XFTable (XFTableVal* t) {
	int  i;
	for (i = 0; t[i].name; i ++) {
	    insert (XFTable::value_type (ustring (t[i].name, t[i].namelen), &t[i]));
	}
    };
    virtual  ~XFTable () {};
};

class  MTable: public FTable {
public:
    boost::ptr_vector<FTable>  iSTableV;

    MTable (FTableVal* t, FTableVal* s): FTable (t) {
	int  i;
	FTable*  f;
	ustring  p;
	iterator  it;
	
	for (i = 0; t[i].name; i ++) {
	    f = new FTable;
	    iSTableV.push_back (f);
	    t[i].stable = f;
	}
	for (i = 0; s[i].name; i ++) {
	    p = ustring (s[i].module, s[i].modlen);
	    it = find (p);
	    if (it != end ()) {
		it->second->stable->insert (FTable::value_type (ustring (s[i].name, s[i].namelen), &s[i]));
	    }
	}
    };
    virtual  ~MTable () {};
};
class  XMTable: public XFTable {
public:
    boost::ptr_vector<XFTable>  iSTableV;

    XMTable (XFTableVal* t, XFTableVal* s): XFTable (t) {
	int  i;
	XFTable*  f;
	ustring  p;
	iterator  it;
	
	for (i = 0; t[i].name; i ++) {
	    f = new XFTable;
	    iSTableV.push_back (f);
	    t[i].stable = f;
	}
	for (i = 0; s[i].name; i ++) {
	    p = ustring (s[i].module, s[i].modlen);
	    it = find (p);
	    if (it != end ()) {
		it->second->stable->insert (XFTable::value_type (ustring (s[i].name, s[i].namelen), &s[i]));
	    }
	}
    };
    virtual  ~XMTable () {};
};

extern XFTable  GFTable;
extern XMTable  GMTable;
extern XFTable  GWikiFTable;

#endif /* FTABLE_H */
