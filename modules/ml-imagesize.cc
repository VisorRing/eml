#include "config.h"
#include "ml-imagesize.h"
#include "ml-store.h"
#include "ml.h"
#include "mlenv.h"
#include "expr.h"
#include "util_string.h"
#include "util_const.h"
#include "memcursor.h"
#include "ustring.h"
#include "filemacro.h"

/*DOC:
==image size function==
画像ファイルのサイズを取得する。

*/

static bool  gifSize (ustring* data, size_t& x, size_t& y) {
    MemCursor  csr (data);

    csr.skip (6);
    if (csr.size () >= 4) {
	x = csr.ui16 ();
	y = csr.ui16 ();
	return true;
    }
    return false;
}

static bool  jpegSize (ustring* data, size_t& x, size_t& y) {
    MemCursor  csr (data);
    int  n;
    uint32_t  marker, code;

    csr.skip (2);
#ifdef MSJFIFHACK
    if (matchSkip (b, e, CharConst ("\xff\xfe"))) { // JPEG_COM
	n = readWord (b, e);
	b += n;
    }
#endif
    while (! csr.empty ()) {
	marker = csr.ui8 ();
	code = csr.ui8 ();
	while (code == 0xff) {
	    code = csr.ui8 ();
	}
	n = csr.ui16n ();
	if (marker != 0xff || csr.empty ())
	    return false;
	if (0xc0 <= code && code <= 0xc3) {
	    csr.ui8 ();
	    y = csr.ui16n ();
	    x = csr.ui16n ();
	    return true;
	}
	csr.skip (n - 2);
    }
    return false;
}

static bool  pngSize (ustring* data, size_t& x, size_t& y) {
    MemCursor  csr (data);

    csr.skip (12);
    if (csr.matchSkip (CharConst ("IHDR"))) {
	if (csr.size () >= 8) {
	    x = csr.ui32n ();
	    y = csr.ui32n ();
	    return true;
	}
    }
    return false;
}

static bool  swfSize (ustring* data, size_t& x, size_t& y) {
    MemCursor  csr (data);
    int  n;
    int  minx, maxx, miny, maxy;

    csr.b += 8;
    csr.initBit ();
    n = csr.ub (5);
    minx = csr.sb (n);
    maxx = csr.sb (n);
    miny = csr.sb (n);
    maxy = csr.sb (n);
    if (! csr.empty ()) {
	x = (maxx - minx) / 20. + 0.5;
	y = (maxy - miny) / 20. + 0.5;
	return true;
    } else {
	return false;
    }
}

static bool  imageSize (ustring* data, size_t& x, size_t& y, ustring& type) {
    if (matchHead (*data, CharConst ("GIF87a"))) {
	type.assign (CharConst ("gif"));
	return gifSize (data, x, y);
    } else if (matchHead (*data, CharConst ("GIF89a"))) {
	type.assign (CharConst ("gif"));
	return gifSize (data, x, y);
    } else if (matchHead (*data, CharConst ("\xFF\xD8"))) {
	type.assign (CharConst ("jpeg"));
	return jpegSize (data, x, y);
    } else if (matchHead (*data, CharConst ("\x89PNG\x0d\x0a\x1a\x0a"))) {
	type.assign (CharConst ("png"));
	return pngSize (data, x, y);
#if 0
    } else if (matchHead (*data, CharConst ("MM\x00\x2a"))) {
	type.assign (CharConst ("tiff"));
	return tiffSize (data, x, y);
    } else if (matchHead (*data, CharConst ("II\x2a\x00"))) {
	type.assign (CharConst ("tiff"));
	return tiffSize (data, x, y);
    } else if (matchHead (*data, CharConst ("BM"))) {
	type.assign (CharConst ("bmp"));
	return bmpSize (data, x, y);
#endif
    } else if (matchHead (*data, CharConst ("FWS"))) {
	type.assign (CharConst ("swf"));
	return swfSize (data, x, y);
#if 0
    } else if (matchHead (*data, CharConst ("CWS"))) {
	type.assign (CharConst ("swf"));
	return cswfSize (data, x, y);
#endif
    } else {
	type = uEmpty;
	return false;
    }
}

/*DOC:
===image-size===
 (image-size FILENAME [#serial | #named | #static]) -> (WIDTH HEIGHT TYPE) or NIL
// (image-size [:source-serial FILENAME | :source-named FILENAME | :source-static FILENAME]) -> (WIDTH HEIGHT TYPE) or NIL

TYPEは、gif, jpeg, png, swf。

*/
//#XAFUNC	image-size	ml_image_size
MNode*  ml_image_size (bool fev, MNode* cell, MlEnv* mlenv) {
    paramType  posList[] = {EV_LIST, EV_END};
    MNodePtr  posParams[1];
    kwParam  kwList[] = {{CharConst ("serial"), EV_LIST},	 // 0
			 {CharConst ("named"), EV_LIST},	 // 1
			 {CharConst ("static"), EV_LIST},	 // 2
			 {NULL, 0, EV_END}};
    MNodePtr  kwParams[3];
    evalParams (fev, mlenv, cell, posList, posParams, kwList, kwParams);
    StoreType  storetype (mlenv);
    storetype.setParam (to_string (posParams[0]()));
    if (to_bool (kwParams[0]()))	// serial
	storetype.setSerial ();
    if (to_bool (kwParams[1]()))	// named
	storetype.setNamed ();
    if (to_bool (kwParams[2]()))	// static
	storetype.setStatic ();
    ustring  src = storetype.src ();
    if (src.length () > 0) {
	FileMacro  f;
	ustring  data;
	size_t  x, y;
	ustring  type;

	if (f.openRead (src.c_str ())) {
	    data.resize (32768);
	    data.resize (f.read ((char*)data.data (), 32768));
	    if (imageSize (&data, x, y, type)) {
		MNodeList  ans;
		ans.append (newMNode_num (x));
		ans.append (newMNode_num (y));
		ans.append (newMNode_str (new ustring (type)));
		return ans.release ();
	    }
	}
    }
    return NULL;
}
