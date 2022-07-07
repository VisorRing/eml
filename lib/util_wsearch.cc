#include "util_wsearch.h"
#include "utf16.h"
#include "util_const.h"
#include <boost/regex.hpp>

bool  wsearch (const ustring& text, boost::wsmatch& m, const ustring& reg, boost::wregex::flag_type reg_flags, boost::match_flag_type search_flags) {
    try {
	std::wstring  wtext = utow (text);
	std::wstring  wreg = utow (reg);
	boost::wregex  wre (wreg, reg_flags);
	return regex_search (wtext, m, wre, search_flags);
    } catch (boost::regex_error& err) {
	throw (uErrorRegexp);
    }
}

bool  wsearch_env (WSearchEnv& wsenv, const ustring& text, const ustring& reg, boost::wregex::flag_type reg_flags, boost::match_flag_type search_flags) {
    try {
	wsenv.regtext = utow (text);
	std::wstring  wreg = utow (reg);
	boost::wregex  wre (wreg, reg_flags);
	return regex_search (wsenv.regtext, wsenv.regmatch, wre, search_flags);
    } catch (boost::regex_error& err) {
	throw (uErrorRegexp);
    }
}

ustring  wreplace (const ustring& text, const ustring& reg, const ustring& fmt, boost::wregex::flag_type reg_flags, boost::match_flag_type match_flags) {
    try {
	std::wstring  wtext = utow (text);
	std::wstring  wreg = utow (reg);
	std::wstring  wfmt = utow (fmt);
	boost::wregex  wre (wreg, reg_flags);
	std::wstring  ans = regex_replace (wtext, wre, wfmt, match_flags);
	return wtou (ans);
    } catch (boost::regex_error& err) {
	throw (uErrorRegexp);
    }
}

