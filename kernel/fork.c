/*
 *  linux/kernel/fork.c
 *
 *  (C) 1991  Linus Torvalds
 */

/*
 *  'fork.c' contains the help-routines for the 'fork' system call
 * (see also system_call.s), and some misc functions ('verify_area').
 * Fork is rather simple, once you get the hang of it, but the memory
 * management can be a bitch. See 'mm/mm.c': 'copy_page_tables()'
 */
#include <errno.h>

#include <linux/sched.h>
#include <linux/kernel.h>
#include <asm/segment.h>
#include <asm/system.h>

extern void write_verify(unsigned long address);
extern void first_return_from_kernel(void);

long last_pid=0;

void verify_area(void * addr,int size)
{
	unsigned long start;

	start = (unsigned long) addr;
	size += start & 0xfff;
	start &= 0xfffff000;
	start += get_base(current->ldt[2]);
	while (size>0) {
		size -= 4096;
		write_verify(start);
		start += 4096;
	}
}

int copy_mem(int nr,struct task_struct * p)
{
	unsigned long old_data_base,new_data_base,data_limit;
	unsigned long old_code_base,new_code_base,code_limit;

	code_limit=get_limit(0x0f);
	data_limit=get_limit(0x17);
	old_code_base = get_base(current->ldt[1]);
	old_data_base = get_base(current->ldt[2]);
	if (old_data_base != old_code_base)
		panic("We don't support separate I&D");
	if (data_limit < code_limit)
		panic("Bad data_limit");
	new_data_base = new_code_base = nr * 0x4000000;
	p->start_code = new_code_base;
	set_base(p->ldt[1],new_code_base);
	set_base(p->ldt[2],new_data_base);
	if (copy_page_tables(old_data_base,new_data_base,data_limit)) {
		printk("free_page_tables: from copy_mem\n");
		free_page_tables(new_data_base,data_limit);
		return -ENOMEM;
	}
	return 0;
}

/*
 *  Ok, this is the main fork-routine. It copies the system process
 * information (task[nr]) and sets up the necessary registers. It
 * also copies the data segment in it's entirety.
 */
// sys_fork 系统调用的处理函数
// ① CPU 执行中断指令压入的用户栈地址 ss 和 esp、标志 eflags 和返回地址 cs 和 eip；
// ② 第 85--91 行在刚进入 system_call 时入栈的段寄存器 ds、es、fs 和 edx、ecx、ebx；
// ③ 第 97 行上调用 sys_call_table 中 sys_fork 函数时入栈的返回地址（参数 none 表示）；
// ④ 第 226--230 行在调用 copy_process()之前入栈的 gs、esi、edi、ebp 和 eax（nr）。
// 其中参数 nr 是调用 find_empty_process()分配的任务数组项号。
int copy_process(int nr,long ebp,long edi,long esi,long gs,long none,
		long ebx,long ecx,long edx,
		long fs,long es,long ds,
		long eip,long cs,long eflags,long esp,long ss)
{
	struct task_struct *p;
	int i;
	struct file *f;
    long *kernelstack;

    // 为新任务分配内存
	p = (struct task_struct *) get_free_page();
	if (!p)
		return -EAGAIN;
	task[nr] = p;
    // 复制进程结构
	*p = *current;	/* NOTE! this doesn't copy the supervisor stack */
	p->state = TASK_UNINTERRUPTIBLE;
	p->pid = last_pid;
	p->father = current->pid;
	p->counter = p->priority;
	p->signal = 0;
	p->alarm = 0;
	p->leader = 0;		/* process leadership doesn't inherit */
	p->utime = p->stime = 0;
	p->cutime = p->cstime = 0;
	p->start_time = jiffies;

    kernelstack = (long*)(PAGE_SIZE + (long)p);

    // 做出进入switch_to函数的样子，用于被调度时返回
    // CPU自动弹入
    *(--kernelstack) = ss & 0xffff;
    *(--kernelstack) = esp;
    *(--kernelstack) = eflags;
    *(--kernelstack) = cs & 0xffff;
    *(--kernelstack) = eip;

    // 给 first_return_from_kernel 使用
    *(--kernelstack) = ds & 0xffff;
    *(--kernelstack) = es & 0xffff;
    *(--kernelstack) = fs & 0xffff;
    *(--kernelstack) = gs & 0xffff;
    *(--kernelstack) = esi;
    *(--kernelstack) = edi;
    *(--kernelstack) = edx;
    *(--kernelstack) = (long)first_return_from_kernel;

    // switch_to 中最后弹栈
    *(--kernelstack) = ebp;
    *(--kernelstack) = ecx;
    *(--kernelstack) = ebx;
    *(--kernelstack) = 0;

    p->kernelstack = kernelstack;

    /*// 修改任务状态段TSS内容*/
	/*p->tss.back_link = 0;*/
    /*// ss0:esp0 用作内核栈*/
	/*p->tss.esp0 = PAGE_SIZE + (long) p;*/
	/*p->tss.ss0 = 0x10;*/
	/*p->tss.eip = eip;*/
	/*p->tss.eflags = eflags;*/
    /*// fork返回0*/
	/*p->tss.eax = 0;*/
	/*p->tss.ecx = ecx;*/
	/*p->tss.edx = edx;*/
	/*p->tss.ebx = ebx;*/
	/*p->tss.esp = esp;*/
	/*p->tss.ebp = ebp;*/
	/*p->tss.esi = esi;*/
	/*p->tss.edi = edi;*/
	/*p->tss.es = es & 0xffff;*/
	/*p->tss.cs = cs & 0xffff;*/
	/*p->tss.ss = ss & 0xffff;*/
	/*p->tss.ds = ds & 0xffff;*/
	/*p->tss.fs = fs & 0xffff;*/
	/*p->tss.gs = gs & 0xffff;*/
	/*p->tss.ldt = _LDT(nr);*/
	/*p->tss.trace_bitmap = 0x80000000;*/
	if (last_task_used_math == current)
		__asm__("clts ; fnsave %0"::"m" (p->tss.i387));
	if (copy_mem(nr,p)) {
		task[nr] = NULL;
		free_page((long) p);
		return -EAGAIN;
	}
	for (i=0; i<NR_OPEN;i++)
		if ((f=p->filp[i]))
			f->f_count++;
	if (current->pwd)
		current->pwd->i_count++;
	if (current->root)
		current->root->i_count++;
	if (current->executable)
		current->executable->i_count++;
	set_tss_desc(gdt+(nr<<1)+FIRST_TSS_ENTRY,&(p->tss));
	set_ldt_desc(gdt+(nr<<1)+FIRST_LDT_ENTRY,&(p->ldt));
	p->state = TASK_RUNNING;	/* do this last, just in case */
	return last_pid;
}

int find_empty_process(void)
{
	int i;

	repeat:
		if ((++last_pid)<0) last_pid=1;
		for(i=0 ; i<NR_TASKS ; i++)
			if (task[i] && task[i]->pid == last_pid) goto repeat;
	for(i=1 ; i<NR_TASKS ; i++)
		if (!task[i])
			return i;
	return -EAGAIN;
}
