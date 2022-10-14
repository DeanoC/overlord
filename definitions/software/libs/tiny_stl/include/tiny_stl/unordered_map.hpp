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
#include "memory/memory.h"
#include "tiny_stl/buffer.hpp"
#include "tiny_stl/hash.hpp"
#include "tiny_stl/hash_base.hpp"
#include "tiny_stl/initializer_list.hpp"

namespace tiny_stl {

template<typename Key, typename Value, class Hash = hash<Key>>
class unordered_map {
public:
	typedef pair<Key, Value> value_type;

	typedef unordered_hash_node<Key, Value> const const_hash_node;
	typedef unordered_hash_node<Key, Value> hash_node;

	typedef unordered_hash_iterator<const unordered_hash_node<Key, Value> > const_iterator;
	typedef unordered_hash_iterator<unordered_hash_node<Key, Value> > iterator;

	constexpr explicit unordered_map(Memory_Allocator* allocator_);
	constexpr unordered_map(const unordered_map &other);
	constexpr unordered_map(unordered_map &&other) noexcept ;
	~unordered_map();
	constexpr unordered_map(initializer_list<value_type> const &init, Memory_Allocator* allocator_);

	unordered_map &operator=(const unordered_map &other);
	unordered_map &operator=(unordered_map &&other) noexcept ;

	iterator begin();
	iterator end();

	const_iterator begin() const;
	const_iterator end() const;
	const_iterator cbegin() const { return begin(); }
	const_iterator cend() const { return end(); }

	void clear();
	bool empty() const;
	size_t size() const;

	const_iterator find(const Key &key) const;
	iterator find(const Key &key);
	pair<iterator, bool> insert(const pair<Key, Value> &p);
	pair<iterator, bool> emplace(pair<Key, Value> &&p);
	void erase(const_iterator where);

	Value &operator[](const Key &key);

	void swap(unordered_map &other);

	pair<iterator, bool> insert_or_assign(const Key &k, Value &&obj);
	pair<unordered_map<Key, Value, Hash>::iterator, bool> insert_or_assign(const_iterator hint, const Key &k, Value &&obj);
	pair<iterator, bool> insert_or_assign(Key &&k, Value &&obj);
	pair<unordered_map<Key, Value, Hash>::iterator, bool> insert_or_assign(const_iterator hint, Key &&k, Value &&obj);

private:
	void rehash(size_t nbuckets);

	typedef unordered_hash_node<Key, Value> *pointer;

	size_t m_size;
	buffer<pointer> m_buckets;
};

template<typename Key, typename Value, class Hash>
constexpr inline unordered_map<Key, Value, Hash>::unordered_map(Memory_Allocator* allocator_)
		: m_size(0) {
	buffer_init<pointer>(&m_buckets, allocator_);
	buffer_resize<pointer>(&m_buckets, 9, 0);
}

template<typename Key, typename Value, class Hash>
constexpr inline unordered_map<Key, Value, Hash>::unordered_map(const unordered_map &other)
		: m_size(other.m_size) {
	const size_t nbuckets = (size_t) (other.m_buckets.last - other.m_buckets.first);
	buffer_init<pointer>(&m_buckets, other.m_buckets.allocator);
	buffer_resize<pointer>(&m_buckets, nbuckets, 0);

	for (pointer it = *other.m_buckets.first; it; it = it->next) {
		void* mem = MALLOC(m_buckets.allocator, sizeof(unordered_hash_node<Key, Value>));
		auto newnode = new(placeholder(), mem) unordered_hash_node<Key, Value>(it->first, it->second);
		newnode->next = newnode->prev = 0;

		unordered_hash_node_insert(newnode, Hash()(it->first), m_buckets.first, nbuckets - 1);
	}
}

template<typename Key, typename Value, class Hash>
constexpr inline unordered_map<Key, Value, Hash>::unordered_map(unordered_map &&other) noexcept
		: m_size(other.m_size) {
	buffer_move(&m_buckets, &other.m_buckets);
	other.m_size = 0;
}

template<typename Key, typename Value, class Hash>
inline unordered_map<Key, Value, Hash>::~unordered_map() {
	if (m_buckets.first != m_buckets.last)
		clear();
	buffer_destroy<pointer>(&m_buckets);
}

template<typename Key, typename Value, class Hash>
inline unordered_map<Key, Value, Hash> &unordered_map<Key,Value,Hash>::operator=(const unordered_map<Key,Value,Hash> &other) {
	unordered_map<Key, Value, Hash>(other).swap(*this);
	return *this;
}

template<typename Key, typename Value, class Hash>
inline unordered_map<Key, Value, Hash> &unordered_map<Key,Value,Hash>::operator=(unordered_map &&other) noexcept {
	unordered_map(static_cast<unordered_map &&>(other)).swap(*this);
	return *this;
}

template<typename Key, typename Value, class Hash>
inline typename unordered_map<Key, Value, Hash>::iterator unordered_map<Key, Value, Hash>::begin() {
	iterator it;
	it.node = *m_buckets.first;
	return it;
}

template<typename Key, typename Value, class Hash>
inline typename unordered_map<Key, Value, Hash>::iterator unordered_map<Key, Value, Hash>::end() {
	iterator it;
	it.node = 0;
	return it;
}

template<typename Key, typename Value, class Hash>
inline typename unordered_map<Key, Value, Hash>::const_iterator unordered_map<Key, Value,Hash>::begin() const {
	const_iterator cit;
	cit.node = *m_buckets.first;
	return cit;
}

template<typename Key, typename Value, class Hash>
inline typename unordered_map<Key, Value, Hash>::const_iterator unordered_map<Key,Value,Hash>::end() const {
	const_iterator cit;
	cit.node = 0;
	return cit;
}

template<typename Key, typename Value, class Hash>
inline bool unordered_map<Key, Value, Hash>::empty() const {
	return m_size == 0;
}

template<typename Key, typename Value, class Hash>
inline size_t unordered_map<Key, Value, Hash>::size() const {
	return m_size;
}

template<typename Key, typename Value, class Hash>
inline void unordered_map<Key, Value, Hash>::clear() {
	pointer it = *m_buckets.first;
	while (it) {
		const pointer next = it->next;
		it->~unordered_hash_node<Key, Value>();
		MFREE(m_buckets.allocator, it);

		it = next;
	}

	m_buckets.last = m_buckets.first;
	buffer_resize<pointer>(&m_buckets, 9, 0);
	m_size = 0;
}

template<typename Key, typename Value, class Hash>
inline typename unordered_map<Key, Value, Hash>::iterator unordered_map<Key,Value,Hash>::find(const Key &key) {
	iterator result;
	result.node = unordered_hash_find(key, m_buckets.first, (size_t) (m_buckets.last - m_buckets.first), Hash()(key));
	return result;
}

template<typename Key, typename Value, class Hash>
inline typename unordered_map<Key, Value, Hash>::const_iterator unordered_map<Key, Value, Hash>::find(
		const Key &key) const {
	iterator result;
	result.node = unordered_hash_find(key, m_buckets.first, (size_t) (m_buckets.last - m_buckets.first));
	return result;
}

template<typename Key, typename Value, class Hash>
inline void unordered_map<Key, Value, Hash>::rehash(size_t nbuckets) {
	if (m_size + 1 > 4 * nbuckets) {
		pointer root = *m_buckets.first;

		const size_t newnbuckets = ((size_t) (m_buckets.last - m_buckets.first) - 1) * 8;
		m_buckets.last = m_buckets.first;
		buffer_resize<pointer>(&m_buckets, newnbuckets + 1, 0);
		unordered_hash_node<Key, Value> **buckets = m_buckets.first;

		while (root) {
			const pointer next = root->next;
			root->next = root->prev = 0;
			unordered_hash_node_insert(root, Hash()(root->first), buckets, newnbuckets);
			root = next;
		}
	}
}

template<typename Key, typename Value, class Hash>
inline pair<typename unordered_map<Key, Value, Hash>::iterator, bool> unordered_map<Key,Value,Hash>::insert(
		const pair<Key,Value> &p) {
	pair<iterator, bool> result;
	result.second = false;

	result.first = find(p.first);
	if (result.first.node != 0)
		return result;

	void* mem = MALLOC(m_buckets.allocator, sizeof(unordered_hash_node<Key, Value>));
	auto newnode = new(placeholder(), mem) unordered_hash_node<Key,Value>(p.first,p.second);
	newnode->next = newnode->prev = 0;

	const size_t nbuckets = (size_t) (m_buckets.last - m_buckets.first);
	unordered_hash_node_insert(newnode, Hash()(p.first), m_buckets.first, nbuckets - 1);

	++m_size;
	rehash(nbuckets);

	result.first.node = newnode;
	result.second = true;
	return result;
}

template<typename Key, typename Value, class Hash>
inline pair<typename unordered_map<Key, Value, Hash>::iterator, bool> unordered_map<Key,Value,Hash>::emplace(
		pair<Key,Value> &&p) {
	pair<iterator, bool> result;
	result.second = false;

	result.first = find(p.first);
	if (result.first.node != 0)
		return result;

	const size_t keyhash = Hash(p.first);
	void* mem = MALLOC(m_buckets.allocator, sizeof(unordered_hash_node<Key, Value>));
	auto newnode = new(placeholder(), mem) unordered_hash_node<Key,Value>(static_cast<Key &&>(p.first), static_cast<Value &&>(p.second));
	newnode->next = newnode->prev = 0;

	const size_t nbuckets = (size_t) (m_buckets.last - m_buckets.first);
	unordered_hash_node_insert(newnode, keyhash, m_buckets.first, nbuckets - 1);

	++m_size;
	rehash(nbuckets);

	result.first.node = newnode;
	result.second = true;
	return result;
}

template<typename Key, typename Value, class Hash>
inline void unordered_map<Key, Value, Hash>::erase(const_iterator where) {
	unordered_hash_node_erase(where.node,
														hash<Hash>(where->first),
														m_buckets.first,
														(size_t) (m_buckets.last - m_buckets.first) - 1);

	where->~unordered_hash_node<Key, Value>();
	MFREE(m_buckets.allocator, (void *) where.node);
	--m_size;
}

template<typename Key, typename Value, class Hash>
inline Value &unordered_map<Key, Value, Hash>::operator[](const Key &key) {
	return insert(pair<Key, Value>(key, Value())).first->second;
}

template<typename Key, typename Value, class Hash>
inline void unordered_map<Key, Value, Hash>::swap(unordered_map &other) {
	size_t tsize = other.m_size;
	other.m_size = m_size, m_size = tsize;
	buffer_swap(&m_buckets, &other.m_buckets);
}

template<typename Key, typename Value, class Hash>
inline pair<typename unordered_map<Key, Value, Hash>::iterator, bool> unordered_map<Key,Value,Hash>::insert_or_assign(
		const Key &k,Value &&obj) {
	pair<iterator, bool> result;

	result.first = find(k);

	if (result.first.node != 0) {
		result.first.node->second = obj;
		result.second = false;
	} else {
		insert(pair<Key,Value>(k, obj));
		result.second = true;
	}
	return result;
}

template<typename Key, typename Value, class Hash>
inline pair<typename unordered_map<Key, Value, Hash>::iterator, bool> unordered_map<Key,Value,Hash>::insert_or_assign(
		const_iterator hint, const Key &k, Value &&obj) {
	pair<iterator, bool> result;

	if(hint.node->first == k) {
		result.first = hint;
	} else {
		result.first = find(k);
	}

	if (result.first.node != 0) {
		result.first.node->second = obj;
		result.second = false;
	} else {
		insert(pair<Key, Value>(k, obj));
		result.second = true;
	}
	return result;
}

template<typename Key, typename Value, class Hash>
inline pair<typename unordered_map<Key, Value, Hash>::iterator, bool> unordered_map<Key, Value, Hash>::insert_or_assign(Key &&k,
																			 Value &&obj) {
	pair<iterator, bool> result;

	result.first = find(k);

	if (result.first.node != 0) {
		result.first.node->second = obj;
		result.second = false;
	} else {
		insert(pair<Key, Value>(k, obj));
		result.second = true;
	}
	return result;
}

template<typename Key, typename Value, class Hash>
inline pair<typename unordered_map<Key, Value, Hash>::iterator, bool> unordered_map<Key, Value, Hash>::insert_or_assign(
		const_iterator hint, Key &&k, Value &&obj) {
	pair<iterator, bool> result;

	if(hint.node->first == k) {
		result.first = hint;
	} else {
		result.first = find(k);
	}

	if (result.first.node != 0) {
		result.first.node->second = obj;
		result.second = false;
	} else {
		insert(pair<Key,Value>(k, obj));
		result.second = true;
	}
	return result;

}

} // end namespace
