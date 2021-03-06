/*
 * powerd       Catch power failure signals from
 *              a Trust Energy Protector 400/650
 *              and notify init
 *
 * Usage:       powerd /dev/cua3 (or any other serial device)
 *
 * Author:      Ciro Cattuto <ciro@stud.unipg.it>
 * 
 * Version 1.0 - 31 March 1997
 *
 * This code is heavily based on the original powerd.c code
 * by Miquel van Smoorenburg <miquels@drinkel.ow.org>.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 * 
 */

/* state 0 - power is good */
#define T0_SLEEP        10      /* interval between port reads, in seconds */
#define T0_DCD          3       /* number of seconds DCD has to be high
                                   to cause an action                      */
#define T0_CTS          3       /* number of seconds CTS has to be high
                                   to cause an action                      */
/* state 1 - power is failing */
#define T1_SLEEP        2       /* interval between ports reads            */
#define T1_DCD          3       /* same as T0_DCD                          */
#define T1_CTS          3       /* same as T0_CTS                          */

#define DSR_SLEEP       2
#define DSR_TRIES       60

/* This is the file needed by SysVInit */
#define PWRSTAT         "/etc/powerstatus"

#define PIDPATH "/var/run/powerd.pid"

/* Use the new way of communicating with init. */
//#define NEWINIT

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <syslog.h>
#include <string.h>
#include "paths.h"
#ifdef NEWINIT
#include "initreq.h"
#endif

#ifndef SIGPWR
#  define SIGPWR SIGUSR1
#endif

#ifdef NEWINIT
void alrm_handler()
{
}
#endif

#if 0
/* Tell init that the power has gone (1), is back (0),
   or the UPS batteries are low (2). */
void powerfail(int event)
{
  int fd;
#ifdef NEWINIT
  struct init_request req;

  /* Fill out the request struct. */
  memset(&req, 0, sizeof(req));
  req.magic = INIT_MAGIC;
  switch (event)
        {
        case 0:
                req.cmd = INIT_CMD_POWEROK;
                break;
        case 1:
                req.cmd = INIT_CMD_POWERFAIL;
                break;
        case 2:
        default:
                req.cmd = INIT_CMD_POWERFAILNOW;
        } 

  /* Open the fifo (with timeout) */
  signal(SIGALRM, alrm_handler);
  alarm(3);
  if ((fd = open(INIT_FIFO, O_WRONLY)) >= 0
                && write(fd, &req, sizeof(req)) == sizeof(req)) {
        close(fd);
        return;
  }
  /* Fall through to the old method.. */
#endif

  /* Create an info file for init. */
  unlink(PWRSTAT);
  if ((fd = open(PWRSTAT, O_CREAT|O_WRONLY, 0644)) >= 0) {
  switch (event)
        { 
        case 0:
                write(fd, "OK\n", 3);
                break;

        case 1:
                write(fd, "FAIL\n", 5);
                break;

        case 2:
        default:
                write(fd, "LOW\n", 4);
                break;
        }
  close(fd);
  }

  kill(1, SIGPWR);
}
#endif

static char *buf = NULL;
static int ui = 0;
static char pidstring[16];

extern void inject_string(int ui, char *buf);

void send_psw(int sig)
{
  switch(sig) {
  case SIGWINCH:
	inject_string(ui, "ABCD");
	break;
  case SIGHUP:
  default:
	inject_string(ui, buf);
  }
//  printf("%c\n", getchar_ne());
}

extern int cleanup(int ui);

void terminate(int sig)
{
  if (buf != NULL)
    free(buf);

  cleanup(ui);
  unlink(PIDPATH);
  exit(0);
}

//extern int getchar_ne(void);


extern int load_pass(char **buf);

/* Main program. */
int main(int argc, char *argv[])
{
#if 0
  int fd;
  int dtr_bit = TIOCM_DTR;
  int flags;
  int DCD, CTS;
  int status = -1;
  int DCD_count = 0, CTS_count = 0;
  int tries;
#endif
  int f = 0;

  if (argc < 2) {
        fprintf(stderr, "Usage: powerd <device>\n");
        exit(1);
  }

  /* Start syslog. */
  openlog("iButton-KI", LOG_CONS|LOG_PERROR, LOG_DAEMON);

  f = creat(PIDPATH, 0644);
  if (f < 0)
    syslog(LOG_ERR, "%s: %s", PIDPATH, sys_errlist[errno]);

  ui = load_pass(&buf);
  if (ui < 0)
    exit(1);

#if 0
  /* Open monitor device. */
  if ((fd = open(argv[1], O_RDWR | O_NDELAY)) < 0) {
        syslog(LOG_ERR, "%s: %s", argv[1], sys_errlist[errno]);
        closelog();
        exit(1);
  }

  /* Line is opened, so DTR is high. Force it anyway to be sure. */
  ioctl(fd, TIOCMBIS, &dtr_bit);
#endif

  /* Daemonize. */
  switch(fork()) {
        case 0: /* Child */
                closelog();
                setsid();
                close(0);
                close(1);
                close(2);
                (void)chdir("/");
                sprintf(pidstring, "%d", getpid()); 
                write(f, pidstring, strlen(pidstring));
                close(f);
                signal(SIGHUP, send_psw);
		signal(SIGWINCH, send_psw);
                signal(SIGTERM, terminate);
                break;
        case -1: /* Error */
                syslog(LOG_ERR, "can't fork.");
                closelog();
                exit(1);
        default: /* Parent */
                closelog();
                exit(0);
  }

  /* Restart syslog. */
  openlog("iButton-KI", LOG_CONS, LOG_DAEMON);

#if 0
  /* Now sample the DCD line. */
  while(1) {
        /* Get the status. */
        ioctl(fd, TIOCMGET, &flags);

        /* Check the connection: DSR should be high. */
        tries = 0;
        while((flags & TIOCM_DSR) == 0) {
                /* Keep on trying, and warn every two minutes. */
                if ((tries % DSR_TRIES) == 0)
                    syslog(LOG_ALERT, "UPS connection error");
                sleep(DSR_SLEEP);
                tries++;
                ioctl(fd, TIOCMGET, &flags);
        }
        if (tries > 0)
                syslog(LOG_ALERT, "UPS connection OK");

        /* Calculate present status. */
        DCD = flags & TIOCM_CAR;
        CTS = flags & TIOCM_CTS;

        if (status == -1)
                {
                status = (DCD != 0) ? 0 : 1;
                if (DCD == 0)
                        {
                        syslog(LOG_ALERT, "Power Failure. UPS active.");
                        powerfail(1);
                        }
                }

        switch (status)
                {
                case 0:
                        if ((DCD != 0) && (CTS != 0))
                                {
                                DCD_count = 0;
                                CTS_count = 0;
                                sleep(T0_SLEEP);
                                continue;
                                }
                        if (DCD == 0)
                                DCD_count++;
                        if (CTS == 0)
                                CTS_count++;
                        if ((DCD_count < T0_DCD) && (CTS_count < T0_CTS))
                                {
                                sleep(1);
                                continue;
                                }
                        if (CTS_count == T0_CTS)
                                {
                                status = 2;
                                syslog(LOG_ALERT, "UPS batteries low!");
                                break;
                                }
                        status = 1;
                        DCD_count = 0;
                        syslog(LOG_ALERT, "Power Failure. UPS active."); 
                        break;

                case 1:
                        if ((DCD == 0) && (CTS != 0))
                                {
                                DCD_count = 0;
                                CTS_count = 0;
                                sleep(T1_SLEEP);
                                continue;
                                }
                        if (DCD != 0)
                                DCD_count++;
                        if (CTS == 0)
                                CTS_count++;
                        if ((DCD_count < T1_DCD) && (CTS_count < T1_CTS))
                                {
                                sleep(1);
                                continue;
                                }
                        if (CTS_count == T1_CTS)
                                {
                                status = 2;
                                syslog(LOG_ALERT, "UPS batteries low!");
                                break;
                                }
                        status = 0;
                        DCD_count = 0;
                        CTS_count = 0;                  
                        syslog(LOG_ALERT, "Power okay.");
                        break;

                case 2:
                        sleep(1);
                        continue;

                default:
                        break;
                }

        powerfail(status);
  }
#endif

   while(1)
     sleep(100);

  /* Never happens */
  return(0);
}


