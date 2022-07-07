#include "util_time.h"
#include "ml.h"
#include "ustring.h"
#include <boost/ptr_container/ptr_vector.hpp>
#include <time.h>
#include <sys/time.h>

ustring  strYMD (time_t t) {
    struct tm  tv;

    localtime_r (&t, &tv);
    char  b[64];
    size_t  s;

    s = snprintf (b, 64, "%.4d%.2d%.2d", tv.tm_year + 1900, tv.tm_mon + 1, tv.tm_mday);
    return ustring (b, s);
}

ustring  mailDate () {
    size_t  s;
    char  b[64];
    time_t  t;
    struct tm  tv;

    t = now ();
    localtime_r (&t, &tv);
// Date: Sun, 04 Jan 2009 17:32:11 +0900
    s = strftime (b, 64, "Date: %a, %d %b %Y %T %z\n", &tv);
    return ustring (b, s);
}

time_t  now () {
    return time (NULL);
}

double  now_microsec () {
    struct timeval  tv;

    if (! gettimeofday (&tv, NULL)) {
	return (double)tv.tv_sec + (double)tv.tv_usec / 1000000.;
    } else {
	return 0.;
    }
}

ustring  dateCookie (time_t clock) {
    /* Netscape Cookie expireフォーマット */
    ustring  ans;
    struct tm  t;
    int  s;
    static const char* const  w[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    static const char* const  m[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

    gmtime_r (&clock, &t);
//    t = *gmtime( &clock );
    ans.reserve (64);
    ans.resize (64);
    s = snprintf (&ans[0], 64,
		  "%s, %.2d-%s-%.4d %.2d:%.2d:%.2d GMT",
		  w[t.tm_wday], t.tm_mday, m[t.tm_mon], t.tm_year + 1900,
		  t.tm_hour, t.tm_min, t.tm_sec);
    ans.resize (s);
    return ans;
}

void  datetime_list (boost::ptr_vector<MNodePtr>& par, time_t tm) {
    struct tm  v;

    localtime_r (&tm, &v);
    par.push_back (new MNodePtr);
    par.back () = newMNode_num (v.tm_year + 1900); // [1]  year
    par.push_back (new MNodePtr);
    par.back () = newMNode_num (v.tm_mon + 1); // [2]  month
    par.push_back (new MNodePtr);
    par.back () = newMNode_num (v.tm_mday); // [3]  day
    par.push_back (new MNodePtr);
    par.back () = newMNode_num (v.tm_hour); // [4]  hour
    par.push_back (new MNodePtr);
    par.back () = newMNode_num (v.tm_min); // [5]  minute
    par.push_back (new MNodePtr);
    par.back () = newMNode_num (v.tm_sec); // [6]  second
    par.push_back (new MNodePtr);
    par.back () = newMNode_num (v.tm_wday); // [7]  week
}
