#define BITOP_FM_CREATE_UNSIGNED(postfix, type) 																										\
CONST_EXPR ALWAYS_INLINE type BitOp_Not##_##postfix(const type a) { return ~a; } 										\
CONST_EXPR ALWAYS_INLINE type BitOp_And##_##postfix(const type a, const type b) { return a & b; } 	\
CONST_EXPR ALWAYS_INLINE type BitOp_Or##_##postfix(const type a, const type b) { return a | b; } 		\
CONST_EXPR ALWAYS_INLINE type BitOp_Xor##_##postfix(const type a, const type b) { return a ^ b; } \
CONST_EXPR ALWAYS_INLINE type BitOp_Nand##_##postfix(const type a, const type b) { return ~(a & b); } 	\
CONST_EXPR ALWAYS_INLINE type BitOp_Nor##_##postfix(const type a, const type b) { return ~(a | b); } 		\
CONST_EXPR ALWAYS_INLINE type BitOp_Xnor##_##postfix(const type a, const type b) { return (a | (~b)) & ((~a) | b); } \
CONST_EXPR ALWAYS_INLINE type BitOp_Clz##_##postfix(const type a) {       	\
	if(sizeof(type) >= 8) return __builtin_clzl(a);     											\
	else if(sizeof(type) >= 4) return __builtin_clz(a);      									\
	else if(sizeof(type) >= 2) return (type)__builtin_clz( ((uint32_t)a) << 16);		\
	else if(sizeof(type) >= 1) return (type)__builtin_clz( ((uint32_t)a) << 24);		\
} 																																					\
CONST_EXPR ALWAYS_INLINE type BitOp_Ctz##_##postfix(const type a) { return (sizeof(type)*8) - BitOp_Clz##_##postfix(a & -a); }       \
CONST_EXPR ALWAYS_INLINE type BitOp_Clo##_##postfix(const type a) { return  BitOp_Clz##_##postfix(~a); }       \
CONST_EXPR ALWAYS_INLINE type BitOp_Cto##_##postfix(const type a) { return  BitOp_Ctz##_##postfix(~a); }       \
CONST_EXPR ALWAYS_INLINE type BitOp_Pop##_##postfix(const type a) {      	\
	if(sizeof(type) >= 8) return __builtin_popcountl(a);     								\
	else if(sizeof(type) >= 4) return __builtin_popcount(a);      					\
	else if(sizeof(type) >= 1) return (type)__builtin_popcount(a);					\
}                                                                         \
CONST_EXPR ALWAYS_INLINE type BitOp_PowerOfTwoContaining##_##postfix(type x) { return x <= 1 ? 1 : ((type)1) << ((sizeof(type) * 8) - BitOp_Clz##_##postfix(x-1)); } \
CONST_EXPR ALWAYS_INLINE type BitOp_LogTwo##_##postfix(type const v) { return (sizeof(type) * 8) - BitOp_Clz##_##postfix(v) - 1; } \
CONST_EXPR ALWAYS_INLINE type BitOp_PowerOfTwoToMask##_##postfix(type const v) { return v - 1; } \
CONST_EXPR ALWAYS_INLINE type BitOp_CountToRightmostMask##_##postfix(const type a) { return ((1 << a)-1); }          \
CONST_EXPR ALWAYS_INLINE type BitOp_RightmostMaskToCount##_##postfix(const type a) { return BitOp_Clz##_##postfix(a+1); }    \
CONST_EXPR ALWAYS_INLINE type BitOp_SingleBitToCount##_##postfix(const type a) { return BitOp_Clz##_##postfix(a); }       \
CONST_EXPR ALWAYS_INLINE type BitOp_CountLeadingZeros##_##postfix(const type a) { return BitOp_Clz##_##postfix(a); }       \
CONST_EXPR ALWAYS_INLINE type BitOp_CountLeadingOnes##_##postfix(const type a) { return BitOp_Clo##_##postfix(a); }       \
CONST_EXPR ALWAYS_INLINE type BitOp_CountTrailingZeros##_##postfix(const type a) { return BitOp_Ctz##_##postfix(a); }       \
CONST_EXPR ALWAYS_INLINE type BitOp_CountTrailingOnes##_##postfix(const type a) { return BitOp_Cto##_##postfix(a); }       \
CONST_EXPR ALWAYS_INLINE type BitOp_FindFirstSet##_##postfix(const type a) { return BitOp_Ctz##_##postfix(a); }   \
CONST_EXPR ALWAYS_INLINE type BitOp_PopulationCount##_##postfix(const type a) { return BitOp_Pop##_##postfix(a); } \
/* Basic algorithm from Hackers Delight revision 2 Figure 6-5 */ \
/* We however define as the inverse of 1 << (n-1), so modify the clz */ \
/* Not found will return ~0 */ \
CONST_EXPR ALWAYS_INLINE type BitOp_FindFirstStringOfOnes##_##postfix(type x, unsigned int n) { \
	type s = 0; 																																\
	while (n > 1) {																															\
		s = n >> 1;																 																\
		x = x & (x >> s);																													\
		n = n - s;																																\
	}																																						\
	return (sizeof(type)*8) - BitOp_Clz##_##postfix(x) - 1;											\
}

