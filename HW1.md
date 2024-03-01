# HW1 report


## Exercise: What is on the stack?

###1. Begin by restarting qemu and gdb, and set a break-point at 0x7c00, the start of the boot block (bootasm.S). Single step through the instructions (type si at the gdb prompt). Where in bootasm.S is the stack pointer initialized? (Single step until you see an instruction that moves a value into %esp, the register for the stack pointer.)

We can just search for keyword esp in bootasm.S. The first result shows the initialization of esp after setting up the protected-mode data segment registers. In this step, the value of esp is setting to 7c00.

.code32  # Tell assembler to generate 32-bit code now.
  # Set up the protected-mode data segment registers
  # Set up the stack pointer and call into C.
  movl    $start, %esp
    7c43:	bc 00 7c 00 00       	mov    $0x7c00,%esp

###2. Single step through the call to bootmain; what is on the stack now?

- Right before calling bootmain, the esp is still at 7c00(initial point)
The target architecture is set to "i386".
=> 0x7c48:	call   0x7d2a<bootmain>

Thread 1 hit Breakpoint 1, 0x00007c48 in ?? ()
(gdb) p $esp
$1 = (void *) 0x7c00
(gdb) 

- After the call to bootmain, esp is decremeted by 4 and the return address of function bootmain() is stored on the stack pointer now(7c4d).
(gdb) si
=> 0x7d2a:	push   %ebp
0x00007d2a in ?? ()
(gdb) p $esp
$2 = (void *) 0x7bfc
(gdb) x/2x $esp
0x7bfc:	0x00007c4d	0x8ec031fa

###3. What do the first assembly instructions of bootmain do to the stack? Look for bootmain in bootblock.asm.

The first assembly instuction of bootmain is pushing the ebp.
void
bootmain(void)
{
    7d2a:	55                   	push   %ebp
ebp is known as the base pointer or the frame pointer, once enter a function, the ebp is pushed on the stack and the value of the current stack is copyed to the ebp so that the function can return to the stack after execution.
(gdb) p $esp
$3 = (void *) 0x7bf8
(gdb) x/3x $esp
0x7bf8:	
0x00000000	current ebp
0x00007c4d	return addr of bootmain()
0x8ec031fa	initial point of stack pointer

###4. Continue tracing via gdb (using breakpoints if necessary -- see hint below) and look for the call that changes eip to 0x10000c. What does that call do to the stack? (Hint: Think about what this call is trying to accomplish in the boot sequence and try to identify this point in bootmain.c, and the corresponding instruction in the bootmain code in bootblock.asm. This might help you set suitable breakpoints to speed things up.)

Firstly, we cannot find step address 0x10000c in bootblock.asm which means 0x10000c might not exist in bootmain.c and it is in another file that gets called by bootmain.c. Hence, it could be the entry() that gets called at the end of the bootmain function. From bootlock.asm, we can see that the step address of entry() is at 7db2 so we make a breakpoint here and continue to this point.
(gdb) b *0x7db2
Breakpoint 4 at 0x7db2
(gdb) c
Continuing.
=> 0x7db2:	call   *0x10018

Thread 1 hit Breakpoint 4, 0x00007db2 in ?? ()
(gdb) si
=> 0x10000c:	mov    %cr4,%eax
0x0010000c in ?? ()
luckily, the next step after 7db2 is 0x10000c which means *0x10018 is 10000c. This call decremented the stack pointer by 4 and put the return address of entry() in this memory address.

(gdb) info register
eax            0x0                 0
ecx            0x0                 0
edx            0x1f0               496
ebx            0x10074             65652
esp            0x7bbc              0x7bbc
ebp            0x7bf8              0x7bf8
esi            0x107000            1077248
edi            0x1144a8            1131688
eip            0x10000c            0x10000c
eflags         0x46                [ PF ZF ]
cs             0x8                 8
ss             0x10                16
ds             0x10                16
es             0x10                16
fs             0x0                 0
gs             0x0                 0

after step 10000c
(gdb) x/24x $esp
0x7bbc:	0x00007db8	0x00107000	0x00002516	0x00008000 
0x7bcc:	0x00000000	0x00000000	0x00000000	0x00000000
0x7bdc:	0x00010074	0x00000000	0x00000000	0x00000000
0x7bec:	0x00000000	0x00000000	0x00000000	0x00000000
0x7bfc:	0x00007c4d	0x8ec031fa	0x8ec08ed8	0xa864e4d0
0x7c0c:	0xb0fa7502	0xe464e6d1	0x7502a864	0xe6dfb0fa
Only 7bbc to 7c00 are valid stacks excluding the zero values.
- 7db8 is the retern address of entry()
- 107000 is the esi register(from info register)
- 10074 is the ebx register
- 7bf8 is the old ebp register(from question 3)


