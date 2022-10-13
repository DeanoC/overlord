// MIT like see full license at end of file
// Modified by Deano for Al2O3 and Confetti for TheForge
#pragma once

#include "core/core.h"
#include "tiny_stl/hash.hpp"

namespace tiny_stl {

class string_view {
 public:
  typedef char value_type;
  typedef char *pointer;
  typedef const char *const_pointer;
  typedef char& reference;
  typedef const char& const_reference;
  typedef const_pointer iterator;
  typedef const_pointer const_iterator;
  typedef size_t size_type;
  typedef ptrdiff_t difference_type;

  static constexpr size_type npos = size_type(-1);

  constexpr string_view();
  constexpr string_view(const char *s, size_type count);
  constexpr string_view(const char *s);
  constexpr string_view(const string_view&) = default;
  string_view& operator=(const string_view&) = default;

	template<typename allocator>
	inline string_view& operator=(const class string &other);

  constexpr const char *data() const;
  constexpr char operator[](size_type pos) const;
  constexpr size_type size() const;
  constexpr bool empty() const;
  constexpr iterator begin() const;
  constexpr const_iterator cbegin() const;
  constexpr iterator end() const;
  constexpr const_iterator cend() const;
  constexpr string_view substr(size_type pos = 0, size_type count = npos) const;
  constexpr void swap(string_view& v);

  constexpr bool starts_with(string_view x) const noexcept;
  constexpr bool starts_with(value_type x) const noexcept;
  constexpr bool ends_with(string_view x) const noexcept;
  constexpr bool ends_with(value_type x) const noexcept;
  constexpr size_type find(value_type ch, size_type pos = 0) const noexcept;
  constexpr size_type rfind(value_type ch, size_type pos = 0) const noexcept;
	constexpr bool operator==(string_view const& v) const noexcept;
	constexpr bool operator!=(string_view const& v) const noexcept;

 private:
  string_view(decltype(nullptr)) = delete;

  static constexpr size_type strlen(const char *);

  const char *m_str;
  size_type m_size;
};

constexpr string_view::string_view()
    : m_str(nullptr), m_size(0) {
}

constexpr string_view::string_view(const char *s, size_type count)
    : m_str(s), m_size(count) {
}

constexpr string_view::string_view(const char *s)
    : m_str(s), m_size(strlen(s)) {
}

constexpr const char *string_view::data() const {
  return m_str;
}

constexpr char string_view::operator[](size_type pos) const {
  return m_str[pos];
}

constexpr string_view::size_type string_view::size() const {
  return m_size;
}

constexpr bool string_view::empty() const {
  return 0 == m_size;
}

constexpr string_view::iterator string_view::begin() const {
  return m_str;
}

constexpr string_view::const_iterator string_view::cbegin() const {
  return m_str;
}

constexpr string_view::iterator string_view::end() const {
  return m_str + m_size;
}

constexpr string_view::const_iterator string_view::cend() const {
  return m_str + m_size;
}

constexpr string_view string_view::substr(size_type pos, size_type count) const {
  return string_view(m_str + pos, npos == count ? m_size - pos : count);
}

constexpr void string_view::swap(string_view& v) {
  const char *strtmp = m_str;
  size_type sizetmp = m_size;
  m_str = v.m_str;
  m_size = v.m_size;
  v.m_str = strtmp;
  v.m_size = sizetmp;
}

constexpr string_view::size_type string_view::strlen(const char *s) {
  for (size_t len = 0;; ++len) {
    if (0 == s[len]) {
      return len;
    }
  }
}
constexpr bool string_view::starts_with(string_view x) const noexcept {
  if(m_size < x.m_size) return false;

  for (auto i = 0u; i < x.m_size; ++i) {
    if(m_str[i] != x.m_str[i]) return false;
  }
  return true;
}

constexpr bool string_view::starts_with(value_type x) const noexcept {
  if(m_size == 0) return false;
  return m_str[0] == x;
}

constexpr bool string_view::ends_with(string_view x) const noexcept {
  if(m_size < x.m_size) return false;

  for (auto i = 0u; i < x.m_size; ++i) {
    if(m_str[m_size-1-i] != x.m_str[x.m_size-1-i]) return false;
  }
  return true;
}

constexpr bool string_view::ends_with(value_type x) const noexcept {
  if(m_size == 0) return false;
  return m_str[m_size-1] == x;
}

constexpr string_view::size_type string_view::find(value_type ch, size_type pos) const noexcept {
  if(pos >= m_size) return npos;

  for (auto i = pos; i < m_size ; ++i) {
    if(m_str[i] == ch) return i;
  }
  return npos;
}
constexpr string_view::size_type string_view::rfind(value_type ch, size_type pos) const noexcept {
  if(pos >= m_size) return npos;
  for (auto i = m_size - pos; i > 0; --i) {
    if (m_str[i - 1] == ch) { return i - 1; }
  }
  return npos;
}

constexpr bool string_view::operator==(string_view const& v) const noexcept {
	if(v.size() != size()) return false;

	char const* src = m_str;
	char const* dst = v.data();

	while(src < m_str + m_size) {
		if(*src != *dst) return false;
		src++;
		dst++;
	}
	return true;

}
constexpr bool string_view::operator!=(string_view const& v) const noexcept {
	if(v.size() != size()) return true;

	char const* src = m_str;
	char const* dst = v.data();

	while(src < m_str + m_size) {
		if(*src != *dst) return true;
		src++;
		dst++;
	}
	return false;
}

template<>
struct hash<string_view> {
	size_t operator()(const string_view &value_) {
		return hash_string(value_.data(), value_.size());
	}
};

}
/*-
 * Copyright 2012-1017 Matthew Endsley
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
