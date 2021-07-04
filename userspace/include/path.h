#ifndef _PATH_H_
#define _PATH_H_

char*   path_absolute   (const char* path, char* buffer);
char*   path_normalize  (const char* path, char* buffer);
char*   path_append     (char* path, const char* subpath);
char*   path_basename   (char* path, char* buffer);
char*   path_dirname    (char* path, char* buffer);

#endif
