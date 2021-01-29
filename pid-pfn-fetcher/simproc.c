#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/kernel.h>   
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include <linux/uaccess.h>

#include <linux/module.h>         // Core header for loading LKMs into the kernel
#include <linux/device.h>         // Header to support the kernel Driver Model
#include <linux/kernel.h>         // Contains types, macros, functions for the kernel
#include <linux/fs.h>             // Header for the Linux file system support
#include <linux/uaccess.h>          // Required for the copy to user function
#include <linux/sched/signal.h>
#include <linux/sched.h>
#include <linux/slab.h> 

#include <linux/init.h>
#include <linux/module.h>

#include <linux/pid.h>
#include<linux/sched.h>
#include <linux/pgtable.h>
#include <linux/mm_types.h>
#include<asm/mmzone.h>


#include <linux/version.h>



#define BUFSIZE  100

static char message[25600] = {0};           ///< Memory for the string that is passed from userspace

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Prathamesh Patil");

static int processId = 20;
module_param(processId, int, 0660);


static struct proc_dir_entry *ent;

long  virtualToPhysical(struct mm_struct * mm, unsigned long vaddr ) {
    long pfn_output = -1;
    pgd_t *pgd;
    pgd = pgd_offset( mm,  vaddr);
    if (!pgd_none(*pgd) || !pgd_bad(*pgd)) {
        p4d_t *p4d;
        p4d = p4d_offset( pgd , vaddr);
        pud_t *pud;
        pud = pud_offset( p4d , vaddr);
        if (!pud_none(*pud) || !pud_bad(*pud)) {
            pmd_t *pmd;
            pmd = pmd_offset( pud,  vaddr);
            if (!pmd_none(*pmd) || !pmd_bad(*pmd)) {
                pte_t * pte_offset;
                pte_offset = pte_offset_map( pmd,  vaddr);
                if(!pte_none(*pte_offset) && pte_present( * pte_offset)) {
                    unsigned long pfn = pte_pfn(*pte_offset);
                    if (pfn && pfn_valid(pfn))
                    {
                        pfn_output = pfn;
                    }
                }
            }
        }
    }               
    return pfn_output;
}

static ssize_t mywrite(struct file *file, const char __user *ubuf,size_t count, loff_t *ppos) 
{
	int num,c,i;
	char buf[BUFSIZE];
	if(*ppos > 0 || count > BUFSIZE)
		return -EFAULT;
	if(copy_from_user(buf,ubuf,count))
		return -EFAULT;

	num = sscanf(buf,"%d",&i);
	if(num != 1)
		return -EFAULT;
	processId = i; 
	c = strlen(buf);

	message[0] = '\0';

	struct task_struct * pid_struct;
	struct mm_struct * pid_mm_struct;
	printk("proc: mywrite: simpleproc: %d\n", processId);
	
	if (processId) {
		struct pid * pid;
		pid = find_get_pid(processId);
		if (pid) {
			pid_struct = pid_task(pid, PIDTYPE_PID);
			pid_mm_struct = pid_struct -> mm;

			struct vm_area_struct * vma;
			unsigned long vaddr;

			unsigned long vaddrStart = vma -> vm_start;
			unsigned long vaddrEnd = vma -> vm_end;
			unsigned long phyAddress;
			int i = 0;
			char * tmp = message;
			for (vma = pid_mm_struct -> mmap; vma; vma = vma -> vm_next) {
				phyAddress = virtualToPhysical(pid_mm_struct, vma -> vm_start);
				

				tmp += sprintf(tmp, "virtual start long %ld, virtual start %lx ,virtual end %lx, pfn %lx\n ", vma -> vm_start, vma -> vm_end, phyAddress);
				
				printk("proc: mywrite:virtual start long %ld, virtual start %lx ,virtual end %lx, pfn %lx\n ", vma -> vm_start, vma -> vm_end, phyAddress);
				// if( ++i == 10) break;

			}
		}
	}

	printk("\n");
	*ppos = c;
	return c;
}

static ssize_t myread(struct file *file, char __user *buffer, size_t count, loff_t *ppos) 
{

		
	int error_count = 0;

	if( ppos > strlen(message)) {
		return 0;
	}

	if(!ppos) {
		*ppos = message;
	}
	
	error_count = copy_to_user(buffer, ppos, count);
	// if(*ppos > 0) {
	// 	return 0;
	// }
	// if( count < (strlen(message))) {
	// 	printk(KERN_INFO "ProcListChar: buffer count smaller than excpected %d", count);
	// 	return 0;
	// }
	if (error_count == 0){            // if true then have success
		*ppos += count;
		printk(KERN_INFO "ProcListChar: Sent %ld characters to the user\n", count);
		return count;  // clear the position to the start and return 0
	}
	else {
		printk(KERN_INFO "ProcListChar: Failed to send %d characters to the user\n", error_count);
		return -EFAULT;              // Failed -- return a bad address message (i.e. -14)
	}
	memset(message,0,strlen(message));
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0)
#define HAVE_PROC_OPS
#endif

#ifdef HAVE_PROC_OPS
static const struct proc_ops myops = 
{
	.proc_read = myread,
	.proc_write = mywrite,
};
#else
static struct file_operations myops = 
{
	.owner = THIS_MODULE,
	.read = myread,
	.write = mywrite,
};
#endif

static int simple_init(void)
{
	ent=proc_create("mydev",0660,NULL,&myops);
	printk(KERN_ALERT "hello...\n");
	return 0;
}

static void simple_cleanup(void)
{
	proc_remove(ent);
	printk(KERN_WARNING "bye ...\n");
}

module_init(simple_init);
module_exit(simple_cleanup);
