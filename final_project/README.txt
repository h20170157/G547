1. PROJECT TITLE: GPIO Subsystem

final_proj
	|_driver
	|	|_ _GPIO_SUBSYSTEM.c
	|	|_ _Makefile
	|
	|
	|_firmware
		|_ _usbdrv
		|_ _checksize
		|_ _main.c
		|_ _Makefile
		|_ _usbconfig.h


2. SUMMARY
The project demonstrates the GPIO of a given firmware over USB. To display the pins as output pins, we have used 4 LEDs. Depending on the pin number mappings, the LEDs will glow. To display the pins as input pins, the state of the button is displayed, depending on the button press.

Following 18 pins are being used as GPIO pins respectively:
PD0, PD1, PD3, PD5, PD6, PD7 (PD2 and PD4 are D+ and D- of V-USB respectively).
PB0, PB1, PB2, PB3, PB4, PB5 (PB6 and PB7 are used for 16 MHz crystal).
PC0, PC1, PC2, PC3, PC4, PC5


3. BUILDING THE CODE
3.1 Firmware
$ make all
$ sudo avrdude -c usbasp -p m8 -U flash:w:main.hex

3.2 Driver
$ make clean && make all
$ sudo insmod GPIO_SUBSYSTEM.ko

3.3 After inserting the module, chip should get detected in the followng directory
$ cd /sys/class/gpio
$ ls
Check whether a chip is displayed or not. Kernel will assign some random number to that chip. Eg: chip<NUMBER> .This number will be the starting gpio pin offset. We can export NUMBER+18 GPIO pins.

3.4 $ sudo chmod 777 export unexport
So, if gpio500 is created, then we can export pin numbers ranging from 500-517.

$ echo 509 > export
$ cd gpio509

3.5 This one makes the pin as output pin. 
$ sudo sh -c 'echo out > direction'

3.6 Setting the value to 1 glows the LED.
$ sudo sh -c 'echo 1 > value'

3.7 For input:
$sudo sh -c 'echo in > direction'

To check the value:
Press the button and then execute:
$ cat value

The button state will change from 0 to 1 or vice-versa.

3.8 Unexporting the gpio pin
$ cd /sys/class/gpio
$ echo 509 > unexport
