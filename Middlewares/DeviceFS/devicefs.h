#ifndef DEVICEFS_H
#define DEVICEFS_H

#include <stddef.h>

///////////////////////////////////////////////////////////////////////////////
/** device fs has two interfaces
    one for apps (user, userspace) and one for driver (hal, provider, kernel etc.)
		the former looks like a file i/o system
		the latter looks like char device/drivers in kernel.
**/

struct device;										// simulate cdev & inode in kernel
struct file;											// simulate file in kernel
struct file_operations;						// simulate file_operations in kernel

struct device
{
	const char											*name;		/** device name, such as "UART2" 	**/
	const	int												number;		/** device number if any					**/
	const struct file_operations		*f_ops;
	struct file_operations* const		test;
};

/** represent an opened (device) file **/
struct file	
{
	const struct file_operations    *f_ops;
	void														*private_data;
};

struct file_operations
{
	// unsigned int (*llseek) (struct file *, unsigned int, int);
	size_t (*read) (struct file *filp, char* buf, size_t size, unsigned int *f_pos);				/** f_pos, new position if applicable **/
	size_t (*write) (struct file *filp, const char* buf, size_t size, unsigned int *f_pos);	
	int (*open) (struct device *, struct file *);																						/** different than that of kernel, inode is replaced by device **/
	int (*flush) (struct file *);
	int (*release) (struct device *, struct file *);
};

struct device_entry
{
	struct device * dev;
};


/** one list for devices 										**/
/** one array for file descriptors -> filp 	**/
#define MAX_DEVICE						10
#define MAX_OPEN_FILE					10

struct device_fs
{
	struct device_entry 			*dent[MAX_DEVICE];
	struct file 							*filp[MAX_OPEN_FILE];
};

extern struct device_fs DevFS;

int open(struct device_fs * dfs, const char* device_name, int flags);
int read(struct device_fs	* dfs, int fd, char * buf, size_t size);
int write(struct device_fs * dfs, int fd, char * buf, size_t size);
int close(struct device_fs* dfs, int fd);



#endif



