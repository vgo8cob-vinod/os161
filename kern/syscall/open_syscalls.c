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
 * Example system call: open the file.
 */
int sys_open(const_userptr_t filename, int flags)
{
Error_Struct *ErrStruct=kmalloc(sizeof(Error_Struct));
struct vnode *v;
int result,i=3;
char *des= kmalloc(sizeof(filename)) ;
char *sttr;
FileHandle_st *open_in= kmalloc(sizeof(FileHandle_st));
struct lock *lock = kmalloc(sizeof(*lock));
ErrStruct->O_fd =-1;
ErrStruct->Err_No =0;

for(i=3;i<OPEN_MAX;i++)
{
	if(curproc->filetable_a[i]==NULL)
	{
		break;
	}
}
if(i==OPEN_MAX)
{
	ErrStruct->Err_No=EMFILE;//file table full
	return (int)ErrStruct;
}
if(flags>32)
{
	ErrStruct->Err_No=EINVAL;//invalid flag
	return (int)ErrStruct;
}

lock=lock_create("lck");
open_in->lock=lock;
open_in->open_Flags = flags;
open_in->offset = 0u;
open_in->Counter =0;

result=copyin(filename, des,sizeof(filename));
if(result)
{
	ErrStruct->Err_No=result;
	return (int)ErrStruct;
}
sttr=kstrdup((const char*)filename);
result = vfs_open(sttr, flags, 0, &v);
if(result)
{
	ErrStruct->Err_No=result;
	return (int)ErrStruct;
}
//open_in->Counter++;
open_in->v = v;
curproc->filetable_a[i]=open_in;
ErrStruct->O_fd =i;
	return (int)ErrStruct;
}


