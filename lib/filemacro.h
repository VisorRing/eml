#ifndef FILEMACRO_H
#define FILEMACRO_H

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <unistd.h>
#include <assert.h>
#ifndef  HAVE_OPENLOCK
#include <sys/file.h>
#endif

class  FileMacro {
 public:
    int  fd;

    FileMacro (): fd (-1) {};
    virtual  ~FileMacro () {
	close ();
    };

    virtual bool  open (const char* path, int flags) {
	assert (fd < 0);
	fd = ::open (path, flags);
	return (fd >= 0);
    };
    virtual bool  open (const char* path, int flags, mode_t mode) {
	assert (fd < 0);
	fd = ::open (path, flags, mode);
	return (fd >= 0);
    };
    virtual bool  openRead (const char* path) {
	return open (path, O_RDONLY);
    };
    virtual bool  openReadLock (const char* path) {
#ifdef  HAVE_OPENLOCK
	return open (path, O_RDONLY | O_SHLOCK);
#else
	if (open (path, O_RDONLY)) {
	    flock (fd, LOCK_SH);
	    return true;
	} else {
	    return false;
	}
#endif
    };
    virtual bool  openWrite (const char* path) {
	return open (path, O_WRONLY | O_TRUNC | O_CREAT, 0666);
    };
    virtual bool  openReadWrite (const char* path) {
	return open (path, O_RDWR | O_CREAT, 0666);
    };
    virtual bool  openAppendLock (const char* path) {
#ifdef  HAVE_OPENLOCK
	return open (path, O_WRONLY | O_CREAT | O_APPEND | O_EXLOCK, 0666);
#else
	if (open (path, O_WRONLY | O_CREAT | O_APPEND, 0666)) {
	    flock (fd, LOCK_EX);
	    return true;
	} else {
	    return false;
	}
#endif
    };
    virtual void  close () {
	if (fd >= 0)
	    ::close (fd);
	fd = -1;
    };
    virtual int  isOpen () {
	return (fd != -1);
    };
    virtual off_t  size () {
	struct stat  sb;

	if (fstat (fd, &sb) == 0) {
	    return sb.st_size;
	} else {
	    return 0;
	}
    };
    virtual ssize_t  read (void* buf, size_t n) {
	return ::read (fd, buf, n);
    };
    virtual ssize_t  write (const void* buf, size_t n) {
	return ::write (fd, buf, n);
    };
    virtual off_t  seekTo (off_t offset) {
	return lseek (fd, offset, SEEK_SET);
    };
    virtual off_t  seekEnd (off_t offset) {
	return lseek (fd, offset, SEEK_END);
    };
};

#endif /* FILEMACRO_H */
