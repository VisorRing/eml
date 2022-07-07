#ifndef MFTABLE_H
#define MFTABLE_H

#include "motor.h"
#include "ustring.h"
#include <boost/unordered_map.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <vector>

class  MlEnv;

typedef  struct {
    const char*  name;
    int  namelen;
    void  (*fn) (const std::vector<ustring>& args, MlEnv* mlenv);
}  MFTableVal;

typedef  struct {
    const char*  name;
    int  namelen;
    void  (*fn) (const std::vector<ustring>& args, const ustring& arg2, MlEnv* mlenv);
}  MFTable2Val;

class  MFTable: public boost::unordered_map<ustring, MFTableVal*> {
public:
//    boost::ptr_vector<MFTable>  iSTableV;
//    boost::unordered_map<ustring, MFTable*>  iSTableM;

    MFTable () {};
    MFTable (MFTableVal* t) {
	int  i;
	for (i = 0; t[i].name; i ++) {
	    insert (MFTable::value_type (ustring (t[i].name, t[i].namelen), &t[i]));
	}
    };
    virtual  ~MFTable () {};
};

class  MFTable2: public boost::unordered_map<ustring, MFTable2Val*> {
public:
//    boost::ptr_vector<MFTable2>  iSTableV;
//    boost::unordered_map<ustring, MFTable2*>  iSTableM;

    MFTable2 () {};
    MFTable2 (MFTable2Val* t) {
	int  i;
	for (i = 0; t[i].name; i ++) {
	    insert (MFTable2::value_type (ustring (t[i].name, t[i].namelen), &t[i]));
	}
    };
    virtual  ~MFTable2 () {};
};

extern MFTable  IMFTable;
extern MFTable2  IMFTable2;

#endif /* MFTABLE_H */
