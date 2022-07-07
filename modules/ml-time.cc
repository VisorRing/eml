#include "ml-time.h"
#include "ml.h"
#include "mlenv.h"
#include "motorenv.h"
#include "expr.h"
#include "util_const.h"
#include "util_time.h"
#include <exception>
#include <time.h>
#include <string.h>

/*DOC:
==time function==

*/
static MNode*  timeval6 (struct tm* v) {
    MNodeList  ans;

    ans.append (newMNode_num (v->tm_year + 1900));
    ans.append (newMNode_num (v->tm_mon + 1));
    ans.append (newMNode_num (v->tm_mday));
    ans.append (newMNode_num (v->tm_hour));
    ans.append (newMNode_num (v->tm_min));
    ans.append (newMNode_num (v->tm_sec));

    return ans.release ();
}

static MNode*  dateval7 (struct tm* v) {
    MNodeList  ans;

    ans.append (newMNode_num (v->tm_year + 1900));
    ans.append (newMNode_num (v->tm_mon + 1));
    ans.append (newMNode_num (v->tm_mday));
    ans.append (newMNode_num (v->tm_wday));
    ans.append (newMNode_num (v->tm_hour));
    ans.append (newMNode_num (v->tm_min));
    ans.append (newMNode_num (v->tm_sec));

    return ans.release ();
}

static MNode*  timeval3 (struct tm* v) {
    MNodeList  ans;

    ans.append (newMNode_num (v->tm_hour));
    ans.append (newMNode_num (v->tm_min));
    ans.append (newMNode_num (v->tm_sec));

    return ans.release ();
}

/*DOC:
===now===
 (now) -> INTEGER

*/
//#XAFUNC	now	ml_now
//#XWIKIFUNC	now
MNode*  ml_now (bool fev, MNode* cell, MlEnv* mlenv) {
    evalParams (fev, mlenv, cell);
    time_t  tm = now ();
    return newMNode_num (tm);
}

/*DOC:
===now-microsec===
 (now-microsec) -> SECOND.MICROSECOND

*/
//#XAFUNC	now-microsec	ml_now_microsec
//#XWIKIFUNC	now-microsec
MNode*  ml_now_microsec (bool fev, MNode* cell, MlEnv* mlenv) {
    evalParams (fev, mlenv, cell);
    return newMNode_num (now_microsec ());
}

/*DOC:
===to-date===
 (to-date INTEGER) -> (YEAR MONTH DAY HOUR MINUTE SECOND)

*/
//#XAFUNC	to-date	ml_datetime3
//#XWIKIFUNC	to-date
MNode*  ml_datetime3 (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    evalParams (fev, mlenv, cell, posList, posParams);
    time_t  tm = (time_t)to_int64 (posParams[0]());
    struct tm  v;
    localtime_r (&tm, &v);
    return timeval6 (&v);
}

/*DOC:
===to-date4===
 (to-date4 INTEGER) -> (YEAR MONTH DAY WEEK HOUR MINUTE SECOND)

*/
//#XAFUNC	to-date4	ml_date4
//#XWIKIFUNC	to-date4
MNode*  ml_date4 (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    evalParams (fev, mlenv, cell, posList, posParams);
    time_t  tm = (time_t)to_int64 (posParams[0]());
    struct tm  v;
    localtime_r (&tm, &v);
    return dateval7 (&v);
}

/*DOC:
===to-time===
 (to-time INTEGER) -> (HOUR MINUTE SECOND)

*/
//#XAFUNC	to-time	ml_time3
//#XWIKIFUNC	to-time
MNode*  ml_time3 (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    evalParams (fev, mlenv, cell, posList, posParams);
    MNodePtr  ans;
    time_t  tm = (time_t)to_int64 (posParams[0]());
    struct tm  v;
    localtime_r (&tm, &v);
    return timeval3 (&v);
}

/*DOC:
===date-to-time===
 (date-to-time YEAR MONTH DAY [HOUR [MINUTE [SECOND]]]) -> NUMBER
 (date-to-time (YEAR MONTH DAY [HOUR [MINUTE [SECOND]]])) -> NUMBER

*/
//#XAFUNC	date-to-time	ml_datetotime
//#XWIKIFUNC	date-to-time
MNode*  ml_datetotime (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_LIST, EV_LIST, EV_LIST, EV_LIST, EV_LIST, EV_END};
    MNodePtr  posParams[6];
    evalParams (fev, mlenv, cell, posList, posParams);
    struct tm  tm;
    memset (&tm, 0, sizeof (tm));
    if (isCons (posParams[0]())) {
	MNode*  args = posParams[0]();
	tm.tm_year = to_int64 (args->car ()) - 1900;
	nextNode (args);
	tm.tm_mon = to_int64 (args->car ()) - 1;
	nextNode (args);
	tm.tm_mday = to_int64 (args->car ());
	nextNode (args);
	tm.tm_hour = to_int64 (args->car ());
	nextNode (args);
	tm.tm_min = to_int64 (args->car ());
	nextNode (args);
	tm.tm_sec = to_int64 (args->car ());
    } else {
	tm.tm_year = to_int64 (posParams[0]()) - 1900;
	tm.tm_mon = to_int64 (posParams[1]()) - 1;
	tm.tm_mday = to_int64 (posParams[2]());
	tm.tm_hour = to_int64 (posParams[3]());
	tm.tm_min = to_int64 (posParams[4]());
	tm.tm_sec = to_int64 (posParams[5]());
    }
    return newMNode_num (mktime (&tm));
}

/*DOC:
===to-gmdate===
 (to-gmdate INTEGER) -> (YEAR MONTH DAY HOUR MINUTE SECOND)

*/
//#XAFUNC	to-gmdate	ml_gmdatetime3
//#XWIKIFUNC	to-gmdate
MNode*  ml_gmdatetime3 (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    evalParams (fev, mlenv, cell, posList, posParams);
    time_t  tm = (time_t)to_int64 (posParams[0]());
    struct tm  v;
    gmtime_r (&tm, &v);
    return timeval6 (&v);
}

/*DOC:
===to-gmdate4===
 (to-gmdate4 INTEGER) -> (YEAR MONTH DAY WEEK HOUR MINUTE SECOND)

*/
//#XAFUNC	to-gmdate4	ml_gmdate4
//#XWIKIFUNC	to-gmdate4
MNode*  ml_gmdate4 (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    evalParams (fev, mlenv, cell, posList, posParams);
    time_t  tm = (time_t)to_int64 (posParams[0]());
    struct tm  v;
    gmtime_r (&tm, &v);
    return dateval7 (&v);
}

/*DOC:
===to-gmtime===
 (to-gmtime INTEGER) -> (HOUR MINUTE SECOND)

*/
//#XAFUNC	to-gmtime	ml_gmtime3
//#XWIKIFUNC	to-gmtime
MNode*  ml_gmtime3 (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    evalParams (fev, mlenv, cell, posList, posParams);
    time_t  tm = (time_t)to_int64 (posParams[0]());
    struct tm  v;
    gmtime_r (&tm, &v);
    return timeval3 (&v);
}

/*DOC:
===gmdate-to-time===
 (gmdate-to-time YEAR MONTH DAY [HOUR [MINUTE [SECOND]]]) -> NUMBER
 (gmdate-to-time (YEAR MONTH DAY [HOUR [MINUTE [SECOND]]])) -> NUMBER

*/
//#XAFUNC	gmdate-to-time	ml_gmdatetotime
//#XWIKIFUNC	gmdate-to-time
MNode*  ml_gmdatetotime (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_LIST, EV_LIST, EV_LIST, EV_LIST, EV_LIST, EV_END};
    MNodePtr  posParams[6];
    evalParams (fev, mlenv, cell, posList, posParams);
    struct tm  tm;
    memset (&tm, 0, sizeof (tm));
    if (isCons (posParams[0]())) {
	MNode*  args = posParams[0]();
	tm.tm_year = to_int64 (args->car ()) - 1900;
	nextNode (args);
	tm.tm_mon = to_int64 (args->car ()) - 1;
	nextNode (args);
	tm.tm_mday = to_int64 (args->car ());
	nextNode (args);
	tm.tm_hour = to_int64 (args->car ());
	nextNode (args);
	tm.tm_min = to_int64 (args->car ());
	nextNode (args);
	tm.tm_sec = to_int64 (args->car ());
    } else {
	tm.tm_year = to_int64 (posParams[0]()) - 1900;
	tm.tm_mon = to_int64 (posParams[1]()) - 1;
	tm.tm_mday = to_int64 (posParams[2]());
	tm.tm_hour = to_int64 (posParams[3]());
	tm.tm_min = to_int64 (posParams[4]());
	tm.tm_sec = to_int64 (posParams[5]());
    }
    return newMNode_num (timegm (&tm));
}

/*DOC:
===date-delta===
 (date-delta TIME YEAR MONTH DAY [HOUR [MINUTE [SECOND]]]) -> TIME

*/
//#XAFUNC	date-delta	ml_date_delta
//#XWIKIFUNC	date-delta
MNode*  ml_date_delta (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_LIST, EV_LIST, EV_LIST, EV_LIST, EV_LIST, EV_LIST, EV_END};
    MNodePtr  posParams[7];
    evalParams (fev, mlenv, cell, posList, posParams);
    time_t  tmv = (time_t)to_int64 (posParams[0]());
    struct tm  tm;
    memset (&tm, 0, sizeof (tm));
    gmtime_r (&tmv, &tm);
    tm.tm_year += to_int64 (posParams[1]());
    tm.tm_mon += to_int64 (posParams[2]());
    tm.tm_mday += to_int64 (posParams[3]());
    tm.tm_hour += to_int64 (posParams[4]());
    tm.tm_min += to_int64 (posParams[5]());
    tm.tm_sec += to_int64 (posParams[6]());
    return newMNode_num (timegm (&tm));
}

/*DOC:
===date-format, gmdate-format===
 (date-format FORMAT INTEGER) -> STRING
 (gmdate-format FORMAT INTEGER) -> STRING

*/
//#XAFUNC	date-format	ml_date_format
//#XWIKIFUNC	date-format
MNode*  ml_date_format (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_LIST, EV_END};
    MNodePtr  posParams[2];
    evalParams (fev, mlenv, cell, posList, posParams);
    ustring  format = to_string (posParams[0]());
    time_t  tm = to_int64 (posParams[1]());
    struct tm  tmv;
    localtime_r (&tm, &tmv);
    return newMNode_str (new ustring (formatDateString (format, tmv)));
}

//#XAFUNC	gmdate-format	ml_gmdate_format
//#XWIKIFUNC	gmdate-format
MNode*  ml_gmdate_format (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_LIST, EV_END};
    MNodePtr  posParams[2];
    evalParams (fev, mlenv, cell, posList, posParams);
    ustring  format = to_string (posParams[0]());
    time_t  tm = to_int64 (posParams[1]());
    struct tm  tmv;
    gmtime_r (&tm, &tmv);
//    return newMNode_str (new ustring (formatDateString (format, tmv)));
    size_t  n = format.length () * 2;
    if (n > 65535)
	n = 65535;
    char*  buf = new char[n];
    size_t  s = strftime (buf, n, format.c_str (), &tmv);
    MNode*  ans = newMNode_str (new ustring (buf, s));
    delete[] buf;
    return ans;
}

/*DOC:
===read-rfc3339-date===
 (read-rfc3339-date STRING) -> (SECOND FRACTION) | nil

オフセット非対応。

*/
//#XAFUNC	read-rfc3339-date	ml_read_rfc3339_date
//#XWIKIFUNC	read-rfc3339-date
MNode*  ml_read_rfc3339_date (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    evalParams (fev, mlenv, cell, posList, posParams);
    ustring  dt = to_string (posParams[0]());
    umatch  m;
    static uregex  re ("^([0-9]{4})-([0-9]{2})-([0-9]{2})T([0-9]{2}):([0-9]{2}):([0-9]{2})(\\.[0-9]+)?Z$");
    if (usearch (dt, m, re)) {
	struct tm  tm;
	memset (&tm, 0, sizeof (tm));
	tm.tm_year = to_int32(ustring (m[1].first, m[1].second)) - 1900;
	tm.tm_mon = to_int32(ustring (m[2].first, m[2].second)) - 1;
	tm.tm_mday = to_int32(ustring (m[3].first, m[3].second));
	tm.tm_hour = to_int32(ustring (m[4].first, m[4].second));
	tm.tm_min = to_int32(ustring (m[5].first, m[5].second));
	tm.tm_sec = to_int32(ustring (m[6].first, m[6].second));
	double frac = m[7].first != m[7].second ? to_double (ustring (m[7].first, m[7].second)) : 0.;
	return newMNode_cons (newMNode_num (timegm (&tm)), newMNode_cons (newMNode_num (frac)));
    } else {
	return NULL;
    }
}

/*DOC:
===to-rfc3339-date===
 (to-rfc3339-date INTEGER) -> "yyyy-mm-ddThh:mm:ssZ"

オフセット非対応。

*/
//#XAFUNC	to-rfc3339-date	ml_to_rfc3339_date
//#XWIKIFUNC	to-rfc3339-date
MNode*  ml_to_rfc3339_date (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    evalParams (fev, mlenv, cell, posList, posParams);
    time_t  tm = to_int64 (posParams[0]());
    struct tm  tmv;
    gmtime_r (&tm, &tmv);
    char*  buf = new char[32];
    size_t  s = strftime (buf, 32, "%Y-%m-%dT%H:%M:%SZ", &tmv);
    MNode*  ans = newMNode_str (new ustring (buf, s));
    delete[] buf;
    return ans;
}
