#pragma once
#include "core/core.h"
#include "memory/memory.h"

#define ALLOC_CLASS(allocator, clas, ...) new( (clas*) MALLOC(allocator, sizeof(clas))) clas(__VA_ARGS__)
#define FREE_CLASS(allocator, clas, ptr) ptr->~clas(); MFREE(allocator, ptr);

namespace Memory {
		// a std allocator that uses our allocator framework.
		template<class T>
		class StdAllocatorAdaptor {
		public:
				using value_type = T;

				explicit StdAllocatorAdaptor(struct Memory_Allocator *allocator_) : allocator(allocator_) {}

				template<class U>
				explicit StdAllocatorAdaptor(Memory::StdAllocatorAdaptor <U> const &other) noexcept :
						allocator(other.allocator) {}

				value_type *allocate(size_t n) {
					return (value_type *) MCALLOC(allocator, n, sizeof(value_type));
				}

				void deallocate(value_type *p, size_t) noexcept  // Use pointer if pointer is not a value_type*
				{
					MFREE(allocator, p);
				}

		private:
				Memory_Allocator *allocator;
		};
}

template <class T, class U>
bool operator==(Memory::StdAllocatorAdaptor<T> const&, Memory::StdAllocatorAdaptor<U> const&) noexcept
{
	return true;
}

template <class T, class U>
bool operator!=(Memory::StdAllocatorAdaptor<T> const& x, Memory::StdAllocatorAdaptor<U> const& y) noexcept
{
	return !(x == y);
}
