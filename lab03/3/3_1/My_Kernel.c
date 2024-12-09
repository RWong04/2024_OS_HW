#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/printk.h>
#include <linux/proc_fs.h>
#include <asm/current.h>
#include <linux/sched.h>
#include <linux/sched/types.h>


#define procfs_name "Mythread_info"
#define BUFSIZE  1024
char buf[BUFSIZE];

static ssize_t Mywrite(struct file *fileptr, const char __user *ubuf, size_t buffer_len, loff_t *offset){
    /* Do nothing */
	return 0;
}


static ssize_t Myread(struct file *fileptr, char __user *ubuf, size_t buffer_len, loff_t *offset){
    size_t len_to_copy;
    ssize_t bytes_read = 0;
    struct task_struct *thread;
    int len;
    
    if (*offset >= BUFSIZE) {
        return 0;  // No more data to read
    }  


    for_each_thread(current, thread) {
        if (current->pid == thread->pid) continue;
        
        len += snprintf(buf + len, BUFSIZE - len,
                        "Process ID: %d Thread TID: %d Priority: %d State: %u\n",
                        current->pid, thread->pid, thread->prio, thread->__state);
        if (len >= BUFSIZE) break;
    }

    
    len_to_copy = min((unsigned long)buffer_lenmak, (unsigned long)(BUFSIZE - *offset));


    if (copy_to_user(ubuf, buf + *offset, len_to_copy)) {
        return -EFAULT;  // Error in copying data to user space
    }

    
    *offset += len_to_copy;

    
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
    pr_info("My kernel says GOODBYE");
}

module_init(My_Kernel_Init);
module_exit(My_Kernel_Exit);

MODULE_LICENSE("GPL");
