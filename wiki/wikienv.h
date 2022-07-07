#ifndef WIKIENV_H
#define WIKIENV_H

#include "motorvar.h"
#include "wikiformat.h"
#include "ustring.h"

class  MacroVar: public boost::unordered_map<ustring, WikiMacro> {
 public:
    MacroVar () {};
    virtual  ~MacroVar () {};
    virtual void  setVar (const ustring& name, MNode* var, WikiLine::linevec* wl);
    virtual WikiMacro*  getVar (const ustring& name);
    virtual bool  defined (const ustring& name);
};

class  ElementVar: public boost::unordered_map<ustring, WikiLine*> {
 public:
    ElementVar () {};
    virtual  ~ElementVar () {};
    virtual void  setVar (const ustring& name, WikiLine* block);
    virtual WikiLine*  getVar (const ustring& name);
};

class  WikiEnv {
 public:
    MotorVar  wikiFunc2;	// inline2 function
    MotorVar  wikiFunc;		// inline function
    MotorVar  wikiLink;		// link command
    MotorVar  wikiCmd;		// wiki command
    MotorVar  wikiCmd2;		// wiki command2
    MacroVar  wikiMacro;
    MotorSet  wikiGuestFunc;
    ElementVar  wikiElement;

    WikiEnv () {};
    virtual  ~WikiEnv () {};
};

#endif /* WIKIENV_H */
