#include <linux/syscalls.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/ioctl.h>
#include <linux/workqueue.h>
#include <linux/interrupt.h>
#include <asm/io.h>
#include <linux/types.h>
#include <linux/timer.h>
#include <asm/ioctls.h>
#include <linux/fs.h>
#include <asm/segment.h>
#include <asm/uaccess.h>
#include <linux/buffer_head.h>
#include <linux/hidraw.h>
#include <linux/version.h>
#include <linux/input.h>
#include <linux/kd.h>
#include <linux/unistd.h>
#include <linux/leds.h>
#include <linux/vt.h>
#include <linux/tty.h>
#include <linux/console_struct.h>
#include <linux/hid.h>
#include <linux/power_supply.h>
#include <asm/ioctl.h>

//some of the libraries which were included are not used. Decided to leave them be, just in case there's a need to research a similar topic. Our apologies for the inconvinience.


MODULE_LICENSE("GPL");
int present_time;
int start_jiffies;
//simple jiffies ints to track the passed time
struct file *f; //unused, since 5 of the ioctl method do not apply to it
int ctrl_pressed = 0; //tracks whether we have pressed a ctrl before, since the color should be changed as a hotkey
typedef unsigned char byte; //buffer with color flags that are set. The buffer works fine, ioctl which is 3-5 lines of code, but does not work. Tested multiply times in user space
struct color newcolor, newcolor2, newcolor3; //left,middle,and right color flags respectively.
atomic_t counter1; //counter to count the entered symbols
atomic_t changed; // 0 if no change since no hotkey was pressed, 1 if 1 region has to be changed, 3 if all keyboard is using a preset

struct color
{
    byte R;
    byte G;
    byte B;
}; //colors struct. nothing much

#ifndef HIDIOCSFEATURE
#define HIDIOCSFEATURE(len) _IOC(_IOC_WRITE | _IOC_READ, 'H', 0x06, len)
#endif //for HID settings, since sometimes they are not correctly set

#ifndef HIDIOCGFEATURE
#define HIDIOCGFEATURE(len) _IOC(_IOC_WRITE | _IOC_READ, 'H', 0x07, len)
#endif

// struct hid_device_
// {
//     int device_handle;
//     int blocking;
//     int uses_numbered_reports;
// };
// static hid_device *new_hid_device(void)
// {
//     hid_device *dev = kmalloc(1, sizeof(hid_device));
//     dev->device_handle = -1;
//     dev->blocking = 1;
//     dev->uses_numbered_reports = 0;

//     return dev;
// } //DOES NOT WORK

void *HANDLER_KEY;

irqreturn_t irq_handler(int irq, void *dev_id)
{

    static unsigned char scancode; //key pressed value checking
    unsigned char status;

    /*
* Read keyboard status
*/
    status = inb(0x64);
    scancode = inb(0x60);

    atomic_set(&changed, 1); //using atomic variables to make sure no interrupts change anything
    byte region;

    switch (scancode)
    {
    case 0x1D:
        atomic_set(&changed, 0);
        atomic_inc(&counter1);
        ctrl_pressed = 1;
        break;
    case 0x02:
        atomic_inc(&counter1);
        if (ctrl_pressed == 1)
        {
            printk(KERN_INFO "! You pressed 1 and selected the left region, no color ...\n");
            region = 1;

            newcolor.R = 0;
            newcolor.G = 0;
            newcolor.B = 255;
            start_jiffies = jiffies;
        }
        break;
    case 0x03:
        atomic_inc(&counter1);
        if (ctrl_pressed == 1)
        {
            printk(KERN_INFO "! You pressed 2 and selected the left region, red color ...\n");
            region = 1;
            newcolor.R = 255;
            newcolor.G = 0;
            newcolor.B = 0;
            start_jiffies = jiffies;
        }
        break;
    case 0x04:
        atomic_inc(&counter1);
        if (ctrl_pressed == 1)
        {
            printk(KERN_INFO "! You pressed 3 and selected the left region, green color ...\n");
            region = 1;
            newcolor.R = 0;
            newcolor.G = 255;
            newcolor.B = 0;
            start_jiffies = jiffies;
        }
        break;
    case 0x05:
        atomic_inc(&counter1);
        if (ctrl_pressed == 1)
        {
            printk(KERN_INFO "! You pressed 4 and selected the left region, blue color ...\n");
            region = 1;
            newcolor.R = 0;
            newcolor.G = 0;
            newcolor.B = 255;
            start_jiffies = jiffies;
        }
        break;
    case 0x06:
        atomic_inc(&counter1);
        if (ctrl_pressed == 1)
        {
            printk(KERN_INFO "! You pressed 5 and selected the middle region, no color ...\n");
            region = 2;
            newcolor2.R = 0;
            newcolor2.G = 0;
            newcolor2.B = 255;
            start_jiffies = jiffies;
        }
        break;
    case 0x07:
        atomic_inc(&counter1);
        if (ctrl_pressed == 1)
        {
            printk(KERN_INFO "! You pressed 6 and selected the middle region, red color ...\n");
            region = 2;
            newcolor2.R = 255;
            newcolor2.G = 0;
            newcolor2.B = 0;
            start_jiffies = jiffies;
        }
        break;
    case 0x08:
        atomic_inc(&counter1);
        if (ctrl_pressed == 1)
        {
            printk(KERN_INFO "! You pressed 7 and selected the middle region, green color ...\n");
            region = 2;
            newcolor2.R = 0;
            newcolor2.G = 255;
            newcolor2.B = 0;
            start_jiffies = jiffies;
        }
        break;
    case 0x09:
        atomic_inc(&counter1);
        if (ctrl_pressed == 1)
        {
            printk(KERN_INFO "! You pressed 8 and selected the middle region, blue color ...\n");
            region = 2;
            newcolor2.R = 0;
            newcolor2.G = 0;
            newcolor2.B = 255;
            start_jiffies = jiffies;
        }
        break;
    case 0x0A:
        atomic_inc(&counter1);
        if (ctrl_pressed == 1)
        {
            printk(KERN_INFO "! You pressed 9 and selected the right region, no color ...\n");
            region = 3;
            newcolor3.R = 0;
            newcolor3.G = 0;
            newcolor3.B = 255;
            start_jiffies = jiffies;
        }
        break;
    case 0x0B:
        atomic_inc(&counter1);
        if (ctrl_pressed == 1)
        {
            printk(KERN_INFO "! You pressed 0 and selected the right region, red color ...\n");
            region = 3;
            newcolor3.R = 255;
            newcolor3.G = 0;
            newcolor3.B = 0;
            start_jiffies = jiffies;
        }
        break;
    case 0x0C:
        atomic_inc(&counter1);
        if (ctrl_pressed == 1)
        {
            printk(KERN_INFO "! You pressed - and selected the right region, green color ...\n");
            region = 3;
            newcolor3.R = 0;
            newcolor3.G = 255;
            newcolor3.B = 0;
            start_jiffies = jiffies;
        }
        break;
    case 0x0D:
        atomic_inc(&counter1);
        if (ctrl_pressed == 1)
        {
            printk(KERN_INFO "! You pressed = and selected the right region, blue color ...\n");
            region = 3;
            newcolor3.R = 0;
            newcolor3.G = 0;
            newcolor3.B = 255;
            start_jiffies = jiffies;
        }
        break;
    case 0x4F:
        atomic_inc(&counter1);
        if (ctrl_pressed == 1)
        {
            atomic_set(&changed, 3);
            printk(KERN_INFO "! You pressed 'Keypad 1' and selected the RGB preset ...\n");

            region = 1;
            newcolor.R = 0;
            newcolor.G = 0;
            newcolor.B = 255;

            newcolor2.R = 0;
            newcolor2.G = 255;
            newcolor2.B = 0;

            newcolor3.R = 0;
            newcolor3.G = 0;
            newcolor3.B = 255;
            start_jiffies = jiffies;
        }
        break;
    case 0x50:
        atomic_inc(&counter1);
        if (ctrl_pressed == 1)
        {
            atomic_set(&changed, 3);
            printk(KERN_INFO "! You pressed 'Keypad 2' and selected the seashore preset ...\n");
            region = 1;
            newcolor.R = 0;
            newcolor.G = 150;
            newcolor.B = 255;

            newcolor2.R = 0;
            newcolor2.G = 255;
            newcolor2.B = 255;

            newcolor3.R = 0;
            newcolor3.G = 255;
            newcolor3.B = 150;
            start_jiffies = jiffies;
        }
        break;
    case 0x51:
        atomic_inc(&counter1);
        if (ctrl_pressed == 1)
        {
            atomic_set(&changed, 3);
            printk(KERN_INFO "! You pressed 'Keypad 3' and selected the sandstorm preset ...\n");

            region = 1;
            newcolor.R = 255;
            newcolor.G = 150;
            newcolor.B = 0;

            newcolor2.R = 255;
            newcolor2.G = 150;
            newcolor2.B = 100;

            newcolor3.R = 255;
            newcolor3.G = 200;
            newcolor3.B = 50;
            start_jiffies = jiffies;
        }
        break;
    case 0x52:
        atomic_set(&changed, 0);
        atomic_inc(&counter1);
        present_time = jiffies_to_msecs(jiffies - start_jiffies) / 1000;
        printk(KERN_INFO "! R:%d,G:%d,B:%d|R:%d,G:%d,B:%d.|R:%d,G:%d,B:%d. This color is working for %d seconds  ...\n", newcolor.R, newcolor.G, newcolor.B, newcolor2.R, newcolor2.G, newcolor2.B,
               newcolor3.R, newcolor3.G, newcolor3.B, present_time);
        break;
    default:
        atomic_set(&changed, 0);
        ctrl_pressed = 0;
        atomic_inc(&counter1);
        break;
    }
    byte buffer[8];

    buffer[0] = 1;
    buffer[1] = 2;
    buffer[2] = 64;
    buffer[3] = region;
    if (atomic_read(&counter1) / 2 % 70 == 0) //can be 7000
    {
        printk(KERN_INFO "\nGO AND HAVE SOME REST. YOU'VE TYPED 7000 SYMBOLS\n");
        buffer[4] = 255;
        buffer[5] = 0;
        buffer[6] = 0;
    }
    else
    {
        buffer[4] = newcolor.R;
        buffer[5] = newcolor.G;
        buffer[6] = newcolor.B;
    }

    if (atomic_read(&changed) == 3)
    {
        //send the buffer to the ioctl

        buffer[3] = 2;
        if (atomic_read(&counter1) % 7000 == 0)
        {
            printk(KERN_INFO "\nGO AND HAVE SOME REST. YOU'VE TYPED 7000 SYMBOLS\n");
            buffer[4] = 255;
            buffer[5] = 0;
            buffer[6] = 0;
        }
        else
        {
            buffer[4] = newcolor.R;
            buffer[5] = newcolor.G;
            buffer[6] = newcolor.B;
        }
        //send the second buffer to the ioctl

        buffer[3] = region;
        if (atomic_read(&counter1) % 7000 == 0)
        {
            printk(KERN_INFO "GO AND HAVE SOME REST. YOU'VE TYPED 7000 SYMBOLS");
            buffer[4] = 255;
            buffer[5] = 0;
            buffer[6] = 0;
        }
        else
        {
            buffer[4] = newcolor.R;
            buffer[5] = newcolor.G;
            buffer[6] = newcolor.B;
        }
        //send the third buffer to the ioctl and the program is done
    }


    //Here lie all of the methods that were tried for the ioctl. Earlier, ioctl was easily used in such cases, since there was a BKL. Now, it was moved, thus we got some problems.
    //libudev,hidapy,kd,simple unlocked_ioctl did not work. Sadly.
    //....................................................................................................
    //hid_device *keyboard = NULL;
    //keyboard = hid_open(0x1770, 0xff00, 0);
    //hid_send_feature_report(keyboard, buffer, 8);

    //struct file *f;
    //f = filp_open("/sys/class/hidraw/hidraw0", O_RDWR, 0);
    //mm_segment_t old_fs = get_fs();
    //set_fs(KERNEL_DS);

    //f->f_op->unlocked_ioctl(f, HIDIOCSFEATURE(8), buffer);

    // Restore space
    //set_fs(old_fs);

    // Close file
    //filp_close(f, 0);

    //hidraw_report_event(device_handle, HIDIOCSFEATURE(8), buffer);
    // mm_segment_t fs;

    // fs = get_fs();     /* save previous value */
    // set_fs (get_ds()); /* use kernel limit */

    /* system calls can be invoked */
    //  ksys_ioctl(device_handle, HIDIOCSFEATURE(8), buffer);

    // set_fs(fs); /* restore before returning to user space */
    //..................................................................................................................
    return IRQ_HANDLED;
}

/*
* Initialize the module - register the IRQ handler
*/
static int __init irq_ex_init(void)
{
    atomic_set(&counter1, 0); //set a counter for further incrementing and tracking changes
    HANDLER_KEY = (void *)(irq_handler);
    /* Free interrupt*/
    free_irq(1, NULL);

    /*
    * Request IRQ 1, the keyboard IRQ, to go to our irq_handler.
    */
    return request_irq(1, (irq_handler_t)irq_handler, IRQF_SHARED, "test_keyboard_irq_handler", HANDLER_KEY);
}

static void __exit irq_ex_exit(void)
{
    printk(KERN_INFO "! Module is unload... \n");
    free_irq(1, HANDLER_KEY);
}

module_init(irq_ex_init);
module_exit(irq_ex_exit);