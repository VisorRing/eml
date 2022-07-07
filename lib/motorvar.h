#ifndef MOTORVAR_H
#define MOTORVAR_H

#include "ml.h"
#include "ustring.h"
#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

class  MotorSet: public boost::unordered_set<ustring> {
 public:
    MotorSet () {};
    virtual  ~MotorSet () {};
    virtual void  set (const ustring& name);
    virtual void  unset (const ustring& name);
    virtual bool  get (const ustring& name);
};

class  MotorVar: public boost::unordered_map<ustring, MNodePtr> {
 public:
    MotorVar () {};
    virtual  ~MotorVar () {};
    virtual void  setVar (const ustring& name, MNode* val);
    virtual MNode*  getVar (const ustring& name);
    virtual void  eraseVar (const ustring& name);
    virtual bool  defined (const ustring& name);
};

class  MotorErrorVar: public boost::unordered_map<ustring, bool> { /* XXX: use set. */
 public:
    MotorErrorVar () {};
    virtual  ~MotorErrorVar () {};
    virtual void  setVar (const ustring& name);
    virtual bool  getVar (const ustring& name);
};

class  MotorVector: public boost::ptr_vector<MNodePtr> {
 public:
    MotorVector () {};
    virtual  ~MotorVector () {};

    virtual void  push (MNode* val);
    virtual MNode*  pop ();
    virtual MNode*  get (size_type pos);	// 0-base
    virtual void  put (size_type pos, MNode* val);
    virtual void  unshift (MNode* val);
    virtual MNode*  shift ();
};

#endif /* MOTORVAR_H */
