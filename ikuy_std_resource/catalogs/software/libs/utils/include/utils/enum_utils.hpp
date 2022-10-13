#include "core/core.h"
#include "core/utils.hpp"
#include <type_traits>

// overload this for a scoped enum you want to treat as a bit mask enum
template<typename E>
constexpr auto is_bitmask_enum(E) -> bool
{
	// normal enums allow the bitwise ops regardless
	// for scoped enums (enum class) only if this function is overridden
	if constexpr (Core::is_scoped_enum<E>::value == false)
	{
		return true;
	} else
	{
		return false;
	}
}

// the enum class bitwise operators are boosted outside the Core namespace
// so that they operate the same as the normal operators do
template<typename E, typename = typename std::enable_if<std::is_enum<E>{}>::type>
constexpr auto operator|(E lhs, E rhs)
{
	if constexpr (is_bitmask_enum(E{}))
	{
		using basetype = typename std::underlying_type<E>::type;
		return static_cast<E>(
				static_cast<basetype>(lhs) | static_cast<basetype>(rhs));
	}
}

template<typename E, typename = typename std::enable_if<std::is_enum<E>{}>::type>
constexpr auto operator&(E lhs, E rhs)
{
	if constexpr (is_bitmask_enum(E{}))
	{
		using basetype = typename std::underlying_type<E>::type;
		return static_cast<E>(
				static_cast<basetype>(lhs) & static_cast<basetype>(rhs));
	}
}

template<typename E, typename = typename std::enable_if<std::is_enum<E>{}>::type>
constexpr auto operator^(E lhs, E rhs)
{
	if constexpr (is_bitmask_enum(E{}))
	{
		using basetype = typename std::underlying_type<E>::type;
		return static_cast<E>(
				static_cast<basetype>(lhs) ^ static_cast<basetype>(rhs));
	}
}

template<typename E, typename = typename std::enable_if<std::is_enum<E>{}>::type>
constexpr auto operator~(E lhs)
{
	if constexpr (is_bitmask_enum(E{}))
	{
		using basetype = typename std::underlying_type<E>::type;
		return static_cast<E>(~static_cast<basetype>(lhs));
	}
}


template<typename E, typename = typename std::enable_if<std::is_enum<E>{}>::type>
constexpr auto operator|=(E& lhs, E rhs)
{
	if constexpr (is_bitmask_enum(E{}))
	{
		using basetype = typename std::underlying_type<E>::type;
		lhs = static_cast<E>(
				static_cast<basetype>(lhs) | static_cast<basetype>(rhs));
		return lhs;
	}
}

template<typename E, typename = typename std::enable_if<std::is_enum<E>{}>::type>
constexpr auto operator&=(E& lhs, E rhs)
{
	if constexpr (is_bitmask_enum(E{}))
	{
		using basetype = typename std::underlying_type<E>::type;
		lhs = static_cast<E>(
				static_cast<basetype>(lhs) & static_cast<basetype>(rhs));
		return lhs;
	}
}

template<typename E, typename = typename std::enable_if<std::is_enum<E>{}>::type>
constexpr auto operator^=(E& lhs, E rhs)
{
	if constexpr (is_bitmask_enum(E{}))
	{
		using basetype = typename std::underlying_type<E>::type;
		lhs = static_cast<E>(
				static_cast<basetype>(lhs) ^ static_cast<basetype>(rhs));
		return lhs;
	}
}


namespace Utils {

		template<typename E, typename = typename std::enable_if<std::is_enum<E>{}>::type>
		constexpr auto test_equal(E lhs, E rhs)
		{
			if constexpr (is_bitmask_enum(E{}))
			{
				return (lhs & rhs) == rhs;
			}
		}

		template<typename E, typename = typename std::enable_if<std::is_enum<E>{}>::type>
		constexpr auto test_any(E lhs, E rhs)
		{
			if constexpr (is_bitmask_enum(E{}))
			{
				return bool(lhs & rhs);
			}
		}


		template<typename E, typename = typename std::enable_if<std::is_enum<E>{}>::type>
		constexpr auto zero()
		{
			return static_cast<E>(0);
		}

		template<typename E, typename = typename std::enable_if<std::is_enum<E>{}>::type>
		constexpr auto from_uint(typename std::underlying_type<E>::type v)
		{
			if constexpr (is_bitmask_enum(E{}))
			{
				return static_cast<E>(v);
			}
		}

		template<typename E, typename = typename std::enable_if<std::is_enum<E>{}>::type>
		constexpr auto to_uint(E e)
		{
			if constexpr (is_bitmask_enum(E{}))
			{
				using basetype = typename std::underlying_type<E>::type;
				return static_cast<basetype>(e);
			}
		}
}
