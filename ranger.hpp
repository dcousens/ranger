#pragma once

#include <algorithm>
#include <cassert>
#include <iterator>
#include <type_traits>

namespace __ranger {
	template <typename R, typename F>
	auto pop_until (R& r, const F f) {
		if constexpr(not R::is_forward::value) {
			while (not r.empty()) {
				if (f(r.front())) break;
				r.pop_front();
			}
		} else {
			auto copy = r;

			while (not r.empty()) {
				if (f(r.front())) break;
				r.pop_front();
			}

			return R(copy.begin(), r.begin());
		}
	}

	template <typename R, bool Condition = R::is_forward::value>
	typename std::conditional_t<Condition, R, void>
	pop_front (R& r, const size_t un) {
		const auto n = static_cast<typename R::distance_type>(un);
		if constexpr(std::is_signed<typename R::distance_type>::value) {
			assert(n >= 0);
		}

		if constexpr(not R::is_forward::value) {
			if (r.empty()) return;
			std::advance(r._begin, n);
			return;
		} else {
			if (r.empty()) return r;
			auto it = r._begin;
			std::advance(r._begin, n);
			if constexpr(R::is_random_access::value) {
				if (r._begin > r._end) {
					r._begin = r._end;
				}
			}

			return R(it, r._begin);
		}
	}

	template <typename R, bool Condition = R::is_bidirectional::value>
	typename std::conditional_t<Condition, R, void>
	pop_back (R& r, const size_t un) {
		const auto n = static_cast<typename R::distance_type>(un);
		if constexpr(std::is_signed<typename R::distance_type>::value) {
			assert(n >= 0);
		}

		if constexpr(not R::is_forward::value) {
			if (r.empty()) return;
			std::advance(r._end, -n);
			return;
		} else {
			if (r.empty()) return r;
			auto it = r._end;
			std::advance(r._end, -n);
			if constexpr(R::is_random_access::value) {
				if (r._end < r._begin) {
					r._end = r._begin;
				}
			}

			return R(r._end, it);
		}
	}

	template <typename R>
	void put (R& r, typename R::value_type e) {
		r.front() = e;
		r.pop_front();
	}

	template <
		typename R,
		typename E,
		typename V = typename std::enable_if_t<
			std::is_same_v<typename R::value_type, typename E::value_type>,
			E
		>
	>
	void put (R& r, E e) {
		while (not e.empty()) {
			r.front() = e.front();
			r.pop_front();
			e.pop_front();
		}
	}

	template <typename I>
	struct Range {
		I _begin;
		I _end;

		using iterator = I;
		using value_type = typename std::remove_const_t<
			typename std::remove_reference_t<decltype(*I())>
		>;
		using distance_type = decltype(std::distance(I(), I()));
		using iterator_category = typename std::iterator_traits<I>::iterator_category;
		using is_bidirectional = std::is_base_of<std::bidirectional_iterator_tag, iterator_category>;
		using is_forward = std::is_base_of<std::forward_iterator_tag, iterator_category>;
		using is_input = std::is_base_of<std::input_iterator_tag, iterator_category>;
		using is_random_access = std::is_base_of<std::random_access_iterator_tag, iterator_category>;

		Range (I begin, I end) : _begin(begin), _end(end) {
			if constexpr(is_random_access::value) {
				assert(end >= begin);
			}
		}

		auto begin () const { return this->_begin; }
		auto end () const { return this->_end; }
		auto empty () const { return this->begin() == this->end(); }

		template <bool Condition = is_forward::value>
		typename std::enable_if_t<Condition, Range>
		drop (const size_t un) const {
			auto copy = *this;
			copy.pop_front(un);
			return copy;
		}

		template <bool Condition = is_bidirectional::value>
		typename std::enable_if_t<Condition, Range>
		drop_back (const size_t un) const {
			auto copy = *this;
			copy.pop_back(un);
			return copy;
		}

		template <typename F, bool Condition = is_forward::value>
		typename std::enable_if_t<Condition, Range>
		drop_until (const F f) const {
			auto copy = *this;
			copy.pop_until(f);
			return copy;
		}

		auto take (const size_t n) const {
			return Range(this->begin(), this->drop(n).begin());
		}

		auto take_back (const size_t un) const {
			return Range(this->drop_back(un).end(), this->end());
		}

		template <typename F>
		auto take_until (const F f) const {
			return Range(this->begin(), this->drop_until(f).begin());
		}

		auto& front () {
			assert(not this->empty());
			return *this->begin();
		}

		auto& front () const {
			assert(not this->empty());
			return *this->begin();
		}

		auto& back () {
			return this->take_back(1).front();
		}

		auto& back () const {
			return this->take_back(1).front();
		}

		template <bool Condition = std::is_pointer_v<I>>
		typename std::enable_if_t<Condition, I>
		data () {
			return this->begin();
		}

		template <bool Condition = is_random_access::value>
		typename std::enable_if_t<Condition, size_t>
		size () const {
			assert(this->end() >= this->begin());
			return static_cast<size_t>(std::distance(this->begin(), this->end()));
		}

		template <bool Condition = is_random_access::value>
		typename std::enable_if_t<Condition, value_type&>
		operator[] (const size_t i) {
			return this->drop(i).front();
		}

		template <bool Condition = is_random_access::value>
		typename std::enable_if_t<Condition, value_type>
		operator[] (const size_t i) const {
			return this->drop(i).front();
		}

		template <typename E, bool Condition = is_forward::value>
		typename std::enable_if_t<Condition, bool>
		operator< (const E& rhs) const {
			return std::lexicographical_compare(this->begin(), this->end(), rhs.begin(), rhs.end());
		}

		template <typename E, bool Condition = is_forward::value>
		typename std::enable_if_t<Condition, bool>
		operator== (const E& rhs) const {
			return std::equal(this->begin(), this->end(), rhs.begin(), rhs.end());
		}

		template <typename F>
		bool any (const F f) const {
			for (auto x : *this) {
				if (f(x)) return true;
			}

			return false;
		}

		template <typename F>
		bool all (const F f) const {
			for (auto x : *this) {
				if (not f(x)) return false;
			}

			return true;
		}

		template <typename F>
		size_t count (const F f) const {
			size_t result = 0;

			for (auto x : *this) {
				if (f(x)) ++result;
			}

			return result;
		}

		// mutators
		auto pop_back () {
			return __ranger::pop_back(*this, 1);
		}

		auto pop_back (const size_t un) {
			return __ranger::pop_back(*this, un);
		}

		auto pop_front () {
			return __ranger::pop_front(*this, 1);
		}

		auto pop_front (const size_t un) {
			return __ranger::pop_front(*this, un);
		}

		template <typename F>
		auto pop_until (const F f) {
			return __ranger::pop_until(*this, f);
		}

		template <typename E>
		void put (E e) { __ranger::put(*this, e); }
	};

	template <typename I, typename F>
	struct OrderedRange : public Range<I> {
		OrderedRange (I begin, I end) : Range<I>(begin, end) {}

		using value_type = typename Range<I>::value_type;

		auto contains (const value_type& value) const {
			return std::binary_search(this->begin(), this->end(), value, F());
		}

		auto lower_bound (const value_type& value) const {
			return std::lower_bound(this->begin(), this->end(), value, F());
		}

		auto upper_bound (const value_type& value) const {
			return std::upper_bound(this->begin(), this->end(), value, F());
		}
	};
}

namespace ranger {
	template <typename I> using range_t = __ranger::Range<I>;

	template <typename I>
	auto range (I begin, I end) {
		return range_t<I>(begin, end);
	}

	template <typename R>
	auto range (R& r) {
		using iterator = decltype(r.begin());
		return range_t<iterator>(r.begin(), r.end());
	}

	template <typename R>
	auto ptr_range (R& r) {
		using pointer = decltype(r.data());
		return range_t<pointer>(r.data(), r.data() + r.size());
	}

	inline auto zstr_range (const char* z) {
		auto r = z;
		while (*r != '\0') r++;
		return range(z, r);
	}

	template <typename R>
	auto reverse (R& r) {
		using reverse_iterator = std::reverse_iterator<decltype(r.begin())>;
		return range(reverse_iterator(r.end()), reverse_iterator(r.begin()));
	}

	template <typename F, typename R>
	auto ordered (R& r) {
		using iterator = decltype(r.begin());

		return __ranger::OrderedRange<iterator, F>(r.begin(), r.end());
	}

	template <typename R>
	auto ordered (R& r) {
		return ordered<std::less<>, R>(r);
	}

	template <typename I>
	typename std::enable_if_t<
		range_t<I>::is_input::value,
		range_t<I>
	>
	input_range (I begin) {
		return range_t<I>(begin, I{});
	}

	// rvalue references wrappers
	template <typename R> auto range (R&& r) { return range<R>(r); }
	template <typename R> auto ptr_range (R&& r) { return ptr_range<R>(r); }
	template <typename R> auto reverse (R&& r) { return reverse<R>(r); }
	template <typename R> auto ordered (R&& r) { return ordered<R>(r); }
	template <typename F, typename R> auto ordered (R&& r) { return ordered<R, F>(r); }
}
