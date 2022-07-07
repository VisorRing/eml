#ifndef FORM_H
#define FORM_H

#include "ustring.h"
#include <boost/unordered_map.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <vector>
#include <iostream>

class  MotorEnv;
class  CGIForm {
 public:
    typedef boost::unordered_map<ustring, int>  map_t;
    typedef boost::ptr_vector<std::vector<int> >  indexary;
    typedef std::vector<ustring>  pool_t;
    typedef enum {
	M_NONE,
	M_GET,
	M_HEAD,
	M_POST,
	M_OPTIONS,
	M_PUT,
	M_DELETE,
	M_OTHER,
    }  method_type;
    typedef enum {
	T_NONE,
	T_URLENCODED,
	T_MULTIPART,
	T_XML,
	T_JSON,
    }  content_type;

    map_t  iarg;
    indexary  index;
    pool_t  values;
    ustring  requestMethod;
    method_type  method;
    content_type  contentType;
    ustring  boundary;		/* multipart boundary */
    ustring  queryString;

    CGIForm () {
	method = methodType ();
	checkContentType ();
    };
    virtual  ~CGIForm () {};
    virtual void  parseURLEncoded (const ustring& query);
    virtual void  read_get ();
    virtual void  read_post (size_t limit);
    virtual void  read_raw (size_t limit);
    virtual method_type  methodType ();
    virtual void  checkContentType ();
    virtual size_t  postSize ();
    virtual int  at (const ustring& name, ustring& ans);
    virtual int  at (const ustring& name, size_t i, ustring& ans);
    virtual size_t  atSize (const ustring& name);
    virtual int  at (map_t& mp, const ustring& name, size_t i, ustring& ans);
    virtual size_t  atSize (map_t& mp, const ustring& name);
#ifdef DEBUG2
    virtual void  dump (std::ostream& o);
#endif /* DEBUG */
    virtual int  insert (map_t& mp, const ustring& name, const ustring& value);
    virtual void  decode (ustring& key) = 0;
    virtual void  fix (ustring& key) = 0;
    virtual void  insertNameValue (map_t& mp, uiterator& b, uiterator& e);
    virtual std::vector<int>*  insertName (map_t& mp, const ustring& name);
};

#endif /* FORM_H */
