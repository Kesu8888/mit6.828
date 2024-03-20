# xv6 chapter 3 notes


## X86 protection

- The x86 has 4 protection levels, numbered 0 (most privilege) to 3 (least privilege). In practice, most operating systems use only 2 levels: 0 and 3, which are then called kernel mode and user mode, respectively

To make a system call on the x86, a program invokes the int n instruction, where n specifies the index into the IDT. The int instruction performs the following steps:
• Fetch the n’th descriptor from the IDT, where n is the argument of int.
• Check that CPL in %cs is <= DPL, where DPL is the privilege level in the descriptor.
• Save %esp and %ss in CPU-internal registers, but only if the target segment selector’s PL < CPL.
• Load %ss and %esp from a task segment descriptor.
• Push %ss.
• Push %esp.
• Push %eflags.
• Push %cs.
• Push %eip.
• Clear the IF bit in %eflags, but only on an interrupt.
• Set %cs and %eip to the values in the descriptor.


## Code: Assembly trap handlers

X86 allows for 256 different interrupts:
0~31 are difined for software interrupts such as divide error or attemps to access invalid memory address
32~63 are defined for hardware interrupts
64 is system call interrupt

1. Code:
*<Tvinit>: called from main, sets up the 256 entries in the table idt. Interrupt i is handled by the code at the address in vectors[i]. Each entry point is different, because the x86 does not provide the trap number to the interrupt handler. Using 256 different handlers is the only way to distinguish the 256 cases.
Tvinit handles T_SYSCALL, the user system call trap, specially: it specifies that the gate is of type ‘‘trap’’ by passing a value of 1 as second argument. Trap gates don’t clear the IF flag, allowing other interrupts during the system call handler.

*<DPL_USER>: sets with system call gate privilege, it allows a user program to generate a trap with an explicit int instruction. xv6 doesn’t allow processes to raise other interrupts (e.g., device interrupts) with int

2. Trap handling in user mode:
When changing protection level from user mode to kernel mode, the kernel shouldn't use the stack of the user process, because it may be invalid. Xv6 programs the x86 hardware to perform a stack switch on a trap by setting up a task segment descriptor through which the hardware loads a stack segment selector and a new value for %esp. The function
switchuvm (1860) stores the address of the top of the kernel stack of the user process
into the task segment descriptor.

3. When a trap occurs, the processor hardware :
- loads %esp and %ss from the task segment descriptor, pushes the old user %esp and %ss onto the new stack if it is executing in user mode.
- pushes the %eflags, %cs and %eip registers(For some traps, the processor even pushes an error word.). then loads %eip and %cs from the relevent IDT entry.

xv6 uses a Perl script to generate the entry points that the IDT entries point to. Each entry pushes an error code if the processor didn’t, pushes the interrupt number, and then jumps to alltraps. Alltraps continues to save processor registers: it pushes %ds, %es, %fs, %gs, and the general-purpose registers. The result is that now kernel stack contains a struct trapframe containing the processor registers at the time of the trap.[havephoto_trapframe] The The trap frame contains all the neccessary informations to restore the user program registers when the kernel returns to the current process, so that the process can continue exactly as it was when the trap started.

Trap handling in kernel mode:
The hardware does not switch stacks or save the stack pointer or stack segment selector; otherwise the same steps occur as in traps from user mode, and the same xv6 trap handling code executes. When iret later restores a kernel mode %cs, the processor continues executing in kernel mode.


## Code: C trap handler

After receiving the trap call, trap looks at the hardware trap number tf->trapno to decide what happens before and what needs to be done. It first checks whether a system call, if not it checks whether a hardware requires attention. If also not hardware trap, then the trap will assume this trap is caused by incorrect behavior.(e.g., divide by zero) If the process caused the trap a user process, the system will kill sets proc->killed to clean the user process. If it was the kernel running, there must be a kernel bug: trap prints details about the surprise and then calls panic.


## Code: System call

System calls conventionally return negative numbers to indicate errors, positive numbers for success. If the system call number is invalid, syscall prints an error and returns –1.

*<argint> calls fetchint to read the value at that address from user memory and write it to *ip. 

*<argptr> fetches the nth system call argument and checks that this argument is a valid user-space pointer. 

*<argstr> interprets the nth argument as a pointer. It ensures that the pointer points at a NUL-terminated string and that the complete string is located below the end of the user part of the address space.

*<argfd> uses argint to retrieve a file descriptor number, checks if it is valid file descriptor, and returns the corresponding struct file.


## Code: interrupts

Devices on the motherboard can generate interrupts, and xv6 must set up the hardware to handle these interrupts. Interrupts are usually optional in the sense that the kernel could instead periodically check the device hardware for new events.

Interrupts are similar to system call, except devices generate them at any time. There is hardware on the motherboard to signal the CPU when a device needs attention (e.g., the user has typed a character on the keyboard). 

Timer hardware is a device that generates a signal at a equal time intervial(e.g., 100 times per second) With this, the kernel can time-slice among multiple running processes.

XV6 is designed for multi-processor board. It ignores the interrupts from the PIC(interrupt controller for single CPU board), and configures the IOAPIC and local APIC.

*The IO APIC has a table and the processor can program entries in the table through memory-mapped I/O. During initialization, xv6 programs to map interrupt 0 to IRQ 0, and so on, but disables them all. Specific devices enable particular interrupts and say to which processor the interrupt should be routed. For example, xv6 routes keyboard interrupts to processor 0.

*The timer chip is inside the LAPIC, so that each processor can receive timer interrupts independently. Xv6 sets it up in lapicinit(7408). The key line is the one that programs the timer. This line tells the LAPIC to periodically generate an interrupt at IRQ_TIMER, which is IRQ 0. Line (7451) enables interrupts on a CPU’s LAPIC, which will cause it to deliver interrupts to the local processor.

To control that certain code fragments are not interrupted, xv6 disables interrupts during these code fragments (e.g., see switchuvm (1860)).

1. Drivers:

A driver is the code in an operating system that manages a particular device: it tells the device hardware to perform operations, configure the device to generate interrupts when done, and handles the resulting interrupts.

Disk drive: Disk driver copies data from and back to the disk. Disk hardware presents the data on the disk as a numbered sequence of 512-byte blocks(also called sectors). sector 0 is the first 512 bytes, sector 1 is the next, and so on. The block and sector size can vary with different system but usually the block size is the multiple of sector size. Xv6’s block size is identical to the disk’s sector size. The block is presented as the struct 'buf'. The data stored in this structure is often out of sync with the disk: it might have not yet been read in from disk (the disk is working on it but hasn’t returned the sector’s content yet), or it might have been updated but not yet written out. The driver must ensure that the rest of xv6 doesn’t get confused when the structure is out of sync with the disk.


## Code: Disk driver

IDE device provide access to disks connected to the PC standard IDE controller(disk port I think). IDE is now falling out of fashion in favor of SCSI and SATA. XV6 represent file system blocks using struct buf. BSIZE, block size (4055) is identical to the IDE’s sector size and thus each buffer represents the contents of one sector on a particular disk device.

The <dev> and <sector> fields give the device and sector number. the <data> field is an in-memory copy of the disk sector. Although the BSIZE for xv6 is identical to the IDE's selector size, but systems like to use bigger BSIZE to increase the data transfering throuput.

<flags> track the relationship between memory and disk: the <B_VALID> flag means that data has been read in, and the <B_DIRTY> flag means the data needs to be written out.

- The kernel initializes the disk driver at boot time by calling <ideinit> (4251) from main (1232). Ideinit calls <ioapicenable> to enable the <IDE_IRQ> interrupt (4256). The call to ioapicenable enables the interrupt only on the last CPU (ncpu-1): on a two-processor system, CPU 1 handles disk interrupts.

- Next, <ideinit> probes the disk hardware. It begins by calling <idewait> (4257) to wait for the disk to be able to accept commands. A PC motherboard presents the status bits of the disk hardware on I/O port <0x1f7>. <Idewait> (4238) polls the status bits until the busy bit (IDE_BSY) is clear and the ready bit (IDE_DRDY) is set.

- Now that the disk controller is ready, ideinit can check how many disks are present. It assumes that disk 0 is present, because the boot loader and the kernel were both loaded from disk 0, but it must check for disk 1. It writes to I/O port 0x1f6 to select disk 1 and then waits a while for the status bit to show that the disk is ready (4259-4266). If not, ideinit assumes the disk is absent.

- After <ideinit>, the disk is not used again until the buffer cache calls <iderw>, which updates a locked buffer as indicated by the flags. If <B_DIRTY> is set, <iderw> writes the buffer to the disk; if <B_VALID> is not set, <iderw> reads the buffer from the disk.

- Iderw takes this latter approach, keeping the list of pending disk requests in a queue and using interrupts to find out when each request has finished. Iderw (4354) adds the buffer b to the end of the queue (4367-4371). If the buffer is at the front of the queue, iderw must send it to the disk hardware by calling idestart (4326-4328); otherwise the buffer will be started once the buffers ahead of it are taken care of.

- Idestart (4274) issues either a read or a write for the buffer’s device and sector, according to the flags. If the operation is a write, idestart must supply the data now (4296). idestart moves the data to a buffer in the disk controller using the outsl instruction; using CPU instructions to move data to/from device hardware is called programmed I/O. Eventually the disk hardware will raise an interrupt to signal that the data has been written to disk. 

- Eventually, the disk will finish its operation and trigger an interrupt. trap will call ideintr to handle it (3424). Ideintr (4304) consults the first buffer in the queue to find out which operation was happening. If the buffer was being read and the disk controller has data waiting, ideintr reads the data from a buffer in the disk controller into memory with insl (4317-4319). Now the buffer is ready: ideintr sets B_VALID, clears B_DIRTY, and wakes up any process sleeping on the buffer (4321-4324). Finally, ideintr must pass the next waiting buffer to the disk (4326-4328).




















