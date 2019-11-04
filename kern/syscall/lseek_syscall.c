/*
 * Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2008, 2009
 *	The President and Fellows of Harvard College.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE UNIVERSITY AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE UNIVERSITY OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
#include <kern/errno.h>
#include <types.h>
#include <copyinout.h>
#include <syscall.h>
#include <proc.h>
#include <uio.h>
#include <current.h>
#include <vnode.h>
#include <kern/fcntl.h>
#include <kern/seek.h>
#include <kern/stat.h>
#include <synch.h>
/*
 * Example system call: get the time of day.
 */
int
sys_lseek(int32_t fd_t, off_t pos,int* whence)
{
	bool result;
int64_t off_chk;
struct stat* stat1=kmalloc(sizeof(struct stat));
Error_Struct_64 *Error_Struct_64=kmalloc(sizeof(Error_Struct_64));
Error_Struct_64->Err_No =0;
Error_Struct_64->O_fd =-1;
if((fd_t <0) || (fd_t>=OPEN_MAX))
{
	Error_Struct_64->Err_No=EBADF;
	return (int)Error_Struct_64;
}
if(curproc->filetable_a[fd_t]==NULL)
{
	Error_Struct_64->Err_No=EBADF;
	return (int)Error_Struct_64;
}
if((fd_t <3) )
{
	Error_Struct_64->Err_No=ESPIPE;
	return (int)Error_Struct_64;
}

result = VOP_ISSEEKABLE(curproc->filetable_a[fd_t]->v);
if (result==0) 
{
	Error_Struct_64->Err_No=ESPIPE;
	return (int)Error_Struct_64;
}
off_chk = curproc->filetable_a[fd_t]->offset;
	switch (*whence) {
	    case SEEK_SET:
		off_chk =pos;
		break;
	    case SEEK_CUR:
		off_chk +=pos;
		break;
	    case SEEK_END:
	  VOP_STAT(curproc->filetable_a[fd_t]->v,stat1);//check later if lock has to be changed
		off_chk =stat1->st_size+pos;
		break;
	    default:
		Error_Struct_64->Err_No=EINVAL;
		return (int)Error_Struct_64;
	}
if(off_chk<0)
{
	Error_Struct_64->Err_No=EINVAL;
	return (int)Error_Struct_64;
}
curproc->filetable_a[fd_t]->offset =off_chk;
Error_Struct_64->O_fd =curproc->filetable_a[fd_t]->offset;
	return (int)Error_Struct_64;
}
