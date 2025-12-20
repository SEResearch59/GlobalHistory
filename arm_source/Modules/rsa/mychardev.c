#include <linux/init.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/fs.h>

#define MAX_DEV 2

#define MOD (9973*9903)

#define rdtsc() ({ \
    uint64_t time; \
    asm volatile( \
        "dsb sy\n\t" \
        "isb\n\t" \
        "mrs %0, PMCCNTR_EL0\n\t" \
        "dsb sy" \
        : "=r" (time)        \
        :\
        : "memory");\
    time; \
})

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
static struct class *mychardev_class = NULL;
static struct mychar_device_data mychardev_data[MAX_DEV];

static uint64_t bin_exp(uint64_t a, uint64_t b, int M) {
	uint64_t r0 = 1, tmpb = b, r1 = a;
	int bitLen = -1;
	while(tmpb > 0) {
		bitLen++;
		tmpb >>= 1;
	}
	for (; bitLen >= 0; bitLen --) {
		if (b & (1 << bitLen)) {
			r1 = r0*r1%M;
			r0 = r0*r0%M;	
		} else {
		       	r0 = r0*r1%M;
			r1 = r1*r1%M;
		}
		
	}
	/*for (; bitLen >= 0; bitLen--) {
		if (b & (1 << bitLen)) r0 = r0*r1%M;
		r1 = r1*r1%M;
	}*/
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
    private_key = 0x16f79c857e596a3;
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
    uint8_t databuf[30];
    size_t maxdatalen = 30, ncopied;
	uint64_t cipher = 0;
    int minorId = MINOR(file->f_path.dentry->d_inode->i_rdev);
    int i = 0;

    printk("Reading device: %d\n", minorId);
    // printk("size of uint64: %d\n", sizeof(uint64_t));

    if (count < maxdatalen) {
        maxdatalen = count;
    }
    ncopied = copy_from_user(databuf, buf, maxdatalen);

    if (ncopied != 0) {
        printk("Could't copy %zd bytes from the user\n", ncopied);
    }
    databuf[maxdatalen] = 0;

    // if (count & 1) {
    //     cnt = data[0]*count;
    // }

    // dev_times += data[0];
	for (i = 0; i < count; i++)
		cipher = (uint64_t)databuf[count-i-1]+(cipher*256);

    dev_times+=bin_exp(cipher, mychardev_data[minorId].sec, MOD);
    // uint64_t v = 0;
    // __asm__ volatile (
    //     "ic iallu\n\t"  
    //     "dsb nsh\n\t"   
    //     "isb\n\t"       
    //     : : : "memory"
    // );

    volatile long long rand_num;
    rand_num = rdtsc();
    for (i=0;i<48;i++)
    {
    	rand_num ^= rand_num<<13;
        rand_num ^= rand_num>>17;
        rand_num ^= rand_num<<15;
        asm volatile("" : "+r"(rand_num));
        // if (rand_num % 2) asm volatile("" : "+r"(rand_num));
    }

    // if (count & 1) {
    //     cipher >>= 3;
    //     dev_times += cipher & 0xfe;
    //     dev_times = cipher*dev_times % MOD;
    // }
    return 0;
}

static ssize_t mychardev_write(struct file *file, const char __user *buf, size_t count, loff_t *offset)
{
    size_t maxdatalen = 30, ncopied;
    uint8_t databuf[30];
    int minorId = MINOR(file->f_path.dentry->d_inode->i_rdev);
    int i;
    uint64_t cipher;

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

    for (i = 0; i < count; i++)
	mychardev_data[minorId].sec = (uint64_t)databuf[count-i-1]+(mychardev_data[minorId].sec*256);
    cipher = mychardev_data[minorId].sec;
    /*if (count & 1) {
        cipher >>= 3;
        dev_times += cipher & 0xfe;
        dev_times = cipher*dev_times % MOD;
    }*/
    return count;
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Oleg Kutkov <elenbert@gmail.com>");

module_init(mychardev_init);
module_exit(mychardev_exit);
