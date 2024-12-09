#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/printk.h>
#include <linux/proc_fs.h>
#include <asm/current.h>

#define procfs_name "Mythread_info"
#define BUFSIZE  1024
char buf[BUFSIZE]; //kernel buffer

static ssize_t Mywrite(struct file *fileptr, const char __user *ubuf, size_t buffer_len, loff_t *offset){
    /*Your code here*/
    size_t len_to_copy;

    
    len_to_copy = min((unsigned long)buffer_len, (unsigned long)(BUFSIZE - *offset));

    
    if (copy_from_user(buf + *offset, ubuf, len_to_copy)) {
        return -EFAULT;
    }

    *offset += len_to_copy;

    
    return len_to_copy;
    /****************/
}


static ssize_t Myread(struct file *fileptr, char __user *ubuf, size_t buffer_len, loff_t *offset){
    size_t len_to_copy;
    ssize_t bytes_read = 0;
    int len;
    
    if (*offset >= BUFSIZE) {
        return 0;  // No more data to read
    }  
    
    len += snprintf(buf + len, BUFSIZE - len, "%s", buf);
    len += snprintf(buf + len, BUFSIZE - len,
                    "Process ID: %d Thread TID: %d time: %lld\n",
                    current->tgid, current->pid, current->utime/100/1000);

    
    len_to_copy = min((unsigned long)buffer_len, (unsigned long)(BUFSIZE - *offset));

    
    if (copy_to_user(ubuf, buf + *offset, len_to_copy)) {
        return -EFAULT;  // Error in copying data to user space
    }
    memset(buf, 0, BUFSIZE);
    
    *offset += len_to_copy;

    // Return the number of bytes read
    bytes_read = len_to_copy;
    return bytes_read;
}

static struct proc_ops Myops = {
    .proc_read = Myread,
    .proc_write = Mywrite,
};

static int My_Kernel_Init(void){
    proc_create(procfs_name, 0644, NULL, &Myops);   
    pr_info("My kernel says Hi");
    return 0;
}

static void My_Kernel_Exit(void){
    // remove_proc_entry(procfs_name, NULL);
    pr_info("My kernel says GOODBYE");
}

module_init(My_Kernel_Init);
module_exit(My_Kernel_Exit);

MODULE_LICENSE("GPL");
