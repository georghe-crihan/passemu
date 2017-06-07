#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <sys/ioctl.h>

#include <linux/uinput.h>

#include "passemu.h"

int load_pass(char **buf)
{
    int fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if (fd < 0) 
      perror("open");

    /* FIXME: stat for filesize
       FIXME: ssh decrypt
       but OK for now.
    */
    *buf = malloc(1024);

    char *home = getenv("HOME");
    sprintf(*buf, "%s/.creds.aes", home);
    int cred = open(*buf, O_RDONLY);

    /* Drop privileges */
    if (setuid(1000) < 0)
      return -1;
 
    int cnt = read(cred, *buf, 1024);
    close(cred);

    if ((*buf)[cnt-1] == '\n')
      cnt--;
    (*buf)[cnt] = '\0';

    int ret = dev_setup(fd, "iButton-crypto");
    if (ret < 0)
      return ret;

    usleep(TO * 10);

    return fd;
}

int cleanup(int fd)
{
    int ret = ioctl(fd, UI_DEV_DESTROY);
    if (ret < 0)
      perror("ioctl UI_DEV_DESTROY");

    close(fd);

    return ret;
}

#if 0
//extern int getchar_ne(void);

int main(int ac, char **av)
{
char *buf = NULL;

    int fd = load_pass(&buf);
    inject_string(fd, buf);
//    printf("%c\n", getchar_ne());

    if (buf != NULL)
      free(buf);

    return cleanup(fd)==-1 ? EXIT_FAILURE : EXIT_SUCCESS;
}
#endif

