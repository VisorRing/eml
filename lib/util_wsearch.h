#ifndef UTIL_WSEARCH_H
#define UTIL_WSEARCH_H

#include "ustring.h"
#include <boost/regex.hpp>

class  WSearchEnv {
 public:
    std::wstring  regtext;
    boost::wsmatch  regmatch;

    WSearchEnv () {};
    virtual  ~WSearchEnv () {};
};

bool  wsearch (const ustring& text, boost::wsmatch& m, const ustring& reg, boost::wregex::flag_type reg_flags = boost::regex_constants::normal, boost::match_flag_type search_flags = boost::regex_constants::match_single_line);
bool  wsearch_env (WSearchEnv& wsenv, const ustring& text, const ustring& reg, boost::wregex::flag_type reg_flags = boost::regex_constants::normal, boost::match_flag_type search_flags = boost::regex_constants::match_single_line);
ustring  wreplace (const ustring& text, const ustring& reg, const ustring& fmt, boost::wregex::flag_type reg_flags = boost::regex_constants::normal, boost::match_flag_type match_flags = boost::regex_constants::match_single_line);

#endif /* UTIL_WSEARCH_H */
