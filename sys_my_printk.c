#include <linux/kernel.h>
#include <linux/ktime.h>
#include <linux/timekeeping.h>
#include <linux/linkage.h>

asmlinkage int sys_my_printk(char* dmesg_infor)
{
    printk("%s", dmesg_infor);
    return 0;
}