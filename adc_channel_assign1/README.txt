KINDLY NOTE:

Running of device file (/dev/adc) requires some permission changes. So kindly run user space output file using sudo as mentioned below:

$ gcc usr_space.c -o usr_prg
$ sudo ./usr_prg

This will set correct permissions in /dev/adc
