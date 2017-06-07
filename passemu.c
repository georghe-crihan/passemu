#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include <sys/ioctl.h>

//#include <sys/types.h>
//#include <sys/stat.h>
//#include <fcntl.h>
#include <ctype.h>

#include <linux/input.h>
#include <linux/uinput.h>

#include "passemu.h"

/* /usr/include/linux/input-event-codes.h */
static int ASCII_TO_VK[] = {
/*        0          1      2      3      4      5      6      7      8              9      A      B           C      D          E      F */
/* 0 */   0,         0,     0,     0,     0,     0,     0,     0,     KEY_BACKSPACE, 0,     0,     0,          0,     KEY_ENTER, 0,     0, 
/* 1 */   0,         0,     0,     0,     0,     0,     0,     0,     0,             0,     0,     KEY_ESC,    0,     0,         0,     0,
/* 2 */   KEY_SPACE, KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7, KEY_8,         KEY_9, 0,     0,          0,     0,         0,     0,
/* 3 */   KEY_0,     KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7, KEY_8,         KEY_9, 0,     0,          0,     0,         0,     0, 
/* 4 */   KEY_1,     KEY_A, KEY_B, KEY_C, KEY_D, KEY_E, KEY_F, KEY_G, KEY_H,         KEY_I, KEY_J, KEY_K,      KEY_L, KEY_M,     KEY_N, KEY_O,
/* 5 */   KEY_P,     KEY_Q, KEY_R, KEY_S, KEY_T, KEY_U, KEY_V, KEY_W, KEY_X,         KEY_Y, KEY_Z, 0,          0,     0,         0,     0,
/* 6 */   0,         KEY_A, KEY_B, KEY_C, KEY_D, KEY_E, KEY_F, KEY_G, KEY_H,         KEY_I, KEY_J, KEY_K,      KEY_L, KEY_M,     KEY_N, KEY_O,
/* 7 */   KEY_P,     KEY_Q, KEY_R, KEY_S, KEY_T, KEY_U, KEY_V, KEY_W, KEY_X,         KEY_Y, KEY_Z, 0,          0,     0,         0,     0
};

static int key_evt(int fd, int c, int p)
{
    struct input_event ev = {0};
    memset(&ev, 0, sizeof(ev));
    gettimeofday(&ev.time, NULL);
    ev.type = EV_KEY;
    ev.code = c;
    ev.value = p;
    return write(fd, &ev, sizeof(ev));
}

static void inject_key(int fd, int bSync, int c)
{
    int ret  = key_evt(fd, c, 1); 
    if (ret < 0)
      perror("write ev value=1");

    ret = key_evt(fd, c, 0);
    if (ret < 0)
      perror("write ev value=0");

    if (bSync) { /* Sync event */
      struct input_event ev;
      memset(&ev, 0, sizeof(ev));
      gettimeofday(&ev.time, NULL);
      ev.type = EV_SYN;
      ev.code = SYN_REPORT;
      ev.value = 0;

      ret = write(fd, &ev, sizeof(ev));
      if (ret < 0)
        perror("write EV_SYNC");
    }
}

int dev_setup(int fd, const char *name)
{
    int ret = ioctl(fd, UI_SET_EVBIT, EV_KEY);
    if (ret < 0)
      perror("Ioctl SET_EVBIT");

    /* Enable keyboard bits */
    for (int i = 0; i < sizeof(ASCII_TO_VK)/sizeof(int); i++)
      if (ASCII_TO_VK[i])
        ret = ioctl(fd, UI_SET_KEYBIT, ASCII_TO_VK[i]);

    ret = ioctl(fd, UI_SET_KEYBIT, KEY_RIGHTSHIFT);

    struct uinput_user_dev uidev = {0};
    memset(&uidev, 0, sizeof(uidev));
    snprintf(uidev.name, UINPUT_MAX_NAME_SIZE, name);
    ret = write(fd, &uidev, sizeof(uidev));
    if (ret < 0)
      perror("write uidev");
    ret = ioctl(fd, UI_DEV_CREATE);
    if (ret < 0)
      perror("ioctl UI_DEV_CREATE");
    return ret;
}

void inject_string(int fd, const char *s)
{
    for (char *p = (char *)s; p[0]!= '\0'; p++) {
      /* FIXME: All shifts */
      if (isupper(p[0]) || p[0] == '@' || ( p[0] > 0x20 && p[0] < 0x2c ) )
        key_evt(fd, KEY_RIGHTSHIFT, 1);
      inject_key(fd, 1, ASCII_TO_VK[(int)p[0]]);
      if (isupper(p[0]) || p[0] == '@' || ( p[0] > 0x20 && p[0] < 0x2c ) )
        key_evt(fd, KEY_RIGHTSHIFT, 0);
      usleep(TO);
    }
}

