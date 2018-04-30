
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/usb.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/spinlock.h>


/* usb_control_msg(struct usb_device *dev, unsigned int pipe,
 u8 request, u8 requesttype,
 u16 value, u16 index,
 void *data, u16 size, int timeout);
 */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Vaibhav and Akshay");
MODULE_DESCRIPTION("usb gpio");

// structure to define all the USB data
struct my_usb
{
   struct usb_device *my_dev;
   struct gpio_chip chip; //this is our GPIO chip
   u8 buf[4];
   //spinlock_t lock;
};

// commands to work on the board
typedef enum _command
{
   BOARD_INIT, // This does the init of board
   PINMODE_INPUT, // Set GPIO as input
   PINMODE_OUTPUT, // Set GPIO as output
   PIN_READ,   // Read GPIO
   PIN_WRITE, // Write to GPIO
} command;


// data sent via USB
typedef struct _gpio_data_handle
{
   uint8_t command;
   uint8_t no;
   uint8_t data;
   uint8_t read_val;
} gpio_data_handle;

// setting the data, executes when value file is edited in /sys/class/gpio echo 1/0 > value
static void _gpioa_set(struct gpio_chip *chip,unsigned offset, int value)
{
   struct my_usb *data = container_of(chip, struct my_usb, chip);
   static uint8_t gpio_val;
   int ret;
   gpio_data_handle *gp_d_hd;
   //spin_lock(&data->lock);

   gpio_val = value;
   ret = usb_control_msg(data->my_dev,
                         usb_sndctrlpipe(data->my_dev, 0),
                         PIN_WRITE, USB_TYPE_VENDOR | USB_DIR_OUT,
                         (offset + 1) | (gpio_val << 8), 0,
                         0, 0,
                         1000);
   ret = usb_control_msg(data->my_dev,
                         usb_rcvctrlpipe(data->my_dev, 0),
                         PIN_WRITE, USB_TYPE_VENDOR | USB_DIR_IN,
                         (offset + 1) | (gpio_val << 8), 0,
                         (u8 *)data->buf,3,
                         1000);
   //spin_unlock(&data->lock);
	gp_d_hd = (gpio_data_handle *)data->buf;
        
	printk(KERN_INFO "data write = %d ",gp_d_hd->data);
	printk(KERN_INFO "pin no write = %d ",gp_d_hd->no);
	printk(KERN_INFO "command write = %d ",gp_d_hd->command);
}

// reading the data, executes when value file is read in /sys/class/gpio using cat value
static int _gpioa_get(struct gpio_chip *chip,unsigned offset)
{
   struct my_usb *data = container_of(chip, struct my_usb, chip);
   gpio_data_handle *gp_d_hd;
   int ret;

   printk(KERN_INFO "GPIO GET INFO: %d", offset);

   //spin_lock(&data->lock);
   ret = usb_control_msg(data->my_dev,
                         usb_sndctrlpipe(data->my_dev, 0),
                         PIN_READ, USB_TYPE_VENDOR | USB_DIR_OUT,
                         (offset + 1), 0,
                         0, 0,
                         1000);
   ret = usb_control_msg(data->my_dev,
                         usb_rcvctrlpipe(data->my_dev, 0),
                         PIN_READ, USB_TYPE_VENDOR | USB_DIR_IN,
                         (offset + 1), 0,
                         (u8 *)data->buf,3,
                         1000);
   //spin_unlock(&data->lock);
gp_d_hd = (gpio_data_handle *)data->buf;
	printk(KERN_INFO "data read = %d ",gp_d_hd->data );
	printk(KERN_INFO "pin no read = %d ",gp_d_hd->no );
	printk(KERN_INFO "command read = %d ",gp_d_hd->command );
  return gp_d_hd->data ;
}

// setting direction as output, called when direction is set as out in /sys/class/gpio
static int _direction_output(struct gpio_chip *chip,unsigned offset, int value)
{
   struct my_usb *data = container_of(chip, struct my_usb, chip);
   int ret;
   gpio_data_handle *gp_d_hd;
   //spin_lock(&data->lock);

   ret = usb_control_msg(data->my_dev,
                         usb_sndctrlpipe(data->my_dev, 0),
                         PINMODE_OUTPUT, USB_TYPE_VENDOR | USB_DIR_OUT,
                         (offset + 1), 0,
                         0, 0,
                         1000);
   ret = usb_control_msg(data->my_dev,
                         usb_rcvctrlpipe(data->my_dev, 0),
                         PINMODE_OUTPUT, USB_TYPE_VENDOR | USB_DIR_IN,
                         (offset + 1), 0,
                         (u8 *)data->buf,3,
                         1000);

   //spin_unlock(&data->lock);
	gp_d_hd = (gpio_data_handle *)data->buf;
	printk(KERN_INFO "data output = %d ",gp_d_hd->data );
	printk(KERN_INFO "pin no output = %d ",gp_d_hd->no );
	printk(KERN_INFO "command output = %d ",gp_d_hd->command );
   return 0;
}

// setting direction as input, called when direction is set as 'in' in /sys/class/gpio
static int _direction_input(struct gpio_chip *chip,unsigned offset)
{
   struct my_usb *data = container_of(chip, struct my_usb, chip);
   int ret;
   gpio_data_handle *gp_d_hd;
   //spin_lock(&data->lock);
   ret = usb_control_msg(data->my_dev,
                         usb_sndctrlpipe(data->my_dev, 0),
                         PINMODE_INPUT, USB_TYPE_VENDOR | USB_DIR_OUT,
                         (offset + 1), 0,
                         0, 0,
                         1000);

   ret = usb_control_msg(data->my_dev,
                         usb_rcvctrlpipe(data->my_dev, 0),
                         PINMODE_INPUT, USB_TYPE_VENDOR | USB_DIR_IN,
                         (offset + 1), 0,
                         (u8 *)data->buf,3,
                         1000);
   //spin_unlock(&data->lock);
	gp_d_hd = (gpio_data_handle *)data->buf;
	printk(KERN_INFO "data input = %d ",gp_d_hd->data );
	printk(KERN_INFO "pin no input = %d ",gp_d_hd->no );
	printk(KERN_INFO "command input = %d ",gp_d_hd->command);
   return 0;
}

// USB probe function, called when device is connected
static int my_usb_probe(struct usb_interface *interface,const struct usb_device_id *id)
{
   struct usb_device *my_dev = interface_to_usbdev(interface);
   struct usb_host_interface *iface_desc;
   struct my_usb *data;
   int ret;
   gpio_data_handle *gp_d_hd;
   printk(KERN_INFO "manufacturer: %s", my_dev->manufacturer);
   printk(KERN_INFO "product: %s", my_dev->product);

   iface_desc = interface->cur_altsetting;
   printk(KERN_INFO "GPIO-SUBSYSTEM board %d probed: (%04X:%04X)",iface_desc->desc.bInterfaceNumber, id->idVendor, id->idProduct);
  printk(KERN_INFO "bNumEndpoints: %d", iface_desc->desc.bNumEndpoints);

   data = kzalloc(sizeof(struct my_usb), GFP_KERNEL);
   if (data == NULL)
     {
        printk(KERN_INFO "Failed to alloc data");
        return -ENODEV;
     }

   //increase ref count, make sure u call usb_put_dev() in disconnect()
   data->my_dev = usb_get_dev(my_dev);

   data->chip.label = "GPIO-SUBSYSTEM";
   data->chip.parent = &data->my_dev->dev; // optional device providing the GPIOs
   data->chip.owner = THIS_MODULE; // helps prevent removal of modules exporting active GPIOs, so this is required for proper cleanup
   data->chip.base = -1;
   data->chip.ngpio = 18; /* 18 GPIO pins, PD0, PD1, PD3, PD5, PD6, PD7 (1 - 6);
                             PB0, PB1, PB2, PB3, PB4, PB5 (7 - 12) PC0, PC1, PC2, PC3, PC4, PC5 (13 - 18) */
   data->chip.can_sleep = false;

   data->chip.set = _gpioa_set;
   data->chip.get = _gpioa_get;

   data->chip.direction_input = _direction_input;
   data->chip.direction_output = _direction_output;

   if (gpiochip_add(&data->chip) < 0)
   {
	   printk(KERN_INFO "Failed to add gpio chip");
	   return -ENODEV;
   }

   usb_set_intfdata(interface, data);
   //spin_lock_init(&data->lock);
   //init the board
   //spin_lock(&data->lock);
   ret = usb_control_msg(data->my_dev, usb_sndctrlpipe(data->my_dev, 0),
                         BOARD_INIT, USB_TYPE_VENDOR | USB_DIR_OUT,
                         0, 0,
                         0, 0,
                         1000);
  
   ret = usb_control_msg(data->my_dev,
                         usb_rcvctrlpipe(data->my_dev, 0),
                         BOARD_INIT, USB_TYPE_VENDOR | USB_DIR_IN,
                         0, 0,
                         (u8 *)data->buf,3,
                         1000);
   //spin_unlock(&data->lock);
	gp_d_hd = (gpio_data_handle *)data->buf;
	printk(KERN_INFO "data USB = %d buffer value",gp_d_hd->data );
	printk(KERN_INFO "pin no USB = %d buffer value",gp_d_hd->no );
	printk(KERN_INFO "command USB = %d buffer value",gp_d_hd->command );
   return 0;
}

// disconnect function, called when device is disconnected
static void
my_usb_disconnect(struct usb_interface *interface)
{
   struct my_usb *data;

   data = usb_get_intfdata(interface);

   gpiochip_remove(&data->chip);

   usb_set_intfdata(interface, NULL);

   //deref the count
   usb_put_dev(data->my_dev);

   kfree(data);
}

// defining vendor_id and product_id
static struct usb_device_id my_usb_table[] = {
       { USB_DEVICE(0x16c0, 0x03e8) },
       {},
};

// linking usb device and usb device table
MODULE_DEVICE_TABLE(usb, my_usb_table);

static struct usb_driver my_usb_driver = {
     .name = "GPIO_SUBSYSTEM",
     .id_table = my_usb_table,
     .probe = my_usb_probe,
     .disconnect = my_usb_disconnect,
};

static int __init _usb_init(void)
{
  return usb_register(&my_usb_driver);
}

static void __exit _usb_exit(void)
{
   printk(KERN_INFO "usb driver is unloaded");
   usb_deregister(&my_usb_driver);
}

module_init(_usb_init);
module_exit(_usb_exit);
