#pragma once
#define AL2O3_STB_STB_DICT_H
// few changes from stbs original
// now have support fixed size hash tables for common cases
// uses a memory allocator
#include "memory/memory.h"
#include "stb_hash.h"

//////////////////////////////////////////////////////////////////////////////
//
//                     Instantiated data structures
//
// This is an attempt to implement a templated data structure.
//
// Hash table: call stb_define_hash(TYPE,N,KEY,K1,K2,HASH,VALUE)
//     TYPE     -- will define a structure type containing the hash table
//     N        -- the name, will prefix functions named:
//                        N create
//                        N destroy
//                        N get
//                        N set, N add, N update,
//                        N remove
//     KEY      -- the type of the key. 'x == y' must be valid
//       K1,K2  -- keys never used by the app, used as flags in the hashtable
//       HASH   -- a piece of code ending with 'return' that hashes key 'k'
//     VALUE    -- the type of the value. 'x = y' must be valid
//
//  Note that stb_define_hash_base can be used to define more sophisticated
//  hash tables, e.g. those that make copies of the key or use special
//  comparisons (e.g. strcmp).

#define STB_(prefix, name)     stb__##prefix##name
#define STB__(prefix, name)    prefix##name
#define STB__use(x)           x
#define STB__skip(x)

#define stb_declare_hash(PREFIX, TYPE, N, KEY, VALUE) \
   typedef struct stb__st_##TYPE TYPE;\
   typedef struct STB_(N, _hashpair)\
   {\
      KEY   k;\
      VALUE v;\
   } STB_(N,_hashpair);\
   PREFIX int STB__(N, init)(TYPE *h, size_t count, Memory_Allocator* allocator);\
   PREFIX size_t STB__(N, memory_usage)(TYPE *h);\
   PREFIX TYPE * STB__(N, create)(Memory_Allocator* allocator);\
   PREFIX TYPE * STB__(N, create_with_reserve)(Memory_Allocator* allocator, size_t count);\
   PREFIX TYPE * STB__(N, copy)(TYPE *h);\
   PREFIX void STB__(N, destroy)(TYPE *h);\
   PREFIX int STB__(N,get_flag)(TYPE const *a, KEY k, VALUE *v);\
   PREFIX VALUE STB__(N,get)(TYPE const *a, KEY k);\
   PREFIX int STB__(N, set)(TYPE *a, KEY k, VALUE v);\
   PREFIX int STB__(N, add)(TYPE *a, KEY k, VALUE v);\
   PREFIX int STB__(N, update)(TYPE*a,KEY k,VALUE v);\
   PREFIX void STB__(N, reserve)(TYPE*a, size_t count); \
   PREFIX int STB__(N, remove)(TYPE *a, KEY k, VALUE *v);

#define stb_declare_fixed_sized_hash(PREFIX, TYPE, N, KEY, VALUE) 							\
    stb_declare_hash(PREFIX,TYPE,N,KEY,VALUE) 																	\
    typedef struct stb__st_##TYPE {                                             \
     STB_(N,_hashpair) *table;                                                  \
     size_t mask;                                                         			\
     size_t count, limit;                                                       \
     size_t deleted;                                                            \
                                                                                \
     size_t delete_threshhold;                                                  \
     size_t grow_threshhold;                                                    \
     size_t shrink_threshhold;                                                  \
     unsigned char alloced, has_empty, has_del;                                 \
     VALUE ev; VALUE dv;                                                        \
     Memory_Allocator* allocator;                                               \
    } TYPE;

#define STB_nocopy(x)        (x)
#define STB_nodelete(x)
#define STB_nofields
#define STB_nonullvalue(x)
#define STB_nullvalue(x)     x
#define STB_safecompare(x)   x
#define STB_nosafe(x)
#define STB_noprefix

#ifdef __GNUC__
#define STB__nogcc(x)
#else
#define STB__nogcc(x)  x
#endif

#define stb_define_fixed_sized_hash_base(PREFIX, TYPE, N, NC, LOAD_FACTOR, 		\
                             KEY, EMPTY, DEL, COPY, DISPOSE, SAFE, 						\
                             VCOMPARE, CCOMPARE, HASH, 												\
                             VALUE, HASVNULL, VNULL)                          \
                                                                              \
                                                                              \
STB__nogcc( typedef struct stb__st_##TYPE TYPE;  )                            \
                                                                              \
static size_t STB_(N, hash)(KEY k)                                      			\
{                                                                             \
   HASH                                                                       \
}                                                                             \
                                                                              \
PREFIX int STB__(N, init)(TYPE *h, size_t count, Memory_Allocator* allocator) \
{                                                                             \
   size_t i;                                                                  \
   if (count < 4) count = 4;                                                  \
   h->limit = count;                                                          \
   h->count = 0;                                                              \
   h->mask  = count-1;                                                        \
   h->deleted = 0;                                                            \
   h->grow_threshhold = (size_t) (count * LOAD_FACTOR);                       \
   h->has_empty = h->has_del = 0;                                             \
   h->alloced = 0;                                                            \
   h->allocator = allocator;                                                  \
   if (count <= 64)                                                           \
      h->shrink_threshhold = 0;                                               \
   else                                                                       \
      h->shrink_threshhold = (size_t) (count * (LOAD_FACTOR/2.25));           \
   h->delete_threshhold = (size_t) (count * (1-LOAD_FACTOR)/2);               \
   h->table = (STB_(N,_hashpair)*) MALLOC(h->allocator, sizeof(h->table[0]) * count);  \
   if (h->table == NULL) return 0;                                            \
   /* ideally this gets turned into a memset32 automatically */               \
   for (i=0; i < count; ++i)                                                  \
      h->table[i].k = EMPTY;                                                  \
   return 1;                                                                  \
}                                                                             \
                                                                              \
PREFIX size_t STB__(N, memory_usage)(TYPE *h)                                 \
{                                                                             \
   return sizeof(*h) + h->limit * sizeof(h->table[0]);                        \
}                                                                             \
                                                                              \
PREFIX TYPE * STB__(N, create_with_reserve)(Memory_Allocator* allocator, size_t count)           \
{                                                                             \
   TYPE *h = (TYPE *) MALLOC(allocator, sizeof(*h));                          \
   if (h) {                                                                   \
      if (STB__(N, init)(h, count, allocator))                                \
         h->alloced = 1;                                                      \
      else { MFREE(allocator,h); h=NULL; }                                    \
   }                                                                          \
   return h;                                                                  \
}                                                                             \
                                                                              \
PREFIX TYPE * STB__(N, create)(Memory_Allocator* allocator)                   \
{                                                                             \
	return (STB__(N, create_with_reserve)(allocator, 16));                      \
}                                                                             \
                                                                              \
PREFIX void STB__(N, destroy)(TYPE *a)                                        \
{                                                                             \
   size_t i;                                                                  \
   for (i=0; i < a->limit; ++i)                                               \
      if (!CCOMPARE(a->table[i].k,EMPTY) && !CCOMPARE(a->table[i].k, DEL))    \
         DISPOSE(a->table[i].k);                                              \
   MFREE(a->allocator, a->table);                                             \
   if (a->alloced)                                                            \
      MFREE(a->allocator, a);                                                 \
}                                                                             \
                                                                              \
static void STB_(N, rehash)(TYPE *a, size_t count);                           \
                                                                              \
PREFIX int STB__(N,get_flag)(TYPE const *a, KEY k, VALUE *v)                  \
{                                                                             \
   size_t h = STB_(N, hash)(k);                                         			\
   size_t n = h & a->mask, s;                             			              \
   if (CCOMPARE(k,EMPTY)){ if (a->has_empty) *v = a->ev; return a->has_empty;}\
   if (CCOMPARE(k,DEL)) { if (a->has_del  ) *v = a->dv; return a->has_del;   }\
   if (CCOMPARE(a->table[n].k,EMPTY)) return 0;                               \
   SAFE(if (!CCOMPARE(a->table[n].k,DEL)))                                    \
   if (VCOMPARE(a->table[n].k,k)) { *v = a->table[n].v; return 1; }           \
   s = STB_(N, hash)((KEY)h) | 1;                                             \
   for(;;) {                                                                  \
      n = (n + s) & a->mask;                                                  \
      if (CCOMPARE(a->table[n].k,EMPTY)) return 0;                            \
      SAFE(if (CCOMPARE(a->table[n].k,DEL)) continue;)                        \
      if (VCOMPARE(a->table[n].k,k))                                          \
         { *v = a->table[n].v; return 1; }                                    \
   }                                                                          \
}                                                                             \
                                                                              \
HASVNULL(                                                                     \
   PREFIX VALUE STB__(N,get)(TYPE const *a, KEY k)                            \
   {                                                                          \
      VALUE v;                                                                \
      if (STB__(N,get_flag)(a,k,&v)) return v;                                \
      else                           return VNULL;                            \
   }                                                                          \
)                                                                             \
                                                                              \
PREFIX int STB__(N,getkey)(TYPE const *a, KEY k, KEY *kout)                   \
{                                                                             \
   size_t h = STB_(N, hash)(k);                                         			\
   size_t n = h & a->mask, s;                                           			\
   if (CCOMPARE(k,EMPTY)||CCOMPARE(k,DEL)) return 0;                          \
   if (CCOMPARE(a->table[n].k,EMPTY)) return 0;                               \
   SAFE(if (!CCOMPARE(a->table[n].k,DEL)))                                    \
   if (VCOMPARE(a->table[n].k,k)) { *kout = a->table[n].k; return 1; }        \
   s = STB_(N, hash)((KEY)h) | 1;                                             \
   for(;;) {                                                                  \
      n = (n + s) & a->mask;                                                  \
      if (CCOMPARE(a->table[n].k,EMPTY)) return 0;                            \
      SAFE(if (CCOMPARE(a->table[n].k,DEL)) continue;)                        \
      if (VCOMPARE(a->table[n].k,k))                                          \
         { *kout = a->table[n].k; return 1; }                                 \
   }                                                                          \
}                                                                             \
PREFIX void STB__(N,reserve)(TYPE *a, size_t count)               						\
{																																							\
  STB_(N,rehash)(a, count);					                                  				\
}                                                                             \
static int STB_(N,addset)(TYPE *a, KEY k, VALUE v,                            \
                             int allow_new, int allow_old, int copy)          \
{                                                                             \
   size_t h = STB_(N, hash)(k);                                         			\
   size_t n = h & a->mask;                                              			\
   int64_t b = -1;                                                            \
   if (CCOMPARE(k,EMPTY)) {                                                   \
      if (a->has_empty ? allow_old : allow_new) {                             \
          n=a->has_empty; a->ev = v; a->has_empty = 1; return !n;             \
      } else return 0;                                                        \
   }                                                                          \
   if (CCOMPARE(k,DEL)) {                                                     \
      if (a->has_del ? allow_old : allow_new) {                               \
          n=a->has_del; a->dv = v; a->has_del = 1; return !n;                 \
      } else return 0;                                                        \
   }                                                                          \
   if (!CCOMPARE(a->table[n].k, EMPTY)) {                                     \
      size_t s;                                                         			\
      if (CCOMPARE(a->table[n].k, DEL))                                       \
         b = n;                                                               \
      else if (VCOMPARE(a->table[n].k,k)) {                                   \
         if (allow_old)                                                       \
            a->table[n].v = v;                                                \
         return !allow_new;                                                   \
      }                                                                       \
   		s = STB_(N, hash)((KEY)h) | 1;                                          \
      for(;;) {                                                               \
         n = (n + s) & a->mask;                                               \
         if (CCOMPARE(a->table[n].k, EMPTY)) break;                           \
         if (CCOMPARE(a->table[n].k, DEL)) {                                  \
            if (b < 0) b = n;                                                 \
         } else if (VCOMPARE(a->table[n].k,k)) {                              \
            if (allow_old)                                                    \
               a->table[n].v = v;                                             \
            return !allow_new;                                                \
         }                                                                    \
      }                                                                       \
   }                                                                          \
   if (!allow_new) return 0;                                                  \
   if (b < 0) b = n; else --a->deleted;                                       \
   a->table[b].k = copy ? COPY(k) : k;                                        \
   a->table[b].v = v;                                                         \
   ++a->count;                                                                \
   if (a->count > a->grow_threshhold)                                         \
      STB_(N,rehash)(a, a->limit*2);                                          \
   return 1;                                                                  \
}                                                                             \
                                                                              \
PREFIX int STB__(N, set)(TYPE *a, KEY k, VALUE v){return STB_(N,addset)(a,k,v,1,1,1);}\
PREFIX int STB__(N, add)(TYPE *a, KEY k, VALUE v){return STB_(N,addset)(a,k,v,1,0,1);}\
PREFIX int STB__(N, update)(TYPE*a,KEY k,VALUE v){return STB_(N,addset)(a,k,v,0,1,1);}\
                                                                              \
PREFIX int STB__(N, remove)(TYPE *a, KEY k, VALUE *v)                         \
{                                                                             \
   size_t h = STB_(N, hash)(k);                                         			\
   size_t n = h & a->mask, s;                                           			\
   if (CCOMPARE(k,EMPTY)) { if (a->has_empty) { if(v)*v = a->ev; a->has_empty=0; return 1; } return 0; } \
   if (CCOMPARE(k,DEL))   { if (a->has_del  ) { if(v)*v = a->dv; a->has_del  =0; return 1; } return 0; } \
   if (CCOMPARE(a->table[n].k,EMPTY)) return 0;                               \
   if (SAFE(CCOMPARE(a->table[n].k,DEL) || ) !VCOMPARE(a->table[n].k,k)) {    \
   		s = STB_(N, hash)((KEY)h) | 1;                                          \
      for(;;) {                                                               \
         n = (n + s) & a->mask;                                               \
         if (CCOMPARE(a->table[n].k,EMPTY)) return 0;                         \
         SAFE(if (CCOMPARE(a->table[n].k, DEL)) continue;)                    \
         if (VCOMPARE(a->table[n].k,k)) break;                                \
      }                                                                       \
   }                                                                          \
   DISPOSE(a->table[n].k);                                                    \
   a->table[n].k = DEL;                                                       \
   --a->count;                                                                \
   ++a->deleted;                                                              \
   if (v != NULL)                                                             \
      *v = a->table[n].v;                                                     \
   if (a->count < a->shrink_threshhold)                                       \
      STB_(N, rehash)(a, a->limit >> 1);                                      \
   else if (a->deleted > a->delete_threshhold)                                \
      STB_(N, rehash)(a, a->limit);                                           \
   return 1;                                                                  \
}                                                                             \
                                                                              \
PREFIX TYPE * STB__(NC, copy)(TYPE *a)                                        \
{                                                                             \
   size_t i;                                                                  \
   TYPE *h = (TYPE *) MALLOC(a->allocator, sizeof(*h));                        \
   if (!h) return NULL;                                                       \
   if (!STB__(N, init)(h, a->limit, a->allocator)) { MFREE(a->allocator, a); return NULL; }    \
   h->count = a->count;                                                       \
   h->deleted = a->deleted;                                                   \
   h->alloced = 1;                                                            \
   h->ev = a->ev; h->dv = a->dv;                                              \
   h->has_empty = a->has_empty; h->has_del = a->has_del;                      \
   memcpy(h->table, a->table, h->limit * sizeof(h->table[0]));                \
   for (i=0; i < a->limit; ++i)                                               \
      if (!CCOMPARE(h->table[i].k,EMPTY) && !CCOMPARE(h->table[i].k,DEL))     \
         h->table[i].k = COPY(h->table[i].k);                                 \
   return h;                                                                  \
}                                                                             \
                                                                              \
static void STB_(N, rehash)(TYPE *a, size_t count)                            \
{                                                                             \
   size_t i;                                                                  \
   TYPE b;                                                                    \
   STB__(N, init)(&b, count, a->allocator);                                   \
   for (i=0; i < a->limit; ++i)                                               \
      if (!CCOMPARE(a->table[i].k,EMPTY) && !CCOMPARE(a->table[i].k,DEL))     \
         STB_(N,addset)(&b, a->table[i].k, a->table[i].v,1,1,0);              \
   MFREE(a->allocator, a->table);                                             \
   a->table = b.table;                                                        \
   a->mask = b.mask;                                                          \
   a->count = b.count;                                                        \
   a->limit = b.limit;                                                        \
   a->deleted = b.deleted;                                                    \
   a->delete_threshhold = b.delete_threshhold;                                \
   a->grow_threshhold = b.grow_threshhold;                                    \
   a->shrink_threshhold = b.shrink_threshhold;                                \
}

#define stb_define_hash_base(PREFIX, TYPE, FIELDS, N, NC, LOAD_FACTOR, \
                             KEY, EMPTY, DEL, COPY, DISPOSE, SAFE, \
                             VCOMPARE, CCOMPARE, HASH, \
                             VALUE, HASVNULL, VNULL)                            \
   typedef struct stb__st_##TYPE {                                              \
     FIELDS                                                                     \
     STB_(N,_hashpair) *table;                                                  \
     size_t mask;                                                  				      \
     size_t count, limit;                                                       \
     size_t deleted;                                                            \
                                                                                \
     size_t delete_threshhold;                                                  \
     size_t grow_threshhold;                                                    \
     size_t shrink_threshhold;                                                  \
     unsigned char alloced, has_empty, has_del;                                 \
     VALUE ev; VALUE dv;                                                        \
     Memory_Allocator* allocator;                                               \
  } TYPE;                                                                       \
  stb_define_hash_base(PREFIX, TYPE, N, NC, LOAD_FACTOR,                        \
                      KEY, EMPTY, DEL, COPY, DISPOSE, SAFE,                     \
                      VCOMPARE, CCOMPARE, HASH                                  \
                      VALUE, HASVNULL, VNULL                                    \
      );

#define STB_equal(a, b)  ((a) == (b))

#define stb_define_fixed_sized_hash(TYPE, N, KEY, EMPTY, DEL, HASH, VALUE)      \
   stb_define_fixed_sized_hash_base(STB_noprefix, TYPE, N, N, 0.85f,            \
              KEY,EMPTY,DEL,STB_nocopy,STB_nodelete,STB_nosafe,                 \
              STB_equal,STB_equal,HASH,                                         \
              VALUE,STB_nonullvalue,0)

#define stb_define_fixed_sized_hash_vnull(TYPE, N, KEY, EMPTY, DEL, HASH, VALUE, VNULL)          \
   stb_define_fixed_sized_hash_base(STB_noprefix, TYPE,N,N,0.85f,               \
              KEY,EMPTY,DEL,STB_nocopy,STB_nodelete,STB_nosafe,                 \
              STB_equal,STB_equal,HASH,                                         \
              VALUE,STB_nullvalue,VNULL)

//////////////////////////////////////////////////////////////////////////////
//
//                        stb_ptrmap
//
// An stb_ptrmap data structure is an O(1) hash table between pointers. One
// application is to let you store "extra" data associated with pointers,
// which is why it was originally called stb_extra.

stb_declare_fixed_sized_hash(EXTERN_C, stb_dictPtr, stb_dictPtr_, void *, void *)
stb_declare_fixed_sized_hash(EXTERN_C, stb_dictI32, stb_dictI32_, int32_t, int32_t)
stb_declare_fixed_sized_hash(EXTERN_C, stb_dictU32, stb_dictU32_, uint32_t, uint32_t)
stb_declare_fixed_sized_hash(EXTERN_C, stb_dictI64, stb_dictI64_, int64_t, int64_t)
stb_declare_fixed_sized_hash(EXTERN_C, stb_dictU64, stb_dictU64_, uint64_t, uint64_t)
stb_declare_fixed_sized_hash(EXTERN_C, stb_dictSizeT, stb_dictSizeT_, size_t, size_t)

#ifdef STB_DEFINE

stb_define_fixed_sized_hash_vnull(stb_dictPtr, stb_dictPtr_, void*,
                ((void *) 2),((void *) 6),
                return stb_hash_ptrSizeT(k);, void*, NULL)
/*
stb_define_fixed_sized_hash(stb_dictI32, stb_dictI32_, int32_t,
    (-1),(-2),
    return stb_rehashU32(k);, int32_t) ;

stb_define_fixed_sized_hash(stb_dictU32, stb_dictU32_, uint32_t,
    (~0),(~1),
    return stb_rehashU32(k);, uint32_t);

stb_define_fixed_sized_hash(stb_dictI64, stb_dictI64_, int64_t,
    (-1),(-2),
    return stb_rehashU64(k);, int64_t);

stb_define_fixed_sized_hash(stb_dictU64, stb_dictU64_, uint64_t,
    (~0),(~1),
    return stb_rehashU64(k);, uint64_t);
*/
#endif
