#include<linux/module.h>
#include<linux/fs.h>
#include<linux/cdev.h>
#include<linux/device.h>
#include<linux/kdev_t.h>
#include<linux/uaccess.h>

#undef pr_fmt
#define pr_fmt(fmt) "%s: "fmt,__func__

#define NUM_OF_DEVICES 4

#define MEM_SIZE_MAX_DEV1 1024
#define MEM_SIZE_MAX_DEV2 512
#define MEM_SIZE_MAX_DEV3 1024
#define MEM_SIZE_MAX_DEV4 512


#define RDONLY 0x01
#define WRONLY 0x10
#define RDWR   0x11

char device_buffer_dev1[MEM_SIZE_MAX_DEV1];
char device_buffer_dev2[MEM_SIZE_MAX_DEV2];
char device_buffer_dev3[MEM_SIZE_MAX_DEV3];
char device_buffer_dev4[MEM_SIZE_MAX_DEV4];

struct cdev_private_data{
    char *buffer;
    unsigned size;
    const char *serial_number;
    int perm;
    struct cdev cdev;
};


struct driver_private_data{
    int total_devices;
    dev_t device_number;
    struct class *class_dev;
    struct device *device;
    struct cdev_private_data cdev_data[NUM_OF_DEVICES];
};


struct driver_private_data drv_data = {
    .total_devices = NUM_OF_DEVICES,
    .cdev_data = {
        [0] = {
            .buffer = device_buffer_dev1,
            .size = MEM_SIZE_MAX_DEV1,
            .serial_number = "vividev1",
            .perm = RDONLY
        },

        [1] = {
            .buffer = device_buffer_dev2,
            .size = MEM_SIZE_MAX_DEV2,
            .serial_number = "vividev2",
            .perm = WRONLY
        },

        [2] = {
            .buffer = device_buffer_dev3,
            .size = MEM_SIZE_MAX_DEV3,
            .serial_number = "vividev3",
            .perm = RDWR
        },

        [3] = {
            .buffer = device_buffer_dev4,
            .size = MEM_SIZE_MAX_DEV4,
            .serial_number = "vividev4",
            .perm = RDWR
        },
    }
};

loff_t drv_lseek(struct file *filp, loff_t offset, int whence)
{
    struct cdev_private_data *cdev_data = (struct cdev_private_data *)filp->private_data;
    int max_size = cdev_data->size;

    loff_t temp;
    pr_info("lseek requested\n");
    pr_info("cur value of file pos is:%lld\n", filp->f_pos);

    switch (whence)
    {
    case SEEK_SET:
        if ((offset > max_size) || (offset < 0))
            return -EINVAL;
        filp->f_pos = offset;
        break;

    case SEEK_CUR:
        temp = filp->f_pos + offset;
        if ((temp > max_size) || (temp < 0))
            return -EINVAL;
        filp->f_pos = temp;
        break;

    case SEEK_END:
        temp = max_size + offset;
        if ((temp > max_size) || (temp < 0))
            return -EINVAL;
        filp->f_pos = temp;
        break;
    default:
        return -EINVAL;
    }

    pr_info("the new value of file pos is %lld\n", filp->f_pos);
    return filp->f_pos;
}

ssize_t drv_read(struct file *filp, char __user *buff, size_t count, loff_t *f_pos){

    struct cdev_private_data *cdev_data = (struct cdev_private_data *)filp->f_pos;
    int max_size = cdev_data->size;

    pr_info("request for read for <%zu> bytes \n", count);
    pr_info("current file pos is <%lld>\n", *f_pos);

    /* remodify the count */
    if ((*f_pos + count) > max_size)
        count = max_size - *f_pos;

    /*copy the cdev.buffer to user memory*/
    if (copy_to_user(buff, cdev_data->buffer + (*f_pos), count))
    {
        return -EFAULT;
    }

    *f_pos += count;

    pr_info("<%zu> bytes were read\n", count);
    pr_info("new file pos is %lld\n", *f_pos);

    return count;
}

ssize_t drv_write(struct file *filp, const char __user *buff, size_t count, loff_t *f_pos){
    struct cdev_private_data *cdev_data = (struct cdev_private_data *)filp->private_data;


    int max_size = cdev_data->size;

    pr_info("write requsest for <%zu> bytes\n", count);
    pr_info("current file pos is <%lld>\n",*f_pos);

    /*remodify the write count*/
    if((*f_pos+count)>max_size)
        count = max_size - *f_pos;
    
    if(!count){
        pr_err("there are no space for mem\n");
        return -ENOMEM;
    }

    if (copy_from_user(cdev_data->buffer + (*f_pos), buff, count))
    {
        return -EFAULT;
    }

    *f_pos += count;
    pr_info("<%zu> bytes were  written\n", count);
    pr_info("new file pos were <%lld>\n",*f_pos);

    return count;
}

int check_permission(int dev_perm, int acc_mode){
    if (dev_perm == RDWR)
        return 0;
    
    if((dev_perm == RDONLY) && ((acc_mode & FMODE_READ) && !(acc_mode & FMODE_WRITE)))
        return 0;

    if((dev_perm == WRONLY) && ((acc_mode & FMODE_WRITE) && !(acc_mode & FMODE_READ)))
        return 0;

    return -EPERM;
}

int drv_open(struct inode *inode, struct file *filp){
    int ret;
    int minor;
    pr_info("execute open\n");
    struct cdev_private_data *cdev_data;

    minor = MINOR(inode->i_rdev);
    pr_info("minor access =%d\n", minor);

    cdev_data = container_of(inode->i_cdev, struct cdev_private_data, cdev);

    filp->private_data = cdev_data;

    ret = check_permission(cdev_data->perm, filp->f_mode);
    if(!ret)
        pr_info("open was successful!\n");
    else
        pr_info("open failed\n");
    
    return ret;
}

int drv_release(struct inode *inode, struct file *filp){
    pr_info("release was successful\n");
    return 0;
}

struct file_operations drv_fops = {
    .open = drv_open,
    .release = drv_release,
    .read = drv_read,
    .write = drv_write,
    .llseek = drv_lseek,
    .owner = THIS_MODULE
};


static int __init mod_driver_init(void){
    int ret;
    int i;


    /*Dynamically allocate  device numbers */
    ret = alloc_chrdev_region(&drv_data.device_number,0,NUM_OF_DEVICES,"psdfrm drivers");
    if (ret < 0){
        pr_err("alloc dev number failed\n");
        goto out;
    }

    /*create device class under /sys/class/ */
    drv_data.class_dev = class_create(THIS_MODULE,"psdfrm_driver_class");
    if(IS_ERR(drv_data.class_dev)){
        pr_err("psd driver class creation failed\n");
        ret = PTR_ERR(drv_data.class_dev);
        goto unreg_chrdev;
    }

    for (i=0;i<NUM_OF_DEVICES;i++){
        pr_info("Device number Major:Minor is = %d:%d\n",MAJOR(drv_data.device_number+i),MINOR(drv_data.device_number+i));

        cdev_init(&drv_data.cdev_data[i].cdev, &drv_fops);

        drv_data.cdev_data[i].cdev.owner = THIS_MODULE;
        ret = cdev_add(&drv_data.cdev_data[i].cdev, drv_data.device_number + i, 1);
        if (ret < 0){
            pr_err("cdev add failed\n");
            goto cdev_del;
        }

        drv_data.device = device_create(drv_data.class_dev, NULL, drv_data.device_number + i, NULL, "cdev-%d", i + 1);
        if(IS_ERR(drv_data.device)){
            pr_err("device create failed\n");
            ret = PTR_ERR(drv_data.device);
            goto class_del;
        }
    }

    pr_info("Module init were successfully!\n");

    return 0;
cdev_del:
class_del:
    for(;i>=0;i--){
        device_destroy(drv_data.class_dev,drv_data.device_number+i);
        cdev_del(&drv_data.cdev_data[i].cdev);
    }
    class_destroy(drv_data.class_dev);
unreg_chrdev:
    unregister_chrdev_region(drv_data.device_number,NUM_OF_DEVICES);
out:
    pr_info("Module insert failed\n");
    return ret;
}

static void __exit mod_driver_cleanup(void)
{
    int i;
    for (i = 0; i < NUM_OF_DEVICES; i++)
    {
        device_destroy(drv_data.class_dev, drv_data.device_number + i);
        cdev_del(&drv_data.cdev_data[i].cdev);
    }
    class_destroy(drv_data.class_dev);

    unregister_chrdev_region(drv_data.device_number, NUM_OF_DEVICES);
    pr_info("module unloaded\n");
}

module_init(mod_driver_init);
module_exit(mod_driver_cleanup);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Zhang Mingzhe");
MODULE_DESCRIPTION("A framework for hardware driver");