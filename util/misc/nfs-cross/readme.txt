Contributed by Ajaykumar S Patil [ajaykumar.patil@arm.com]
==========================================================

On cross compilation platforms, I have written some code (perl on host, C code for target)
to automate running of tests. 
This method involves the target accessing files over NFS. 
It might be particularly useful when using targets with limited memory. 
Since we copy data during building of benchmarks, 
all the datasets put together may not fit on target memory and hence NFS.

If above method makes sense here are some steps in using the
infrastructure:

1. Attached run_MITHserver.c should be compiled for target 
(currently works on linux). 
Put the resulting executable run_MIThserver.exe in root installation directory on host (example: beta2).

2. Client.pl also goes to beta2

3. linux.mak goes to util/make/linux.mak

4. Configure target IP address in Client.pl

5. Mount the beta2 directory on target (over NFS) 
and start run_MIThserver on target. 
Everytime we say "make TARGET=linux TOOLCHAIN=gcc", 
the client.pl sends a request to server and logs are put in appropriate runlogs. 

