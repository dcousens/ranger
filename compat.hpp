#pragma once

#include <charconv>
#include <system_error>

namespace compat {
	template <typename R, typename E>
	auto put_to_chars (R& r, E const e) {
		auto const result = std::to_chars(r.data(), r.data() + r.size(), e);
		if (result.ec != std::errc()) return false;

		r = R(result.ptr, r.end());
		return true;
	}

	template <typename R, typename E>
	auto read_from_chars (R& r, E value) {
		auto result = std::from_chars(r.data(), r.data() + r.size(), value);
		if (result.ec != std::errc()) return value;

		// TODO: refactor
		while (not r.empty()) {
			if (r.data() == result.ptr) break;
			r.pop_front();
		}

		return value;
	}
}
