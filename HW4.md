# HW4 Answer

## Part One: Eliminate allocation from sbrk()

For this part, we just follow the insturctions from the question:
Your new sbrk(n) should just increment the process's size (myproc()->sz) by n and return the old size. It should not allocate memory -- so you should delete the call to growproc() (but you still need to increase the process's size!). Below is my code.

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  struct proc *process = myproc();
  addr = process->sz;
  process->sz += n;
  return addr;
}

## Part Two: Lazy allocation

For this part, we follow the hint.
Hint 1: look at the cprintf arguments to see how to find the virtual address that caused the page fault.
In trap.C, we get the cprintf that prints the error message. We get that the virtual address can be found using rcr2().
cprintf("pid %d %s: trap %d err %d on cpu %d "
            "eip 0x%x addr 0x%x--kill proc\n",
            myproc()->pid, myproc()->name, tf->trapno,
            tf->err, cpuid(), tf->eip, rcr2());

Hint 2: you can check whether a fault is a page fault by checking if tf->trapno is equal to T_PGFLT in trap().
So we add this piece of code to my pageFaultAlloc function.
if (tf->trapno == T_PGFLT) 
{
	body;
}

Hint 3: you'll need to call mappages(). In order to do this you'll need to delete the static in the declaration of mappages() in vm.c, and you'll need to declare mappages() in trap.c. Add this declaration to trap.c before any call to mappages():
int mappages(pde_t *pgdir, void *va, uint size, uint pa, int perm);

For this hint, we just delete the static keyword in vm.c/mappages(), and add the piece of lines above to trap.C. You can add it to almost anywhere in trap.C.
// Interrupt descriptor table (shared by all CPUs).
struct gatedesc idt[256];
extern uint vectors[];  // in vectors.S: array of 256 entry pointers
struct spinlock tickslock;
int mappages(pde_t *pgdir, void *va, uint size, uint pa, int perm);
uint ticks;

Hint 4: steal code from allocuvm() in vm.c, which is what sbrk() calls (via growproc()).

Below is my answer code.
if (pageFaultAlloc(tf->trapno, rcr2()) == 1) {//This part is my answer.
        return;
    }
    // In user space, assume process misbehaved.
    cprintf("pid %d %s: trap %d err %d on cpu %d "
            "eip 0x%x addr 0x%x--kill proc\n",
            myproc()->pid, myproc()->name, tf->trapno,
            tf->err, cpuid(), tf->eip, rcr2());
And below is the body of function pageFaultAlloc() that I wrote inside trap.C. Also remember to add "int pageFaultAlloc(unsigned int, unsigned int)" to "defs.h" under //trap.C
If you compare this code to allocuvm, you will see that they are almost the same except for two values, "pgdir" argument that is used in allocuvm() and oldsz.
int
pageFaultAlloc(uint trap_no, uint faultVA)
{
    if (trap_no == T_PGFLT) {
        cprintf("Page fault occurs\n");
        uint oldsz, newsz, a;
        char *mem;
        struct proc *curproc = myproc();
        newsz = faultVA;
        oldsz = PGROUNDDOWN(newsz);// Now we get the newsize which is the fault address that should be allocated. Since We only need to allocate one page per pageFault, the oldsz is just one pagesize below new size so it's PGROUNDDOWN(newsz).

        a = PGROUNDUP(oldsz);
	mem = kalloc();
	if(mem == 0){
	cprintf("lazy page allocation out of memory\n");
	deallocuvm(curproc->pgdir, newsz, oldsz); //The reason that the passing "pgdir" is curproc ->pgdir, I just copy from growproc.
	return 0;
	}
	memset(mem, 0, PGSIZE);
	if(mappages(curproc->pgdir, (char*)a, PGSIZE, V2P(mem), PTE_W|PTE_U) < 0){
	cprintf("lazy page allocation out of memory (2)\n");
	deallocuvm(curproc->pgdir, newsz, oldsz);
	kfree(mem);
	return 0;
	}
        return 1;
    }
    return 0;
}





























