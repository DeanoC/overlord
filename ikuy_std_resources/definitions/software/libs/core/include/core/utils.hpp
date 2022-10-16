#pragma once
#include "core/core.h"

// TODO add type_traits to tiny_stl
#if CPU_host
#include <type_traits>
#endif

namespace Core {

	/// \brief	Align input to align val.
	/// \param	k		The value to align.
	/// \param	align	The alignment boundary.
	/// \return	k aligned to align.
	template<typename T, typename T2>
	inline T alignTo(T k, T2 align) {
		return ((k + align - 1) & ~(align - 1));
	}

#if CPU_host
/// \brief	return the bit of the number.
	/// \param	num	returns the bit representing this index.
	template<typename T>
	constexpr auto Bit(T num) -> T {
		static_assert(std::is_integral<T>::value, "Integral required.");
		return (T(1) << (num));
	}

	// is an enum a scoped enum (enum class) or an old-fashioned one?
	// from StackOverflow https://stackoverflow.com/questions/15586163/c11-type-trait-to-differentiate-between-enum-class-and-regular-enum
	template<typename E>
	using is_scoped_enum = std::integral_constant<bool, std::is_enum<E>::value && !std::is_convertible<E, int>::value>;
#endif

} // end namespace Core


