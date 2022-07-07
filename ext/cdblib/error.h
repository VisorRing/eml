#ifndef ERROR_H
#define ERROR_H

#include <errno.h>
#ifdef EINTR
#define error_intr EINTR
#else
#define error_intr -1
#endif
#ifdef ENOMEM
#define error_nomem ENOMEM
#else
#define error_nomem -2
#endif
#ifdef ENOENT
#define error_noent ENOENT
#else
#define error_noent -3
#endif
#ifdef ETXTBSY
#define error_txtbsy ETXTBSY
#else
#define error_txtbsy -4
#endif
#ifdef EIO
#define error_io EIO
#else
#define error_io -5
#endif
#ifdef EEXIST
#define error_exist EEXIST
#else
#define error_exist -6
#endif
#ifdef ETIMEDOUT
#define error_timeout ETIMEDOUT
#else
#define error_timeout -7
#endif
#ifdef EINPROGRESS
#define error_inprogress EINPROGRESS
#else
#define error_inprogress -8
#endif
#ifdef EWOULDBLOCK
#define error_wouldblock EWOULDBLOCK
#else
#define error_wouldblock -9
#endif
#ifdef EAGAIN
#define error_again EAGAIN
#else
#define error_again -10
#endif
#ifdef EPIPE
#define error_pipe EPIPE
#else
#define error_pipe -11
#endif
#ifdef EPERM
#define error_perm EPERM
#else
#define error_perm -12
#endif
#ifdef EACCES
#define error_access EACCES
#else
#define error_access -13
#endif
#ifdef ENXIO
#define error_nodevice ENXIO
#else
#define error_nodevice -14
#endif
#ifdef EPROTO
#define error_proto EPROTO
#else
#define error_proto -15
#endif

extern char *error_str(int);
extern int error_temp(int);

#endif
