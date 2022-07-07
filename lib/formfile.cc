#include "formfile.h"
#include "motorenv.h"
#include "util_const.h"
#include "util_string.h"
#include "ustring.h"
#include "filemacro.h"
#include <sys/types.h>
#include <unistd.h>
#include <sys/mman.h>
#include <assert.h>

int  CGIFormFile::partAt (int i) {
    if (i >= 0) {
	sary_t::iterator  it = datamap.find (i);
	if (it == datamap.end ()) {
	    return -1;
	} else {
	    return it->second;
	}
    } else {
	return -1;
    }
}

ustring  CGIFormFile::typeAt (int i) {
    if (i >= 0) {
	tary_t::iterator  it = typemap.find (i);
	if (it == typemap.end ()) {
	    return uEmpty;
	} else {
	    return it->second;
	}
    } else {
	return uEmpty;
    }
}

void  CGIFormFile::read_multipart (MotorEnv* env) {
#ifdef DEBUG2
    std::cerr << "boundary:" << boundary << "\n";
#endif /* DEBUG */
    compileReg ();
    if (saveData (env)) {
	searchPart (env);
#ifdef DEBUG2
	dump (std::cerr);
	for (int i = 0; i < parts.size (); i ++) {
	    std::cerr << "file:" << i << " (" << (void*)parts[i].first << ", " << (void*)parts[i].second << ")\n";
	}
#endif /* DEBUG */
    }
}

bool  CGIFormFile::saveData (MotorEnv* env) {
    static const size_t  bsize = 65536 * 4;
    size_t  size = postSize ();
    ustring  b;
    size_t  s;
    pid_t  mypid = getpid ();

    b.resize (bsize);
    if (size == 0 || size > env->appenv->postfilelimit)
	return false;		// NG
    tmpfile = env->path_to_posttemp ();
    mapsize = 0;
    if (fp.openReadWrite (tmpfile.c_str ())) {
	unlink (tmpfile.c_str ());
	while (size > 0) {
	    s = (size < bsize) ? size : bsize;
	    s = std::cin.read (&b[0], s).gcount ();
	    if (s <= 0)
		break;
	    ::write (fp.fd, &b[0], s);
	    size -= s;
	    mapsize += s;
	}
//	fp.close ();	mmapした後にクローズする。
//	std::cerr << "post-file[" << mypid << ":" << env->scriptName << "]: done    " << mapsize << "Bytes\n";		// 出力サイズが大きいと、デッドロックする。
	mapdata = (char*)mmap (NULL, mapsize, PROT_READ, MAP_PRIVATE, fp.fd, 0);
#ifdef DEBUG2
	std::cerr << (void*) mapdata << ": " << mapsize << "\n";
#endif /* DEBUG */
    } else {
	throw (ustring (CharConst ("configuration error: can't open temporary file.")));
    }
    return true;
}

void  CGIFormFile::unlinkTmpFile () {
    if (mapdata != NULL) {
	munmap (mapdata, mapsize);
	mapdata = NULL;
    }
    if (tmpfile.size () > 0) {
//	unlink (tmpfile.c_str ());
	tmpfile.resize (0);
    }
}

void  CGIFormFile::searchPart (MotorEnv* env) {
    char*  b = mapdata;
    char*  e = mapdata + mapsize;
    char*  x;
    boost::match_results<char*>  m;
    ustring  disp;
    ustring  name;
    ustring  filename;
    ustring  type;
    ustring  v;
    size_t  size;

#ifdef DEBUG2
    std::cerr << "b:" << (void*)b << " e:" << (void*)e << "\n";
    std::cerr << "mapdata:" << ustring (b, b + 40) << "\n";
#endif /* DEBUG */
    if (b != e && regex_search (b, e, m, re1, boost::regex_constants::match_single_line)) {
#ifdef DEBUG2
	std::cerr << "match:" << ustring (m[0].first, m[0].second) << "\n";
#endif /* DEBUG */
	b = m[0].second;
	while (b != e && regex_search (b, e, m, reN, boost::regex_constants::match_single_line)) {
	    x = m[0].first;
#ifdef DEBUG2
	    std::cerr << "match:" << ustring (m[0].first, m[0].second) << "\n";
#endif /* DEBUG */
	    readMimeHead (b, x, disp, name, filename, type);
#ifdef DEBUG2
	    std::cerr << "disp:" << disp << " name:" << name << " filename:" << filename << " type:" << type << "\n";
#endif /* DEBUG */
	    if (filename.size () > 0) {
		int  k1, k2;
		k2 = parts.size ();
		fix (name);
		parts.push_back (part (b, x));
		fix (filename);
		fix (type);
		k1 = insert (iarg, name, filePart_osSafe (filename));
		datamap.insert (sary_t::value_type (k1, k2));
		typemap.insert (tary_t::value_type (k2, type));
#ifdef DEBUG2
		std::cerr << "insert(" << k1 << "," << k2 << ")\n";
#endif /* DEBUG */
	    } else {
		// no filename
		size = x - b;
		if (size < env->appenv->postlimit) {
		    v = ustring (b, x);
		    name = omitNul (name);
		    fix (name);
		    v = omitNul (v);
		    fix (v);
		    insert (iarg, name, v);
		} else {
		    *env->log << "form variable '" << name << "': size limit.\n";
		}
	    }

	    b = m[0].second;
	    if (e - b < 2) {
		break;
	    } else if (b[0] == '-' && b[1] == '-') {
#ifdef DEBUG2
		std::cerr << "break\n";
#endif /* DEBUG */
		break;
	    } else if (b[0] == '\r' && b[1] == '\n') {
		b += 2;
	    } else {
		break;		// format error;
	    }
	}
    }
}

void  CGIFormFile::compileReg () {
    ustring  a;
    ustring  t = escape_re (boundary);

    a.append (CharConst ("^--"));
    a.append (t);
    a.append (uCRLF);
#ifdef DEBUG2
    std::cerr << "re1:" << a << "\n";
#endif /* DEBUG */
    re1.assign (a);
    a = uCRLF;
    a.append (CharConst ("--"));
    a.append (t);
#ifdef DEBUG2
    std::cerr << "reN:" << a << "\n";
#endif /* DEBUG */
    reN.assign (a);
}

class  ChSplitterNL {
 public:
    char*  b;		// 先頭
    char*  t;		// 区切り文字列先頭
    char*  u;		// 区切り文字列末尾
    char*  e;		// 末尾

    ChSplitterNL (char* _begin, char* _end) {
	b = t = u = _begin;
	e = _end;
    };
    ~ChSplitterNL () {};

    bool  isEnd () {
	return b == e;
    };
    ustring  pre () {
	return ustring (b, t);
    };
    bool  next () {
	b = t = u;
	if (b < e) {
	    if (findNL ()) {
	    } else {
		t = u = e;
	    }
	    return true;
	} else {
	    return false;
	}
    };
    bool  nextSep () {
	b = t = u;
	if (b < e) {
	    if (findNL ()) {
		return true;
	    } else {
		t = u = e;
		return false;
	    }
	} else {
	    t = u = e;
	    return false;
	}
    };
    bool  findNL () {
	for (; t < e; ++ t) {
	    if (*t == '\n') {
		u = t + 1;
		return true;
	    } else if (*t == '\r') {
		u = t + 1;
		if (u < e && *u == '\n')
		    ++ u;
		return true;
	    }
	}
	return false;
    };
};

void  CGIFormFile::readMimeHead (char*& b, char* e, ustring& disp, ustring& name, ustring& filename, ustring& type) {
    ChSplitterNL  sp (b, e);
    boost::match_results<char*>  m2;
    static uregex  re_disp1 ("^Content-Disposition:\\s*(.*);\\s*name=\"(.*)\";\\s*filename=\"(.*)\"$");
    static uregex  re_disp2 ("^Content-Disposition:\\s*(.*);\\s*name=\"(.*)\"$");
    static uregex  re_type ("^Content-Type:\\s*([a-zA-Z_0-9/.+-]*)(;\\s*(.*))?$");

    disp.resize (0);
    name.resize (0);
    filename.resize (0);
    type.resize (0);
    while (sp.next ()) {
#ifdef DEBUG2
	std::cerr << "line:" << sp.pre () << "\n";
#endif /* DEBUG */
	if (sp.b == sp.t) {
	    b = sp.u;
	    break;
	}
	if (regex_search (sp.b, sp.t, m2, re_disp1, boost::regex_constants::match_single_line)) {
	    disp.assign (m2[1].first, m2[1].second - m2[1].first);
	    name.assign (m2[2].first, m2[2].second - m2[2].first);
	    filename.assign (m2[3].first, m2[3].second - m2[3].first);
	} else if (regex_search (sp.b, sp.t, m2, re_disp2, boost::regex_constants::match_single_line)) {
	    disp.assign (m2[1].first, m2[1].second - m2[1].first);
	    name.assign (m2[2].first, m2[2].second - m2[2].first);
	} else if (regex_search (sp.b, sp.t, m2, re_type, boost::regex_constants::match_single_line)) {
	    type.assign (m2[1].first, m2[1].second - m2[1].first);
	} else {
#ifdef DEBUG2
	    std::cerr << "not match:" << sp.pre () << "\n";
#endif /* DEBUG */
	}
    }
}

bool  CGIFormFile::saveFile (int i, const ustring& path, size_t max) {
    static size_t  bsize = 65536;
    part  p;
    char*  b;
    size_t  s, size;
    FileMacro  f;

    if (0 <= i && i < parts.size ()) {
	p = parts[i];
	assert (mapdata <= p.first && p.first <= p.second && p.second < mapdata + mapsize);
	b = p.first;
	size = p.second - p.first;
	if (max > 0 && size > max)
	    return false;

#ifdef DEBUG2
	std::cerr << "saveFile(" << i << "," << path << "): " << (void*)p.first << "--" << (p.second - p.first) << "\n";
#endif /* DEBUG */
	f.openWrite (path.c_str ());
#ifdef DEBUG2
	std::cerr << "write:" << path << "\n";
#endif /* DEBUG */
	while (size > 0) {
	    s = (size < bsize) ? size : bsize;
	    s = ::write (f.fd, b, s);
	    size -= s;
	    b += s;
	}
	f.close ();
	return true;
    }
    return false;
}

