/*	$OpenBSD: diffreg.c,v 1.67 2007/03/18 21:12:27 espie Exp $	*/

/*
 * Copyright (C) Caldera International Inc.  2001-2002.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code and documentation must retain the above
 *    copyright notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed or owned by Caldera
 *	International, Inc.
 * 4. Neither the name of Caldera International, Inc. nor the names of other
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * USE OF THE SOFTWARE PROVIDED FOR UNDER THIS LICENSE BY CALDERA
 * INTERNATIONAL, INC. AND CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL CALDERA INTERNATIONAL, INC. BE LIABLE FOR ANY DIRECT,
 * INDIRECT INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
/*-
 * Copyright (c) 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)diffreg.c   8.1 (Berkeley) 6/6/93
 */

#include <sys/param.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include <ctype.h>
//#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

#include "diff.h"
//#include "pathnames.h"
//#include "variable.h"
#include "mlenv.h"
#include "ustring.h"
//#ifdef HEAPDEBUG
//#include "heapdebug.h"
//#endif

/*
 * Output format options
 */
#define	D_NORMAL	0	/* Normal output */
#define	D_EDIT		-1	/* Editor script out */
#define	D_REVERSE	1	/* Reverse editor script */
#define	D_CONTEXT	2	/* Diff with context */
#define	D_UNIFIED	3	/* Unified context diff */
#define	D_IFDEF		4	/* Diff with merged #ifdef's */
#define	D_NREVERSE	5	/* Reverse ed script with numbered
				   lines and no trailing . */
#define	D_BRIEF		6	/* Say if the files differ */

/*
 * Output flags
 */
#define	D_HEADER	1	/* Print a header/footer between files */
#define	D_EMPTY1	2	/* Treat first file as empty (/dev/null) */
#define	D_EMPTY2	4	/* Treat second file as empty (/dev/null) */

/*
 * Status values for print_status() and diffreg() return values
 */
#define	D_SAME		0	/* Files are the same */
#define	D_DIFFER	1	/* Files are different */
#define	D_BINARY	2	/* Binary files are different */
#define	D_COMMON	3	/* Subdirectory common to both dirs */
#define	D_ONLY		4	/* Only exists in one directory */
#define	D_MISMATCH1	5	/* path1 was a dir, path2 a file */
#define	D_MISMATCH2	6	/* path1 was a file, path2 a dir */
#define	D_ERROR		7	/* An error occurred */
#define	D_SKIPPED1	8	/* path1 was a special file */
#define	D_SKIPPED2	9	/* path2 was a special file */


static __inline int min(int, int);

class  BBlockFILE {
 public:
//    BBlock*  text;
//    BBlock  ptr;
    ustring*  text;
    uiterator  b;
    uiterator  e;
    int  err;

//    BBlockFILE (BBlock* p): text (p) {
    BBlockFILE (ustring* p): text (p) {
//	ptr.assign (text);
	b = text->begin ();
	e = text->end ();
	err = 0;
    };
    virtual  ~BBlockFILE () {};
    virtual void  rewind () {
//	ptr.assign (text);
	b = text->begin ();
	e = text->end ();
    };
    virtual int  mgetc () {
//	if (ptr.length > 0) {
//	    return ptr.shift ();
	if (b < e) {
	    uiterator  i = b;
	    b ++;
	    return *i;
	} else {
	    return EOF;
	    err = 1;
	}
    };
    virtual int  merror () {
	return err;
    };
    virtual size_t  fread (char* p, size_t s, size_t n) {
	size_t  ans = min (e - b, s * n) / s;
	memcpy (p, &b[0], ans * s);
	b += ans * s;
	return ans;
    };
    virtual size_t  fread (u_char* p, size_t s, size_t n) {
	return fread ((char*)p, s, n);
    };
    virtual int  fseek (long offset, int whence) {
	switch (whence) {
	case SEEK_SET:
	    if (offset <= text->length ()) {
//		ptr.assign (text->start + offset, text->length - offset);
		b = text->begin () + offset;
		e = text->end ();
		return 0;
	    } else {
//		ptr.assign (text->start + text->length, 0);
		b = text->end ();
		e = text->end ();
		err = 1;
		return -1;
	    }
	    break;
	case SEEK_CUR:
	    if (offset <= e - b) {
//		ptr.skip (offset);
		b += offset;
		return 0;
	    } else {
//		ptr.skip (ptr.length);
		b = e;
		err = 1;
		return -1;
	    }
	    break;
	case SEEK_END:
	    if (offset <= text->length ()) {
//		ptr.assign (text->start + text->length - offset, offset);
		e = text->end ();
		b = e - offset;
		return 0;
	    } else {
//		ptr.assign (text);
		b = text->begin ();
		e = text->end ();
		err = 1;
		return -1;
	    }
	    break;
	default:;
	    return -1;
	}
    };
};

#if 0
class  OutputVar {
public:
    ustring  vlc;
    ustring  vll;
    ustring  vrc;
    ustring  vrl;
    MlEnv*  mlenv;
    int  n;

    OutputVar (ustring v1, ustring v2, ustring v3, ustring v4, MlEnv* e) {
	vlc = v1;
	vll = v2;
	vrc = v3;
	vrl = v4;
	mlenv = e;
	n = 0;
    };
    virtual void  out (char c1, ustring* v1, char c2, ustring* v2) {
//	u_char  b[4];
//	uiterator  p, q;

	n ++;
	mlenv->setAry (vlc, n, newMNode_str (new ustring (1, c1)));
	mlenv->setAry (vll, n, newMNode_str (new ustring (*v1)));
	mlenv->setAry (vrc, n, newMNode_str (new ustring (1, c2)));
	mlenv->setAry (vrl, n, newMNode_str (new ustring (*v2)));
    };
    virtual void  linenum (int n1, int n2) {
	static ustring  p (CharConst ("#"));

	n ++;
	mlenv->setAry (vlc, n, newMNode_str (new ustring (p)));
	mlenv->setAry (vll, n, newMNode_num (n1));
	mlenv->setAry (vrc, n, newMNode_str (new ustring (p)));
	mlenv->setAry (vrl, n, newMNode_num (n2));
    };
    virtual void  finish () {
	mlenv->setArySize (vlc, n);
	mlenv->setArySize (vll, n);
	mlenv->setArySize (vrc, n);
	mlenv->setArySize (vrl, n);
    };
};
#endif
class  OutputList {
public:
    MNodeList  vlc;
    MNodeList  vll;
    MNodeList  vrc;
    MNodeList  vrl;
    MlEnv*  mlenv;

    OutputList (MlEnv* e) {
	mlenv = e;
    };
    virtual  ~OutputList () {};

    virtual void  out (char c1, ustring* v1, char c2, ustring* v2) {
	vlc.append (newMNode_str (new ustring (1, c1)));
	vll.append (newMNode_str (new ustring (*v1)));
	vrc.append (newMNode_str (new ustring (1, c2)));
	vrl.append (newMNode_str (new ustring (*v2)));
    };
    virtual void  linenum (int n1, int n2) {
	static ustring  p (CharConst ("#"));
	vlc.append (newMNode_str (new ustring (p)));
	vll.append (newMNode_num (n1));
	vrc.append (newMNode_str (new ustring (p)));
	vrl.append (newMNode_num (n2));
    };
};

/*
 * diff - compare two files.
 */

/*
 *	Uses an algorithm due to Harold Stone, which finds
 *	a pair of longest identical subsequences in the two
 *	files.
 *
 *	The major goal is to generate the match vector J.
 *	J[i] is the index of the line in file1 corresponding
 *	to line i file0. J[i] = 0 if there is no
 *	such line in file1.
 *
 *	Lines are hashed so as to work in core. All potential
 *	matches are located by sorting the lines of each file
 *	on the hash (called ``value''). In particular, this
 *	collects the equivalence classes in file1 together.
 *	Subroutine equiv replaces the value of each line in
 *	file0 by the index of the first element of its
 *	matching equivalence in (the reordered) file1.
 *	To save space equiv squeezes file1 into a single
 *	array member in which the equivalence classes
 *	are simply concatenated, except that their first
 *	members are flagged by changing sign.
 *
 *	Next the indices that point into member are unsorted into
 *	array class according to the original order of file0.
 *
 *	The cleverness lies in routine stone. This marches
 *	through the lines of file0, developing a vector klist
 *	of "k-candidates". At step i a k-candidate is a matched
 *	pair of lines x,y (x in file0 y in file1) such that
 *	there is a common subsequence of length k
 *	between the first i lines of file0 and the first y
 *	lines of file1, but there is no such subsequence for
 *	any smaller y. x is the earliest possible mate to y
 *	that occurs in such a subsequence.
 *
 *	Whenever any of the members of the equivalence class of
 *	lines in file1 matable to a line in file0 has serial number
 *	less than the y of some k-candidate, that k-candidate
 *	with the smallest such y is replaced. The new
 *	k-candidate is chained (via pred) to the current
 *	k-1 candidate so that the actual subsequence can
 *	be recovered. When a member has serial number greater
 *	that the y of all k-candidates, the klist is extended.
 *	At the end, the longest subsequence is pulled out
 *	and placed in the array J by unravel
 *
 *	With J in hand, the matches there recorded are
 *	check'ed against reality to assure that no spurious
 *	matches have crept in due to hashing. If they have,
 *	they are broken, and "jackpot" is recorded--a harmless
 *	matter except that a true match for a spuriously
 *	mated line may now be unnecessarily reported as a change.
 *
 *	Much of the complexity of the program comes simply
 *	from trying to minimize core utilization and
 *	maximize the range of doable problems by dynamically
 *	allocating what is needed and reusing what is not.
 *	The core requirements for problems larger than somewhat
 *	are (in words) 2*length(file0) + length(file1) +
 *	3*(number of k-candidates installed),  typically about
 *	6n words for files of length n.
 */

struct cand {
	int x;
	int y;
	int pred;
};

struct line {
	int serial;
	int value;
} *file[2];

/*
 * The following struct is used to record change information when
 * doing a "context" or "unified" diff.  (see routine "change" to
 * understand the highly mnemonic field names)
 */
struct context_vec {
	int a;			/* start line in old file */
	int b;			/* end line in old file */
	int c;			/* start line in new file */
	int d;			/* end line in new file */
};

static int  *J;			/* will be overlaid on class */
static int  *vclass;		/* will be overlaid on file[0] */
static int  *klist;		/* will be overlaid on file[0] after class */
static int  *member;		/* will be overlaid on file[1] */
static int   clen;
//static int   inifdef;		/* whether or not we are in a #ifdef block */
static int   len[2];
static int   pref, suff;	/* length of prefix and suffix */
static int   slen[2];
static int   anychange;
static long *ixnew;		/* will be overlaid on file[1] */
static long *ixold;		/* will be overlaid on klist */
static struct cand *clist;	/* merely a free storage pot for candidates */
static int   clistlen;		/* the length of clist */
static struct line *sfile[2];	/* shortened by pruning common prefix/suffix */
static u_char *chrtran;		/* translation table for case-folding */
static struct context_vec *context_vec_start;
static struct context_vec *context_vec_end;
static struct context_vec *context_vec_ptr;

#define FUNCTION_CONTEXT_SIZE	55
static char lastbuf[FUNCTION_CONTEXT_SIZE];
static int lastline;
static int lastmatchline;

static int	bflag, dflag, iflag, wflag;
static int	format, context, status;

//static FILE *opentemp(const char *);
//static void output(char *, FILE *, char *, FILE *, int);
//static void  output(BBlockFILE* f1, BBlockFILE* f2, int flags, OutputVar* out);
static void  output(BBlockFILE* f1, BBlockFILE* f2, int flags, OutputList* out);
//static void check(char *, FILE *, char *, FILE *);
static void  check(BBlockFILE *f1, BBlockFILE *f2);
//static void range(int, int, char *);
//static void uni_range(int, int);
//static void dump_context_vec(FILE *, FILE *);
//static void  dump_context_vec(BBlockFILE*f1, BBlockFILE*f2);
//static void  dump_context_vec(BBlockFILE*f1, BBlockFILE*f2, OutputVar* out);
static void  dump_context_vec(BBlockFILE*f1, BBlockFILE*f2, OutputList* out);
//static void dump_unified_vec(FILE *, FILE *);
//static void  dump_unified_vec(BBlockFILE* f1, BBlockFILE* f2);
//static void prepare(int, FILE *, off_t);
static void prepare(int, BBlockFILE *);
static void prune(void);
static void equiv(struct line *, int, struct line *, int, int *);
static void unravel(int);
static void unsort(struct line *, int, int *);
//static void change(char *, FILE *, char *, FILE *, int, int, int, int, int *);
//static void  change(BBlockFILE* f1, BBlockFILE* f2, int a, int b, int c, int d, int *pflags, OutputVar* out);
static void  change(BBlockFILE* f1, BBlockFILE* f2, int a, int b, int c, int d, int *pflags, OutputList* out);
static void sort(struct line *, int);
//static void print_header(const char *, const char *);
//static void print_header();
//static int  ignoreline(char *);
//static int  asciifile(FILE *);
//static int  fetch(long *, int, int, FILE *, int, int);
//static int  fetch(long *f, int a, int b, BBlockFILE*lb, int ch);
static int  newcand(int, int, int);
static int  search(int *, int, int);
//static int  skipline(FILE *);
static int  skipline(BBlockFILE *);
static int  isqrt(int);
static int  stone(int *, int, int *, int *);
//static int  readhash(FILE *);
static int  readhash(BBlockFILE *);
//static int  files_differ(FILE *, FILE *, int);
static int  files_differ (BBlockFILE* f1, BBlockFILE* f2, int flags);
static __inline int min(int, int);
static __inline int max(int, int);
//static char *match_function(const long *, int, FILE *);
//static char* match_function(const long *f, int pos, BBlockFILE *file);
//static char *preadline(int, size_t, off_t);


/*
 * chrtran points to one of 2 translation tables: cup2low if folding upper to
 * lower case clow2low if not folding case
 */
u_char clow2low[256] = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a,
	0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15,
	0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20,
	0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b,
	0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36,
	0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f, 0x40, 0x41,
	0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c,
	0x4d, 0x4e, 0x4f, 0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
	0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f, 0x60, 0x61, 0x62,
	0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d,
	0x6e, 0x6f, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78,
	0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f, 0x80, 0x81, 0x82, 0x83,
	0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e,
	0x8f, 0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99,
	0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f, 0xa0, 0xa1, 0xa2, 0xa3, 0xa4,
	0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf,
	0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba,
	0xbb, 0xbc, 0xbd, 0xbe, 0xbf, 0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5,
	0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf, 0xd0,
	0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xdb,
	0xdc, 0xdd, 0xde, 0xdf, 0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6,
	0xe7, 0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef, 0xf0, 0xf1,
	0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc,
	0xfd, 0xfe, 0xff
};

u_char cup2low[256] = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a,
	0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15,
	0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20,
	0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b,
	0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36,
	0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f, 0x60, 0x61,
	0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c,
	0x6d, 0x6e, 0x6f, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
	0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f, 0x60, 0x61, 0x62,
	0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d,
	0x6e, 0x6f, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78,
	0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f, 0x80, 0x81, 0x82, 0x83,
	0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e,
	0x8f, 0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99,
	0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f, 0xa0, 0xa1, 0xa2, 0xa3, 0xa4,
	0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf,
	0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba,
	0xbb, 0xbc, 0xbd, 0xbe, 0xbf, 0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5,
	0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf, 0xd0,
	0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xdb,
	0xdc, 0xdd, 0xde, 0xdf, 0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6,
	0xe7, 0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef, 0xf0, 0xf1,
	0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc,
	0xfd, 0xfe, 0xff
};

static void*  emalloc (size_t s) {
#ifdef HEAPDEBUG0
    void*  ans;
    ans = malloc (s);
    fmdbAdd (ans, s);
    return ans;
#else
    return malloc (s);
#endif
}
static void*  erealloc (void* p, size_t s) {
#ifdef HEAPDEBUG0
    void*  ans;
    if (p)
	mdbDelete (p);
    ans = realloc (p, s);
    fmdbAdd (ans, s);
    return ans;
#else
    return realloc (p, s);
#endif
}
static void  efree (void* p) {
#ifdef HEAPDEBUG0
    mdbDelete (p);
#endif
    free (p);
}

int
//diffreg(char *ofile1, char *ofile2, int flags)
//diffreg (ustring* text1, ustring* text2, ustring* varleftc, ustring* varleftline, ustring* varrightc, ustring* varrightline, MlEnv* mlenv)
diffreg (ustring* text1, ustring* text2, MNodePtr* pleftc, MNodePtr* pleftline, MNodePtr* prightc, MNodePtr* prightline, MlEnv* mlenv)
{
//	char *file1 = ofile1;
//	char *file2 = ofile2;
//	FILE *f1 = NULL;
//	FILE *f2 = NULL;
	BBlockFILE  f1 (text1);
	BBlockFILE  f2 (text2);
	int rval = D_SAME;
//	int i, ostdout = -1;
	int i;
//	pid_t pid = -1;
	int  flags;
//	OutputVar  out (*varleftc, *varleftline, *varrightc, *varrightline, mlenv);
	OutputList  out (mlenv);

	bflag = dflag = iflag = wflag = 0;
//	format = D_UNIFIED;
	format = D_CONTEXT;
	context = 1;
	status = 0;
	flags = 0;
	
	len[0] = len[1] = 0;
	pref = suff = 0;
	slen[0] = slen[1] = 0;
	anychange = 0;
	context_vec_start = context_vec_end = NULL;
	lastline = 0;
	lastmatchline = 0;
	context_vec_ptr = context_vec_start - 1;
	chrtran = (iflag ? cup2low : clow2low);
#if 0
	if (S_ISDIR(stb1.st_mode) != S_ISDIR(stb2.st_mode))
		return (S_ISDIR(stb1.st_mode) ? D_MISMATCH1 : D_MISMATCH2);
	if (strcmp(file1, "-") == 0 && strcmp(file2, "-") == 0)
		goto closem;
#endif
#if 0
	if (flags & D_EMPTY1)
		f1 = fopen(_PATH_DEVNULL, "r");
	else {
		if (!S_ISREG(stb1.st_mode)) {
			if ((f1 = opentemp(file1)) == NULL ||
			    fstat(fileno(f1), &stb1) < 0) {
				warn("%s", file1);
				status |= 2;
				goto closem;
			}
		} else if (strcmp(file1, "-") == 0)
			f1 = stdin;
		else
			f1 = fopen(file1, "r");
	}
	if (f1 == NULL) {
		warn("%s", file1);
		status |= 2;
		goto closem;
	}
#endif
#if 0
	if (flags & D_EMPTY2)
		f2 = fopen(_PATH_DEVNULL, "r");
	else {
		if (!S_ISREG(stb2.st_mode)) {
			if ((f2 = opentemp(file2)) == NULL ||
			    fstat(fileno(f2), &stb2) < 0) {
				warn("%s", file2);
				status |= 2;
				goto closem;
			}
		} else if (strcmp(file2, "-") == 0)
			f2 = stdin;
		else
			f2 = fopen(file2, "r");
	}
	if (f2 == NULL) {
		warn("%s", file2);
		status |= 2;
		goto closem;
	}
#endif
//	switch (files_differ(f1, f2, flags)) {
	switch (files_differ(&f1, &f2, flags)) {
	case 0:
		goto closem;
	case 1:
		break;
	default:
		/* error */
		status |= 2;
		goto closem;
	}

#if 0
	if (!asciifile(f1) || !asciifile(f2)) {
		rval = D_BINARY;
		status |= 1;
		goto closem;
	}
#endif
#if 0
	if (lflag) {
		/* redirect stdout to pr */
		int pfd[2];
		char *header;
		char *prargv[] = { "pr", "-h", NULL, "-f", NULL };

		easprintf(&header, "%s %s %s", diffargs, file1, file2);
		prargv[2] = header;
		fflush(stdout);
		rewind(stdout);
		pipe(pfd);
		switch ((pid = fork())) {
		case -1:
			warnx("No more processes");
			status |= 2;
//			free(header);
			efree(header);
			return (D_ERROR);
		case 0:
			/* child */
			if (pfd[0] != STDIN_FILENO) {
				dup2(pfd[0], STDIN_FILENO);
				close(pfd[0]);
			}
			close(pfd[1]);
			execv(_PATH_PR, prargv);
			_exit(127);
		default:
			/* parent */
			if (pfd[1] != STDOUT_FILENO) {
				ostdout = dup(STDOUT_FILENO);
				dup2(pfd[1], STDOUT_FILENO);
				close(pfd[1]);
			}
			close(pfd[0]);
			rewind(stdout);
//			free(header);
			efree(header);
		}
	}
#endif
//	prepare(0, f1, stb1.st_size);
//	prepare(1, f2, stb2.st_size);
	prepare(0, &f1);
	prepare(1, &f2);
	prune();
	sort(sfile[0], slen[0]);
	sort(sfile[1], slen[1]);

	member = (int *)file[1];
	equiv(sfile[0], slen[0], sfile[1], slen[1], member);
	member = (int*)erealloc(member, (slen[1] + 2) * sizeof(int));

	vclass = (int *)file[0];
	unsort(sfile[0], slen[0], vclass);
	vclass = (int*)erealloc(vclass, (slen[0] + 2) * sizeof(int));

	klist = (int*)emalloc((slen[0] + 2) * sizeof(int));
	clen = 0;
	clistlen = 100;
	clist = (struct cand*)emalloc(clistlen * sizeof(struct cand));
	i = stone(vclass, slen[0], member, klist);
//	free(member);
//	free(vclass);
	efree(member);
	efree(vclass);

	J = (int*)erealloc(J, (len[0] + 2) * sizeof(int));
	unravel(klist[i]);
//	free(clist);
//	free(klist);
	efree(clist);
	efree(klist);

	ixold = (long*)erealloc(ixold, (len[0] + 2) * sizeof(long));
	ixnew = (long*)erealloc(ixnew, (len[1] + 2) * sizeof(long));
//	check(file1, f1, file2, f2);
	check (&f1, &f2);
//	output(file1, f1, file2, f2, (flags & D_HEADER));
	output(&f1, &f2, (flags & D_HEADER), &out);
#if 0
	if (ostdout != -1) {
		int wstatus;

		/* close the pipe to pr and restore stdout */
		fflush(stdout);
		rewind(stdout);
		if (ostdout != STDOUT_FILENO) {
			close(STDOUT_FILENO);
			dup2(ostdout, STDOUT_FILENO);
			close(ostdout);
		}
		waitpid(pid, &wstatus, 0);
	}
#endif
closem:
//	out.finish ();
	*pleftc = out.vlc.release ();
	*pleftline = out.vll.release ();
	*prightc = out.vrc.release ();
	*prightline = out.vrl.release ();
	if (anychange) {
		status |= 1;
		if (rval == D_SAME)
			rval = D_DIFFER;
	}
#if 0
	if (f1 != NULL)
		fclose(f1);
	if (f2 != NULL)
		fclose(f2);
	if (file1 != ofile1)
//		free(file1);
		efree(file1);
	if (file2 != ofile2)
//		free(file2);
		efree(file2);
#endif
	if (J) {
	    efree (J);
	    J = NULL;
	}
	if (ixold) {
	    efree (ixold);
	    efree (ixnew);
	    ixold = ixnew = NULL;
	}
	if (context_vec_start) {
	    efree (context_vec_start);
	    context_vec_start = NULL;
	}
	return (rval);
}

/*
 * Check to see if the given files differ.
 * Returns 0 if they are the same, 1 if different, and -1 on error.
 * XXX - could use code from cmp(1) [faster]
 */
static int
//files_differ(FILE *f1, FILE *f2, int flags)
files_differ (BBlockFILE* f1, BBlockFILE* f2, int flags)
{
	char buf1[BUFSIZ], buf2[BUFSIZ];
	size_t i, j;

//	if ((flags & (D_EMPTY1|D_EMPTY2)) || stb1.st_size != stb2.st_size ||
//	    (stb1.st_mode & S_IFMT) != (stb2.st_mode & S_IFMT))
	if ((flags & (D_EMPTY1|D_EMPTY2))
	    || f1->text->length () != f2->text->length ())
		return (1);
	for (;;) {
//		i = fread(buf1, 1, sizeof(buf1), f1);
//		j = fread(buf2, 1, sizeof(buf2), f2);
		i = f1->fread(buf1, 1, sizeof(buf1));
		j = f2->fread(buf2, 1, sizeof(buf2));
		if (i != j)
			return (1);
		if (i == 0 && j == 0) {
//			if (ferror(f1) || ferror(f2))
			if (f1->merror() || f2->merror())
				return (1);
			return (0);
		}
		if (memcmp(buf1, buf2, i) != 0)
			return (1);
	}
}

#if 0
static FILE *
opentemp(const char *file)
{
	char buf[BUFSIZ], *tempdir, tempfile[MAXPATHLEN];
	ssize_t nread;
	int ifd, ofd;

	if (strcmp(file, "-") == 0)
		ifd = STDIN_FILENO;
	else if ((ifd = open(file, O_RDONLY, 0644)) < 0)
		return (NULL);

	if ((tempdir = getenv("TMPDIR")) == NULL)
//		tempdir = _PATH_TMP;
	    tempdir = "/tmp";
	
	if (strlcpy(tempfile, tempdir, sizeof(tempfile)) >= sizeof(tempfile) ||
	    strlcat(tempfile, "/diff.XXXXXXXX", sizeof(tempfile)) >=
	    sizeof(tempfile)) {
		close(ifd);
		errno = ENAMETOOLONG;
		return (NULL);
	}

	if ((ofd = mkstemp(tempfile)) < 0)
		return (NULL);
	unlink(tempfile);
	while ((nread = read(ifd, buf, BUFSIZ)) > 0) {
		if (write(ofd, buf, nread) != nread) {
			close(ifd);
			close(ofd);
			return (NULL);
		}
	}
	close(ifd);
	lseek(ofd, (off_t)0, SEEK_SET);
	return (fdopen(ofd, "r"));
}
#endif

#if 0
char *
splice(char *dir, char *file)
{
	char *tail, *buf;

	if ((tail = strrchr(file, '/')) == NULL)
		tail = file;
	else
		tail++;
	easprintf(&buf, "%s/%s", dir, tail);
	return (buf);
}
#endif

static void
//prepare(int i, FILE *fd, off_t filesize)
prepare(int i, BBlockFILE *fd)
{
	struct line *p;
	int j, h;
	size_t sz;

//	rewind(fd);
	fd->rewind ();

//	sz = (filesize <= SIZE_MAX ? filesize : SIZE_MAX) / 25;
	sz = (fd->text->length () <= SSIZE_MAX ? fd->text->length () : SSIZE_MAX) / 25;
	if (sz < 100)
		sz = 100;

	p = (line*)emalloc((sz + 3) * sizeof(struct line));
	for (j = 0; (h = readhash(fd));) {
		if (j == sz) {
			sz = sz * 3 / 2;
			p = (line*)erealloc(p, (sz + 3) * sizeof(struct line));
		}
		p[++j].value = h;
	}
	len[i] = j;
	file[i] = p;
}

static void
prune(void)
{
	int i, j;

	for (pref = 0; pref < len[0] && pref < len[1] &&
	    file[0][pref + 1].value == file[1][pref + 1].value;
	    pref++)
		;
	for (suff = 0; suff < len[0] - pref && suff < len[1] - pref &&
	    file[0][len[0] - suff].value == file[1][len[1] - suff].value;
	    suff++)
		;
	for (j = 0; j < 2; j++) {
		sfile[j] = file[j] + pref;
		slen[j] = len[j] - pref - suff;
		for (i = 0; i <= slen[j]; i++)
			sfile[j][i].serial = i;
	}
}

static void
equiv(struct line *a, int n, struct line *b, int m, int *c)
{
	int i, j;

	i = j = 1;
	while (i <= n && j <= m) {
		if (a[i].value < b[j].value)
			a[i++].value = 0;
		else if (a[i].value == b[j].value)
			a[i++].value = j;
		else
			j++;
	}
	while (i <= n)
		a[i++].value = 0;
	b[m + 1].value = 0;
	j = 0;
	while (++j <= m) {
		c[j] = -b[j].serial;
		while (b[j + 1].value == b[j].value) {
			j++;
			c[j] = b[j].serial;
		}
	}
	c[j] = -1;
}

/* Code taken from ping.c */
static int
isqrt(int n)
{
	int y, x = 1;

	if (n == 0)
		return(0);

	do { /* newton was a stinker */
		y = x;
		x = n / x;
		x += y;
		x /= 2;
	} while ((x - y) > 1 || (x - y) < -1);

	return (x);
}

static int
stone(int *a, int n, int *b, int *c)
{
	int i, k, y, j, l;
	int oldc, tc, oldl;
	u_int numtries;

	const u_int bound = dflag ? UINT_MAX : max(256, isqrt(n));

	k = 0;
	c[0] = newcand(0, 0, 0);
	for (i = 1; i <= n; i++) {
		j = a[i];
		if (j == 0)
			continue;
		y = -b[j];
		oldl = 0;
		oldc = c[0];
		numtries = 0;
		do {
			if (y <= clist[oldc].y)
				continue;
			l = search(c, k, y);
			if (l != oldl + 1)
				oldc = c[l - 1];
			if (l <= k) {
				if (clist[c[l]].y <= y)
					continue;
				tc = c[l];
				c[l] = newcand(i, y, oldc);
				oldc = tc;
				oldl = l;
				numtries++;
			} else {
				c[l] = newcand(i, y, oldc);
				k++;
				break;
			}
		} while ((y = b[++j]) > 0 && numtries < bound);
	}
	return (k);
}

static int
newcand(int x, int y, int pred)
{
	struct cand *q;

	if (clen == clistlen) {
		clistlen = clistlen * 11 / 10;
		clist = (struct cand*)erealloc(clist, clistlen * sizeof(struct cand));
	}
	q = clist + clen;
	q->x = x;
	q->y = y;
	q->pred = pred;
	return (clen++);
}

static int
search(int *c, int k, int y)
{
	int i, j, l, t;

	if (clist[c[k]].y < y)	/* quick look for typical case */
		return (k + 1);
	i = 0;
	j = k + 1;
	while (1) {
		l = i + j;
		if ((l >>= 1) <= i)
			break;
		t = clist[c[l]].y;
		if (t > y)
			j = l;
		else if (t < y)
			i = l;
		else
			return (l);
	}
	return (l + 1);
}

static void
unravel(int p)
{
	struct cand *q;
	int i;

	for (i = 0; i <= len[0]; i++)
		J[i] = i <= pref ? i :
		    i > len[0] - suff ? i + len[1] - len[0] : 0;
	for (q = clist + p; q->y != 0; q = clist + q->pred)
		J[q->x + pref] = q->y + pref;
}

/*
 * Check does double duty:
 *  1.	ferret out any fortuitous correspondences due
 *	to confounding by hashing (which result in "jackpot")
 *  2.  collect random access indexes to the two files
 */
static void
//check(char *file1, FILE *f1, char *file2, FILE *f2)
check(BBlockFILE *f1, BBlockFILE *f2)
{
	int i, j, jackpot, c, d;
	long ctold, ctnew;

//	rewind(f1);
//	rewind(f2);
	f1->rewind ();
	f2->rewind ();
	j = 1;
	ixold[0] = ixnew[0] = 0;
	jackpot = 0;
	ctold = ctnew = 0;
	for (i = 1; i <= len[0]; i++) {
		if (J[i] == 0) {
			ixold[i] = ctold += skipline(f1);
			continue;
		}
		while (j < J[i]) {
			ixnew[j] = ctnew += skipline(f2);
			j++;
		}
		if (bflag || wflag || iflag) {
			for (;;) {
//				c = getc(f1);
//				d = getc(f2);
				c = f1->mgetc ();
				d = f2->mgetc ();
				/*
				 * GNU diff ignores a missing newline
				 * in one file if bflag || wflag.
				 */
				if ((bflag || wflag) &&
				    ((c == EOF && d == '\n') ||
				    (c == '\n' && d == EOF))) {
					break;
				}
				ctold++;
				ctnew++;
				if (bflag && isspace(c) && isspace(d)) {
					do {
						if (c == '\n')
							break;
						ctold++;
//					} while (isspace(c = getc(f1)));
					} while (isspace(c = f1-> mgetc()));
					do {
						if (d == '\n')
							break;
						ctnew++;
//					} while (isspace(d = getc(f2)));
					} while (isspace(d = f2->mgetc()));
				} else if (wflag) {
					while (isspace(c) && c != '\n') {
//						c = getc(f1);
						c = f1->mgetc();
						ctold++;
					}
					while (isspace(d) && d != '\n') {
//						d = getc(f2);
						d = f2->mgetc();
						ctnew++;
					}
				}
				if (chrtran[c] != chrtran[d]) {
					jackpot++;
					J[i] = 0;
					if (c != '\n' && c != EOF)
						ctold += skipline(f1);
					if (d != '\n' && c != EOF)
						ctnew += skipline(f2);
					break;
				}
				if (c == '\n' || c == EOF)
					break;
			}
		} else {
			for (;;) {
				ctold++;
				ctnew++;
//				if ((c = getc(f1)) != (d = getc(f2))) {
				if ((c = f1->mgetc()) != (d = f2->mgetc())) {
					if ((c == EOF && d == '\n')
					    || (c == '\n' && d == EOF))
						break;
					/* jackpot++; */
					J[i] = 0;
					if (c != '\n' && c != EOF)
						ctold += skipline(f1);
					if (d != '\n' && c != EOF)
						ctnew += skipline(f2);
					break;
				}
				if (c == '\n' || c == EOF)
					break;
			}
		}
		ixold[i] = ctold;
		ixnew[j] = ctnew;
		j++;
	}
	for (; j <= len[1]; j++)
		ixnew[j] = ctnew += skipline(f2);
	/*
	 * if (jackpot)
	 *	fprintf(stderr, "jackpot\n");
	 */
}

/* shellsort CACM #201 */
static void
sort(struct line *a, int n)
{
	struct line *ai, *aim, w;
	int j, m = 0, k;

	if (n == 0)
		return;
	for (j = 1; j <= n; j *= 2)
		m = 2 * j - 1;
	for (m /= 2; m != 0; m /= 2) {
		k = n - m;
		for (j = 1; j <= k; j++) {
			for (ai = &a[j]; ai > a; ai -= m) {
				aim = &ai[m];
				if (aim < ai)
					break;	/* wraparound */
				if (aim->value > ai[0].value ||
				    (aim->value == ai[0].value &&
					aim->serial > ai[0].serial))
					break;
				w.value = ai[0].value;
				ai[0].value = aim->value;
				aim->value = w.value;
				w.serial = ai[0].serial;
				ai[0].serial = aim->serial;
				aim->serial = w.serial;
			}
		}
	}
}

static void
unsort(struct line *f, int l, int *b)
{
	int *a, i;

	a = (int*)emalloc((l + 1) * sizeof(int));
	for (i = 1; i <= l; i++)
		a[f[i].serial] = f[i].value;
	for (i = 1; i <= l; i++)
		b[i] = a[i];
//	free(a);
	efree(a);
}

static int
//skipline(FILE *f)
skipline(BBlockFILE *f)
{
	int i, c;

//	for (i = 1; (c = getc(f)) != '\n' && c != EOF; i++)
	for (i = 1; (c = f->mgetc()) != '\n' && c != EOF; i++)
		continue;
	return (i);
}

static void
//output(char *file1, FILE *f1, char *file2, FILE *f2, int flags)
//output(BBlockFILE* f1, BBlockFILE* f2, int flags, OutputVar* out)
output(BBlockFILE* f1, BBlockFILE* f2, int flags, OutputList* out)
{
	int m, i0, i1, j0, j1;

//	rewind(f1);
//	rewind(f2);
	f1->rewind();
	f2->rewind();
	m = len[0];
	J[0] = 0;
	J[m + 1] = len[1] + 1;
	if (format != D_EDIT) {
		for (i0 = 1; i0 <= m; i0 = i1 + 1) {
			while (i0 <= m && J[i0] == J[i0 - 1] + 1)
				i0++;
			j0 = J[i0 - 1] + 1;
			i1 = i0 - 1;
			while (i1 < m && J[i1 + 1] == 0)
				i1++;
			j1 = J[i1 + 1] - 1;
			J[i1] = j1;
//			change(file1, f1, file2, f2, i0, i1, j0, j1, &flags);
			change(f1, f2, i0, i1, j0, j1, &flags, out);
		}
	} else {
		for (i0 = m; i0 >= 1; i0 = i1 - 1) {
			while (i0 >= 1 && J[i0] == J[i0 + 1] - 1 && J[i0] != 0)
				i0--;
			j0 = J[i0 + 1] - 1;
			i1 = i0 + 1;
			while (i1 > 1 && J[i1 - 1] == 0)
				i1--;
			j1 = J[i1 - 1] + 1;
			J[i1] = j1;
//			change(file1, f1, file2, f2, i1, i0, j1, j0, &flags);
			change(f1, f2, i1, i0, j1, j0, &flags, out);
		}
	}
	if (m == 0)
//		change(file1, f1, file2, f2, 1, 0, 1, len[1], &flags);
		change(f1, f2, 1, 0, 1, len[1], &flags, out);
#if 0
	if (format == D_IFDEF) {
		for (;;) {
#define	c i0
			if ((c = getc(f1)) == EOF)
				return;
			putc(c, stdout);
		}
#undef c
	}
#endif
	if (anychange != 0) {
		if (format == D_CONTEXT)
			dump_context_vec(f1, f2, out);
#if 0
		else if (format == D_UNIFIED)
			dump_unified_vec(f1, f2);
#endif
	}
}

#if 0
static __inline void
range(int a, int b, char *separator)
{
	fprintf(stdout, "%d", a > b ? b : a);
//	if (a < b)
		fprintf(stdout, "%s%d", separator, b);
}
#endif
#if 0
static __inline void
uni_range(int a, int b)
{
	if (a < b)
		fprintf(stdout, "%d,%d", a, b - a + 1);
	else if (a == b)
		fprintf(stdout, "%d", b);
	else
		fprintf(stdout, "%d,0", b);
}
#endif
#if 0
static char *
preadline(int fd, size_t len, off_t off)
{
	char *line;
	ssize_t nr;

	line = (char*)emalloc(len + 1);
	if ((nr = pread(fd, line, len, off)) < 0)
		err(1, "preadline");
	if (nr > 0 && line[nr-1] == '\n')
		nr--;
	line[nr] = '\0';
	return (line);
}
#endif
#if 0
static int
ignoreline(char *line)
{
	int ret;

	ret = regexec(&ignore_re, line, 0, NULL, 0);
//	free(line);
	efree(line);
	return (ret == 0);	/* if it matched, it should be ignored. */
}
#endif

/*
 * Indicate that there is a difference between lines a and b of the from file
 * to get to lines c to d of the to file.  If a is greater then b then there
 * are no lines in the from file involved and this means that there were
 * lines appended (beginning at b).  If c is greater than d then there are
 * lines missing from the to file.
 */
static void
//change(char *file1, FILE *f1, char *file2, FILE *f2, int a, int b, int c, int d, int *pflags)
//change(BBlockFILE* f1, BBlockFILE* f2, int a, int b, int c, int d, int *pflags, OutputVar* out)
change(BBlockFILE* f1, BBlockFILE* f2, int a, int b, int c, int d, int *pflags, OutputList* out)
{
	static size_t max_context = 64;
	int i;

restart:
	if (format != D_IFDEF && a > b && c > d)
		return;
#if 0
	if (ignore_pats != NULL) {
		char *line;
		/*
		 * All lines in the change, insert, or delete must
		 * match an ignore pattern for the change to be
		 * ignored.
		 */
		if (a <= b) {		/* Changes and deletes. */
			for (i = a; i <= b; i++) {
				line = preadline(fileno(f1),
				    ixold[i] - ixold[i - 1], ixold[i - 1]);
				if (!ignoreline(line))
					goto proceed;
			}
		}
		if (a > b || c <= d) {	/* Changes and inserts. */
			for (i = c; i <= d; i++) {
				line = preadline(fileno(f2),
				    ixnew[i] - ixnew[i - 1], ixnew[i - 1]);
				if (!ignoreline(line))
					goto proceed;
			}
		}
		return;
	}
#endif
proceed:
	if (*pflags & D_HEADER) {
//		printf("%s %s %s\n", diffargs, file1, file2);
//		fprintf (stdout, "\n");
		*pflags &= ~D_HEADER;
	}
	if (format == D_CONTEXT || format == D_UNIFIED) {
		/*
		 * Allocate change records as needed.
		 */
		if (context_vec_ptr == context_vec_end - 1) {
			ptrdiff_t offset = context_vec_ptr - context_vec_start;
			max_context <<= 1;
			context_vec_start = (struct context_vec*)erealloc(context_vec_start,
			    max_context * sizeof(struct context_vec));
			context_vec_end = context_vec_start + max_context;
			context_vec_ptr = context_vec_start + offset;
		}
		if (anychange == 0) {
			/*
			 * Print the context/unidiff header first time through.
			 */
//			print_header(file1, file2);
//			print_header();
			anychange = 1;
		} else if (a > context_vec_ptr->b + (2 * context) + 1 &&
		    c > context_vec_ptr->d + (2 * context) + 1) {
			/*
			 * If this change is more than 'context' lines from the
			 * previous change, dump the record and reset it.
			 */
			if (format == D_CONTEXT)
				dump_context_vec(f1, f2, out);
#if 0
			else
				dump_unified_vec(f1, f2);
#endif
		}
		context_vec_ptr++;
		context_vec_ptr->a = a;
		context_vec_ptr->b = b;
		context_vec_ptr->c = c;
		context_vec_ptr->d = d;
		return;
	}
#if 0
	if (anychange == 0)
		anychange = 1;
#if 0
	switch (format) {
	case D_BRIEF:
		return;
	case D_NORMAL:
	case D_EDIT:
		range(a, b, ",");
		putc(a > b ? 'a' : c > d ? 'd' : 'c', stdout);
		if (format == D_NORMAL)
			range(c, d, ",");
		putc('\n', stdout);
		break;
	case D_REVERSE:
		putc(a > b ? 'a' : c > d ? 'd' : 'c', stdout);
		range(a, b, " ");
		putc('\n', stdout);
		break;
	case D_NREVERSE:
		if (a > b)
			fprintf(stdout, "a%d %d\n", b, d - c + 1);
		else {
			fprintf(stdout, "d%d %d\n", a, b - a + 1);
			if (!(c > d))
				/* add changed lines */
				fprintf(stdout, "a%d %d\n", b, d - c + 1);
		}
		break;
	}
#endif
#if 0
	if (format == D_NORMAL || format == D_IFDEF) {
		fetch(ixold, a, b, f1, '<', 1);
		if (a <= b && c <= d && format == D_NORMAL)
			fputs("---", stdout);
	}
#endif
//	i = fetch(ixnew, c, d, f2, format == D_NORMAL ? '>' : '\0', 0);
	i = fetch(ixnew, c, d, f2, format == D_NORMAL ? '>' : '\0');
#if 0
	if (i != 0 && format == D_EDIT) {
		/*
		 * A non-zero return value for D_EDIT indicates that the
		 * last line printed was a bare dot (".") that has been
		 * escaped as ".." to prevent ed(1) from misinterpreting
		 * it.  We have to add a substitute command to change this
		 * back and restart where we left off.
		 */
		fputs(".", stdout);
		fprintf(stdout, "%ds/^\\.\\././\n", a);
		a += i;
		c += i;
		goto restart;
	}
#endif
#if 0
	if ((format == D_EDIT || format == D_REVERSE) && c <= d)
		fputs(".", stdout);
#endif
#if 0
	if (inifdef) {
//		fprintf(stdout, "#endif /* %s */\n", ifdefname);
		inifdef = 0;
	}
#endif
#endif
}

#if 0
static int
//fetch(long *f, int a, int b, FILE *lb, int ch, int oldfile)
fetch(long *f, int a, int b, BBlockFILE*lb, int ch, int oldfile)
{
//	int i, j, c, lastc, col, nc;
	int i, j, c, nc;
	BBlock  p;

#if 0
	/*
	 * When doing #ifdef's, copy down to current line
	 * if this is the first file, so that stuff makes it to output.
	 */
	if (format == D_IFDEF && oldfile) {
		long curpos = ftell(lb);
		/* print through if append (a>b), else to (nb: 0 vs 1 orig) */
		nc = f[a > b ? b : a - 1] - curpos;
		for (i = 0; i < nc; i++)
			putc(getc(lb), stdout);
	}
#endif
	if (a > b)
		return (0);
#if 0
	if (format == D_IFDEF) {
		if (inifdef) {
			fprintf(stdout, "#else /* %s%s */\n",
			    oldfile == 1 ? "!" : "", ifdefname);
		} else {
			if (oldfile)
				fprintf(stdout,"#ifndef %s\n", ifdefname);
			else
				fprintf(stdout,"#ifdef %s\n", ifdefname);
		}
		inifdef = 1 + oldfile;
	}
#endif
	for (i = a; i <= b; i++) {
#if 0
//		fseek(lb, f[i - 1], SEEK_SET);
		lb->fseek(f[i - 1], SEEK_SET);
#endif
		nc = f[i] - f[i - 1];
		if (format != D_IFDEF && ch != '\0') {
			/* 1カラム目 */
			putc(ch, stdout);
#if 0
			if (Tflag && (format == D_NORMAL || format == D_CONTEXT
			    || format == D_UNIFIED))
				putc('\t', stdout);
			else if (format != D_UNIFIED)
#endif
			    /* ２カラム目の空白*/
//				putc(' ', stdout);
		}
#if 0
		col = 0;
		for (j = 0, lastc = '\0'; j < nc; j++, lastc = c) {
//			if ((c = getc(lb)) == EOF) {
			if ((c = lb->mgetc()) == EOF) {
#if 0
				if (format == D_EDIT
				    || format == D_REVERSE
				    || format == D_NREVERSE)
					warnx("No newline at end of file");
				else
#endif
					fputs("\n\\ No newline at end of file", stdout);
				return (0);
			}
#if 0
			if (c == '\t' && tflag) {
				do {
					putc(' ', stdout);
				} while (++col & 7);
			} else {
#endif
#if 0
				if (format == D_EDIT && j == 1 && c == '\n'
				    && lastc == '.') {
					/*
					 * Don't print a bare "." line
					 * since that will confuse ed(1).
					 * Print ".." instead and return,
					 * giving the caller an offset
					 * from which to restart.
					 */
					fputs(".", stdout);
					return (i - a + 1);
				}
#endif
				putc(c, stdout);
				col++;
#if 0
			}
#endif
		}
#endif
		/* 行末のLFが無いかもしれない */
		p.assign (&(*lb->text)[f[i - 1]], min(lb->text->length - f[i - 1], nc));
		pfprintf (stdout, "%B", &p);
		if (p.length == 0 || p.last () != '\n')
			putc ('\n', stdout);
	}
	return (0);
}
#endif
#if 0
static int  fetch(long *f, int a, int b, BBlockFILE*lb, int ch) {
	int i, j, c, nc;
	BBlock  p;

	for (i = a; i <= b; i++) {
		nc = f[i] - f[i - 1];
		if (ch != '\0') {
			/* 1カラム目 */
			putc(ch, stdout);
		}
		/* 行末のLFが無いかもしれない */
		p.assign (&(*lb->text)[f[i - 1]], min(lb->text->length - f[i - 1], nc));
		pfprintf (stdout, "%B", &p);
		if (p.length == 0 || p.last () != '\n')
			putc ('\n', stdout);
	}
	return (0);
}
#endif

//static void  fetchline (long* f, int i, BBlockFILE* lb, BBlock* ans) {
static void  fetchline (long* f, int i, BBlockFILE* lb, ustring* ans) {
    ans->assign (&(*lb->text)[f[i - 1]], min (lb->text->length () - f[i - 1], f[i] - f[i - 1]));
//    if (ans->length > 0 && ans->last () == '\n')
//	ans->pop ();
    if (ans->length () > 0 && (*ans)[ans->length () - 1] == '\n')
	ans->resize (ans->length () - 1);
}

/*
 * Hash function taken from Robert Sedgewick, Algorithms in C, 3d ed., p 578.
 */
static int
//readhash(FILE *f)
readhash(BBlockFILE *f)
{
	int i, t, space;
	int sum;

	sum = 1;
	space = 0;
	if (!bflag && !wflag) {
		if (iflag)
//			for (i = 0; (t = getc(f)) != '\n'; i++) {
			for (i = 0; (t = f->mgetc()) != '\n'; i++) {
				if (t == EOF) {
					if (i == 0)
						return (0);
					break;
				}
				sum = sum * 127 + chrtran[t];
			}
		else
//			for (i = 0; (t = getc(f)) != '\n'; i++) {
			for (i = 0; (t = f->mgetc()) != '\n'; i++) {
				if (t == EOF) {
					if (i == 0)
						return (0);
					break;
				}
				sum = sum * 127 + t;
			}
	} else {
		for (i = 0;;) {
//			switch (t = getc(f)) {
			switch (t = f->mgetc()) {
			case '\t':
			case '\r':
			case '\v':
			case '\f':
			case ' ':
				space++;
				continue;
			default:
				if (space && !wflag) {
					i++;
					space = 0;
				}
				sum = sum * 127 + chrtran[t];
				i++;
				continue;
			case EOF:
				if (i == 0)
					return (0);
				/* FALLTHROUGH */
			case '\n':
				break;
			}
			break;
		}
	}
	/*
	 * There is a remote possibility that we end up with a zero sum.
	 * Zero is used as an EOF marker, so return 1 instead.
	 */
	return (sum == 0 ? 1 : sum);
}

#if 0
static int
asciifile(FILE *f)
{
	unsigned char buf[BUFSIZ];
	int i, cnt;

	if (aflag || f == NULL)
		return (1);

	rewind(f);
	cnt = fread(buf, 1, sizeof(buf), f);
	for (i = 0; i < cnt; i++)
		if (!isprint(buf[i]) && !isspace(buf[i]))
			return (0);
	return (1);
}
#endif

static __inline int min(int a, int b)
{
	return (a < b ? a : b);
}

static __inline int max(int a, int b)
{
	return (a > b ? a : b);
}

#define begins_with(s, pre) (strncmp(s, pre, sizeof(pre)-1) == 0)

#if 0
static char *
//match_function(const long *f, int pos, FILE *file)
match_function(const long *f, int pos, BBlockFILE *file)
{
	unsigned char buf[FUNCTION_CONTEXT_SIZE];
	size_t nc;
	int last = lastline;
	char *p;
	char *state = NULL;

	lastline = pos;
	while (pos > last) {
//		fseek(file, f[pos - 1], SEEK_SET);
		file->fseek(f[pos - 1], SEEK_SET);
		nc = f[pos] - f[pos - 1];
		if (nc >= sizeof(buf))
			nc = sizeof(buf) - 1;
//		nc = fread(buf, 1, nc, file);
		nc = file->fread(buf, 1, nc);
		if (nc > 0) {
			buf[nc] = '\0';
			p = strchr(char_type (buf), '\n');
			if (p != NULL)
				*p = '\0';
			if (isalpha(buf[0]) || buf[0] == '_' || buf[0] == '$') {
				if (begins_with(char_type (buf), "private:")) {
					if (!state)
						state = " (private)";
				} else if (begins_with(char_type (buf), "protected:")) {
					if (!state)
						state = " (protected)";
				} else if (begins_with(char_type (buf), "public:")) {
					if (!state)
						state = " (public)";
				} else {
					strlcpy(lastbuf, char_type (buf), sizeof lastbuf);
					if (state)
						strlcat(lastbuf, state, 
						    sizeof lastbuf);
					lastmatchline = pos;
					return lastbuf;
				}
			}
		}
		pos--;
	}
	return lastmatchline > 0 ? lastbuf : NULL;
}
#endif

#if 0
/* dump accumulated "context" diff changes */
static void
//dump_context_vec(FILE *f1, FILE *f2)
dump_context_vec(BBlockFILE*f1, BBlockFILE*f2)
{
	struct context_vec *cvp = context_vec_start;
	int lowa, upb, lowc, upd, do_output;
	int a, b, c, d;
	char ch, *f;

	if (context_vec_start > context_vec_ptr)
		return;

	b = d = 0;		/* gcc */
	lowa = max(1, cvp->a - context);
	upb = min(len[0], context_vec_ptr->b + context);
	lowc = max(1, cvp->c - context);
	upd = min(len[1], context_vec_ptr->d + context);

//	fprintf(stdout,"***************");
#if 0
	if (pflag) {
		f = match_function(ixold, lowa-1, f1);
		if (f != NULL) {
			putc(' ', stdout);
			fputs(f, stdout);
		}
	}
#endif
    fprintf (stdout, "context:%d", context);
	fprintf(stdout,"\n*** ");
	range(lowa, upb, ",");
	fprintf(stdout," ****(old)\n");

	/*
	 * Output changes to the "old" file.  The first loop suppresses
	 * output if there were no changes to the "old" file (we'll see
	 * the "old" lines as context in the "new" list).
	 */
	do_output = 0;
	for (; cvp <= context_vec_ptr; cvp++)
		if (cvp->a <= cvp->b) {
			cvp = context_vec_start;
			do_output++;
			break;
		}
	if (do_output) {
		while (cvp <= context_vec_ptr) {
			a = cvp->a;
			b = cvp->b;
			c = cvp->c;
			d = cvp->d;

			if (a <= b && c <= d)
				ch = 'c';
			else
				ch = (a <= b) ? 'd' : 'a';

			if (ch == 'a')
//				fetch(ixold, lowa, b, f1, ' ', 0);
				fetch(ixold, lowa, b, f1, ' ');
			else {
//				fetch(ixold, lowa, a - 1, f1, ' ', 0);
//				fetch(ixold, a, b, f1, ch == 'c' ? '!' : '-', 0);
				fetch(ixold, lowa, a - 1, f1, ' ');
				fetch(ixold, a, b, f1, ch == 'c' ? '!' : '-');
			}
			lowa = b + 1;
			cvp++;
		}
//		fetch(ixold, b + 1, upb, f1, ' ', 0);
		fetch(ixold, b + 1, upb, f1, ' ');
	}
	/* output changes to the "new" file */
	fprintf(stdout,"--- ");
	range(lowc, upd, ",");
	fprintf(stdout," ----(new)\n");

	do_output = 0;
	for (cvp = context_vec_start; cvp <= context_vec_ptr; cvp++)
		if (cvp->c <= cvp->d) {
			cvp = context_vec_start;
			do_output++;
			break;
		}
	if (do_output) {
		while (cvp <= context_vec_ptr) {
			a = cvp->a;
			b = cvp->b;
			c = cvp->c;
			d = cvp->d;

			if (a <= b && c <= d)
				ch = 'c';
			else
				ch = (a <= b) ? 'd' : 'a';

			if (ch == 'd')
//				fetch(ixnew, lowc, d, f2, ' ', 0);
				fetch(ixnew, lowc, d, f2, ' ');
			else {
//				fetch(ixnew, lowc, c - 1, f2, ' ', 0);
//				fetch(ixnew, c, d, f2, ch == 'c' ? '!' : '+', 0);
				fetch(ixnew, lowc, c - 1, f2, ' ');
				fetch(ixnew, c, d, f2, ch == 'c' ? '!' : '+');
			}
			lowc = d + 1;
			cvp++;
		}
//		fetch(ixnew, d + 1, upd, f2, ' ', 0);
		fetch(ixnew, d + 1, upd, f2, ' ');
	}
	context_vec_ptr = context_vec_start - 1;
}
#else
#if 0
static void  testout (char c1, BBlock* p1, char c2, BBlock* p2) {
    int  i, col;
    int  ch;

    pfprintf (stdout, "%c", c1);
    for (i = 0, col = 0; i < p1->length && col < 35; i ++, col ++) {
	if ((ch = (*p1)[i]) == '\t') {
	    putc (' ', stdout);
	    for (col ++; col % 8 == 0; col ++)
		putc (' ', stdout);
	    col --;
	} else {
	    putc (ch, stdout);
	}
    }
    for (; col < 35; col ++)
	putc (' ', stdout);
    pfprintf (stdout, " |%c", c2);
    for (i = 0, col = 0; i < p2->length && col < 35; i ++, col ++) {
	if ((ch = (*p2)[i]) == '\t') {
	    putc (' ', stdout);
	    for (col ++; col % 8 == 0; col ++)
		putc (' ', stdout);
	    col --;
	} else {
	    putc (ch, stdout);
	}
    }
    putc ('\n', stdout);
}
#endif

static void
//dump_context_vec(FILE *f1, FILE *f2)
//dump_context_vec(BBlockFILE*f1, BBlockFILE*f2, OutputVar* out)
dump_context_vec(BBlockFILE*f1, BBlockFILE*f2, OutputList* out)
{
    struct context_vec *cvp = context_vec_start;
    int lowa, upb, lowc, upd, do_output;
    int a, b, c, d;
    char ch;
    int  i, n;
//    BBlock  p1, p2;
    ustring  p1, p2;
    int  c1, c2;

    if (context_vec_start > context_vec_ptr)
	return;

    b = d = 0;		/* gcc */
    lowa = max(1, cvp->a - context);
    upb = min(len[0], context_vec_ptr->b + context);
    lowc = max(1, cvp->c - context);
    upd = min(len[1], context_vec_ptr->d + context);
    
#if 0
    fprintf(stdout,"***************");
    fprintf(stdout,"\n*** ");
    range(lowa, upb, ",");
    fprintf (stdout, "  ");
    range(lowc, upd, ",");
    fprintf(stdout," ****\n");
#endif
    out->linenum (lowa, lowc);

    do_output = 0;
    for (; cvp <= context_vec_ptr; cvp++)
	if (cvp->a <= cvp->b || cvp->c <= cvp->d) {
	    cvp = context_vec_start;
	    do_output++;
	    break;
	}
    if (do_output) {
	while (cvp <= context_vec_ptr) {
	    a = cvp->a;
	    b = cvp->b;
	    c = cvp->c;
	    d = cvp->d;

	    if (a <= b && c <= d)
		ch = 'c';
	    else
		ch = (a <= b) ? 'd' : 'a';
	    
	    switch (ch) {
	    case 'a':
#if 0
		printf ("<<<\n");
		fetch(ixold, lowa, b, f1, ' ');
		printf (">>>\n");
		fetch(ixnew, lowc, c - 1, f2, ' ');
		fetch(ixnew, c, d, f2, ch == 'c' ? '!' : '+');
#endif
		n = max (b - lowa, c - 1 - lowc);
		for (i = 0; i <= n; i ++) {
		    if (lowa + i <= b) {
			fetchline (ixold, lowa + i, f1, &p1);
			c1 = 1;
		    } else {
//			p1.init ();
			p1.resize (0);
			c1 = 0;
		    }
		    if (lowc + i <= c - 1) {
			fetchline (ixnew, lowc + i, f2, &p2);
			c2 = 1;
		    } else {
//			p2.init ();
			p2.resize (0);
			c2 = 0;
		    }
//		    pfprintf (stdout, "< %B\n", &p1);
//		    pfprintf (stdout, "> %B\n", &p2);
//		    testout (' ', &p1, ' ', &p2);
		    out->out (c1?' ':'*', &p1, c2?' ':'*', &p2);
		}
		n = d - c;
		for (i = 0; i <= n; i ++) {
//		    p1.init ();
		    p1.resize (0);
		    fetchline (ixnew, c + i, f2, &p2);
		    if (ch == 'c') {
//			pfprintf (stdout, "<*\n");
//			pfprintf (stdout, ">!%B\n", &p2);
//			testout ('*', &p1, '!', &p2);
//			out->out ('*', &p1, '!', &p2);
			out->out ('*', &p1, '+', &p2);
		    } else {
//			pfprintf (stdout, "<*\n");
//			pfprintf (stdout, ">+%B\n", &p2);
//			testout ('*', &p1, '+', &p2);
			out->out ('*', &p1, '+', &p2);
		    }
		}
		break;
	    case 'c':
#if 0
		printf ("<<<\n");
		fetch(ixold, lowa, a - 1, f1, ' ');
		fetch(ixold, a, b, f1, ch == 'c' ? '!' : '-');
		printf (">>>\n");
		fetch(ixnew, lowc, c - 1, f2, ' ');
		fetch(ixnew, c, d, f2, ch == 'c' ? '!' : '+');
#endif
		n = max (a - 1 - lowa, c - 1 - lowc);
		for (i = 0; i <= n; i ++) {
		    if (lowa + i <= a - 1) {
			fetchline (ixold, lowa + i, f1, &p1);
			c1 = 1;
		    } else {
//			p1.init ();
			p1.resize (0);
			c1 = 0;
		    }
		    if (lowc + i <= c - 1) {
			fetchline (ixnew, lowc + i, f2, &p2);
			c2 = 1;
		    } else {
//			p2.init ();
			p2.resize (0);
			c2 = 0;
		    }
//		    pfprintf (stdout, "> %B\n", &p1);
//		    pfprintf (stdout, "< %B\n", &p2);
//		    testout (' ', &p1, ' ', &p2);
		    out->out (c1?' ':'*', &p1, c2?' ':'*', &p2);
		}
		n = max (b - a, d - c);
		for (i = 0; i <= n; i ++) {
		    if (a + i <= b) {
			fetchline (ixold, a + i, f1, &p1);
			c1 = 1;
		    } else {
//			p1.init ();
			p1.resize (0);
			c1 = 0;
		    }
		    if (c + i <= d) {
			fetchline (ixnew, c + i, f2, &p2);
			c2 = 1;
		    } else {
//			p2.init ();
			p2.resize (0);
			c2 = 0;
		    }
		    if (ch == 'c') {
//			pfprintf (stdout, ">!%B\n", &p1);
//			pfprintf (stdout, "<!%B\n", &p2);
//			testout ('!', &p1, '!', &p2);
//			out->out (c1?'!':'*', &p1, c2?'!':'*', &p2);
			out->out (c1?'-':'*', &p1, c2?'+':'*', &p2);
		    } else {
//			pfprintf (stdout, ">-%B\n", &p1);
//			pfprintf (stdout, "<+%B\n", &p2);
//			testout ('-', &p1, '+', &p2);
			out->out (c1?'-':'*', &p1, c2?'+':'*', &p2);
		    }
		}
		break;
	    case 'd':
#if 0
		printf ("<<<\n");
		fetch(ixold, lowa, a - 1, f1, ' ');
		fetch(ixold, a, b, f1, ch == 'c' ? '!' : '-');
		printf (">>>\n");
		fetch(ixnew, lowc, d, f2, ' ');
#endif
		n = max (a - 1 - lowa, d - lowc);
		for (i = 0; i <= n; i ++) {
		    if (lowa + i <= a - 1) {
			fetchline (ixold, lowa + i, f1, &p1);
			c1 = 1;
		    } else {
//			p1.init ();
			p1.resize (0);
			c1 =0;
		    }
		    if (lowc + i <= d) {
			fetchline (ixnew, lowc + i, f2, &p2);
			c2 = 1;
		    } else {
//			p2.init ();
			p2.resize (0);
			c2 = 0;
		    }
//		    pfprintf (stdout, "> %B\n", &p1);
//		    pfprintf (stdout, "< %B\n", &p2);
//		    testout (' ', &p1, ' ', &p2);
		    out->out (c1?' ':'*', &p1, c2?' ':'*', &p2);
		}
		n = b - a;
		for (i = 0; i <= n; i ++) {
		    fetchline (ixold, a + i, f1, &p1);
//		    p2.init ();
		    p2.resize (0);
		    if (ch == 'c') {
//			pfprintf (stdout, ">!%B\n", &p1);
//			pfprintf (stdout, "<*\n");
//			testout ('!', &p1, '*', &p2);
//			out->out ('!', &p1, '*', &p2);
			out->out ('-', &p1, '*', &p2);
		    } else {
//			pfprintf (stdout, ">-%B\n", &p1);
//			pfprintf (stdout, "<*\n");
//			testout ('-', &p1, '*', &p2);
			out->out ('-', &p1, '*', &p2);
		    }
		}
		break;
	    }
	    lowa = b + 1;
	    lowc = d + 1;
	    cvp++;
	}
//	printf ("<<<\n");
//	fetch(ixold, b + 1, upb, f1, ' ');
//	printf (">>>\n");
//	fetch(ixnew, d + 1, upd, f2, ' ');
	n = max (upb - b - 1, upd - d - 1);
	for (i = 0; i <= n; i ++) {
	    if (b + 1 + i <= upb) {
		fetchline (ixold, b + 1 + i, f1, &p1);
		c1 = 1;
	    } else {
//		p1.init ();
		p1.resize (0);
		c1 = 0;
	    }
	    if (d + 1 + i <= upd) {
		fetchline (ixnew, d + 1 + i, f2, &p2);
		c2 = 1;
	    } else {
//		p2.init ();
		p2.resize (0);
		c2 = 0;
	    }
//	    testout (' ', &p1, ' ', &p2);
	    out->out (c1?' ':'*', &p1, c2?' ':'*', &p2);
	}
    }
    context_vec_ptr = context_vec_start - 1;
}
#endif

#if 0
/* dump accumulated "unified" diff changes */
static void
//dump_unified_vec(FILE *f1, FILE *f2)
dump_unified_vec(BBlockFILE* f1, BBlockFILE* f2)
{
	struct context_vec *cvp = context_vec_start;
	int lowa, upb, lowc, upd;
	int a, b, c, d;
	char ch, *f;

	if (context_vec_start > context_vec_ptr)
		return;

	b = d = 0;		/* gcc */
	lowa = max(1, cvp->a - context);
	upb = min(len[0], context_vec_ptr->b + context);
	lowc = max(1, cvp->c - context);
	upd = min(len[1], context_vec_ptr->d + context);

	fputs("@@ -", stdout);
	uni_range(lowa, upb);
	fputs(" +", stdout);
	uni_range(lowc, upd);
	fputs(" @@", stdout);
#if 0
	if (pflag) {
		f = match_function(ixold, lowa-1, f1);
		if (f != NULL) {
			putc(' ', stdout);
			fputs(f, stdout);
		}
	}
#endif
	putc('\n', stdout);

	/*
	 * Output changes in "unified" diff format--the old and new lines
	 * are printed together.
	 */
	for (; cvp <= context_vec_ptr; cvp++) {
		a = cvp->a;
		b = cvp->b;
		c = cvp->c;
		d = cvp->d;

		/*
		 * c: both new and old changes
		 * d: only changes in the old file
		 * a: only changes in the new file
		 */
		if (a <= b && c <= d)
			ch = 'c';
		else
			ch = (a <= b) ? 'd' : 'a';

		switch (ch) {
		case 'c':
			fetch(ixold, lowa, a - 1, f1, ' ', 0);
			fetch(ixold, a, b, f1, '-', 0);
			fetch(ixnew, c, d, f2, '+', 0);
			break;
		case 'd':
			fetch(ixold, lowa, a - 1, f1, ' ', 0);
			fetch(ixold, a, b, f1, '-', 0);
			break;
		case 'a':
			fetch(ixnew, lowc, c - 1, f2, ' ', 0);
			fetch(ixnew, c, d, f2, '+', 0);
			break;
		}
		lowa = b + 1;
		lowc = d + 1;
	}
	fetch(ixnew, d + 1, upd, f2, ' ', 0);

	context_vec_ptr = context_vec_start - 1;
}
#endif

#if 0
static void
//print_header(const char *file1, const char *file2)
print_header ()
{
#if 0
	if (label[0] != NULL)
		fprintf(stdout,"%s %s\n", format == D_CONTEXT ? "***" : "---",
		    label[0]);
	else
#endif
//		fprintf(stdout,"%s %s\t%s", format == D_CONTEXT ? "***" : "---",
//		    file1, ctime(&stb1.st_mtime));
		fprintf(stdout,"%s\n", format == D_CONTEXT ? "***" : "---");
#if 0
	if (label[1] != NULL)
		fprintf(stdout,"%s %s\n", format == D_CONTEXT ? "---" : "+++",
		    label[1]);
	else
#endif
//		fprintf(stdout,"%s %s\t%s", format == D_CONTEXT ? "---" : "+++",
//		    file2, ctime(&stb2.st_mtime));
		fprintf(stdout,"%s\n", format == D_CONTEXT ? "---" : "+++");
}
#endif
