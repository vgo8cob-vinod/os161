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

/*
 * Sample/test code for running a user program.  You can use this for
 * reference when implementing the execv() system call. Remember though
 * that execv() needs to do more than runprogram() does.
 */

#include <types.h>
#include <kern/errno.h>
#include <kern/fcntl.h>
#include <lib.h>
#include <proc.h>
#include <current.h>
#include <addrspace.h>
#include <vm.h>
#include <vfs.h>
#include <syscall.h>
#include <test.h>
#include <synch.h>
#include <copyinout.h>
/*
 * Load program "progname" and start running it in usermode.
 * Does not return except on error.
 *
 * Calls vfs_open on progname and thus may destroy it.
 */
#define p(addr)      (*(int8_t*)addr)
int
sys_execv(const_userptr_t progname,char **argv)
{
	struct addrspace *as;
volatile struct proc *t_proc= kmalloc(sizeof(*t_proc));
	struct vnode *v;
	vaddr_t entrypoint, stackptr;//stckptr_temp;//stackptr_final;
	char **argv1;
	int result,sum=0,argc=0,temp;
	int *strlen;
	char *lval;
	char **fin;
vaddr_t mo; 
struct vnode *p_cwd;
char *sttr;
char *des= kmalloc(sizeof(progname)) ;
	int i,j;

	/* Open the file. */
	result=copyin(progname, des,sizeof(progname));

p_cwd=curproc->p_cwd;
if(p_cwd)
{
}

	for(i=0;argv[i]!=NULL;i++)
	{		
	}
argc=i;
argv1=kmalloc(i*32);
strlen=kmalloc(i*32);
	for(i=0;argv[i]!=NULL;i++)
	{
		for(j=0;argv[i][j]!='\0';j++)
		{
			sum++;	
		}
	argv1[i]=kmalloc(j+1);
	strlen[i]=j+1;		
	}

	for(i=0;argv[i]!=NULL;i++)
	{
		for(j=0;argv[i][j]!='\0';j++)
		{
			argv1[i][j]=argv[i][j];	
		}
	argv1[i][j]='\0';
		
	}
	argv1[i]='\0';
sttr=kstrdup((const char*)progname);
	if(result)
	{
		return result;
	}
	result = vfs_open(sttr, O_RDONLY, 0, &v);
	if (result) {
		return result;
	}

	/* We should be a new process. */
	//KASSERT(proc_getas() == NULL);

	/* Create a new address space. */
	as = as_create();
	if (as == NULL) {
		vfs_close(v);
		return ENOMEM;
	}

	/* Switch to it and activate it. */
	proc_setas(as);
	as_activate();

	/* Load the executable. */
	result = load_elf(v, &entrypoint);
	if (result) {
		/* p_addrspace will go away when curproc is destroyed */
		vfs_close(v);
		return result;
	}

	/* Done with the file now. */
	vfs_close(v);
t_proc = curproc;
	curproc->p_cwd=p_cwd;
	/* Define the user stack in the address space */
	result = as_define_stack(as, &stackptr);
	if (result) {
		/* p_addrspace will go away when curproc is destroyed */
		return result;
	}
sum=0;
	for(i=0;i<=argc;i++)
	{
		temp=(strlen[i]%4);
		if(temp!=0)
		{
			strlen[i] +=(4-temp);
		}
	sum +=strlen[i];
	}	
	mo=(stackptr-((sum) + (argc*4)+4));
	//&stackptr_final=&stckptr_temp;
	lval =(char *)(stackptr);
	for(i=0;i<argc;i++)
	{	
		lval =(char *)(lval -(strlen[i])) ;
		for(j=0;argv1[i][j]!='\0';j++)
		{
			*(lval+j)=argv1[i][j];

		}
		for(;j<strlen[i];j++)
		{
			*(lval+j)='\0';
		}
	argv1[i]=lval;
	}
fin =(char **)(mo);
	for(i=0;i<=argc;i++)
	{
		*(fin+i)=argv1[i];
	}
	/* Warp to user mode. */
	enter_new_process(argc/*argc*/, (userptr_t)fin /*userspace addr of argv*/,
			  (userptr_t)as /*userspace addr of environment*/,
			  mo, entrypoint);

	/* enter_new_process does not return. */
	panic("enter_new_process returned\n");
	return EINVAL;
}
