#pragma once

#include "vector.hpp"
namespace tiny_stl {

		template<typename T, typename Container = vector<T>>
		class stack {
		public:
				stack(Memory_Allocator* allocator) : container(allocator) {}
				explicit stack(Container const &cont) : container(cont) {}

				WARN_UNUSED_RESULT auto top() -> T & { return container.back(); }
				WARN_UNUSED_RESULT auto top() const -> T const & { return container.back(); }
				WARN_UNUSED_RESULT auto empty() const -> bool { return container.empty(); }
				WARN_UNUSED_RESULT auto size() -> size_t const { return container.size(); }

				void push(T const & value) { container.push_back(value); }
				void push(T && value) { container.push_back(std::move(value)); }
				void pop() { container.pop_back(); }

				template< class... Args >
				void emplace( Args&&... args ) { container.emplace_back(std::forward<Args>(args)...); }
				template< class... Args >
				auto emplace( Args&&... args ) -> decltype(auto) { container.emplace_back(std::forward<Args>(args)...); }

				void swap( stack& other ) noexcept { std::swap(container, other.container); }
		private:
				Container container;
		};
}