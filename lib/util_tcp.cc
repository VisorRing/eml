#include "util_tcp.h"
#include "config.h"
#include "http.h"
#include "util_const.h"
#include "util_base64.h"
#include "util_check.h"
#include "util_string.h"
#include "ustring.h"
#include <boost/regex.hpp>
#include <sys/uio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <locale>

#define SSL_NAME_LEN	256

#if 0
static int  case_diffs (const char *s, const char *t) {
    unsigned char  x;
    unsigned char  y;

    for (;;) {
	x = *s++ - 'A';
	if (x <= 'Z' - 'A')
	    x += 'a';
	else
	    x += 'A';
	y = *t++ - 'A';
	if (y <= 'Z' - 'A')
	    y += 'a';
	else
	    y += 'A';
	if (x != y)
	    break;
	if (!x)
	    break;
    }
    return ((int)(unsigned int) x) - ((int)(unsigned int) y);
}
#endif

//============================================================
bool  TcpBuf::empty () {
    return start == tail;
}

bool  TcpBuf::fill (TcpClient& tc) {
    ssize_t  s;

    if (start != buf.begin ()) {
	if (tail == start) {
	    start = tail = buf.begin ();
	} else {
	    memmove (&*buf.begin (), &*start, tail - start);
	    tail -= start - buf.begin ();
	    start = buf.begin ();
	}
    }
    s = tc.read (&*tail, buf.end () - tail);
    if (s <= 0)
	return false;
    tail += s;

    return true;
}

bool  TcpBuf::getln (TcpClient& tc, ustring& ans) {
    boost::match_results<ustring::iterator>  m;
    static uregex  re_crlf ("\\r\\n");

    if (empty ())
	fill (tc);
    if (regex_search (start, tail, m, re_crlf, boost::regex_constants::match_single_line)) {
	ans.assign (start, m[0].first);
	start = m[0].second;
	return true;
    } else if (fill (tc)) {
	if (regex_search (start, tail, m, re_crlf, boost::regex_constants::match_single_line)) {
	    ans.assign (start, m[0].first);
	    start = m[0].second;
	    return true;
	}
    }
    return false;
}

bool  TcpBuf::getln2 (TcpClient& tc, ustring& ans) {
    boost::match_results<ustring::iterator>  m;
    static uregex  re_crlf ("\\r\\n");

    if (regex_search (start, tail, m, re_crlf, boost::regex_constants::match_single_line)) {
	ans.assign (start, m[0].first);
	start = m[0].second;
	return true;
    }
    return false;
}

size_t  TcpBuf::size () {
    return tail - start;
}

void  TcpBuf::consume () {
    start = tail = buf.begin ();
}

//============================================================
bool  TcpClient::connect (const HostSpec* host) {
    char pbuf[10];
    const char*  bindaddr = NULL;
    struct addrinfo  hints;
    struct addrinfo*  res;
    struct addrinfo*  res0;
    bool  rc;

    switch (host->ipv) {
    case HostSpec::IPV4:
	af = AF_INET;
	break;
    case HostSpec::IPV6:
	af = AF_INET6;
	break;
    }

    snprintf (pbuf, sizeof(pbuf), "%d", host->port);
    memset (&hints, 0, sizeof (hints));
    hints.ai_family = af;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = 0;
    if ((err = getaddrinfo (host->host.c_str (), pbuf, &hints, &res0)) != 0) {
//	seterr (err);
	return false;
    }
    for (sd = -1, res = res0; res; res = res->ai_next) {
	if ((sd = socket (res->ai_family, res->ai_socktype, res->ai_protocol)) == -1)
	    continue;
	if (bindaddr != NULL && *bindaddr != '\0' && ! bind (bindaddr)) {
//	    failed to bind to 'bindaddr'
	    ::close (sd);
	    sd = -1;
	    continue;
	}
	if (::connect (sd, res->ai_addr, res->ai_addrlen) == 0)
	    break;
	::close (sd);
	sd = -1;
    }
    freeaddrinfo (res0);
    if (sd == -1) {
//	syserr
	return false;
    }
    
#if 0
    if (! reopen ()) {
//	syserr
	::close(sd);
    }
#endif

    rc = connect2 ();
    if (! rc) {
	close ();
	return false;
    }

    noPush ();
    return true;
}

bool  TcpClient::connect2 () {
    // none
    return true;
}

bool  TcpClient::bind (const char* addr) {
    struct addrinfo  hints;
    struct addrinfo*  res;
    struct addrinfo*  res0;

    memset (&hints, 0, sizeof (hints));
    hints.ai_family = af;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = 0;
    if ((err = getaddrinfo (addr, NULL, &hints, &res0)) != 0)
	return false;
    for (res = res0; res; res = res->ai_next)
	if (::bind (sd, res->ai_addr, res->ai_addrlen) == 0)
	    return true;
    return false;
}

void  TcpClient::noPush () {
    int  val = 1;

    setsockopt(sd, IPPROTO_TCP, TCP_NOPUSH, &val, sizeof(val));
}

void  TcpClient::close () {
    if (sd >= 0) {
	::close (sd);
	sd = -1;
    }
}

ssize_t  TcpClient::read (void* buf, size_t nbytes) {
    struct timeval  now;
    struct timeval  timeout;
    struct timeval  delta;
    fd_set rfds;
    ssize_t rlen, total;
    int r;

    if (timeLimit) {
	FD_ZERO (&rfds);
	gettimeofday (&timeout, NULL);
	timeout.tv_sec += timeLimit;
    }

    total = 0;
    while (nbytes > 0) {
	while (timeLimit && ! FD_ISSET (sd, &rfds)) {
	    FD_SET (sd, &rfds);
	    gettimeofday (&now, NULL);
	    delta.tv_sec = timeout.tv_sec - now.tv_sec;
	    delta.tv_usec = timeout.tv_usec - now.tv_usec;
	    if (delta.tv_usec < 0) {
		delta.tv_usec += 1000000;
		delta.tv_sec --;
	    }
	    if (delta.tv_sec < 0) {
		errno = ETIMEDOUT;
//		syserr();
		return -1;
	    }
	    errno = 0;
	    r = select (sd + 1, &rfds, NULL, NULL, &delta);
	    if (r == -1) {
		if (errno == EINTR)
		    continue;
//		syserr();
		return -1;
	    }
	}
	rlen = read2 (buf, nbytes);
	if (rlen == 0)
	    break;
	if (rlen < 0) {
	    if (errno == EINTR)
		continue;
	    return -1;
	}
	nbytes -= rlen;
	buf = (char*)buf + rlen;
	total += rlen;
    }
    return total;
}

ssize_t  TcpClient::write (const void* buf, size_t nbytes) {
    struct iovec iov;

    iov.iov_base = __DECONST(char *, buf);
    iov.iov_len = nbytes;
    return write (&iov, 1);
}

void  TcpClient::flush_write () {
    int  val;

    val = 0;
    setsockopt(sd, IPPROTO_TCP, TCP_NOPUSH, &val, sizeof(val));
    val = 1;
    setsockopt(sd, IPPROTO_TCP, TCP_NODELAY, &val, sizeof(val));
}

ssize_t  TcpClient::write (struct iovec* iov, int iovcnt) {
    struct timeval  now;
    struct timeval  timeout;
    struct timeval  delta;
    fd_set writefds;
    ssize_t wlen, total;
    int r;

    if (timeLimit) {
	FD_ZERO (&writefds);
	gettimeofday (&timeout, NULL);
	timeout.tv_sec += timeLimit;
    }

    total = 0;
    while (iovcnt > 0) {
	while (timeLimit && ! FD_ISSET (sd, &writefds)) {
	    FD_SET (sd, &writefds);
	    gettimeofday (&now, NULL);
	    delta.tv_sec = timeout.tv_sec - now.tv_sec;
	    delta.tv_usec = timeout.tv_usec - now.tv_usec;
	    if (delta.tv_usec < 0) {
		delta.tv_usec += 1000000;
		delta.tv_sec --;
	    }
	    if (delta.tv_sec < 0) {
		errno = ETIMEDOUT;
//		syserr();
		return -1;
	    }
	    errno = 0;
	    r = select (sd + 1, NULL, &writefds, NULL, &delta);
	    if (r == -1) {
		if (errno == EINTR)
		    continue;
		return -1;
	    }
	}
	errno = 0;
	wlen = write2 (iov, iovcnt);
	if (wlen == 0) {
	    /* we consider a short write a failure */
	    errno = EPIPE;
//	    syserr();
	    return -1;
	}
	if (wlen < 0) {
	    if (errno == EINTR)
		continue;
	    return -1;
	}
	total += wlen;
	while (iovcnt > 0 && wlen >= (ssize_t)iov->iov_len) {
	    wlen -= iov->iov_len;
	    iov++;
	    iovcnt--;
	}
	if (iovcnt > 0) {
	    iov->iov_len -= wlen;
	    iov->iov_base = __DECONST(char *, iov->iov_base) + wlen;
	}
    }
    return total;
}

ssize_t  TcpClient::write2 (struct iovec* iov, int iovcnt) {
    return writev (sd, iov, iovcnt);
}

ssize_t  TcpClient::read2 (void* buf, size_t nbytes) {
    return ::read (sd, buf, nbytes);
}

//============================================================
bool  SslClient::connect (const HostSpec* conhost, const HostSpec* _ephost) {
    bool  rc;

    ephost = _ephost;
    rc = TcpClient::connect (conhost);
    return rc;
}

bool  SslClient::connect2 () {
    return sslOpen ();
}

bool  SslClient::sslOpen () {
    if (!SSL_library_init ()){
	close ();
	throw (ustring (CharConst ("SSL library init failed\n")));
	return false;
    }

    SSL_load_error_strings ();

    ssl_meth = SSLv23_client_method ();
    ssl_ctx.reset (SSL_CTX_new (ssl_meth));
    SSL_CTX_set_mode (ssl_ctx.get (), SSL_MODE_AUTO_RETRY);
    SSL_CTX_set_options (ssl_ctx.get (), SSL_OP_ALL | SSL_OP_NO_TICKET | SSL_OP_NO_SSLv2);
    if (! setupPeerVerification (kCERTFILE)) {
	close ();
	throw (ustring (CharConst ("SSL certificate error\n")));
	return false;
    }

    ssl.reset (SSL_new (ssl_ctx.get ()));
    if (ssl.get () == NULL){
	close ();
	throw (ustring (CharConst ("SSL context creation failed\n")));
	return false;
    }
    SSL_set_fd (ssl.get (), sd);
//    if (SSL_connect (ssl.get ()) == -1){
#if OPENSSL_VERSION_NUMBER >= 0x0090806fL && !defined(OPENSSL_NO_TLSEXT)
    if (! SSL_set_tlsext_host_name (ssl.get (), ephost->host.c_str ())) {
	fprintf (stderr, "TLS server name indication extension failed for host %s\n", ephost->host.c_str ());
	close ();
	return false;
    }
#endif

    int  rc;
    while ((rc = SSL_connect (ssl.get ())) == -1) {
	int  ssl_err = SSL_get_error (ssl.get (), rc);
	if (ssl_err != SSL_ERROR_WANT_READ && ssl_err != SSL_ERROR_WANT_WRITE) {
	    ERR_print_errors_fp (stderr);
	    close ();
	    return false;
	}
    }
    if (fnoverify || verifyCA ()) {
	sslmode = true;
	return true;
    } else {
	close ();
	return false;
    }
}

void  SslClient::loadCAFile (const char* certfile, int depth) {
    if (!SSL_CTX_load_verify_locations (ssl_ctx.get (), certfile, NULL))
	return;
    SSL_CTX_set_verify_depth (ssl_ctx.get (), depth);
}

/*
 * Callback for SSL certificate verification, this is called on server
 * cert verification. It takes no decision, but informs the user in case
 * verification failed.
 */
static int  fetch_ssl_cb_verify_crt (int verified, X509_STORE_CTX *ctx) {
    X509*  crt;
    X509_NAME*  name;
    char*  str;

    str = NULL;
    if (! verified) {
	if ((crt = X509_STORE_CTX_get_current_cert (ctx)) != NULL &&
	    (name = X509_get_subject_name (crt)) != NULL)
	    str = X509_NAME_oneline (name, 0, 0);
	fprintf (stderr, "Certificate verification failed for %s\n",
		 str != NULL ? str : "no relevant certificate");
	OPENSSL_free (str);
    }
    return verified;
}

bool  SslClient::setupPeerVerification (const char* certfile, int depth) {
    if (! fnoverify) {
	SSL_CTX_set_verify (ssl_ctx.get (), SSL_VERIFY_PEER, fetch_ssl_cb_verify_crt);
	SSL_CTX_load_verify_locations (ssl_ctx.get (), certfile, NULL);
    }
    return true;
}

bool  SslClient::verifyCA () {
    X509_autoptr  cert;
    long  rc;
    ustring  lhost;
    bool  ans;

    if ((rc = SSL_get_verify_result (ssl.get ())) != X509_V_OK) {
	switch (rc) {
	case X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT_LOCALLY:
	    throw (ustring (CharConst ("unable to get issuer cert locally.")));
	default:
	    std::cerr << rc << ": X509 failed\n";
	}
	return false;
    }
    cert.reset (SSL_get_peer_certificate (ssl.get ()));
    if (!cert.get ())
	return false;

    lhost.resize (ephost->host.length ());
    std::transform (ephost->host.begin (), ephost->host.end (), lhost.begin (), ::tolower);
    ans = verifyCN (lhost, cert.get ()) || verifyAltName (lhost, cert.get ());
    if (! ans) {
	std::cerr << lhost << ": hostname not match\n";
    }
    return ans;
}

bool  SslClient::verifyCN (const ustring& lhost, X509* cert) {
    char  buf[SSL_NAME_LEN];
    long  rc;
    int  len;
    ustring  cn;

    len = X509_NAME_get_text_by_NID (X509_get_subject_name (cert), NID_commonName, buf, sizeof (buf));

    if (len > 0 && lhost.length () > 0) {
	if (len >= sizeof (buf)) {
	    std::cerr << "Too long common name of the server certificate\n";
	    return false;
	}
	cn.assign (buf, len);
	std::transform (cn.begin (), cn.end (), cn.begin (), ::tolower);
	return globMatch (lhost, cn);
    }
    return false;
}

bool  SslClient::verifyAltName (const ustring& lhost, X509* cert) {
    int  pos;
    X509_EXTENSION*  ext = NULL;
    GENERAL_NAMES*  names;
    GENERAL_NAME*  name;
    int  i, n;
    unsigned char*  dns;

    pos = X509_get_ext_by_NID (cert, NID_subject_alt_name, -1);
    if (pos >= 0) {
	ext = X509_get_ext (cert, pos);
	if (ext) {
	    names = (GENERAL_NAMES*)X509V3_EXT_d2i (ext);
	    n = sk_GENERAL_NAME_num (names);
	    for (i = 0; i < n; ++ i) {
		name = sk_GENERAL_NAME_value (names, i);
		if (name->type == GEN_DNS) {
		    ASN1_STRING_to_UTF8 (&dns, name->d.dNSName);
//		    std::cerr << "dns:" << dns << "\n";
		    ustring  name (char_type (dns));
		    std::transform (name.begin (), name.end (), name.begin (), ::tolower);
		    if (globMatch (lhost, ustring (char_type (dns)))) {
			OPENSSL_free (dns);
			return true;
		    } else {
			OPENSSL_free (dns);
		    }
		}
	    }
	}
    }
    return false;
}

bool  SslClient::globMatch (const ustring& lhost, const ustring& name) {
    if (matchHead (name, CharConst ("*."))) {
	// XXX RFC2459（= HTTP over TLS）では、*は一つのサブドメインのみにマッチする。
	ustring::size_type  p = lhost.find ('.');
	if (p != ustring::npos) {
	    return ustring (lhost.begin () + p, lhost.end ()) == ustring (name.begin () + 1, name.end ());
	} else {
	    return false;
	}
    } else {
	return lhost == name;
    }
}

ssize_t  SslClient::write2 (struct iovec* iov, int iovcnt) {
    if (sslmode) {
	return SSL_write(ssl.get (), iov->iov_base, iov->iov_len);
    } else {
	return TcpClient::write2 (iov, iovcnt);
    }
}

ssize_t  SslClient::read2 (void* buf, size_t nbytes) {
    if (sslmode) {
	return SSL_read (ssl.get (), buf, nbytes);
    } else {
	return TcpClient::read2 (buf, nbytes);
    }
}

//============================================================
bool  ProxySslClient::connect2 () {
    ustring  msg;

    if (matchHostname (ephost->host)) {
	msg.assign (CharConst ("CONNECT "));
	msg.append (ephost->host).append (uColon).append (to_ustring (ephost->port)).append (CharConst (" HTTP/1.0" kCRLF));
	msg.append (CharConst ("Host: ")).append (ephost->host).append (uCRLF);
	// no User-Agent:
	if (proxyid.length () > 0) {
	    ustring idpw;
	    idpw.assign (proxyid).append (uColon).append (proxypw);
	    msg.append (CharConst ("Proxy-Authorization: Basic ")).append (base64Encode (idpw.begin (), idpw.end ())).append (uCRLF);
	}
	msg.append (uCRLF);
	write (&*msg.begin (), msg.length ()); // CONNECT HOST...

	int  rc = readReplyHead ();
	if (rc == 200) {
	    return SslClient::connect2 ();
	} else {
	}
    }
    return false;
}

int  ProxySslClient::readReplyHead () {
    TcpBuf  buf;
    ustring  line;
    int  responseCode = 0;
    umatch  m;
    static uregex  re_crlf ("\\r\\n");

    buf.tail += read3 (&*buf.start, buf.buf.length ());
    if (buf.getln2 (*this, line)) {
    } else {
	return 0;
    }

    if (! match (line.substr (0, 5), CharConst ("HTTP/"))) {
	return 0;		// bad protocol
    } else {
	ustring::size_type  p1 = line.find (' ', 0);
	ustring::size_type  p2 = line.find (' ', p1 + 1);
	responseCode = strtoul (line.substr (p1 + 1, p2));
	while (buf.getln2 (*this, line)) {
	    // XXX
	    if (line.empty ())
		break;
	}
    }
    return responseCode;
}

ssize_t  ProxySslClient::read3 (void* buf, size_t nbytes) {
    struct timeval  delta;
    fd_set rfds;
    ssize_t rlen, total;
    int r;

    FD_ZERO (&rfds);
    total = 0;

    FD_SET (sd, &rfds);
    delta.tv_sec = timeLimit;
    delta.tv_usec = 0;
    errno = 0;
    r = select (sd + 1, &rfds, NULL, NULL, &delta);
    if (r <= 0)
	return total;
    FD_CLR (sd, &rfds);
    rlen = read2 (buf, nbytes);
    if (rlen <= 0)
	return total;
    nbytes -= rlen;
    buf = (char*)buf + rlen;
    total += rlen;

    while (nbytes > 0) {
	FD_SET (sd, &rfds);
	delta.tv_sec = 0;
	delta.tv_usec = 0;
	errno = 0;
	r = select (sd + 1, &rfds, NULL, NULL, &delta);
	if (r <= 0)
	    return total;
	FD_CLR (sd, &rfds);
	rlen = read2 (buf, nbytes);
	if (rlen <= 0)
	    return total;
	nbytes -= rlen;
	buf = (char*)buf + rlen;
	total += rlen;
    }
    return total;
}

