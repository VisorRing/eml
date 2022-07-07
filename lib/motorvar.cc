#include "motorvar.h"
#include "ustring.h"
#include <iostream>

using namespace  std;

void  MotorSet::set (const ustring& name) {
    insert (name);
}

void  MotorSet::unset (const ustring& name) {
    MotorSet::iterator  it = find (name);
    if (it != end ())
	erase (it);
}

bool  MotorSet::get (const ustring& name) {
    MotorSet::iterator  it = find (name);
    return (it != end ());
}

void  MotorVar::setVar (const ustring& name, MNode* val) {
    std::pair<MotorVar::iterator, bool>  x;
    erase (name);
    x = insert (MotorVar::value_type (name, MNodePtr ()));
    x.first->second = val;
}

MNode*  MotorVar::getVar (const ustring& name) {
    MotorVar::iterator  it = find (name);
    if (it == end ()) {
	return NULL;
    } else {
	return it->second ();
    }
}

void  MotorVar::eraseVar (const ustring& name) {
#ifdef HEAPDEBUG_VERBOSE
    std::cerr << "MotorVar::eraseVar (" << name << ":(" << std::hex << getVar (name) << std::dec << ")";
    getVar (name)->dump (std::cerr);	// XXX
    std::cerr << ")\n";
#endif /* DEBUG */
    erase (name);
}

bool  MotorVar::defined (const ustring& name) {
    MotorVar::iterator  it = find (name);
    return (it != end ());
}

void  MotorErrorVar::setVar (const ustring& name) {
    erase (name);
    insert (boost::unordered_map<ustring, bool>::value_type (name, true));
}

bool  MotorErrorVar::getVar (const ustring& name) {
    MotorErrorVar::iterator  it = find (name);
    if (it == end ()) {
	return false;
    } else {
	return true;
    }
}

void  MotorVector::push (MNode* val) {
    MNodePtr*  p = new MNodePtr;
    *p = val;
    push_back (p);
}

MNode*  MotorVector::pop () {
    if (size () > 0) {
	MNodePtr  ans;
	ans = back () ();
	pop_back ();
	return ans.release ();
    } else {
	return NULL;
    }
}

MNode*  MotorVector::get (size_type pos) {
    // 0-base
    if (pos < size ()) {
	return at (pos) ();
    } else {
	return NULL;
    }
}

void  MotorVector::put (size_type pos, MNode* val) {
    if (pos < size ()) {
    } else {
	while (pos >= size ())
	    push (NULL);
    }
    at (pos) = val;
}

void  MotorVector::unshift (MNode* val) {
    size_t  n = size ();
    if (n > 0) {
	push (get (n - 1));
	for (size_t i = n - 1; i > 0; -- i) {
	    put (i, get (i - 1));
	}
	put (0, val);
    } else {
	push (val);
    }
}

MNode*  MotorVector::shift () {
    size_t  n = size ();
    if (n > 0) {
	MNodePtr  ans;
	ans = get (0);
	for (size_t i = 1; i < n; ++ i) {
	    put (i - 1, get (i));
	}
	pop ();
	return ans.release ();
    } else {
	return NULL;
    }
}
