#pragma once
#include "tiny_stl/pdqsort.hpp"

namespace tiny_stl {

template<class Iter, class Compare>
inline void sort(Iter begin, Iter end, Compare comp) {
	tiny_stl::pdqsort(begin, end, comp);
}

template<class Iter>
inline void sort(Iter begin, Iter end) {
	typedef typename std::iterator_traits<Iter>::value_type T;
	tiny_stl::pdqsort(begin, end, std::less<T>());
}

template<class Iter>
inline ptrdiff_t distance(Iter const lsh, Iter const rhs) {
	return lsh - rhs;
}

} // end namespace
