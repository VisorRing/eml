#ifndef FORMFILE_H
#define FORMFILE_H

#include "form.h"
#include "motorenv.h"
#include "filemacro.h"
#include <vector>
#include <utility>

class  CGIFormFile: public CGIForm {
 public:
    typedef std::pair<char*,char*>  part;
    typedef boost::unordered_map<int,int>  sary_t;
    typedef boost::unordered_map<int,ustring>  tary_t;

    ustring  tmpfile;
    uregex  re1;
    uregex  reN;
    FileMacro  fp;
    char*  mapdata;
    size_t  mapsize;
    sary_t  datamap;
    tary_t  typemap;
    std::vector<part>  parts;

    CGIFormFile () {
	mapdata = NULL;
    };
    virtual  ~CGIFormFile () {
	unlinkTmpFile ();
    };

    virtual int  partAt (int i);
    virtual ustring  typeAt (int i);
    virtual void  read_multipart (MotorEnv* env);
    virtual bool  saveData (MotorEnv* env);
    virtual void  unlinkTmpFile ();
    virtual void  searchPart (MotorEnv* env);
    virtual void  compileReg ();
    virtual void  readMimeHead (char*& b, char* e, ustring& disp, ustring& name, ustring& filename, ustring& type);
    virtual bool  saveFile (int i, const ustring& path, size_t max);
};

#endif /* FORMFILE_H */
