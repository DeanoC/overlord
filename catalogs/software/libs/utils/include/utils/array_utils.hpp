#include "core/core.h"
#include <array>
#include <type_traits>

namespace Utils {
		namespace detail {
				template<typename T, std::size_t...Is>
				std::array<T, sizeof...(Is)> make_array(const T &value, std::index_sequence<Is...>) {
					return {{(static_cast<void>(Is), value)...}};
				}
		}

		template<std::size_t N, typename T>
		std::array<T, N> make_array(const T &value) {
			return detail::make_array(value, std::make_index_sequence<N>());
		}
}