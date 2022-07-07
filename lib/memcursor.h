#ifndef MEMCURSOR_H
#define MEMCURSOR_H

class  MemCursor {
 public:
    uiterator  b;
    uiterator  e;
    int  bitOffset;
    uint32_t  bitPool;

    MemCursor (const ustring* data) {
	b = data->begin ();
	e = data->end ();
    };
    virtual  ~MemCursor () {};

    virtual bool  empty () {
	return b >= e;
    };
    virtual ssize_t  size () {
	return e - b;
    };
//    virtual bool  cap (size_t size) {
//	return b + size <= e;
//    };
    virtual uint32_t  ui8 () {
	uint8_t  ans = *b;
	++ b;
	return ans;
    };
    virtual uint32_t  ui16n () {
	return ui8 () * 256 + ui8 ();
    };
    virtual uint32_t  ui16 () {
	return ui8 () + ui8 () * 256;
    };
    virtual uint32_t  ui32n () {
	return ((ui8 () * 256 + ui8 ()) * 256 + ui8 ()) * 256 + ui8 ();
    };
//    virtual uint32_t  ui32 ();
//    virtual int32_t  si16 ();
//    virtual int32_t  si32 ();
//    virtual ustring  block (size_t len);
//    virtual ustring  block ();
    virtual void  skip (size_t len) {
	b += len;
    };
//    virtual ustring  cstring ();
//    virtual ustring  pstring ();
    virtual bool  matchSkip (const char* t, size_t s) {
	return ::matchSkip (b, e, t, s);
    };
    virtual void  initBit () {
	bitOffset = 0;
    };
    virtual uint32_t  ub (int n) {
	uint32_t  ans = 0;
	int  nc = n;
	
	while (nc > 0) {
	    if (bitOffset == 0) {
		bitPool = ui8 ();
		bitOffset = 8;
	    }
	    if (nc >= bitOffset) {
		nc -= bitOffset;
		ans |= bitPool << nc;
		bitOffset = 0;
	    } else {
		bitOffset -= nc;
		ans |= bitPool >> bitOffset;
		bitPool &= (1 << bitOffset) - 1;
		nc = 0;
	    }
	}
	return ans;
    };
    virtual int32_t  sb (int n) {
	int32_t  ans = 0;
	
	if (n > 0) {
	    if (ub (1)) {
		ans = ub (n - 1) - (1L << (n - 1));
	    } else {
		ans = ub (n - 1);
	    }
	} else {
	    ans = 0;
	}
	return ans;
    };
};

#endif /* MEMCURSOR_H */
