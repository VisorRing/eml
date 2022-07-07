#ifndef BF_H
#define BF_H

#include "ustring.h"

typedef  enum {
    PAD_ZERO,
    PAD_PKCS5,
}  crypto_padding_t;

bool  bf_cbc_decode (ustring& ans, const ustring& text, const ustring& key, const ustring& iv, crypto_padding_t padding);

#endif /* BF_H */
