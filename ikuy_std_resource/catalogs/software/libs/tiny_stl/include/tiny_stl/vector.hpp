/*-
 * Copyright 2012-2018 Matthew Endsley
 * All rights reserved
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted providing that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#pragma once

#include "core/core.h"
#include "dbg/assert.h"
#include "memory/memory.h"
#include "tiny_stl/buffer.hpp"
#include "tiny_stl/new.hpp"
#include "tiny_stl/iterators.hpp"

namespace tiny_stl {

template<typename T>
class vector {
public:
	typedef T value_type;
	typedef T* pointer_type;
	typedef iterator_impl<T, base_iterator_tag> iterator;
	typedef forward_iterator_impl<T> forward_iterator;
	typedef reverse_iterator_impl<T> reverse_iterator;
	typedef forward_iterator_impl<T const> const_forward_iterator;
	typedef reverse_iterator_impl<T const> const_reverse_iterator;

	explicit vector(Memory_Allocator* allocator_) {buffer_init(&m_buffer, allocator_); }
	vector(const vector& other);
	vector(vector&& other) noexcept { buffer_move(&m_buffer, &other.m_buffer); }
	vector(size_t size, Memory_Allocator* allocator_);
	vector(size_t size, const T& value, Memory_Allocator* allocator_);
	vector(const T *first, const T *last, Memory_Allocator* allocator_);
	~vector() { buffer_destroy(&m_buffer); };

	vector& operator=(const vector& other);
	vector& operator=(vector&& other) noexcept ;

	void assign(const T *first, const T *last);

	const T *data() const { return m_buffer.first; }
	T *data() { return m_buffer.first; }
	WARN_UNUSED_RESULT size_t size() const { return (size_t) (m_buffer.last - m_buffer.first); }
	WARN_UNUSED_RESULT size_t capacity() const { return (size_t) (m_buffer.capacity - m_buffer.first); }
	WARN_UNUSED_RESULT bool empty() const { return m_buffer.last == m_buffer.first; }

	T& operator[](size_t idx) {	return m_buffer.first[idx]; }
	const T& operator[](size_t idx) const {	return m_buffer.first[idx]; }

	T& at(size_t idx) { assert(m_buffer.first+idx < m_buffer.last); return m_buffer.first[idx]; }
	const T& at(size_t idx) const {	assert(m_buffer.first+idx < m_buffer.last); return m_buffer.first[idx]; }

	const T& front() const { return m_buffer.first[0]; }
	T& front() {return m_buffer.first[0]; }
	const T& back() const {	return m_buffer.last[-1]; }
	T& back() {	return m_buffer.last[-1]; }

	void resize(size_t size) { buffer_resize(&m_buffer, size); }
	void resize(size_t size, const T& value) { buffer_resize(&m_buffer, size, value); }
	void clear() { buffer_clear(&m_buffer); }
	void reserve(size_t capacity) { buffer_reserve(&m_buffer, capacity); }

	void push_back(const T& t) { buffer_append(&m_buffer, &t); }
	void emplace_back() { buffer_append(&m_buffer); }
	template<typename Param> void emplace_back(const Param& param) { buffer_append(&m_buffer, &param); }
	void pop_back() {	buffer_erase(&m_buffer, m_buffer.last - 1, m_buffer.last); }


	void shrink_to_fit() { buffer_shrink_to_fit(&m_buffer); }

	void swap(vector& other) { buffer_swap(&m_buffer, &other.m_buffer);}

	forward_iterator begin() { return forward_iterator{ m_buffer.first }; }
	forward_iterator end() { return forward_iterator{ m_buffer.last }; }
	reverse_iterator rbegin() { return reverse_iterator{ m_buffer.last-1 }; }
	reverse_iterator rend() { return reverse_iterator{ m_buffer.first-1 }; }

	const_forward_iterator cbegin() const { return const_forward_iterator{ m_buffer.first }; }
	const_forward_iterator cend() const { return const_forward_iterator{ m_buffer.last }; }
	const_forward_iterator begin() const { return const_forward_iterator{ m_buffer.first }; }
	const_forward_iterator end() const { return const_forward_iterator{ m_buffer.last }; }
	const_reverse_iterator rbegin() const { return const_reverse_iterator{ m_buffer.last - 1 }; }
	const_reverse_iterator rend() const { return const_reverse_iterator{ m_buffer.first - 1 }; }

	void insert(iterator where) { buffer_insert(&m_buffer, where, 1);}
	void insert(iterator where, const T& value) {	buffer_insert(&m_buffer, where, &value, &value + 1);}
	void insert(iterator where, const T *first, const T *last) { buffer_insert(&m_buffer, where, first, last);}

	template<typename Param>
	void emplace(iterator where, const Param& param) { buffer_insert(&m_buffer, where, &param, &param + 1); }

	iterator erase(iterator where) { return buffer_erase(&m_buffer, where, where + 1);}
	iterator erase(iterator first, iterator last) { return buffer_erase(&m_buffer, first, last);}

	iterator erase_unordered(iterator where){	return buffer_erase_unordered(&m_buffer, where, where + 1);}
	iterator erase_unordered(iterator first, iterator last){ return buffer_erase_unordered(&m_buffer, first, last); }

private:
	buffer <T> m_buffer;
};

template<typename T>
inline vector<T>::vector(const vector& other) {
	buffer_init(&m_buffer, other.m_buffer.allocator);
	buffer_reserve(&m_buffer, other.size());
	buffer_insert(&m_buffer, m_buffer.last, other.m_buffer.first, other.m_buffer.last);
}

template<typename T>
inline vector<T>::vector(size_t size, Memory_Allocator* allocator_) {
	buffer_init(&m_buffer, allocator_);
	buffer_resize(&m_buffer, size);
}

template<typename T>
inline vector<T>::vector(size_t size, const T& value, Memory_Allocator* allocator_) {
	buffer_init(&m_buffer, allocator_);
	buffer_resize(&m_buffer, size, value);
}

template<typename T>
inline vector<T>::vector(const T *first, const T *last, Memory_Allocator* allocator_) {
	buffer_init(&m_buffer, allocator_);
	buffer_insert(&m_buffer, m_buffer.last, first, last);
}

template<typename T>
inline vector<T>& vector<T>::operator=(const vector& other) {
	vector(other).swap(*this);
	return *this;
}

template<typename T>
vector<T>& vector<T>::operator=(vector&& other) noexcept {
	buffer_destroy(&m_buffer);
	buffer_move(&m_buffer, &other.m_buffer);
	return *this;
}

template<typename T>
inline void vector<T>::assign(const T *first, const T *last) {
	buffer_clear(&m_buffer);
	buffer_insert(&m_buffer, m_buffer.last, first, last);
}

} //end namespace tinystl
