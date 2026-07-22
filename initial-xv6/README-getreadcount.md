# Adding the `getreadcount` system call to xv6

The goal is to add `getreadcount()`, which lets a user program ask the kernel
how many `read()` system calls have been attempted since boot.

The call follows this path:

```text
user program -> usys.S -> syscall dispatcher -> sys_getreadcount()
```

## 1. Assign a system call number

Add a unique number in `syscall.h`:

```c
#define SYS_getreadcount 22
```

The kernel uses this number to identify the requested system call.

## 2. Count calls to `read()` and register the new handler

In `syscall.c`, add a global counter:

```c
int readcounts = 0;
```

Declare the new kernel handler and add it to the syscall table:

```c
extern int sys_getreadcount(void);

static int (*syscalls[])(void) = {
  // Existing system calls...
  [SYS_getreadcount] sys_getreadcount,
};
```

Update the counter when the dispatcher receives `SYS_read`:

```c
num = curproc->tf->eax;
if(num == SYS_read){
  readcounts++;
}
curproc->readcount = readcounts;
```

This happens before the `read()` handler runs, so failed read attempts are also
counted.

## 3. Store the current count in the process

Add a field to `struct proc` in `proc.h`:

```c
struct proc {
  // Existing fields...
  int readcount;
};
```

The syscall dispatcher copies the latest global count into this field for the
currently running process.

## 4. Implement the kernel handler

Add the handler to `sysproc.c`:

```c
int
sys_getreadcount(void)
{
  return myproc()->readcount;
}
```

The return value is placed in the process's `%eax` register by xv6's syscall
dispatcher and is then returned to the user program.

## 5. Expose the call to user programs

Declare the user-facing function in `user.h`:

```c
int getreadcount(void);
```

The declaration tells the C compiler that the function exists, but xv6 still
needs a small user-space function that enters the kernel. That function is
generated in `usys.S`.

`usys.S` defines this macro for all user-facing system calls:

```asm
#define SYSCALL(name) \
  .globl name; \
  name: \
    movl $SYS_ ## name, %eax; \
    int $T_SYSCALL; \
    ret
```

Add the new call to the bottom of the file:

```asm
SYSCALL(getreadcount)
```

For `getreadcount`, the macro creates a global assembly function named
`getreadcount`. Calling it performs three steps:

1. `movl $SYS_getreadcount, %eax` puts system call number 22 in `%eax`.
2. `int $T_SYSCALL` raises xv6's system-call interrupt and transfers control to
   the kernel.
3. After the kernel finishes, `ret` returns to the calling user program. The
   integer returned by `sys_getreadcount()` is already in `%eax`, which is also
   where C expects an integer return value.

The `##` in `SYS_ ## name` joins the tokens `SYS_` and `getreadcount`, producing
`SYS_getreadcount`. Its value comes from `syscall.h`, which is included at the
top of `usys.S`.

## 6. Add a program that uses it

`hello.c` calls `getreadcount()` before and after creating read activity:

```c
int before = getreadcount();
int pid = fork();

// Both processes make read() calls here.

if(pid > 0){
  wait();
  int after = getreadcount();
  printf(1, "XV6_TEST_OUTPUT %d\n", after - before);
}
```

Add `_hello` to `UPROGS` in `Makefile` so it is copied into xv6's file-system
image:

```make
UPROGS=\
	_cat\
	_hello\
	_echo\
```

The Makefile also selects the `i386-elf-` cross-compiler, allows two warnings
needed by newer compilers, and runs xv6 with one CPU.

## Build and run

```sh
make qemu-nox
```

At the xv6 shell, run:

```sh
hello
```

The printed difference is the number of `read()` calls observed between the
two calls to `getreadcount()`.
