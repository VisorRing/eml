#include "wikienv.h"
#include "motorconst.h"
#include "ml.h"
#include "ustring.h"

void  MacroVar::setVar (const ustring& name, MNode* var, WikiLine::linevec* wl) {
    std::pair<MacroVar::iterator, bool>  x;
    erase (name);
    x = insert (value_type (name, WikiMacro ()));
    x.first->second.vars = var;
    x.first->second.wl = wl;
}

WikiMacro*  MacroVar::getVar (const ustring& name) {
    iterator  it = find (name);
    if (it == end ()) {
	return NULL;
    } else {
	return &it->second;
    }
}

bool  MacroVar::defined (const ustring& name) {
    iterator  it = find (name);
    return (it != end ());
}

void  ElementVar::setVar (const ustring& name, WikiLine* block) {
    std::pair<ElementVar::iterator, bool>  x;
    erase (name);
    x = insert (value_type (name, block));
}

WikiLine*  ElementVar::getVar (const ustring& name) {
    iterator  it = find (name);
    if (it == end ()) {
	return NULL;
    } else {
	return it->second;
    }
}

