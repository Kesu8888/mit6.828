# Lab2 Report

## Exercise 1. In the file kern/pmap.c, you must implement code for the following functions (probably in the order given).

1. boot_alloc()
result = nextfree;
    if (n > 0) {
        nextfree = ROUNDUP(nextfree + n, PGSIZE);
        if (((uint32_t) nextfree - KERNBASE) > (npages * PGSIZE)) 
            panic("boot_alloc: out of memory. Requested %uK, available %uK.\n",
                  (uint32_t) nextfree / 1024, npages * PGSIZE / 1024);
    }
	return result;
Linear memory from KERNBASE to nextfree kernel virtual memory are allocated, if it is bigger than the total physical memory npages * PGSIZE, we are out of memory.

2. mem_init()
	// Allocate an array of npages 'struct PageInfo's and store it in 'pages'.
	// The kernel uses this array to keep track of physical pages: for
	// each physical page, there is a corresponding struct PageInfo in this
	// array.  'npages' is the number of physical pages in memory.  Use memset
	// to initialize all fields of each struct PageInfo to 0.
	// Your code goes here:
    pages = boot_alloc(npages * sizeof(struct PageInfo));
    memset(pages, 0, npages * sizeof(struct PageInfo));
We first allocate the memory required for npages of pages, the memory size for each pageInfo struct is sizeof(struct PageInfo). Then we set these memory to 0.

3. page_init()
size_t i;
    pages[0].pp_ref = 1; //1) Mark physical page 0 as in use.
	for (i = 1; i < npages_basemem; i++) { //2) [PGSIZE, npages_basemem * PGSIZE) is free.
		pages[i].pp_ref = 0;
		pages[i].pp_link = page_free_list;
		page_free_list = &pages[i];
	}
    for (i = IOPHYSMEM/PGSIZE; i < EXTPHYSMEM/PGSIZE; i++) { //3) The IO hole [IOPHYSMEM, EXTPHYSMEM), which must never be allocated.
        pages[i].pp_ref = 1;
    }
    for (i = EXTPHYSMEM/PGSIZE; i < npages; i++) {
        if (i*PGSIZE < (PADDR(boot_alloc(0)))) {
            pages[i].pp_ref = 1;
        } else {
            pages[i].pp_ref = 0;
            pages[i].pp_link = page_free_list;
            page_free_list = &pages[i];
        }
    }
For the fourth part, we use boot_alloc(0) to get the next free kernel virtual memory address. Any memory before this address should already be allocated.

4. page_alloc()
struct PageInfo *result;
    if (page_free_list == NULL) // If no enough memory, return NULL.
        return NULL;

    result = page_free_list;
    page_free_list = page_free_list->pp_link; 
    result->pp_link = NULL; // Be sure to set the pp_link field of the allocated page to NULL so page_free can check for double-free bugs.
    if (alloc_flags & ALLOC_ZERO) // If (alloc_flags & ALLOC_ZERO), fills the entire returned physical page with '\0' bytes.
        memset(result, '\0', sizeof(struct PageInfo));
	return result;

	
5. page_free()
if (pp->pp_ref != 0 || pp->pp_link != NULL) // You may want to panic if pp->pp_ref is nonzero or pp->pp_link is not NULL.
        panic("Page free panics");
    pp->pp_link = page_free_list; // Return a page to the free list.
    page_free_list = pp;


## Exercise 3. While GDB can only access QEMU's memory by virtual address, it's often useful to be able to inspect physical memory while setting up virtual memory. Review the QEMU monitor commands from the lab tools guide, especially the xp command, which lets you inspect physical memory. To access the QEMU monitor, press Ctrl-a c in the terminal (the same binding returns to the serial console). 

Note: If your QEMU can only switch to parallel console using hotkey (Ctrl + Shift + 2). Make sure to set the QEMUOPTS in GDBmakeFile:(around 143) to the code shown below.
=======================
QEMUOPTS = -drive file=$(OBJDIR)/kern/kernel.img,index=0,media=disk,format=raw -serial mon:stdio -gdb tcp::$(GDBPORT) >>>>changes to "QEMUOPTS = -drive file=$(OBJDIR)/kern/kernel.img,index=0,media=disk,format=raw -serial stdio -gdb tcp::$(GDBPORT)"
And !!!remember to set it back when you are doing the make grade.

Set a breakpoint at 0xf0100039(Any point after paging is enable) in gdb.
(qemu) xp/4 0x10000c
000000000010000c: 0x7205c766 0x34000004 0xa000b812 0x220f0011

(gdb) x/4x 0xf010000c
0xf010000c <entry>:	0x7205c766	0x34000004	0xa000b812	0x220f0011
QEMU and gdb prints the same content at the corresponding physical and virtual address.

Question

Assuming that the following JOS kernel code is correct, what type should variable x have, uintptr_t or physaddr_t?
	mystery_t x;
	char* value = return_a_pointer();
	*value = 10;
	x = (mystery_t) value;
x should be uintptr_t because it is a raw memory address, there is no address translation and it can't be a physical address. 


## Exercise 4. In the file kern/pmap.c, you must implement code for the following functions.

1. pgdir_walk()
pte_t *
pgdir_walk(pde_t *pgdir, const void *va, int create)
{
    pde_t *pde = pgdir + PDX(va); // THe address of the page directory entry(pde)
    uint32_t *PT_ADDR;

    if ((*pde & PTE_P) == 0) { // Test whether the page table page pde points to is present
        if (create == false) //If cannot create then return NULL
            return NULL;
        struct PageInfo *pp;
        if ((pp = page_alloc(ALLOC_ZERO)) == NULL) //Else, allocate a new page 
            return NULL;
        pp->pp_ref += 1;
        *pde = (page2pa(pp) | PTE_P | PTE_U | PTE_W); //Put this new Page inside the page directory entry
    }

    PT_ADDR = KADDR(PTE_ADDR(*pde)); //PTE_ADDR Musk out the flag bits and KADDR turn it to a kernel virtual address
	return (pde_t *) (PT_ADDR + PTX(va)); // PT_ADDR + PTX(va) will be the page table entry address based on mmu.h.
}

2. page_lookup()
struct PageInfo *
page_lookup(pde_t *pgdir, void *va, pte_t **pte_store)
{
    pde_t *pte;

    if ((pte = pgdir_walk(pgdir, va, false)) == NULL) { //This step is to get the page table entry at our va.
        return NULL;
    }
    if (pte_store) { 
        *pte_store = pte; // Store the page table entry
    }
	return pa2page(*pte); //Since *pte is a physical address, we need to convert it to get the real page.
}

3. page_remove()
void
page_remove(pde_t *pgdir, void *va)
{
    struct PageInfo *pp;
    pde_t *pte;
    if ((pp = page_lookup(pgdir, va, &pte)) == NULL) { //Get the page to be removed.
        return;
    }
    page_decref(pp); //decrement the page's ref and free it if ref <= 0
    *pte = 0; //clear the page table entry
    tlb_invalidate(pgdir, va);
}

4. page_insert()
int
page_insert(pde_t *pgdir, struct PageInfo *pp, void *va, int perm)
{
    pde_t *pte;
	if ((pte = pgdir_walk(pgdir, va, true)) == NULL) { //If no memory, return -E_NO_MEN
        return -E_NO_MEM;
    }
    if (*pte & PTE_P) { //Remove the current page if it is exist.
        page_remove(pgdir, va);
    }
    *pte = page2pa(pp) | PTE_P | perm; //Put the "to be inserted page" into the page table entry.
    pp->pp_ref += 1; //Increment the ref by 1.
    return 0;
}

5. boot_map_region
static void
boot_map_region(pde_t *pgdir, uintptr_t va, size_t size, physaddr_t pa, int perm)
{
    pde_t *pte;
    int i;
    for (i = 0; i < size/PGSIZE; i++) {
        if ((pte = pgdir_walk(pgdir, (const void*)va, true)) == NULL) { // Since pa and va are linear and the size is PGSIZE aligned. we don't need to roundup or down.
            panic("The allocation failed");
        }
        *pte = pa | perm | PTE_P; // For each virtual address pte, put the physical address in it.
        va += PGSIZE; // Jump to next page
        pa += PGSIZE; // Jump to next page
    }
}

After implementing all these functions and run "make qemu", you will realise one bug at around p.map:774.
	// pp2 should NOT be on the free list
	// could happen in ref counts are handled sloppily in page_insert
	assert(!page_alloc(0));
This is due to inserting the same page. 
int
page_insert(pde_t *pgdir, struct PageInfo *pp, void *va, int perm)
{
    pde_t *pte;
	if ((pte = pgdir_walk(pgdir, va, true)) == NULL) { //When we are inserting the same page, this page is already in the page table entry *pte.
        return -E_NO_MEM;
    }
    if (*pte & PTE_P) { // Hence, the presence bit is exist.
        page_remove(pgdir, va); //If the page only have 1 ref, the page_insert will remove and free the page.
    }
    *pte = page2pa(pp) | PTE_P | perm; //When we insert the new page into page table entry, we didn't allocate a page.
    pp->pp_ref += 1; //Increment the ref by 1.
    return 0;
}
As a result, the number of pages on free_list will increase by one which shouldn't be. To address this problem, we can just increment the ref count before removing that page. The page_insert should be change to 
page_insert(pde_t *pgdir, struct PageInfo *pp, void *va, int perm)
{
    pde_t *pte;
	if ((pte = pgdir_walk(pgdir, va, true)) == NULL) {
        return -E_NO_MEM;
    }

    pp->pp_ref += 1;
    if (*pte & PTE_P) {
        page_remove(pgdir, va);
    }
    *pte = page2pa(pp) | PTE_P | perm;
    return 0;
}


## Exercise 5. Fill in the missing code in mem_init() after the call to check_page().

1.
// Map 'pages' read-only by the user at linear address UPAGES
	// Permissions:
	//    - the new image at UPAGES -- kernel R, user R
	//      (ie. perm = PTE_U | PTE_P)
	//    - pages itself -- kernel RW, user NONE
	// Your code goes here:
    boot_map_region(kern_pgdir, UPAGES, PTSIZE, PADDR(pages), PTE_U | PTE_P);
- According to inc/memlayout.h, the region size of UPAGES is 0x400000 which is PTSIZE.

2. 
// Use the physical memory that 'bootstack' refers to as the kernel
	// stack.  The kernel stack grows down from virtual address KSTACKTOP.
	// We consider the entire range from [KSTACKTOP-PTSIZE, KSTACKTOP)
	// to be the kernel stack, but break this into two pieces:
	//     * [KSTACKTOP-KSTKSIZE, KSTACKTOP) -- backed by physical memory
	//     * [KSTACKTOP-PTSIZE, KSTACKTOP-KSTKSIZE) -- not backed; so if
	//       the kernel overflows its stack, it will fault rather than
	//       overwrite memory.  Known as a "guard page".
	//     Permissions: kernel RW, user NONE
	// Your code goes here:
    boot_map_region(kern_pgdir, KSTACKTOP-KSTKSIZE, KSTKSIZE, PADDR(bootstack), PTE_W | PTE_P);
- Since the [KSTACKTOP-PTSIZE, KSTACKTOP-KSTKSIZE) is not backed. we should only map [KSTACKTOP-KSTKSIZE, KSTACKTOP)

3.
// Map all of physical memory at KERNBASE.
	// Ie.  the VA range [KERNBASE, 2^32) should map to
	//      the PA range [0, 2^32 - KERNBASE)
	// We might not have 2^32 - KERNBASE bytes of physical memory, but
	// we just set up the mapping anyway.
	// Permissions: kernel RW, user NONE
	// Your code goes here:
    boot_map_region(kern_pgdir, KERNBASE, ~0 - KERNBASE, 0, PTE_W | PTE_P);


### Question
2. What entries (rows) in the page directory have been filled in at this point? What addresses do they map and where do they point? In other words, fill out this table as much as possible:

Entry	Base Virtual Address	Points to (logically):
1023	0xffc00000	Page table for top 4MB of phys memory
1022	0xff800000	Page table for second 4mb of phys memory
960	0xefc00000	Page table for first 4mb of phys memory
959     0xefc00000      First 8 PDES are for bootstack
957	0xef400000	page directory
956	0xef000000	page data structure
2	0x00800000	empty memory
1	0x00400000	empty memory
0	0x00000000	empty memory

3. We have placed the kernel and user environment in the same address space. Why will user programs not be able to read or write the kernel's memory? What specific mechanisms protect the kernel memory?

Due to protection bits like PTE_U, user program cannot access data without PTE_U. This mechanisms is called read and write permission.

4. What is the maximum amount of physical memory that this operating system can support? Why?

In JOS, the space for storing pageinfo pages is 0x400000(1 PTSIZE) at UPAGES. Since the size of Pageinfo is 8 bytes. By right JOS system can have 0x400000/0x8 ~= 0x80000(524288) pages and have control over 2147483648bytes (each page control 4096 bytes) which is around 2GB. In fact, when I put -m 2.0G at the end of GNUMAKEFILE:143 and "make qemu", the gdb reports fault at memset.

Program received signal SIGTRAP, Trace/breakpoint trap.
The target architecture is set to "i386".
=> 0xf0103846 <memset+60>:	rep stos %eax,%es:(%edi)
0xf0103846 in memset (v=0xf0121000, c=0, n=4194304) at lib/string.c:131
131			asm volatile("cld; rep stosl\n"

The related function step is shown below. When we do memset for pageInfo struct.

// Allocate an array of npages 'struct PageInfo's and store it in 'pages'.
	// The kernel uses this array to keep track of physical pages: for
	// each physical page, there is a corresponding struct PageInfo in this
	// array.  'npages' is the number of physical pages in memory.  Use memset
	// to initialize all fields of each struct PageInfo to 0.
	// Your code goes here:
    pages = boot_alloc(npages * sizeof(struct PageInfo));
    memset(pages, 0, npages * sizeof(struct PageInfo));

I think the reason is that at this stage, we only mapped the first 4mb physical memory and that's all we have(phys: 0x0~0x400000 -> virt: 0xf0000000~0xf0400000). We use bootAlloc(0) to get the amount of free physical memory we have(0xf0400000 - bootAlloc).
In my case, bootAlloc(0) get me 0xf0112000. So the free physical memory is 0x2EE000, the memory before 0xf0112000 could be occupied by the bios and something else I am not sure. Hence, the free space for struct pageinfo is just 0x2EE000 and we can only have 0x2EE000/8 number of pages which is 5DC00(384000). The maximum memory my system can support is 1.57gb(384000 * 4096). 

5. How much space overhead is there for managing memory, if we actually had the maximum amount of physical memory? How is this overhead broken down?

The maximum space for storing page table pages is 0x400000(1 PTSIZE) and it can store 0x80000(0x400000/8) pages. Since 1 page directory entry size is 0x400000, it can contain 0x400 pages. So we should have 0x200(0x80000/0x400) pages for containing all the pages. The memory required for page directory entrys is 0x200 * 8 = 4096(4KB).
We totally need 4MB + 4KB.

6. Revisit the page table setup in kern/entry.S and kern/entrypgdir.c. Immediately after we turn on paging, EIP is still a low number (a little over 1MB). At what point do we transition to running at an EIP above KERNBASE? What makes it possible for us to continue executing at a low EIP between when we enable paging and when we begin running at an EIP above KERNBASE? Why is this transition necessary?

This is because entrypgdir has already mapped the first 4mb of virtual memory to physical memory. It's important to transit to 





