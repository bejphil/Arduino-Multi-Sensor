#!/bin/bash

#Script to read information from sensors attached to Arduino via a USB connection
#Procedure is send command -> read response

#Set serial port flags. Of particular importance is the "-hupcl" flag which prevents the Arduino from reseting every time the serial port is opened
#Note that the Arduino has to be located at port /dev/ttyACM0 for this script to operate properly

stty -F /dev/ttyACM0 cs8 cread clocal 115200 ignbrk -brkint -icrnl -imaxbel -opost -onlcr -isig -icanon -iexten -echo -echoe -echok -echoctl -echoke noflsh -ixon -crtscts -hupcl

#alternative stty settings to try in case things get desperate

# stty -F /dev/ttyACM0 115200 -parenb -parodd cs8 -hupcl \
# -cstopb cread clocal -crtscts -iuclc -ixany -imaxbel \
# -iutf8 -opost -olcuc -ocrnl -onlcr -onocr -onlret -ofill \
#  -ofdel nl0 cr0 tab0 bs0 vt0 ff0 -isig -icanon -iexten \
# -echo -echoe -echok -echonl -noflsh -xcase -tostop -echoprt \
# -echoctl -echoke

#loop sends command and reads response
#after reading response we check to see if we have a blank string
#if an emptry string is read, restart the loop
#if a non-empty string is read exit the loop and print the result

while true; do
	echo "GET" > /dev/ttyACM0
	#flags for read: -r, ignore escape characters, -t 2, wait two seconds for input, -N 120, read 120 characters
	#omitting the -N flag will result in reading a random amount of characters < 20, from a random place in the full string.
	read -r -t 2 -N 120 line < /dev/ttyACM0

	#check to see if string contains zero characters, if it does repeat loop, if not break
	if [ ! -z "$line" ]; then
		break
	fi
done

printf "%s\n" "$line"
