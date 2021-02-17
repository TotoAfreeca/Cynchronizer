#!/bin/bash
rm -R Source
rm -R Destination
mkdir Source
mkdir Destination
mkdir Source/A
touch Source/A/a.txt
touch Source/b.txt
echo "TEST" >> Source/b.txt
dd if=/dev/zero of=Source/A/c.txt count=1 bs=1048576
dd if=/dev/zero of=Source/A/d.txt count=10 bs=1048576
#dd if=/dev/zero of=Source/A/e.txt count=100 bs=1048576
#dd if=/dev/zero of=Source/A/f.txt count=500 bs=1048576
#dd if=/dev/zero of=Source/A/g.txt count=1000 bs=1048576
mkdir Destination/B
touch Destination/B/a.txt
echo "TEST TEST TEST TEST" >> Destination/B/a.txt
touch Destination/b.txt
mkdir Destination/B/H

#testing
./Cynchronizer ./Source ./Destination -R -s 20
sleep 5
less /var/log/syslog | grep Cynchronizer


