#include <linux/init.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/fs.h>

#define MAX_DEV 2

#define MOD 9973*9903

static int mychardev_open(struct inode *inode, struct file *file);
static int mychardev_release(struct inode *inode, struct file *file);
static long mychardev_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
static ssize_t mychardev_read(struct file *file, char __user *buf, size_t count, loff_t *offset);
static ssize_t mychardev_write(struct file *file, const char __user *buf, size_t count, loff_t *offset);

static const struct file_operations mychardev_fops = {
    .owner      = THIS_MODULE,
    .open       = mychardev_open,
    .release    = mychardev_release,
    .unlocked_ioctl = mychardev_ioctl,
    .read       = mychardev_read,
    .write       = mychardev_write
};

struct mychar_device_data {
    struct cdev cdev;
	long long sec;
};

static int dev_major = 0;
static long long dev_times = 0;
static struct class *mychardev_class = NULL;
static struct mychar_device_data mychardev_data[MAX_DEV];

static long long compute_wNAF(const long long a, int w, size_t *ret_len) {
    int window_val;
    int sign = 1;
    int bit, next_bit, mask;
    size_t len=64, j;
    long long r = 0; // useless here

    bit = 1 << w;               /* at most 128 */
    next_bit = bit << 1;        /* at most 256 */
    mask = next_bit - 1;        /* at most 255 */

    window_val = a & mask;
    j = 0;
    while ((window_val != 0) || (j + w + 1 < len)) {
        int digit = 0;
        /* 0 <= window_val <= 2^(w+1) */
        if (window_val & 1) {
            /* 0 < window_val < 2^(w+1) */
            if (window_val & bit) {
                // digit = j + w + 1 >= len ? window_val & (mask >> 1) : window_val - next_bit; 

                digit = window_val - next_bit; /* -2^w < digit < 0 */
                if (j + w + 1 >= len)
                    digit = window_val & (mask >> 1); /* 0 < digit < 2^w */
                // else
            } else
                digit = window_val; /* 0 < digit < 2^w */
            window_val -= digit;
        }
        r += sign * digit;
        ++j;
        window_val >>= 1;
        window_val += bit * ((a >> (j + w)) & 1);

    }

    *ret_len = j;
    return r;
}

static int mychardev_uevent(struct device *dev, struct kobj_uevent_env *env)
{
    add_uevent_var(env, "DEVMODE=%#o", 0666);
    return 0;
}

static int __init mychardev_init(void)
{
    int err, i;
    dev_t dev;
    long long private_key;

    err = alloc_chrdev_region(&dev, 0, MAX_DEV, "mychardev");

    dev_major = MAJOR(dev);

    mychardev_class = class_create(THIS_MODULE, "mychardev");
    mychardev_class->dev_uevent = mychardev_uevent;

    for (i = 0; i < MAX_DEV; i++) {
        cdev_init(&mychardev_data[i].cdev, &mychardev_fops);
        mychardev_data[i].cdev.owner = THIS_MODULE;
		mychardev_data[i].sec = 0;
        cdev_add(&mychardev_data[i].cdev, MKDEV(dev_major, i), 1);

        device_create(mychardev_class, NULL, MKDEV(dev_major, i), NULL, "mychardev-%d", i);
    }
    private_key = 0xE973e596a473224f;
    mychardev_data[1].sec = private_key;

    return 0;
}

static void __exit mychardev_exit(void)
{
    int i;

    for (i = 0; i < MAX_DEV; i++) {
        device_destroy(mychardev_class, MKDEV(dev_major, i));
    }

    class_unregister(mychardev_class);
    class_destroy(mychardev_class);

    unregister_chrdev_region(MKDEV(dev_major, 0), MINORMASK);
}

static int mychardev_open(struct inode *inode, struct file *file)
{
    printk("MYCHARDEV: Device open\n");
    return 0;
}

static int mychardev_release(struct inode *inode, struct file *file)
{
    printk("MYCHARDEV: Device close\n");
    return 0;
}

static long mychardev_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    printk("MYCHARDEV: Device ioctl\n");
    return 0;
}

static ssize_t mychardev_read(struct file *file, char __user *buf, size_t count, loff_t *offset)
{
    // uint8_t *data = "Hello from the kernel world!\n";
    uint8_t databuf[30];
    size_t maxdatalen = 30, ncopied, ret_len;
	long long cipher = 0;
    int minorId = MINOR(file->f_path.dentry->d_inode->i_rdev), i;

    // printk("Reading device: %d\n", minorId);

    if (count < maxdatalen) {
        maxdatalen = count;
    }
    ncopied = copy_from_user(databuf, buf, maxdatalen);

    // if (ncopied != 0) {
        // printk("Could't copy %zd bytes from the user\n", ncopied);
    // }
    databuf[maxdatalen] = 0;

    // if (count & 1) {
    //     cnt = data[0]*count;
    //     dev_times ++;
    // }
	for (i = 0; i < count; i++)
		cipher = databuf[count-i-1]+(cipher<<8);
    // if (count > 5) count = 1;
    dev_times += compute_wNAF(mychardev_data[minorId].sec, 3, &ret_len);
    dev_times += ret_len;
    // if (count & 1) {
    //     cipher >>= 3;
    //     dev_times += cipher & 0x;
    //     dev_times = cipher*dev_times % MOD;
    // }
    return count;
}

static ssize_t mychardev_write(struct file *file, const char __user *buf, size_t count, loff_t *offset)
{
    size_t maxdatalen = 30, ncopied;
    uint8_t databuf[30];
    int minorId = MINOR(file->f_path.dentry->d_inode->i_rdev), i;

    // printk("Writing device: %d\n", minorId);

    if (count < maxdatalen) {
        maxdatalen = count;
    }

    ncopied = copy_from_user(databuf, buf, maxdatalen);

    // if (ncopied != 0) {
    //     printk("Could't copy %zd bytes from the user\n", ncopied);
    // }

    databuf[maxdatalen] = 0;
    mychardev_data[minorId].sec = 0;
    // printk("Data from the user: %s\n", databuf);

    for (i = 0; i < count; i++)
		mychardev_data[minorId].sec = 1LL*((mychardev_data[minorId].sec<<8)+databuf[count-i-1]);
    printk("secret written: %llx\n", mychardev_data[minorId].sec);
    return count;
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Oleg Kutkov <elenbert@gmail.com>");

module_init(mychardev_init);
module_exit(mychardev_exit);
