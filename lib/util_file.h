#ifndef UTIL_FILE_H
#define UTIL_FILE_H

#include "config.h"
#include "ustring.h"

bool  isPlainFile (const ustring& name);
bool  isExecutableFile (const ustring& name);
bool  isDirectory (const ustring& name);
bool  isPlainOrNoFile (const ustring& name);
bool  fileSize (const ustring& name, off_t& size);
bool  readFile (const ustring& filename, ustring& ans, size_t max = cResourceFileMax);
void  writeFile (const ustring& filename, ustring& data);
void  makeSubdir (ustring& top, const ustring& sub);
void  shapePath (ustring& path);
bool  isAbsolutePath (const ustring& path);

#endif /* UTIL_FILE_H */
