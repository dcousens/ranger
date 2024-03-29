#pragma once

#include <algorithm>
#include "ranger.hpp"

#ifdef __APPLE__
#include <machine/endian.h>
#else
#include "endian.h"
#endif

#if __BYTE_ORDER != __LITTLE_ENDIAN
#error "big endian architecture not supported"
#endif

namespace serial {
	template <typename E, bool BE = false, typename R>
	auto peek (R const& r) {
		using T = typename R::value_type;

		static_assert(std::is_same<T, uint8_t>::value, "Expected uint8_t elements");
		static_assert(sizeof(E) % sizeof(T) == 0, "Padding is unsupported");

		constexpr auto count = sizeof(E) / sizeof(T);

		E value;
		auto copy = ranger::range(r);
		auto ptr = reinterpret_cast<T*>(&value);

		if (BE) {
			for (size_t i = 0; i < count; ++i, copy.pop_front()) {
				ptr[count - 1 - i] = copy.front();
			}
		} else {
			for (size_t i = 0; i < count; ++i, copy.pop_front()) {
				ptr[i] = copy.front();
			}
		}

		return value;
	}

	template <typename E, bool BE = false, typename R>
	void place (R& r, E const value) {
		using T = typename R::value_type;

		static_assert(std::is_same<T, uint8_t>::value, "Expected uint8_t elements");
		static_assert(sizeof(E) % sizeof(T) == 0, "Padding is unsupported");

		constexpr auto count = sizeof(E) / sizeof(T);
		auto copy = ranger::range(r);
		auto ptr = reinterpret_cast<const T*>(&value);

		if (BE) {
			for (size_t i = 0; i < count; ++i, copy.pop_front()) {
				copy.front() = ptr[count - 1 - i];
			}
		} else {
			for (size_t i = 0; i < count; ++i, copy.pop_front()) {
				copy.front() = ptr[i];
			}
		}
	}

	template <typename E, bool BE = false, typename R>
	auto read (R& r) {
		using T = typename R::value_type;

		auto const e = peek<E, BE, R>(r);
		r = r.drop(sizeof(E) / sizeof(T));
		return e;
	}

	template <typename E, bool BE = false, typename R>
	void put (R& r, E const e) {
		using T = typename R::value_type;

		place<E, BE, R>(r, e);
		r = r.drop(sizeof(E) / sizeof(T));
	}

	// rvalue references wrappers
	template <typename E, bool BE = false, typename R> void place (R&& r, const E e) { place<E, BE, R>(r, e); }
	template <typename E, bool BE = false, typename R> auto read (R&& r) { return read<E, BE, R>(r); }
	template <typename E, bool BE = false, typename R> void put (R&& r, const E e) { put<E, BE, R>(r, e); }
}
