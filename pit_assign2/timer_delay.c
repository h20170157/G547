#include <linux/module.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <asm/io.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/kdev_t.h>
#include <linux/uaccess.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Akshay Joshi");
MODULE_DESCRIPTION("1ms delay through PIT concept !!");
 
// register address of counter0
#define counter0_reg 0x40

// register location for setting operaton mode
#define ctrl_word_reg 0x43

// 1193 count is the timer resolution for 8254, which is equivalent to 1ms
int initial_count=1193;

// defining the control word
// counter 0 = 00
// read LSB, then MSB = 11
// mode 3 = 011 (square wave generator)
// Decimal/BCD = 0
// So, control string = 00110110 
#define ctrl_word 0x34

char dev_file_name[]="timer0";
char driver_name[]="pit_timer";
struct class *dev_class;
struct cdev ext_cdev;
dev_t dev=0;
unsigned long delayMS;

void causeSleep(void)
{
	outb_p(ctrl_word, ctrl_word_reg);

	// reading MSB
	outb_p(initial_count, counter0_reg);

	// reading LSB
	outb_p(initial_count>>8, counter0_reg);
}

int current_counter_val(void)
{
	int pit_val;
	outb_p(0x00, ctrl_word_reg);
	pit_val=inb(counter0_reg);
	pit_val=(pit_val | (inb(counter0_reg)<<8));
	return pit_val;
}

static int my_write(struct file *filp, char *buf, size_t length, size_t *offset)
{
	// from user space, delay comes in char[]. Convert it to unsigned long
	kstrtol(buf,10,&delayMS);

	printk(KERN_INFO "Driver:write() !!\n");
	printk(KERN_INFO "Delay MS:%lu\n",delayMS);
        
	// implement delay for delayMS milliseconds
	while(delayMS--)
		causeSleep();
        return 0;
}

static int my_read(struct file *filp, char __user *buf, size_t length, size_t *offset)
{
        int curr_count_val=0;
        printk(KERN_INFO "Driver:read() !!\n");
	
	// get current value of PIT counter
	curr_count_val=current_counter_val();
        printk(KERN_INFO "Currnet PIT timer value:%d\n",curr_count_val);

	// write PIT timer count to user space buffer
        sprintf(buf,"%d",curr_count_val);
        return 0;
}

struct file_operations fops={
        .owner=THIS_MODULE,
        .read=my_read,
	.write=my_write,
};

int init_func(void)
{
	printk(KERN_INFO "Entering timer program !!\n");

        // allocating major/minor number
        if(alloc_chrdev_region(&dev,0,1,driver_name)<0)
        {
                printk(KERN_INFO "Cannot add device to system. Aborting !!\n");
                return -1;
        }
        printk(KERN_INFO "Device %s created with <%i, %i>\n",driver_name, MAJOR(dev), MINOR(dev));

        // creating cdev structure
        cdev_init(&ext_cdev,&fops);
        ext_cdev.owner=THIS_MODULE;
        ext_cdev.ops=&fops;

        // adding character device to the system
        if(cdev_add(&ext_cdev,dev,1)<0)
        {
                printk(KERN_INFO "Cannot add device to the system !!\n");
                goto r_class;
        }

        // creating struct class
        if((dev_class=class_create(THIS_MODULE,"ext_class")) == NULL)
        {
                printk(KERN_INFO "Cannot create struct class !!\n");
                goto r_class;
        }

        // creating device
        if((device_create(dev_class,NULL,dev,NULL,dev_file_name))==NULL)
        {
                printk(KERN_INFO "Cannot create device file !!\n");
                goto r_device;
        }
        printk(KERN_INFO "Device file created successfully !!\n");

        return 0;

        r_device:
                class_destroy(dev_class);
        r_class:
                unregister_chrdev_region(dev,1);

        return -1;
}

void exit_func(void)
{
        device_destroy(dev_class,dev);
        class_destroy(dev_class);
        cdev_del(&ext_cdev);
        unregister_chrdev(dev, 1);
	printk(KERN_INFO "Leaving timer program !!\n");
}

module_init(init_func);
module_exit(exit_func);

