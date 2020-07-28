# Bomb
## Phase_1
In the disassembly of Phase_1, we can see function call `string_not_equal()` with a ptr argument(register SI) point to string: `'Border relations with Canada have never been better.'`, if not equal, then `explode_bomb()`:
```assembly
0x0000000000400ee0 <+0>:	sub    rsp,0x8
0x0000000000400ee4 <+4>:	mov    esi,0x402400
0x0000000000400ee9 <+9>:	call   0x401338 <strings_not_equal>
0x0000000000400eee <+14>:	test   eax,eax
0x0000000000400ef0 <+16>:	je     0x400ef7 <phase_1+23>
0x0000000000400ef2 <+18>:	call   0x40143a <explode_bomb>
0x0000000000400ef7 <+23>:	add    rsp,0x8
0x0000000000400efb <+27>:	ret 
```
So, the answer is obvious: `'Border relations with Canada have never been better.'`
## Phase_2
Firstly, function `phase_2()` read six numbers form input to buffer, in the function `read_six_numbers()`, if return of `sscanf()`(num of read args) is less than 6, then `explode_bomb()`:
```assembly
0x000000000040145c <+0>:	sub    rsp,0x18
0x0000000000401460 <+4>:	mov    rdx,rsi
0x0000000000401463 <+7>:	lea    rcx,[rsi+0x4]
0x0000000000401467 <+11>:	lea    rax,[rsi+0x14]
0x000000000040146b <+15>:	mov    QWORD PTR [rsp+0x8],rax
0x0000000000401470 <+20>:	lea    rax,[rsi+0x10]
0x0000000000401474 <+24>:	mov    QWORD PTR [rsp],rax
0x0000000000401478 <+28>:	lea    r9,[rsi+0xc]
0x000000000040147c <+32>:	lea    r8,[rsi+0x8]
0x0000000000401480 <+36>:	mov    esi,0x4025c3
0x0000000000401485 <+41>:	mov    eax,0x0
0x000000000040148a <+46>:	call   0x400bf0 <__isoc99_sscanf@plt>
0x000000000040148f <+51>:	cmp    eax,0x5
0x0000000000401492 <+54>:	jg     0x401499 <read_six_numbers+61>
0x0000000000401494 <+56>:	call   0x40143a <explode_bomb>
0x0000000000401499 <+61>:	add    rsp,0x18
0x000000000040149d <+65>:	ret
```
After read is finished, `phase_2()` would check if the next number is equal to 2 times of present number in buffer. if not, then `explode_bomb()`, else defused.
```assembly
0x0000000000400f0a <+14>:	cmp    DWORD PTR [rsp],0x1
0x0000000000400f0e <+18>:	je     0x400f30 <phase_2+52>
0x0000000000400f10 <+20>:	call   0x40143a <explode_bomb>
0x0000000000400f15 <+25>:	jmp    0x400f30 <phase_2+52>
0x0000000000400f17 <+27>:	mov    eax,DWORD PTR [rbx-0x4]
0x0000000000400f1a <+30>:	add    eax,eax
0x0000000000400f1c <+32>:	cmp    DWORD PTR [rbx],eax
0x0000000000400f1e <+34>:	je     0x400f25 <phase_2+41>
0x0000000000400f20 <+36>:	call   0x40143a <explode_bomb>
0x0000000000400f25 <+41>:	add    rbx,0x4
0x0000000000400f29 <+45>:	cmp    rbx,rbp
0x0000000000400f2c <+48>:	jne    0x400f17 <phase_2+27>

```
## Phase_3
`phase_3()` read 2 numbers from input(num of read is also checked as befofe) and do checks as follow:
- check if num1 is less than 7 or not
```assembly
0x0000000000400f6a <+39>:	cmp    DWORD PTR [rsp+0x8],0x7
0x0000000000400f6f <+44>:	ja     0x400fad <phase_3+106>
0x0000000000400fad <+106>:	call   0x40143a <explode_bomb>
```
- check if num2 is equal to 0x185 or not.
```assembly
0x0000000000400fb9 <+118>:	mov    eax,0x137
0x0000000000400fbe <+123>:	cmp    eax,DWORD PTR [rsp+0xc]
0x0000000000400fc2 <+127>:	je     0x400fc9 <phase_3+134>
0x0000000000400fc4 <+129>:	call   0x40143a <explode_bomb>
```
## Phase_4
`phase_4()` firstly read 2 numbers from imput(numer of read check is done as before). Then do checks as follow:
- check if num1 is less than 0xe or not
```assembly
0x000000000040102e <+34>:	cmp    DWORD PTR [rsp+0x8],0xe
0x0000000000401033 <+39>:	jbe    0x40103a <phase_4+46>
0x0000000000401035 <+41>:	call   0x40143a <explode_bomb>
```
- check if num2 is 0 or not:
```assembly
0x0000000000401051 <+69>:	cmp    DWORD PTR [rsp+0xc],0x0
0x0000000000401056 <+74>:	je     0x40105d <phase_4+81>
0x0000000000401058 <+76>:	call   0x40143a <explode_bomb>
```
## Phase_5
`phase_5()` firstly read 6 chars from input into buffer,`string_length()` is used to check if length of inputs is equal to 6 or not.then do some maths as follow:
- Let char's higher 4 bits be zero:
```assembly
0x0000000000401096 <+52>:	and    edx,0xf
```
- use result as offset to index some static data:
```assembly
0x0000000000401099 <+55>:	movzx  edx,BYTE PTR [rdx+0x4024b0]
0x00000000004010a0 <+62>:	mov    BYTE PTR [rsp+rax*1+0x10],dl
```
- In the end, if the 6 indexed data are:'flyers', this phase would pass:
```assembly
0x00000000004010bd <+91>:	call   0x401338 <strings_not_equal>
0x00000000004010c2 <+96>:	test   eax,eax
0x00000000004010c4 <+98>:	je     0x4010d9 <phase_5+119>
0x00000000004010c6 <+100>:	call   0x40143a <explode_bomb>
```
## Phase_6
This phase is a little bit long. Steps it does as follow:
- read 6 numbers from input into buffer(`read_six_numers()`, same as phase_1)
- check if 6 numbers have same values or not, if true, `explode_bomb()`, i.e. 6 numbers must be different
- check if 6 numbers are less than 7
- update buffer by using 7 to sub every number.
- use these new 6 numbers as order to load 6 nodes from static area one by one.
- check if the data in nodes are in descent order or not, if no, `explode_bomb()`, if yes, pass `phase_6()`.