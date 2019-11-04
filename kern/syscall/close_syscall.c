 /* Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2008, 2009
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
#include <current.h>
#include <vfs.h>
#include <synch.h>
#include <kern/fcntl.h>
/*
 * Example system call: close the file.
 */
int sys_close(int32_t fd_t)
{
Error_Struct *ErrStruct=kmalloc(sizeof(Error_Struct));

ErrStruct->O_fd =-1;
ErrStruct->Err_No =0;
if((fd_t <0) || (fd_t>=OPEN_MAX))
{
	ErrStruct->Err_No=EBADF;
	return (int)ErrStruct;
}
if(curproc->filetable_a[fd_t]==NULL)
{
	ErrStruct->Err_No=EBADF;
	return (int)ErrStruct;
}

if(curproc->filetable_a[fd_t]->Counter>0)
{
lock_acquire(curproc->filetable_a[fd_t]->lock);
curproc->filetable_a[fd_t]->Counter--;
lock_release(curproc->filetable_a[fd_t]->lock);
}
if(curproc->filetable_a[fd_t]->Counter < 0)
{panic("kmalloc failed in forksyscall\n");}
if(curproc->filetable_a[fd_t]->Counter==0)
{
lock_acquire(curproc->filetable_a[fd_t]->lock);
vfs_close(curproc->filetable_a[fd_t]->v);
ErrStruct->O_fd =0;
lock_release(curproc->filetable_a[fd_t]->lock);
lock_destroy(curproc->filetable_a[fd_t]->lock);
curproc->filetable_a[fd_t]=NULL;
}

	return (int)ErrStruct;
}

