OBJECTIVE: To implement PIT delay with 1ms resolution.

#1
To insert char driver into linux kernel, execte following commands:

$ make all
$ sudo insmod timer_delay.ko

The char driver will enter into kernel.

#2
The user space program does the following:
a. Taking delay (in milliseconds).
b. Passing the delay to kernel space.
c. From kernel space, it'll return current PIT timer value ranging between 0-1193.

To pass value from user space, enter following commands in sequence:

$ gcc user_space.c -o usr_prg
$ sudo ./usr_prg

NOTE: usr_prg will create device file (/dev/timer0) on it's own. So t must always be run with sudo, else /dev/timer0 will not have appropriate permissons, and user program will not execute further.
