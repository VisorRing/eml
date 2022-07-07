#ifndef UTIL_RANDOM_H
#define UTIL_RANDOM_H

#include "ustring.h"

ustring  randomKey ();
ustring  randomKey (unsigned long n);
ustring  smallRandomKey ();
ustring  makeSalt ();
ustring  makeSalt (char digit, size_t len);
ustring  bcryptGenSalt (int log_rounds);
double  randDouble ();
long  random_int (unsigned long n);

#endif /* UTIL_RANDOM_H */
