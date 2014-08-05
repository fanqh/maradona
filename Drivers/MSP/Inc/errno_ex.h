#ifndef ERRNO_EX_H_
#define ERRNO_EX_H_

#include <errno.h>

/** ENOMEM & EINVAL is already defined **/


#define EBASE				(0x10000000)
#define	EIO					(EBASE + 5)
#define EBUSY				(EBASE + 16)
#define ENODEV			(EBASE + 19)
#define EFATAL			(EBASE + 100)		/** no way to recover until a system reset **/

#endif
