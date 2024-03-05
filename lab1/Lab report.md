# Lab1 report


## Exercise 2. Use GDB's si (Step Instruction) command to trace into the ROM BIOS for a few more instructions, and try to guess what it might be doing. You might want to look at Phil Storrs I/O Ports Description, as well as other materials on the 6.828 reference materials page. No need to figure out all the details - just the general idea of what the BIOS is doing first.

This is the start of make qemu-gdb
The target architecture is set to "i8086".
[f000:fff0]    0xffff0:	ljmp   $0xf000,$0xe05b
0x0000fff0 in ?? ()
+ symbol-file obj/kern/kernel
(gdb) 

now we trace for a few more instructions using si
(gdb) si
[f000:e05b]    0xfe05b:	cmpl   $0x0,%cs:0x6ac8 compare
0x0000e05b in ?? ()
(gdb) si
[f000:e062]    0xfe062:	jne    0xfd2e1 jump to fd2e1 if %cs:0x6ac8 not equals to 0
0x0000e062 in ?? ()
(gdb) si
[f000:e066]    0xfe066:	xor    %dx,%dx ~dx
0x0000e066 in ?? ()
(gdb) si
[f000:e068]    0xfe068:	mov    %dx,%ss move value from dx to ss register
0x0000e068 in ?? ()
(gdb) si
[f000:e06a]    0xfe06a:	mov    $0x7000,%esp move value 0x7000 esp register
0x0000e06a in ?? ()
(gdb) si
[f000:e070]    0xfe070:	mov    $0xf34c2,%edx move value 0xf34c2 to edx register
0x0000e070 in ?? ()
(gdb) si
[f000:e076]    0xfe076:	jmp    0xfd15c jump to address 0xfd15c
0x0000e076 in ?? ()
(gdb) si
[f000:d15c]    0xfd15c:	mov    %eax,%ecx move instruction
0x0000d15c in ?? ()
(gdb) si
[f000:d15f]    0xfd15f:	cli clear interupt flag
0x0000d15f in ?? ()
(gdb) si
[f000:d160]    0xfd160:	cld clear clear direction flag
0x0000d160 in ?? ()
we set a break point at 7c00 the beginning of boot loader.
after we continue to this breakpoint, we see a few lines appeared on the qemu screen.
"SeaBIOS...
iPXE...
Booting from dist..."
so we know that the pc is running Seabios for the first step.
bios is used to initilize the pc hardwire e.g., vga, audio, keyboard etc., at early stage and ensures the operating system can be correctly loaded into memory.

## Exercise 3:

- At what point does the processor start executing 32-bit code? What exactly causes the switch from 16- to 32-bit mode?
from boot.asm, ths piece of code below switches the processor to 32-bit mode
/# Jump to next instruction, but in 32-bit code segment.
  # Switches processor into 32-bit mode.
  ljmp    $PROT_MODE_CSEG, $protcseg
    7c2d:	ea 32 7c 08 00 66 b8 	ljmp   $0xb866,$0x87c32

- What is the last instruction of the boot loader executed, and what is the first instruction of the kernel it just loaded?
The piece of code below is the last instruction of the boot loader
	// call the entry point from the ELF header
	// note: does not return!
	((void (*)(void)) (ELFHDR->e_entry))();
    7d63:	ff 15 18 00 01 00    	call   *0x10018
From kernel.asm, the first instruction it just loaded is shown below.
f0100000:	02 b0 ad 1b 00 00    	add    0x1bad(%eax),%dh

- Where is the first instruction of the kernel?
The first instruction of the kernel is shown in kernel.asm
f0100000:	02 b0 ad 1b 00 00    	add    0x1bad(%eax),%dh

- How does the boot loader decide how many sectors it must read in order to fetch the entire kernel from disk? Where does it find this information?
The boot loader decides the number of sectors using the ELFHDR->e_phnum.

## Exercise 5: Trace through the first few instructions of the boot loader again and identify the first instruction that would "break" or otherwise do the wrong thing if you were to get the boot loader's link address wrong. Then change the link address in boot/Makefrag to something wrong, run make clean, recompile the lab with make, and trace into the boot loader again to see what happens. Don't forget to change the link address back and make clean again afterward!

The link address in boot/Makefrag is 7c00 as in obj/boot/boot.asm displays so. We can change this value in Makefrag to any number like 0x1111.
$(OBJDIR)/boot/boot: $(BOOT_OBJS)
	@echo + ld boot/boot
	$(V)$(LD) $(LDFLAGS) -N -e start -Ttext 0x7C00 -o $@.out $^
	
Before changing the value, we step through a few instructions as shown below.
[   0:7c00] => 0x7c00:	cli 
[   0:7c02] => 0x7c02:	xor    %ax,%ax
[   0:7c04] => 0x7c04:	mov    %ax,%ds
[   0:7c06] => 0x7c06:	mov    %ax,%es
[   0:7c08] => 0x7c08:	mov    %ax,%ss
According to boot.asm, the protected mode enable flag is explicitly set before the first instruction 7c00: cli.
00007c00 <start>:
.set CR0_PE_ON,      0x1         # protected mode enable flag

After changing the value, the first two lines in boot.asm is shown below. The first few insturctions become nop which means it does nothing and the protected mode enable flag might not be successfully set on.
[   0:7c01] => 0x7c01:	nop
[   0:7c02] => 0x7c02:	nop
[   0:7c03] => 0x7c03:	cli
[   0:7c04] => 0x7c04:	cld
[   0:7c05] => 0x7c05:	xor    %ax,%ax

After stepping through a few instructions, the qemu shows a triple fault after instruction 7c30.
[   0:7c30] => 0x7c30:	ljmp   $0x8,$0x1146
According to boot.asm, this step is to switches the system to 32-bit protected mode. Since the flag is not successfully set on in the previous steps, the triple fault is therefore displayed.

## Exercise 6: Reset the machine (exit QEMU/GDB and start them again). Examine the 8 words of memory at 0x00100000 at the point the BIOS enters the boot loader, and then again at the point the boot loader enters the kernel. Why are they different? What is there at the second breakpoint? (You do not really need to use QEMU to answer this question. Just think.)

We set two breakpoints at *0x7c00(boot loader) and 0x100000(kernel).
Continue to step address *0x7c00, the next 8 values from memory address 0x100000 is shown below. The values are all 0, which means these values have not been set yet.
(gdb) x/8x 0x100000
0x100000:	0x00000000	0x00000000	0x00000000	0x00000000
0x100010:	0x00000000	0x00000000	0x00000000	0x00000000
Continue to step address *0x10000c, the next 8 values from memory address 0x100000 is shown below. The values are assigned.
(gdb) x/8x 0x100000
0x100000:	0x1badb002	0x00000000	0xe4524ffe	0x7205c766
0x100010:	0x34000004	0x7000b812	0x220f0011	0xc0200fd8
The values at 0x100000 are the instructions for the kernel setup. Before running into boot loader(*0x7c00), the instructions for kernel has not been stored in 0x100000. Right before calling kernel(*0x100000), the instructions has stored in the 0x100000 locations so the kernel setup can be called afterwards.

## Exercise 7: Use QEMU and GDB to trace into the JOS kernel and stop at the movl %eax, %cr0. Examine memory at 0x00100000 and at 0xf0100000. Now, single step over that instruction using the stepi GDB command. Again, examine memory at 0x00100000 and at 0xf0100000. Make sure you understand what just happened.

From the boot.asm, the step address of movl %eax, %cr0 is at 0x100025(Although the boot.asm may display 0xf0100025 which is the virtual memory, but the system assume no enough physical memory so it map 0xf0100025 to physical memory 0x100025, minus 0xf0000000). We make a breakpoint at this point. And continue to this breakpoint. The next 10 values from memory address 0x100000 and 0xf0100000 are shown below.
(gdb) x/8x 0x100000
0x100000:	0x1badb002	0x00000000	0xe4524ffe	0x7205c766
0x100010:	0x34000004	0x7000b812	0x220f0011	0xc0200fd8
(gdb) x/8x 0xf0100000
0xf0100000 <_start-268435468>:	0x00000000	0x00000000	0x00000000	0x00000000
0xf0100010 <entry+4>:	0x00000000	0x00000000	0x00000000	0x00000000

si the current intructions "0x100025:	mov    %eax,%cr0", and display the next 8 values at these memory address again.
(gdb) x/8x 0x100000
0x100000:	0x1badb002	0x00000000	0xe4524ffe	0x7205c766
0x100010:	0x34000004	0x7000b812	0x220f0011	0xc0200fd8
(gdb) x/8x 0xf0100000
0xf0100000 <_start-268435468>:	0x1badb002	0x00000000	0xe4524ffe	0x7205c766
0xf0100010 <entry+4>:	0x34000004	0x7000b812	0x220f0011	0xc0200fd8

We can see that the value at f0100000(high virtual address) is the same as physical address 0x100000. The reason is that in the previous steps, flags bits PE(protected mode enable), PG(paging) and WP(Write protect) have been loaded into %eax and in last step, these flags are loaded into cr0. 
orl	$(CR0_PE|CR0_PG|CR0_WP), %eax

Before this instruction executed, memory at 0x00100000 points to the kernel while memory at 0xf0100000 exceeds physical memory's size and the memory there is filled with 0. After it is executed, the MMU starts working. Now memory at 0x00100000 and at 0xf0100000 points to the same physical memory where the kernel is located. So their content is the same.

Question: What is the first instruction after the new mapping is established that would fail to work properly if the mapping weren't in place? Comment out the movl %eax, %cr0 in kern/entry.S, trace into it, and see if you were right.

If the mappint weren't in place, the first instruction that will failed is f0100028 in boot.asm as the virtual address 0xf010002f does not exist(exceeding physical memory).
f0100028:	b8 2f 00 10 f0       	mov    $0xf010002f,%eax
	jmp	*%eax
f010002d:	ff e0                	jmp    *%eax

Comment out the f0100025:	0f 22 c0             	mov    %eax,%cr0 in kern/kernel.S and run qemu-gdb again. Set a breakpoint at 0x100020. We si a few instructions, and the gdb reports nuke frame pointer after jmp *%eax instruction and then qemu closes which justifies our assumption.
=> 0x10002a:	jmp    *%eax
0x0010002a in ?? ()
(gdb) si
=> 0xf010002c <relocated>:	add    %al,(%eax)
relocated () at kern/entry.S:74
74		movl	$0x0,%ebp			# nuke frame pointer

## Exercise 8. We have omitted a small fragment of code - the code necessary to print octal numbers using patterns of the form "%o". Find and fill in this code fragment.

The statement to print octal numbers is cprintf("6828 decimal is %o octal!\n", 6828); in kern/init.c. cprintf is a function in kern/printf.c. below is the code for cprintf()
	va_list ap;
	int cnt;

	va_start(ap, fmt);
	cnt = vcprintf(fmt, ap);
	va_end(ap);

	return cnt;
Cprintf calls vcprintf and vcprintf called vprintfmt in lib/printfmt.c. The format code for octal number is %o, so we search for character 'o' in vprintfmt.c. Below is the code
		case 'o':
			// Replace this with your code.
			putch('X', putdat);
			putch('X', putdat);
			putch('X', putdat);
			break;
Above these piece of code, we see the code to print numbers in decimal.
			case 'u':
			num = getuint(&ap, lflag);
			base = 10;
			goto number;
So we just copy this code into case 'o' and change the base number = 8;

question 1: Explain the interface between printf.c and console.c. Specifically, what function does console.c export? How is this function used by printf.c?

console.c controls the display resources lend it to prinf.c. console.c export the function cputchar(). The function putch() in printf.c calls the cputchar() in console.c to print each character it received.

question 2: Explain the following from console.c:
1      if (crt_pos >= CRT_SIZE) { // crt_pos is the position of cursor to display the next character. If it is bigger than the size of the screen
2              int i;             // which means no more space to put character, it will delete some content on the screen.
3              memmove(crt_buf, crt_buf + CRT_COLS, (CRT_SIZE - CRT_COLS) * sizeof(uint16_t)); // memmove removes 1 column from the screen.
4              for (i = CRT_SIZE - CRT_COLS; i < CRT_SIZE; i++)
5                      crt_buf[i] = 0x0700 | ' '; //Turn each of the removed column into 'blank' color.
6              crt_pos -= CRT_COLS;
7      }

question 3:     
int x = 1, y = 3, z = 4;
cprintf("x %d, y %x, z %d\n", x, y, z);
- In the call to cprintf(), to what does fmt point? To what does ap point?

We add this piece of code into i386_init in kernel/init.C. "make" in the lab directory and "make qemu-gdb". Now trace into this piece of code in i386_init(). From kernel.asm. we get the piece of source code below.
    int x = 1, y = 3, z = 4;
    cprintf("x %d, y %x, z %d\n", x, y, z);
We set a break point after calling vprintfmt() in gdb(i.e., 0xf0100e51) and print the fmt and ap values.
(gdb) p fmt
$4 = 0xf0101933 " %d, y %x, z %d\n"
(gdb) p ap
$5 = (va_list) 0xf0117fd4 "\001"
We get the answer that ap is pointing to 0xf0116fe4 which is the start of the special formatting values(1, 3, 4) and fmt is pointing to the string content.

- List (in order of execution) each call to cons_putc, va_arg, and vcprintf. For cons_putc, list its argument as well. For va_arg, list what ap points to before and after the call. For vcprintf list the values of its two arguments.

Below is the executing sequence for putting first char 'x'(ASCII number 120) onto the console screen.
cprintf("x %d, y %x, z %d\n", x, y, z) >> va_arg(ap(1, 3, 4) >> vcprintf(*fmt("x %d, y %x, z %d\n"), ap) >> vprintfmt((void*)putch, &cnt(0), fmt, ap) >> putch(120('x'), &cnt(0)) >> cons_putc(120('x'))

question 4:
-     unsigned int i = 0x00646c72;
    cprintf("H%x Wo%s", 57616, &i);
What is the output? Explain how this output is arrived at in the step-by-step manner of the previous exercise. Here's an ASCII table that maps bytes to characters.

    unsigned int i = 0x00646c72; Define i as an unsigned int
f0100102:	c7 45 f4 72 6c 64 00 	movl   $0x646c72,-0xc(%ebp)    System load i in little endian manner
    cprintf("H%x Wo%s", 57616, &i);
f0100109:	8d 45 f4             	lea    -0xc(%ebp),%eax
f010010c:	89 44 24 08          	mov    %eax,0x8(%esp)
f0100110:	c7 44 24 04 10 e1 00 	movl   $0xe110,0x4(%esp)       57616 = 0xe110
f0100117:	00 
f0100118:	c7 04 24 24 18 10 f0 	movl   $0xf0101824,(%esp)
f010011f:	e8 02 08 00 00       	call   f0100926 <cprintf>
Type the code into kern/init.c(i386_init).Trace into this piece of code in kernel.asm.
executing sequence is: cprintf(cprintf("H%x Wo%s", 57616, 0x00646c72) >> va_arg(ap = 57616, 0x00646c72, fmt = "H%x Wo%s") >> vcprintf(ap, fmt) >> vprintfmt(): Assuming we are putting %x(57616) onto the screen. After detecting the format char 'x', the vprintfmt will take the next 4 bytes (int size) from ap which is 57616, interpret it in hexdecimal(e110) and display it >> vprintfmt: Assuming we are putting %s, 0x00646c72 onto the screen. After format char's', the vprintfmt will take the next 1 byte(char size) data from ap which is 0x72(little endian reads data from right to left), interpret it to char 'r' and display it.

- The output depends on that fact that the x86 is little-endian. If the x86 were instead big-endian what would you set i to in order to yield the same output? Would you need to change 57616 to a different value?

If the x86 is big-endian, int i will be loaded as 0x726c6400 and therefore the char sequence will not be the same. So we should set int i to 0x726c6400 before calling cprintf. for the second question, we don't need to change 57616 as it is loaded as int and read by vprintfmt as int as well. They are all fetched unsigned int.

question 5:
In the following code, what is going to be printed after 'y='? (note: the answer is not a specific value.) Why does this happen?
    cprintf("x=%d y=%d", 3);

On my screen, it prints x=3 y=-267292692. before calling cprintf, the ap value(3) is stored in $esp + 4.
    cprintf("x=%d y=%d", 3);
f0100124:	c7 44 24 04 03 00 00 	movl   $0x3,0x4(%esp)
f010012b:	00 
f010012c:	c7 04 24 4d 18 10 f0 	movl   $0xf010184d,(%esp)
f0100133:	e8 02 08 00 00       	call   f010093a <cprintf>
Based on the assembly calling convention, when vprintfmt detect the second %d, it will print the next ap value. At this moment, the ap value of 3 is already consumed by previous %d, so the ap pointer will go 4 bytes up which is 8(%esp), and get the next ap value 8(%esp) points to which could be anything we don't know.

question 6: Let's say that GCC changed its calling convention so that it pushed arguments on the stack in declaration order, so that the last argument is pushed last. How would you have to change cprintf or its interface so that it would still be possible to pass it a variable number of arguments?

(answer from https://qiita.com/kagurazakakotori/items/b092fc0dbe3c3ec09e8e)
For normal arguments, GCC will generate the address for them automatically and these addresses are independent of the order.

For the variable argument list, functions like va_start() are defined in inc/stdarg.h, and they are implemented with GCC's built-in functions like __builtin_va_start(). That is to say, if the calling convention of GCC has changed, these built-in functions will also be changed, thus our code can still run properly.

Challenges: Not done yet. 


## Exercise 9. Determine where the kernel initializes its stack, and exactly where in memory its stack is located. How does the kernel reserve space for its stack? And at which "end" of this reserved area is the stack pointer initialized to point to?

From kern/entry.S, we get that the kernel stack is initialized at line 76 by moving bootstacktop into %esp.
	# Set the stack pointer
	movl	$(bootstacktop),%esp
From boj/kern/kernel.asm, we trace to the same piece of code and get the exact stack memory at 0xf0117000.
	# Set the stack pointer
	movl	$(bootstacktop),%esp
f0100034:	bc 00 70 11 f0       	mov    $0xf0117000,%esp


## Exercise 10. To become familiar with the C calling conventions on the x86, find the address of the test_backtrace function in obj/kern/kernel.asm, set a breakpoint there, and examine what happens each time it gets called after the kernel starts. How many 32-bit words does each recursive nesting level of test_backtrace push on the stack, and what are those words?

set a breakpoint at test_backtrace
continue to the first breakpoint and record the esp value.
gdb) p $esp
$1 = (void *) 0xf0116fcc
continue to the second breakpoint and record the esp value
(gdb) p $esp
$2 = (void *) 0xf0116fac

Number of 32-bit words each nesting level of test_backtrace push on the stack = (0xf0116fcc - 0xf0116fac)/4 = 8.
0xf0116fcc  <- base pointer of current test_backtrace
0xf0116fc8  <- %ebx register
0xf0116fc4  <- reserved stack space for cprintf
0xf0116fc0  <- reserved stack space for cprintf
0xf0116fbc  <- reserved stack space for cprintf
0xf0116fb8  <- reserved stack space for cprintf
0xf0116fb4  <- reserved stack space for cprintf
0xf0116fb0  <- return address of the next test_backtrace


## Exercise 11. Implement the backtrace function as specified above. Use the same format as in the example, since otherwise the grading script will be confused.

The answer is shown below.
mon_backtrace(int argc, char **argv, struct Trapframe *tf)
{
    cprintf("Stack backtrace:\n");
    uint32_t *ebp = (uint32_t *)read_ebp();
    while (ebp != 0) {
        cprintf("  ebp %08x  eip %08x  args %08x %08x %08x %08x %08x\n", ebp, ebp[1], ebp[2], ebp[3], ebp[4], ebp[5], ebp[6]);
        ebp = (uint32_t *)*ebp;
    }
	return 0;
}
The before calling any of the function, the old function will push all the arguments onto the stack. After calling that function, the stack explicitly save the return address(eip) at the current address and decrement by 4. Then the new function will push it's base pointer(ebp). Below is the %esp value status
<test_backtrace(2)>
0xf0117f48 <- ebp of test_backtrace(2) that is pointing to the caller function i.e., previous test_backtrace(2)
0xf0117f44 
0xf0117f40
0xf0117f3c
0xf0117f38
0xf0117f34 <- may or may not be the argument depending on the number of arguments passed to the new function
0xf0117f30 <- arguments for <test_backtrace(1)> i.e., here 1 is the argument
0xf0117f2c <- return address of <test_backtrace(1)>
<test_backtrace(1)>
0xf0117f28 <- current ebp that is pointing to the caller function i.e., previous test_backtrace(2)
Based on this stack frame, we could get the eip and the arguments at the memory address above the ebp i.e., ebp[1], ebp[2], etc,.

The special format %08x  '0' fills the empty width of display value with 0 instead of space' ', and '8' changes the displaying width to 8 so it prints 8 digits and the x print the digit in hexdecimal. 


## Exercise 12. Modify your stack backtrace function to display, for each eip, the function name, source file name, and line number corresponding to that eip.

Firstly, we got to implement the function stab_binsearch in kdebug.c. The answer is as follow.
stab_binsearch(stabs, &lline, &rline, N_SLINE, addr);
// Search within [lline, rline] for the line number stab.
	// If found, set info->eip_line to the right line number.
	// If not found, return -1.
	//
	// Hint:
	//	There's a particular stabs type used for line numbers.
	//	Look at the STABS documentation and <inc/stab.h> to find
	//	which one.
	// Your code here.
    stab_binsearch(stabs, &lline, &rline, N_SLINE, addr);
    if (lline <= rline) {
        info->eip_line = stabs[rline].n_desc;
    } else {
        return -1;
    }
The reason why N_SLINE and n_desc are used is based on the stab documentation for line numbers(link: https://sourceware.org/gdb/current/onlinedocs/stabs.html/Line-Numbers.html). 

Back to Mon_backtrace function, below is the answer code.
mon_backtrace(int argc, char **argv, struct Trapframe *tf)
{
    cprintf("Stack backtrace:\n");
    uint32_t *ebp = (uint32_t *)read_ebp();
    while (ebp != 0) {
        cprintf("  ebp %08x  eip %08x  args %08x %08x %08x %08x %08x\n", ebp, ebp[1], ebp[2], ebp[3], ebp[4], ebp[5], ebp[6]);
        struct Eipdebuginfo info;
        debuginfo_eip(ebp[1], &info);
        cprintf("       %s:%d: %.*s+%u\n", info.eip_file, info.eip_line, info.eip_fn_namelen,
                info.eip_fn_name, ebp[1] - info.eip_fn_addr);
        ebp = (uint32_t *)*ebp;
    }
	return 0;
}
We just put each stab information according to the given format.














