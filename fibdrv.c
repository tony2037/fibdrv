#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kdev_t.h>
#include <linux/kernel.h>
#include <linux/limits.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/slab.h>


MODULE_LICENSE("Dual MIT/GPL");
MODULE_AUTHOR("National Cheng Kung University, Taiwan");
MODULE_DESCRIPTION("Fibonacci engine driver");
MODULE_VERSION("0.1");

#define DEV_FIBONACCI_NAME "fibonacci"

/* MAX_LENGTH is set to 92 because
 * ssize_t can't fit the number > 92
 */
#define MAX_LENGTH 100

static dev_t fib_dev = 0;
static struct cdev *fib_cdev;
static struct class *fib_class;
static DEFINE_MUTEX(fib_mutex);

static unsigned long long *subtractor(unsigned long long *k1,
                                      unsigned long long *k2)
{
    /* Assume k1 >= k2, return positive, or NULL as fail */
    if (k1 == NULL || k2 == NULL)
        return NULL;
    if (k1[1] < k2[1])
        return NULL;
    if ((k1[1] == k2[1]) && (k1[0] < k2[0]))
        return NULL;
    unsigned long long *r = kmalloc(2 * sizeof(unsigned long long), GFP_KERNEL);
    if (r == NULL) {
        printk("kmalloc error");
        return NULL;
    }
    if (k1[0] < k2[0]) {
        /* Borrow */
        k1[1] -= 1;
        r[0] = ULONG_MAX + 1 - k2[0] + k1[0];
        r[1] = k1[1] - k2[1];
        return r;
    } else {
        r[1] = k1[1] - k2[1];
        r[0] = k1[0] - k2[0];
        return r;
    }
}

static unsigned long long *adder(unsigned long long *k1, unsigned long long *k2)
{
    unsigned long long *r = kmalloc(2 * sizeof(unsigned long long), GFP_KERNEL);
    if (r == NULL) {
        printk("kmalloc error");
        return NULL;
    }
    char carry = 0;
    if ((ULONG_MAX - k2[0]) < k1[0])
        carry = 1;
    r[0] = k1[0] + k2[0];
    r[1] = k1[1] + k2[1] + (unsigned long long) (carry);
    return r;
}

static unsigned long long *multiplier(unsigned long long *k1,
                                      unsigned long long *k2)
{
    unsigned long long *r = kmalloc(2 * sizeof(unsigned long long), GFP_KERNEL);
    if (r == NULL) {
        printk("kmalloc error");
        return NULL;
    }
    r[0] = 0;
    r[1] = 0;
    size_t width = 8 * sizeof(unsigned long long);
    for (size_t i = 0; i < width; i++) {
        if ((k2[0] >> i) & 0x1) {
            r[1] += k1[1] << i;
            unsigned long long t = k1[0];
            (i == 0) ? (t = 0) : (t = t >> (width - i));
            r[1] += t;
            unsigned long long tmp[2] = {0};
            tmp[1] = 0;
            tmp[0] = k1[0] << i;
            r = adder(r, tmp);
            // r[0] += k1[0] << i;
        }
    }
    for (size_t i = 0; i < width; i++) {
        if ((k2[1] >> i) & 0x1) {
            r[1] += k1[0] << i;
        }
    }
    return r;
}

static unsigned long long *fast_fib(int k)
{
    if (k == 0) {
        unsigned long long *r =
            kmalloc(2 * sizeof(unsigned long long), GFP_KERNEL);
        if (r == NULL) {
            printk("kmalloc error");
            return NULL;
        }
        r[0] = 0;
        r[1] = 0;
        return r;
    }
    if (k == 1) {
        unsigned long long *r =
            kmalloc(2 * sizeof(unsigned long long), GFP_KERNEL);
        if (r == NULL) {
            printk("kmalloc error");
            return NULL;
        }
        r[0] = 1;
        r[1] = 0;
        return r;
    }
    if (k == 2) {
        unsigned long long *r =
            kmalloc(2 * sizeof(unsigned long long), GFP_KERNEL);
        if (r == NULL) {
            printk("kmalloc error");
            return NULL;
        }
        r[0] = 1;
        r[1] = 0;
        return r;
    }
    /* f(2n) = 2 * f(n+1) * f(n) - [f(n)]^2 */
    /* f(2n+1) = [f(n+1)]^2 + [f(n)]^2 */
    if (k % 2) {
        /* Odd */
        unsigned long long *fn1, *fn;
        fn1 = fast_fib((k >> 1) + 1);
        fn = fast_fib(k >> 1);
        return adder(multiplier(fn1, fn1), multiplier(fn, fn));
    } else {
        unsigned long long *fn1, *fn, *front;
        unsigned long long two[2] = {0};
        two[0] = 2;
        fn1 = fast_fib((k >> 1) + 1);
        fn = fast_fib(k >> 1);
        front = multiplier(fn1, fn);
        front = multiplier(two, front);
        return subtractor(front, multiplier(fn, fn));
    }
}

static unsigned long long *fib_sequence(int k)
{
    /* FIXME: use clz/ctz and fast algorithms to speed up */
    unsigned long long *f[k + 2];
    f[0] = kmalloc(2 * sizeof(unsigned long long), GFP_KERNEL);
    f[1] = kmalloc(2 * sizeof(unsigned long long), GFP_KERNEL);
    if (f[0] == NULL || f[1] == NULL) {
        printk("kmalloc error");
        return NULL;
    }
    f[0][1] = 0;
    f[1][1] = 0;
    f[0][0] = 0;
    f[1][0] = 1;

    for (int i = 2; i <= k; i++) {
        f[i] = adder(f[i - 1], f[i - 2]);
    }
    return f[k];
}

static int fib_open(struct inode *inode, struct file *file)
{
    if (!mutex_trylock(&fib_mutex)) {
        printk(KERN_ALERT "fibdrv is in use");
        return -EBUSY;
    }
    return 0;
}

static int fib_release(struct inode *inode, struct file *file)
{
    mutex_unlock(&fib_mutex);
    return 0;
}

/* calculate the fibonacci number at given offset */
static ssize_t fib_read(struct file *file,
                        char *buf,
                        size_t size,
                        loff_t *offset)
{
    unsigned long long *f;
    if (!memcmp(buf, "fast", 4)) {
        f = fast_fib(*offset);
    } else {
        f = fib_sequence(*offset);
    }

    memset(buf, 0, 16);
    for (size_t i = 0; i < 8; i++) {
        buf[i] = (f[0] >> (4 * i)) & 0xFF;
    }
    for (size_t i = 8; i < 16; i++) {
        buf[i] = (f[1] >> (4 * (i - 8))) & 0xFF;
    }
    return f[0];
}

/* write operation is skipped */
static ssize_t fib_write(struct file *file,
                         const char *buf,
                         size_t size,
                         loff_t *offset)
{
    return 1;
}

static loff_t fib_device_lseek(struct file *file, loff_t offset, int orig)
{
    loff_t new_pos = 0;
    switch (orig) {
    case 0: /* SEEK_SET: */
        new_pos = offset;
        break;
    case 1: /* SEEK_CUR: */
        new_pos = file->f_pos + offset;
        break;
    case 2: /* SEEK_END: */
        new_pos = MAX_LENGTH - offset;
        break;
    }

    if (new_pos > MAX_LENGTH)
        new_pos = MAX_LENGTH;  // max case
    if (new_pos < 0)
        new_pos = 0;        // min case
    file->f_pos = new_pos;  // This is what we'll use now
    return new_pos;
}

const struct file_operations fib_fops = {
    .owner = THIS_MODULE,
    .read = fib_read,
    .write = fib_write,
    .open = fib_open,
    .release = fib_release,
    .llseek = fib_device_lseek,
};

static int __init init_fib_dev(void)
{
    int rc = 0;

    mutex_init(&fib_mutex);

    // Let's register the device
    // This will dynamically allocate the major number
    rc = alloc_chrdev_region(&fib_dev, 0, 1, DEV_FIBONACCI_NAME);

    if (rc < 0) {
        printk(KERN_ALERT
               "Failed to register the fibonacci char device. rc = %i",
               rc);
        return rc;
    }

    fib_cdev = cdev_alloc();
    if (fib_cdev == NULL) {
        printk(KERN_ALERT "Failed to alloc cdev");
        rc = -1;
        goto failed_cdev;
    }
    cdev_init(fib_cdev, &fib_fops);
    rc = cdev_add(fib_cdev, fib_dev, 1);

    if (rc < 0) {
        printk(KERN_ALERT "Failed to add cdev");
        rc = -2;
        goto failed_cdev;
    }

    fib_class = class_create(THIS_MODULE, DEV_FIBONACCI_NAME);

    if (!fib_class) {
        printk(KERN_ALERT "Failed to create device class");
        rc = -3;
        goto failed_class_create;
    }

    if (!device_create(fib_class, NULL, fib_dev, NULL, DEV_FIBONACCI_NAME)) {
        printk(KERN_ALERT "Failed to create device");
        rc = -4;
        goto failed_device_create;
    }
    return rc;
failed_device_create:
    class_destroy(fib_class);
failed_class_create:
    cdev_del(fib_cdev);
failed_cdev:
    unregister_chrdev_region(fib_dev, 1);
    return rc;
}

static void __exit exit_fib_dev(void)
{
    mutex_destroy(&fib_mutex);
    device_destroy(fib_class, fib_dev);
    class_destroy(fib_class);
    cdev_del(fib_cdev);
    unregister_chrdev_region(fib_dev, 1);
}

module_init(init_fib_dev);
module_exit(exit_fib_dev);
