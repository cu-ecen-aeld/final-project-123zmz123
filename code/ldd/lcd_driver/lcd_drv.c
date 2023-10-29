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
MODULE_DESCRIPTION("the lcd driver for lcd1602");

static dev_t lcd_dev_num;
static struct class *my_lcd_class;
static struct cdev lcd_dev;

#define DRIVER_NAME "lcd_driver"
#define DRIVER_CLASS "my_lcd_class"

static char lcd_buffer[17];

#define ENABLE_PIN 30 // 11
#define CMD_DATA_SEL_PIN 31 // 13
#define RES_SEL_PIN 31
#define RW_PIN 48 // 15

#define DATA0 60 // 12
#define DATA1 3  // 21
#define DATA2 51 // 16
#define DATA3 4  // 18
#define DATA4 2  // 22
#define DATA5 15 // 24
#define DATA6 14 // 26
#define DATA7 7  // 42

void lcd_enable(void)
{
    gpio_set_value(ENABLE_PIN,1);
    msleep(5);
    gpio_set_value(ENABLE_PIN,0);
}

void lcd_send_byte(char data)
{
    gpio_set_value(DATA0, ((data) & (1 << 0)) >> 0);
    gpio_set_value(DATA1, ((data) & (1 << 1)) >> 1);
    gpio_set_value(DATA2, ((data) & (1 << 2)) >> 2);
    gpio_set_value(DATA3, ((data) & (1 << 3)) >> 3);
    gpio_set_value(DATA4, ((data) & (1 << 4)) >> 4);
    gpio_set_value(DATA5, ((data) & (1 << 5)) >> 5);
    gpio_set_value(DATA6, ((data) & (1 << 6)) >> 6);
    gpio_set_value(DATA7, ((data) & (1 << 7)) >> 7);
    lcd_enable();
    msleep(5);
}

void lcd_send_command(uint8_t data)
{
    gpio_set_value(CMD_DATA_SEL_PIN,0);/*RS pin set as command*/
    lcd_send_byte(data);
}

void lcd_send_data(uint8_t data)
{
    gpio_set_value(CMD_DATA_SEL_PIN, 1); /*RS pin set as data*/
    lcd_send_byte(data);
}

void lcd_w_set(void)
{
    gpio_set_value(RW_PIN,0);
}
void lcd_r_set(void)
{
    gpio_set_value(RW_PIN,1);
}

static ssize_t driver_write(struct file *File, const char* user_buffer, size_t count, loff_t *offs)
{
    int to_copy, not_copied, delta, i;
    to_copy = min(count, sizeof(lcd_buffer));
    not_copied = copy_from_user(lcd_buffer, user_buffer, to_copy);
    delta = to_copy - not_copied;
    lcd_send_command(0x01);
    for(i=0;i<to_copy;i++)
        lcd_send_data(lcd_buffer[i]);
    return delta;
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
    printk("init the lcd driver\n");
    if (alloc_chrdev_region(&lcd_dev_num, 0, 1, DRIVER_NAME) < 0)
    {
        printk("Device Nr can not be allocated\n");
        return -1;
    }

    printk("Device Nr. Major:%d, Minor:%d was registered\n",lcd_dev_num>>20, lcd_dev_num &&0xffff);

    if((my_lcd_class = class_create(THIS_MODULE, DRIVER_CLASS)) == NULL){
        printk("Device class can not be created!\n");
        goto ClassError;
    }

    if (device_create(my_lcd_class, NULL, lcd_dev_num, NULL, DRIVER_NAME) == NULL){
        printk("cannot create device file!\n");
        goto FileError;
    }

    cdev_init(&lcd_dev,&fops);
    if (cdev_add(&lcd_dev, lcd_dev_num, 1) == -1){
        printk("lcd-driver register of device to kernel failed\n");
        goto AddError;
    }

    printk("lcd driver - GPIO Init\n");
    if(gpio_request(ENABLE_PIN,"enable pin")){
        printk("Enable pin init error\n");
        goto GpioInitError;
    } 
    if(gpio_request(CMD_DATA_SEL_PIN,"cmd data pin")){
        printk("cmd-data-req pin init error\n");
        goto GpioInitError;
    } 
    if(gpio_request(RW_PIN,"r/w pin")){
        printk("r/w pin init error\n");
        goto GpioInitError;
    }
    if(gpio_request(DATA0,"data0 pin")){
        printk("data0 pin init error\n");
        goto GpioInitError;
    }
    if(gpio_request(DATA1,"data1 pin")){
        printk("data1 pin init error\n");
        goto GpioInitError;
    } 
    if(gpio_request(DATA2,"data2 pin")){
        printk("data2 pin init error\n");
        goto GpioInitError;
    }
    if(gpio_request(DATA3,"data3 pin")){
        printk("data3 pin init error\n");
        goto GpioInitError;
    } 
    if(gpio_request(DATA4,"data4 pin")) {
        printk("data4 pin init error\n");
        goto GpioInitError;
    }
    if(gpio_request(DATA5,"data5 pin")){
        printk("data5 pin init error\n");
        goto GpioInitError;
    } 
    if(gpio_request(DATA6,"data6 pin")){
        printk("data6 pin init error\n");
        goto GpioInitError;
    } 
    if(gpio_request(DATA7,"data7 pin")){
        printk("data7 pin init error\n");
        goto GpioInitError;
    } 


    printk("lcd driver - Set GPIOS to output\n");

    if(gpio_direction_output(ENABLE_PIN,0)){
        printk("enable pin direction set error\n");
        goto GpioDirectionError;
    }
    if(gpio_direction_output(CMD_DATA_SEL_PIN,0)){
        printk("command-data-selection pin direction set error\n");
        goto GpioDirectionError;
    }
    if(gpio_direction_output(RW_PIN,0)){
        printk("r/w-selection pin direction set error\n");
        goto GpioDirectionError;
    }
    if(gpio_direction_output(DATA0,0)){
        printk("data0 pin direction set error\n");
        goto GpioDirectionError;
    }
    if(gpio_direction_output(DATA1,0)){
        printk("data1 pin direction set error\n");
        goto GpioDirectionError;
    }
    if(gpio_direction_output(DATA2,0)){
        printk("data2 pin direction set error\n");
        goto GpioDirectionError;
    }
    if(gpio_direction_output(DATA3,0)){
        printk("data3 pin direction set error\n");
        goto GpioDirectionError;
    }
    if(gpio_direction_output(DATA4,0)){
        printk("data4 pin direction set error\n");
        goto GpioDirectionError;
    }
    if(gpio_direction_output(DATA5,0)){
        printk("data5 pin direction set error\n");
        goto GpioDirectionError;
    }
    if(gpio_direction_output(DATA6,0)){
        printk("data6 pin direction set error\n");
        goto GpioDirectionError;
    }
    if(gpio_direction_output(DATA7,0)){
        printk("data7 pin direction set error\n");
        goto GpioDirectionError;
    }

    lcd_send_command(0x30); /*set the display mode for 8 bit communication*/
    lcd_send_command(0xf); /*turn on*/
    lcd_send_command(0x01);/*clear the display*/

    char content[] = "Hello World!";
    for(i=0;i<sizeof(content)-1;i++)
        lcd_send_data(content[i]);
    
    // gpio_set_value(ENABLE_PIN,1);
    //gpio_set_value(CMD_DATA_SEL_PIN,0);
    //gpio_set_value(RW_PIN,0);

    //gpio_set_value(DATA0,0);
    //gpio_set_value(DATA1,0);
    //  gpio_set_value(DATA2,0);
    //  gpio_set_value(DATA3,0);
    // gpio_set_value(DATA4,0);
    // gpio_set_value(DATA5,0);
    //gpio_set_value(DATA6,0);
    //gpio_set_value(DATA7,0);


    return 0;

GpioDirectionError:
GpioInitError:
    gpio_free(ENABLE_PIN);
    gpio_free(RW_PIN);
    gpio_free(CMD_DATA_SEL_PIN);
    gpio_free(DATA0);
    gpio_free(DATA1);
    gpio_free(DATA2);
    gpio_free(DATA3);
    gpio_free(DATA4);
    gpio_free(DATA5);
    gpio_free(DATA6);
    gpio_free(DATA7);
AddError:
    device_destroy(my_lcd_class, lcd_dev_num);
FileError:
    class_destroy(my_lcd_class);
ClassError:
    unregister_chrdev_region(lcd_dev_num, 1);
    return -1;
}

static void __exit ModuleExit(void){
    int i;
    lcd_send_command(0x01);/*clear display*/
    gpio_set_value(ENABLE_PIN,0);
    gpio_free(ENABLE_PIN);

    gpio_set_value(CMD_DATA_SEL_PIN,0);
    gpio_free(CMD_DATA_SEL_PIN);

    gpio_set_value(RW_PIN,0);
    gpio_free(RW_PIN);

    gpio_set_value(DATA0,0);
    gpio_free(DATA0);
    gpio_set_value(DATA1,0);
    gpio_free(DATA1);
    gpio_set_value(DATA2,0);
    gpio_free(DATA2);
    gpio_set_value(DATA3,0);
    gpio_free(DATA3);
    gpio_set_value(DATA4,0);
    gpio_free(DATA4);
    gpio_set_value(DATA5,0);
    gpio_free(DATA5);
    gpio_set_value(DATA6,0);
    gpio_free(DATA6);
    gpio_set_value(DATA7,0);
    gpio_free(DATA7);

    cdev_del(&lcd_dev);
    device_destroy(my_lcd_class,lcd_dev_num);
    class_destroy(my_lcd_class);
    unregister_chrdev_region(lcd_dev_num, 1);
    printk("lcd driver unload from kernel\n");
}

module_init(ModuleInit);
module_exit(ModuleExit);