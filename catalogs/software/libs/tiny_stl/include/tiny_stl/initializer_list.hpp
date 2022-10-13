#pragma once

#include "core/core.h"

namespace tiny_stl {

template<class T> class initializer_list {
	T const * start;
	size_t    len;

	inline constexpr initializer_list(T const* s, size_t l) noexcept
			: start(s), len(l)	{}
public:
	typedef T        	value_type;
	typedef T const & reference;
	typedef T const & const_reference;
	typedef size_t 		size_type;

	typedef T const * iterator;
	typedef T const * const_iterator;

	inline constexpr initializer_list() noexcept : start(nullptr), len(0) {}

	inline constexpr size_t size()  const noexcept {return size;}

	inline constexpr T const * begin() const noexcept {return start;}

	inline constexpr T const * end()   const noexcept {return start + len;}
};

template< class E >
constexpr const E* begin( initializer_list<E> il ) noexcept {
	return il.begin();
}

template< class E >
constexpr const E* end( initializer_list<E> il ) noexcept {
	return il.end();
}

template< class E >
constexpr const E* rbegin( initializer_list<E> il ) noexcept {
	return il.end();
}

template< class E >
constexpr const E* rend( initializer_list<E> il ) noexcept {
	return il.begin();
}
}
