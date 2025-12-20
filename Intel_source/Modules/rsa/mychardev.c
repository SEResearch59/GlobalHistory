#include <linux/init.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <asm/processor.h>
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
	uint64_t sec;
};

static int dev_major = 0;
static int dev_times = 0;
//static int bitLen = 0;
static struct class *mychardev_class = NULL;
static struct mychar_device_data mychardev_data[MAX_DEV];

static uint64_t __attribute__((optimize("O0"))) bin_exp(uint64_t a, uint64_t b, int M) {
	 uint64_t r0 = 1, tmpb = b, r1 = a, mask;
	 int bitLen = -1;
	 while(tmpb > 0) {
	        bitLen++;
	 	tmpb >>= 1;
	 }
	 mask = 1ULL << bitLen;
	  for (; mask > 0; mask >>= 1) {
	 	if (b & mask) {
	 		r1 = r0*r1%M;
	 		r0 = r0*r0%M;	
	 	} 
	 	else {
	 	       r0 = r0*r1%M;
	 		r1 = r1*r1%M;
	 	}
		
	 }
	 return r0;
    
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
    uint64_t private_key;

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
    private_key = 0x5555587ac;
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
    //printk("MYCHARDEV: Device open\n");
    return 0;
}

static int mychardev_release(struct inode *inode, struct file *file)
{
    //printk("MYCHARDEV: Device close\n");
    return 0;
}

static long mychardev_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    //printk("MYCHARDEV: Device ioctl\n");
    return 0;
}

static ssize_t __attribute__((optimize("O1"))) mychardev_read(struct file *file, char __user *buf, size_t count, loff_t *offset)
{
    uint8_t *data = "Hello from the kernel world!\n";
    uint8_t databuf[60];
    size_t maxdatalen = 30, ncopied;
	uint64_t cipher = 0;
    int minorId = MINOR(file->f_path.dentry->d_inode->i_rdev);
    int i =0;

    if (count < maxdatalen) {
        maxdatalen = count;
    }
    
    ncopied = copy_from_user(databuf, buf, maxdatalen);

    databuf[maxdatalen] = 0;


    dev_times += data[0];

	for (i = 0; i < count; i++)
		cipher = (uint64_t)databuf[count-i-1]+(cipher*256);

    dev_times+=bin_exp(cipher, mychardev_data[minorId].sec, MOD);
  
    return count;
}

static ssize_t mychardev_write(struct file *file, const char __user *buf, size_t count, loff_t *offset)
{
    size_t maxdatalen = 30, ncopied;
    uint8_t databuf[30];
    int minorId = MINOR(file->f_path.dentry->d_inode->i_rdev);
    int i =0;
    //printk("Writing device: %d\n", minorId);

    if (count < maxdatalen) {
        maxdatalen = count;
    }

    ncopied = copy_from_user(databuf, buf, maxdatalen);

    if (ncopied != 0) {
        //printk("Could't copy %zd bytes from the user\n", ncopied);
    }

    databuf[maxdatalen] = 0;
    mychardev_data[minorId].sec = 0;
    // printk("Data from the user: %s\n", databuf);
    for (i = 0; i < count; i++)
		mychardev_data[minorId].sec = (uint64_t)databuf[count-i-1]+(mychardev_data[minorId].sec*256);
    return count;
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Oleg Kutkov <elenbert@gmail.com>");

module_init(mychardev_init);
module_exit(mychardev_exit);
