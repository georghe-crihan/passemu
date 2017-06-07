#ifndef PASSEMU_H_INCLUDED
#define PASSEMU_H_INCLUDED

#define TO 10000

extern int dev_setup(int fd, const char *name);
extern void inject_string(int fd, const char *s);

#endif

