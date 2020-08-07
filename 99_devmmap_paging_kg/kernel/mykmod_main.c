#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/mm.h>

#include <mydev.h>

MODULE_DESCRIPTION("My kernel module - mykmod");
MODULE_AUTHOR("maruthisi.inukonda [at] gmail.com");
MODULE_LICENSE("GPL");

// Dynamically allocate major no
#define MYKMOD_MAX_DEVS 256
#define MYKMOD_DEV_MAJOR 0

static int mykmod_init_module(void);
static void mykmod_cleanup_module(void);

static int mykmod_open(struct inode *inode, struct file *filp);
static int mykmod_close(struct inode *inode, struct file *filp);
static int mykmod_mmap(struct file *filp, struct vm_area_struct *vma);

module_init(mykmod_init_module);
module_exit(mykmod_cleanup_module);

static struct file_operations mykmod_fops = {
	.owner = THIS_MODULE,	/* owner (struct module *) */
	.open = mykmod_open,	/* open */
	.release = mykmod_close,	/* release */
	.mmap = mykmod_mmap,	/* mmap */
};

static void mykmod_vm_open(struct vm_area_struct *vma);
static void mykmod_vm_close(struct vm_area_struct *vma);
//static int mykmod_vm_fault(struct vm_fault *vmf);
static int mykmod_vm_fault(struct vm_area_struct *vma, struct vm_fault *vmf);

// TODO Data-structure to keep per device info 
struct devinfo {
	char *data;
	size_t size;
};
typedef struct devinfo devinfo;
// TODO Device table data-structure to keep all devices
struct list {
	struct devinfo *info;
	int minor;		//minor number for reference
	struct list *p;
	struct list *pre_p;
};

typedef struct list list;
list *head = NULL;
list *tail = NULL;

// TODO Data-structure to keep per VMA info 
struct vma_struct {
	struct devinfo *info;
	int npagefaults;
};

static const struct vm_operations_struct mykmod_vm_ops = {
	.open = mykmod_vm_open,
	.close = mykmod_vm_close,
	.fault = mykmod_vm_fault
};

int mykmod_major;

static int mykmod_init_module(void)
{
	printk("mykmod loaded\n");
	printk("mykmod initialized at=%p\n", init_module);

	if ((mykmod_major =
	     register_chrdev(MYKMOD_DEV_MAJOR, "mykmod", &mykmod_fops)) < 0) {
		printk(KERN_WARNING "Failed to register character device\n");
		return 1;
	} else {
		printk("register character device %d\n", mykmod_major);
	}
	// TODO initialize device table
	head = tail = NULL;

	return 0;
}

static void mykmod_cleanup_module(void)
{
	printk("mykmod unloaded\n");
	unregister_chrdev(mykmod_major, "mykmod");
	// TODO free device info structures from device table
	while (head) {		//till nothing is left to free

		if (head == tail) {	//if there is only one node in device table
			kfree(head);
			head = tail = NULL;
		} else {	//remove the last device info structure and move tail to the previous
			list *temp = tail;
			temp->pre_p->p = temp->p;
			tail = temp->pre_p;
			kfree(temp->info);	//free the data
			kfree(temp);	//then free the struct
		}

	}
	return;
}

static int mykmod_open(struct inode *inodep, struct file *filep)
{

	// TODO: Allocate memory for devinfo and store in device table and i_private.
	if (inodep->i_private == NULL) {
		//allocating for device info structure
		struct devinfo *info =
		    kmalloc(sizeof(struct devinfo), GFP_KERNEL);
		//storing list structure containing devinfo in device table
		list *temp = kmalloc(sizeof(list), GFP_KERNEL);
		//allocating 1MB data
		info->data = kzalloc(MYDEV_LEN, GFP_KERNEL);
		info->size = MYDEV_LEN;
		//store devinfo in i_private
		inodep->i_private = info;

		temp->info = (devinfo *) inodep->i_private;
		temp->minor = MINOR(inodep->i_rdev);	//storing minor number
		temp->p = NULL;	//next pointer
		temp->pre_p = NULL;	//previous pointer

		if (head == NULL) {	//if no node is present
			head = temp;
			tail = temp;
		} else {
			temp->pre_p = tail;
			tail->p = temp;
			tail = temp;
		}
	}
	// Store device info in file's private_data aswell
	filep->private_data = inodep->i_private;
	printk("mykmod_open: filep=%p f->private_data=%p "
	       "inodep=%p i_private=%p i_rdev=%x maj:%d min:%d\n",
	       filep, filep->private_data,
	       inodep, inodep->i_private, inodep->i_rdev, MAJOR(inodep->i_rdev),
	       MINOR(inodep->i_rdev));

	return 0;
}

static int mykmod_close(struct inode *inodep, struct file *filep)
{
	// TODO: Release memory allocated for data-structures.

	/*releasing all memory in mykmod_cleanup module
	   struct devinfo *info =(struct devinfo *)inodep->i_private;
	   kfree(info);
	   inodep->i_private = NULL;
	   filep->private_data = NULL; */

	printk("mykmod_close: inodep=%p filep=%p\n", inodep, filep);
	return 0;
}

static int mykmod_mmap(struct file *filp, struct vm_area_struct *vma)
{
	//data structure for storing vma data
	struct vma_struct *vminfo =
	    kmalloc(sizeof(struct vma_struct), GFP_KERNEL);

	printk("mykmod_mmap: filp=%p vma=%p flags=%lx\n", filp, vma,
	       vma->vm_flags);

	//TODO setup vma's flags, save private data (devinfo, npagefaults) in vm_private_data
	vma->vm_ops = &mykmod_vm_ops;
	vma->vm_flags |= VM_DONTEXPAND | VM_DONTDUMP;
	//saving private data (devinfo, npagefaults) in vm_private_data
	//vminfo->npagefaults = 0;
	//initializing npagefaults in mykmod_vm_open
	vminfo->info = filp->private_data;
	vma->vm_private_data = vminfo;

	mykmod_vm_open(vma);	//start mapping

	//return -ENOSYS; // Remove this once mmap is implemented.
	return 0;
}

static void mykmod_vm_open(struct vm_area_struct *vma)
{
	struct vma_struct *vminfo = vma->vm_private_data;

	vminfo->npagefaults = 0;
	//initializing npagefaults to 0
	printk("mykmod_vm_open: vma=%p npagefaults:%d\n", vma,
	       vminfo->npagefaults);
}

static void mykmod_vm_close(struct vm_area_struct *vma)
{
	struct vma_struct *vminfo = vma->vm_private_data;
	//just printing npagefaults
	printk("mykmod_vm_close: vma=%p npagefaults:%d\n", vma,
	       vminfo->npagefaults);
	kfree(vminfo);		//free the per vma structure data vma_struct
}

static int mykmod_vm_fault(struct vm_area_struct *vma, struct vm_fault *vmf)
{
	struct vma_struct *vminfo = vma->vm_private_data;
	struct devinfo *info = (struct devinfo *)vminfo->info;
	struct page *page;
	unsigned long paddress;
	unsigned long firstoff;
	unsigned long secondoff;
	unsigned long pagenumber;

	vminfo->npagefaults++;	//incrementing for each page fault

	//printk("mykmod_vm_fault: vma=%p vmf=%p pgoff=%lu page=%p\n", vma, vmf, vmf->pgoff, vmf->page);
	// TODO: build virt->phys mappings

	//get physical address of stored 1MB data
	paddress = (unsigned long)virt_to_phys(info->data);
	//offset of mapping in bytes
	firstoff = (vma->vm_pgoff) << PAGE_SHIFT;
	//offset of current fault page in bytes
	secondoff = (vmf->pgoff) << PAGE_SHIFT;
	//get physical page number
	pagenumber = (paddress + firstoff + secondoff) >> PAGE_SHIFT;
	//get struct page from page number
	page = pfn_to_page(pagenumber);
	//allocate page
	get_page(page);
	//attach page in the required position for access
	vmf->page = page;

	return 0;
}
