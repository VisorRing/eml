#include "util_inet.h"
#include "util_string.h"
#include "ustring.h"
#include <vector>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

ustring  getnameinfo (const ustring& ip) {
    struct sockaddr_in  sa;
    char  host[NI_MAXHOST + 4];
    char*  p;
    umatch  m;
    static uregex  re ("^([0-9]{1,3})\\.([0-9]{1,3})\\.([0-9]{1,3})\\.([0-9]{1,3})$");

    if (regex_search (ip, m, re)) {
	memset (&sa, 0, sizeof (sa));
	sa.sin_family = AF_INET;
	p = (char*)&sa.sin_addr;
	p[0] = strtoul(ustring (m[1].first, m[1].second));
	p[1] = strtoul(ustring (m[2].first, m[2].second));
	p[2] = strtoul(ustring (m[3].first, m[3].second));
	p[3] = strtoul(ustring (m[4].first, m[4].second));
	if (getnameinfo ((struct sockaddr*)&sa, sizeof (sa), host, NI_MAXHOST, NULL, 0, 0) == 0) {
	    return ustring (host);
	}
    }
    return ip;
}

void  getAddrInfo (const ustring& hostname, std::vector<ustring>& ans) {
    struct addrinfo*  ai;
    struct addrinfo*  p;
    char  b[INET6_ADDRSTRLEN];
    ustring  ip;

    if (getaddrinfo (hostname.c_str (), NULL, NULL, &ai) == 0) {
	for (p = ai; p; p = p->ai_next) {
	    if (p->ai_socktype == SOCK_STREAM) {
		switch (p->ai_family) {
		case PF_INET:
		    {
			struct sockaddr_in*  sa = (struct sockaddr_in*)p->ai_addr;
			ip.assign (inet_ntop (p->ai_family, &sa->sin_addr, b, INET_ADDRSTRLEN));
			ans.push_back (ip);
		    }
		    break;
		case PF_INET6:
		    {
			struct sockaddr_in6*  sa = (struct sockaddr_in6*)p->ai_addr;
			ip.assign (inet_ntop (p->ai_family, &sa->sin6_addr, b, INET6_ADDRSTRLEN));
			ans.push_back (ip);
		    }
		    break;
		default:;
		}
	    }
	}
    } else {
    }
}
