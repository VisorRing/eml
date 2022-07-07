#ifndef DIFF_H
#define DIFF_H

#include "ustring.h"
#include "ml.h"
class  MlEnv;

int  diffreg (ustring* text1, ustring* text2, MNodePtr* pleftc, MNodePtr* pleftline, MNodePtr* prightc, MNodePtr* prightline, MlEnv* mlenv);

#endif /* DIFF_H */
