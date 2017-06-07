#!/bin/sh

if [ -z ${1} ]; then
  /bin/kill -HUP $(/bin/cat /var/run/powerd.pid)
else
  /bin/kill -WINCH $(/bin/cat /var/run/powerd.pid)
fi
