#include "util_random.h"
#include "ustring.h"
#include "util_base64.h"
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <assert.h>

//static int  Inited = 0;
//static unsigned long  Seed;
//static ustring  RChar (CharConst ("ABCDEFGHJKLMNPQRTUVWXYZabcdefghijkmnopqrstuvwxyz23456789"));
static ustring  SaltChar (CharConst ("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"));

#if 0
static void  init () {
    int  fd;

    fd = open ("/dev/urandom", O_RDONLY);
    if (fd < 0)
	assert (0);
    read (fd, &Seed, sizeof (Seed));
    srandom (Seed);
    Inited = 1;
}
#endif

static void  enc (unsigned long v, char* b) {
    int  i;
    int  n = SaltChar.size ();

    for (i = 0; i < 5; i ++) {
	*b ++ = SaltChar[v % n];
	v /= n;
    }
}

ustring  randomKey () {
    char  b[32];

//    if (! Inited)
//	init ();
    enc (arc4random (), b);
    enc (arc4random (), b + 5);
    enc (arc4random (), b + 10);
    enc (arc4random (), b + 15);
    return ustring (b, 20);
}

ustring  smallRandomKey () {
    char  b[32];

//    if (! Inited)
//	init ();
    enc (arc4random (), b);
    enc (arc4random (), b + 5);
    return ustring (b, 10);
}

ustring  randomKey (unsigned long n) {
    char  b[32];
    size_t  s;

    s = snprintf (b, 32, "%.10lu", n);
    return ustring (b, s) + randomKey ();
}

static void  encSalt (unsigned long v, char* b) {
    int  i;
    int  n = SaltChar.size ();

    for (i = 0; i < 4; i ++) {
	b[i] = SaltChar[v % n];
	v /= n;
    }
}

ustring  makeSalt () {
    char  b[16];

//    if (! Inited)
//	init ();
    b[0] = '$';
    b[1] = '1';
    b[2] = '$';
    enc (arc4random (), b + 3);
    enc (arc4random (), b + 7);
    b[11] = '$';
    return ustring (b, 12);
}

ustring  makeSalt (char digit, size_t len) {
    char  b[48];

    if (len > 32)
	len = 32;
//    if (! Inited)
//	init ();
    b[0] = '$';
    b[1] = digit;
    b[2] = '$';
    for (int i = 0; i < 35; i += 5) {
	if (len > i) {
	    enc (arc4random (), b + i + 3);
	} else {
	    break;
	}
    }
    b[len + 3] = '$';
    return ustring (b, len + 4);
}

static ustring  bcrypt_base64encode (char* buf, size_t len) {
    static const u_int8_t base64Code[] =
	"./ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    ustring  ans;
    uint8_t  c1, c2;
    while (len -- > 0) {
	c1 = *buf ++;
	ans.append (1, base64Code[c1 >> 2]);
	c1 = (c1 & 0x03) << 4;
	if (len == 0) {
	    ans.append (1, base64Code[c1]);
	    break;
	}
	c2 = *buf ++;
	-- len;
	c1 |= (c2 >> 4) & 0x0f;
	ans.append (1, base64Code[c1]);
	c1 = (c2 & 0x0f) << 2;
	if (len == 0) {
	    ans.append (1, base64Code[c1]);
	    break;
	}
	c2 = *buf ++;
	-- len;
	c1 |= (c2 >> 6) & 0x03;
	ans.append (1, base64Code[c1]);
	ans.append (1, base64Code[c2 & 0x3f]);
    }
    return ans;
}

ustring  bcryptGenSalt (int log_rounds) {
#define BCRYPT_MAXSALT 16
    char  pre[8];
    char  csalt[BCRYPT_MAXSALT];
    arc4random_buf (csalt, sizeof (csalt));
//    ustring  key (csalt, sizeof (csalt));
    if (log_rounds < 4)
	log_rounds = 4;
    else if (log_rounds > 31)
	log_rounds = 31;
    size_t  n = snprintf (pre, 8, "$2b$%2.2u$", log_rounds);
//    return ustring (pre, n) + base64Encode (key.begin (), key.end ());
    ustring  ans = ustring (pre, n) + bcrypt_base64encode (csalt, sizeof (csalt));
    return ans;
}

double  randDouble () { 
//    if (! Inited)
//	init ();
    return arc4random () / double (0x80000000UL);
} 

long  random_int (unsigned long n) {
//    if (! Inited)
//	init ();
    return arc4random () % n;
}
