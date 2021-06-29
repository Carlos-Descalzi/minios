#ifndef _SPAWN_H_
#define _SPAWN_H_

int spawn   (const char* path, int nargs, char** argv, int nenvs, char** env);
int waitpid (int pid);

#endif
