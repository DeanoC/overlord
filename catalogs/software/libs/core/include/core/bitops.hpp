#pragma once

namespace BitOp {

template<typename type>
CONST_EXPR ALWAYS_INLINE type Not(const type a) { return ~a; }
template<typename type>
CONST_EXPR ALWAYS_INLINE type And(const type a, const type b) { return a & b; }
template<typename type>
CONST_EXPR ALWAYS_INLINE type Or(const type a, const type b) { return a | b; }
template<typename type>
CONST_EXPR ALWAYS_INLINE type Xor(const type a, const type b) { return a ^ b; }
template<typename type>
CONST_EXPR ALWAYS_INLINE type Nand(const type a, const type b) { return ~(a & b); }
template<typename type>
CONST_EXPR ALWAYS_INLINE type Nor(const type a, const type b) { return ~(a | b); }
template<typename type>
CONST_EXPR ALWAYS_INLINE type Xnor(const type a, const type b) { return (a | (~b)) & ((~a) | b); }
/*template<typename type>
CONST_EXPR ALWAYS_INLINE type Clz(const type a) {
	if(sizeof(type) >= 8) return __builtin_clzl(a);
	else if(sizeof(type) >= 4) return __builtin_clz(a);
	else if(sizeof(type) >= 2) return (type)__builtin_clz( ((uint32_t)a) << 16);
	else if(sizeof(type) >= 1) return (type)__builtin_clz( ((uint32_t)a) << 24);
}
*/
template<typename type>
CONST_EXPR ALWAYS_INLINE type Clz(type x) {
	if(sizeof(type) >= 8) {
		int n;
		if (x == 0) return(64);
		n = 0;
		if (x <= 0x0000FFFFFFFFFFFF) {n = n +32; x = x <<32;}
		if (x <= 0x0000FFFFFFFFFFFF) {n = n +16; x = x <<16;}
		if (x <= 0x00FFFFFFFFFFFFFF) {n = n + 8; x = x << 8;}
		if (x <= 0x0FFFFFFFFFFFFFFF) {n = n + 4; x = x << 4;}
		if (x <= 0x3FFFFFFFFFFFFFFF) {n = n + 2; x = x << 2;}
		if (x <= 0x7FFFFFFFFFFFFFFF) {n = n + 1;}
		return n;

	} else if(sizeof(type) >= 4) {
		int n;
		if (x == 0) return(32);
		n = 0;
		if (x <= 0x0000FFFF) {n = n +16; x = x <<16;}
		if (x <= 0x00FFFFFF) {n = n + 8; x = x << 8;}
		if (x <= 0x0FFFFFFF) {n = n + 4; x = x << 4;}
		if (x <= 0x3FFFFFFF) {n = n + 2; x = x << 2;}
		if (x <= 0x7FFFFFFF) {n = n + 1;}
		return n;
	}	else if(sizeof(type) >= 2) return (type)Clz( ((uint32_t)x) << 16);
	else if(sizeof(type) >= 1) return (type)Clz( ((uint32_t)x) << 24);
}

template<typename type>
CONST_EXPR ALWAYS_INLINE type Ctz(const type a) { return (sizeof(type)*8) - Clz(a & -a) -1; }
template<typename type>
CONST_EXPR ALWAYS_INLINE type Clo(const type a) { return  Clz(~a); }
template<typename type>
CONST_EXPR ALWAYS_INLINE type Cto(const type a) { return  Ctz(~a); }
template<typename type>
CONST_EXPR ALWAYS_INLINE type Pop(const type a) {
	if(sizeof(type) >= 8) return __builtin_popcountl(a);
	else if(sizeof(type) >= 4) return __builtin_popcount(a);
	else if(sizeof(type) >= 1) return (type)__builtin_popcount(a);
}

template<typename type>
CONST_EXPR ALWAYS_INLINE type PowerOfTwoContaining(type x) { return x <= 1 ? 1 : ((type)1) << ((sizeof(type) * 8) - BitOp::Clz(x-1)); }

template<typename type>
CONST_EXPR ALWAYS_INLINE type LogTwo(type const v) { return (sizeof(type) * 8) - BitOp::Clz(v) - 1; }

template<typename type>
CONST_EXPR ALWAYS_INLINE type PowerOfTwoToMask(type const v) { return v - 1; }

template<typename type>
CONST_EXPR ALWAYS_INLINE type CountToRightmostMask(const type a) { return (((1 << (a-1))-1)<<1)|1; } // handles overflow with a == sizeof(type)*8 without a branch, 0 is illegal
template<typename type>
CONST_EXPR ALWAYS_INLINE type RightmostMaskToCount(const type a) { return Clz(a+1); }
template<typename type>
CONST_EXPR ALWAYS_INLINE type SingleBitToCount(const type a) { return Clz(a); }
template<typename type>
CONST_EXPR ALWAYS_INLINE type CountLeadingZeros(const type a) { return Clz(a); }
template<typename type>
CONST_EXPR ALWAYS_INLINE type CountLeadingOnes(const type a) { return Clo(a); }
template<typename type>
CONST_EXPR ALWAYS_INLINE type CountTrailingZeros(const type a) { return Ctz(a); }
template<typename type>
CONST_EXPR ALWAYS_INLINE type CountTrailingOnes(const type a) { return Cto(a); }
template<typename type>
CONST_EXPR ALWAYS_INLINE type FindFirstSet(const type a) { return Ctz(a); }
template<typename type>
CONST_EXPR ALWAYS_INLINE type PopulationCount(const type a) { return Pop(a); }
/* Basic algorithm from Hackers Delight revision 2 Figure 6-5
 * Not found will return bit width of type
 * */
template<typename type>
CONST_EXPR ALWAYS_INLINE type FindFirstStringOfOnes(type x, unsigned int n) {
	type s;
	while (n > 1) {
		s = n >> 1;
		x = x & (x << s);
		n = n - s;
	}
	return Clz(x);
}

} // end namespace