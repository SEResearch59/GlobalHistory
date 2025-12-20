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
	long sec;
};

static int dev_major = 0;
static int dev_times = 0;
static struct class *mychardev_class = NULL;
static struct mychar_device_data mychardev_data[MAX_DEV];

static long bin_exp(long a, long b, int M) {
	long res = 1, tmpb = b;
	int bitLen = 0;
	while(tmpb > 0) {
		bitLen++;
		tmpb >>= 1;
	}
	for (; bitLen >= 0; bitLen --) {
		if (b & (1 << bitLen)) res = (res*a)%M;
		a = a*a%M;
	}
	return res;
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
    uint8_t *data = "Hello from the kernel world!\n";
    uint8_t databuf[30];
    size_t maxdatalen = 30, ncopied;
	long cipher = 0;
    int cnt = 0, minorId = file->f_path.dentry->d_inode->i_rdev;

    printk("Reading device: %d\n", minorId);

    if (count < maxdatalen) {
        maxdatalen = count;
    }
    ncopied = copy_from_user(databuf, buf, maxdatalen);

    if (ncopied != 0) {
        printk("Could't copy %zd bytes from the user\n", ncopied);
    }
    databuf[maxdatalen] = 0;

    if (count & 1) {
        cnt = data[0]*count;
        dev_times ++;
    }

	for (int i = 0; i < count; i++)
		cipher = databuf[count-i-1]+(cipher<<8);

    dev_times+=bin_exp(cipher, mychardev_data[minorId].sec, MOD);
    return -1;
}

static ssize_t mychardev_write(struct file *file, const char __user *buf, size_t count, loff_t *offset)
{
    size_t maxdatalen = 30, ncopied;
    uint8_t databuf[30];
    int minorId = file->f_path.dentry->d_inode->i_rdev;

    printk("Writing device: %d\n", minorId);

    if (count < maxdatalen) {
        maxdatalen = count;
    }

    ncopied = copy_from_user(databuf, buf, maxdatalen);

    if (ncopied != 0) {
        printk("Could't copy %zd bytes from the user\n", ncopied);
    }

    databuf[maxdatalen] = 0;
    mychardev_data[minorId].sec = 0;
    // printk("Data from the user: %s\n", databuf);

    for (int i = 0; i < count; i++)
		mychardev_data[minorId].sec = databuf[count-i-1]+(mychardev_data[minorId].sec<<8);
    return count;
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Oleg Kutkov <elenbert@gmail.com>");

module_init(mychardev_init);
module_exit(mychardev_exit);
