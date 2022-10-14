#pragma once

#define STB_EXTERN EXTERN_C

//////////////////////////////////////////////////////////////////////////////
//
//                               Hashing
//
//      typical use for this is to make a power-of-two hash table.
//
//      let N = size of table (2^n)
//      let H = stb_hash(str)
//      let S = stb_rehash(H) | 1
//
//      then hash probe sequence P(i) for i=0..N-1
//         P(i) = (H + S*i) & (N-1)
//
//      the idea is that H has 32 bits of hash information, but the
//      table has only, say, 2^20 entries so only uses 20 of the bits.
//      then by rehashing the original H we get 2^12 different probe
//      sequences for a given initial probe location. (So it's optimal
//      for 64K tables and its optimality decreases past that.)


STB_EXTERN uint32_t stb_hash_numberU32(uint32_t number);
STB_EXTERN uint32_t stb_hash_stringU32(char const *str);
STB_EXTERN uint32_t stb_hash_ptrU32(void const *p);
STB_EXTERN uint32_t stb_hash_dataU32(uint8_t const * data, size_t len);
STB_EXTERN uint32_t stb_rehashU32(uint32_t v);

STB_EXTERN uint64_t stb_hash_numberU64(uint64_t number);
STB_EXTERN uint64_t stb_hash_stringU64(char const *str);
STB_EXTERN uint64_t stb_hash_ptrU64(void const *p);
STB_EXTERN uint64_t stb_hash_dataU64(uint8_t const * data, size_t len);
STB_EXTERN uint64_t stb_rehashU64(uint64_t v);

STB_EXTERN size_t stb_hash_numberSizeT(size_t number) ;
STB_EXTERN size_t stb_hash_stringSizeT(char const *str);
STB_EXTERN size_t stb_hash_ptrSizeT(void const *p);
STB_EXTERN size_t stb_hash_dataSizeT(uint8_t const * data, size_t len);
STB_EXTERN size_t stb_rehashSizeT(uint64_t v);


#ifdef STB_DEFINE

uint32_t stb_hash_numberU32(uint32_t hash)
{
   hash ^= hash << 3;
   hash += hash >> 5;
   hash ^= hash << 4;
   hash += hash >> 17;
   hash ^= hash << 25;
   hash += hash >> 6;
   return hash;
}

uint32_t stb_hash_stringU32(char const *str)
{
   uint32_t hash = 0;
   while (*str)
      hash = (hash << 7) + (hash >> 25) + *str++;
   return hash + (hash >> 16);
}

uint32_t stb_hash_ptrU32(void const *p)
{
#define stb_rehash(x)  ((x) + ((x) >> 6) + ((x) >> 19))
    uint32_t x = (uint32_t)(size_t) p;

   // typically lacking in low bits and high bits
   x = stb_rehash(x);
   x += x << 16;

   // pearson's shuffle
   x ^= x << 3;
   x += x >> 5;
   x ^= x << 2;
   x += x >> 15;
   x ^= x << 10;
   return stb_rehash(x);
#undef stb_rehash
}

uint32_t stb_rehashU32(uint32_t v)
{
   return stb_hash_ptrU32((void *)(size_t) v);
}

uint32_t stb_hash_dataU32(uint8_t const *q, size_t len)
{
// Paul Hsieh hash
#define stb__get16(p) ((p)[0] | ((p)[1] << 8))
   uint32_t hash = (uint32_t)len;

   if (len <= 0 || q == NULL) return 0;

   /* Main loop */
   for (;len > 3; len -= 4) {
      unsigned int val;
      hash +=  stb__get16(q);
      val   = (stb__get16(q+2) << 11);
      hash  = (hash << 16) ^ hash ^ val;
      q    += 4;
      hash += hash >> 11;
   }

   /* Handle end cases */
   switch (len) {
      case 3: hash += stb__get16(q);
              hash ^= hash << 16;
              hash ^= q[2] << 18;
              hash += hash >> 11;
              break;
      case 2: hash += stb__get16(q);
              hash ^= hash << 11;
              hash += hash >> 17;
              break;
      case 1: hash += q[0];
              hash ^= hash << 10;
              hash += hash >> 1;
              break;
      case 0: break;
   }
#undef stb__get16

   /* Force "avalanching" of final 127 bits */
   hash ^= hash << 3;
   hash += hash >> 5;
   hash ^= hash << 4;
   hash += hash >> 17;
   hash ^= hash << 25;
   hash += hash >> 6;

   return hash;
}

uint64_t stb_hash_numberU64(uint64_t hash)
{
   hash ^= hash << 3;
   hash += hash >> 5;
   hash ^= hash << 4;
   hash += hash >> 17;
   hash ^= hash << 25;
   hash += hash >> 6;

   hash ^= hash << 35;
   hash += hash >> 37;
   hash ^= hash << 36;
   hash += hash >> 49;
   hash ^= hash << 57;
   hash += hash >> 38;
   return hash;
}

uint64_t stb_hash_stringU64(char const *str)
{
   uint64_t hash = 0;
   while (*str)
      hash = (hash << 7) + (hash >> 25) + (hash << 39) + (hash >> 49) + *str++;
   return hash + (hash >> 16) + (hash >> 48);
}

uint64_t stb_hash_ptrU64(void const *p)
{
#define stb_rehash(x)  ((x) + ((x) >> 6) + ((x) >> 19) + ((x) >> 38) + ((x) >> 51))
    uint64_t x = (uint64_t)(size_t) p;

   // typically lacking in low bits and high bits
   x = stb_rehash(x);
   x += x << 16;
   x += x << 48;

   // pearson's shuffle
   x ^= x << 3;
   x += x >> 5;
   x ^= x << 2;
   x += x >> 15;
   x ^= x << 10;

   x ^= x << 35;
   x += x >> 37;
   x ^= x << 34;
   x += x >> 47;
   x ^= x << 42;
   return stb_rehash(x);
#undef stb_rehash
}

uint64_t stb_rehashU64(uint64_t v)
{
   return stb_hash_ptrU64((void *)(size_t) v);
}

uint64_t stb_hash_dataU64(uint8_t const *q, size_t len)
{
// Paul Hsieh hash
#define stb__get16(p) ((p)[0] | ((p)[1] << 8))
   size_t hash = len;

   if (len <= 0 || q == NULL) return 0;

   /* Main loop */
   for (;len > 3; len -= 4) {
      unsigned int val;
      hash +=  stb__get16(q);
      val   = (stb__get16(q+2) << 11);
      hash  = (hash << 16) ^ hash ^ val;
      q    += 4;
      hash += hash >> 11;
   }

   /* Handle end cases */
   switch (len) {
      case 3: hash += stb__get16(q);
              hash ^= hash << 16;
              hash ^= q[2] << 18;
              hash += hash >> 11;
              break;
      case 2: hash += stb__get16(q);
              hash ^= hash << 11;
              hash += hash >> 17;
              break;
      case 1: hash += q[0];
              hash ^= hash << 10;
              hash += hash >> 1;
              break;
      case 0: break;
   }
#undef stb__get16

   /* Force "avalanching" of final 127 bits */
   hash ^= hash << 3;
   hash += hash >> 5;
   hash ^= hash << 4;
   hash += hash >> 17;
   hash ^= hash << 25;
   hash += hash >> 6;

   return hash;
}

size_t stb_hash_numberSizeT(size_t number) {
	return stb_hash_numberU64(number);
}
size_t stb_hash_stringSizeT(char const *str) {
	return stb_hash_stringU64(str);
}
size_t stb_hash_ptrSizeT(void const *p) {
	return stb_hash_ptrU64(p);
}
size_t stb_hash_dataSizeT(uint8_t const * data, size_t len) {
	return stb_hash_dataU64(data, len);
}
size_t stb_rehashSizeT(uint64_t v) {
	return stb_rehashU64(v);
}
#endif
