#include "core/core.h"

namespace tiny_stl {
		template<typename T1, typename T2>
		struct pair {
				T1 first;
				T2 second;

				pair() : first{}, second{} {}
				pair(T1 first_, T2 second_) : first(first_), second(second_) {}
				pair(pair<T1, T2> const &other_) : first(other_.first), second(other_.second) {}

				auto make_pair(T1, T2) -> pair;

				auto operator=(pair<T1, T2> const &other_) -> pair {
					first = other_.first;
					second = other_.second;
					return *this;
				}

				auto operator==(pair<T1, T2> const &other_) -> bool {
					return (first == other_.first && second == other_.second);
				}

				auto operator!=(pair<T1, T2> const &other_) -> bool {
					return (first != other_.first || second != other_.second);
				}

				auto operator>(const pair<T1, T2> &other_) -> bool {
					return (this->first > other_.first && this->second > other_.second);
				}

				auto operator<(const pair<T1, T2> &other_) -> bool{
					return (this->first < other_.first && this->second < other_.second);
				}

				auto swap(pair<T1, T2> &other_) -> pair {
					T1 tmp1 = first;
					first = other_.first;
					other_.first = tmp1;
					T2 tmp2 = second;
					second = other_.second;
					other_.second = tmp2;
					return *this;
				}

				auto swap(pair<T1, T2> &, pair<T1, T2> &) -> pair;
		};

		template<typename T1, typename T2>
		auto make_pair(T1 const& first_, T2 const& second_) -> pair<T1, T2> {
			return pair<T1, T2>(first_, second_);
		}

		template<typename T1, typename T2>
		void swap(pair<T1, T2> &lhs_, pair<T1, T2> &rhs_) {
			lhs_.swap(rhs_);
		}
}