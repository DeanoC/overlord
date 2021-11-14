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
template<typename type>
CONST_EXPR ALWAYS_INLINE type Clz(const type a) {
	if(sizeof(type) >= 8) return __builtin_clzl(a);
	else if(sizeof(type) >= 4) return __builtin_clz(a);
	else if(sizeof(type) >= 2) return (type)__builtin_clz( ((uint32_t)a) << 16);
	else if(sizeof(type) >= 1) return (type)__builtin_clz( ((uint32_t)a) << 24);
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
CONST_EXPR ALWAYS_INLINE type CountToRightmostMask(const type a) { return ((1 << a)-1); }
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
 * We however define as the inverse of 1 << (n-1), so modify the clz
 * Not found will return ~0
 * */
template<typename type>
CONST_EXPR ALWAYS_INLINE type FindFirstStringOfOnes(type x, unsigned int n) {
	type s;
	while (n > 1) {
		s = n >> 1;
		x = x & (x >> s);
		n = n - s;
	}
	return (sizeof(type)*8) - Clz(x) - 1;
}

} // end namespace