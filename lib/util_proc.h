#ifndef UTIL_PROC_H
#define UTIL_PROC_H

#include "ustring.h"
//#include <boost/circular_buffer.hpp>
#include <unistd.h>

class  ProcRW {
 public:
    pid_t  pid;
    int  rfd;
    int  wfd;
//    boost::circular_buffer<int> cb(3);
//    boost::circular_buffer<char>  buf(4096);

    ProcRW () {
	pid = 0;
	rfd = -1;
	wfd = -1;
    };
    virtual  ~ProcRW () {
	close ();
    };
    virtual int  open (char* const argv[], ustring* dir = NULL);
    virtual int  close ();
    virtual void  closeWriter ();
    virtual ssize_t  read (char* b, size_t n);
    virtual void  read (ustring& ans);
    virtual void  writeln (const ustring& text);
    virtual void  write (const char* v, ssize_t size);
    virtual void  write (const ustring& text);
};

int  exec_cmd (char* const argv[]);

#endif /* UTIL_PROC_H */
