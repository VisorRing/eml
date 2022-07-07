#include "form_utf8.h"
#include "util_string.h"
#include "utf8.h"

/*
  Decode both Name and Value in FORM value.
  Delete NUL chars.
  Delete char which is not a valid UTF-8 text.
*/

void  CGIFormUTF8::decode (ustring& key) {
    key = fixUTF8 (urldecode_nonul (key));
}

void  CGIFormUTF8::fix (ustring& key) {
    key = fixUTF8 (key);
}

