# Cynchronizer
A linux daemon to synchronize 2 catalogs

1. Compile the code using `make`
2. Run `skrypt.sh` script or just use `make test` in the folder source - test will create the testing structure of catalogs but you can of course create your own
3. You can of course start the daemon yourself by using previously compiled file for example with:
```bash
./Cynchronizer ./Source ./Destination -t 1 -R -m 0 -s 420
```
or
```bash
./Cynchronizer ./Source ./Destination -t 5646876 -m 1
```

The arguments go as follows:
./Cynchronizer source_path target_path [-t threshold] [-R] [-s seconds_sleeping]

NOTE: First and second argument are always source and target folders path

-s seconds_sleeping - optional, time in seconds deciding on for how long the daemon should sleep (default is 5 minutes)

-R - optional,  if it exists (true), the daemon will synchronize every nested folder in the recursive pattern - it will recreate the structure and copy the files from source to target
-t threshold - optional, a threshold of byte size deciding whether the file should be copied using standard read/write operations, or in a more efficient way 
-m 0/1 - optional, argument chooses a "more efficient" of copying files above the threshold (0 - MMAP, 1- copy_file_range) - default is 0

A comparison of copying time with various methods using *SSD Kingston 240GB M.2 2280 PCI-E x2 NVMe (READ/WRITE [MB/s] - 1500/800)*
Averaged on 5 samples
File size [MB] | AVG read/write speed (buffer 128kb) [CPU Time] | AVG MMAP speed [CPU Time] | AVG copy_file_range speed [CPU Time]
------------ | -------------|-------------| ------------- 
1 c | 0,002895 | 0.001087 | 0.000991
10 c | 0.012218 | 0.010152 | 0.025026
100 c | 0.173643 | 0.183509 | 0.149131
500 c | 0.781143 | 0.939122 | 0.741254
1000 c | 1.874431 | 1.819829 | 1.525884
