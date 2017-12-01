/*
 * File-related system call implementations.
 */

#include <types.h>
#include <kern/errno.h>
#include <kern/fcntl.h>
#include <kern/limits.h>
#include <kern/seek.h>
#include <kern/stat.h>
#include <lib.h>
#include <uio.h>
#include <proc.h>
#include <current.h>
#include <synch.h>
#include <copyinout.h>
#include <vfs.h>
#include <vnode.h>
#include <openfile.h>
#include <filetable.h>
#include <syscall.h>

/*
 * open() - get the path with copyinstr, then use openfile_open and
 * filetable_place to do the real work.
 */
int
sys_open(const_userptr_t upath, int flags, mode_t mode, int *retval)
{
	const int allflags = O_ACCMODE | O_CREAT | O_EXCL | O_TRUNC | O_APPEND | O_NOCTTY;

	char kpath[PATH_MAX + NAME_MAX];
	struct openfile *file;
	int result;
	size_t T;

	if(flags == allflags){
		*retval = EFAULT;
		return -1;
	}


	if(upath == NULL){
		*retval = EFAULT;
		return -1;
	}

	result = copyinstr((const_userptr_t)upath, kpath, (strlen((char *)upath)+1) * sizeof(char), &T);
	if(result){
		*retval = EFAULT;
		return -1;
	}

	result = openfile_open((char *)kpath, flags, mode, &file);
	if(result){
		*retval = EFAULT;
		return -1;
	}
	result = filetable_place(curthread->t_proc->p_filetable, file, retval);
	if(result){
		*retval = result;
		return -1;
	}
result = *retval;
*retval = 0;
	return result;
}

/*
 * read() - read data from a file
 */
int
sys_read(int fd, userptr_t buf, size_t size, int *retval)
{
       int result = 0;
			 struct openfile *file;

			 if(fd < 0 || fd > PATH_MAX){
				 *retval = EBADF;
				 return -1;
			 }
			 result = filetable_get(curthread->t_proc->p_filetable, fd, &file);
			 if(result){
				 *retval = result;
				 return -1;
			 }


			 lock_acquire(file->of_offsetlock);
			 if(file->of_accmode == O_WRONLY){
				 *retval = EBADF;
				 lock_release(file->of_offsetlock);
				 return EBADF;
			 }

			 struct uio x;
			 struct iovec y;
			 uio_kinit(&y, &x, buf, size, 0, UIO_READ);
			 result = VOP_READ(file->of_vnode, &x);
			 if(result){
				 *retval = result;
				 return -1;
			 }
			 file->of_offset = x.uio_offset;
			 lock_release(file->of_offsetlock);
			 *retval = size - x.uio_resid;
			 *retval = 0;


       /*
        * Your implementation of system call read starts here.
        *
        * Check the design document design/filesyscall.txt for the steps
        */


       return 0;
}

/*
 * write() - write data to a file
 */
 int
 sys_write(int fd, userptr_t buf, size_t size, int *retval){
	 int result = 0;
	 struct openfile *file;

	 if(fd < 0 || fd > PATH_MAX){
		 *retval = EBADF;
		 return -1;
	 }
	 result = filetable_get(curthread->t_proc->p_filetable, fd, &file);

	 lock_acquire(file->of_offsetlock);
	 if(file->of_accmode == O_RDONLY){
		 *retval = EBADF;
		 lock_release(file->of_offsetlock);
		 return EBADF;
	 }

	 struct uio x;
	 struct iovec y;
	 uio_kinit(&y, &x, buf, size, 0, UIO_WRITE);
	 //x.uio_rw = UIO_WRITE;
	 result = VOP_WRITE(file->of_vnode, &x);
	 if(result){
		 *retval = result;
		 return -1;
	 }
	 file->of_offset = x.uio_offset;
	 lock_release(file->of_offsetlock);
	 *retval = size - x.uio_resid;
	 *retval =0;

	 return 0;
 }

/*
 * close() - remove from the file table.
 sys_close needs to:
    - validate the fd number (use filetable_okfd)
    - use filetable_placeat to replace curproc's file table entry with NULL
    - check if the previous entry in the file table was also NULL
      (this means no such file was open)
    - decref the open file returned by filetable_placeat


 */
 int sys_close(int fd){
	 int result =0;
	 struct openfile *file = NULL;
	 struct openfile *oldFile, *otherFile;

	 //filetable_okfd(curthread->t_proc->p_filetable, fd);
	 if(!filetable_okfd(curthread->t_proc->p_filetable, fd)){
		 return -1;
	 }
	 filetable_placeat(curthread->t_proc->p_filetable, file, fd, &oldFile);
	 int df = fd-1;
	 if(df >= 0){
		  result = filetable_get(curthread->t_proc->p_filetable, df, &otherFile);
			if(!result){

	 	 }

	 }
	 openfile_decref(oldFile);

	 return 0;
 }

/*
* meld () - combine the content of two files word by word into a new file
sys_meld needs to:
   - copy in the supplied pathnames (pn1, pn2, and pn3)
   - open the first two files (use openfile_open) for reading
   - open the third file (use openfile_open) for writing
   - return if any file is not open'ed correctly
   - place them into curproc's file table (use filetable_place)
   - refer to sys_read() for reading the first two files
   - refer to sys_write() for writing the third file
   - refer to sys_close() to complete the use of three files
   - set the return value correctly for successful completion

	 sys_open(const_userptr_t upath, int flags, mode_t mode, int *retval)
	 sys_write(int fd, userptr_t buf, size_t size, int *retval){
	 int sys_close(int fd


*/
int sys_meld(const_userptr_t filenameA, const_userptr_t filenameB, const_userptr_t filenameC){

	char pathA [1000];
	char pathB [1000];
	char pathC [1000];
	mode_t mode = 0666;
	int *retval = 0;


	struct openfile *fileA, *fileB, *fileC;
	int result;
	size_t T;


	result = copyinstr((const_userptr_t)filenameA, pathA, (strlen((char *)filenameA)+1) * sizeof(char), &T);
	if(result){
		return -1;
	}
	result = copyinstr((const_userptr_t)filenameB, pathB, (strlen((char *)filenameA)+1) * sizeof(char), &T);
	if(result){
		return -1;
	}
	result = copyinstr((const_userptr_t)filenameC, pathC, (strlen((char *)filenameA)+1) * sizeof(char), &T);
	if(result){
		return -1;
	}

	result = openfile_open((char *)pathA, O_RDONLY, mode, &fileA);
	if(result){
		return -1;
	}
	result = openfile_open((char *)pathB, O_RDONLY, mode, &fileB);
	if(result){
		return -1;
	}
	result = openfile_open((char *)pathC, O_RDONLY, mode, &fileC);
	if(result){
		return -1;
	}
	result = filetable_place(curthread->t_proc->p_filetable, fileA, retval);
	if(result){
		*retval = result;
		return -1;
	}
	result = filetable_place(curthread->t_proc->p_filetable, fileB, retval);
	if(result){
		*retval = result;
		return -1;
	}
	result = filetable_place(curthread->t_proc->p_filetable, fileC, retval);
	if(result){
		*retval = result;
		return -1;
	}

	/*lock_acquire(fileA->of_offsetlock);
	if(fileA->of_accmode == O_WRONLY){
		*retval = EBADF;
		lock_release(file->of_offsetlock);
		return EBADF;
	}

	struct uio x;
	struct iovec y;
	uio_kinit(&y, &x, buf, size, 0, UIO_READ);
	result = VOP_READ(file->of_vnode, &x);
	if(result){
		*retval = result;
		return -1;
	}
	file->of_offset = x.uio_offset;
	lock_release(file->of_offsetlock);
	*retval = size - x.uio_resid;
	*retval = 0;*/

	//int result = 0;
	//result = sys_open(filenameA, O_RDONLY)



	return 0;
}
