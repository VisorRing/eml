#include "form_utf8-jp.h"
#include "util_string.h"
#include "utf8.h"
#include "utf8-jp.h"

void  CGIFormUTF8JPMS::decode (ustring& key) {
    key = fixFromMS (fixUTF8 (urldecode_nonul (key)));
}

void  CGIFormUTF8JPMS::fix (ustring& key) {
    key = fixFromMS (fixUTF8 (key));
}

