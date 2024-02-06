/***************************************************************************//**
*  \file       driver.c
*
*  \details    Simple Linux device driver (sysfs)
*
*  \author     EmbeTronicX
*
*  \Tested with Linux raspberrypi 5.10.27-v7l-embetronicx-custom+
*
*******************************************************************************/
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include<linux/slab.h>                 //kmalloc()
#include<linux/uaccess.h>              //copy_to/from_user()
#include<linux/sysfs.h>
#include<linux/kobject.h>
#include <linux/err.h>
#include <linux/version.h>
#include <linux/semaphore.h>
#include <linux/wait.h>
#include <linux/sched.h>

/**
 *
 * @see https://www.oreilly.com/library/view/linux-device-drivers/0596000081/ch05s02.html
 *
*/
volatile int etx_value = 0;


dev_t dev = 0;
static struct class *dev_class;
static struct cdev etx_cdev;
struct kobject *kobj_ref;

// struct my_data {
//     unsigned int value;
//     struct semaphore sem;
//     wait_queue_head_t wq;
// };

// static struct my_data my_instance = {
//     .value = 33,
//     .sem = __SEMAPHORE_INITIALIZER(my_instance.sem, 0),
//     .wq = __WAIT_QUEUE_HEAD_INITIALIZER(my_instance.wq),
// };

u32 etx_attr_value = 55;
DECLARE_WAIT_QUEUE_HEAD(etx_attr_wq);
atomic_t ext_attr_data_available = ATOMIC_INIT(0);


u32 notify_attribute_value = 22;
// DECLARE_WAIT_QUEUE_HEAD(notify_attribute_wq);
// atomic_t notify_attribute_data_available = ATOMIC_INIT(0);

/*
** Function Prototypes
*/
static int      __init etx_driver_init(void);
static void     __exit etx_driver_exit(void);

/*************** Driver functions **********************/
static int      etx_open(struct inode *inode, struct file *file);
static int      etx_release(struct inode *inode, struct file *file);
static ssize_t  etx_read(struct file *filp,
                        char __user *buf, size_t len,loff_t * off);
static ssize_t  etx_write(struct file *filp,
                         const char *buf, size_t len, loff_t * off);

/*************** Sysfs functions **********************/
static ssize_t  sysfs_show(struct kobject *kobj,
                          struct kobj_attribute *attr, char *buf);
static ssize_t  sysfs_store(struct kobject *kobj,
                           struct kobj_attribute *attr,const char *buf, size_t count);


static ssize_t  notify_attribute_show(struct kobject *kobj,
                          struct kobj_attribute *attr, char *buf);
static ssize_t  notify_attribute_store(struct kobject *kobj,
                           struct kobj_attribute *attr,const char *buf, size_t count);

struct kobj_attribute etx_attr = __ATTR(etx_value, 0660, sysfs_show, sysfs_store);
struct kobj_attribute notify_attribute_attr = __ATTR(notify_attribute, 0660, notify_attribute_show, notify_attribute_store);

/*
** File operation sturcture
*/
static struct file_operations fops =
    {
        .owner          = THIS_MODULE,
        .read           = etx_read,
        .write          = etx_write,
        .open           = etx_open,
        .release        = etx_release,
};

/*
** This function will be called when we read the sysfs file
*/
static ssize_t sysfs_show(struct kobject *kobj,
                          struct kobj_attribute *attr, char *buf)
{
    atomic_inc(&ext_attr_data_available);
    wait_event_interruptible(etx_attr_wq, atomic_read(&ext_attr_data_available) == 0);
    return sprintf(buf, "%u\n", etx_attr_value);
}

/*
** This function will be called when we write the sysfsfs file
*/
static ssize_t sysfs_store(struct kobject *kobj,
                           struct kobj_attribute *attr,const char *buf, size_t count)
{
    sscanf(buf, "%u", &(etx_attr_value));

    atomic_set(&ext_attr_data_available, 0);
    wake_up_interruptible(&etx_attr_wq);

    return count;
}


/*
** This function will be called when we read the sysfs file
*/
static ssize_t notify_attribute_show(struct kobject *kobj,
                                     struct kobj_attribute *attr, char *buf)
{
    return sprintf(buf, "%u\n", notify_attribute_value);
}

/*
** This function will be called when we write the sysfsfs file
*/
static ssize_t notify_attribute_store(struct kobject *kobj,
                                      struct kobj_attribute *attr,const char *buf, size_t count)
{
    sscanf(buf, "%u", &notify_attribute_value);

    // Notify the sysfs attribute that the task has completed
    sysfs_notify(kobj, NULL, "notify_attribute");

    return count;
}


/*
** This function will be called when we open the Device file
*/
static int etx_open(struct inode *inode, struct file *file)
{
    pr_info("Device File Opened...!!!\n");
    return 0;
}

/*
** This function will be called when we close the Device file
*/
static int etx_release(struct inode *inode, struct file *file)
{
    pr_info("Device File Closed...!!!\n");
    return 0;
}

/*
** This function will be called when we read the Device file
*/
static ssize_t etx_read(struct file *filp,
                        char __user *buf, size_t len, loff_t *off)
{
    pr_info("Read function\n");
    return 0;
}

/*
** This function will be called when we write the Device file
*/
static ssize_t etx_write(struct file *filp,
                         const char __user *buf, size_t len, loff_t *off)
{
    pr_info("Write Function\n");
    return len;
}

/*
** Module Init function
*/
static int __init etx_driver_init(void)
{

    /*Allocating Major number*/
    if((alloc_chrdev_region(&dev, 0, 1, "etx_Dev")) <0){
        pr_info("Cannot allocate major number\n");
        return -1;
    }
    pr_info("Major = %d Minor = %d \n",MAJOR(dev), MINOR(dev));

    /*Creating cdev structure*/
    cdev_init(&etx_cdev,&fops);

    /*Adding character device to the system*/
    if((cdev_add(&etx_cdev,dev,1)) < 0){
        pr_info("Cannot add the device to the system\n");
        goto r_class;
    }

    /*Creating struct class*/
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 4, 0)
    if(IS_ERR(dev_class = class_create(THIS_MODULE,"etx_class"))){
        pr_info("Cannot create the struct class\n");
        goto r_class;
    }
#else
    if(IS_ERR(dev_class = class_create("etx_class"))){
        pr_info("Cannot create the struct class\n");
        goto r_class;
    }
#endif

    /*Creating device*/
    if(IS_ERR(device_create(dev_class,NULL,dev,NULL,"etx_device"))){
        pr_info("Cannot create the Device 1\n");
        goto r_device;
    }

    /*Creating a directory in /sys/kernel/ */
    kobj_ref = kobject_create_and_add("etx_sysfs",kernel_kobj);

    /*Creating sysfs file for etx_value*/
    if(sysfs_create_file(kobj_ref,&etx_attr.attr)){
        pr_err("Cannot create sysfs file......\n");
        goto r_sysfs;
    }

    // Creating the notify_attribute_attr attribute
    if(sysfs_create_file(kobj_ref,&notify_attribute_attr.attr)){
        pr_err("Cannot create sysfs file......\n");
        goto r_sysfs;
    }

    pr_info("Device Driver Insert...Done!!!\n");
    return 0;

r_sysfs:
    kobject_put(kobj_ref);
    sysfs_remove_file(kernel_kobj, &etx_attr.attr);
    sysfs_remove_file(kernel_kobj, &notify_attribute_attr.attr);

r_device:
    class_destroy(dev_class);
r_class:
    unregister_chrdev_region(dev,1);
    cdev_del(&etx_cdev);
    return -1;
}

/*
** Module exit function
*/
static void __exit etx_driver_exit(void)
{
    kobject_put(kobj_ref);
    sysfs_remove_file(kernel_kobj, &etx_attr.attr);
    sysfs_remove_file(kernel_kobj, &notify_attribute_attr.attr);
    device_destroy(dev_class,dev);
    class_destroy(dev_class);
    cdev_del(&etx_cdev);
    unregister_chrdev_region(dev, 1);
    pr_info("Device Driver Remove...Done!!!\n");
}

module_init(etx_driver_init);
module_exit(etx_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("EmbeTronicX <embetronicx@gmail.com>");
MODULE_DESCRIPTION("Simple Linux device driver (sysfs)");
MODULE_VERSION("1.8");
