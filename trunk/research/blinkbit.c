/**
 *  blinkbit.c -  create a "file" /proc/blinkbit to enable/disable blinking
 *
 */

#include <linux/module.h>	 // Specifically, a module
#include <linux/kernel.h>	 // We're doing kernel work
#include <linux/proc_fs.h> // Necessary because we use the proc fs
#include <asm/uaccess.h>	 // for copy_from_user
#include <video/vga.h>     // for vga_io_r/vga_io_w

#define PROCFS_NAME 		"blinkbit"

/**
 * This structure hold information about the /proc file
 *
 */
static struct proc_dir_entry *Our_Proc_File;

/**
 * This function is called then the /proc file is read
 *
 */
int
procfile_read(char *buffer,
	      char **buffer_location,
	      off_t offset, int buffer_length, int *eof, void *data)
{
	if (offset > 0)	// finished read, return 0
		return 0;

	unsigned char b;

	// reset flipflop something something
	b = vga_io_r(0x3DA);

	// select attribute register 0x10
	vga_io_w(0x3C0, 0x10 + 0x20);

	// read state
	b = vga_io_r(0x3C1);

	// mask off bit 3, blink state (8)
	b >>= 3;
	b &= 1;

	// convert to ascii char
	b += '0';
        unsigned char c[2];
        c[0]=b;
        c[1]='\n';
 	// fill the buffer, return the buffer size
	memcpy(buffer, &c, 2);
	return 2;
}

/**
 * This function is called with the /proc file is written
 *
 */
int procfile_write(struct file *file, const char *buffer, unsigned long count,
		   void *data)
{
	if (count == 0)
		return 0;

	unsigned char c;

	if ( copy_from_user(&c, buffer, 1) ) {
		return -EFAULT;
	}

	// check valid input
	if (c != '0' && c != '1')
		return 1;

	// convert c to bitmask
	c -= '0';
	c <<= 3;

	unsigned char b;
	// reset flipflop something something
	b = vga_io_r(0x3DA);

	// select attribute register 0x10
	vga_io_w(0x3C0, 0x10 + 0x20);

	// read current state
	b = vga_io_r(0x3C1);

	// mask in blink state
	b &= ~(1 << 3);
	b |= c;

	// write the modified byte back
	vga_io_w(0x3C0, b);

	return 1;
}

/**
 *This function is called when the module is loaded
 *
 */
int init_module()
{
	/* create the /proc file */
	Our_Proc_File = create_proc_entry(PROCFS_NAME, 0644, NULL);

	if (Our_Proc_File == NULL) {
		remove_proc_entry(PROCFS_NAME, &proc_root);
		printk(KERN_ALERT "Error: Could not initialize /proc/%s\n",
			PROCFS_NAME);
		return -ENOMEM;
	}

	Our_Proc_File->read_proc  = procfile_read;
	Our_Proc_File->write_proc = procfile_write;
	Our_Proc_File->owner 	    = THIS_MODULE;
	Our_Proc_File->mode 	    = S_IFREG | S_IRUGO;
	Our_Proc_File->uid 	      = 0;
	Our_Proc_File->gid 	      = 0;
	Our_Proc_File->size    	  = 1;

	printk(KERN_INFO "/proc/%s created\n", PROCFS_NAME);
	return 0;	/* everything is ok */
}

/**
 *This function is called when the module is unloaded
 *
 */
void cleanup_module()
{
	remove_proc_entry(PROCFS_NAME, &proc_root);
	printk(KERN_INFO "/proc/%s removed\n", PROCFS_NAME);
}
