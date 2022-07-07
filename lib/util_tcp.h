#ifndef UTIL_TCP_H
#define UTIL_TCP_H

#include "ustring.h"
#include <assert.h>
#include <ctype.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <openssl/crypto.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#ifdef DEBUG
#define DEFAULT_TIMELIMIT  10
#else
#define DEFAULT_TIMELIMIT  120
#endif

class  TcpClient;
class  TcpBuf {
 public:
    ustring  buf;
    ustring::iterator  start;
    ustring::iterator  tail;

    TcpBuf () {
	buf.assign (65536, '\0');
//	buf.assign (256, 0);	// debug
	start = tail = buf.begin ();
    };
    virtual  ~TcpBuf () {};
    virtual bool  empty ();
    virtual bool  fill (TcpClient& tc);
    virtual bool  getln (TcpClient& tc, ustring& ans);
    virtual bool  getln2 (TcpClient& tc, ustring& ans);
    virtual size_t  size ();
    virtual void  consume ();
};

class  HostSpec {
 public:
    enum {
	IPV4,
	IPV6,
    }  ipv;
    ustring  host;
    int  port;
    
    HostSpec () {
	ipv = IPV4;
	port = 0;
    };
    virtual  ~HostSpec () {};
    void  setIPv4 () {
	ipv = IPV4;
    };
    void  setIPv6 () {
	ipv = IPV6;
    };
};

class  TcpClient {
 public:
    int  sd;
    int  af;
    int  err;
    int  timeLimit;

    TcpClient () {
	sd = -1;
	af = AF_UNSPEC;
	err = 0;
	timeLimit = DEFAULT_TIMELIMIT;
    };
    virtual  ~TcpClient () {
	close ();
    };
    virtual bool  connect (const HostSpec* host);
    virtual void  close ();
    virtual ssize_t  read (void* buf, size_t nbytes);
    virtual ssize_t  write (const void* buf, size_t nbytes);
    virtual void  flush_write ();
 protected:
    virtual bool  bind (const char* addr);
    virtual bool  connect2 ();
    virtual void  noPush ();
    virtual ssize_t  write (struct iovec* iov, int iovcnt);
    virtual ssize_t  write2 (struct iovec* iov, int iovcnt);
    virtual ssize_t  read2 (void* buf, size_t nbytes);
};

class  SSL_autoptr {
 public:
    SSL*  ptr;
    SSL_autoptr (SSL* p = NULL) {
	ptr = p;
    };
    ~SSL_autoptr () {
	if (ptr) {
	    SSL_free (ptr);
	    ptr = NULL;
	}
    };
    void  reset (SSL* p = NULL) {
	assert (p == 0 || p != ptr);
	if (ptr)
	    SSL_free (ptr);
	ptr = p;
    };
    SSL&  operator * () const {
	assert (ptr != NULL);
	return *ptr;
    };
    SSL*  operator -> () const {
	assert (ptr != NULL);
	return ptr;
    };
    SSL*  get () const {
	return ptr;
    };
};

class  SSL_CTX_autoptr {
 public:
    SSL_CTX*  ptr;
    SSL_CTX_autoptr (SSL_CTX* p = NULL) {
	ptr = p;
    };
    ~SSL_CTX_autoptr () {
	if (ptr) {
	    SSL_CTX_free (ptr);
	    ptr = NULL;
	}
    };
    void  reset (SSL_CTX* p = NULL) {
	assert (p == 0 || p != ptr);
	if (ptr)
	    SSL_CTX_free (ptr);
	ptr = p;
    };
    SSL_CTX&  operator * () const {
	assert (ptr != NULL);
	return *ptr;
    };
    SSL_CTX*  operator -> () const {
	assert (ptr != NULL);
	return ptr;
    };
    SSL_CTX*  get () const {
	return ptr;
    };
};

class  X509_autoptr {
 public:
    X509*  ptr;
    X509_autoptr (X509* p = NULL) {
	ptr = p;
    };
    ~X509_autoptr () {
	if (ptr) {
	    X509_free (ptr);
	    ptr = NULL;
	}
    };
    void  reset (X509* p = NULL) {
	assert (p == 0 || p != ptr);
	if (ptr)
	    X509_free (ptr);
	ptr = p;
    };
    X509&  operator * () const {
	assert (ptr != NULL);
	return *ptr;
    };
    X509*  operator -> () const {
	assert (ptr != NULL);
	return ptr;
    };
    X509*  get () const {
	return ptr;
    };
};

class  SslClient: public TcpClient {
 public:
    const HostSpec*  ephost;
    SSL_autoptr  ssl;
    SSL_CTX_autoptr  ssl_ctx;
#if OPENSSL_VERSION_NUMBER >= 0x1000000fL
    const SSL_METHOD*  ssl_meth;
#else
    SSL_METHOD*  ssl_meth;
#endif
    bool  fnoverify;
    bool  sslmode;

    SslClient (bool _fnoverify = false) {
	ephost = NULL;
	ssl_meth = NULL;
	fnoverify = _fnoverify;
	sslmode = false;
    };
    virtual  ~SslClient () {};

    virtual bool  connect (const HostSpec* conhost, const HostSpec* _ephost);
 protected:
    virtual bool  connect2 ();
    virtual bool  sslOpen ();
    virtual void  loadCAFile (const char* certfile, int depth = 5);
    virtual bool  setupPeerVerification (const char* certfile, int depth = 5);
    virtual bool  verifyCA ();
    virtual bool  verifyCN (const ustring& lhost, X509* cert);
    virtual bool  verifyAltName (const ustring& lhost, X509* cert);
    virtual bool  globMatch (const ustring& lhost, const ustring& name);
    virtual ssize_t  write2 (struct iovec* iov, int iovcnt);
    virtual ssize_t  read2 (void* buf, size_t nbytes);
};

class  HTTPSend;
class  ProxySslClient: public SslClient {
 public:
    ustring  proxyid;
    ustring  proxypw;

    ProxySslClient (bool _fnoverify = false): SslClient (_fnoverify) {};
    virtual  ~ProxySslClient () {};

 protected:
    virtual bool  connect2 ();
    virtual int  readReplyHead ();
    virtual ssize_t  read3 (void* buf, size_t nbytes);
};

#endif /* UTIL_TCP_H */
