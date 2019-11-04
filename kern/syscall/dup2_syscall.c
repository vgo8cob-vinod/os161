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
#include <vfs.h>
#include <synch.h>
/*
 * Example system call: get the time of day.
 */
int
sys_dup2(int fd_t,int New_fd)
{
Error_Struct *ErrStruct=kmalloc(sizeof(Error_Struct));
ErrStruct->Err_No =0;
ErrStruct->O_fd =-1;
if((fd_t <0) || (fd_t>OPEN_MAX))
{
	ErrStruct->Err_No=EBADF;
	return (int)ErrStruct;
}
if((New_fd==OPEN_MAX))
{
	ErrStruct->Err_No=EMFILE;
	return (int)ErrStruct;
}
if((New_fd <0) || (New_fd>OPEN_MAX))
{
	ErrStruct->Err_No=EBADF;
	return (int)ErrStruct;
}

if(curproc->filetable_a[fd_t]==NULL)
{
	ErrStruct->Err_No=EBADF;
	return (int)ErrStruct;
}
lock_acquire(curproc->filetable_a[fd_t]->lock);
if(curproc->filetable_a[New_fd]!=NULL)
{
vfs_close(curproc->filetable_a[New_fd]->v);
}

curproc->filetable_a[New_fd]=curproc->filetable_a[fd_t];
ErrStruct->O_fd =New_fd;
lock_release(curproc->filetable_a[fd_t]->lock);
	return (int)ErrStruct;
}
