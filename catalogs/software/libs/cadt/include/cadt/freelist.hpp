#pragma once

#include "cadt/freelist.h"
#include "dbg/assert.h"
#include "memory/memory.h"

namespace Cadt {

// currently BaseFreeList and FreeList c++ only handle POD types (no ctor or dtor will be called)
struct BaseFreeList {
	template<typename T>
	WARN_UNUSED_RESULT static auto Create( size_t capacity, Memory_Allocator *allocator ) -> BaseFreeList * {
		return (BaseFreeList *) CADT_FreeListCreate( sizeof( T ), capacity, allocator );
	}

	WARN_UNUSED_RESULT static auto From( CADT_FreeListHandle fl ) -> BaseFreeList * {
		return (BaseFreeList *) fl;
	}

	static auto Destroy( BaseFreeList *v ) -> void {
		CADT_FreeListDestroy((CADT_FreeListHandle) v );
	}

	WARN_UNUSED_RESULT static auto Clone( BaseFreeList const *src ) -> BaseFreeList * {
		return (BaseFreeList *) CADT_FreeListClone((CADT_FreeListHandle) src );
	}

	explicit operator CADT_FreeListHandle() { return (CADT_FreeListHandle) this; }

	WARN_UNUSED_RESULT void *alloc() { return CADT_FreeListAlloc((CADT_FreeListHandle) this ); };

	void release( void *ptr ) { return CADT_FreeListRelease((CADT_FreeListHandle) this, ptr ); };

	BaseFreeList() = delete;

	~BaseFreeList() = delete;
};

template<typename T, bool ispod = __is_pod( T )>
struct FreeList;

template<typename T>
struct FreeList<T, true> : private BaseFreeList {

	WARN_UNUSED_RESULT static auto Create( size_t capacity, Memory_Allocator *allocator ) -> FreeList * {
		return (FreeList *) BaseFreeList::Create<T>( capacity, allocator );
	}

	WARN_UNUSED_RESULT static auto From( CADT_FreeListHandle fl ) -> FreeList * {
		assert( CADT_FreeListElementSize( fl ) == sizeof( T ));
		return (FreeList *) BaseFreeList::From( fl );
	}

	static auto Destroy( FreeList *v ) -> void {
		BaseFreeList::Destroy( v );
	}

	WARN_UNUSED_RESULT static auto Clone( FreeList const *src ) -> FreeList * {
		return (FreeList *) BaseFreeList::Clone( src );
	}

	WARN_UNUSED_RESULT T * alloc() { return (T*) BaseFreeList::alloc(); };

	auto clone() { return Clone( this ); }

	auto destroy() { Destroy( this ); }

	void release( T *ptr ) { return BaseFreeList::release( ptr ); };

	FreeList() = delete;

	~FreeList() = delete;
};
};