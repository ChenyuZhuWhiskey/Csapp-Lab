# Datalab
## bitXor
This one can be solved by simply using equation of xor: a⊕b = (¬a ∧ b) ∨ (a ∧¬b)
```c
int bitXor(int x, int y) {
  return ~(~x & ~y)&(~(x&y));
}
```

## tmin
the bit pattern of minimum int is 0x80000000, i.e. 1 for sign bit and 0 for others.
```c
int tmin(void) {
  return 0x1 << 31;
}
```
## isTmax
the bit pattern of maximum int is 0x7FFFFFFF, i.e. 0 for sign bit and 1 for others. So to verify if a int number is max, just compare them in bit level.
```c
int isTmax(int x) {
  return !(x ^ 0x7FFFFFFF);
}
```

## allOddBits
verifying if all odd bits in a number are 1 or not can be done simply  with 2 steps:
- convert all even bits to 1
- compare with 0xFFFFFFFF in bit level.
```c
int allOddBits(int x) {
  return !~(x|0x55555555&0xFFFFFFFF);
}
```
## negate
using definition of 2's complement.
```c
int negate(int x) {
  return ~x+1;
}
```
## isAsciiDigit
The key mind is to verify if a number is in range of [0x30 ,0x39] or not. This can be realized by checking the sign bit of the range minus result.
```c
int isAsciiDigit(int x) {
  return !(((0x39+(~x+1))>>31) | ((x+(~0x30+1))>>31));
}
```
## conditional
In the mathematical perspective, this triple operator is equivilant to:
$$conditional(a,b,c) = istrue(a)\cdot b + isfalse(a)\cdot c$$
So, our target is constructing multiplicative operation using legal ops: ! ~ & ^ | + << >>. Notice that 1-1 = 0x0, 0-1 = 0xFFFFFFF:
```c
int conditional(int x, int y, int z) {
  return ((!x-1)&y)+(~(!x-1)&z);
}
```
## isLessOrEqual
compare sign bit of x, y, y-x. Take care of that: y-x could result in arithmethical overflow and underflow.
```c
int isLessOrEqual(int x, int y) {
  int sign_x = (x >>31)&1;
  int sign_y = (y >>31)&1;
  int sign_minus = ((y + (~x+1)) >> 31)&1;
  int is_equal = !(y ^ x);
  return (sign_x&(!sign_y))| (!(sign_x^sign_y)&!sign_minus);
}
```
## logicalNeg
The key mind is that `!` operator return true if any bit is not 0. So we can 'accumulate' all bits value to the first bit, and then check if the value is 0 or not.
```c
int logicalNeg(int x) {
  x=(x>>16)|x;
  x=(x>>8)|x;
  x=(x>>4)|x;
  x=(x>>2)|x;
  x=(x>>1)|x;
  return ~x&0x1;
}
```

## howManyBits
In this problem, we can simply check the top non-zero bit of a pos number, but this cannot be used to do the check of neg number because of 2's complete. A neg number need more than 1 bit to represent than a pos number (value is equal without sign). So we can first convert neg number to a pos number with representation of 1 bit more then original, then find the highst non-zero bit and return the position.
```c
int howManyBits(int x) {
  int n = 0 ;
  x^=(x<<1);
  n +=  (!!( x & ((~0) << (n + 16)) )) << 4;
  n +=  (!!( x & ((~0) << (n + 8)) )) << 3;
  n +=  (!!( x & ((~0) << (n + 4)) )) << 2;
  n +=  (!!( x & ((~0) << (n + 2)) )) << 1;
  n +=  (!!( x & ((~0) << (n + 1)) ));
  return n+1;
}
```
## floatScale2, floatFloat2Int,floatPower2
These three float-point-number-based problem is not difficult, the solution is straightforward. Just need to pay attention to something subtle:
- 0-22 is fraction aera
- 23-31 is exponential area
- 32 is sign bit
- exponential bits use shift code, the range is 00000001(-126) to 11111111(127).
- denormalized number: 
  - +0: 0x0
  - -0: 0x80000000
  - +Infinity: 0 for sign bit,1 for all exponent, zero for all fraction.
  - -Infinity: 1 for sign bit,1 for all exponent, zero for all fraction.
  - NAN: 1 for all exponent, non-zero bit in fraction.

```c
unsigned floatScale2(unsigned uf) {
  int tmp=uf;
  int sign=((uf>>31)<<31); /* 0x80000000 or 0x0 */
  int exp=uf&0x7f800000;
  int f=uf&0x7fffff;
  tmp=tmp&0x7fffffff; /* remove sign */
  if((tmp>>23)==0x0){
    tmp=tmp<<1|sign;
    return tmp;
  } else if((tmp>>23)==0xff){
    return uf;
  }  else{
    if((exp>>23)+1==0xff){
      return sign|0x7f800000;
    }else{
      return sign|(((exp>>23)+1)<<23)|f;
    }
  }
}

int floatFloat2Int(unsigned uf) {
  int sign = uf >> 31;
  int exp = ((uf & 0x7f800000) >> 23) - 127;
  int frac = (uf & 0x007fffff) | 0x00800000;
  if (!(uf & 0x7fffffff))
    return 0;

  if (exp > 31)
    return 0x80000000;
  if (exp < 0)
    return 0;

  if (exp > 23)
    frac <<= (exp - 23);
  else
    frac >>= (23 - exp);

  if (!((frac >> 31) ^ sign))
    return frac;
  else if (frac >> 31)
    return 0x80000000;
  else
    return ~frac + 1;
}

unsigned floatPower2(int x) {
  int exp = x + 127;
  if (exp <= 0)
    return 0;
  if (exp >= 255)
    return 0x7f800000;
  return exp << 23;
}
```