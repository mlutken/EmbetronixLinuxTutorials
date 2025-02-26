#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <asm/uaccess.h>

/**
 *  @see https://blog.sourcerer.io/writing-a-simple-linux-kernel-module-d9dc3762c234
 *  @see Find major number: $ cat /proc/devices | grep mykernelmodule
 *      https://unix.stackexchange.com/questions/730659/is-there-a-way-to-determine-which-module-corresponds-to-which-major-number
 *
 *
 * @see
*/

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Robert W. Oliver II");
MODULE_DESCRIPTION("A simple example Linux module.");
MODULE_VERSION("0.01");

#define DEVICE_NAME "mykernelmodule"
#define EXAMPLE_MSG "Hello, World!\n"
#define MSG_BUFFER_LEN 15

/* Prototypes for device functions */
static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);


static int major_num;
static int device_open_count = 0;
static char msg_buffer[MSG_BUFFER_LEN];
static char *msg_ptr;

/* This structure points to all of the device functions */
static struct file_operations file_ops = {
    .read = device_read,
    .write = device_write,
    .open = device_open,
    .release = device_release
};

/* When a process reads from our device, this gets called. */
static ssize_t device_read(struct file *flip, char *buffer, size_t len, loff_t *offset) {
    int bytes_read = 0;
    /* If we’re at the end, loop back to the beginning */
    if (*msg_ptr == 0) {
        msg_ptr = msg_buffer;
    }
    /* Put data in the buffer */
    while (len && *msg_ptr) {
        /* Buffer is in user data, not kernel, so you can’t just reference
            with a pointer. The function put_user handles this for us */
        put_user(*(msg_ptr++), buffer++);
        len--;
        bytes_read++;
    }
    printk(KERN_INFO "mykernelmodul::device_read(), len: %lu,  offset: %lld", len, *offset);

    return bytes_read;
}

/* Called when a process tries to write to our device */
static ssize_t device_write(struct file *flip, const char *buffer, size_t len, loff_t *offset) {
    /* This is a read-only device */
    printk(KERN_ALERT "This operation is not supported.\n");
    return -EINVAL;
}

/* Called when a process opens our device */
static int device_open(struct inode *inode, struct file *file) {
    /* If device is open, return busy */
    printk(KERN_INFO "mykernelmodul::device_open(), device_open_count: %d ", device_open_count);
    if (device_open_count) {
        return -EBUSY;
    }
    device_open_count++;
    try_module_get(THIS_MODULE);
    return 0;
}

/* Called when a process closes our device */
static int device_release(struct inode *inode, struct file *file) {
    /* Decrement the open counter and usage count. Without this, the module would not unload. */
    printk(KERN_INFO "mykernelmodul::device_release(), device_open_count: %d ", device_open_count);
    device_open_count--;
    module_put(THIS_MODULE);
    return 0;
}

static int __init mykernelmodule_init(void) {
    /* Fill buffer with our message */
    strncpy(msg_buffer, EXAMPLE_MSG, MSG_BUFFER_LEN);
    /* Set the msg_ptr to the buffer */
    msg_ptr = msg_buffer;
    /* Try to register character device */
    major_num = register_chrdev(0, DEVICE_NAME, &file_ops);
    if (major_num < 0) {
        printk(KERN_ALERT "Could not register device: %d\n", major_num);
        return major_num;
    } else {
        printk(KERN_INFO "mykernelmodule module loaded with device major number %d\n", major_num);
        return 0;
    }
}
static void __exit mykernelmodule_exit(void) {
    /* Remember — we have to clean up after ourselves. Unregister the character device. */
    unregister_chrdev(major_num, DEVICE_NAME);
    printk(KERN_INFO "Goodbye, World!\n");
}
/* Register module functions */
module_init(mykernelmodule_init);
module_exit(mykernelmodule_exit);
