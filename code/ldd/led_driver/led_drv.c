#include <linux/module.h>
#include <linux/version.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/gpio.h>
#include <linux/delay.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ZhangMingzhe");
MODULE_DESCRIPTION("the simple led driver for test led status");

static dev_t led_dev_num;
static struct class *my_led_class;
static struct cdev led_dev;
static char cmd_buffer[20];
#define DRIVER_NAME "led_driver"
#define DRIVER_CLASS "my_led_class"

#define USER_LED3 56
#define USER_LED1 54




static ssize_t driver_write(struct file *File, const char* user_buffer, size_t count, loff_t *offs)
{
    
    int not_copied,to_copy,res;
    printk("write were executed!\n");
    to_copy = min(count,sizeof(cmd_buffer));
    not_copied = copy_from_user(cmd_buffer, user_buffer, to_copy);
    res = to_copy - not_copied;
    printk("the command is %d\n",cmd_buffer[0]);
    if (cmd_buffer[0]=='1'){
        gpio_set_value(USER_LED1,1);
        gpio_set_value(USER_LED3,1);
    }
    else{
        gpio_set_value(USER_LED1,0);
        gpio_set_value(USER_LED3,0);
    }
    return res;
}

static int driver_open(struct inode* device_file, struct file *instance)
{
    printk("dev open was called\n");
    return 0;
}

static int driver_close(struct inode* device_file, struct file *instance)
{
    printk("dev close was called\n");
    return 0;
}

static struct file_operations fops =
{
    .owner = THIS_MODULE,
    .open = driver_open,
    .release = driver_close,
    .write = driver_write
};

static int __init ModuleInit(void)
{
    int i;
    printk("init the led driver\n");
    if (alloc_chrdev_region(&led_dev_num, 0, 1, DRIVER_NAME) < 0)
    {
        printk("Device Nr can not be allocated\n");
        return -1;
    }

    printk("Device Nr. Major:%d, Minor:%d was registered\n",led_dev_num>>20, led_dev_num &&0xffff);

    if((my_led_class = class_create(THIS_MODULE, DRIVER_CLASS)) == NULL){
        printk("Device class can not be created!\n");
        goto ClassError;
    }

    if (device_create(my_led_class, NULL, led_dev_num, NULL, DRIVER_NAME) == NULL){
        printk("cannot create device file!\n");
        goto FileError;
    }

    cdev_init(&led_dev,&fops);
    if (cdev_add(&led_dev, led_dev_num, 1) == -1){
        printk("led-driver register of device to kernel failed\n");
        goto AddError;
    }

    printk("led driver - GPIO Init\n");
    if(gpio_request(USER_LED1,"usr led1")){
        printk("usr led1 pin init error\n");
        goto GpioInitError;
    } 
    if(gpio_request(USER_LED3,"usr led3")){
        printk("usr led3 pin init error\n");
        goto GpioInitError;
    } 

    printk("led driver - Set GPIOS to output\n");

    if(gpio_direction_output(USER_LED1,0)){
        printk("USER_LED1 pin direction set error\n");
        goto GpioDirectionError;
    }
    if(gpio_direction_output(USER_LED3,0)){
        printk("USER_LED3 pin direction set error\n");
        goto GpioDirectionError;
    }

    gpio_set_value(USER_LED1,1);
    gpio_set_value(USER_LED3,1);


    return 0;

GpioDirectionError:

GpioInitError:
    gpio_free(USER_LED1);
    gpio_free(USER_LED3);
AddError:
    device_destroy(my_led_class, led_dev_num);
FileError:
    class_destroy(my_led_class);
ClassError:
    unregister_chrdev_region(led_dev_num, 1);
    return -1;
}

static void __exit ModuleExit(void){
    int i;
    gpio_set_value(USER_LED1,0);
    gpio_free(USER_LED1);

    gpio_set_value(USER_LED3,0);
    gpio_free(USER_LED3);

    cdev_del(&led_dev);
    device_destroy(my_led_class,led_dev_num);
    class_destroy(my_led_class);
    unregister_chrdev_region(led_dev_num, 1);
    printk("led driver unload from kernel\n");
}

module_init(ModuleInit);
module_exit(ModuleExit);