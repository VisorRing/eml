#include "util_file.h"
#include "config.h"
#include "ustring.h"
#include "util_const.h"
#include "util_splitter.h"
#include "filemacro.h"
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>

bool  isPlainFile (const ustring& name) {
    struct stat  sb;

    if (name.size () > 0 && stat (name.c_str (), &sb) == 0 && S_ISREG (sb.st_mode)) {
	return true;
    } else {
	return false;
    }
}

bool  isExecutableFile (const ustring& name) {
    struct stat  sb;

    if (name.size () > 0 && stat (name.c_str (), &sb) == 0 && (sb.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH))) {
	return true;
    } else {
	return false;
    }
}

bool  isDirectory (const ustring& name) {
    struct stat  sb;

    if (name.size () > 0 && stat (name.c_str (), &sb) == 0 && S_ISDIR (sb.st_mode)) {
	return true;
    } else {
	return false;
    }
}

bool  isPlainOrNoFile (const ustring& name) {
    struct stat  sb;

    if (name.size () > 0) {
	if (stat (name.c_str (), &sb) == 0) {
	    if (S_ISREG (sb.st_mode)) {
		return true;
	    } else {
		return false;
	    }
	} else {
	    if (errno == ENOENT) {
		return true;
	    } else {
		return true;
		return false;
	    }
	}
    } else {
	return false;
    }
}

bool  fileSize (const ustring& name, off_t& size) {
    struct stat  sb;

    if (name.size () > 0 && stat (name.c_str (), &sb) == 0 && S_ISREG (sb.st_mode)) {
	size = sb.st_size;
	return true;
    } else {
	return false;
    }
}

bool  readFile (const ustring& filename, ustring& ans, size_t max) {
    if (filename.size () > 0) {
	FileMacro  f;
	off_t  s;

	if (f.openRead (filename.c_str ())) {
	    s = f.size ();
	    if (max > 0 && s > max)
		throw (filename + uErrorFileSize);
	    ans.resize (s);
	    if (s > 0)
		f.read (&ans.at (0), s);
	    f.close ();
	} else {
	    return false;
	}
    } else {
	return false;		// NG
    }
    return true;		// OK
}

void  writeFile (const ustring& filename, ustring& data) {
    if (filename.size () > 0) {
	FileMacro  f;
	if (f.openWrite (filename.c_str ())) {
	    if (data.size () > 0)
		f.write (&data.at (0), data.size ());
	}
    }
}

/*
  top must end with slash.
*/
void  makeSubdir (ustring& top, const ustring& sub) {
    SplitterCh  sp (sub, '/');

    while (sp.next ()) {
	top.append (sp.b, sp.u);
	mkdir (top.c_str (), 0777);
#ifdef DEBUG2
	std::cerr << "mkdir:" << top << "\n";
#endif /* DEBUG */
    }
}

void  shapePath (ustring& path) {
    static uregex  re1 ("//+");
    static uregex  re2 ("[^/]+/\\.\\.(/|$)");
    static uregex  re3 ("(/|^)\\.(/|$)");
    static uregex  re4 ("^/\\.\\./");
    umatch  m;
    uiterator  b, e;

    while (usearch (path, m, re1)) {
	b = path.begin ();
	e = path.end ();
	path = ustring (b, m[0].first + 1).append (m[0].second, e);
    }
    while (usearch (path, m, re2)) {
	if (m[1].first == m[1].second) { // '$'
	    path.resize (m[0].first - path.begin ());
	} else {		// '/'
	    b = path.begin ();
	    e = path.end ();
	    path = ustring (b, m[0].first).append (m[0].second, e);
	}
    }
    while (usearch (path, m, re3)) {
	switch (m[0].second - m[0].first) {
	case 1:
	    goto Bp1;
	case 2:
	    b = path.begin ();
	    e = path.end ();
	    path = ustring (b, m[0].first).append (m[0].second, e);
	    break;
	case 3:
	    b = path.begin ();
	    e = path.end ();
	    path = ustring (b, m[0].first).append (m[2].first, e);
	    break;
	default:;
	    assert (0);
	}
    }
    while (usearch (path, m, re4)) {
	e = path.end ();
	path = ustring (m[0].second - 1, e);
    }
 Bp1:;
}

bool  isAbsolutePath (const ustring& path) {
    return (path.length () > 0 && path[0] == '/');
}
