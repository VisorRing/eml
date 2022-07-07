#ifndef SIGSAFE_H
#define SIGSAFE_H

#include <signal.h>
#include <assert.h>
#include <stdlib.h>

extern int  SigSafeFlag;
class  SigSafe {
 public:
    void  (*sighup)(int);
    void  (*sigint)(int);
    void  (*sigquit)(int);
    void  (*sigpipe)(int);
    void  (*sigterm)(int);

    SigSafe () {
	sighup = signal (SIGHUP, handler);
	sigint = signal (SIGINT, handler);
	sigquit = signal (SIGQUIT, handler);
	sigpipe = signal (SIGPIPE, handler);
	sigterm = signal (SIGTERM, handler);
    };
    ~SigSafe () {
	signal (SIGHUP, sighup);
	signal (SIGINT, sigint);
	signal (SIGQUIT, sigquit);
	signal (SIGPIPE, sigpipe);
	signal (SIGTERM, sigterm);

	if (SigSafeFlag) {
	    void  (*fn)(int);
	    int  s = SigSafeFlag;

	    switch (s) {
	    case SIGHUP:
		fn = sighup;
		break;
	    case SIGINT:
		fn = sigint;
		break;
	    case SIGQUIT:
		fn = sigquit;
		break;
	    case SIGPIPE:
		fn = sigpipe;
		break;
	    case SIGTERM:
		fn = sigterm;
		break;
	    default:
		assert (0);
	    }
	    if (fn == SIG_DFL) {
		exit (1);
	    } else {
		if (fn != handler)
		    fn (s);
	    }
	}
    };

    static void  handler (int sig) {
	SigSafeFlag = sig;
    };
};

#endif /* SIGSAFE_H */
