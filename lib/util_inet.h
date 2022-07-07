#ifndef UTIL_INET_H
#define UTIL_INET_H

#include "ustring.h"
#include <vector>

ustring  getnameinfo (const ustring& ip);
void  getAddrInfo (const ustring& hostname, std::vector<ustring>& ans);

#endif /* UTIL_INET_H */
