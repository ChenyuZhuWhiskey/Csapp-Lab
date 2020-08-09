# Architecture Lab

## Part A

### sum_list

source code in C:

```c
long sum_list(list_ptr ls)
{
    long val = 0;
    while (ls) {
	val += ls->val;
	ls = ls->next;
    }
    return val;
}
```

`sum.ys`:

```
#   Execution begins at address 0
        .pos 0
        irmovq stack, %rsp          #initial stack ptr
        call main                   #invoke main
        halt                        #end program
#   Sample linked list
        .align 8
    ele1:
        .quad 0x00a                 #quad = quard word
        .quad ele2
    ele2:
        .quad 0x0b0
        .quad ele3
    ele3:
        .quad 0xc00
        .quad 0
main:
    irmoveq ele1,%rdi               #pass arg1
    call sum_list
    ret

sum_list:
        xorq    %rax,%rax           # long val = 0, return value is passed into %rax defaultly
        pushq   %rcx
        pushq   %rdx
        andq    %rdi,%rdi
        je      end                 # if ls=0, return
loop:
        mrmoveq (%rdi),%rcx         # save value of ls->val
        addq    %rcx,%rax           # val += ls->val
        irmoveq $8,%rbx
        addq    %rbx,%rdi           #list ptr+8，i.e. ls->next
        mrmoveq (%rdi),%rdi         # ls=ls->next
        andq    %rdi,%rdi
        jne     loop                # ls != 0 then loop
end:
        popq %rdx
        popq %rcx
        ret


#   stack starts here and grows to lower address
.pos 0x200
stack:
```

### rsum_list

source code in C:

```c
long rsum_list(list_ptr ls)
{
    if (!ls)
	return 0;
    else {
	long val = ls->val;
	long rest = rsum_list(ls->next);
	return val + rest;
    }
}
```

`rsum.ys`:

```
#   Execution begins at address 0
        .pos 0
        irmovq stack, %rsp          #initial stack ptr
        call main                   #invoke main
        halt                        #end program
#   Sample linked list
        .align 8
    ele1:
        .quad 0x00a                 #quad = quard word
        .quad ele2
    ele2:
        .quad 0x0b0
        .quad ele3
    ele3:
        .quad 0xc00
        .quad 0
main:
    xorq %rax, %rax
    irmoveq ele1,%rdi               #pass arg1
    call rsum_list
    ret

rsum_list:
        pushq   %rcx
        pushq   %rbx
        andq    %rdi,%rdi
        je      end
        mrmoveq (%rdi),%rcx     # ls->val
        irmoveq $8,%rbx
        addq    %rbx,%rdi       # ls->next
        mrmoveq (%rdi),%rdi     # ls = ls->next
        call    rsumlist
        addq    %rcx,%rax       # return val+rest
end:
        popq    %rbx
        popq    %rcx
        ret
```

### copy_block

source code in C:

```c
long copy_block(long *src, long *dest, long len)
{
    long result = 0;
    while (len > 0) {
	long val = *src++;
	*dest++ = val;
	result ^= val;
	len--;
    }
    return result;
}
```

`copy_block.ys`:

```
#   Execution begins at address 0
        .pos 0
        irmovq stack, %rsp          #initial stack ptr
        call main                   #invoke main
        halt                        #end program
#   Sample linked list
        .align 8
    ele1:
        .quad 0x00a                 #quad = quard word
        .quad ele2
    ele2:
        .quad 0x0b0
        .quad ele3
    ele3:
        .quad 0xc00
        .quad 0
main:
    irmoveq src,%rdi               #pass arg1
    irmoveq dest,%rsi              #pass arg2
    call copy_block
    ret

copy_block:
    pushq %rbx
    pushq %rcx
    xorq %rax, %rax             #long val=0
loop:
    addq %rdx, %rdx
    jle  end                    #if len > 0 then loop
    mrmoveq (%rdi),%rcx         #long val = *src
    irmoveq $8,rbx
    addq %rbx,%rdi              #src++
    rmmoveq %rcx,(%rsi)         #*dest = val
    addq %rbx,%rsi              #dest++
    xorq %rcx, %rax             #result ^= val
    irmoveq $1, %rbx
    subq %rbx,%rdx              #len--
    jmp loop

end：
    popq %rcx
    popq %rbx
    ret
```

## Part B

description of `iaddq`:

| state     |                    |
| --------- | ------------------ |
| fetch     | icode:ifun<-M1[PC] |
|           | rA,rB<-M1[PC+1]    |
|           | valC<-M1[PC+2]     |
|           | ValP<-PC+10        |
| decode    | valB<-R[rB]        |
| execute   | ValE<-ValB+ValC    |
| memory    |                    |
| writeback | R[rB]<-ValE        |
|           | PC<-valP           |

`seq-full.hcl`:

```c
################ Fetch Stage     ###################################

# Determine instruction code
word icode = [
        imem_error: INOP;
        1: imem_icode;          # Default: get from instruction memory
];

# Determine instruction function
word ifun = [
        imem_error: FNONE;
        1: imem_ifun;           # Default: get from instruction memory
];

bool instr_valid = icode in
        { INOP, IHALT, IRRMOVQ, IIRMOVQ, IRMMOVQ, IMRMOVQ,
               IOPQ, IJXX, ICALL, IRET, IPUSHQ, IPOPQ,IIADDQ };
# Does fetched instruction require a regid byte?
bool need_regids =
        icode in { IRRMOVQ, IOPQ, IPUSHQ, IPOPQ,
                     IIRMOVQ, IRMMOVQ, IMRMOVQ, IIADDQ };

# Does fetched instruction require a constant word?
bool need_valC =
        icode in { IIRMOVQ, IRMMOVQ, IMRMOVQ, IJXX, ICALL, IIADDQ };

################ Decode Stage    ###################################

## What register should be used as the A source?
word srcA = [
        icode in { IRRMOVQ, IRMMOVQ, IOPQ, IPUSHQ  } : rA;
        icode in { IPOPQ, IRET } : RRSP;
        1 : RNONE; # Don't need register
];

## What register should be used as the B source?
word srcB = [
        icode in { IOPQ, IRMMOVQ, IMRMOVQ, IIADDQ  } : rB;
        icode in { IPUSHQ, IPOPQ, ICALL, IRET } : RRSP;
        1 : RNONE;  # Don't need register
];
## What register should be used as the E destination?
word dstE = [
        icode in { IRRMOVQ } && Cnd : rB;
        icode in { IIRMOVQ, IOPQ, IIADDQ} : rB;
        icode in { IPUSHQ, IPOPQ, ICALL, IRET } : RRSP;
        1 : RNONE;  # Don't write any register
];

## What register should be used as the M destination?
word dstM = [
        icode in { IMRMOVQ, IPOPQ } : rA;
        1 : RNONE;  # Don't write any register
];

################ Execute Stage   ###################################

## Select input A to ALU
word aluA = [
        icode in { IRRMOVQ, IOPQ } : valA;
        icode in { IIRMOVQ, IRMMOVQ, IMRMOVQ, IIADDQ } : valC;
        icode in { ICALL, IPUSHQ } : -8;
        icode in { IRET, IPOPQ } : 8;
        # Other instructions don't need ALU
];
## Select input B to ALU
word aluB = [
        icode in { IRMMOVQ, IMRMOVQ, IOPQ, ICALL,
                      IPUSHQ, IRET, IPOPQ, IIADDQ } : valB;
        icode in { IRRMOVQ, IIRMOVQ } : 0;
        # Other instructions don't need ALU
];

## Set the ALU function
word alufun = [
        icode == IOPQ : ifun;
        1 : ALUADD;
];

## Should the condition codes be updated?
bool set_cc = icode in { IOPQ, IIADDQ };

################ Memory Stage    ###################################

## Set read control signal
bool mem_read = icode in { IMRMOVQ, IPOPQ, IRET };

## Set write control signal
bool mem_write = icode in { IRMMOVQ, IPUSHQ, ICALL };
## Select memory address
word mem_addr = [
        icode in { IRMMOVQ, IPUSHQ, ICALL, IMRMOVQ } : valE;
        icode in { IPOPQ, IRET } : valA;
        # Other instructions don't need address
];

## Select memory input data
word mem_data = [
        # Value from register
        icode in { IRMMOVQ, IPUSHQ } : valA;
        # Return PC
        icode == ICALL : valP;
        # Default: Don't write anything
];

## Determine instruction status
word Stat = [
        imem_error || dmem_error : SADR;
        !instr_valid: SINS;
        icode == IHALT : SHLT;
        1 : SAOK;
];

################ Program Counter Update ############################

## What address should instruction be fetched at
word new_pc = [
        # Call.  Use instruction constant
        icode == ICALL : valC;
        # Taken branch.  Use instruction constant
        icode == IJXX && Cnd : valC;
        # Completion of RET instruction.  Use value from stack
        icode == IRET : valM;
        # Default: Use incremented PC
        1 : valP;
];
#/* $end seq-all-hcl */
```

## Part C

Too hard for me.