/* 
 * CS:APP Data Lab 
 * 
 * <Please put your name and userid here>
 * 
 * bits.c - Source file with your solutions to the Lab.
 *          This is the file you will hand in to your instructor.
 *
 * WARNING: Do not include the <stdio.h> header; it confuses the dlc
 * compiler. You can still use printf for debugging without including
 * <stdio.h>, although you might get a compiler warning. In general,
 * it's not good practice to ignore compiler warnings, but in this
 * case it's OK.  
 */

#if 0
/*
 * Instructions to Students:
 *
 * STEP 1: Read the following instructions carefully.
 */

You will provide your solution to the Data Lab by
editing the collection of functions in this source file.

INTEGER CODING RULES:
 
  Replace the "return" statement in each function with one
  or more lines of C code that implements the function. Your code 
  must conform to the following style:
 
  int Funct(arg1, arg2, ...) {
      /* brief description of how your implementation works */
      int var1 = Expr1;
      ...
      int varM = ExprM;

      varJ = ExprJ;
      ...
      varN = ExprN;
      return ExprR;
  }

  Each "Expr" is an expression using ONLY the following:
  1. Integer constants 0 through 255 (0xFF), inclusive. You are
      not allowed to use big constants such as 0xffffffff.
  2. Function arguments and local variables (no global variables).
  3. Unary integer operations ! ~
  4. Binary integer operations & ^ | + << >>
    
  Some of the problems restrict the set of allowed operators even further.
  Each "Expr" may consist of multiple operators. You are not restricted to
  one operator per line.

  You are expressly forbidden to:
  1. Use any control constructs such as if, do, while, for, switch, etc.
  2. Define or use any macros.
  3. Define any additional functions in this file.
  4. Call any functions.
  5. Use any other operations, such as &&, ||, -, or ?:
  6. Use any form of casting.
  7. Use any data type other than int.  This implies that you
     cannot use arrays, structs, or unions.

 
  You may assume that your machine:
  1. Uses 2s complement, 32-bit representations of integers.
  2. Performs right shifts arithmetically.
  3. Has unpredictable behavior when shifting an integer by more
     than the word size.

EXAMPLES OF ACCEPTABLE CODING STYLE:
  /*
   * pow2plus1 - returns 2^x + 1, where 0 <= x <= 31
   */
  int pow2plus1(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     return (1 << x) + 1;
  }

  /*
   * pow2plus4 - returns 2^x + 4, where 0 <= x <= 31
   */
  int pow2plus4(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     int result = (1 << x);
     result += 4;
     return result;
  }

FLOATING POINT CODING RULES

For the problems that require you to implent floating-point operations,
the coding rules are less strict.  You are allowed to use looping and
conditional control.  You are allowed to use both ints and unsigneds.
You can use arbitrary integer and unsigned constants.

You are expressly forbidden to:
  1. Define or use any macros.
  2. Define any additional functions in this file.
  3. Call any functions.
  4. Use any form of casting.
  5. Use any data type other than int or unsigned.  This means that you
     cannot use arrays, structs, or unions.
  6. Use any floating point data types, operations, or constants.


NOTES:
  1. Use the dlc (data lab checker) compiler (described in the handout) to 
     check the legality of your solutions.
  2. Each function has a maximum number of operators (! ~ & ^ | + << >>)
     that you are allowed to use for your implementation of the function. 
     The max operator count is checked by dlc. Note that '=' is not 
     counted; you may use as many of these as you want without penalty.
  3. Use the btest test harness to check your functions for correctness.
  4. Use the BDD checker to formally verify your functions
  5. The maximum number of ops for each function is given in the
     header comment for each function. If there are any inconsistencies 
     between the maximum ops in the writeup and in this file, consider
     this file the authoritative source.

/*
 * STEP 2: Modify the following functions according the coding rules.
 * 
 *   IMPORTANT. TO AVOID GRADING SURPRISES:
 *   1. Use the dlc compiler to check that your solutions conform
 *      to the coding rules.
 *   2. Use the BDD checker to formally verify that your solutions produce 
 *      the correct answers.
 */


#endif
//1
/* 
 * bitXor - x^y using only ~ and & 
 *   Example: bitXor(4, 5) = 1
 *   Legal ops: ~ &
 *   Max ops: 14
 *   Rating: 1
 */
// Reference link: https://stackoverflow.com/questions/12375808/how-to-make-bit-wise-xor-in-c
// It is easily seen that: x ^ y = (x | y) & ~(x & y)
// so it remains to express | by only & and ~. 
// De Morgan's laws tell us x | y = ~(~x & ~y)
int bitXor(int x, int y) {
    int AND = x & y;
    int NOR = ~x & ~y;
    // int result = ~a & ~b;
    // return result;
    // Following the De Morgan's laws
    return ~AND & ~NOR; // And | NOR
}
/* 
 * tmin - return minimum two's complement integer 
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 4
 *   Rating: 1
 */
int tmin(void) {

  return 1<<31;

}

/* 
 * isNotEqual - return 0 if x == y, and 1 otherwise 
 *   Examples: isNotEqual(5,5) = 0, isNotEqual(4,5) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 6
 *   Rating: 2
 */
// int isNotEqual(int x, int y) {
//   return !(!(x ^ y));
// }


//2
/*
 * isTmax - returns 1 if x is the maximum, two's complement number,
 *     and 0 otherwise 
 *   Legal ops: ! ~ & ^ | +
 *   Max ops: 10
 *   Rating: 2
 */
// reference link: https://stackoverflow.com/questions/7300650/how-to-find-tmax-without-using-shifts
int isTmax(int x) {
  // Tmax should be follow the format of 0x01111...111, where the signed bit is zero, and rest of bits is all one

  // 0x7FFFFFFF is the maximum positive signed 32 bit two's complement number, where F = 1111, 7 = 0111
  // return !(x ^ 0x7FFFFFFF);
  // Solution 1:
  // Note that: 0xffffffff = -1
  int neg1 = !(~x); // if x == -1, neg1 == 1, else, neg1 == 0 
  return !((~(x+1)^x)|neg1); // add 1, flip, xor, becomes 0 if Tmax (or if -1)

  // Solution 2: Tmax + Tmax + 2 = 00000000; !(Tmax + Tmax +2 ) == 1; 
  // return !(x+x+2) & !!(~x) ;   //x !=-1 && x+x+2 == 0
}

/**
 * isTmin - returns 1 if x is the minimum, two's complement number,
 *     and 0 otherwise 
 *   Legal ops: ! ~ & ^ | +
 *   Max ops: 10
 *   Rating: 1
int isTmin(int x) {
  // return !(x + x) & !(!x);
  return !((x + x) | !x);
}
**/

/*
 * getByte - Extract byte n from word x
 *   Bytes numbered from 0 (LSB) to 3 (MSB)
 *   Examples: getByte(0x12345678,1) = 0x56
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 6
 *   Rating: 2
 */
// int getByte(int x, int n)
// {
//   //Shifting n left by three has the effect of multiplying it by 8. X is then
//   //bit shifted this amount and isolated to result in the correct byte.
//   return 0xff & (x >> (n << 3));
// }

/* 
 * allOddBits - return 1 if all odd-numbered bits in word set to 1
 *   Examples allOddBits(0xFFFFFFFD) = 0, allOddBits(0xAAAAAAAA) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 2
 */
int allOddBits(int x) {
  // Use mask Oxaa (1010 1010) to check the odd bit
    // Solution 1:
    // int slice = 170, oddOneEvenZero = 170;
    // oddOneEvenZero = (oddOneEvenZero << 8) + slice;
    // oddOneEvenZero = (oddOneEvenZero << 8) + slice;
    // oddOneEvenZero = (oddOneEvenZero << 8) + slice;
    // return !(~(oddOneEvenZero & x) + oddOneEvenZero + 1);

    
    // Solution 2:
    // If you want to check if integer a and b are equal:
    // ~a + b + 1 evaluates to 0 if a == b, using 3 operators.
    // x = (x>>16) & x;
    // x = (x>>8) & x;
    // x = (x>>4) & x;
    // x = (x>>2) & x;
    // return (x>>1)&1;

    // Solution 3:
    // build mask: 0xAAAAAAAA
    int mask = (0xAA << 8) + 0xAA; // 0xaaaa
    mask = (mask << 16) + mask; // 0xaaaaaaaa

    return !((x & mask) ^ mask);
}

/* 
 * negate - return -x 
 *   Example: negate(1) = -1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 5
 *   Rating: 2
 */
int negate(int x) {
  return (~x)+1;
}
//3
/* 
 * isAsciiDigit - return 1 if 0x30 <= x <= 0x39 (ASCII codes for characters '0' to '9')
 *   Example: isAsciiDigit(0x35) = 1.
 *            isAsciiDigit(0x3a) = 0.
 *            isAsciiDigit(0x05) = 0.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 3
 */
int isAsciiDigit(int x) {
  /**
   * If the x is larger than 0x35, then the sign bit should be 0 after x-0x35
   * If the x is larger than 0x35, then the sign bit should be 1 after x-0x39
  **/
  // int lowerBound = 0x30;
  // int upperBound = 0x3a;
  // return !((x + (~lowerBound + 1)) >> 31) & (x + (~upperBound + 1)) >> 31; 
  int neg_0x30 = ~(0x30) + 1;
  int x_minus_0x30 = x + neg_0x30;
  int neg_0x3a = ~(0x3a) + 1;
  int x_minus_0x3a = x + neg_0x3a;
  int lower_bound_mask = !(x_minus_0x30 >> 31); //0 if x >= 0x30, 1 if x < 0x30
  int upper_bound_mask = !!(x_minus_0x3a >> 31); //0 if x > 0x3a, 1 if x <= 0x39
  return lower_bound_mask & upper_bound_mask; //TOTAL OPS: 12
}
/* 
 * conditional - same as x ? y : z 
 *   Example: conditional(2,4,5) = 4
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 16
 *   Rating: 3
 */
int conditional(int x, int y, int z) {
  x = !!x; // Put x into the logical form
  x = ~x+1; // negate the x
  return ( x&y )|(~x&z);
}
/* 
 * isLessOrEqual - if x <= y  then return 1, else return 0 
 *   Example: isLessOrEqual(4,5) = 1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 24
 *   Rating: 3
 */
int isLessOrEqual(int x, int y) {
  int z,s,sx,sy;
  sx = (x>>31)&1; // the sign bit of x
  sy = (y>>31)&1; // the sign bit of y
  z = x + ~y + 1; //x-y
  s = ((z>>31) & 1) | (!(z^0)); //negative or 0 : less or equal
  //The sign of the difference when the x and y symbols are the same
  //x<0 && y >=0 returns true
  return  ((!(sx^sy))&s) | (sx&(!sy));
}

//4
/* 
 * logicalNeg - implement the ! operator, using all of 
 *              the legal operators except !
 *   Examples: logicalNeg(3) = 0, logicalNeg(0) = 1
 *   Legal ops: ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 4 
 */
int logicalNeg(int x) {
  // zero is the only number that complemented
  // and inversed-and-complemented is the
  // same: (-1) -- check for signbit sameness
  // In other words, if x or -x has a 1 in the sign place, it is not 0. (isNonZero)

  // Solutions 1:
  // return ((x | (1 + ~x)) >> 31) + 1;

  // Solutions 2:
  return ( ( (~x) & (~(~x+1)) ) >> 31 ) & 1;

  // Solutions 3: Using bit smearing
  //Smear the bits which is to say take the left most bit and make sure all the bits to its left become ones

  //Explenation of bit smearing
  /*1000 0000 0000 0000 0000 0000 0000 0000 (Initial)
    *1000 0000 0000 0000 1000 0000 0000 0000 (x |= x >> 16)
    *1000 0000 1000 0000 1000 0000 1000 0000 (x |= x >> 8)
    *1000 1000 1000 1000 1000 1000 1000 1000 (x |= x >> 4)
    *1010 1010 1010 1010 1010 1010 1010 1010 (x |= x >> 2)
    *1111 1111 1111 1111 1111 1111 1111 1111 (x |= x >> 2) 
    */
  // int smeard = x;
  // smeard = smeard | smeard >> 16;
  // smeard = smeard | smeard >> 8;
  // smeard = smeard | smeard >> 4;
  // smeard = smeard | smeard >> 2;
  // smeard = smeard | smeard >> 1;
  // return (~smeard & 1);

}

/*
 * ilog2 - return floor(log base 2 of x), where x > 0
 *   Example: ilog2(16) = 4
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 90
 *   Rating: 4
 */
// int ilog2(int x) {
//     //This program takes refrence to the Internet
//     //First consider the First 16 bit of x,if x>0xffff then the last 16 bit is useless so we can do right shift
//     //After the right shift,what is left is the original First 16 bits
//     //t records the answer
//     //use (!!x) as a representation of (x!=0)
//     //use bit-or to do add operation
//     int s,t,u;
//     u=x;
//     t = (!!(u >> 16)) << 4;
//     u >>= t;
//     s = (!!(u >> 8)) << 3;
//     u >>= s;
//     t |= s;
//     s= (!!(u >> 4)) << 2;
//     u >>= s;
//     t |= s;
//     s=(!!(u >> 2)) << 1;
//     u >>= s;
//     t |= s;
//     return (t | (u >> 1));
// }

/* howManyBits - return the minimum number of bits required to represent x in
 *             two's complement
 *  Examples: howManyBits(12) = 5
 *            howManyBits(298) = 10
 *            howManyBits(-5) = 4
 *            howManyBits(0)  = 1
 *            howManyBits(-1) = 1
 *            howManyBits(0x80000000) = 32
 *  Legal ops: ! ~ & ^ | + << >>
 *  Max ops: 90
 *  Rating: 4
 */
int howManyBits(int x) {
  // Solutions 1:
  /*
     * We first bit invert all negative numbers and
     * use binary search to find out the log2(n).
     * Then we add 1 to the final result since we need
     * the MSB to represent the sign.
     * Note: finding the following things are equal:
     * 1. find the most significant bit of 1 for positive numbers
     * 2. find the most significant bit of 0 for negative numbers
  */
  int sign, bit0, bit1, bit2, bit4, bit8, bit16;

  sign = x >> 31;
  
  /* Bit invert x as needed */
  x = (sign & ~x) | (~sign & x); // Keep the same sign bit for positive number, invert all bits for negative number otherwise
  
  /* Binary Search on bit level */
  bit16 = !!(x >> 16) << 4; // Chech whether the highest 16 bit contain the bit, which is 1
  x = x >> bit16;
  
  bit8 = !!(x >> 8) << 3; // Chech whether the highest 8 bit contain the bit, which is 1
  x = x >> bit8; // if it is, then at least has 16+8 = 24 bits for this number
  
  bit4 = !!(x >> 4) << 2;
  x = x >> bit4;
  
  bit2 = !!(x >> 2) << 1;
  x = x >> bit2;
  
  bit1 = !!(x >> 1);
  x = x >> bit1;
  
  bit0 = x;

  return bit16 + bit8 + bit4 + bit2 + bit1 + bit0 + 1; // add the sign bit

  
  // Solutions 2:
  // int y, result, mask16, mask8, mask4, mask2, mask1, bitnum;
  // /*
  // * The idea here is to apply binary search in order to get log number of operations
  // */
  // mask1 = 0x2;  		            // 0x1 << 1
  // mask2 = 0xC;  		            // 0x3 << 2
  // mask4 = 0xF0;		  	          // 0x000000F0
  // mask8 = 0xFF<<8;		          // 0x0000FF00
  // mask16 = (mask8 | 0xFF) << 16;// 0xFFFF0000

  // result = 1;
  // y = x^(x>>31); //cast the number to positive with the same howManyBits result

  // // Check first 16 bits, if they have at least one bit - result > 16
  // bitnum = (!!(y & mask16)) << 4; // 16 OR zero
  // result += bitnum; 
  // y = y >> bitnum;

  // bitnum = (!!(y & mask8)) << 3;  // 8 OR zero
  // result += bitnum;
  // y = y >> bitnum;

  // bitnum = (!!(y & mask4)) << 2;  // 4 OR zero
  // result += bitnum;
  // y = y >> bitnum;

  // bitnum = (!!(y & mask2)) << 1;  // 2 OR zero
  // result += bitnum;  
  // y = y >> bitnum;

  // bitnum = !!(y & mask1);  // 1 OR zero
  // result += bitnum;  
  // y = y >> bitnum;

  // return result + (y&1);

  // Solution 3:
  // int s,c1,c2,c3,c4,c5,c6;
  // int cnt = 0;
  // s = (x>>31)&1;
  // x = ((s<<31)>>31) ^ x;

  //    s = !!(x>>16); //High 16 bits are not 0
  //    c1 = s<<4; // if it is not 0, shift 16 bits to the right
  // x >>= c1;       

  // s = !!(x>>8);
  // c2 = s<<3;
  // x >>= c2;

  // s = !!(x>>4);
  // c3 = s<<2;
  // x >>= c3;

  // s = !!(x>>2);
  // c4 = s<<1;
  // x >>= c4;

  // s = !!(x>>1);
  // c5 = s;
  // x >>= c5;

  // c6 = !!x;
  // cnt = c1+c2+c3+c4+c5+c6+1;
  // return cnt;
}

//float
/**
 * The form of floating points representation: (-1)^s M 2^E;
 * where s(sign bit) stand for the sign bit
 * M(fraction bit) stand for a fractional value in range [1.0,2.0): 23-bits for single precision; 52-bits for double precision
 * E(exponent bit) weights value by power of two : 8-bits for single precision; 11-bits for double precision
 * 
 * Normalize: exp != 000...000 or exp != 111...111
 *    E = exp - bias;
 *    Bias = 2^(k-1) - 1, where k is number of exponent bits: single is 127, double is 1023
 * Denormalize: exp is all zero
 *    E = 1-Bias;
 * 
 * Special value: exp = 111...111
 *  represents value infinity
 *  Operation that overflow
 *  Both positive and negative
 * 
 * Not-a-Number(NaN): exp = 111...111, frac = 000...000
 *    Represents case when no numeric value can be determined.
**/

/* 
 * float_neg - Return bit-level equivalent of expression -f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representations of
 *   single-precision floating point values.
 *   When argument is NaN, return argument.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 10
 *   Rating: 2
 */
// unsigned float_neg(unsigned uf) {
//   unsigned tmp = uf&(0x7fffffff);
//   unsigned result = uf^(0x80000000);
//   if(tmp>0x7f800000) result=uf;
//   return result; 
// }


/* 
 * float_twice - Return bit-level equivalent of expression 2*f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representation of
 *   single-precision floating point values.
 *   When argument is NaN, return argument
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
unsigned float_twice(unsigned uf) {
    // Solution 1:
    // 0x7f800000 = 0111 1111 1000 0000 0000 0000 0000 0000
    // int exp = (uf&0x7f800000)>>23; // the fracition bit take 23 bits in float
    // int sign = uf&(1<<31);
    // Donormalize: to double the float just left shift fraction bit to one and keep the sign bit
    // if(exp==0) return uf<<1|sign;
    // Special value: overflow cause infinify or NaN
    // if(exp==255) return uf;
    // Normalize: add one for exponent bit for exponent bit when it less than 255(all one) to double the float
    // exp++;
    // Overflow to become infinity
    // if(exp==255) return 0x7f800000|sign;
    // return (exp<<23)|(uf&0x807fffff);
    
    // Solution 2:
    unsigned tmp = uf;
    if((tmp&0x7f800000)==0){
      tmp = (tmp&0x80000000)|((tmp&0x007fffff)<<1);
    }
    else if((tmp&0x7f800000)!=0x7f800000){
      tmp+=(1<<23);
      if((tmp&0x7f800000)==0x7f800000){
        tmp=(tmp>>23)<<23;
      }
    }
    return tmp;
}
/* 
 * float_i2f - Return bit-level equivalent of expression (float) x
 *   Result is returned as unsigned int, but
 *   it is to be interpreted as the bit-level representation of a
 *   single-precision floating point values.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
unsigned float_i2f(int x) {
  // Solution1 :
    unsigned shiftLeft=0;
    unsigned afterShift, tmp, flag;
    unsigned absX=x;
    unsigned sign=0;
    if (x==0) return 0;
    //if x < 0, sign = 1000...,abs_x = -x
    if (x<0)
    {
        sign=0x80000000;
        absX=-x;
    }
    afterShift=absX;
    //count shift_left and after_shift
    while (1)
    {
        tmp=afterShift;
        afterShift<<=1;
        shiftLeft++;
        if (tmp & 0x80000000) break;
    }
    if ((afterShift & 0x01ff)>0x0100)
        flag=1;
    else if ((afterShift & 0x03ff)==0x0300)
        flag=1;
    else
        flag=0;
    return sign + (afterShift>>9) + ((159-shiftLeft)<<23) + flag;

  // Solution 2 :
	// int flag = (x & 0x80000000), i, val1, val2, len, tempp, temp;//flag equals to (1 << 31) or 0,indicating the sign bit of x
	// if(x == 0x80000000)//if x equals to INT_MIN
	// 	return 0xcf000000;//Then we can not take -x, so just return the answer
	// if(flag)//flag equals to (1 << 31) or 0, and flag is considered true when flag equals to (1 << 31)
	// 	x = -x;//x now equals to abs(x)
	// i = 31;//initialization of i, i will be the first bit of 1(from left to right, except the sign bit)
	// do
	// {
	// 	if( x >> (--i) & 0x1)
	// 		break;
	// }
	// while(i);
	// if(x == 0)//x equals to zero
	// 	return 0;
	// temp = i + 127;//This temp is exp, 127 is the bias
	// len = i - 23;//if i is greater than 23, than len is the number of bits that needed to be rounded
	// if(len > 0)
	// {
	// 	val2 = x & ((0xffffffffU) >> (32 - len) );//Save the bits that will be rounded in val2
	// 	val1 = (x >> len) & 0x007fffff;//val1 is the frac domain
	// 	tempp = 1 << (len - 1);//set tempp to justify the round stat
	// 	if((val2 > tempp) || ((val2 == tempp) && ((val1 & 0x1))))//There is a carry '1' here
	// 		val1++;
	// }
	// else
	// 	val1 = (x << (-len)) & 0x007fffff;//if len <= 0, which means the 23 bit of frac is enough, we do not need to consider val2

	// if(val1 == 0x00800000 )//corner case, special judge: 0x00800000 :  0000 0000 1000 0000 0000 0000 0000 0000
	// {
	// 	val1 = 0;
	// 	temp++;
	// }
	// return flag  | ((temp << 23)) | (val1);

  // Solution3:
  // int s_ = x&0x80000000;
  // int n_ = 30;
  // if(!x) return 0;
  // if(x==0x80000000) return 0xcf000000;
  // if(s_) x = ~x+1;
  // while(!(x&(1<<n_))) n_--;
  // if(n_<=23) x<<=(23-n_);
  // else{
  //   x+=(1<<(n_-24));
  //   if(x<<(55-n_)) ;else x&=(0xffffffff<<(n_-22));
  //   if(x&(1<<n_))  ;else n_++;
  //   x >>= (n_-23);
  // }
  // x=x&0x007fffff;
  // n_=(n_+127)<<23;
  // return x|n_|s_;
}
/* 
 * float_f2i - Return bit-level equivalent of expression (int) f
 *   for floating point argument f.
 *   Argument is passed as unsigned int, but
 *   it is to be interpreted as the bit-level representation of a
 *   single-precision floating point value.
 *   Anything out of range (including NaN and infinity) should return
 *   0x80000000u.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
int float_f2i(unsigned uf) {
  // int s_    = uf>>31;
  // int exp_  = ((uf&0x7f800000)>>23)-127;
  // int frac_ = (uf&0x007fffff)|0x00800000;
  // if(!(uf&0x7fffffff)) return 0;

  // if(exp_ > 31) return 0x80000000;
  // if(exp_ < 0) return 0;

  // if(exp_ > 23) frac_ <<= (exp_-23);
  // else frac_ >>= (23-exp_);

  // if(!((frac_>>31)^s_)) return frac_;
  // else if(frac_>>31) return 0x80000000;
  // else return ~frac_+1;
  int exp = (uf >> 23) & 0xFF; /*8 exponent bits*/
  int frac = uf & 0x7FFFFF; /*23 fraction bits*/
  int e = exp - 127; /*amount to shift normalized values by (bias of 127)*/
 
  /*returns if NaN*/
  if(exp == 0x7F800000)
    return 0x80000000u;
  /*rounds down to zero if exponent is zero*/
  if(!exp)
    return 0;
  /*rounds down to zero if there are no left shifts*/
  if(e < 0)
    return 0;
  /*return if out of range for ints*/
  if(e > 30)
    return 0x80000000u;

  frac = frac | 0x800000; /*normalized, append a 1 to the left of the frac*/
  if (e >= 23)
    frac = frac << (e-23); /*shift left if shift > 23*/
  else
    frac = frac >> (23 -e); /*else we need to shift right*/

  if(( uf >> 31 ) & 1) 
    return ~frac + 1; /*return negated value if sign bit is 1*/

  return frac;
}

/* 
 * floatPower2 - Return bit-level equivalent of the expression 2.0^x
 *   (2.0 raised to the power x) for any 32-bit integer x.
 *
 *   The unsigned value that is returned should have the identical bit
 *   representation as the single-precision floating-point number 2.0^x.
 *   If the result is too small to be represented as a denorm, return
 *   0. If too large, return +INF.
 * 
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. Also if, while 
 *   Max ops: 30 
 *   Rating: 4
 */
// unsigned floatPower2(int x) {
//   unsigned INF = 0xff << 23;
//   int e = 127 + x;
//   if (x < 0) return 0;
//   if (e >= 255) return INF;
//   return e << 23;
// }

