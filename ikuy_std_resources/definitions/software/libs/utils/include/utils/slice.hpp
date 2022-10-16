#pragma once
#include "core/core.h"
#include "dbg/assert.h"
#include "core/utils.hpp"

namespace Utils {
	/// non owning slice of memory
	template<typename T> struct Slice {
			T * const data;
			size_t const size;
	};

	template<typename T> struct TrackingSlice {
			Slice<T> slice;
			T * current;
			explicit TrackingSlice(Slice<T> slice_, size_t startOffset_ = 0) : slice(slice_), current(slice.data + startOffset_) {}

			void increment(size_t val) {
				assert(current+val <= slice.data + slice.size);
				current += val;
			}

			WARN_UNUSED_RESULT size_t left() const { return (slice.data + slice.size) - current; }

			// pre-increment
			T * operator++() {
				assert(current+1 <= slice.data + slice.size);
				return ++current;
			}
			T const * operator--() {
				assert(current-1 >= slice.data);
				return --current;
			}
			// post-increment
			T * operator++(int) {
				assert(current+1 <= slice.data + slice.size);
				return current++;
			}
			T * operator--(int) {
				assert(current-1 >= slice.data);
				return current--;
			}

			// deference
			T const& operator*() const {
				return *current;
			}
			T & operator*() {
				return *current;
			}
	};
}