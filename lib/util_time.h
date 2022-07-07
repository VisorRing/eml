#ifndef UTIL_TIME_H
#define UTIL_TIME_H

#include "ustring.h"
#include <boost/ptr_container/ptr_vector.hpp>
#include <time.h>

class  MNodePtr;

ustring  strYMD (time_t t);
ustring  mailDate ();
time_t  now ();
double  now_microsec ();
ustring  dateCookie (time_t clock);
void  datetime_list (boost::ptr_vector<MNodePtr>& par, time_t tm);

#endif /* UTIL_TIME_H */
