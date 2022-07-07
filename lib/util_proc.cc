#include "util_proc.h"
#include "config.h"
#include "ustring.h"
#include <iostream>
#include <strings.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>

#ifdef HEAPDEBUG
void  mdbClose ();
#endif

int  exec_cmd (char* const argv[]) {
    pid_t  pid;
    int  status;

    pid = fork ();
    if (pid < 0) {		// error
	throw (ustring (CharConst ("fork failed.")));
    }
    if (pid == 0) {
	// child
	close (0);
	open (path_devnull, O_RDONLY);
	close (1);
	open (path_devnull, O_WRONLY);
//	close (2);
//	open (path_devnull, O_WRONLY);
	execv (argv[0], argv);
#ifdef HEAPDEBUG
	mdbClose ();
#endif
	exit (1);		// heap lost
    } else {
	// parent
	waitpid (pid, &status, 0);
    }
    return (status >> 8);
}


int  ProcRW::open (char* const argv[], ustring* dir) {
    int  fd1[2];
    int  fd2[2];

    pipe (fd1);
    pipe (fd2);
    pid = fork ();
    if (pid < 0) {		// error
	::close (fd1[0]);
	::close (fd1[1]);
	::close (fd2[0]);
	::close (fd2[1]);
	throw (ustring (CharConst ("fork failed.")));
//	return 0;		// NG
    }
    if (pid == 0) {
	// child
#ifdef DEBUG2
	std::cerr << "fork child pid:" << getpid () << "\n";
	atexit (NULL);
#endif /* DEBUG */
	::close (fd1[1]);
	::close (fd2[0]);
	::close (0);		// stdin
	dup (fd1[0]);		// --stdin
	::close (fd1[0]);
	::close (1);		// stdout
	dup (fd2[1]);		// --stdout
	::close (fd2[1]);
	if (dir) {
	    chdir (dir->c_str ());
	}
#ifdef DEBUG2
	fprintf (stderr, "exec %s\n", argv[0]);
#endif /* DEBUG */
	execv (argv[0], argv);
#ifdef DEBUG
	char*  p = rindex (argv[0], '/');
	if (p)
	    std::cerr << (++ p) << ": can't execute.\n";
	else
	    std::cerr << argv[0] << ": can't execute.\n";
#endif /* DEBUG */
#ifdef HEAPDEBUG
	mdbClose ();
#endif
	exit (1);
    } else {
	// parent
	::close (fd1[0]);
	::close (fd2[1]);
	wfd = fd1[1];
	rfd = fd2[0];
    }
    return 1;			// OK
}

int  ProcRW::close () {
    int  status = 0;

    if (wfd >= 0)
	::close (wfd);
    if (rfd >= 0)
	::close (rfd);
    if (pid > 0)
	waitpid (pid, &status, 0);
    rfd = wfd = -1;
    pid = 0;

    return (status >> 8);
}

void  ProcRW::closeWriter () {
    if (wfd >= 0)
	::close (wfd);
    wfd = -1;
}

ssize_t  ProcRW::read (char* b, size_t n) {
    return ::read (rfd, b, n);
}

void  ProcRW::read (ustring& ans) {
    char*  b = (char*)malloc (4096);
    ssize_t  s;

//    while ((s = ::read (rfd, b, 4096)) > 0) {
    while ((s = read (b, 4096)) > 0) {
	ans.append (b, s);
    }

    free (b);
}

void  ProcRW::writeln (const ustring& text) {
    ustring  a (text);

    a.append (CharConst ("\n"));
    ::write (wfd, a.data (), a.size ());
}

void  ProcRW::write (const char* v, ssize_t size) {
    ::write (wfd, v, size);
}

void  ProcRW::write (const ustring& text) {
    ::write (wfd, text.data (), text.size ());
}
