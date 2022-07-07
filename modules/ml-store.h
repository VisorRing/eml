#ifndef ML_STORE_H
#define ML_STORE_H

#include "ustring.h"
#include "filemacro.h"
#include "ml.h"
#include "ml-id.h"

class  MNode;
class  MlEnv;

class  StoreType {
 public:
    enum {
	F_NONE,
	F_SERIAL,
	F_NAMED,
	F_STATIC,
	F_TEXT,
    }  type;

    MlEnv*  mlenv;
    ustring  param;
    ustring  encoding;

    StoreType (MlEnv* _mlenv);
    virtual  ~StoreType () {};

    virtual void  setParam (const ustring& name) {
	param = name;
    };
    virtual bool  operator () () {
	return type != F_NONE;
    };
    virtual bool  isEmpty () {
	return param.length () == 0;
    };
    virtual bool  isText () {
	return type == F_TEXT;
    };
    virtual void  setSerial () {
	type = F_SERIAL;
    };
    virtual void  setNamed () {
	type = F_NAMED;
    };
    virtual void  setStatic () {
	type = F_STATIC;
    };
    virtual void  setText () {
	type = F_TEXT;
    };
    virtual void  srcSerial (const ustring& name) {
	param = name;
	setSerial ();
    };
    virtual void  srcNamed (const ustring& name) {
	param = name;
	setNamed ();
    };
    virtual void  srcStatic (const ustring& name) {
	param = name;
	setStatic ();
    };
    virtual void  srcText (const ustring& name) {
	param = name;
	setText ();
    };
    virtual void  srcEncoding (const ustring& name) {
	encoding = name;
    };
    virtual const ustring  src (const ustring& name);
    virtual const ustring  src ();
    virtual ustring  read ();
};

class MLOpenFile: public MLFunc {
public:
    FileMacro  fp;
    MLOpenFile (MlEnv* _mlenv): MLFunc (cMLOpenFileID, _mlenv) {};
    virtual  ~MLOpenFile () {};
};

MNode*  newStoreSerial (MlEnv* mlenv);

MNode*  ml_new_xserial (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_set_xserial (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_read_file (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_write_file (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_delete_store (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_clean_store (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_datastore (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_datastore_progn (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_new_storage (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_set_storage (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_delete_storage (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_rename_storage (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_save_file (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_restore_file (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_delete_file (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_rename_file (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_response_motor (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_filesize (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_list_files (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_open_for_write (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_open_for_write_file_write (bool fev, MNode* cell, MlEnv* mlenv, MLFunc* mobj);

#endif /* ML_STORE_H */
