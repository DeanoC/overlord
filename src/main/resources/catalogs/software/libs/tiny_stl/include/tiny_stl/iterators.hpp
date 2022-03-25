
namespace tiny_stl {
struct base_iterator_tag; // can't increment
struct forward_iterator_tag;
struct reverse_iterator_tag;


template<typename T, typename tag>
struct iterator_impl {
		typedef T value_type;

		explicit iterator_impl(T* ptr_) : ptr(ptr_){}

		iterator_impl& operator=(const iterator_impl& rhs) { if(ptr != rhs.ptr) { ptr = rhs.ptr; } return *this; }
		T& operator*() const { return *ptr; }

		operator T*() const { return ptr; };
		operator void*() const { return ptr; };

		bool operator==(iterator_impl const& rhs) const { return ptr == rhs.ptr; };
		bool operator!=(iterator_impl const& rhs) const { return ptr != rhs.ptr; };
		bool operator<(iterator_impl const& rhs) const { return ptr < rhs.ptr; };
		value_type * operator->() { return ptr; };
		value_type const * operator->() const { return ptr; };

		template<typename T_, typename tag_>
		friend void swap(iterator_impl& lhs, iterator_impl& rhs);

		T* ptr;
};

template<typename T, typename tag>
void swap(iterator_impl<T, tag>& lhs, iterator_impl<T, tag>& rhs) {
	auto tmp = lhs;
	lhs = rhs;
	rhs = tmp;
}

template<typename T>
struct forward_iterator_impl : public iterator_impl<T, forward_iterator_tag> {
		typedef forward_iterator_impl tag;
		explicit forward_iterator_impl( T * ptr_) : iterator_impl<T, forward_iterator_tag>(ptr_){}
		explicit forward_iterator_impl( iterator_impl<T, base_iterator_tag> const & in) : forward_iterator_impl(in.ptr) {}

		explicit operator iterator_impl<T, base_iterator_tag>() const {
			return iterator_impl<T, base_iterator_tag>(iterator_impl<T, base_iterator_tag>::ptr);
		}

		// pre-increment
		forward_iterator_impl<T>& operator++() {
			++this->ptr;
			return *this;
		}
		// post-increment
		forward_iterator_impl const operator++(int) {
			forward_iterator_impl existing(*this);
			++this->ptr;
			return existing;
		}

};

template<typename T>
struct reverse_iterator_impl : public iterator_impl<T, reverse_iterator_tag> {
		typedef reverse_iterator_tag tag;
		explicit reverse_iterator_impl(T* ptr_) : iterator_impl<T, reverse_iterator_tag>(ptr_){}
		explicit reverse_iterator_impl( iterator_impl<T, base_iterator_tag> const & in) : reverse_iterator_impl(in.ptr) {}

		explicit operator iterator_impl<T, base_iterator_tag>() {
			return iterator_impl<T, base_iterator_tag>(iterator_impl<T, base_iterator_tag>::ptr);
		}

		// pre-increment
		reverse_iterator_impl& operator++() {
			--this->ptr;
			return *this;
		}
		// post-increment
		reverse_iterator_impl const operator++(int) {
			reverse_iterator_impl const existing(*this);
			--this->ptr;
			return existing;
		}
};


} // end namespace