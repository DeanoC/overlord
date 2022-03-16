#pragma once
#include "core/core.h"
#include "dbg/assert.h"
#include "core/utils.hpp"

namespace Utils {
	/// non owning slice of memory
	template<typename T> struct Slice {
			T const * const data;
			size_t const size;

			Slice(T *ptr_, size_t size_) : data(ptr_), size(size_) {}
			Slice(T *begin_, T * end_) : data(begin_), size(end_ - begin_) {}
	};

	template<typename T> struct TrackingSlice {
			Slice<T> slice;
			T const * current;
			TrackingSlice(Slice<T> slice_, size_t startOffset_ = 0) : slice(slice_), current(slice.data + startOffset_) {}

			T const * operator++() {
				current++;
				assert(current < slice.data+slice.size);
				return current;
			}
			T const * operator--() {
				current--;
				assert(current >= slice.data);
				return current;
			}
			T const& operator*() {
				return *current;
			}
	};
}