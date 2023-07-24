#!/bin/sh

hour=$(date +%H)

test $hour -gt 17 && echo "Good Evening!" && exit
test $hour -gt 11 && echo "Good Afternoon!" && exit
echo "Good Morning!"
