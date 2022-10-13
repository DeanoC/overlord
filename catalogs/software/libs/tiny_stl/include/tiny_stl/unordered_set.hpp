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

namespace tiny_stl {

	template<typename Key, class Hash = hash<Key>>
	class unordered_set {
	public:
		unordered_set(Memory_Allocator* allocator_);
		unordered_set(const unordered_set& other);
		unordered_set(unordered_set&& other) noexcept;
		~unordered_set();

		unordered_set& operator=(const unordered_set& other);
		unordered_set& operator=(unordered_set&& other);

		typedef unordered_hash_iterator<const unordered_hash_node<Key, void> > const_iterator;
		typedef const_iterator iterator;

		iterator begin() const;
		iterator end() const;

		void clear();
		bool empty() const;
		size_t size() const;

		iterator find(const Key& key) const;
		pair<iterator, bool> insert(const Key& key);
		pair<iterator, bool> emplace(Key&& key);
		void erase(iterator where);
		size_t erase(const Key& key);

		void swap(unordered_set& other);

	private:

		void rehash(size_t nbuckets);

		typedef unordered_hash_node<Key, void>* pointer;

		size_t m_size;
		buffer<pointer> m_buckets;
	};

	template<typename Key, class Hash>
	inline unordered_set<Key, Hash>::unordered_set(Memory_Allocator* allocator_)
		: m_size(0)
	{
		buffer_init<pointer>(&m_buckets, allocator_);
		buffer_resize<pointer>(&m_buckets, 9, 0);
	}

	template<typename Key, class Hash>
	inline unordered_set<Key, Hash>::unordered_set(const unordered_set& other)
		: m_size(other.m_size)
	{
		const size_t nbuckets = (size_t)(other.m_buckets.last - other.m_buckets.first);
		buffer_init<pointer>(&m_buckets, other.allocator);
		buffer_resize<pointer>(&m_buckets, nbuckets, 0);

		for (pointer it = *other.m_buckets.first; it; it = it->next) {
			void* mem = MALLOC(m_buckets.allocator, sizeof(unordered_hash_node<Key, void>));
			auto newnode = new(placeholder(), mem) unordered_hash_node<Key, void>(*it);
			newnode->next = newnode->prev = 0;
			unordered_hash_node_insert(newnode, hash<Hash>(it->first), m_buckets.first, nbuckets - 1);
		}
	}

	template<typename Key, class Hash>
	inline unordered_set<Key, Hash>::unordered_set(unordered_set&& other) noexcept
	: m_size(other.m_size)
	{
		buffer_move(&m_buckets, &other.m_buckets);
		other.m_size = 0;
	}

	template<typename Key, class Hash>
	inline unordered_set<Key, Hash>::~unordered_set() {
		if (m_buckets.first != m_buckets.last)
			clear();
		buffer_destroy<pointer>(&m_buckets);
	}

	template<typename Key, class Hash>
	inline unordered_set<Key, Hash>& unordered_set<Key, Hash>::operator=(const unordered_set<Key, Hash>& other) {
		unordered_set<Key, Hash>(other).swap(*this);
		return *this;
	}

	template<typename Key, class Hash>
	inline unordered_set<Key, Hash>& unordered_set<Key, Hash>::operator=(unordered_set&& other) {
		unordered_set(static_cast<unordered_set&&>(other)).swap(*this);
		return *this;
	}

	template<typename Key, class Hash>
	inline typename unordered_set<Key, Hash>::iterator unordered_set<Key, Hash>::begin() const {
		iterator cit;
		cit.node = *m_buckets.first;
		return cit;
	}

	template<typename Key, class Hash>
	inline typename unordered_set<Key, Hash>::iterator unordered_set<Key, Hash>::end() const {
		iterator cit;
		cit.node = 0;
		return cit;
	}

	template<typename Key, class Hash>
	inline bool unordered_set<Key, Hash>::empty() const {
		return m_size == 0;
	}

	template<typename Key, class Hash>
	inline size_t unordered_set<Key, Hash>::size() const {
		return m_size;
	}

	template<typename Key, class Hash>
	inline void unordered_set<Key, Hash>::clear() {
		pointer it = *m_buckets.first;
		while (it) {
			const pointer next = it->next;
			it->~unordered_hash_node<Key, void>();
			MFREE(m_buckets.allocator, it);

			it = next;
		}

		m_buckets.last = m_buckets.first;
		buffer_resize<pointer>(&m_buckets, 9, 0);
		m_size = 0;
	}

	template<typename Key, class Hash>
	inline typename unordered_set<Key, Hash>::iterator unordered_set<Key, Hash>::find(const Key& key) const {
		iterator result;
		result.node = unordered_hash_find(key, m_buckets.first, (size_t)(m_buckets.last - m_buckets.first), Hash()(key));
		return result;
	}

	template<typename Key, class Hash>
	inline void unordered_set<Key, Hash>::rehash(size_t nbuckets) {
		if (m_size + 1 > 4 * nbuckets) {
			pointer root = *m_buckets.first;

			const size_t newnbuckets = ((size_t)(m_buckets.last - m_buckets.first) - 1) * 8;
			m_buckets.last = m_buckets.first;
			buffer_resize<pointer>(&m_buckets, newnbuckets + 1, 0);
			unordered_hash_node<Key, void>** buckets = m_buckets.first;

			while (root) {
				const pointer next = root->next;
				root->next = root->prev = 0;
				unordered_hash_node_insert(root, Hash()(root->first), buckets, newnbuckets);
				root = next;
			}
		}
	}

	template<typename Key, class Hash>
	inline pair<typename unordered_set<Key, Hash>::iterator, bool> unordered_set<Key, Hash>::insert(const Key& key) {
		pair<iterator, bool> result;
		result.second = false;

		result.first = find(key);
		if (result.first.node != 0)
			return result;

		void * mem = MALLOC(m_buckets.allocator, sizeof(unordered_hash_node<Key, void>));
		auto* newnode = new(placeholder(), mem) unordered_hash_node<Key, void>(key);
		newnode->next = newnode->prev = 0;

		const size_t keyhash = Hash()(key);
		const size_t nbuckets = (size_t)(m_buckets.last - m_buckets.first);
		unordered_hash_node_insert(newnode, keyhash, m_buckets.first, nbuckets - 1);

		++m_size;
		rehash(nbuckets);

		result.first.node = newnode;
		result.second = true;
		return result;
	}

	template<typename Key, class Hash>
	inline pair<typename unordered_set<Key, Hash>::iterator, bool> unordered_set<Key, Hash>::emplace(Key&& key) {
				pair<iterator, bool> result;
		result.second = false;

		result.first = find(key);
		if (result.first.node != 0)
			return result;

		const size_t keyhash = Hash(key);
		void* mem = MALLOC(m_buckets.allocator, sizeof(unordered_hash_node<Key, void>));
		auto newnode = new(placeholder(), mem) unordered_hash_node<Key, void>(static_cast<Key&&>(key));
		newnode->next = newnode->prev = 0;

		const size_t nbuckets = (size_t)(m_buckets.last - m_buckets.first);
		unordered_hash_node_insert(newnode, keyhash, m_buckets.first, nbuckets - 1);

		++m_size;
		rehash(nbuckets);

		result.first.node = newnode;
		result.second = true;
		return result;
	}

	template<typename Key, class Hash>
	inline void unordered_set<Key, Hash>::erase(iterator where) {
		unordered_hash_node_erase(where.node, hash<Hash>(where.node->first), m_buckets.first, (size_t)(m_buckets.last - m_buckets.first) - 1);

		where.node->~unordered_hash_node<Key, void>();
		MFREE(m_buckets.allocator, (void*)where.node);
		--m_size;
	}

	template<typename Key, class Hash>
	inline size_t unordered_set<Key, Hash>::erase(const Key& key) {
		const iterator it = find(key);
		if (it.node == 0)
			return 0;

		erase(it);
		return 1;
	}

	template <typename Key, class Hash>
	void unordered_set<Key, Hash>::swap(unordered_set& other) {
		size_t tsize = other.m_size;
		other.m_size = m_size, m_size = tsize;
		buffer_swap(&m_buckets, &other.m_buckets);
	}
}