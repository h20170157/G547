#include<linux/fs.h>
#include<linux/init.h>
#include<linux/module.h>
#include<linux/cdev.h>
#include<linux/device.h>
#include<linux/kernel.h>
#include<linux/kdev_t.h>
#include<linux/uaccess.h>
#include<linux/random.h>

// defining the IOCTL code

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Akshay Joshi");
MODULE_DESCRIPTION("Testing a character driver !!");

char dev_file_name[]="adc";
char driver_name[]="ADC_Driver";
size_t ret_val=0;
struct class *dev_class;
struct cdev ext_cdev;
unsigned int voltage;
int channel_num;
dev_t dev=0;

void generate_voltage(void)
{
	switch(channel_num)
	{
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
			printk(KERN_INFO "Channel number:%d",channel_num);
			get_random_bytes(&voltage, sizeof(unsigned int));
			break;
		default:
			printk(KERN_INFO "Enter valid channel number !!\n");
	}
}

static int my_read(struct file *filp, char __user *buf, size_t length, size_t *offset)
{
	unsigned int ret_val=0;
	printk(KERN_INFO "Driver:read() !!\n");
	printk(KERN_INFO "Channel_num:%d\n",channel_num);
	generate_voltage();
	ret_val=voltage%1024;
	printk(KERN_INFO "Voltage generated:%u\n",ret_val);
	sprintf(buf,"%u",ret_val);
	return 0;
}

static int my_write(struct file *filp, char *buf, size_t length, size_t *offset)
{
	printk(KERN_INFO "Driver:write() !!\n");
	channel_num=(int)buf-'0';
	printk(KERN_INFO "Channel number:%d\n",channel_num);
	return 0;
}

static int my_open(void)
{
	printk(KERN_INFO "Driver:open() !!\n");
	return 0;
}

static int my_close(void)
{
	printk(KERN_INFO "Driver:close() !!\n");
	return 0;
}

struct file_operations fops={
	.owner=THIS_MODULE,
	.read=my_read,
	.write=my_write,
	.open=my_open,
	.release=my_close
};

int start_module(void)
{
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

void end_module(void)
{
	device_destroy(dev_class,dev);
	class_destroy(dev_class);
	cdev_del(&ext_cdev);
	unregister_chrdev(dev, 1);
	printk(KERN_INFO "Driver unregistered !!\n");
}

module_init(start_module);
module_exit(end_module);
