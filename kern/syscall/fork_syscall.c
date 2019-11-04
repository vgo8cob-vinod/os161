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
#include <mips/trapframe.h>
#include <proc.h>
#include <current.h>
#include <vfs.h>
#include <synch.h>
#include <addrspace.h>
#include <kern/fcntl.h>
#include <proc.h>
#include <vnode.h>
/*
 * Example system call: open the file.
 */
int sys_fork(struct trapframe *tf)
{
struct proc *t_proc= kmalloc(sizeof(*t_proc));
Error_Struct *ErrStruct=kmalloc(sizeof(Error_Struct));
struct trapframe *tf1= kmalloc(sizeof(*tf1));
struct lock *lock = kmalloc(sizeof(lock));
struct cv *cv =kmalloc(sizeof(cv)) ;
char p[6]="child" ;
void (*func)(void *,unsigned long);
int result,i;
if ((t_proc == NULL) || (ErrStruct == NULL) || (tf1 == NULL) || (lock == NULL) || (cv == NULL)) 
{
	panic("kmalloc failed in forksyscall\n");
}

ErrStruct->O_fd =-1;
ErrStruct->Err_No =0;

func = (void*)enter_forked_process;

for(i=PID_MIN;i<PID_MAX;i++)
{
	if(processtable_a[i]==NULL)
	{
		processtable_a[i] = t_proc;
		break;
	}
}
t_proc->PPID_i=curproc->PID_i;
t_proc->PID_i=i;
t_proc->p_name=p;
t_proc->p_numthreads=0;

	spinlock_acquire(&curproc->p_lock);	
	if (curproc->p_cwd != NULL) {
		VOP_INCREF(curproc->p_cwd);
		t_proc->p_cwd = curproc->p_cwd;
	}
	spinlock_release(&curproc->p_lock);


spinlock_init(&t_proc->p_lock);
lock=lock_create("parent");
t_proc->proc_lock=lock;
cv=cv_create("child1");
t_proc->proc_cv=cv;

for(i=0;i<OPEN_MAX;i++)
{
     t_proc->filetable_a[i]=NULL;

}

	/* VM fields */
	t_proc->p_addrspace = NULL;



*tf1=*tf;
result=as_copy(curproc->p_addrspace, &t_proc->p_addrspace);
if(result)
{
	ErrStruct->Err_No=result;
	return (int)ErrStruct;
}

for(i=0;i<OPEN_MAX;i++)
{
	if(curproc->filetable_a[i]!=NULL)
	{
		curproc->filetable_a[i]->Counter++;
		t_proc->filetable_a[i]=curproc->filetable_a[i];
		
	}
}

//lock_acquire(curproc->proc_lock);
result=thread_fork("child",
	    t_proc,
	    func,
	    tf1, 0);
if(result)
{
	ErrStruct->Err_No=result;
	return (int)ErrStruct;
}
ErrStruct->O_fd =t_proc->PID_i;
	return (int)ErrStruct;
}


