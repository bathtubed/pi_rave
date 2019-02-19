#!/usr/bin/bash

echo "This is running"

if [ -f /var/run/pigpio.pid ]
then
	rm /var/run/pigpio.pid
fi

exec sense
