/*-
 * Copyright 2012-2018 Matthew Endsley
 * Modified by Confetti and DeanoC
 * All rights reserved
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted providing that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other_ materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR other_WISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#pragma once

#include "core/core.h"
#include "dbg/assert.h"
#include "core/utf8.h"
#include "memory/memory.h"
#include "tiny_stl/hash.hpp"
#include "tiny_stl/string_view.hpp"
#include <cinttypes>

namespace tiny_stl {
	class string {
	public:
			typedef char value_type;
			typedef char *pointer;
			typedef const char *const_pointer;
			typedef char &reference;
			typedef const char &const_reference;
			typedef const_pointer iterator;
			typedef const_pointer const_iterator;
			typedef size_t size_type;

			explicit string(Memory_Allocator* allocator_);
			string(const string &other_);
			string(string &&other_) noexcept ;
			string(const char *sz_, Memory_Allocator* allocator_);
			explicit string(const char *sz_, size_t len_, Memory_Allocator* allocator_);
			explicit string(const char *begin_, const char *end_, Memory_Allocator* allocator_);
			string(const string_view &other_, Memory_Allocator* allocator_);
			string(string const &other_, size_t pos_, size_t len_);
			~string();

			static const size_type npos = -1;

			string &operator=(const string &other_);
			string &operator=(string &&other_) noexcept;

			operator string_view() const { return {first, size()}; }

			WARN_UNUSED_RESULT char const * c_str() const { return first; }

			WARN_UNUSED_RESULT char at(size_t index_) const { assert(index_ < size()); return first[index_]; }
			char& at(size_t index_) { assert(index_ < size()); return first[index_]; }

			char operator[](size_t index_) const { return first[index_]; }
			char &operator[](size_t index_) { return first[index_]; }

			WARN_UNUSED_RESULT size_type size() const;
			WARN_UNUSED_RESULT size_type length() const { return size(); }

			WARN_UNUSED_RESULT bool empty() const { return size() == 0; }

			WARN_UNUSED_RESULT string substr(size_type start_, size_t len_) const { return string(first + start_, len_, allocator); }

			void reserve(size_type capacity_);
			void resize(size_type size_);

			void clear();
			string& append(string const &str_) { return append(str_.data(), str_.size()); };
			string& append(const char *first_, const char *last_);
			string& append(size_t n_, const char c_) { return assign(&c_, n_); };
			string& append(const char c_) { return append(&c_, 1); };
			string& append(const char *first_, size_t n_) { return append(first_, first_ + n_); };
			string& assign(const char *sz_, size_t n_);
			void push_back(char c_) { append(c_); }

			WARN_UNUSED_RESULT char const * data() const { return first; }
			WARN_UNUSED_RESULT char * data() { return first; }
			WARN_UNUSED_RESULT char const * begin() const { return first; }
			WARN_UNUSED_RESULT char * begin() { return first; }
			WARN_UNUSED_RESULT char const * end() const { return first + size(); }
			WARN_UNUSED_RESULT char * end() { return first + size(); }

			WARN_UNUSED_RESULT char front() const { return *first; }
			WARN_UNUSED_RESULT char back() const;

			void shrink_to_fit();
			void swap(string & other_);

			void replace(char replaceThis_, char replaceWith_, bool caseSensitive_ = true);

			void erase(size_t start_, size_t end_ = npos);

			void pop_front() { erase(0,1); }
			void pop_back() { resize(size()-1); }

			WARN_UNUSED_RESULT size_type find(char c_, size_type startPos_ = npos, bool caseSensitive_ = true) const;
			WARN_UNUSED_RESULT size_type find(string_view const & str_, size_type startPos_ = npos, bool caseSensitive_ = true) const;

			WARN_UNUSED_RESULT size_type rfind(char ch_, size_t pos_ = npos, bool caseSensitive_ = true) const;
			WARN_UNUSED_RESULT size_type rfind(string_view const & str_, size_type startPos_ = npos, bool caseSensitive_ = true) const;

			WARN_UNUSED_RESULT size_type find_last(char c_, size_type startPos_ = npos, bool caseSensitive_ = true) const;
			WARN_UNUSED_RESULT size_type find_last(string_view const & str_, size_type startPos_ = npos, bool caseSensitive_ = true) const;
			WARN_UNUSED_RESULT size_type find_first_of(char const *tofind_, size_t startPos_ = npos) const;
			WARN_UNUSED_RESULT size_type find_first_not_of(char const *tofind_, size_t startPos_ = npos) const;
			WARN_UNUSED_RESULT size_type find_first_of(char tofind_, size_t startPos_ = npos) const;
			WARN_UNUSED_RESULT size_type find_last_of(char tofind_, size_t startPos_ = npos) const;

			WARN_UNUSED_RESULT int compare(char const * str_) const;
			WARN_UNUSED_RESULT int compare(string const & str_) const;

			WARN_UNUSED_RESULT string to_lower() const;
			WARN_UNUSED_RESULT string to_upper() const;

			WARN_UNUSED_RESULT Memory_Allocator * get_allocator() const { return allocator; }

	private:
			Memory_Allocator * allocator;
			// TODO string interning ~20 bytes extra for 64 bit pointers!
			pointer first;
			pointer last;
			uint32_t capacity;
			// 3 pointers at 64bit = 24 bytes at 32 bit = 12 bytes
			// + 4 bytes for capacity = 28 and 16 bytes
			// 32 byte fixed size
			// small string buffer capacity = 4 or 16 bytes
			static const size_t c_nbuffer = sizeof(pointer) == 8 ? 4 : 16;
			char m_buffer[c_nbuffer];
	};

	static_assert(sizeof(string) == 32);

	inline string::string(Memory_Allocator* allocator_) :
			m_buffer(),
			first(m_buffer),
			last(m_buffer),
			capacity(c_nbuffer),
			allocator(allocator_) {
		resize(0);
	}

	inline string::string(const string &other_) :
			m_buffer(),
			first(m_buffer),
			last(m_buffer),
			capacity(c_nbuffer),
			allocator(other_.allocator) {
		reserve(other_.size());
		append(other_.first, other_.last);
	}


	inline string::string(string &&other_) noexcept : m_buffer() {
		if (other_.first == other_.m_buffer) {
			first = m_buffer;
			last = m_buffer;
			capacity = c_nbuffer;
			allocator = other_.allocator;
			reserve(other_.size());
			append(other_.first, other_.last);
		} else {
			first = other_.first;
			last = other_.last;
			capacity = other_.capacity;
			allocator = other_.allocator;
		}
		other_.first = other_.last = other_.m_buffer;
		other_.capacity = c_nbuffer;
		other_.resize(0);
	}


	inline string::string(const char *sz_, Memory_Allocator* allocator_) :
			m_buffer(),
			first(m_buffer),
			last(m_buffer),
			capacity(c_nbuffer),
			allocator(allocator_) {
		size_t len = 0;
		for (const char *it = sz_; *it; ++it) {
			++len;
		}

		reserve(len);
		append(sz_, sz_ + len);
	}

	inline string::string(const char *sz_, size_t len_, Memory_Allocator* allocator_) :
			m_buffer(),
			first(m_buffer),
			last(m_buffer),
			capacity(c_nbuffer),
			allocator(allocator_) {
		reserve(len_);
		append(sz_, sz_ + len_);
	}

	inline string::string(const char *begin_, const char *end_, Memory_Allocator* allocator_) :
			m_buffer(),
			first(m_buffer),
			last(m_buffer),
			capacity(c_nbuffer),
			allocator(allocator_) {
		if (end_ - begin_ < 0) {
			return;
		}
		reserve(end_ - begin_);
		append(begin_, end_);
	}

	inline string::string(string const &other_, size_t pos_, size_t len_) :
			string(other_.data() + pos_, len_, other_.allocator) {}

	inline string::string(const string_view &other_, Memory_Allocator* allocator_) :
			string(other_.data(), other_.size(), allocator_) {}


	inline string::~string() {
		if (first != m_buffer) {
			MFREE(allocator, first);
		}
	}


	inline string &string::operator=(const string &other_) {
		string(other_).swap(*this);
		return *this;
	}


	inline string &string::operator=(string &&other_) noexcept {
		string(static_cast<string &&>(other_)).swap(*this);
		return *this;
	}

	inline typename string::size_type string::size() const {
		return (size_t) (last - first);
	}

	inline void string::reserve(size_type capacity_) {
		if (capacity_ + 1 <= capacity) {
			return;
		}

		const size_t size = (size_t) (last - first);

		pointer newfirst = (pointer) MALLOC(allocator, capacity_ + 1);
		for (pointer it = first, newit = newfirst, end = last; it != end; ++it, ++newit) {
			*newit = *it;
		}
		if (first != m_buffer) {
			MFREE(allocator, first);
		}

		first = newfirst;
		last = newfirst + size;
		capacity = capacity_;
	}

	inline void string::resize(size_type size_) {
		const size_t prevSize = last - first;
		reserve(size_);
		if (size_ > prevSize) {
			for (pointer it = last, end = first + size_ + 1; it < end; ++it) {
				*it = 0;
			}
		} else if (last != first) {
			first[size_] = 0;
		}

		last = first + size_;
	}

	inline void string::clear() {
		resize(0);
	}


	inline string& string::append(const char *first_, const char *last_) {
		const size_t newsize = (size_t) ((last - first) + (last_ - first_) + 1);
		if (newsize > capacity) {
			reserve((newsize * 3) / 2);
		}

		for (; first_ != last_; ++last, ++first_) {
			*last = *first_;
		}
		*last = 0;
		return *this;
	}

	inline string& string::assign(const char *sz_, size_t n_) {
		clear();
		return append(sz_, sz_ + n_);
	}


	inline void string::shrink_to_fit() {
		if (first == m_buffer) {
			// small string buffer, so nothing to shrink
		} else if (last == first) {
			// zero sized, so free buffer and restore small string buffer
			MFREE(allocator, first);
			first = m_buffer;
			last = m_buffer;
			capacity = c_nbuffer;
		} else if (capacity != (last - first)) {
			// need to shrink, so realloc
			// TODO we could use the memory alloactor realloc here if is_pos?
			const size_t size = (size_t) (last - first);
			char *newfirst = (pointer) MALLOC(allocator, size + 1);
			for (pointer in = first, out = newfirst;
			     in != last + 1;
			     ++in, ++out) {
				*out = *in;
			}
			MFREE(allocator, first);
			first = newfirst;
			last = newfirst + size;
			capacity = size;
		}
	}

	inline void string::swap(string &other_) {
		const pointer tfirst = first, tlast = last;
		const uint32_t tcapacity = capacity;
		first = other_.first, last = other_.last, capacity = other_.capacity;
		other_.first = tfirst, other_.last = tlast, other_.capacity = tcapacity;
		Memory_Allocator* tmp = other_.allocator;
		other_.allocator = allocator;
		allocator = tmp;

		char tbuffer[c_nbuffer];
		if (first == other_.m_buffer) {
			for (pointer it = other_.m_buffer, end = last, out = tbuffer; it != end; ++it, ++out) {
				*out = *it;
			}
		}

		if (other_.first == m_buffer) {
			other_.last = (other_.last - other_.first) + other_.m_buffer;
			other_.first = other_.m_buffer;
			other_.capacity = c_nbuffer;

			for (pointer it = other_.first, end = other_.last, in = m_buffer;
			     it != end;
			     ++it, ++in) {
				*it = *in;
			}
			*other_.last = 0;
		}

		if (first == other_.m_buffer) {
			last = last - first + m_buffer;
			first = m_buffer;
			capacity = c_nbuffer;

			for (pointer it = first, end = last, in = tbuffer;
			     it != end;
			     ++it, ++in) {
				*it = *in;
			}
			*last = 0;
		}
	}

	inline void string::replace(char replaceThis_, char replaceWith_, bool caseSensitive_ /* = true*/) {
		if (caseSensitive_) {
			for (unsigned i = 0; i < (unsigned) size(); ++i) {
				if (first[i] == replaceThis_) {
					first[i] = replaceWith_;
				}
			}
		} else {
			replaceThis_ = (char) ::tolower(replaceThis_);
			for (unsigned i = 0; i < (unsigned) size(); ++i) {
				if (tolower(first[i]) == replaceThis_) {
					first[i] = replaceWith_;
				}
			}
		}
	}

	inline string::size_type string::find_first_of(char const *tofind_, size_t startPos_) const {
		size_t const fc = strlen(tofind_);
		if (startPos_ == npos)
			startPos_ = 0;

		for (size_type i = startPos_; i < size(); ++i) {
			for (size_t j = 0; j < fc; ++j) {
				if (first[i] == tofind_[j]) {
					return i;
				}
			}
		}
		return npos;
	}

	inline string::size_type string::find_first_of(
			char const tofind_,
			size_t startPos_ /* = npos*/
	) const {
		if (startPos_ == npos)
			startPos_ = 0;

		for (size_type i = startPos_; i < size(); ++i) {
			if (first[i] == tofind_) {
				return i;
			}
		}
		return npos;
	}


	inline string::size_type string::find_first_not_of(
			char const *tofind_,
			size_t startPos_ /* = npos*/
	) const {
		size_t const fc = strlen(tofind_);
		if (startPos_ == npos)
			startPos_ = 0;

		for (size_type i = startPos_; i < size(); ++i) {
			for (size_t j = 0; j < fc; ++j) {
				if (first[i] == tofind_[j]) {
					break;
				}
				return i;
			}
		}
		return npos;
	}


	inline string::size_type string::find_last_of(
			char const tofind_,
			size_t startPos_ /* = npos*/
	) const {
		if (startPos_ == npos || startPos_ >= size() )
			startPos_ = size()-1;
		else
			startPos_ = size()  - startPos_;

		for (size_type i = startPos_; i >= 0; --i) {
			if (first[i] == tofind_) {
				return i;
			}
		}
		return npos;
	}

	inline string::size_type string::find_last(
			char c,
			size_t startPos /* = npos*/,
			bool caseSensitive /* = true*/) const {

		if (startPos >= size()) {
			startPos = size() - 1;
		}

		if (caseSensitive) {
			for (size_type i = startPos+1; i >= 1; --i) {
				if (first[i-1] == c) {
					return i-1;
				}
			}
		} else {
			c = (char) tolower(c);
			for (size_type i = startPos; i >= 1; --i) {
				if (tolower(first[i-1]) == c) {
					return i-1;
				}
			}
		}

		return npos;
	}


	inline string::size_type string::find_last(
			string_view const & str_, size_type startPos_ /* = npos*/, bool caseSensitive_ /* = true*/) const {
		if (str_.empty() || str_.size() > size()) {
			return npos;
		}
		if (startPos_ > size() - str_.size()) {
			startPos_ = size() - str_.size();
		}

		char f = str_[0];
		if (!caseSensitive_) {
			f = (char) tolower(f);
		}

		for (size_type i = startPos_+1; i >= 1; --i) {
			char c = first[i-1];
			if (!caseSensitive_) {
				c = (char) tolower(c);
			}

			if (c == f) {
				bool found = true;
				for (size_type j = 1; j < str_.size(); ++j) {
					c = first[i + j + 1];
					char d = str_[j];
					if (!caseSensitive_) {
						c = (char) tolower(c);
						d = (char) tolower(d);
					}

					if (c != d) {
						found = false;
						break;
					}
				}
				if (found) {
					return i - 1;
				}
			}
		}

		return npos;
	}


	inline void string::erase(size_t start_, size_t end_) {
		if (end_ == npos) {
			resize(start_);
			return;
		}
		size_t const d = end_ - start_;
		if (d > 0) {
			memmove(first + start_, first + start_ + d, d);
			resize(size() - d);
		}
	}


	inline string::size_type string::find(char c_, size_type startPos_, bool caseSensitive_ /* = true*/) const {
		if(startPos_ == npos) {
			startPos_ = 0;
		}

		if (caseSensitive_) {
			for (size_type i = startPos_; i < size(); ++i) {
				if (first[i] == c_) {
					return i;
				}
			}
		} else {
			c_ = (char) tolower(c_);
			for (size_type i = startPos_; i < size(); ++i) {
				if (tolower(first[i]) == c_) {
					return i;
				}
			}
		}

		return npos;
	}


	inline string::size_type string::find(string_view const &str_,
	                                        size_t startPos_,
	                                        bool caseSensitive_ /* = true*/) const {
		if (str_.empty() || str_.size() > size()) {
			return npos;
		}

		if(startPos_ == npos) {
			startPos_ = 0;
		}

		char f = str_[0];
		if (!caseSensitive_) {
			f = (char) tolower(f);
		}

		for (size_type i = startPos_; i <= size() - str_.size(); ++i) {
			char c = first[i];
			if (!caseSensitive_) {
				c = (char) tolower(c);
			}

			if (c == f) {
				size_type skip = npos;
				bool found = true;
				for (unsigned j = 1; j < (unsigned) str_.size(); ++j) {
					c = first[i + j];
					char d = str_[j];
					if (!caseSensitive_) {
						c = (char) tolower(c);
						d = (char) tolower(d);
					}

					if (skip == npos && c == f) {
						skip = i + j - 1;
					}

					if (c != d) {
						found = false;
						if (skip != npos) {
							i = skip;
						}
						break;
					}
				}
				if (found) {
					return i;
				}
			}
		}

		return npos;
	}

	inline string::size_type string::rfind(char c_, size_type pos_, bool caseSensitive_) const {

		if(pos_ == npos) {
			pos_ = size()-1;
		}

		if (caseSensitive_) {
			for (size_type i = pos_; i >= 0; --i) {
				if (first[i] == c_) {
					return i;
				}
			}
		} else {
			c_ = (char) tolower(c_);
			for (size_type i = pos_; i >= 0; --i) {
				if (tolower(first[i]) == c_) {
					return i;
				}
			}
		}

		return npos;
	}


	inline string::size_type string::rfind(string_view const & str_, size_type startPos_, bool caseSensitive_) const {
		if (str_.empty() || str_.size() > size()) {
			return npos;
		}

		if(str_.size() == 1) {
			return rfind(str_[0], startPos_);
		}

		if(startPos_ == npos) startPos_ = size()-1;

		char f = str_[str_.size()-1];
		if (!caseSensitive_) {
			f = (char) tolower(f);
		}

		for (size_type i = startPos_; i > str_.size(); --i) {
			char c = first[i];
			if (!caseSensitive_) {
				c = (char) tolower(c);
			}

			if (c == f) {
				size_type skip = npos;
				bool found = true;
				for (size_type j = str_.size() - 2; j >= 0 ; --j) {
					c = first[i - (str_.size()-1 - j)];
					char d = str_[j];
					if (!caseSensitive_) {
						c = (char) tolower(c);
						d = (char) tolower(d);
					}

					if (skip == npos && c == f) {
						skip = i - (str_.size()-1 - j);
					}

					if (c != d) {
						found = false;
						if (skip != npos) {
							i = skip;
						}
						break;
					}
				}
				if (found) {
					return i;
				}
			}
		}

		return npos;

	}

	inline int string::compare(char const *str_) const {
		return strcmp(c_str(), str_);
	}


	inline int string::compare(string const &str_) const {
		return strcmp(c_str(), str_.c_str());
	}

	inline char string::back() const {
		if (empty()) {
			return 0;
		} else {
			return *(c_str() + size() - 1);
		}
	}

	inline string string::to_lower() const {
		string ret = *this;
		for (unsigned i = 0; i < (unsigned) ret.size(); ++i) {
			ret.first[i] = (char) tolower(first[i]);
		}

		return ret;
	}

	inline string string::to_upper() const {
		string ret = *this;
		for (unsigned i = 0; i < (unsigned) ret.size(); ++i) {
			ret.first[i] = (char) toupper(first[i]);
		}

		return ret;
	}

	inline bool operator==(const string &lhs_, const string &rhs_) {
		const size_t lsize = lhs_.size(), rsize = rhs_.size();
		if (lsize != rsize) {
			return false;
		}

		// use memcmp - this is usually an intrinsic on most compilers
		return memcmp(lhs_.c_str(), rhs_.c_str(), lsize) == 0;
	}


	inline bool operator==(const string &lhs_, const string_view &rhs_) {
		const size_t lsize = lhs_.size(), rsize = rhs_.size();
		if (lsize != rsize) {
			return false;
		}

		// use memcmp - this is usually an intrinsic on most compilers
		return memcmp(lhs_.c_str(), rhs_.data(), lsize) == 0;
	}

	inline bool operator==(const string_view &rhs_, const string &lhs_) {
		const size_t lsize = lhs_.size(), rsize = rhs_.size();
		if (lsize != rsize) {
			return false;
		}

		// use memcmp - this is usually an intrinsic on most compilers
		return memcmp(lhs_.c_str(), rhs_.data(), lsize) == 0;
	}


	inline  bool operator==(const string &lhs_, const char *rhs_) {
		const size_t lsize = lhs_.size(), rsize = strlen(rhs_);
		if (lsize != rsize) {
			return false;
		}

		return memcmp(lhs_.c_str(), rhs_, lsize) == 0;
	}

	inline bool operator<(const string &lhs_, const string &rhs_) {
		const size_t lsize = lhs_.size(), rsize = rhs_.size();
		if (lsize != rsize) {
			return lsize < rsize;
		}
		return memcmp(lhs_.c_str(), rhs_.c_str(), lsize) < 0;
	}

	inline bool operator>(const string &lhs_, const string &rhs_) {
		const size_t lsize = lhs_.size(), rsize = rhs_.size();
		if (lsize != rsize) {
			return lsize > rsize;
		}
		return memcmp(lhs_.c_str(), rhs_.c_str(), lsize) > 0;
	}

	inline bool operator<=(const string &lhs_, const string &rhs_) {
		return !(lhs_ > rhs_);
	}


	inline bool operator>=(const string &lhs_, const string &rhs_) {
		return !(lhs_ < rhs_);
	}

	inline bool operator!=(const string &lhs_, const string &rhs_) {
		return !(lhs_ == rhs_);
	}

	inline bool operator!=(const string &lhs_, const string_view &rhs_) {
		return !(lhs_ == rhs_);
	}

	inline bool operator!=(const string_view &rhs_, const string &lhs_) {
		return !(lhs_ == rhs_);
	}

	inline bool operator!=(const string &lhs_, const char *rhs_) {
		return !(lhs_ == rhs_);
	}

	inline bool operator!=(const char *lhs_, const string &rhs_) {
		return !(rhs_ == lhs_);
	}


	inline string operator+(const string &lhs_, const string &rhs_) {
		string ret(lhs_);
		ret.append(rhs_.begin(), rhs_.end());
		return ret;
	}

	inline string operator+(const char *lhs_, const string &rhs_) {
		string ret(lhs_, rhs_.get_allocator());
		ret.append(rhs_.begin(), rhs_.end());
		return ret;
	}

	inline string &operator+=(string &lhs_, const string &rhs_) {
		lhs_.append(rhs_.begin(), rhs_.end());
		return lhs_;
	}

	inline string operator+(const string &lhs_, char const *cs_) {
		string ret(lhs_);

		ret.append(cs_, cs_ + strlen(cs_));
		return ret;
	}

	inline string &operator+=(string &lhs_, char const *cs_) {
		lhs_.append(cs_, cs_ + strlen(cs_));
		return lhs_;
	}

// basic to_string support, probably slow etc.
#define TO_STRING_GEN(TYPE, FMT) \
inline string to_string(TYPE const t_, Memory_Allocator* allocator_) { \
	char tmp[1024]; \
	sprintf(tmp, FMT, t_); \
	return string(tmp, allocator_); \
}

	TO_STRING_GEN(uint8_t, "%" PRIu8)
	TO_STRING_GEN(uint16_t, "%" PRIu16)
	TO_STRING_GEN(uint32_t, "%" PRIu32)
	TO_STRING_GEN(uint64_t, "%" PRIu64)
	TO_STRING_GEN(int8_t, "%" PRIi8)
	TO_STRING_GEN(int16_t, "%" PRIi16)
	TO_STRING_GEN(int32_t, "%" PRIi32)
	TO_STRING_GEN(int64_t, "%" PRIi64)
	TO_STRING_GEN(float, "%f")
	TO_STRING_GEN(double, "%f")

#undef TO_STRING_GEN

	template<>
	struct hash<string> {
			size_t operator()(const string &value_) {
				return hash_string(value_.data(), value_.size());
			}
	};
} // end namespace
