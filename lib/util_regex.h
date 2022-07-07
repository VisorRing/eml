#ifndef UTIL_REGEXP_H
#define UTIL_REGEXP_H

#include "ustring.h"
#include <boost/regex.hpp>

typedef boost::match_results<ustring::const_iterator>  umatch;
typedef boost::basic_regex<char, boost::regex_traits<char> >  uregex;

inline bool  usearch (ustring::const_iterator first, ustring::const_iterator last, umatch& m, const uregex& re, boost::match_flag_type flags = boost::regex_constants::match_single_line) {
    return regex_search (first, last, m, re, flags);
}
inline bool  usearch (const ustring& s, umatch& m, const uregex& re, boost::match_flag_type flags = boost::regex_constants::match_single_line) {
    return regex_search (s.begin (), s.end (), m, re, flags);
}


#endif /* UTIL_REGEXP_H */
